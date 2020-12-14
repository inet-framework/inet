//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/clock/ClockUserModuleMixinImpl.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#ifdef INET_WITH_QUEUEING
#include "inet/queueing/base/ActivePacketSourceBase.h"
#include "inet/queueing/base/PacketGateBase.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/base/PacketPusherBase.h"
#include "inet/queueing/base/PassivePacketSourceBase.h"
#endif // #ifdef INET_WITH_QUEUEING

namespace inet {

#ifdef INET_WITH_CLOCK
template class ClockUserModuleMixin<cSimpleModule>;
template class ClockUserModuleMixin<ApplicationBase>;

#ifdef INET_WITH_QUEUEING
template class ClockUserModuleMixin<queueing::ActivePacketSourceBase>;
template class ClockUserModuleMixin<queueing::PacketGateBase>;
template class ClockUserModuleMixin<queueing::PacketProcessorBase>;
template class ClockUserModuleMixin<queueing::PacketPusherBase>;
template class ClockUserModuleMixin<queueing::PassivePacketSourceBase>;
template class ClockUserModuleMixin<OperationalMixin<queueing::PacketProcessorBase>>;
#endif // #ifdef INET_WITH_QUEUEING

#endif // #ifdef INET_WITH_CLOCK

} // namespace inet

