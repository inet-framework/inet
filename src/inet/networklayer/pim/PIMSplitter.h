//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#ifndef __INET_PIMSPLITTER_H
#define __INET_PIMSPLITTER_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/networklayer/pim/tables/PIMInterfaceTable.h"
#include "inet/networklayer/pim/PIMPacket_m.h"


/**
 * PIMSplitter register itself for PIM protocol (103) in the network layer,
 * and dispatches the received packets either to PIMDM or PIMSM according
 * to the PIM mode of the incoming interface.
 * Packets received from the PIM modules are simply forwarded to the
 * network layer.
 */
class INET_API PIMSplitter : public cSimpleModule
{
	private:
	    IInterfaceTable *ift;
	    PIMInterfaceTable *pimIft;

	    cGate *ipIn;
	    cGate *ipOut;
	    cGate *pimDMIn;
	    cGate *pimDMOut;
        cGate *pimSMIn;
        cGate *pimSMOut;

    public:
        PIMSplitter() : ift(NULL), pimIft(NULL) {};

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
        virtual void initialize(int stage);
		virtual void handleMessage(cMessage *msg);
		virtual void processPIMPacket(PIMPacket *pkt);
};

#endif

