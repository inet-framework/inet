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
#include "RTPApplication.h"
#include "RTPInterfacePacket.h"

Define_Module(RTPApplication)


void RTPApplication::initialize()
{

    // read all omnet parameters

    // the common name (CNAME) of this host
    _commonName = par("commonName");

    // which rtp profile is to be used (usually RTPAVProfile)
    _profileName = par("profileName");

    // bandwidth in bytes per second for this session
    _bandwidth = par("bandwidth");

    // the ip address to connect to (unicast or multicast)
    _destinationAddress = IPAddress(par("destinationAddress").stringValue());

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

    ev<< "commonName" <<  _commonName <<endl;
    ev<< "profileName" <<  _profileName <<endl;
    ev<< "bandwidth" <<  _bandwidth <<endl;
    ev<< "destinationAddress" <<  _destinationAddress <<endl;
    ev<< "portNumber" <<  _port <<endl;
    ev<< "fileName" <<  _fileName <<endl;
    ev<< "payloadType" <<  _payloadType <<endl;
}


void RTPApplication::activity()
{


    bool sessionEntered = false;
    bool transmissionStarted = false;
    bool transmissionFinished = false;
    bool sessionLeft = false;

    cMessage *msg1 = new cMessage("enterSession");
    scheduleAt(simTime() + _sessionEnterDelay, msg1);

    uint32 ssrc = 0;

    while (!sessionLeft) {

        cMessage *msgIn = receive();
        if (msgIn->isSelfMessage()) {
            if (!opp_strcmp(msgIn->getName(), "enterSession")) {
                ev << "enterSession"<<endl;
                // create an RTPInterfacePacket to enter the session
                RTPInterfacePacket *rifpOut1 = new RTPInterfacePacket("enterSession()");
                rifpOut1->enterSession(opp_strdup(_commonName), opp_strdup(_profileName), _bandwidth, _destinationAddress, _port);
                // and send it to the rtp layer
                send(rifpOut1, "rtpOut");
            }
            else if (!opp_strcmp(msgIn->getName(), "startTransmission")) {
                ev << "startTransmission"<<endl;
                RTPSenderControlMessage *rscm = new RTPSenderControlMessage();
                rscm->setCommand("PLAY");
                RTPInterfacePacket *rifpOut = new RTPInterfacePacket("senderModuleControl(PLAY)");
                rifpOut->senderModuleControl(ssrc, rscm);
                send(rifpOut, "rtpOut");
                transmissionStarted = true;

                cMessage *msg4 = new cMessage("stopTransmission");
                scheduleAt(simTime() + _transmissionStopDelay, msg4);
            }
            else if (!opp_strcmp(msgIn->getName(), "stopTransmission")) {
                ev << "stopTransmission"<<endl;
                RTPSenderControlMessage *rscm = new RTPSenderControlMessage();
                rscm->setCommand("STOP");
                RTPInterfacePacket *rifpOut = new RTPInterfacePacket("senderModuleControl(STOP)");
                rifpOut->senderModuleControl(ssrc, rscm);
                send(rifpOut, "rtpOut");
            }
            else if (!opp_strcmp(msgIn->getName(), "leaveSession")) {
                ev<< "leaveSession"<<endl;
                RTPInterfacePacket *rifpOut = new RTPInterfacePacket("leaveSession()");
                rifpOut->leaveSession();
                send(rifpOut, "rtpOut");
            }
        }
        else {
            RTPInterfacePacket *rifpIn = check_and_cast<RTPInterfacePacket *>(msgIn);

            if (rifpIn->getType() == RTPInterfacePacket::RTP_IFP_SESSION_ENTERED) {
                ev << "Session Entered"<<endl;
                ssrc = rifpIn->getSSRC();
                sessionEntered = true;
                if (opp_strcmp(_fileName, "")) {
                    RTPInterfacePacket *rifpOut = new RTPInterfacePacket("createSenderModule()");
                    rifpOut->createSenderModule(ssrc, _payloadType, opp_strdup(_fileName));
                    ev << "CreateSenderModule"<<endl;
                    send(rifpOut, "rtpOut");
                }
                else {
                    cMessage *msg2 = new cMessage("leaveSession");
                    ev << "Receiver Module : leaveSession"<<endl;
                    scheduleAt(simTime() + _sessionLeaveDelay, msg2);
                }
            }
            else if (rifpIn->getType() == RTPInterfacePacket::RTP_IFP_SENDER_MODULE_CREATED) {
                cMessage *msg3 = new cMessage("startTransmission");
                ev << "Sender Module Created"<<endl;
                scheduleAt(simTime() + _transmissionStartDelay, msg3);
            }
            else if (rifpIn->getType() == RTPInterfacePacket::RTP_IFP_SENDER_STATUS) {
                RTPSenderStatusMessage *rsim = (RTPSenderStatusMessage *)(rifpIn->decapsulate());
                if (!opp_strcmp(rsim->getStatus(), "PLAYING")) {
                    ev << "PLAYING"<<endl;
                }
                else if (!opp_strcmp(rsim->getStatus(), "FINISHED")) {
                    transmissionFinished = true;
                    ev << "FINISHED"<<endl;
                    cMessage *msg5 = new cMessage("leaveSession");
                    scheduleAt(simTime() + _sessionLeaveDelay, msg5);
                }
                else if (!opp_strcmp(rsim->getStatus(), "STOPPED")) {
                    transmissionFinished = true;
                    ev << "FINISHED"<<endl;
                    cMessage *msg6 = new cMessage("leaveSession");
                    scheduleAt(simTime() + _sessionLeaveDelay, msg6);
                }
                else {
                    delete rifpIn;
                }
                cancelAndDelete(rsim);
            }
            else if (rifpIn->getType() == RTPInterfacePacket::RTP_IFP_SESSION_LEFT) {
                sessionLeft = true;
            }
        }
        delete msgIn;

    }
}
