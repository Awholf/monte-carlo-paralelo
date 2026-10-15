// main.cpp — Interfaz de consola, validación e integración
// Responsable: Kiltom
//
// Dos formas de uso:
//   1) Interactivo:   ./monte_carlo
//   2) Por argumentos (lo usa benchmark.py):
//      ./monte_carlo --modo omp --sims 1000000 --hilos 4 --csv
//
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "parametros.h"
#include "simulacion.h"

// ---------------------------------------------------------------
// Lectura validada desde consola
// ---------------------------------------------------------------
template <typename T>
T leer_valor(const std::string& mensaje, T valor_defecto, T minimo, T maximo) {
    while (true) {
        std::cout << mensaje << " [" << valor_defecto << "]: ";
        std::string linea;
        if (!std::getline(std::cin, linea)) return valor_defecto;
        if (linea.empty()) return valor_defecto;
        try {
            size_t pos = 0;
            double v = std::stod(linea, &pos);
            if (pos != linea.size()) throw std::invalid_argument("basura");
            if (v < static_cast<double>(minimo) || v > static_cast<double>(maximo)) {
                std::cout << "  Valor fuera de rango (" << minimo << " - "
                          << maximo << "). Intente de nuevo.\n";
                continue;
            }
            return static_cast<T>(v);
        } catch (...) {
            std::cout << "  Entrada no válida. Ingrese un número.\n";
        }
    }
}

int leer_opcion(const std::string& mensaje, int minimo, int maximo, int defecto) {
    return leer_valor<int>(mensaje, defecto, minimo, maximo);
}

// ---------------------------------------------------------------
// Validación de parámetros (se usa en ambos modos de entrada)
// ---------------------------------------------------------------
void validar(const ParametrosOpcion& p, const ConfigSimulacion& c) {
    if (p.S0 <= 0)      throw std::invalid_argument("S0 debe ser > 0");
    if (p.K <= 0)       throw std::invalid_argument("K debe ser > 0");
    if (p.sigma <= 0)   throw std::invalid_argument("sigma debe ser > 0");
    if (p.T <= 0)       throw std::invalid_argument("T debe ser > 0");
    if (p.r < -1 || p.r > 1) throw std::invalid_argument("r debe estar entre -1 y 1");
    if (c.num_simulaciones < 1) throw std::invalid_argument("sims debe ser >= 1");
    if (c.num_hilos < 1)        throw std::invalid_argument("hilos debe ser >= 1");
}

// ---------------------------------------------------------------
// Modo interactivo
// ---------------------------------------------------------------
void pedir_datos(ParametrosOpcion& p, ConfigSimulacion& c) {
    std::cout << "\n=== Parámetros de la opción europea ===\n";
    p.S0    = leer_valor<double>("Precio inicial del activo S0", p.S0, 0.01, 1e9);
    p.K     = leer_valor<double>("Precio de ejercicio K", p.K, 0.01, 1e9);
    p.r     = leer_valor<double>("Tasa libre de riesgo r (ej. 0.05)", p.r, -1.0, 1.0);
    p.sigma = leer_valor<double>("Volatilidad sigma (ej. 0.20)", p.sigma, 0.0001, 5.0);
    p.T     = leer_valor<double>("Tiempo al vencimiento T (años)", p.T, 0.001, 100.0);
    int t   = leer_opcion("Tipo de opción (1 = Call, 2 = Put)", 1, 2, 1);
    p.tipo  = (t == 1) ? TipoOpcion::CALL : TipoOpcion::PUT;

    std::cout << "\n=== Configuración de la simulación ===\n";
    c.num_simulaciones = leer_valor<long long>("Número de simulaciones",
                                               c.num_simulaciones, 1LL, 2000000000LL);
    int m = leer_opcion("Modo (1 = Secuencial, 2 = Paralelo, 3 = Ambos)", 1, 3, 3);
    c.modo = (m == 1) ? ModoEjecucion::SECUENCIAL
           : (m == 2) ? ModoEjecucion::PARALELO
                      : ModoEjecucion::AMBOS;
    if (c.modo != ModoEjecucion::SECUENCIAL)
        c.num_hilos = leer_opcion("Número de hilos", 1, 256, c.num_hilos);
}

void mostrar_resultados(const ParametrosOpcion& p, const ConfigSimulacion& c) {
    const double bs = black_scholes(p);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n=============== RESULTADOS ===============\n";
    std::cout << "Opción " << tipo_a_texto(p.tipo) << " | S0=" << p.S0 << " K=" << p.K
              << " r=" << p.r << " sigma=" << p.sigma << " T=" << p.T << "\n";
    std::cout << "Simulaciones: " << c.num_simulaciones << "\n";
    std::cout << "Black-Scholes (analítico): " << bs << "\n\n";

    ResultadoSimulacion seq{}, par{};
    bool hay_seq = false, hay_par = false;

    if (c.modo != ModoEjecucion::PARALELO) {
        seq = monte_carlo_secuencial(p, c);
        hay_seq = true;
        std::cout << "[Secuencial] precio = " << seq.precio
                  << "  ± " << 1.96 * seq.error_std << " (IC 95%)"
                  << "  | error vs BS = " << std::abs(seq.precio - bs)
                  << "  | tiempo = " << std::setprecision(2) << seq.tiempo_ms << " ms\n"
                  << std::setprecision(6);
    }
    if (c.modo != ModoEjecucion::SECUENCIAL) {
        par = monte_carlo_paralelo(p, c);
        hay_par = true;
        std::cout << "[Paralelo " << c.num_hilos << " hilos] precio = " << par.precio
                  << "  ± " << 1.96 * par.error_std << " (IC 95%)"
                  << "  | error vs BS = " << std::abs(par.precio - bs)
                  << "  | tiempo = " << std::setprecision(2) << par.tiempo_ms << " ms\n";
    }
    if (hay_seq && hay_par && par.tiempo_ms > 0) {
        std::cout << std::setprecision(2)
                  << "\nSpeedup = " << seq.tiempo_ms / par.tiempo_ms << "x\n";
    }
    std::cout << "==========================================\n";
}

// ---------------------------------------------------------------
// Modo por argumentos (para benchmark.py)
// Imprime: modo,hilos,simulaciones,precio,error_std,black_scholes,tiempo_ms
// ---------------------------------------------------------------
void uso() {
    std::cout <<
        "Uso: monte_carlo [opciones]\n"
        "  (sin opciones)     modo interactivo\n"
        "  --modo seq|omp     tipo de ejecución\n"
        "  --sims N           número de simulaciones\n"
        "  --hilos H          hilos OpenMP\n"
        "  --semilla S        semilla base\n"
        "  --S0 --K --r --sigma --T   parámetros financieros\n"
        "  --tipo call|put\n"
        "  --csv              salida en una línea CSV\n";
}

int ejecutar_por_argumentos(int argc, char** argv) {
    ParametrosOpcion p;
    ConfigSimulacion c;
    c.modo = ModoEjecucion::SECUENCIAL;
    bool csv = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto sig = [&]() -> std::string {
            if (i + 1 >= argc) throw std::invalid_argument("Falta valor para " + a);
            return argv[++i];
        };
        if      (a == "--modo")    { std::string m = sig();
                                     if (m == "seq") c.modo = ModoEjecucion::SECUENCIAL;
                                     else if (m == "omp") c.modo = ModoEjecucion::PARALELO;
                                     else throw std::invalid_argument("modo debe ser seq u omp"); }
        else if (a == "--sims")    c.num_simulaciones = std::stoll(sig());
        else if (a == "--hilos")   c.num_hilos = std::stoi(sig());
        else if (a == "--semilla") c.semilla = std::stoull(sig());
        else if (a == "--S0")      p.S0 = std::stod(sig());
        else if (a == "--K")       p.K = std::stod(sig());
        else if (a == "--r")       p.r = std::stod(sig());
        else if (a == "--sigma")   p.sigma = std::stod(sig());
        else if (a == "--T")       p.T = std::stod(sig());
        else if (a == "--tipo")    { std::string t = sig();
                                     p.tipo = (t == "put") ? TipoOpcion::PUT : TipoOpcion::CALL; }
        else if (a == "--csv")     csv = true;
        else if (a == "--help" || a == "-h") { uso(); return 0; }
        else throw std::invalid_argument("Argumento desconocido: " + a);
    }
    validar(p, c);

    const bool es_seq = (c.modo == ModoEjecucion::SECUENCIAL);
    ResultadoSimulacion res = es_seq ? monte_carlo_secuencial(p, c)
                                     : monte_carlo_paralelo(p, c);
    const double bs = black_scholes(p);
    const int hilos = es_seq ? 1 : c.num_hilos;

    std::cout << std::fixed << std::setprecision(6);
    if (csv) {
        std::cout << (es_seq ? "seq" : "omp") << "," << hilos << ","
                  << c.num_simulaciones << "," << res.precio << "," << res.error_std
                  << "," << bs << "," << res.tiempo_ms << "\n";
    } else {
        std::cout << "Modo: " << (es_seq ? "secuencial" : "paralelo") << " | hilos: " << hilos
                  << "\nPrecio MC: " << res.precio << " (error std " << res.error_std << ")"
                  << "\nBlack-Scholes: " << bs
                  << "\nTiempo: " << res.tiempo_ms << " ms\n";
    }
    return 0;
}

// ---------------------------------------------------------------
int main(int argc, char** argv) {
    try {
        if (argc > 1) return ejecutar_por_argumentos(argc, argv);

        std::cout << "==========================================\n"
                  << " Simulacion Monte Carlo - Opcion Europea\n"
                  << " Programacion Concurrente y Paralela\n"
                  << "==========================================\n";
        ParametrosOpcion p;
        ConfigSimulacion c;
        while (true) {
            pedir_datos(p, c);
            validar(p, c);
            mostrar_resultados(p, c);
            int otra = leer_opcion("\n¿Otra simulacion? (1 = Si, 0 = Salir)", 0, 1, 0);
            if (otra == 0) break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
