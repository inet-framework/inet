//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"

namespace inet {

namespace physicallayer {

simsignal_t IRadioMedium::radioAddedSignal = cComponent::registerSignal("radioAdded");
simsignal_t IRadioMedium::radioRemovedSignal = cComponent::registerSignal("radioRemoved");

simsignal_t IRadioMedium::signalAddedSignal = cComponent::registerSignal("signalAdded");
simsignal_t IRadioMedium::signalRemovedSignal = cComponent::registerSignal("signalRemoved");

simsignal_t IRadioMedium::signalDepartureStartedSignal = cComponent::registerSignal("signalDepartureStarted");
simsignal_t IRadioMedium::signalDepartureEndedSignal = cComponent::registerSignal("signalDepartureEnded");

simsignal_t IRadioMedium::signalArrivalStartedSignal = cComponent::registerSignal("signalArrivalStarted");
simsignal_t IRadioMedium::signalArrivalEndedSignal = cComponent::registerSignal("signalArrivalEnded");

} // namespace physicallayer

} // namespace inet

