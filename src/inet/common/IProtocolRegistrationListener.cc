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

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

void registerService(const Protocol& protocol, cGate *in, ServicePrimitive servicePrimitive)
{
    auto out = in->getPathStartGate();
    IProtocolRegistrationListener *protocolRegistration = dynamic_cast<IProtocolRegistrationListener *>(out->getOwner());
    if (protocolRegistration != nullptr)
        protocolRegistration->handleRegisterService(protocol, out, servicePrimitive);
}

void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationIn, cGate *responseIn, cGate *confirmIn)
{
    if (requestIn != nullptr)
        registerService(protocol, requestIn, SP_REQUEST);
    if (indicationIn != nullptr)
        registerService(protocol, indicationIn, SP_INDICATION);
    if (responseIn != nullptr)
        registerService(protocol, responseIn, SP_RESPONSE);
    if (confirmIn != nullptr)
        registerService(protocol, confirmIn, SP_CONFIRM);
}

void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationIn)
{
    registerService(protocol, requestIn, indicationIn, requestIn, indicationIn);
}

void registerProtocol(const Protocol& protocol, cGate *out, ServicePrimitive servicePrimitive)
{
    auto in = out->getPathEndGate();
    IProtocolRegistrationListener *protocolRegistration = dynamic_cast<IProtocolRegistrationListener *>(in->getOwner());
    if (protocolRegistration != nullptr)
        protocolRegistration->handleRegisterProtocol(protocol, in, servicePrimitive);
}

void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationOut)
{
    registerProtocol(protocol, requestOut, indicationOut, requestOut, indicationOut);
}

void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationOut, cGate *responseOut, cGate *confirmOut)
{
    if (requestOut != nullptr)
        registerProtocol(protocol, requestOut, SP_REQUEST);
    if (indicationOut != nullptr)
        registerProtocol(protocol, indicationOut, SP_INDICATION);
    if (responseOut != nullptr)
        registerProtocol(protocol, responseOut, SP_RESPONSE);
    if (confirmOut != nullptr)
        registerProtocol(protocol, confirmOut, SP_CONFIRM);
}

} // namespace inet

