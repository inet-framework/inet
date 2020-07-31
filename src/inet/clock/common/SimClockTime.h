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

#ifndef __INET_SIMCLOCKTIME_H
#define __INET_SIMCLOCKTIME_H

#include "inet/common/INETDefs.h"

namespace inet {

#ifndef WITH_CLOCK_SUPPORT

#define SimClockTime SimTime
#define CLOCKTIME_AS_SIMTIME(x)  (x)
#define SIMTIME_AS_CLOCKTIME(x)  (x)

#else

/**
 * This class is a proxy for the SimTime class. The reason behind this design
 * is that the simulation time and the clock time must not be confused. So this
 * class doesn't have implicit conversion operators to and from the simulation
 * time.
 */
class INET_API SimClockTime
{
  private:
    SimTime impl;

  public:
    /**
     * Represents the zero simulation time. Using SimClockTime::ZERO may be faster
     * than writing SimClockTime() or conversion from 0.0, because the constructor
     * call is spared.
     */
    static const SimClockTime ZERO;

  public:
    /** @name Constructors */
    //@{
    /**
     * Constructor initializes to zero.
     */
    SimClockTime() {}

    /**
     * Initialize simulation time from a double-precision number. This constructor
     * is recommended if the value is the result of some computation done in
     * <tt>double</tt>. For integer-based computations and time constants, the
     * <tt>SimClockTime(int64_t x, int exponent)</tt> constructor is usually a better
     * choice, because it does not have rounding errors caused by double-to-integer
     * conversion.
     */
    SimClockTime(double d) {operator=(d);}

    /**
     * Initialize simulation time from a module or channel parameter. It uses
     * conversion to <tt>double</tt>. It currently does not check the measurement
     * unit of the parameter (@unit NED property), although this may change in
     * future releases.
     */
    SimClockTime(cPar& d) {operator=(d);}

    /**
     * Initialize simulation time from value specified in the given units.
     * This constructor allows one to specify precise constants, without
     * conversion errors incurred by the <tt>double</tt> constructor.
     * An error will be thrown if the resulting value cannot be represented
     * (overflow) or it cannot be represented precisely (precision loss) with
     * the current resolution (i.e. scale exponent). Note that the unit
     * parameter actually represents a base-10 exponent, so the constructor
     * will also work correctly for values not in the SimTimeUnit enum.
     *
     * Examples:
     *   Simtime(15, SIMTIME_US) -> 15us
     *   Simtime(-3, SIMTIME_S) -> -1s
     *   Simtime(5, (SimTimeUnit)2) -> 500s;
     */
    SimClockTime(int64_t value, SimTimeUnit unit) : impl(value, unit) {}

    /**
     * Copy constructor.
     */
    SimClockTime(const SimClockTime& x) : impl(x.impl) {}
    //@}

    /** @name Arithmetic operations */
    //@{
    const SimClockTime& operator=(const SimClockTime& x) {impl=x.impl; return *this;}
    const SimClockTime& operator=(const cPar& d) {impl=d; return *this;}
    const SimClockTime& operator=(double d) {impl=d; return *this;}
    const SimClockTime& operator=(short d) {impl=d; return *this;}
    const SimClockTime& operator=(int d) {impl=d; return *this;}
    const SimClockTime& operator=(long d) {impl=d; return *this;}
    const SimClockTime& operator=(long long d) {impl=d; return *this;}
    const SimClockTime& operator=(unsigned short d) {impl=d; return *this;}
    const SimClockTime& operator=(unsigned int d) {impl=d; return *this;}
    const SimClockTime& operator=(unsigned long d) {impl=d; return *this;}
    const SimClockTime& operator=(unsigned long long d) {impl=d; return *this;}

    bool operator==(const SimClockTime& x) const  {return impl==x.impl;}
    bool operator!=(const SimClockTime& x) const  {return impl!=x.impl;}
    bool operator< (const SimClockTime& x) const  {return impl<x.impl;}
    bool operator> (const SimClockTime& x) const  {return impl>x.impl;}
    bool operator<=(const SimClockTime& x) const  {return impl<=x.impl;}
    bool operator>=(const SimClockTime& x) const  {return impl>=x.impl;}

    SimClockTime operator-() const  {return from(-impl);}

    const SimClockTime& operator+=(const SimClockTime& x) {impl+=x.impl; return *this;}
    const SimClockTime& operator-=(const SimClockTime& x) {impl-=x.impl; return *this;}
    friend const SimClockTime operator+(const SimClockTime& x, const SimClockTime& y)  { return SimClockTime(x)+=y; }
    friend const SimClockTime operator-(const SimClockTime& x, const SimClockTime& y) { return SimClockTime(x)-=y; }

    const SimClockTime& operator*=(double d) {impl*=d; return *this;}
    const SimClockTime& operator*=(short d) {impl*=d; return *this;}
    const SimClockTime& operator*=(int d) {impl*=d; return *this;}
    const SimClockTime& operator*=(long d) {impl*=d; return *this;}
    const SimClockTime& operator*=(long long d) {impl*=d; return *this;}
    const SimClockTime& operator*=(unsigned short d) {impl*=d; return *this;}
    const SimClockTime& operator*=(unsigned int d) {impl*=d; return *this;}
    const SimClockTime& operator*=(unsigned long d) {impl*=d; return *this;}
    const SimClockTime& operator*=(unsigned long long d) {impl*=d; return *this;}
    const SimClockTime& operator*=(const cPar& p) {impl*=p; return *this;}

    const SimClockTime& operator/=(double d) {impl/=d; return *this;}
    const SimClockTime& operator/=(short d) {impl/=d; return *this;}
    const SimClockTime& operator/=(int d) {impl/=d; return *this;}
    const SimClockTime& operator/=(long d) {impl/=d; return *this;}
    const SimClockTime& operator/=(long long d) {impl/=d; return *this;}
    const SimClockTime& operator/=(unsigned short d) {impl/=d; return *this;}
    const SimClockTime& operator/=(unsigned int d) {impl/=d; return *this;}
    const SimClockTime& operator/=(unsigned long d) {impl/=d; return *this;}
    const SimClockTime& operator/=(unsigned long long d) {impl/=d; return *this;}
    const SimClockTime& operator/=(const cPar& p) {impl/=p; return *this;}

    friend const SimClockTime operator*(const SimClockTime& x, double d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, short d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, int d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, long d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, long long d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, unsigned short d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, unsigned int d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, unsigned long d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, unsigned long long d) {return from(x.impl*d);}
    friend const SimClockTime operator*(const SimClockTime& x, const cPar& p) {return from(x.impl*p);}

    friend const SimClockTime operator*(double d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(short d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(int d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(long d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(long long d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(unsigned short d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(unsigned int d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(unsigned long d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(unsigned long long d, const SimClockTime& x) {return from(d*x.impl);}
    friend const SimClockTime operator*(const cPar& p, const SimClockTime& x) {return from(p*x.impl);}

    friend const SimClockTime operator/(const SimClockTime& x, double d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, short d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, int d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, long d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, long long d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, unsigned short d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, unsigned int d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, unsigned long d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, unsigned long long d) {return from(x.impl/d);}
    friend const SimClockTime operator/(const SimClockTime& x, const cPar& p) {return from(x.impl/p);}

    friend double operator/(const SimClockTime& x, const SimClockTime& y) {return x.impl/y.impl;}

    friend double operator/(double x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(short x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(int x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(long x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(long long x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(unsigned short x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(unsigned int x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(unsigned long x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(unsigned long long x, const SimClockTime& y) {return x/y.impl;}
    friend double operator/(const cPar& p, const SimClockTime& x) {return p/x.impl;}
    //@}

    /** @name Misc operations and utilities */
    //@{
    /**
     * Convert from SimTime.
     */
    static SimClockTime from(SimTime t) {SimClockTime tmp; tmp.impl=t; return tmp;}

    /**
     * Convert to SimTime.
     */
    SimTime asSimTime() const {return impl;}

    /**
     * Returns true if this simulation time is zero, false otherwise. This is
     * more efficient than comparing the variable to a (double) 0.0, and shorter
     * than comparing against SimClockTime::ZERO.
     */
    bool isZero() const {return impl.isZero();}

    /**
     * Converts simulation time (in seconds) to a double. Note that conversion to
     * and from double may lose precision. We do not provide implicit conversion
     * to double as it would conflict with other overloaded operators, and would
     * cause ambiguities during compilation.
     */
    double dbl() const  {return impl.dbl();}

    /**
     * Converts the simulation time to the given time unit, discarding the
     * fractional part (i.e. rounding towards zero). If the return type is
     * not wide enough to hold the result, an error will be thrown.
     *
     * Examples:
     *   1.7ms in us --> 1700;
     *   3.8ms in s --> 3;
     *   -3.8ms in s --> -3;
     *   999ms in s --> 0
     */
    int64_t inUnit(SimTimeUnit unit) const {return impl.inUnit(unit);}

    /**
     * Returns a new simulation time that is truncated (rounded towards zero)
     * to the precision of the specified time unit.
     *
     * Examples:
     *   3750ms truncated to s --> 3;
     *   -3750ms truncated to s --> -3
     */
    SimClockTime trunc(SimTimeUnit unit) const {return from(impl.trunc(unit));}

    /**
     * Returns a simtime that is the difference between the simulation time and
     * its truncated value (see trunc()).
     *
     * That is, t == t.trunc(unit) + t.remainderforUnit(unit) for any unit.
     */
    SimClockTime remainderForUnit(SimTimeUnit unit) const {return from(impl.remainderForUnit(unit));}

    /**
     * Convenience method: splits the simulation time into a whole and a
     * fractional part with regard to a time unit.
     * <tt>t.split(exponent, outValue, outRemainder)</tt> is equivalent to:
     *
     * <pre>
     * outValue = t.inUnit(unit);
     * outRemainder = t.remainderForUnit(unit);
     * </pre>
     */
    void split(SimTimeUnit unit, int64_t& outValue, SimClockTime& outRemainder) const {impl.split(unit, outValue, outRemainder.impl);}

    /**
     * Converts the time to a numeric string. The number expresses the simulation
     * time precisely (including all significant digits), in seconds.
     * The measurement unit (seconds) is not part of the string.
     */
    std::string str() const {return impl.str();}

    /**
     * Converts to a string in the same way as str() does. Use this variant
     * over str() where performance is critical. The result is placed somewhere
     * in the given buffer (pointer is returned), but for performance reasons,
     * not necessarily at the buffer's beginning. Please read the documentation
     * of ttoa() for the minimum required buffer size.
     */
    char *str(char *buf) const {return impl.str(buf);}

    /**
     * Converts the time to a numeric string with unit in the same format as
     * ustr(SimTimeUnit) does, but chooses the time unit automatically.
     * The function tries to choose the "natural" time unit, e.g. 0.0033 is
     * returned as "3.3ms".
     */
    std::string ustr() const {return impl.ustr();}

    /**
     * Converts the time to a numeric string in the given time unit. The unit can
     * be "s", "ms", "us", "ns", "ps", "fs", or "as". The result represents
     * the simulation time precisely (includes all significant digits), and the
     * unit is appended.
     */
    std::string ustr(SimTimeUnit unit) const {return impl.ustr(unit);}

    /**
     * Returns the underlying 64-bit integer.
     */
    int64_t raw() const  {return impl.raw();}

    /**
     * Directly sets the underlying 64-bit integer.
     */
    const SimClockTime& setRaw(int64_t l) {impl.setRaw(l); return *this;}

    /**
     * Returns the largest simulation time that can be represented using the
     * present scale exponent.
     */
    static const SimClockTime getMaxTime() {return from(SimTime::getMaxTime());}

    /**
     * Returns the time resolution as the number of units per second,
     * e.g. for microsecond resolution it returns 1000000.
     */
    static int64_t getScale()  {return SimTime::getScale();}

    /**
     * Returns the scale exponent, which is an integer in the range -18..0.
     * For example, for microsecond resolution it returns -6.
     */
    static int getScaleExp() {return SimTime::getScaleExp();}

    /**
     * Converts the given string to simulation time. Throws an error if
     * there is an error during conversion. Accepted format is: \<number\>
     * or (\<number\>\<unit\>)+.
     */
    static const SimClockTime parse(const char *s) {return from(SimTime::parse(s));}

    /**
     * Converts a prefix of the given string to simulation time, up to the
     * first character that cannot occur in simulation time strings:
     * not (digit or letter or "." or "+" or "-" or whitespace).
     * Throws an error if there is an error during conversion of the prefix.
     * If the prefix is empty (whitespace only), then 0 is returned, and
     * endp is set equal to s; otherwise,  endp is set to point to the
     * first character that was not converted.
     */
    static const SimClockTime parse(const char *s, const char *&endp) {return from(SimTime::parse(s, endp));}

    /**
     * Utility function to convert a 64-bit fixed point number into a string
     * buffer. scaleexp must be in the -18..0 range, and the buffer must be
     * at least 64 bytes long. A pointer to the result string will be
     * returned. A pointer to the terminating '\\0' will be returned in endp.
     *
     * ATTENTION: For performance reasons, the returned pointer will point
     * *somewhere* into the buffer, but NOT necessarily at the beginning.
     */
    static char *ttoa(char *buf, int64_t impl, int scaleexp, char *&endp) {return SimTime::ttoa(buf, impl, scaleexp, endp);}
    //@}
};

inline std::ostream& operator<<(std::ostream& os, const SimClockTime& x)
{
    char buf[64]; char *endp;
    return os << SimClockTime::ttoa(buf, x.raw(), SimClockTime::getScaleExp(), endp);
}

/**
 * Conversion.
 */
#define CLOCKTIME_AS_SIMTIME(x)  (x).asSimTime()

/**
 * Conversion.
 */
#define SIMTIME_AS_CLOCKTIME(x)  SimClockTime::from(x)


#endif // WITH_CLOCK_SUPPORT

typedef SimClockTime simclocktime_t;

/**
 * The maximum representable simulation time with the current resolution.
 */
#define SIMCLOCKTIME_MAX    SimClockTime::getMaxTime()

/**
 * Constant for zero simulation time. Using SIMCLOCKTIME_ZERO can be more efficient
 * than using the 0 constant.
 */
#define SIMCLOCKTIME_ZERO   SimClockTime::ZERO

} // namespace inet

#endif // ifndef __INET_SIMCLOCKTIME_H

