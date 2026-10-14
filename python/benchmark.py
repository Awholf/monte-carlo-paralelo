#!/usr/bin/env python3
"""
benchmark.py — Automatización de pruebas de rendimiento
Responsable: Kiltom

Ejecuta el simulador en modo secuencial y paralelo (1, 2, 4, 8 hilos)
para distintos tamaños de simulación, repite cada prueba varias veces
y guarda todos los tiempos en results/benchmarks.csv.

Uso:
    python python/benchmark.py
    python python/benchmark.py --sims 1000000 10000000 --hilos 1 2 4 8 --reps 5
"""
import argparse
import csv
import platform
import statistics
import subprocess
import sys
from datetime import datetime
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent

COLUMNAS = ["fecha", "modo", "hilos", "simulaciones", "repeticion",
            "precio", "error_std", "black_scholes", "tiempo_ms"]


def buscar_ejecutable() -> Path:
    """Busca el ejecutable compilado en las rutas típicas de CMake."""
    nombre = "monte_carlo.exe" if platform.system() == "Windows" else "monte_carlo"
    for c in [RAIZ / "build" / nombre,
              RAIZ / "build" / "Release" / nombre,
              RAIZ / "build" / "Debug" / nombre]:
        if c.exists():
            return c
    sys.exit("No se encontró el ejecutable. Compile primero:\n"
             "  cmake -S . -B build && cmake --build build --config Release")


def ejecutar(exe: Path, modo: str, sims: int, hilos: int, semilla: int) -> dict:
    """Ejecuta el programa una vez y devuelve su salida CSV como diccionario."""
    cmd = [str(exe), "--modo", modo, "--sims", str(sims),
           "--hilos", str(hilos), "--semilla", str(semilla), "--csv"]
    salida = subprocess.run(cmd, capture_output=True, text=True, check=True)
    modo_r, hilos_r, sims_r, precio, err, bs, t = salida.stdout.strip().split(",")
    return {"modo": modo_r, "hilos": int(hilos_r), "simulaciones": int(sims_r),
            "precio": float(precio), "error_std": float(err),
            "black_scholes": float(bs), "tiempo_ms": float(t)}


def main():
    ap = argparse.ArgumentParser(description="Benchmarks Monte Carlo secuencial vs OpenMP")
    ap.add_argument("--sims", type=int, nargs="+", default=[1_000_000, 5_000_000, 10_000_000])
    ap.add_argument("--hilos", type=int, nargs="+", default=[1, 2, 4, 8])
    ap.add_argument("--reps", type=int, default=3, help="repeticiones por configuración")
    ap.add_argument("--salida", default=str(RAIZ / "results" / "benchmarks.csv"))
    args = ap.parse_args()

    exe = buscar_ejecutable()
    salida = Path(args.salida)
    salida.parent.mkdir(parents=True, exist_ok=True)
    fecha = datetime.now().isoformat(timespec="seconds")

    # Configuraciones: secuencial + paralelo con cada número de hilos
    configs = [("seq", 1)] + [("omp", h) for h in args.hilos]
    total = len(args.sims) * len(configs) * args.reps
    filas, n = [], 0

    print(f"Ejecutable: {exe}")
    print(f"Ejecutando {total} pruebas...\n")
    for sims in args.sims:
        for modo, hilos in configs:
            tiempos = []
            for rep in range(1, args.reps + 1):
                n += 1
                r = ejecutar(exe, modo, sims, hilos, semilla=42 + rep)
                r.update({"fecha": fecha, "repeticion": rep})
                filas.append(r)
                tiempos.append(r["tiempo_ms"])
            print(f"[{n:>3}/{total}] {modo:>3} | hilos={hilos:<2} | sims={sims:>11,} | "
                  f"tiempo medio={statistics.mean(tiempos):10.2f} ms | precio={r['precio']:.4f}")

    with open(salida, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNAS)
        w.writeheader()
        w.writerows(filas)

    print(f"\nResultados guardados en: {salida}")


if __name__ == "__main__":
    main()
