
#include <omnetpp.h>
#include "tcp.h"
#include "IPAddress.h"

class MyTCPServerProc : public cSimpleModule
{
  Module_Class_Members(MyTCPServerProc, cSimpleModule, 16384);

  private:
  double keepAliveTime,timeout;

  long msgLng;

  //vector<cMessage*> dataQueue;

  int tcp_mss, local_port, rem_port, local_addr, rem_addr;
  int sendNo;

  TcpConnId tcp_conn_id;

  //PSH and URG flag
  TcpFlag tcp_flag_psh;
  TcpFlag tcp_flag_urg;

  public:
  virtual void initialize();

  virtual void activity();

  //Send TCP_RECEIVE to tcp
  void issueTCP_RECEIVE();

  //Send KEEP ALIVE to peer
  void sendKEEP_ALIVE();

  //Process data from peer
  void processData(cMessage* msg);

  //Process KEEP ALIVE timer
  void processSELF_KEEP_ALIVE(cMessage* msg);

  //Process messages
  void processMessage(cMessage* msg);

  void doClose();
};

Define_Module(MyTCPServerProc);

void MyTCPServerProc::initialize()
{


        rem_addr = -1;

    tcp_conn_id=0;

    tcp_flag_psh = TCP_F_SET;

    tcp_flag_urg = TCP_F_NSET;

    local_port = 1;//ConstType::ldp_port;

    rem_port = 1;//ConstType::ldp_port;

}


void MyTCPServerProc::activity()
{


    cModule* mod = parentModule();

    local_addr    =  IPAddress(mod->par("local_addr").stringValue()).getInt();

    timeout = mod->par("timeout");

    keepAliveTime   =  mod->par("data_interarrival_time") ;



      //Expect SYN message

      cMessage* msg = receive(); // no timeout here


      if ((msg->kind() == TCP_I_CLOSED) || (msg->kind() !=TCP_I_RCVD_SYN))
      {
            delete msg;

            doClose();
           }

      rem_port = msg->par("src_port"); //client TCP-port at remote client side

      rem_addr = msg->par("src_addr"); //client IP-address at remote client side

      local_port = msg->par("dest_port"); //own server TCP-port

      local_addr = msg->par("dest_addr"); //own server IP-address



      delete msg;


      //Expect ESTAB message

      msg = receive();

      rem_port = msg->par("src_port"); //client TCP-port at remote client side

      rem_addr = msg->par("src_addr"); //client IP-address at remote client side

      local_port = msg->par("dest_port"); //own server TCP-port

      local_addr = msg->par("dest_addr"); //own server IP-address

      if(msg->hasPar("mss"))
              tcp_mss   = 8*((msg->par("mss")).longValue());   //bits
          else
              tcp_mss = 8; //Length is not important as this stage, dummy value

      msgLng = tcp_mss/4;  //bits

       ev << "MY_TCP_SERVER_PROC DEBUG:  LSR(" << IPAddress(local_addr) << ") received ESTAB from LSR(" <<
       IPAddress(rem_addr) << ")\n";;

      delete msg;


      tcp_conn_id = id();

      //Start to operate as peer


      //Start the self KEEP ALIVE cycle

      cMessage* ka_msg =new cMessage();

      scheduleAt(simTime() + keepAliveTime, ka_msg);

      //Send KEEP ALIVE to peer

      sendKEEP_ALIVE();

      // following while(true) line was changed to while(this!=0) [which is equally true]
      // because the former crashed MSVC6.0sp5 (!!) with:
      //   fatal error C1001: INTERNAL COMPILER ERROR
      //     (compiler file 'E:\8966\vc98\p2\src\P2\main.c', line 494)
      //while(true)
      while(this!=0)
      {

           sendNo=0;

          //Issue a TCP_RECEIVE, possible data from peer
          issueTCP_RECEIVE();

          cMessage* msg = receive();

          processMessage(msg);


      }

}


void MyTCPServerProc::issueTCP_RECEIVE()
{
        cMessage* receive_call = new cMessage("TCP_C_RECEIVE", TCP_C_RECEIVE);

            receive_call->addPar("src_port")  = local_port;

        receive_call->addPar("src_addr")  = local_addr;

             receive_call->addPar("dest_port") = rem_port;

          receive_call->addPar("dest_addr") = rem_addr;

          receive_call->addPar("tcp_conn_id") = tcp_conn_id;

            receive_call->setLength(0);

          receive_call->addPar("rec_pks")     = 1; //(long) rec_pks;

          receive_call->setTimestamp();

          //send "receive" to "TcpModule"
          send(receive_call, "out");

        ev << "TCP_SERVER_PROC DEBUG: " << IPAddress(local_addr) << " send TCP_C_RECEIVE to " <<
        IPAddress(rem_addr) << "\n";
}


void MyTCPServerProc::sendKEEP_ALIVE()
{
      cMessage *send_call = new cMessage("TCP_C_SEND", TCP_C_SEND);

      send_call->addPar("tcp_conn_id")  =  tcp_conn_id;

      send_call->addPar("src_port")     = local_port;

      send_call->addPar("src_addr")     = local_addr;

      send_call->addPar("dest_port")    = rem_port;

      send_call->addPar("dest_addr")    = rem_addr;

      send_call->addPar("tcp_flag_psh") = (int) tcp_flag_psh;

      send_call->addPar("tcp_flag_urg") = (int) tcp_flag_urg;

      send_call->addPar("timeout")      = (int) timeout;

      send_call->setLength(msgLng); //(tcp_mss * 8/4);

      send_call->addPar("rec_pks") = 0;

      send_call->addPar("keep_alive") =0;

      send_call->setTimestamp();

      ev << "TCP_SERVER_PROC DEBUG: " << IPAddress(local_addr) << " send DATA to " <<
        IPAddress(rem_addr) << "\n";
      send(send_call, "out");


}



void MyTCPServerProc::processSELF_KEEP_ALIVE(cMessage* msg)
{



     if(sendNo==0)
     {
            sendKEEP_ALIVE();
            sendNo=1;
     }

      scheduleAt(simTime()+ keepAliveTime, msg);

     cMessage *msgN= receive();

     //Iterate of processMessage till we get message from peer
      processMessage(msgN);


}



void MyTCPServerProc::processMessage(cMessage* msg)
{

        if(msg->isSelfMessage())
    {
        processSELF_KEEP_ALIVE(msg);

      }

    else  //Message is data from peer
    {
        processData(msg);

    }
}



void MyTCPServerProc::processData(cMessage* msg)
{
    if ((msg->kind()) == TCP_I_SEG_FWD)
    {

    ev << "MY_TCP_SERVER_PROC DEBUG: " << IPAddress(local_addr) << " receives data from " <<
    IPAddress(rem_addr) << "\n";

    delete msg;

      }

   else
   {

      delete msg;

   }
}


void MyTCPServerProc::doClose()
{
  deleteModule();
}







