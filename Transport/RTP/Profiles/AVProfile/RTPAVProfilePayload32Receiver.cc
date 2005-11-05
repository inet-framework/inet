/***************************************************************************
                          RTPAVProfilePayload32Receiver.cc  -  description
                             -------------------
    begin                : Sun Jan 6 2002
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


/** \file RTPAVProfilePayload32Receiver.cc
 *  * In this file member functions of RTPAVProfilePayload32Receiver are
 *  * implemented.
 *  */

#include <omnetpp.h>

#include "RTPPayloadReceiver.h"
#include "RTPAVProfilePayload32Receiver.h"
#include "RTPPacket.h"
#include "RTPMpegPacket.h"

Define_Module_Like(RTPAVProfilePayload32Receiver, RTPPayloadReceiver);


RTPAVProfilePayload32Receiver::~RTPAVProfilePayload32Receiver() {
    delete _queue;
};


void RTPAVProfilePayload32Receiver::initialize() {
    RTPPayloadReceiver::initialize();
    _payloadType = 32;
    _queue = new cQueue("IncomingQueue", &(RTPPacket::compareFunction));
    _lowestAllowedTimeStamp = 0;
};


void RTPAVProfilePayload32Receiver::processPacket(RTPPacket *rtpPacket) {
    // the first packet sets the lowest allowed time stamp
    if (_lowestAllowedTimeStamp == 0) {
        _lowestAllowedTimeStamp = rtpPacket->timeStamp();
    };

    if (rtpPacket->timeStamp() < _lowestAllowedTimeStamp) {
        delete rtpPacket;
    }
    else {
        // is this packet from the next frame ?
        // this can happen when the marked packet has been
        // lost or arrives late
        bool nextTimeStamp = rtpPacket->timeStamp() > _lowestAllowedTimeStamp;

        // is this packet marked, which means that it's
        // the last packet of this frame
        bool marked = rtpPacket->marker();

        // test if end of frame reached

        // check if we received the last (= marked)
        // packet of the frame or
        // we received a packet of the next frame

        if (nextTimeStamp || marked) {

            // a marked packet belongs to this frame
            if (marked && !nextTimeStamp) {
                _queue->insert(rtpPacket);
            }

            int pictureType = 0;
            int frameSize = 0;

            // the queue contains all packets for this frame
            // we have received
            while (!_queue->empty()) {
                RTPPacket *readPacket = (RTPPacket *)(_queue->head());
                _queue->remove(readPacket);
                RTPMpegPacket *mpegPacket = (RTPMpegPacket *)(readPacket->decapsulate());
                if (pictureType == 0)
                    pictureType = mpegPacket->pictureType();
                frameSize = frameSize + mpegPacket->payloadLength();

                delete mpegPacket;
                delete readPacket;
            };

            // we start the next frame
            // set the allowed time stamp
            if (nextTimeStamp) {
                _lowestAllowedTimeStamp = rtpPacket->timeStamp();
                _queue->insert(rtpPacket);
            };

            // we have calculated a frame
            if (frameSize > 0) {
                char line[100];
                // what picture type is it
                char picture = ' ';
                if (pictureType == 1)
                    picture = 'I';
                else if (pictureType == 2)
                    picture = 'P';
                else if (pictureType == 3)
                    picture = 'B';
                else if (pictureType == 4)
                    picture = 'D';

                // create sim line
                sprintf(line, "%f %i %c-Frame", simTime(), frameSize * 8, picture);
                // and write it to the file
                _outputFileStream << line << endl;
            }
        }
        // we are not at the end of the frame
        // so just insert this packet
        else {
            _queue->insert(rtpPacket);
        }
    }
};




