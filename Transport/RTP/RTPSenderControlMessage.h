/***************************************************************************
                          RTPSenderControlMessage.h  -  description
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

#ifndef RTPSENDERCONTROLMESSAGE_H
#define RTPSENDERCONTROLMESSAGE_H

#include <omnetpp.h>
#include "INETDefs.h"


/**
 * A message of the type RTPSenderControlMessage is created by an application to control
 * the behavior of an rtp sender module. With this class the application can send a command
 * string to the sender module like "PLAY" or "STOP". A message can have up to two float
 * parameters.
 *
 * Following commands are evaluated in RTPPayloadSender (although correct behavior must
 * be implemented in modules for certain payload types):
 *    - PLAY : start playing from current position
 *    - PAUSE : pause playing, stay at current position
 *    - STOP : stop playing, go to beginning
 *    - PLAY_UNTIL_TIME : start playing from current position and play until given temporal position (relative to
 *      start of file is reached)
 *    - PLAY_UNTIL_BYTE * start playing from current position and play until this data byte is reached
 *    - SEEK_TIME : go to temporal position within the file (not allowed while playing)
 *    - SEEK_BYTE : go to data byte (not allowed while playing)
 */
class INET_API RTPSenderControlMessage : public cMessage
{

public:

        /**
         * Default constructor.
         */
        RTPSenderControlMessage(const char *name = NULL);

        /**
         * Copy constructor.
         */
        RTPSenderControlMessage(const RTPSenderControlMessage& message);

        /**
         * Destructor.
         */
        virtual ~RTPSenderControlMessage();

        /**
         * Assignment operator.
         */
        RTPSenderControlMessage& operator=(const RTPSenderControlMessage& message);

        /**
         * Duplicates the object.
         */
        virtual cObject *dup() const;

        /**
         * Returns the class name "RTPSenderControlMessage".
         */
        virtual const char *className() const;

        /**
         * Returns the command string in this message.
         */
        virtual const char *command() const;

        /**
         * Set the command string this message transports to the sender module.
         */
        virtual void setCommand(const char *command);

        virtual void setCommand(const char *command, float commandParameter1);
        virtual void setCommand(const char *command, float commandParameter1, float commandParameter2);

        virtual float commandParameter1();
        virtual float commandParameter2();

private:

        /**
         * The command string stored in the message.
         */
        const char *_command;

        float _commandParameter1, _commandParameter2;

};

#endif

