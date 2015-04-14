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

bool SummaryLSA::update(const OSPFSummaryLSA *lsa)
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

bool SummaryLSA::differsFrom(const OSPFSummaryLSA *summaryLSA) const
{
    const OSPFLSAHeader& thisHeader = getHeader();
    const OSPFLSAHeader& lsaHeader = summaryLSA->getHeader();
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
                    (thisTosData.tosMetric[0] != lsaTosData.tosMetric[0]) ||
                    (thisTosData.tosMetric[1] != lsaTosData.tosMetric[1]) ||
                    (thisTosData.tosMetric[2] != lsaTosData.tosMetric[2]))
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

