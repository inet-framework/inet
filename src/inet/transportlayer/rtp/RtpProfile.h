//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi <ahmed.ayadi@sophia.inria.fr>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_RTPPROFILE_H
#define __INET_RTPPROFILE_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace rtp {

// Forward declarations:
class RtpInnerPacket;

/**
 * The class RtpProfile is a module which handles RtpPayloadSender and
 * RtpPayloadReceiver modules. It creates them dynamically on demand.
 * This class offers all functionality for the above tasks, subclasses
 * just need to set variables like profile name, rtcp percentage and
 * preferred port in their initialize() method.
 * The dynamically created sender and receiver modules must
 * have have following class names:
 * Rtp<profileName>Payload<payloadType>Sender
 * Rtp<profileName>Payload<payloadType>Receiver
 */
class INET_API RtpProfile : public cSimpleModule
{
  protected:
    // helper class to store the association between an ssrc identifier
    // and the gate which leads to the RtpPayloadReceiver module.
    // Note: in the original, this used to be a hundred lines, as RTPSSRCGate.cc/h,
    // but even this class is an overkill --Andras
    class INET_API SsrcGate : public cNamedObject { // FIXME why is it a namedObject?
      protected:
        uint32_t ssrc;
        int gateId;

      public:
        SsrcGate(uint32_t ssrc = 0) { this->ssrc = ssrc; gateId = 0; }
        uint32_t getSsrc() { return ssrc; }
        void setSSRC(uint32_t ssrc) { this->ssrc = ssrc; }
        int getGateId() { return gateId; }
        void setGateId(int gateId) { this->gateId = gateId; }
    };

  public:
    RtpProfile();

  protected:
    /**
     * Initializes variables. Must be overwritten by subclasses.
     */
    virtual void initialize() override;

    virtual ~RtpProfile();

    /**
     * Creates and removes payload sender and receiver modules on demand.
     */
    virtual void handleMessage(cMessage *msg) override;

    /**
     * Handles messages received from the rtp module.
     */
    virtual void handleMessageFromRTP(cMessage *msg);

    /**
     * Handles messages coming from the sender module.
     */
    virtual void handleMessageFromPayloadSender(cMessage *msg);

    /**
     * Handles messages coming from a receiver module.
     */
    virtual void handleMessageFromPayloadReceiver(cMessage *msg);

    /**
     * Initialization message received from rtp module.
     */
    virtual void initializeProfile(RtpInnerPacket *rinp);

    /**
     * This method is called when the application issued the creation
     * of an rtp payload sender module to transmit data.
     * It creates a new sender module and connects it. The profile
     * module informs the rtp module of having finished this task.
     * Then it initializes the newly create sender module with
     * a inititalizeSenderModule message.
     */
    virtual void createSenderModule(RtpInnerPacket *rinp);

    /**
     * When a sender module is no longer needed it can be deleted by the
     * profile module.
     */
    virtual void deleteSenderModule(RtpInnerPacket *rinp);

    /**
     * The profile module forwards sender control messages to the
     * sender module.
     */
    virtual void senderModuleControl(RtpInnerPacket *rinp);

    /**
     * Handles incoming data packets: If there isn't a receiver module
     * for this sender it creates one. The data packet is forwarded to
     * the receiver module after calling processIncomingPacket.
     */
    virtual void dataIn(RtpInnerPacket *rinp);

    /**
     * The sender module returns a senderModuleInitialized message after
     * being initialized. The profile module forwards this message to
     * the rtp module which delivers it to its destination, the rtcp
     * module.
     */
    virtual void senderModuleInitialized(RtpInnerPacket *rinp);

    /**
     * After having received a sender module control message the sender
     * module returns a sender status message to inform the application
     * what it's doing at the moment.
     */
    virtual void senderModuleStatus(RtpInnerPacket *rinp);

    /**
     * Handles outgoing data packets: Calls processOutgoingPacket and
     * forwards the packet to the rtp module.
     */
    virtual void dataOut(RtpInnerPacket *rinp);

    /**
     * Every time a rtp packet is received it it pre-processed by this
     * method to remove profile specific extension which are not handled
     * by the payload receiver module. In this implementation the packet
     * isn't changed.
     * Important: This method works with RtpInnerPacket. So the rtp
     * packet must be decapsulated, changed and encapsulated again.
     */
    virtual void processIncomingPacket(RtpInnerPacket *rinp);

    /**
     * Simular to the procedure for incoming packets, this adds profile
     * specific extensions to outgoing rtp packets.
     */
    virtual void processOutgoingPacket(RtpInnerPacket *rinp);

    /**
     * Finds the gate of the receiver module for rtp data
     * packets from this ssrc.
     */
    virtual SsrcGate *findSSRCGate(uint32_t ssrc);

    /**
     * Creates a new association ssrc/gateId for this ssrc.
     */
    virtual SsrcGate *newSSRCGate(uint32_t ssrc);

    /**
     * The name of this profile. Needed for dynamic creating
     * of sender and receiver modules.
     */
    const char *_profileName = nullptr;

    /**
     * The maximum number of incoming data streams this profile
     * module can handle. It is set to the gate size of
     * "payloadReceiverOut", "payloadReceiverIn".
     */
    int _maxReceivers = 0;

    /**
     * Stores information to which gate rtp data packets
     * from a ssrc must be forwarded.
     */
    typedef std::map<uint32_t, SsrcGate *> SsrcGateMap;
    SsrcGateMap _ssrcGates;

    /**
     * The percentage of the available bandwidth to be used for rtcp.
     */
    int _rtcpPercentage = 0;

    /**
     * The rtp port this profile uses if no port is given.
     */
    int _preferredPort = -1;

    /**
     * The maximum size an RtpPacket can have.
     */
    int _mtu = 0;

    /**
     * If this is set true the RtpProfile automatically sets the output
     * file name for payload receiver modules so the user is not bothered
     * to set them manually during simulation runtime.
     */
    bool _autoOutputFileNames = false;
};

} // namespace rtp

} // namespace inet

#endif

