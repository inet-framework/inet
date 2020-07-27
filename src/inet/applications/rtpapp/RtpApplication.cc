//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
// Copyright (C) 2010 Zoltan Bojthe
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

#include "inet/applications/rtpapp/RtpApplication.h"
#include "inet/applications/rtpapp/RtpApplication_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/rtp/RtpInterfacePacket_m.h"

namespace inet {

Define_Module(RtpApplication)

void RtpApplication::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    // because of L3AddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage == INITSTAGE_LOCAL) {
        // the common name (CNAME) of this host
        commonName = par("commonName");

        // which rtp profile is to be used (usually RtpAvProfile)
        profileName = par("profileName");

        // bandwidth in bytes per second for this session
        bandwidth = par("bandwidth");

        // port number which is to be used; to ports are actually used: one
        // for rtp and one for rtcp
        port = par("portNumber");

        // fileName of file to be transmitted
        // nullptr or "" means this system acts only as a receiver
        fileName = par("fileName");

        // payload type of file to transmit
        payloadType = par("payloadType");

        sessionEnterDelay = par("sessionEnterDelay");
        transmissionStartDelay = par("transmissionStartDelay");
        transmissionStopDelay = par("transmissionStopDelay");
        sessionLeaveDelay = par("sessionLeaveDelay");

        ssrc = 0;
        isActiveSession = false;
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        // the ip address to connect to (unicast or multicast)
        destinationAddress = L3AddressResolver().resolve(par("destinationAddress")).toIpv4();

        EV_DETAIL << "commonName" << commonName << endl
                  << "profileName" << profileName << endl
                  << "bandwidth" << bandwidth << endl
                  << "destinationAddress" << destinationAddress << endl
                  << "portNumber" << port << endl
                  << "fileName" << fileName << endl
                  << "payloadType" << payloadType << endl;

        cMessage *selfMsg = new cMessage("enterSession", RTPAPP_ENTER_SESSION);
        scheduleAfter(sessionEnterDelay, selfMsg);
    }
}

void RtpApplication::handleMessage(cMessage *msgIn)
{
    using namespace rtp;

    if (msgIn->isSelfMessage()) {
        switch (msgIn->getKind()) {
            case RTPAPP_ENTER_SESSION:
                EV_INFO << "enterSession" << endl;
                if (isActiveSession) {
                    EV_WARN << "Session already entered\n";
                }
                else {
                    isActiveSession = true;
                    RtpCiEnterSession *ci = new RtpCiEnterSession();
                    ci->setCommonName(commonName);
                    ci->setProfileName(profileName);
                    ci->setBandwidth(bandwidth);
                    ci->setDestinationAddress(destinationAddress);
                    ci->setPort(port);
                    cMessage *msg = new RtpControlMsg("Enter Session");
                    msg->setControlInfo(ci);
                    send(msg, "rtpOut");
                }
                break;

            case RTPAPP_START_TRANSMISSION:
                EV_INFO << "startTransmission" << endl;
                if (!isActiveSession) {
                    EV_WARN << "Session already left\n";
                }
                else {
                    RtpCiSenderControl *ci = new RtpCiSenderControl();
                    ci->setCommand(RTP_CONTROL_PLAY);
                    ci->setSsrc(ssrc);
                    cMessage *msg = new RtpControlMsg("senderModuleControl(PLAY)");
                    msg->setControlInfo(ci);
                    send(msg, "rtpOut");

                    cMessage *selfMsg = new cMessage("stopTransmission", RTPAPP_STOP_TRANSMISSION);
                    scheduleAfter(transmissionStopDelay, selfMsg);
                }
                break;

            case RTPAPP_STOP_TRANSMISSION:
                EV_INFO << "stopTransmission" << endl;
                if (!isActiveSession) {
                    EV_WARN << "Session already left\n";
                }
                else {
                    RtpCiSenderControl *ci = new RtpCiSenderControl();
                    ci->setCommand(RTP_CONTROL_STOP);
                    ci->setSsrc(ssrc);
                    cMessage *msg = new RtpControlMsg("senderModuleControl(STOP)");
                    msg->setControlInfo(ci);
                    send(msg, "rtpOut");
                }
                break;

            case RTPAPP_LEAVE_SESSION:
                EV_INFO << "leaveSession" << endl;
                if (!isActiveSession) {
                    EV_WARN << "Session already left\n";
                }
                else {
                    RtpCiLeaveSession *ci = new RtpCiLeaveSession();
                    cMessage *msg = new RtpControlMsg("Leave Session");
                    msg->setControlInfo(ci);
                    send(msg, "rtpOut");
                }
                break;

            default:
                throw cRuntimeError("Invalid msgKind value %d in message '%s'",
                    msgIn->getKind(), msgIn->getName());
        }
    }
    else if (isActiveSession) {
        cObject *obj = msgIn->removeControlInfo();
        RtpControlInfo *ci = dynamic_cast<RtpControlInfo *>(obj);
        if (ci) {
            switch (ci->getType()) {
                case RTP_IFP_SESSION_ENTERED: {
                    EV_INFO << "Session Entered" << endl;
                    ssrc = (check_and_cast<RtpCiSessionEntered *>(ci))->getSsrc();
                    if (opp_strcmp(fileName, "")) {
                        EV_INFO << "CreateSenderModule" << endl;
                        RtpCiCreateSenderModule *ci = new RtpCiCreateSenderModule();
                        ci->setSsrc(ssrc);
                        ci->setPayloadType(payloadType);
                        ci->setFileName(fileName);
                        cMessage *msg = new RtpControlMsg("createSenderModule()");
                        msg->setControlInfo(ci);
                        send(msg, "rtpOut");
                    }
                    else {
                        cMessage *selfMsg = new cMessage("leaveSession", RTPAPP_LEAVE_SESSION);
                        EV_INFO << "Receiver Module : leaveSession" << endl;
                        scheduleAfter(sessionLeaveDelay, selfMsg);
                    }
                }
                break;

                case RTP_IFP_SENDER_MODULE_CREATED: {
                    EV_INFO << "Sender Module Created" << endl;
                    cMessage *selfMsg = new cMessage("startTransmission", RTPAPP_START_TRANSMISSION);
                    scheduleAfter(transmissionStartDelay, selfMsg);
                }
                break;

                case RTP_IFP_SENDER_STATUS: {
                    cMessage *selfMsg;
                    RtpCiSenderStatus *rsim = check_and_cast<RtpCiSenderStatus *>(ci);
                    switch (rsim->getStatus()) {
                        case RTP_SENDER_STATUS_PLAYING:
                            EV_INFO << "PLAYING" << endl;
                            break;

                        case RTP_SENDER_STATUS_FINISHED:
                            EV_INFO << "FINISHED" << endl;
                            selfMsg = new cMessage("leaveSession", RTPAPP_LEAVE_SESSION);
                            scheduleAfter(sessionLeaveDelay, selfMsg);
                            break;

                        case RTP_SENDER_STATUS_STOPPED:
                            EV_INFO << "STOPPED" << endl;
                            selfMsg = new cMessage("leaveSession", RTPAPP_LEAVE_SESSION);
                            scheduleAfter(sessionLeaveDelay, selfMsg);
                            break;

                        default:
                            throw cRuntimeError("Invalid sender status: %d", rsim->getStatus());
                    }
                }
                break;

                case RTP_IFP_SESSION_LEFT:
                    if (!isActiveSession)
                        EV_WARN << "Session already left\n";
                    else
                        isActiveSession = false;
                    break;

                case RTP_IFP_SENDER_MODULE_DELETED:
                    EV_INFO << "Sender Module Deleted" << endl;
                    break;

                default:
                    break;
            }
        }
        delete obj;
    }
    delete msgIn;
}

} // namespace inet

