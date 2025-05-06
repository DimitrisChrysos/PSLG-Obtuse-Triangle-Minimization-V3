# 📐 opt_triangulation — Adaptive Steiner Point Insertion with Randomized Rescue Strategies (CGSHOP 2025)

This C++ project implements an adaptive framework for generating non-obtuse triangulations of Planar Straight Line Graphs (PSLGs) using various metaheuristic methods. It was developed for the course “Ανάπτυξη Λογισμικού για Δύσκολα Αλγοριθμικά Προβλήματα”, targeting high-quality triangulations for CGSHOP 2025 competition instances.

---

## 👥 Team

- Αναστάσιος Μουμουλίδης – ΑΜ: 1115202100108  
- Δημήτριος Χρυσός – ΑΜ: 1115202100275

---

## 🧠 Description

The program reads a `.json` input file defining a triangulation problem instance. It selects and executes one of three algorithms:
- **Local Search (ls)**
- **Simulated Annealing (sa)**
- **Ant Colony Optimization (ant)**

When local methods fail to converge, **randomized Steiner point insertion** is triggered as a rescue mechanism. The result is written back as a `.json` output file in CGSHOP 2025 format.

---

## 📦 File Structure

### Main File
- `ex.cpp`: Main entry point. Parses input, selects method, executes it, writes output.

### Method Selection
- `handle_methods()` calls one of:
  - `local_search()`
  - `sim_annealing()`
  - `ant_colony_optimization()`

### Output
- `write_output()`: Creates valid JSON output.

### Submodules

#### `custom_cdt_class/`
- `custom_cdt_class.hpp / .cpp`: Custom wrapper over CGAL’s CDT. Adds:
  - `insert_steiner_x_y()`, `insert_no_flip()`, `remove_no_flip()`
  - `my_is_flippable()`: Custom edge flipping checker

#### `read_write_file/`
- Handles input/output JSON:
  - `read_write_file.hpp / .cpp`

#### `steiner_methods/`
- Five Steiner point insertion strategies:
  - `-proj`, `-centr`, `-mid`, `-circum`, `-merge`

#### `evaluate_instance/`
- `evaluate_instance.hpp / .cpp`: Effectiveness evaluation for each method

#### `utils/`
- Helper classes & methods: `utils.hpp / .cpp`

---

## 🔀 Randomization Strategy

If initial strategies fail:
- Random triangle is selected among remaining obtuse ones
- Random barycentric coordinates generate a candidate point
- Point is tested; accepted only if it reduces obtuse angles
- Max 5 failed attempts before termination

---

## 📊 Data Analysis

- `instances_combinations_values/`: Contains optimal parameters, convergence rates, and energy comparisons
- `method_averages.xlsx`: Mean value per category-method
- `compare_randomization.xlsx`: Pre/post randomization effectiveness comparison

---

## 🧪 Usage & Compilation

### Compile:
```bash
cmake -DCGAL_DIR=/usr/lib/CGAL
make
```

### Run (Manual)
```bash
./opt_triangulation -i input.json -o outputs/temp.json -ls -proj -circum -merge
./opt_triangulation -i ... -o ... -preselected_params
```

- Method: `-ls`, `-sa`, `-ant`, or `-preselected_params`
- Steiner strategies: `-proj`, `-centr`, `-mid`, `-circum`, `-merge`

---

## 🗂 Dataset Folders

- `challenge_instances_cgshop25/`: Official test cases
- `outputs/`: Solution outputs
- `test_categories/`, `tests_categorized/`: Category mappings
- `instances_combinations_values/`: Analysis files and performance metrics

---

## ✅ Features

- Auto-selection of best method per instance type
- Evaluation via convergence rate or triangulation energy
- Full CGSHOP2025-compatible I/O
- Randomization for escaping local optima

---

## 📌 GitHub Repository

🔗 [github.com/DimitrisChrysos/project-emiris-3](https://github.com/DimitrisChrysos/project-emiris-3.git)

---

Developed as the final project of the **2024–2025** course:  
**Ανάπτυξη Λογισμικού για Δύσκολα Αλγοριθμικά Προβλήματα**
