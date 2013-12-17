/***************************************************************************
                       RTPAVProfilePayload32Sender.cpp  -  description
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

#include "RTPAVProfilePayload32Sender.h"
#include "RTPInnerPacket.h"
#include "RTPMpegPacket_m.h"
#include "RTPPacket.h"


Define_Module(RTPAVProfilePayload32Sender);


void RTPAVProfilePayload32Sender::initialize()
{
    RTPPayloadSender::initialize();

    _clockRate = 90000;
    _payloadType = 32;
}

void RTPAVProfilePayload32Sender::initializeSenderModule(RTPInnerPacket *rinpIn)
{
    EV << "initializeSenderModule Enter"<<endl;
    char line[100];
    char unit[100];
    char description[100];

    RTPPayloadSender::initializeSenderModule(rinpIn);

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
    // scheduleAt(simTime() + _initialDelay, reminderMessage);
    EV << "initializeSenderModule Exit" << endl;
}

bool RTPAVProfilePayload32Sender::sendPacket()
{
    EV << "sendPacket() "<< endl;
    // read next frame line
    int bits;
    char unit[100];
    char description[100];

    _inputFileStream >> bits;
    _inputFileStream >> unit;
    _inputFileStream.get(description, 100, '\n');

    int pictureType;
    char *ptr;

    for (ptr = description; *ptr == ' '; ptr++)
        ;
    switch (*ptr)
    {
        case 'I': pictureType = 1; break;
        case 'P': pictureType = 2; break;
        case 'B': pictureType = 3; break;
        case 'D': pictureType = 4; break;
        default: pictureType = 0; break;
    }

    int bytesRemaining = bits / 8;

    if (!_inputFileStream.eof())
    {
        while (bytesRemaining > 0)
        {
            RTPPacket *rtpPacket = new RTPPacket("RTPPacket");
            RTPMpegPacket *mpegPacket = new RTPMpegPacket("MpegPacket");

            // the only mpeg information we know is the picture type
            mpegPacket->setPictureType(pictureType);

            // the maximum number of real data bytes
            int maxDataSize = _mtu - rtpPacket->getBitLength() - mpegPacket->getBitLength();

            if (bytesRemaining > maxDataSize)
            {
                // we do not know where slices in the
                // mpeg picture begin
                // so we simulate by assuming a slice
                // has a length of 64 bytes
                int slicedDataSize = (maxDataSize / 64) * 64;

                mpegPacket->addBitLength(slicedDataSize);


                rtpPacket->encapsulate(mpegPacket);

                bytesRemaining = bytesRemaining - slicedDataSize;
            }
            else
            {
                mpegPacket->addBitLength(bytesRemaining);
                rtpPacket->encapsulate(mpegPacket);
                // set marker because this is
                // the last packet of the frame
                rtpPacket->setMarker(1);
                bytesRemaining = 0;
            }

            rtpPacket->setPayloadType(_payloadType);
            rtpPacket->setSequenceNumber(_sequenceNumber);
            _sequenceNumber++;


            rtpPacket->setTimeStamp(_timeStampBase + (_initialDelay + (1 / _framesPerSecond) * (double)_frameNumber) * _clockRate);
            rtpPacket->setSsrc(_ssrc);


            RTPInnerPacket *rinpOut = new RTPInnerPacket("dataOut()");


            rinpOut->setDataOutPkt(rtpPacket);

            send(rinpOut, "profileOut");
        }
        _frameNumber++;

        _reminderMessage = new cMessage("nextFrame");
        scheduleAt(simTime() + 1.0 / _framesPerSecond, _reminderMessage);
        return true;
    }
    else
    {
        std::cout <<"LastSequenceNumber "<< _sequenceNumber << endl;
        return false;
    }
    EV << "sendPacket() Exit"<< endl;
}
