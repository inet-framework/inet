//
//-----------------------------------------------------------------------------
//-- fileName: TCPClient.cc
//--
//-- generated to test the TCP-FSM
//--
//-- V. Boehm, June 19 1999
//--
//-- modified by J. Reber for IP architecture, October 2000
//-----------------------------------------------------------------------------
//
// Copyright (C) 2000 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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

#include <omnetpp.h>
#include "tcp.h"

class TCPClient:public cSimpleModule
{
    Module_Class_Members(TCPClient, cSimpleModule, 16384);
    virtual void activity();
};

Define_Module_Like(TCPClient, TCPApp);

void TCPClient::activity()
{
    //module parameters
    bool debug = par("debug");
    double timeout = par("timeout");
    double appl_timeout = par("appl_timeout");
    //the following variables are taken by reference, since they contain
    //random variables
    cPar & conn_ia_time = par("conn_ia_time");
    cPar & msg_ia_time = par("msg_ia_time");
    //cPar& num_message   = par("num_message");                           // ((!!))
    //cPar& rec_pks       = par("rec_pks");
    cPar & message_length = par("message_length");

    //client appl. calls
    cMessage *open_active, *receive_call, *close, *abort;
    //messages received from TCP
    cMessage *e_msg, *r_msg, *c_msg;

    //misc. variable defs
    double dact_num_message = 0;
    int act_num_message = 0, i = 0, tcp_mss = 0;
    TcpConnId tcp_conn_id = 0;

    WATCH(tcp_conn_id);

    for (;;)
    {
        //address and port management (local and remote socket)
        //Usually the client doesn't care much about its local port number.
        //These so called "ephemeral ports" are usually allocated between
        //1024 and 5000.
        //Because we have here only one application connected to "TcpModule" on
        //the client side, the local port number will be 0.
        int local_port = gate("out")->toGate()->index();
        if (debug)
            ev << "Local port: " << local_port << endl;
        //The destination port on the server side is chosen according to the
        //service requested by the client, e.g. the remote port will be set to
        //80 for HTTP.
        //Since the server provides only one service (there is only one
        //TCPServer module running on the server side) the remote port will be
        //set to 0 in this test environment.
        int rem_port = 0;
        if (debug)
            ev << "Remote port: " << rem_port << endl;
        //If a server provides more than one service chose rem_port at random:
        //int rem_port = intrand(num_services); //rem_port=0...num_services-1

        //As IP-addresses we simply use the gate number of the compound modules
        //"Client" and "Server" connected to the Switch.
        //int local_addr  = parentModule()->gate("to_switch")->toGate()->index(); //->findGate("to_switch")-1;

        // changed to parameter
        int local_addr = par("local_addr");

        ev << "Local address: " << local_addr << endl;
        //if only one server is in the network use the next line

        // int rem_addr    = parentModule()->gate("to_switch")->toGate()->size()-1; //->findGate("to_switch");
        int rem_addr = par("server_addr");

        //If there is more than one server chose rem_addr at random:
        //(this only works if all clients are connected to the switch gates
        //0 ... number of clients)
        //int adest     = 1+intrand(num_serv_clie); //num_serv_clie=number of servers
        //int par_index = parentModule()->index(); //index of this client
        //int par_size  = parentModule()->size(); //number of clients
        //int pos       = par_size - par_index;
        //int rem_addr  = parentModule()->findGate("to_switch") + pos + adest;
        if (debug)
            ev << "Remote address: " << rem_addr << endl;

        //intervall between subsequent connections
        wait((double) conn_ia_time);

        //send "active open" (client) call to "TcpModule"
        open_active = new cMessage("TCP_C_OPEN_ACTIVE", TCP_C_OPEN_ACTIVE);

        open_active->addPar("src_port") = local_port;
        open_active->addPar("src_addr") = local_addr;
        open_active->addPar("dest_port") = rem_port;
        open_active->addPar("dest_addr") = rem_addr;

        open_active->addPar("timeout") = timeout;       //global default is 5min; here 5sec

        //precedence, security/compartment and options are not included here

        //no data bits to send
        open_active->setLength(0);
        //open_active->addPar("send_bits") = 0;
        //no data packets to receive
        open_active->addPar("rec_pks") = 0;

        //number of bits requested by a client
        open_active->addPar("num_bit_req") = (unsigned long) message_length;
        unsigned long msg_leng = open_active->par("num_bit_req");       //(unsigned long) open_active->par("num_bit_req")
        if (debug)
            ev << "Number of bits requested by the client application: " << msg_leng << "\n";

        //make delay checking possible
        open_active->setTimestamp();

        //send "active open" to "TcpModule"
        if (debug)
            ev << "Client appl. sending ACTIVE OPEN to TCP.\n";
        send(open_active, "out");

        if (debug)
            ev << "Client appl. waiting for TCP to establish connection ...\n";
        e_msg = receive(appl_timeout);
        while (e_msg == NULL)
        {
            if (debug)
                ev << "Client app timeout occurred while waiting for TCP_I_ESTAB after"
                    << appl_timeout << " s\n";
            e_msg = receive(appl_timeout);
        }
        if (e_msg->kind() == TCP_I_CLOSED)
        {
            if (debug)
                ev << "Client received TCP_I_CLOSED.\n";
            delete e_msg;
            e_msg = 0;
            continue;
        }
        else if (e_msg->kind() != TCP_I_ESTAB)
        {
            if (debug)
                ev << "Client received something other than TCP_I_ESTAB - broken\n";
            goto broken;
        }
        //id set by server (dynamic model creation)
        tcp_conn_id = e_msg->par("tcp_conn_id");
        if (debug)
            ev << "OPEN SUCCESS, TCP in ESTABLISHED, my TCP connection id is ID = "
                << tcp_conn_id << endl;

        //get maximum segment size from TCP (TCP-MSS)
        tcp_mss = e_msg->par("mss");

        delete e_msg;           //((!!))
        e_msg = 0;

        //communication after TCP entered state "ESTABLISHED"
        //issue receive calls to receive data from TCP

        //message_length  = (long) message_length;
        //act_num_message = (long) num_message; //number of receive calls issued ((!!))
        if (debug)
        {
            ev << "Length of message requested (number of bits requested): " << msg_leng << "\n";
            ev << "TCP is using a MSS of " << tcp_mss << " bytes.\n";
            //ev << "leng/mss " << (double) msg_leng / tcp_mss << "\n";
        }
        dact_num_message = (double) msg_leng / 8 / tcp_mss;
        //ev << "Client application will issue " << dact_num_message << " RECEIVE commands.\n";
        act_num_message = (long) ceil(dact_num_message);
        if (debug)
            ev << "Client application will issue " << act_num_message << " RECEIVE commands.\n";

        for (i = 0; i < act_num_message; i++)   //((!!))
        {
            if (debug)
                ev << "Client appl. is requesting data (RECEIVE call) from TCP.\n";
            receive_call = new cMessage("TCP_C_RECEIVE", TCP_C_RECEIVE);

            receive_call->addPar("src_port") = local_port;
            receive_call->addPar("src_addr") = local_addr;
            receive_call->addPar("dest_port") = rem_port;
            receive_call->addPar("dest_addr") = rem_addr;

            receive_call->addPar("tcp_conn_id") = tcp_conn_id;

            //no data bits to send
            receive_call->setLength(0);
            //receive_call->addPar("send_bits") = 0;
            //number of data packets the appl. is ready to receive
            receive_call->addPar("rec_pks") = 1;        //(long) rec_pks;

            //make delay checking possible
            receive_call->setTimestamp();

            //send "receive" to "TcpModule"
            send(receive_call, "out");

            if (debug)
                ev << "Client appl. waiting for data from TCP.\n";
            r_msg = receive(appl_timeout);
            while (r_msg == NULL)
            {
                if (debug)
                    ev << "Client app timeout occurred after " << (double) appl_timeout
                        << "s while waiting for  TCP_I_SEG_FWD\n";
                r_msg = receive(appl_timeout);
            }
            if (r_msg->kind() != TCP_I_SEG_FWD) //((!!))
            {
                if (debug)
                    ev << "Client app received something other than TCP_I_SEG_FWD - broken\n";
                goto broken;
            }
            if (debug)
                ev << "Received data of " << r_msg->length() / 8
                    << " bytes. Deleting data message.\n";
            delete r_msg;
            r_msg = 0;

            //time to wait between receive calls
            wait((double) msg_ia_time);
        }

        //send "close" to teardown TCP connection
        if (debug)
            ev << "Client appl. is sending CLOSE to TCP.\n";
        close = new cMessage("TCP_CLOSE", TCP_C_CLOSE);

        close->addPar("src_port") = local_port;
        close->addPar("src_addr") = local_addr;
        close->addPar("dest_port") = rem_port;
        close->addPar("dest_addr") = rem_addr;

        close->addPar("tcp_conn_id") = tcp_conn_id;

        //no data bits to send
        close->setLength(0);
        //close->addPar("send_bits") = 0;
        //no data packets to receive
        close->addPar("rec_pks") = 0;

        //make delay checking possible
        close->setTimestamp();

        //send "receive" to "TcpModule"
        send(close, "out");

        if (debug)
            ev << "Client appl. is waiting for CLOSED indicated by TCP.\n";
        c_msg = receive(appl_timeout);
        while (c_msg == NULL)
        {
            if (debug)
                ev << "Client app timeout occurred after " << (double) appl_timeout
                    << "s while waiting for TCP_I_CLOSED\n";
            c_msg = receive(appl_timeout);
        }
        if (c_msg->kind() != TCP_I_CLOSED)
        {
            if (debug)
                ev << "Client app received something other than TCP_I_CLOSED - broken\n";
            goto broken;
        }
        ev << "TCP connection CLOSED.\n";
        delete c_msg;
        c_msg = 0;

        //ignore "broken"
        continue;

      broken:
        if (debug)
        {
            ev << "TCP connection broken due to timeout or protocol error (ABORT etc.)!\n";
            //Send ABORT to TCP ???
            // yes! please... anything...
            //send "close" to teardown TCP connection
            ev << "Client appl. is sending CLOSE to TCP.\n";
        }
        abort = new cMessage("TCP_ABORT", TCP_C_ABORT);

        abort->addPar("src_port") = local_port;
        abort->addPar("src_addr") = local_addr;
        abort->addPar("dest_port") = rem_port;
        abort->addPar("dest_addr") = rem_addr;

        abort->addPar("tcp_conn_id") = tcp_conn_id;

        //no data bits to send
        abort->setLength(0);
        //abort->addPar("send_bits") = 0;
        //no data packets to receive
        abort->addPar("rec_pks") = 0;

        //make delay checking possible
        abort->setTimestamp();

        //send "receive" to "TcpModule"
        send(abort, "out");

        //Clean up from where ever goto was used (evil)
        delete c_msg;
        delete r_msg;
        delete e_msg;
        c_msg = r_msg = e_msg = 0;

        if (debug)
            ev << "Client appl. is waiting for CLOSED indicated by TCP.\n";
        c_msg = receive(appl_timeout);  //(appl_timeout);
        while (c_msg == NULL)
        {
            ev << "Client app timeout occurred after " << appl_timeout <<
                "s while waiting for TCP_I_CLOSED\n";
            c_msg = receive(appl_timeout);
        }
        if (c_msg->kind() != TCP_I_CLOSED)
            if (debug)
                ev << "TCP probably did not really close. Other connection can be opened.\n";
        if (debug)
            ev << "TCP connection CLOSED.\n";

        delete c_msg;
        c_msg = 0;
    }
}
