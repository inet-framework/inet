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


void RTPApplication::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    // read all omnet parameters

    // the common name (CNAME) of this host
    _commonName = par("commonName");

    // which rtp profile is to be used (usually RTPAVProfile)
    _profileName = par("profileName");

    // bandwidth in bytes per second for this session
    _bandwidth = par("bandwidth");

    // the ip address to connect to (unicast or multicast)
    _destinationAddress = IPvXAddressResolver().resolve(par("destinationAddress").stringValue()).get4();

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

    ev << "commonName" << _commonName << endl;
    ev << "profileName" << _profileName << endl;
    ev << "bandwidth" << _bandwidth << endl;
    ev << "destinationAddress" << _destinationAddress << endl;
    ev << "portNumber" << _port << endl;
    ev << "fileName" << _fileName << endl;
    ev << "payloadType" << _payloadType << endl;

    ssrc = 0;
    isActiveSession = false;
    cMessage *selfMsg = new cMessage("enterSession", ENTER_SESSION);
    scheduleAt(simTime() + _sessionEnterDelay, selfMsg);
}

void RTPApplication::handleMessage(cMessage* msgIn)
{
    if (msgIn->isSelfMessage())
    {
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
                RTPCIEnterSession* ci = new RTPCIEnterSession();
                ci->setCommonName(_commonName);
                ci->setProfileName(_profileName);
                ci->setBandwidth(_bandwidth);
                ci->setDestinationAddress(_destinationAddress);
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
                        cMessage *selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                        ev << "Receiver Module : leaveSession" << endl;
                        scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                    }
                }
                break;

            case RTP_IFP_SENDER_MODULE_CREATED:
                {
                    ev << "Sender Module Created" << endl;
                    cMessage *selfMsg = new cMessage("startTransmission", START_TRANSMISSION);
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
                        scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                        break;

                    case RTP_SENDER_STATUS_STOPPED:
                        ev << "STOPPED" << endl;
                        selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
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
