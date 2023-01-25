//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211DSSSMODE_H
#define __INET_IEEE80211DSSSMODE_H

#include "inet/physicallayer/wireless/common/modulation/DbpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/DqpskModulation.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211DsssChunkMode
{
};

class INET_API Ieee80211DsssPreambleMode : public Ieee80211DsssChunkMode, public IIeee80211PreambleMode
{
  public:
    Ieee80211DsssPreambleMode() {}

    b getSyncFieldLength() const { return b(128); }
    b getSfdFieldLength() const { return b(16); }
    b getLength() const { return getSyncFieldLength() + getSfdFieldLength(); }

    virtual bps getNetBitrate() const { return Mbps(1); }
    virtual bps getGrossBitrate() const { return getNetBitrate(); }
    virtual const simtime_t getDuration() const override { return (double)getLength().get() / getNetBitrate().get(); }
    virtual const DbpskModulation *getModulation() const { return &DbpskModulation::singleton; }

    virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211DsssPhyPreamble>(); }
};

class INET_API Ieee80211DsssHeaderMode : public Ieee80211DsssChunkMode, public IIeee80211HeaderMode
{
  public:
    Ieee80211DsssHeaderMode() {}

    b getSignalFieldLength() const { return b(8); }
    b getServiceFieldLength() const { return b(8); }
    b getLengthFieldLength() const { return b(16); }
    b getCrcFieldLength() const { return b(16); }

    virtual b getLength() const override { return getSignalFieldLength() + getServiceFieldLength() + getLengthFieldLength() + getCrcFieldLength(); }
    virtual bps getNetBitrate() const override { return Mbps(1); }
    virtual bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual const simtime_t getDuration() const override { return (double)getLength().get() / getNetBitrate().get(); }
    virtual const simtime_t getSymbolInterval() const override { return -1; }
    virtual const DbpskModulation *getModulation() const override { return &DbpskModulation::singleton; }

    virtual Ptr<Ieee80211PhyHeader> createHeader() const override { return makeShared<Ieee80211DsssPhyHeader>(); }
};

class INET_API Ieee80211DsssDataMode : public Ieee80211DsssChunkMode, public IIeee80211DataMode
{
  protected:
    const DpskModulationBase *modulation;

  public:
    Ieee80211DsssDataMode(const DpskModulationBase *modulation);

    virtual Hz getBandwidth() const override { return MHz(22); }
    virtual bps getNetBitrate() const override { return Mbps(1) * modulation->getConstellationSize() / 2; }
    virtual bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual b getPaddingLength(b dataLength) const override { return b(0); }
    virtual b getCompleteLength(b dataLength) const override { return dataLength; }
    virtual const simtime_t getDuration(b length) const override;
    virtual const simtime_t getSymbolInterval() const override { return -1; }
    virtual const DpskModulationBase *getModulation() const override { return modulation; }
    virtual int getNumberOfSpatialStreams() const override { return 1; }
};

/**
 * Represents a Direct Sequence Spread Spectrum PHY mode as described in IEEE
 * 802.11-2012 specification clause 16.
 */
class INET_API Ieee80211DsssMode : public Ieee80211ModeBase
{
  protected:
    const Ieee80211DsssPreambleMode *preambleMode;
    const Ieee80211DsssHeaderMode *headerMode;
    const Ieee80211DsssDataMode *dataMode;

  protected:
    virtual int getLegacyCwMin() const override { return 31; }
    virtual int getLegacyCwMax() const override { return 1023; }

  public:
    Ieee80211DsssMode(const char *name, const Ieee80211DsssPreambleMode *preambleMode, const Ieee80211DsssHeaderMode *headerMode, const Ieee80211DsssDataMode *dataMode);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "Ieee80211DsssMode"; }

    virtual const IIeee80211PreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const IIeee80211HeaderMode *getHeaderMode() const override { return headerMode; }
    virtual const IIeee80211DataMode *getDataMode() const override { return dataMode; }

    virtual Hz getChannelSpacing() const { return MHz(5); }
    virtual Hz getBandwidth() const { return MHz(22); }

    virtual const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + headerMode->getDuration() + dataMode->getDuration(dataLength); }

    // Table 19-8—ERP characteristics
    virtual const simtime_t getSlotTime() const override { return 20E-6; }
    virtual const simtime_t getShortSlotTime() const { return 9E-6; }
    virtual const simtime_t getSifsTime() const override { return 10E-6; }
    virtual const simtime_t getRifsTime() const override;
    virtual const simtime_t getCcaTime() const override { return 15E-6; }
    virtual const simtime_t getPhyRxStartDelay() const override { return 192E-6; }
    virtual const simtime_t getRxTxTurnaroundTime() const override { return 5E-6; }
    virtual const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual const simtime_t getPlcpHeaderLength() const override { return headerMode->getDuration(); }
    virtual int getMpduMaxLength() const override { return 8191; }
};

/**
 * Provides the compliant Direct Sequence Spread Spectrum PHY modes as described
 * in the IEEE 802.11-2012 specification clause 16.
 */
class INET_API Ieee80211DsssCompliantModes
{
  public:
    // preamble modes
    static const Ieee80211DsssPreambleMode dsssPreambleMode1Mbps;

    // header modes
    static const Ieee80211DsssHeaderMode dsssHeaderMode1Mbps;

    // data modes
    static const Ieee80211DsssDataMode dsssDataMode1Mbps;
    static const Ieee80211DsssDataMode dsssDataMode2Mbps;

    // modes
    static const Ieee80211DsssMode dsssMode1Mbps;
    static const Ieee80211DsssMode dsssMode2Mbps;
};

} // namespace physicallayer
} // namespace inet

#endif

