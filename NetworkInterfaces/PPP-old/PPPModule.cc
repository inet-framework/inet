//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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

#include <omnetpp.h>
#include <string.h>

#include "PPPModule.h"
#include "PPPFrame.h"
#include "IPDatagram.h"
#include "hook_types.h"   // NWI_IDLE -- FIXME what the heck's this?


Define_Module_Like(PPPModule, NetworkInterface);


void PPPModule::endService(cMessage *msg)
{
    if (!strcmp(msg->arrivalGate()->name(), "ipOutputQueueIn"))
    {
        cMessage *nwiIdleMsg = new cMessage();
        nwiIdleMsg->setKind(NWI_IDLE);

        // encapsulate IP datagram in PPP frame
        PPPFrame *outFrame = new PPPFrame();
        outFrame->encapsulate((IPDatagram *)msg);

        send(outFrame, "physicalOut");
        send(nwiIdleMsg, "ipOutputQueueOut");

    }
    else // from Network
    {
        PPPFrame *recFrame = check_and_cast<PPPFrame *>(msg);

        // decapsulate IP datagram
        if (recFrame->protocol() == PPP_PROT_IP)
        {
            // break off for /all/ bit errors
            if (recFrame->hasBitError())
            {
                ev << "\n+ PPPLink of " << fullPath()
                   << " receive error: Error in transmission.\n";
                delete msg;
                return;
            }

            IPDatagram *ipdatagram = check_and_cast<IPDatagram *>(recFrame->decapsulate());
            delete recFrame;

            send(ipdatagram, "ipInputQueueOut");
        }
        else // print error message if it's not an IP datagram
        {
            // FIXME why should it be always IP protocol???????
            ev << "\n+ PPPLink of " << fullPath() << " receive error: Not IP protocol.\n";
        }
    }
}

