// $Header$
//-----------------------------------------------------------------------------
//-- fileName: server.cc
//--
//-- generated to test UDP socket layer
//--
//-- generated to test the appli-server
//-- 11,09,2001
//--
//-- -----------------------------------------------------------------------------
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

#include "omnetpp.h"
#include "SocketInterfacePacket.h"
//#include "tcp.h"

class Server: public cSimpleModule
{
  Module_Class_Members(Server, cSimpleModule, 16384);
  virtual void activity();
};

Define_Module_Like( Server, SocketApp);

void Server::activity()
{
  //module parameters
  bool   debug        = par("debug");

  SocketInterfacePacket* sockipack;
  Socket::Filedesc filedesc, filedesc_reply;
  SocketInterfacePacket:: SockAction action;
  IN_Addr remote_addr;
  IN_Port remote_port;


  // wait(2000);

  //
  // SOCKET
  //
  sockipack = new SocketInterfacePacket("SocketCall");

  if (debug)
    {
      ev << "Calling function socket()\n";
    }

  sockipack->socket(Socket::SOCKET_AF_INET, Socket::SOCKET_DGRAM, Socket::UDP);
  //sockipack->socket(Socket::AF_INET, Socket::SOCK_DGRAM, Socket::UDP);


  if (debug)
    {
      ev << "Sending message to Socketlayer\n";
    }
  send(sockipack, "out");

  if (debug)
    {
      ev << "Waiting for message from Socketlayer\n";
    }


  sockipack = (SocketInterfacePacket*) receive();

  filedesc = sockipack->filedesc();
  if (debug)
    {
      ev << "Received message from Socketlayer\n";
      ev << "Value of Filedescriptor: " << filedesc << endl;
    }

  delete sockipack;

  // BIND

  sockipack = new SocketInterfacePacket("BindCall");

  if (debug)
    {
      ev <<"Calling function bind()\n";
    }
  sockipack->bind(filedesc, IN_Addr(IN_Addr::ADDR_UNDEF), 30);

  if (debug)
    {
      ev << "Sending message to Socketlayer\n";
    }
  send(sockipack, "out");

  wait(1);

  //
  // READ
  //
      
  sockipack = (SocketInterfacePacket*) receive();

  cMessage* msg=sockipack->decapsulate();

  ev << "Message from Client: " << msg->par("Index") << " " << msg->par("Message") << endl;

  remote_addr = sockipack->fAddr();
  remote_port = sockipack->fPort();

  delete msg;
  delete sockipack;
      
  //
  // SOCKET
  //

  sockipack = new SocketInterfacePacket("SocketCall");

  if (debug)
    {
      ev << "Calling function socket()\n";
    }

  sockipack->socket(Socket::SOCKET_AF_INET, Socket::SOCKET_DGRAM, Socket::UDP);


  if (debug)
    {
      ev << "Sending message to Socketlayer\n";
    }
  send(sockipack, "out");

  if (debug)
    {
      ev << "Waiting for message from Socketlayer\n";
    }


  sockipack = (SocketInterfacePacket*) receive();

  filedesc_reply = sockipack->filedesc();
  if (debug)
    {
      ev << "Received message from Socketlayer\n";
      ev << "Value of Filedescriptor: " << filedesc_reply << endl;
    }

  delete sockipack;

  //
  // CONNECT
  //
      
  sockipack = new SocketInterfacePacket("ConnectCall");


  if (debug)
    {
      ev <<"Calling function connect()\n";
    }
  sockipack->connect(filedesc_reply, remote_addr, remote_port);

  if (debug)
    {
      ev << "Sending message to Socketlayer\n";
    }
  send(sockipack, "out");
  {
    ev <<"Waiting for Socketlayer to establish connection\n";
  }


  sockipack = (SocketInterfacePacket*) receive();

  action = sockipack->action();
  ev <<"Received message from Socketlayer\n";
  ev <<"Status of Client: " << action << endl;

  if (action == SocketInterfacePacket::SA_CONNECT_RET)
    {
      ev <<"Status of Client: SA_CONNECT_RET, Connection OK\n" ;
    }

  else
    {
      error ("Connection failed");
    }

  delete sockipack;
      
  //
  // WRITE
  //

      
  cMessage* datapack = new cMessage("Payload");

  datapack->addPar("Message") = "A reply";
  sockipack = new SocketInterfacePacket("APPL-DATA");
  sockipack->write(filedesc_reply, datapack);
  send(sockipack, "out");
  wait(20000);

  //
  // CLOSE
  //
      
  sockipack = new SocketInterfacePacket("CloseCall");

  if (debug)
    {
      ev <<"Calling function close() on filedesc " << (int) filedesc << endl;
    }
  sockipack->close(filedesc);
  send(sockipack, "out");

  sockipack = new SocketInterfacePacket("CloseCall");

  if (debug)
    {
      ev <<"Calling function close() on filedesc " << (int) filedesc_reply << endl;
    }
  sockipack->close(filedesc_reply);
  send(sockipack, "out");
  ev <<"Connection is closed\n";

  wait(4000);
}
