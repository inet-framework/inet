/* -*- mode:c++ -*- ********************************************************
 * author:      Jerome Rousselot <jerome.rousselot@csem.ch>
 *
 * copyright:   (C) 2008 Centre Suisse d'Electronique et Microtechnique (CSEM) SA
 * 				Systems Engineering
 *              Real-Time Software and Networking
 *              Jaquet-Droz 1, CH-2002 Neuchatel, Switzerland.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 * description: this class holds constants specified in IEEE 802.15.4A UWB-IR Phy
 * acknowledgment: this work was supported (in part) by the National Competence
 * 			    Center in Research on Mobile Information and Communication Systems
 * 				NCCR-MICS, a center supported by the Swiss National Science
 * 				Foundation under grant number 5005-67322.
 ***************************************************************************/

#include "inet/physicallayer/ieee802154/bitlevel/Ieee802154UwbIrMode.h"

namespace inet {

namespace physicallayer {

const double Ieee802154UwbIrMode::maxPulse = 1;
const double Ieee802154UwbIrMode::mandatory_pulse = 0.000000002003203125;

// Ci values
const short Ieee802154UwbIrMode::C31[8][31] = {
    // C1
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // C2
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // C3
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // C4
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // C5
    { -1, 0, +1, -1, 0, 0, +1, +1, +1, -1, +1, 0, 0, 0, -1, +1, 0, +1, +1, +1, 0, -1, 0, +1, 0, 0, 0, 0, -1, 0, 0 },
    // C6
    { +1, +1, 0, 0, +1, 0, 0, -1, -1, -1, +1, -1, 0, +1, +1, -1, 0, 0, 0, +1, 0, +1, 0, -1, +1, 0, +1, 0, 0, 0, 0 },
    // C7
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    // C8
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0, 0, 0, 0, 0 },
};

const short Ieee802154UwbIrMode::shortSFD[8] = { 0, 1, 0, -1, 1, 0, 0, -1 };

short Ieee802154UwbIrMode::s_array[maxS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 };
int Ieee802154UwbIrMode::last_s = 15;

const Ieee802154UwbIrMode Ieee802154UwbIrMode::cfg_mandatory_4M = {
    3,                  // channel
    NOMINAL_4_M,        // PRF
    NON_RANGING,        // Frame type
    PSR_DEFAULT,        // preamble length (number of preamble symbols)
    31,                 // Spreading code length
    64,                 // spreading delta L
    4,                  // chips per burst
    bps(850000),        // bit rate (bps)
    4,                  // pulses per data burst
    3974.36E-9,         // preamble symbol duration
    1023.64E-9,         // data symbol duration
    512.82E-9,          // burst time shift duration
    2.003E-9,           // pulse duration (chip)
    8.01E-9,            // burst duration (pulses per data burst * chip)
    286.2E-6,           // synchronization preamble duration
    MHz(4492.8),        // center frequency
    MHz(499.2)          // bandwidth
};

const Ieee802154UwbIrMode Ieee802154UwbIrMode::cfg_mandatory_16M = {
    3,					// channel
    NOMINAL_16_M, 		// PRF
    NON_RANGING, 		// Frame type
    PSR_DEFAULT,		// preamble length (number of preamble symbols)
    31,					// Spreading code length
    16,					// spreading delta L
    16,					// chips per burst
    bps(850000),        // bit rate (bps)
    16,					// pulses per data burst
    993.6E-9,			// preamble symbol duration
    1023.64E-9,			// data symbol duration
    512.82E-9,			// burst time shift duration
    2.003E-9,			// pulse duration (chip)
    32.05E-9,			// burst duration (pulses per data burst * chip)
    71.5E-6,			// synchronization preamble duration
    MHz(4492.8),	    // center frequency
    MHz(499.2)          // bandwidth
};

// TODO: do this in a static initializer once (even better: simply create a function from symbol index to hopping position)
int Ieee802154UwbIrMode::s(int n) const
{
    ASSERT(n < maxS);
    for (; last_s < n; last_s = last_s + 1) {
        // compute missing values as necessary
        s_array[last_s] = (s_array[last_s - 14] + s_array[last_s - 15]) % 2;
    }
    ASSERT(s_array[n] == 0 || s_array[n] == 1);
    return s_array[n];
}

int Ieee802154UwbIrMode::getHoppingPos(int sym) const
{
    //int m = 3;  // or 5 with 4M
    int pos = 0;
    int kNcpb = sym * Ncpb;
    switch (prf) {
        case NOMINAL_4_M:
            pos = s(kNcpb) + 2 * s(1 + kNcpb) + 4 * s(2 + kNcpb) + 8 * s(3 + kNcpb) + 16 * s(4 + kNcpb);
            break;
        case NOMINAL_16_M:
            pos = s(kNcpb) + 2 * s(1 + kNcpb) + 4 * s(2 + kNcpb);
            break;
        case NOMINAL_64_M:
            pos = s(kNcpb);
            break;
        case PRF_OFF:
        default:
            ASSERT(0==1);  // unimplemented or invalid PRF value
            break;
    }
    // ASSERT(pos > -1 && pos < 8); // TODO: update to reflect number of hopping pos for current config
    return pos;
}

} // namespace physicallayer

} // namespace inet

