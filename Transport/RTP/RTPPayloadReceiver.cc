/***************************************************************************
                          RTPPayloadReceiver.cc  -  description
                             -------------------
    begin                : Fri Nov 2 2001
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


/** \file RTPPayloadReceiver.cc
 * This file contains method implementations for RTPPayloadReceiver.
 */

#include <fstream>
#include "RTPPayloadReceiver.h"
#include "RTPPacket.h"
#include "RTPInnerPacket.h"


Define_Module(RTPPayloadReceiver);


RTPPayloadReceiver::~RTPPayloadReceiver() {
    closeOutputFile();
    delete _packetArrival;
};


void RTPPayloadReceiver::initialize() {
    const char *fileName = par("outputFileName");
    openOutputFile(fileName);
    _packetArrival = new cOutVector("packet arrival");
};


void RTPPayloadReceiver::handleMessage(cMessage *msg) {
    RTPInnerPacket *rinp = (RTPInnerPacket *)msg;
    if (rinp->type() == RTPInnerPacket::RTP_INP_DATA_IN) {
        RTPPacket *packet = (RTPPacket *)(rinp->decapsulate());
        processPacket(packet);
        delete rinp;
    }
    else {
        EV << "receiver module: RTPInnerPacket of wrong type received" << endl;
        delete rinp;
    }
};


void RTPPayloadReceiver::processPacket(RTPPacket *packet) {
    _packetArrival->record((double)(packet->timeStamp()));
};


void RTPPayloadReceiver::openOutputFile(const char *fileName) {
    _outputFileStream.open(fileName);
};


void RTPPayloadReceiver::closeOutputFile() {
    _outputFileStream.close();
};
