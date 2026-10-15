# Documentación de implementación paralela con OpenMP

**Proyecto:** Simulación Paralela de Monte Carlo – Opción Europea  
**Curso:** Programación Concurrente y Paralela  
**Equipo:** 9  
**Responsable:** Jhonatan  
**Tecnologías:** C++17, OpenMP 5.2, Python, CMake, Ninja, Pandas y Matplotlib

---

## 1. Introducción

El presente documento describe el trabajo realizado en la implementación paralela del algoritmo Monte Carlo utilizando OpenMP, así como las pruebas de rendimiento, el análisis del speedup y la generación de gráficos comparativos.

El objetivo principal fue aprovechar múltiples hilos de procesamiento para reducir el tiempo necesario para estimar el precio de una opción financiera europea de tipo CALL o PUT.

La implementación fue integrada con la estructura existente del proyecto, respetando las interfaces compartidas y utilizando los módulos desarrollados por los demás integrantes.

## 2. Responsabilidades desarrolladas

Las actividades realizadas corresponden a las semanas 6, 7 y 8 del cronograma del proyecto.

| Semana | Actividad | Estado |
|---|---|---|
| 6 | Implementación paralela con OpenMP | Completada |
| 7 | Pruebas de rendimiento y benchmarks | Completada |
| 8 | Análisis de speedup y generación de gráficos | Completada |

La validación final con las versiones definitivas de los módulos del equipo queda pendiente de integración.

## 3. Archivos utilizados

### Archivos desarrollados

**src/monte_carlo_omp.cpp**

Contiene la implementación del algoritmo paralelo Monte Carlo utilizando OpenMP.

Sus funcionalidades principales son:

- Ejecución de simulaciones en múltiples hilos.
- Distribución estática de las iteraciones.
- Generación independiente de números aleatorios por hilo.
- Acumulación segura de resultados mediante reduction.
- Estimación del precio de opciones europeas CALL y PUT.
- Cálculo del error estándar.
- Medición del tiempo de ejecución.
- Validación de parámetros de entrada.

**python/graficar_speedup.py**

Permite procesar los resultados de rendimiento y generar automáticamente:

- Gráficos de tiempo de ejecución.
- Gráficos de aceleración real e ideal.
- Cálculo del speedup.
- Cálculo de eficiencia paralela.
- Archivo CSV con los resultados resumidos.

### Archivos utilizados sin modificarlos

- `src/main.cpp`: interfaz principal, desarrollada por Kiltom.
- `src/monte_carlo.cpp`: implementación secuencial de referencia.
- `src/black_scholes.cpp`: fórmula analítica de referencia.
- `include/parametros.h`: estructuras compartidas.
- `include/simulacion.h`: declaraciones de las funciones.
- `python/benchmark.py`: automatización de las pruebas de rendimiento.

## 4. Entorno de desarrollo

El proyecto se desarrolló y probó en Windows utilizando Visual Studio Code.

Herramientas verificadas:

| Herramienta | Versión |
|---|---|
| Python | 3.14.5 |
| GCC / G++ | 16.2.0 |
| CMake | 4.4.4 |
| Ninja | 1.13.2 |
| OpenMP | 5.2 |

Para compilar el programa se utilizó el entorno MSYS2 UCRT64.

## 5. Explicación de la implementación paralela

### 5.1. Uso de OpenMP

Se empleó OpenMP para ejecutar las simulaciones de Monte Carlo mediante múltiples hilos.

La región paralela se crea utilizando:

```cpp
#pragma omp parallel num_threads(cfg.num_hilos) \
    reduction(+:suma, suma_cuadrados)
```

Esta instrucción permite configurar la cantidad solicitada de hilos y realizar acumulaciones sin condiciones de carrera.

### 5.2. Distribución de iteraciones

Se utilizó:

```cpp
#pragma omp for schedule(static)
```

Esta directiva distribuye las iteraciones del ciclo entre los hilos participantes.

Cada hilo procesa una parte de las simulaciones, permitiendo ejecutar el trabajo de manera concurrente.

### 5.3. Generadores aleatorios independientes

Cada hilo dispone de su propio generador de números aleatorios:

```cpp
const int id_hilo = omp_get_thread_num();

const unsigned long long semilla_hilo =
    cfg.semilla + 1000003ULL * (id_hilo + 1);

std::mt19937_64 generador(semilla_hilo);
std::normal_distribution<double> normal(0.0, 1.0);
```

Esto evita compartir el estado de un mismo generador entre diferentes hilos.

Las semillas se diferencian por identificador de hilo. Esto permite generar secuencias reproducibles para una configuración dada, aunque no garantiza que sean estadísticamente independientes ni que el resultado sea idéntico al cambiar el número de hilos.

### 5.4. Prevención de condiciones de carrera

Se utilizó la cláusula `reduction` para acumular:

- La suma de los beneficios simulados.
- La suma de los cuadrados de los beneficios.

OpenMP mantiene acumuladores privados y combina sus resultados al finalizar la región paralela.

### 5.5. Cálculo del precio y error estándar

Cada simulación genera un posible precio futuro del activo utilizando el modelo financiero definido en el proyecto.

Posteriormente se calcula el beneficio correspondiente a una opción CALL o PUT.

El precio estimado es el promedio de los beneficios, descontado al valor presente.

También se calcula una estimación del error estándar para cuantificar la incertidumbre estadística del resultado.

### 5.6. Medición del tiempo

Se empleó la función:

```cpp
omp_get_wtime()
```

Esta función permite medir el tiempo transcurrido durante la ejecución del algoritmo paralelo.

El resultado se convierte a milisegundos para facilitar la comparación con la versión secuencial.

## 6. Configuración y compilación

Para configurar el proyecto se ejecutó en MSYS2 UCRT64:

```bash
cd /c/Users/jaraj/Downloads/monte-carlo-paralelo-main
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

CMake detectó correctamente el compilador GNU C++ y OpenMP 5.2.

Posteriormente se compiló:

```bash
cmake --build build
```

La compilación terminó correctamente y se generó el ejecutable:

`build/monte_carlo.exe`

## 7. Ejecución de las simulaciones

### Ejecución secuencial

```bash
./build/monte_carlo.exe --modo seq --sims 10000000
```

### Ejecución paralela con 4 hilos

```bash
./build/monte_carlo.exe --modo omp --sims 10000000 --hilos 4
```

### Ejecución paralela con 8 hilos

```bash
./build/monte_carlo.exe --modo omp --sims 10000000 --hilos 8
```

El programa muestra el precio estimado, el error estándar, el valor analítico de Black-Scholes y el tiempo de ejecución.

## 8. Pruebas de rendimiento

Se utilizó el script proporcionado por Kiltom:

```powershell
python python/benchmark.py --sims 1000000 10000000 --hilos 1 2 4 8 --reps 5
```

Se ejecutaron 50 pruebas considerando:

- 1 000 000 y 10 000 000 de simulaciones.
- Ejecución secuencial.
- OpenMP con 1, 2, 4 y 8 hilos.
- Cinco repeticiones por configuración.

Los resultados se guardaron en:

`results/benchmarks.csv`

## 9. Resultados de rendimiento

Los siguientes datos corresponden a las pruebas realizadas después de actualizar la implementación paralela.

| Modo | Hilos | 1 millón (ms) | 10 millones (ms) |
|---|---:|---:|---:|
| Secuencial | 1 | 26.48 | 268.52 |
| OpenMP | 1 | 25.40 | 254.40 |
| OpenMP | 2 | 13.40 | 130.00 |
| OpenMP | 4 | 9.40 | 69.40 |
| OpenMP | 8 | 5.00 | 42.40 |

Los valores corresponden a los promedios reportados por los programas de pruebas.

## 10. Análisis de speedup

El speedup expresa cuánto se acelera la ejecución paralela respecto al algoritmo secuencial.

**Fórmula:**

Speedup = Tiempo secuencial / Tiempo paralelo

La eficiencia paralela se calcula mediante:

Eficiencia (%) = (Speedup / Número de hilos) × 100

### Resultados con 10 millones de simulaciones

| Hilos OpenMP | Speedup | Eficiencia |
|---|---:|---:|
| 1 | 1.055 | 105.55 % |
| 2 | 2.066 | 103.28 % |
| 4 | 3.869 | 96.73 % |
| 8 | 6.333 | 79.16 % |

La mejor aceleración observada se produjo utilizando 8 hilos, alcanzando un speedup aproximado de 6.33.

Esto significa que, en las condiciones de prueba, la ejecución paralela fue aproximadamente 6.33 veces más rápida que la secuencial.

La reducción del tiempo de ejecución fue de aproximadamente 84.2 %.

Las eficiencias superiores al 100 % en ciertas configuraciones deben interpretarse con cautela. Pueden relacionarse con diferencias entre implementaciones, fluctuaciones de medición u otras condiciones de ejecución.

## 11. Generación de gráficos

Se desarrolló el archivo:

`python/graficar_speedup.py`

Para ejecutarlo:

```powershell
python python/graficar_speedup.py
```

El script utiliza Pandas para procesar los resultados del CSV y Matplotlib para generar las visualizaciones.

Archivos generados:

| Archivo | Descripción |
|---|---|
| `tiempo_1000000.png` | Tiempo de ejecución con 1 millón de simulaciones |
| `tiempo_10000000.png` | Tiempo de ejecución con 10 millones de simulaciones |
| `speedup_1000000.png` | Speedup con 1 millón de simulaciones |
| `speedup_10000000.png` | Speedup con 10 millones de simulaciones |
| `resumen_speedup.csv` | Resumen de tiempos, speedup y eficiencia |

Todos estos archivos se generan en la carpeta `results`.

## 12. Problemas encontrados y soluciones

### Problema 1: CMake y G++ no reconocidos

Inicialmente, Windows no reconocía los comandos `cmake` y `g++`.

**Solución:** instalación de MSYS2 y de las herramientas GCC, CMake y Ninja correspondientes al entorno UCRT64.

### Problema 2: Herramientas no reconocidas en MSYS

Después de la instalación, las herramientas no aparecían disponibles al ejecutar los comandos en la terminal MSYS.

**Solución:** utilizar la terminal MSYS2 UCRT64, que contiene el entorno para el cual se instalaron las herramientas.

### Problema 3: Python no reconocido desde MSYS2

El comando `python` no estaba disponible en la terminal MSYS2 utilizada.

**Solución:** ejecutar los scripts Python desde PowerShell de Visual Studio Code, donde Python ya estaba instalado.

### Problema 4: Error al ejecutar el benchmark

Al intentar ejecutar el programa desde PowerShell apareció el código de error:

`3221225781`

Este error indicaba que Windows no encontraba una dependencia necesaria para iniciar el ejecutable.

**Solución:** agregar temporalmente el directorio de bibliotecas MSYS2 UCRT64 al PATH de PowerShell:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
```

Después de aplicar esta configuración, los benchmarks se ejecutaron correctamente.

La modificación es temporal y debe repetirse al abrir una nueva sesión de PowerShell si las bibliotecas no están disponibles.

## 13. Indicaciones para los compañeros

Para volver a ejecutar las pruebas:

1. Descargar o clonar el repositorio.
2. Disponer de C++17, CMake y un compilador con soporte OpenMP.
3. Configurar y compilar el proyecto.
4. Verificar la ejecución secuencial y paralela.
5. Ejecutar `python/benchmark.py`.
6. Ejecutar `python/graficar_speedup.py`.
7. Consultar los archivos CSV y PNG generados en `results`.

Los tiempos pueden variar según el procesador, la cantidad de núcleos y las condiciones de ejecución.

Para obtener comparaciones válidas, se recomienda utilizar una misma computadora, mantener configuraciones consistentes y ejecutar varias repeticiones.

## 14. Consideraciones pendientes

Aunque la implementación paralela compiló y funcionó correctamente, todavía deben comprobarse los siguientes aspectos durante la integración final:

- Compatibilidad con la versión definitiva del algoritmo secuencial de Leonardo.
- Validación estadística con más casos y configuraciones.
- Comparación definitiva con el modelo de Black-Scholes.
- Verificación del comportamiento con distintas cantidades de simulaciones e hilos.
- Revisión de los archivos por parte de Kiltom antes de integrar los cambios.

## 15. Conclusiones

Se implementó y probó satisfactoriamente una versión paralela del algoritmo Monte Carlo utilizando C++ y OpenMP. La distribución de las simulaciones entre múltiples hilos permitió reducir los tiempos de procesamiento respecto a la ejecución secuencial.

Las pruebas realizadas con 10 millones de simulaciones demostraron una aceleración máxima observada de aproximadamente 6.33 veces utilizando 8 hilos, con una eficiencia paralela del 79.16 %.

Además, se desarrolló un script en Python para automatizar el análisis de rendimiento, calcular el speedup y generar gráficos comparativos a partir de los resultados experimentales.

En conclusión, el trabajo realizado demuestra el potencial de la programación paralela para acelerar simulaciones computacionalmente intensivas. La implementación queda preparada para su integración y validación final con los módulos definitivos del equipo.
