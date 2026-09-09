//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/physicallayer/PhysicalLinkVsgVisualizer.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace visualizer {

Define_Module(PhysicalLinkVsgVisualizer);

bool PhysicalLinkVsgVisualizer::isLinkStart(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr;
}

bool PhysicalLinkVsgVisualizer::isLinkEnd(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr;
}

} // namespace visualizer

} // namespace inet
