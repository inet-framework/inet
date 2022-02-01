//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/physicallayer/RadioCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/Quaternion.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

Define_Module(RadioCanvasVisualizer);

RadioCanvasVisualizer::RadioCanvasVisualization::RadioCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, IndexedImageFigure *radioModeFigure, IndexedImageFigure *receptionStateFigure, IndexedImageFigure *transmissionStateFigure, cPolygonFigure *antennaLobeFigure, cOvalFigure *antennaLobeUnitGainFigure, cOvalFigure *antennaLobeMaxGainFigure, const int radioModuleId) :
    RadioVisualization(radioModuleId),
    networkNodeVisualization(networkNodeVisualization),
    radioModeFigure(radioModeFigure),
    receptionStateFigure(receptionStateFigure),
    transmissionStateFigure(transmissionStateFigure),
    antennaLobeFigure(antennaLobeFigure),
    antennaLobeUnitGainFigure(antennaLobeUnitGainFigure),
    antennaLobeMaxGainFigure(antennaLobeMaxGainFigure)
{
}

RadioCanvasVisualizer::RadioCanvasVisualization::~RadioCanvasVisualization()
{
    delete radioModeFigure;
}

void RadioCanvasVisualizer::initialize(int stage)
{
    RadioVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
        canvasProjection = CanvasProjection::getCanvasProjection(visualizationTargetModule->getCanvas());
    }
}

RadioVisualizerBase::RadioVisualization *RadioCanvasVisualizer::createRadioVisualization(const IRadio *radio) const
{
    auto module = check_and_cast<const cModule *>(radio);
    // TODO use RadioFigure?
    IndexedImageFigure *radioModeFigure = nullptr;
    if (displayRadioMode) {
        radioModeFigure = new IndexedImageFigure("radioMode");
        radioModeFigure->setTags((std::string("radioMode ") + tags).c_str());
        radioModeFigure->setTooltip("This figure represents the radio mode of a radio");
        radioModeFigure->setAssociatedObject(const_cast<cModule *>(module));
        radioModeFigure->setImages(radioModeImages);
        radioModeFigure->setSize(cFigure::Point(width, height));
    }
    IndexedImageFigure *receptionStateFigure = nullptr;
    if (displayReceptionState) {
        receptionStateFigure = new IndexedImageFigure("receptionState");
        receptionStateFigure->setTags((std::string("receptionState ") + tags).c_str());
        receptionStateFigure->setTooltip("This figure represents the reception state of a radio");
        receptionStateFigure->setImages(receptionStateImages);
        receptionStateFigure->setSize(cFigure::Point(width, height));
    }
    IndexedImageFigure *transmissionStateFigure = nullptr;
    if (displayTransmissionState) {
        transmissionStateFigure = new IndexedImageFigure("transmissionState");
        transmissionStateFigure->setTags((std::string("transmissionState ") + tags).c_str());
        transmissionStateFigure->setTooltip("This figure represents the transmission state of a radio");
        transmissionStateFigure->setImages(transmissionStateImages);
        transmissionStateFigure->setSize(cFigure::Point(width, height));
    }
    cPolygonFigure *antennaLobeFigure = nullptr;
    cOvalFigure *antennaLobeUnitGainFigure = nullptr;
    cOvalFigure *antennaLobeMaxGainFigure = nullptr;
    if (displayAntennaLobes) {
        antennaLobeFigure = new cPolygonFigure("antenna_lobe");
        antennaLobeFigure->setTags("antennaLobe");
        antennaLobeFigure->setTooltip("This figure represents the antenna lobe of a radio");
        antennaLobeFigure->setZIndex(zIndex);
        antennaLobeFigure->setOutlined(true);
        antennaLobeFigure->setLineColor(antennaLobeLineColor);
        antennaLobeFigure->setLineStyle(antennaLobeLineStyle);
        antennaLobeFigure->setLineWidth(antennaLobeLineWidth);
        antennaLobeFigure->setFilled(true);
        antennaLobeFigure->setFillColor(antennaLobeFillColor);
        antennaLobeFigure->setFillOpacity(antennaLobeOpacity);
        antennaLobeFigure->setSmooth(antennaLobeLineSmooth);
        antennaLobeUnitGainFigure = new cOvalFigure("antenna_lobe unit_gain");
        antennaLobeUnitGainFigure->setTags("antennaLobe unitGain");
        antennaLobeUnitGainFigure->setTooltip("This figure represents the 0dB gain of a radio antenna");
        antennaLobeUnitGainFigure->setZIndex(zIndex);
        antennaLobeUnitGainFigure->setOutlined(true);
        antennaLobeUnitGainFigure->setLineColor(cFigure::GREY);
        antennaLobeUnitGainFigure->setLineStyle(cFigure::LINE_DOTTED);
        antennaLobeMaxGainFigure = new cOvalFigure("antenna_lobe max_gain");
        antennaLobeMaxGainFigure->setTags("antennaLobe maxGain");
        antennaLobeMaxGainFigure->setTooltip("This figure represents the maximum gain of a radio antenna");
        antennaLobeMaxGainFigure->setZIndex(zIndex);
        antennaLobeMaxGainFigure->setOutlined(true);
        antennaLobeMaxGainFigure->setLineColor(cFigure::GREY);
        antennaLobeMaxGainFigure->setLineStyle(cFigure::LINE_DOTTED);
        auto antenna = radio->getAntenna();
        refreshAntennaLobe(antenna, antennaLobeFigure, antennaLobeUnitGainFigure, antennaLobeMaxGainFigure);
    }
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new RadioCanvasVisualization(networkNodeVisualization, radioModeFigure, receptionStateFigure, transmissionStateFigure, antennaLobeFigure, antennaLobeUnitGainFigure, antennaLobeMaxGainFigure, module->getId());
}

void RadioCanvasVisualizer::addRadioVisualization(const RadioVisualization *radioVisualization)
{
    RadioVisualizerBase::addRadioVisualization(radioVisualization);
    auto radioCanvasVisualization = static_cast<const RadioCanvasVisualization *>(radioVisualization);
    if (displayRadioMode)
        radioCanvasVisualization->networkNodeVisualization->addAnnotation(radioCanvasVisualization->radioModeFigure, radioCanvasVisualization->radioModeFigure->getSize(), placementHint, placementPriority);
    if (displayReceptionState)
        radioCanvasVisualization->networkNodeVisualization->addAnnotation(radioCanvasVisualization->receptionStateFigure, radioCanvasVisualization->receptionStateFigure->getSize(), placementHint, placementPriority);
    if (displayTransmissionState)
        radioCanvasVisualization->networkNodeVisualization->addAnnotation(radioCanvasVisualization->transmissionStateFigure, radioCanvasVisualization->transmissionStateFigure->getSize(), placementHint, placementPriority);
    if (displayAntennaLobes) {
        radioCanvasVisualization->networkNodeVisualization->addFigure(radioCanvasVisualization->antennaLobeFigure);
        radioCanvasVisualization->networkNodeVisualization->addFigure(radioCanvasVisualization->antennaLobeUnitGainFigure);
        radioCanvasVisualization->networkNodeVisualization->addFigure(radioCanvasVisualization->antennaLobeMaxGainFigure);
    }
}

void RadioCanvasVisualizer::removeRadioVisualization(const RadioVisualization *radioVisualization)
{
    RadioVisualizerBase::removeRadioVisualization(radioVisualization);
    auto radioCanvasVisualization = static_cast<const RadioCanvasVisualization *>(radioVisualization);
    if (networkNodeVisualizer != nullptr) {
        if (displayRadioMode)
            radioCanvasVisualization->networkNodeVisualization->removeAnnotation(radioCanvasVisualization->radioModeFigure);
        if (displayReceptionState)
            radioCanvasVisualization->networkNodeVisualization->removeAnnotation(radioCanvasVisualization->receptionStateFigure);
        if (displayTransmissionState)
            radioCanvasVisualization->networkNodeVisualization->removeAnnotation(radioCanvasVisualization->transmissionStateFigure);
        if (displayAntennaLobes) {
            radioCanvasVisualization->networkNodeVisualization->removeFigure(radioCanvasVisualization->antennaLobeFigure);
            radioCanvasVisualization->networkNodeVisualization->removeFigure(radioCanvasVisualization->antennaLobeUnitGainFigure);
            radioCanvasVisualization->networkNodeVisualization->removeFigure(radioCanvasVisualization->antennaLobeMaxGainFigure);
        }
    }
}

void RadioCanvasVisualizer::refreshRadioVisualization(const RadioVisualization *radioVisualization) const
{
    auto radioCanvasVisualization = static_cast<const RadioCanvasVisualization *>(radioVisualization);
    auto module = getSimulation()->getComponent(radioCanvasVisualization->radioModuleId);
    if (module != nullptr) {
        auto radio = check_and_cast<IRadio *>(module);
        if (displayRadioMode)
            setImageIndex(radioCanvasVisualization->radioModeFigure, radio->getRadioMode());
        if (displayReceptionState) {
            setImageIndex(radioCanvasVisualization->receptionStateFigure, radio->getReceptionState());
            radioCanvasVisualization->receptionStateFigure->setAssociatedObject(const_cast<cObject *>(dynamic_cast<const cObject *>(radio->getReceptionInProgress())));
        }
        if (displayTransmissionState) {
            setImageIndex(radioCanvasVisualization->transmissionStateFigure, radio->getTransmissionState());
            radioCanvasVisualization->transmissionStateFigure->setAssociatedObject(const_cast<cObject *>(dynamic_cast<const cObject *>(radio->getTransmissionInProgress())));
        }
        if (displayAntennaLobes)
            refreshAntennaLobe(radio->getAntenna(), radioCanvasVisualization->antennaLobeFigure, radioCanvasVisualization->antennaLobeUnitGainFigure, radioCanvasVisualization->antennaLobeMaxGainFigure);
    }
}

void RadioCanvasVisualizer::refreshAntennaLobe(const IAntenna *antenna, cPolygonFigure *antennaLobeFigure, cOvalFigure *antennaLobeUnitGainFigure, cOvalFigure *antennaLobeMaxGainFigure) const
{
    EulerAngles direction;
    auto antennaPosition = antenna->getMobility()->getCurrentPosition();
    double antennaCanvasDepth;
    double maxGain = antenna->getGain()->getMaxGain();
    auto antennaCanvasPosition = canvasProjection->computeCanvasPoint(antennaPosition, antennaCanvasDepth);
    auto antennaDirection = Quaternion(antenna->getMobility()->getCurrentAngularPosition()).inverse();
    for (double i = 0; i < unit(deg(360) / antennaLobeStep).get(); i++) {
        deg angle = i * antennaLobeStep;
        cFigure::Point lobeCanvasOffset(cos(rad(angle).get()), sin(rad(angle).get()));
        if (!strcmp(antennaLobePlane, "view")) {
            auto lobePosition = canvasProjection->computeCanvasPointInverse(antennaCanvasPosition + lobeCanvasOffset, antennaCanvasDepth);
            auto lobeDirection = Quaternion::rotationFromTo(Coord::X_AXIS, lobePosition - antennaPosition);
            direction = (antennaDirection * lobeDirection).toEulerAngles();
        }
        else if (!strcmp(antennaLobePlane, "xy")) {
            if (antennaLobePlaneGlobal) {
                auto lobePosition = antennaPosition + Coord(lobeCanvasOffset.x, lobeCanvasOffset.y, 0);
                auto lobeDirection = Quaternion::rotationFromTo(Coord::X_AXIS, lobePosition - antennaPosition);
                direction = (antennaDirection * lobeDirection).toEulerAngles();
            }
            else
                direction.alpha = angle;
        }
        else if (!strcmp(antennaLobePlane, "xz")) {
            if (antennaLobePlaneGlobal) {
                auto lobePosition = antennaPosition + Coord(lobeCanvasOffset.x, 0, lobeCanvasOffset.y);
                auto lobeDirection = Quaternion::rotationFromTo(Coord::X_AXIS, lobePosition - antennaPosition);
                direction = (antennaDirection * lobeDirection).toEulerAngles();
            }
            else
                direction.beta = angle;
        }
        else if (!strcmp(antennaLobePlane, "yz")) {
            if (antennaLobePlaneGlobal) {
                auto lobePosition = antennaPosition + Coord(0, lobeCanvasOffset.x, lobeCanvasOffset.y);
                auto lobeDirection = Quaternion::rotationFromTo(Coord::X_AXIS, lobePosition - antennaPosition);
                direction = (antennaDirection * lobeDirection).toEulerAngles();
            }
            else {
                direction.alpha = deg(90);
                direction.beta = angle;
            }
        }
        else
            throw cRuntimeError("Unknown antennaLobePlane");
        double gain = antenna->getGain()->computeGain(Quaternion(direction.normalize()));
        double radius = getGainRadius(gain, maxGain);
        cFigure::Point point = lobeCanvasOffset * radius;
        if (antennaLobeFigure->getNumPoints() > i)
            antennaLobeFigure->setPoint(i, point);
        else
            antennaLobeFigure->addPoint(point);
    }
    double unitRadius = getGainRadius(1, maxGain);
    antennaLobeUnitGainFigure->setBounds(cFigure::Rectangle(-unitRadius, -unitRadius, 2 * unitRadius, 2 * unitRadius));
    double maxRadius = getGainRadius(maxGain, maxGain);
    antennaLobeMaxGainFigure->setBounds(cFigure::Rectangle(-maxRadius, -maxRadius, 2 * maxRadius, 2 * maxRadius));
}

void RadioCanvasVisualizer::setImageIndex(IndexedImageFigure *indexedImageFigure, int index) const
{
    indexedImageFigure->setValue(0, simTime(), index);
    indexedImageFigure->setVisible(indexedImageFigure->getImages()[index] != "-");
}

double RadioCanvasVisualizer::getGainRadius(double gain, double maxGain) const
{
    if (antennaLobeNormalize)
        gain /= maxGain;
    if (!strcmp("logarithmic", antennaLobeMode))
        return std::max(0.0, antennaLobeRadius + antennaLobeLogarithmicScale * std::log(gain) / std::log(antennaLobeLogarithmicBase));
    else if (!strcmp("linear", antennaLobeMode))
        return antennaLobeRadius * gain;
    else
        throw cRuntimeError("Unknown antenna lobe mode");
}

} // namespace visualizer

} // namespace inet

