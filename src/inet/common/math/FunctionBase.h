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

#ifndef __INET_MATH_FUNCTIONBASE_H_
#define __INET_MATH_FUNCTIONBASE_H_

#include "inet/common/math/IFunction.h"

namespace inet {

namespace math {

template<typename R, typename D>
class INET_API ConstantFunction;

template<typename R, typename D>
class INET_API AddedFunction;

template<typename R, typename D>
class INET_API SubtractedFunction;

template<typename R, typename D>
class INET_API MultipliedFunction;

template<typename R, typename D>
class INET_API DividedFunction;

/**
 * Useful base class for most IFunction implementations with some default behavior.
 */
template<typename R, typename D>
class INET_API FunctionBase : public IFunction<R, D>
{
  public:
    virtual void partition(const typename D::I& i, const std::function<void (const typename D::I&, const IFunction<R, D> *)> callback) const override {
        auto m = (1 << std::tuple_size<typename D::P::type>::value) - 1;
        if (i.getFixed() == m) {
            ASSERT(i.getLower() == i.getUpper());
            ConstantFunction<R, D> g(this->getValue(i.getLower()));
            callback(i, &g);
        }
        else
            throw cRuntimeError("Cannot partition %s, interval = %s", this->getClassName(), i.str().c_str());
    }

    virtual Interval<R> getRange() const override {
        return getRange(getDomain());
    }

    virtual Interval<R> getRange(const typename D::I& i) const override {
        return Interval<R>(getLowerBound<R>(), getUpperBound<R>(), 0b1, 0b1, 0b0);
    }

    virtual typename D::I getDomain() const override {
        return typename D::I(D::P::getLowerBounds(), D::P::getUpperBounds(), 0b0, 0b0, 0b0);
    }

    virtual bool isFinite() const override { return isFinite(getDomain()); }
    virtual bool isFinite(const typename D::I& i) const override {
        bool result = true;
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            result &= f1->isFinite(i1);
        });
        return result;
    }

    virtual bool isNonZero() const override { return isNonZero(getDomain()); }
    virtual bool isNonZero(const typename D::I& i) const override {
        bool result = true;
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            result &= f1->isNonZero(i1);
        });
        return result;
    }

    virtual R getMin() const override { return getMin(getDomain()); }
    virtual R getMin(const typename D::I& i) const override {
        R result(getUpperBound<R>());
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            result = std::min(f1->getMin(i1), result);
        });
        return result;
    }

    virtual R getMax() const override { return getMax(getDomain()); }
    virtual R getMax(const typename D::I& i) const override {
        R result(getLowerBound<R>());
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            result = std::max(f1->getMax(i1), result);
        });
        return result;
    }

    virtual R getMean() const override { return getMean(getDomain()); }
    virtual R getMean(const typename D::I& i) const override {
        return getIntegral(i) / i.getVolume();
    }

    virtual R getIntegral() const override { return getIntegral(getDomain()); }
    virtual R getIntegral(const typename D::I& i) const override {
        R result(0);
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            double volume = i1.getVolume();
            R value = f1->getMean(i1);
            if (!(value == R(0) && std::isinf(volume)))
                result += volume * value;
        });
        return result;
    }

    virtual const Ptr<const IFunction<R, D>> add(const Ptr<const IFunction<R, D>>& o) const override {
        return makeShared<AddedFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<R, D>> subtract(const Ptr<const IFunction<R, D>>& o) const override {
        return makeShared<SubtractedFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<R, D>> multiply(const Ptr<const IFunction<double, D>>& o) const override {
        return makeShared<MultipliedFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual const Ptr<const IFunction<double, D>> divide(const Ptr<const IFunction<R, D>>& o) const override {
        return makeShared<DividedFunction<R, D>>(const_cast<FunctionBase<R, D> *>(this)->shared_from_this(), o);
    }

    virtual void print(std::ostream& os, int level = 0) const override {
        print(os, getDomain(), level);
    }

    virtual void print(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << std::string(level, ' ') << "function" << D() << " → ";
        printUnit(os, R());
        os << std::string(level, ' ') << " {\n  domain = " << i << " → range = " << getRange() << "\n";
        os << std::string(level, ' ') << "  structure =\n    ";
        printStructure(os, level + 4);
        os << std::string(level, ' ') << "\n";
        os << std::string(level, ' ') << "  partitioning = {\n";
        printPartitioning(os, i, level + 4);
        os << std::string(level, ' ') << "  } min = " << getMin(i) << ", max = " << getMax(i) << ", mean = " << getMean(i) << "\n}\n";
    }

    virtual void printPartitioning(std::ostream& os, const typename D::I& i, int level) const override {
        this->partition(i, [&] (const typename D::I& i1, const IFunction<R, D> *f1) {
            os << std::string(level, ' ');
            f1->printPartition(os, i1, level);
        });
    }

    virtual void printPartition(std::ostream& os, const typename D::I& i, int level = 0) const override {
        os << "over " << i << " → {";
        iterateCorners(i, std::function<void (const typename D::P&)>([&] (const typename D::P& p) {
            os << "\n" << std::string(level + 2, ' ') << "at " << p << " → " << this->getValue(p);
        }));
        os << "\n" << std::string(level, ' ') << "} min = " << getMin(i) << ", max = " << getMax(i) << ", mean = " << getMean(i) << "\n";
    }

    virtual void printStructure(std::ostream& os, int level = 0) const override {
        auto className = this->getClassName();
        if (!strncmp(className, "inet::math::", 12))
            className += 12;
        else if (!strncmp(className, "inet::", 6))
            className += 6;
        os << className;
    }
};

} // namespace math

} // namespace inet

#endif // #ifndef __INET_MATH_FUNCTIONBASE_H_

