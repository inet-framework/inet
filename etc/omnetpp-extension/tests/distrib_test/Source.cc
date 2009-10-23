//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include <omnetpp.h>

namespace fifo {

/**
 * Generates messages or jobs; see NED file for more info.
 */
class Source : public cSimpleModule
{
  private:
    cMessage *sendMessageEvent;
    cStdDev jobStats;	// job interarrival time statistics
    cOutVector jobIat;	// job interarrival time vector

  public:
     Source();
     virtual ~Source();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

Define_Module(Source);

Source::Source()
{
    sendMessageEvent = NULL;
}

Source::~Source()
{
    cancelAndDelete(sendMessageEvent);
}

void Source::initialize()
{
    jobStats.setName("job interarrival time stats");
    jobIat.setName("job interarrival time vector");

    sendMessageEvent = new cMessage("sendMessageEvent");
    scheduleAt(simTime(), sendMessageEvent);
}

void Source::handleMessage(cMessage *msg)
{
    ASSERT(msg==sendMessageEvent);

    cMessage *job = new cMessage("job");
    send(job, "out");

    simtime_t iat = par("sendIaTime").doubleValue();
    scheduleAt(simTime()+iat, sendMessageEvent);
    EV << "Job arrived with interarrival time: " << iat << "sec" << endl;
    jobStats.collect( iat );
    jobIat.record( iat );
}

void Source::finish()
{
    EV << "Total jobs processed: " << jobStats.getCount() << endl;
    EV << "Avg job interarrival time:    " << jobStats.getMean() << endl;
    EV << "Max job interarrival time:    " << jobStats.getMax() << endl;
    EV << "Standard deviation:   " << jobStats.getStddev() << endl;
}

}; //namespace

