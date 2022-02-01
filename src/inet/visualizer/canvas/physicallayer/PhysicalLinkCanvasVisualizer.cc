//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/physicallayer/PhysicalLinkCanvasVisualizer.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace visualizer {

Define_Module(PhysicalLinkCanvasVisualizer);

bool PhysicalLinkCanvasVisualizer::isLinkStart(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr;
}

bool PhysicalLinkCanvasVisualizer::isLinkEnd(cModule *module) const
{
    return dynamic_cast<inet::physicallayer::IRadio *>(module) != nullptr;
}

const LinkVisualizerBase::LinkVisualization *PhysicalLinkCanvasVisualizer::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto linkVisualization = static_cast<const LinkCanvasVisualization *>(LinkCanvasVisualizerBase::createLinkVisualization(source, destination, packet));
    linkVisualization->figure->setTags((std::string("physical_link ") + tags).c_str());
    linkVisualization->figure->setTooltip("This arrow represents a physical link between two network nodes");
    linkVisualization->shiftPriority = 1;
    return linkVisualization;
}

} // namespace visualizer

} // namespace inet

