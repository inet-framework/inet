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

#ifdef WITH_IEEE80211
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.h"
#endif

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/linklayer/Ieee80211CanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(Ieee80211CanvasVisualizer);

Ieee80211CanvasVisualizer::Ieee80211CanvasVisualization::~Ieee80211CanvasVisualization()
{
    delete figure;
}

Ieee80211CanvasVisualizer::Ieee80211CanvasVisualization::Ieee80211CanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, LabeledIconFigure *figure, int networkNodeId, int interfaceId) :
    Ieee80211Visualization(networkNodeId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

Ieee80211CanvasVisualizer::~Ieee80211CanvasVisualizer()
{
    if (displayAssociations)
        removeAllIeee80211Visualizations();
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

void Ieee80211CanvasVisualizer::refreshDisplay() const
{
#ifdef WITH_IEEE80211
    auto simulation = getSimulation();
    for (auto& entry : ieee80211Visualizations) {
        auto networkNode = simulation->getModule(entry.second->networkNodeId);
        if (networkNode != nullptr) {
            L3AddressResolver addressResolver;
            auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode);
            if (interfaceTable != nullptr) {
                auto networkInterface = interfaceTable->getInterfaceById(entry.second->interfaceId);
                auto mgmt = dynamic_cast<inet::ieee80211::Ieee80211MgmtSta *>(networkInterface->getSubmodule("mgmt"));
                if (mgmt != nullptr) {
                    auto apInfo = mgmt->getAssociatedAp();
                    std::string icon = getIcon(W(apInfo->rxPower));
                    auto canvasVisualization = check_and_cast<const Ieee80211CanvasVisualization *>(entry.second);
                    auto iconFigure = check_and_cast<LabeledIconFigure *>(canvasVisualization->figure)->getIconFigure();
                    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
                }
            }
        }
    }
#endif
}

Ieee80211VisualizerBase::Ieee80211Visualization *Ieee80211CanvasVisualizer::createIeee80211Visualization(cModule *networkNode, InterfaceEntry *interfaceEntry, std::string ssid, W power)
{
    std::string icon = getIcon(power);
    auto labeledIconFigure = new LabeledIconFigure("ieee80211Association");
    labeledIconFigure->setTags((std::string("ieee80211_association ") + tags).c_str());
    labeledIconFigure->setAssociatedObject(interfaceEntry);
    labeledIconFigure->setZIndex(zIndex);
    auto iconFigure = labeledIconFigure->getIconFigure();
    iconFigure->setTooltip("This icon represents an IEEE 802.11 association");
    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
    std::hash<std::string> hasher;
    iconFigure->setTintColor(iconColorSet.getColor(hasher(ssid)));
    iconFigure->setTintAmount(1);
    auto labelFigure = labeledIconFigure->getLabelFigure();
    labelFigure->setTooltip("This label represents the SSID of an IEEE 802.11 association");
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    labelFigure->setText(ssid.c_str());
    labelFigure->setPosition(iconFigure->getBounds().getSize() / 2);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create IEEE 802.11 visualization for '%s', because network node visualization is not found for '%s'", interfaceEntry->getInterfaceName(), networkNode->getFullPath().c_str());
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

