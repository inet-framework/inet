/***************************************************************************
                       RTPApplication.cpp  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
    (C) 2010 Zoltan Bojthe
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "RTPApplication.h"

#include "IPvXAddressResolver.h"
#include "RTPInterfacePacket_m.h"

Define_Module(RTPApplication)

RTPApplication::RTPApplication()
{
    _commonName = NULL;
    _profileName = NULL;
    _fileName = NULL;
}

RTPApplication::~RTPApplication()
{
    cancelAndDeleteSelfMsgs();
}

void RTPApplication::initialize(int stage)
{
    AppBase::initialize(stage);

    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage == 0)
    {
        // the common name (CNAME) of this host
        _commonName = par("commonName");

        // which rtp profile is to be used (usually RTPAVProfile)
        _profileName = par("profileName");

        // bandwidth in bytes per second for this session
        _bandwidth = par("bandwidth");

        // port number which is to be used; to ports are actually used: one
        // for rtp and one for rtcp
        _port = (int)par("portNumber").longValue();

        // fileName of file to be transmitted
        // NULL or "" means this system acts only as a receiver
        _fileName = par("fileName");

        // payload type of file to transmit
        _payloadType = par("payloadType");

        _sessionEnterDelay = par("sessionEnterDelay");
        _transmissionStartDelay = par("transmissionStartDelay");
        _transmissionStopDelay = par("transmissionStopDelay");
        _sessionLeaveDelay = par("sessionLeaveDelay");

        ssrc = 0;
        isActiveSession = false;
    }
}

void RTPApplication::handleMessageWhenUp(cMessage* msgIn)
{
    if (msgIn->isSelfMessage())
    {
        selfMessages.erase(msgIn);

        switch (msgIn->getKind())
        {
        case ENTER_SESSION:
            ev << "enterSession" << endl;
            if (isActiveSession)
            {
                ev << "Session already entered\n";
            }
            else
            {
                isActiveSession = true;

                // the ip address to connect to (unicast or multicast)
                IPv4Address destinationAddress = IPvXAddressResolver().resolve(par("destinationAddress").stringValue()).get4();

                RTPCIEnterSession* ci = new RTPCIEnterSession();
                ci->setCommonName(_commonName);
                ci->setProfileName(_profileName);
                ci->setBandwidth(_bandwidth);
                ci->setDestinationAddress(destinationAddress);
                ci->setPort(_port);
                cMessage *msg = new RTPControlMsg("Enter Session");
                msg->setControlInfo(ci);
                send(msg, "rtpOut");
            }
            break;

        case START_TRANSMISSION:
            ev << "startTransmission" << endl;
            if (!isActiveSession)
            {
                ev << "Session already left\n";
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
                selfMessages.insert(selfMsg);
                scheduleAt(simTime() + _transmissionStopDelay, selfMsg);
            }
            break;

        case STOP_TRANSMISSION:
            ev << "stopTransmission" << endl;
            if (!isActiveSession)
            {
                ev << "Session already left\n";
            }
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
            ev << "leaveSession" << endl;
            if (!isActiveSession)
            {
                ev << "Session already left\n";
            }
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
            break;
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
                    ev << "Session Entered" << endl;
                    ssrc = (check_and_cast<RTPCISessionEntered *>(ci))->getSsrc();
                    if (opp_strcmp(_fileName, ""))
                    {
                        ev << "CreateSenderModule" << endl;
                        RTPCICreateSenderModule* ci = new RTPCICreateSenderModule();
                        ci->setSsrc(ssrc);
                        ci->setPayloadType(_payloadType);
                        ci->setFileName(_fileName);
                        cMessage *msg = new RTPControlMsg("createSenderModule()");
                        msg->setControlInfo(ci);
                        send(msg, "rtpOut");
                    }
                    else
                    {
                        ev << "Receiver Module : leaveSession" << endl;
                        cMessage *selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                        selfMessages.insert(selfMsg);
                        scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                    }
                }
                break;

            case RTP_IFP_SENDER_MODULE_CREATED:
                {
                    ev << "Sender Module Created" << endl;
                    cMessage *selfMsg = new cMessage("startTransmission", START_TRANSMISSION);
                    selfMessages.insert(selfMsg);
                    scheduleAt(simTime() + _transmissionStartDelay, selfMsg);
                }
                break;

            case RTP_IFP_SENDER_STATUS:
                {
                    cMessage *selfMsg;
                    RTPCISenderStatus *rsim = check_and_cast<RTPCISenderStatus *>(ci);
                    switch (rsim->getStatus())
                    {
                    case RTP_SENDER_STATUS_PLAYING:
                        ev << "PLAYING" << endl;
                        break;

                    case RTP_SENDER_STATUS_FINISHED:
                        ev << "FINISHED" << endl;
                        selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                        selfMessages.insert(selfMsg);
                        scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                        break;

                    case RTP_SENDER_STATUS_STOPPED:
                        ev << "STOPPED" << endl;
                        selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                        selfMessages.insert(selfMsg);
                        scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                        break;

                    default:
                        error("Invalid sender status: %d", rsim->getStatus());
                    }
                }
                break;

            case RTP_IFP_SESSION_LEFT:
                if (!isActiveSession)
                    ev << "Session already left\n";
                else
                    isActiveSession = false;
                break;

            case RTP_IFP_SENDER_MODULE_DELETED:
                ev << "Sender Module Deleted" << endl;
                break;

            default:
                break;
            }
        }
        delete obj;
    }
    delete msgIn;
}

void RTPApplication::cancelAndDeleteSelfMsgs()
{
    for (std::set<cMessage*>::iterator it=selfMessages.begin(); it!=selfMessages.end(); ++it)
        cancelAndDelete(*it);
    selfMessages.clear();
}

void RTPApplication::reset()
{
    ssrc = 0;
    isActiveSession = false;
    cancelAndDeleteSelfMsgs();
}

bool RTPApplication::startApp(IDoneCallback *doneCallback)
{
    cMessage *selfMsg = new cMessage("enterSession", ENTER_SESSION);
    selfMessages.insert(selfMsg);
    scheduleAt(simTime() + _sessionEnterDelay, selfMsg);
    return true;
}

bool RTPApplication::stopApp(IDoneCallback *doneCallback)
{
    reset();
    return true;
}

bool RTPApplication::crashApp(IDoneCallback *doneCallback)
{
    reset();
    return true;
}

