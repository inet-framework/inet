//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/node/base/NodeBase.h"

#include "inet/common/SubmoduleLayout.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/IRoutingTable.h"
#include "inet/networks/contract/INetwork.h"

namespace inet {

Define_Module(NodeBase);

void NodeBase::initialize(int stage)
{
    if (stage == INITSTAGE_LAST)
        layoutSubmodulesWithoutGates(this);
}

INetwork *NodeBase::getNetwork() const
{
    return check_and_cast<INetwork *>(getParentModule());
}

IInterfaceTable *NodeBase::getInterfaceTable() const
{
    return check_and_cast<IInterfaceTable *>(getSubmodule("interfaceTable"));
}

IRoutingTable *NodeBase::getRoutingTable() const
{
    return check_and_cast<IRoutingTable *>(getModuleByPath("ipv4.routingTable"));
}

void NodeBase::startup() const
{
}

void NodeBase::shutdown() const
{
}

void NodeBase::crash() const
{
}

} // namespace inet

