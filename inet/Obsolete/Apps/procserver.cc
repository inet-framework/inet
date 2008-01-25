//
//-----------------------------------------------------------------------------
//-- fileName: procserver.cc
//--
//-- generated to test the TCP-FSM
//--
//-- V. Boehm, June 20 1999
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

class ProcServer : public cSimpleModule
{
  Module_Class_Members(ProcServer, cSimpleModule, 16384);
  virtual void activity();
private:
  bool debug;
};

Define_Module(ProcServer);

void ProcServer::activity()
{
  //module parameters
  debug = parentModule()->par("debug");

  cPar& timeout         = parentModule()->par("timeout");
  cPar& appl_timeout    = parentModule()->par("appl_timeout");
  cPar& processing_time = parentModule()->par("processing_time");
  //cPar& num_message = parentModule()->par("num_message");
  //cPar& msg_length = parentModule()->par("msg_length");

  unsigned long msg_length;

  //server appl. calls
  cMessage* payload_msg = NULL;
  cMessage* send_call   = NULL;
  cMessage* close       = NULL;
  //TCP response msg
  cMessage *msg = NULL;

  //ports and addresses
  int rem_port, rem_addr, local_port, local_addr;

  //PSH and URG flag
  TcpFlag tcp_flag_psh;
  TcpFlag tcp_flag_urg;

  // TPC is listening
  // expecting TCP_I_RCVD_SYN)
  if (debug)
    ev << "waiting for TCP_I_RCVD_SYN\n";
  msg = receive(); // no timeout here
  if (msg->kind() == TCP_I_CLOSED)
    {
      if (debug)
        ev << "received TCP_I_CLOSED while waiting for TCP_I_RCVD_SYN.\n";
      goto closed;
    }
  else if (msg->kind() != TCP_I_RCVD_SYN)
    {
      if (debug)
        ev << "received something other than TCP_I_RCVD_SYN - broken.\n";
      goto broken;
    }
  else if (debug)
    ev << id() << "received TCP_I_RCVD_SYN\n";

  // received TCP_I_RCVD_SYN
  // address and port management
  rem_port = msg->par("src_port"); //client TCP-port at remote client side
  rem_addr = msg->par("src_addr"); //client IP-address at remote client side
  local_port = msg->par("dest_port"); //own server TCP-port
  local_addr = msg->par("dest_addr"); //own server IP-address

  // get the number of bits requested by the client
  msg_length = msg->par("num_bit_req");

  delete msg;


  // receive msg from ProcServer/TCP
  // expecting TCP_I_ESTAB or TCP_I_CLOSED
  // FIXME: MBI should maybe use other message if connection drop?
  if (debug)
    ev << "waiting for TCP_I_ESTAB or TCP_I_CLOSED\n";
  msg = receive(appl_timeout);
  while (msg == NULL)
    {
      if (debug)
        ev << "timeout occured after " << (double) appl_timeout << "s while waiting for TCP_I_ESTAB or TCP_I_CLOSED\n";
      msg = receive(appl_timeout);
    }
  if (msg->kind() == TCP_I_CLOSED)
    {
      if (debug)
        ev << ": Connection CLOSED by TCP.\n";
      goto closed;
    }
  else if (msg->kind() != TCP_I_ESTAB)
    {
      if (debug)
        ev << "received something other than TCP_I_ESTAB - broken\n";
      goto broken;
    }
  else if (debug)
    ev << "received TCP_I_ESTAB\n";

  //address and port management
  rem_port = msg->par("src_port"); //client TCP-port at remote client side
  rem_addr = msg->par("src_addr"); //client IP-address at remote client side
  local_port = msg->par("dest_port"); //own server TCP-port
  local_addr = msg->par("dest_addr"); //own server IP-address
  delete msg;

  tcp_flag_psh = TCP_F_NSET;
  tcp_flag_urg = TCP_F_NSET;

  send_call = new cMessage("TCP_C_SEND", TCP_C_SEND);

  // Create a Payload-Data-Message for send_call;
  payload_msg = new cMessage("APPL_PAYLOAD");
  payload_msg->setLength((long) msg_length); // number of Bits
  send_call->addPar("APPL_PAYLOAD") = payload_msg;

  send_call->addPar("tcp_conn_id")  = id();
  send_call->addPar("src_port")     = local_port;
  send_call->addPar("src_addr")     = local_addr;
  send_call->addPar("dest_port")    = rem_port;
  send_call->addPar("dest_addr")    = rem_addr;

  send_call->addPar("tcp_flag_psh") = (int) tcp_flag_psh;
  send_call->addPar("tcp_flag_urg") = (int) tcp_flag_urg;
  send_call->addPar("timeout")      = (int) timeout;

  //number of bits to be sent by TCP
  send_call->setLength((long) msg_length);
  //send_call->addPar("send_bits") = (long) msg_length;

  // FIXME: Was macht das?
  //no data packets to receive
  send_call->addPar("rec_pks") = 0;

  //make delay checking possible
  send_call->setTimestamp();

  //send "send" to TCP
  if (debug)
    ev << "sending SEND call/data bits to TCP.\n";
  send(send_call, "out");

  wait((double) processing_time);
  if (debug)
    ev << "waiting for TCP_I_CLOSED or TCP_I_CLOSE_WAIT ...\n";
  msg = receive(appl_timeout);
  while (msg == NULL)
    {
      if (debug)
        ev << "timeout occurred after " << (double) appl_timeout << "s while waiting for TCP_I_CLOSED or TCP_I_CLOSE_WAIT\n";
      msg = receive(appl_timeout);
    }
  if (msg->kind() == TCP_I_CLOSED)
    {
      goto closed;
    }

  if (msg->kind() == TCP_I_CLOSE_WAIT)
    {
      if (debug)
        ev << "TCP entered CLOSE_WAIT. Closing connection by sending CLOSE.\n";
      close = new cMessage("TCP_CLOSE", TCP_C_CLOSE);
      close->addPar("src_port") = local_port;
      close->addPar("src_addr") = local_addr;
      close->addPar("dest_port") = rem_port;
      close->addPar("dest_addr") = rem_addr;
      close->addPar("tcp_conn_id") = id();

      //no data packets to send
      close->setLength(0);
      //close->addPar("send_bits") = 0;
      //no data bytes to receive
      close->addPar("rec_pks") = 0;

      //make delay checking possible
      close->setTimestamp();

      //send "receive" to "TcpModule"
      if (debug)
        ev << "sending CLOSE call to TCP.\n";
      send(close, "out");

      if (debug)
        ev << "waiting for TCP_I_CLOSED.\n";
      msg = receive(appl_timeout);
      while (msg == NULL)
        {
          if (debug)
            ev << "timeout occured after " << (double) appl_timeout << "s while waiting for TCP_I_CLOSED\n";
          msg = receive(appl_timeout);
        }
      if (msg->kind() != TCP_I_CLOSED)
        {
          if (debug)
            ev << "received something other than TCP_I_CLOSED. - broken\n";
          goto broken;
        }

      if (debug)
        ev << "TCP connection CLOSED.\n";
      delete msg;
      deleteModule();
      return;
    }
  else
    {
      if (debug)
        ev << "received something other than TCP_I_CLOSE_WAIT - broken\n";
      goto broken;
    }

 broken:
  if (debug)
    ev << "TCP connection broken due to timeout or protocol error (ABORT etc.)!\n";

 closed:
  if (debug)
    ev << "TCP connection CLOSED (without closewait).\n";
  delete msg;
  deleteModule();
}
