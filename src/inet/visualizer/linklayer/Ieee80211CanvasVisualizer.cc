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

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/linklayer/Ieee80211CanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(Ieee80211CanvasVisualizer);

Ieee80211CanvasVisualizer::Ieee80211CanvasVisualization::Ieee80211CanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, LabeledIconFigure *figure, int networkNodeId, int interfaceId) :
    Ieee80211Visualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

void Ieee80211CanvasVisualizer::initialize(int stage)
{
    Ieee80211VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211CanvasVisualizer::createIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry, std::string ssid)
{
    std::hash<std::string> hasher;
    std::string icon(this->icon);
    auto labeledIconFigure = new LabeledIconFigure("ieee80211Association");
    labeledIconFigure->setTags((std::string("ieee80211_association ") + tags).c_str());
    labeledIconFigure->setAssociatedObject(interfaceEntry);
    labeledIconFigure->setZIndex(zIndex);
    auto iconFigure = labeledIconFigure->getIconFigure();
    iconFigure->setTooltip("This icon represents an IEEE 802.11 association");
    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
    iconFigure->setTintColor(iconColorSet.getColor(hasher(ssid)));
    iconFigure->setTintAmount(1);
    auto labelFigure = labeledIconFigure->getLabelFigure();
    labelFigure->setTooltip("This label represents the SSID of an IEEE 802.11 association");
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    labelFigure->setText(ssid.c_str());
    labelFigure->setPosition(iconFigure->getBounds().getSize() / 2);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new Ieee80211CanvasVisualization(networkNodeVisualization, labeledIconFigure, networkNode->getId(), interfaceEntry->getInterfaceId());
}

void Ieee80211CanvasVisualizer::addIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::addIeee80211Visualization(ieee80211Visualization);
    auto ieee80211CanvasVisualization = static_cast<const Ieee80211CanvasVisualization *>(ieee80211Visualization);
    ieee80211CanvasVisualization->networkNodeVisualization->addAnnotation(ieee80211CanvasVisualization->figure, ieee80211CanvasVisualization->figure->getBounds().getSize(), placementHint, placementPriority);
}

void Ieee80211CanvasVisualizer::removeIeee80211Visualization(const Ieee80211Visualization *ieee80211Visualization)
{
    Ieee80211VisualizerBase::removeIeee80211Visualization(ieee80211Visualization);
    auto ieee80211CanvasVisualization = static_cast<const Ieee80211CanvasVisualization *>(ieee80211Visualization);
    ieee80211CanvasVisualization->networkNodeVisualization->removeAnnotation(ieee80211CanvasVisualization->figure);
}

} // namespace visualizer

} // namespace inet

