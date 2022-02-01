//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
            rescheduleAt(ch->getTransmissionFinishTime(), timer);
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

