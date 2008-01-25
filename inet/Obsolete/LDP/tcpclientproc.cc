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
#include "tcp.h"
#include "IPAddress.h"
#include "ConstType.h"

/**
 * This module are dynamically instantiated by LDPInterface.
 * FIXME should be renamed to LDPConnHandler or something like this!!!
 *
 * Attempts to provide a simplified TCP interface for the LDP protocol:
 * just establishes the connection (FIXME no error handling!)
 * and relays messages in both directions. FIXME really needs a rewrite.
 */
class TCPClientProc:public cSimpleModule
{
    Module_Class_Members(TCPClientProc, cSimpleModule, 16384);

  private:

    double keepAliveTime, timeout, appl_timeout;
    int kaKind;
    long msgLng;

    // vector<cMessage*> dataQueue;

    int tcp_mss, local_port, rem_port, local_addr, rem_addr;

    // Number of messages have been sent in one cycle
    int sendNo;
    cMessage *ka_msg;

    TcpConnId tcp_conn_id;

    // PSH and URG flag
    TcpFlag tcp_flag_psh;
    TcpFlag tcp_flag_urg;

    // Send TCP_RECEIVE to tcp
    void issueTCP_RECEIVE();

    // Send KEEP ALIVE to peer
    void sendKEEP_ALIVE();

    // Send data to peer
    void sendData(cMessage * payload);

    // Process data from peer
    void processData(cMessage * msg);

    // Process KEEP ALIVE timer
    void processSELF_KEEP_ALIVE(cMessage * msg);

    // Process data from LDP
    void processSIGNAL_DATA(cMessage * msg);

    // Process messages
    void processMessage(cMessage * msg);

  public:
      virtual void initialize();
    virtual void activity();
};

Define_Module(TCPClientProc);


void TCPClientProc::initialize()
{
    kaKind = 9;

    tcp_conn_id = 0;
    tcp_flag_psh = TCP_F_SET;
    tcp_flag_urg = TCP_F_NSET;
    local_port = ConstType::ldp_port;
    rem_port = ConstType::ldp_port;
}

void TCPClientProc::activity()
{
    // Wait for message about who to communicate to
    cMessage *whoMsg = receive();

    // Initialization of communication parameters

    if ((whoMsg->kind()) == ConstType::HOW_KIND)
    {
        local_addr = whoMsg->par("local_addr").longValue();
        rem_addr = whoMsg->par("rem_addr").longValue();
        keepAliveTime = whoMsg->par("keep_alive_time");
        timeout = whoMsg->par("timeout");
        appl_timeout = whoMsg->par("appl_timeout");
    }
    else
    {
        error("No message of peer address");
    }
    delete whoMsg;

    WATCH(tcp_conn_id);

    local_port = id();

    // send "active open" (client) call to "TcpModule"
    cMessage *open_active = new cMessage("TCP_C_OPEN_ACTIVE", TCP_C_OPEN_ACTIVE);

    open_active->addPar("src_port") = local_port;
    open_active->addPar("src_addr") = local_addr;
    open_active->addPar("dest_port") = rem_port;
    open_active->addPar("dest_addr") = rem_addr;
    open_active->addPar("timeout") = timeout;
    open_active->setLength(0);
    open_active->addPar("rec_pks") = 0;
    open_active->setTimestamp();

    ev << "TCP_CLIENT_PROC DEBUG:  LDP/TCP ACTIVE_OPEN are sent from LSR(" <<
        IPAddress(local_addr) << ") to LSR(" << IPAddress(rem_addr) << ")\n";
    ev << "TCP_CLIENT_PROC DEBUG: Connection parameters are:\n";
    ev << "        src_port=" << id() << "\n";
    ev << "        rem_port=" << rem_port << "\n";
    ev << "        timeout =" << timeout << "\n";
    ev << "        keepAliveTime= " << keepAliveTime << "\n";

    send(open_active, "to_tcp");

    // Waiting for ESTAB message
    // we ignore timeout or other possible errors

    cMessage *e_msg = receive();

    tcp_conn_id = e_msg->par("tcp_conn_id");
    tcp_mss = 8 * ((e_msg->par("mss")).longValue());  // bits
    msgLng = tcp_mss / 4;

    ev << "TCP_CLIENT_PROC DEBUG: LSR(" << IPAddress(local_addr);
    ev << ") received TCP_ESTAB from LSR(" << IPAddress(rem_addr);
    ev << ") with tcp_con_id=" << tcp_conn_id << "\n";

    delete e_msg;

    // Start to operate as peer
    wait(5);

    // Start the self keep alive cycle

    ka_msg = new cMessage();

    ka_msg->setKind(kaKind);

    scheduleAt(simTime() + keepAliveTime, ka_msg);

    // Send KEEP ALIVE to peer
    sendKEEP_ALIVE();

    // following while(true) line was changed to while(this!=0) [which is equally true]
    // because the former crashed MSVC6.0sp5 (!!) with:
    //   fatal error C1001: INTERNAL COMPILER ERROR
    //     (compiler file 'E:\8966\vc98\p2\src\P2\main.c', line 494)
    // while(true)
    while (this != 0)
    {
        sendNo = 0;

        // Issue a TCP_RECEIVE, possible data from peer
        issueTCP_RECEIVE();

        cMessage *msg = receive();

        // Process one and only one message from peer
        processMessage(msg);
    }
}


void TCPClientProc::issueTCP_RECEIVE()
{
    cMessage *receive_call = new cMessage("TCP_C_RECEIVE", TCP_C_RECEIVE);

    receive_call->addPar("src_port") = local_port;
    receive_call->addPar("src_addr") = local_addr;
    receive_call->addPar("dest_port") = rem_port;
    receive_call->addPar("dest_addr") = rem_addr;
    receive_call->addPar("tcp_conn_id") = tcp_conn_id;
    receive_call->setLength(0);
    receive_call->addPar("rec_pks") = 1;  // (long) rec_pks;
    receive_call->setTimestamp();

    // send "receive" to "TcpModule"
    send(receive_call, "to_tcp");

    ev << "TCP_CLIENT_PROC DEBUG: " << IPAddress(local_addr) <<
        " send TCP_C_RECEIVE to " << IPAddress(rem_addr);
}


void TCPClientProc::sendKEEP_ALIVE()
{
    cMessage *send_call = new cMessage("TCP_C_SEND", TCP_C_SEND);
    send_call->addPar("tcp_conn_id") = tcp_conn_id;
    send_call->addPar("src_port") = local_port;
    send_call->addPar("src_addr") = local_addr;
    send_call->addPar("dest_port") = rem_port;
    send_call->addPar("dest_addr") = rem_addr;
    send_call->addPar("tcp_flag_psh") = (int) tcp_flag_psh;
    send_call->addPar("tcp_flag_urg") = (int) tcp_flag_urg;
    send_call->addPar("timeout") = (int) timeout;
    send_call->setLength(msgLng);  // (tcp_mss * 8/4);
    send_call->addPar("rec_pks") = 0;
    send_call->addPar("keep_alive") = 0;
    send_call->setTimestamp();


    ev << "TCP_CLIENT_PROC DEBUG: " << IPAddress(local_addr) << " send KEEP ALIVE to " <<
        IPAddress(rem_addr) << "\n";

    send(send_call, "to_tcp");
}

void TCPClientProc::sendData(cMessage * payload)
{
    cMessage *send_call = new cMessage("TCP_C_SEND", TCP_C_SEND);

    // Change to appropriate kind for LDP to understand
    send_call->addPar("APPL_PAYLOAD") = payload;
    send_call->addPar("tcp_conn_id") = tcp_conn_id;
    send_call->addPar("src_port") = local_port;
    send_call->addPar("src_addr") = local_addr;
    send_call->addPar("dest_port") = rem_port;
    send_call->addPar("dest_addr") = rem_addr;
    send_call->addPar("tcp_flag_psh") = (int) tcp_flag_psh;
    send_call->addPar("tcp_flag_urg") = (int) tcp_flag_urg;
    send_call->addPar("timeout") = (int) timeout;
    send_call->setLength(msgLng);  // (tcp_mss * 8);
    send_call->addPar("rec_pks") = 0;
    send_call->setTimestamp();

    ev << "TCP_CLIENT_PROC DEBUG: " << IPAddress(local_addr) << " send data to " <<
        IPAddress(rem_addr) << "\n";

    send(send_call, "to_tcp");
}

void TCPClientProc::processSELF_KEEP_ALIVE(cMessage * msg)
{
    if (sendNo == 0)
    {
        sendKEEP_ALIVE();
        // sendNo = 1;
    }
    scheduleAt(simTime() + keepAliveTime, msg);

    cMessage *msgN = receive();

    // Iterate of processMessage till we get message from peer
    processMessage(msgN);
}

void TCPClientProc::processSIGNAL_DATA(cMessage * msg)
{
    // We don't want to lost data
    // Order not important
    // dataQueue.push_back(msg);
    cancelEvent(ka_msg);

    sendData(msg);
    sendNo = 0;

    // Re-schedule KeepAlive message
    scheduleAt(simTime() + keepAliveTime, ka_msg);

    cMessage *msgN = receive();
    processMessage(msgN);  // Iterate till we get message from peer
}

void TCPClientProc::processMessage(cMessage * msg)
{

    // LDPPacket* ldpPacket = dynamic_cast <LDPPacket*> msg;
    /*
       LDP_CLIEN_CREATE=10,
       LDP_FORWARD_REQUEST,
       LDP_RETURN_REPLY,
       LDP_BROADCAST_REQUEST
     */

    int msgKind = msg->kind();
    if (msg->isSelfMessage())
    {
        // Send KA to peer;
        processSELF_KEEP_ALIVE(msg);
        // scheduleAt(simTime()+keepAliveTime, msg);
    }
    else if ((msgKind > 9) && (msgKind < 20))
    {
        ev << "TCP_CLIENT_PROC DEBUG: Receive packet from LDP layer\n";
        this->processSIGNAL_DATA(msg);
    }
    else  // Message is data from peer
    {
        processData(msg);
    }
}



void TCPClientProc::processData(cMessage * msg)
{
    if ((msg->kind()) == TCP_I_SEG_FWD)
    {
        if (msg->hasPar("APPL_PAYLOAD"))
        {
            cMessage *ldpPacket = (cMessage *) msg->par("APPL_PAYLOAD").objectValue();

            if (ldpPacket != NULL)
            {
                ldpPacket->addPar("src_lsr_addr") = (msg->par("src_addr"));

                ev << "TCP_CLIENT_PROC DEBUG: Receive data, forward to LDP\n";

                /*
                   // FIXME following redundant dup() concealed "not owner" error
                   cMessage *dupMsg = (cMessage *)ldpPacket->dup();
                   send(dupMsg, "to_ldp");
                   delete ldpPacket;
                 */
                send(ldpPacket, "to_ldp");
            }
            else
            {
                ev << "TCP_CLIENT_PROC DEBUG: Receive non ldp packet\n";
                delete msg;
            }
        }

        else
        {
            ev << "TCP_CLIENT_PROC DEBUG: " << IPAddress(local_addr) <<
                " receive KEEP ALIVE message from " << IPAddress(rem_addr) << "\n";

            delete msg;
        }
    }

    else
    {

        ev << " TCP_CLIENT_DEBUG: " << IPAddress(local_addr) <<
            " received something other than TCP_I_SEG_FWD from " << IPAddress(rem_addr) <<
            ", kind is " << msg->kind() << "\n";

        delete msg;
    }
}
