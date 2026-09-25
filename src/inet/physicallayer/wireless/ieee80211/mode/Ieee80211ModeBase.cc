//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeBase.h"

namespace inet {
namespace physicallayer {

bps Ieee80211ModeBase::computeNonHtReferenceRate(unsigned int constellationSize, double codeRate)
{
    // IEEE Std 802.11-2024, Table 10-10. Width, stream count, and GI do not enter this mapping.
    switch (constellationSize) {
        case 2:
            if (codeRate == 1.0 / 2) return Mbps(6);
            if (codeRate == 3.0 / 4) return Mbps(9);
            break;
        case 4:
            if (codeRate == 1.0 / 2) return Mbps(12);
            if (codeRate == 3.0 / 4) return Mbps(18);
            break;
        case 16:
            if (codeRate == 1.0 / 2) return Mbps(24);
            if (codeRate == 3.0 / 4) return Mbps(36);
            break;
        case 64:
            if (codeRate == 1.0 / 2 || codeRate == 2.0 / 3) return Mbps(48);
            if (codeRate == 3.0 / 4 || codeRate == 5.0 / 6) return Mbps(54);
            break;
        case 256:
        case 1024:
            if (codeRate == 3.0 / 4 || codeRate == 5.0 / 6) return Mbps(54);
            break;
    }
    return bps(NaN);
}

} /* namespace physicallayer */
} /* namespace inet */
