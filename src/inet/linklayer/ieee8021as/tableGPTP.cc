//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

#include "tableGPTP.h"

#include "EtherGPTP.h"

namespace inet {

Define_Module(TableGPTP);

void TableGPTP::initialize()
{
    correctionField = par("correctionField");
    rateRatio = par("rateRatio");
    peerDelay = 0;
    receivedTimeSync = 0;
    receivedTimeFollowUp = 0;
}

void TableGPTP::handleGptpCall(cMessage *msg)
{
    Enter_Method("handleGptpCall");

    take (msg);
    //TODO tags?
    for (auto gptp : gptps) {
        gptp.second->handleTableGptpCall(msg->dup());
    }
    delete msg;
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

void TableGPTP::addGptp(EtherGPTP *gptp)
{
    gptps.insert(std::pair<int, EtherGPTP*>(gptp->getId(), gptp));
}

void TableGPTP::removeGptp(EtherGPTP *gptp)
{
    gptps.erase(gptp->getId());
}

}
