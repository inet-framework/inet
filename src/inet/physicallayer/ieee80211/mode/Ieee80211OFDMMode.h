//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_IEEE80211OFDMMODE_H
#define __INET_IEEE80211OFDMMODE_H

#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMTimingRelatedParametersBase
{
  protected:
    Hz channelSpacing;

  public:
    Ieee80211OFDMTimingRelatedParametersBase(Hz channelSpacing) : channelSpacing(channelSpacing) {}

    Hz getSubcarrierFrequencySpacing() const { return channelSpacing / 64; }
    const simtime_t getFFTTransformPeriod() const { return simtime_t(1 / getSubcarrierFrequencySpacing().get()); }
    const simtime_t getGIDuration() const { return getFFTTransformPeriod() / 4; }
    const simtime_t getSymbolInterval() const { return getGIDuration() + getFFTTransformPeriod(); }

    const Hz getChannelSpacing() const { return channelSpacing; }
};

class INET_API Ieee80211OFDMModeBase : public Ieee80211OFDMTimingRelatedParametersBase
{
  protected:
    const Ieee80211OFDMModulation *modulation;
    const Ieee80211OFDMCode *code;
    const Hz bandwidth;
    mutable bps netBitrate; // cached
    mutable bps grossBitrate; // cached

  protected:
    bps computeGrossBitrate(const Ieee80211OFDMModulation *modulation) const;
    bps computeNetBitrate(bps grossBitrate, const Ieee80211OFDMCode *code) const;

  public:
    Ieee80211OFDMModeBase(const Ieee80211OFDMModulation *modulation, const Ieee80211OFDMCode *code, Hz channelSpacing, Hz bandwidth);
    virtual ~Ieee80211OFDMModeBase() {}

    int getNumberOfDataSubcarriers() const { return 48; }
    int getNumberOfPilotSubcarriers() const { return 4; }
    int getNumberOfTotalSubcarriers() const { return getNumberOfDataSubcarriers() + getNumberOfPilotSubcarriers(); }

    virtual bps getGrossBitrate() const;
    virtual bps getNetBitrate() const;
    Hz getBandwidth() const { return bandwidth; }
};

class INET_API Ieee80211OFDMPreambleMode : public IIeee80211PreambleMode, public Ieee80211OFDMTimingRelatedParametersBase
{
  public:
    Ieee80211OFDMPreambleMode(Hz channelSpacing);
    virtual ~Ieee80211OFDMPreambleMode() {}

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    const simtime_t getTrainingSymbolGIDuration() const { return getFFTTransformPeriod() / 2; }
    const simtime_t getShortTrainingSequenceDuration() const { return 10 * getFFTTransformPeriod() / 4; }
    const simtime_t getLongTrainingSequenceDuration() const { return getTrainingSymbolGIDuration() + 2 * getFFTTransformPeriod(); }
    virtual const simtime_t getDuration() const override { return getShortTrainingSequenceDuration() + getLongTrainingSequenceDuration(); }
};

class INET_API Ieee80211OFDMSignalMode : public IIeee80211HeaderMode, public Ieee80211OFDMModeBase
{
  protected:
    unsigned int rate;

  public:
    Ieee80211OFDMSignalMode(const Ieee80211OFDMCode *code, const Ieee80211OFDMModulation *modulation, Hz channelSpacing, Hz bandwidth, unsigned int rate);
    virtual ~Ieee80211OFDMSignalMode() {}

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    unsigned int getRate() const { return rate; }
    inline int getRateBitLength() const { return 4; }
    inline int getReservedBitLength() const { return 1; }
    inline int getLengthBitLength() const { return 12; }
    inline int getParityBitLength() const { return 1; }
    inline int getTailBitLength() const { return 6; }

    virtual int getBitLength() const override { return getRateBitLength() + getReservedBitLength() + getLengthBitLength() + getParityBitLength() + getTailBitLength(); }
    virtual const simtime_t getDuration() const override { return getSymbolInterval(); }

    const Ieee80211OFDMCode* getCode() const { return code; }
    const Ieee80211OFDMModulation* getModulation() const override { return modulation; }

    virtual bps getGrossBitrate() const override { return Ieee80211OFDMModeBase::getGrossBitrate(); }
    virtual bps getNetBitrate() const override { return Ieee80211OFDMModeBase::getNetBitrate(); }
};

class INET_API Ieee80211OFDMDataMode : public IIeee80211DataMode, public Ieee80211OFDMModeBase
{
  public:
    Ieee80211OFDMDataMode(const Ieee80211OFDMCode *code, const Ieee80211OFDMModulation *modulation, Hz channelSpacing, Hz bandwidth);
    virtual ~Ieee80211OFDMDataMode() {}

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    inline int getServiceBitLength() const { return 16; }
    inline int getTailBitLength() const { return 6; }

    virtual int getBitLength(int dataBitLength) const override;
    virtual const simtime_t getDuration(int dataBitLength) const override;

    const Ieee80211OFDMCode* getCode() const { return code; }
    const Ieee80211OFDMModulation* getModulation() const override { return modulation; }
    virtual bps getGrossBitrate() const override { return Ieee80211OFDMModeBase::getGrossBitrate(); }
    virtual bps getNetBitrate() const override { return Ieee80211OFDMModeBase::getNetBitrate(); }
    virtual int getNumberOfSpatialStreams() const override { return 1; }
};

class INET_API Ieee80211OFDMMode : public Ieee80211ModeBase, public Ieee80211OFDMTimingRelatedParametersBase
{
  protected:
    const Ieee80211OFDMPreambleMode *preambleMode;
    const Ieee80211OFDMSignalMode *signalMode;
    const Ieee80211OFDMDataMode *dataMode;

  protected:
    virtual int getLegacyCwMin() const override { return 15; }
    virtual int getLegacyCwMax() const override { return 1023; }

  public:
    Ieee80211OFDMMode(const char *name, const Ieee80211OFDMPreambleMode *preambleMode, const Ieee80211OFDMSignalMode *signalMode, const Ieee80211OFDMDataMode *dataMode, Hz channelSpacing, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const Ieee80211OFDMPreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const Ieee80211OFDMSignalMode *getHeaderMode() const override { return signalMode; }
    virtual const Ieee80211OFDMDataMode *getDataMode() const override { return dataMode; }
    virtual const Ieee80211OFDMSignalMode *getSignalMode() const { return signalMode; }

    virtual inline const simtime_t getDuration(int dataBitLength) const override { return preambleMode->getDuration() + signalMode->getDuration() + dataMode->getDuration(dataBitLength); }

    // Table 18-17—OFDM PHY characteristics
    virtual const simtime_t getSlotTime() const override;
    virtual const simtime_t getSifsTime() const override;
    virtual const simtime_t getRifsTime() const override;
    virtual const simtime_t getCcaTime() const override;
    virtual const simtime_t getPhyRxStartDelay() const override;
    virtual const simtime_t getRxTxTurnaroundTime() const override;
    virtual inline const simtime_t getPreambleLength() const override { return preambleMode->getDuration(); }
    virtual inline const simtime_t getPlcpHeaderLength() const override { return signalMode->getDuration(); }
    virtual inline int getMpduMaxLength() const override { return 4095; }
};

class INET_API Ieee80211OFDMCompliantModes
{
  public:
    // Preamble modes: 18.3.3 PLCS preamble (SYNC).
    static const Ieee80211OFDMPreambleMode ofdmPreambleModeCS5MHz;
    static const Ieee80211OFDMPreambleMode ofdmPreambleModeCS10MHz;
    static const Ieee80211OFDMPreambleMode ofdmPreambleModeCS20MHz;

    // Signal modes: Table 18-6—Contents of the SIGNAL field
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate13;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate15;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate5;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate7;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate9;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate11;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate1;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode6MbpsRate3;

    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate13;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate15;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate5;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate7;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate9;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate11;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate1;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode3MbpsRate3;

    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate13;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate15;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate5;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate7;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate9;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate11;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate1;
    static const Ieee80211OFDMSignalMode ofdmHeaderMode1_5MbpsRate3;

    // Data modes: Table 18-4—Modulation-dependent parameters
    static const Ieee80211OFDMDataMode ofdmDataMode1_5Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode2_25Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode3MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode3MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode4_5MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode4_5MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode6MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode6MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode6MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode9MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode9MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode9MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode12MbpsCS5MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode12MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode12MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode13_5Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode18MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode18MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode24MbpsCS10MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode24MbpsCS20MHz;
    static const Ieee80211OFDMDataMode ofdmDataMode27Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode36Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode48Mbps;
    static const Ieee80211OFDMDataMode ofdmDataMode54Mbps;

    // Modes
    static const Ieee80211OFDMMode ofdmMode1_5Mbps;
    static const Ieee80211OFDMMode ofdmMode2_25Mbps;
    static const Ieee80211OFDMMode ofdmMode3MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode3MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode4_5MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode4_5MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode6MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode6MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode6MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode9MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode9MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode9MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode12MbpsCS5MHz;
    static const Ieee80211OFDMMode ofdmMode12MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode12MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode13_5Mbps;
    static const Ieee80211OFDMMode ofdmMode18MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode18MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode24MbpsCS10MHz;
    static const Ieee80211OFDMMode ofdmMode24MbpsCS20MHz;
    static const Ieee80211OFDMMode ofdmMode27Mbps;
    static const Ieee80211OFDMMode ofdmMode36Mbps;
    static const Ieee80211OFDMMode ofdmMode48Mbps;
    static const Ieee80211OFDMMode ofdmMode54Mbps;

  public:
    static const Ieee80211OFDMMode& getCompliantMode(unsigned int signalRateField, Hz channelSpacing);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211OFDMMODE_H

