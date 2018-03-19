#ifndef __INET_EULERANGLES_H
#define __INET_EULERANGLES_H

#include "inet/common/INETMath.h"

namespace inet {

/**
 * Orientations are represented by 3D double precision Tait-Bryan (Euler) tuples
 * called EulerAngles. The angles are in Z, Y', X" order that is often called
 * intrinsic rotations, they are measured in radians. The default (unrotated)
 * orientation is along the X axis. Conceptually, the Z axis rotation is heading,
 * the Y' axis rotation is descending, the X" axis rotation is bank. For example,
 * positive rotation along the Z axis rotates X into Y (turns left), positive
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
    double alpha;
    double beta;
    double gamma;

  private:
    void copy(const EulerAngles& other) { alpha = other.alpha; beta = other.beta; gamma = other.gamma; }

  public:
    EulerAngles()
        : alpha(0.0), beta(0.0), gamma(0.0) {}

    EulerAngles(double alpha, double beta = 0.0, double gamma = 0.0)
        : alpha(alpha), beta(beta), gamma(gamma) {}

    double getAlpha() const { return alpha; }
    void setAlpha(double alpha) { this->alpha = alpha; }

    double getBeta() const { return beta; }
    void setBeta(double beta) { this->beta = beta; }

    double getGamma() const { return gamma; }
    void setGamma(double gamma) { this->gamma = gamma; }

    std::string str() const;

    bool isNil() const
    {
        return this == &NIL;
    }

    bool isUnspecified() const
    {
        return std::isnan(alpha) && std::isnan(beta) && std::isnan(gamma);
    }

    EulerAngles operator+(const EulerAngles a) const { return EulerAngles(alpha + a.alpha, beta + a.beta, gamma + a.gamma); }

    EulerAngles operator-(const EulerAngles a) const { return EulerAngles(alpha - a.alpha, beta - a.beta, gamma - a.gamma); }

    EulerAngles operator*(float f) const { return EulerAngles(alpha * f, beta * f, gamma * f); }
};

inline std::ostream& operator<<(std::ostream& os, const EulerAngles& a)
{
    return os << "(" << a.alpha << ", " << a.beta << ", " << a.gamma << ")";
}

inline std::string EulerAngles::str() const
{
    std::stringstream os;
    os << *this;
    return os.str();
}

} // namespace inet

#endif // ifndef __INET_EULERALNGLES_H

