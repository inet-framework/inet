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

#include "LSA.h"

bool OSPF::ASExternalLSA::Update(const OSPFASExternalLSA* lsa)
{
    bool different = DiffersFrom(lsa);
    (*this) = (*lsa);
    ResetInstallTime();
    if (different) {
        ClearNextHops();
        return true;
    } else {
        return false;
    }
}

bool OSPF::ASExternalLSA::DiffersFrom(const OSPFASExternalLSA* asExternalLSA) const
{
    const OSPFLSAHeader& lsaHeader = asExternalLSA->getHeader();
    bool differentHeader = ((header_var.getLsOptions() != lsaHeader.getLsOptions()) ||
                            ((header_var.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE)) ||
                            ((header_var.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE)) ||
                            (header_var.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody   = false;

    if (!differentHeader) {
        const OSPFASExternalLSAContents& lsaContents  = asExternalLSA->getContents();
        unsigned int                     tosInfoCount = contents_var.getExternalTOSInfoArraySize();

        differentBody = ((contents_var.getNetworkMask() != lsaContents.getNetworkMask()) ||
                         (contents_var.getE_ExternalMetricType() != lsaContents.getE_ExternalMetricType()) ||
                         (contents_var.getRouteCost() != lsaContents.getRouteCost()) ||
                         (contents_var.getForwardingAddress() != lsaContents.getForwardingAddress()) ||
                         (contents_var.getExternalRouteTag() != lsaContents.getExternalRouteTag()) ||
                         (tosInfoCount != lsaContents.getExternalTOSInfoArraySize()));

        if (!differentBody) {
            for (unsigned int i = 0; i < tosInfoCount; i++) {
                const ExternalTOSInfo& currentTOSInfo = contents_var.getExternalTOSInfo(i);
                const ExternalTOSInfo& lsaTOSInfo     = lsaContents.getExternalTOSInfo(i);

                if ((currentTOSInfo.tosData.tos != lsaTOSInfo.tosData.tos) ||
                    (currentTOSInfo.tosData.tosMetric[0] != lsaTOSInfo.tosData.tosMetric[0]) ||
                    (currentTOSInfo.tosData.tosMetric[1] != lsaTOSInfo.tosData.tosMetric[1]) ||
                    (currentTOSInfo.tosData.tosMetric[2] != lsaTOSInfo.tosData.tosMetric[2]) ||
                    (currentTOSInfo.E_ExternalMetricType != lsaTOSInfo.E_ExternalMetricType) ||
                    (currentTOSInfo.forwardingAddress != lsaTOSInfo.forwardingAddress) ||
                    (currentTOSInfo.externalRouteTag != lsaTOSInfo.externalRouteTag))
                {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return (differentHeader || differentBody);
}
