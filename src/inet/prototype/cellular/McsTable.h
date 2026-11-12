//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MCSTABLE_H
#define __INET_MCSTABLE_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace cellular {

/**
 * A dummy modulation and coding scheme table, shared by the link adaptation of the MACs and by
 * the error model. MCS index i needs an effective SNIR of -6 dB + i dB for a 50% block error
 * rate, and carries 0.15 + 0.2 * i bits per resource element.
 */
class INET_API McsTable
{
  public:
    static constexpr int MAX_MCS = 27;

    static double getRequiredSnir(int mcs) { return -6 + mcs; }
    static double getSpectralEfficiency(int mcs) { return 0.15 + 0.2 * mcs; }

    // the highest MCS whose required SNIR is at most snir minus the margin (both in dB)
    static int selectMcs(double snir, double margin) {
        int mcs = (int)std::floor(snir - margin + 6);
        return std::max(0, std::min(MAX_MCS, mcs));
    }
};

} // namespace cellular

} // namespace inet

#endif

