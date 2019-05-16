#pragma once
#ifndef __INET_MASSIVEMIMOURPA_H
#define __INET_MASSIVEMIMOURPA_H

#include <iostream>
#include "inet/physicallayer/base/packetlevel/AntennaBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/power/contract/IEnergySource.h"
#include "inet/power/contract/IEnergyStorage.h"
#include "inet/power/storage/SimpleEpEnergyStorage.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarReceiver.h"
#include "inet/physicallayer/antenna/IMassiveAntennaGain.h"

#include <tuple>
#include <vector>
namespace inet {

namespace physicallayer {

using std::cout;
using namespace inet::power;
//extern double risInt;
class INET_API MassiveMIMOURPA : public AntennaBase, protected cListener
{
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
         static double calcolaInt();
         static void initializeCoeff(Mat &coeff,int size);
         static double Integral(double (*fun)(double,double), const Mat &coeff, limits xLimit, limits yLimits, int size);
         static double Integral(double (*fun)(double,double), limits xLimit, limits yLimits,int);
    protected:

    private:
         Mat simpsonCoef;

    };

   protected:
    class AntennaGain : public IMassiveAntennaGain
    {
      protected:
        m length;
        static simsignal_t MassiveMIMOURPAConfigureChange;
        static int M;
        static int N;
        static double risInt;
        double freq;
        double distance;

        mutable double  phiz;
        IRadio *radio;
        IEnergySource *energySource;
        // internal state
        int energyConsumerId;
        bool pendingConfiguration = false;
        double newConfigurtion = 0;
        W actualConsumption = W(0);
      public:
        AntennaGain(const char *wireAxis, m length, double phiz);
        virtual m getLength() const override {return length;}
        virtual double getMinGain() const override;
        virtual double getMaxGain() const override;
        virtual double computeGain(const Quaternion direction) const override;
        virtual double getAngolo(Coord p1, Coord p2)const override;
        virtual double getPhizero() override {return phiz; }
    };

  protected:
    virtual void initialize(int stage) override;

  public:
    MassiveMIMOURPA();
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};



} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MASSIVEMIMOURPA_H
