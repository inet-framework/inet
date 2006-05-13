//
// Copyright (C) 2006 Andras Varga
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

#ifndef IEEE80211_MGMT_AP_BASE_H
#define IEEE80211_MGMT_AP_BASE_H

#include <omnetpp.h>
#include "Ieee80211MgmtBase.h"
#include "NotificationBoard.h"

class EtherFrame;

/**
 * Used in 802.11 infrastructure mode: handles management frames for
 * an access point (AP). See corresponding NED file for a detailed description.
 * This implementation ignores many details.
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
     * Utility function: converts the given frame to EtherFrame. The original
     * frame is left untouched (the encapsulated payload msg gets duplicated).
     * This function is needed because MACRelayUnit which we use for LAN bridging
     * functionality deals with EtherFrames.
     */
    virtual EtherFrame *createEtherFrame(Ieee80211DataFrame *frame);
};

#endif

