//
// Copyright (C) 2006 Sam Jansen, Andras Varga,
//               2009 Zoltan Bojthe
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
//

#ifndef __INET_TCP_NSC_H
#define __INET_TCP_NSC_H

#ifndef HAVE_NSC
#error Please install NSC or disable 'TCP_NSC' feature
#endif // ifndef HAVE_NSC

#include <map>

#include "inet/common/INETDefs.h"

#include <sim_interface.h>    // NSC. We need this here to derive from classes

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "inet/transportlayer/tcp_nsc/TCP_NSC_Connection.h"

namespace inet {

// forward declarations:
class TCPCommand;

namespace tcp {

// forward declarations:
class TCPSegment;
class TCP_NSC_SendQueue;
class TCP_NSC_ReceiveQueue;

/**
 * Encapsulates a Network Simulation Cradle (NSC) instance.
 */
class INET_API TCP_NSC : public cSimpleModule, ISendCallback, IInterruptCallback, public ILifecycle
{
  protected:
    enum { MAX_SEND_BYTES = 500000 };

  public:
    TCP_NSC();
    virtual ~TCP_NSC();

    // Implement NSC callbacks:

    //   Implement ISendCallback:
    virtual void send_callback(const void *, int) override;

    //   Implement IInterruptCallback:
    virtual void wakeup() override;
    virtual void gettime(unsigned int *, unsigned int *) override;

  protected:
    // called by the OMNeT++ simulation kernel:

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msgP) override;
    virtual void finish() override;

    // internal utility functions:

    void changeAddresses(TCP_NSC_Connection& connP,
            const TCP_NSC_Connection::SockPair& inetSockPairP,
            const TCP_NSC_Connection::SockPair& nscSockPairP);

    // find a TCP_NSC_Connection by connection ID
    TCP_NSC_Connection *findAppConn(int connIdP);

    // find a TCP_NSC_Connection by inet sockpair
    TCP_NSC_Connection *findConnByInetSockPair(TCP_NSC_Connection::SockPair const& sockPairP);

    // find a TCP_NSC_Connection by nsc sockpair
    TCP_NSC_Connection *findConnByNscSockPair(TCP_NSC_Connection::SockPair const& sockPairP);

    virtual void updateDisplayString();
    void removeConnection(int connIdP);
    void printConnBrief(TCP_NSC_Connection& connP);
    void loadStack(const char *stacknameP, int bufferSizeP);

    void handleAppMessage(cMessage *msgP);
    void handleIpInputMessage(TCPSegment *tcpsegP);

    void sendDataToApp(TCP_NSC_Connection& c);
    void sendErrorNotificationToApp(TCP_NSC_Connection& c, int err);

    // function to be called back from the NSC stack:

    void sendToIP(const void *dataP, int lenP);

    // to be refined...

    void processAppCommand(TCP_NSC_Connection& connP, cMessage *msgP);

    // to be refined and filled in with calls into the NSC stack

    void process_OPEN_ACTIVE(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP);
    void process_OPEN_PASSIVE(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP);
    void process_SEND(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cPacket *msgP);
    void process_CLOSE(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP);
    void process_ABORT(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP);
    void process_STATUS(TCP_NSC_Connection& connP, TCPCommand *tcpCommandP, cMessage *msgP);

    void do_SEND(TCP_NSC_Connection& connP);
    void do_SEND_all();

    // return mapped remote IP in host byte order
    // if addrP not exists in map, it's create a new nsc addr, and insert it to map
    u_int32_t mapRemote2Nsc(L3Address const& addrP);

    // return original remote ip from mapped ip
    // assert if not exists in map
    // nscAddrP has IP in host byte order
    // x == mapNsc2Remote(mapRemote2Nsc(x))
    L3Address const& mapNsc2Remote(u_int32_t nscAddrP);

    // send a connection established msg to application layer
    void sendEstablishedMsg(TCP_NSC_Connection& connP);

    /**
     * To be called from TCPConnection: create a new send queue.
     */
    virtual TCP_NSC_SendQueue *createSendQueue(TCPDataTransferMode transferModeP);

    /**
     * To be called from TCPConnection: create a new receive queue.
     */
    virtual TCP_NSC_ReceiveQueue *createReceiveQueue(TCPDataTransferMode transferModeP);

    // ILifeCycle:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  protected:
    typedef std::map<int, TCP_NSC_Connection> TcpAppConnMap;    // connId-to-TCP_NSC_Connection
    typedef std::map<u_int32_t, L3Address> Nsc2RemoteMap;
    typedef std::map<L3Address, u_int32_t> Remote2NscMap;
    typedef std::map<TCP_NSC_Connection::SockPair, int> SockPair2ConnIdMap;

    // Maps:
    TcpAppConnMap tcpAppConnMapM;
    SockPair2ConnIdMap inetSockPair2ConnIdMapM;
    SockPair2ConnIdMap nscSockPair2ConnIdMapM;

    Nsc2RemoteMap nsc2RemoteMapM;
    Remote2NscMap remote2NscMapM;

    INetStack *pStackM;

    cMessage *pNsiTimerM;

  protected:
    bool isAliveM;    // true when I between initialize() and finish()

    int curAddrCounterM;    // incr, when set curLocalAddr, decr when "felhasznaltam"
    TCP_NSC_Connection *curConnM;    // store current connection in connect/listen command

    static const L3Address localInnerIpS;    // local NSC IP addr
    static const L3Address localInnerGwS;    // local NSC gateway IP addr
    static const L3Address localInnerMaskS;    // local NSC Network Mask
    static const L3Address remoteFirstInnerIpS;    // first remote NSC IP addr

    static const char *stackNameParamNameS;    // name of stackname parameter
    static const char *bufferSizeParamNameS;    // name of buffersize parameter

    // statistics
    cOutVector *sndNxtVector;    // sent seqNo
    cOutVector *sndAckVector;    // sent ackNo
    cOutVector *rcvSeqVector;    // received seqNo
    cOutVector *rcvAckVector;    // received ackNo (= snd_una)
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCP_NSC_H

