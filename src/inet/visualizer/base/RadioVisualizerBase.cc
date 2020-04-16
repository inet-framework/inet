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
#include "inet/visualizer/base/RadioVisualizerBase.h"

namespace inet {

namespace visualizer {

using namespace inet::physicallayer;

RadioVisualizerBase::RadioVisualization::RadioVisualization(const int radioModuleId) :
    radioModuleId(radioModuleId)
{
}

RadioVisualizerBase::~RadioVisualizerBase()
{
    if (displayRadios)
        unsubscribe();
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
    if (name != nullptr) {
        if (!strcmp(name, "radioFilter"))
            radioFilter.setPattern(par("radioFilter"));
    }
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
    auto visualizationSubjectModule = getModuleFromPar<cModule>(par("visualizationSubjectModule"), this, false);
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
    if (it == radioVisualizations.end())
        return nullptr;
    else
        return it->second;
}

void RadioVisualizerBase::addRadioVisualization(const RadioVisualization *radioVisualization)
{
    radioVisualizations[radioVisualization->radioModuleId] = radioVisualization;
}

void RadioVisualizerBase::removeRadioVisualization(const RadioVisualization *radioVisualization)
{
    radioVisualizations.erase(radioVisualizations.find(radioVisualization->radioModuleId));
}

void RadioVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, intval_t value, cObject *details)
{
    Enter_Method_Silent();
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

