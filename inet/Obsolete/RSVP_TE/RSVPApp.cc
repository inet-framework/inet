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
#include <omnetpp.h>
#include "RSVPApp.h"
#include "RoutingTable.h"
#include "RSVPTesterCommands.h"
#include "MPLSModule.h"
#include "IPAddressResolver.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"

Define_Module(RSVPAppl);


void RSVPAppl::initialize(int stage)
{
    // we have to wait for stage 2 until interfaces get registered (stage 0)
    // and get their auto-assigned IP addresses (stage 2)
    if (stage!=3)
        return;

    // Get router IP Address
    ift = InterfaceTableAccess().get();
    rt = RoutingTableAccess().get();

    routerId = rt->getRouterId().getInt();
    ASSERT(routerId!=0);

    isIR = par("isIR").boolValue();
    isER = par("isER").boolValue();
    const char *trafficFile = par("traffic").stringValue(); // FIXME change to xml-typed NED param

    if (isIR)
    {
        initFromFile(ev.getXMLDocument(trafficFile));

        /*
        // "tell MPLS we're ready" -- FIXME remove this code
        MPLSModule *mplsMod = mplsAccess.get();
        cMessage *readyMsg = new cMessage();
        sendDirect(readyMsg, 0.0, mplsMod, "fromSignalModule");
        */
    }
}


void RSVPAppl::handleMessage(cMessage *msg)
{
    if (!strcmp(msg->arrivalGate()->name(), "from_rsvp"))
    {
        processMsgFromRSVP(msg);
    }
    else if (!strcmp(msg->arrivalGate()->name(), "from_mpls_switch"))
    {
        processSignalFromMPLSSwitch(msg);
    }
    else if (!strcmp(msg->arrivalGate()->name(), "from_tester"))
    {
        processSignalFromTester(msg);
    }
    else
    {
        error("Unexpected message received");
    }
}

void RSVPAppl::processMsgFromRSVP(cMessage *msg)
{
    switch(msg->kind())
    {
        case PERROR_MESSAGE: processRSVP_PERROR(check_and_cast<RSVPPathError *>(msg)); break;
        case PTEAR_MESSAGE:  processRSVP_PTEAR(check_and_cast<RSVPPathTear *>(msg)); break;
        case RTEAR_MESSAGE:  processRSVP_RTEAR(check_and_cast<RSVPResvTear *>(msg)); break;
        case PATH_MESSAGE:   processRSVP_PATH(check_and_cast<RSVPPathMsg *>(msg)); break;
        case RESV_MESSAGE:   processRSVP_RESV(check_and_cast<RSVPResvMsg *>(msg)); break;
        default: error("unrecognised RSVP message, kind=%d", msg->kind());
    }
}

void RSVPAppl::processRSVP_PERROR(RSVPPathError *pe)
{
    // create new PATH TEAR
    RSVPPathTear *pt = new RSVPPathTear("PathTear");
    pt->setSenderTemplate(pe->getSenderTemplate());
    pt->setSession(pe->getSession());

    // Setup PHOP
    RsvpHopObj_t hop;
    hop.Logical_Interface_Handle = -1;
    hop.Next_Hop_Address = routerId;
    pt->setHop(hop);
    send(pt, "to_rsvp");

    delete pe;
}

void RSVPAppl::processRSVP_PTEAR(RSVPPathTear *msg)
{
    ev << "Successfully received the PTEAR at ER\n";
    delete msg;
}

void RSVPAppl::processRSVP_RTEAR(RSVPResvTear *msg)
{
    ev << "Successfully received the RTEAR at IR\n";
    delete msg;
}

void RSVPAppl::processRSVP_PATH(RSVPPathMsg *pMessage)
{
    LIBTable *lt = libTableAccess.get();

    IPADDR receiverIP = pMessage->getDestAddress();
    int lspId = pMessage->getLspId();
    int outInf = 0;
    int inInf = 0;

    getIncInet(pMessage->getLIH(), &inInf);
    Unicast_Route_Query(receiverIP, &outInf);

    // Convert to name
    InterfaceEntry *outInfP = rt->interfaceByAddress(IPAddress(outInf));
    InterfaceEntry *inInfP = rt->interfaceByAddress(IPAddress(inInf));
    if (!outInfP) error("no interface with outInf address %s",IPAddress(outInf).str().c_str());
    if (!inInfP) error("no interface with inInf address %s",IPAddress(inInf).str().c_str());

    const char *outInfName = outInfP->name();
    const char *inInfName = inInfP->name();

    if (isER)
    {
        int inLabel = lt->installNewLabel(-1, string(inInfName), string(outInfName), lspId, POP_OPER);
        ev << "INSTALL new label:\n";
        ev << "(inL, inInf, outL, outInf,fec)=(," << inLabel << "," << inInfName <<
            "," << "-1," << outInfName << "," << lspId << ")\n";

        // Send LABEL MAPPING upstream
        sendResvMessage(pMessage, inLabel);
    }
    else
    {
        delete pMessage;
    }
}


void RSVPAppl::processRSVP_RESV(RSVPResvMsg *rMessage)
{
    LIBTable *lt = libTableAccess.get();
    MPLSModule *mplsMod = mplsAccess.get();

    if (!isIR)
    {
        ev << "We're not ingress router, ignoring message\n";
        delete rMessage;
        return;
    }

    ev << "Processing RESV message\n";

    std::vector<lsp_tunnel_t>::iterator iterF;
    lsp_tunnel_t aTunnel;
    int lsp_id;
    int label;

    FlowDescriptor_t *flow_d = rMessage->getFlowDescriptorList();
    for (int k = 0; k < InLIST_SIZE; k++)
    {
        for (iterF = FecSenderBinds.begin(); iterF != FecSenderBinds.end(); iterF++)
        {
            aTunnel = (lsp_tunnel_t) (*iterF);

            if (aTunnel.Sender_Template.SrcAddress == 0)
                continue;
            lsp_id = flow_d[k].Filter_Spec_Object.Lsp_Id;

            // If the tunnel has been in operating, ignore this Resv Message
            // Otherwise, signal the MPLS module
            // If the Resv Message is to re-route, signal the MPLS module as well
            if (lsp_id == aTunnel.Sender_Template.Lsp_Id ||
                (2 * MAX_LSP_NO - lsp_id) == aTunnel.Sender_Template.Lsp_Id)
            {
                if ((!aTunnel.operating) ||
                    ((2 * MAX_LSP_NO - lsp_id) == aTunnel.Sender_Template.Lsp_Id))
                {

                    // Delete all invalid record before
                    std::vector < routing_info_t >::iterator iterR;
                    routing_info_t rEle;

                    for (iterR = routingInfo.begin(); iterR != routingInfo.end(); iterR++)
                    {
                        rEle = (routing_info_t) * iterR;
                        if ((rEle.lspId == lsp_id) || (rEle.lspId == (2 * MAX_LSP_NO - lsp_id)))
                        {
                            break;
                        }

                    }
                    if (iterR != routingInfo.end())
                        routingInfo.erase(iterR);

                    // Add new record

                    bool includeMe = false;
                    routing_info_t *rInfo = new routing_info_t;
                    rInfo->lspId = lsp_id;
                    ev << "Record route for LSP with id=" << lsp_id << "\n";
                    ev << "Route ={ ";

                    for (int c = 0; c < MAX_ROUTE; c++)
                    {
                        (rInfo->route)[c] = flow_d[k].RRO[c];
                        if (((rInfo->route)[c] == 0) && (!includeMe))
                        {
                            (rInfo->route)[c] = routerId;
                            includeMe = true;
                        }
                        ev << IPAddress((rInfo->route)[c]) << " ";
                    }
                    ev << "\n";

                    routingInfo.push_back(*rInfo);

                    cMessage *signalMPLS = new cMessage("path created");
                    label = flow_d[k].label;
                    signalMPLS->addPar("label") = label;
                    signalMPLS->addPar("fecId") = lsp_id;
                    // signalMPLS->addPar("src") = aTunnel.Sender_Template.SrcAddress;
                    // signalMPLS->addPar("dest") = aTunnel.Session.DestAddress;
                    // Install new label
                    int outInf = rMessage->getLIH();
                    int outInfIndex = rt->interfaceByAddress(IPAddress(outInf))->outputPort(); // FIXME ->outputPort(): is this OK? --AV
                    const char *outInfName = (ift->interfaceByPortNo(outInfIndex))->name();
                    const char *inInfName = (ift->interfaceByPortNo(aTunnel.inInfIndex))->name();

                    ev << "INSTALL new label \n";
                    ev << "src=" << IPAddress(aTunnel.Sender_Template.
                                              SrcAddress) << "\n";
                    lt->installNewLabel(label, string(inInfName),
                                        string(outInfName), lsp_id, PUSH_OPER);

                    aTunnel.operating = true;
                    sendDirect(signalMPLS, 0.0, mplsMod, "fromSignalModule");

                    /*************************************************
                    Sending Path Tear Message
                    **************************************************/
                    if (lsp_id > MAX_LSP_NO)
                    {
                        RSVPPathTear *pt = new RSVPPathTear("PathTear");
                        SenderTemplateObj_t sTemplate;
                        sTemplate.Lsp_Id = 2 * MAX_LSP_NO - lsp_id;
                        sTemplate.SrcAddress = flow_d[k].Filter_Spec_Object.SrcAddress;
                        sTemplate.SrcPort = flow_d[k].Filter_Spec_Object.SrcPort;

                        pt->setSenderTemplate(sTemplate);
                        pt->setSession(rMessage->getSession());
                        // Setup PHOP
                        RsvpHopObj_t hop;
                        hop.Logical_Interface_Handle = -1;
                        hop.Next_Hop_Address = routerId;
                        pt->setHop(hop);
                        send(pt, "to_rsvp");
                    }
                }
            }
        }
    }
    delete rMessage;
}


void RSVPAppl::processSignalFromMPLSSwitch(cMessage *msg)
{
    if (msg->hasPar("teardown"))
        processSignalFromMPLSSwitch_TEAR_DOWN(msg);
    else
        processSignalFromMPLSSwitch_PATH_REQUEST(msg);
}

void RSVPAppl::processSignalFromMPLSSwitch_TEAR_DOWN(cMessage *msg)
{
    ev << "Tear down request\n";

    int dest = msg->par("dest_addr");
    int src = msg->par("src_addr");

    lsp_tunnel_t aTunnel;
    std::vector<lsp_tunnel_t>::iterator iterF;
    for (iterF = FecSenderBinds.begin(); iterF != FecSenderBinds.end(); iterF++)
    {
        aTunnel = (lsp_tunnel_t) (*iterF);
        if (aTunnel.Session.DestAddress == dest &&
            aTunnel.Sender_Template.SrcAddress == src)
        {
            break;
        }
    }
    if (iterF == FecSenderBinds.end())
        error("Cannot find the session to teardown");

    RSVPPathTear *pt = new RSVPPathTear("PathTear");
    pt->setSenderTemplate(aTunnel.Sender_Template);
    pt->setSession(aTunnel.Session);
    RsvpHopObj_t hop;
    hop.Logical_Interface_Handle = -1;
    hop.Next_Hop_Address = routerId;
    pt->setHop(hop);
    send(pt, "to_rsvp");

    delete msg;
}

void RSVPAppl::processSignalFromMPLSSwitch_PATH_REQUEST(cMessage *msg)
{
    ev << "Handling PATH REQUEST\n";

    int index = msg->par("gateIndex");
    int dest = msg->par("dest_addr");
    int src = msg->par("src_addr");
    int fecInt = msg->par("fecId");

    int lspId = fecInt;

    lsp_tunnel_t *newTunnel = NULL;
    int tunnelId = 0;
    int ex_tunnelId = 0;
    bool foundSession = false;

    // check if any similar previous requests

    /*
       Check whether the request had been recorded.
       Initiliaze new TE-Tunnel Id, and lsp id with new values if not found
     */

    std::vector<lsp_tunnel_t>::iterator iterF;
    for (iterF = FecSenderBinds.begin(); iterF != FecSenderBinds.end(); iterF++)
    {
        lsp_tunnel_t aTunnel = (lsp_tunnel_t) (*iterF);
        if (aTunnel.Session.DestAddress == dest)
        {
            tunnelId = aTunnel.Session.Tunnel_Id;
            ex_tunnelId = aTunnel.Session.Extended_Tunnel_Id;
            foundSession = true;

            if (aTunnel.Sender_Template.SrcAddress == src)
            {
                newTunnel = &aTunnel;  // FIXME what for? we obviously won't use this value
                break;
            }
        }
        if (!foundSession)
        {
            if (tunnelId < aTunnel.Session.Tunnel_Id)
                tunnelId = aTunnel.Session.Tunnel_Id + 1;

            if (ex_tunnelId < aTunnel.Session.Extended_Tunnel_Id)
                ex_tunnelId = aTunnel.Session.Extended_Tunnel_Id + 1;
        }
    }

    if (iterF != FecSenderBinds.end())
    {
        // There is already a previous same requests
        delete msg;
        return;
    }

    newTunnel = new lsp_tunnel_t;
    newTunnel->operating = false;
    newTunnel->Sender_Template.Lsp_Id = lspId;
    newTunnel->Sender_Template.SrcAddress = src;
    newTunnel->Sender_Template.SrcPort = DEFAULT_SRC_PORT;
    newTunnel->Session.DestAddress = dest;
    newTunnel->Session.DestPort = DEFAULT_DEST_PORT;
    newTunnel->Session.Protocol_Id = 1;
    newTunnel->Session.holdingPri = 7;      // Lowest
    newTunnel->Session.setupPri = 7;

    newTunnel->Session.Extended_Tunnel_Id = ex_tunnelId;
    newTunnel->Session.Tunnel_Id = tunnelId;
    newTunnel->inInfIndex = index;

    // Genarate new PATH message and send downstream
    std::vector < traffic_request_t >::iterator iterT;
    traffic_request_t trafficR;

    for (iterT = tr.begin(); iterT != tr.end(); iterT++)
    {
        trafficR = (traffic_request_t) * iterT;
        if (trafficR.dest == dest && trafficR.src == src)
        {
            newTunnel->Session.holdingPri = trafficR.holdingPri;
            newTunnel->Session.setupPri = trafficR.setupPri;
            break;
        }
    }

    FecSenderBinds.push_back(*newTunnel);

    if (iterT != tr.end())
        sendPathMessage(&newTunnel->Session, &trafficR, lspId);
    else
    {
        traffic_request_t *aTR = new traffic_request_t;
        aTR->bandwidth = 0;
        aTR->delay = 0;
        aTR->dest = dest;
        aTR->holdingPri = 7;
        aTR->setupPri = 7;
        aTR->src = src;
        aTR->isER = false;

        sendPathMessage(&newTunnel->Session, aTR, lspId);
    }
    delete msg;
}

void RSVPAppl::processSignalFromTester(cMessage *msg)
{
    ev << "Received msg from Tester\n";

    if (!(msg->hasPar("test_command")))
        error("Command message from Tester doesn't have test_command parameter");

    // Process the test command
    int command = msg->par("test_command").longValue();
    switch (command)
    {
        case NEW_BW_REQUEST:     processCommand_NEW_BW_REQUEST(msg); break;
        case NEW_ROUTE_DISCOVER: processCommand_NEW_ROUTE_DISCOVER(msg); break;
        default: error("unrecognised command from Tester, test_command=%d", command);
    }
}


void RSVPAppl::processCommand_NEW_BW_REQUEST(cMessage *msg)
{
    int rSrc = IPAddress(msg->par("src").stringValue()).getInt();
    int rDest = IPAddress(msg->par("dest").stringValue()).getInt();
    double bw = msg->par("bandwidth").doubleValue();

    int k = 0;
    int n = 0;
    int newLsp_id = -1;
    for (k = 0; k < tr.size(); k++)
    {
        if (tr[k].src == rSrc && tr[k].dest == rDest)
        {
            tr[k].bandwidth = bw;
            ev << "New Bandwidth request = " << bw << " \n";
            break;
        }
    }
    if (k == tr.size())
    {
        delete msg;
        return;
    }

    for (n = 0; n < FecSenderBinds.size(); n++)
    {
        if (tr[k].dest == FecSenderBinds[n].Session.DestAddress &&
            tr[k].src == FecSenderBinds[n].Sender_Template.SrcAddress)
        {
            newLsp_id = FecSenderBinds[n].Sender_Template.Lsp_Id;
            break;
        }
    }

    if (n == FecSenderBinds.size())
    {
        delete msg;
        return;
    }

    newLsp_id = 2 * MAX_LSP_NO - newLsp_id;

    sendPathMessage(&FecSenderBinds[n].Session, &tr[k], newLsp_id);

    delete msg;
}


void RSVPAppl::processCommand_NEW_ROUTE_DISCOVER(cMessage *msg)
{
    std::vector < routing_info_t >::iterator iterR;
    routing_info_t rInfo;

    for (iterR = routingInfo.begin(); iterR != routingInfo.end(); iterR++)
    {
        rInfo = (routing_info_t) *iterR;
        // Try to find new route
        ev << "CSPF calculates to find new path for LSP with id =" << rInfo.lspId << "\n";
        if (hasPath(rInfo.lspId, NULL))
        {
            ev << "Going to install new path\n";
            break;
        }
    }

    std::vector < lsp_tunnel_t >::iterator iterF;
    lsp_tunnel_t aTunnel;
    int newLspId = 2 * MAX_LSP_NO - rInfo.lspId;

    for (iterF = FecSenderBinds.begin(); iterF != FecSenderBinds.end(); iterF++)
    {
        aTunnel = (lsp_tunnel_t) (*iterF);
        if (aTunnel.Sender_Template.Lsp_Id == rInfo.lspId)
            break;
    }
    if (iterF == FecSenderBinds.end())
        error("cannot locate the tunnel");

    std::vector < traffic_request_t >::iterator iterT;
    traffic_request_t trafficR;

    for (iterT = tr.begin(); iterT != tr.end(); iterT++)
    {
        trafficR = (traffic_request_t) *iterT;
        if (trafficR.dest == aTunnel.Session.DestAddress &&
            trafficR.src == aTunnel.Sender_Template.SrcAddress)
        {
            break;
        }
    }

    if (iterT == tr.end())
    {
        ev << "No traffic spec required for the LSP\n";
        delete msg;
        return; // was: continue;
    }

    // Create new Path message and send out
    ev << "Sending Path message for the new discovered route\n";
    sendPathMessage(&aTunnel.Session, &trafficR, newLspId);
    delete msg;
}


void RSVPAppl::sendResvMessage(RSVPPathMsg * pMsg, int inLabel)
{
    ev << "Generate a Resv Message\n";
    RSVPResvMsg *rMsg = new RSVPResvMsg("Resv");
    RsvpHopObj_t *rsvp_hop = new RsvpHopObj_t;
    FlowSpecObj_t *Flowspec_Object = new FlowSpecObj_t;
    FilterSpecObj_t *Filter_Spec_Object = new FilterSpecObj_t;
    FlowSpecObj_t *Flowspec_Object_Default = new FlowSpecObj_t;
    FilterSpecObj_t *Filter_Spec_Object_Default = new FilterSpecObj_t;

    // Setup  rsvp hop
    rsvp_hop->Logical_Interface_Handle = -1;    // pMsg->getLIH();
    rsvp_hop->Next_Hop_Address = pMsg->getNHOP();


    /*
       Note this assume that the request bw = sender bw
       Need to make it more generic later
     */
    // Setup flowspec
    Flowspec_Object->link_delay = pMsg->getDelay();
    Flowspec_Object->req_bandwidth = pMsg->getBW();

    Flowspec_Object_Default->link_delay = pMsg->getDelay();
    Flowspec_Object_Default->req_bandwidth = pMsg->getBW();

    // Setup Filterspec
    Filter_Spec_Object->SrcAddress = pMsg->getSrcAddress();
    Filter_Spec_Object->SrcPort = pMsg->getSrcPort();
    Filter_Spec_Object->Lsp_Id = pMsg->getLspId();

    Filter_Spec_Object_Default->SrcAddress = 0;
    Filter_Spec_Object_Default->SrcPort = 0;
    Filter_Spec_Object_Default->Lsp_Id = -1;


    FlowDescriptor_t *flow_descriptor_list = new FlowDescriptor_t[InLIST_SIZE];
    flow_descriptor_list[0].Filter_Spec_Object = (*Filter_Spec_Object);
    flow_descriptor_list[0].Flowspec_Object = (*Flowspec_Object);

    // flow_descriptor_list[0].RRO[0] = routerId;
    for (int c = 0; c < MAX_ROUTE; c++)
        flow_descriptor_list[0].RRO[c] = 0;

    flow_descriptor_list[0].label = inLabel;

    for (int i = 1; i < InLIST_SIZE; i++)
    {
        flow_descriptor_list[i].Filter_Spec_Object = (*Filter_Spec_Object_Default);
        flow_descriptor_list[i].Flowspec_Object = (*Flowspec_Object_Default);
        flow_descriptor_list[i].label = -1;
    }

    // Check if this one is equivalent to reroute request
    if ((pMsg->getLspId()) > MAX_LSP_NO)
    {
        int old_lsp_id = 2 * MAX_LSP_NO - (pMsg->getLspId());
        Filter_Spec_Object->Lsp_Id = old_lsp_id;
        flow_descriptor_list[1].Filter_Spec_Object = (*Filter_Spec_Object);
        flow_descriptor_list[1].Flowspec_Object = (*Flowspec_Object);
        for (int c = 0; c < MAX_ROUTE; c++)
            flow_descriptor_list[1].RRO[c] = 0;
    }

    rMsg->setFlowDescriptor(flow_descriptor_list);
    rMsg->setHop(*rsvp_hop);
    rMsg->setSession(pMsg->getSession());

    rMsg->setStyle(SF_STYLE);

    // int peerIp=0;
    // getPeerIPAddress(inInf, &peerIp);

    rMsg->addPar("dest_addr") = IPAddress(pMsg->getNHOP()).str().c_str();
    ev << "Next peer " << IPAddress(pMsg->getNHOP());
    ev << "RESV MESSAGE content: \n";
    print(rMsg);
    send(rMsg, "to_rsvp");
}



void RSVPAppl::sendPathMessage(SessionObj_t * s, traffic_request_t * t, int lspId)
{
    OspfTe *ospfte = ospfteAccess.get();

    RSVPPathMsg *pMsg = new RSVPPathMsg("Path");
    RsvpHopObj_t *rsvp_hop = new RsvpHopObj_t;
    FlowSpecObj_t *Flowspec_Object = new FlowSpecObj_t;

    FilterSpecObj_t *Filter_Spec_Object = new FilterSpecObj_t;

    // Setup PHOP
    rsvp_hop->Logical_Interface_Handle = -1;
    rsvp_hop->Next_Hop_Address = routerId;
    IPADDR destAddr = s->DestAddress;
    int outInterface;

    Unicast_Route_Query(destAddr, &outInterface);

    rsvp_hop->Logical_Interface_Handle = outInterface;

    // Setup Sender Descriptor
    Flowspec_Object->link_delay = t->delay;
    Flowspec_Object->req_bandwidth = t->bandwidth;

    Filter_Spec_Object->SrcAddress = t->src;
    Filter_Spec_Object->SrcPort = DEFAULT_SRC_PORT;
    Filter_Spec_Object->Lsp_Id = lspId;

    pMsg->setHop(*rsvp_hop);
    pMsg->setSession(*s);
    pMsg->setSenderTemplate(* static_cast < SenderTemplateObj_t * >(Filter_Spec_Object));
    pMsg->setSenderTspec(* static_cast < SenderTspecObj_t * >(Flowspec_Object));

    // Setup routing information
    pMsg->addPar("src_addr") = IPAddress(routerId).str().c_str();

    IPADDR peerIP = 0;
    int peerInf = 0;
    // Normal way to get the peerIP
    getPeerIPAddress(destAddr, &peerIP, &peerInf);


    pMsg->addPar("dest_addr") = IPAddress(peerIP).str().c_str();
    pMsg->addPar("peerInf") = peerInf;


    /**************************************************************
    More control options of ERO here
    ***************************************************************/


    // Calculate ERO

    EroObj_t ERO[MAX_ROUTE];
    for (int m = 0; m < MAX_ROUTE; m++)
    {
        ERO[m].node = 0;
        ERO[m].L = false;
    }
    // If the option of ER is on
    if (t->isER)
    {
        if (isIR)
        {
            if (t->route[0].node == 0)
            {
                // No input ER from the administrator for this path
                // Find ER based on CSPF routing algorithm
                ev << "OSPF PATH calculation: from " << IPAddress(routerId) <<
                    " to " << IPAddress(destAddr) << "\n";

                double delayTime = 0;
                std::vector<IPADDR> ero = ospfte->CalculateERO(destAddr, *Flowspec_Object, delayTime);
                if (ero.back() == 0)    // Unknow reason
                {
                    ero.pop_back();
                    ero.push_back(routerId);
                }

                // copy returned path into ERO structure
                int hopCount = 0;
                for (unsigned int n = 0; n < ero.size(); n++)
                {
                    ev << IPAddress(ero[n]) << "\n";
                    ERO[hopCount].node = ero[n];
                    ERO[hopCount].L = false;
                    hopCount++;
                }
                /*
                   if(hopCount>0)
                   {
                   for( ; hopCount-1 < MAX_ROUTE; hopCount++)
                   {
                   ERO[hopCount].node =0;
                   ERO[hopCount].L =false;
                   }
                   }
                   else
                 */
                if (hopCount == 0)
                {
                    ev << "No resource available\n";
                    // delete pmsg;
                    return;
                }

                // pMsg->setERO(ERO);
            }
            else                // Use the input from admin
            {
                int lastIndex = 0;
                int index = 0;
                for (lastIndex = 0; lastIndex < MAX_ROUTE; lastIndex++)
                    if (t->route[lastIndex].node == 0)
                        break;
                lastIndex--;
                for (; lastIndex >= 0; lastIndex--)
                {
                    ERO[index].L = t->route[lastIndex].L;
                    ERO[index].node = t->route[lastIndex].node;
                    index++;

                }
            }
        }
        pMsg->setHasERO(true);
        pMsg->setEROArray(ERO);
    }
    else                        // Hop-by-Hop routing
    {
        pMsg->setHasERO(false);

    }

    /*
       routing_info_t* rInfo = new routing_info_t;
       rInfo->lspId = lspId;
       for(int c=0;c<MAX_ROUTE;c++)
       (rInfo->route)[c]= ERO[c].node;
       routingInfo.push_back(*rInfo);
     */



    // delete [] ERO;


    // Finish buidling packet, send it off
    ev << "Final Dest Address is : " << IPAddress(destAddr) << "\n";
    ev << "Set Dest Address to: " << IPAddress(peerIP) << "\n";


    ev << "PATH message content sent:\n";
    print(pMsg);

    send(pMsg, "to_rsvp");
}



void RSVPAppl::Unicast_Route_Query(IPADDR da, int *outl)
{
    int foundIndex;
    // int j=0;
    foundIndex = rt->outputPortNo(IPAddress(da));
    (*outl) = ift->interfaceByPortNo(foundIndex)->ipv4()->inetAddress().getInt();   // FIXME why not return outl???

    return;

}

void RSVPAppl::Mcast_Route_Query(IPADDR sa, int iad, IPADDR da, int *outl)
{
    int foundIndex;
    // int j=0;
    foundIndex = rt->outputPortNo(IPAddress(da));
    (*outl) = ift->interfaceByPortNo(foundIndex)->ipv4()->inetAddress().getInt();   // FIXME why not return outl???

    return;
}

void RSVPAppl::getPeerInet(IPADDR peerIP, int *peerInf)
{
    updateTED();

    std::vector < TELinkState >::iterator tedIter;
    for (tedIter = ted.begin(); tedIter != ted.end(); tedIter++)
    {
        const TELinkState & linkstate = *tedIter;
        if (linkstate.linkid.getInt() == peerIP && linkstate.advrouter.getInt() == routerId)
        {
            (*peerInf) = linkstate.remote.getInt();
            break;
        }
    }
}

void RSVPAppl::getIncInet(int remoteInet, int *incInet)
{
    std::vector < TELinkState >::iterator tedIter;

    ev << "Find incoming inf for peerOutgoinglink=" << IPAddress(remoteInet) << "\n";
    updateTED();
    for (tedIter = ted.begin(); tedIter != ted.end(); tedIter++)
    {
        const TELinkState & linkstate = *tedIter;
        if (linkstate.remote.getInt() == remoteInet && linkstate.advrouter.getInt() == routerId)
        {
            (*incInet) = linkstate.local.getInt();
            break;
        }
    }
}

void RSVPAppl::getPeerIPAddress(IPADDR dest, IPADDR *peerIP, int *peerInf)
{
    int outl = 0;
    Mcast_Route_Query(0, 0, dest, &outl);
    updateTED();

    std::vector < TELinkState >::iterator tedIter;
    for (tedIter = ted.begin(); tedIter != ted.end(); tedIter++)
    {
        const TELinkState & linkstate = *tedIter;
        if (linkstate.local.getInt() == outl && linkstate.advrouter.getInt() == routerId)
        {
            *peerIP = linkstate.linkid.getInt();
            *peerInf = linkstate.remote.getInt();
            break;
        }
    }
}

void RSVPAppl::getPeerIPAddress(int peerInf, IPADDR *peerIP)
{
    updateTED();

    std::vector < TELinkState >::iterator tedIter;
    for (tedIter = ted.begin(); tedIter != ted.end(); tedIter++)
    {
        const TELinkState & linkstate = *tedIter;
        if (linkstate.local.getInt() == peerInf && linkstate.advrouter.getInt() == routerId)
        {
            *peerIP = linkstate.linkid.getInt();
            break;
        }
    }
}

void RSVPAppl::updateTED()
{
    // copy the full table
    ted = TED::getGlobalInstance()->getTED();
}


bool RSVPAppl::initFromFile(const cXMLElement *root)
{
    if (!root)
        throw new cRuntimeError("No traffic configuration");
    if (strcmp(root->getTagName(),"traffic"))
        throw new cRuntimeError("Traffic configuration: wrong document type, root node is not <traffic>");

    cXMLElementList list = root->getChildrenByTagName("conn");
    for (cXMLElementList::iterator i=list.begin(); i!=list.end(); i++)
    {
        traffic_request_t aTR = parseTrafficRequest(*i);
        tr.push_back(aTR);
    }
    return true;
}

RSVPAppl::traffic_request_t RSVPAppl::parseTrafficRequest(const cXMLElement *connNode)
{
    traffic_request_t aTR;
    for (int c = 0; c < MAX_ROUTE; c++)
    {
        aTR.route[c].node = 0;
        aTR.route[c].L = false;
    }
    aTR.isER = true;

    int rCount = 0;

    for (cXMLElement *child=connNode->getFirstChild(); child; child=child->getNextSibling())
    {
        if (!strcmp(child->getTagName(),"src"))
        {
            aTR.src = IPAddress(child->getNodeValue()).getInt();
        }
        else if (!strcmp(child->getTagName(),"dest"))
        {
            aTR.dest = IPAddress(child->getNodeValue()).getInt();
        }
        else if (!strcmp(child->getTagName(),"setupPri"))
        {
            aTR.setupPri = strtol(child->getNodeValue(), NULL, 0);
        }
        else if (!strcmp(child->getTagName(),"holdingPri"))
        {
            aTR.holdingPri = strtol(child->getNodeValue(), NULL, 0);
        }
        else if (!strcmp(child->getTagName(),"delay"))
        {
            aTR.delay = strtod(child->getNodeValue(), NULL);
        }
        else if (!strcmp(child->getTagName(),"bandwidth"))
        {
            aTR.bandwidth = strtod(child->getNodeValue(), NULL);
        }
        else if (!strcmp(child->getTagName(),"ER"))
        {
            const char *ER_Option = child->getNodeValue();
            ev << "ER Option is " << ER_Option << "\n";
            if ((!strcmp(ER_Option, "false")) || (!strcmp(ER_Option, "no")))
            {
                ev << "A hop-by-hop request\n";
                aTR.isER = false;
            }
        }
        else if (!strcmp(child->getTagName(),"route"))
        {
            const char *line = child->getNodeValue();
            cStringTokenizer tokenizer(line, ",");
            const char *aField;
            while ((aField = tokenizer.nextToken()) != NULL)
            {
                aTR.route[rCount].node = IPAddress(aField).getInt();
                if ((aField = tokenizer.nextToken()) != NULL)
                    aTR.route[rCount].L = atoi(aField);
                else
                    aTR.route[rCount].L = 0;
                ev << "Add node from admin, node=" << IPAddress(aTR.route[rCount].node) <<
                    " lBit=" << aTR.route[rCount].L << "\n";
                rCount++;
                if (rCount > MAX_ROUTE)
                    break;
            }
        }
    }
    ev << "Adding (src, dest, delay, bw) = (" << IPAddress(aTR.src) << "," <<
        IPAddress(aTR.dest) << "," << aTR.delay << "," << aTR.bandwidth << ")\n";
    if (aTR.holdingPri > aTR.setupPri)
        error("Holding priority is greater than setup priority (setup priority must be greater than or equal)");

    return aTR;
}



void RSVPAppl::addRouteInfo(RSVPResvMsg * rmsg)
{
    FlowDescriptor_t *flow_d = rmsg->getFlowDescriptorList();
    for (int k = 0; k < InLIST_SIZE; k++)
    {
        if (flow_d[k].Filter_Spec_Object.SrcAddress != 0)
        {
            routing_info_t *rInfo = new routing_info_t;
            rInfo->lspId = flow_d[k].Filter_Spec_Object.Lsp_Id;

            for (int c = 0; c < MAX_ROUTE; c++)
            {
                rInfo->route[c] = flow_d[k].RRO[c];
            }
            routingInfo.push_back(*rInfo);
        }
    }
}

bool RSVPAppl::hasPath(int lspid, FlowSpecObj_t * newFlowspec)
{
    OspfTe *ospfte = ospfteAccess.get();

    std::vector < lsp_tunnel_t >::iterator iterF;
    lsp_tunnel_t aTunnel;

    for (iterF = FecSenderBinds.begin(); iterF != FecSenderBinds.end(); iterF++)
    {
        aTunnel = (lsp_tunnel_t) (*iterF);
        if (aTunnel.Sender_Template.Lsp_Id == lspid)
            break;
    }
    if (iterF == FecSenderBinds.end())
    {
        ev << "No LSP with id =" << lspid << "\n";
        return false;
    }

    std::vector < traffic_request_t >::iterator iterT;
    traffic_request_t trafficR;

    for (iterT = tr.begin(); iterT != tr.end(); iterT++)
    {
        trafficR = (traffic_request_t) * iterT;
        if (trafficR.dest == aTunnel.Session.DestAddress &&
            trafficR.src == aTunnel.Sender_Template.SrcAddress)
        {
            break;
        }
    }

    if (iterT == tr.end())
    {
        ev << "No traffic spec required for LSP with id =" << lspid << "\n";
        return false;
    }

    // Locate the current used route
    std::vector < routing_info_t >::iterator iterR;
    routing_info_t rInfo;

    for (iterR = routingInfo.begin(); iterR != routingInfo.end(); iterR++)
    {
        rInfo = (routing_info_t) * iterR;
        if (rInfo.lspId == lspid)
            break;

    }
    if (iterR == routingInfo.end())
        ev << "Warning, no recorded path found\n";

    if (iterR == routingInfo.end())
    {
        ev << "No route currently used for LSP with id =" << lspid << "\n";
        return false;
    }

    // Get destination
    IPADDR destAddr = aTunnel.Session.DestAddress;

    // Get old flowspec
    FlowSpecObj_t *Flowspec_Object = new FlowSpecObj_t;
    Flowspec_Object->link_delay = trafficR.delay;
    Flowspec_Object->req_bandwidth = trafficR.bandwidth;

    // Get links used
    std::vector < simple_link_t > linksInUse;
    for (int c = 0; c < (MAX_ROUTE - 1); c++)
    {
        simple_link_t aLink;
        if ((rInfo.route[c] != 0) && (rInfo.route[c + 1] != 0))
        {
            aLink.advRouter = rInfo.route[c + 1];
            aLink.id = rInfo.route[c];
            linksInUse.push_back(aLink);

        }
    }

    double currentTotalDelay = getTotalDelay(&linksInUse);
    ev << "Current total path metric: " << currentTotalDelay << "\n";
    // Calculate ERO

    ev << "OSPF PATH calculation: from " << IPAddress(routerId) <<
        " to " << IPAddress(destAddr) << "\n";

    std::vector<IPADDR> ero;
    double totalDelay = 0;
    if (newFlowspec == NULL)
    {

        ero = ospfte->CalculateERO(destAddr, linksInUse, *Flowspec_Object, *Flowspec_Object, totalDelay);

        if (currentTotalDelay < totalDelay)
        {
            ev << "The metric for new route is " << totalDelay << "\n";
            ev << "No better route detected for LSP with id=" << lspid << "\n";
            return false;
        }
        else
        {
            ev << "New route discovered with metric=" << totalDelay <<
                " < current metric=" << currentTotalDelay << "\n";
        }
    }
    else
    {
        ero = ospfte->CalculateERO(destAddr, linksInUse, *Flowspec_Object, *newFlowspec, totalDelay);
    }

    std::vector<IPADDR>::iterator ero_iterI;

    int inx = 0;

    // compare the old route an new route, to the ER only
    for (ero_iterI = ero.begin(); ero_iterI != ero.end(); ero_iterI++)
    {
        if ((*ero_iterI) == rInfo.route[0])
            break;
    }

    for (; ero_iterI != ero.end(); ero_iterI++)
    {
        if (rInfo.route[inx] != (*ero_iterI))
            break;
        inx = inx + 1;
    }

    if (ero_iterI == ero.end())
        return false;

    // print old route and new route for debugging
    ev << "Old route for LSP with id=" << lspid << " was:\n";
    for (int k = 0; k < MAX_ROUTE; k++)
        ev << IPAddress(rInfo.route[k]) << " ";
    ev << "\n";

    ev << "New route for this LSP is :\n";
    for (ero_iterI = ero.begin(); ero_iterI != ero.end(); ero_iterI++)
    {
        ev << IPAddress((*ero_iterI)) << " ";
    }
    ev << "\n";
    return true;
}

double RSVPAppl::getTotalDelay(std::vector < simple_link_t > *links)
{
    double totalDelay = 0;

    std::vector < simple_link_t >::iterator iterS;
    simple_link_t aLink;

    for (iterS = (*links).begin(); iterS != (*links).end(); iterS++)
    {
        aLink = (simple_link_t) (*iterS);

        std::vector < TELinkState >::iterator tedIter;
        for (tedIter = ted.begin(); tedIter != ted.end(); tedIter++)
        {
            const TELinkState & linkstate = *tedIter;
            if ((linkstate.linkid.getInt() == aLink.id) &&
                (linkstate.advrouter.getInt() == aLink.advRouter))
            {
                totalDelay = totalDelay + linkstate.metric;
            }
        }
    }
    return totalDelay;
}
