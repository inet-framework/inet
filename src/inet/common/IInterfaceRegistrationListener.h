//
// Copyright (C) 2012 OpenSim Ltd.
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

#ifndef __INET_IINTERFACEREGISTRATIONLISTENER_H
#define __INET_IINTERFACEREGISTRATIONLISTENER_H

#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

INET_API void registerInterface(const NetworkInterface& interface, cGate *in, cGate *out);

class INET_API IInterfaceRegistrationListener
{
  public:
    virtual void handleRegisterInterface(const NetworkInterface &interface, cGate *out, cGate *in) = 0;
};

} // namespace inet

#endif

