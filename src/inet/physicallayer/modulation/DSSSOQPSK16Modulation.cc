//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/physicallayer/modulation/DSSSOQPSK16Modulation.h"

namespace inet {

namespace physicallayer {

DSSSOQPSK16Modulation::DSSSOQPSK16Modulation() :
    APSKModulationBase(nullptr)
{
}

double DSSSOQPSK16Modulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    // Taken from MiXiM 802.15.4 decider by Karl Wessel
    // Valid for IEEE 802.15.4 2.45 GHz OQPSK modulation
    // Following formula is defined in IEEE 802.15.4 standard, please check the
    // 2006 standard, page 268, section E.4.1.8 Bit error rate (BER)
    // calculations, formula 7). Here you can see that the factor of 20.0 is correct ;).
    const double dSNRFct = 20.0 *snir *bandwidth.get() / bitrate.get(); // TODO is this correct?
    double dSumK = 0;
    register int k = 2;

    /* following loop was optimized by using n_choose_k symmetries
       for (k=2; k <= 16; ++k) {
        dSumK += pow(-1.0, k) * n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
       }
     */

    // n_choose_k(16, k) == n_choose_k(16, 16-k)
    for ( ; k < 8; k += 2) {
        // k will be 2, 4, 6 (symmetric values: 14, 12, 10)
        dSumK += math::n_choose_k(16, k) * (exp(dSNRFct * (1.0 / k - 1.0)) + exp(dSNRFct * (1.0 / (16 - k) - 1.0)));
    }

    // for k =  8 (which does not have a symmetric value)
    k = 8;
    dSumK += math::n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
    for (k = 3; k < 8; k += 2) {
        // k will be 3, 5, 7 (symmetric values: 13, 11, 9)
        dSumK -= math::n_choose_k(16, k) * (exp(dSNRFct * (1.0 / k - 1.0)) + exp(dSNRFct * (1.0 / (16 - k) - 1.0)));
    }

    // for k = 15 (because of missing k=1 value)
    k = 15;
    dSumK -= math::n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));

    // for k = 16 (because of missing k=0 value)
    k = 16;
    dSumK += math::n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
    return (8.0 / 15) * (1.0 / 16) * dSumK;
}

double DSSSOQPSK16Modulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not yet implemented");
}

} // namespace physicallayer

} // namespace inet

