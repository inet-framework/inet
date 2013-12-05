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
// Authors: Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#ifndef __INET_PIMSM_H
#define __INET_PIMSM_H

#include <omnetpp.h>
#include "PIMPacket_m.h"
#include "PIMTimer_m.h"

#include "InterfaceTableAccess.h"
#include "NotifierConsts.h"
#include "PIMNeighborTable.h"
#include "PIMInterfaceTable.h"
#include "IPv4ControlInfo.h"
#include "IPv4InterfaceData.h"
#include "PIMMulticastRoute.h"
#include "PIMRoutingTable.h"
#include "PIMBase.h"

#define KAT 180.0                       /**< Keep alive timer, if RPT is disconnect */
#define KAT2 210.0                      /**< Keep alive timer, if RPT is connect */
#define RST 60.0                        /**< Register-stop Timer*/
#define JT 60.0                         /**< Join Timer */
#define REGISTER_PROBE_TIME 5.0         /**< Register Probe Time */
#define HOLDTIME 210.0                  /**< Holdtime for Expiry Timer */
#define HOLDTIME_HOST 180.0             /**< Holdtime for interface ET connected to host */
#define PPT 3.0                         /**< value for Prune-Pending Timer*/
#define ALL_PIM_ROUTERS "224.0.0.13"    /**< Multicast address for PIM routers */
#define MAX_TTL 255                     /**< Maximum TTL */
#define NO_INT_TIMER -1
#define CISCO_SPEC_SIM 1                /**< Enable Cisco specific simulation; 1 = enable, 0 = disable */


struct multDataInfo
{
    IPv4Address origin;
    IPv4Address group;
    unsigned interface_id;
    IPv4Address srcAddr;
};

enum joinPruneMsg
{
    JoinMsg = 0,
    PruneMsg = 1
};

enum JPMsgType
{
    G = 1,
    SG = 2,
    SGrpt = 3
};


/**
 * @brief Class implements PIM-SM (sparse mode).
 */
class PIMSM : public PIMBase, protected cListener
{
    private:
        IPv4Address RPAddress;
        std::string SPTthreshold;

    private:
        void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
        void newMulticastRegisterDR(IPv4Address srcAddr, IPv4Address destAddr);
        void newMulticastReceiver(PIMInterface *pimInterface, IPv4Address multicastGroup);
        void removeMulticastReceiver(PIMInterface *pimInterface, IPv4Address multicastGroup);


        // process timers
        void processPIMTimer(PIMTimer *timer);
        void processKeepAliveTimer(PIMkat *timer);
        void processRegisterStopTimer(PIMrst *timer);
        void processExpiryTimer(PIMet *timer);
        void processJoinTimer(PIMjt *timer);
        void processPrunePendingTimer(PIMppt *timer);


        void restartExpiryTimer(PIMMulticastRoute *route, InterfaceEntry *originIntf, int holdTime);
        void dataOnRpf(PIMMulticastRoute *route);

        // set timers
        PIMkat* createKeepAliveTimer(IPv4Address source, IPv4Address group);
        PIMrst* createRegisterStopTimer(IPv4Address source, IPv4Address group);
        PIMet*  createExpiryTimer(int intID, int holdtime, IPv4Address group, IPv4Address source, int StateType);
        PIMjt*  createJoinTimer(IPv4Address group, IPv4Address JPaddr, IPv4Address upstreamNbr, int JoinType);
        PIMppt* createPrunePendingTimer(IPv4Address group, IPv4Address JPaddr, IPv4Address upstreamNbr, JPMsgType JPtype);

        // pim messages
        void sendPIMRegister(IPv4Datagram *datagram);
        void sendPIMRegisterStop(IPv4Address source, IPv4Address dest, IPv4Address multGroup, IPv4Address multSource);
        void sendPIMRegisterNull(IPv4Address multSource, IPv4Address multDest);
        void sendPIMJoinPrune(IPv4Address multGroup, IPv4Address joinPruneIPaddr, IPv4Address upstreamNbr, joinPruneMsg JoinPrune, JPMsgType JPtype);
        void sendPIMJoinTowardSource(multDataInfo *info);
        void forwardMulticastData(IPv4Datagram *datagram, multDataInfo *info);

        // process PIM messages
        void processPIMPkt(PIMPacket *pkt);
        void processRegisterPacket(PIMRegister *pkt);
        void processRegisterStopPacket(PIMRegisterStop *pkt);
        void processJoinPacket(PIMJoinPrune *pkt, IPv4Address multGroup, EncodedAddress encodedAddr);
        void processPrunePacket(PIMJoinPrune *pkt, IPv4Address multGroup, EncodedAddress encodedAddr);
        void processJoinPrunePacket(PIMJoinPrune *pkt);
        void processSGJoin(PIMJoinPrune *pkt,IPv4Address multOrigin, IPv4Address multGroup);
        void processJoinRouteGexistOnRP(IPv4Address multGroup, IPv4Address packetOrigin, int msgHoldtime);

        PIMInterface *getIncomingInterface(IPv4Datagram *datagram);

    public:
        PIMSM() : PIMBase(PIMInterface::SparseMode) {}
        //PIM-SM clear implementation
        void setRPAddress(std::string address);
        void setSPTthreshold(std::string address);
        IPv4Address getRPAddress () {return RPAddress;}
        std::string getSPTthreshold () {return SPTthreshold;}
        virtual bool IamRP (IPv4Address RPaddress);
        bool IamDR (IPv4Address sourceAddr);
        IPv4ControlInfo *setCtrlForMessage (IPv4Address destAddr,IPv4Address srcAddr,int protocol, int interfaceId, int TTL);

	protected:
		virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void handleMessage(cMessage *msg);
		virtual void initialize(int stage);
};

#endif
