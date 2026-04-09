# Inteligentne Metody Optymalizacji

Repozytorium zawiera framework w języku C++ z otoczką analityczną w języku Python, stworzony na potrzeby realizacji projektów z przedmiotu "Inteligentne Metody Optymalizacji". Głównym celem projektów jest rozwiązywanie wariantów problemu komiwojażera (TSP / TSPA / TSPB) z wykorzystaniem różnorodnych algorytmów heurystycznych i optymalizacyjnych.

## Struktura projektu

Projekt oparty jest na architekturze modułowej (monorepo), co pozwala na współdzielenie kodu między poszczególnymi etapami (laboratoriami) bez jego duplikacji.

* **`common/`** - Współdzielona biblioteka (nagłówki i źródła). Zawiera m.in. struktury danych dla instancji problemu, interfejsy solverów, sprawdzanie poprawności rozwiązań (Solution Checker) oraz operacje We/Wy (parsowanie CSV, eksport do JSON).
* **`lab1/`** - Zadanie 1: Heurystyki konstrukcyjne. Zawiera implementacje algorytmów takich jak: Random, Nearest Neighbor, Greedy Cycle, Regret Heuristics.
* **`lab2/`** - Zadanie 2: Lokalne przeszukiwanie (Steepest, Greedy) z różnymi definicjami sąsiedztwa (wymiana wierzchołków, wymiana krawędzi).
* **`instances/`** - Pliki wejściowe CSV reprezentujące instancje problemu (np. `TSPA.csv`, `TSPB.csv`).
* **`scripts/`** - Narzędzia w Pythonie służące do masowego uruchamiania eksperymentów, zbierania logów, generowania statystyk oraz wizualizacji tras na wykresach.
* **`results/`** - Wygenerowane wyniki eksperymentów (tabele CSV, dane JSON) oraz wizualizacje tras (PNG), podzielone na podkatalogi dla każdego z laboratoriów.

## Wymagania

Do skompilowania i uruchomienia projektu wymagane są:
* Kompilator wspierający **C++17** (GCC / Clang)
* **CMake** (min. wersja 3.16)
* **OpenMP** (dla zrównoleglenia obliczeń)
* **Python 3.8+** wraz z bibliotekami: `pandas`, `matplotlib`, `networkx`

> **Wskazówka:** Projekt jest przystosowany do pracy w środowiskach opartych na kontenerach (DevContainers), co pozwala na automatyczną instalację wszystkich zależności.

## Kompilacja i uruchamianie

Najprostszą metodą uruchomienia pełnego potoku (od kompilacji kodu C++ po wygenerowanie wykresów w Pythonie) jest użycie dołączonego skryptu Bash.

Uruchomienie dla konkretnego laboratorium (domyślnie `lab1`):
```bash
# Uruchomienie Lab 1 (Heurystyki konstrukcyjne)
./scripts/build_and_run.sh lab1

# Uruchomienie Lab 2 (Lokalne przeszukiwanie)
./scripts/build_and_run.sh lab2