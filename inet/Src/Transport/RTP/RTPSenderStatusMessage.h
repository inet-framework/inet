/***************************************************************************
                          RTPSenderStatusMessage.h  -  description
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

#ifndef RTPSenderStatusMessage_H
#define RTPSenderStatusMessage_H

#include <omnetpp.h>
#include "INETDefs.h"
#include "types.h"

/**
 * Messages of type RTPSenderStatusMessage are used to send information
 * from an rtp sender module to the application. Within this class a status
 * string is defined in which the information is stored. This can be "PLAYING",
 * "STOPPED" or "FINISHED".
 * If a message must provide more information than just a string, a new class
 * defining this parameter can derived.
 */
class INET_API RTPSenderStatusMessage : public cMessage
{
public:

    /**
     * Default constructor.
     */
    RTPSenderStatusMessage(const char *name = NULL);

    /**
     * Copy constructor.
     */
    RTPSenderStatusMessage(const RTPSenderStatusMessage& message);

    /**
     * Destructor.
     */
    ~RTPSenderStatusMessage();

    /**
     * Assignment operator.
     */
    RTPSenderStatusMessage& operator=(const RTPSenderStatusMessage& message);

    /**
     * Duplicates the object.
     */
    cObject *dup() const;

    /**
     * Returns the class name "RTPSenderStatusMessage".
     */
    const char *className() const;

    /**
     * Returns the status string stored in this message.
     */
    virtual const char *status() const;

    /**
     * Sets the status string to be stored in this message.
     */
    virtual void setStatus(const char *status);

    virtual void setTimeStamp(const u_int32 timestamp);

    virtual const u_int32 timeStamp();

private:

    /**
     * The status string.
     */
    const char *_status;

    u_int32 _timeStamp;

};

#endif

