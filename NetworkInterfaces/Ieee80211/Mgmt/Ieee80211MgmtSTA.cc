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


#include "Mgmt80211STA.h"
#include "Ieee802Ctrl_m.h"


Define_Module(Mgmt80211STA);


void Mgmt80211STA::initialize(int)
{
    //...
}

void Mgmt80211STA::handleMessage(cMessage *msg)
{
}

void Mgmt80211STA::receiveChangeNotification(int category, cPolymorphic *details)
{
}

/*
void Mgmt80211STA::processFrame(W80211BasicFrame *frame)
{
    const FrameControl& frameControl = frame->getFrameControl();

    switch(frameControl.subtype)
    {
      case ST_BEACON:
        handleBeacon(check_and_cast<W80211ManagementFrame *>(frame));
        break;
      case ST_PROBEREQUEST:
        handleProbeRequest(static_cast<WirelessAccessPoint*>(mod), signal);
        break;
      case ST_PROBERESPONSE:
        handleProbeResponse(mod, signal);
        break;
      case ST_ASSOCIATIONREQUEST:
        handleAssociationRequest(static_cast<WirelessAccessPoint*>(mod), signal);
        break;
      case ST_ASSOCIATIONRESPONSE:
        handleAssociationResponse(mod, signal);
        break;
      case ST_REASSOCIATIONREQUEST:
        handleReAssociationRequest(static_cast<WirelessAccessPoint*>(mod), signal);
        break;
      case ST_REASSOCIATIONRESPONSE:
        handleReAssociationResponse(mod, signal);
        break;
      case ST_DISASSOCIATION:
        handleDisAssociation(mod, signal);
        break;
      case ST_DATA:
        handleData(mod, signal);
        break;
      case ST_ACK:
        handleAck(mod, signal);
        break;
      case ST_AUTHENTICATION:
        handleAuthentication(mod, signal);
        break;
      case ST_DEAUTHENTICATION:
        handleDeAuthentication(mod, signal);
        break;
    }
}
*/

