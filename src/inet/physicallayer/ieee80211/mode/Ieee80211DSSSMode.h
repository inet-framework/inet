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

#ifndef __INET_IEEE80211DSSSMODE_H
#define __INET_IEEE80211DSSSMODE_H

#include "inet/physicallayer/modulation/DBPSKModulation.h"
#include "inet/physicallayer/modulation/DQPSKModulation.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211DsssChunkMode
{
};

class INET_API Ieee80211DsssPreambleMode : public Ieee80211DsssChunkMode, public IIeee80211PreambleMode
{
  public:
    Ieee80211DsssPreambleMode() {}

    inline int getSYNCBitLength() const { return 128; }
    inline int getSFDBitLength() const { return 16; }
    inline int getBitLength() const { return getSYNCBitLength() + getSFDBitLength(); }

    virtual inline bps getNetBitrate() const { return Mbps(1); }
    virtual inline bps getGrossBitrate() const { return getNetBitrate(); }
    virtual inline const simtime_t getDuration() const override { return getBitLength() / getNetBitrate().get(); }
    virtual const DBPSKModulation *getModulation() const { return &DBPSKModulation::singleton; }
};

class INET_API Ieee80211DsssHeaderMode : public Ieee80211DsssChunkMode, public IIeee80211HeaderMode
{
  public:
    Ieee80211DsssHeaderMode() {}

    inline int getSignalBitLength() const { return 8; }
    inline int getServiceBitLength() const { return 8; }
    inline int getLengthBitLength() const { return 16; }
    inline int getCRCBitLength() const { return 16; }

    virtual inline int getBitLength() const override { return getSignalBitLength() + getServiceBitLength() + getLengthBitLength() + getCRCBitLength(); }
    virtual inline bps getNetBitrate() const override { return Mbps(1); }
    virtual inline bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual inline const simtime_t getDuration() const override { return getBitLength() / getNetBitrate().get(); }
    virtual const DBPSKModulation *getModulation() const override { return &DBPSKModulation::singleton; }
};

class INET_API Ieee80211DsssDataMode : public Ieee80211DsssChunkMode, public IIeee80211DataMode
{
  protected:
    const DPSKModulationBase *modulation;

  public:
    Ieee80211DsssDataMode(const DPSKModulationBase *modulation);

    virtual inline bps getNetBitrate() const override { return Mbps(1) * modulation->getConstellationSize() / 2; }
    virtual inline bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual int getBitLength(int dataBitLength) const override { return dataBitLength; }
    virtual const simtime_t getDuration(int bitLength) const override;
    virtual const DPSKModulationBase *getModulation() const override { return modulation; }
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

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211DsssMode"; }

    virtual const IIeee80211PreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const IIeee80211HeaderMode *getHeaderMode() const override { return headerMode; }
    virtual const IIeee80211DataMode *getDataMode() const override { return dataMode; }

    virtual inline Hz getChannelSpacing() const { return MHz(5); }
    virtual inline Hz getBandwidth() const { return MHz(22); }

    virtual inline const simtime_t getDuration(int dataBitLength) const override { return preambleMode->getDuration() + headerMode->getDuration() + dataMode->getDuration(dataBitLength); }

    // Table 19-8—ERP characteristics
    virtual inline const simtime_t getSlotTime() const override { return 20E-6; }
    virtual inline const simtime_t getShortSlotTime() const { return 9E-6; }
    virtual inline const simtime_t getSifsTime() const override { return 10E-6; }
    virtual const simtime_t getRifsTime() const override;
    virtual inline const simtime_t getCcaTime() const override { return 15E-6; }
    virtual inline const simtime_t getPhyRxStartDelay() const override { return 192E-6; }
    virtual inline const simtime_t getRxTxTurnaroundTime() const override { return 5E-6; }
    virtual inline const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual inline const simtime_t getPlcpHeaderLength() const override { return headerMode->getDuration(); }
    virtual inline int getMpduMaxLength() const override { return 8191; }
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

#endif // ifndef __INET_IEEE80211DSSSMODE_H

