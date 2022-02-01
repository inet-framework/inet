//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/physicallayer/PhysicalLinkOsgVisualizer.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace visualizer {

Define_Module(PhysicalLinkOsgVisualizer);

bool PhysicalLinkOsgVisualizer::isLinkStart(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr;
}

bool PhysicalLinkOsgVisualizer::isLinkEnd(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr;
}

} // namespace visualizer

} // namespace inet

