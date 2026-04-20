#include "regex.hpp"
#include <iostream>
#include <string>

using namespace Grammar;

int main() {
    int n;
    std::cin >> n;

    for (int i = 0; i < n; ++i) {
        std::string regex;
        std::string test_string;
        std::cin >> regex >> test_string;

        RegexChecker checker(regex);
        if (checker.Check(test_string)) {
            std::cout << "YES" << std::endl;
        } else {
            std::cout << "NO" << std::endl;
        }
    }

    return 0;
}
