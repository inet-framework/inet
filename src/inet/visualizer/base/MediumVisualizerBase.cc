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

#ifdef WITH_RADIO
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalAnalogModel.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalTransmission.h"
#endif // WITH_RADIO

namespace inet {

namespace visualizer {

#ifdef WITH_RADIO

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
        displaySpectrums = par("displaySpectrums");
        spectrumFigureWidth = par("spectrumFigureWidth");
        spectrumFigureHeight = par("spectrumFigureHeight");
        spectrumFigureInterpolationSize = par("spectrumFigureInterpolationSize");
        spectrumAutoFrequencyAxis = par("spectrumAutoFrequencyAxis");
        spectrumMinFrequency = Hz(par("spectrumMinFrequency"));
        spectrumMaxFrequency = Hz(par("spectrumMaxFrequency"));
        spectrumAutoPowerAxis = par("spectrumAutoPowerAxis");
        spectrumMinPower = WpHz(dBmWpMHz2WpHz(par("spectrumMinPower")));
        spectrumMaxPower = WpHz(dBmWpMHz2WpHz(par("spectrumMaxPower")));
        spectrumPlacementHint = parsePlacement(par("spectrumPlacementHint"));
        spectrumPlacementPriority = par("spectrumPlacementPriority");
        mediumPowerFunction = makeShared<SumFunction<WpHz, Domain<m, m, m, simsec, Hz>>>();
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
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        if (radioMedium != nullptr && displaySpectrums) {
            if (auto backgroundNoise = radioMedium->getBackgroundNoise())
                mediumPowerFunction->addElement(makeShared<BackgroundNoisePowerFunction>(backgroundNoise));
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
    if (!hasGUI()) return;
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
    auto interfaceEntry = getContainingNicModule(radio);
    if (!interfaceFilter.matches(interfaceEntry))
        return false;
    auto packet = transmission->getPacket();
    return packet == nullptr || packetFilter.matches(packet);
}

void MediumVisualizerBase::handleSignalAdded(const physicallayer::ITransmission *transmission)
{
    if (auto dimensionalTransmission = dynamic_cast<const DimensionalTransmission *>(transmission)) {
        auto transmissionPowerFunction = dimensionalTransmission->getPower();
        const auto& transmitterAntennaGainFunction = makeShared<AntennaGainFunction>(transmission->getTransmitter()->getAntenna()->getGain().get());
        const auto& pathLossFunction = makeShared<PathLossFunction>(radioMedium->getPathLoss());
        mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
        Point<m, m, m> startPosition(m(transmission->getStartPosition().x), m(transmission->getStartPosition().y), m(transmission->getStartPosition().z));
        const auto& startOrientation = transmission->getStartOrientation();
        const auto& propagatedTransmissionPowerFunction = makeShared<PropagatedTransmissionPowerFunction>(transmissionPowerFunction, startPosition, propagationSpeed);
        Ptr<const IFunction<WpHz, Domain<m, m, m, simsec, Hz>>> receptionPowerFunction;
        const Ptr<const IFunction<double, Domain<m, m, m, m, m, m, Hz>>>& obstacleLossFunction = radioMedium->getObstacleLoss() != nullptr ? makeShared<ObstacleLossFunction>(radioMedium->getObstacleLoss()) : nullptr;
        bool attenuateWithCenterFrequency = check_and_cast<const DimensionalAnalogModel *>(radioMedium->getAnalogModel())->par("attenuateWithCenterFrequency");
        if (attenuateWithCenterFrequency) {
            const auto& attenuationFunction = makeShared<SpaceDependentAttenuationFunction>(transmitterAntennaGainFunction, pathLossFunction, obstacleLossFunction, startPosition, startOrientation, propagationSpeed, dimensionalTransmission->getCenterFrequency());
            receptionPowerFunction = propagatedTransmissionPowerFunction->multiply(attenuationFunction);
        }
        else {
            Hz lower = dimensionalTransmission->getCenterFrequency() - dimensionalTransmission->getBandwidth() / 2;
            Hz upper = dimensionalTransmission->getCenterFrequency() + dimensionalTransmission->getBandwidth() / 2;
            Hz step = dimensionalTransmission->getBandwidth() / 2;
            const auto& attenuationFunction = makeShared<SpaceAndFrequencyDependentAttenuationFunction>(transmitterAntennaGainFunction, pathLossFunction, obstacleLossFunction, startPosition, startOrientation, propagationSpeed);
            const auto& approximatedAtteunuationFunction = makeShared<ApproximatedFunction<double, Domain<m, m, m, simsec, Hz>, 4, Hz>>(lower, upper, step, &AverageInterpolator<Hz, double>::singleton, attenuationFunction);
            receptionPowerFunction = propagatedTransmissionPowerFunction->multiply(approximatedAtteunuationFunction);
        }
        mediumPowerFunction->addElement(receptionPowerFunction);
        receptionPowerFunctions[transmission] = receptionPowerFunction;
    }
}

void MediumVisualizerBase::handleSignalRemoved(const physicallayer::ITransmission *transmission)
{
    auto it = receptionPowerFunctions.find(transmission);
    if (it != receptionPowerFunctions.end()) {
        mediumPowerFunction->removeElement(it->second);
        receptionPowerFunctions.erase(it);
    }
}

#endif // WITH_RADIO

} // namespace visualizer

} // namespace inet

