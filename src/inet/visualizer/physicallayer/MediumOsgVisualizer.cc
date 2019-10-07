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
#include "inet/visualizer/physicallayer/MediumOsgVisualizer.h"

#ifdef WITH_RADIO
#include "inet/physicallayer/pathloss/FreeSpacePathLoss.h"
#endif // WITH_RADIO

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

#ifdef WITH_RADIO
#ifdef WITH_OSG

using namespace physicallayer;

void MediumOsgVisualizer::initialize(int stage)
{
    MediumVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
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
        signalFadingDistance = par("signalFadingDistance");
        signalFadingFactor = par("signalFadingFactor");
        signalWaveLength = par("signalWaveLength");
        signalWaveAmplitude = par("signalWaveAmplitude");
        signalWaveFadingAnimationSpeedFactor = par("signalWaveFadingAnimationSpeedFactor");
        if (displaySignalDepartures) {
            const char *transmissionImageName = par("transmissionImage");
            transmissionImage = inet::osg::createImageFromResource(transmissionImageName);
            auto imageStream = dynamic_cast<osg::ImageStream *>(transmissionImage);
            if (imageStream != nullptr)
                imageStream->play();
        }
        if (displaySignalArrivals) {
            const char *receptionImageName = par("receptionImage");
            receptionImage = inet::osg::createImageFromResource(receptionImageName);
            auto imageStream = dynamic_cast<osg::ImageStream *>(receptionImage);
            if (imageStream != nullptr)
                imageStream->play();
        }
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}
void MediumOsgVisualizer::refreshDisplay() const
{
    if (displaySignals) {
        for (auto transmission : transmissions) {
            auto node = getSignalOsgNode(transmission);
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

void MediumOsgVisualizer::setAnimationSpeed() const
{
    double animationSpeed = DBL_MAX;
    if (displaySignals) {
        for (auto transmission : transmissions) {
            if (isSignalPropagationInProgress(transmission))
                animationSpeed = std::min(animationSpeed, signalPropagationAnimationSpeed);
            else if (isSignalTransmissionInProgress(transmission))
                animationSpeed = std::min(animationSpeed, signalTransmissionAnimationSpeed);
        }
    }
    animationSpeed = animationSpeed == DBL_MAX ? 0 : animationSpeed;
    // TODO: switch to osg canvas when API is extended
    visualizationTargetModule->getCanvas()->setAnimationSpeed(animationSpeed, this);
}

osg::Node *MediumOsgVisualizer::getRadioOsgNode(const IRadio *radio) const
{
    auto it = radioOsgNodes.find(radio);
    if (it == radioOsgNodes.end())
        return nullptr;
    else
        return it->second;
}

void MediumOsgVisualizer::setRadioOsgNode(const IRadio *radio, osg::Node *node)
{
    radioOsgNodes[radio] = node;
}

osg::Node *MediumOsgVisualizer::removeRadioOsgNode(const IRadio *radio)
{
    auto it = radioOsgNodes.find(radio);
    if (it == radioOsgNodes.end())
        return nullptr;
    else {
        radioOsgNodes.erase(it);
        return it->second;
    }
}

osg::Node *MediumOsgVisualizer::getSignalOsgNode(const ITransmission *transmission) const
{
    auto it = signalOsgNodes.find(transmission);
    if (it == signalOsgNodes.end())
        return nullptr;
    else
        return it->second;
}

void MediumOsgVisualizer::setSignalOsgNode(const ITransmission *transmission, osg::Node *node)
{
    signalOsgNodes[transmission] = node;
}

osg::Node *MediumOsgVisualizer::removeSignalOsgNode(const ITransmission *transmission)
{
    auto it = signalOsgNodes.find(transmission);
    if (it == signalOsgNodes.end())
        return nullptr;
    else {
        signalOsgNodes.erase(it);
        return it->second;
    }
}

osg::Node *MediumOsgVisualizer::createSignalNode(const ITransmission *transmission) const
{
    switch (signalShape) {
        case SIGNAL_SHAPE_RING:
            return createRingSignalNode(transmission);
        case SIGNAL_SHAPE_SPHERE:
            return createSphereSignalNode(transmission);
        case SIGNAL_SHAPE_BOTH: {
            auto group = new osg::Group();
            group->addChild(createRingSignalNode(transmission));
            group->addChild(createSphereSignalNode(transmission));
            return group;
        }
        default:
            throw cRuntimeError("Unimplemented signal shape");
    }
}

osg::Node *MediumOsgVisualizer::createRingSignalNode(const ITransmission *transmission) const
{
    auto color = signalColorSet.getColor(transmission->getId());
    auto depth = new osg::Depth();
    depth->setWriteMask(false);
    auto stateSet = inet::osg::createStateSet(color, 0.99, false); // <1 opacity so it will be in the TRANSPARENT_BIN
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
    label->setText(transmission->getPacket()->getName());
    auto labelAutoTransform = inet::osg::createAutoTransform(label, osg::AutoTransform::ROTATE_TO_SCREEN, true);
    labelAutoTransform->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto positionAttitudeTransform = inet::osg::createPositionAttitudeTransform(transmissionStart, Quaternion::IDENTITY);
    positionAttitudeTransform->setNodeMask(0);
    positionAttitudeTransform->addChild(signalAutoTransform);
    positionAttitudeTransform->addChild(labelAutoTransform);
    if (signalFadingDistance > 0) {
        auto program = new osg::Program();
        auto vertexShader = new osg::Shader(osg::Shader::VERTEX);
        auto fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
        vertexShader->setShaderSource(R"(
            varying vec4 verpos;
            varying vec4 color;
            void main() {
                gl_Position = ftransform();
                verpos = gl_Vertex;
                color = gl_Color;
                gl_TexCoord[0] = gl_MultiTexCoord0;
            }
        )");
        fragmentShader->setShaderSource(R"(
            varying vec4 verpos;
            varying vec4 color;
            uniform float waveLength, waveAmplitude, waveOffset, waveFadingFactor, fadingFactor, fadingDistance;
            void main() {
                float d = length(verpos);
                float phi = (d - waveOffset) / waveLength * 2.0 * 3.1415926535897932384626433832795;
                float a = waveAmplitude * waveFadingFactor / 2.0;
                float alpha = pow(fadingFactor, -d / fadingDistance) * (1.0 - a + cos(phi) * a);
                gl_FragColor = vec4(color.rgb, alpha);
            }
        )");
        program->addShader(vertexShader);
        program->addShader(fragmentShader);
        stateSet->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        stateSet->addUniform(new osg::Uniform("waveLength", (float)signalWaveLength));
        stateSet->addUniform(new osg::Uniform("waveAmplitude", (float)signalWaveAmplitude));
        stateSet->addUniform(new osg::Uniform("waveOffset", 0.0f));
        stateSet->addUniform(new osg::Uniform("waveFadingFactor", 1.0f));
        stateSet->addUniform(new osg::Uniform("fadingFactor", (float)signalFadingFactor));
        stateSet->addUniform(new osg::Uniform("fadingDistance", (float)signalFadingDistance));
    }
    return positionAttitudeTransform;
}

osg::Node *MediumOsgVisualizer::createSphereSignalNode(const ITransmission *transmission) const
{
    auto transmissionStart = transmission->getStartPosition();
    auto startSphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(transmissionStart.x, transmissionStart.y, transmissionStart.z), 0));
    auto endSphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(transmissionStart.x, transmissionStart.y, transmissionStart.z), 0));
    cFigure::Color color = signalColorSet.getColor(transmission->getId());;
    auto depth = new osg::Depth();
    depth->setWriteMask(false);
    auto startStateSet = inet::osg::createStateSet(color, 0.99, false); // <1 opacity so it will be in the TRANSPARENT_BIN
    startStateSet->setAttributeAndModes(depth, osg::StateAttribute::ON);
    startSphere->setStateSet(startStateSet);
    auto endStateSet = inet::osg::createStateSet(color, 0.99, false); // <1 opacity so it will be in the TRANSPARENT_BIN
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

void MediumOsgVisualizer::refreshRingTransmissionNode(const ITransmission *transmission, osg::Node *node) const
{
    auto propagation = radioMedium->getPropagation();
    // TODO: auto transmissionStart = transmission->getStartPosition();
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
    auto stateSet = annulus->getOrCreateStateSet();
    stateSet->getUniform("waveOffset")->set((float)startRadius);
    // TODO: add parameter to control
    double waveFadingFactor = std::min(1.0, signalPropagationAnimationSpeed / getSimulation()->getEnvir()->getAnimationSpeed() / signalWaveFadingAnimationSpeedFactor);
    stateSet->getUniform("waveFadingFactor")->set((float)waveFadingFactor);
}

void MediumOsgVisualizer::refreshSphereTransmissionNode(const ITransmission *transmission, osg::Node *node) const
{
    auto propagation = radioMedium->getPropagation();
    // TODO: auto transmissionStart = transmission->getStartPosition();
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
    double startAlpha = std::min(1.0, pow(signalFadingFactor, -startRadius / signalFadingDistance));
    auto startMaterial = static_cast<osg::Material *>(startSphere->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    startMaterial->setAlpha(osg::Material::FRONT_AND_BACK, std::max(0.1, startAlpha));
    double endAlpha = std::min(1.0, pow(signalFadingFactor, -endRadius / signalFadingDistance));
    auto endMaterial = static_cast<osg::Material *>(endSphere->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    endMaterial->setAlpha(osg::Material::FRONT_AND_BACK, std::max(0.1, endAlpha));
}

void MediumOsgVisualizer::handleRadioAdded(const IRadio *radio)
{
    Enter_Method_Silent();
    if (displaySignalDepartures || displaySignalArrivals || displayInterferenceRanges || displayCommunicationRanges) {
        auto group = new osg::Group();
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
        auto networkNode = getContainingNode(module);
        auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
        if (networkNodeVisualization == nullptr)
            throw cRuntimeError("Cannot create medium visualization for '%s', because network node visualization is not found for '%s'", module->getFullPath().c_str(), networkNode->getFullPath().c_str());
        networkNodeVisualization->addAnnotation(group, osg::Vec3d(0.0, 0.0, 0.0), 100.0);
        if (displaySignalDepartures) {
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
        if (displaySignalArrivals) {
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
            auto stateSet = inet::osg::createLineStateSet(interferenceRangeLineColor, interferenceRangeLineStyle, interferenceRangeLineWidth);
            circle->setStateSet(stateSet);
            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::NO_ROTATION, false);
//            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::ROTATE_TO_SCREEN, false);
            networkNodeVisualization->addChild(autoTransform);
        }
        if (displayCommunicationRanges) {
            auto maxCommunicationRange = radioMedium->getMediumLimitCache()->getMaxCommunicationRange(radio);
            auto circle = inet::osg::createCircleGeometry(Coord::ZERO, maxCommunicationRange.get(), 100);
            auto stateSet = inet::osg::createLineStateSet(communicationRangeLineColor, communicationRangeLineStyle, communicationRangeLineWidth);
            circle->setStateSet(stateSet);
            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::NO_ROTATION, false);
//            auto autoTransform = inet::osg::createAutoTransform(circle, osg::AutoTransform::ROTATE_TO_SCREEN, false);
            networkNodeVisualization->addChild(autoTransform);
        }
        setRadioOsgNode(radio, group);
    }
}

void MediumOsgVisualizer::handleRadioRemoved(const IRadio *radio)
{
    Enter_Method_Silent();
    auto node = removeRadioOsgNode(radio);
    if (node != nullptr) {
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
        auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(module));
        networkNodeVisualization->removeAnnotation(node);
    }
}

void MediumOsgVisualizer::handleSignalAdded(const ITransmission *transmission)
{
    Enter_Method_Silent();
    MediumVisualizerBase::handleSignalAdded(transmission);
    if (displaySignals) {
        transmissions.push_back(transmission);
        auto node = createSignalNode(transmission);
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        scene->addChild(node);
        setSignalOsgNode(transmission, node);
        setAnimationSpeed();
    }
}

void MediumOsgVisualizer::handleSignalRemoved(const ITransmission *transmission)
{
    Enter_Method_Silent();
    MediumVisualizerBase::handleSignalRemoved(transmission);
    if (displaySignals) {
        transmissions.erase(std::remove(transmissions.begin(), transmissions.end(), transmission));
        auto node = removeSignalOsgNode(transmission);
        if (node != nullptr)
            node->getParent(0)->removeChild(node);
        setAnimationSpeed();
    }
}

void MediumOsgVisualizer::handleSignalDepartureStarted(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalDepartures) {
        auto group = static_cast<osg::Group *>(getRadioOsgNode(transmission->getTransmitter()));
        auto node = static_cast<osg::Node *>(group->getChild(0));
        node->setNodeMask(1);
    }
}

void MediumOsgVisualizer::handleSignalDepartureEnded(const ITransmission *transmission)
{
    Enter_Method_Silent();
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalDepartures) {
        auto transmitter = transmission->getTransmitter();
        auto group = static_cast<osg::Group *>(getRadioOsgNode(transmitter));
        auto node = static_cast<osg::Node *>(group->getChild(0));
        node->setNodeMask(0);
    }
}

void MediumOsgVisualizer::handleSignalArrivalStarted(const IReception *reception)
{
    Enter_Method_Silent();
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalArrivals) {
        auto group = static_cast<osg::Group *>(getRadioOsgNode(reception->getReceiver()));
        auto node = static_cast<osg::Node *>(group->getChild(1));
        node->setNodeMask(1);
    }
}

void MediumOsgVisualizer::handleSignalArrivalEnded(const IReception *reception)
{
    Enter_Method_Silent();
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalArrivals) {
        auto receiver = reception->getReceiver();
        auto group = static_cast<osg::Group *>(getRadioOsgNode(receiver));
        auto node = static_cast<osg::Node *>(group->getChild(1));
        node->setNodeMask(0);
    }
}

#endif // ifdef WITH_OSG
#endif // ifdef WITH_RADIO

} // namespace visualizer

} // namespace inet

