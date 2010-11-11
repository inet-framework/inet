/***************************************************************************
                       RTPApplication.cpp  -  description
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


#include "IPAddress.h"
#include "IPAddressResolver.h"
#include "RTPApplication.h"
#include "RTPInterfacePacket.h"

Define_Module(RTPApplication)


void RTPApplication::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage!=3)
        return;

    // read all omnet parameters

    // the common name (CNAME) of this host
    _commonName = par("commonName");

    // which rtp profile is to be used (usually RTPAVProfile)
    _profileName = par("profileName");

    // bandwidth in bytes per second for this session
    _bandwidth = par("bandwidth");

    // the ip address to connect to (unicast or multicast)
    _destinationAddress = IPAddressResolver().resolve(par("destinationAddress").stringValue()).get4();

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
    sessionEntered = false;
    transmissionStarted = false;
    transmissionFinished = false;
    sessionLeft = false;
    cMessage *selfMsg = new cMessage("enterSession", ENTER_SESSION);
    scheduleAt(simTime() + _sessionEnterDelay, selfMsg);
}

void RTPApplication::handleMessage(cMessage* msgIn)
{
    if (!sessionLeft)
    {
        if (msgIn->isSelfMessage())
        {
            switch (msgIn->getKind())
            {
                case ENTER_SESSION:
                {
                    ev << "enterSession" << endl;
                    // create an RTPInterfacePacket to enter the session
                    RTPInterfacePacket *rifpOut1 = new RTPInterfacePacket("enterSession()");
                    rifpOut1->enterSession(opp_strdup(_commonName), opp_strdup(_profileName), _bandwidth, _destinationAddress, _port);
                    // and send it to the rtp layer
                    send(rifpOut1, "rtpOut");
                    break;
                }

                case START_TRANSMISSION:
                {
                    ev << "startTransmission" << endl;
                    RTPSenderControlMessage *rscm = new RTPSenderControlMessage();
                    rscm->setCommand("PLAY");
                    RTPInterfacePacket *rifpOut = new RTPInterfacePacket("senderModuleControl(PLAY)");
                    rifpOut->senderModuleControl(ssrc, rscm);
                    send(rifpOut, "rtpOut");
                    transmissionStarted = true;

                    cMessage *selfMsg = new cMessage("stopTransmission", STOP_TRANSMISSION);
                    scheduleAt(simTime() + _transmissionStopDelay, selfMsg);
                    break;
                }

                case STOP_TRANSMISSION:
                {
                    ev << "stopTransmission" << endl;
                    RTPSenderControlMessage *rscm = new RTPSenderControlMessage();
                    rscm->setCommand("STOP");
                    RTPInterfacePacket *rifpOut = new RTPInterfacePacket("senderModuleControl(STOP)");
                    rifpOut->senderModuleControl(ssrc, rscm);
                    send(rifpOut, "rtpOut");
                    break;
                }

                case LEAVE_SESSION:
                {
                    ev << "leaveSession" << endl;
                    RTPInterfacePacket *rifpOut = new RTPInterfacePacket("leaveSession()");
                    rifpOut->leaveSession();
                    send(rifpOut, "rtpOut");
                    break;
                }

                default:
                    opp_error("Invalid msgKind value %d in message '%s'", msgIn->getKind(), msgIn->getName());
                    break;
            }
        }
        else
        {
            RTPInterfacePacket *rifpIn = check_and_cast<RTPInterfacePacket *>(msgIn);

            if (rifpIn->getType() == RTP_IFP_SESSION_ENTERED)
            {
                ev << "Session Entered" << endl;
                ssrc = rifpIn->getSsrc();
                sessionEntered = true;
                if (opp_strcmp(_fileName, ""))
                {
                    ev << "CreateSenderModule" << endl;
                    RTPInterfacePacket *rifpOut = new RTPInterfacePacket("createSenderModule()");
                    rifpOut->createSenderModule(ssrc, _payloadType, opp_strdup(_fileName));
                    send(rifpOut, "rtpOut");
                }
                else
                {
                    cMessage *selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                    ev << "Receiver Module : leaveSession" << endl;
                    scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                }
            }
            else if (rifpIn->getType() == RTP_IFP_SENDER_MODULE_CREATED)
            {
                ev << "Sender Module Created" << endl;
                cMessage *selfMsg = new cMessage("startTransmission", START_TRANSMISSION);
                scheduleAt(simTime() + _transmissionStartDelay, selfMsg);
            }
            else if (rifpIn->getType() == RTP_IFP_SENDER_STATUS)
            {
                RTPSenderStatusMessage *rsim = (RTPSenderStatusMessage *)(rifpIn->decapsulate());
                if (rsim->getStatus() == RTP_STATUS_PLAYING)
                {
                    ev << "PLAYING" << endl;
                }
                else if (rsim->getStatus() == RTP_STATUS_FINISHED)
                {
                    transmissionFinished = true;
                    ev << "FINISHED" << endl;
                    cMessage *selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                    scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                }
                else if (rsim->getStatus() == RTP_STATUS_STOPPED)
                {
                    transmissionFinished = true;
                    ev << "FINISHED" << endl;
                    cMessage *selfMsg = new cMessage("leaveSession", LEAVE_SESSION);
                    scheduleAt(simTime() + _sessionLeaveDelay, selfMsg);
                }
                else
                {
                    delete rifpIn;
                }
                cancelAndDelete(rsim);
            }
            else if (rifpIn->getType() == RTP_IFP_SESSION_LEFT)
            {
                sessionLeft = true;
            }
        }
    }
    delete msgIn;
}
