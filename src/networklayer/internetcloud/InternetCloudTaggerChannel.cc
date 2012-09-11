//
// Copyright (C) 2012 OpenSim Ltd.
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

#include "InternetCloudTaggerChannel.h"

#include "InterfaceEntry.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"

Define_Channel(InternetCloudTaggerChannel);

void InternetCloudTaggerChannel::initialize(int stage)
{
    if (stage == 1)
    {
        IInterfaceTable *ift = InterfaceTableAccess().get(getParentModule());
        ie = ift->getInterfaceByNetworkLayerGateIndex(getSourceGate()->getPathEndGate()->getIndex());
    }
}

void InternetCloudTaggerChannel::processMessage(cMessage *msg, simtime_t t, result_t& result)
{
    if (ie)
    {
        int interfaceId = ie->getInterfaceId();
        msg->addPar("incomingInterfaceID") = interfaceId;
    }
    cIdealChannel::processMessage(msg, t, result);
}
