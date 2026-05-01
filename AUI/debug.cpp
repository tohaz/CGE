#include <execinfo.h>
#include <cxxabi.h>
#include <string>

void print_stack() {
  void* buffer[64];
  size_t first = 0, last = 0;
  char* demangled = 0;
  std::string entry = "", mangled = "";
  int size = backtrace(buffer, 64), status = 0;
  char** symbols = backtrace_symbols(buffer, size);
  for (int i = 1; i < size; ++i) {
    entry = symbols[i];
    first = entry.find('(');
    last = entry.find('+');
    if (first != std::string::npos && last != std::string::npos && first < last) {
      mangled = entry.substr(first + 1, last - first - 1);
      demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
      if (status == 0) {
        printf("  #%d %s\n", i, demangled);
        free(demangled);
        continue;
      }
    }
    printf(" #%d %s\n", i, symbols[i]);
  }
  free(symbols);
}



