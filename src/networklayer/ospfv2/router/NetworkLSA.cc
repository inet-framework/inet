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

bool OSPF::NetworkLSA::Update(const OSPFNetworkLSA* lsa)
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

bool OSPF::NetworkLSA::DiffersFrom(const OSPFNetworkLSA* networkLSA) const
{
    const OSPFLSAHeader& lsaHeader = networkLSA->getHeader();
    bool differentHeader = ((header_var.getLsOptions() != lsaHeader.getLsOptions()) ||
                            ((header_var.getLsAge() == MAX_AGE) && (lsaHeader.getLsAge() != MAX_AGE)) ||
                            ((header_var.getLsAge() != MAX_AGE) && (lsaHeader.getLsAge() == MAX_AGE)) ||
                            (header_var.getLsaLength() != lsaHeader.getLsaLength()));
    bool differentBody   = false;

    if (!differentHeader) {
        differentBody = ((networkMask_var != networkLSA->getNetworkMask()) ||
                         (attachedRouters_arraysize != networkLSA->getAttachedRoutersArraySize()));

        if (!differentBody) {
            unsigned int routerCount = attachedRouters_arraysize;
            for (unsigned int i = 0; i < routerCount; i++) {
                if (attachedRouters_var[i] != networkLSA->getAttachedRouters(i)) {
                    differentBody = true;
                    break;
                }
            }
        }
    }

    return (differentHeader || differentBody);
}
