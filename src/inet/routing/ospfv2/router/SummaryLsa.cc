//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/router/Lsa.h"

namespace inet {

namespace ospfv2 {

bool SummaryLsa::update(const Ospfv2SummaryLsa *lsa)
{
    bool different = differsFrom(lsa);
    (*this) = (*lsa);
    resetInstallTime();
    if (different) {
        clearNextHops();
        return true;
    }
    else {
        return false;
    }
}

bool SummaryLsa::differsFrom(const Ospfv2SummaryLsa *summaryLSA) const
{
    const Ospfv2LsaHeader& thisHeader = getHeader();
    const Ospfv2LsaHeader& lsaHeader = summaryLSA->getHeader();
    bool differentHeader = ((thisHeader.getLsOptions() != lsaHeader.getLsOptions()) ||
                            ((thisHeader.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE)) ||
                            ((thisHeader.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((getNetworkMask() != summaryLSA->getNetworkMask()) ||
                         (getRouteCost() != summaryLSA->getRouteCost()) ||
                         (getTosDataArraySize() != summaryLSA->getTosDataArraySize()));

        if (!differentBody) {
            unsigned int tosCount = tosData_arraysize;
            for (unsigned int i = 0; i < tosCount; i++) {
                auto thisTosData = getTosData(i);
                auto lsaTosData = summaryLSA->getTosData(i);
                if ((thisTosData.tos != summaryLSA->getTosData(i).tos) ||
                    (thisTosData.tosMetric != lsaTosData.tosMetric))
                {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}

} // namespace ospfv2

} // namespace inet

