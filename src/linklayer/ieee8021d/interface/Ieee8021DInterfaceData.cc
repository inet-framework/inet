//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Benjamin Martin Seregi
//

#include "Ieee8021DInterfaceData.h"

Ieee8021DInterfaceData::Ieee8021DInterfaceData()
{
    // Default port data for STP
    defaultStpPort.state = DISCARDING;
    defaultStpPort.role = NOTASSIGNED;
    defaultStpPort.priority = 128;
    defaultStpPort.rootPathCost = INT16_MAX;
    defaultStpPort.rootPriority = 65536;
    defaultStpPort.rootAddress = MACAddress("FF-FF-FF-FF-FF-FF");
    defaultStpPort.bridgePriority = 65536;
    defaultStpPort.bridgeAddress = MACAddress("FF-FF-FF-FF-FF-FF");
    defaultStpPort.portPriority = 256;
    defaultStpPort.portNum = 256;
    defaultStpPort.age = 0;
    defaultStpPort.fdWhile = 0;
    defaultStpPort.maxAge = 20;
    defaultStpPort.fwdDelay = 15;
    defaultStpPort.helloTime = 2;
    defaultStpPort.linkCost = 19;     // todo: set cost according to the bandwidth
    defaultStpPort.edge = false;
    portData = defaultStpPort;

    // If there is no STP module then all ports
    // must be in forwarding state.
    portData.state = FORWARDING;
}
