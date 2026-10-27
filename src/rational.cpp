#include <iostream>
#include <numeric>
#include <stdexcept>
#include <cmath>

class Rational {
private:
    int64_t num;
    int64_t den;

    void simplify() {
        if (den == 0) {
            throw std::invalid_argument("Denominator cannot be zero");
        }
        if (den < 0) {
            num = -num;
            den = -den;
        }
        int64_t g = std::gcd(std::abs(num), den);
        num /= g;
        den /= g;
    }

public:
    Rational(int64_t numerator = 0, int64_t denominator = 1) 
        : num(numerator), den(denominator) {
        simplify();
    }

	friend Rational operator+(const Rational& lhs, const Rational& rhs) {
        return Rational(lhs.num * rhs.den + rhs.num * lhs.den, lhs.den * rhs.den);
    }

    friend Rational operator-(const Rational& lhs, const Rational& rhs) {
        return Rational(lhs.num * rhs.den - rhs.num * lhs.den, lhs.den * rhs.den);
    }

    friend Rational operator*(const Rational& lhs, const Rational& rhs) {
        return Rational(lhs.num * rhs.num, lhs.den * rhs.den);
    }

    friend Rational operator/(const Rational& lhs, const Rational& rhs) {
        if (rhs.num == 0) throw std::invalid_argument("Division by zero");
        return Rational(lhs.num * rhs.den, lhs.den * rhs.num);
    }

    // Comparisons as Friend Functions
    friend bool operator==(const Rational& lhs, const Rational& rhs) {
        return lhs.num == rhs.num && lhs.den == rhs.den;
    }

    friend bool operator!=(const Rational& lhs, const Rational& rhs) {
        return !(lhs == rhs);
    }

    friend bool operator<(const Rational& lhs, const Rational& rhs) {
        return lhs.num * rhs.den < rhs.num * lhs.den;
    }

    friend bool operator>(const Rational& lhs, const Rational& rhs) {
        return rhs < lhs;
    }

    friend bool operator<=(const Rational& lhs, const Rational& rhs) {
        return !(rhs < lhs);
    }

    friend bool operator>=(const Rational& lhs, const Rational& rhs) {
        return !(lhs < rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const Rational& r) {
        os << r.num;
        if (r.den != 1) { 
            os << "/" << r.den;
        }
        return os;
    }
};
