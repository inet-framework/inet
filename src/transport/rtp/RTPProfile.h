/***************************************************************************
                       RTPProfile.h  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef __INET_RTPPROFILE_H
#define __INET_RTPPROFILE_H


#include "INETDefs.h"


//Forward declarations:
class RTPInnerPacket;


/**
 * The class RTPProfile is a module which handles RTPPayloadSender and
 * RTPPayloadReceiver modules. It creates them dynamically on demand.
 * This class offers all functionality for the above tasks, subclasses
 * just need to set variables like profile name, rtcp percentage and
 * preferred port in their initialize() method.
 * The dynamically created sender and receiver modules must
 * have have following class names:
 * RTP<profileName>Payload<payloadType>Sender
 * RTP<profileName>Payload<payloadType>Receiver
 */
class INET_API RTPProfile : public cSimpleModule
{
  protected:
    // helper class to store the association between an ssrc identifier
    // and the gate which leads to the RTPPayloadReceiver module.
    // Note: in the original, this used to be a hundred lines, as RTPSSRCGate.cc/h,
    // but even this class is an overkill --Andras
    class SSRCGate : public cNamedObject  //FIXME why is it a namedObject?
    {
      protected:
        uint32 ssrc;
        int gateId;
      public:
        SSRCGate(uint32 ssrc = 0) {this->ssrc = ssrc; gateId = 0;}
        uint32 getSsrc() {return ssrc;}
        void setSSRC(uint32 ssrc) {this->ssrc = ssrc;}
        int getGateId() {return gateId;}
        void setGateId(int gateId) {this->gateId = gateId;}
    };

  public:
    RTPProfile();

  protected:
    /**
     * Initializes variables. Must be overwritten by subclasses.
     */
    virtual void initialize();

    virtual ~RTPProfile();

    /**
     * Creates and removes payload sender and receiver modules on demand.
     */
    virtual void handleMessage(cMessage *msg);

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
    virtual void initializeProfile(RTPInnerPacket *rinp);

    /**
     * This method is called when the application issued the creation
     * of an rtp payload sender module to transmit data.
     * It creates a new sender module and connects it. The profile
     * module informs the rtp module of having finished this task.
     * Then it initializes the newly create sender module with
     * a inititalizeSenderModule message.
     */
    virtual void createSenderModule(RTPInnerPacket *rinp);

    /**
     * When a sender module is no longer needed it can be deleted by the
     * profile module.
     */
    virtual void deleteSenderModule(RTPInnerPacket *rinp);

    /**
     * The profile module forwards sender control messages to the
     * sender module.
     */
    virtual void senderModuleControl(RTPInnerPacket *rinp);

    /**
     * Handles incoming data packets: If there isn't a receiver module
     * for this sender it creates one. The data packet is forwarded to
     * the receiver module after calling processIncomingPacket.
     */
    virtual void dataIn(RTPInnerPacket *rinp);

    /**
     * The sender module returns a senderModuleInitialized message after
     * being initialized. The profile module forwards this message to
     * the rtp module which delivers it to its destination, the rtcp
     * module.
     */
    virtual void senderModuleInitialized(RTPInnerPacket *rinp);

    /**
     * After having received a sender module control message the sender
     * module returns a sender status message to inform the application
     * what it's doing at the moment.
     */
    virtual void senderModuleStatus(RTPInnerPacket *rinp);

    /**
     * Handles outgoing data packets: Calls processOutgoingPacket and
     * forwards the packet to the rtp module.
     */
    virtual void dataOut(RTPInnerPacket *rinp);

    /**
     * Every time a rtp packet is received it it pre-processed by this
     * method to remove profile specific extension which are not handled
     * by the payload receiver module. In this implementation the packet
     * isn't changed.
     * Important: This method works with RTPInnerPacket. So the rtp
     * packet must be decapsulated, changed and encapsulated again.
     */
    virtual void processIncomingPacket(RTPInnerPacket *rinp);

    /**
     * Simular to the procedure for incoming packets, this adds profile
     * specific extensions to outgoing rtp packets.
     */
    virtual void processOutgoingPacket(RTPInnerPacket *rinp);

    /**
     * Finds the gate of the receiver module for rtp data
     * packets from this ssrc.
     */
    virtual SSRCGate *findSSRCGate(uint32 ssrc);

    /**
     * Creates a new association ssrc/gateId for this ssrc.
     */
    virtual SSRCGate *newSSRCGate(uint32 ssrc);

    /**
     * The name of this profile. Needed for dynamic creating
     * of sender and receiver modules.
     */
    const char *_profileName;

    /**
     * The maximum number of incoming data streams this profile
     * module can handle. It is set to the gate size of
     * "payloadReceiverOut", "payloadReceiverIn".
     */
    int _maxReceivers;

    /**
     * Stores information to which gate rtp data packets
     * from a ssrc must be forwarded.
     */
    typedef std::map<uint32, SSRCGate *> SSRCGateMap;
    SSRCGateMap _ssrcGates;

    /**
     * The percentage of the available bandwidth to be used for rtcp.
     */
    int _rtcpPercentage;

    /**
     * The rtp port this profile uses if no port is given.
     */
    int _preferredPort;

    /**
     * The maximum size an RTPPacket can have.
     */
    int _mtu;

    /**
     * If this is set true the RTPProfile automatically sets the output
     * file name for payload receiver modules so the user is not bothered
     * to set them manually during simulation runtime.
     */
    bool _autoOutputFileNames;
};

#endif
