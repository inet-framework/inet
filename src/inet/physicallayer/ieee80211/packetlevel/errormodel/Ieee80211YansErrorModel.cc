/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "inet/physicallayer/modulation/BPSKModulation.h"
#include "inet/physicallayer/modulation/QPSKModulation.h"
#include "inet/physicallayer/modulation/QAM16Modulation.h"
#include "inet/physicallayer/modulation/QAM64Modulation.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211YansErrorModel.h"
#include <math.h>

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211YansErrorModel);

Ieee80211YansErrorModel::Ieee80211YansErrorModel()
{
}

double Ieee80211YansErrorModel::Log2(double val) const
{
    return log(val) / log(2.0);
}

double Ieee80211YansErrorModel::GetBpskBer(double snr, Hz signalSpread, bps phyRate) const
{
    double EbNo = snr * signalSpread.get() / phyRate.get();
    double z = sqrt(EbNo);
    double ber = 0.5 * erfc(z);
    EV << "bpsk snr=" << snr << " ber=" << ber << endl;
    return ber;
}

double Ieee80211YansErrorModel::GetQamBer(double snr, unsigned int m, Hz signalSpread, bps phyRate) const
{
    double EbNo = snr * signalSpread.get() / phyRate.get();
    double z = sqrt((1.5 * Log2(m) * EbNo) / (m - 1.0));
    double z1 = ((1.0 - 1.0 / sqrt((double)m)) * erfc(z));
    double z2 = 1 - pow((1 - z1), 2.0);
    double ber = z2 / Log2(m);
    EV << "Qam m=" << m << " rate=" << phyRate << " snr=" << snr << " ber=" << ber << endl;
    return ber;
}

uint32_t Ieee80211YansErrorModel::Factorial(uint32_t k) const
{
    uint32_t fact = 1;
    while (k > 0) {
        fact *= k;
        k--;
    }
    return fact;
}

double Ieee80211YansErrorModel::Binomial(uint32_t k, double p, uint32_t n) const
{
    double retval = Factorial(n) / (Factorial(k) * Factorial(n - k)) * pow(p, (int)k) * pow(1 - p, (int)(n - k));
    return retval;
}

double Ieee80211YansErrorModel::CalculatePdOdd(double ber, unsigned int d) const
{
    ASSERT((d % 2) == 1);
    unsigned int dstart = (d + 1) / 2;
    unsigned int dend = d;
    double pd = 0;

    for (unsigned int i = dstart; i < dend; i++) {
        pd += Binomial(i, ber, d);
    }
    return pd;
}

double Ieee80211YansErrorModel::CalculatePdEven(double ber, unsigned int d) const
{
    ASSERT((d % 2) == 0);
    unsigned int dstart = d / 2 + 1;
    unsigned int dend = d;
    double pd = 0;

    for (unsigned int i = dstart; i < dend; i++) {
        pd += Binomial(i, ber, d);
    }
    pd += 0.5 * Binomial(d / 2, ber, d);

    return pd;
}

double Ieee80211YansErrorModel::CalculatePd(double ber, unsigned int d) const
{
    double pd;
    if ((d % 2) == 0) {
        pd = CalculatePdEven(ber, d);
    }
    else {
        pd = CalculatePdOdd(ber, d);
    }
    return pd;
}

double Ieee80211YansErrorModel::GetFecBpskBer(double snr, double nbits,
        Hz signalSpread, bps phyRate,
        uint32_t dFree, uint32_t adFree) const
{
    double ber = GetBpskBer(snr, signalSpread, phyRate);
    if (ber == 0.0) {
        return 1.0;
    }
    double pd = CalculatePd(ber, dFree);
    double pmu = adFree * pd;
    pmu = std::min(pmu, 1.0);
    double pms = pow(1 - pmu, nbits);
    return pms;
}

double Ieee80211YansErrorModel::GetFecQamBer(double snr, uint32_t nbits,
        Hz signalSpread,
        bps phyRate,
        uint32_t m, uint32_t dFree,
        uint32_t adFree, uint32_t adFreePlusOne) const
{
    double ber = GetQamBer(snr, m, signalSpread, phyRate);
    if (ber == 0.0) {
        return 1.0;
    }
    /* first term */
    double pd = CalculatePd(ber, dFree);
    double pmu = adFree * pd;
    /* second term */
    pd = CalculatePd(ber, dFree + 1);
    pmu += adFreePlusOne * pd;
    pmu = std::min(pmu, 1.0);
    double pms = pow(1 - pmu, (int)nbits);
    return pms;
}

//
//
// This method return the probability of NO ERROR
//
double Ieee80211YansErrorModel::GetChunkSuccessRate(const IIeee80211ChunkMode *chunkMode, double snr, uint32_t nbits) const
{
    if (dynamic_cast<const Ieee80211OFDMChunkMode *>(chunkMode) /* TODO: || dynamic_cast<const Ieee80211ERPOFDMChunkMode *>(mode)*/) {
        const Ieee80211OFDMChunkMode *ofdmChunkMode = dynamic_cast<const Ieee80211OFDMChunkMode *>(chunkMode);
        const ConvolutionalCode *convolutionalCode = ofdmChunkMode->getCode()->getConvolutionalCode();
        if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &BPSKModulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2) {
                return GetFecBpskBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        10,    // dFree
                        11    // adFree
                        );
            }
            else {
                return GetFecBpskBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        5,    // dFree
                        8    // adFree
                        );
            }
        }
        else if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &QPSKModulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2) {
                return GetFecQamBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        4,    // m
                        10,    // dFree
                        11,    // adFree
                        0    // adFreePlusOne
                        );
            }
            else {
                return GetFecQamBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        4,    // m
                        5,    // dFree
                        8,    // adFree
                        31    // adFreePlusOne
                        );
            }
        }
        else if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &QAM16Modulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 1 && convolutionalCode->getCodeRatePuncturingN() == 2) {
                return GetFecQamBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        16,    // m
                        10,    // dFree
                        11,    // adFree
                        0    // adFreePlusOne
                        );
            }
            else {
                return GetFecQamBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        16,    // m
                        5,    // dFree
                        8,    // adFree
                        31    // adFreePlusOne
                        );
            }
        }
        else if (ofdmChunkMode->getModulation()->getSubcarrierModulation() == &QAM64Modulation::singleton) {
            if (convolutionalCode->getCodeRatePuncturingK() == 2 && convolutionalCode->getCodeRatePuncturingN() == 3) {
                return GetFecQamBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        64,    // m
                        6,    // dFree
                        1,    // adFree
                        16    // adFreePlusOne
                        );
            }
            else {
                return GetFecQamBer(snr,
                        nbits,
                        ofdmChunkMode->getBandwidth(),    // signal spread
                        chunkMode->getGrossBitrate(),    // phy rate
                        64,    // m
                        5,    // dFree
                        8,    // adFree
                        31    // adFreePlusOne
                        );
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

