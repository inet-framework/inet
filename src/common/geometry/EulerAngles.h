
#ifndef EULERALNGLES_H_
#define EULERALNGLES_H_

#include "FWMath.h"

namespace inet {

class EulerAngles
{
    public:
        // Constant with all values set to 0
        static const EulerAngles IDENTITY;

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

        EulerAngles operator+(const EulerAngles a) const {return EulerAngles(alpha+a.alpha, beta+a.beta, gamma+a.gamma);}

        EulerAngles operator-(const EulerAngles a) const {return EulerAngles(alpha-a.alpha, beta-a.beta, gamma-a.gamma);}

        EulerAngles operator*(float f) const {return EulerAngles(alpha*f, beta*f, gamma*f);}
};

inline std::ostream& operator<<(std::ostream& os, const EulerAngles& a)
{
    return os << "(" << a.alpha << ", " << a.beta << ", " << a.gamma << ")";
}

}


#endif /* EULERALNGLES_H_ */
