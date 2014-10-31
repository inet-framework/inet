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
#ifndef __INET_YANSERRORRATEMODEL_H
#define __INET_YANSERRORRATEMODEL_H

#include "inet/physicallayer/ieee80211/errormodel/Ieee80211ErrorModelBase.h"
#include "inet/physicallayer/ieee80211/errormodel/dsss-error-rate-model.h"

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
class Ieee80211YansErrorModel : public Ieee80211ErrorModelBase
{
  public:
    Ieee80211YansErrorModel();

    virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211YansErrorModel"; }
    virtual double GetChunkSuccessRate(ModulationType mode, double snr, uint32_t nbits) const;

  private:
    double Log2(double val) const;
    double GetBpskBer(double snr, uint32_t signalSpread, uint32_t phyRate) const;
    double GetQamBer(double snr, unsigned int m, uint32_t signalSpread, uint32_t phyRate) const;
    uint32_t Factorial(uint32_t k) const;
    double Binomial(uint32_t k, double p, uint32_t n) const;
    double CalculatePdOdd(double ber, unsigned int d) const;
    double CalculatePdEven(double ber, unsigned int d) const;
    double CalculatePd(double ber, unsigned int d) const;
    double GetFecBpskBer(double snr, double nbits,
            uint32_t signalSpread, uint32_t phyRate,
            uint32_t dFree, uint32_t adFree) const;
    double GetFecQamBer(double snr, uint32_t nbits,
            uint32_t signalSpread,
            uint32_t phyRate,
            uint32_t m, uint32_t dfree,
            uint32_t adFree, uint32_t adFreePlusOne) const;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_YANSERRORRATEMODEL_H

