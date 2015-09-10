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

#include "inet/routing/ospfv2/router/LSA.h"

namespace inet {

namespace ospf {

bool ASExternalLSA::update(const OSPFASExternalLSA *lsa)
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

bool ASExternalLSA::differsFrom(const OSPFASExternalLSA *asExternalLSA) const
{
    const OSPFLSAHeader& thisHeader = getHeader();
    const OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader();
    bool differentHeader = ((thisHeader.getLsOptions() != lsaHeader.getLsOptions()) ||
                            ((thisHeader.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE)) ||
                            ((thisHeader.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        const OSPFASExternalLSAContents& thisContents = getContents();
        const OSPFASExternalLSAContents& lsaContents = asExternalLSA->getContents();

        unsigned int thisTosInfoCount = thisContents.getExternalTOSInfoArraySize();

        differentBody = ((thisContents.getNetworkMask() != lsaContents.getNetworkMask()) ||
                         (thisContents.getE_ExternalMetricType() != lsaContents.getE_ExternalMetricType()) ||
                         (thisContents.getRouteCost() != lsaContents.getRouteCost()) ||
                         (thisContents.getForwardingAddress() != lsaContents.getForwardingAddress()) ||
                         (thisContents.getExternalRouteTag() != lsaContents.getExternalRouteTag()) ||
                         (thisTosInfoCount != lsaContents.getExternalTOSInfoArraySize()));

        if (!differentBody) {
            for (unsigned int i = 0; i < thisTosInfoCount; i++) {
                const ExternalTOSInfo& thisTOSInfo = thisContents.getExternalTOSInfo(i);
                const ExternalTOSInfo& lsaTOSInfo = lsaContents.getExternalTOSInfo(i);

                if ((thisTOSInfo.tosData.tos != lsaTOSInfo.tosData.tos) ||
                    (thisTOSInfo.tosData.tosMetric[0] != lsaTOSInfo.tosData.tosMetric[0]) ||
                    (thisTOSInfo.tosData.tosMetric[1] != lsaTOSInfo.tosData.tosMetric[1]) ||
                    (thisTOSInfo.tosData.tosMetric[2] != lsaTOSInfo.tosData.tosMetric[2]) ||
                    (thisTOSInfo.E_ExternalMetricType != lsaTOSInfo.E_ExternalMetricType) ||
                    (thisTOSInfo.forwardingAddress != lsaTOSInfo.forwardingAddress) ||
                    (thisTOSInfo.externalRouteTag != lsaTOSInfo.externalRouteTag))
                {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return differentHeader || differentBody;
}

} // namespace ospf

} // namespace inet

