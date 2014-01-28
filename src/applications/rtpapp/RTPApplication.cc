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

#include "RTPApplication.h"

#include "AddressResolver.h"
#include "LifecycleOperation.h"
#include "ModuleAccess.h"
#include "NodeStatus.h"
#include "RTPInterfacePacket_m.h"

Define_Module(RTPApplication)

RTPApplication::RTPApplication()
{
    fileName = NULL;
    commonName = NULL;
    profileName = NULL;
}

void RTPApplication::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    // because of AddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage == INITSTAGE_LOCAL)
    {
        // the common name (CNAME) of this host
        commonName = par("commonName");

        // which rtp profile is to be used (usually RTPAVProfile)
        profileName = par("profileName");

        // bandwidth in bytes per second for this session
        bandwidth = par("bandwidth");

        // port number which is to be used; to ports are actually used: one
        // for rtp and one for rtcp
        port = (int)par("portNumber").longValue();

        // fileName of file to be transmitted
        // NULL or "" means this system acts only as a receiver
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
    else if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");

        // the ip address to connect to (unicast or multicast)
        destinationAddress = AddressResolver().resolve(par("destinationAddress").stringValue()).toIPv4();

        EV << "commonName" << commonName << endl;
        EV << "profileName" << profileName << endl;
        EV << "bandwidth" << bandwidth << endl;
        EV << "destinationAddress" << destinationAddress << endl;
        EV << "portNumber" << port << endl;
        EV << "fileName" << fileName << endl;
        EV << "payloadType" << payloadType << endl;

        cMessage *selfMsg = new cMessage("enterSession", ENTER_SESSION);
        scheduleAt(simTime() + sessionEnterDelay, selfMsg);
    }
}

void RTPApplication::handleMessage(cMessage* msgIn)
{
    if (msgIn->isSelfMessage())
    {
        switch (msgIn->getKind())
        {
        case ENTER_SESSION:
            EV << "enterSession" << endl;
            if (isActiveSession)
                EV << "Session already entered\n";
            else
            {
                isActiveSession = true;
                RTPCIEnterSession* ci = new RTPCIEnterSession();
                ci->setCommonName(commonName);
                ci->setProfileName(profileName);
                ci->setBandwidth(bandwidth);
                ci->setDestinationAddress(destinationAddress);
                ci->setPort(port);
                cMessage *msg = new RTPControlMsg("Enter Session");
                msg->setControlInfo(ci);
                send(msg, "rtpOut");
            }
            break;

        case START_TRANSMISSION:
            EV << "startTransmission" << endl;
            if (!isActiveSession)
            {
                EV << "Session already left\n";
            }
            else
            {
                RTPCISenderControl *ci = new RTPCISenderControl();
                ci->setCommand(RTP_CONTROL_PLAY);
                ci->setSsrc(ssrc);
                cMessage *msg = new RTPControlMsg("senderModuleControl(PLAY)");
                msg->setControlInfo(ci);
                send(msg, "rtpOut");

                cMessage *selfMsg = new cMessage("stopTransmission", STOP_TRANSMISSION);
                scheduleAt(simTime() + transmissionStopDelay, selfMsg);
            }
            break;

        case STOP_TRANSMISSION:
            EV << "stopTransmission" << endl;
            if (!isActiveSession)
                EV << "Session already left\n";
            else
            {
                RTPCISenderControl *ci = new RTPCISenderControl();
                ci->setCommand(RTP_CONTROL_STOP);
                ci->setSsrc(ssrc);
                cMessage *msg = new RTPControlMsg("senderModuleControl(STOP)");
                msg->setControlInfo(ci);
                send(msg, "rtpOut");
            }
            break;

        case LEAVE_SESSION:
            EV << "leaveSession" << endl;
            if (!isActiveSession)
                EV << "Session already left\n";
            else
            {
                RTPCILeaveSession* ci = new RTPCILeaveSession();
                cMessage *msg = new RTPControlMsg("Leave Session");
                msg->setControlInfo(ci);
                send(msg, "rtpOut");
            }
            break;

        default:
            throw cRuntimeError("Invalid msgKind value %d in message '%s'",
                    msgIn->getKind(), msgIn->getName());
        }
    }
    else if (isActiveSession)
    {
        cObject *obj = msgIn->removeControlInfo();
        RTPControlInfo *ci = dynamic_cast<RTPControlInfo *>(obj);
        if (ci)
        {
            switch (ci->getType())
            {
            case RTP_IFP_SESSION_ENTERED:
                {
                    EV << "Session Entered" << endl;
                    ssrc = (check_and_cast<RTPCISessionEntered *>(ci))->getSsrc();
                    if (opp_strcmp(fileName, ""))
                    {
                        EV << "CreateSenderModule" << endl;
                        RTPCICreateSenderModule* ci = new RTPCICreateSenderModule();
                        ci->setSsrc(ssrc);
                        ci->setPayloadType(payloadType);
                        ci->setFileName(fileName);
                        cMessage *msg = new RTPControlMsg("createSenderModule()");
                        msg->setControlInfo(ci);
                        send(msg, "rtpOut");
                    }
                    else
                    {
                        cMessage *selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                        EV << "Receiver Module : leaveSession" << endl;
                        scheduleAt(simTime() + sessionLeaveDelay, selfMsg);
                    }
                }
                break;

            case RTP_IFP_SENDER_MODULE_CREATED:
                {
                    EV << "Sender Module Created" << endl;
                    cMessage *selfMsg = new cMessage("startTransmission", START_TRANSMISSION);
                    scheduleAt(simTime() + transmissionStartDelay, selfMsg);
                }
                break;

            case RTP_IFP_SENDER_STATUS:
                {
                    cMessage *selfMsg;
                    RTPCISenderStatus *rsim = check_and_cast<RTPCISenderStatus *>(ci);
                    switch (rsim->getStatus())
                    {
                    case RTP_SENDER_STATUS_PLAYING:
                        EV << "PLAYING" << endl;
                        break;

                    case RTP_SENDER_STATUS_FINISHED:
                        EV << "FINISHED" << endl;
                        selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                        scheduleAt(simTime() + sessionLeaveDelay, selfMsg);
                        break;

                    case RTP_SENDER_STATUS_STOPPED:
                        EV << "STOPPED" << endl;
                        selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                        scheduleAt(simTime() + sessionLeaveDelay, selfMsg);
                        break;

                    default:
                        error("Invalid sender status: %d", rsim->getStatus());
                    }
                }
                break;

            case RTP_IFP_SESSION_LEFT:
                if (!isActiveSession)
                    EV << "Session already left\n";
                else
                    isActiveSession = false;
                break;

            case RTP_IFP_SENDER_MODULE_DELETED:
                EV << "Sender Module Deleted" << endl;
                break;

            default:
                break;
            }
        }
        delete obj;
    }
    delete msgIn;
}

bool RTPApplication::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

