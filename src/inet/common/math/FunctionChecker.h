//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FUNCTIONCHECKER_H
#define __INET_FUNCTIONCHECKER_H

#include "inet/common/math/IFunction.h"

namespace inet {

namespace math {

using namespace inet::units::values;

/**
 * Verifies that partitioning on a domain is correct in the sense that the
 * original function and the function over the partition return the same
 * values at the corners and center of the subdomain.
 */
template<typename R, typename D>
class INET_API FunctionChecker
{
  protected:
    const Ptr<const IFunction<R, D>> function;

  public:
    FunctionChecker(const Ptr<const IFunction<R, D>>& function) : function(function) {}

    void check() const {
        check(function->getDomain());
    }

    void check(const typename D::I& i) const {
        function->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            auto check = std::function<void(const typename D::P&)>([&] (const typename D::P& p) {
                if (i1.contains(p)) {
                    R r = function->getValue(p);
                    R r1 = f1->getValue(p);
                    ASSERT(r == r1 || (std::isnan(toDouble(r)) && std::isnan(toDouble(r1))));
                }
            });
            iterateCorners(i1, check);
            check((i1.getLower() + i1.getUpper()) / 2);
        });
    }
};

} // namespace math

} // namespace inet

#endif

