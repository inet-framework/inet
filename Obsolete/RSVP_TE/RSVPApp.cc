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
#include "RSVPTester.h"
#include "MPLSModule.h"
#include "StringTokenizer.h"

Define_Module( RSVPAppl);


void RSVPAppl::initialize()
{
    //Get router IP Address

    local_addr   = IPAddress(par("local_addr").stringValue()).getInt();
    //dest_addr   = IPAddress(par("dest_addr").stringValue()).getInt();

    isSender = par("isSender").boolValue();
    isIR = par("isIR").boolValue();
    isER = par("isER").boolValue();
    const char* trafficFile = par("traffic").stringValue();

    if(isIR)
        initFromFile(trafficFile);
}



void RSVPAppl::activity()
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();
    MPLSModule *mplsMod = mplsAccess.get();

    cMessage *msg;

    if(isIR)
    {
        cMessage* readyMsg = new cMessage();
        sendDirect(readyMsg, 0.0, mplsMod, "fromSignalModule");
    }
    /*
    if(isSender)
    {
        cMessage* sendMsg = new cMessage();
        sendMsg->addPar("dest_addr") = IPAddress(dest_addr).getString();
        sendMsg->addPar("src_addr") = IPAddress(local_addr).getString();
        send(sendMsg, "to_ip");
    }
    */

    while(true)
    {
      msg = receive();

/******************************************************************************
            Process packets from RSVP deamon
*******************************************************************************/
      if (!strcmp(msg->arrivalGate()->name(), "from_rsvp"))
      {

          if(msg->kind() == PERROR_MESSAGE)
          {
              PathErrorMessage* pe = check_and_cast<PathErrorMessage*>(msg);

              //create new PATH TEAR
              PathTearMessage* pt = new PathTearMessage();
              pt->setSenderTemplate(pe->getSenderTemplate());
              pt->setSession(pe->getSession());
              //Setup PHOP
              RsvpHopObj_t* rsvp_hop = new RsvpHopObj_t;
              rsvp_hop->Logical_Interface_Handle =-1;
              rsvp_hop->Next_Hop_Address = local_addr;
              pt->setHop(rsvp_hop);
              send(pt, "to_rsvp");


          }
          if(msg->kind() == PTEAR_MESSAGE)
          {
              ev << "Successfully receive the PTEAR at ER\n";
              //delete msg;
          }
          if(msg->kind() == RTEAR_MESSAGE)
          {
              ev << "Successfully receive the RTEAR at IR\n";
              //delete msg;
          }
          if(msg->kind() == PATH_MESSAGE)
          {

              PathMessage* pMessage = check_and_cast<PathMessage*>(msg);

              int receiverIP = pMessage->getDestAddress();
              int lspId = pMessage->getLspId();
              int outInf =0;
              int inInf = 0;

              getIncInet( pMessage->getLIH(),&inInf);
              Unicast_Route_Query(receiverIP, &outInf);

              //Convert to name
              int outInfIndex=rt->findInterfaceByAddress(IPAddress(outInf));
              int inInfIndex= rt->findInterfaceByAddress(IPAddress(inInf));

              const char* outInfName = rt->getInterfaceByIndex(outInfIndex)->name.c_str();
              const char* inInfName = rt->getInterfaceByIndex(inInfIndex)->name.c_str();

              if(isER)
              {
                int inLabel= (lt->installNewLabel(-1, string(inInfName),string(outInfName),lspId, POP_OPER));
                ev << "INSTALL new label:\n";
                ev << "(inL, inInf, outL, outInf,fec)=(," << inLabel << "," << inInfName <<
                "," << "-1," << outInfName << "," << lspId << ")\n";

                 //Send LABEL MAPPING upstream
                sendResvMessage(pMessage, inLabel);


              }
          }
          else if(msg->kind() == RESV_MESSAGE)
          {
              ResvMessage* rMessage = check_and_cast<ResvMessage*>(msg);
              if(isIR )
              {
                  std::vector<lsp_tunnel_t>::iterator iterF;
                  lsp_tunnel_t aTunnel;
                  int lsp_id;
                  int label;

                  FlowDescriptor_t* flow_d = rMessage->getFlowDescriptor();
                  for(int k=0;k<InLIST_SIZE;k++)
                  {

                      for(iterF=FecSenderBinds.begin();iterF!= FecSenderBinds.end();iterF++)
                      {
                          aTunnel = (lsp_tunnel_t)(*iterF);

                          if(aTunnel.Sender_Template.SrcAddress==0)
                          continue;
                            lsp_id=(*(flow_d+k)).Filter_Spec_Object.Lsp_Id;

                            //If the tunnel has been in operating, ignore this Resv Message
                            //Otherwise, signal the MPLS module
                            //If the Resv Message is to re-route, signal the MPLS module as well
                          if(lsp_id == aTunnel.Sender_Template.Lsp_Id ||
                              (2*MAX_LSP_NO - lsp_id) == aTunnel.Sender_Template.Lsp_Id)
                          {
                              if((!aTunnel.operating) ||
                                  ((2*MAX_LSP_NO - lsp_id) == aTunnel.Sender_Template.Lsp_Id))
                              {

                                  //Delete all invalid record before
                                  std::vector<routing_info_t>::iterator iterR;
                                  routing_info_t rEle;

                                  for(iterR = routingInfo.begin(); iterR != routingInfo.end(); iterR++)
                                  {
                                      rEle = (routing_info_t)*iterR;
                                      if((rEle.lspId == lsp_id) || (rEle.lspId == (2*MAX_LSP_NO - lsp_id)))
                                      {
                                          break;
                                      }

                                  }
                                  if(iterR != routingInfo.end())
                                      routingInfo.erase(iterR);

                                    //Add new record

                                    bool includeMe=false;
                                    routing_info_t* rInfo = new routing_info_t;
                                    rInfo->lspId = lsp_id;
                                    ev << "Record route for LSP with id=" << lsp_id << "\n";
                                    ev << "Route ={ ";

                                    for(int c=0;c<MAX_ROUTE;c++)
                                    {
                                        (rInfo->route)[c]= (*(flow_d+k)).RRO[c];
                                        if(((rInfo->route)[c] ==0) && (!includeMe))
                                        {
                                            (rInfo->route)[c] =local_addr;
                                            includeMe =true;
                                        }
                                        ev << IPAddress((rInfo->route)[c]).getString() << " ";
                                    }
                                    ev << "\n";


                                    routingInfo.push_back(*rInfo);

                                    cMessage* signalMPLS = new cMessage();
                                    label =(*(flow_d+k)).label;
                                    signalMPLS->addPar("label") = label;
                                    signalMPLS->addPar("fec")= lsp_id;
                                    //signalMPLS->addPar("src") = aTunnel.Sender_Template.SrcAddress;
                                    //signalMPLS->addPar("dest") = aTunnel.Session.DestAddress;
                                    //Install new label
                                    int outInf = rMessage->getLIH();
                                    int outInfIndex = rt->findInterfaceByAddress(IPAddress(outInf));
                                    const char* outInfName = (rt->getInterfaceByIndex(outInfIndex))->name.c_str();
                                    const char* inInfName =  (rt->getInterfaceByIndex(aTunnel.inInfIndex))->name.c_str();

                                    ev << "INSTALL new label \n";
                                    ev << "src=" <<   IPAddress(aTunnel.Sender_Template.SrcAddress).getString()<<"\n";
                                    lt->installNewLabel(label, string(inInfName), string(outInfName),lsp_id, PUSH_OPER);

                                    aTunnel.operating =true;
                                    sendDirect(signalMPLS, 0.0, mplsMod, "fromSignalModule");

                                    /*************************************************
                                    Sending Path Tear Message
                                    **************************************************/
                                    if(lsp_id > MAX_LSP_NO)
                                    {
                                        PathTearMessage* pt = new PathTearMessage();
                                        SenderTemplateObj_t* sTemplate = new SenderTemplateObj_t;
                                        sTemplate->Lsp_Id = 2*MAX_LSP_NO - lsp_id;
                                        sTemplate->SrcAddress = (*(flow_d+k)).Filter_Spec_Object.SrcAddress;
                                        sTemplate->SrcPort=(*(flow_d+k)).Filter_Spec_Object.SrcPort;

                                        pt->setSenderTemplate(sTemplate);
                                        pt->setSession(rMessage->getSession());
                                        //Setup PHOP
                                        RsvpHopObj_t* rsvp_hop = new RsvpHopObj_t;
                                        rsvp_hop->Logical_Interface_Handle =-1;
                                        rsvp_hop->Next_Hop_Address = local_addr;
                                        pt->setHop(rsvp_hop);
                                        send(pt, "to_rsvp");
                                    }

                              }
                          }





                      }
                  }


              }

          }


        delete msg;

      }

      /***********************************************************************
         Signal from MPLS switch - Initialise Lable Request
        ***********************************************************************/

      else if (!strcmp(msg->arrivalGate()->name(), "from_mpls_switch"))
      {
          //This is a request for new label finding
          std::vector<lsp_tunnel_t>::iterator iterF;
          lsp_tunnel_t aTunnel;
          lsp_tunnel_t * newTunnel = NULL;
          int tunnelId=0;
          int ex_tunnelId =0;
          bool foundSession =false;

          int fecInt = msg->par("FEC");
          int dest =msg->par("dest_addr");
          int src = msg->par("src_addr");

          int lspId =fecInt;

          //TEAR DOWN HANDLING
          if(msg->hasPar("teardown"))
          {
              ev << "Tear down request\n";
              PathTearMessage* pt = new PathTearMessage();
               for(iterF=FecSenderBinds.begin();iterF!= FecSenderBinds.end();iterF++)
               {
                   aTunnel = (lsp_tunnel_t)(*iterF);
                   if(aTunnel.Session.DestAddress == dest &&
                       aTunnel.Sender_Template.SrcAddress==src)
                   {
                       break;

                   }
               }
               if(iterF == FecSenderBinds.end())
               {
                   error("Cannot find the session to teardown");
               }
               else
               {
                   pt->setSenderTemplate(&aTunnel.Sender_Template);
                   pt->setSession(&aTunnel.Session);
                   RsvpHopObj_t* rsvp_hop = new RsvpHopObj_t;
                   rsvp_hop->Logical_Interface_Handle =-1;
                   rsvp_hop->Next_Hop_Address = local_addr;
                   pt->setHop(rsvp_hop);
                   send(pt, "to_rsvp");

               }

              continue;
          }

          //PATH REQUEST HANDLING

          int index =msg->par("gateIndex");

          //check if any similar previous requests

          /*
          Check whether the request had been recorded.
          Initiliaze new TE-Tunnel Id, and lsp id with new values if not found
          */

          for(iterF=FecSenderBinds.begin();iterF!= FecSenderBinds.end();iterF++)
          {
              aTunnel = (lsp_tunnel_t)(*iterF);
              if(aTunnel.Session.DestAddress == dest)
              {
                tunnelId = aTunnel.Session.Tunnel_Id;
                ex_tunnelId = aTunnel.Session.Extended_Tunnel_Id;
                foundSession =true;

                if(aTunnel.Sender_Template.SrcAddress == src)
                {
                    newTunnel = &aTunnel;
                  break;
                }
              }
              if(!foundSession)
              {
                  if(tunnelId < aTunnel.Session.Tunnel_Id)
                    tunnelId = aTunnel.Session.Tunnel_Id +1;

                  if(ex_tunnelId < aTunnel.Session.Extended_Tunnel_Id)
                    ex_tunnelId =aTunnel.Session.Extended_Tunnel_Id+1;
              }


          }


          if(iterF==FecSenderBinds.end())//There is no previous same requests
          {

              newTunnel = new lsp_tunnel_t;
              newTunnel->operating =false;
              newTunnel->Sender_Template.Lsp_Id = lspId;
              newTunnel->Sender_Template.SrcAddress = src;
              newTunnel->Sender_Template.SrcPort =DEFAULT_SRC_PORT;
              newTunnel->Session.DestAddress =dest;
              newTunnel->Session.DestPort =DEFAULT_DEST_PORT;
              newTunnel->Session.Protocol_Id =1;
              newTunnel->Session.holdingPri =7; //Lowest
              newTunnel->Session.setupPri =7;

              newTunnel->Session.Extended_Tunnel_Id =ex_tunnelId;
              newTunnel->Session.Tunnel_Id =tunnelId;
              newTunnel->inInfIndex = index;

              //Genarate new PATH message and send downstream
              std::vector<traffic_request_t>::iterator iterT;
              traffic_request_t trafficR;

              for(iterT = tr.begin(); iterT != tr.end(); iterT++)
              {
                  trafficR = (traffic_request_t)*iterT;
                  if(trafficR.dest == dest && trafficR.src == src)
                  {
                      newTunnel->Session.holdingPri = trafficR.holdingPri;
                      newTunnel->Session.setupPri  = trafficR.setupPri;
                      break;
                  }

              }

              FecSenderBinds.push_back(*newTunnel);

              if(iterT != tr.end())
                sendPathMessage(&newTunnel->Session, &trafficR, lspId);
              else
              {
                 traffic_request_t* aTR = new traffic_request_t;
                 aTR->bandwidth =0;
                 aTR->delay =0;
                 aTR->dest =dest;
                 aTR->holdingPri =7;
                 aTR->setupPri =7;
                 aTR->src =src;
                 aTR->isER = false;

                 sendPathMessage(&newTunnel->Session, aTR, lspId);
              }


          }
          delete msg;


      }//End if
/************************************************************************************
                Message for testing purpose
************************************************************************************/
      else if (!strcmp(msg->arrivalGate()->name(), "from_tester"))
      {
          ev << "Receive msg from Tester\n";

          if(!(msg->hasPar("test_command")))
          {
              ev << "Unrecognized test command\n";
              delete msg;
              continue;
          }

          //Process the test command
          int command = msg->par("test_command").longValue();
          if(command == NEW_BW_REQUEST)
          {
              int rSrc = IPAddress(msg->par("src").stringValue()).getInt();
              int rDest = IPAddress(msg->par("dest").stringValue()).getInt();
              double bw = msg->par("bandwidth").doubleValue();

              int k=0;
              int n=0;
              int newLsp_id =-1;
              for(k=0;k<tr.size();k++)
              {
                  if(tr[k].src == rSrc && tr[k].dest == rDest)
                  {
                      tr[k].bandwidth = bw;

            ev << "New Bandwidth request = "  << bw << " \n";
                      break;

                  }

              }
              if(k == tr.size()   )
                  continue;

              for(n =0 ;n<FecSenderBinds.size();n++)
              {
                   if(tr[k].dest == FecSenderBinds[n].Session.DestAddress &&
                      tr[k].src ==  FecSenderBinds[n].Sender_Template.SrcAddress)
                   {
                       newLsp_id = FecSenderBinds[n].Sender_Template.Lsp_Id;
                       break;
                   }
              }

              if(n == FecSenderBinds.size())
                  continue;

             newLsp_id= 2*MAX_LSP_NO - newLsp_id;

              sendPathMessage(&FecSenderBinds[n].Session, &tr[k], newLsp_id);



          }
          if(command ==NEW_ROUTE_DISCOVER)
          {

              std::vector<routing_info_t>::iterator iterR;
              routing_info_t rInfo;

              for(iterR = routingInfo.begin(); iterR != routingInfo.end(); iterR++)
              {
                      rInfo = (routing_info_t)*iterR;
                      //Try to find new route
                      ev << "CSPF calculates to find new path for LSP with id ="<< rInfo.lspId << "\n";
                      if(hasPath(rInfo.lspId, NULL))
                      {
                          ev << "Going to install new path\n";

                          break;
                      }

              }



                std::vector<lsp_tunnel_t>::iterator iterF;
              lsp_tunnel_t aTunnel;
              int newLspId = 2*MAX_LSP_NO - rInfo.lspId;



              for(iterF=FecSenderBinds.begin();iterF!= FecSenderBinds.end();iterF++)
              {
                  aTunnel = (lsp_tunnel_t)(*iterF);
                  if(aTunnel.Sender_Template.Lsp_Id ==rInfo.lspId)
                      break;
              }
              if(iterF == FecSenderBinds.end())
                  error("cannot locate the tunnel");

              std::vector<traffic_request_t>::iterator iterT;
              traffic_request_t trafficR;

              for(iterT = tr.begin(); iterT != tr.end(); iterT++)
              {
                  trafficR = (traffic_request_t)*iterT;
                  if(trafficR.dest == aTunnel.Session.DestAddress &&
                      trafficR.src == aTunnel.Sender_Template.SrcAddress)
                  {
                    break;
                  }

              }

              if(iterT == tr.end())
              {
                  ev << "No traffic spec required for the LSP\n";
                  continue;
              }

              //Create new Path message and send out
              ev << "Sending Path message for the new discovered route\n";
              sendPathMessage(&aTunnel.Session, &trafficR, newLspId);



          } //End NEW ROUTE command




      }


    }//End while


}

void RSVPAppl::sendResvMessage(PathMessage* pMsg, int inLabel)
{
        ev << "Generate a Resv Message\n";
        ResvMessage* rMsg = new ResvMessage();
        RsvpHopObj_t* rsvp_hop = new RsvpHopObj_t;
        FlowSpecObj_t* Flowspec_Object = new FlowSpecObj_t;
        FilterSpecObj_t* Filter_Spec_Object = new FilterSpecObj_t;
        FlowSpecObj_t* Flowspec_Object_Default = new FlowSpecObj_t;
        FilterSpecObj_t* Filter_Spec_Object_Default = new FilterSpecObj_t;

       //Setup  rsvp hop
        rsvp_hop->Logical_Interface_Handle = -1;//pMsg->getLIH();
        rsvp_hop->Next_Hop_Address = pMsg->getNHOP();


        /*
        Note this assume that the request bw = sender bw
        Need to make it more generic later
        */
        //Setup flowspec
        Flowspec_Object->link_delay = pMsg->getDelay();
        Flowspec_Object->req_bandwidth = pMsg->getBW();

        Flowspec_Object_Default->link_delay =pMsg->getDelay();
        Flowspec_Object_Default->req_bandwidth =pMsg->getBW();

        //Setup Filterspec
        Filter_Spec_Object->SrcAddress = pMsg->getSrcAddress();
        Filter_Spec_Object->SrcPort    = pMsg->getSrcPort();
        Filter_Spec_Object->Lsp_Id     = pMsg->getLspId();

        Filter_Spec_Object_Default->SrcAddress =0;
        Filter_Spec_Object_Default->SrcPort=0;
        Filter_Spec_Object_Default->Lsp_Id =-1;


        FlowDescriptor_t* flow_descriptor_list = new FlowDescriptor_t[InLIST_SIZE];
        flow_descriptor_list[0].Filter_Spec_Object = (*Filter_Spec_Object);
        flow_descriptor_list[0].Flowspec_Object =(*Flowspec_Object);

        //flow_descriptor_list[0].RRO[0] = local_addr;
        for(int c=0;c<MAX_ROUTE;c++)
            flow_descriptor_list[0].RRO[c] =0;

        flow_descriptor_list[0].label =inLabel;

        for(int i=1;i<InLIST_SIZE;i++)
        {
        flow_descriptor_list[i].Filter_Spec_Object = (*Filter_Spec_Object_Default);
        flow_descriptor_list[i].Flowspec_Object =(*Flowspec_Object_Default);
        flow_descriptor_list[i].label =-1;
        }

        //Check if this one is equivalent to reroute request
        if((pMsg->getLspId()) > MAX_LSP_NO)
        {
            int old_lsp_id = 2*MAX_LSP_NO -(pMsg->getLspId());
            Filter_Spec_Object->Lsp_Id = old_lsp_id;
            flow_descriptor_list[1].Filter_Spec_Object = (*Filter_Spec_Object);
            flow_descriptor_list[1].Flowspec_Object =(*Flowspec_Object);
            for(int c=0;c<MAX_ROUTE;c++)
            flow_descriptor_list[1].RRO[c] =0;



        }




        rMsg->setFlowDescriptor(flow_descriptor_list);
        rMsg->setHop(rsvp_hop);
        rMsg->setSession(pMsg->getSession());

        rMsg->setStyle(SF_STYLE);




        //int peerIp=0;
        //getPeerIPAddress(inInf, &peerIp);

        rMsg->addPar("dest_addr")= IPAddress(pMsg->getNHOP()).getString();
        ev << "Next peer " <<  IPAddress(pMsg->getNHOP()).getString();
        ev << "RESV MESSAGE content: \n";
        rMsg->print();
        send(rMsg, "to_rsvp");



}



void RSVPAppl::sendPathMessage(SessionObj_t* s, traffic_request_t* t, int lspId)
{
        OspfTe *ospfte = ospfteAccess.get();

        PathMessage* pMsg = new PathMessage();
        RsvpHopObj_t* rsvp_hop = new RsvpHopObj_t;
        FlowSpecObj_t* Flowspec_Object = new FlowSpecObj_t;

        FilterSpecObj_t* Filter_Spec_Object= new FilterSpecObj_t;

        //Setup PHOP
        rsvp_hop->Logical_Interface_Handle =-1;
        rsvp_hop->Next_Hop_Address = local_addr;
        int destAddr = s->DestAddress;
        int outInterface;

        Unicast_Route_Query( destAddr, &outInterface );

        rsvp_hop->Logical_Interface_Handle = outInterface;

        //Setup Sender Descriptor
        Flowspec_Object->link_delay = t->delay;
        Flowspec_Object->req_bandwidth = t->bandwidth;

        Filter_Spec_Object->SrcAddress = t->src;
        Filter_Spec_Object->SrcPort = DEFAULT_SRC_PORT;
        Filter_Spec_Object->Lsp_Id = lspId;

        pMsg->setHop(rsvp_hop);
        pMsg->setSession(s);
        pMsg->setSenderTemplate(static_cast<SenderTemplateObj_t*>(Filter_Spec_Object));
        pMsg->setSenderTspec(static_cast<SenderTspecObj_t*>(Flowspec_Object));

        //Setup routing information
        pMsg->addPar("src_addr") = IPAddress(local_addr).getString();

        int peerIP =0;
        int peerInf=0;
        //Normal way to get the peerIP
        getPeerIPAddress(destAddr, &peerIP, &peerInf);


        pMsg->addPar("dest_addr") = IPAddress(peerIP).getString();
        pMsg->addPar("peerInf") = peerInf;


        /**************************************************************
        More control options of ERO here
        ***************************************************************/


        //Calculate ERO

        EroObj_t ERO[MAX_ROUTE];
        for(int m=0; m< MAX_ROUTE;m++)
        {
        ERO[m].node =0;
        ERO[m].L =false;
        }
        //If the option of ER is on
        if(t->isER)
        {
            if(isIR)
            {
                if(t->route[0].node ==0)
                {
                    //No input ER from the administrator for this path
                    //Find ER based on CSPF routing algorithm
                    ev<< "OSPF PATH calculation: from " << IPAddress(local_addr).getString() <<
                    " to " << IPAddress(destAddr).getString() << "\n";

                    double delayTime=0;

                    std::vector<int> ero =ospfte->CalculateERO(new
                    IPAddress(destAddr),Flowspec_Object, &delayTime);
                    if(ero.back() == 0)    //Unknow reason
                    {
                    ero.pop_back();
                    ero.push_back(local_addr);
                    }



                    //std::vector<int>::iterator ero_iterI;
                    int hopCount =0;

                        for (int n=0; n< ero.size(); n++)
                        {
                            ev << IPAddress(ero[n]).getString() << "\n";
                            ERO[hopCount].node  = ero[n];
                            ERO[hopCount].L =false;
                            hopCount=hopCount+1;
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
                        if(hopCount == 0)
                        {
                            ev << "No resource available\n";
                            //delete pmsg;
                            return;
                        }

                    //pMsg->setERO(ERO);
                }
                else //Use the input from admin
                {
                    int lastIndex =0;
                    int index =0;
                    for(lastIndex =0; lastIndex < MAX_ROUTE; lastIndex++)
                        if(t->route[lastIndex].node ==0)
                            break;
                    lastIndex--;
                    for(; lastIndex >=0; lastIndex--)
                    {
                        ERO[index].L = t->route[lastIndex].L;
                        ERO[index].node = t->route[lastIndex].node;
                        index++;

                    }
                }
            }
            pMsg->addERO(true);
            pMsg->setERO(ERO);
        }
        else //Hop-by-Hop routing
        {
            pMsg->addERO(false);

        }

        /*
        routing_info_t* rInfo = new routing_info_t;
        rInfo->lspId = lspId;
        for(int c=0;c<MAX_ROUTE;c++)
            (rInfo->route)[c]= ERO[c].node;
        routingInfo.push_back(*rInfo);
        */



        //delete [] ERO;


        //Finish buidling packet, send it off
        ev << "Final Dest Address is : " << IPAddress(destAddr).getString() << "\n";
        ev << "Set Dest Address to: " << IPAddress(peerIP).getString() << "\n";


        ev << "PATH message content sent:\n";
        pMsg->print();

        send(pMsg, "to_rsvp");




}



void
RSVPAppl::Unicast_Route_Query(int da, int* outl)
{
    RoutingTable *rt = routingTableAccess.get();
    int foundIndex;
    //int j=0;
    foundIndex = rt->outputPortNo(IPAddress(da));
    (*outl) = rt->getInterfaceByIndex(foundIndex)->inetAddr.getInt(); //FIXME why not return outl???

    return;

}

void
RSVPAppl::Mcast_Route_Query(int sa, int iad, int da, int *outl)
{
    RoutingTable *rt = routingTableAccess.get();

    int foundIndex;
    //int j=0;
    foundIndex = rt->outputPortNo(IPAddress(da));
    (*outl) = rt->getInterfaceByIndex(foundIndex)->inetAddr.getInt(); //FIXME why not return outl???

    return;
}

void RSVPAppl::getPeerInet(int peerIP, int* peerInf)
{
    std::vector<telinkstate>::iterator ted_iterI;
    telinkstate ted_iter;
    updateTED();
    for (ted_iterI=ted.begin(); ted_iterI != ted.end(); ted_iterI++)
    {
        ted_iter = (telinkstate)*ted_iterI;
        if (ted_iter.linkid.getInt()==peerIP &&
            ted_iter.advrouter.getInt()==local_addr)
        {
            (*peerInf) = ted_iter.remote.getInt();
            break;
        }
    }
}

void RSVPAppl::getIncInet(int remoteInet, int* incInet)
{
    std::vector<telinkstate>::iterator ted_iterI;
    telinkstate ted_iter;

    ev << "Find incoming inf for peerOutgoinglink=" << IPAddress(remoteInet).getString() << "\n";
    updateTED();
    for (ted_iterI=ted.begin(); ted_iterI != ted.end(); ted_iterI++)
    {
        ted_iter = (telinkstate)*ted_iterI;
        if (ted_iter.remote.getInt() == remoteInet &&
            ted_iter.advrouter.getInt()==local_addr)
        {
            (*incInet) = ted_iter.local.getInt();
            break;
        }
    }
}

void RSVPAppl::getPeerIPAddress(int dest, int* peerIP, int* peerInf)
{
    int outl=0;
    Mcast_Route_Query(0, 0, dest, &outl);
    std::vector<telinkstate>::iterator ted_iterI;
    telinkstate ted_iter;

    updateTED();
    for (ted_iterI=ted.begin(); ted_iterI != ted.end(); ted_iterI++)
    {
        ted_iter = (telinkstate)*ted_iterI;
        if(ted_iter.local.getInt() == outl &&
                          ted_iter.advrouter.getInt() ==local_addr)
        {
            *peerIP = ted_iter.linkid.getInt();
            *peerInf = ted_iter.remote.getInt();
            break;
        }
    }
}

void RSVPAppl::getPeerIPAddress(int peerInf, int* peerIP)
{
  updateTED();
  std::vector<telinkstate>::iterator ted_iterI;
    telinkstate ted_iter;

    for (ted_iterI=ted.begin(); ted_iterI != ted.end(); ted_iterI++)
    {
        ted_iter = (telinkstate)*ted_iterI;
        if(ted_iter.local.getInt() == peerInf &&
                          ted_iter.advrouter.getInt() ==local_addr)
        {
            *peerIP = ted_iter.linkid.getInt();
            break;
        }
    }
}

void RSVPAppl::updateTED()
{
    // copy the full table
    ted = TED::getGlobalInstance()->getTED();
}


bool RSVPAppl::initFromFile(const char *filename)
{
    xmlDocPtr doc;

    xmlNodePtr cur;

    // Build an XML tree from a the file
    doc = xmlParseFile(filename);
    if (doc == NULL)
        return false;

    // Check the document is of the right kind
    cur = xmlDocGetRootElement(doc);
    if (cur == NULL) {
        ev <<"Empty document\n";
        xmlFreeDoc(doc);
        return false;
    }
    if (xmlStrcmp(cur->name, (const xmlChar *) "traffic")) {
        ev <<"Document of the wrong type, root node != traffic\n";
        xmlFreeDoc(doc);
        return false;
    }


    // Walk the tree.
    cur = cur->xmlChildrenNode;
    while (cur && xmlIsBlankNode(cur)) {
        cur = cur->next;
    }
    if (cur == NULL) {
        ev <<"No records in the document\n";
        xmlFreeDoc(doc);
        return false;
    }

    //cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if (!xmlStrcmp(cur->name, (const xmlChar *) "conn")) {
            TrafficRequest(doc, cur);

        }
        cur = cur->next;
    }

    return true;
}

void RSVPAppl::TrafficRequest (xmlDocPtr doc, xmlNodePtr cur)
{
    traffic_request_t* aTR = new traffic_request_t;
    for(int c=0;c< MAX_ROUTE;c++)
    {
        aTR->route[c].node =0;
        aTR->route[c].L = false;
    }
    aTR->isER = true;


    int rCount =0;

    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if (!xmlStrcmp(cur->name, (const xmlChar *)"src")) {

            aTR->src = IPAddress((const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)).getInt();
        }
        else if  (!xmlStrcmp(cur->name, (const xmlChar *)"dest")) {
            aTR->dest= IPAddress(
                (const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1)).getInt();
        }
        else if  (!xmlStrcmp(cur->name, (const xmlChar *)"setupPri")) {
            aTR->setupPri= strtol(
                (const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL, 0);
        }
        else if  (!xmlStrcmp(cur->name, (const xmlChar *)"holdingPri")) {
            aTR->holdingPri= strtol(
                (const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL, 0);
        }
        else if  (!xmlStrcmp(cur->name, (const xmlChar *)"delay")) {
            aTR->delay = strtod(
                (const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL);
        }
        else if  (!xmlStrcmp(cur->name, (const xmlChar *)"bandwidth")) {
            aTR->bandwidth = strtod(
                (const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1),NULL);
        }
        else if  (!xmlStrcmp(cur->name, (const xmlChar *)"ER")) {
            const char* ER_Option=
                (const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                ev << "ER Option is " << ER_Option << "\n";
            if((!strcmp(ER_Option, "false")) || (!strcmp(ER_Option, "no")) )
            {
            ev << "A hop-by-hop request\n";
                aTR->isER = false;
            }

        }
        else if  (!xmlStrcmp(cur->name, (const xmlChar *)"route")) {

            //aTR->route[rCount].node= IPAddress(
            const char* line=(const char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            string s(line);
            StringTokenizer tokenizer(s, string(","));
            int node;
            int lBit;
            string* aField;

            while ((aField=(tokenizer.nextToken()))!=NULL)
            {
                  node = IPAddress(aField->c_str()).getInt();
                  if((aField = tokenizer.nextToken()) !=NULL)
                  lBit = atoi(aField->c_str());

                  aTR->route[rCount].node = node;
                  if(lBit ==1)
                  aTR->route[rCount].L = true;
                  ev << "Add node from admin, node=" << IPAddress(node).getString() << " lBit=" << lBit <<"\n";
                  rCount = rCount+1;
                  if(rCount > MAX_ROUTE)
                  break;
            }
        }
        cur = cur->next;
    }
    ev << "Adding (src, dest, delay, bw) = (" << IPAddress(aTR->src).getString() << "," <<
                             IPAddress(aTR->dest).getString() << "," <<
                             aTR->delay << "," <<
                             aTR->bandwidth << ")\n";
    if(aTR->holdingPri > aTR->setupPri)
    {
        error("Holding priority is greater than setup priority (setup priority must be greater than or equal)");
    }
    tr.push_back(*aTR);
}



void RSVPAppl::addRouteInfo(ResvMessage* rmsg)
{
          FlowDescriptor_t* flow_d = rmsg->getFlowDescriptor();
           for(int k=0;k<InLIST_SIZE;k++)
           {
               if((*(flow_d+k)).Filter_Spec_Object.SrcAddress !=0)
               {
                   routing_info_t* rInfo = new routing_info_t;
                   rInfo->lspId = (*(flow_d+k)).Filter_Spec_Object.Lsp_Id;

                   for(int c=0;c< MAX_ROUTE;c++)
                   {
                   rInfo->route[c] = (*(flow_d+k)).RRO[c];
                   }
                   routingInfo.push_back(*rInfo);


               }
           }


}

bool RSVPAppl::hasPath(int lspid, FlowSpecObj_t* newFlowspec)
{
        OspfTe *ospfte = ospfteAccess.get();

          std::vector<lsp_tunnel_t>::iterator iterF;
          lsp_tunnel_t aTunnel;

          for(iterF=FecSenderBinds.begin();iterF!= FecSenderBinds.end();iterF++)
          {
              aTunnel = (lsp_tunnel_t)(*iterF);
              if(aTunnel.Sender_Template.Lsp_Id ==lspid)
                  break;
          }
          if(iterF == FecSenderBinds.end())
          {
              ev << "No LSP with id =" << lspid << "\n";
              return false;
          }

          std::vector<traffic_request_t>::iterator iterT;
          traffic_request_t trafficR;

          for(iterT = tr.begin(); iterT != tr.end(); iterT++)
          {
              trafficR = (traffic_request_t)*iterT;
              if(trafficR.dest == aTunnel.Session.DestAddress &&
                  trafficR.src == aTunnel.Sender_Template.SrcAddress)
              {
                break;
              }

          }

          if(iterT == tr.end())
          {
              ev << "No traffic spec required for LSP with id =" << lspid << "\n";
              return false;
          }

          //Locate the current used route
          std::vector<routing_info_t>::iterator iterR;
          routing_info_t rInfo;

          for(iterR = routingInfo.begin(); iterR != routingInfo.end(); iterR++)
          {
              rInfo = (routing_info_t)*iterR;
              if(rInfo.lspId == lspid)
                  break;

          }
          if(iterR == routingInfo.end())
              ev << "Warning, no recorded path found\n";

          if(iterR == routingInfo.end())
          {
              ev << "No route currently used for LSP with id =" << lspid << "\n";
              return false;
          }

          //Get destination
          int destAddr = aTunnel.Session.DestAddress;

          //Get old flowspec
          FlowSpecObj_t* Flowspec_Object = new FlowSpecObj_t;
          Flowspec_Object->link_delay = trafficR.delay;
          Flowspec_Object->req_bandwidth = trafficR.bandwidth;

          //Get links used
          std::vector<simple_link_t> linksInUse;
          for(int c=0;c< (MAX_ROUTE-1);c++)
          {
              simple_link_t* aLink = new simple_link_t;
              if((rInfo.route[c] !=0) && (rInfo.route[c+1]!=0))
              {
                  aLink->advRouter= rInfo.route[c+1];
                  aLink->id =rInfo.route[c];
                  linksInUse.push_back(*aLink);

              }
          }


          double currentTotalDelay = getTotalDelay(&linksInUse);
          ev << "Current total path metric: " << currentTotalDelay << "\n";
              //Calculate ERO

          ev<< "OSPF PATH calculation: from " << IPAddress(local_addr).getString() <<
          " to " << IPAddress(destAddr).getString() << "\n";

          std::vector<int> ero;
          double totalDelay =0;
          if(newFlowspec ==NULL)
          {

              ero = ospfte->CalculateERO(new IPAddress(destAddr),&linksInUse,Flowspec_Object, Flowspec_Object, &totalDelay);

              if(currentTotalDelay < totalDelay)
              {
                  ev << "The metric for new route is " << totalDelay << "\n";
                  ev << "No better route detected for LSP with id=" << lspid << "\n";
                  return false;
              }
              else
                  ev << "New route discovered with metric=" << totalDelay <<
                  " < current metric=" <<currentTotalDelay<<"\n";
          }
          else
            ero = ospfte->CalculateERO(new IPAddress(destAddr),&linksInUse,Flowspec_Object, newFlowspec, &totalDelay);


          std::vector<int>::iterator ero_iterI;

          int inx = 0;

          //compare the old route an new route, to the ER only
          for (ero_iterI=ero.begin(); ero_iterI != ero.end(); ero_iterI++)
                {
                if((*ero_iterI) == rInfo.route[0])
                    break;
                }

          for (; ero_iterI != ero.end(); ero_iterI++)
                {

                    if(rInfo.route[inx]!=(*ero_iterI))
                        break;
                    inx =inx+1;

                }
            if(ero_iterI == ero.end())
                return false;
            else
            {
                ev << "Old route for LSP with id=" << lspid << " was:\n";
                for(int c=0;c<MAX_ROUTE;c++)
                    ev << IPAddress(rInfo.route[c]).getString() << " ";
                ev << "\n";
                ev << "New route for this LSP is :\n";
                for (ero_iterI=ero.begin(); ero_iterI != ero.end(); ero_iterI++)
                {
                    ev << IPAddress((*ero_iterI)).getString() << " ";

                }
                ev << "\n";

                return true;
            }





}

double RSVPAppl::getTotalDelay(std::vector<simple_link_t> *links)
{
    double totalDelay=0;

    std::vector<telinkstate>::iterator ted_iterI;
    std::vector<simple_link_t>::iterator iterS;
    telinkstate ted_iter;
    simple_link_t aLink;

    for(iterS = (*links).begin(); iterS != (*links).end(); iterS++)
    {
        aLink = (simple_link_t)(*iterS);

        for (ted_iterI=ted.begin(); ted_iterI != ted.end(); ted_iterI++)
        {
            ted_iter = (telinkstate)*ted_iterI;
            if((ted_iter.linkid.getInt() == aLink.id) &&
                (ted_iter.advrouter.getInt()==aLink.advRouter))
                {
                    totalDelay = totalDelay + ted_iter.metric;

                }


        }
    }
    return totalDelay;


}

