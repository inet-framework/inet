//
// Copyright (c) 2010 The Boeing Company
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
// Author: Gary Pei <guangyu.pei@boeing.com>
//

#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211NistErrorModel.h"

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

double Ieee80211NistErrorModel::getFecBpskBer(double snr, double nbits, uint32_t bValue) const
{
    double ber = getBpskBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
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
    pe = std::min(pe, 1.0);
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
    pe = std::min(pe, 1.0);
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
    pe = std::min(pe, 1.0);
    double pms = pow(1 - pe, (double)nbits);
    return pms;
}

double Ieee80211NistErrorModel::getOFDMAndERPOFDMChunkSuccessRate(const APSKModulationBase *subcarrierModulation, const ConvolutionalCode *convolutionalCode, unsigned int bitLength, double snr) const
{
    if (subcarrierModulation == &BPSKModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecBpskBer(snr, bitLength, 1);
        return getFecBpskBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &QPSKModulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFecQpskBer(snr, bitLength, 1);
        return getFecQpskBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &QAM16Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2)
            return getFec16QamBer(snr, bitLength, 1);
        return getFec16QamBer(snr, bitLength, 3);
    }
    else if (subcarrierModulation == &QAM64Modulation::singleton) {
        if (convolutionalCode->getCodeRatePuncturingK() == 2 && convolutionalCode->getCodeRatePuncturingN() == 3)
            return getFec64QamBer(snr, bitLength, 2);
        return getFec64QamBer(snr, bitLength, 3);
    }
    else
        throw cRuntimeError("Unknown modulation");
}

double Ieee80211NistErrorModel::getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const
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

double Ieee80211NistErrorModel::getHeaderSuccessRate(const IIeee80211Mode* mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OFDMMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getHeaderMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getHeaderMode()->getCode()->getConvolutionalCode(),
                                                        bitLength,
                                                        snr);
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getHeaderMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "Min SNIR = " << snr << ", bit length = " << bitLength << ", header error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

double Ieee80211NistErrorModel::getDataSuccessRate(const IIeee80211Mode* mode, unsigned int bitLength, double snr) const
{
    double successRate = 0;
    if (auto ofdmMode = dynamic_cast<const Ieee80211OFDMMode *>(mode))
        successRate = getOFDMAndERPOFDMChunkSuccessRate(ofdmMode->getDataMode()->getModulation()->getSubcarrierModulation(),
                                                        ofdmMode->getDataMode()->getCode()->getConvolutionalCode(),
                                                        bitLength,
                                                        snr);
    else if (auto dsssMode = dynamic_cast<const Ieee80211DsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(dsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else if (auto hrDsssMode = dynamic_cast<const Ieee80211HrDsssMode *>(mode))
        successRate = getDSSSAndHrDSSSChunkSuccessRate(hrDsssMode->getDataMode()->getNetBitrate(), bitLength, snr);
    else
        throw cRuntimeError("Unsupported 802.11 mode");
    EV_DEBUG << "Min SNIR = " << snr << ", bit length = " << bitLength << ", data error rate = " << 1 - successRate << endl;
    if (successRate >= 1)
        successRate = 1;
    return successRate;
}

} // namespace physicallayer

} // namespace inet

