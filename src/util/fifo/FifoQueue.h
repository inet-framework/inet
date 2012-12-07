//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#ifndef __FIFOQUEUE_H
#define __FIFOQUEUE_H

#include "AbstractFifoQueue.h"


/**
 * Single-server queue with a given service time.
 */
class FifoQueue : public AbstractFifoQueue
{
  protected:
    virtual simtime_t startService(cMessage *msg);
    virtual void endService(cMessage *msg);
};


#endif
