//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OPERATOR_H
#define __INET_OPERATOR_H

#include <memory>
#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

using namespace z3;

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

inline expr operator&&(const expr& a, const std::shared_ptr<expr>& b) {
    return a && *b;
}

inline expr operator&&(const std::shared_ptr<expr>& a, const expr& b) {
    return *a && b;
}

inline expr operator&&(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
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

