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

#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmModulation.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OfdmTimingRelatedParametersBase
{
  protected:
    Hz channelSpacing;

  public:
    Ieee80211OfdmTimingRelatedParametersBase(Hz channelSpacing) : channelSpacing(channelSpacing) {}

    Hz getSubcarrierFrequencySpacing() const { return channelSpacing / 64; }
    const simtime_t getFFTTransformPeriod() const { return simtime_t(1 / getSubcarrierFrequencySpacing().get()); }
    const simtime_t getGIDuration() const { return getFFTTransformPeriod() / 4; }
    const simtime_t getSymbolInterval() const { return getGIDuration() + getFFTTransformPeriod(); }

    const Hz getChannelSpacing() const { return channelSpacing; }
};

class INET_API Ieee80211OfdmModeBase : public Ieee80211OfdmTimingRelatedParametersBase
{
  protected:
    const Ieee80211OfdmModulation *modulation;
    const Ieee80211OfdmCode *code;
    const Hz bandwidth;
    mutable bps netBitrate; // cached
    mutable bps grossBitrate; // cached

  protected:
    bps computeGrossBitrate(const Ieee80211OfdmModulation *modulation) const;
    bps computeNetBitrate(bps grossBitrate, const Ieee80211OfdmCode *code) const;

  public:
    Ieee80211OfdmModeBase(const Ieee80211OfdmModulation *modulation, const Ieee80211OfdmCode *code, Hz channelSpacing, Hz bandwidth);
    virtual ~Ieee80211OfdmModeBase() {}

    int getNumberOfDataSubcarriers() const { return 48; }
    int getNumberOfPilotSubcarriers() const { return 4; }
    int getNumberOfTotalSubcarriers() const { return getNumberOfDataSubcarriers() + getNumberOfPilotSubcarriers(); }

    virtual bps getGrossBitrate() const;
    virtual bps getNetBitrate() const;
    virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API Ieee80211OfdmPreambleMode : public IIeee80211PreambleMode, public Ieee80211OfdmTimingRelatedParametersBase
{
  public:
    Ieee80211OfdmPreambleMode(Hz channelSpacing);
    virtual ~Ieee80211OfdmPreambleMode() {}

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    const simtime_t getTrainingSymbolGIDuration() const { return getFFTTransformPeriod() / 2; }
    const simtime_t getShortTrainingSequenceDuration() const { return 10 * getFFTTransformPeriod() / 4; }
    const simtime_t getLongTrainingSequenceDuration() const { return getTrainingSymbolGIDuration() + 2 * getFFTTransformPeriod(); }
    virtual const simtime_t getDuration() const override { return getShortTrainingSequenceDuration() + getLongTrainingSequenceDuration(); }

    virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211OfdmPhyPreamble>(); }
};

class INET_API Ieee80211OfdmSignalMode : public IIeee80211HeaderMode, public Ieee80211OfdmModeBase
{
  protected:
    unsigned int rate;

  public:
    Ieee80211OfdmSignalMode(const Ieee80211OfdmCode *code, const Ieee80211OfdmModulation *modulation, Hz channelSpacing, Hz bandwidth, unsigned int rate);
    virtual ~Ieee80211OfdmSignalMode() {}

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    unsigned int getRate() const { return rate; }
    b getRateFieldLength() const { return b(4); }
    b getReservedFieldLength() const { return b(1); }
    b getLengthFieldLength() const { return b(12); }
    b getParityFieldLength() const { return b(1); }
    b getTailFieldLength() const { return b(6); }
    b getServiceFieldLength() const { return b(16); }

    virtual b getLength() const override { return getRateFieldLength() + getReservedFieldLength() + getLengthFieldLength() + getParityFieldLength() + getTailFieldLength() + getServiceFieldLength(); }
    virtual const simtime_t getDuration() const override { return getSymbolInterval(); }

    const Ieee80211OfdmCode* getCode() const { return code; }
    const Ieee80211OfdmModulation* getModulation() const override { return modulation; }

    virtual bps getGrossBitrate() const override { return Ieee80211OfdmModeBase::getGrossBitrate(); }
    virtual bps getNetBitrate() const override { return Ieee80211OfdmModeBase::getNetBitrate(); }

    virtual Ptr<Ieee80211PhyHeader> createHeader() const override { return makeShared<Ieee80211OfdmPhyHeader>(); }
};

class INET_API Ieee80211OfdmDataMode : public IIeee80211DataMode, public Ieee80211OfdmModeBase
{
  public:
    Ieee80211OfdmDataMode(const Ieee80211OfdmCode *code, const Ieee80211OfdmModulation *modulation, Hz channelSpacing, Hz bandwidth);
    virtual ~Ieee80211OfdmDataMode() {}

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    b getServiceFieldLength() const { return b(16); }
    b getTailFieldLength() const { return b(6); }

    virtual Hz getBandwidth() const override { return bandwidth; }
    virtual b getPaddingLength(b dataLength) const override;
    virtual b getCompleteLength(b dataLength) const override;
    virtual const simtime_t getDuration(b dataLength) const override;

    const Ieee80211OfdmCode* getCode() const { return code; }
    const Ieee80211OfdmModulation* getModulation() const override { return modulation; }
    virtual bps getGrossBitrate() const override { return Ieee80211OfdmModeBase::getGrossBitrate(); }
    virtual bps getNetBitrate() const override { return Ieee80211OfdmModeBase::getNetBitrate(); }
    virtual int getNumberOfSpatialStreams() const override { return 1; }
};

class INET_API Ieee80211OfdmMode : public Ieee80211ModeBase, public Ieee80211OfdmTimingRelatedParametersBase
{
  protected:
    const Ieee80211OfdmPreambleMode *preambleMode;
    const Ieee80211OfdmSignalMode *signalMode;
    const Ieee80211OfdmDataMode *dataMode;

  protected:
    virtual int getLegacyCwMin() const override { return 15; }
    virtual int getLegacyCwMax() const override { return 1023; }

  public:
    Ieee80211OfdmMode(const char *name, const Ieee80211OfdmPreambleMode *preambleMode, const Ieee80211OfdmSignalMode *signalMode, const Ieee80211OfdmDataMode *dataMode, Hz channelSpacing, Hz bandwidth);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const Ieee80211OfdmPreambleMode *getPreambleMode() const override { return preambleMode; }
    virtual const Ieee80211OfdmSignalMode *getHeaderMode() const override { return signalMode; }
    virtual const Ieee80211OfdmDataMode *getDataMode() const override { return dataMode; }
    virtual const Ieee80211OfdmSignalMode *getSignalMode() const { return signalMode; }

    virtual inline const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + signalMode->getDuration() + dataMode->getDuration(dataLength); }

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

class INET_API Ieee80211OfdmCompliantModes
{
  public:
    // Preamble modes: 18.3.3 PLCS preamble (SYNC).
    static const Ieee80211OfdmPreambleMode ofdmPreambleModeCS5MHz;
    static const Ieee80211OfdmPreambleMode ofdmPreambleModeCS10MHz;
    static const Ieee80211OfdmPreambleMode ofdmPreambleModeCS20MHz;

    // Signal modes: Table 18-6—Contents of the SIGNAL field
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate13;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate15;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate5;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate7;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate9;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate11;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate1;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode6MbpsRate3;

    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate13;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate15;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate5;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate7;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate9;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate11;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate1;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode3MbpsRate3;

    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate13;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate15;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate5;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate7;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate9;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate11;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate1;
    static const Ieee80211OfdmSignalMode ofdmHeaderMode1_5MbpsRate3;

    // Data modes: Table 18-4—Modulation-dependent parameters
    static const Ieee80211OfdmDataMode ofdmDataMode1_5Mbps;
    static const Ieee80211OfdmDataMode ofdmDataMode2_25Mbps;
    static const Ieee80211OfdmDataMode ofdmDataMode3MbpsCS5MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode3MbpsCS10MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode4_5MbpsCS5MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode4_5MbpsCS10MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode6MbpsCS5MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode6MbpsCS10MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode6MbpsCS20MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode9MbpsCS5MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode9MbpsCS10MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode9MbpsCS20MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode12MbpsCS5MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode12MbpsCS10MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode12MbpsCS20MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode13_5Mbps;
    static const Ieee80211OfdmDataMode ofdmDataMode18MbpsCS10MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode18MbpsCS20MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode24MbpsCS10MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode24MbpsCS20MHz;
    static const Ieee80211OfdmDataMode ofdmDataMode27Mbps;
    static const Ieee80211OfdmDataMode ofdmDataMode36Mbps;
    static const Ieee80211OfdmDataMode ofdmDataMode48Mbps;
    static const Ieee80211OfdmDataMode ofdmDataMode54Mbps;

    // Modes
    static const Ieee80211OfdmMode ofdmMode1_5Mbps;
    static const Ieee80211OfdmMode ofdmMode2_25Mbps;
    static const Ieee80211OfdmMode ofdmMode3MbpsCS5MHz;
    static const Ieee80211OfdmMode ofdmMode3MbpsCS10MHz;
    static const Ieee80211OfdmMode ofdmMode4_5MbpsCS5MHz;
    static const Ieee80211OfdmMode ofdmMode4_5MbpsCS10MHz;
    static const Ieee80211OfdmMode ofdmMode6MbpsCS5MHz;
    static const Ieee80211OfdmMode ofdmMode6MbpsCS10MHz;
    static const Ieee80211OfdmMode ofdmMode6MbpsCS20MHz;
    static const Ieee80211OfdmMode ofdmMode9MbpsCS5MHz;
    static const Ieee80211OfdmMode ofdmMode9MbpsCS10MHz;
    static const Ieee80211OfdmMode ofdmMode9MbpsCS20MHz;
    static const Ieee80211OfdmMode ofdmMode12MbpsCS5MHz;
    static const Ieee80211OfdmMode ofdmMode12MbpsCS10MHz;
    static const Ieee80211OfdmMode ofdmMode12MbpsCS20MHz;
    static const Ieee80211OfdmMode ofdmMode13_5Mbps;
    static const Ieee80211OfdmMode ofdmMode18MbpsCS10MHz;
    static const Ieee80211OfdmMode ofdmMode18MbpsCS20MHz;
    static const Ieee80211OfdmMode ofdmMode24MbpsCS10MHz;
    static const Ieee80211OfdmMode ofdmMode24MbpsCS20MHz;
    static const Ieee80211OfdmMode ofdmMode27Mbps;
    static const Ieee80211OfdmMode ofdmMode36Mbps;
    static const Ieee80211OfdmMode ofdmMode48Mbps;
    static const Ieee80211OfdmMode ofdmMode54Mbps;

  public:
    static const Ieee80211OfdmMode& getCompliantMode(unsigned int signalRateField, Hz channelSpacing);
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_IEEE80211OFDMMODE_H

