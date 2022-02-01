//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

