//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITCPSERVERSOCKETIO_H
#define __INET_ITCPSERVERSOCKETIO_H

#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet {

class INET_API ITcpServerSocketIo
{
  public:
    virtual TcpSocket *getSocket() = 0;
    virtual void acceptSocket(TcpAvailableInfo *availableInfo) = 0;
    virtual void close() = 0;
    virtual void deleteModule() = 0;
};

} // namespace inet

#endif

