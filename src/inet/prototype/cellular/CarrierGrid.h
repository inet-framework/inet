//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CARRIERGRID_H
#define __INET_CARRIERGRID_H

#include "inet/common/INETDefs.h"
#include "inet/common/Units.h"

namespace inet {

namespace cellular {

using namespace inet::units::values;

/**
 * The resource grid of one component carrier: PRBs of 12 subcarriers in frequency and slots of
 * 14 OFDM symbols in time, both scaled by the numerology (the subcarrier spacing).
 */
class INET_API CarrierGrid
{
  public:
    static constexpr int SUBCARRIERS_PER_PRB = 12;
    static constexpr int SYMBOLS_PER_SLOT = 14;

  protected:
    Hz centerFrequency = Hz(NaN);
    Hz subcarrierSpacing = Hz(NaN);
    int numPrbs = 0;

  public:
    CarrierGrid() {}
    CarrierGrid(Hz centerFrequency, Hz subcarrierSpacing, int numPrbs) :
        centerFrequency(centerFrequency), subcarrierSpacing(subcarrierSpacing), numPrbs(numPrbs) {}

    Hz getCenterFrequency() const { return centerFrequency; }
    Hz getSubcarrierSpacing() const { return subcarrierSpacing; }
    int getNumPrbs() const { return numPrbs; }

    Hz getPrbWidth() const { return subcarrierSpacing * SUBCARRIERS_PER_PRB; }
    Hz getBandwidth() const { return getPrbWidth() * numPrbs; }
    Hz getLowerEdge() const { return centerFrequency - getBandwidth() / 2; }

    // 1 ms for 15 kHz, halved with each doubling of the subcarrier spacing
    simtime_t getSlotDuration() const { return SimTime(1E-3 * 15E+3 / subcarrierSpacing.get()); }
    simtime_t getSymbolDuration() const { return getSlotDuration() / SYMBOLS_PER_SLOT; }
};

} // namespace cellular

} // namespace inet

#endif

