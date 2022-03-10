//
// Copyright (c) 2005, 2006 INRIA
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
//

#include "inet/physicallayer/wireless/ieee80211/packetlevel/errormodel/Ieee80211YansErrorModel.h"

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

Define_Module(Ieee80211YansErrorModel);

double Ieee80211YansErrorModel::getBpskBer(double snr, Hz signalSpread, bps phyRate) const
{
    double EbNo = snr * signalSpread.get() / phyRate.get();
    double z = sqrt(EbNo);
    double ber = 0.5 * erfc(z);
    EV << "bpsk snr=" << snr << " ber=" << ber << endl;
    return ber;
}

double Ieee80211YansErrorModel::getQamBer(double snr, unsigned int m, Hz signalSpread, bps phyRate) const
{
    double EbNo = snr * signalSpread.get() / phyRate.get();
    double z = sqrt((1.5 * log2(m) * EbNo) / (m - 1.0));
    double z1 = ((1.0 - 1.0 / sqrt((double)m)) * erfc(z));
    double z2 = 1 - pow((1 - z1), 2.0);
    double ber = z2 / log2(m);
    EV << "Qam m=" << m << " rate=" << phyRate << " snr=" << snr << " ber=" << ber << endl;
    return ber;
}

uint32_t Ieee80211YansErrorModel::factorial(uint32_t k) const
{
    uint32_t fact = 1;
    while (k > 0) {
        fact *= k;
        k--;
    }
    return fact;
}

double Ieee80211YansErrorModel::binomialCoefficient(uint32_t k, double p, uint32_t n) const
{
    double retval = factorial(n) / (factorial(k) * factorial(n - k)) * pow(p, (int)k) * pow(1 - p, (int)(n - k));
    return retval;
}

double Ieee80211YansErrorModel::calculatePdOdd(double ber, unsigned int d) const
{
    ASSERT((d % 2) == 1);
    unsigned int dstart = (d + 1) / 2;
    unsigned int dend = d;
    double pd = 0;

    for (unsigned int i = dstart; i < dend; i++) {
        pd += binomialCoefficient(i, ber, d);
    }
    return pd;
}

double Ieee80211YansErrorModel::calculatePdEven(double ber, unsigned int d) const
{
    ASSERT((d % 2) == 0);
    unsigned int dstart = d / 2 + 1;
    unsigned int dend = d;
    double pd = 0;

    for (unsigned int i = dstart; i < dend; i++) {
        pd += binomialCoefficient(i, ber, d);
    }
    pd += 0.5 * binomialCoefficient(d / 2, ber, d);

    return pd;
}

double Ieee80211YansErrorModel::calculatePd(double ber, unsigned int d) const
{
    double pd;
    if ((d % 2) == 0) {
        pd = calculatePdEven(ber, d);
    }
    else {
        pd = calculatePdOdd(ber, d);
    }
    return pd;
}

double Ieee80211YansErrorModel::getFecBpskBer(double snr, double nbits, Hz signalSpread, bps phyRate, uint32_t dFree, uint32_t adFree) const
{
    double ber = getBpskBer(snr, signalSpread, phyRate);
    if (ber == 0.0) {
        return 1.0;
    }
    double pd = calculatePd(ber, dFree);
    double pmu = adFree * pd;
    pmu = math::minnan(pmu, 1.0);
    double pms = pow(1 - pmu, nbits);
    return pms;
}

double Ieee80211YansErrorModel::getFecQamBer(double snr, uint32_t nbits, Hz signalSpread, bps phyRate, uint32_t m, uint32_t dFree, uint32_t adFree, uint32_t adFreePlusOne) const
{
    double ber = getQamBer(snr, m, signalSpread, phyRate);
    if (ber == 0.0) {
        return 1.0;
    }
    /* first term */
    double pd = calculatePd(ber, dFree);
    double pmu = adFree * pd;
    /* second term */
    pd = calculatePd(ber, dFree + 1);
    pmu += adFreePlusOne * pd;
    pmu = math::minnan(pmu, 1.0);
    double pms = pow(1 - pmu, (int)nbits);
    return pms;
}

double Ieee80211YansErrorModel::getOFDMAndERPOFDMChunkSuccessRate(const ApskModulationBase *subcarrierModulation, const ConvolutionalCode *convolutionalCode, unsigned int bitLength, bps grossBitrate, Hz bandwidth, double snr) const
{
    if (subcarrierModulation == &BpskModulation::singleton || subcarrierModulation == &QbpskModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecBpskBer(snr, bitLength, bandwidth, grossBitrate, 10, 11);
        else
            return getFecBpskBer(snr, bitLength, bandwidth, grossBitrate, 5, 8);
    }
    else if (subcarrierModulation == &QpskModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 4, 10, 11, 0);
        else
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 4, 5, 8, 31);
    }
    else if (subcarrierModulation == &Qam16Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 16, 10, 11, 0);
        else
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 16, 5, 8, 31);
    }
    else if (subcarrierModulation == &Qam64Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 2 && convolutionalCode->getCodeRatePuncturingN() == 3)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 64, 6, 1, 16);
        else if (convolutionalCode->getCodeRatePuncturingK() == 5 && convolutionalCode->getCodeRatePuncturingN() == 6)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 64, 4, 14, 69);
        else
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 64, 5, 8, 31);
    }
    else if (subcarrierModulation == &Qam256Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 5 && convolutionalCode->getCodeRatePuncturingN() == 6)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 256, 4, 14, 69);
        else
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 256, 5, 8, 31);
    }
    else
        throw cRuntimeError("Unknown modulation");
}

double Ieee80211YansErrorModel::getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const
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

double Ieee80211YansErrorModel::getHeaderSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OfdmMode *>(mode)) {
        int chunkLength = bitLength - b(ofdmMode->getHeaderMode()->getServiceFieldLength()).get();
        ASSERT(chunkLength == 24);
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getHeaderMode()->getCode()->getConvolutionalCode(),
                                                        chunkLength,
                                                        ofdmMode->getHeaderMode()->getGrossBitrate(),
                                                        ofdmMode->getHeaderMode()->getBandwidth(),
                                                        snr);
    }
    else if (auto htMode = dynamic_cast<const Ieee80211HtMode *>(mode)) {
//        int chunkLength = bitLength - b(htMode->getHeaderMode()->getHTLengthLength()).get();
//        ASSERT(chunkLength == 24);
        int chunkLength = bitLength;
        successRate = getOFDMAndERPOFDMChunkSuccessRate(htMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        htMode->getHeaderMode()->getCode()->getForwardErrorCorrection(),
                                                        chunkLength,
                                                        htMode->getHeaderMode()->getGrossBitrate(),
                                                        htMode->getHeaderMode()->getBandwidth(),
                                                        snr);
    }
    else if (auto vhtMode = dynamic_cast<const Ieee80211VhtMode *>(mode)) {
//        int chunkLength = bitLength - b(vhtMode->getHeaderMode()->getHTLengthLength()).get();
//        ASSERT(chunkLength == 24);
        int chunkLength = bitLength;
        successRate = getOFDMAndERPOFDMChunkSuccessRate(vhtMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        vhtMode->getHeaderMode()->getCode()->getForwardErrorCorrection(),
                                                        chunkLength,
                                                        vhtMode->getHeaderMode()->getGrossBitrate(),
                                                        vhtMode->getHeaderMode()->getBandwidth(),
                                                        snr);
    }
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "SNIR = " << snr << ", header bit length = " << bitLength << ", header error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

double Ieee80211YansErrorModel::getDataSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OfdmMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getDataMode()->getCode()->getConvolutionalCode(),
                                                        bitLength + b(ofdmMode->getHeaderMode()->getServiceFieldLength()).get(),
                                                        ofdmMode->getDataMode()->getGrossBitrate(),
                                                        ofdmMode->getHeaderMode()->getBandwidth(),
                                                        snr);
    else if (auto htMode = dynamic_cast<const Ieee80211HtMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(htMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        htMode->getDataMode()->getCode()->getForwardErrorCorrection(),
                                                        //bitLength + b(htMode->getHeaderMode()->getHTLengthLength()).get(),
                                                        bitLength,
                                                        htMode->getDataMode()->getGrossBitrate(),
                                                        htMode->getDataMode()->getBandwidth(),
                                                        snr);
    else if (auto vhtMode = dynamic_cast<const Ieee80211VhtMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(vhtMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        vhtMode->getDataMode()->getCode()->getForwardErrorCorrection(),
                                                        //bitLength + b(vhtMode->getHeaderMode()->getHTLengthLength()).get(),
                                                        bitLength,
                                                        vhtMode->getDataMode()->getGrossBitrate(),
                                                        vhtMode->getDataMode()->getBandwidth(),
                                                        snr);
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "SNIR = " << snr << ", data bit length = " << bitLength << ", data error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

} // namespace physicallayer

} // namespace inet

