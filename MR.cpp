#include <iostream>
#include <ctime>

#include "int1024.h"

/* Miller-Rabin test algorithm */
bool MR(const Int1024& N, int t = 10) {
    if (N < 5) return N == 2 || N == 3;
    // build mods regarding N
    Int1024::buildMods(N);
    // constants that may be often used
    Int1024 ONE(1), TWO(2);
    Int1024 N_MINUS_1 = N - 1;
    Int1024 N_MINUS_3 = N - 3;
    // decompose N - 1 into the form m * 2 ^ k
    Int1024 m; int k = N_MINUS_1.get2Power(m);
    // test for t rounds
    for (int i = 0; i < t; ++i) {
        // 2 <= a + 2 <= N - 2
        Int1024 a = N_MINUS_3.randomSmaller();
        // y = (a ^ m) % N
        Int1024 y = (a + 2).pow(m);
        if (y != ONE && y != N_MINUS_1) {
            for (int j = 1; j < k; ++j) {
                y = y.pow(TWO); // mod N included
                if (y == N_MINUS_1) break;
                if (y == ONE) return false;
            }
            if (y != N_MINUS_1) return false;
        }
    }
    return true;
}

int main() {
    double start = clock();
    // read the integers
    Int1024 N1("data/N1"), N2("data/N2");
    // apply M-R test
    std::cout << "M-R test for N1: " << (MR(N1) ? "Is prime" : "Not prime") << std::endl;
    std::cout << "M-R test for N2: " << (MR(N2) ? "Is prime" : "Not prime") << std::endl;
    std::cout << "Time usage (in seconds): " << (clock() - start) / CLOCKS_PER_SEC << std::endl;
    return 0;
}