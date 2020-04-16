//
// Copyright (C) 2013 OpenSim Ltd
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

#include "inet/transportlayer/base/TransportProtocolBase.h"

namespace inet {

bool TransportProtocolBase::isUpperMessage(cMessage *msg)
{
    return msg->arrivedOn("appIn");
}

bool TransportProtocolBase::isLowerMessage(cMessage *msg)
{
    return msg->arrivedOn("ipIn");
}

bool TransportProtocolBase::isInitializeStage(int stage)
{
    return stage == INITSTAGE_TRANSPORT_LAYER;
}

bool TransportProtocolBase::isModuleStartStage(int stage)
{
    return stage == ModuleStartOperation::STAGE_TRANSPORT_LAYER;
}

bool TransportProtocolBase::isModuleStopStage(int stage)
{
    return stage == ModuleStopOperation::STAGE_TRANSPORT_LAYER;
}

} // namespace inet

