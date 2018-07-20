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

#ifndef __INET_IEEE80211HRDSSSMODE_H
#define __INET_IEEE80211HRDSSSMODE_H

#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeBase.h"
#include "inet/physicallayer/modulation/DbpskModulation.h"
#include "inet/physicallayer/modulation/DqpskModulation.h"

namespace inet {

namespace physicallayer {

enum Ieee80211HrDsssPreambleType {
    IEEE80211_HRDSSS_PREAMBLE_TYPE_SHORT,
    IEEE80211_HRDSSS_PREAMBLE_TYPE_LONG,
};

class INET_API Ieee80211HrDsssPreambleMode : public IIeee80211PreambleMode
{
  protected:
    const Ieee80211HrDsssPreambleType preambleType;

  public:
    Ieee80211HrDsssPreambleMode(const Ieee80211HrDsssPreambleType preambleType);

    inline Ieee80211HrDsssPreambleType getPreambleType() const { return preambleType; }

    inline b getSyncFieldLength() const { return preambleType == IEEE80211_HRDSSS_PREAMBLE_TYPE_SHORT ? b(72) : b(128); }
    inline b getSfdFieldLength() const { return b(16); }
    inline b getBitLength() const { return getSyncFieldLength() + getSfdFieldLength(); }

    virtual inline bps getNetBitrate() const { return Mbps(1); }
    virtual inline bps getGrossBitrate() const { return getNetBitrate(); }
    virtual inline const simtime_t getDuration() const override { return (double)getBitLength().get() / getNetBitrate().get(); }
    virtual const DbpskModulation *getModulation() const { return &DbpskModulation::singleton; }

    virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211HrDsssPhyPreamble>(); }
};

class INET_API Ieee80211HrDsssHeaderMode : public IIeee80211HeaderMode
{
  protected:
    const Ieee80211HrDsssPreambleType preambleType;

  public:
    Ieee80211HrDsssHeaderMode(const Ieee80211HrDsssPreambleType preambleType);

    b getSignalFieldLength() const { return b(8); }
    b getServiceFieldLength() const { return b(8); }
    b getLengthFieldLength() const { return b(16); }
    b getCrcFieldLength() const { return b(16); }

    virtual inline b getLength() const override { return getSignalFieldLength() + getServiceFieldLength() + getLengthFieldLength() + getCrcFieldLength(); }
    virtual inline bps getNetBitrate() const override { return preambleType == IEEE80211_HRDSSS_PREAMBLE_TYPE_SHORT ? Mbps(2) : Mbps(1); }
    virtual inline bps getGrossBitrate() const override { return getNetBitrate(); }
    virtual inline const simtime_t getDuration() const override { return (double)getLength().get() / getNetBitrate().get(); }
    virtual const DpskModulationBase *getModulation() const override { return preambleType == IEEE80211_HRDSSS_PREAMBLE_TYPE_SHORT ? static_cast<const DpskModulationBase *>(&DqpskModulation::singleton) : static_cast<const DpskModulationBase *>(&DbpskModulation::singleton); }

    virtual Ptr<Ieee80211PhyHeader> createHeader() const override { return makeShared<Ieee80211HrDsssPhyHeader>(); }
};

class INET_API Ieee80211HrDsssDataMode : public IIeee80211DataMode
{
  protected:
    const bps bitrate;

  public:
    Ieee80211HrDsssDataMode(bps bitrate);

    virtual Hz getBandwidth() const override { return MHz(22); }
    virtual inline bps getNetBitrate() const override { return bitrate; }
    virtual inline bps getGrossBitrate() const override { return bitrate; }
    virtual b getPaddingLength(b dataLength) const override { return b(0); }
    virtual b getCompleteLength(b dataLength) const override { return dataLength; }
    virtual const simtime_t getDuration(b length) const override;
    virtual IModulation *getModulation() const override { return nullptr; } // TODO:
    virtual int getNumberOfSpatialStreams() const override { return 1; }
};

/**
 * Represents a High Rate Direct Sequence Spread Spectrum PHY mode as described
 * in the IEEE 802.11-2012 specification clause 17.
 */
class INET_API Ieee80211HrDsssMode : public Ieee80211ModeBase
{
  protected:
    const Ieee80211HrDsssPreambleMode *preambleMode;
    const Ieee80211HrDsssHeaderMode *headerMode;
    const Ieee80211HrDsssDataMode *dataMode;

  protected:
    virtual int getLegacyCwMin() const override { return 31; }
    virtual int getLegacyCwMax() const override { return 1023; }

  public:
    Ieee80211HrDsssMode(const char *name, const Ieee80211HrDsssPreambleMode *preambleMode, const Ieee80211HrDsssHeaderMode *headerMode, const Ieee80211HrDsssDataMode *dataMode);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211HrDsssMode"; }

    inline Hz getChannelSpacing() const { return MHz(5); }
    inline Hz getBandwidth() const { return MHz(22); }

    virtual const IIeee80211PreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const IIeee80211HeaderMode *getHeaderMode() const override { return headerMode; }
    virtual const IIeee80211DataMode *getDataMode() const override { return dataMode; }

    virtual inline const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + headerMode->getDuration() + dataMode->getDuration(dataLength); }

    // TODO: fill in
    virtual inline const simtime_t getSlotTime() const override { return 20E-6; }
    virtual inline const simtime_t getSifsTime() const override { return 10E-6; }
    virtual inline const simtime_t getCcaTime() const override { return 15E-6; }
    virtual const simtime_t getRifsTime() const override;
    virtual inline const simtime_t getPhyRxStartDelay() const override { return preambleMode->getPreambleType() == IEEE80211_HRDSSS_PREAMBLE_TYPE_LONG ? 192E-6 : 96E-6; }
    virtual inline const simtime_t getRxTxTurnaroundTime() const override { return 5E-6; }
    virtual inline const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual inline const simtime_t getPlcpHeaderLength() const override { return headerMode->getDuration(); }
    virtual inline int getMpduMaxLength() const override { return 4095; }
};

/**
 * Provides the compliant High Rate Direct Sequence Spread Spectrum PHY modes as
 * described in the IEEE 802.11-2012 specification clause 17.
 */
class INET_API Ieee80211HrDsssCompliantModes
{
  public:
    // preamble modes
    static const Ieee80211HrDsssPreambleMode hrDsssPreambleMode1MbpsLongPreamble;
    static const Ieee80211HrDsssPreambleMode hrDsssPreambleMode1MbpsShortPreamble;

    // header modes
    static const Ieee80211HrDsssHeaderMode hrDsssHeaderMode1MbpsLongPreamble;
    static const Ieee80211HrDsssHeaderMode hrDsssHeaderMode2MbpsShortPreamble;

    // data modes
    static const Ieee80211HrDsssDataMode hrDsssDataMode1MbpsLongPreamble;

    static const Ieee80211HrDsssDataMode hrDsssDataMode2MbpsLongPreamble;
    static const Ieee80211HrDsssDataMode hrDsssDataMode2MbpsShortPreamble;

    static const Ieee80211HrDsssDataMode hrDsssDataMode5_5MbpsCckLongPreamble;
    static const Ieee80211HrDsssDataMode hrDsssDataMode5_5MbpsPbccLongPreamble;
    static const Ieee80211HrDsssDataMode hrDsssDataMode5_5MbpsCckShortPreamble;

    static const Ieee80211HrDsssDataMode hrDsssDataMode11MbpsCckLongPreamble;
    static const Ieee80211HrDsssDataMode hrDsssDataMode11MbpsPbccLongPreamble;
    static const Ieee80211HrDsssDataMode hrDsssDataMode11MbpsCckShortPreamble;
    static const Ieee80211HrDsssDataMode hrDsssDataMode11MbpsPbccShortPreamble;

    // modes
    static const Ieee80211HrDsssMode hrDsssMode1MbpsLongPreamble;

    static const Ieee80211HrDsssMode hrDsssMode2MbpsLongPreamble;
    static const Ieee80211HrDsssMode hrDsssMode2MbpsShortPreamble;

    static const Ieee80211HrDsssMode hrDsssMode5_5MbpsCckLongPreamble;
    static const Ieee80211HrDsssMode hrDsssMode5_5MbpsPbccLongPreamble;
    static const Ieee80211HrDsssMode hrDsssMode5_5MbpsCckShortPreamble;

    static const Ieee80211HrDsssMode hrDsssMode11MbpsCckLongPreamble;
    static const Ieee80211HrDsssMode hrDsssMode11MbpsPbccLongPreamble;
    static const Ieee80211HrDsssMode hrDsssMode11MbpsCckShortPreamble;
    static const Ieee80211HrDsssMode hrDsssMode11MbpsPbccShortPreamble;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211HRDSSSMODE_H

