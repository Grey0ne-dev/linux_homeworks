#include "complex.h"


Complex::Complex(double r, double i) : real(r), imag(i) {}


double Complex::getReal() const { return real; }
double Complex::getImag() const { return imag; }


void Complex::setReal(double r) { real = r; }
void Complex::setImag(double i) { imag = i; }


Complex Complex::operator+(const Complex& other) const {
    return Complex(real + other.real, imag + other.imag);
}

Complex Complex::operator-(const Complex& other) const {
    return Complex(real - other.real, imag - other.imag);
}

Complex Complex::operator*(const Complex& other) const {
    return Complex(
        real * other.real - imag * other.imag,
        real * other.imag + imag * other.real
    );
}

Complex Complex::operator/(const Complex& other) const {
    double denom = other.real*other.real + other.imag*other.imag;
    if(!denom){ throw std::runtime_error("Division by zero!");}
    return Complex(
        (real*other.real + imag*other.imag) / denom,
        (imag*other.real - real*other.imag) / denom
    );
}


bool Complex::operator==(const Complex& other) const {
    return real == other.real && imag == other.imag;
}

bool Complex::operator!=(const Complex& other) const {
    return !(*this == other);
}

double Complex::abs() const {
    return std::sqrt(real*real + imag*imag);
}

Complex Complex::conjugate() const {
    return Complex(real, -imag);
}

std::ostream& operator<<(std::ostream& os, const Complex& c) {
    if (c.imag >= 0)
        os << c.real << " + " << c.imag << "i";
    else
        os << c.real << " - " << -c.imag << "i";
    return os;
}

std::istream& operator>>(std::istream& is, Complex& c) {
    std::cout << "Enter real part: ";
    is >> c.real;
    std::cout << "Enter imaginary part: ";
    is >> c.imag;
    return is;
}


bool Complex::operator<(const Complex& other) const {
         double mag1 = real * real + imag * imag;
        double mag2 = other.real * other.real + other.imag * other.imag;
        return mag1 < mag2;
    } // comparision by absolute value(magnitude)
