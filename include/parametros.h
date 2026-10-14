// parametros.h — Estructuras compartidas entre módulos
// Responsable: Kiltom
#ifndef PARAMETROS_H
#define PARAMETROS_H

#include <string>

// Tipo de opción europea
enum class TipoOpcion { CALL, PUT };

// Modo de ejecución del simulador
enum class ModoEjecucion { SECUENCIAL, PARALELO, AMBOS };

// Parámetros financieros de la opción
struct ParametrosOpcion {
    double S0    = 100.0;  // Precio inicial del activo
    double K     = 100.0;  // Precio de ejercicio (strike)
    double r     = 0.05;   // Tasa libre de riesgo (anual)
    double sigma = 0.20;   // Volatilidad (anual)
    double T     = 1.0;    // Tiempo al vencimiento (años)
    TipoOpcion tipo = TipoOpcion::CALL;
};

// Configuración de la simulación
struct ConfigSimulacion {
    long long num_simulaciones = 1000000;  // Número de trayectorias
    int num_hilos = 4;                     // Hilos OpenMP (solo paralelo)
    unsigned long long semilla = 42;       // Semilla base del generador
    ModoEjecucion modo = ModoEjecucion::AMBOS;
};

// Resultado que devuelve cada simulador
struct ResultadoSimulacion {
    double precio     = 0.0;  // Precio estimado de la opción
    double error_std  = 0.0;  // Error estándar de la estimación
    double tiempo_ms  = 0.0;  // Tiempo de ejecución en milisegundos
};

inline std::string tipo_a_texto(TipoOpcion t) {
    return t == TipoOpcion::CALL ? "call" : "put";
}

#endif  // PARAMETROS_H
