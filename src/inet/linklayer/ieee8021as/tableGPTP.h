//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

#ifndef __IEEE8021AS_TABLEGPTP_H_
#define __IEEE8021AS_TABLEGPTP_H_

#include <omnetpp.h>

using namespace omnetpp;

class TableGPTP : public cSimpleModule
{
    SimTime correctionField;
    SimTime rateRatio;
    SimTime originTimestamp;
    SimTime peerDelay;
    int numberOfGates;

    // Below timestamps are not drifted and they are in simtime
    SimTime receivedTimeSync;
    SimTime receivedTimeFollowUp;

    /* time to receive Sync message before synchronize local time with master */
    SimTime timeBeforeSync;

    // This is used to calculate residence time within time-aware system
    // Its value has the time receiving Sync message from master port of other system
    SimTime receivedTimeAtHandleMessage;

    // Adjusted time when Sync received
    // For constant drift, setTime = sentTime + delay
    SimTime setTime;

  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

  public:
    void setCorrectionField(SimTime cf);
    void setRateRatio(SimTime cf);
    void setPeerDelay(SimTime cf);
    void setReceivedTimeSync(SimTime cf);
    void setReceivedTimeFollowUp(SimTime cf);
    void setReceivedTimeAtHandleMessage(SimTime cf);
    void setOriginTimestamp(SimTime cf);

    SimTime getCorrectionField();
    SimTime getRateRatio();
    SimTime getPeerDelay();
    SimTime getReceivedTimeSync();
    SimTime getReceivedTimeFollowUp();
    SimTime getReceivedTimeAtHandleMessage();
    SimTime getOriginTimestamp();
};

#endif
