#pragma once
#ifndef __INET_MASSIVEARRAY_H__
#define __INET_MASSIVEARRAY_H__

#include <iostream>
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"

#include <tuple>
#include <vector>
namespace inet {

namespace physicallayer {
using std::cout;
class INET_API MassiveArray : public AntennaBase, protected cListener
{
  public:
    static int M;
    static int N;
  protected:
    class Simpson2D
    {

    public:
        class limits {
            std::tuple<double,double> limit;
        public:
            void setUpper(double l) {std::get<1>(limit) = l;}
            void setLower(double l) {std::get<0>(limit) = l;}
            double getUpper() {return std::get<1>(limit);}
            double getLower() {return std::get<0>(limit);}
        };

        typedef std::vector<double> Vec;
        typedef std::vector<Vec> Mat;

        static void initializeCoeff(Mat &coeff,int size);
        static double Integral(double (*fun)(double,double), const Mat &coeff, limits xLimit, limits yLimits, int size);
        static double Integral(double (*fun)(double,double), limits xLimit, limits yLimits,int);
        Mat simpsonCoef;
    };
  protected:
    static double risInt;
    bool pendingConfiguration = false;
    virtual void initialize(int stage) override;
    static simsignal_t MassiveArrayConfigureChange;

  public:
    static int getM() {return M;}
    static int getN() {return N;}
    MassiveArray();
    virtual Ptr<const IAntennaGain> getGain() const override = 0;
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override = 0;
};



} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MASSIVEMIMOURPA_H
