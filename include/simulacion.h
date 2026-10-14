// simulacion.h — Interfaz compartida (contrato entre módulos)
// Kiltom define las firmas; Leonardo y Jonathan las implementan.
#ifndef SIMULACION_H
#define SIMULACION_H

#include "parametros.h"

// Implementa Leonardo — src/monte_carlo.cpp
ResultadoSimulacion monte_carlo_secuencial(const ParametrosOpcion& p,
                                           const ConfigSimulacion& cfg);

// Implementa Leonardo — src/black_scholes.cpp
double black_scholes(const ParametrosOpcion& p);

// Implementa Jonathan — src/monte_carlo_omp.cpp
ResultadoSimulacion monte_carlo_paralelo(const ParametrosOpcion& p,
                                         const ConfigSimulacion& cfg);

#endif  // SIMULACION_H
