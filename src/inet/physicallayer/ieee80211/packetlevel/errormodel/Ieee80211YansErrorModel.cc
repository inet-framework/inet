//
// Copyright (c) 2005, 2006 INRIA
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
// Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
//

#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211YansErrorModel.h"

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
    pmu = std::min(pmu, 1.0);
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
    pmu = std::min(pmu, 1.0);
    double pms = pow(1 - pmu, (int)nbits);
    return pms;
}

double Ieee80211YansErrorModel::getOFDMAndERPOFDMChunkSuccessRate(const APSKModulationBase* subcarrierModulation, const ConvolutionalCode* convolutionalCode, unsigned int bitLength, bps grossBitrate, Hz bandwidth, double snr) const
{
    if (subcarrierModulation == &BPSKModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecBpskBer(snr, bitLength, bandwidth, grossBitrate, 10, 11);
        else
            return getFecBpskBer(snr, bitLength, bandwidth, grossBitrate, 5, 8 );
    }
    else if (subcarrierModulation == &QPSKModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 4, 10, 11, 0 );
        else
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 4, 5, 8, 31);
    }
    else if (subcarrierModulation == &QAM16Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 16, 10, 11, 0);
        else
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 16, 5, 8, 31);
    }
    else if (subcarrierModulation == &QAM64Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 2 && convolutionalCode->getCodeRatePuncturingN() == 3)
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 64, 6, 1, 16);
        else
            return getFecQamBer(snr, bitLength, bandwidth, grossBitrate, 64, 5, 8, 31);
    }
    else
        throw cRuntimeError("Unknown modulation");
}

double Ieee80211YansErrorModel::getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const
{
    switch ((int)bitrate.get()) {
        case 1000000:
            return DsssErrorRateModel::GetDsssDbpskSuccessRate(snr, bitLength);
        case 2000000:
            return DsssErrorRateModel::GetDsssDqpskSuccessRate(snr, bitLength);
        case 5500000:
            return DsssErrorRateModel::GetDsssDqpskCck5_5SuccessRate(snr, bitLength);
        case 11000000:
            return DsssErrorRateModel::GetDsssDqpskCck11SuccessRate(snr, bitLength);
    }
    throw cRuntimeError("Unsupported bitrate");
}

double Ieee80211YansErrorModel::getHeaderSuccessRate(const IIeee80211Mode* mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OFDMMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getHeaderMode()->getCode()->getConvolutionalCode(),
                                                        bitLength,
                                                        ofdmMode->getHeaderMode()->getGrossBitrate(),
                                                        ofdmMode->getHeaderMode()->getBandwidth(),
                                                        snr);
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "Min SNIR = " << snr << ", header bit length = " << bitLength << ", header error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

double Ieee80211YansErrorModel::getDataSuccessRate(const IIeee80211Mode* mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OFDMMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getDataMode()->getCode()->getConvolutionalCode(),
                                                        bitLength,
                                                        ofdmMode->getDataMode()->getGrossBitrate(),
                                                        ofdmMode->getHeaderMode()->getBandwidth(),
                                                        snr);
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "Min SNIR = " << snr << ", data bit length = " << bitLength << ", data error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

} // namespace physicallayer

} // namespace inet

