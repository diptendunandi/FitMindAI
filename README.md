# FitMindAI

AI-powered Smart Fitness Recommendation System (rule-based “AI-like” logic) with a menu-driven CLI.

## Features
- User profile input (name, age, gender, height, weight, goal, workout frequency, activity level, sleep hours, diet preference)
- BMI + daily calorie estimation
- Protein/carbs/fats recommendation
- Workout split recommendation
- Recovery + hydration + sleep-quality advice
- Weekly progress tracker persisted locally as JSON
- Clean modular architecture (C++20 + OOP)

## Requirements
- C++20 compiler
- CMake

## Windows (GCC 13.2.0 + MinGW Makefiles)

### Build (from project root)
```bat
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### Run
- **Windows CMD (recommended):**
```bat
build\FitMindAI.exe
```

- **Git Bash:**
```bash
cd /d/Embedded_Projects/FitMind
./build/FitMindAI.exe
```

- If the program can’t find `data/` files, ensure the working directory is the project root (because the app uses relative paths like `data/progress.json`):

**Git Bash:**
```bash
cd /d/Embedded_Projects/FitMind
./build/FitMindAI.exe
```

**Windows CMD:**
```bat
cd /d D:\Embedded_Projects\FitMind
build\FitMindAI.exe
```


## macOS / Linux
### Build
```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Run
From the project root (so `data/` paths resolve):
```bash
./build/FitMindAI
```

## Sample data
- `data/sample_profile.json`
- `data/sample_progress.json`

## Notes
- Progress is stored in `data/progress.json` under the project root by default.
- If the file doesn’t exist or is empty, it will be created on first save.

# FitMindAI
