/***************************************************************************
                          RTPAVProfilePayload10Receiver.h  -  description
                             -------------------
    begin                : Fri Sep 20 2002
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

/** \file RTPAVProfilePayload10Receiver.h"
 *
 */

#include <omnetpp.h>

#include "RTPPayloadReceiver.h"
#include "RTPAVProfileSampleBasedAudioReceiver.h"


/**
 *
 */
class INET_API RTPAVProfilePayload10Receiver : public RTPAVProfileSampleBasedAudioReceiver
{
  protected:
    virtual void initialize();
    virtual void insertSilence(simtime_t duration);
};
