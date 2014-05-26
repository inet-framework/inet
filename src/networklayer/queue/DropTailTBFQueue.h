//
// Copyright (C) 2011 Kyeong Soo (Joseph) Kim
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#ifndef __INET_DROPTAILTBFQUEUE_H
#define __INET_DROPTAILTBFQUEUE_H

#include <omnetpp.h>
#include "DropTailQueue.h"

/**
 * Returns the maximum of a and b.
 */
inline double max(const double a, const double b) { return (a > b) ? a : b; }

/**
 * Drop-front queue with token bucket filter (TBF) traffic shaper.
 * See NED for more info.
 */
class INET_API DropTailTBFQueue : public DropTailQueue
{
  protected:
    // configuration
    long long bucketSize;   // in bit; note that the corresponding parameter in NED/INI is in byte.
    double meanRate;
    int mtu;   // in bit; note that the corresponding parameter in NED/INI is in byte.
    double peakRate;

    // state
    long long meanBucketLength;  // the number of tokens (bits) in the bucket for mean rate/burst control
    int peakBucketLength;  // the number of tokens (bits) in the bucket for peak rate/MTU control
    simtime_t lastTime; // the last time the TBF used
    bool isTxScheduled; // flag to indicate whether there is any scheduled frame transmission

    // statistics
    int numQueueShaped;
    int numQueueSent;

    // timer
    cMessage *resumeTransmissionTimer;

  public:
    DropTailTBFQueue();
    virtual ~DropTailTBFQueue();

  protected:
    virtual void initialize();

    /**
	 * Redefined from DropTailQueue
	 */
    virtual void handleMessage(cMessage *msg);

    /**
     * Redefined from DropTailQueue
     */
    virtual void requestPacket();

    /**
     * Newly defined.
     */
    virtual bool isConformed(int pktLength);

    /**
     * Newly defined.
     */
    virtual void scheduleTransmission(int pktLength);

    /**
     * Newly defined.
     */
    virtual void dumpTbfStatus();

    /**
	 * Redefined from DropTailQueue
	 */
    virtual void finish();
};

#endif
