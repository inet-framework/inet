#pragma once
#ifndef __INET_MASSIVEMIMOURPA_H
#define __INET_MASSIVEMIMOURPA_H

#include <iostream>
#include "inet/physicallayer/wireless/common/antenna/massivearray/MassiveArray.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/power/contract/IEnergySource.h"
#include "inet/power/contract/IEnergyStorage.h"
#include "inet/power/storage/SimpleEpEnergyStorage.h"
#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ScalarReceiver.h"

namespace inet {
namespace physicallayer {

using std::cout;
using namespace inet::power;
//extern double risInt;

//class INET_API MassiveMIMOURPA : public MassiveArray
class INET_API MassiveMIMOURPA : public MassiveArray
{
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

        IEnergySource *energySource = nullptr;
        int numAntennas;
        // internal state
        int energyConsumerId;
        double newConfigurtion = 0;
        W actualConsumption = W(0);
        MassiveMIMOURPA *ourpa;
        IRadio *radio = nullptr;

      public:
        AntennaGain(m length, int M, int N, double phiz, double freq, double distance, double risInt, MassiveMIMOURPA *ourpa, IRadio *radio):
            length(length),
            M(M),
            N(N),
            phiz(phiz),
            freq(freq),
            distance (distance),
            risInt(risInt),
            ourpa(ourpa),
            radio(radio){}
        virtual m getLength() const {return length;}
        virtual double getMinGain() const override {return 0;}
        virtual double getMaxGain() const override;
        virtual double computeGain(const Quaternion &direction) const override;
        virtual double getAngolo(Coord p1, Coord p2)const;
        virtual double getPhizero() {return phiz; }
        virtual void setPhizero(double o) {phiz = o; }
        virtual double computeRecGain(const rad &direction) const;
    };
    Ptr<AntennaGain> gain;
  protected:
    bool pendingConfiguration = false;
    double nextValue = NaN;
    virtual double computeIntegral();
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t d, cObject *details) override;

  public:
    virtual void setDirection(const double &angle) override;
    MassiveMIMOURPA();
    virtual Ptr<const IAntennaGain> getGain() const override { return gain; }
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};



} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MASSIVEMIMOURPA_H
