//
// Copyright (C) 2004 OpenSim Ltd.
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

#ifndef __INET_TCPGENERICSERVERTHREAD_H
#define __INET_TCPGENERICSERVERTHREAD_H

#include "inet/applications/tcpapp/TcpServerHostApp.h"
#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Example server thread, to be used with TcpServerHostApp.
 */
class INET_API TcpGenericServerThread : public TcpServerThreadBase
{
  public:
    TcpGenericServerThread() {}

    virtual void established() override;
    virtual void dataArrived(Packet *msg, bool urgent) override;
    virtual void timerExpired(cMessage *timer) override;
};

} // namespace inet

#endif

