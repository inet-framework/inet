/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 The Boeing Company
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Gary Pei <guangyu.pei@boeing.com>
 */

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

Ieee80211NistErrorModel::Ieee80211NistErrorModel()
{
}

Ieee80211NistErrorModel::~Ieee80211NistErrorModel()
{
}

double Ieee80211NistErrorModel::GetBpskBer(double snr) const
{
    double z = sqrt(snr);
    double ber = 0.5 * erfc(z);
    EV << "bpsk snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::GetQpskBer(double snr) const
{
    double z = sqrt(snr / 2.0);
    double ber = 0.5 * erfc(z);
    EV << "qpsk snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::Get16QamBer(double snr) const
{
    double z = sqrt(snr / (5.0 * 2.0));
    double ber = 0.75 * 0.5 * erfc(z);
    EV << "16-Qam" << " snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::Get64QamBer(double snr) const
{
    double z = sqrt(snr / (21.0 * 2.0));
    double ber = 7.0 / 12.0 * 0.5 * erfc(z);
    EV << "64-Qam" << " snr=" << snr << " ber=" << ber << "\n";
    return ber;
}

double Ieee80211NistErrorModel::GetFecBpskBer(double snr, double nbits, uint32_t bValue) const
{
    double ber = GetBpskBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = pow(1 - pe, nbits);
    return pms;
}

double Ieee80211NistErrorModel::GetFecQpskBer(double snr, double nbits, uint32_t bValue) const
{
    double ber = GetQpskBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = pow(1 - pe, nbits);
    return pms;
}

double Ieee80211NistErrorModel::CalculatePe(double p, uint32_t bValue) const
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

double Ieee80211NistErrorModel::GetFec16QamBer(double snr, uint32_t nbits, uint32_t bValue) const
{
    double ber = Get16QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = pow(1 - pe, (double)nbits);
    return pms;
}

double Ieee80211NistErrorModel::GetFec64QamBer(double snr, uint32_t nbits, uint32_t bValue) const
{
    double ber = Get64QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = CalculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = pow(1 - pe, (double)nbits);
    return pms;
}

double Ieee80211NistErrorModel::GetChunkSuccessRate(const IIeee80211ChunkMode *chunkMode, double snr, uint32_t nbits) const
{
    if (dynamic_cast<const Ieee80211OFDMChunkMode *>(chunkMode) /* TODO: || dynamic_cast<const Ieee80211ERPOFDMChunkMode *>(mode)*/) {
        const Ieee80211OFDMChunkMode *ofdmChunkMode = dynamic_cast<const Ieee80211OFDMChunkMode *>(chunkMode);
        const ConvolutionalCode *convolutionalCode = ofdmChunkMode->getCode()->getConvolutionalCode();
        if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &BPSKModulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2) {
                return GetFecBpskBer(snr, nbits, 1);
            }
            else {
                return GetFecBpskBer(snr, nbits, 3);
            }
        }
        else if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &QPSKModulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2) {
                return GetFecQpskBer(snr, nbits, 1);
            }
            else {
                return GetFecQpskBer(snr, nbits, 3);
            }
        }
        else if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &QAM16Modulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2) {
                return GetFec16QamBer(snr, nbits, 1);
            }
            else {
                return GetFec16QamBer(snr, nbits, 3);
            }
        }
        else if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &QAM64Modulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 2 && convolutionalCode->getCodeRatePuncturingN() == 3) {
                return GetFec64QamBer(snr, nbits, 2);
            }
            else {
                return GetFec64QamBer(snr, nbits, 3);
            }
        }
        else
            throw cRuntimeError("Unknown modulation");
    }
    else if (dynamic_cast<const Ieee80211DsssChunkMode *>(chunkMode) || dynamic_cast<const Ieee80211HrDsssChunkMode *>(chunkMode)) {
        switch ((int)chunkMode->getNetBitrate().get()) {
            case 1000000:
                return DsssErrorRateModel::GetDsssDbpskSuccessRate(snr, nbits);

            case 2000000:
                return DsssErrorRateModel::GetDsssDqpskSuccessRate(snr, nbits);

            case 5500000:
                return DsssErrorRateModel::GetDsssDqpskCck5_5SuccessRate(snr, nbits);

            case 11000000:
                return DsssErrorRateModel::GetDsssDqpskCck11SuccessRate(snr, nbits);
        }
    }
    return 0;
}

} // namespace physicallayer

} // namespace inet

