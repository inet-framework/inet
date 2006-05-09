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


#include "Ieee80211MgmtBase.h"
#include "Ieee802Ctrl_m.h"


void Ieee80211MgmtBase::processFrame(Ieee80211BasicFrame *frame)
{
    const Ieee80211FrameControl& frameControl = frame->getFrameControl();
    switch(frameControl.subtype)
    {
      case ST_DATA:
        handleDataFrame(check_and_cast<Ieee80211DataFrame *>(frame));
        break;
      case ST_AUTHENTICATION:
        handleAuthenticationFrame(check_and_cast<Ieee80211AuthenticationFrame *>(frame));
        break;
      case ST_DEAUTHENTICATION:
        handleDeauthenticationFrame(check_and_cast<Ieee80211DeauthenticationFrame *>(frame));
        break;
      case ST_ASSOCIATIONREQUEST:
        handleAssociationRequestFrame(check_and_cast<Ieee80211AssociationRequestFrame *>(frame));
        break;
      case ST_ASSOCIATIONRESPONSE:
        handleAssociationResponseFrame(check_and_cast<Ieee80211AssociationResponseFrame *>(frame));
        break;
      case ST_REASSOCIATIONREQUEST:
        handleReassociationRequestFrame(check_and_cast<Ieee80211ReassociationRequestFrame *>(frame));
        break;
      case ST_REASSOCIATIONRESPONSE:
        handleReassociationResponseFrame(check_and_cast<Ieee80211ReassociationResponseFrame *>(frame)); break;
      case ST_DISASSOCIATION:
        handleDisassociationFrame(check_and_cast<Ieee80211DisassociationFrame *>(frame));
        break;
      case ST_BEACON:
        handleBeaconFrame(check_and_cast<Ieee80211BeaconFrame *>(frame));
        break;
      case ST_PROBEREQUEST:
        handleProbeRequestFrame(check_and_cast<Ieee80211ProbeRequestFrame *>(frame));
        break;
      case ST_PROBERESPONSE:
        handleProbeResponseFrame(check_and_cast<Ieee80211ProbeResponseFrame *>(frame));
        break;
      default:
        error("unexpected frame type (%s)%s", frame->className(), frame->name());
    }
}

