//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/***************************************************************************
                          RtpAvProfilePayload32Receiver.cc  -  description
                             -------------------
    begin                : Sun Jan 6 2002
    copyright            : (C) 2002 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
***************************************************************************/

/** \file RtpAvProfilePayload32Receiver.cc
 *  * In this file member functions of RtpAvProfilePayload32Receiver are
 *  * implemented.
 *  */

#include "inet/transportlayer/rtp/profiles/avprofile/RtpAvProfilePayload32Receiver.h"

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"
#include "inet/transportlayer/rtp/profiles/avprofile/RtpMpegPacket_m.h"

namespace inet {

namespace rtp {

Define_Module(RtpAvProfilePayload32Receiver);

int compareRTPPacketsBySequenceNumber(cObject *packet1, cObject *packet2)
{
    auto pk1 = check_and_cast<Packet *>(packet1);
    auto pk2 = check_and_cast<Packet *>(packet2);
    const auto& h1 = pk1->peekAtFront<RtpHeader>();
    const auto& h2 = pk2->peekAtFront<RtpHeader>();
    return h1->getSequenceNumber() - h2->getSequenceNumber();
}

RtpAvProfilePayload32Receiver::~RtpAvProfilePayload32Receiver()
{
    delete _queue;
}

void RtpAvProfilePayload32Receiver::initialize()
{
    RtpPayloadReceiver::initialize();
    _payloadType = 32;
    _queue = new cQueue("IncomingQueue", &compareRTPPacketsBySequenceNumber);
    _lowestAllowedTimeStamp = 0;
    _highestSequenceNumber = 0;
}

void RtpAvProfilePayload32Receiver::processRtpPacket(Packet *rtpPacket)
{
    const auto& rtpHeader = rtpPacket->peekAtFront<RtpHeader>();
    // the first packet sets the lowest allowed time stamp
    if (_lowestAllowedTimeStamp == 0) {
        _lowestAllowedTimeStamp = rtpHeader->getTimeStamp();
        _highestSequenceNumber = rtpHeader->getSequenceNumber();
        if (_outputLogLoss.is_open())
            _outputLogLoss << "sequenceNumberBase" << rtpHeader->getSequenceNumber() << endl;
    }
    else if (_outputLogLoss.is_open()) {
        for (int i = _highestSequenceNumber + 1; i < rtpHeader->getSequenceNumber(); i++) {
//            char line[100];
//            sprintf(line, "%i", i);
            _outputLogLoss << i << endl;
        }
    }

    if ((rtpHeader->getTimeStamp() < _lowestAllowedTimeStamp) ||
        (rtpHeader->getSequenceNumber() <= _highestSequenceNumber))
    {
        delete rtpPacket;
    }
    else {
        // is this packet from the next frame ?
        // this can happen when the marked packet has been
        // lost or arrives late
        bool nextTimeStamp = rtpHeader->getTimeStamp() > _lowestAllowedTimeStamp;

        // is this packet marked, which means that it's
        // the last packet of this frame
        bool marked = rtpHeader->getMarker();

        // test if end of frame reached

        // check if we received the last (= marked)
        // packet of the frame or
        // we received a packet of the next frame
        _highestSequenceNumber = rtpHeader->getSequenceNumber();

        if (nextTimeStamp || marked) {
            // a marked packet belongs to this frame
            if (marked && !nextTimeStamp) {
                _queue->insert(rtpPacket);
            }

            int pictureType = 0;
            int frameSize = 0;

            // the queue contains all packets for this frame
            // we have received
            while (!_queue->isEmpty()) {
                Packet *qPacket = check_and_cast<Packet *>(_queue->pop());
                const auto& qRtpHeader = qPacket->popAtFront<RtpHeader>();
                (void)qRtpHeader; // unused variable
                const auto& mpegPacket = qPacket->peekAtFront<RtpMpegHeader>();
                if (pictureType == 0)
                    pictureType = mpegPacket->getPictureType();
                frameSize = frameSize + mpegPacket->getPayloadLength();

                delete qPacket;
            }

            // we start the next frame
            // set the allowed time stamp
            if (nextTimeStamp) {
                _lowestAllowedTimeStamp = rtpHeader->getTimeStamp();
                _queue->insert(rtpPacket);
            }

            // we have calculated a frame
            if (frameSize > 0 && _outputFileStream.is_open()) {
                char line[100];
                // what picture type is it
                char picture;
                switch (pictureType) {
                    case 1:
                        picture = 'I';
                        break;

                    case 2:
                        picture = 'P';
                        break;

                    case 3:
                        picture = 'B';
                        break;

                    case 4:
                        picture = 'D';
                        break;

                    default:
                        picture = ' ';
                        break;
                }

                // create sim line
                sprintf(line, "%f %i %c-Frame", simTime().dbl(), frameSize * 8, picture);
                // and write it to the file
                _outputFileStream << line << endl;
            }
        }
        else {
            // we are not at the end of the frame
            // so just insert this packet
            _queue->insert(rtpPacket);
        }
    }
}

} // namespace rtp

} // namespace inet

