//
// Copyright (C) 2016 OpenSim Ltd.
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
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/networknode/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/statistic/StatisticOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticOsgVisualizer);

#ifdef WITH_OSG

StatisticOsgVisualizer::OsgCacheEntry::OsgCacheEntry(const char *unit, NetworkNodeOsgVisualization *visualization, osg::Node *node) :
    CacheEntry(unit),
    visualization(visualization),
    node(node)
{
}

StatisticVisualizerBase::CacheEntry *StatisticOsgVisualizer::createCacheEntry(cComponent *source, simsignal_t signal)
{
    auto label = new osgText::Text();
    label->setCharacterSize(18);
    label->setBoundingBoxColor(osg::Vec4(color.red / 255.0, color.green / 255.0, color.blue / 255.0, 0.5));
    label->setColor(osg::Vec4(0.0, 0.0, 0.0, 1.0));
    label->setAlignment(osgText::Text::CENTER_BOTTOM);
    label->setText("");
    label->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
    label->setPosition(osg::Vec3(0.0, 0.0, 0.0));
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->addDrawable(label);
    auto networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
    auto visualization = networkNodeVisualizer->getNeworkNodeVisualization(networkNode);
    return new OsgCacheEntry(getUnit(source), visualization, geode);
}

void StatisticOsgVisualizer::addCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry)
{
    StatisticVisualizerBase::addCacheEntry(moduleAndSignal, cacheEntry);
    auto osgCacheEntry = static_cast<OsgCacheEntry *>(cacheEntry);
    osgCacheEntry->visualization->addAnnotation(osgCacheEntry->node, osg::Vec3d(100, 18, 0), 1.0);
}

void StatisticOsgVisualizer::removeCacheEntry(std::pair<int, int> moduleAndSignal, CacheEntry *cacheEntry)
{
    StatisticVisualizerBase::removeCacheEntry(moduleAndSignal, cacheEntry);
    auto osgCacheEntry = static_cast<OsgCacheEntry *>(cacheEntry);
    osgCacheEntry->visualization->removeAnnotation(osgCacheEntry->node);
}

void StatisticOsgVisualizer::refreshStatistic(CacheEntry *cacheEntry)
{
    auto osgCacheEntry = static_cast<OsgCacheEntry *>(cacheEntry);
    auto geode = check_and_cast<osg::Geode *>(osgCacheEntry->node);
    auto label = check_and_cast<osgText::Text *>(geode->getDrawable(0));
    label->setText(getText(cacheEntry));
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

