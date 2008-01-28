// from MPLS models  --- FIXME merge or eliminate!

#include "tcp.h"
#include <omnetpp.h>
#include "IPAddress.h"

class MyTCPServer: public cSimpleModule
{
private:
  void passiveOpen(double timeout, cModuleType* procserver_type);

public:
  Module_Class_Members(MyTCPServer, cSimpleModule, 16384);
  virtual void activity();
};

Define_Module_Like( MyTCPServer, MyTCPApp);

void MyTCPServer::activity()
{
    TcpConnId tcp_conn_id;
    WATCH(tcp_conn_id);

    cModule* mod;
    cArray* msg_list;

    double timeout = par("timeout");
    int i;

    cModuleType *procserver_type = findModuleType("MyTCPServerProc");
    if (!procserver_type)
        error("Cannot find module type MyTCPServerProc");

    passiveOpen(timeout, procserver_type);

    for(;;)
    {
        cMessage *msg = receive(); // no application timeout here
        switch(msg->kind())
        {
            case TCP_I_RCVD_SYN:
                tcp_conn_id = msg->par("tcp_conn_id");
                mod = simulation.module(tcp_conn_id);
                if (!mod)
                {
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
                mod = simulation.module(tcp_conn_id);
                if (!mod)
                {
                    delete msg;
                }
                else
                {
                    sendDirect(msg, 0.0, mod, "in");
                }
                //passiveOpen(timeout, procserver_type);
                break;

            case TCP_I_SEG_FWD:

                msg_list = (cArray*)(msg->parList().get("msg_list"));
                for (i = 0; i < msg_list->items(); i++) {
                    if (msg_list->exist(i)) {
                        cMessage* tcp_send_msg = (cMessage*) ((cMessage*)(msg_list->get(i)))->dup();
                        tcp_send_msg->setKind(TCP_I_SEG_FWD);

                        tcp_conn_id = tcp_send_msg->par("tcp_conn_id");
                        mod = simulation.module(tcp_conn_id);
                        if (!mod)
                            error("no module found for TCP_I_SEG_FWD");
                        sendDirect(tcp_send_msg, 0.0, mod, "in");
                    }
                }
                delete msg;
                break;

            default:
                tcp_conn_id = msg->par("tcp_conn_id");
                mod = simulation.module(tcp_conn_id);
                if (!mod)
                {
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

void MyTCPServer::passiveOpen(double timeout, cModuleType* procserver_type)
{
    //server appl. calls
    cMessage *open_passive;

    int local_port = 1;
    int local_addr = IPAddress(par("local_addr").stringValue()).getInt();

    int rem_port = -1; //set to -1 since not yet specified (passive open)
    int rem_addr = -1; //set to -1 since not yet specified (passive open)

    open_passive = new cMessage("TCP_C_OPEN_PASSIVE", TCP_C_OPEN_PASSIVE);
    open_passive->addPar("src_port")  = local_port;
    open_passive->addPar("src_addr")  = local_addr;
    open_passive->addPar("dest_port") = rem_port;
    open_passive->addPar("dest_addr") = rem_addr;

    open_passive->addPar("timeout") = timeout; //global default is 5 min; here 5 sec

    open_passive->setLength(0);
    //no data packets to receive
    open_passive->addPar("rec_pks") = 0;

    //make delay checking possible
    open_passive->setTimestamp();

    //create module "MyTCPServerProc" dynamically within module "MyTCPServer"
    cModule* mod;

    mod = procserver_type->createScheduleInit("mytcpserverproc", this);

    mod->gate("out")->setTo(gate("out"));
    open_passive->addPar("tcp_conn_id") = mod->id();

    send(open_passive, "out");
}
