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
#include "LDPInterface.h"
#include "ConstType.h"
#include "LDPproc.h"
#include "IPAddressResolver.h"

Define_Module(LDPInterface);

void LDPInterface::initialize()
{
    local_addr = IPAddress(par("local_addr").stringValue()).getInt();
    //local_addr = IPAddressResolver().getAddressFrom(RoutingTableAccess().get()).getInt();

    local_port = ConstType::ldp_port;
    rem_port = ConstType::ldp_port;


    keepAliveTime = par("keepAliveTime");

    timeout = par("timeout");

    appl_timeout = 0;  // FIXME Unused par("appl_timeout").doubleValue();

    // rem_addr = IPAddress(par("server_addr").stringValue()).getInt();
}


void LDPInterface::activity()
{
    TcpConnId tcp_conn_id;
    WATCH(tcp_conn_id);

    cModule *mod;
    cMessage *msg;
    int modID;
    int i;

    procserver_type = findModuleType("TCPServerProc");
    if (!procserver_type)
        error("Cannot find module type TCPServerProc");

    procclient_type = findModuleType("TCPClientProc");
    if (!procclient_type)
        error("Cannot find module type TCPClientProc");


    // Act as a server to accept peer tcp conn requests

    passiveOpen(timeout, procserver_type);


    while (true)
    {
        cArray *msg_list;
        int pIP, conID;

        msg = receive();
        int myKind = msg->kind();
        ev << "LDP INTERFACE DEBUG: Message received kind is " << myKind << "\n";

        switch (myKind)
        {

            // PART I: Messages from LDP proc
        case LDP_CLIENT_CREATE:
            // LDPproc sends message with peerIP (int)tagged
            createClient(msg->par("peerIP").longValue());
            delete msg;
            break;

        case LABEL_REQUEST:
            // LDPproc forwards request with peerIP tagged
            modID = getModidByPeerIP(msg->par("peerIP").longValue());
            if (modID == id())  // Invalid mod id
            {
                ev << "LDP INTERFACE DEBUG: LABEL_REQUEST unhandled in LDPInterface\n";
                delete msg;
                break;
            }
            mod = simulation.module(modID);
            if (!mod)
            {

                ev << "LDP INTERFACE DEBUG: This connection is invalid, deleting msg.\n";
                delete msg;
                break;
            }
            else
            {
                sendDirect(msg, 0.0, mod, "from_ldp");
                ev << "LDP INTERFACE DEBUG: Dispatch LABEL_REQUEST to client/server components\n";
                break;
            }

        case LABEL_MAPPING:
            // LDPproc return reply with peerIP tagged
            modID = getModidByPeerIP(msg->par("peerIP").longValue());
            if (modID == (this->id()))
            {
                ev << "LDP INTERFACE DEBUG: LABEL_MAPPING unhandled in LDPInterface\n";
                delete msg;
                break;
            }
            mod = simulation.module(modID);
            if (!mod)
            {

                ev << "LDP DEBUG: This connection is invalid, deleting msg.\n";
                delete msg;
                break;
            }
            else
            {
                sendDirect(msg, 0.0, mod, "from_ldp");
                break;
            }


            // PART II : Messages from TCP layers
        case TCP_I_RCVD_SYN:

            modID = msg->par("tcp_conn_id").longValue();  // From client to server only

            mod = simulation.module(modID);

            if (!mod)
            {
                if (debug)
                    ev << "LDP DEBUG: This ProcServer exited, deleting msg.\n";
                delete msg;
            }
            else
            {
                pIP = msg->par("src_addr").longValue();

                // Update ldpSession for server entries
                for (i = 0; i < ldpSessions.size(); i++)
                {
                    if (ldpSessions[i].mod_id == (int) modID)
                    {

                        ldpSessions[i].peerAddr = pIP;

                        break;
                    }

                }

                sendDirect(msg, 0.0, mod, "from_tcp");
            }

            passiveOpen(timeout, procserver_type);

            break;

        case TCP_I_ESTAB:

            pIP = msg->par("src_addr").longValue();

            conID = msg->par("tcp_conn_id");

            // TCP established by peer, update client entries

            for (i = 0; i < ldpSessions.size(); i++)
            {
                if (ldpSessions[i].peerAddr == (int) pIP)
                {

                    ldpSessions[i].tcp_conn_id = conID;
                    ev << "LDP INTERFACE DEBUG: Update modid-tcpCon-peerIP " <<
                        ldpSessions[i].mod_id << "    " << ldpSessions[i].tcp_conn_id << "    " <<
                        IPAddress(ldpSessions[i].peerAddr) << "\n";
                    modID = ldpSessions[i].mod_id;
                    break;
                }

            }
            mod = simulation.module(modID);

            if (mod == NULL)
                error("no module found for TCP_I_ESTAB");

            sendDirect(msg, 0.0, mod, "from_tcp");
            break;


        case TCP_I_SEG_FWD:

            msg_list = (cArray *) (msg->parList().get("msg_list"));
            for (i = 0; i < msg_list->items(); i++)
            {
                if (msg_list->exist(i))
                {

                    cMessage *tcp_send_msg = (cMessage *) ((cMessage *) (msg_list->get(i)))->dup();
                    tcp_send_msg->setKind(TCP_I_SEG_FWD);

                    modID = getModidByPeerIP(tcp_send_msg->par("src_addr").longValue());
                    mod = simulation.module(modID);
                    if (mod == NULL)
                        error("no module found for TCP_I_SEG_FWD");
                    sendDirect(tcp_send_msg, 0.0, mod, "from_tcp");
                }
            }
            delete msg;

            break;

        default:

            if (msg->hasPar("src_addr"))
                modID = getModidByPeerIP(msg->par("src_addr").longValue());
            else
                modID = getModidByPeerIP(msg->par("peerIP").longValue());

            ev <<
                "LDP INTERFACE DEBUG: Unknown message kind. Redirecting msg to module with TCP connection ID = "
                << modID << "\n";

            mod = simulation.module(modID);
            if (modID == id() || (!mod))
            {
                if (debug)
                    ev <<
                        "LDP INTERFACE DEBUG:  no module found for an unknown message, deleting msg.\n";
                delete msg;
            }
            else
            {
                sendDirect(msg, 0.0, mod, "from_tcp");
            }

        }  // End switch

    }  // End while



}


int LDPInterface::getModidByPeerIP(int peerIP)
{
    int i;
    for (i = 0; i < ldpSessions.size(); i++)
    {
        if (peerIP == ldpSessions[i].peerAddr)
            return ldpSessions[i].mod_id;
    }
    ev << "LDP PROC DEBUG: Unknown Peer IP: " << IPAddress(peerIP) << "\n";
    return id();  // FIXME is this good??? Andras
}

void LDPInterface::createClient(int destAddr)
{
    cModule *mod;

    mod = procclient_type->createScheduleInit("tcpclientproc", this);

    ev << "LDP INTERFACE DEBUG: Created TCP Client process, modID = " << mod->id() << "\n";

    // We want to classify imcomming messages only
    mod->gate("to_tcp")->setTo(gate("to_tcp"));

    mod->gate("to_ldp")->setTo(gate("to_ldp"));

    // The only way to tell this module about how it should communicate to ?

    cMessage *howMsg = new cMessage();
    howMsg->addPar("rem_addr") = destAddr;
    howMsg->addPar("local_addr") = local_addr;
    howMsg->addPar("timeout") = timeout;
    howMsg->addPar("appl_timeout") = appl_timeout;
    howMsg->addPar("keep_alive_time") = keepAliveTime;
    howMsg->setKind(ConstType::HOW_KIND);

    sendDirect(howMsg, 0.0, mod, "from_tcp");

    // Record the mod
    ldp_session_type newSession;
    newSession.mod_id = mod->id();
    newSession.peerAddr = destAddr;
    newSession.tcp_conn_id = -1;  // IS THIS AN INVALID VALUE?

    ldpSessions.push_back(newSession);
}




void LDPInterface::passiveOpen(double timeout, cModuleType * procserver_type)
{
    // server appl. calls
    cMessage *open_passive;

    open_passive = new cMessage("TCP_C_OPEN_PASSIVE", TCP_C_OPEN_PASSIVE);
    open_passive->addPar("src_port") = local_port;
    open_passive->addPar("src_addr") = local_addr;
    open_passive->addPar("dest_port") = -1;  // rem_port;
    open_passive->addPar("dest_addr") = -1;  // rem_addr;

    open_passive->addPar("timeout") = 3600 * 5000;  // timeout; // global default is 5 min

    open_passive->setLength(0);
    // no data packets to receive
    open_passive->addPar("rec_pks") = 0;

    // make delay checking possible
    open_passive->setTimestamp();

    // create module "TCPServerProc" dynamically within module "ClientPart"
    cModule *mod;

    mod = procserver_type->createScheduleInit("tcpserverproc", this);

    ev << "LDP INTERFACE DEBUG : Created TCP server process, modID = " << mod->id() << "\n";

    mod->gate("to_tcp")->setTo(gate("to_tcp"));
    mod->gate("to_ldp")->setTo(gate("to_ldp"));

    // The only way to tell this module about how it should communicate to ?

    cMessage *howMsg = new cMessage();
    // howMsg->addPar("rem_addr") = destAddr;
    howMsg->addPar("local_addr") = local_addr;
    howMsg->addPar("timeout") = timeout;
    howMsg->addPar("appl_timeout") = appl_timeout;
    howMsg->addPar("keep_alive_time") = keepAliveTime;
    howMsg->setKind(ConstType::HOW_KIND);

    sendDirect(howMsg, 0.0, mod, "from_tcp");

    int modID = mod->id();

    open_passive->addPar("tcp_conn_id") = modID;;

    // send "passive open" to "TcpModule"
    send(open_passive, "to_tcp");

    // Record the mod
    ldp_session_type newSession;
    newSession.tcp_conn_id = modID;
    newSession.mod_id = modID;
    newSession.peerAddr = -1;
    ldpSessions.push_back(newSession);
}
