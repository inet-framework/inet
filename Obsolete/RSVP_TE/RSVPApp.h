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
//#include "tcp.h"
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

    int local_addr, dest_addr;
    double BW, delay;
    bool isSender, isER, isIR;
    std::vector<TELinkState>  ted;
    std::vector<traffic_request_t> tr;
    std::vector<lsp_tunnel_t> FecSenderBinds;
    std::vector<routing_info_t> routingInfo;

    bool initFromFile(const cXMLElement *root);
    traffic_request_t parseTrafficRequest(const cXMLElement *connNode);
    void addRouteInfo(ResvMessage* rmsg);
    bool hasPath(int lspid, FlowSpecObj_t* newFlowspec);
    double getTotalDelay(std::vector<simple_link_t> *links);

public:
    Module_Class_Members(RSVPAppl, cSimpleModule, 16384);

    virtual void initialize();
    virtual void activity();

    //void sendPathMessage(SessionObj_t* s, traffic_request_t* t);
    void sendPathMessage(SessionObj_t* s, traffic_request_t* t, int lspId);
    void sendResvMessage(PathMessage* pMsg, int inLabel);

    void Mcast_Route_Query(int sa, int iad, int da, int *outl);
    void Unicast_Route_Query(int da, int* outl);
    void getPeerIPAddress(int dest, int* peerIP, int* peerInf);
    void getPeerIPAddress(int peerInf, int* peerIP);
    void getPeerInet(int peerIP, int* peerInf);
    void getIncInet(int remoteInet, int* incInet);

    void updateTED();
};

#endif
