//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/node/base/NodeBase.h"

#include "inet/common/SubmoduleLayout.h"

namespace inet {

Define_Module(NodeBase);

void NodeBase::initialize(int stage)
{
    if (stage == INITSTAGE_LAST)
        layoutSubmodulesWithoutGates(this);
}

} // namespace inet

