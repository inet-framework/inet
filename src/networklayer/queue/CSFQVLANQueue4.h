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


#ifndef __INET_CSFQVLANQUEUE4_H
#define __INET_CSFQVLANQUEUE4_H

#include "CSFQVLANQueue3.h"

/**
 * Incoming packets are classified by an external VLAN classifier and
 * metered by an external token bucket meters before being put into
 * a common FIFO queue, non-conformant ones being processed core-stateless
 * fair queueing (CSFQ) algorithm for proportional allocation of excess
 * bandwidth.
 * See NED for more info.
 */
class INET_API CSFQVLANQueue4 : public CSFQVLANQueue3
{
  protected:
    // CSFQ++: System-wide variables
    bool thresholdPassed;   // flag indicating whether queue passes threshold or not
    double thresholdScaleFactor;    // scaling factor used to determine queue threshold lower and upper limits
    int lowerThreshold;
    int upperThreshold;
    int max_beta;   // maximum number of times fair rate (alpha) can be decreased when the queue passes threshold, during a time interval of length K_alpha
    int kbeta;      // counter for the number of times the fair rate decreases due to queue's passing the threshold

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
