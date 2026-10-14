cmake_minimum_required(VERSION 3.16)
project(MonteCarloParalelo VERSION 1.0 LANGUAGES CXX)

# --- Estándar de C++ ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Release por defecto (importante para medir tiempos) ---
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de compilación" FORCE)
endif()

# --- OpenMP ---
find_package(OpenMP REQUIRED)

# --- Ejecutable ---
add_executable(monte_carlo
    src/main.cpp
    src/monte_carlo.cpp
    src/black_scholes.cpp
    src/monte_carlo_omp.cpp
)

target_include_directories(monte_carlo PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(monte_carlo PRIVATE OpenMP::OpenMP_CXX)

if(MSVC)
    target_compile_options(monte_carlo PRIVATE /W4)
else()
    target_compile_options(monte_carlo PRIVATE -Wall -Wextra -O2)
endif()

# El ejecutable queda en build/ (Linux/macOS) o build/Release/ (Visual Studio)
