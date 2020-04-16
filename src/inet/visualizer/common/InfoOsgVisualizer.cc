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
#include "inet/visualizer/common/InfoOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(InfoOsgVisualizer);

#ifdef WITH_OSG

InfoOsgVisualizer::InfoOsgVisualization::InfoOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, int moduleId) :
    InfoVisualization(moduleId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

InfoOsgVisualizer::~InfoOsgVisualizer()
{
    if (displayInfos)
        removeAllInfoVisualizations();
}

void InfoOsgVisualizer::initialize(int stage)
{
    InfoVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto infoVisualization : infoVisualizations) {
            auto infoOsgVisualization = static_cast<const InfoOsgVisualization *>(infoVisualization);
            auto node = infoOsgVisualization->node;
            infoOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

InfoVisualizerBase::InfoVisualization *InfoOsgVisualizer::createInfoVisualization(cModule *module) const
{
    auto text = new osgText::Text();
    text->setCharacterSize(18);
    text->setBoundingBoxColor(osg::Vec4(backgroundColor.red / 255.0, backgroundColor.green / 255.0, backgroundColor.blue / 255.0, 0.5));
    text->setColor(osg::Vec4(textColor.red / 255.0, textColor.green / 255.0, textColor.blue / 255.0, 1.0));
    text->setAlignment(osgText::Text::CENTER_BOTTOM);
    text->setText("");
    text->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
    text->setPosition(osg::Vec3(0.0, 0.0, 0.0));
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->addDrawable(text);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create info visualization for '%s', because network node visualization is not found for '%s'", module->getFullPath().c_str(), networkNode->getFullPath().c_str());
    return new InfoOsgVisualization(networkNodeVisualization, geode, module->getId());
}

void InfoOsgVisualizer::refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const
{
    auto infoOsgVisualization = static_cast<const InfoOsgVisualization *>(infoVisualization);
    auto node = infoOsgVisualization->node;
    auto text = static_cast<osgText::Text *>(node->getDrawable(0));
    text->setText(info);
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

