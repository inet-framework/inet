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

#include <tuple>
#include <vector>
namespace inet {

namespace physicallayer {

using std::cout;
using namespace inet::power;
//extern double risInt;
class INET_API MassiveMIMOURPA : public AntennaBase, protected cListener
{
  public:
    static int M;
    static int N;
  private:

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

    private:
         Mat simpsonCoef;

    };

   protected:
    class AntennaGain : public IAntennaGain
    {
      protected:
        m length;
        int M = -1;
        int N = -1;
        double  phiz;
        double freq;
        double distance;
        double risInt;

        IRadio *radio = nullptr;
        IEnergySource *energySource = nullptr;
        int numAntennas;
        // internal state
        int energyConsumerId;
        double newConfigurtion = 0;
        W actualConsumption = W(0);
        MassiveMIMOURPA *ourpa;

      public:
        AntennaGain(m length, int M, int N, double phiz, double freq, double distance, double risInt, MassiveMIMOURPA *ourpa ):
            length(length),
            M(M),
            N(N),
            phiz(phiz),
            freq(freq),
            distance (distance),
            risInt(risInt),
            ourpa(ourpa) {}
        virtual m getLength() const {return length;}
        virtual double getMinGain() const override;
        virtual double getMaxGain() const override;
        virtual double computeGain(const Quaternion direction) const override;
        virtual double getAngolo(Coord p1, Coord p2)const;
        virtual double getPhizero() {return phiz; }
        virtual void setPhizero(double o) {phiz = o; }
        virtual double computeRecGain(const rad &direction) const;
    };

    Ptr<AntennaGain> gain;
  protected:
    static double risInt;

    bool pendingConfiguration = false;

    virtual void initialize(int stage) override;
    static simsignal_t MassiveMIMOURPAConfigureChange;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long d, cObject *details) override;

  public:
    static int getM() {return M;}
    static int getN() {return N;}
    MassiveMIMOURPA();
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};



} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MASSIVEMIMOURPA_H
