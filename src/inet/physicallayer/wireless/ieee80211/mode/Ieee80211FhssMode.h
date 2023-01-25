//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211FHSSMODE_H
#define __INET_IEEE80211FHSSMODE_H

#include "inet/physicallayer/wireless/common/base/packetlevel/GfskModulationBase.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211FhssPreambleMode : public IIeee80211PreambleMode
{
  public:
    Ieee80211FhssPreambleMode() {}

    b getSyncFieldLength() const { return b(80); }
    b getSfdFieldLength() const { return b(16); }
    b getLength() const { return getSyncFieldLength() + getSfdFieldLength(); }

    virtual bps getNetBitrate() const { return Mbps(1); }
    virtual bps getGrossBitrate() const { return getNetBitrate(); }
    virtual const simtime_t getDuration() const override { return (double)getLength().get() / getNetBitrate().get(); }

    virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211FhssPhyPreamble>(); }
};

class INET_API Ieee80211FhssHeaderMode : public IIeee80211HeaderMode
{
  public:
    Ieee80211FhssHeaderMode() {}

    b getPlwFieldLength() const { return b(12); }
    b getPsfFieldLength() const { return b(4); }
    b getHecFieldLength() const { return b(16); }

    virtual b getLength() const override { return getPlwFieldLength() + getPsfFieldLength() + getHecFieldLength(); }
    virtual bps getNetBitrate() const override { return Mbps(1); }
    virtual bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual const simtime_t getDuration() const override { return (double)getLength().get() / getNetBitrate().get(); }
    virtual const simtime_t getSymbolInterval() const override { return -1; }
    virtual const GfskModulationBase *getModulation() const override { return nullptr; }

    virtual Ptr<Ieee80211PhyHeader> createHeader() const override { return makeShared<Ieee80211FhssPhyHeader>(); }
};

class INET_API Ieee80211FhssDataMode : public IIeee80211DataMode
{
  protected:
    const GfskModulationBase *modulation;

  public:
    Ieee80211FhssDataMode(const GfskModulationBase *modulation);

    virtual Hz getBandwidth() const override { return Hz(NaN); }
    virtual bps getNetBitrate() const override { return Mbps(1) * modulation->getConstellationSize() / 2; }
    virtual bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual b getPaddingLength(b dataLength) const override { return b(0); }
    virtual b getCompleteLength(b dataLength) const override { return dataLength; }
    virtual const simtime_t getDuration(b length) const override { return (double)length.get() / getNetBitrate().get(); }
    virtual const simtime_t getSymbolInterval() const override { return -1; }
    virtual const GfskModulationBase *getModulation() const override { return modulation; }
    virtual int getNumberOfSpatialStreams() const override { return 1; }
};

/**
 * Represents a Frequency-Hopping Spread Spectrum PHY mode as described in IEEE
 * 802.11-2012 specification clause 14.
 */
class INET_API Ieee80211FhssMode : public Ieee80211ModeBase
{
  protected:
    const Ieee80211FhssPreambleMode *preambleMode;
    const Ieee80211FhssHeaderMode *headerMode;
    const Ieee80211FhssDataMode *dataMode;

  protected:
    virtual int getLegacyCwMin() const override { return 15; }
    virtual int getLegacyCwMax() const override { return 1023; }

  public:
    Ieee80211FhssMode(const char *name, const Ieee80211FhssPreambleMode *preambleMode, const Ieee80211FhssHeaderMode *headerMode, const Ieee80211FhssDataMode *dataMode);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211FhssMode"; }

    virtual const IIeee80211PreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const IIeee80211HeaderMode *getHeaderMode() const override { return headerMode; }
    virtual const IIeee80211DataMode *getDataMode() const override { return dataMode; }

    virtual const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + headerMode->getDuration() + dataMode->getDuration(dataLength); }

    // TODO fill in
    virtual const simtime_t getSlotTime() const override { return 50E-6; }
    virtual const simtime_t getSifsTime() const override { return 28E-6; }
    virtual const simtime_t getRifsTime() const override;
    virtual const simtime_t getCcaTime() const override { return 27E-6; }
    virtual const simtime_t getPhyRxStartDelay() const override { return 128E-6; }
    virtual const simtime_t getRxTxTurnaroundTime() const override { return 20E-6; }
    virtual const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual const simtime_t getPlcpHeaderLength() const override { return headerMode->getDuration(); }
    virtual int getMpduMaxLength() const override { return 4095; }
};

/**
 * Provides the compliant Frequency-Hopping Spread Spectrum PHY modes as described
 * in the IEEE 802.11-2012 specification clause 14.
 */
class INET_API Ieee80211FhssCompliantModes
{
  public:
    // preamble modes
    static const Ieee80211FhssPreambleMode fhssPreambleMode1Mbps;

    // header modes
    static const Ieee80211FhssHeaderMode fhssHeaderMode1Mbps;

    // data modes
    static const Ieee80211FhssDataMode fhssDataMode1Mbps;
    static const Ieee80211FhssDataMode fhssDataMode2Mbps;

    // modes
    static const Ieee80211FhssMode fhssMode1Mbps;
    static const Ieee80211FhssMode fhssMode2Mbps;
};

} // namespace physicallayer

} // namespace inet

#endif

