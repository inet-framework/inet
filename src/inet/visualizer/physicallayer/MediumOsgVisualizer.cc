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
#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/physicallayer/pathloss/FreeSpacePathLoss.h"
#include "inet/visualizer/physicallayer/MediumOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/ImageStream>
#include <osg/Material>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(MediumOsgVisualizer);

#ifdef WITH_OSG

MediumOsgVisualizer::~MediumOsgVisualizer()
{
    cancelAndDelete(signalPropagationUpdateTimer);
}

void MediumOsgVisualizer::initialize(int stage)
{
    MediumVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        opacityHalfLife = par("opacityHalfLife");
        const char *signalShapeString = par("signalShape");
        if (!strcmp(signalShapeString, "ring"))
            signalShape = SIGNAL_SHAPE_RING;
        else if (!strcmp(signalShapeString, "sphere"))
            signalShape = SIGNAL_SHAPE_SPHERE;
        else if (!strcmp(signalShapeString, "both"))
            signalShape = SIGNAL_SHAPE_BOTH;
        else
            throw cRuntimeError("Unknown signalShape parameter value: '%s'", signalShapeString);
        signalPlane = par("signalPlane");
        if (displayTransmissions) {
            const char *transmissionImageString = par("transmissionImage");
            auto path = resolveResourcePath(transmissionImageString);
            transmissionImage = inet::osg::createImage(path.c_str());
            auto imageStream = dynamic_cast<osg::ImageStream *>(transmissionImage);
            if (imageStream != nullptr)
                imageStream->play();
            if (transmissionImage == nullptr)
                throw cRuntimeError("Transmission image '%s' not found", transmissionImageString);
        }
        if (displayReceptions) {
            const char *receptionImageString = par("receptionImage");
            auto path = resolveResourcePath(receptionImageString);
            receptionImage = inet::osg::createImage(path.c_str());
            auto imageStream = dynamic_cast<osg::ImageStream *>(receptionImage);
            if (imageStream != nullptr)
                imageStream->play();
            if (receptionImage == nullptr)
                throw cRuntimeError("Reception reception '%s' not found", receptionImageString);
        }
        signalPropagationUpdateTimer = new cMessage("signalPropagation");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

void MediumOsgVisualizer::handleMessage(cMessage *message)
{
    if (message == signalPropagationUpdateTimer)
        scheduleSignalPropagationUpdateTimer();
    else
        throw cRuntimeError("Unknown message");
}

osg::Node *MediumOsgVisualizer::getCachedOsgNode(const IRadio *radio) const
{
    auto it = radioOsgNodes.find(radio);
    if (it == radioOsgNodes.end())
        return nullptr;
    else
        return it->second;
}

void MediumOsgVisualizer::setCachedOsgNode(const IRadio *radio, osg::Node *node)
{
    radioOsgNodes[radio] = node;
}

osg::Node *MediumOsgVisualizer::removeCachedOsgNode(const IRadio *radio)
{
    auto it = radioOsgNodes.find(radio);
    if (it == radioOsgNodes.end())
        return nullptr;
    else {
        radioOsgNodes.erase(it);
        return it->second;
    }
}

osg::Node *MediumOsgVisualizer::getCachedOsgNode(const ITransmission *transmission) const
{
    auto it = transmissionOsgNodes.find(transmission);
    if (it == transmissionOsgNodes.end())
        return nullptr;
    else
        return it->second;
}

void MediumOsgVisualizer::setCachedOsgNode(const ITransmission *transmission, osg::Node *node)
{
    transmissionOsgNodes[transmission] = node;
}

osg::Node *MediumOsgVisualizer::removeCachedOsgNode(const ITransmission *transmission)
{
    auto it = transmissionOsgNodes.find(transmission);
    if (it == transmissionOsgNodes.end())
        return nullptr;
    else {
        transmissionOsgNodes.erase(it);
        return it->second;
    }
}

osg::Node *MediumOsgVisualizer::createTransmissionNode(const ITransmission *transmission) const
{
    switch (signalShape) {
        case SIGNAL_SHAPE_RING:
            return createRingTransmissionNode(transmission);
        case SIGNAL_SHAPE_SPHERE:
            return createSphereTransmissionNode(transmission);
        case SIGNAL_SHAPE_BOTH: {
            auto group = new osg::Group();
            group->addChild(createRingTransmissionNode(transmission));
            group->addChild(createSphereTransmissionNode(transmission));
            return group;
        }
        default:
            throw cRuntimeError("Unimplemented signal shape");
    }
}

osg::Node *MediumOsgVisualizer::createRingTransmissionNode(const ITransmission *transmission) const
{
    cFigure::Color color = cFigure::GOOD_DARK_COLORS[transmission->getId() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
    auto depth = new osg::Depth();
    depth->setWriteMask(false);
    auto stateSet = inet::osg::createStateSet(color, 1.0, false);
    stateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);
    auto transmissionStart = transmission->getStartPosition();
    // FIXME: there's some random overlapping artifact due to clipping degenerate triangles
    // FIXME: when the inner radius is very small and the outer radius is very large
    // FIXME: split up shape into multiple annuluses having more and more vertices, and being wider outwards
    auto annulus = inet::osg::createAnnulusGeometry(Coord::ZERO, 0, 0, 100);
    annulus->setStateSet(stateSet);
    osg::AutoTransform::AutoRotateMode autoRotateMode;
    if (!strcmp(signalPlane, "xy"))
        autoRotateMode = osg::AutoTransform::NO_ROTATION;
    else if (!strcmp(signalPlane, "xz"))
        autoRotateMode = osg::AutoTransform::ROTATE_TO_AXIS;
    else if (!strcmp(signalPlane, "yz"))
        autoRotateMode = osg::AutoTransform::ROTATE_TO_AXIS;
    else if (!strcmp(signalPlane, "camera"))
        autoRotateMode = osg::AutoTransform::ROTATE_TO_SCREEN;
    else
        throw cRuntimeError("Unknown signalPlane parameter value: '%s'", signalPlane);
    auto signalAutoTransform = inet::osg::createAutoTransform(annulus, autoRotateMode, false);
    auto label = new osgText::Text();
    label->setCharacterSize(18);
    label->setColor(osg::Vec4(color.red / 255.0 / 2, color.green / 255.0 / 2, color.blue / 255.0 / 2, 1.0));
    label->setAlignment(osgText::Text::CENTER_BOTTOM);
    label->setText(transmission->getMacFrame()->getName());
    auto labelAutoTransform = inet::osg::createAutoTransform(label, osg::AutoTransform::ROTATE_TO_SCREEN, true);
    labelAutoTransform->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto positionAttitudeTransform = inet::osg::createPositionAttitudeTransform(transmissionStart, EulerAngles::ZERO);
    positionAttitudeTransform->setNodeMask(0);
    positionAttitudeTransform->addChild(signalAutoTransform);
    positionAttitudeTransform->addChild(labelAutoTransform);
    if (opacityHalfLife > 0) {
        auto program = new osg::Program();
        auto vertexShader = new osg::Shader(osg::Shader::VERTEX);
        auto fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
        vertexShader->setShaderSource(R"(
            varying vec4 verpos;
            varying vec4 color;
            void main(){
                gl_Position = ftransform();
                verpos = gl_Vertex;
                color = gl_Color;
                gl_TexCoord[0]=gl_MultiTexCoord0;
            }
        )");
        fragmentShader->setShaderSource(R"(
            varying vec4 verpos;
            varying vec4 color;
            uniform float min, max, opacityHalfLife;
            void main( void )
            {
                float alpha = pow(2.0, -length(verpos) / opacityHalfLife);
                gl_FragColor = vec4(color.rgb, alpha);
            }
        )");
        program->addShader(vertexShader);
        program->addShader(fragmentShader);
        stateSet->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateSet->addUniform(new osg::Uniform("min", 0.0f));
        stateSet->addUniform(new osg::Uniform("max", 200.0f));
        stateSet->addUniform(new osg::Uniform("opacityHalfLife", (float)opacityHalfLife));
    }
    return positionAttitudeTransform;
}

osg::Node *MediumOsgVisualizer::createSphereTransmissionNode(const ITransmission *transmission) const
{
    auto transmissionStart = transmission->getStartPosition();
    auto startSphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(transmissionStart.x, transmissionStart.y, transmissionStart.z), 0));
    auto endSphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(transmissionStart.x, transmissionStart.y, transmissionStart.z), 0));
    cFigure::Color color = cFigure::GOOD_DARK_COLORS[transmission->getId() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))];
    auto depth = new osg::Depth();
    depth->setWriteMask(false);
    auto startStateSet = inet::osg::createStateSet(color, 1.0, false);
    startStateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);
    startSphere->setStateSet(startStateSet);
    auto endStateSet = inet::osg::createStateSet(color, 1.0, false);
    endStateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);
    endSphere->setStateSet(endStateSet);
    auto startGeode = new osg::Geode();
    startGeode->addDrawable(startSphere);
    startGeode->setNodeMask(0);
    auto endGeode = new osg::Geode();
    endGeode->addDrawable(endSphere);
    endGeode->setNodeMask(0);
    auto group = new osg::Group();
    group->addChild(startGeode);
    group->addChild(endGeode);
    return group;
}

void MediumOsgVisualizer::radioAdded(const IRadio *radio)
{
    Enter_Method_Silent();
    if (displayTransmissions || displayReceptions || displayInterferenceRanges || displayCommunicationRanges) {
        auto group = new osg::Group();
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
        auto networkNodeVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(module));
        networkNodeVisualization->addAnnotation(group, osg::Vec3d(0.0, 0.0, 0.0), 100.0);
        if (displayTransmissions) {
            auto texture = new osg::Texture2D();
            texture->setImage(transmissionImage);
            auto geometry = osg::createTexturedQuadGeometry(osg::Vec3(-transmissionImage->s() / 2, 0.0, 0.0), osg::Vec3(transmissionImage->s(), 0.0, 0.0), osg::Vec3(0.0, transmissionImage->t(), 0.0), 0.0, 0.0, 1.0, 1.0);
            auto stateSet = geometry->getOrCreateStateSet();
            stateSet->setTextureAttributeAndModes(0, texture);
            stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
            stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
            stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            auto geode = new osg::Geode();
            geode->addDrawable(geometry);
            geode->setNodeMask(0);
            group->addChild(geode);
        }
        if (displayReceptions) {
            auto texture = new osg::Texture2D();
            texture->setImage(receptionImage);
            auto geometry = osg::createTexturedQuadGeometry(osg::Vec3(-transmissionImage->s() / 2, 0.0, 0.0), osg::Vec3(receptionImage->s(), 0.0, 0.0), osg::Vec3(0.0, receptionImage->t(), 0.0), 0.0, 0.0, 1.0, 1.0);
            auto stateSet = geometry->getOrCreateStateSet();
            stateSet->setTextureAttributeAndModes(0, texture);
            stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
            stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
            stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            auto geode = new osg::Geode();
            geode->addDrawable(geometry);
            geode->setNodeMask(0);
            group->addChild(geode);
        }
        if (displayInterferenceRanges) {
            auto maxInterferenceRange = radioMedium->getMediumLimitCache()->getMaxInterferenceRange(radio);
            auto circle = inet::osg::createCircleGeometry(Coord::ZERO, maxInterferenceRange.get(), 100);
            auto stateSet = inet::osg::createStateSet(cFigure::GREY, 1);
            circle->setStateSet(stateSet);
            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::NO_ROTATION, false);
//            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::ROTATE_TO_SCREEN, false);
            networkNodeVisualization->addChild(autoTransform);
        }
        if (displayCommunicationRanges) {
            auto maxCommunicationRange = radioMedium->getMediumLimitCache()->getMaxCommunicationRange(radio);
            auto circle = inet::osg::createCircleGeometry(Coord::ZERO, maxCommunicationRange.get(), 100);
            auto stateSet = inet::osg::createStateSet(cFigure::BLUE, 1);
            circle->setStateSet(stateSet);
            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::NO_ROTATION, false);
//            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::ROTATE_TO_SCREEN, false);
            networkNodeVisualization->addChild(autoTransform);
        }
        setCachedOsgNode(radio, group);
    }
}

void MediumOsgVisualizer::radioRemoved(const IRadio *radio)
{
    Enter_Method_Silent();
    auto node = removeCachedOsgNode(radio);
    if (node != nullptr) {
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
        auto networkVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(module));
        networkVisualization->removeAnnotation(node);
    }
}

void MediumOsgVisualizer::transmissionAdded(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displaySignals) {
        transmissions.push_back(transmission);
        auto node = createTransmissionNode(transmission);
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
        scene->addChild(node);
        setCachedOsgNode(transmission, node);
        if (signalPropagationUpdateInterval > 0)
            scheduleSignalPropagationUpdateTimer();
    }
}

void MediumOsgVisualizer::transmissionRemoved(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displaySignals) {
        transmissions.erase(std::remove(transmissions.begin(), transmissions.end(), transmission));
        auto node = removeCachedOsgNode(transmission);
        if (node != nullptr)
            node->getParent(0)->removeChild(node);
    }
}

void MediumOsgVisualizer::transmissionStarted(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displayTransmissions) {
        auto group = static_cast<osg::Group *>(getCachedOsgNode(transmission->getTransmitter()));
        auto node = static_cast<osg::Node *>(group->getChild(0));
        node->setNodeMask(1);
    }
}

void MediumOsgVisualizer::transmissionEnded(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displayTransmissions) {
        auto group = static_cast<osg::Group *>(getCachedOsgNode(transmission->getTransmitter()));
        auto node = static_cast<osg::Node *>(group->getChild(0));
        node->setNodeMask(0);
    }
}

void MediumOsgVisualizer::receptionStarted(const IReception *reception)
{
    Enter_Method_Silent();
    if (displayReceptions) {
        auto group = static_cast<osg::Group *>(getCachedOsgNode(reception->getReceiver()));
        auto node = static_cast<osg::Node *>(group->getChild(1));
        node->setNodeMask(1);
    }
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    const ITransmission *transmission = reception->getTransmission();
    if (displayRadioFrames) {
        Coord transmissionPosition = transmission->getStartPosition();
        Coord receptionPosition = reception->getStartPosition();
        Coord centerPosition = (transmissionPosition + receptionPosition) / 2;
        osg::Geometry *linesGeom = new osg::Geometry();
        osg::Vec3Array *vertexData = new osg::Vec3Array();
        vertexData->push_back(osg::Vec3(transmissionPosition.x, transmissionPosition.y, transmissionPosition.z));
        vertexData->push_back(osg::Vec3(receptionPosition.x, receptionPosition.y, receptionPosition.z));
        osg::DrawArrays *drawArrayLines = new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP);
        drawArrayLines->setFirst(0);
        drawArrayLines->setCount(vertexData->size());
        linesGeom->setVertexArray(vertexData);
        linesGeom->addPrimitiveSet(drawArrayLines);
        osg::Geode *geode = new osg::Geode();
        geode->addDrawable(linesGeom);
        scene->addChild(geode);
    }
}

void MediumOsgVisualizer::receptionEnded(const IReception *reception)
{
    Enter_Method_Silent();
    if (displayReceptions) {
        auto group = static_cast<osg::Group *>(getCachedOsgNode(reception->getReceiver()));
        auto node = static_cast<osg::Node *>(group->getChild(1));
        node->setNodeMask(0);
    }
}

void MediumOsgVisualizer::refreshDisplay() const
{
    if (displaySignals) {
        for (auto transmission : transmissions) {
            auto node = getCachedOsgNode(transmission);
            if (node != nullptr) {
                switch (signalShape) {
                    case SIGNAL_SHAPE_RING: {
                        refreshRingTransmissionNode(transmission, node);
                        break;
                    }
                    case SIGNAL_SHAPE_SPHERE: {
                        refreshSphereTransmissionNode(transmission, node);
                        break;
                    }
                    case SIGNAL_SHAPE_BOTH: {
                        auto group = static_cast<osg::Group *>(node);
                        refreshRingTransmissionNode(transmission, group->getChild(0));
                        refreshSphereTransmissionNode(transmission, group->getChild(1));
                        break;
                    }
                    default:
                        throw cRuntimeError("Unimplemented signal shape");
                }
            }
        }
    }
}

void MediumOsgVisualizer::refreshRingTransmissionNode(const ITransmission *transmission, osg::Node *node) const
{
    auto propagation = radioMedium->getPropagation();
    auto transmissionStart = transmission->getStartPosition();
    double startRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getStartTime()).dbl();
    double endRadius = std::max(0.0, propagation->getPropagationSpeed().get() * (simTime() - transmission->getEndTime()).dbl());
    auto positionAttitudeTransform = static_cast<osg::PositionAttitudeTransform *>(node);
    auto annulusAutoTransform = static_cast<osg::AutoTransform *>(positionAttitudeTransform->getChild(0));
    auto annulus = static_cast<osg::Geometry *>(static_cast<osg::Geode *>(annulusAutoTransform->getChild(0))->getDrawable(0));
    auto labelAutoTransform = static_cast<osg::AutoTransform *>(positionAttitudeTransform->getChild(1));
    auto vertices = inet::osg::createAnnulusVertices(Coord::ZERO, startRadius, endRadius, 100);
    annulus->setVertexArray(vertices);
    node->setNodeMask(startRadius > 0 ? 1 : 0);
    double phi = transmission->getId();
    labelAutoTransform->setPosition(osg::Vec3d(endRadius * sin(phi), endRadius * cos(phi), 0.0));
}

void MediumOsgVisualizer::refreshSphereTransmissionNode(const ITransmission *transmission, osg::Node *node) const
{
    auto propagation = radioMedium->getPropagation();
    auto transmissionStart = transmission->getStartPosition();
    double startRadius = propagation->getPropagationSpeed().get() * (simTime() - transmission->getStartTime()).dbl();
    double endRadius = std::max(0.0, propagation->getPropagationSpeed().get() * (simTime() - transmission->getEndTime()).dbl());
    auto group = static_cast<osg::Group *>(node);
    auto startGeode = static_cast<osg::Geode *>(group->getChild(0));
    startGeode->setNodeMask(startRadius != 0);
    auto endGeode = static_cast<osg::Geode *>(group->getChild(1));
    endGeode->setNodeMask(endRadius != 0);
    auto startSphere = startGeode->getDrawable(0);
    auto endSphere = endGeode->getDrawable(0);
    auto startShape = static_cast<osg::Sphere *>(startSphere->getShape());
    auto endShape = static_cast<osg::Sphere *>(endSphere->getShape());
    startShape->setRadius(startRadius);
    endShape->setRadius(endRadius);
    startSphere->dirtyDisplayList();
    startSphere->dirtyBound();
    endSphere->dirtyDisplayList();
    endSphere->dirtyBound();
    double startAlpha = std::min(1.0, pow(2.0, -startRadius / opacityHalfLife));
    auto startMaterial = static_cast<osg::Material *>(startSphere->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    startMaterial->setAlpha(osg::Material::FRONT_AND_BACK, std::max(0.1, startAlpha));
    double endAlpha = std::min(1.0, pow(2.0, -endRadius / opacityHalfLife));
    auto endMaterial = static_cast<osg::Material *>(endSphere->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    endMaterial->setAlpha(osg::Material::FRONT_AND_BACK, std::max(0.1, endAlpha));
}

void MediumOsgVisualizer::scheduleSignalPropagationUpdateTimer()
{
    if (signalPropagationUpdateTimer->isScheduled())
        cancelEvent(signalPropagationUpdateTimer);
    simtime_t earliestUpdateTime = SimTime::getMaxTime();
    for (auto transmission : transmissions) {
        simtime_t nextSignalPropagationUpdateTime = getNextSignalPropagationUpdateTime(transmission);
        if (nextSignalPropagationUpdateTime < earliestUpdateTime)
            earliestUpdateTime = nextSignalPropagationUpdateTime;
    }
    if (earliestUpdateTime != SimTime::getMaxTime()) {
        scheduleAt(earliestUpdateTime, signalPropagationUpdateTimer);
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

