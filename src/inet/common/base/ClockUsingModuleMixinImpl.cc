//
// Copyright (C) OpenSim Ltd
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

#include "inet/common/base/ClockUsingModuleMixinImpl.h"
#include "inet/queueing/base/PacketGateBase.h"
#include "inet/queueing/base/PacketProcessorBase.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace inet {

#ifdef WITH_CLOCK_SUPPORT
template class ClockUsingModuleMixin<cSimpleModule>;

#ifdef WITH_QUEUEING
template class ClockUsingModuleMixin<queueing::PacketProcessorBase>;
template class ClockUsingModuleMixin<queueing::PacketPusherBase>;
template class ClockUsingModuleMixin<queueing::PacketGateBase>;
#endif // #ifdef WITH_QUEUEING

#endif // #ifdef WITH_CLOCK_SUPPORT

} // namespace inet
