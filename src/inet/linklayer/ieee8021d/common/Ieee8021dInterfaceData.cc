//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"

namespace inet {

Ieee8021dInterfaceData::PortInfo::PortInfo()
{
    priority = 0;
    linkCost = 1;
    edge = false;

    // If there is no STP module then all ports
    // must be in forwarding state.
    state = FORWARDING;
    role = NOTASSIGNED;

    rootPriority = 0;
    rootPathCost = 0;
    bridgePriority = 0;
    portPriority = 0;
    portNum = -1;

    lostBPDU = 0;
}

Ieee8021dInterfaceData::Ieee8021dInterfaceData()
    : InterfaceProtocolData(NetworkInterface::F_IEEE8021D_DATA)
{
}

std::string Ieee8021dInterfaceData::str() const
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
    switch (role) {
        case ALTERNATE:
            return "ALTERNATE";

        case NOTASSIGNED:
            return "NOTASSIGNED";

        case DISABLED:
            return "DISABLED";

        case DESIGNATED:
            return "DESIGNATED";

        case BACKUP:
            return "BACKUP";

        case ROOT:
            return "ROOT";

        default:
            throw cRuntimeError("Unknown port role %d", role);
    }
}

const char *Ieee8021dInterfaceData::getStateName(PortState state)
{
    switch (state) {
        case DISCARDING:
            return "DISCARDING";

        case LEARNING:
            return "LEARNING";

        case FORWARDING:
            return "FORWARDING";

        default:
            throw cRuntimeError("Unknown port state %d", state);
    }
}

} // namespace inet

