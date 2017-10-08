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
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/visualizer/base/MediumVisualizerBase.h"

namespace inet {

namespace visualizer {

MediumVisualizerBase::~MediumVisualizerBase()
{
    // NOTE: lookup the medium module again because it may have been deleted first
    auto radioMediumModule = getModuleFromPar<cModule>(par("mediumModule"), this, false);
    if (radioMediumModule != nullptr) {
        radioMediumModule->unsubscribe(IRadioMedium::radioAddedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::radioRemovedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::transmissionAddedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::transmissionRemovedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::transmissionStartedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::transmissionEndedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::receptionStartedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::receptionEndedSignal, this);
    }
}

void MediumVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setPattern(par("packetFilter"));
        displaySignals = par("displaySignals");
        signalColorSet.parseColors(par("signalColor"));
        signalPropagationAnimationSpeed = par("signalPropagationAnimationSpeed");
        signalPropagationAnimationTime = par("signalPropagationAnimationTime");
        signalPropagationAdditionalTime = par("signalPropagationAdditionalTime");
        signalTransmissionAnimationSpeed = par("signalTransmissionAnimationSpeed");
        signalTransmissionAnimationTime = par("signalTransmissionAnimationTime");
        signalAnimationSpeedChangeTime = par("signalAnimationSpeedChangeTime");
        displayTransmissions = par("displayTransmissions");
        displayReceptions = par("displayReceptions");
        transmissionPlacementHint = parsePlacement(par("transmissionPlacementHint"));
        receptionPlacementHint = parsePlacement(par("receptionPlacementHint"));
        transmissionPlacementPriority = par("transmissionPlacementPriority");
        receptionPlacementPriority = par("receptionPlacementPriority");
        displayInterferenceRanges = par("displayInterferenceRanges");
        interferenceRangeLineColor = cFigure::Color(par("interferenceRangeLineColor"));
        interferenceRangeLineStyle = cFigure::parseLineStyle(par("interferenceRangeLineStyle"));
        interferenceRangeLineWidth = par("interferenceRangeLineWidth");
        displayCommunicationRanges = par("displayCommunicationRanges");
        communicationRangeLineColor = cFigure::Color(par("communicationRangeLineColor"));
        communicationRangeLineStyle = cFigure::parseLineStyle(par("communicationRangeLineStyle"));
        communicationRangeLineWidth = par("communicationRangeLineWidth");
        signalPropagationAnimationSpeed = par("signalPropagationAnimationSpeed");
        signalTransmissionAnimationSpeed = par("signalTransmissionAnimationSpeed");
        radioMedium = getModuleFromPar<IRadioMedium>(par("mediumModule"), this, false);
        if (radioMedium != nullptr) {
            cModule *radioMediumModule = check_and_cast<cModule *>(radioMedium);
            radioMediumModule->subscribe(IRadioMedium::radioAddedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::radioRemovedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::transmissionAddedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::transmissionRemovedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::transmissionStartedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::transmissionEndedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::receptionStartedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::receptionEndedSignal, this);
        }
    }
    else if (stage == INITSTAGE_LAST) {
        if (std::isnan(signalPropagationAnimationSpeed) && radioMedium != nullptr) {
            double maxPropagationDuration = radioMedium->getMediumLimitCache()->getMaxConstraintArea().distance(radioMedium->getMediumLimitCache()->getMinConstraintArea()) / mps(radioMedium->getPropagation()->getPropagationSpeed()).get();
            defaultSignalPropagationAnimationSpeed = maxPropagationDuration / signalPropagationAnimationTime;
        }
    }
}

void MediumVisualizerBase::handleParameterChange(const char *name)
{
    if (name != nullptr) {
        if (!strcmp(name, "networkNodeFilter"))
            networkNodeFilter.setPattern(par("nodeFilter"));
        else if (!strcmp(name, "interfaceFilter"))
            interfaceFilter.setPattern(par("interfaceFilter"));
        else if (!strcmp(name, "packetFilter"))
            packetFilter.setPattern(par("packetFilter"));
        else if (!strcmp(name, "signalPropagationAnimationSpeed"))
            signalPropagationAnimationSpeed = par("signalPropagationAnimationSpeed");
        else if (!strcmp(name, "signalTransmissionAnimationSpeed"))
            signalTransmissionAnimationSpeed = par("signalTransmissionAnimationSpeed");
        // TODO:
    }
}

void MediumVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method_Silent();
    if (signal == IRadioMedium::radioAddedSignal)
        radioAdded(check_and_cast<IRadio *>(object));
    else if (signal == IRadioMedium::radioRemovedSignal)
        radioRemoved(check_and_cast<IRadio *>(object));
    else if (signal == IRadioMedium::transmissionAddedSignal)
        transmissionAdded(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::transmissionRemovedSignal)
        transmissionRemoved(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::transmissionStartedSignal)
        transmissionStarted(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::transmissionEndedSignal)
        transmissionEnded(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::receptionStartedSignal)
        receptionStarted(check_and_cast<IReception *>(object));
    else if (signal == IRadioMedium::receptionEndedSignal)
        receptionEnded(check_and_cast<IReception *>(object));
    else
        throw cRuntimeError("Unknown signal");
}

bool MediumVisualizerBase::isSignalPropagationInProgress(const ITransmission *transmission) const
{
    simtime_t now = simTime();
    ICommunicationCache *communicationCache = const_cast<ICommunicationCache *>(radioMedium->getCommunicationCache());
    const IMediumLimitCache *mediumLimitCache = radioMedium->getMediumLimitCache();
    const simtime_t transmissionStartTime = transmission->getStartTime();
    const simtime_t transmissionEndTime = transmission->getEndTime();
    const simtime_t interferenceEndTime = communicationCache->getCachedInterferenceEndTime(transmission);
    simtime_t maxPropagationTime = interferenceEndTime - transmissionEndTime - mediumLimitCache->getMaxTransmissionDuration() + signalPropagationAdditionalTime;
    return (transmissionStartTime <= now && now < transmissionStartTime + maxPropagationTime) ||
           (transmissionEndTime <= now && now < transmissionEndTime + maxPropagationTime);
}

bool MediumVisualizerBase::isSignalTransmissionInProgress(const ITransmission *transmission) const
{
    simtime_t now = simTime();
    return transmission->getStartTime() <= now && now < transmission->getEndTime();
}

bool MediumVisualizerBase::matchesTransmission(const ITransmission *transmission) const
{
    auto radio = check_and_cast<const cModule *>(transmission->getTransmitter());
    auto networkNode = getContainingNode(radio);
    if (!networkNodeFilter.matches(networkNode))
        return false;
    L3AddressResolver addressResolver;
    if (auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode)) {
        auto interfaceEntry = interfaceTable->getInterfaceByInterfaceModule(radio->getParentModule());
        if (!interfaceFilter.matches(interfaceEntry))
            return false;
    }
    auto packet = transmission->getPhyFrame() != nullptr ? transmission->getPhyFrame() : transmission->getMacFrame();
    return packetFilter.matches(packet);
}

} // namespace visualizer

} // namespace inet

