// -*- C++ -*-
// $Header$
//
// Copyright (C) 2001 Institut fuer Nachrichtentechnik, Universitaet Karlsruhe
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


#ifndef SOCKET_H
#define SOCKET_H

#include <iostream>
#include <omnetpp.h>

#include "pcb.h"


class INET_API Socket : public cObject
{
public:

  enum Domain
    {
      IPSuite_AF_UNIX,  // file system pathnames not used
      IPSuite_AF_INET,   // internet address
      IPSuite_AF_UNDEF
    };

  static const char* const domain_string[]; // defined in sockets.cc

  enum Type
    {
      IPSuite_SOCK_STREAM,      // sequenced, reliable, bidirectional, connection-mode
                                // byte streams, and may provide a transmission
                                // mechanism for out-of-band data (TCP)
      IPSuite_SOCK_DGRAM,       // provides datagrams, which are connectionless-mode,
                                // unreliable messages of fixed maximum length (UDP)
      IPSuite_SOCK_RAW,         // best-effort network-level datagram service (ICMP,
                                // IGMP, raw IP)
      IPSuite_SOCK_SEQPACKET,   // provides sequenced, reliable, bidirectional,
                                // connection-mode transmission path for records.
      IPSuite_SOCK_UNDEF
    };

  static const char* const type_string[]; // defined in sockets.cc

  enum Protocol
    {
      UDP,
      TCP,
      ICMP,
      IGMP,
      ROUTE,
      PROTO_UNDEF
    };

  static const char* const protocol_string[]; // defined in sockets.cc

  enum ConnectionState
    {
      CONN_LISTEN,
      CONN_HALFESTAB,
      CONN_ESTAB,
      CONN_UNDEF
    };

  static const char* const connstate_string[]; // defined in sockets.cc

  // FIXME: Document which options are used
  struct Options
  {
    unsigned int IPSuite_SO_ACCEPTCONN  : 1; // socket accepts incoming connections
                                             // (kernel only)
    unsigned int IPSuite_SO_BROADCAST   : 1; // socket can send broadcast messages
    unsigned int IPSuite_SO_DEBUG       : 1; // socket records debugging information
    unsigned int IPSuite_SO_DONTROUTE   : 1; // output operations bypass routing tables
    unsigned int IPSuite_SO_KEEPALIVE   : 1; // socket probes idle connections
    unsigned int IPSuite_SO_OOBINLINE   : 1; // socket keeps out-of-band data inline
    unsigned int IPSuite_SO_REUSEADDR   : 1; // socket can reuse a local address
    unsigned int IPSuite_SO_REUSEPORT   : 1; // socket can reuse a local port
    unsigned int IPSuite_SO_USELOOPBACK : 1; // routing domain sockets only; sending
                                             // process receives its own routing
                                             // requests
  };

  // FIXME: Document which state fields are used
  struct State
  {
    unsigned int SS_CANTRCVMORE     : 1; // socket cannot receive more data from
                                         // peer
    unsigned int SS_CANTSENDMORE    : 1; // socket cannot send more data to peer
    unsigned int SS_ISCONFIRMING    : 1; // socket is negotiating a connection request
    unsigned int SS_ISCONNECTED     : 1; // socket is connected to a foreign socket
    unsigned int SS_ISCONNECTING    : 1; // socket is connecting to a foreign socket
    unsigned int SS_ISDISCONNECTING : 1; // socket is disconnecting from peer
    unsigned int SS_NOFDREF         : 1; // socket is not associated with a descriptor
    unsigned int SS_PRIV            : 1; // socket was created by a process with
                                         // superuser privileges
    unsigned int SS_RCVATMARK       : 1; // process has consumed all data
                                         // received before the most recent
                                         // out-of-band data was received
  };

  typedef int Filedesc; // file descriptor

  static const Filedesc FILEDESC_UNDEF = -1;

private:
  Socket::Domain   _domain; // in fact, belongs to the protosw structure but
                               // we have none so far
  Socket::Type     _type;
  Socket::Protocol _proto;
  Socket::Options  _options;
  Socket::State    _state;
  Socket::ConnectionState _connstate; // connection specific state of the
                                      // socket. In real world implementation
                                      // this is handled by different queues
                                      // attached to the listening socket
  bool _pending_accept; // for listening sockets, if an accept() has already
                        // been issued by the application

  PCB*             _pcb; // Pointer to Protocol Control Block class
  cQueue           _sockqueue; // queue for incoming data packets

  void _initOptions();
  void _initState();
  void _init();
public:


  // creation, destruction, copying
  Socket(const Socket& socket);
  Socket();
  explicit Socket(const char* name);
  Socket(Socket::Domain domain, Socket::Type type, Socket::Protocol proto);
  //Socket(const char* name, cOjbect* ownerobj);
  virtual ~Socket();
  virtual const char* className() const {return "Socket";}
  virtual cObject* dup() const {return new Socket(*this);}
  virtual void info(char* buf);
  virtual void writeContents(std::ostream& os);
  Socket& operator=(const Socket& socket);

  // new member functions
  Type type() const {return _type;}
  Protocol protocol() const {return _proto;}
  PCB* pcb() const {return _pcb;}

  // Options
  void setIPSuite_SO_ACCEPTCONN(bool val = true)  {_options.IPSuite_SO_ACCEPTCONN  = val;}
  void setIPSuite_SO_BROADCAST(bool val = true)   {_options.IPSuite_SO_BROADCAST   = val;}
  void setIPSuite_SO_DEBUG(bool val = true)       {_options.IPSuite_SO_DEBUG       = val;}
  void setIPSuite_SO_DONTROUTE(bool val = true)   {_options.IPSuite_SO_DONTROUTE   = val;}
  void setIPSuite_SO_KEEPALIVE(bool val = true)   {_options.IPSuite_SO_KEEPALIVE   = val;}
  void setIPSuite_SO_OOBINLINE(bool val = true)   {_options.IPSuite_SO_OOBINLINE   = val;}
  void setIPSuite_SO_REUSEADDR(bool val = true)   {_options.IPSuite_SO_REUSEADDR   = val;}
  void setIPSuite_SO_REUSEPORT(bool val = true)   {_options.IPSuite_SO_REUSEPORT   = val;}
  void setIPSuite_SO_USELOOPBACK(bool val = true) {_options.IPSuite_SO_USELOOPBACK = val;}

  bool IPSuite_SO_ACCEPTCONN()  {return _options.IPSuite_SO_ACCEPTCONN;}
  bool IPSuite_SO_BROADCAST()   {return _options.IPSuite_SO_BROADCAST;}
  bool IPSuite_SO_DEBUG()       {return _options.IPSuite_SO_DEBUG;}
  bool IPSuite_SO_DONTROUTE()   {return _options.IPSuite_SO_DONTROUTE;}
  bool IPSuite_SO_KEEPALIVE()   {return _options.IPSuite_SO_KEEPALIVE;}
  bool IPSuite_SO_OOBINLINE()   {return _options.IPSuite_SO_OOBINLINE;}
  bool IPSuite_SO_REUSEADDR()   {return _options.IPSuite_SO_REUSEADDR;}
  bool IPSuite_SO_REUSEPORT()   {return _options.IPSuite_SO_REUSEPORT;}
  bool IPSuite_SO_USELOOPBACK() {return _options.IPSuite_SO_USELOOPBACK;}

  // State
  void setSS_CANTRCVMORE(bool val = true)     {_state.SS_CANTRCVMORE     = val;}
  void setSS_CANTSENDMORE(bool val = true)    {_state.SS_CANTSENDMORE    = val;}
  void setSS_ISCONFIRMING(bool val = true)    {_state.SS_ISCONFIRMING    = val;}
  void setSS_ISCONNECTED(bool val = true)     {_state.SS_ISCONNECTED     = val;}
  void setSS_ISCONNECTING(bool val = true)    {_state.SS_ISCONNECTING    = val;}
  void setSS_ISDISCONNECTING(bool val = true) {_state.SS_ISDISCONNECTING = val;}
  void setSS_NOFDREF(bool val = true)         {_state.SS_NOFDREF         = val;}
  void setSS_PRIV(bool val = true)            {_state.SS_PRIV            = val;}
  void setSS_RCVATMARK(bool val = true)       {_state.SS_RCVATMARK       = val;}

  bool SS_CANTRCVMORE()     {return _state.SS_CANTRCVMORE;}
  bool SS_CANTSENDMORE()    {return _state.SS_CANTSENDMORE;}
  bool SS_ISCONFIRMING()    {return _state.SS_ISCONFIRMING;}
  bool SS_ISCONNECTED()     {return _state.SS_ISCONNECTED;}
  bool SS_ISCONNECTING()    {return _state.SS_ISCONNECTING;}
  bool SS_ISDISCONNECTING() {return _state.SS_ISDISCONNECTING;}
  bool SS_NOFDREF()         {return _state.SS_NOFDREF;}
  bool SS_PRIV()            {return _state.SS_PRIV;}
  bool SS_RCVATMARK()       {return _state.SS_RCVATMARK;}

  void setConnState(enum ConnectionState state) {_connstate = state;}
  ConnectionState connState() const {return _connstate;}

  void setPendingAccept(bool val = true) {_pending_accept = val;}
  bool pendingAccept() {return _pending_accept;}
  bool isFullySpecified();
};

// helper functions
PCB* _createPCB(Socket::Protocol proto);


#endif // SOCKET_H
