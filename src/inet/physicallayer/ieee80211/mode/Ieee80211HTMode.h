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

#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTCode.h"
#include "inet/common/DelayedInitializer.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211HTTimingRelatedParametersBase
{
    public:
        const simtime_t getDFTPeriod() const { return 3.2E-6; } // DFT
        const simtime_t getGIDuration() const { return getDFTPeriod() / 4; } // GI
        const simtime_t getShortGIDuration() const { return getDFTPeriod() / 8; } // GIS
        const simtime_t getSymbolInterval() const { return getDFTPeriod() + getGIDuration(); } // SYM
        const simtime_t getShortGISymbolInterval() const { return getDFTPeriod() + getShortGIDuration(); } // SYMS
};

class INET_API Ieee80211HTModeBase
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
        Ieee80211HTModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType);

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

class INET_API Ieee80211HTSignalMode : public IIeee80211HeaderMode, public Ieee80211HTModeBase, public Ieee80211HTTimingRelatedParametersBase
{
    protected:
        const Ieee80211OFDMModulation *modulation;
        const Ieee80211HTCode *code;

    protected:
        virtual bps computeGrossBitrate() const override;
        virtual bps computeNetBitrate() const override;

    public:
        Ieee80211HTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation *modulation, const Ieee80211HTCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType);
        Ieee80211HTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation *modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType);
        virtual ~Ieee80211HTSignalMode();

        /* Table 20-11—HT-SIG fields, 1699p */

        // HT-SIG_1 (24 bits)
        virtual inline int getMCSLength() const { return 7; }
        virtual inline int getCBWLength() const { return 1; }
        virtual inline int getHTLengthLength() const { return 16; }

        // HT-SIG_2 (24 bits)
        virtual inline int getSmoothingLength() const { return 1; }
        virtual inline int getNotSoundingLength() const { return 1; }
        virtual inline int getReservedLength() const { return 1; }
        virtual inline int getAggregationLength() const { return 1; }
        virtual inline int getSTBCLength() const { return 2; }
        virtual inline int getFECCodingLength() const { return 1; }
        virtual inline int getShortGILength() const { return 1; }
        virtual inline int getNumOfExtensionSpatialStreamsLength() const { return 2; }
        virtual inline int getCRCLength() const { return 8; }
        virtual inline int getTailBitsLength() const { return 6; }
        virtual unsigned int getSTBC() const { return 0; } // Limitation: We assume that STBC is not used

        virtual const inline simtime_t getHTSIGDuration() const { return 2 * getSymbolInterval(); } // HT-SIG

        virtual unsigned int getModulationAndCodingScheme() const { return mcsIndex; }
        virtual const simtime_t getDuration() const override { return getHTSIGDuration(); }
        virtual int getBitLength() const override;
        virtual bps getNetBitrate() const override { return Ieee80211HTModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211HTModeBase::getGrossBitrate(); }
        virtual const IModulation *getModulation() const override { return modulation; }
};

/*
 * The HT preambles are defined in HT-mixed format and in HT-greenfield format to carry the required
 * information to operate in a system with multiple transmit and multiple receive antennas. (20.3.9 HT preamble)
 */
class INET_API Ieee80211HTPreambleMode : public IIeee80211PreambleMode, public Ieee80211HTTimingRelatedParametersBase
{
    public:
        enum HighTroughputPreambleFormat
        {
            HT_PREAMBLE_MIXED,      // can be received by non-HT STAs compliant with Clause 18 or Clause 19
            HT_PREAMBLE_GREENFIELD  // all of the non-HT fields are omitted
        };

    protected:
        const Ieee80211HTSignalMode *highThroughputSignalMode; // In HT-terminology the HT-SIG (signal field) and L-SIG are part of the preamble
        const Ieee80211OFDMSignalMode *legacySignalMode; // L-SIG
        const HighTroughputPreambleFormat preambleFormat;
        const unsigned int numberOfHTLongTrainings; // N_LTF, 20.3.9.4.6 HT-LTF definition

    protected:
        virtual unsigned int computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const;
        virtual unsigned int computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const;

    public:
        Ieee80211HTPreambleMode(const Ieee80211HTSignalMode* highThroughputSignalMode, const Ieee80211OFDMSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream);
        virtual ~Ieee80211HTPreambleMode() { delete highThroughputSignalMode; }

        HighTroughputPreambleFormat getPreambleFormat() const { return preambleFormat; }
        virtual const Ieee80211HTSignalMode *getSignalMode() const { return highThroughputSignalMode; }
        virtual const Ieee80211OFDMSignalMode *getLegacySignalMode() const { return legacySignalMode; }
        virtual const Ieee80211HTSignalMode* getHighThroughputSignalMode() const { return highThroughputSignalMode; }
        virtual inline unsigned int getNumberOfHTLongTrainings() const { return numberOfHTLongTrainings; }

        virtual const inline simtime_t getDoubleGIDuration() const { return 2 * getGIDuration(); } // GI2
        virtual const inline simtime_t getLSIGDuration() const { return getSymbolInterval(); } // L-SIG
        virtual const inline simtime_t getNonHTShortTrainingSequenceDuration() const { return 10 * getDFTPeriod() / 4;  } // L-STF
        virtual const inline simtime_t getHTGreenfieldShortTrainingFieldDuration() const { return 10 * getDFTPeriod() / 4; } // HT-GF-STF
        virtual const inline simtime_t getNonHTLongTrainingFieldDuration() const { return 2 * getDFTPeriod() + getDoubleGIDuration(); } // L-LTF
        virtual const inline simtime_t getHTShortTrainingFieldDuration() const { return 4E-6; } // HT-STF
        virtual const simtime_t getFirstHTLongTrainingFieldDuration() const;
        virtual const inline simtime_t getSecondAndSubsequentHTLongTrainingFielDuration() const { return 4E-6; } // HT-LTFs, s = 2,3,..,n
        virtual const inline unsigned int getNumberOfHtLongTrainings() const { return numberOfHTLongTrainings; }

        virtual const simtime_t getDuration() const override;

};

class INET_API Ieee80211HTMCS
{
    protected:
        const unsigned int mcsIndex;
        const Ieee80211OFDMModulation *stream1Modulation;
        const Ieee80211OFDMModulation *stream2Modulation;
        const Ieee80211OFDMModulation *stream3Modulation;
        const Ieee80211OFDMModulation *stream4Modulation;
        const Ieee80211HTCode *code;
        const Hz bandwidth;

    public:
        Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211HTCode *code, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211OFDMModulation *stream4Modulation);
        Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211OFDMModulation *stream4Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211HTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        virtual ~Ieee80211HTMCS();

        const Ieee80211HTCode* getCode() const { return code; }
        unsigned int getMcsIndex() const { return mcsIndex; }
        virtual const Ieee80211OFDMModulation* getModulation() const { return stream1Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension1Modulation() const { return stream2Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension2Modulation() const { return stream3Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension3Modulation() const { return stream4Modulation; }
        virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API Ieee80211HTDataMode : public IIeee80211DataMode, public Ieee80211HTModeBase, public Ieee80211HTTimingRelatedParametersBase
{
    protected:
        const Ieee80211HTMCS *modulationAndCodingScheme;
        const unsigned int numberOfBccEncoders;

    protected:
        bps computeGrossBitrate() const override;
        bps computeNetBitrate() const override;
        unsigned int computeNumberOfSpatialStreams(const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) const;
        unsigned int computeNumberOfCodedBitsPerSubcarrierSum() const;
        unsigned int computeNumberOfBccEncoders() const;

    public:
        Ieee80211HTDataMode(const Ieee80211HTMCS *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType);

        inline int getServiceBitLength() const { return 16; }
        inline int getTailBitLength() const { return 6 * numberOfBccEncoders; }

        virtual int getNumberOfSpatialStreams() const override { return Ieee80211HTModeBase::getNumberOfSpatialStreams(); }
        virtual int getBitLength(int dataBitLength) const override;
        virtual const simtime_t getDuration(int dataBitLength) const override;
        virtual bps getNetBitrate() const override { return Ieee80211HTModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211HTModeBase::getGrossBitrate(); }
        virtual const Ieee80211HTMCS *getModulationAndCodingScheme() const { return modulationAndCodingScheme; }
        virtual const Ieee80211HTCode* getCode() const { return modulationAndCodingScheme->getCode(); }
        virtual const Ieee80211OFDMModulation* getModulation() const override { return modulationAndCodingScheme->getModulation(); }
};

class INET_API Ieee80211HTMode : public Ieee80211ModeBase
{
    public:
        enum BandMode
        {
            BAND_2_4GHZ,
            BAND_5GHZ
        };

    protected:
        const Ieee80211HTPreambleMode *preambleMode;
        const Ieee80211HTDataMode *dataMode;
        const BandMode carrierFrequencyMode;

    protected:
        virtual inline int getLegacyCwMin() const override { return 15; }
        virtual inline int getLegacyCwMax() const override { return 1023; }

    public:
        Ieee80211HTMode(const char *name, const Ieee80211HTPreambleMode *preambleMode, const Ieee80211HTDataMode *dataMode, const BandMode carrierFrequencyMode);
        virtual ~Ieee80211HTMode() { delete preambleMode; delete dataMode; }

        virtual const Ieee80211HTDataMode* getDataMode() const override { return dataMode; }
        virtual const Ieee80211HTPreambleMode* getPreambleMode() const override { return preambleMode; }
        virtual const Ieee80211HTSignalMode *getHeaderMode() const override { return preambleMode->getSignalMode(); }
        virtual const Ieee80211OFDMSignalMode *getLegacySignalMode() const { return preambleMode->getLegacySignalMode(); }

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
        virtual BandMode getCarrierFrequencyMode() const { return carrierFrequencyMode; }

        virtual const simtime_t getDuration(int dataBitLength) const override { return preambleMode->getDuration() + dataMode->getDuration(dataBitLength); }
};

// A specification of the high-throughput (HT) physical layer (PHY)
// parameters that consists of modulation order (e.g., BPSK, QPSK, 16-QAM,
// 64-QAM) and forward error correction (FEC) coding rate (e.g., 1/2, 2/3,
// 3/4, 5/6).
class INET_API Ieee80211HTMCSTable
{
    public:
        // Table 20-30—MCS parameters for mandatory 20 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211HTMCS> htMcs0BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs1BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs2BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs3BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs4BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs5BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs6BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs7BW20MHz;

        // Table 20-31—MCS parameters for optional 20 MHz, N_SS = 2, N_ES = 1, EQM
        static const DI<Ieee80211HTMCS> htMcs8BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs9BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs10BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs11BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs12BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs13BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs14BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs15BW20MHz;

        // Table 20-32—MCS parameters for optional 20 MHz, N_SS = 3, N_ES = 1, EQM
        static const DI<Ieee80211HTMCS> htMcs16BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs17BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs18BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs19BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs20BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs21BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs22BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs23BW20MHz;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 4, N_ES = 1, EQM
        static const DI<Ieee80211HTMCS> htMcs24BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs25BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs26BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs27BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs28BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs29BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs30BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs31BW20MHz;

        // Table 20-34—MCS parameters for optional 40 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211HTMCS> htMcs0BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs1BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs2BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs3BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs4BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs5BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs6BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs7BW40MHz;

        // Table 20-35—MCS parameters for optional 40 MHz, N_SS = 2, N_ES = 1, EQM
        static const DI<Ieee80211HTMCS> htMcs8BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs9BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs10BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs11BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs12BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs13BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs14BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs15BW40MHz;

        // Table 20-36—MCS parameters for optional 40 MHz, N_SS = 3, EQM
        static const DI<Ieee80211HTMCS> htMcs16BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs17BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs18BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs19BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs20BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs21BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs22BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs23BW40MHz;

        // Table 20-37—MCS parameters for optional 40 MHz, N_SS = 4, EQM
        static const DI<Ieee80211HTMCS> htMcs24BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs25BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs26BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs27BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs28BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs29BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs30BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs31BW40MHz;

        // Table 20-38—MCS parameters for optional 40 MHz MCS 32 format, N_SS = 1, N_ES = 1
        static const DI<Ieee80211HTMCS> htMcs32BW40MHz;

        // Table 20-39—MCS parameters for optional 20 MHz, N_SS = 2, N_ES = 1, UEQM
        static const DI<Ieee80211HTMCS> htMcs33BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs34BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs35BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs36BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs37BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs38BW20MHz;

        // Table 20-40—MCS parameters for optional 20 MHz, N SS = 3, N ES = 1, UEQM
        static const DI<Ieee80211HTMCS> htMcs39BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs40BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs41BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs42BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs43BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs44BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs45BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs46BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs47BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs48BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs49BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs50BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs51BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs52BW20MHz;

        // Table 20-41—MCS parameters for optional 20 MHz, N_SS = 4, N_ES = 1, UEQM
        static const DI<Ieee80211HTMCS> htMcs53BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs54BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs55BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs56BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs57BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs58BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs59BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs60BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs61BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs62BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs63BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs64BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs65BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs66BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs67BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs68BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs69BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs70BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs71BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs72BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs73BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs74BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs75BW20MHz;
        static const DI<Ieee80211HTMCS> htMcs76BW20MHz;

        // Table 20-42—MCS parameters for optional 40 MHz, N_SS = 2, N_ES = 1, UEQM
        static const DI<Ieee80211HTMCS> htMcs33BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs34BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs35BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs36BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs37BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs38BW40MHz;

        // Table 20-43—MCS parameters for optional 40 MHz, N SS = 3, UEQM
        static const DI<Ieee80211HTMCS> htMcs39BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs40BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs41BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs42BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs43BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs44BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs45BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs46BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs47BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs48BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs49BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs50BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs51BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs52BW40MHz;

        // Table 20-44—MCS parameters for optional 40 MHz, N_SS = 4, UEQM
        static const DI<Ieee80211HTMCS> htMcs53BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs54BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs55BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs56BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs57BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs58BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs59BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs60BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs61BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs62BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs63BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs64BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs65BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs66BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs67BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs68BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs69BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs70BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs71BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs72BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs73BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs74BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs75BW40MHz;
        static const DI<Ieee80211HTMCS> htMcs76BW40MHz;
};

class INET_API Ieee80211HTCompliantModes
{
    protected:
        static Ieee80211HTCompliantModes singleton;

        std::map<std::tuple<Hz, unsigned int, Ieee80211HTModeBase::GuardIntervalType>, const Ieee80211HTMode *> modeCache;

    public:
        Ieee80211HTCompliantModes();
        virtual ~Ieee80211HTCompliantModes();

        static const Ieee80211HTMode *getCompliantMode(const Ieee80211HTMCS *mcsMode, Ieee80211HTMode::BandMode carrierFrequencyMode, Ieee80211HTPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211HTModeBase::GuardIntervalType guardIntervalType);

};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211HTMODE_H
