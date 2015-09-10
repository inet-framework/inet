//
// Copyright 2012 OpenSim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// @author Zoltan Bojthe
//


#include "inet/common/INETDefs.h"

namespace inet {

class INET_API QQ : public cSimpleModule
{
  protected:
    cQueue queue;
    cGate *outGate;
    cMessage *timer;

  public:
    QQ() : outGate(0), timer(0) {}
    ~QQ();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(QQ);



void QQ::initialize()
{
    outGate = gate("out");
    timer = new cMessage("TIMER");
}

void QQ::handleMessage(cMessage *msg)
{
    if (msg != timer)
        queue.insert(msg);

    cChannel *ch = outGate->findTransmissionChannel();

    if (outGate->isConnected())
    {
        if (ch && ch->isBusy())
        {
            cancelEvent(timer);
            scheduleAt(ch->getTransmissionFinishTime(), timer);
        }
        else
        {
            msg = check_and_cast<cMessage *>(queue.pop());
            send(msg, outGate);
        }
    }
}

QQ::~QQ()
{
    cancelAndDelete(timer);
}

} // namespace inet

