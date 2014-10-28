//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_TCPSESSIONAPP_H
#define __INET_TCPSESSIONAPP_H

#include <vector>

#include "INETDefs.h"
#include "LifecycleOperation.h"
#include "TCPAppBase.h"
#include "NodeStatus.h"

/**
 * Single-connection TCP application.
 */
class INET_API TCPSessionApp : public TCPAppBase
{
  protected:
    // parameters
    struct Command
    {
        simtime_t tSend;
        long numBytes;
        Command(simtime_t t, long n) {tSend=t; numBytes=n;}
    };
    typedef std::vector<Command> CommandVector;
    CommandVector commands;

    bool activeOpen;
    simtime_t tOpen;
    simtime_t tSend;
    simtime_t tClose;
    int sendBytes;

    // state
    int commandIndex;
    cMessage *timeoutMsg;
    NodeStatus *nodeStatus;

  public:
    TCPSessionApp();
    virtual ~TCPSessionApp();

  protected:
    virtual bool isNodeUp();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);

    virtual int numInitStages() const { return 4; }
    virtual void initialize(int stage);
    virtual void finish();

    virtual void parseScript(const char *script);
    virtual cPacket *createDataPacket(long sendBytes);
    virtual void sendData();

    virtual void handleTimer(cMessage *msg);
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
};

#endif

