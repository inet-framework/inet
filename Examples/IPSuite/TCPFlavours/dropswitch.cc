//-----------------------------------------------------------------------------
//-- fileName: tcptester.cc
//--
//-- generated to test some TCP features
//--
//-- S.Klapp
//-- based on switch.cc from V. Boehm
//-----------------------------------------------------------------------------

#include "nw.h"
#include "ip.h"
#include "tcp.h"
#include "omnetpp.h"


class DropSwitch : public cSimpleModule
{
  Module_Class_Members(DropSwitch, cSimpleModule, 16384);
  virtual void activity();
};

Define_Module(DropSwitch);

void DropSwitch::activity()
{
  cPar& pk_delay   = par("pk_delay");

  cPar& syn_client_del   = par("syn_client_del");
  cPar& syn_server_del   = par("syn_server_del");
  cPar& ack_client_del   = par("ack_client_del");
  cPar& ack_server_del   = par("ack_server_del");
  cPar& tcp_client_del   = par("tcp_client_del");
  cPar& tcp_server_del   = par("tcp_server_del");
  cPar& fin_client_del   = par("fin_client_del");
  cPar& fin_server_del   = par("fin_server_del");

  cPar& del_prob = par("delete_probability");

  cPar& burst_del_prob = par("burst_delete_probability");
  int error_burst_len = par("error_burst_len");

  int server_gate = gate("in", (int) par("server_port"))->id();
  
  bool from_server; 
  bool msg_deleted;
  int msg_kind;
  int burst_error_cnt = 0;

  for(;;)
    {
      //receive frame, implicit queuing
      cMessage* rframe = receive();


      // delete frame with errors
      if(rframe->hasBitError())
    {
      ev << "DELETING frame (BitError)" << endl;
      delete rframe;
    }
      // random error check
      else if(dblrand()*100<(double)del_prob)
    {
          ev << "DELETING frame (RANDOM)" << endl;
          delete rframe;
        }
      else
    {
      msg_deleted=false;
      
      //get the datagram from the incoming frame/packet
      cMessage* datagram  = rframe->decapsulate();
      
      //get length of frame after decapsulation
      int nw_length = rframe->length() / 8;

      //get IP header information about the IP destination address
      IpHeader* ip_header = (IpHeader*)(datagram->par("ipheader").pointerValue());
      int dest = ip_header->ip_dst;

      //get type of message
      cMessage* paket=datagram->decapsulate();
      msg_kind=paket->kind();
      datagram->encapsulate(paket);
      
      from_server = rframe->arrivedOn(server_gate);  
      
      delete rframe;
      
      //create new frame to send to destination
      cMessage* sframe = new cMessage("NW_FRAME", NW_FRAME);
      
      //set length
      sframe->setLength(nw_length * 8);
      
      //encapsulate datagram
      sframe->encapsulate(datagram);
      
      
      wait(pk_delay);


      
      if(from_server)
        {
              ev << "Message arrived from server" << endl;
              if (burst_error_cnt)
                {
                  if (burst_error_cnt == error_burst_len)
                    burst_error_cnt = 0;
                  else
                    {
                      burst_error_cnt++;
                      ev << "Deleting frame number " << burst_error_cnt << endl;
                      msg_deleted=true;
                    }
                }
              else if(dblrand()*100<(double)burst_del_prob)
                {
                  if (error_burst_len)
                    {
                      burst_error_cnt++;
                      ev << "Deleting frame number " << burst_error_cnt << endl;
                      msg_deleted=true;
                    }
                }
              else
                {
                  // datagram from server 
                  // check if datagram should be deleted or send
                  switch(msg_kind)
                    {
                    case SYN_SEG:
                      if((int)syn_server_del>0)
                        {
                          syn_server_del=(int)syn_server_del-1;
                          msg_deleted=true;
                        }
                      break;
                    case ACK_SEG:
                      if((int)ack_server_del>0)
                        {
                          ack_server_del=(int)ack_server_del-1;
                          msg_deleted=true;
                        }
                      break;
                    case TCP_SEG:
                      if((int)tcp_server_del>0)
                        {
                          tcp_server_del=(int)tcp_server_del-1;
                          msg_deleted=true;
                        }
                      break;
                    case FIN_SEG:
                      if((int)fin_server_del>0)
                        {
                          fin_server_del=(int)fin_server_del-1;
                          msg_deleted=true;
                        }
                      break;
                    } // of switch
                }
            }
      else
        {
              ev << "Message arrived from client" << endl;              
          // data from client
          // check if datagram should be deleted or send
          switch(msg_kind)
        {
        case SYN_SEG:
          if((int)syn_client_del>0)
            {
              syn_client_del=(int)syn_client_del-1;
              msg_deleted=true;
            }
          break;
        case ACK_SEG:
          if((int)ack_client_del>0)
            {
              ack_client_del=(int)ack_client_del-1;
              msg_deleted=true;
            }
          break;
        case TCP_SEG:
          if((int)tcp_client_del>0)
            {
              tcp_client_del=(int)tcp_client_del-1;
              msg_deleted=true;
            }
          break;
        case FIN_SEG:
          if((int)fin_client_del>0)
            {
              fin_client_del=(int)fin_client_del-1;
              msg_deleted=true;
            }
          break;
        } // of switch
        } 
      if(!msg_deleted)
        {
          ev << "Relaying frame to " << dest << " (destination IP address).\n";
          send(sframe, "out", dest);
        }
      else
        {
          ev << "DELETING frame (PARAMETER)" << endl;
          delete sframe;
        } 
    }
    }
};








