//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
//


#include "UDPAppConnector.h"


Define_Module(UDPAppConnector);


//UDPAppConnector::UDPAppConnector()
//{
//    msgServiced = endServiceMsg = NULL;
//}
//
//UDPAppConnector::~UDPAppConnector()
//{
//    delete msgServiced;
//    cancelAndDelete(endServiceMsg);
//}

//void UDPAppConnector::initialize()
//{
//}

void UDPAppConnector::handleMessage(cMessage *msg)
{
    if (msg->isPacket() == true)
    {
        send(msg, "out");
    }
    else
    {   // filter non-cPacket messages
        delete msg;
    }
}
