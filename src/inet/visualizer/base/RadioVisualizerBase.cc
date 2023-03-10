//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/RadioVisualizerBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

RadioVisualizerBase::RadioVisualization::RadioVisualization(const int radioModuleId) :
    radioModuleId(radioModuleId)
{
}

void RadioVisualizerBase::preDelete(cComponent *root)
{
    if (displayRadios) {
        unsubscribe();
        removeAllRadioVisualizations();
    }
}

void RadioVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayRadios = par("displayRadios");
        displayRadioMode = par("displayRadioMode");
        displayReceptionState = par("displayReceptionState");
        displayTransmissionState = par("displayTransmissionState");
        cStringTokenizer radioModeImagesTokenizer(par("radioModeImages"));
        while (radioModeImagesTokenizer.hasMoreTokens())
            radioModeImages.push_back(radioModeImagesTokenizer.nextToken());
        cStringTokenizer receptionStateImagesTokenizer(par("receptionStateImages"));
        while (receptionStateImagesTokenizer.hasMoreTokens())
            receptionStateImages.push_back(receptionStateImagesTokenizer.nextToken());
        cStringTokenizer transmissionStateImagesTokenizer(par("transmissionStateImages"));
        while (transmissionStateImagesTokenizer.hasMoreTokens())
            transmissionStateImages.push_back(transmissionStateImagesTokenizer.nextToken());
        radioFilter.setPattern(par("radioFilter"));
        width = par("width");
        height = par("height");
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        // antenna lobe
        displayAntennaLobes = par("displayAntennaLobes");
        antennaLobeNormalize = par("antennaLobeNormalize");
        antennaLobeRelativeLabels = par("antennaLobeRelativeLabels");
        antennaLobePlaneGlobal = par("antennaLobePlaneGlobal");
        antennaLobePlane = par("antennaLobePlane");
        antennaLobeMode = par("antennaLobeMode");
        antennaLobeLogarithmicBase = par("antennaLobeLogarithmicBase");
        antennaLobeLogarithmicScale = par("antennaLobeLogarithmicScale");
        antennaLobeRadius = par("antennaLobeRadius");
        antennaLobeStep = deg(par("antennaLobeStep"));
        antennaLobeOpacity = par("antennaLobeOpacity");
        antennaLobeLineSmooth = par("antennaLobeLineSmooth");
        antennaLobeLineColor = cFigure::parseColor(par("antennaLobeLineColor"));
        antennaLobeLineStyle = cFigure::parseLineStyle(par("antennaLobeLineStyle"));
        antennaLobeLineWidth = par("antennaLobeLineWidth");
        antennaLobeFillColor = cFigure::parseColor(par("antennaLobeFillColor"));
        if (displayRadios)
            subscribe();
    }
}

void RadioVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "radioFilter"))
        radioFilter.setPattern(par("radioFilter"));
}

void RadioVisualizerBase::subscribe()
{
    visualizationSubjectModule->subscribe(IRadio::radioModeChangedSignal, this);
    visualizationSubjectModule->subscribe(IRadio::receptionStateChangedSignal, this);
    visualizationSubjectModule->subscribe(IRadio::transmissionStateChangedSignal, this);
}

void RadioVisualizerBase::unsubscribe()
{
    // NOTE: lookup the module again because it may have been deleted first
    auto visualizationSubjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (visualizationSubjectModule != nullptr) {
        visualizationSubjectModule->unsubscribe(IRadio::radioModeChangedSignal, this);
        visualizationSubjectModule->unsubscribe(IRadio::receptionStateChangedSignal, this);
        visualizationSubjectModule->unsubscribe(IRadio::transmissionStateChangedSignal, this);
    }
}

void RadioVisualizerBase::refreshDisplay() const
{
    for (auto it : radioVisualizations)
        refreshRadioVisualization(it.second);
}

const RadioVisualizerBase::RadioVisualization *RadioVisualizerBase::getRadioVisualization(int radioModuleId)
{
    auto it = radioVisualizations.find(radioModuleId);
    return (it == radioVisualizations.end()) ? nullptr : it->second;
}

void RadioVisualizerBase::addRadioVisualization(const RadioVisualization *radioVisualization)
{
    radioVisualizations[radioVisualization->radioModuleId] = radioVisualization;
}

void RadioVisualizerBase::removeRadioVisualization(const RadioVisualization *radioVisualization)
{
    radioVisualizations.erase(radioVisualizations.find(radioVisualization->radioModuleId));
}

void RadioVisualizerBase::removeAllRadioVisualizations()
{
    for (auto radioVisualization : std::map<int, const RadioVisualization *>(radioVisualizations)) {
        removeRadioVisualization(radioVisualization.second);
        delete radioVisualization.second;
    }
}

void RadioVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == IRadio::radioModeChangedSignal || signal == IRadio::receptionStateChangedSignal || signal == IRadio::transmissionStateChangedSignal) {
        auto module = check_and_cast<cModule *>(source);
        auto radio = check_and_cast<IRadio *>(module);
        if (getRadioVisualization(module->getId()) == nullptr && radioFilter.matches(module))
            addRadioVisualization(createRadioVisualization(radio));
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

