//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef INET_LORA_LORAPHY_LORAMODE_H_
#define INET_LORA_LORAPHY_LORAMODE_H_

#include <vector>
#include <omnetpp/cobject.h>
#include "inet/physicallayer/contract/packetlevel/IModulation.h"
#include "inet/lora/loraphy/LoRaModulation.h"
#include "inet/lora/loraphy/LoraMode.h"

namespace inet {
namespace physicallayer {

class ILoraMode: public omnetpp::cObject, public IPrintableObject {
public:
    virtual int getLegacyCwMin() const = 0;
    virtual int getLegacyCwMax() const = 0;
    virtual const char *getName() const = 0;
    virtual Hz getBandwidth() const = 0;
    virtual int getSpreadingFator() const = 0;
    virtual const IModulation *getModulation() const = 0;
    virtual const simtime_t getDuration(b dataLength) const = 0;
    virtual const simtime_t getSlotTime() const = 0;
    virtual const simtime_t getSifsTime() const = 0;
    virtual const simtime_t getDifsTime() const = 0;
    virtual const simtime_t getCcaTime() const = 0;
    virtual const simtime_t getPhyRxStartDelay() const = 0;
    virtual const simtime_t getRxTxTurnaroundTime() const = 0;
    virtual const simtime_t getPreambleLength() const = 0;
    virtual const simtime_t getPayloadLength(b bits) const = 0;
    virtual const simtime_t getHeaderLength(b ) const = 0;
    virtual int getMpduMaxLength() const = 0;
    virtual int getCodeRate() const = 0;
    virtual void setPreambleSimbolsLength(const int &) = 0;
    virtual int getPreambleSimbolsLength() const  = 0;
    virtual void setCrcStatus(const bool &) = 0;
    virtual bool getCrcStatus() const  = 0;
    virtual void setHeaderEnable(const bool &p)  = 0;
    virtual bool getHeaderEnable() const  = 0;
};


class LoraMode: public ILoraMode {
private:
    std::string name;
    Hz bandwith;
    int spreadingFactor;
    LoRaModulation *modulation;
    int cwMin;
    int cwMax;
    b pading;
    b completeLen;
    simtime_t slot;
    simtime_t sif;
    simtime_t difs;
    simtime_t cca;
    simtime_t rxStartDelay;
    simtime_t rxTxTurn;
    int maxLeng = 0;
    double codeRate = 0;
    int preambleLength = 8;
    bool crc = true;
    int header = 1;

public:
    LoraMode(const char *name,
            Hz bandwith,
            int spreadingFactor,
            int cwMin,
            int cwMax,
            b pading,
            simtime_t slot,
            simtime_t sif,
            simtime_t difs,
            simtime_t cca,
            simtime_t rxStartDelay,
            simtime_t rxTxTurn,
            double codeRate);

    virtual void setPreambleSimbolsLength(const int &p) override {preambleLength = p;}
    virtual int getPreambleSimbolsLength() const override {return preambleLength;}
    virtual void setHeaderEnable(const bool &p) override;
    virtual bool getHeaderEnable() const override {return header == 0;}

    virtual void setCrcStatus(const bool &p) override {crc = p;}
    virtual bool getCrcStatus() const override {return crc;}

    virtual ~LoraMode();
    virtual const char *getName() const override { return name.c_str();}

    virtual int getCodeRate() const override  { return codeRate;};
    virtual int getLegacyCwMin() const override  { return cwMin;};
    virtual int getLegacyCwMax() const override  { return cwMax;};
    virtual Hz getBandwidth() const override  { return bandwith;};
    virtual int getSpreadingFator() const override  { return spreadingFactor;};
    virtual const IModulation *getModulation() const override  { return modulation;};
    virtual const simtime_t getDuration(b dataLength) const override;
    virtual const simtime_t getSlotTime() const override  { return slot;};
    virtual const simtime_t getSifsTime() const override  { return sif;};
    virtual const simtime_t getDifsTime() const override  { return difs;};
    virtual const simtime_t getCcaTime() const override  { return cca;};
    virtual const simtime_t getPhyRxStartDelay() const override  { return rxStartDelay;};
    virtual const simtime_t getRxTxTurnaroundTime() const override  { return rxTxTurn;};
    virtual const simtime_t getPreambleLength() const override;
    virtual const simtime_t getPayloadLength(b bits) const override;
    virtual const simtime_t getHeaderLength(b ) const override;
    virtual int getMpduMaxLength() const override  { return maxLeng;};
};

class INET_API LoraCompliantModes
{
  public:


    // preamble modes
    static const LoraMode EULoraD0;
    static const LoraMode EULoraD1;
    static const LoraMode EULoraD2;
    static const LoraMode EULoraD3;
    static const LoraMode EULoraD4;
    static const LoraMode EULoraD5;
    static const LoraMode EULoraD6;
   // static const LoraMode EULoraD7;

    static const LoraMode USALoraD0;
    static const LoraMode USALoraD1;
    static const LoraMode USALoraD2;
    static const LoraMode USALoraD3;
    static const LoraMode USALoraD4;
    static const LoraMode USALoraD8;
    static const LoraMode USALoraD9;
    static const LoraMode USALoraD10;
    static const LoraMode USALoraD11;
    static const LoraMode USALoraD12;
    static const LoraMode USALoraD13;

    //static const std::vector<LoraMode *> LoraModeEu;
    static const int LoraModeEuTotal;
    static const int LoraModeUsaTotal;
    static const LoraMode * LoraModeEu[];
    static const LoraMode * LoraModeUsa[];

};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* INET_LORA_LORAPHY_LORAMODE_H_ */
