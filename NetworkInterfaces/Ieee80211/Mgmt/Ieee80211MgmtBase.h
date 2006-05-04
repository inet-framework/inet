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

#ifndef MGMT80211BASE_H
#define MGMT80211BASE_H

#include <omnetpp.h>
#include "MACAddress.h"
#include "NotificationBoard.h"
#include "WLANFrame_m.h"
#include "WLANMgmtFrames_m.h"


/**
 * Base class for 802.11 infrastructure mode management component.
 *
 * @author Andras Varga
 */
class INET_API Mgmt80211Base : public cSimpleModule, public INotifiable
{
  protected:
    /** Dispatch to frame processing methods according to frame type */
    virtual void processFrame(W80211BasicFrame *frame);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(W80211DataFrame *frame) = 0;
    virtual void handleAuthenticationFrame(W80211AuthenticationFrame *frame) = 0;
    virtual void handleDeauthenticationFrame(W80211DeauthenticationFrame *frame) = 0;
    virtual void handleAssociationRequestFrame(W80211AssociationRequestFrame *frame) = 0;
    virtual void handleAssociationResponseFrame(W80211AssociationResponseFrame *frame) = 0;
    virtual void handleReassociationRequestFrame(W80211ReassociationRequestFrame *frame) = 0;
    virtual void handleReassociationResponseFrame(W80211ReassociationResponseFrame *frame) = 0;
    virtual void handleDisassociationFrame(W80211DisassociationFrame *frame) = 0;
    virtual void handleBeaconFrame(W80211BeaconFrame *frame) = 0;
    virtual void handleProbeRequestFrame(W80211ProbeRequestFrame *frame) = 0;
    virtual void handleProbeResponseFrame(W80211ProbeResponseFrame *frame) = 0;
    //@}
};



#endif
