// from MPLS models  --- FIXME merge or eliminate!

#include <omnetpp.h>
#include "tcp.h"
#include "IPAddress.h"


class MyTCPClient: public cSimpleModule
{
  Module_Class_Members(MyTCPClient, cSimpleModule, 16384);

private:

  double keepAliveTime,timeout;

  long msgLng;

  //vector<cMessage*> dataQueue;

  int tcp_mss, local_port, rem_port, local_addr, rem_addr;

  //Number of messages have been sent in one cycle
  int sendNo;

  TcpConnId tcp_conn_id;

  //PSH and URG flag
  TcpFlag tcp_flag_psh;
  TcpFlag tcp_flag_urg;

  void sendOpenActive();

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

  void sendBroken();

};

Define_Module_Like( MyTCPClient, MyTCPApp);


void MyTCPClient::initialize()
{
    tcp_conn_id=0;
    tcp_flag_psh = TCP_F_SET;
    tcp_flag_urg = TCP_F_NSET;
    local_port = 1;  //Any port !=-1
    rem_port = 1;     //Any port   !=-1
}

void MyTCPClient::activity()
{
    local_addr = IPAddress(par("local_addr").stringValue()).getInt();
    rem_addr= IPAddress(par("server_addr").stringValue()).getInt();
    keepAliveTime = par("data_interarrival_time");
    timeout = par("timeout");;
    double startTime = par("start_time");

    //Sleep sometime for LDP to complete
    wait(startTime);

    WATCH(tcp_conn_id);

    local_port= id();
    sendOpenActive();

    //Waiting for ESTAB message
    //we ignore timeout or other possible errors
    cMessage* e_msg;

    e_msg = receive();

    while((e_msg->kind() == TCP_I_CLOSED) || (e_msg->kind() != TCP_I_ESTAB))
    {
        if (e_msg->kind() == TCP_I_CLOSED)
        {
            delete e_msg;
            wait(2 * startTime);
            sendOpenActive();
            e_msg = receive();
            continue;
        }

        if (e_msg->kind() != TCP_I_ESTAB)
        {
            delete e_msg;
            sendBroken();
            wait(2*startTime);
            sendOpenActive();
            e_msg= receive();
            continue;
        }
    }
    tcp_conn_id   = e_msg->par("tcp_conn_id");
    tcp_mss = 8*((e_msg->par("mss")).longValue()); //bits
    msgLng = tcp_mss/4;
    delete e_msg;

    //Start to operate as peer
    //Start the self keep alive cycle
    cMessage* ka_msg =new cMessage();
    scheduleAt(simTime() + keepAliveTime, ka_msg);

    //Send KEEP ALIVE to peer
    sendKEEP_ALIVE();

    while(true)
    {
        sendNo=0;

        //Issue a TCP_RECEIVE, possible data from peer
        issueTCP_RECEIVE();

        cMessage* msg = receive();

        //Process one and only one message from peer
        processMessage(msg);
    }
}


void MyTCPClient::issueTCP_RECEIVE()
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
}


void MyTCPClient::sendKEEP_ALIVE()
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

    ev << "MY TCP CLIENT DEBUG: " << IPAddress(local_addr) << " send data to " <<
    IPAddress(rem_addr) << "\n";
    send(send_call, "out");
}



void MyTCPClient::processSELF_KEEP_ALIVE(cMessage* msg)
{
    if(sendNo == 0)
    {
        sendKEEP_ALIVE();
        sendNo = 1;
    }

    scheduleAt(simTime()+ keepAliveTime, msg);

    cMessage *msgN= receive();
    //Iterate of processMessage till we get message from peer
    processMessage(msgN);  // FIXME this is recursive!!!!!!
}



void MyTCPClient::processMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        processSELF_KEEP_ALIVE(msg);
    }
    else  //Message is data from peer
    {
        processData(msg);
    }
}



void MyTCPClient::processData(cMessage* msg)
{
    if ((msg->kind()) == TCP_I_SEG_FWD)
    {
        ev << "MY TCP CLIENT DEBUG: " << IPAddress(local_addr) << " receives data from " <<
        IPAddress(rem_addr) << "\n";
        delete msg;
    }
    else
    {
        delete msg;
    }
}

void MyTCPClient::sendOpenActive()
{
    //send "active open" (client) call to "TcpModule"
    cMessage *open_active = new cMessage("TCP_C_OPEN_ACTIVE", TCP_C_OPEN_ACTIVE);
    open_active->addPar("src_port")  = local_port;
    open_active->addPar("src_addr")  = local_addr;
    open_active->addPar("dest_port") = rem_port;
    open_active->addPar("dest_addr") = rem_addr;
    open_active->addPar("timeout") = timeout;
    open_active->setLength(0);
    open_active->addPar("rec_pks")     = 0;
    open_active->addPar("num_bit_req") = msgLng;
    open_active->setTimestamp();

    send(open_active, "out");
}

void MyTCPClient::sendBroken()
{
    cMessage* abort = new cMessage("TCP_ABORT", TCP_C_ABORT);

    abort->addPar("src_port")  = local_port;
    abort->addPar("src_addr")  = local_addr;
    abort->addPar("dest_port") = rem_port;
    abort->addPar("dest_addr") = rem_addr;

    abort->addPar("tcp_conn_id") = tcp_conn_id;

    //no data bits to send
    abort->setLength(0);
    //abort->addPar("send_bits") = 0;
    //no data packets to receive
    abort->addPar("rec_pks")     = 0;

    //make delay checking possible
    abort->setTimestamp();

    //send "receive" to "TcpModule"
    send(abort, "out");

    cMessage* c_msg = receive();

    delete c_msg;
}





