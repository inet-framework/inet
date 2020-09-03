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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/base/ClockUserModuleMixinImpl.h"
#include "inet/common/lifecycle/OperationalMixin.h"
#ifdef WITH_QUEUEING
#include "inet/queueing/base/PacketGateBase.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/base/PacketPusherBase.h"
#endif // #ifdef WITH_QUEUEING

namespace inet {

#ifdef WITH_CLOCK_SUPPORT
template class ClockUserModuleMixin<cSimpleModule>;
template class ClockUserModuleMixin<ApplicationBase>;

#ifdef WITH_QUEUEING
template class ClockUserModuleMixin<queueing::PacketProcessorBase>;
template class ClockUserModuleMixin<queueing::PacketPusherBase>;
template class ClockUserModuleMixin<queueing::PacketGateBase>;
template class ClockUserModuleMixin<OperationalMixin<queueing::PacketProcessorBase>>;
#endif // #ifdef WITH_QUEUEING

#endif // #ifdef WITH_CLOCK_SUPPORT

} // namespace inet

