//
// Copyright (C) 2012 Opensim Ltd
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

#ifndef __INET_IPROTOCOLREGISTRATIONLISTENER_H
#define __INET_IPROTOCOLREGISTRATIONLISTENER_H

#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"

namespace inet {

/**
 * Registers a service primitive (SDU processing) at the given gate.
 *
 * For example, IP receives service requests on upperLayerIn from transport
 * protocols and sends service indications (e.g. TCP/UDP protocol indiciations)
 * on upperLayerOut.
 *
 * For another example, Ethernet receives service requests on upperLayerIn from
 * network protocols and sends service indications (e.g. IP protocol indications)
 * on upperLayerOut.
 */
INET_API void registerService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut);
INET_API void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut);

/**
 * Registers a protocol primitive (PDU processing) at the given gate.
 *
 * For example, IP receives protocol indiciations on lowerLayerIn and sends IP
 * protocol packets (e.g. Ethernet service requests) on lowerLayerOut.
 *
 * For another example, Ethernet receives protocol indications on lowerLayerIn
 * and sends protocol packets (e.g. transmission service requests) on lowerLayerOut.
 */
INET_API void registerProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive);
INET_API void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn);
INET_API void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn);

/**
 * This interface defines methods that are called during protocol service
 * primitve registration.
 */
class INET_API IProtocolRegistrationListener
{
  public:
    virtual void handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) = 0;
    virtual void handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive) = 0;
};

} // namespace inet

#endif // ifndef __INET_IPROTOCOLREGISTRATIONLISTENER_H

