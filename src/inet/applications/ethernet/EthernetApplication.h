/**
 ******************************************************
 * @file EthernetApplication.h
 * @brief Simple traffic generator.
 * It generates Etherapp requests and responses. Based in EtherAppClient and EtherAppServer.
 *
 * @author Juan Luis Garrote Molinero
 * @version 1.0
 * @date Feb 2011
 *
 *
 ******************************************************/
#ifndef __INET_ETHERNETAPPLICATION_H
#define __INET_ETHERNETAPPLICATION_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"

namespace inet {

#define MAX_REPLY_CHUNK_SIZE    1497

/**
 * Ethernet application. Both, server and client side.
 */
class INET_API EthernetApplication : public cSimpleModule
{
  protected:
    // send parameters
    long seqNum = 0;
    cPar *reqLength = nullptr;
    cPar *respLength = nullptr;
    cPar *waitTime = nullptr;

    MacAddress destMACAddress;

    // Reception statistics
    long packetsSent = 0;
    long packetsReceived = 0;

  protected:

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    /*
     *	Gets the MAC address in case a host name was set in destAddress parameter
     */
    virtual MacAddress resolveDestMACAddress();

    /*
     * sendPacket function for the server side
     */
    virtual void sendPacket();

    /*
     * sendPacket function for the client side
     */
    void sendPacket(Packet *datapacket, const MacAddress& destAddr);

    /*
     * generates response packet. Server side.
     */
    virtual void receivePacket(cMessage *msg);
};

} // namespace inet

#endif // ifndef __INET_ETHERNETAPPLICATION_H

