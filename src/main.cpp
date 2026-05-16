#include <iostream>
#include <exception>

#include "services/MenuSystem.h"

int main() {
  try {
    fitmind::MenuSystem app;
    return app.run();
  } catch (const std::exception& ex) {
    std::cerr << "Fatal error: " << ex.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "Fatal error: unknown exception\n";
    return 1;
  }
}

