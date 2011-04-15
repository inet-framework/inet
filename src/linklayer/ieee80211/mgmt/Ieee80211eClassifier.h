//
// Copyright (C) 2010 Alfonso Ariza
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


#ifndef __INET_IEEE80211ECLASSIFIER_H
#define __INET_IEEE80211ECLASSIFIER_H

#include "IQoSClassifier.h"

/**
 * Just an example for packet classifiers, based on IPv4 DSCP/IPv6 Traffic
 * class. You'll probably need to implement others if your research interest
 * lies in QoS.
 */
class INET_API Ieee80211eClassifier : public IQoSClassifier
{
  private:
    int defaultAC;
  public:
    /**
     * Returns the largest value plus one classifyPacket() returns.
     */
    Ieee80211eClassifier();
    virtual int getNumQueues();
    virtual void setDefaultClass(int i=0){defaultAC=0;}
    virtual int getDefaultClass(){return defaultAC;}

    /**
     * The method should return the priority (the index of subqueue)
     * for the given packet, a value between 0 and getNumQueues()-1
     * (inclusive).
     */
    virtual int classifyPacket(cMessage *msg);
};

#endif

