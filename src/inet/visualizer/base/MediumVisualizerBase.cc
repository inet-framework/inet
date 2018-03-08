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

using namespace inet::physicallayer;

MediumVisualizerBase::~MediumVisualizerBase()
{
    // NOTE: lookup the medium module again because it may have been deleted first
    auto radioMediumModule = getModuleFromPar<cModule>(par("mediumModule"), this, false);
    if (radioMediumModule != nullptr) {
        radioMediumModule->unsubscribe(IRadioMedium::radioAddedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::radioRemovedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::signalAddedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::signalRemovedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::signalDepartureStartedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::signalDepartureEndedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::signalArrivalStartedSignal, this);
        radioMediumModule->unsubscribe(IRadioMedium::signalArrivalEndedSignal, this);
    }
}

void MediumVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeFilter.setPattern(par("nodeFilter"));
        interfaceFilter.setPattern(par("interfaceFilter"));
        packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
        displaySignals = par("displaySignals");
        signalColorSet.parseColors(par("signalColor"));
        signalPropagationAnimationSpeed = par("signalPropagationAnimationSpeed");
        signalPropagationAnimationTime = par("signalPropagationAnimationTime");
        signalPropagationAdditionalTime = par("signalPropagationAdditionalTime");
        signalTransmissionAnimationSpeed = par("signalTransmissionAnimationSpeed");
        signalTransmissionAnimationTime = par("signalTransmissionAnimationTime");
        signalAnimationSpeedChangeTime = par("signalAnimationSpeedChangeTime");
        displaySignalDepartures = par("displaySignalDepartures");
        displaySignalArrivals = par("displaySignalArrivals");
        signalDeparturePlacementHint = parsePlacement(par("signalDeparturePlacementHint"));
        signalArrivalPlacementHint = parsePlacement(par("signalArrivalPlacementHint"));
        signalDeparturePlacementPriority = par("signalDeparturePlacementPriority");
        signalArrivalPlacementPriority = par("signalArrivalPlacementPriority");
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
            radioMediumModule->subscribe(IRadioMedium::signalAddedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::signalRemovedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::signalDepartureStartedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::signalDepartureEndedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::signalArrivalStartedSignal, this);
            radioMediumModule->subscribe(IRadioMedium::signalArrivalEndedSignal, this);
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
            packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
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
        handleRadioAdded(check_and_cast<IRadio *>(object));
    else if (signal == IRadioMedium::radioRemovedSignal)
        handleRadioRemoved(check_and_cast<IRadio *>(object));
    else if (signal == IRadioMedium::signalAddedSignal)
        handleSignalAdded(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::signalRemovedSignal)
        handleSignalRemoved(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::signalDepartureStartedSignal)
        handleSignalDepartureStarted(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::signalDepartureEndedSignal)
        handleSignalDepartureEnded(check_and_cast<ITransmission *>(object));
    else if (signal == IRadioMedium::signalArrivalStartedSignal)
        handleSignalArrivalStarted(check_and_cast<IReception *>(object));
    else if (signal == IRadioMedium::signalArrivalEndedSignal)
        handleSignalArrivalEnded(check_and_cast<IReception *>(object));
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
    auto radio = dynamic_cast<const cModule *>(transmission->getTransmitter());
    if (!radio)
        return false;
    auto networkNode = getContainingNode(radio);
    if (!networkNodeFilter.matches(networkNode))
        return false;
    L3AddressResolver addressResolver;
    if (auto interfaceTable = addressResolver.findInterfaceTableOf(networkNode)) {
        auto interfaceEntry = interfaceTable->getInterfaceByInterfaceModule(radio->getParentModule());
        if (!interfaceFilter.matches(interfaceEntry))
            return false;
    }
    return packetFilter.matches(transmission->getPacket());
}

} // namespace visualizer

} // namespace inet

