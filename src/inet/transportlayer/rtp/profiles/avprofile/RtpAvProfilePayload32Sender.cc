/***************************************************************************
                       RtpAvProfilePayload32Sender.cpp  -  description
                             -------------------
    begin            : Fri Aug 2 2007
    copyright        : (C) 2007 by Matthias Oppitz, Ahmed Ayadi
    email            : <Matthias.Oppitz@gmx.de> <ahmed.ayadi@sophia.inria.fr>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <fstream>
#include <string.h>

#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"
#include "inet/transportlayer/rtp/profiles/avprofile/RtpAvProfilePayload32Sender.h"
#include "inet/transportlayer/rtp/profiles/avprofile/RtpMpegPacket_m.h"

namespace inet {

namespace rtp {

Define_Module(RtpAvProfilePayload32Sender);

void RtpAvProfilePayload32Sender::initialize()
{
    RtpPayloadSender::initialize();

    _clockRate = 90000;
    _payloadType = 32;
}

void RtpAvProfilePayload32Sender::initializeSenderModule(RtpInnerPacket *rinpIn)
{
    EV_TRACE << "initializeSenderModule Enter" << endl;
    char line[100];
    char unit[100];
    char description[100];

    RtpPayloadSender::initializeSenderModule(rinpIn);

    // first line: fps unit description
    _inputFileStream.get(line, 100, '\n');

    float fps;
    sscanf(line, "%f %s %s", &fps, unit, description);
    _framesPerSecond = fps;

    _frameNumber = 0;

    // get new line character
    char c;
    _inputFileStream.get(c);

    // second line: initial delay unit description
    _inputFileStream.get(line, 100, '\n');

    float delay;
    sscanf(line, "%f %s %s", &delay, unit, description);

    _initialDelay = delay;

    // wait initial delay
    // cPacket *reminderMessage = new cMessage("next frame");
    // scheduleAfter(_initialDelay, reminderMessage);
    EV_TRACE << "initializeSenderModule Exit" << endl;
}

bool RtpAvProfilePayload32Sender::sendPacket()
{
    EV_TRACE << "sendPacket() " << endl;
    // read next frame line
    int bits;
    char unit[100];
    char description[100];
    bool ret;

    _inputFileStream >> bits;
    _inputFileStream >> unit;
    _inputFileStream.get(description, 100, '\n');

    int pictureType;
    char *ptr;

    for (ptr = description; *ptr == ' ' || *ptr == '\t' ; ptr++)
        ;
    switch (*ptr) {
        case 'I':
            pictureType = 1;
            break;

        case 'P':
            pictureType = 2;
            break;

        case 'B':
            pictureType = 3;
            break;

        case 'D':
            pictureType = 4;
            break;

        default:
            pictureType = 0;
            break;
    }

    int bytesRemaining = bits / 8;

    if (!_inputFileStream.eof()) {
        while (bytesRemaining > 0) {
            Packet *packet = new Packet("RtpPacket");
            const auto& rtpHeader = makeShared<RtpHeader>();
            const auto& mpegHeader = makeShared<RtpMpegHeader>();
            const auto& mpegPayload = makeShared<ByteCountChunk>();

            // the only mpeg information we know is the picture type
            mpegHeader->setPictureType(pictureType);

            // the maximum number of real data bytes
            int maxDataSize = _mtu - B(rtpHeader->getChunkLength() + mpegHeader->getChunkLength()).get();

            if (bytesRemaining > maxDataSize) {
                // we do not know where slices in the
                // mpeg picture begin
                // so we simulate by assuming a slice
                // has a length of 64 bytes
                int slicedDataSize = (maxDataSize / 64) * 64;

                mpegHeader->setPayloadLength(slicedDataSize);
                mpegPayload->setLength(B(slicedDataSize));

                bytesRemaining = bytesRemaining - slicedDataSize;
            }
            else {
                mpegHeader->setPayloadLength(bytesRemaining);
                mpegPayload->setLength(B(bytesRemaining));
                // set marker because this is
                // the last packet of the frame
                rtpHeader->setMarker(1);
                bytesRemaining = 0;
            }

            rtpHeader->setPayloadType(_payloadType);
            rtpHeader->setSequenceNumber(_sequenceNumber);
            _sequenceNumber++;

            rtpHeader->setTimeStamp(_timeStampBase + (_initialDelay + (1 / _framesPerSecond) * (double)_frameNumber) * _clockRate);
            rtpHeader->setSsrc(_ssrc);
            packet->insertAtFront(rtpHeader);
            packet->insertAtBack(mpegHeader);
            packet->insertAtBack(mpegPayload);

            RtpInnerPacket *rinpOut = new RtpInnerPacket("dataOut()");
            rinpOut->setDataOutPkt(packet);

            send(rinpOut, "profileOut");
        }
        _frameNumber++;

        _reminderMessage = new cMessage("nextFrame");
        scheduleAfter(1.0 / _framesPerSecond, _reminderMessage);
        ret = true;
    }
    else {
        std::cout << "LastSequenceNumber " << _sequenceNumber << endl;
        ret = false;
    }
    EV_TRACE << "sendPacket() Exit" << endl;
    return ret;
}

} // namespace rtp

} // namespace inet

