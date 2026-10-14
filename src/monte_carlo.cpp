// NOTA: versión PROVISIONAL 
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

#include "simulacion.h"

ResultadoSimulacion monte_carlo_secuencial(const ParametrosOpcion& p,
                                           const ConfigSimulacion& cfg) {
    auto inicio = std::chrono::high_resolution_clock::now();

    std::mt19937_64 gen(cfg.semilla);
    std::normal_distribution<double> normal(0.0, 1.0);

    const double drift = (p.r - 0.5 * p.sigma * p.sigma) * p.T;
    const double vol = p.sigma * std::sqrt(p.T);
    double suma = 0.0, suma2 = 0.0;

    for (long long i = 0; i < cfg.num_simulaciones; ++i) {
        double ST = p.S0 * std::exp(drift + vol * normal(gen));
        double payoff = (p.tipo == TipoOpcion::CALL) ? std::max(ST - p.K, 0.0)
                                                     : std::max(p.K - ST, 0.0);
        suma += payoff;
        suma2 += payoff * payoff;
    }

    const double n = static_cast<double>(cfg.num_simulaciones);
    const double desc = std::exp(-p.r * p.T);
    const double media = suma / n;
    const double var = std::max(suma2 / n - media * media, 0.0);

    ResultadoSimulacion res;
    res.precio = desc * media;
    res.error_std = desc * std::sqrt(var / n);
    res.tiempo_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - inicio).count();
    return res;
}
