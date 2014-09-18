//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPGENERICSRVTHREAD_H
#define __INET_TCPGENERICSRVTHREAD_H

#include "inet/common/INETDefs.h"

#include "inet/applications/tcpapp/TCPSrvHostApp.h"

namespace inet {

/**
 * Example server thread, to be used with TCPSrvHostApp.
 */
class INET_API TCPGenericSrvThread : public TCPServerThreadBase
{
  public:
    TCPGenericSrvThread() {}

    virtual void established();
    virtual void dataArrived(cMessage *msg, bool urgent);
    virtual void timerExpired(cMessage *timer);
};

} // namespace inet

#endif // ifndef __INET_TCPGENERICSRVTHREAD_H

