
/*
 * Archivo: monte_carlo_omp.cpp
 * Responsable: Jhonatan
 * Proyecto: Simulacion Paralela de Monte Carlo
 *
 * Objetivo:
 * Calcular el precio de una opcion europea CALL o PUT
 * utilizando simulaciones de Monte Carlo en paralelo.
 *
 * Tecnologia: C++17 y OpenMP
 */

#include <omp.h>
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

#include "simulacion.h"

// Implementacion paralela de Monte Carlo
ResultadoSimulacion monte_carlo_paralelo(
    const ParametrosOpcion& p,
    const ConfigSimulacion& cfg
) {
    // 1. Verificar que los parametros sean validos
    if (cfg.num_simulaciones <= 0 || cfg.num_hilos <= 0) {
        throw std::invalid_argument(
            "Las simulaciones y los hilos deben ser mayores que cero."
        );
    }

    if (!std::isfinite(p.S0) || !std::isfinite(p.K) ||
        !std::isfinite(p.r) || !std::isfinite(p.sigma) ||
        !std::isfinite(p.T) ||
        p.S0 <= 0 || p.K <= 0 || p.sigma <= 0 || p.T <= 0) {
        throw std::invalid_argument(
            "Parametros financieros invalidos."
        );
    }

    // 2. Iniciar la medicion del tiempo
    const double inicio = omp_get_wtime();

    // 3. Calcular parametros del modelo financiero
    const double drift =
        (p.r - 0.5 * p.sigma * p.sigma) * p.T;

    const double volatilidad =
        p.sigma * std::sqrt(p.T);

    // Variables acumuladoras
    double suma = 0.0;
    double suma_cuadrados = 0.0;

    // 4. Crear la region paralela
    // reduction evita condiciones de carrera al sumar
    #pragma omp parallel num_threads(cfg.num_hilos) \
        reduction(+:suma, suma_cuadrados)
    {
        // Cada hilo tiene su propio generador aleatorio
        const int id_hilo = omp_get_thread_num();

        const unsigned long long semilla_hilo =
            cfg.semilla + 1000003ULL * (id_hilo + 1);

        std::mt19937_64 generador(semilla_hilo);
        std::normal_distribution<double> normal(0.0, 1.0);

        // 5. Distribuir las simulaciones entre los hilos
        #pragma omp for schedule(static)
        for (long long i = 0; i < cfg.num_simulaciones; ++i) {

            // Generar un valor aleatorio normal
            const double z = normal(generador);

            // Simular el precio futuro de la accion
            const double precio_futuro =
                p.S0 * std::exp(drift + volatilidad * z);

            // Calcular beneficio segun el tipo de opcion
            double beneficio;

            if (p.tipo == TipoOpcion::CALL) {
                beneficio = std::max(
                    precio_futuro - p.K, 0.0
                );
            } else {
                beneficio = std::max(
                    p.K - precio_futuro, 0.0
                );
            }

            // Acumulaciones protegidas mediante reduction
            suma += beneficio;
            suma_cuadrados += beneficio * beneficio;
        }
    }

    // 6. Calcular promedio de las simulaciones
    const double n =
        static_cast<double>(cfg.num_simulaciones);

    const double promedio = suma / n;

    // Factor de descuento financiero
    const double descuento = std::exp(-p.r * p.T);

    // Calcular el precio estimado de la opcion
    const double precio_estimado = descuento * promedio;

    // 7. Calcular varianza y error estandar
    const double varianza = std::max(
        (suma_cuadrados / n) - (promedio * promedio),
        0.0
    );

    const double error_estandar =
        descuento * std::sqrt(varianza / n);

    // 8. Finalizar medicion del tiempo
    const double tiempo_ms =
        (omp_get_wtime() - inicio) * 1000.0;

    // 9. Devolver los resultados
    ResultadoSimulacion resultado;

    resultado.precio = precio_estimado;
    resultado.error_std = error_estandar;
    resultado.tiempo_ms = tiempo_ms;

    return resultado;
}
