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

#include "inet/linklayer/ieee80211/mac/IQoSClassifier.h"

namespace inet {

namespace ieee80211 {

/**
 * An example packet classifier based on the UDP/TCP port number.
 * Access point management frames are classified into the 'defaultManagement' class (3 by default).
 * ICMP messages are classified to: 1
 * Traffic on TCP/UDP port 21: class 0
 * Traffic on TCP/UDP port 80: class 1
 * Traffic on TCP/UDP port 4000: class 2
 * Traffic on TCP/UDP port 5000: class 3
 * All other traffic is classified as 'defaultAC' (0 by default)
 */
class INET_API Ieee80211eClassifier : public IQoSClassifier
{
  private:
    int defaultAC;
    int defaultManagement;

  public:
    Ieee80211eClassifier();
    virtual int getNumQueues();
    virtual void setDefaultClass(int i) { defaultAC = i; }
    virtual int getDefaultClass() { return defaultAC; }
    virtual void setDefaultManagementClass(int i) { defaultManagement = i; }
    virtual int getDefaultManagementClass() { return defaultManagement; }

    /**
     * The method should return the priority (the index of subqueue)
     * for the given packet, a value between 0 and getNumQueues()-1
     * (inclusive).
     */
    virtual int classifyPacket(cMessage *msg);
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211ECLASSIFIER_H

