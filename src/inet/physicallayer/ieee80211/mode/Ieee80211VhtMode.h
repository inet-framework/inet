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

#ifndef __INET_IEEE80211VHTMODE_H
#define __INET_IEEE80211VHTMODE_H

#define DI DelayedInitializer

#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HtCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211VhtCode.h"

namespace inet {
namespace physicallayer {


class INET_API Ieee80211VhtModeBase
{
    public:
        enum GuardIntervalType
        {
            HT_GUARD_INTERVAL_SHORT, // 400 ns
            HT_GUARD_INTERVAL_LONG // 800 ns
        };

    protected:
        const Hz bandwidth;
        const GuardIntervalType guardIntervalType;
        const unsigned int mcsIndex; // MCS
        const unsigned int numberOfSpatialStreams; // N_SS

        mutable bps netBitrate; // cached
        mutable bps grossBitrate; // cached

    protected:
        virtual bps computeGrossBitrate() const = 0;
        virtual bps computeNetBitrate() const = 0;

    public:
        Ieee80211VhtModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType);

        virtual int getNumberOfDataSubcarriers() const;
        virtual int getNumberOfPilotSubcarriers() const;
        virtual int getNumberOfTotalSubcarriers() const { return getNumberOfDataSubcarriers() + getNumberOfPilotSubcarriers(); }
        virtual GuardIntervalType getGuardIntervalType() const { return guardIntervalType; }
        virtual int getNumberOfSpatialStreams() const { return numberOfSpatialStreams; }
        virtual unsigned int getMcsIndex() const { return mcsIndex; }
        virtual Hz getBandwidth() const { return bandwidth; }
        virtual bps getNetBitrate() const;
        virtual bps getGrossBitrate() const;
};


class INET_API Ieee80211VhtSignalMode : public IIeee80211HeaderMode, public Ieee80211VhtModeBase, public Ieee80211HtTimingRelatedParametersBase
{
    protected:
        const Ieee80211OfdmModulation *modulation;
        const Ieee80211VhtCode *code;

    protected:
        virtual bps computeGrossBitrate() const override;
        virtual bps computeNetBitrate() const override;

    public:
        Ieee80211VhtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation *modulation, const Ieee80211VhtCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType);
        Ieee80211VhtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation *modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType);
        virtual ~Ieee80211VhtSignalMode();

        /* Table 20-11—HT-SIG fields, 1699p */

        // HT-SIG_1 (24 bits)
        virtual inline b getMCSLength() const { return b(9); }
        virtual inline b getCBWLength() const { return b(1); }
        virtual inline b getHTLengthLength() const { return b(16); }

        // HT-SIG_2 (24 bits)
        virtual inline b getSmoothingLength() const { return b(1); }
        virtual inline b getNotSoundingLength() const { return b(1); }
        virtual inline b getReservedLength() const { return b(1); }
        virtual inline b getAggregationLength() const { return b(1); }
        virtual inline b getSTBCLength() const { return b(2); }
        virtual inline b getFECCodingLength() const { return b(1); }
        virtual inline b getShortGILength() const { return b(1); }
        virtual inline b getNumOfExtensionSpatialStreamsLength() const { return b(2); }
        virtual inline b getCRCLength() const { return b(8); }
        virtual inline b getTailBitsLength() const { return b(6); }
        virtual unsigned int getSTBC() const { return 0; } // Limitation: We assume that STBC is not used

        virtual const inline simtime_t getHTSIGDuration() const { return 2 * getSymbolInterval(); } // HT-SIG

        virtual unsigned int getModulationAndCodingScheme() const { return mcsIndex; }
        virtual const simtime_t getDuration() const override { return getHTSIGDuration(); }
        virtual b getLength() const override;
        virtual bps getNetBitrate() const override { return Ieee80211VhtModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211VhtModeBase::getGrossBitrate(); }
        virtual const Ieee80211OfdmModulation *getModulation() const override { return modulation; }
        virtual const Ieee80211VhtCode * getCode() const {return code;}

        virtual Ptr<Ieee80211PhyHeader> createHeader() const override { return makeShared<Ieee80211HtPhyHeader>(); }
};

/*
 * The HT preambles are defined in HT-mixed format and in HT-greenfield format to carry the required
 * information to operate in a system with multiple transmit and multiple receive antennas. (20.3.9 HT preamble)
 */
class INET_API Ieee80211VhtPreambleMode : public IIeee80211PreambleMode, public Ieee80211HtTimingRelatedParametersBase
{
    public:
        enum HighTroughputPreambleFormat
        {
            HT_PREAMBLE_MIXED,      // can be received by non-HT STAs compliant with Clause 18 or Clause 19
            HT_PREAMBLE_GREENFIELD  // all of the non-HT fields are omitted
        };

    protected:
        const Ieee80211VhtSignalMode *highThroughputSignalMode; // In HT-terminology the HT-SIG (signal field) and L-SIG are part of the preamble
        const Ieee80211OfdmSignalMode *legacySignalMode; // L-SIG
        const HighTroughputPreambleFormat preambleFormat;
        const unsigned int numberOfHTLongTrainings; // N_LTF, 20.3.9.4.6 HT-LTF definition

    protected:
        virtual unsigned int computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const;
        virtual unsigned int computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const;

    public:
        Ieee80211VhtPreambleMode(const Ieee80211VhtSignalMode* highThroughputSignalMode, const Ieee80211OfdmSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream);
        virtual ~Ieee80211VhtPreambleMode() { delete highThroughputSignalMode; }

        HighTroughputPreambleFormat getPreambleFormat() const { return preambleFormat; }
        virtual const Ieee80211VhtSignalMode *getSignalMode() const { return highThroughputSignalMode; }
        virtual const Ieee80211OfdmSignalMode *getLegacySignalMode() const { return legacySignalMode; }
        virtual const Ieee80211VhtSignalMode* getHighThroughputSignalMode() const { return highThroughputSignalMode; }

        virtual const inline simtime_t getDoubleGIDuration() const { return 2 * getGIDuration(); } // GI2
        virtual const inline simtime_t getLSIGDuration() const { return getSymbolInterval(); } // L-SIG
        virtual const inline simtime_t getNonHTShortTrainingSequenceDuration() const { return 10 * getDFTPeriod() / 4;  } // L-STF
        virtual const inline simtime_t getHTGreenfieldShortTrainingFieldDuration() const { return 10 * getDFTPeriod() / 4; } // HT-GF-STF
        virtual const inline simtime_t getNonHTLongTrainingFieldDuration() const { return 2 * getDFTPeriod() + getDoubleGIDuration(); } // L-LTF
        virtual const inline simtime_t getNonHTSignalField() const { return 4E-6; } // L-SIG
        virtual const inline simtime_t getVHTSignalFieldA() const { return 8E-6; } // VHT-SIG-A
        virtual const inline simtime_t getVHTShortTrainingFieldDuration() const { return 4E-6; } // VHT-STF
        virtual const inline simtime_t getVHTSignalFieldB() const { return 4E-6; } // VHT-SIG-A

        virtual const simtime_t getFirstHTLongTrainingFieldDuration() const;
        virtual const inline simtime_t getSecondAndSubsequentHTLongTrainingFielDuration() const { return 4E-6; } // HT-LTFs, s = 2,3,..,n
        virtual inline unsigned int getNumberOfHtLongTrainings() const { return numberOfHTLongTrainings; }

        virtual const simtime_t getDuration() const override;

        virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211VhtPhyPreamble>(); }

};


class INET_API Ieee80211Vhtmcs
{
    protected:
        const unsigned int mcsIndex;
        const Ieee80211OfdmModulation *stream1Modulation = nullptr;
        const Ieee80211OfdmModulation *stream2Modulation = nullptr;
        const Ieee80211OfdmModulation *stream3Modulation = nullptr;
        const Ieee80211OfdmModulation *stream4Modulation = nullptr;
        const Ieee80211OfdmModulation *stream5Modulation = nullptr;
        const Ieee80211OfdmModulation *stream6Modulation = nullptr;
        const Ieee80211OfdmModulation *stream7Modulation = nullptr;
        const Ieee80211OfdmModulation *stream8Modulation = nullptr;
        const Ieee80211VhtCode *code;
        const Hz bandwidth;

    public:
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211VhtCode *code, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation, const Ieee80211OfdmModulation *stream5Modulation, const Ieee80211OfdmModulation *stream6Modulation, const Ieee80211OfdmModulation *stream7Modulation, const Ieee80211OfdmModulation *stream8Modulation);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation, const Ieee80211OfdmModulation* stream5Modulation, const Ieee80211OfdmModulation* stream6Modulation, const Ieee80211OfdmModulation* stream7Modulation, const Ieee80211OfdmModulation* stream8Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation, const Ieee80211OfdmModulation* stream5Modulation, const Ieee80211OfdmModulation* stream6Modulation, const Ieee80211OfdmModulation* stream7Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation, const Ieee80211OfdmModulation* stream5Modulation, const Ieee80211OfdmModulation* stream6Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation, const Ieee80211OfdmModulation* stream5Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211Vhtmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth, int nss);

        virtual ~Ieee80211Vhtmcs();

        const Ieee80211VhtCode* getCode() const { return code; }
        unsigned int getMcsIndex() const { return mcsIndex; }
        virtual const Ieee80211OfdmModulation* getModulation() const { return stream1Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension1Modulation() const { return stream2Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension2Modulation() const { return stream3Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension3Modulation() const { return stream4Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension4Modulation() const { return stream5Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension5Modulation() const { return stream6Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension6Modulation() const { return stream7Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension7Modulation() const { return stream8Modulation; }
        virtual Hz getBandwidth() const { return bandwidth; }
        virtual unsigned int getNumNss() const {return (stream1Modulation ? 1 : 0) + (stream2Modulation ? 1 : 0) +
                                              (stream3Modulation ? 1 : 0) + (stream4Modulation ? 1 : 0) +
                                              (stream5Modulation ? 1 : 0) + (stream6Modulation ? 1 : 0) +
                                              (stream7Modulation ? 1 : 0) + (stream8Modulation ? 1 : 0);}
};

class INET_API Ieee80211VhtDataMode : public IIeee80211DataMode, public Ieee80211VhtModeBase, public Ieee80211HtTimingRelatedParametersBase
{
    protected:
        const Ieee80211Vhtmcs *modulationAndCodingScheme;
        const unsigned int numberOfBccEncoders;

    protected:
        bps computeGrossBitrate() const override;
        bps computeNetBitrate() const override;
        unsigned int computeNumberOfSpatialStreams(const Ieee80211Vhtmcs*) const;
        unsigned int computeNumberOfCodedBitsPerSubcarrierSum() const;
        unsigned int computeNumberOfBccEncoders() const;
        unsigned int getNumberOfBccEncoders20MHz() const;
        unsigned int getNumberOfBccEncoders40MHz() const;
        unsigned int getNumberOfBccEncoders80MHz() const;
        unsigned int getNumberOfBccEncoders160MHz() const;

    public:
        Ieee80211VhtDataMode(const Ieee80211Vhtmcs *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType);

        inline b getServiceFieldLength() const { return b(16); }
        inline b getTailFieldLength() const { return b(6) * numberOfBccEncoders; }

        virtual int getNumberOfSpatialStreams() const override { return Ieee80211VhtModeBase::getNumberOfSpatialStreams(); }
        virtual Hz getBandwidth() const override { return bandwidth; }
        virtual b getPaddingLength(b dataLength) const override { return b(0); }
        virtual b getCompleteLength(b dataLength) const override;
        virtual const simtime_t getDuration(b dataLength) const override;
        virtual bps getNetBitrate() const override { return Ieee80211VhtModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211VhtModeBase::getGrossBitrate(); }
        virtual const Ieee80211Vhtmcs *getModulationAndCodingScheme() const { return modulationAndCodingScheme; }
        virtual const Ieee80211VhtCode* getCode() const { return modulationAndCodingScheme->getCode(); }
        virtual const Ieee80211OfdmModulation* getModulation() const override { return modulationAndCodingScheme->getModulation(); }
};

class INET_API Ieee80211VhtMode : public Ieee80211ModeBase
{
    public:
        enum BandMode
        {
            BAND_2_4GHZ,
            BAND_5GHZ
        };

    protected:
        const Ieee80211VhtPreambleMode *preambleMode;
        const Ieee80211VhtDataMode *dataMode;
        const BandMode centerFrequencyMode;

    protected:
        virtual inline int getLegacyCwMin() const override { return 15; }
        virtual inline int getLegacyCwMax() const override { return 1023; }

    public:
        Ieee80211VhtMode(const char *name, const Ieee80211VhtPreambleMode *preambleMode, const Ieee80211VhtDataMode *dataMode, const BandMode centerFrequencyMode);
        virtual ~Ieee80211VhtMode() { delete preambleMode; delete dataMode; }

        virtual const Ieee80211VhtDataMode* getDataMode() const override { return dataMode; }
        virtual const Ieee80211VhtPreambleMode* getPreambleMode() const override { return preambleMode; }
        virtual const Ieee80211VhtSignalMode *getHeaderMode() const override { return preambleMode->getSignalMode(); }
        virtual const Ieee80211OfdmSignalMode *getLegacySignalMode() const { return preambleMode->getLegacySignalMode(); }

        // Table 20-25—MIMO PHY characteristics
        virtual const simtime_t getSlotTime() const override;
        virtual const simtime_t getShortSlotTime() const;
        virtual inline const simtime_t getSifsTime() const override;
        virtual inline const simtime_t getRifsTime() const override { return 2E-6; }
        virtual inline const simtime_t getCcaTime() const override { return 4E-6; } // < 4
        virtual inline const simtime_t getPhyRxStartDelay() const override { return 33E-6; }
        virtual inline const simtime_t getRxTxTurnaroundTime() const override { return 2E-6; } // < 2
        virtual inline const simtime_t getPreambleLength() const override { return 16E-6; }
        virtual inline const simtime_t getPlcpHeaderLength() const override { return 4E-6; }
        virtual inline int getMpduMaxLength() const override { return 65535; } // in octets
        virtual BandMode getCenterFrequencyMode() const { return centerFrequencyMode; }

        virtual const simtime_t getDuration(b dataBitLength) const override { return preambleMode->getDuration() + dataMode->getDuration(dataBitLength); }
};

// A specification of the high-throughput (HT) physical layer (PHY)
// parameters that consists of modulation order (e.g., BPSK, QPSK, 16-QAM,
// 64-QAM) and forward error correction (FEC) coding rate (e.g., 1/2, 2/3,
// 3/4, 5/6).
class INET_API Ieee80211VhtmcsTable
{
    public:
        // Table 20-30—MCS parameters for mandatory 20 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss1; // No valid

        // Table 20-31—MCS parameters for optional 20 MHz, N_SS = 2
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss2; // No valid

        // Table 20-32—MCS parameters for optional 20 MHz, N_SS = 3
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss3;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 4
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss4; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 5
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss5; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 6
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss6;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 7
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss7; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 8
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW20MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW20MHzNss8; // No valid

        // Table 20-30—MCS parameters for mandatory 40 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss1;

        // Table 20-31—MCS parameters for optional 40 MHz, N_SS = 2
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss2;

        // Table 20-32—MCS parameters for optional 40 MHz, N_SS = 3
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss3;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 4
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss4;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 5
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss5;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 6
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss6;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 7
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss7;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 8
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW40MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW40MHzNss8;
        // Table 20-30—MCS parameters for mandatory 80 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss1;

        // Table 20-31—MCS parameters for optional 80 MHz, N_SS = 2
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss2;

        // Table 20-32—MCS parameters for optional 80 MHz, N_SS = 3
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss3;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 4
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss4;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 5
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss5;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 6
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss6;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 7
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss7;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 8
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW80MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW80MHzNss8;

        // Table 20-30—MCS parameters for mandatory 160 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss1;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss1;

        // Table 20-31—MCS parameters for optional 160 MHz, N_SS = 2
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss2;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss2;

        // Table 20-32—MCS parameters for optional 160 MHz, N_SS = 3
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss3;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss3;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 4
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss4;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss4;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 5
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss5;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss5;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 6
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss6;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss6;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 7
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss7;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss7;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 8
        static const DI<Ieee80211Vhtmcs> vhtMcs0BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs1BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs2BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs3BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs4BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs5BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs6BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs7BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs8BW160MHzNss8;
        static const DI<Ieee80211Vhtmcs> vhtMcs9BW160MHzNss8;


};

class INET_API Ieee80211VhtCompliantModes
{
    protected:
        static Ieee80211VhtCompliantModes singleton;

        std::map<std::tuple<Hz, unsigned int, Ieee80211VhtModeBase::GuardIntervalType, unsigned int>, const Ieee80211VhtMode *> modeCache;

    public:
        Ieee80211VhtCompliantModes();
        virtual ~Ieee80211VhtCompliantModes();

        static const Ieee80211VhtMode *getCompliantMode(const Ieee80211Vhtmcs *mcsMode, Ieee80211VhtMode::BandMode centerFrequencyMode, Ieee80211VhtPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211VhtModeBase::GuardIntervalType guardIntervalType);

};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211VHTMODE_H
