#ifndef __LDPInterface_H__
#define __LDPInterface_H__



#include <string>
#include <omnetpp.h>
#include <iostream>
#include <vector>
#include "tcp.h"
#include "LDPpacket.h"
#include "LDPproc.h"


//Parameter
//timeout, appl_timeout, peerNo, local_addr

typedef struct
{
    int tcp_conn_id;
    int mod_id;
    int peerAddr;
} ldp_session_type;



class LDPInterface: public cSimpleModule
{
private:
  bool debug;
  int local_addr;
  int local_port;
  int rem_port;
  double       timeout;
  double       appl_timeout, keepAliveTime;
  vector<ldp_session_type> ldpSessions;
  //vector<fec_session_bind> fecSessionBinds;
  cModuleType *procserver_type;

  cModuleType *procclient_type;

  void passiveOpen(double timeout, cModuleType* procserver_type);
  void createClient(int destAddr);
  void createServer();
  int getModidByPeerIP(int peerIP);

  public:
//Message received from peers
  cQueue msgQueue;
  //Duplicated request messages
   cArray requestMessageQueue;
  //My own requests from MPLSSwitch
   cArray my_requestMessageQueue;


  Module_Class_Members(LDPInterface, cSimpleModule, 16384);
  virtual void activity();
  virtual void initialize();
  //int id(){return this->mod_id;}


  void printDebugInfo(string dInfo);
  virtual void finish(){};


  ~LDPInterface(){}
};

#endif


