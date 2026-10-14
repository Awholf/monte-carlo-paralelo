// black_scholes.cpp — Fórmula analítica de Black-Scholes
// Responsable: Leonardo
// NOTA: versión PROVISIONAL creada por Kiltom para poder integrar.
#include <cmath>

#include "simulacion.h"

static double normal_cdf(double x) { return 0.5 * std::erfc(-x / std::sqrt(2.0)); }

double black_scholes(const ParametrosOpcion& p) {
    const double d1 = (std::log(p.S0 / p.K) + (p.r + 0.5 * p.sigma * p.sigma) * p.T) /
                      (p.sigma * std::sqrt(p.T));
    const double d2 = d1 - p.sigma * std::sqrt(p.T);
    const double desc = std::exp(-p.r * p.T);
    if (p.tipo == TipoOpcion::CALL)
        return p.S0 * normal_cdf(d1) - p.K * desc * normal_cdf(d2);
    return p.K * desc * normal_cdf(-d2) - p.S0 * normal_cdf(-d1);
}
