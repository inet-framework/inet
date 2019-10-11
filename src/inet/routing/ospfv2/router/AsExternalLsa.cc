//
// Copyright (C) 2006 Andras Babos and Andras Varga
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

                if (
                        (thisTOSInfo.E_ExternalMetricType != lsaTOSInfo.E_ExternalMetricType) ||
                        (thisTOSInfo.routeCost != lsaTOSInfo.routeCost) ||
                        (thisTOSInfo.forwardingAddress != lsaTOSInfo.forwardingAddress) ||
                        (thisTOSInfo.externalRouteTag != lsaTOSInfo.externalRouteTag) ||
                        (thisTOSInfo.tos != lsaTOSInfo.tos)
                        ) {
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

