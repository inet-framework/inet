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
    int M = NaN;
    int N = NaN;
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
        static double Integral(double (*fun)(double,double, int, int), const Mat &coeff, limits xLimit, limits yLimits, int size, int M, int N);
        static double Integral(double (*fun)(double,double, int, int), limits xLimit, limits yLimits,int, int M, int N);
        Mat simpsonCoef;
    };
  protected:
    typedef std::tuple<int,int> ConfigAntenna;

    static std::map<int, double> risValuesUcpa;
    static std::map<int, double> risValuesUhpa;
    static std::map<ConfigAntenna, double> risValuesUrpa;
    double risInt;
    bool pendingConfiguration = false;
    virtual void initialize(int stage) override;
    static simsignal_t MassiveArrayConfigureChange;

  public:
    int getM() const {return M;}
    int getN() const {return N;}
    virtual void setDirection(const double &angle) = 0;
    MassiveArray();
    virtual Ptr<const IAntennaGain> getGain() const override = 0;
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override = 0;
};



} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MASSIVEMIMOURPA_H
