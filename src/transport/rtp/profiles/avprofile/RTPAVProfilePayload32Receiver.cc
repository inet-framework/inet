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

#include "RTPPayloadReceiver.h"
#include "RTPAVProfilePayload32Receiver.h"
#include "RTPPacket.h"
#include "RTPMpegPacket_m.h"

Define_Module(RTPAVProfilePayload32Receiver);

int compareRTPPacketsBySequenceNumber(cObject *packet1, cObject *packet2)
{
    return ((RTPPacket *)packet1)->getSequenceNumber() - ((RTPPacket *)packet2)->getSequenceNumber();
}

RTPAVProfilePayload32Receiver::~RTPAVProfilePayload32Receiver()
{
    delete _queue;
}


void RTPAVProfilePayload32Receiver::initialize()
{
    RTPPayloadReceiver::initialize();
    _payloadType = 32;
    _queue = new cQueue("IncomingQueue", &compareRTPPacketsBySequenceNumber);
    _lowestAllowedTimeStamp = 0;
    _highestSequenceNumber = 0;
}


void RTPAVProfilePayload32Receiver::processPacket(RTPPacket *rtpPacket)
{
    // the first packet sets the lowest allowed time stamp
    if (_lowestAllowedTimeStamp == 0) {
        _lowestAllowedTimeStamp = rtpPacket->getTimeStamp();
        _highestSequenceNumber = rtpPacket->getSequenceNumber();
        _outputLogLoss <<"sequenceNumberBase"<< rtpPacket->getSequenceNumber() << endl;
    }
    else
    {
        if (rtpPacket->getSequenceNumber() > _highestSequenceNumber+1)
        {
            for (int i = _highestSequenceNumber+1; i< rtpPacket->getSequenceNumber(); i++ )
            {
                //char line[100];
                //sprintf(line, "%i", i);
                _outputLogLoss << i << endl;
            }
         }
         }

    if ((rtpPacket->getTimeStamp() < _lowestAllowedTimeStamp) || (rtpPacket->getSequenceNumber()<= _highestSequenceNumber)) {
        delete rtpPacket;
    }
    else {
        // is this packet from the next frame ?
        // this can happen when the marked packet has been
        // lost or arrives late
        bool nextTimeStamp = rtpPacket->getTimeStamp() > _lowestAllowedTimeStamp;

        // is this packet marked, which means that it's
        // the last packet of this frame
        bool marked = rtpPacket->getMarker();

        // test if end of frame reached

        // check if we received the last (= marked)
        // packet of the frame or
        // we received a packet of the next frame
        _highestSequenceNumber = rtpPacket->getSequenceNumber();

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
                RTPPacket *readPacket = (RTPPacket *)(_queue->pop());
                RTPMpegPacket *mpegPacket = (RTPMpegPacket *)(readPacket->decapsulate());
                if (pictureType == 0)
                    pictureType = mpegPacket->getPictureType();
                frameSize = frameSize + mpegPacket->getPayloadLength();

                delete mpegPacket;
                delete readPacket;
            }

            // we start the next frame
            // set the allowed time stamp
            if (nextTimeStamp) {
                _lowestAllowedTimeStamp = rtpPacket->getTimeStamp();
                _queue->insert(rtpPacket);
            }

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
                sprintf(line, "%f %i %c-Frame", simTime().dbl(), frameSize * 8, picture);
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
}


