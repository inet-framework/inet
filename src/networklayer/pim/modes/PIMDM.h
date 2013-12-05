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

#ifndef __INET_PIMDM_H
#define __INET_PIMDM_H

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"
#include "InterfaceTableAccess.h"
#include "PIMRoutingTableAccess.h"
#include "NotifierConsts.h"
#include "PIMNeighborTable.h"
#include "PIMInterfaceTable.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "IPv4Route.h"
#include "PIMBase.h"

#define PT 180.0						/**< Prune Timer = 180s (3min). */
#define GRT 3.0							/**< Graft Retry Timer = 3s. */
#define SAT 210.0						/**< Source Active Timer = 210s, Cisco has 180s, after that, route is flushed */
#define SRT 60.0						/**< State Refresh Timer = 60s. */

/**
 * @brief Class implements PIM-DM (dense mode).
 */
class PIMDM : public PIMBase, protected cListener
{
	public:
        PIMDM() : PIMBase(PIMInterface::DenseMode) {}

	private:
	    // process events
        void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
	    void newMulticast(IPv4Address srcAddress, IPv4Address destAddress);
	    void newMulticastAddr(PIMInterface *pimInt, IPv4Address newAddr);
	    void oldMulticastAddr(PIMInterface *pimInt, IPv4Address oldAddr);
	    void dataOnPruned(IPv4Address destAddr, IPv4Address srcAddr);
	    void dataOnNonRpf(IPv4Address group, IPv4Address source, int intId);
	    void dataOnRpf(IPv4Datagram *datagram);
	    void rpfIntChange(PIMMulticastRoute *route);

	    // process timers
	    void processPIMTimer(PIMTimer *timer);
	    void processPruneTimer(PIMpt * timer);
	    void processGraftRetryTimer(PIMgrt *timer);
	    void processSourceActiveTimer(PIMsat * timer);
	    void processStateRefreshTimer(PIMsrt * timer);

	    // create timers
	    PIMpt* createPruneTimer(IPv4Address source, IPv4Address group, int intId, int holdTime);
	    PIMgrt* createGraftRetryTimer(IPv4Address source, IPv4Address group);
	    PIMsat* createSourceActiveTimer(IPv4Address source, IPv4Address group);
	    PIMsrt* createStateRefreshTimer(IPv4Address source, IPv4Address group);

	    // process PIM packets
	    void processPIMPkt(PIMPacket *pkt);
	    void processJoinPruneGraftPacket(PIMJoinPrune *pkt, PIMPacketType type);
	    void processPrunePacket(PIMMulticastRoute *route, int intId, int holdTime);
	    void processGraftPacket(IPv4Address source, IPv4Address group, IPv4Address sender, int intId);
	    void processGraftAckPacket(PIMMulticastRoute *route);
	    void processStateRefreshPacket(PIMStateRefresh *pkt);

	    //create PIM packets
	    void sendPimJoinPrune(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendPimGraft(IPv4Address nextHop, IPv4Address src, IPv4Address grp, int intId);
	    void sendPimGraftAck(PIMGraftAck *msg);
	    void sendPimStateRefresh(IPv4Address originator, IPv4Address src, IPv4Address grp, int intId, bool P);

	    PIMInterface *getIncomingInterface(IPv4Datagram *datagram);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
