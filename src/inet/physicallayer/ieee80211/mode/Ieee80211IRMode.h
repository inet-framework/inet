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

#ifndef __INET_IEEE80211IRMODE_H
#define __INET_IEEE80211IRMODE_H

#include "inet/physicallayer/base/packetlevel/PPMModulationBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211IrPreambleMode : public IIeee80211PreambleMode
{
  protected:
    const int syncSlotLength;

  public:
    Ieee80211IrPreambleMode(int syncSlotLength);

    inline int getSyncSlotLength() const { return syncSlotLength; }
    inline int getSFDSlotLength() const { return 4; }
    inline int getSlotLength() const { return getSyncSlotLength() + getSFDSlotLength(); }
    inline const simtime_t getSlotDuration() const { return 250E-9; }

    virtual inline const simtime_t getDuration() const override { return getSlotLength() * getSlotDuration(); }
};

class INET_API Ieee80211IrHeaderMode : public IIeee80211HeaderMode
{
  protected:
    const PPMModulationBase *modulation;

  public:
    Ieee80211IrHeaderMode(const PPMModulationBase *modulation);

    inline int getDRSlotLength() const { return 3; }
    inline int getDCLASlotLength() const { return 32; }
    inline int getLengthBitLength() const { return 16; }
    inline int getCRCBitLength() const { return 16; }
    inline int getSlotLength() const { return getDRSlotLength() + getDCLASlotLength(); }
    inline const simtime_t getSlotDuration() const { return 250E-9; }

    virtual inline int getBitLength() const override { return getLengthBitLength() + getCRCBitLength(); }
    virtual inline bps getNetBitrate() const override { return Mbps(1); }
    virtual inline bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual inline const simtime_t getDuration() const override { return getBitLength() / getNetBitrate().get() + getSlotLength() * getSlotDuration(); }
    virtual const PPMModulationBase *getModulation() const override { return modulation; }
};

class INET_API Ieee80211IrDataMode : public IIeee80211DataMode
{
  protected:
    const PPMModulationBase *modulation;

  public:
    Ieee80211IrDataMode(const PPMModulationBase *modulation);

    virtual inline bps getNetBitrate() const override { return Mbps(1) * modulation->getConstellationSize() / 2; }
    virtual inline bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual int getBitLength(int dataBitLength) const override { return dataBitLength; }
    virtual inline const simtime_t getDuration(int bitLength) const override { return bitLength / getGrossBitrate().get(); }
    virtual const PPMModulationBase *getModulation() const override { return modulation; }
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

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211IrMode"; }

    virtual const IIeee80211PreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const IIeee80211HeaderMode *getHeaderMode() const override { return headerMode; }
    virtual const IIeee80211DataMode *getDataMode() const override { return dataMode; }

    virtual inline const simtime_t getDuration(int dataBitLength) const override { return preambleMode->getDuration() + headerMode->getDuration() + dataMode->getDuration(dataBitLength); }

    // TODO: fill in
    virtual inline const simtime_t getSlotTime() const override { return 8E-6; }
    virtual inline const simtime_t getSifsTime() const override { return 10E-6; }
    virtual inline const simtime_t getCcaTime() const override { return 5E-6; }
    virtual const simtime_t getRifsTime() const override;
    virtual inline const simtime_t getPhyRxStartDelay() const override { return 57E-6; }
    virtual inline const simtime_t getRxTxTurnaroundTime() const override { return 0; }
    virtual inline const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual inline const simtime_t getPlcpHeaderLength() const override { return headerMode->getDuration(); }
    virtual inline int getMpduMaxLength() const override { return 2500; }
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

#endif // ifndef __INET_IEEE80211IRMODE_H

