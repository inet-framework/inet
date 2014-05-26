//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <omnetpp.h>


/**
 * Packet sink; see NED file for more info.
 */
class FifoSink : public cSimpleModule
{
  private:
    simsignal_t lifetimeSignal;
    double timeScale;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module( FifoSink );

void FifoSink::initialize()
{
    timeScale = par("timeScale").doubleValue();
    lifetimeSignal = registerSignal("lifetime");
}

void FifoSink::handleMessage(cMessage *msg)
{
    simtime_t lifetime = simTime() - msg->getCreationTime();
    EV << "Received " << msg->getName() << ", lifetime: " << lifetime << "s" << endl;
    cTimestampedValue tmp(timeScale*simTime(), lifetime);
    emit(lifetimeSignal, &tmp);
    delete msg;
}
