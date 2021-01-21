//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

#include "tableGPTP.h"

Define_Module(TableGPTP);

void TableGPTP::initialize(int stage)
{
    correctionField = par("correctionField");
    rateRatio = par("rateRatio");
    peerDelay = 0;
    receivedTimeSync = 0;
    receivedTimeFollowUp = 0;
    numberOfGates = gateSize("gptpLayerIn");
}

void TableGPTP::handleMessage(cMessage *msg)
{
    if(msg->arrivedOn("gptpLayerIn"))
    {
        for (int i = 0; i < numberOfGates; i++)
        {
            send(msg->dup(), "gptpLayerOut", i);
        }
        delete msg;
    }
}

void TableGPTP::setCorrectionField(SimTime cf)
{
    correctionField = cf;
}

SimTime TableGPTP::getCorrectionField()
{
    return correctionField;
}

void TableGPTP::setRateRatio(SimTime cf)
{
    rateRatio = cf;
}

SimTime TableGPTP::getRateRatio()
{
    return rateRatio;
}

void TableGPTP::setPeerDelay(SimTime cf)
{
    peerDelay = cf;
}

SimTime TableGPTP::getPeerDelay()
{
    return peerDelay;
}

void TableGPTP::setReceivedTimeSync(SimTime cf)
{
    receivedTimeSync = cf;
}

SimTime TableGPTP::getReceivedTimeSync()
{
    return receivedTimeSync;
}

void TableGPTP::setReceivedTimeFollowUp(SimTime cf)
{
    receivedTimeFollowUp = cf;
}

SimTime TableGPTP::getReceivedTimeFollowUp()
{
    return receivedTimeFollowUp;
}

void TableGPTP::setReceivedTimeAtHandleMessage(SimTime cf)
{
    receivedTimeAtHandleMessage = cf;
}

SimTime TableGPTP::getReceivedTimeAtHandleMessage()
{
    return receivedTimeAtHandleMessage;
}

void TableGPTP::setOriginTimestamp(SimTime cf)
{
    originTimestamp = cf;
}

SimTime TableGPTP::getOriginTimestamp()
{
    return originTimestamp;
}
