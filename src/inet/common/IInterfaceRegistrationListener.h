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

#ifndef __INET_IINTERFACEREGISTRATIONLISTENER_H
#define __INET_IINTERFACEREGISTRATIONLISTENER_H

#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

INET_API void registerInterface(const InterfaceEntry& interface, cGate *in, cGate *out);

class INET_API IInterfaceRegistrationListener
{
  public:
    virtual void handleRegisterInterface(const InterfaceEntry &interface, cGate *out, cGate *in) = 0;
};

} // namespace inet

#endif // ifndef __INET_IINTERFACEREGISTRATIONLISTENER_H

