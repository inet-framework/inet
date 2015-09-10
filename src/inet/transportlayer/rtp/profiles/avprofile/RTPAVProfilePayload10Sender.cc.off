/***************************************************************************
                          RTPAVProfilePayload10Sender.cc  -  description
                             -------------------
    begin                : Sat Sep 7 2002
    copyright            : (C) 2002 by
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/** \file RTPAVProfilePayload10Sender.cc
 */

#include <audiofile.h>

#include "RTPSenderControlMessage.h"
#include "RTPAVProfilePayload10Sender.h"

Define_Module(RTPAVProfilePayload10Sender);

void RTPAVProfilePayload10Sender::initialize() {
    RTPPayloadSender::initialize();
    _payloadType = 10;
    _clockRate = 44100;
    _samplingRate = 44100;
    _sampleWidth = 16;
    _numberOfChannels = 2;
};