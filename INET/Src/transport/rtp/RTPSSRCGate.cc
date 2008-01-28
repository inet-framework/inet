/***************************************************************************
                          RTPSSRCGate.cc  -  description
                             -------------------
    begin                : Tue Jan 1 2002
    copyright            : (C) 2002 by Matthias Oppitz
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


/** \file RTPSSRCGate.cc
 * This file contains the implementation of member functions of the class RTPSSRCGate.
 */

#include <omnetpp.h>

#include "types.h"
#include "RTPSSRCGate.h"
#include "RTPParticipantInfo.h"


Register_Class(RTPSSRCGate);

RTPSSRCGate::RTPSSRCGate(u_int32 ssrc) : cObject() {
    _ssrc = ssrc;
    setName(RTPParticipantInfo::ssrcToName(_ssrc));
};


RTPSSRCGate::RTPSSRCGate(const RTPSSRCGate& rtpSSRCGate) {
    cObject::operator=(rtpSSRCGate);
    _ssrc = rtpSSRCGate._ssrc;
};


RTPSSRCGate::~RTPSSRCGate() {

};


u_int32 RTPSSRCGate::ssrc() {
    return _ssrc;
};


void RTPSSRCGate::setSSRC(u_int32 ssrc) {
    _ssrc = ssrc;
};


int RTPSSRCGate::gateId() {
    return _gateId;
};


void RTPSSRCGate::setGateId(int gateId) {
    _gateId = gateId;
};
