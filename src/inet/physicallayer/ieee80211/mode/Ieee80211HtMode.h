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

#ifndef __INET_IEEE80211HTMODE_H
#define __INET_IEEE80211HTMODE_H

#define DI DelayedInitializer

#include "inet/common/DelayedInitializer.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HtCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OfdmMode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211HtTimingRelatedParametersBase
{
    public:
        const simtime_t getDFTPeriod() const { return 3.2E-6; } // DFT
        const simtime_t getGIDuration() const { return getDFTPeriod() / 4; } // GI
        const simtime_t getShortGIDuration() const { return getDFTPeriod() / 8; } // GIS
        const simtime_t getSymbolInterval() const { return getDFTPeriod() + getGIDuration(); } // SYM
        const simtime_t getShortGISymbolInterval() const { return getDFTPeriod() + getShortGIDuration(); } // SYMS
};

class INET_API Ieee80211HtModeBase
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
        Ieee80211HtModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType);

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

class INET_API Ieee80211HtSignalMode : public IIeee80211HeaderMode, public Ieee80211HtModeBase, public Ieee80211HtTimingRelatedParametersBase
{
    protected:
        const Ieee80211OfdmModulation *modulation;
        const Ieee80211HtCode *code;

    protected:
        virtual bps computeGrossBitrate() const override;
        virtual bps computeNetBitrate() const override;

    public:
        Ieee80211HtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation *modulation, const Ieee80211HtCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType);
        Ieee80211HtSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OfdmModulation *modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType);
        virtual ~Ieee80211HtSignalMode();

        /* Table 20-11—HT-SIG fields, 1699p */

        // HT-SIG_1 (24 bits)
        b getMCSLength() const { return b(7); }
        b getCBWLength() const { return b(1); }
        b getHTLengthLength() const { return b(16); }

        // HT-SIG_2 (24 bits)
        b getSmoothingLength() const { return b(1); }
        b getNotSoundingLength() const { return b(1); }
        b getReservedLength() const { return b(1); }
        b getAggregationLength() const { return b(1); }
        b getSTBCLength() const { return b(2); }
        b getFECCodingLength() const { return b(1); }
        b getShortGILength() const { return b(1); }
        b getNumOfExtensionSpatialStreamsLength() const { return b(2); }
        b getCRCLength() const { return b(8); }
        b getTailBitsLength() const { return b(6); }
        virtual unsigned int getSTBC() const { return 0; } // Limitation: We assume that STBC is not used

        virtual const inline simtime_t getHTSIGDuration() const { return 2 * getSymbolInterval(); } // HT-SIG

        virtual unsigned int getModulationAndCodingScheme() const { return mcsIndex; }
        virtual const simtime_t getDuration() const override { return getHTSIGDuration(); }
        virtual b getLength() const override;
        virtual bps getNetBitrate() const override { return Ieee80211HtModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211HtModeBase::getGrossBitrate(); }
        virtual const Ieee80211OfdmModulation *getModulation() const override { return modulation; }
        virtual const Ieee80211HtCode * getCode() const {return code;}

        virtual Ptr<Ieee80211PhyHeader> createHeader() const override { return makeShared<Ieee80211HtPhyHeader>(); }
};

/*
 * The HT preambles are defined in HT-mixed format and in HT-greenfield format to carry the required
 * information to operate in a system with multiple transmit and multiple receive antennas. (20.3.9 HT preamble)
 */
class INET_API Ieee80211HtPreambleMode : public IIeee80211PreambleMode, public Ieee80211HtTimingRelatedParametersBase
{
    public:
        enum HighTroughputPreambleFormat
        {
            HT_PREAMBLE_MIXED,      // can be received by non-HT STAs compliant with Clause 18 or Clause 19
            HT_PREAMBLE_GREENFIELD  // all of the non-HT fields are omitted
        };

    protected:
        const Ieee80211HtSignalMode *highThroughputSignalMode; // In HT-terminology the HT-SIG (signal field) and L-SIG are part of the preamble
        const Ieee80211OfdmSignalMode *legacySignalMode; // L-SIG
        const HighTroughputPreambleFormat preambleFormat;
        const unsigned int numberOfHTLongTrainings; // N_LTF, 20.3.9.4.6 HT-LTF definition

    protected:
        virtual unsigned int computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const;
        virtual unsigned int computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const;

    public:
        Ieee80211HtPreambleMode(const Ieee80211HtSignalMode* highThroughputSignalMode, const Ieee80211OfdmSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream);
        virtual ~Ieee80211HtPreambleMode() { delete highThroughputSignalMode; }

        HighTroughputPreambleFormat getPreambleFormat() const { return preambleFormat; }
        virtual const Ieee80211HtSignalMode *getSignalMode() const { return highThroughputSignalMode; }
        virtual const Ieee80211OfdmSignalMode *getLegacySignalMode() const { return legacySignalMode; }
        virtual const Ieee80211HtSignalMode* getHighThroughputSignalMode() const { return highThroughputSignalMode; }

        virtual const inline simtime_t getDoubleGIDuration() const { return 2 * getGIDuration(); } // GI2
        virtual const inline simtime_t getLSIGDuration() const { return getSymbolInterval(); } // L-SIG
        virtual const inline simtime_t getNonHTShortTrainingSequenceDuration() const { return 10 * getDFTPeriod() / 4;  } // L-STF
        virtual const inline simtime_t getHTGreenfieldShortTrainingFieldDuration() const { return 10 * getDFTPeriod() / 4; } // HT-GF-STF
        virtual const inline simtime_t getNonHTLongTrainingFieldDuration() const { return 2 * getDFTPeriod() + getDoubleGIDuration(); } // L-LTF
        virtual const inline simtime_t getHTShortTrainingFieldDuration() const { return 4E-6; } // HT-STF
        virtual const simtime_t getFirstHTLongTrainingFieldDuration() const;
        virtual const inline simtime_t getSecondAndSubsequentHTLongTrainingFielDuration() const { return 4E-6; } // HT-LTFs, s = 2,3,..,n
        virtual inline unsigned int getNumberOfHtLongTrainings() const { return numberOfHTLongTrainings; }

        virtual const simtime_t getDuration() const override;

        virtual Ptr<Ieee80211PhyPreamble> createPreamble() const override { return makeShared<Ieee80211HtPhyPreamble>(); }
};

class INET_API Ieee80211Htmcs
{
    protected:
        const unsigned int mcsIndex;
        const Ieee80211OfdmModulation *stream1Modulation;
        const Ieee80211OfdmModulation *stream2Modulation;
        const Ieee80211OfdmModulation *stream3Modulation;
        const Ieee80211OfdmModulation *stream4Modulation;
        const Ieee80211HtCode *code;
        const Hz bandwidth;

    public:
        Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211HtCode *code, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation);
        Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211OfdmModulation *stream4Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211OfdmModulation *stream3Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211OfdmModulation *stream2Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211Htmcs(unsigned int mcsIndex, const Ieee80211OfdmModulation *stream1Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        virtual ~Ieee80211Htmcs();

        const Ieee80211HtCode* getCode() const { return code; }
        unsigned int getMcsIndex() const { return mcsIndex; }
        virtual const Ieee80211OfdmModulation* getModulation() const { return stream1Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension1Modulation() const { return stream2Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension2Modulation() const { return stream3Modulation; }
        virtual const Ieee80211OfdmModulation* getStreamExtension3Modulation() const { return stream4Modulation; }
        virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API Ieee80211HtDataMode : public IIeee80211DataMode, public Ieee80211HtModeBase, public Ieee80211HtTimingRelatedParametersBase
{
    protected:
        const Ieee80211Htmcs *modulationAndCodingScheme;
        const unsigned int numberOfBccEncoders;

    protected:
        bps computeGrossBitrate() const override;
        bps computeNetBitrate() const override;
        unsigned int computeNumberOfSpatialStreams(const Ieee80211OfdmModulation* stream1Modulation, const Ieee80211OfdmModulation* stream2Modulation, const Ieee80211OfdmModulation* stream3Modulation, const Ieee80211OfdmModulation* stream4Modulation) const;
        unsigned int computeNumberOfCodedBitsPerSubcarrierSum() const;
        unsigned int computeNumberOfBccEncoders() const;

    public:
        Ieee80211HtDataMode(const Ieee80211Htmcs *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType);

        b getServiceFieldLength() const { return b(16); }
        b getTailFieldLength() const { return b(6) * numberOfBccEncoders; }

        virtual Hz getBandwidth() const override { return bandwidth; }
        virtual int getNumberOfSpatialStreams() const override { return Ieee80211HtModeBase::getNumberOfSpatialStreams(); }
        virtual b getPaddingLength(b dataLength) const override { return b(0); }
        virtual b getCompleteLength(b dataLength) const override;
        virtual const simtime_t getDuration(b dataLength) const override;
        virtual bps getNetBitrate() const override { return Ieee80211HtModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211HtModeBase::getGrossBitrate(); }
        virtual const Ieee80211Htmcs *getModulationAndCodingScheme() const { return modulationAndCodingScheme; }
        virtual const Ieee80211HtCode* getCode() const { return modulationAndCodingScheme->getCode(); }
        virtual const Ieee80211OfdmModulation* getModulation() const override { return modulationAndCodingScheme->getModulation(); }
};

class INET_API Ieee80211HtMode : public Ieee80211ModeBase
{
    public:
        enum BandMode
        {
            BAND_2_4GHZ,
            BAND_5GHZ
        };

    protected:
        const Ieee80211HtPreambleMode *preambleMode;
        const Ieee80211HtDataMode *dataMode;
        const BandMode centerFrequencyMode;

    protected:
        virtual inline int getLegacyCwMin() const override { return 15; }
        virtual inline int getLegacyCwMax() const override { return 1023; }

    public:
        Ieee80211HtMode(const char *name, const Ieee80211HtPreambleMode *preambleMode, const Ieee80211HtDataMode *dataMode, const BandMode centerFrequencyMode);
        virtual ~Ieee80211HtMode() { delete preambleMode; delete dataMode; }

        virtual const Ieee80211HtDataMode* getDataMode() const override { return dataMode; }
        virtual const Ieee80211HtPreambleMode* getPreambleMode() const override { return preambleMode; }
        virtual const Ieee80211HtSignalMode *getHeaderMode() const override { return preambleMode->getSignalMode(); }
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

        virtual const simtime_t getDuration(b dataLength) const override { return preambleMode->getDuration() + dataMode->getDuration(dataLength); }
};

// A specification of the high-throughput (HT) physical layer (PHY)
// parameters that consists of modulation order (e.g., BPSK, QPSK, 16-QAM,
// 64-QAM) and forward error correction (FEC) coding rate (e.g., 1/2, 2/3,
// 3/4, 5/6).
class INET_API Ieee80211HtmcsTable
{
    public:
        // Table 20-30—MCS parameters for mandatory 20 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211Htmcs> htMcs0BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs1BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs2BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs3BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs4BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs5BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs6BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs7BW20MHz;

        // Table 20-31—MCS parameters for optional 20 MHz, N_SS = 2, N_ES = 1, EQM
        static const DI<Ieee80211Htmcs> htMcs8BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs9BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs10BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs11BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs12BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs13BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs14BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs15BW20MHz;

        // Table 20-32—MCS parameters for optional 20 MHz, N_SS = 3, N_ES = 1, EQM
        static const DI<Ieee80211Htmcs> htMcs16BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs17BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs18BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs19BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs20BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs21BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs22BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs23BW20MHz;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 4, N_ES = 1, EQM
        static const DI<Ieee80211Htmcs> htMcs24BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs25BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs26BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs27BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs28BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs29BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs30BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs31BW20MHz;

        // Table 20-34—MCS parameters for optional 40 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211Htmcs> htMcs0BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs1BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs2BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs3BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs4BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs5BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs6BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs7BW40MHz;

        // Table 20-35—MCS parameters for optional 40 MHz, N_SS = 2, N_ES = 1, EQM
        static const DI<Ieee80211Htmcs> htMcs8BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs9BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs10BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs11BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs12BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs13BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs14BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs15BW40MHz;

        // Table 20-36—MCS parameters for optional 40 MHz, N_SS = 3, EQM
        static const DI<Ieee80211Htmcs> htMcs16BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs17BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs18BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs19BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs20BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs21BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs22BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs23BW40MHz;

        // Table 20-37—MCS parameters for optional 40 MHz, N_SS = 4, EQM
        static const DI<Ieee80211Htmcs> htMcs24BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs25BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs26BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs27BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs28BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs29BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs30BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs31BW40MHz;

        // Table 20-38—MCS parameters for optional 40 MHz MCS 32 format, N_SS = 1, N_ES = 1
        static const DI<Ieee80211Htmcs> htMcs32BW40MHz;

        // Table 20-39—MCS parameters for optional 20 MHz, N_SS = 2, N_ES = 1, UEQM
        static const DI<Ieee80211Htmcs> htMcs33BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs34BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs35BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs36BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs37BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs38BW20MHz;

        // Table 20-40—MCS parameters for optional 20 MHz, N SS = 3, N ES = 1, UEQM
        static const DI<Ieee80211Htmcs> htMcs39BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs40BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs41BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs42BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs43BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs44BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs45BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs46BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs47BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs48BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs49BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs50BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs51BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs52BW20MHz;

        // Table 20-41—MCS parameters for optional 20 MHz, N_SS = 4, N_ES = 1, UEQM
        static const DI<Ieee80211Htmcs> htMcs53BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs54BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs55BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs56BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs57BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs58BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs59BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs60BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs61BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs62BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs63BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs64BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs65BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs66BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs67BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs68BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs69BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs70BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs71BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs72BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs73BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs74BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs75BW20MHz;
        static const DI<Ieee80211Htmcs> htMcs76BW20MHz;

        // Table 20-42—MCS parameters for optional 40 MHz, N_SS = 2, N_ES = 1, UEQM
        static const DI<Ieee80211Htmcs> htMcs33BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs34BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs35BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs36BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs37BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs38BW40MHz;

        // Table 20-43—MCS parameters for optional 40 MHz, N SS = 3, UEQM
        static const DI<Ieee80211Htmcs> htMcs39BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs40BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs41BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs42BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs43BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs44BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs45BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs46BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs47BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs48BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs49BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs50BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs51BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs52BW40MHz;

        // Table 20-44—MCS parameters for optional 40 MHz, N_SS = 4, UEQM
        static const DI<Ieee80211Htmcs> htMcs53BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs54BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs55BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs56BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs57BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs58BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs59BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs60BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs61BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs62BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs63BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs64BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs65BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs66BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs67BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs68BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs69BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs70BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs71BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs72BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs73BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs74BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs75BW40MHz;
        static const DI<Ieee80211Htmcs> htMcs76BW40MHz;
};

class INET_API Ieee80211HtCompliantModes
{
    protected:
        static Ieee80211HtCompliantModes singleton;

        std::map<std::tuple<Hz, unsigned int, Ieee80211HtModeBase::GuardIntervalType>, const Ieee80211HtMode *> modeCache;

    public:
        Ieee80211HtCompliantModes();
        virtual ~Ieee80211HtCompliantModes();

        static const Ieee80211HtMode *getCompliantMode(const Ieee80211Htmcs *mcsMode, Ieee80211HtMode::BandMode centerFrequencyMode, Ieee80211HtPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211HtModeBase::GuardIntervalType guardIntervalType);

};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211HTMODE_H
