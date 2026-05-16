cmake_minimum_required(VERSION 3.21)

project(FitMindAI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Helps with deterministic builds on GCC/MinGW.
if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
endif()

add_executable(FitMindAI
  src/main.cpp
  src/models/UserProfile.cpp
  src/services/FitnessCalculator.cpp
  src/services/RecommendationEngine.cpp
  src/services/ProgressTracker.cpp
  src/services/FileManager.cpp
  src/services/MenuSystem.cpp
  src/utils/InputUtils.cpp

  src/utils/StringUtils.cpp
  src/utils/JsonUtils.cpp
)


target_include_directories(FitMindAI PRIVATE
  include
)


# For Windows MinGW runtime, keep stdc++ filesystem/link stable.
if (MINGW)
  target_link_libraries(FitMindAI PRIVATE stdc++fs)
endif()

