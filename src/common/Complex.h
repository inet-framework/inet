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

#ifndef __INET_COMPLEX_H
#define __INET_COMPLEX_H

#include "INETDefs.h"

class INET_API Complex
{
    private:
        double re;
        double im;

    public:
        Complex(const double& r) : re(r), im(0) {}
        Complex(const double& r, const double& i) : re(r), im(i) {}
        Complex(const Complex& w) : re(w.re), im(w.im) {}
        Complex() : re(0), im(0) {}
        
        Complex operator+(const Complex& rhs) const;
        Complex operator-(const Complex& rhs) const;
        Complex operator*(const Complex& rhs) const;
        Complex operator/(const Complex& rhs) const;
        bool operator==(const Complex& rhs) const;
        bool operator!=(const Complex& rhs) const;
        
        friend std::ostream& operator<<(std::ostream& out, const Complex& w);
        
        double abs() const;
        Complex conjugate() const;
        double getReal() const { return re; }
        double getImaginary() const { return im; }
};

#endif /* __INET_COMPLEX_H */

