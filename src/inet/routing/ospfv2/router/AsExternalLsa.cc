//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/routing/ospfv2/router/Lsa.h"

namespace inet {
namespace ospfv2 {

bool AsExternalLsa::update(const Ospfv2AsExternalLsa *lsa)
{
    bool different = differsFrom(lsa);
    (*this) = (*lsa);
    resetInstallTime();
    if (different) {
        clearNextHops();
    }
    return different;
}

bool AsExternalLsa::differsFrom(const Ospfv2AsExternalLsa *asExternalLSA) const
{
    const Ospfv2LsaHeader& thisHeader = getHeader();
    const Ospfv2LsaHeader& lsaHeader = asExternalLSA->getHeader();
    bool differentHeader = ((thisHeader.getLsOptions() != lsaHeader.getLsOptions()) ||
                            ((thisHeader.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE)) ||
                            ((thisHeader.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        const Ospfv2AsExternalLsaContents& thisContents = getContents();
        const Ospfv2AsExternalLsaContents& lsaContents = asExternalLSA->getContents();

        unsigned int thisTosInfoCount = thisContents.getExternalTOSInfoArraySize();

        differentBody = ((thisContents.getNetworkMask() != lsaContents.getNetworkMask()) ||
                         (thisTosInfoCount != lsaContents.getExternalTOSInfoArraySize()));

        if (!differentBody) {
            for (unsigned int i = 0; i < thisTosInfoCount; i++) {
                const auto& thisTOSInfo = thisContents.getExternalTOSInfo(i);
                const auto& lsaTOSInfo = lsaContents.getExternalTOSInfo(i);

                if ((thisTOSInfo.E_ExternalMetricType != lsaTOSInfo.E_ExternalMetricType) ||
                    (thisTOSInfo.routeCost != lsaTOSInfo.routeCost) ||
                    (thisTOSInfo.forwardingAddress != lsaTOSInfo.forwardingAddress) ||
                    (thisTOSInfo.externalRouteTag != lsaTOSInfo.externalRouteTag) ||
                    (thisTOSInfo.tos != lsaTOSInfo.tos))
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

