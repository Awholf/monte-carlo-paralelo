
# graficar_speedup.py
# Responsable: Jhonatan
# Análisis de rendimiento de Monte Carlo paralelo con OpenMP

from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt

RAIZ = Path(__file__).resolve().parent.parent
ARCHIVO = RAIZ / "results" / "benchmarks.csv"
SALIDA = RAIZ / "results"

def main():
    if not ARCHIVO.exists():
        print("No se encontró benchmarks.csv. Ejecuta primero benchmark.py")
        return

    # Leer los resultados de las pruebas
    df = pd.read_csv(ARCHIVO)

    # Calcular los tiempos promedio por configuración
    resumen = (
        df.groupby(["simulaciones", "modo", "hilos"], as_index=False)
        ["tiempo_ms"].mean()
    )

    registros = []

    # Analizar cada cantidad de simulaciones por separado
    for sims, grupo in resumen.groupby("simulaciones"):
        secuencial = grupo[grupo["modo"] == "seq"]

        if secuencial.empty:
            continue

        tiempo_seq = secuencial["tiempo_ms"].iloc[0]
        paralelo = grupo[grupo["modo"] == "omp"].sort_values("hilos").copy()

        # Fórmulas de rendimiento
        paralelo["speedup"] = tiempo_seq / paralelo["tiempo_ms"]
        paralelo["eficiencia"] = paralelo["speedup"] / paralelo["hilos"] * 100
        paralelo["tiempo_secuencial_ms"] = tiempo_seq

        registros.append(paralelo)

        # Gráfico 1: tiempo de ejecución
        plt.figure(figsize=(8, 5))
        plt.plot(paralelo["hilos"], paralelo["tiempo_ms"],
                 marker="o", label="OpenMP")
        plt.axhline(tiempo_seq, linestyle="--", label="Secuencial")
        plt.xlabel("Número de hilos")
        plt.ylabel("Tiempo promedio (ms)")
        plt.title(f"Tiempo de ejecución - {sims:,} simulaciones")
        plt.xticks(paralelo["hilos"])
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(SALIDA / f"tiempo_{sims}.png", dpi=300)
        plt.close()

        # Gráfico 2: aceleración obtenida
        plt.figure(figsize=(8, 5))
        plt.plot(paralelo["hilos"], paralelo["speedup"],
                 marker="o", label="Speedup real")
        plt.plot(paralelo["hilos"], paralelo["hilos"],
                 linestyle="--", label="Speedup ideal")
        plt.xlabel("Número de hilos")
        plt.ylabel("Aceleración (veces)")
        plt.title(f"Speedup OpenMP - {sims:,} simulaciones")
        plt.xticks(paralelo["hilos"])
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(SALIDA / f"speedup_{sims}.png", dpi=300)
        plt.close()

    if not registros:
        print("No hay datos suficientes para calcular el speedup.")
        return

    tabla = pd.concat(registros, ignore_index=True)
    tabla.to_csv(SALIDA / "resumen_speedup.csv", index=False)

    print("\nRESULTADOS DE RENDIMIENTO:")
    print(tabla[["simulaciones", "hilos", "tiempo_ms",
                 "speedup", "eficiencia"]].round(3).to_string(index=False))
    print("\nGráficos y resumen guardados en results/")

if __name__ == "__main__":
    main()
