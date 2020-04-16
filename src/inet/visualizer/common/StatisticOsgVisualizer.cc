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
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/common/StatisticOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticOsgVisualizer);

#ifdef WITH_OSG

StatisticOsgVisualizer::StatisticOsgVisualization::StatisticOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int moduleId, simsignal_t signal, const char *unit) :
    StatisticVisualization(moduleId, signal, unit),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

StatisticOsgVisualizer::~StatisticOsgVisualizer()
{
    if (displayStatistics)
        removeAllStatisticVisualizations();
}

void StatisticOsgVisualizer::initialize(int stage)
{
    StatisticVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

StatisticVisualizerBase::StatisticVisualization *StatisticOsgVisualizer::createStatisticVisualization(cComponent *source, simsignal_t signal)
{
    auto label = new osgText::Text();
    label->setCharacterSize(18);
    label->setBoundingBoxColor(osg::Vec4(backgroundColor.red / 255.0, backgroundColor.green / 255.0, backgroundColor.blue / 255.0, 0.5));
    label->setColor(osg::Vec4(textColor.red / 255.0, textColor.green / 255.0, textColor.blue / 255.0, 0.5));
    label->setAlignment(osgText::Text::CENTER_BOTTOM);
    label->setText("");
    label->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
    label->setPosition(osg::Vec3(0.0, 0.0, 0.0));
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->addDrawable(label);
    auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create statistic visualization for '%s', because network node visualization is not found for '%s'", source->getFullPath().c_str(), networkNode->getFullPath().c_str());
    return new StatisticOsgVisualization(networkNodeVisualization, geode, source->getId(), signal, getUnit(source));
}

void StatisticOsgVisualizer::addStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::addStatisticVisualization(statisticVisualization);
    auto statisticOsgVisualization = static_cast<const StatisticOsgVisualization *>(statisticVisualization);
    statisticOsgVisualization->networkNodeVisualization->addAnnotation(statisticOsgVisualization->node, osg::Vec3d(100, 18, 0), 1.0);
}

void StatisticOsgVisualizer::removeStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::removeStatisticVisualization(statisticVisualization);
    auto statisticOsgVisualization = static_cast<const StatisticOsgVisualization *>(statisticVisualization);
    statisticOsgVisualization->networkNodeVisualization->removeAnnotation(statisticOsgVisualization->node);
}

void StatisticOsgVisualizer::refreshStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::refreshStatisticVisualization(statisticVisualization);
    auto statisticOsgVisualization = static_cast<const StatisticOsgVisualization *>(statisticVisualization);
    auto geode = check_and_cast<osg::Geode *>(statisticOsgVisualization->node);
    auto label = check_and_cast<osgText::Text *>(geode->getDrawable(0));
    label->setText(getText(statisticVisualization));
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

