/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#ifndef __RSVP_HOST_H__
#define __RSVP_HOST_H__

#include <string>
#include <vector>
#include <omnetpp.h>

#include "RoutingTable.h"
#include "rsvp_message.h"
#include "RSVP.h"
#include "TED.h"
#include "OspfTe.h"
#include "OSPFTEAccess.h"
#include "LIBtable.h"
#include "LIBTableAccess.h"
#include "MPLSModule.h"
#include "MPLSAccess.h"



#define DEFAULT_DEST_PORT    80
#define DEFAULT_SRC_PORT    80
#define DEFAULT_SEND_TTL    10
#define DEFAULT_SEND_BW        60000
#define DEFAULT_SEND_DELAY    1
#define DEFAULT_RECV_BW        50000
#define DEFAULT_RECV_DELAY    1



struct traffic_request_t{
    int src;
    int dest;
    int setupPri;
    int holdingPri;
    double delay;
    double bandwidth;
    bool isER;
    EroObj_t route[MAX_ROUTE];
};

struct lsp_tunnel_t
{
    SenderTemplateObj_t Sender_Template;
    SessionObj_t         Session;
    bool operating;
    int inInfIndex;
};

struct routing_info_t{
    int lspId;
    int route[MAX_ROUTE];
};


class RSVPAppl: public cSimpleModule
{
private:
    RoutingTableAccess routingTableAccess;
    LIBTableAccess libTableAccess;
    OSPFTEAccess ospfteAccess;
    MPLSAccess mplsAccess;

    int routerId, dest_addr;  // FIXME make them IPAddress
    double BW, delay;
    bool isER, isIR;
    std::vector<TELinkState>  ted;
    std::vector<traffic_request_t> tr;
    std::vector<lsp_tunnel_t> FecSenderBinds;
    std::vector<routing_info_t> routingInfo;

    bool initFromFile(const cXMLElement *root);
    traffic_request_t parseTrafficRequest(const cXMLElement *connNode);
    void addRouteInfo(RSVPResvMsg* rmsg);
    bool hasPath(int lspid, FlowSpecObj_t* newFlowspec);
    double getTotalDelay(std::vector<simple_link_t> *links);

public:
    Module_Class_Members(RSVPAppl, cSimpleModule, 0);

    virtual int numInitStages() const  {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    /**
     * Process packets from RSVP daemon. Dispatches to various
     * processRSVP_xxx() methods, based in the RSVP message type.
     */
    void processMsgFromRSVP(cMessage *msg);

    /**
     * Process signal from MPLS switch to initiate LabelRequest
     */
    void processSignalFromMPLSSwitch(cMessage *msg);

    /**
     * Process message from Tester. Invokes various processCommand_xxx() methods.
     */
    void processSignalFromTester(cMessage *msg);

    /** @name Process various RSVP message types */
    //@{
    void processRSVP_PERROR(RSVPPathError *pe);
    void processRSVP_PTEAR(cMessage *msg);
    void processRSVP_RTEAR(cMessage *msg);
    void processRSVP_PATH(RSVPPathMsg *pMessage);
    void processRSVP_RESV(RSVPResvMsg *rMessage);
    //@}

    /** @name Process various commands from Tester */
    //@{
    void processCommand_NEW_BW_REQUEST(cMessage *msg);
    void processCommand_NEW_ROUTE_DISCOVER(cMessage *msg);
    //@}

    /** @name Process various signals from MPLS switch */
    //@{
    void processSignalFromMPLSSwitch_TEAR_DOWN(cMessage *msg);
    void processSignalFromMPLSSwitch_PATH_REQUEST(cMessage *msg);
    //@}

    //void sendPathMessage(SessionObj_t* s, traffic_request_t* t);
    void sendPathMessage(SessionObj_t* s, traffic_request_t* t, int lspId);
    void sendResvMessage(RSVPPathMsg* pMsg, int inLabel);

    void Mcast_Route_Query(int sa, int iad, int da, int *outl);
    void Unicast_Route_Query(int da, int* outl);
    void getPeerIPAddress(int dest, int* peerIP, int* peerInf);
    void getPeerIPAddress(int peerInf, int* peerIP);
    void getPeerInet(int peerIP, int* peerInf);
    void getIncInet(int remoteInet, int* incInet);

    void updateTED();
};

#endif
