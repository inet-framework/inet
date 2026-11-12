//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

namespace inet::cellular;

//
// Describes the allocation that a MAC asks a ~CellularTransmitter to send the packet in: a set
// of PRBs over a number of OFDM symbols of the carrier, starting when the radio gets the packet.
//
class AllocationReq extends TagBase
{
    int prbs[];                  // Indices of the allocated PRBs, in increasing order
    int numSymbols = 0;          // Number of OFDM symbols of the allocation
    W powerPerPrb = W(NaN);      // Transmission power of each allocated PRB
    int beam = -1;               // Beam of the transmitter's panel antenna, -1 means no beam
    int intendedReceiverId = -1; // Radio id (IRadio::getId()) of the receiver of the allocation
    int mcs = 0;                 // Modulation and coding scheme index, see McsTable
}
