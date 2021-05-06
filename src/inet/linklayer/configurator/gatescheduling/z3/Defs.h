//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_Z3_DEFS_H
#define __INET_Z3_DEFS_H

#include <memory>
#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

using namespace z3;

extern int assertionCount;

inline void addAssert(solver& solver, const expr& expr) {
    solver.add(expr, (std::string("a") + std::to_string(assertionCount++)).c_str());
}

inline expr mkReal2Int(expr const & a) { Z3_ast i = Z3_mk_real2int(a.ctx(), a); a.check_error(); return expr(a.ctx(), i); }

inline expr operator==(const expr& a, const std::shared_ptr<expr>& b) {
    return a == *b;
}

inline expr operator==(const std::shared_ptr<expr>& a, const expr& b) {
    return *a == b;
}

inline expr operator==(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a == *b;
}

inline expr operator>(const expr& a, const std::shared_ptr<expr>& b) {
    return a > *b;
}

inline expr operator>(const std::shared_ptr<expr>& a, const expr& b) {
    return *a > b;
}

inline expr operator>(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a > *b;
}

inline expr operator<(const expr& a, const std::shared_ptr<expr>& b) {
    return a < *b;
}

inline expr operator<(const std::shared_ptr<expr>& a, const expr& b) {
    return *a < b;
}

inline expr operator<(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a < *b;
}

inline expr operator>=(const expr& a, const std::shared_ptr<expr>& b) {
    return a >= *b;
}

inline expr operator>=(const std::shared_ptr<expr>& a, const expr& b) {
    return *a >= b;
}

inline expr operator>=(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a >= *b;
}

inline expr operator<=(const expr& a, const std::shared_ptr<expr>& b) {
    return a <= *b;
}

inline expr operator<=(const std::shared_ptr<expr>& a, const expr& b) {
    return *a <= b;
}

inline expr operator<=(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a <= *b;
}

inline expr mkAnd(const expr& a, const expr& b) {
    return a && b;
}

inline expr mkAnd(const expr& a, const std::shared_ptr<expr>& b) {
    return a && *b;
}

inline expr mkAnd(const std::shared_ptr<expr>& a, const expr& b) {
    return *a && b;
}

inline expr mkAnd(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a && *b;
}

inline expr operator||(const expr& a, const std::shared_ptr<expr>& b) {
    return a || *b;
}

inline expr operator||(const std::shared_ptr<expr>& a, const expr& b) {
    return *a || b;
}

inline expr operator||(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a || *b;
}

inline expr mkAdd(const expr& a, const expr& b) {
    return a + b;
}

inline expr mkAdd(const expr& a, const std::shared_ptr<expr>& b) {
    return a + *b;
}

inline expr mkAdd(const std::shared_ptr<expr>& a, const expr& b) {
    return *a + b;
}

inline expr mkAdd(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a + *b;
}

inline expr mkSub(const expr& a, const expr& b) {
    return a - b;
}

inline expr mkSub(const expr& a, const std::shared_ptr<expr>& b) {
    return a - *b;
}

inline expr mkSub(const std::shared_ptr<expr>& a, const expr& b) {
    return *a - b;
}

inline expr mkSub(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a - *b;
}

inline expr operator+(const expr& a, const std::shared_ptr<expr>& b) {
    return a + *b;
}

inline expr operator+(const std::shared_ptr<expr>& a, const expr& b) {
    return *a + b;
}

inline expr operator+(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a + *b;
}

inline expr operator-(const expr& a, const std::shared_ptr<expr>& b) {
    return a - *b;
}

inline expr operator-(const std::shared_ptr<expr>& a, const expr& b) {
    return *a - b;
}

inline expr operator-(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a - *b;
}

inline expr operator*(const expr& a, const std::shared_ptr<expr>& b) {
    return a * *b;
}

inline expr operator*(const std::shared_ptr<expr>& a, const expr& b) {
    return *a * b;
}

inline expr operator*(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a * *b;
}

inline expr operator/(const expr& a, const std::shared_ptr<expr>& b) {
    return a / *b;
}

inline expr operator/(const std::shared_ptr<expr>& a, const expr& b) {
    return *a / b;
}

inline expr operator/(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a / *b;
}

}

#endif

