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
    // If there is no STP module then all ports
    // must be in forwarding state.
    portData.state = FORWARDING;
}

const char *Ieee8021DInterfaceData::getRoleName(){
    switch (portData.role)
    {
        case 0:
            return "ALTERNATE";
            break;
        case 1:
            return "NOTASSIGNED";
            break;
        case 2:
            return "DISABLED";
            break;
        case 3:
            return "DESIGNATED";
            break;
        case 4:
            return "BACKUP";
            break;
        case 5:
            return "ROOT";
            break;
    }
    return "";
}

const char *Ieee8021DInterfaceData::getStateName(){
    switch (portData.state)
    {
        case 0:
            return "DISCARDING";
            break;
        case 1:
            return "LEARNING";
            break;
        case 2:
            return "FORWARDING";
            break;
    }
    return "";
}
