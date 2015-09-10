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

bool NetworkLSA::update(const OSPFNetworkLSA *lsa)
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

bool NetworkLSA::differsFrom(const OSPFNetworkLSA *networkLSA) const
{
    const OSPFLSAHeader& thisHeader = getHeader();
    const OSPFLSAHeader& lsaHeader = networkLSA->getHeader();
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

} // namespace ospf

} // namespace inet

