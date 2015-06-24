//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "Ieee80211MacMacmib.h"

namespace inet {
namespace ieee80211 {

Define_Module(Ieee80211MacMacmibPackage);

void Ieee80211MacMacmibPackage::handleMessage(cMessage* msg)
{
    throw cRuntimeError("This module doesn't handle self message");
}

void Ieee80211MacMacmibPackage::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        countersTable = new Ieee80211MacMacmibCountersTable();
        operationTable = new Ieee80211MacMacmibOperationTable();
        stationConfigTable = new Ieee80211MacMacmibStationConfigTable();
        phyOperationTable = new Ieee80211MacMacmibPhyOperationTable();
    }
}

Ieee80211MacMacmibPackage::~Ieee80211MacMacmibPackage()
{
    delete countersTable;
    delete operationTable;
    delete stationConfigTable;
    delete phyOperationTable;
}

} /* namespace inet */
} /* namespace ieee80211 */

