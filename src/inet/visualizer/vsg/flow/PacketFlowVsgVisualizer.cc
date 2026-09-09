//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/INETDefs.h"

#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/MacRelayUnit.h"
#endif

#ifdef INET_WITH_IEEE8021D
#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#endif

#include "inet/visualizer/vsg/flow/PacketFlowVsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(PacketFlowVsgVisualizer);

bool PacketFlowVsgVisualizer::isPathStart(cModule *module) const
{
    return true;
}

bool PacketFlowVsgVisualizer::isPathEnd(cModule *module) const
{
    return true;
}

bool PacketFlowVsgVisualizer::isPathElement(cModule *module) const
{
#ifdef INET_WITH_ETHERNET
    if (dynamic_cast<MacRelayUnit *>(module) != nullptr)
        return true;
#endif

#ifdef INET_WITH_IEEE8021D
    if (dynamic_cast<Ieee8021dRelay *>(module) != nullptr)
        return true;
#endif

    return false;
}

} // namespace visualizer

} // namespace inet
