#include "../include/simulacion.h"
#include <cmath>
#include <algorithm>

using namespace std;

// Función auxiliar para calcular la Distribución Normal Acumulada (CDF)
// Utiliza la función de error complementario (erfc) de la librería estándar de C++
double normal_cdf(double x) {
    return 0.5 * erfc(-x / sqrt(2.0));
}

double black_scholes(const ParametrosOpcion& p) {
    // Protección en caso de que el tiempo al vencimiento sea 0
    if (p.T <= 0.0) {
        if (p.tipo == TipoOpcion::CALL) return max(p.S0 - p.K, 0.0);
        else return max(p.K - p.S0, 0.0);
    }

    // Calcular d1 y d2
    double d1 = (log(p.S0 / p.K) + (p.r + 0.5 * p.sigma * p.sigma) * p.T) / (p.sigma * sqrt(p.T));
    double d2 = d1 - p.sigma * sqrt(p.T);

    // Calcular el precio dependiendo de si es CALL o PUT
    double precio_teorico = 0.0;
    
    if (p.tipo == TipoOpcion::CALL) {
        precio_teorico = p.S0 * normal_cdf(d1) - p.K * exp(-p.r * p.T) * normal_cdf(d2);
    } else { // Si es PUT
        precio_teorico = p.K * exp(-p.r * p.T) * normal_cdf(-d2) - p.S0 * normal_cdf(-d1);
    }

    return precio_teorico;
}