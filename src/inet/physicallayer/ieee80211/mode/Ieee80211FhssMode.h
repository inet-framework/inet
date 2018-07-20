//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211FHSSMODE_H
#define __INET_IEEE80211FHSSMODE_H

#include "inet/physicallayer/base/packetlevel/GfskModulationBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211FhssPreambleMode : public IIeee80211PreambleMode
{
  public:
    Ieee80211FhssPreambleMode() {}

    inline b getSyncFieldLength() const { return b(80); }
    inline b getSfdFieldLength() const { return b(16); }
    inline b getLength() const { return getSyncFieldLength() + getSfdFieldLength(); }

    virtual inline bps getNetBitrate() const { return Mbps(1); }
    virtual inline bps getGrossBitrate() const { return getNetBitrate(); }
    virtual inline const simtime_t getDuration() const override { return (double)getLength().get() / getNetBitrate().get(); }

    virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211FhssPhyPreamble>(); }
};

class INET_API Ieee80211FhssHeaderMode : public IIeee80211HeaderMode
{
  public:
    Ieee80211FhssHeaderMode() {}

    b getPlwFieldLength() const { return b(12); }
    b getPsfFieldLength() const { return b(4); }
    b getHecFieldLength() const { return b(16); }

    virtual inline b getLength() const override { return getPlwFieldLength() + getPsfFieldLength() + getHecFieldLength(); }
    virtual inline bps getNetBitrate() const override { return Mbps(1); }
    virtual inline bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual inline const simtime_t getDuration() const override { return (double)getLength().get() / getNetBitrate().get(); }
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
    virtual inline bps getNetBitrate() const override { return Mbps(1) * modulation->getConstellationSize() / 2; }
    virtual inline bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual b getPaddingLength(b dataLength) const override { return b(0); }
    virtual b getCompleteLength(b dataLength) const override { return dataLength; }
    virtual inline const simtime_t getDuration(b length) const override { return (double)length.get() / getNetBitrate().get(); }
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
    virtual inline int getLegacyCwMin() const override { return 15; }
    virtual inline int getLegacyCwMax() const override { return 1023; }

  public:
    Ieee80211FhssMode(const char *name, const Ieee80211FhssPreambleMode *preambleMode, const Ieee80211FhssHeaderMode *headerMode, const Ieee80211FhssDataMode *dataMode);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211FhssMode"; }

    virtual const IIeee80211PreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const IIeee80211HeaderMode *getHeaderMode() const override { return headerMode; }
    virtual const IIeee80211DataMode *getDataMode() const override { return dataMode; }

    virtual inline const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + headerMode->getDuration() + dataMode->getDuration(dataLength); }

    // TODO: fill in
    virtual inline const simtime_t getSlotTime() const override { return 50E-6; }
    virtual inline const simtime_t getSifsTime() const override { return 28E-6; }
    virtual const simtime_t getRifsTime() const override;
    virtual inline const simtime_t getCcaTime() const override { return 27E-6; }
    virtual inline const simtime_t getPhyRxStartDelay() const override { return 128E-6; }
    virtual inline const simtime_t getRxTxTurnaroundTime() const override { return 20E-6; }
    virtual inline const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual inline const simtime_t getPlcpHeaderLength() const override { return headerMode->getDuration(); }
    virtual inline int getMpduMaxLength() const override { return 4095; }
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

#endif // ifndef __INET_IEEE80211FHSSMODE_H

