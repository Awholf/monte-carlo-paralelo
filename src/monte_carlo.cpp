#include "../include/simulacion.h"
#include <random>
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include <chrono> // Necesario para medir el tiempo_ms

using namespace std;
using namespace std::chrono;

ResultadoSimulacion monte_carlo_secuencial(const ParametrosOpcion& p, const ConfigSimulacion& cfg) {
    // 1. Iniciar el cronómetro
    auto inicio = high_resolution_clock::now();

    // 2. Configurar el generador usando la semilla compartida
    mt19937 gen(cfg.semilla); 
    normal_distribution<> d(0.0, 1.0);

    double suma_payoffs = 0.0;
    double suma_payoffs_cuadrados = 0.0; // Necesario para el error estándar

    // 3. Bucle de simulación (usando long long como en ConfigSimulacion)
    for (long long i = 0; i < cfg.num_simulaciones; ++i) {
        double z = d(gen);
        
        // Calcular S_T (Precio en el vencimiento)
        double S_T = p.S0 * exp((p.r - 0.5 * p.sigma * p.sigma) * p.T + p.sigma * sqrt(p.T) * z);
        
        // Calcular el payoff dependiendo de si es CALL o PUT
        double payoff = 0.0;
        if (p.tipo == TipoOpcion::CALL) {
            payoff = max(S_T - p.K, 0.0);
        } else {
            payoff = max(p.K - S_T, 0.0);
        }
        
        suma_payoffs += payoff;
        suma_payoffs_cuadrados += payoff * payoff;
    }

    // 4. Calcular el precio estimado
    double precio_promedio = suma_payoffs / cfg.num_simulaciones;
    double factor_descuento = exp(-p.r * p.T);
    double precio_estimado = precio_promedio * factor_descuento;

    // 5. Calcular el error estándar
    double varianza = (suma_payoffs_cuadrados - (suma_payoffs * suma_payoffs) / cfg.num_simulaciones) / (cfg.num_simulaciones - 1.0);
    double error_estandar = sqrt(varianza) * factor_descuento / sqrt(cfg.num_simulaciones);

    // 6. Detener el cronómetro y calcular el tiempo en milisegundos
    auto fin = high_resolution_clock::now();
    double tiempo_ms = duration<double, milli>(fin - inicio).count();

    printf("Secuencial -> Precio: %f | Error: %f | Tiempo: %.2f ms\n", precio_estimado, error_estandar, tiempo_ms);

    // 7. Llenar el resultado estructurado
    ResultadoSimulacion resultado;
    resultado.precio = precio_estimado;
    resultado.error_std = error_estandar;
    resultado.tiempo_ms = tiempo_ms;

    return resultado;
}