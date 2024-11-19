//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211IRMODE_H
#define __INET_IEEE80211IRMODE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/PpmModulationBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211IrPreambleMode : public IIeee80211PreambleMode
{
  protected:
    const int syncSlotLength;

  public:
    Ieee80211IrPreambleMode(int syncSlotLength);

    int getSyncSlotLength() const { return syncSlotLength; }
    int getSFDSlotLength() const { return 4; }
    int getSlotLength() const { return getSyncSlotLength() + getSFDSlotLength(); }
    const simtime_t getSlotDuration() const { return 250E-9; }

    virtual const simtime_t getDuration() const override { return getSlotLength() * getSlotDuration(); }

    virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211IrPhyPreamble>(); }
};

class INET_API Ieee80211IrHeaderMode : public IIeee80211HeaderMode
{
  protected:
    const PpmModulationBase *modulation;

  public:
    Ieee80211IrHeaderMode(const PpmModulationBase *modulation);

    int getDRSlotLength() const { return 3; }
    int getDCLASlotLength() const { return 32; }
    b getLengthFieldLength() const { return b(16); }
    b getCrcFieldLength() const { return b(16); }
    int getSlotLength() const { return getDRSlotLength() + getDCLASlotLength(); }
    const simtime_t getSlotDuration() const { return 250E-9; }

    virtual b getLength() const override { return getLengthFieldLength() + getCrcFieldLength(); }
    virtual bps getNetBitrate() const override { return Mbps(1); }
    virtual bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual const simtime_t getDuration() const override { return (double)getLength().get<b>() / getNetBitrate().get<bps>() + getSlotLength() * getSlotDuration(); }
    virtual const simtime_t getSymbolInterval() const override { return -1; }
    virtual const PpmModulationBase *getModulation() const override { return modulation; }

    virtual Ptr<Ieee80211PhyHeader> createHeader() const override { return makeShared<Ieee80211IrPhyHeader>(); }
};

class INET_API Ieee80211IrDataMode : public IIeee80211DataMode
{
  protected:
    const PpmModulationBase *modulation;

  public:
    Ieee80211IrDataMode(const PpmModulationBase *modulation);

    virtual Hz getBandwidth() const override { return Hz(NaN); }
    virtual bps getNetBitrate() const override { return Mbps(1) * modulation->getConstellationSize() / 2; }
    virtual bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual b getPaddingLength(b dataLength) const override { return b(0); }
    virtual b getCompleteLength(b dataLength) const override { return dataLength; }
    virtual const simtime_t getDuration(b length) const override { return (double)length.get<b>() / getGrossBitrate().get<bps>(); }
    virtual const simtime_t getSymbolInterval() const override { return -1; }
    virtual const PpmModulationBase *getModulation() const override { return modulation; }
    virtual int getNumberOfSpatialStreams() const override { return 1; }
};

/**
 * Represents an Infrared PHY mode as described in IEEE 802.11-2012 specification
 * clause 15.
 */
class INET_API Ieee80211IrMode : public Ieee80211ModeBase
{
  protected:
    const Ieee80211IrPreambleMode *preambleMode;
    const Ieee80211IrHeaderMode *headerMode;
    const Ieee80211IrDataMode *dataMode;

  protected:
    virtual int getLegacyCwMin() const override { return 63; }
    virtual int getLegacyCwMax() const override { return 1023; }

  public:
    Ieee80211IrMode(const char *name, const Ieee80211IrPreambleMode *preambleMode, const Ieee80211IrHeaderMode *headerMode, const Ieee80211IrDataMode *dataMode);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211IrMode"; }

    virtual const IIeee80211PreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const IIeee80211HeaderMode *getHeaderMode() const override { return headerMode; }
    virtual const IIeee80211DataMode *getDataMode() const override { return dataMode; }

    virtual const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + headerMode->getDuration() + dataMode->getDuration(dataLength); }

    // TODO fill in
    virtual const simtime_t getSlotTime() const override { return 8E-6; }
    virtual const simtime_t getSifsTime() const override { return 10E-6; }
    virtual const simtime_t getCcaTime() const override { return 5E-6; }
    virtual const simtime_t getRifsTime() const override;
    virtual const simtime_t getPhyRxStartDelay() const override { return 57E-6; }
    virtual const simtime_t getRxTxTurnaroundTime() const override { return 0; }
    virtual const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual const simtime_t getPlcpHeaderLength() const override { return headerMode->getDuration(); }
    virtual int getMpduMaxLength() const override { return 2500; }
};

/**
 * Provides the compliant Infrared PHY modes as described in the IEEE 802.11-2012
 * specification clause 15.
 */
class INET_API Ieee80211IrCompliantModes
{
  public:
    // preamble modes
    static const Ieee80211IrPreambleMode irPreambleMode64SyncSlots;

    // header modes
    static const Ieee80211IrHeaderMode irHeaderMode1Mbps;
    static const Ieee80211IrHeaderMode irHeaderMode2Mbps;

    // data modes
    static const Ieee80211IrDataMode irDataMode1Mbps;
    static const Ieee80211IrDataMode irDataMode2Mbps;

    // modes
    static const Ieee80211IrMode irMode1Mbps;
    static const Ieee80211IrMode irMode2Mbps;
};

} // namespace physicallayer

} // namespace inet

#endif

