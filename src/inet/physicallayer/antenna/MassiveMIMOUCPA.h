#pragma once
#ifndef __INET_MassiveMIMOUCPA_H
#define __INET_MassiveMIMOUCPA_H

#include <iostream>
#include <vector>
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
class INET_API MassiveMIMOUCPA : public AntennaBase, protected cListener
{
    class Simpson2D3
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
    protected:

    private:
         Mat simpsonCoef;


    };


    public:
         static simsignal_t MassiveMIMOUCPAConfigureChange;
         static int M;

    protected:

         class AntennaGain : public IAntennaGain
           {
             protected:
               m length;
               int M = -1;
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
               MassiveMIMOUCPA *ourpa;

             public:
               AntennaGain(m length, int M, double phiz, double freq, double distance, double risInt, MassiveMIMOUCPA *ourpa ):
                   length(length),
                   M(M),
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
    static double risInt;
    bool pendingConfiguration = false;
    
  protected:
    virtual void initialize(int stage) override;

  public:
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    MassiveMIMOUCPA();
    ~MassiveMIMOUCPA() {

    }
    static int getM() {return M;}
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long d, cObject *details) override;

    double calcolaInt();


   // Consumption methods
//   virtual W getPowerConsumption() const override {return actualConsumption;}
/*   virtual W getPowerConsumption() const {return actualConsumption;}
   virtual void setConsumption()
   {
       if (energySource)
           energySource->setPowerConsumption(energyConsumerId,getPowerConsumption());
   }*/


};


} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MassiveMIMOUCPA_H
