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

#include "inet/common/lifecycle/OperationalMixinImpl.h"
#include "inet/protocol/transceiver/base/PacketReceiverBase.h"
#include "inet/protocol/transceiver/base/PacketTransmitterBase.h"

namespace inet {

template class OperationalMixin<cSimpleModule>;

#ifdef WITH_PROTOCOL
template class OperationalMixin<PacketTransmitterBase>;
template class OperationalMixin<PacketReceiverBase>;
#endif // #ifdef WITH_PROTOCOL

} // namespace inet

