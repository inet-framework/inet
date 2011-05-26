//
// Copyright (C) 2008 Alfonso Ariza
// Copyright (C) 2010 Alfonso Ariza
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

#define CHEAT_IEEE80211MESH
#include "Ieee80211Mesh.h"
#include "MeshControlInfo_m.h"
#include "lwmpls_data.h"
#include "ControlInfoBreakLink_m.h"
#include "ControlManetRouting_m.h"
#include "OLSRpkt_m.h"
#include "dymo_msg_struct.h"
#include "aodv_msg_struct.h"
#include "InterfaceTableAccess.h"
#include "IPv4Datagram.h"
#include "IPv6Datagram.h"
#include "LinkStatePacket_m.h"
#include "MPLSPacket.h"
#include "ARPPacket_m.h"
#include "OSPFPacket_m.h"
#include <string.h>


/* WMPLS */


#if !defined (UINT32_MAX)
#   define UINT32_MAX  4294967295UL
#endif


Define_Module(Ieee80211Mesh);

Ieee80211Mesh::~Ieee80211Mesh()
{
//  gateWayDataMap.clear();
    if (mplsData)
        delete mplsData;
    if (WMPLSCHECKMAC)
        cancelAndDelete(WMPLSCHECKMAC);
}

Ieee80211Mesh::Ieee80211Mesh()
{
    // Mpls data
    mplsData = NULL;
    // subprocess
    ETXProcess=NULL;
    routingModuleProactive = NULL;
    routingModuleReactive = NULL;
    // packet timers
    WMPLSCHECKMAC =NULL;
    //
    macBaseGateId = -1;
}

void Ieee80211Mesh::initialize(int stage)
{
    EV << "Init mesh proccess \n";
    Ieee80211MgmtBase::initialize(stage);

    if (stage== 0)
    {
        limitDelay = par("maxDelay").doubleValue();
        useLwmpls = par("UseLwMpls");
        maxHopProactiveFeedback = par("maxHopProactiveFeedback");
        maxHopProactive = par("maxHopProactive");
        maxHopReactive = par("maxHopReactive");
        maxTTL=par("maxTTL");
    }
    else if (stage==1)
    {
        bool useReactive = par("useReactive");
        bool useProactive = par("useProactive");
        //if (useReactive)
        //    useProactive = false;

        if (useReactive && useProactive)
        {
            proactiveFeedback  = par("ProactiveFeedback");
        }
        else
            proactiveFeedback = false;
        mplsData = new LWMPLSDataStructure;
         //
        // cambio para evitar que puedan estar los dos protocolos simultaneamente
        // cuidado con esto
        //
        // Proactive protocol
        if (useReactive)
            startReactive();
        // Reactive protocol
        if (useProactive)
            startProactive();

        if (routingModuleProactive==NULL && routingModuleReactive ==NULL)
            error("Ieee80211Mesh doesn't have active routing protocol");

        mplsData->mplsMaxTime()=35;
        activeMacBreak=false;
        if (activeMacBreak)
            WMPLSCHECKMAC = new cMessage();

        ETXProcess = NULL;

        if (par("ETXEstimate"))
            startEtx();
    }
    if (stage==4)
    {
        macBaseGateId = gateSize("macOut")==0 ? -1 : gate("macOut",0)->getId(); // FIXME macBaseGateId is unused, what is it?
        EV << "macBaseGateId :" << macBaseGateId << "\n";
        ift = InterfaceTableAccess ().get();
        nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_LINK_BREAK);
        nb->subscribe(this,NF_LINK_REFRESH);
    }
}

void Ieee80211Mesh::startProactive()
{
    cModuleType *moduleType;
    cModule *module;
    //if (isEtx)
    //  moduleType = cModuleType::find("inet.networklayer.manetrouting.OLSR_ETX");
    //else
    moduleType = cModuleType::find("inet.networklayer.manetrouting.OLSR");
    module = moduleType->create("ManetRoutingProtocolProactive", this);
    routingModuleProactive = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleProactive->gate("to_ip")->connectTo(gate("routingInProactive"));
    gate("routingOutProactive")->connectTo(routingModuleProactive->gate("from_ip"));
    routingModuleProactive->buildInside();
    routingModuleProactive->scheduleStart(simTime());
}


void Ieee80211Mesh::startReactive()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find("inet.networklayer.manetrouting.DYMOUM");
    module = moduleType->create("ManetRoutingProtocolReactive", this);
    routingModuleReactive = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleReactive->gate("to_ip")->connectTo(gate("routingInReactive"));
    gate("routingOutReactive")->connectTo(routingModuleReactive->gate("from_ip"));
    routingModuleReactive->buildInside();
    routingModuleReactive->scheduleStart(simTime());
}

void Ieee80211Mesh::startEtx()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find("inet.linklayer.ieee80211.mgmt.Ieee80211Etx");
    module = moduleType->create("ETXproc", this);
    ETXProcess = dynamic_cast <Ieee80211Etx*> (module);
    ETXProcess->gate("toMac")->connectTo(gate("ETXProcIn"));
    gate("ETXProcOut")->connectTo(ETXProcess->gate("fromMac"));
    ETXProcess->buildInside();
    ETXProcess->scheduleStart(simTime());
    ETXProcess->setAddress(myAddress);
}

void Ieee80211Mesh::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage())
    {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
        return;
    }
    cGate * msggate = msg->getArrivalGate();
    char gateName [40];
    memset(gateName,0,40);
    strcpy(gateName,msggate->getBaseName());
    //if (msg->arrivedOn("macIn"))
    if (strstr(gateName,"macIn")!=NULL)
    {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(msg);
        Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(msg);
        if (frame2)
            frame2->setTTL(frame2->getTTL()-1);
        actualizeReactive(frame,false);
        processFrame(frame);
    }
    //else if (msg->arrivedOn("agentIn"))
    else if (strstr(gateName,"agentIn")!=NULL)
    {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cPolymorphic *ctrl = msg->removeControlInfo();
        delete msg;
        handleCommand(msgkind, ctrl);
    }
    //else if (msg->arrivedOn("routingIn"))
    else if (strstr(gateName,"routingIn")!=NULL)
    {
        handleRoutingMessage(PK(msg));
    }
    else if (strstr(gateName,"ETXProcIn")!=NULL)
    {
        handleEtxMessage(PK(msg));
    }
    else
    {
        cPacket *pk = PK(msg);
        // packet from upper layers, to be sent out
        EV << "Packet arrived from upper layers: " << pk << "\n";
        if (pk->getByteLength() > 2312)
            error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                  pk->getClassName(), pk->getName(), pk->getByteLength());
        handleUpperMessage(pk);
    }
}

void Ieee80211Mesh::handleTimer(cMessage *msg)
{
    //ASSERT(false);
    mplsData->lwmpls_interface_delete_old_path();
    if (WMPLSCHECKMAC==msg)
        mplsCheckRouteTime();
    else if (dynamic_cast<Ieee80211DataFrame*>(msg))
        sendOrEnqueue(PK(msg));
    else
        opp_error("message timer error");
}


void Ieee80211Mesh::handleRoutingMessage(cPacket *msg)
{
    cObject *temp  = msg->removeControlInfo();
    Ieee802Ctrl * ctrl = dynamic_cast<Ieee802Ctrl*> (temp);
    if (!ctrl)
    {
        char name[50];
        strcpy(name,msg->getName());
        error ("Message error");
    }
    Ieee80211DataFrame * frame = encapsulate(msg,ctrl->getDest());
    frame->setKind(ctrl->getInputPort());
    delete ctrl;
    if (frame)
        sendOrEnqueue(frame);
}

void Ieee80211Mesh::handleUpperMessage(cPacket *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    if (frame)
        sendOrEnqueue(frame);
}

void Ieee80211Mesh::handleCommand(int msgkind, cPolymorphic *ctrl)
{
    error("handleCommand(): no commands supported");
}

Ieee80211DataFrame *Ieee80211Mesh::encapsulate(cPacket *msg)
{
    Ieee80211MeshFrame *frame = new Ieee80211MeshFrame(msg->getName());
    frame->setTTL(maxTTL);
    frame->setTimestamp(msg->getCreationTime());
    LWMPLSPacket *lwmplspk = NULL;
    LWmpls_Forwarding_Structure *forwarding_ptr=NULL;

    // copy receiver address from the control info (sender address will be set in MAC)
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    MACAddress dest = ctrl->getDest();
    MACAddress next = ctrl->getDest();
    delete ctrl;
    frame->setFinalAddress(dest);

    frame->setAddress4(dest);
    frame->setAddress3(myAddress);


    if (dest.isBroadcast())
    {
        frame->setReceiverAddress(dest);
        frame->setTTL(1);
        uint32_t cont;

        mplsData->getBroadCastCounter(cont);

        lwmplspk = new LWMPLSPacket(msg->getName());
        cont++;
        mplsData->setBroadCastCounter(cont);
        lwmplspk->setCounter(cont);
        lwmplspk->setSource(myAddress);
        lwmplspk->setDest(dest);
        lwmplspk->setType(WMPLS_BROADCAST);
        lwmplspk->encapsulate(msg);
        frame->encapsulate(lwmplspk);
        return frame;
    }
    //
    // Search in the data base
    //
    int label = mplsData->getRegisterRoute(MacToUint64(dest));

    if (label!=-1)
    {
        forwarding_ptr = mplsData->lwmpls_forwarding_data(label,-1,0);
        if (!forwarding_ptr)
            mplsData->deleteRegisterRoute(MacToUint64(dest));

    }
    if (forwarding_ptr)
    {
        lwmplspk = new LWMPLSPacket(msg->getName());
        lwmplspk->setTTL(maxTTL);
        lwmplspk->setSource(myAddress);
        lwmplspk->setDest(dest);

        if (forwarding_ptr->order==LWMPLS_EXTRACT)
        {
// Source or destination?
            if (forwarding_ptr->output_label>0 || forwarding_ptr->return_label_output>0)
            {
                lwmplspk->setType(WMPLS_NORMAL);
                if (forwarding_ptr->return_label_input==label && forwarding_ptr->output_label>0)
                {
                    next = Uint64ToMac(forwarding_ptr->mac_address);
                    lwmplspk->setLabel(forwarding_ptr->output_label);
                }
                else if (forwarding_ptr->input_label==label && forwarding_ptr->return_label_output>0)
                {
                    next = Uint64ToMac(forwarding_ptr->input_mac_address);
                    lwmplspk->setLabel(forwarding_ptr->return_label_output);
                }
                else
                {
                    opp_error("lwmpls data base error");
                }
            }
            else
            {
                lwmplspk->setType(WMPLS_BEGIN_W_ROUTE);

                int dist = forwarding_ptr->path.size()-2;
                lwmplspk->setVectorAddressArraySize(dist);
                //lwmplspk->setDist(dist);
                next=Uint64ToMac(forwarding_ptr->path[1]);

                for (int i=0; i<dist; i++)
                    lwmplspk->setVectorAddress(i,Uint64ToMac(forwarding_ptr->path[i+1]));
                lwmplspk->setLabel (forwarding_ptr->return_label_input);
            }
        }
        else
        {
            lwmplspk->setType(WMPLS_NORMAL);
            if (forwarding_ptr->input_label==label && forwarding_ptr->output_label>0)
            {
                next = Uint64ToMac(forwarding_ptr->mac_address);
                lwmplspk->setLabel(forwarding_ptr->output_label);
            }
            else if (forwarding_ptr->return_label_input==label && forwarding_ptr->return_label_output>0)
            {
                next = Uint64ToMac(forwarding_ptr->input_mac_address);
                lwmplspk->setLabel(forwarding_ptr->return_label_output);
            }
            else
            {
                opp_error("lwmpls data base error");
            }
        }
        forwarding_ptr->last_use=simTime();
    }
    else
    {
        std::vector<Uint128> add;
        int dist = 0;
        bool noRoute;

        if (routingModuleProactive)
        {
            dist = routingModuleProactive->getRoute(dest,add);
            noRoute = false;
        }

        if (dist==0)
        {
            // Search in the reactive routing protocol
            // Destination unreachable
            if (routingModuleReactive)
            {
                add.resize(1);
                int iface;
                noRoute = true;
                double cost;
                if (!routingModuleReactive->getNextHop(dest,add[0],iface,cost)) //send the packet to the routingModuleReactive
                {
                    ControlManetRouting *ctrlmanet = new ControlManetRouting();
                    ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
                    ctrlmanet->setDestAddress(dest);
                    ctrlmanet->setSrcAddress(myAddress);
                    frame->encapsulate(msg);
                    ctrlmanet->encapsulate(frame);
                    send(ctrlmanet,"routingOutReactive");
                    return NULL;
                }
                else
                {
                    if (add[0].getMACAddress() == dest)
                        dist=1;
                    else
                        dist = 2;
                }
            }
            else
            {
                delete frame;
                delete msg;
                return NULL;
            }
        }
        next=add[0];
        if (dist >1 && useLwmpls)
        {
            lwmplspk = new LWMPLSPacket(msg->getName());
            lwmplspk->setTTL(maxTTL);
            if (!noRoute)
                lwmplspk->setType(WMPLS_BEGIN_W_ROUTE);
            else
                lwmplspk->setType(WMPLS_BEGIN);

            lwmplspk->setSource(myAddress);
            lwmplspk->setDest(dest);
            if (!noRoute)
            {
                next=add[0];
                lwmplspk->setVectorAddressArraySize(dist-1);
                //lwmplspk->setDist(dist-1);
                for (int i=0; i<dist-1; i++)
                    lwmplspk->setVectorAddress(i,add[i]);
                lwmplspk->setByteLength(lwmplspk->getByteLength()+((dist-1)*6));
            }

            int label_in =mplsData->getLWMPLSLabel();

            /* es necesario introducir el nuevo path en la lista de enlace */
            //lwmpls_initialize_interface(lwmpls_data_ptr,&interface_str_ptr,label_in,sta_addr, ip_address,LWMPLS_INPUT_LABEL);
            /* es necesario ahora introducir los datos en la tabla */
            forwarding_ptr = new LWmpls_Forwarding_Structure();
            forwarding_ptr->input_label=-1;
            forwarding_ptr->return_label_input=label_in;
            forwarding_ptr->return_label_output=-1;
            forwarding_ptr->order=LWMPLS_EXTRACT;
            forwarding_ptr->mac_address=MacToUint64(next);
            forwarding_ptr->label_life_limit=mplsData->mplsMaxTime();
            forwarding_ptr->last_use=simTime();

            forwarding_ptr->path.push_back(MacToUint64(myAddress));
            for (int i=0; i<dist-1; i++)
                forwarding_ptr->path.push_back(MacToUint64(add[i]));
            forwarding_ptr->path.push_back(MacToUint64(dest));

            mplsData->lwmpls_forwarding_input_data_add(label_in,forwarding_ptr);
            // lwmpls_forwarding_output_data_add(label_out,sta_addr,forwarding_ptr,true);
            /*lwmpls_label_fw_relations (lwmpls_data_ptr,label_in,forwarding_ptr);*/
            lwmplspk->setLabel (label_in);
            mplsData->registerRoute(MacToUint64(dest),label_in);
        }
    }

    frame->setReceiverAddress(next);
    if (lwmplspk)
    {
        lwmplspk->encapsulate(msg);
        frame->setTTL(lwmplspk->getTTL());
        frame->encapsulate(lwmplspk);
    }
    else
        frame->encapsulate(msg);

    if (frame->getReceiverAddress().isUnspecified())
        ASSERT(!frame->getReceiverAddress().isUnspecified());
    return frame;
}

Ieee80211DataFrame *Ieee80211Mesh::encapsulate(cPacket *msg,MACAddress dest)
{
    Ieee80211MeshFrame *frame = dynamic_cast<Ieee80211MeshFrame*>(msg);
    if (frame==NULL)
        frame = new Ieee80211MeshFrame(msg->getName());
    frame->setTimestamp(msg->getCreationTime());
    frame->setTTL(maxTTL);

    if (msg->getControlInfo())
        delete msg->removeControlInfo();
    LWMPLSPacket* msgAux = dynamic_cast<LWMPLSPacket*> (msg);
    if (msgAux)
    {
        frame->setTTL(msgAux->getTTL());
    }
    frame->setReceiverAddress(dest);
    if (msg!=frame)
        frame->encapsulate(msg);

    if (frame->getReceiverAddress().isUnspecified())
    {
        char name[50];
        strcpy(name,msg->getName());
        opp_error ("Ieee80211Mesh::encapsulate Bad Address");
    }
    if (frame->getReceiverAddress().isBroadcast())
        frame->setTTL(1);
    return frame;
}


void Ieee80211Mesh::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (details==NULL)
        return;

    if (category == NF_LINK_BREAK)
    {
        if (details==NULL)
            return;
        Ieee80211DataFrame *frame  = check_and_cast<Ieee80211DataFrame *>(details);
        MACAddress add = frame->getReceiverAddress();
        mplsBreakMacLink(add);
    }
    else if (category == NF_LINK_REFRESH)
    {
        Ieee80211TwoAddressFrame *frame  = check_and_cast<Ieee80211TwoAddressFrame *>(details);
        if (frame)
            mplsData->lwmpls_refresh_mac (MacToUint64(frame->getTransmitterAddress()),simTime());
    }
}

void Ieee80211Mesh::handleDataFrame(Ieee80211DataFrame *frame)
{
    // The message is forward
    if (forwardMessage (frame))
        return;

    MACAddress finalAddress;
    MACAddress source= frame->getTransmitterAddress();
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(frame);
    short ttl = maxTTL;
    if (frame2)
    {
        ttl = frame2->getTTL();
        finalAddress =frame2->getFinalAddress();
    }
    cPacket *msg = decapsulate(frame);
    ///
    /// If it's a ETX packet to send to the appropriate module
    ///
    if (dynamic_cast<ETXBasePacket*>(msg))
    {
        if (ETXProcess)
        {
            if (msg->getControlInfo())
                delete msg->removeControlInfo();
            send(msg,"ETXProcOut");
        }
        else
            delete msg;
        return;
    }

    LWMPLSPacket *lwmplspk = dynamic_cast<LWMPLSPacket*> (msg);
    mplsData->lwmpls_refresh_mac(MacToUint64(source),simTime());

    if (!lwmplspk)
    {
        //cGate * msggate = msg->getArrivalGate();
        //int baseId = gateBaseId("macIn");
        //int index = baseId - msggate->getId();
        msg->setKind(0);
        if ((routingModuleProactive != NULL) && (routingModuleProactive->isOurType(msg)))
        {
            //sendDirect(msg,0, routingModule, "from_ip");
            send(msg,"routingOutProactive");
        }
        // else if (dynamic_cast<AODV_msg  *>(msg) || dynamic_cast<DYMO_element  *>(msg))
        else if ((routingModuleReactive != NULL) && routingModuleReactive->isOurType(msg))
        {
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
            MeshControlInfo *controlInfo = new MeshControlInfo;
            Ieee802Ctrl *ctrlAux = controlInfo;
            *ctrlAux=*ctrl;
            delete ctrl;
            Uint128 dest;
            msg->setControlInfo(controlInfo);
            if (routingModuleReactive->getDestAddress(msg,dest))
            {
                std::vector<Uint128>add;
                Uint128 src = controlInfo->getSrc();
                int dist = 0;
                if (routingModuleProactive && proactiveFeedback)
                {
                    // int neig = routingModuleProactive))->getRoute(src,add);
                    controlInfo->setPreviousFix(true); // This node is fix
                    dist = routingModuleProactive->getRoute(dest,add);
                }
                else
                    controlInfo->setPreviousFix(false); // This node is not fix
                if (maxHopProactive>0 && dist>maxHopProactive)
                    dist = 0;
                if (dist!=0 && proactiveFeedback)
                {
                    controlInfo->setVectorAddressArraySize(dist);
                    for (int i=0; i<dist; i++)
                        controlInfo->setVectorAddress(i,add[i]);
                }
            }
            send(msg,"routingOutReactive");
        }
        else if (dynamic_cast<OLSR_pkt*>(msg) || dynamic_cast <DYMO_element *>(msg) || dynamic_cast <AODV_msg *>(msg))
        {
            delete msg;
        }
        else // Normal frame test if upper layer frame in other case delete
            sendUp(msg);
        return;
    }
    lwmplspk->setTTL(ttl);
    mplsDataProcess(lwmplspk,source);
}

void Ieee80211Mesh::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}

// Cada ver que se envia un mensaje sirve para generar mensajes de permanencia. usa los propios hellos para garantizar que se env�an mensajes

void Ieee80211Mesh::sendOut(cMessage *msg)
{
    //InterfaceEntry *ie = ift->getInterfaceById(msg->getKind());
    msg->setKind(0);
    //send(msg, macBaseGateId + ie->getNetworkLayerGateIndex());
    send(msg, "macOut",0);
}


//
// mac label address method
// Equivalent to the 802.11s forwarding mechanism
//

bool Ieee80211Mesh::forwardMessage (Ieee80211DataFrame *frame)
{
#if OMNETPP_VERSION > 0x0400
    cPacket *msg = frame->getEncapsulatedPacket();
#else
    cPacket *msg = frame->getEncapsulatedMsg();
#endif
    LWMPLSPacket *lwmplspk = dynamic_cast<LWMPLSPacket*> (msg);

    if (lwmplspk)
        return false;
    if ((routingModuleProactive != NULL) && (routingModuleProactive->isOurType(msg)))
        return false;
    else if ((routingModuleReactive != NULL) && routingModuleReactive->isOurType(msg))
        return false;
    else // Normal frame test if use the mac label address method
        return macLabelBasedSend(frame);

}

bool Ieee80211Mesh::macLabelBasedSend (Ieee80211DataFrame *frame)
{

    if (!frame)
        return false;

    if (frame->getAddress4()==myAddress || frame->getAddress4().isUnspecified())
        return false;

    uint64_t dest = MacToUint64(frame->getAddress4());
    uint64_t src = MacToUint64(frame->getAddress3());
    uint64_t prev = MacToUint64(frame->getTransmitterAddress());
    uint64_t next = mplsData->getForwardingMacKey(src,dest,prev);
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(frame);

    double  delay = SIMTIME_DBL(simTime() - frame->getTimestamp());
    if ((frame2 && frame2->getTTL()<=0) || (delay > limitDelay))
    {
        delete frame;
        return true;
    }

    if (next)
    {
        frame->setReceiverAddress(Uint64ToMac(next));
    }
    else
    {
        std::vector<Uint128> add;
        int dist=0;
        int iface;

        Uint128 gateWayAddress;
        if (routingModuleProactive)
        {
            add.resize(1);
             double cost;
             if (routingModuleProactive->getNextHop(dest,add[0],iface,cost))
                 dist = 1;
        }

        if (dist==0 && routingModuleReactive)
        {
            add.resize(1);
            double cost;
            if (routingModuleReactive->getNextHop(dest,add[0],iface,cost))
                dist = 1;
        }

        if (dist==0)
        {
// Destination unreachable
            if (routingModuleReactive)
            {
                ControlManetRouting *ctrlmanet = new ControlManetRouting();
                ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
                ctrlmanet->setDestAddress(dest);
                //  ctrlmanet->setSrcAddress(myAddress);
                ctrlmanet->setSrcAddress(src);
                ctrlmanet->encapsulate(frame);
                frame = NULL;
                send(ctrlmanet,"routingOutReactive");
            }
            else
            {
                delete frame;
                frame=NULL;
            }
        }
        else
        {
            frame->setReceiverAddress(add[0].getMACAddress());
        }

    }
    //send(msg, macBaseGateId + ie->getNetworkLayerGateIndex());
    if (frame)
        sendOrEnqueue(frame);
    return true;
}

void Ieee80211Mesh::sendUp(cMessage *msg)
{
    if (isUpperLayer(msg))
        send(msg, "uppergateOut");
    else
        delete msg;
}

bool Ieee80211Mesh::isUpperLayer(cMessage *msg)
{
    if (dynamic_cast<IPv4Datagram*>(msg))
        return true;
    else if (dynamic_cast<IPv6Datagram*>(msg))
        return true;
    else if (dynamic_cast<OSPFPacket*>(msg))
        return true;
    else if (dynamic_cast<MPLSPacket*>(msg))
        return true;
    else if (dynamic_cast<LinkStateMsg*>(msg))
        return true;
    else if (dynamic_cast<ARPPacket*>(msg))
        return true;
    return false;
}

cPacket *Ieee80211Mesh::decapsulate(Ieee80211DataFrame *frame)
{
    cPacket *payload = frame->decapsulate();
    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    ctrl->setSrc(frame->getTransmitterAddress());
    ctrl->setDest(frame->getReceiverAddress());
    payload->setControlInfo(ctrl);
    delete frame;
    return payload;
}

void Ieee80211Mesh::actualizeReactive(cPacket *pkt,bool out)
{
    Uint128 dest,prev,next,src;
    if (!routingModuleReactive)
        return;

    Ieee80211DataFrame * frame = dynamic_cast<Ieee80211DataFrame*>(pkt);

    if (!frame )
        return;
/*
    if (!out)
        return;
*/
    /*
        if (frame->getAddress4().isUnspecified() || frame->getAddress4().isBroadcast())
            return;

        ControlManetRouting *ctrlmanet = new ControlManetRouting();
        dest=frame->getAddress4();
        src=frame->getAddress3();
        ctrlmanet->setOptionCode(MANET_ROUTE_UPDATE);
        ctrlmanet->setDestAddress(dest);
        ctrlmanet->setSrcAddress(src);
        send(ctrlmanet,"routingOutReactive");
        return;
    */

    if (out)
    {
        if (!frame->getAddress4().isUnspecified() && !frame->getAddress4().isBroadcast())
            dest=frame->getAddress4();
        else
            return;
        if (!frame->getReceiverAddress().isUnspecified() && !frame->getReceiverAddress().isBroadcast())
            next=frame->getReceiverAddress();
        else
            return;

    }
    else
    {
        if (!frame->getAddress3().isUnspecified() && !frame->getAddress3().isBroadcast() )
            dest=frame->getAddress3();
        else
            return;
        if (!frame->getTransmitterAddress().isUnspecified() && !frame->getTransmitterAddress().isBroadcast())
            prev=frame->getTransmitterAddress();
        else
            return;

    }
    routingModuleReactive->setRefreshRoute(src,dest,next,prev);
}


void Ieee80211Mesh::sendOrEnqueue(cPacket *frame)
{
    Ieee80211MeshFrame * frameAux = dynamic_cast<Ieee80211MeshFrame*>(frame);
    if (frameAux && frameAux->getTTL()<=0)
    {
        delete frame;
        return;
    }
    // Check if the destination is other gateway if true send to it
    actualizeReactive(frame,true);
    PassiveQueueBase::handleMessage(frame);
}


void Ieee80211Mesh::handleEtxMessage(cPacket *pk)
{
    ETXBasePacket * etxMsg = dynamic_cast<ETXBasePacket*>(pk);
    if (etxMsg)
    {
        Ieee80211DataFrame * frame = encapsulate(etxMsg,etxMsg->getDest());
        if (frame)
            sendOrEnqueue(frame);
    }
    else
        delete pk;
}
