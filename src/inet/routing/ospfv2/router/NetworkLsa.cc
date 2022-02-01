//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/router/Lsa.h"

namespace inet {

namespace ospfv2 {

bool NetworkLsa::update(const Ospfv2NetworkLsa *lsa)
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

bool NetworkLsa::differsFrom(const Ospfv2NetworkLsa *networkLSA) const
{
    const Ospfv2LsaHeader& thisHeader = getHeader();
    const Ospfv2LsaHeader& lsaHeader = networkLSA->getHeader();
    bool differentHeader = ((thisHeader.getLsOptions() != lsaHeader.getLsOptions()) ||
                            ((thisHeader.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE)) ||
                            ((thisHeader.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((getNetworkMask() != networkLSA->getNetworkMask()) ||
                         (getAttachedRoutersArraySize() != networkLSA->getAttachedRoutersArraySize()));

        if (!differentBody) {
            unsigned int routerCount = attachedRouters_arraysize;
            for (unsigned int i = 0; i < routerCount; i++) {
                if (getAttachedRouters(i) != networkLSA->getAttachedRouters(i)) {
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

