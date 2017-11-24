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

#include "inet/networklayer/contract/IARP.h"

namespace inet {

Register_Abstract_Class(IArp::Notification);

const simsignal_t IArp::initiatedARPResolutionSignal = cComponent::registerSignal("initiatedARPResolution");
const simsignal_t IArp::completedARPResolutionSignal = cComponent::registerSignal("completedARPResolution");
const simsignal_t IArp::failedARPResolutionSignal = cComponent::registerSignal("failedARPResolution");

} // namespace inet

