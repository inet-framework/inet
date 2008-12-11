//
// Copyright (C) 2008 Irene Ruengeler
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "SCTPAlg.h"
#include "SCTP.h"

Register_Class(SCTPAlg);


SCTPAlg::SCTPAlg() : SCTPAlgorithm()
{
    state = NULL;
	
}

SCTPAlg::~SCTPAlg()
{
    sctpEV3<<"Destructor SCTPAlg\n";
	// Note: don't delete "state" here, it'll be deleted from SCTPAssociation
}

SCTPStateVariables *SCTPAlg::createStateVariables()
{
	ASSERT(state==NULL);
	state = new SCTPAlgStateVariables();
	return state;
}

void SCTPAlg::established(bool active)
{
    if (active)
    {
        sctpEV3 << "\nCompleting connection: sending DATA\n";
    }
}

void SCTPAlg::connectionClosed()
{
}

void SCTPAlg::processTimer(cMessage *timer, SCTPEventCode& event)
{
    // no extra timers in this SCTP variant
}

void SCTPAlg::sendCommandInvoked(IPvXAddress addr)
{

	sctpEV3<<"SCTPAlg: sendAll to "<<addr<<"\n";
	assoc->sendAll(addr);
}


void SCTPAlg::receivedDataAck(uint32)
{

}

void SCTPAlg::receivedDuplicateAck()
{
    sctpEV3 << "Duplicate ACK #\n";
}

void SCTPAlg::receivedAckForDataNotYetSent(uint32 seq)
{
    sctpEV3 << "ACK acks something not yet sent, sending immediate ACK\n";

}

void SCTPAlg::sackSent()
{
}

void SCTPAlg::dataSent(uint32)
{
}


