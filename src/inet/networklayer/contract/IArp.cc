/*
 * Copyright (C) 2014 OpenSim Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "inet/networklayer/contract/IArp.h"

namespace inet {

Register_Abstract_Class(IArp::Notification);

const simsignal_t IArp::arpResolutionInitiatedSignal = cComponent::registerSignal("arpResolutionInitiated");
const simsignal_t IArp::arpResolutionCompletedSignal = cComponent::registerSignal("arpResolutionCompleted");
const simsignal_t IArp::arpResolutionFailedSignal = cComponent::registerSignal("arpResolutionFailed");

} // namespace inet

