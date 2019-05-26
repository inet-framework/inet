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

#ifndef __INET_MATH_INTERPOLATOR_H_
#define __INET_MATH_INTERPOLATOR_H_

#include "inet/common/Units.h"

namespace inet {

namespace math {

using namespace inet::units::values;

template<typename X, typename Y>
class INET_API Interpolator
{
  public:
    virtual ~Interpolator() { }

    virtual Y get(const X x1, const Y y1, const X x2, const Y y2, const X x) const = 0;
};

template<typename X, typename Y>
class INET_API EitherInterpolator : public Interpolator<X, Y>
{
  public:
    virtual Y get(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        ASSERT(y1 == y2);
        return y1;
    }
};

template<typename X, typename Y>
class INET_API SmallerInterpolator : public Interpolator<X, Y>
{
  public:
    virtual Y get(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return y1;
    }
};

template<typename X, typename Y>
class INET_API GreaterInterpolator : public Interpolator<X, Y>
{
  public:
    virtual Y get(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return y2;
    }
};

template<typename X, typename Y>
class INET_API CloserInterpolator : public Interpolator<X, Y>
{
  public:
    virtual Y get(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        return x - x1 < x2 - x ? y1 : y2;
    }
};

template<typename X, typename Y>
class INET_API LinearInterpolator : public Interpolator<X, Y>
{
  public:
    virtual Y get(const X x1, const Y y1, const X x2, const Y y2, const X x) const override {
        ASSERT(x1 <= x && x <= x2);
        if (x1 == x2) {
            ASSERT(y1 == y2);
            return y1;
        }
        else {
            auto a = unit((x - x1) / (x2 - x1)).get();
            return y1 * (1 - a) + y2 * a;
        }
    }
};

template<typename X, typename Y>
const Interpolator<X, Y> *createInterpolator(const char *text) {
    if (!strcmp("either", text))
        return new EitherInterpolator<X, Y>();
    else if (!strcmp("smaller", text))
        return new SmallerInterpolator<X, Y>();
    else if (!strcmp("greater", text))
        return new GreaterInterpolator<X, Y>();
    else if (!strcmp("closer", text))
        return new CloserInterpolator<X, Y>();
    else if (!strcmp("linear", text))
        return new LinearInterpolator<X, Y>();
    else
        throw cRuntimeError("Unknown interpolator: '%s'", text);
}

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_INTERPOLATOR_H_

