
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
#ifndef __RSVP__H__
#define __RSVP__H__

#include <omnetpp.h>
#include "RoutingTable.h"
#include "tcp.h"
#include "intserv.h"
#include "rsvp_message.h"
#include "TED.h"
//#include <list>
#include <vector>
#include "OspfTe.h"
#include "LIBTableAccess.h"

#define InLIST_SIZE        5
#define TABLE_SIZE        10




struct PathStateBlock_t{

    /* SESSION object structure */
  SessionObj_t Session_Object;

    /* SENDER_TEMPLATE structure */
  SenderTemplateObj_t Sender_Template_Object;

    /* SENDER_TSPEC structure */
  SenderTspecObj_t Sender_Tspec_Object;

    /* Previous Hop IP address from PHOP object */
  int  Previous_Hop_Address;

    /* Logical Interface Handle from PHOP object*/
  int  LIH;

    /* Remaining IP TTL */
  //int rem_IP_TTL;

    /* To determine EDGE of the RSVP network */
  int E_Police_Flag;

    /* To determine if the App. is local or not */
  int Local_Only_Flag;

    /* List of outgoing Interfaces for this
       (sender, destination)  single entry for unicast case */
  int  OutInterface_List;//[InLIST_SIZE];

    /* Expected incoming interface, undefined for unicast case */
  int  IncInterface ;
  LabelRequestObj_t LabelRequest;
  bool SAllocated; //Resource has been allocated ?

};

/*
** Reservation State Block (RSB) structure
*/
struct ResvStateBlock_t{

    /* SESSION object structure */
  SessionObj_t Session_Object;

    /* Next Hop IP address from PHOP object */
  int  Next_Hop_Address;

    /* Outgoing Interface on which reservation is
       to be made or has been made*/
  int  OI;


  FilterSpecObj_t Filter_Spec_Object[InLIST_SIZE];
  int            label[InLIST_SIZE];
  int            RRO[InLIST_SIZE][MAX_ROUTE];

    /* STYLE Object structure */
  //StyleObj_t Style_Object
  int style;

    /* FLOWSPEC structure */
  FlowSpecObj_t Flowspec_Object;

    /* SCOPE Object structure */
  //ScopeObj_t Scope_Object             ;

    /* RESV_CONFIRM Object structure */
  int Receiver_Address      ;

};

/*
** Traffic Control State Block
*/
struct TrafficControlStateBlock_t{

    /* SESSION object structure */
  SessionObj_t Session_Object;

    /* Outgoing Interface on which reservation is
       to be made or has been made*/
  int  OI;

    /* Newly added when it was known that FF style need to be considered */
  FilterSpecObj_t Filter_Spec_Object[InLIST_SIZE];

    /* Effective flowspec (LUB of FLOWSPEC's from matching RSB's) structure */
  FlowSpecObj_t TC_Flowspec;

    /* Updated object structure to be forwarded after merging */
  FlowSpecObj_t Fwd_Flowspec;

    /* Effective sender Tspec structure */
  SenderTspecObj_t TC_Tspec;

    /* Entry Police flags */
  int E_Police_Flag;

  int Local_Only_Flag;

    /* Reservation Handle */
  int Rhandle;

    /* Filter handle list */
  int Fhandle;

    /* RESV_CONFIRM  */
  int  Receiver_Address;

};


struct RHandleType_t{
    int handle; //Equivalent with TE_Tunnel_id
    int isfull;
    int  OI;
    int holdingPri; //Holding Pri of the tunnel
    int setupPri; //Setup Pri of the tunnel
    FlowSpecObj_t TC_Flowspec;
    SenderTspecObj_t Path_Te;
} ;

/*
typedef struct {
    int isfull;
    IPAddress OI;
    int Rhandle;
    SessionObj session;
    FilterSpecObj FilterSpecObject;
} FHandleType;
*/

class RSVP :  public cSimpleModule
{
private:
    RoutingTableAccess routingTableAccess;
    LIBTableAccess libTableAccess;

    std::vector<PathStateBlock_t> PSBList;  //Path State Block
    std::vector<ResvStateBlock_t> RSBList;  //Resv State Block
    std::vector<TrafficControlStateBlock_t> TCSBList; //Traffic Control State Block
    std::vector<TELinkState>  ted;

    int my_id;
    int NoOfLinks;
    int LocalAddress[InLIST_SIZE ];
    bool IsIR;
    bool IsER;

    std::vector<RHandleType_t> FlowTable;

    /*Message Processing */
    void PathMsgPro(PathMessage *pmsg, int InIf);
    void ResvMsgPro(ResvMessage *rmsg);
    void PTearMsgPro(PathTearMessage *pmsg);
    void RTearMsgPro(ResvTearMessage *rmsg);
    void PErrorMsgPro(PathErrorMessage *pmsg);
    void RErrorMsgPro(ResvErrorMessage *rmsg);


    bool IsLocalAddress(int ip);
    void PathRefresh( PathStateBlock_t *psbEle , int OI, EroObj_t* ero ) ;
        //PathRefresh( PathStateBlock_t *psbEle , int OI, int* ero );
    void ResvRefresh( ResvStateBlock_t *rsbEle, int PH );
    void RTearFwd(ResvStateBlock_t *rsbEle, int PH);
    int UpdateTrafficControl(ResvStateBlock_t *activeRSB);
    void RemoveTrafficControl(ResvStateBlock_t *activeRSB);
    int TC_AddFlowspec( int tunnelId, int holdingPri, int setupPri, int OI,
                        FlowSpecObj_t fs, SenderTspecObj_t ts, FlowSpecObj_t *fwdFS);
    int TC_ModFlowspec( int tunnelId, int OI,
                        FlowSpecObj_t fs, SenderTspecObj_t ts, FlowSpecObj_t *fwdFS);
    int GetFwdFS(int oi, FlowSpecObj_t *fwdFS);
    bool AllocateResource(int tunnelId, int holdingPri, int setupPri,
                    int oi, FlowSpecObj_t fs);

    void updateTED();
    void getPeerIPAddress(int dest, int* peerIP, int* peerInf);
    void getPeerInet(int peerIP, int* peerInf);
    void getIncInet(int peerIP, int* incInet);
    void getPeerIPAddress(int peerInf, int* peerIP);

    void printSessionObject(SessionObj_t* s);
    void printRSVPHopObject(RsvpHopObj_t* r);
    void printSenderTemplateObject(SenderTemplateObj_t* s);
    void printSenderTspecObject(SenderTspecObj_t* s);
    void printSenderDescriptorObject(SenderDescriptor_t* s);
    void printFlowDescriptorListObject(FlowDescriptor_t* f);
    void printPSB(PathStateBlock_t* p);
    void printRSB(ResvStateBlock_t* r);

    void setSessionforTCSB(TrafficControlStateBlock_t* t, SessionObj_t* s);
    void setFilterSpecforTCSB(TrafficControlStateBlock_t* t, FilterSpecObj_t *f);
    void setTCFlowSpecforTCSB(TrafficControlStateBlock_t* t, FlowSpecObj_t *f);
    void setFwdFlowSpecforTCSB(TrafficControlStateBlock_t* t, FlowSpecObj_t* f);
    void setTCTspecforTCSB(TrafficControlStateBlock_t* t, SenderTspecObj_t* s);
    void printTCSB(TrafficControlStateBlock_t* t);

    bool doCACCheck(PathMessage* pmsg, int OI);
    void preemptTunnel(int tunnelId);
    void propagateTEDchanges();

 public:

    Module_Class_Members(RSVP, cSimpleModule, 16384);

    virtual void initialize();
    virtual void activity();
    void Mcast_Route_Query(int sa, int iad, int da, int *outl);

};












#endif
