//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

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

