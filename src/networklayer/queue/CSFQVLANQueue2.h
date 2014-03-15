//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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


#ifndef __INET_CSFQVLANQUEUE2_H
#define __INET_CSFQVLANQUEUE2_H

#include "CSFQVLANQueue.h"

/**
 * Incoming packets are classified by an external VLAN classifier and
 * metered by an external token bucket meters before being put into
 * a common FIFO queue, non-conformant ones being processed core-stateless
 * fair queueing (CSFQ) algorithm for proportional allocation of excess
 * bandwidth.
 * See NED for more info.
 */
class INET_API CSFQVLANQueue2 : public CSFQVLANQueue
{
  protected:

    // FIFO
    int queueThreshold;     // in byte

    // CSFQ++: System-wide variables
    int max_alpha;          // maximum number of times fair rate (alpha) can be decreased when queue overflows, during a time interval of length K_alpha
    int kalpha;             // counter for the number of times fair rate decreases due to buffer overflow

  protected:
    virtual void initialize(int stage);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void handleMessage(cMessage *msg);

    /**
     * For CSFQ.
     */
    virtual void estimateAlpha(int pktLength, double rate, simtime_t arrvTime, bool dropped);
};

#endif
