//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include "FifoQueue.h"


Define_Module(FifoQueue);

simtime_t FifoQueue::startService(cMessage *msg)
{
    EV << "Starting service of " << msg->getName() << endl;
    return par("serviceTime");
}

void FifoQueue::endService(cMessage *msg)
{
    EV << "Completed service of " << msg->getName() << endl;
    send( msg, "out" );
}


