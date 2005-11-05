/***************************************************************************
                          RTPApplication.cpp  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \file RTPApplication.cc
 * This file contains the implementation for member functions
 * of RTPApplication.
 */

#include <omnetpp.h>

//XXX #include "in_addr.h"
//XXX #include "in_port.h"
#include "tmp/defs.h"

#include "types.h"
#include "RTPApplication.h"
#include "RTPInterfacePacket.h"

Define_Module(RTPApplication)


void RTPApplication::initialize() {

    // read all omnet parameters

    // the common name (CNAME) of this host
    _commonName = par("commonName");

    // which rtp profile is to be used (usually RTPAVProfile)
    _profileName = par("profileName");

    // bandwidth in bytes per second for this session
    _bandwidth = par("bandwidth");

    // the ip address to connect to (unicast or multicast)
    _destinationAddress = IN_Addr(par("destinationAddress").stringValue());

    // port number which is to be used; to ports are actually used: one
    // for rtp and one for rtcp
    _port = IN_Port((int)(par("portNumber").longValue()));

    // fileName of file to be transmitted
    // NULL or "" means this system acts only as a receiver
    _fileName = par("fileName");

    // payload type of file to transmit
    _payloadType = par("payloadType");

    _sessionEnterDelay = par("sessionEnterDelay");
    _transmissionStartDelay = par("transmissionStartDelay");
    _transmissionStopDelay = par("transmissionStopDelay");
    _sessionLeaveDelay = par("sessionLeaveDelay");
}


void RTPApplication::activity() {


    bool sessionEntered = false;
    bool transmissionStarted = false;
    bool transmissionFinished = false;
    bool sessionLeft = false;


    cMessage *msg1 = new cMessage("enterSession");
    scheduleAt(simTime() + _sessionEnterDelay, msg1);

    u_int32 ssrc = 0;

    while (!sessionLeft) {

        cMessage *msgIn = receive();
        if (msgIn->isSelfMessage()) {
            if (!opp_strcmp(msgIn->name(), "enterSession")) {
                // create an RTPInterfacePacket to enter the session
                RTPInterfacePacket *rifpOut1 = new RTPInterfacePacket("enterSession()");
                rifpOut1->enterSession(opp_strdup(_commonName), opp_strdup(_profileName), _bandwidth, _destinationAddress, _port);
                // and send it to the rtp layer
                send(rifpOut1, "toRTP");
            }
            else if (!opp_strcmp(msgIn->name(), "startTransmission")) {
                RTPSenderControlMessage *rscm = new RTPSenderControlMessage();
                rscm->setCommand("PLAY");
                RTPInterfacePacket *rifpOut = new RTPInterfacePacket("senderModuleControl(PLAY)");
                rifpOut->senderModuleControl(ssrc, rscm);
                send(rifpOut, "toRTP");
                transmissionStarted = true;

                cMessage *msg4 = new cMessage("stopTransmission");
                scheduleAt(simTime() + _transmissionStopDelay, msg4);
            }
            else if (!opp_strcmp(msgIn->name(), "stopTransmission")) {
                RTPSenderControlMessage *rscm = new RTPSenderControlMessage();
                rscm->setCommand("STOP");
                RTPInterfacePacket *rifpOut = new RTPInterfacePacket("senderModuleControl(STOP)");
                rifpOut->senderModuleControl(ssrc, rscm);
                send(rifpOut, "toRTP");
            }
            else if (!opp_strcmp(msgIn->name(), "leaveSession")) {
                RTPInterfacePacket *rifpOut = new RTPInterfacePacket("leaveSession()");
                rifpOut->leaveSession();
                send(rifpOut, "toRTP");
            }
        }
        else {
            if (opp_strcmp(msgIn->className(), "RTPInterfacePacket")) {
                opp_error("RTPApplication can only receive packets of type RTPInterfacePacket !");
            }
            RTPInterfacePacket *rifpIn = (RTPInterfacePacket *)msgIn;
            if (rifpIn->type() == RTPInterfacePacket::RTP_IFP_SESSION_ENTERED) {
                ssrc = rifpIn->ssrc();
                sessionEntered = true;
                if (opp_strcmp(_fileName, "")) {
                    RTPInterfacePacket *rifpOut = new RTPInterfacePacket("createSenderModule()");
                    rifpOut->createSenderModule(ssrc, _payloadType, opp_strdup(_fileName));
                    send(rifpOut, "toRTP");
                }
                else {
                    cMessage *msg2 = new cMessage("leaveSession");
                    scheduleAt(simTime() + _sessionLeaveDelay, msg2);
                }
            }
            else if (rifpIn->type() == RTPInterfacePacket::RTP_IFP_SENDER_MODULE_CREATED) {
                cMessage *msg3 = new cMessage("startTransmission");
                scheduleAt(simTime() + _transmissionStartDelay, msg3);
            }
            else if (rifpIn->type() == RTPInterfacePacket::RTP_IFP_SENDER_STATUS) {
                RTPSenderStatusMessage *rsim = (RTPSenderStatusMessage *)(rifpIn->decapsulate());
                if (!opp_strcmp(rsim->status(), "PLAYING")) {
                    //
                }
                else if (!opp_strcmp(rsim->status(), "FINISHED")) {
                    transmissionFinished = true;
                    cMessage *msg5 = new cMessage("leaveSession");
                    scheduleAt(simTime() + _sessionLeaveDelay, msg5);
                }
                else if (!opp_strcmp(rsim->status(), "STOPPED")) {
                    transmissionFinished = true;
                    cMessage *msg6 = new cMessage("leaveSession");
                    scheduleAt(simTime() + _sessionLeaveDelay, msg6);
                }
                else {
                }
            }
            else if (rifpIn->type() == RTPInterfacePacket::RTP_IFP_SESSION_LEFT) {
                sessionLeft = true;
            }
        }
        delete msgIn;

    }
}
