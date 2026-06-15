//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/physicallayer/MediumVsgVisualizer.h"

#include <algorithm>
#include <cmath>

#include <vsg/maths/transform.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/pathloss/FreeSpacePathLoss.h"
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

namespace inet {

namespace visualizer {

Define_Module(MediumVsgVisualizer);

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON

using namespace physicallayer;

void MediumVsgVisualizer::initialize(int stage)
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
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

void MediumVsgVisualizer::refreshDisplay() const
{
    if (displaySignals) {
        for (auto transmission : transmissions) {
            auto node = getSignalVsgNode(transmission);
            if (node != nullptr) {
                switch (signalShape) {
                    case SIGNAL_SHAPE_RING: {
                        refreshRingTransmissionNode(transmission, node.get());
                        break;
                    }
                    case SIGNAL_SHAPE_SPHERE: {
                        refreshSphereTransmissionNode(transmission, node.get());
                        break;
                    }
                    case SIGNAL_SHAPE_BOTH: {
                        auto group = static_cast<::vsg::Group *>(node.get());
                        // child 0 = ring node (MatrixTransform); child 1 = sphere group
                        refreshRingTransmissionNode(transmission, group->children[0].get());
                        refreshSphereTransmissionNode(transmission, group->children[1].get());
                        break;
                    }
                    default:
                        throw cRuntimeError("Unimplemented signal shape");
                }
            }
        }
    }
}

void MediumVsgVisualizer::setAnimationSpeed() const
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
    animationSpeed = (animationSpeed == DBL_MAX) ? 0 : animationSpeed;
    visualizationTargetModule->getCanvas()->setAnimationSpeed(animationSpeed, this);
}

// ---------------------------------------------------------------------------
// Radio VSG node map helpers
// ---------------------------------------------------------------------------

::vsg::ref_ptr<::vsg::Group> MediumVsgVisualizer::getRadioVsgNode(const IRadio *radio) const
{
    auto it = radioVsgNodes.find(radio->getId());
    return (it == radioVsgNodes.end()) ? ::vsg::ref_ptr<::vsg::Group>() : it->second;
}

void MediumVsgVisualizer::setRadioVsgNode(const IRadio *radio, ::vsg::ref_ptr<::vsg::Group> node)
{
    radioVsgNodes[radio->getId()] = node;
}

::vsg::ref_ptr<::vsg::Group> MediumVsgVisualizer::removeRadioVsgNode(const IRadio *radio)
{
    auto it = radioVsgNodes.find(radio->getId());
    if (it == radioVsgNodes.end())
        return ::vsg::ref_ptr<::vsg::Group>();
    auto node = it->second;
    radioVsgNodes.erase(it);
    return node;
}

// ---------------------------------------------------------------------------
// Signal VSG node map helpers
// ---------------------------------------------------------------------------

::vsg::ref_ptr<::vsg::Node> MediumVsgVisualizer::getSignalVsgNode(const ITransmission *transmission) const
{
    auto it = signalVsgNodes.find(transmission->getId());
    return (it == signalVsgNodes.end()) ? ::vsg::ref_ptr<::vsg::Node>() : it->second;
}

void MediumVsgVisualizer::setSignalVsgNode(const ITransmission *transmission, ::vsg::ref_ptr<::vsg::Node> node)
{
    signalVsgNodes[transmission->getId()] = node;
}

::vsg::ref_ptr<::vsg::Node> MediumVsgVisualizer::removeSignalVsgNode(const ITransmission *transmission)
{
    auto it = signalVsgNodes.find(transmission->getId());
    if (it == signalVsgNodes.end())
        return ::vsg::ref_ptr<::vsg::Node>();
    auto node = it->second;
    signalVsgNodes.erase(it);
    return node;
}

// ---------------------------------------------------------------------------
// Signal node creation
// ---------------------------------------------------------------------------

::vsg::ref_ptr<::vsg::Node> MediumVsgVisualizer::createSignalNode(const ITransmission *transmission) const
{
    switch (signalShape) {
        case SIGNAL_SHAPE_RING:
            return createRingSignalNode(transmission);
        case SIGNAL_SHAPE_SPHERE:
            return createSphereSignalNode(transmission);
        case SIGNAL_SHAPE_BOTH: {
            auto group = ::vsg::Group::create();
            group->addChild(createRingSignalNode(transmission));
            group->addChild(createSphereSignalNode(transmission));
            return group;
        }
        default:
            throw cRuntimeError("Unimplemented signal shape");
    }
}

::vsg::ref_ptr<::vsg::Node> MediumVsgVisualizer::createRingSignalNode(const ITransmission *transmission) const
{
    // The wavefront is an annulus with wave-modulated, distance-attenuated opacity, rebuilt each frame
    // by refreshRingTransmissionNode (inet::vsg::createWaveRing bakes the OSG signal-shader's per-vertex
    // alpha — the shader-side approach has no off-screen equivalent). The ring is positioned at the
    // transmission start via a MatrixTransform, tilted per the signalPlane parameter below.

    auto color = signalColorSet.getColor(transmission->getId());
    auto transmissionStart = transmission->getStartPosition();

    // Initial annulus: inner=0, outer=0 (will be grown each frame in refreshRingTransmissionNode)
    // Starts invisible (visibility controlled via outer radius == 0).
    auto ringGroup = ::vsg::Group::create();
    // We store the current radii as user values so refreshRingTransmissionNode can rebuild.
    // The ring geometry itself is a child Group with one annulus node.
    // Child 0: the (possibly empty) annulus geometry holder
    // Child 1: the label
    auto annulusHolder = ::vsg::Group::create();
    // No geometry added yet (zero radius); refreshRingTransmissionNode will add it.
    ringGroup->addChild(annulusHolder); // child 0: annulus geometry

    // Label: packet name, placed at ring edge (edge position updated each frame).
    // TODO: OSG uses an AutoTransform ROTATE_TO_SCREEN so text always faces the camera.
    //       VSG createLabel approximates this with a fixed-facing MatrixTransform.
    if (transmission->getPacket() != nullptr) {
        auto label = inet::vsg::createLabel(
            transmission->getPacket()->getName(),
            Coord::ZERO,   // position updated in refreshRingTransmissionNode
            cFigure::Color(color.red / 2, color.green / 2, color.blue / 2),
            18);
        ringGroup->addChild(label); // child 1: label
    } else {
        // Add a placeholder group so child 1 always exists.
        ringGroup->addChild(::vsg::Group::create());
    }

    // Wrap in a MatrixTransform positioned at the transmission start, oriented by signalPlane:
    // xy = flat on the ground (default); xz/yz tilt the ring into the vertical planes. ("camera" =
    // always facing the viewer needs a live billboard for a world-sized, growing ring, which the
    // off-screen path can't do, so it behaves like xy.)
    auto transform = inet::vsg::createPositionAttitudeTransform(transmissionStart, Quaternion::IDENTITY);
    if (signalPlane != nullptr) {
        if (!strcmp(signalPlane, "xz"))
            transform->matrix = transform->matrix * ::vsg::rotate(M_PI / 2, 1.0, 0.0, 0.0);
        else if (!strcmp(signalPlane, "yz"))
            transform->matrix = transform->matrix * ::vsg::rotate(M_PI / 2, 0.0, 1.0, 0.0);
    }
    transform->addChild(ringGroup);
    return transform;
}

::vsg::ref_ptr<::vsg::Node> MediumVsgVisualizer::createSphereSignalNode(const ITransmission *transmission) const
{
    // The OSG version creates two osg::ShapeDrawable spheres (start / end wavefront)
    // with osg::Material alpha updated every frame for distance-based fade.
    //
    // VSG approximation: two MatrixTransform nodes each holding a solid sphere.
    // Opacity is baked at construction (see refreshSphereTransmissionNode for rebuild logic).
    // Both start hidden (zero radius); refreshSphereTransmissionNode sets radii each frame.
    //
    // TODO: per-instance opacity update currently rebuilds the sphere geometry child
    //       (same approach as PacketDropVsgVisualizer::setAlpha).  A push-constant
    //       or descriptor-set alpha uniform would be more efficient.

    auto transmissionStart = transmission->getStartPosition();
    cFigure::Color color = signalColorSet.getColor(transmission->getId());

    // Start-wavefront sphere (the leading edge of the signal)
    auto startSphereTransform = inet::vsg::createPositionAttitudeTransform(transmissionStart, Quaternion::IDENTITY);
    // Zero-radius sphere: not yet visible.  refreshSphereTransmissionNode will rebuild it.
    // We push a placeholder Group; the actual sphere is added during the first refresh.
    auto startHolder = ::vsg::Group::create();
    startSphereTransform->addChild(startHolder);

    // End-wavefront sphere (the trailing edge, non-zero only while transmission is ongoing)
    auto endSphereTransform = inet::vsg::createPositionAttitudeTransform(transmissionStart, Quaternion::IDENTITY);
    auto endHolder = ::vsg::Group::create();
    endSphereTransform->addChild(endHolder);

    auto group = ::vsg::Group::create();
    group->addChild(startSphereTransform); // child 0: start-wavefront
    group->addChild(endSphereTransform);   // child 1: end-wavefront
    return group;
}

// ---------------------------------------------------------------------------
// Signal node refresh (called every display refresh frame)
// ---------------------------------------------------------------------------

void MediumVsgVisualizer::refreshRingTransmissionNode(const ITransmission *transmission, ::vsg::Node *node) const
{
    auto propagation = radioMedium->getPropagation();
    double startRadius = propagation->getPropagationSpeed().get<mps>() * (simTime() - transmission->getStartTime()).dbl();
    double endRadius   = std::max(0.0, propagation->getPropagationSpeed().get<mps>() * (simTime() - transmission->getEndTime()).dbl());

    // node is the MatrixTransform wrapping the ringGroup.
    auto transform = static_cast<::vsg::MatrixTransform *>(node);
    // ringGroup children: [0] annulusHolder, [1] label placeholder-or-node
    auto ringGroup = static_cast<::vsg::Group *>(transform->children[0].get());
    auto annulusHolder = static_cast<::vsg::Group *>(ringGroup->children[0].get());

    // Rebuild the wavefront geometry with the current radii (VSG bakes geometry, so clear + re-add).
    annulusHolder->children.clear();

    // The signal is drawn as OPAQUE thin expanding circles at the wavefront edges, not a faded/
    // wave-modulated translucent disc: the off-screen backend cannot composite transparency over opaque
    // geometry (a translucent overlay reads the background, not the floor behind it), so any alpha-based
    // signal renders as a dark blob. Opaque circles avoid that entirely. (createWaveRing remains in
    // VsgUtils for when the backend transparency bug is fixed.)
    //
    // Bound the drawn radius to the transmitter's interference range: a signal propagates at light
    // speed, so within a packet's duration its radius reaches hundreds of km — past the range there is
    // nothing meaningful to show, and an off-scene circle would just clutter the horizon.
    double maxRadius = 1e18;
    if (auto transmitterRadio = transmission->getTransmitterRadio())
        maxRadius = radioMedium->getMediumLimitCache()->getMaxInterferenceRange(transmitterRadio).get();

    cFigure::Color color = signalColorSet.getColor(transmission->getId());
    if (startRadius > 0 && startRadius <= maxRadius)  // leading edge
        annulusHolder->addChild(inet::vsg::createCircle(Coord::ZERO, startRadius, color, cFigure::LINE_SOLID, 2.0, 100));
    if (endRadius > 0 && endRadius <= maxRadius)       // trailing edge (after the transmission ends)
        annulusHolder->addChild(inet::vsg::createCircle(Coord::ZERO, endRadius, color, cFigure::LINE_SOLID, 2.0, 100));

    // Update label position (placed at edge of inner radius in the direction of transmission->getId()).
    double phi = transmission->getId();
    Coord labelPos(endRadius * sin(phi), endRadius * cos(phi), 0.0);
    // TODO: label node position is baked at construction; rebuilding it here requires
    //       replacing child 1 of ringGroup.  For now the label stays at Coord::ZERO.
    //       Implement mutable label position once VsgUtils exposes a setPosition on createLabel.
    (void)labelPos;
}

void MediumVsgVisualizer::refreshSphereTransmissionNode(const ITransmission *transmission, ::vsg::Node *node) const
{
    auto propagation = radioMedium->getPropagation();
    double startRadius = propagation->getPropagationSpeed().get<mps>() * (simTime() - transmission->getStartTime()).dbl();
    double endRadius   = std::max(0.0, propagation->getPropagationSpeed().get<mps>() * (simTime() - transmission->getEndTime()).dbl());

    cFigure::Color color = signalColorSet.getColor(transmission->getId());

    // node is the Group holding [0] startSphereTransform, [1] endSphereTransform.
    auto group = static_cast<::vsg::Group *>(node);

    // --- start wavefront ---
    auto startTransform = static_cast<::vsg::MatrixTransform *>(group->children[0].get());
    auto startHolder    = static_cast<::vsg::Group *>(startTransform->children[0].get());
    startHolder->children.clear();
    if (startRadius > 0) {
        double startAlpha = (signalFadingDistance > 0 && signalFadingFactor > 1.0)
            ? std::min(1.0, pow(signalFadingFactor, -startRadius / signalFadingDistance))
            : 0.99;
        startAlpha = std::max(0.1, startAlpha);
        // TODO: the OSG version mutates osg::Material alpha in-place; VSG requires geometry
        //       rebuild.  This matches the approach in PacketDropVsgVisualizer::setAlpha.
        startHolder->addChild(inet::vsg::createSphere(Coord::ZERO, startRadius, color, startAlpha));
    }

    // --- end wavefront ---
    auto endTransform = static_cast<::vsg::MatrixTransform *>(group->children[1].get());
    auto endHolder    = static_cast<::vsg::Group *>(endTransform->children[0].get());
    endHolder->children.clear();
    if (endRadius > 0) {
        double endAlpha = (signalFadingDistance > 0 && signalFadingFactor > 1.0)
            ? std::min(1.0, pow(signalFadingFactor, -endRadius / signalFadingDistance))
            : 0.99;
        endAlpha = std::max(0.1, endAlpha);
        endHolder->addChild(inet::vsg::createSphere(Coord::ZERO, endRadius, color, endAlpha));
    }
}

// ---------------------------------------------------------------------------
// Event handlers
// ---------------------------------------------------------------------------

void MediumVsgVisualizer::handleRadioAdded(const IRadio *radio)
{
    Enter_Method("handleRadioAdded");
    if (displaySignalDepartures || displaySignalArrivals || displayInterferenceRanges || displayCommunicationRanges) {
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
        auto networkNode = getContainingNode(module);
        auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);

        auto annotationGroup = ::vsg::Group::create();

        if (displaySignalDepartures) {
            // The OSG version renders a textured quad (from signalDepartureImage) as a billboard.
            // VSG has no texture helper in VsgUtils yet, so we approximate with a coloured sphere.
            // TODO: replace with a textured billboard quad showing the signalDepartureImage once
            //       inet::vsg::createTexturedBillboard() is available.
            auto indicator = inet::vsg::createSphere(Coord::ZERO, 4.0, cFigure::YELLOW, 0.9);
            auto departureGroup = ::vsg::Group::create();
            departureGroup->addChild(indicator);
            // Start invisible; made visible in handleSignalDepartureStarted.
            // TODO: VSG has no node-mask equivalent.  We approximate by adding/removing the child.
            //       For now the indicator is always visible after radio is added.
            //       Track visibility via the departureGroup being empty or not.
            annotationGroup->addChild(departureGroup); // child 0: departure indicator
        }
        else {
            annotationGroup->addChild(::vsg::Group::create()); // placeholder child 0
        }

        if (displaySignalArrivals) {
            // TODO: same as departure — replace with textured billboard once VsgUtils supports it.
            auto indicator = inet::vsg::createSphere(Coord::ZERO, 4.0, cFigure::GREEN, 0.9);
            auto arrivalGroup = ::vsg::Group::create();
            arrivalGroup->addChild(indicator);
            annotationGroup->addChild(arrivalGroup); // child 1: arrival indicator
        }
        else {
            annotationGroup->addChild(::vsg::Group::create()); // placeholder child 1
        }

        if (displayInterferenceRanges) {
            auto maxInterferenceRange = radioMedium->getMediumLimitCache()->getMaxInterferenceRange(radio);
            // TODO: the OSG version uses createCircleGeometry + a line stateSet (with line
            //       style, color, width) placed via NO_ROTATION AutoTransform (stays in the xy plane).
            //       VSG createCircle draws a circle in the xy plane; line style is approximated
            //       (LINE_SOLID only; see VsgUtils.h capability gaps).
            auto circle = inet::vsg::createCircle(Coord::ZERO, maxInterferenceRange.get(),
                interferenceRangeLineColor, interferenceRangeLineStyle, interferenceRangeLineWidth);
            networkNodeVisualization->addChild(circle);
        }

        if (displayCommunicationRanges) {
            auto maxCommunicationRange = radioMedium->getMediumLimitCache()->getMaxCommunicationRange(radio);
            // TODO: same plane/line-style approximation note as interference range above.
            auto circle = inet::vsg::createCircle(Coord::ZERO, maxCommunicationRange.get(),
                communicationRangeLineColor, communicationRangeLineStyle, communicationRangeLineWidth);
            networkNodeVisualization->addChild(circle);
        }

        // Attach the departure/arrival indicator group as an annotation on the network node.
        networkNodeVisualization->addAnnotation(annotationGroup, ::vsg::dvec3(0.0, 0.0, 0.0), 100.0);
        setRadioVsgNode(radio, annotationGroup);
    }
}

void MediumVsgVisualizer::handleRadioRemoved(const IRadio *radio)
{
    Enter_Method("handleRadioRemoved");
    auto node = removeRadioVsgNode(radio);
    if (node != nullptr) {
        auto module = const_cast<cModule *>(check_and_cast<const cModule *>(radio));
        auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(module));
        networkNodeVisualization->removeAnnotation(node.get());
    }
}

void MediumVsgVisualizer::handleSignalAdded(const ITransmission *transmission)
{
    Enter_Method("handleSignalAdded");
    MediumVisualizerBase::handleSignalAdded(transmission);
    if (displaySignals) {
        transmissions.push_back(transmission);
        auto node = createSignalNode(transmission);
        auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        scene->addChild(node);
        setSignalVsgNode(transmission, node);
        setAnimationSpeed();
    }
}

void MediumVsgVisualizer::handleSignalRemoved(const ITransmission *transmission)
{
    Enter_Method("handleSignalRemoved");
    MediumVisualizerBase::handleSignalRemoved(transmission);
    if (displaySignals) {
        transmissions.erase(std::remove(transmissions.begin(), transmissions.end(), transmission), transmissions.end());
        auto node = removeSignalVsgNode(transmission);
        if (node != nullptr) {
            auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
            auto& ch = scene->children;
            ch.erase(std::remove(ch.begin(), ch.end(), node), ch.end());
        }
        setAnimationSpeed();
    }
}

void MediumVsgVisualizer::handleSignalDepartureStarted(const ITransmission *transmission)
{
    Enter_Method("handleSignalDepartureStarted");
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalDepartures) {
        // Show the departure indicator: child 0 of the annotation group.
        // The OSG version flips a node-mask; VSG lacks node masks, so we show/hide
        // by adding/clearing children of the departure Group (child 0 of annotationGroup).
        auto annotationGroup = getRadioVsgNode(transmission->getTransmitterRadio());
        if (annotationGroup) {
            auto departureGroup = static_cast<::vsg::Group *>(annotationGroup->children[0].get());
            if (departureGroup->children.empty()) {
                // Re-add the indicator sphere (was cleared in DepartureEnded).
                departureGroup->addChild(inet::vsg::createSphere(Coord::ZERO, 4.0, cFigure::YELLOW, 0.9));
            }
        }
    }
}

void MediumVsgVisualizer::handleSignalDepartureEnded(const ITransmission *transmission)
{
    Enter_Method("handleSignalDepartureEnded");
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalDepartures) {
        // Hide the departure indicator by clearing its children.
        auto annotationGroup = getRadioVsgNode(transmission->getTransmitterRadio());
        if (annotationGroup) {
            auto departureGroup = static_cast<::vsg::Group *>(annotationGroup->children[0].get());
            departureGroup->children.clear();
        }
    }
}

void MediumVsgVisualizer::handleSignalArrivalStarted(const IReception *reception)
{
    Enter_Method("handleSignalArrivalStarted");
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalArrivals) {
        // Show the arrival indicator: child 1 of the annotation group.
        auto annotationGroup = getRadioVsgNode(reception->getReceiverRadio());
        if (annotationGroup) {
            auto arrivalGroup = static_cast<::vsg::Group *>(annotationGroup->children[1].get());
            if (arrivalGroup->children.empty()) {
                arrivalGroup->addChild(inet::vsg::createSphere(Coord::ZERO, 4.0, cFigure::GREEN, 0.9));
            }
        }
    }
}

void MediumVsgVisualizer::handleSignalArrivalEnded(const IReception *reception)
{
    Enter_Method("handleSignalArrivalEnded");
    if (displaySignals)
        setAnimationSpeed();
    if (displaySignalArrivals) {
        auto annotationGroup = getRadioVsgNode(reception->getReceiverRadio());
        if (annotationGroup) {
            auto arrivalGroup = static_cast<::vsg::Group *>(annotationGroup->children[1].get());
            arrivalGroup->children.clear();
        }
    }
}

#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

} // namespace visualizer

} // namespace inet
