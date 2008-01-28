/***************************************************************************
                          RTPSenderControlMessage.cc  -  description
                             -------------------
    begin                : Fri Aug 16 2002
    copyright            : (C) 2002 by Matthias Oppitz
    email                : matthias.oppitz@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <omnetpp.h>

#include "RTPSenderControlMessage.h"


RTPSenderControlMessage::RTPSenderControlMessage(const char *name) {
    _command = "";
};


RTPSenderControlMessage::RTPSenderControlMessage(const RTPSenderControlMessage& message) {
    setName(message.name());
    operator=(message);
};


RTPSenderControlMessage::~RTPSenderControlMessage() {
};


RTPSenderControlMessage& RTPSenderControlMessage::operator=(const RTPSenderControlMessage& message) {
    cMessage::operator=(message);
    _command = message.command();
    return *this;
};


cObject *RTPSenderControlMessage::dup() const {
    return new RTPSenderControlMessage(*this);
};


const char *RTPSenderControlMessage::className() const {
    return "RTPSenderControlMessage";
};


const char *RTPSenderControlMessage::command() const {
    return opp_strdup(_command);
};


void RTPSenderControlMessage::setCommand(const char *command) {
    _command = command;
    _commandParameter1 = 0.0;
    _commandParameter2 = 0.0;
};


void RTPSenderControlMessage::setCommand(const char *command, float commandParameter1) {
    _command = command;
    _commandParameter1 = commandParameter1;
    _commandParameter2 = 0.0;
};


void RTPSenderControlMessage::setCommand(const char *command, float commandParameter1, float commandParameter2) {
    _command = command;
    _commandParameter1 = commandParameter1;
    _commandParameter2 = commandParameter2;
};


float RTPSenderControlMessage::commandParameter1() {
    return _commandParameter1;
};


float RTPSenderControlMessage::commandParameter2() {
    return _commandParameter2;
};
