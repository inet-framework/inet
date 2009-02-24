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
#include "ConstType.h"
#include "IPAddress.h"


/**
 * This module are dynamically instantiated by LDPInterface.
 * FIXME should be renamed to LDPConnHandler or something like this!!!
 *
 * Attempts to provide a simplified TCP interface for the LDP protocol:
 * just waits for am incoming connection, then relays messages in both
 * directions.
 *
 * FIXME really needs a rewrite.
 */
class TCPServerProc:public cSimpleModule
{
    Module_Class_Members(TCPServerProc, cSimpleModule, 16384);
  public:

    double keepAliveTime, timeout, appl_timeout;
    int kaKind;
    long msgLng;
    cMessage *ka_msg;

    // vector<cMessage*> dataQueue;

    int tcp_mss, local_port, rem_port, local_addr, rem_addr;
    int sendNo;

    TcpConnId tcp_conn_id;

    // PSH and URG flag
    TcpFlag tcp_flag_psh;
    TcpFlag tcp_flag_urg;

    virtual void initialize();

    virtual void activity();

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

  private:
      bool debug;
};

Define_Module(TCPServerProc);

void TCPServerProc::initialize()
{
    rem_addr = -1;
    kaKind = 9;
    tcp_conn_id = 0;
    tcp_flag_psh = TCP_F_SET;
    tcp_flag_urg = TCP_F_NSET;
    local_port = ConstType::ldp_port;
    rem_port = ConstType::ldp_port;
}

void TCPServerProc::activity()
{

    // Wait for message about who to communicate to
    cMessage *whoMsg = receive();

    // Initialization of communication parameters
    if ((whoMsg->kind()) == ConstType::HOW_KIND)
    {
        local_addr = whoMsg->par("local_addr").longValue();
        keepAliveTime = whoMsg->par("keep_alive_time");
        timeout = whoMsg->par("timeout");
        appl_timeout = whoMsg->par("appl_timeout");
    }
    else
    {
        error("No message of peer address");
    }
    delete whoMsg;

    // Expect SYN message

    cMessage *msg = receive();  // no timeout here
    rem_port = msg->par("src_port");  // client TCP-port at remote client side
    rem_addr = msg->par("src_addr");  // client IP-address at remote client side
    local_port = msg->par("dest_port"); // own server TCP-port
    local_addr = msg->par("dest_addr"); // own server IP-address

    // msg_length = msg->par("num_bit_req");
    ev << "TCP_SERVER_PROC DEBUG:  LSR(" << IPAddress(local_addr) << ") received SYN from LSR(" <<
        IPAddress(rem_addr) << ")\n";;

    delete msg;

    // Expect ESTAB message

    msg = receive();
    rem_port = msg->par("src_port");  // client TCP-port at remote client side
    rem_addr = msg->par("src_addr");  // client IP-address at remote client side
    local_port = msg->par("dest_port"); // own server TCP-port
    local_addr = msg->par("dest_addr"); // own server IP-address
    tcp_mss = 8 * ((msg->par("mss")).longValue());  // bits
    msgLng = tcp_mss / 4;  // bits

    ev << "TCP_SERVER_PROC DEBUG:  LSR(" << IPAddress(local_addr) << ") received ESTAB from LSR(" <<
        IPAddress(rem_addr) << ")\n";;

    delete msg;

    tcp_conn_id = id();

    // Start to operate as peer
    // wait(5);

    // Start the self KEEP ALIVE cycle

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
        processMessage(msg);
        // wait(5);
    }

}

void TCPServerProc::issueTCP_RECEIVE()
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

    ev << "TCP_SERVER_PROC DEBUG: " << IPAddress(local_addr) << " send TCP_C_RECEIVE to " <<
        IPAddress(rem_addr) << "\n";
}

void TCPServerProc::sendKEEP_ALIVE()
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

    ev << "TCP_SERVER_PROC DEBUG: " << IPAddress(local_addr) << " sends KEEP ALIVE to " <<
        IPAddress(rem_addr) << "\n";

    send(send_call, "to_tcp");
}

void TCPServerProc::sendData(cMessage * payload)
{
    cMessage *send_call = new cMessage("TCP_C_SEND", TCP_C_SEND);
    // cMessage *payload_msg = new cMessage();
    // payload_msg->addPar("data") = "Payload data";
    // payload_msg->setLength(100); // number of Bits

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

    ev << "TCP_SERVER_PROC DEBUG: " << IPAddress(local_addr) << " sends data to " <<
        IPAddress(rem_addr) << "\n";

    send(send_call, "to_tcp");
}

void TCPServerProc::processSELF_KEEP_ALIVE(cMessage * msg)
{
    if (sendNo == 0)
    {
        sendKEEP_ALIVE();
        // sendNo=1;
    }

    scheduleAt(simTime() + keepAliveTime, msg);

    cMessage *msgN = receive();

    // Iterate of processMessage till we get message from peer
    processMessage(msgN);
}

void TCPServerProc::processSIGNAL_DATA(cMessage * msg)
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

void TCPServerProc::processMessage(cMessage * msg)
{

    // LDPPacket* ldpPacket = dynamic_cast<LDPPacket*>msg;
    int msgKind = msg->kind();

    if (msg->isSelfMessage())
    {
        // Send KA to peer;

        processSELF_KEEP_ALIVE(msg);

        // scheduleAt(simTime()+keepAliveTime, msg);

    }
    else if ((msgKind > 9) && (msgKind < 20))
    {
        ev << "TCP_SERVER_PROC DEBUG: Receive packet from LDP layer\n";

        this->processSIGNAL_DATA(msg);
    }

    else  // Message is data from peer
    {
        processData(msg);
    }
}

void TCPServerProc::processData(cMessage * msg)
{
    if ((msg->kind()) == TCP_I_SEG_FWD)
    {
        if (msg->hasPar("APPL_PAYLOAD"))
        {
            cMessage *ldpPacket = (cMessage *) (msg->par("APPL_PAYLOAD").objectValue());

            if (ldpPacket != NULL)
            {
                ldpPacket->addPar("src_lsr_addr") = msg->par("src_addr");

                /*
                   // FIXME following redundant dup() concealed "not owner" error
                   cMessage* dupMsg = (cMessage*)ldpPacket->dup();
                   ev << "TCP_SERVER_PROC DEBUG: Receive data, forward to LDP\n";
                   send(dupMsg, "to_ldp");
                   delete ldpPacket;
                 */
                send(ldpPacket, "to_ldp");
            }
            else
            {
                ev << "TCP_SERVER_PROC DEBUG: Receive non ldp packet\n";
                delete msg;
            }
        }

        else
        {
            ev << "TCP_SERVER_PROC DEBUG: " << IPAddress(local_addr) <<
                " receives KEEP ALIVE message from " << IPAddress(rem_addr) << "\n";
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
