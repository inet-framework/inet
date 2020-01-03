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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_MATH_FUNCTIONCHECKER_H_
#define __INET_MATH_FUNCTIONCHECKER_H_

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
    FunctionChecker(const Ptr<const IFunction<R, D>>& function) : function(function) { }

    void check() const {
        check(function->getDomain());
    }

    void check(const typename D::I& i) const {
        function->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            auto check = std::function<void (const typename D::P&)>([&] (const typename D::P& p) {
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

#endif // #ifndef __INET_MATH_FUNCTIONCHECKER_H_

