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
#include "inet/common/OsgScene.h"
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/base/SceneOsgVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/PolygonOffset>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

#ifdef WITH_OSG

void SceneOsgVisualizerBase::initializeScene()
{
    auto osgCanvas = visualizationTargetModule->getOsgCanvas();
    if (osgCanvas->getScene() != nullptr)
        throw cRuntimeError("OSG canvas scene at '%s' has been already initialized", visualizationTargetModule->getFullPath().c_str());
    else {
        auto topLevelScene = new inet::osg::TopLevelScene();
        osgCanvas->setScene(topLevelScene);
        const char *clearColor = par("clearColor");
        if (*clearColor != '\0')
            osgCanvas->setClearColor(cFigure::Color(clearColor));
        osgCanvas->setZNear(par("zNear"));
        osgCanvas->setZFar(par("zFar"));
        osgCanvas->setFieldOfViewAngle(par("fieldOfView"));
        const char *cameraManipulatorString = par("cameraManipulator");
        cOsgCanvas::CameraManipulatorType cameraManipulator;
        if (!strcmp(cameraManipulatorString, "auto"))
            cameraManipulator = cOsgCanvas::CAM_AUTO;
        else if (!strcmp(cameraManipulatorString, "trackball"))
            cameraManipulator = cOsgCanvas::CAM_TRACKBALL;
        else if (!strcmp(cameraManipulatorString, "terrain"))
            cameraManipulator = cOsgCanvas::CAM_TERRAIN;
        else if (!strcmp(cameraManipulatorString, "overview"))
            cameraManipulator = cOsgCanvas::CAM_OVERVIEW;
        else if (!strcmp(cameraManipulatorString, "earth"))
            cameraManipulator = cOsgCanvas::CAM_EARTH;
        else
            throw cRuntimeError("Unknown camera manipulator: '%s'", cameraManipulatorString);
        osgCanvas->setCameraManipulatorType(cameraManipulator);
    }
}

void SceneOsgVisualizerBase::initializeAxis(double axisLength)
{
    auto geode = new osg::Group();
    geode->addChild(inet::osg::createLine(Coord::ZERO, Coord(axisLength, 0.0, 0.0), cFigure::ARROW_NONE, cFigure::ARROW_BARBED));
    geode->addChild(inet::osg::createLine(Coord::ZERO, Coord(0.0, axisLength, 0.0), cFigure::ARROW_NONE, cFigure::ARROW_BARBED));
    geode->addChild(inet::osg::createLine(Coord::ZERO, Coord(0.0, 0.0, axisLength), cFigure::ARROW_NONE, cFigure::ARROW_BARBED));
    auto stateSet = inet::osg::createStateSet(cFigure::BLACK, 1.0, false);
    geode->setStateSet(stateSet);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(geode);
    double spacing = 1;
    scene->addChild(inet::osg::createAutoTransform(inet::osg::createText("X", Coord::ZERO, cFigure::BLACK), osg::AutoTransform::ROTATE_TO_SCREEN, true, Coord(axisLength + spacing, 0.0, 0.0)));
    scene->addChild(inet::osg::createAutoTransform(inet::osg::createText("Y", Coord::ZERO, cFigure::BLACK), osg::AutoTransform::ROTATE_TO_SCREEN, true, Coord(0.0, axisLength + spacing, 0.0)));
    scene->addChild(inet::osg::createAutoTransform(inet::osg::createText("Z", Coord::ZERO, cFigure::BLACK), osg::AutoTransform::ROTATE_TO_SCREEN, true, Coord(0.0, 0.0, axisLength + spacing)));
}

void SceneOsgVisualizerBase::initializeSceneFloor()
{
    Box sceneBounds = getSceneBounds();
    if (sceneBounds.getMin() != sceneBounds.getMax()) {
        const char *imageName = par("sceneImage");
        osg::Image *image = *imageName ? inet::osg::createImageFromResource(imageName) : nullptr;
        double imageSize = par("sceneImageSize");
        auto color = cFigure::Color(par("sceneColor"));
        double opacity = par("sceneOpacity");
        bool shading = par("sceneShading");
        auto sceneFloor = createSceneFloor(sceneBounds.getMin(), sceneBounds.getMax(), color, image, imageSize, opacity, shading);
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        scene->addChild(sceneFloor);
    }
}

osg::Geode *SceneOsgVisualizerBase::createSceneFloor(const Coord& min, const Coord& max, cFigure::Color& color, osg::Image *image, double imageSize, double opacity, bool shading) const
{
    auto dx = max.x - min.x;
    auto dy = max.y - min.y;

    if (!std::isfinite(dx) || !std::isfinite(dy))
        return new osg::Geode();

    auto d = shading ? sqrt(dx * dx + dy * dy) : 0;
    auto width = dx + 2 * d;
    auto height = dy + 2 * d;
    auto r = width / imageSize / 2;
    auto t = height / imageSize / 2;
    osg::Geometry *geometry = nullptr;
    if (image == nullptr)
        geometry = inet::osg::createQuadGeometry(Coord(min.x, min.y, 0.0), Coord(max.x, max.y, 0.0));
    else
        geometry = osg::createTexturedQuadGeometry(osg::Vec3(min.x - d, min.y - d, 0.0), osg::Vec3(width, 0.0, 0.0), osg::Vec3(0.0, height, 0.0), -r, -t, r, t);
    auto stateSet = inet::osg::createStateSet(color, opacity, false);
    geometry->setStateSet(stateSet);
    osg::Texture2D *texture = nullptr;
    if (image != nullptr) {
        texture = new osg::Texture2D();
        texture->setImage(image);
        texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
    }
    stateSet->setTextureAttributeAndModes(0, texture);
    if (shading) {
        auto program = new osg::Program();
        auto vertexShader = new osg::Shader(osg::Shader::VERTEX);
        auto fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
        vertexShader->setShaderSource(R"(
            varying vec4 verpos;
            void main() {
                gl_Position = ftransform();
                verpos = gl_Vertex;
                gl_TexCoord[0]=gl_MultiTexCoord0;
            })");
        if (texture != nullptr) {
            fragmentShader->setShaderSource(R"(
                varying vec4 verpos;
                uniform vec3 center;
                uniform float min, max;
                uniform sampler2D texture;
                void main(void) {
                    float alpha = 1.0 - smoothstep(min, max, length(verpos.xyz - center));
                    gl_FragColor = vec4(texture2D(texture, gl_TexCoord[0].xy).rgb, alpha);
                })");
            stateSet->addUniform(new osg::Uniform("texture", 0));
        }
        else {
            fragmentShader->setShaderSource(R"(
                varying vec4 verpos;
                uniform vec3 center;
                uniform vec3 color;
                uniform float min, max;
                void main(void) {
                    float alpha = 1.0 - smoothstep(min, max, length(verpos.xyz - center));
                    gl_FragColor = vec4(color, alpha);
                })");
            stateSet->addUniform(new osg::Uniform("color", osg::Vec3((double)color.red / 255.0, (double)color.green / 255.0, (double)color.blue / 255.0)));
        }
        program->addShader(vertexShader);
        program->addShader(fragmentShader);
        auto center = (max + min) / 2;
        stateSet->addUniform(new osg::Uniform("center", osg::Vec3(center.x, center.y, center.z)));
        stateSet->addUniform(new osg::Uniform("min", (float)d / 2));
        stateSet->addUniform(new osg::Uniform("max", (float)d));
        stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
    }
    auto polygonOffset = new osg::PolygonOffset();
    polygonOffset->setFactor(1.0);
    polygonOffset->setUnits(1.0);
    stateSet->setAttributeAndModes(polygonOffset, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
    auto geode = new osg::Geode();
    geode->addDrawable(geometry);
    return geode;
}

osg::BoundingSphere SceneOsgVisualizerBase::getNetworkBoundingSphere()
{
    int nodeCount = 0;
    auto nodes = new osg::Group();
    auto networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode)) {
            nodeCount++;
            // NOTE: ignore network node annotations
            auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
            auto mainNode = networkNodeVisualization->getMainPart();
            auto radius = std::max(0.0f, mainNode->computeBound().radius());
            auto drawable = new osg::ShapeDrawable(new osg::Sphere(networkNodeVisualization->getPosition(), radius));
            auto geode = new osg::Geode();
            geode->addDrawable(drawable);
            nodes->addChild(geode);
        }
    }
    if (nodeCount == 0)
        return osg::BoundingSphere(osg::Vec3d(0, 0, 0), 100);
    else if (nodeCount == 1)
        return osg::BoundingSphere(nodes->getBound().center(), 100);
    else
        return nodes->getBound();
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

