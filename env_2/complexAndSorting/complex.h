#ifndef COMPLEX_H
#define COMPLEX_H

#include <iostream>
#include <cmath>

class Complex {
private:
    double real;
    double imag;

public:
    
    Complex(double r = 0.0, double i = 0.0);

    
    double getReal() const;
    double getImag() const;


    void setReal(double r);
    void setImag(double i);

    
    Complex operator+(const Complex& other) const;
    Complex operator-(const Complex& other) const;
    Complex operator*(const Complex& other) const;
    Complex operator/(const Complex& other) const;

    
    bool operator==(const Complex& other) const;
    bool operator!=(const Complex& other) const;
    bool operator<(const Complex&) const;


    double abs() const;
    Complex conjugate() const;


    friend std::ostream& operator<<(std::ostream& os, const Complex& c);
    friend std::istream& operator>>(std::istream& is, Complex& c);
};

#endif
