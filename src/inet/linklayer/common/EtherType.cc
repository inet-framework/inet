//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/common/EtherType_m.h"

#include "inet/common/ProtocolGroup.h"

namespace inet {

std::string etherTypeToString(int etherType)
{
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "0x%04x", etherType);
    auto protocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(etherType);
    return protocol == nullptr ? std::string(buffer) : std::string(buffer) + " (" + protocol->getDescriptiveName() + ")";
}

std::string typeOrLengthToString(int typeOrLength)
{
    if (isEth2Type(typeOrLength))
        return etherTypeToString(typeOrLength);
    else
        return std::to_string(typeOrLength) + " (length)";
}

} // namespace inet
