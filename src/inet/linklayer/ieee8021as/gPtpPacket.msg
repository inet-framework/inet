//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

cplusplus {{
#include "omnetpp/simtime.h"
#include "gPtp.h"
#include "omnetpp.h"
}};


class noncobject omnetpp::SimTime;

packet gPtp_Sync
{
	omnetpp::SimTime sentTime;
	omnetpp::SimTime localDrift;
	omnetpp::SimTime originTimestamp;	
}

packet gPtp_FollowUp
{
	omnetpp::SimTime sentTime;
	omnetpp::SimTime preciseOriginTimestamp;
	omnetpp::SimTime correctionField;
	omnetpp::SimTime rateRatio;
	int numberOfHops;
}

packet gPtp_PdelayReq
{
    omnetpp::SimTime sentTime;
    omnetpp::SimTime originTimestamp;
	int requestingPortIdentity;
}

packet gPtp_PdelayResp
{
    omnetpp::SimTime sentTime;
    omnetpp::SimTime requestReceiptTimestamp;
	int requestingPortIdentity;
}

packet gPtp_PdelayRespFollowUp
{
    omnetpp::SimTime sentTime;
    omnetpp::SimTime responseOriginTimestamp;
	int requestingPortIdentity;
}
