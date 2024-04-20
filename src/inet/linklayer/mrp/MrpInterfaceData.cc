//
// Copyright (C) 2024 Daniel Zeitler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/mrp/MrpInterfaceData.h"

namespace inet {

Register_Enum(MrpInterfaceData::PortRole, (MrpInterfaceData::PRIMARY, MrpInterfaceData::SECONDARY, MrpInterfaceData::INTERCONNECTION, MrpInterfaceData::NOTASSIGNED));
Register_Enum(MrpInterfaceData::PortState, (MrpInterfaceData::BLOCKED, MrpInterfaceData::FORWARDING, MrpInterfaceData::DISABLED));

MrpInterfaceData::MrpInterfaceData()
    : InterfaceProtocolData(NetworkInterface::F_MRP_DATA)
{
}

std::string MrpInterfaceData::str() const
{
    std::stringstream out;
    out << "role:" << getRoleName() << " mode:" << getStateName();
    return out.str();
}

std::string MrpInterfaceData::detailedInfo() const
{
    std::stringstream out;
    out << "role:" << getRoleName() << "\tmode:" << getStateName() << "\n";

    return out.str();
}

const char *MrpInterfaceData::getRoleName(PortRole role)
{
    switch (role) {
        case NOTASSIGNED:
            return "NOTASSIGNED";

        case PRIMARY:
            return "PRIMARY";

        case SECONDARY:
            return "SECONDARY";

        case INTERCONNECTION:
            return "INTERCONNECTION";

        default:
            throw cRuntimeError("Unknown port role %d", role);
    }
}

const char *MrpInterfaceData::getStateName(PortState state)
{
    switch (state) {
        case BLOCKED:
            return "BLOCKED";

        case FORWARDING:
            return "FORWARDING";

        default:
            throw cRuntimeError("Unknown port state %d", state);
    }
}


} // namespace inet

