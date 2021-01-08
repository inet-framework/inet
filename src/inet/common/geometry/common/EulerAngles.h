#ifndef __INET_EULERANGLES_H
#define __INET_EULERANGLES_H

#include "inet/common/INETMath.h"
#include "inet/common/Units.h"

namespace inet {

using namespace inet::units::values;

/**
 * Orientations are represented by 3D double precision Tait-Bryan (Euler) tuples
 * called EulerAngles. The angles are in Z, Y', X" order that is often called
 * intrinsic rotations, they are measured in radians. The default (unrotated)
 * orientation is along the X axis. Conceptually, the Z axis rotation is heading,
 * the Y' axis rotation is descending (negative elevation), the X" axis rotation is bank.
 * For example, positive rotation along the Z axis rotates X into Y (turns left), positive
 * rotation along the Y axis rotates Z into X (leans forward), positive rotation
 * along the X axis rotates Y into Z (leans right).
 */
class INET_API EulerAngles
{
  public:
    // Constant with all values set to 0
    static const EulerAngles ZERO;
    static const EulerAngles NIL;

  public:
    // alpha, beta and gamma angle of the orientation
    rad alpha;
    rad beta;
    rad gamma;

  private:
    void copy(const EulerAngles& other) { alpha = other.alpha; beta = other.beta; gamma = other.gamma; }

  public:
    EulerAngles()
        : alpha(0.0), beta(0.0), gamma(0.0) {}

    EulerAngles(rad alpha, rad beta, rad gamma)
        : alpha(alpha), beta(beta), gamma(gamma) {}

    rad getAlpha() const { return alpha; }
    void setAlpha(rad alpha) { this->alpha = alpha; }

    rad getBeta() const { return beta; }
    void setBeta(rad beta) { this->beta = beta; }

    rad getGamma() const { return gamma; }
    void setGamma(rad gamma) { this->gamma = gamma; }

    inline std::string str() const;

    bool isNil() const
    {
        return this == &NIL;
    }

    bool isUnspecified() const
    {
        return std::isnan(alpha.get()) && std::isnan(beta.get()) && std::isnan(gamma.get());
    }

    EulerAngles& normalize()
    {
        alpha = rad(math::modulo(alpha.get(), 2 * M_PI));
        beta = rad(math::modulo(beta.get(), 2 * M_PI));
        gamma = rad(math::modulo(gamma.get(), 2 * M_PI));
        return *this;
    }

    EulerAngles operator+(const EulerAngles a) const { return EulerAngles(alpha + a.alpha, beta + a.beta, gamma + a.gamma); }

    EulerAngles operator-(const EulerAngles a) const { return EulerAngles(alpha - a.alpha, beta - a.beta, gamma - a.gamma); }

    EulerAngles operator*(float f) const { return EulerAngles(alpha * f, beta * f, gamma * f); }
};

inline std::ostream& operator<<(std::ostream& os, const EulerAngles& a)
{
    return os << "(" << a.alpha.get() << ", " << a.beta.get() << ", " << a.gamma.get() << ") rad";
}

inline std::string EulerAngles::str() const
{
    std::stringstream os;
    os << *this;
    return os.str();
}

} // namespace inet

#endif // ifndef __INET_EULERALNGLES_H

