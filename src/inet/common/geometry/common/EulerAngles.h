#ifndef __INET_EULERANGLES_H
#define __INET_EULERANGLES_H

#include "inet/common/INETMath.h"

namespace inet {

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

} // namespace inet

#endif // ifndef __INET_EULERALNGLES_H

