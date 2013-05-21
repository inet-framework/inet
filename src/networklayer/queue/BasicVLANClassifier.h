//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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


#ifndef __INET_BASICVLANCLASSIFIER_H
#define __INET_BASICVLANCLASSIFIER_H

#include <omnetpp.h>
#include "IQoSClassifier.h"
#include "VLAN.h"

/**
 * Basic Ethernet frame classifier based on IEEE 802.1Q VLAN tag.
 * Currently it uses VLAN ID (VID) only.
 */
class INET_API BasicVLANClassifier : public IQoSClassifier
{
    protected:
        int maxNumQueues;
        int numQueues;

        typedef std::map<VID, int> QueueIndexTable;
        QueueIndexTable indexTable;

        // internal: maps VID to queue number
        virtual int classifyByVID(int dscp);

    public:
        BasicVLANClassifier() : maxNumQueues(0), numQueues(0) {}
        virtual ~BasicVLANClassifier() {}

        /**
         * Initialize the indexTable with a given set of VIDs.
         */
        virtual void initializeIndexTable(const char *str);

        /**
         * Set maximum number of subqueue indexes.
         */
        virtual void setMaxNumQueues(int n) {maxNumQueues = n;}

        /**
         * Returns the current number of subqueue indexes assigned to VIDs.
         */
        virtual int getNumQueues();

        /**
         * The method should return the index of subqueue for the given Ethernet
         * frame with VLAN tag, a value between 0 and getNumQueues()-1 (inclusive).
         */
        virtual int classifyPacket(cMessage *msg);

};

#endif
