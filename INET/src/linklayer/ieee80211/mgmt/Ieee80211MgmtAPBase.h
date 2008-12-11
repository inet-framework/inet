//
// Copyright (C) 2006 Andras Varga
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

#ifndef IEEE80211_MGMT_AP_BASE_H
#define IEEE80211_MGMT_AP_BASE_H

#include <omnetpp.h>
#include "Ieee80211MgmtBase.h"
#include "NotificationBoard.h"

class EtherFrame;

/**
 * Used in 802.11 infrastructure mode: abstract base class for management frame
 * handling for access points (APs). This class extends Ieee80211MgmtBase
 * with utility functions that are useful for implementing AP functionality.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtAPBase : public Ieee80211MgmtBase
{
  protected:
    bool hasRelayUnit;

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /**
     * Utility function for APs: sends back a data frame we received from a
     * STA to the wireless LAN, after tweaking fromDS/toDS bits and shuffling
     * addresses as needed.
     */
    virtual void distributeReceivedDataFrame(Ieee80211DataFrame *frame);

    /**
     * Utility function: converts EtherFrame to Ieee80211Frame. This is needed
     * because MACRelayUnit which we use for LAN bridging functionality deals
     * with EtherFrames.
     */
    virtual Ieee80211DataFrame *convertFromEtherFrame(EtherFrame *ethframe);

    /**
     * Utility function: converts the given frame to EtherFrame, deleting the
     * original frame. This function is needed for LAN bridging functionality:
     * MACRelayUnit deals with EtherFrames.
     */
    virtual EtherFrame *convertToEtherFrame(Ieee80211DataFrame *frame);
};

#endif

