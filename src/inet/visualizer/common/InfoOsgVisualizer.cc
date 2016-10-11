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

void InfoOsgVisualizer::initialize(int stage)
{
    InfoVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
        auto simulation = getSimulation();
        for (int i = 0; i < moduleIds.size(); i++) {
            auto text = new osgText::Text();
            text->setCharacterSize(18);
            text->setBoundingBoxColor(osg::Vec4(backgroundColor.red / 255.0, backgroundColor.green / 255.0, backgroundColor.blue / 255.0, 0.5));
            text->setColor(osg::Vec4(fontColor.red / 255.0, fontColor.green / 255.0, fontColor.blue / 255.0, 1.0));
            text->setAlignment(osgText::Text::CENTER_BOTTOM);
            text->setText("");
            text->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
            text->setPosition(osg::Vec3(0.0, 0.0, 0.0));
            auto geode = new osg::Geode();
            geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
            geode->addDrawable(text);
            auto module = simulation->getModule(moduleIds[i]);
            auto visualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(module));
            visualization->addAnnotation(geode, osg::Vec3d(0, 0, 0), 0);
            labels.push_back(geode);
        }
    }
}

void InfoOsgVisualizer::setInfo(int i, const char *info) const
{
    auto text = static_cast<osgText::Text *>(labels[i]->getDrawable(0));
    text->setText(info);
}

} // namespace visualizer

} // namespace inet

