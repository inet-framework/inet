/**
 ******************************************************
 * @file EthernetApplication.h
 * @brief Simple traffic generator.
 * It generates Etherapp requests and responses. Based in EtherAppCli and EtherAppSrv.
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
#include "inet/linklayer/common/MACAddress.h"

namespace inet {

#define MAX_REPLY_CHUNK_SIZE    1497

/**
 * Ethernet application. Both, server and client side.
 */
class INET_API EthernetApplication : public cSimpleModule
{
  protected:
    // send parameters
    long seqNum;
    cPar *reqLength;
    cPar *respLength;
    cPar *waitTime;

    MACAddress destMACAddress;

    // Reception statistics
    long packetsSent;
    long packetsReceived;

    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

  protected:

    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    /*
     *	Gets the MAC address in case a host name was set in destAddress parameter
     */
    virtual MACAddress resolveDestMACAddress();

    /*
     * sendPacket function for the server side
     */
    virtual void sendPacket();

    /*
     * sendPacket function for the client side
     */
    void sendPacket(cMessage *datapacket, const MACAddress& destAddr);

    /*
     * generates response packet. Server side.
     */
    virtual void receivePacket(cMessage *msg);
};

} // namespace inet

#endif // ifndef __INET_ETHERNETAPPLICATION_H

