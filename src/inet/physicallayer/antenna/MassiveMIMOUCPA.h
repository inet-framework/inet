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


    public:
         static simsignal_t MassiveMIMOUCPAConfigureChange;
         static int M;

    protected:
    static double risInt;
    m length;
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

    
  protected:
    virtual void initialize(int stage) override;

  public:
    MassiveMIMOUCPA();
    ~MassiveMIMOUCPA() {

    }


virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual m getLength() const { return length; }
    virtual double getMaxGain() const override ;
     static int getM() {return M;}
   virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
   virtual void receiveSignal(cComponent *source, simsignal_t signalID, long d, cObject *details) override;

   virtual double getAngolo(Coord p1, Coord p2)const;
   virtual double computeGain(const EulerAngles direction) const override;
   virtual double computeRecGain(const EulerAngles &direction)const;
   virtual double getPhizero() {return phiz; }
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

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MassiveMIMOUCPA_H
