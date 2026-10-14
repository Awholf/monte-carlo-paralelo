// monte_carlo_omp.cpp — Monte Carlo PARALELO con OpenMP
// Responsable: Jonathan
// NOTA: versión PROVISIONAL creada por Kiltom para poder integrar.
// Jonathan reemplaza este archivo respetando la firma de simulacion.h.
#include <omp.h>

#include <algorithm>
#include <cmath>
#include <random>

#include "simulacion.h"

ResultadoSimulacion monte_carlo_paralelo(const ParametrosOpcion& p,
                                         const ConfigSimulacion& cfg) {
    const double inicio = omp_get_wtime();

    const double drift = (p.r - 0.5 * p.sigma * p.sigma) * p.T;
    const double vol = p.sigma * std::sqrt(p.T);
    double suma = 0.0, suma2 = 0.0;

    #pragma omp parallel num_threads(cfg.num_hilos) reduction(+ : suma, suma2)
    {
        // Generador independiente por hilo (semilla distinta)
        std::mt19937_64 gen(cfg.semilla + 1000003ULL * (omp_get_thread_num() + 1));
        std::normal_distribution<double> normal(0.0, 1.0);

        #pragma omp for schedule(static)
        for (long long i = 0; i < cfg.num_simulaciones; ++i) {
            double ST = p.S0 * std::exp(drift + vol * normal(gen));
            double payoff = (p.tipo == TipoOpcion::CALL) ? std::max(ST - p.K, 0.0)
                                                         : std::max(p.K - ST, 0.0);
            suma += payoff;
            suma2 += payoff * payoff;
        }
    }

    const double n = static_cast<double>(cfg.num_simulaciones);
    const double desc = std::exp(-p.r * p.T);
    const double media = suma / n;
    const double var = std::max(suma2 / n - media * media, 0.0);

    ResultadoSimulacion res;
    res.precio = desc * media;
    res.error_std = desc * std::sqrt(var / n);
    res.tiempo_ms = (omp_get_wtime() - inicio) * 1000.0;
    return res;
}
