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

bool RouterLsa::update(const Ospfv2RouterLsa *lsa)
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

bool RouterLsa::differsFrom(const Ospfv2RouterLsa *routerLSA) const
{
    const Ospfv2LsaHeader& thisHeader = getHeader();
    const Ospfv2LsaHeader& lsaHeader = routerLSA->getHeader();
    bool differentHeader = ((thisHeader.getLsOptions() != lsaHeader.getLsOptions()) ||
                            ((thisHeader.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE)) ||
                            ((thisHeader.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE)) ||
                            (thisHeader.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody = false;

    if (!differentHeader) {
        differentBody = ((getV_VirtualLinkEndpoint() != routerLSA->getV_VirtualLinkEndpoint()) ||
                         (getE_ASBoundaryRouter() != routerLSA->getE_ASBoundaryRouter()) ||
                         (getB_AreaBorderRouter() != routerLSA->getB_AreaBorderRouter()) ||
                         (getNumberOfLinks() != routerLSA->getNumberOfLinks()) ||
                         (getLinksArraySize() != routerLSA->getLinksArraySize()));

        if (!differentBody) {
            unsigned int linkCount = links_arraysize;
            for (unsigned int i = 0; i < linkCount; i++) {
                auto thisLink = getLinks(i);
                auto lsaLink = routerLSA->getLinks(i);
                bool differentLink = ((thisLink.getLinkID() != lsaLink.getLinkID()) ||
                                      (thisLink.getLinkData() != lsaLink.getLinkData()) ||
                                      (thisLink.getType() != lsaLink.getType()) ||
                                      (thisLink.getNumberOfTOS() != lsaLink.getNumberOfTOS()) ||
                                      (thisLink.getLinkCost() != lsaLink.getLinkCost()) ||
                                      (thisLink.getTosDataArraySize() != lsaLink.getTosDataArraySize()));

                if (!differentLink) {
                    unsigned int tosCount = thisLink.getTosDataArraySize();
                    for (unsigned int j = 0; j < tosCount; j++) {
                        bool differentTOS = ((thisLink.getTosData(j).tos != lsaLink.getTosData(j).tos) ||
                                             (thisLink.getTosData(j).tosMetric != lsaLink.getTosData(j).tosMetric));

                        if (differentTOS) {
                            differentLink = true;
                            break;
                        }
                    }
                }

                if (differentLink) {
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

