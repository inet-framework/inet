#ifndef __INET_QUATERNION_H
#define __INET_QUATERNION_H

#include "Coord.h"
#include "FWMath.h"
#include "EulerAngles.h"

namespace inet {

class Quaternion
{
    public:
        static const Quaternion IDENTITY;
    protected:
        float s;    // elforgad�si szog fel�nek koszinusza
        Coord d;   // elforgat�si tengely
    public:
        Quaternion() : d(0, 0, 1) { s = 0; }
        Quaternion( float s0, float x0 = 0, float y0 = 0, float z0 = 1 ) : d(x0, y0, z0) { s = s0; }
        Quaternion( float s0, Coord d0 ) : d( d0 ) { s = s0; }

        Quaternion(EulerAngles a)
        {
            s =       cos(a.gamma/2) * cos(a.beta/2) * cos(a.alpha/2) +  sin(a.gamma/2) * sin(a.beta/2) * sin(a.alpha/2);
            float x = sin(a.gamma/2) * cos(a.beta/2) * cos(a.alpha/2) -  cos(a.gamma/2) * sin(a.beta/2) * sin(a.alpha/2);
            float y = cos(a.gamma/2) * sin(a.beta/2) * cos(a.alpha/2) +  sin(a.gamma/2) * cos(a.beta/2) * sin(a.alpha/2);
            float z = cos(a.gamma/2) * cos(a.beta/2) * sin(a.alpha/2) -  sin(a.gamma/2) * sin(a.beta/2) * cos(a.alpha/2);
            d = Coord(x,y,z);
        }

        Quaternion(float m[3][3])
        {
            s = sqrt( m[0][0] + m[1][1] + m[2][2] + 1 ) / 2;
            d = Coord( m[1][2] - m[2][1], m[2][0] - m[0][2], m[0][1] - m[1][0] ) / (4 * s);
        }

        Quaternion operator+(Quaternion& q) {return Quaternion(s+q.s, d+q.d);}

        void operator+=(Quaternion& q) {s += q.s; d += q.d;}

        Quaternion operator*(float f) {return Quaternion(s*f, d*f);}

        float operator*(Quaternion& q) {return s*s + d*d;}

        void normalize() {(*this) = (*this) * (1/sqrt((*this) * (*this)));}

        Quaternion operator%(Quaternion& q) {return Quaternion(s*q.s - d*q.d, q.d*s + d*q.s + d%q.d);}

        EulerAngles getAngles()
        {

        }

        void getMatrix()
        {
            float m[3][3];
            m[0][0] = 1 - 2 * d.y * d.y - 2 * d.z * d.z;
            m[0][1] = 2 * d.x * d.y     + 2 * s * d.z;
            m[0][2] = 2 * d.x * d.z     - 2 * s * d.y;
            m[1][0] = 2 * d.x * d.y     - 2 * s * d.z;
            m[1][1] = 1 - 2 * d.x * d.x - 2 * d.z * d.z;
            m[1][2] = 2 * d.y * d.z     + 2 * s * d.x;
            m[2][0] = 2 * d.x * d.z     + 2 * s * d.y;
            m[2][1] = 2 * d.y * d.z     - 2 * s * d.x;
            m[2][2] = 1 - 2 * d.x * d.x - 2 * d.y * d.y;
        }

        float getRotationAngle() {return FWMath::rad2deg(atan2(d.length(), s) * 2);}

        Coord& getAxis() {return d;}        // forgat�si tengely
};

}


#endif /* QUATERNION_H_ */
