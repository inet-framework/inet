//
//-----------------------------------------------------------------------------
//-- fileName: TCPServer.cc
//--
//-- generated to test the TCP-FSM
//--
//-- V. Boehm, June 20 1999
//--
//-- modified by J. Reber for IP architecture, October 2000
//--
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

#include "tcp.h"
#include <omnetpp.h>

class TCPServer:public cSimpleModule
{
  private:
    bool debug;
    void passiveOpen(double timeout, cModuleType * procserver_type);
  public:
      Module_Class_Members(TCPServer, cSimpleModule, 16384);
    virtual void activity();
};

Define_Module_Like(TCPServer, TCPApp);

void TCPServer::activity()
{
    TcpConnId tcp_conn_id;
    WATCH(tcp_conn_id);

    cModule *mod;

    //module parameters
    debug = par("debug");

    double timeout = par("timeout");
    double appl_timeout = par("appl_timeout");  // FIXME not used

    cModuleType *procserver_type = findModuleType("ProcServer");
    if (!procserver_type)
        error("Cannot find module type ProcServer");

    passiveOpen(timeout, procserver_type);

    for (;;)
    {
        cMessage *msg = receive();      // no application timeout here
        switch (msg->kind())
        {
        case TCP_I_RCVD_SYN:
            tcp_conn_id = msg->par("tcp_conn_id");
            if (debug)
            {
                ev << "ApplServer has been notified by TCP that TCP received a SYN-Segment.\n";
                ev << "Redirecting msg to ProcServer with TCP connection ID = "
                    << tcp_conn_id << endl;
            }
            mod = simulation.module(tcp_conn_id);
            if (!mod)
            {
                if (debug)
                    ev << "This ProcServer exited, deleting msg.\n";
                delete msg;
            }
            else
            {
                sendDirect(msg, 0.0, mod, "in");
            }

            passiveOpen(timeout, procserver_type);

            break;

        case TCP_I_ESTAB:
            tcp_conn_id = msg->par("tcp_conn_id");
            if (debug)
            {
                ev << "OPEN SUCCESS: TCP in ESTABLISHED.\n";
                ev << "Redirecting msg to procserver with TCP connection ID = "
                    << tcp_conn_id << endl;
            }
            mod = simulation.module(tcp_conn_id);
            if (!mod)
            {
                if (debug)
                    ev << "This ProcServer exited, deleting msg.\n";
                delete msg;
            }
            else
            {
                sendDirect(msg, 0.0, mod, "in");
            }

            //passiveOpen(timeout, procserver_type);

            break;

        default:
            tcp_conn_id = msg->par("tcp_conn_id");
            if (debug)
                ev << "Redirecting msg to procserver with TCP connection ID = "
                    << tcp_conn_id << endl;
            mod = simulation.module(tcp_conn_id);
            if (!mod)
            {
                if (debug)
                    ev << "This ProcServer exited, deleting msg.\n";
                delete msg;
            }
            else
            {
                sendDirect(msg, 0.0, mod, "in");
            }
            break;
        }
    }
}

void TCPServer::passiveOpen(double timeout, cModuleType * procserver_type)
{
    //server appl. calls
    cMessage *open_passive;

    //address and port management (local socket only)
    //An unspecified foreign socket is used. Thus, any connection request
    //by any client is accepted (if the foreign socket is also specified the
    //server will accept a connection request by the specified client only).
    int local_port = gate("out")->toGate()->index();
    // local addr parameterized now
    //int local_addr                  = parentModule()->gate("to_switch")->toGate()->index(); //->findGate("to_switch");
    int local_addr = par("local_addr");
    int rem_port = -1;          //set to -1 since not yet specified (passive open)
    int rem_addr = -1;          //set to -1 since not yet specified (passive open)

    if (debug)
    {
        ev << "Local port: " << local_port << endl;
        ev << "Local address: " << local_addr << endl;
        ev << "Remote port: " << rem_port << endl;
        ev << "Remote address: " << rem_addr << endl;
    }
    //send "passive open" to TCP to indicate that the server appl. is
    //ready, processing continues if TCP indicates that it entered the
    //ESTABLISHED state
    open_passive = new cMessage("TCP_C_OPEN_PASSIVE", TCP_C_OPEN_PASSIVE);
    open_passive->addPar("src_port") = local_port;
    open_passive->addPar("src_addr") = local_addr;
    open_passive->addPar("dest_port") = rem_port;
    open_passive->addPar("dest_addr") = rem_addr;

    open_passive->addPar("timeout") = timeout;  //global default is 5 min; here 5 sec
    //precedence, security/compartment and options are not included here

    //no data bits to send
    //open_passive->addPar("send_bits") = 0;
    open_passive->setLength(0);
    //no data packets to receive
    open_passive->addPar("rec_pks") = 0;

    //make delay checking possible
    open_passive->setTimestamp();

    //create module "ProcServer" dynamically within module "TCPServer"
    cModule *mod;

    mod = procserver_type->createScheduleInit("procserver", this);
    if (debug)
        ev << "Created server process, TCP conn. ID = " << mod->id() << endl;
    mod->gate("out")->setTo(gate("out"));
    open_passive->addPar("tcp_conn_id") = mod->id();

    //send "passive open" to "TcpModule"
    if (debug)
        ev << "sending PASSIVE OPEN.\n";
    send(open_passive, "out");
    if (debug)
        ev << "waiting for TCP to establish connection ...\n";
}
