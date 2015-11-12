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

#ifndef __INET_IEEE80211YANSERRORMODEL_H
#define __INET_IEEE80211YANSERRORMODEL_H

#include "inet/physicallayer/ieee80211/packetlevel/errormodel/Ieee80211ErrorModelBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/errormodel/dsss-error-rate-model.h"

namespace inet {

namespace physicallayer {

/**
 * \brief Model the error rate for different modulations.
 *
 * A packet of interest (e.g., a packet can potentially be received by the MAC)
 * is divided into chunks. Each chunk is related to an start/end receiving event.
 * For each chunk, it calculates the ratio (SINR) between received power of packet
 * of interest and summation of noise and interfering power of all the other incoming
 * packets. Then, it will calculate the success rate of the chunk based on
 * BER of the modulation. The success reception rate of the packet is derived from
 * the success rate of all chunks.
 *
 * The 802.11b modulations:
 *    - 1 Mbps mode is based on DBPSK. BER is from equation 5.2-69 from John G. Proakis
 *      Digitial Communications, 2001 edition
 *    - 2 Mbps model is based on DQPSK. Equation 8 from "Tight bounds and accurate
 *      approximations for dqpsk transmission bit error rate", G. Ferrari and G.E. Corazza
 *      ELECTRONICS LETTERS, 40(20):1284-1285, September 2004
 *    - 5.5 Mbps and 11 Mbps are based on equations (18) and (17) from "Properties and
 *      performance of the ieee 802.11b complementarycode-key signal sets",
 *      Michael B. Pursley and Thomas C. Royster. IEEE TRANSACTIONS ON COMMUNICATIONS,
 *      57(2):440-449, February 2009.
 *    - More detailed description and validation can be found in
 *      http://www.nsnam.org/~pei/80211b.pdf
 */
class INET_API Ieee80211YansErrorModel : public Ieee80211ErrorModelBase
{
  protected:
    double getBpskBer(double snr, Hz signalSpread, bps phyRate) const;
    double getQamBer(double snr, unsigned int m, Hz signalSpread, bps phyRate) const;
    uint32_t factorial(uint32_t k) const;
    double binomialCoefficient(uint32_t k, double p, uint32_t n) const;
    double calculatePdOdd(double ber, unsigned int d) const;
    double calculatePdEven(double ber, unsigned int d) const;
    double calculatePd(double ber, unsigned int d) const;
    double getFecBpskBer(double snr, double nbits, Hz signalSpread, bps phyRate, uint32_t dFree, uint32_t adFree) const;
    double getFecQamBer(double snr, uint32_t nbits, Hz signalSpread, bps phyRate, uint32_t m, uint32_t dfree, uint32_t adFree, uint32_t adFreePlusOne) const;

    virtual double getOFDMAndERPOFDMChunkSuccessRate(const APSKModulationBase *subcarrierModulation, const ConvolutionalCode *convolutionalCode, unsigned int bitLength, bps gorssBitrate, Hz bandwidth, double snr) const;
    virtual double getDSSSAndHrDSSSChunkSuccessRate(bps bitrate, unsigned int bitLength, double snr) const;

    virtual double getHeaderSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const override;
    virtual double getDataSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return stream << "Ieee80211YansErrorModel"; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_YANSERRORRATEMODEL_H

