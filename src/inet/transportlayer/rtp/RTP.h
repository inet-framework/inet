/***************************************************************************
                       RTP.h  -  description
                             -------------------
    begin            : Fri Aug 2 2007
    copyright        : (C) 2007 by Matthias Oppitz,  Ahmed Ayadi
    email            : <Matthias.Oppitz@gmx.de> <ahmed.ayadi@sophia.inria.fr>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef __INET_RTP_H
#define __INET_RTP_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/rtp/RTPInnerPacket.h"
#include "inet/transportlayer/rtp/RTPInterfacePacket_m.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

namespace rtp {

/**
 * An RTP is the center of the RTP layer of an endsystem.
 * It creates the profile module, sends and receives RTP data packets
 * and forwards messages.
 * It also communicates with the application.
 */
class INET_API RTP : public cSimpleModule, public ILifecycle
{
  public:
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

  protected:
    /**
     * Initializes variables.
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /**
     * Handles incoming messages.
     */
    virtual void handleMessage(cMessage *msg) override;

    /**
     * Handles messages received from the applicaiton.
     */
    virtual void handleMessageFromApp(cMessage *msg);

    /**
     * Handles messages received from the profile module.
     */
    virtual void handleMessageFromProfile(cMessage *msg);

    /**
     * Handles messages received from the rtcp module.
     */
    virtual void handleMessageFromRTCP(cMessage *msg);

    /**
     * Handles messages received from the UDP layer.
     */
    virtual void handleMessagefromUDP(cMessage *msg);

    /**
     * Creates the profile module and initializes it.
     */
    virtual void enterSession(RTPCIEnterSession *rifp);

    /**
     * Destroys the profile module and orders the rtcp module
     * to send an rtcp bye packet.
     */
    virtual void leaveSession(RTPCILeaveSession *rifp);

    virtual void createSenderModule(RTPCICreateSenderModule *rifp);

    virtual void deleteSenderModule(RTPCIDeleteSenderModule *rifp);

    virtual void senderModuleControl(RTPCISenderControl *rifp);

    /**
     * Called when the profile module is initialized.
     */
    virtual void profileInitialized(RTPInnerPacket *rinp);

    virtual void senderModuleCreated(RTPInnerPacket *rinp);

    virtual void senderModuleDeleted(RTPInnerPacket *rinp);

    virtual void senderModuleInitialized(RTPInnerPacket *rinp);

    virtual void senderModuleStatus(RTPInnerPacket *rinp);

    /**
     * Sends a RTP data packet to the UDP layer and a copy
     * of it to the rtcp module.
     */
    virtual void dataOut(RTPInnerPacket *rinp);

    /**
     * Informs the application that the session is entered.
     */
    virtual void rtcpInitialized(RTPInnerPacket *rinp);

    /**
     * Informs the application that this end system
     * has left the RTP session.
     */
    virtual void sessionLeft(RTPInnerPacket *rinp);

    /**
     * Creates the profile module.
     */
    virtual void createProfile(const char *profileName);

    /**
     * Requests a server socket from the UDP layer.
     */
    virtual void createSocket();

    /**
     * Called when the socket layer returns a socket.
     */
    virtual void socketRet();

    /**
     * Called when the socket layer has connected a socket.
     */
    virtual void connectRet();

    /**
     * Called when data from the socket layer has been received.
     */
    virtual void readRet(cMessage *sifp);

    /**
     * Initializes the profile module.
     */
    virtual void initializeProfile();

    /**
     * Initializes the rtcp module-.
     */
    virtual void initializeRTCP();

    /**
       Determines the maximum transmission unit that can be uses for
       RTP. This implementation assumes that we use an ethernet with
       1500 bytes mtu. The returned value is 1500 bytes minus header
       sizes for ip and udp.
     */
    virtual int resolveMTU();

  protected:
    /**
     * The CNAME of this end system.
     */
    const char *_commonName;

    /**
     * The available bandwidth for this session.
     */
    int _bandwidth;

    /**
     * The destination address.
     */
    IPv4Address _destinationAddress;

    /**
     * The RTP port.
     */
    int _port;

    /**
     * The maximum size of a packet.
     */
    int _mtu;

    /**
     * The percentage of the bandwidth used for rtcp.
     */
    int _rtcpPercentage;

    /**
     * The UDP socket.
     */
    UDPSocket _udpSocket;

    /**
     * True when this end system is about to leave the session.
     */
    bool _leaveSession;

    int appInGate, profileInGate, rtcpInGate, udpInGate;

    //statistics:
    static simsignal_t rcvdPkSignal;
};

} // namespace rtp

} // namespace inet

#endif // ifndef __INET_RTP_H

