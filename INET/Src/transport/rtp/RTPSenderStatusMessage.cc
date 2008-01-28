/***************************************************************************
                          RTPSenderStatusMessage.cc  -  description
                             -------------------
    begin                : Sat Aug 17 2002
    copyright            : (C) 2002 by Matthias Oppitz, Arndt Buschmann
    email                : <matthias.oppitz@gmx.de> <a.buschmann@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "RTPSenderStatusMessage.h"
#include "types.h"

RTPSenderStatusMessage::RTPSenderStatusMessage(const char *name) {
    _status = "";
};


RTPSenderStatusMessage::RTPSenderStatusMessage(const RTPSenderStatusMessage& message) {
    setName(message.name());
    operator=(message);
};


RTPSenderStatusMessage::~RTPSenderStatusMessage(){
};


RTPSenderStatusMessage& RTPSenderStatusMessage::operator=(const RTPSenderStatusMessage& message) {
    cMessage::operator=(message);
    _status = message.status();
    return *this;
};


cObject *RTPSenderStatusMessage::dup() const {
    return new RTPSenderStatusMessage(*this);
};


const char *RTPSenderStatusMessage::className() const {
    return "RTPSenderStatusMessage";
};


const char *RTPSenderStatusMessage::status() const {
    return opp_strdup(_status);
};

const u_int32 RTPSenderStatusMessage::timeStamp() {
    return _timeStamp;
}

void RTPSenderStatusMessage::setStatus(const char *status) {
    _status = status;
}

void RTPSenderStatusMessage::setTimeStamp(const u_int32 timeStamp) {
    _timeStamp = timeStamp;
}

