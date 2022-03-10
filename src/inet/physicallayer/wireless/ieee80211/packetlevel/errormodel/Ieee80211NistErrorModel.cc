//
// Copyright (c) 2010 The Boeing Company
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Author: Gary Pei <guangyu.pei@boeing.com>
//

#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"

#include "inet/physicallayer/wireless/common/modulation/BpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam1024Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam16Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam256Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/Qam64Modulation.h"
#include "inet/physicallayer/wireless/common/modulation/QbpskModulation.h"
#include "inet/physicallayer/wireless/common/modulation/QpskModulation.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211DsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211VhtMode.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211NistErrorModel);

double Ieee80211NistErrorModel::getBpskBer(double snr) const
{
    double z = sqrt(snr);
    double ber = 0.5 * erfc(z);
    EV << "bpsk snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::getQpskBer(double snr) const
{
    double z = sqrt(snr / 2.0);
    double ber = 0.5 * erfc(z);
    EV << "qpsk snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::get16QamBer(double snr) const
{
    double z = sqrt(snr / (5.0 * 2.0));
    double ber = 0.75 * 0.5 * erfc(z);
    EV << "16-Qam" << " snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::get64QamBer(double snr) const
{
    double z = sqrt(snr / (21.0 * 2.0));
    double ber = 7.0 / 12.0 * 0.5 * erfc(z);
    EV << "64-Qam" << " snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::get256QamBer(double snr) const
{
    double z = std::sqrt(snr / (85.0 * 2.0));
    double ber = 15.0 / 32.0 * 0.5 * erfc(z);
    EV << "256-Qam" << " snr=" << snr << " ber=" << ber;
    return ber;
}

double Ieee80211NistErrorModel::get1024QamBer(double snr) const
{
    double z = std::sqrt(snr / (341.0 * 2.0));
    double ber = 31.0 / 160.0 * 0.5 * erfc(z);
    EV << "1024-Qam" << " snr=" << snr << " ber=" << ber;
    return ber;
}

double Ieee80211NistErrorModel::getFecBpskBer(double snr, double nbits, uint32_t bValue) const
{
    double ber = getBpskBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = math::minnan(pe, 1.0);
    double pms = pow(1 - pe, nbits);
    return pms;
}

double Ieee80211NistErrorModel::getFecQpskBer(double snr, double nbits, uint32_t bValue) const
{
    double ber = getQpskBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = math::minnan(pe, 1.0);
    double pms = pow(1 - pe, nbits);
    return pms;
}

double Ieee80211NistErrorModel::calculatePe(double p, uint32_t bValue) const
{
    double D = sqrt(4.0 * p * (1.0 - p));
    double pe = 1.0;
    if (bValue == 1) {
        // code rate 1/2, use table 3.1.1
        pe = 0.5
            * (36.0 * pow(D, 10.0) + 211.0 * pow(D, 12.0) + 1404.0 * pow(D, 14.0) + 11633.0 * pow(D, 16.0)
               + 77433.0 * pow(D, 18.0) + 502690.0 * pow(D, 20.0) + 3322763.0 * pow(D, 22.0)
               + 21292910.0 * pow(D, 24.0) + 134365911.0 * pow(D, 26.0));
    }
    else if (bValue == 2) {
        // code rate 2/3, use table 3.1.2
        pe = 1.0 / (2.0 * bValue)
            * (3.0 * pow(D, 6.0) + 70.0 * pow(D, 7.0) + 285.0 * pow(D, 8.0) + 1276.0 * pow(D, 9.0)
               + 6160.0 * pow(D, 10.0) + 27128.0 * pow(D, 11.0) + 117019.0 * pow(D, 12.0)
               + 498860.0 * pow(D, 13.0) + 2103891.0 * pow(D, 14.0) + 8784123.0 * pow(D, 15.0));
    }
    else if (bValue == 3) {
        // code rate 3/4, use table 3.1.2
        pe = 1.0 / (2.0 * bValue)
            * (42.0 * pow(D, 5.0) + 201.0 * pow(D, 6.0) + 1492.0 * pow(D, 7.0) + 10469.0 * pow(D, 8.0)
               + 62935.0 * pow(D, 9.0) + 379644.0 * pow(D, 10.0) + 2253373.0 * pow(D, 11.0)
               + 13073811.0 * pow(D, 12.0) + 75152755.0 * pow(D, 13.0) + 428005675.0 * pow(D, 14.0));
    }
    else if (bValue == 5) {
        // code rate 5/6, use table V from D. Haccoun and G. Begin, "High-Rate Punctured Convolutional Codes
        // for Viterbi Sequential Decoding", IEEE Transactions on Communications, Vol. 32, Issue 3, pp.315-319.
        pe = 1.0 / (2.0 * bValue)
            * (92.0 * pow(D, 4.0) + 528.0 * pow(D, 5.0) + 8694.0 * pow(D, 6.0) + 79453.0 * pow(D, 7.0)
               + 792114.0 * pow(D, 8.0) + 7375573.0 * pow(D, 9.0) + 67884974.0 * pow(D, 10.0)
               + 610875423.0 * pow(D, 11.0) + 5427275376.0 * pow(D, 12.0) + 47664215639.0 * pow(D, 13.0));
    }
    else {
        ASSERT(false);
    }
    return pe;
}

double Ieee80211NistErrorModel::getFec16QamBer(double snr, uint32_t nbits, uint32_t bValue) const
{
    double ber = get16QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = math::minnan(pe, 1.0);
    double pms = pow(1 - pe, (double)nbits);
    return pms;
}

double Ieee80211NistErrorModel::getFec64QamBer(double snr, uint32_t nbits, uint32_t bValue) const
{
    double ber = get64QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = math::minnan(pe, 1.0);
    double pms = pow(1 - pe, (double)nbits);
    return pms;
}

double Ieee80211NistErrorModel::getFec256QamBer(double snr, uint64_t nbits, uint32_t bValue) const
{
    double ber = get256QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = math::minnan(pe, 1.0);
    double pms = std::pow(1 - pe, nbits);
    return pms;
}

double Ieee80211NistErrorModel::getFec1024QamBer(double snr, uint64_t nbits, uint32_t bValue) const
{
    double ber = get1024QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = math::minnan(pe, 1.0);
    double pms = std::pow(1 - pe, nbits);
    return pms;
}

double Ieee80211NistErrorModel::getOFDMAndERPOFDMChunkSuccessRate(const ApskModulationBase *subcarrierModulation, const ConvolutionalCode *convolutionalCode, unsigned int bitLength, double snr) const
{
    if (subcarrierModulation == &BpskModulation::singleton || subcarrierModulation == &QbpskModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecBpskBer(snr, bitLength, 1);
        return getFecBpskBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &QpskModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecQpskBer(snr, bitLength, 1);
        return getFecQpskBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &Qam16Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFec16QamBer(snr, bitLength, 1);
        return getFec16QamBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &Qam64Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 2 && convolutionalCode->getCodeRatePuncturingN() == 3)
            return getFec64QamBer(snr, bitLength, 2);
        else if (convolutionalCode->getCodeRatePuncturingK() == 5 && convolutionalCode->getCodeRatePuncturingN() == 6)
            return getFec64QamBer(snr, bitLength, 2);
        return getFec64QamBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &Qam256Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 5 && convolutionalCode->getCodeRatePuncturingN() == 6)
            return getFec256QamBer(snr, bitLength, 5);
        return getFec256QamBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &Qam1024Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 5 && convolutionalCode->getCodeRatePuncturingN() == 6)
            return getFec1024QamBer(snr, bitLength, 5);
        return getFec1024QamBer(snr, bitLength, 3);
    }
    else
        throw cRuntimeError("Unknown modulation");
}

double Ieee80211NistErrorModel::getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const
{
    switch ((int)bitrate.get()) {
        case 1000000:
            return getDsssDbpskSuccessRate(bitLength, snr);
        case 2000000:
            return getDsssDqpskSuccessRate(bitLength, snr);
        case 5500000:
            return getDsssDqpskCck5_5SuccessRate(bitLength, snr);
        case 11000000:
            return getDsssDqpskCck11SuccessRate(bitLength, snr);
    }
    throw cRuntimeError("Unsupported bitrate");
}

double Ieee80211NistErrorModel::getHeaderSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OfdmMode *>(mode)) {
        int chunkLength = bitLength - b(ofdmMode->getHeaderMode()->getServiceFieldLength()).get();
        ASSERT(chunkLength == 24);
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getHeaderMode()->getCode()->getConvolutionalCode(),
                                                        chunkLength,
                                                        snr);
    }
    else if (auto htMode = dynamic_cast<const Ieee80211HtMode *>(mode)) {
//        int chunkLength = bitLength - b(htMode->getHeaderMode()->getHTLengthLength()).get();
//        ASSERT(chunkLength == 24);
        int chunkLength = bitLength;
        successRate = getOFDMAndERPOFDMChunkSuccessRate(htMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        htMode->getHeaderMode()->getCode()->getForwardErrorCorrection(),
                                                        chunkLength,
                                                        snr);
    }
    else if (auto vhtMode = dynamic_cast<const Ieee80211VhtMode *>(mode)) {
//        int chunkLength = bitLength - b(vhtMode->getHeaderMode()->getHTLengthLength()).get();
//        ASSERT(chunkLength == 24);
        int chunkLength = bitLength;
        successRate = getOFDMAndERPOFDMChunkSuccessRate(vhtMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        vhtMode->getHeaderMode()->getCode()->getForwardErrorCorrection(),
                                                        chunkLength,
                                                        snr);
    }
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "SNIR = " << snr << ", bit length = " << bitLength << ", header error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

double Ieee80211NistErrorModel::getDataSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OfdmMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getDataMode()->getCode()->getConvolutionalCode(),
                                                        bitLength + b(ofdmMode->getHeaderMode()->getServiceFieldLength()).get(),
                                                        snr);
    else if (auto htMode = dynamic_cast<const Ieee80211HtMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(htMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        htMode->getDataMode()->getCode()->getForwardErrorCorrection(),
                                                        //bitLength + b(htMode->getHeaderMode()->getHTLengthLength()).get(),
                                                        bitLength,
                                                        snr);
    else if (auto vhtMode = dynamic_cast<const Ieee80211VhtMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(vhtMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        vhtMode->getDataMode()->getCode()->getForwardErrorCorrection(),
                                                        //bitLength + b(vhtMode->getHeaderMode()->getHTLengthLength()).get(),
                                                        bitLength,
                                                        snr);
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "SNIR = " << snr << ", bit length = " << bitLength << ", data error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

} // namespace physicallayer

} // namespace inet

