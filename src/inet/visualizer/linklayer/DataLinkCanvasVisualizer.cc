//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/linklayer/base/MacProtocolBase.h"
#include "inet/visualizer/linklayer/DataLinkCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(DataLinkCanvasVisualizer);

bool DataLinkCanvasVisualizer::isLinkStart(cModule *module) const
{
    return dynamic_cast<MacProtocolBase *>(module) != nullptr;
}

bool DataLinkCanvasVisualizer::isLinkEnd(cModule *module) const
{
    return dynamic_cast<MacProtocolBase *>(module) != nullptr;
}

const LinkVisualizerBase::LinkVisualization *DataLinkCanvasVisualizer::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto linkVisualization = static_cast<const LinkCanvasVisualization *>(LinkCanvasVisualizerBase::createLinkVisualization(source, destination, packet));
    linkVisualization->figure->setTags((std::string("data_link ") + tags).c_str());
    linkVisualization->figure->setTooltip("This arrow represents a data link between two network nodes");
    linkVisualization->shiftPriority = 2;
    return linkVisualization;
}

} // namespace visualizer

} // namespace inet

