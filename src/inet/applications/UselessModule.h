#ifndef __INET_USELESSUDPAPP_H
#define __INET_USELESSUDPAPP_H

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UselessUdpApp : public ApplicationBase //, public UdpSocket::ICallback
{
//  protected:
//    enum SelfMsgKinds { START = 1, SEND, STOP };
//
//    // parameters
//    std::vector<L3Address> destAddresses;
//    std::vector<std::string> destAddressStr;
//    int localPort = -1, destPort = -1;
//    simtime_t startTime;
//    simtime_t stopTime;
//    bool dontFragment = false;
//    const char *packetName = nullptr;
//
//    // state
//    UdpSocket socket;
    cMessage *selfMsg = nullptr;

//    // statistics
//    int numSent = 0;
//    int numReceived = 0;

  protected:
//    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *message);
//    virtual void handleMessageWhenUp(cMessage *msg) override;
//    virtual void finish() override;
//    virtual void refreshDisplay() const override;

//    // chooses random destination address
//    virtual L3Address chooseDestAddr();
//    virtual void sendPacket();
//    virtual void processPacket(Packet *msg);
//    virtual void setSocketOptions();
//
//    virtual void processStart();
//    virtual void processSend();
//    virtual void processStop();
//
//    virtual void handleStartOperation(LifecycleOperation *operation) override;
//    virtual void handleStopOperation(LifecycleOperation *operation) override;
//    virtual void handleCrashOperation(LifecycleOperation *operation) override;
//
//    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
//    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
//    virtual void socketClosed(UdpSocket *socket) override;

  public:
    UselessUdpApp() {}
    ~UselessUdpApp();
};

} // namespace inet

#endif // ifndef __INET_USELESSUDPAPP_H
