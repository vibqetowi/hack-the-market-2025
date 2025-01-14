// utils.cc
#include <iostream>

extern "C" {
    int add(int a, int b) {
        std::cout << "Adding " << a << " and " << b << " in C++" << std::endl;
        return a + b;
    }

    void greet(const char* name) {
        std::cout << "Hello, " << name << " from C++!" << std::endl;
    }
}