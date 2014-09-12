//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "Complex.h"
#include <cmath>

Complex Complex::operator+(const Complex& rhs) const
{
    return Complex(rhs.re + this->re, rhs.im + this->im);
}

Complex Complex::operator-(const Complex& rhs) const
{
    return Complex(this->re - rhs.re, this->im - rhs.im);
}

Complex Complex::operator*(const Complex& rhs) const
{
    return Complex(this->re * rhs.re - this->im * rhs.im, this->re * rhs.im + this->im * rhs.re);
}

Complex Complex::operator/(const Complex& rhs) const
{
    double rhsSquare = rhs.re * rhs.re + rhs.im * rhs.im;
    if (rhsSquare == 0.0)
        throw cRuntimeError("Division by zero");
    return Complex((this->re * rhs.re + this->im * rhs.im)/rhsSquare, (this->im * rhs.re - this->re * rhs.im)/rhsSquare);
}

bool Complex::operator==(const Complex& rhs) const
{
    return this->im == rhs.im && this->re == rhs.re;
}

bool Complex::operator!=(const Complex& rhs) const
{
    return !(*this == rhs);
}

std::ostream& operator<<(std::ostream& out, const Complex& w)
{
    return out << "re: " << w.re << " im: " << w.im;
}

double Complex::abs() const
{
    return sqrt(this->re * this->re + this->im * this->im);
}

Complex Complex::operator*(const double& rhs) const
{
    return Complex(rhs * this->re, rhs * this->im);
}

Complex Complex::conjugate() const
{
    return Complex(this->re, -this->im);
}

Complex operator*(const double& scalar, const Complex& comp)
{
    return comp * scalar;
}
