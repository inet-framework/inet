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

inline expr mkITE(const expr& i, const expr& t, const expr& e) {
    return i ? t : e;
}

inline expr mkITE(const expr& i, const std::shared_ptr<expr>& t, const std::shared_ptr<expr>& e) {
    return i ? *t : *e;
}

inline expr mkITE(const std::shared_ptr<expr>& i, const std::shared_ptr<expr>& t, const std::shared_ptr<expr>& e) {
    return *i ? *t : *e;
}

inline expr mkImplies(const expr& a, const expr& b) {
    return implies(a, b);
}

inline expr mkImplies(const expr& a, const std::shared_ptr<expr>& b) {
    return implies(a, *b);
}

inline expr mkImplies(const std::shared_ptr<expr>& a, const expr& b) {
    return implies(*a, b);
}

inline expr mkImplies(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return implies(*a, *b);
}

inline expr mkNot(const expr& a) {
    return !a;
}

inline expr mkNot(const std::shared_ptr<expr>& a) {
    return !(*a);
}

inline expr mkEq(const expr& a, const expr& b) {
    return a == b;
}

inline expr mkEq(const expr& a, const std::shared_ptr<expr>& b) {
    return a == *b;
}

inline expr mkEq(const std::shared_ptr<expr>& a, const expr& b) {
    return *a == b;
}

inline expr mkEq(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a == *b;
}

inline expr mkGt(const expr& a, const expr& b) {
    return a > b;
}

inline expr mkGt(const expr& a, const std::shared_ptr<expr>& b) {
    return a > *b;
}

inline expr mkGt(const std::shared_ptr<expr>& a, const expr& b) {
    return *a > b;
}

inline expr mkGt(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a > *b;
}

inline expr mkLt(const expr& a, const expr& b) {
    return a < b;
}

inline expr mkLt(const expr& a, const std::shared_ptr<expr>& b) {
    return a < *b;
}

inline expr mkLt(const std::shared_ptr<expr>& a, const expr& b) {
    return *a < b;
}

inline expr mkLt(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a < *b;
}

inline expr mkGe(const expr& a, const expr& b) {
    return a >= b;
}

inline expr mkGe(const expr& a, const std::shared_ptr<expr>& b) {
    return a >= *b;
}

inline expr mkGe(const std::shared_ptr<expr>& a, const expr& b) {
    return *a >= b;
}

inline expr mkGe(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a >= *b;
}

inline expr mkLe(const expr& a, const expr& b) {
    return a <= b;
}

inline expr mkLe(const expr& a, const std::shared_ptr<expr>& b) {
    return a <= *b;
}

inline expr mkLe(const std::shared_ptr<expr>& a, const expr& b) {
    return *a <= b;
}

inline expr mkLe(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
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

inline expr mkOr(const expr& a, const expr& b) {
    return a || b;
}

inline expr mkOr(const expr& a, const std::shared_ptr<expr>& b) {
    return a || *b;
}

inline expr mkOr(const std::shared_ptr<expr>& a, const expr& b) {
    return *a || b;
}

inline expr mkOr(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
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

inline expr mkMul(const expr& a, const expr& b) {
    return a * b;
}

inline expr mkMul(const expr& a, const std::shared_ptr<expr>& b) {
    return a * *b;
}

inline expr mkMul(const std::shared_ptr<expr>& a, const expr& b) {
    return *a * b;
}

inline expr mkMul(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a * *b;
}

inline expr mkDiv(const expr& a, const expr& b) {
    return a / b;
}

inline expr mkDiv(const expr& a, const std::shared_ptr<expr>& b) {
    return a / *b;
}

inline expr mkDiv(const std::shared_ptr<expr>& a, const expr& b) {
    return *a / b;
}

inline expr mkDiv(const std::shared_ptr<expr>& a, const std::shared_ptr<expr>& b) {
    return *a / *b;
}

}

#endif

