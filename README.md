# Simulación Paralela de Monte Carlo – Opción Europea

Proyecto del curso **Programación Concurrente y Paralela** – Equipo 9.

Valoriza una opción financiera europea (call/put) mediante simulación de Monte Carlo, con una versión **secuencial** y otra **paralela con OpenMP**, validadas contra la fórmula analítica de **Black-Scholes**.

## Integrantes

| Integrante | Rol |
|---|---|
| Kiltom | Configuración, arquitectura, interfaz, integración y benchmarks |
| Leonardo | Algoritmo secuencial y validación con Black-Scholes |
| Jonathan | Paralelización con OpenMP y análisis de speedup |

## Requisitos

- Compilador C++17 con soporte OpenMP (GCC ≥ 9, Clang con libomp, o MSVC 2019+)
- CMake ≥ 3.16
- Python ≥ 3.9

## Instalación

```bash
git clone https://github.com/<usuario>/monte-carlo-paralelo.git
cd monte-carlo-paralelo
python -m venv .venv
source .venv/bin/activate        # Windows: .venv\Scripts\activate
pip install -r requirements.txt
```

## Compilación

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

El ejecutable queda en `build/monte_carlo` (Linux/macOS) o `build/Release/monte_carlo.exe` (Windows con Visual Studio).

## Ejecución

**Modo interactivo** (pide los parámetros por consola):

```bash
./build/monte_carlo
```

**Modo por argumentos**:

```bash
./build/monte_carlo --modo seq --sims 1000000
./build/monte_carlo --modo omp --sims 1000000 --hilos 4
./build/monte_carlo --modo omp --sims 1000000 --hilos 8 --S0 100 --K 105 --sigma 0.25 --tipo put
./build/monte_carlo --help
```

| Argumento | Descripción | Por defecto |
|---|---|---|
| `--modo` | `seq` o `omp` | `seq` |
| `--sims` | Número de simulaciones | 1 000 000 |
| `--hilos` | Hilos OpenMP | 4 |
| `--semilla` | Semilla base | 42 |
| `--S0 --K --r --sigma --T` | Parámetros financieros | 100, 100, 0.05, 0.20, 1 |
| `--tipo` | `call` o `put` | `call` |
| `--csv` | Salida en una línea CSV | — |

## Benchmarks

```bash
python python/benchmark.py                                   # configuración por defecto
python python/benchmark.py --sims 1000000 10000000 --hilos 1 2 4 8 --reps 5
```

Genera `results/benchmarks.csv` con las columnas:
`fecha, modo, hilos, simulaciones, repeticion, precio, error_std, black_scholes, tiempo_ms`.

## Estructura

```
src/        código C++ (main, secuencial, Black-Scholes, OpenMP)
include/    parametros.h (estructuras) y simulacion.h (interfaz compartida)
python/     validación, benchmarks y gráficos
tests/      pruebas de precisión
results/    CSV y gráficos generados
docs/       metodología y avance
```

## Flujo de trabajo con Git

- `main`: versión estable e integrada.
- `develop`: rama de integración.
- `feature/secuencial` (Leonardo), `feature/openmp` (Jonathan), `feature/interfaz` y `feature/benchmark` (Kiltom).

Cada integrante trabaja en su rama y abre un Pull Request hacia `develop`.
