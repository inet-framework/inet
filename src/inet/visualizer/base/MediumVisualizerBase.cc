//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/MediumVisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/DimensionalTransmission.h"
#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

namespace inet {

namespace visualizer {

#ifdef INET_WITH_PHYSICALLAYERWIRELESSCOMMON

using namespace inet::physicallayer;

void MediumVisualizerBase::preDelete(cComponent *root)
{
    // NOTE: lookup the medium module again because it may have been deleted first
    auto radioMediumModule = findModuleFromPar<cModule>(par("mediumModule"), this);
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
        packetFilter.setExpression(par("packetFilter").objectValue());
        displaySignals = par("displaySignals");
        signalColorSet.parseColors(par("signalColor"));
        signalPropagationAnimationSpeed = par("signalPropagationAnimationSpeed");
        signalPropagationAnimationTime = par("signalPropagationAnimationTime");
        signalPropagationAdditionalTime = par("signalPropagationAdditionalTime");
        signalTransmissionAnimationSpeed = par("signalTransmissionAnimationSpeed");
        signalTransmissionAnimationTime = par("signalTransmissionAnimationTime");
        const char *s = par("signalAnimationSpeedChangeTimeMode");
        if (!strcmp(s, "simulationTime"))
            signalAnimationSpeedChangeTimeMode = AnimationPosition::SIMULATION_TIME;
        else if (!strcmp(s, "animationTime"))
            signalAnimationSpeedChangeTimeMode = AnimationPosition::ANIMATION_TIME;
        else if (!strcmp(s, "realTime"))
            signalAnimationSpeedChangeTimeMode = AnimationPosition::REAL_TIME;
        else
            throw cRuntimeError("Unknown signalAnimationSpeedChangeTimeMode: %s", s);
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
        autoPowerAxis = par("autoPowerAxis");
        signalMinPower = W(dBmW2mW(par("signalMinPower")));
        signalMaxPower = W(dBmW2mW(par("signalMaxPower")));
        signalMinPowerDensity = WpHz(dBmWpMHz2WpHz(par("signalMinPowerDensity")));
        signalMaxPowerDensity = WpHz(dBmWpMHz2WpHz(par("signalMaxPowerDensity")));
        autoTimeAxis = par("autoTimeAxis");
        signalMinTime = par("signalMinTime");
        signalMaxTime = par("signalMaxTime");
        autoFrequencyAxis = par("autoFrequencyAxis");
        signalMinFrequency = Hz(par("signalMinFrequency"));
        signalMaxFrequency = Hz(par("signalMaxFrequency"));
        displayMainPowerDensityMap = par("displayMainPowerDensityMap");
        mainPowerDensityMapPixmapDensity = par("mainPowerDensityMapPixmapDensity");
        mainPowerDensityMapMinX = par("mainPowerDensityMapMinX");
        mainPowerDensityMapMaxX = par("mainPowerDensityMapMaxX");
        mainPowerDensityMapMinY = par("mainPowerDensityMapMinY");
        mainPowerDensityMapMaxY = par("mainPowerDensityMapMaxY");
        const auto& displayString = visualizationTargetModule->getDisplayString();
        if (std::isnan(mainPowerDensityMapMaxX))
            mainPowerDensityMapMaxX = atof(displayString.getTagArg("bgb", 0));
        if (std::isnan(mainPowerDensityMapMaxY))
            mainPowerDensityMapMaxY = atof(displayString.getTagArg("bgb", 1));
        mainPowerDensityMapZ = par("mainPowerDensityMapZ");
        mainPowerDensityMapFigureXTickCount = par("mainPowerDensityMapFigureXTickCount");
        mainPowerDensityMapFigureYTickCount = par("mainPowerDensityMapFigureYTickCount");
        displayPowerDensityMaps = par("displayPowerDensityMaps");
        powerDensityMapMode = par("powerDensityMapMode");
        powerDensityMapPixelMode = par("powerDensityMapPixelMode");
        powerDensityMapApproximationSize = par("powerDensityMapApproximationSize");
        powerDensityMapCenterFrequency = Hz(par("powerDensityMapCenterFrequency"));
        powerDensityMapBandwidth = Hz(par("powerDensityMapBandwidth"));
        powerDensityMapFigureWidth = par("powerDensityMapFigureWidth");
        powerDensityMapFigureHeight = par("powerDensityMapFigureHeight");
        powerDensityMapPixmapWidth = par("powerDensityMapPixmapWidth");
        powerDensityMapPixmapHeight = par("powerDensityMapPixmapHeight");
        powerDensityMapZ = par("powerDensityMapZ");
        powerDensityMapFigureXTickCount = par("powerDensityMapFigureXTickCount");
        powerDensityMapFigureYTickCount = par("powerDensityMapFigureYTickCount");
        displaySpectrums = par("displaySpectrums");
        spectrumMode = par("spectrumMode");
        spectrumFigureWidth = par("spectrumFigureWidth");
        spectrumFigureHeight = par("spectrumFigureHeight");
        spectrumFigureXTickCount = par("spectrumFigureXTickCount");
        spectrumFigureYTickCount = par("spectrumFigureYTickCount");
        spectrumFigureInterpolationSize = par("spectrumFigureInterpolationSize");
        spectrumPlacementHint = parsePlacement(par("spectrumPlacementHint"));
        spectrumPlacementPriority = par("spectrumPlacementPriority");
        displaySpectrograms = par("displaySpectrograms");
        spectrogramMode = par("spectrogramMode");
        spectrogramPixelMode = par("spectrogramPixelMode");
        spectrogramFigureWidth = par("spectrogramFigureWidth");
        spectrogramFigureHeight = par("spectrogramFigureHeight");
        spectrogramPixmapWidth = par("spectrogramPixmapWidth");
        spectrogramPixmapHeight = par("spectrogramPixmapHeight");
        spectrogramFigureXTickCount = par("spectrogramFigureXTickCount");
        spectrogramFigureYTickCount = par("spectrogramFigureYTickCount");
        spectrogramPlacementHint = parsePlacement(par("spectrogramPlacementHint"));
        spectrogramPlacementPriority = par("spectrogramPlacementPriority");
        mediumPowerDensityFunction = makeShared<SummedFunction<WpHz, Domain<m, m, m, simsec, Hz>>>();
        radioMedium = findModuleFromPar<IRadioMedium>(par("mediumModule"), this);
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
        if (radioMedium != nullptr && (displayMainPowerDensityMap || displayPowerDensityMaps || displaySpectrums || displaySpectrograms)) {
            if (auto backgroundNoise = radioMedium->getBackgroundNoise())
                mediumPowerDensityFunction->addElement(makeShared<BackgroundNoisePowerFunction>(backgroundNoise));
            pathLossFunction = makeShared<PathLossFunction>(radioMedium->getPathLoss());
            if (radioMedium->getObstacleLoss() != nullptr) {
                if (radioMedium->getMediumLimitCache()->getMaxSpeed() == mps(0))
                    obstacleLossFunction = makeShared<MemoizedFunction<double, Domain<m, m, m, m, m, m, Hz>>>(makeShared<ObstacleLossFunction>(radioMedium->getObstacleLoss()));
                else
                    obstacleLossFunction = makeShared<ObstacleLossFunction>(radioMedium->getObstacleLoss());
            }
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
    if (!strcmp(name, "networkNodeFilter"))
        networkNodeFilter.setPattern(par("nodeFilter"));
    else if (!strcmp(name, "interfaceFilter"))
        interfaceFilter.setPattern(par("interfaceFilter"));
    else if (!strcmp(name, "packetFilter"))
        packetFilter.setExpression(par("packetFilter").objectValue());
    else if (!strcmp(name, "signalPropagationAnimationSpeed"))
        signalPropagationAnimationSpeed = par("signalPropagationAnimationSpeed");
    else if (!strcmp(name, "signalTransmissionAnimationSpeed"))
        signalTransmissionAnimationSpeed = par("signalTransmissionAnimationSpeed");
    else if (!strcmp(name, "powerDensityMapCenterFrequency"))
        powerDensityMapCenterFrequency = Hz(par("powerDensityMapCenterFrequency"));
    else if (!strcmp(name, "powerDensityMapBandwidth"))
        powerDensityMapBandwidth = Hz(par("powerDensityMapBandwidth"));
    // TODO
}

void MediumVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

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
    auto networkInterface = findContainingNicModule(radio);
    if (networkInterface != nullptr && !interfaceFilter.matches(networkInterface))
        return false;
    auto packet = transmission->getPacket();
    return packet == nullptr || packetFilter.matches(packet);
}

void MediumVisualizerBase::handleSignalAdded(const physicallayer::ITransmission *transmission)
{
    if (displayMainPowerDensityMap || displayPowerDensityMaps || displaySpectrums || displaySpectrograms) {
        auto dimensionalTransmission = check_and_cast<const DimensionalTransmission *>(transmission);
        auto transmissionPowerFunction = dimensionalTransmission->getPower();
        const auto& transmitterAntennaGain = transmission->getTransmitter()->getAntenna()->getGain();
        bool isotropicAntenna = transmitterAntennaGain->getMaxGain() == 1 && transmitterAntennaGain->getMinGain() == 1;
        const auto& transmitterAntennaGainFunction = !isotropicAntenna ? makeShared<AntennaGainFunction>(transmitterAntennaGain.get()) : nullptr;
        mps propagationSpeed = radioMedium->getPropagation()->getPropagationSpeed();
        Point<m, m, m> startPosition(m(transmission->getStartPosition().x), m(transmission->getStartPosition().y), m(transmission->getStartPosition().z));
        auto startTime = transmission->getStartTime();
        const auto& startOrientation = transmission->getStartOrientation();
        const auto& propagatedTransmissionPowerFunction = makeShared<PropagatedTransmissionPowerFunction>(transmissionPowerFunction, startPosition, propagationSpeed);
        Ptr<const IFunction<WpHz, Domain<m, m, m, simsec, Hz>>> signalPowerDensityFunction;
        bool attenuateWithCenterFrequency = check_and_cast<const DimensionalAnalogModel *>(radioMedium->getAnalogModel())->par("attenuateWithCenterFrequency");
        if (attenuateWithCenterFrequency) {
            const auto& attenuationFunction = makeShared<SpaceDependentAttenuationFunction>(transmitterAntennaGainFunction, pathLossFunction, obstacleLossFunction, startPosition, startOrientation, propagationSpeed, dimensionalTransmission->getCenterFrequency());
            signalPowerDensityFunction = propagatedTransmissionPowerFunction->multiply(attenuationFunction);
        }
        else {
            Hz lower = dimensionalTransmission->getCenterFrequency() - dimensionalTransmission->getBandwidth() / 2;
            Hz upper = dimensionalTransmission->getCenterFrequency() + dimensionalTransmission->getBandwidth() / 2;
            Hz step = dimensionalTransmission->getBandwidth() / 2;
            const auto& attenuationFunction = makeShared<SpaceAndFrequencyDependentAttenuationFunction>(transmitterAntennaGainFunction, pathLossFunction, obstacleLossFunction, startPosition, startOrientation, propagationSpeed);
            const auto& approximatedAtteunuationFunction = makeShared<ApproximatedFunction<double, Domain<m, m, m, simsec, Hz>, 4, Hz>>(lower, upper, step, &AverageInterpolator<Hz, double>::singleton, attenuationFunction);
            signalPowerDensityFunction = propagatedTransmissionPowerFunction->multiply(approximatedAtteunuationFunction);
        }
        mediumPowerDensityFunction->addElement(signalPowerDensityFunction);
        signalPowerDensityFunctions[transmission->getId()] = signalPowerDensityFunction;
        for (auto it : noisePowerDensityFunctions)
            it.second->addElement(signalPowerDensityFunction);
        auto noisePowerFunction = makeShared<SummedFunction<WpHz, Domain<m, m, m, simsec, Hz>>>();
        for (auto elementFunction : mediumPowerDensityFunction->getElements())
            if (elementFunction != signalPowerDensityFunction)
                noisePowerFunction->addElement(elementFunction);
        noisePowerDensityFunctions[transmission->getId()] = noisePowerFunction;
        if (autoPowerAxis) {
            auto l = concat(startPosition, transmissionPowerFunction->getDomain().getLower());
            auto u = concat(startPosition, transmissionPowerFunction->getDomain().getUpper());
            Interval<m, m, m, simsec, Hz> domain(l, u, 0b11110, 0b11100, 0b11100);
            mediumPowerDensityFunction->partition(domain, [&] (const Interval<m, m, m, simsec, Hz>& i, const IFunction<WpHz, Domain<m, m, m, simsec, Hz>> *f) {
                WpHz minPowerDensity = f->getMin(i);
                if (minPowerDensity > WpHz(0))
                    signalMinPowerDensity = std::min(signalMinPowerDensity, minPowerDensity);
                signalMaxPowerDensity = std::max(signalMaxPowerDensity, f->getMax(i));
            });
        }
        if (autoPowerAxis || autoTimeAxis || autoFrequencyAxis) {
            transmissionPowerFunction->partition(transmissionPowerFunction->getDomain(), [&] (const Interval<simsec, Hz>& i, const IFunction<WpHz, Domain<simsec, Hz>> *f) {
                if (autoPowerAxis) {
                    WpHz minPowerDensity = f->getMin(i);
                    if (minPowerDensity > WpHz(0))
                        signalMinPowerDensity = std::min(signalMinPowerDensity, minPowerDensity);
                    signalMaxPowerDensity = std::max(signalMaxPowerDensity, f->getMax(i));
                }
                if (auto constantFunction = dynamic_cast<const ConstantFunction<WpHz, Domain<simsec, Hz>> *>(f)) {
                    if (constantFunction->getConstantValue() == WpHz(0))
                        return;
                }
                if (autoTimeAxis) {
                    signalMinTime = std::min(signalMinTime.dbl(), (simsec(std::get<0>(i.getLower())).get() - startTime).dbl());
                    signalMaxTime = std::max(signalMaxTime.dbl(), (simsec(std::get<0>(i.getUpper())).get() - startTime).dbl());
                }
                if (autoFrequencyAxis) {
                    signalMinFrequency = std::min(signalMinFrequency, std::get<1>(i.getLower()));
                    signalMaxFrequency = std::max(signalMaxFrequency, std::get<1>(i.getUpper()));
                }
            });
        }
        if (std::isnan(powerDensityMapCenterFrequency.get()))
            powerDensityMapCenterFrequency = (signalMaxFrequency + signalMinFrequency) / 2;
        if (std::isnan(powerDensityMapBandwidth.get()))
            powerDensityMapBandwidth = signalMaxFrequency - signalMinFrequency;
    }
}

void MediumVisualizerBase::handleSignalRemoved(const physicallayer::ITransmission *transmission)
{
    if (displayMainPowerDensityMap || displayPowerDensityMaps || displaySpectrums || displaySpectrograms) {
        auto it = signalPowerDensityFunctions.find(transmission->getId());
        if (it != signalPowerDensityFunctions.end()) {
            mediumPowerDensityFunction->removeElement(it->second);
            noisePowerDensityFunctions.erase(it->first);
            signalPowerDensityFunctions.erase(it);
        }
    }
}

void MediumVisualizerBase::handleSignalArrivalStarted(const physicallayer::IReception *reception)
{
    if (displayMainPowerDensityMap || displayPowerDensityMaps || displaySpectrums || displaySpectrograms) {
        auto dimensionalReception = check_and_cast<const DimensionalReception *>(reception);
        auto startTime = reception->getStartTime();
        auto receptionPowerFunction = dimensionalReception->getPower();
        if (autoPowerAxis || autoTimeAxis || autoFrequencyAxis) {
            receptionPowerFunction->partition(receptionPowerFunction->getDomain(), [&] (const Interval<simsec, Hz>& i, const IFunction<WpHz, Domain<simsec, Hz>> *f) {
                if (autoPowerAxis) {
                    WpHz minPowerDensity = f->getMin(i);
                    if (minPowerDensity > WpHz(0))
                        signalMinPowerDensity = std::min(signalMinPowerDensity, minPowerDensity);
                    signalMaxPowerDensity = std::max(signalMaxPowerDensity, f->getMax(i));
                }
                if (auto constantFunction = dynamic_cast<const ConstantFunction<WpHz, Domain<simsec, Hz>> *>(f)) {
                    if (constantFunction->getConstantValue() == WpHz(0))
                        return;
                }
                if (autoTimeAxis) {
                    signalMinTime = std::min(signalMinTime.dbl(), (simsec(std::get<0>(i.getLower())).get() - startTime).dbl());
                    signalMaxTime = std::max(signalMaxTime.dbl(), (simsec(std::get<0>(i.getUpper())).get() - startTime).dbl());
                }
                if (autoFrequencyAxis) {
                    signalMinFrequency = std::min(signalMinFrequency, std::get<1>(i.getLower()));
                    signalMaxFrequency = std::max(signalMaxFrequency, std::get<1>(i.getUpper()));
                }
            });
        }
    }
}

#endif // INET_WITH_PHYSICALLAYERWIRELESSCOMMON

} // namespace visualizer

} // namespace inet

