//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#ifndef __ENQUEUEHOOK_H__
#define __ENQUEUEHOOK_H__

#include <omnetpp.h>
#include "INETDefs.h"

/**
 * Abstract base class for enqueue hooks.
 */
class INET_API EnqueueHook : public cPolymorphic
{
  public:
    /**
     * Called with the "this" pointer from the module which installs
     * this object. Subclasses may use the pointer to get access to
     * module pointers, etc. Note that this class doesn't store the pointer
     * but subclasses may well decide to do so.
     */
    virtual void setModule(cSimpleModule *) {}

    /**
     * Called when a packet arrives and the queue is not empty.
     * Implementation of this function should enqueue the packet --
     * or just discard it. It can also do priority queueing or
     * discard other packets already in the queue -- anything.
     */
    virtual void enqueue(cMessage *msg, cQueue& queue) = 0;

    /**
     * Called when a packet arrives and the queue is empty.
     * Implementation of this function should either return the same
     * pointer, or drop the packet and return NULL.
     */
    virtual cMessage *dropIfNotNeeded(cMessage *msg) = 0;
};

#endif

