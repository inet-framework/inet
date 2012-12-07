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
 * Generates messages or jobs; see NED file for more info.
 */
class FifoSource : public cSimpleModule
{
  private:
    cMessage *sendMessageEvent;

  public:
     FifoSource();
     virtual ~FifoSource();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(FifoSource);

FifoSource::FifoSource()
{
    sendMessageEvent = NULL;
}

FifoSource::~FifoSource()
{
    cancelAndDelete(sendMessageEvent);
}

void FifoSource::initialize()
{
    sendMessageEvent = new cMessage("sendMessageEvent");
    scheduleAt(simTime(), sendMessageEvent);
}

void FifoSource::handleMessage(cMessage *msg)
{
    ASSERT(msg==sendMessageEvent);

    cMessage *job = new cMessage("job");
    send(job, "out");

    scheduleAt(simTime()+par("sendIaTime").doubleValue(), sendMessageEvent);
}
