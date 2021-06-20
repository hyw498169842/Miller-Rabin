#ifndef INT1024
#define INT1024

#include <iostream>
#include <fstream>
#include <cstring>
#include <random>

typedef std::uniform_int_distribution<uint32_t> Uniform;

class Int1024 {
    static Int1024 mods[512 + 1];
    // reserve 1024-bit for multiplication
    uint32_t num[32];

public:
    /* build array of mod multiplied by 2-power */
    static void buildMods(Int1024 mod) {
        for (int i = 512; i >= 0; --i) {
            mods[i] = mod;
            mod <<= 1;
        }
    }
    /* read integer from file, needs endian conversion */
    Int1024(std::string file) {
        memset(num, 0, 64); // set high 512 bit
        std::ifstream fin(file, std::ios::binary);
        fin.read((char*)&num[16], 64); // read low 512 bit
        fin.close();
        // endian conversion
        for (int i = 16; i < 32; ++i) {
            num[i] = ((num[i] & 0xff000000) >> 24)
                | ((num[i] & 0x00ff0000) >> 8)
                | ((num[i] & 0x0000ff00) << 8)
                | ((num[i] & 0x000000ff) << 24);
        }
    }
    /* constructor for implicit type conversion */
    Int1024(uint8_t value = 0) {
        memset(num, 0, sizeof(num));
        num[31] = value;
    }
    /* constructor on object copy */
    Int1024(const Int1024& other) {
        memcpy(num, other.num, sizeof(num));
    }
    /* decompose num into the form (remain * 2 ^ power) */
    int get2Power(Int1024& remain) {
        int power = 0;
        memcpy(remain.num, num, sizeof(num));
        while (!(remain.num[31] & 1)) {
            power += __builtin_ctz(remain.num[31]);
            remain >>= __builtin_ctz(remain.num[31]);
        }
        return power;
    }
    /* produce a random number smaller than num */
    Int1024 randomSmaller() {
        // random number smaller than num
        static std::default_random_engine e;
        Int1024 smaller;
        int highIndex = 0;
        bool lastEqual = true;
        while (!num[highIndex]) highIndex++;
        for (int i = highIndex; i < 32; ++i) {
            if (lastEqual) {
                smaller.num[i] = Uniform(0, num[i])(e);
                lastEqual = smaller.num[i] == num[i];
            } else {
                smaller.num[i] = Uniform(0, UINT32_MAX)(e);
            }
        }
        return smaller;
    }
    /* fast power, returns (num ^ p) % N */
    Int1024 pow(Int1024 p) {
        Int1024 base(*this), result(1), ZERO;
        while (ZERO < p) {
            if (p.num[31] & 1)
                result *= base; // mod N included
            base *= base; // mod N included
            p >>= 1;
        }
        return result;
    }
    /* operator less than */
    bool operator<(const Int1024& other) const {
        for (int i = 0; i < 32; ++i) {
            if (num[i] < other.num[i]) return true;
            if (num[i] > other.num[i]) return false;
        }
        return false;
    }
    /* operator equal */
    bool operator==(const Int1024& other) const {
        return memcmp(num, other.num, sizeof(num)) == 0;
    }
    /* operator unequal */
    bool operator!=(const Int1024& other) const {
        return memcmp(num, other.num, sizeof(num)) != 0;
    }
    /* operator add, only support uint32_t */
    Int1024 operator+(uint32_t other) const {
        // always assume the result does not overflow for simplification
        Int1024 result(*this);
        for (int i = 31; i >= 0; --i) {
            if (other == 0) break; // check carry
            result.num[i] += other;
            other = result.num[i] < other; // update carry
        }
        return result;
    }
    /* operator sub, only support uint32_t */
    Int1024 operator-(uint32_t other) const {
        // always assume the result does not underflow for simplification
        Int1024 result(*this);
        for (int i = 31; i >= 0; --i) {
            if (other == 0) break; // check carry
            uint32_t temp = result.num[i];
            result.num[i] -= other;
            other = result.num[i] > temp; // update carry
        }
        return result;
    }
    /* operator self-sub */
    Int1024& operator-=(Int1024 other) {
        // always assume the result does not underflow for simplification
        num[0] -= other.num[0];
        for (int i = 1; i < 32; ++i) {
            uint32_t subtractor = other.num[i];
            for (int j = i; j >= 0; --j) {
                uint32_t temp = num[j];
                num[j] -= subtractor;
                subtractor = num[j] > temp; // update carry
                if (subtractor == 0) break; // check carry
            }
        }
        return *this;
    }
    /* operator self-mul, mod N included */
    Int1024& operator*=(const Int1024& other) {
        // always assume the multipliers are 512-bit for simplification
        uint64_t tempResult[32];
        memset(tempResult, 0, sizeof(tempResult));
        for (int i = 16; i < 32; ++i) {
            // normal multiplication
            for (int j = 16; j < 32; ++j)
                tempResult[i + j - 31] += uint64_t(num[i]) * uint64_t(other.num[j]);
            // handle carries
            for (int i = 31; i > 0; --i) {
                tempResult[i - 1] += tempResult[i] >> 32;
                tempResult[i] = tempResult[i] & UINT32_MAX;
            }
        }
        // copy result, and then mod N
        for (int i = 0; i < 32; ++i) num[i] = tempResult[i];
        for (int i = 0; i <= 512; ++i) // mod N
            if (!(*this < mods[i])) *this -= mods[i];
        return *this;
    }
    /* operator self-rshift */
    Int1024& operator>>=(const int shift) {
        // always assume shift < 32 for simplification
        const uint32_t mask = (1U << shift) - 1;
        for (int i = 31; i > 0; --i) {
            num[i] = (num[i] >> shift) | ((num[i - 1] & mask) << (32 - shift));
        }
        num[0] = num[0] >> shift;
        return *this;
    }
    /* operator self-lshift */
    Int1024& operator<<=(const int shift) {
        // always assume shift < 32 for simplification
        const uint32_t mask = ~(UINT32_MAX >> shift);
        for (int i = 0; i < 31; ++i) {
            num[i] = (num[i] << shift) | ((num[i + 1] & mask) >> (32 - shift));
        }
        num[31] = num[31] << shift;
        return *this;
    }
};

Int1024 Int1024::mods[513];

#endif