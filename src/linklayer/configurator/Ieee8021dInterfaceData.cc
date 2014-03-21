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

#include "Ieee8021dInterfaceData.h"

Ieee8021dInterfaceData::Ieee8021dInterfaceData()
{
    // If there is no STP module then all ports
    // must be in forwarding state.
    portData.role = NOTASSIGNED;
    portData.state = FORWARDING;
}

std::string Ieee8021dInterfaceData::info() const
{
    std::stringstream out;
    out << "role:" << getRoleName() << " state:" << getStateName();
    return out.str();
}

std::string Ieee8021dInterfaceData::detailedInfo() const
{
    std::stringstream out;
    out << "role:" << getRoleName() << "\tstate:" << getStateName() << "\n";
    out << "priority:" << getPriority() << "\n";
    out << "linkCost:" << getLinkCost() << "\n";

    return out.str();
}

const char *Ieee8021dInterfaceData::getRoleName(PortRole role)
{
    switch (role)
    {
        case ALTERNATE: return "ALTERNATE";
        case NOTASSIGNED: return "NOTASSIGNED";
        case DISABLED: return "DISABLED";
        case DESIGNATED: return "DESIGNATED";
        case BACKUP: return "BACKUP";
        case ROOT: return "ROOT";
        default: throw cRuntimeError("Unknown port role %d", role);
    }
}

const char *Ieee8021dInterfaceData::getStateName(PortState state)
{
    switch (state)
    {
        case DISCARDING: return "DISCARDING";
        case LEARNING: return "LEARNING";
        case FORWARDING: return "FORWARDING";
        default: throw cRuntimeError("Unknown port state %d", state);
    }
}
