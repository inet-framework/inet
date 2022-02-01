//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi <ahmed.ayadi@sophia.inria.fr>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RTP_H
#define __INET_RTP_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"
#include "inet/transportlayer/rtp/RtpInterfacePacket_m.h"

namespace inet {
namespace rtp {

/**
 * An Rtp is the center of the Rtp layer of an endsystem.
 * It creates the profile module, sends and receives Rtp data packets
 * and forwards messages.
 * It also communicates with the application.
 */
class INET_API Rtp : public cSimpleModule, public LifecycleUnsupported
{
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
    virtual void enterSession(RtpCiEnterSession *rifp);

    /**
     * Destroys the profile module and orders the rtcp module
     * to send an rtcp bye packet.
     */
    virtual void leaveSession(RtpCiLeaveSession *rifp);

    virtual void createSenderModule(RtpCiCreateSenderModule *rifp);

    virtual void deleteSenderModule(RtpCiDeleteSenderModule *rifp);

    virtual void senderModuleControl(RtpCiSenderControl *rifp);

    /**
     * Called when the profile module is initialized.
     */
    virtual void profileInitialized(RtpInnerPacket *rinp);

    virtual void senderModuleCreated(RtpInnerPacket *rinp);

    virtual void senderModuleDeleted(RtpInnerPacket *rinp);

    virtual void senderModuleInitialized(RtpInnerPacket *rinp);

    virtual void senderModuleStatus(RtpInnerPacket *rinp);

    /**
     * Sends a RTP data packet to the UDP layer and a copy
     * of it to the rtcp module.
     */
    virtual void dataOut(RtpInnerPacket *rinp);

    /**
     * Informs the application that the session is entered.
     */
    virtual void rtcpInitialized(RtpInnerPacket *rinp);

    /**
     * Informs the application that this end system
     * has left the RTP session.
     */
    virtual void sessionLeft(RtpInnerPacket *rinp);

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
    std::string _commonName;

    /**
     * The available bandwidth for this session.
     */
    int _bandwidth;

    /**
     * The destination address.
     */
    Ipv4Address _destinationAddress;

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
    UdpSocket _udpSocket;

    /**
     * True when this end system is about to leave the session.
     */
    bool _leaveSession;

    int appInGate, profileInGate, rtcpInGate, udpInGate;
};

} // namespace rtp
} // namespace inet

#endif

