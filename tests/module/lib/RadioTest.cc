//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <iostream>
#include "RadioTest.h"
#include "StationaryMobility.h"
#include "RadioChannel.h"
#include "MultiThreadedRadioChannel.h"
#include "CUDARadioChannel.h"
#include "IdealImplementation.h"
#include "ScalarImplementation.h"
#include "DimensionalImplementation.h"

#define TIME(CODE) { struct timespec timeStart, timeEnd; clock_gettime(CLOCK_REALTIME, &timeStart); CODE; clock_gettime(CLOCK_REALTIME, &timeEnd); \
    EV_INFO << "Elapsed time is " << ((timeEnd.tv_sec - timeStart.tv_sec) + (timeEnd.tv_nsec - timeStart.tv_nsec) / 1E+9) << "s during running " << #CODE << endl << endl; }

double random(double a, double b)
{
    double r = (double)rand() / RAND_MAX;
    return a + r * (b-a);
}

void testIdealRadio()
{
    cPacket *transmitterMacFrame = new cPacket("Hello", 0, 64 * 8);

    IRadioSignalPropagation *propagation = new ConstantSpeedRadioSignalPropagation(mps(SPEED_OF_LIGHT), 0);
    IRadioSignalAttenuation *attenuation = new IdealRadioSignalFreeSpaceAttenuation();
    IRadioChannel *channel = new RadioChannel(propagation, attenuation, NULL, 1E-12, 10E-3, m(sNaN), m(sNaN));

    IMobility *transmitterMobility = new StationaryMobility(Coord(0, 0));
    IRadioAntenna *transmitterAntenna = new IsotropicRadioAntenna(transmitterMobility);
    IRadioSignalTransmitter *transmitterTransmitter = new IdealRadioSignalTransmitter(bps(2E+6), m(100), m(500), m(1000));
    IRadio *transmitterRadio = new Radio(OldIRadio::RADIO_MODE_TRANSMITTER, transmitterAntenna, transmitterTransmitter, NULL, channel);

    IMobility *receiverMobility = new StationaryMobility(Coord(100, 0));
    IRadioAntenna *receiverAntenna = new IsotropicRadioAntenna(receiverMobility);
    IRadioSignalReceiver *receiverReceiver = new IdealRadioSignalReceiver(false);
    IRadio *receiverRadio = new Radio(OldIRadio::RADIO_MODE_RECEIVER, receiverAntenna, NULL, receiverReceiver, channel);

    IRadioFrame *radioFrame = transmitterRadio->transmitPacket(transmitterMacFrame, simTime());
    cPacket *receiverMacFrame = receiverRadio->receivePacket(radioFrame);
    IRadioSignalReceptionDecision *receptionDecision = check_and_cast<IRadioSignalReceptionDecision *>(receiverMacFrame->getControlInfo());

    EV_INFO << "Ideal radio received " << receiverMacFrame << ", reception is " << (receptionDecision->isReceptionSuccessful() ? "successful" : "unsuccessful") << endl;

    delete channel;
    delete transmitterMobility;
    delete transmitterRadio;
    delete transmitterMacFrame;
    delete receiverMobility;
    delete receiverRadio;
    ASSERT(receiverMacFrame == transmitterMacFrame);
    delete radioFrame;
}

void testScalarRadio()
{
    cPacket *transmitterMacFrame = new cPacket("Hello", 0, 64 * 8);

    IRadioSignalPropagation *propagation = new ConstantSpeedRadioSignalPropagation(mps(SPEED_OF_LIGHT), 0);
    IRadioSignalAttenuation *attenuation = new ScalarRadioSignalFreeSpaceAttenuation(2);
    IRadioBackgroundNoise *backgroundNoise = new ScalarRadioBackgroundNoise(W(1E-14));
    IRadioChannel *channel = new RadioChannel(propagation, attenuation, backgroundNoise, 1E-12, 10E-3, m(sNaN), m(sNaN));

    IMobility *transmitterMobility = new StationaryMobility(Coord(0, 0));
    IRadioAntenna *transmitterAntenna = new IsotropicRadioAntenna(transmitterMobility);
    IRadioSignalTransmitter *transmitterTransmitter = new ScalarRadioSignalTransmitter(100, bps(2E+6), W(1E-3), Hz(2.4E+9), Hz(2E+6));
    IRadio *transmitterRadio = new Radio(OldIRadio::RADIO_MODE_TRANSMITTER, transmitterAntenna, transmitterTransmitter, NULL, channel);

    IMobility *receiverMobility = new StationaryMobility(Coord(100, 0));
    IRadioAntenna *receiverAntenna = new IsotropicRadioAntenna(receiverMobility);
    IRadioSignalReceiver *receiverReceiver = new ScalarRadioSignalReceiver(10, W(1E-12), W(1E-12), Hz(2.4E+9), Hz(2E+6));
    IRadio *receiverRadio = new Radio(OldIRadio::RADIO_MODE_RECEIVER, receiverAntenna, NULL, receiverReceiver, channel);

    IRadioFrame *radioFrame = transmitterRadio->transmitPacket(transmitterMacFrame, simTime());
    cPacket *receiverMacFrame = receiverRadio->receivePacket(radioFrame);
    IRadioSignalReceptionDecision *receptionDecision = check_and_cast<IRadioSignalReceptionDecision *>(receiverMacFrame->getControlInfo());

    EV_INFO << "Scalar radio received " << receiverMacFrame << ", reception is " << (receptionDecision->isReceptionSuccessful() ? "successful" : "unsuccessful") << endl;

    delete channel;
    delete transmitterMobility;
    delete transmitterRadio;
    delete transmitterMacFrame;
    delete receiverMobility;
    delete receiverRadio;
    ASSERT(receiverMacFrame == transmitterMacFrame);
    delete radioFrame;
}

void testDimensionalRadio()
{
    cPacket *transmitterMacFrame = new cPacket("Hello", 0, 64 * 8);

    IRadioSignalPropagation *propagation = new ConstantSpeedRadioSignalPropagation(mps(SPEED_OF_LIGHT), 0);
    IRadioSignalAttenuation *attenuation = new DimensionalRadioSignalFreeSpaceAttenuation(2);
    IRadioBackgroundNoise *backgroundNoise = new DimensionalRadioBackgroundNoise(W(1E-14));
    IRadioChannel *channel = new RadioChannel(propagation, attenuation, backgroundNoise, 1E-12, 10E-3, m(sNaN), m(sNaN));

    IMobility *transmitterMobility = new StationaryMobility(Coord(0, 0));
    IRadioAntenna *transmitterAntenna = new IsotropicRadioAntenna(transmitterMobility);
    IRadioSignalTransmitter *transmitterTransmitter = new DimensionalRadioSignalTransmitter(bps(2E+6), W(1E-3), Hz(2.4E+9), Hz(2E+6));
    IRadio *transmitterRadio = new Radio(OldIRadio::RADIO_MODE_TRANSMITTER, transmitterAntenna, transmitterTransmitter, NULL, channel);

    IMobility *receiverMobility = new StationaryMobility(Coord(100, 0));
    IRadioAntenna *receiverAntenna = new IsotropicRadioAntenna(receiverMobility);
    IRadioSignalReceiver *receiverReceiver = new DimensionalRadioSignalReceiver(10, W(1E-12), W(1E-11));
    IRadio *receiverRadio = new Radio(OldIRadio::RADIO_MODE_RECEIVER, receiverAntenna, NULL, receiverReceiver, channel);

    IRadioFrame *radioFrame = transmitterRadio->transmitPacket(transmitterMacFrame, simTime());
    cPacket *receiverMacFrame = receiverRadio->receivePacket(radioFrame);
    IRadioSignalReceptionDecision *receptionDecision = check_and_cast<IRadioSignalReceptionDecision *>(receiverMacFrame->getControlInfo());

    EV_INFO << "Dimensional radio received " << receiverMacFrame << ", reception is " << (receptionDecision->isReceptionSuccessful() ? "successful" : "unsuccessful") << endl;

    delete channel;
    delete transmitterMobility;
    delete transmitterRadio;
    delete transmitterMacFrame;
    delete receiverMobility;
    delete receiverRadio;
    ASSERT(receiverMacFrame == transmitterMacFrame);
    delete radioFrame;
}

void testMultipleScalarRadios(const char *name, IRadioChannel *channel, int radioCount, int frameCount, simtime_t duration, double playgroundSize)
{
    int successfulReceptionCount = 0;
    int failedReceptionCount = 0;
    std::vector<IRadio *> radios;
    std::vector<IRadioFrame *> radioFrames;
    EV_INFO << "Radio count = " << radioCount << ", frame count = " << frameCount << ", duration = " << duration << ", playground size = " << playgroundSize << endl;

    srand(0);
    for (int i = 0; i < radioCount; i++)
    {
        IMobility *mobility = new StationaryMobility(Coord(random(0, playgroundSize), random(0, playgroundSize)));
        IRadioAntenna *antenna = new IsotropicRadioAntenna(mobility);
        OldIRadio::RadioMode radioMode = random(0, 1) < 0.5 ? OldIRadio::RADIO_MODE_TRANSMITTER : OldIRadio::RADIO_MODE_RECEIVER;
        IRadioSignalTransmitter *transmitter = new ScalarRadioSignalTransmitter(100, bps(2E+6), W(1E-3), Hz(2.4E+9), Hz(2E+6));
        IRadioSignalReceiver *receiver = new ScalarRadioSignalReceiver(10, W(1E-12), W(1E-11), Hz(2.4E+9), Hz(2E+6));
        IRadio *radio = new Radio(radioMode, antenna, transmitter, receiver, channel);
        radios.push_back(radio);
    }

    for (int i = 0; i < frameCount; i++)
    {
        int index = random(0, radioCount);
        IRadio *radio = radios[index];
        simtime_t startTime = random(0, duration.dbl());
        cPacket *transmitterMacFrame = new cPacket("Hello", 0, 64 * 8);
        EV_DEBUG << "Sending at " << startTime << " from " << index << endl;
        IRadioFrame *radioFrame = radio->transmitPacket(transmitterMacFrame, startTime);
        radioFrames.push_back(radioFrame);
    }

    for (std::vector<IRadio *>::iterator it = radios.begin(); it != radios.end(); it++)
    {
        IRadio *radio = *it;
        for (std::vector<IRadioFrame *>::iterator jt = radioFrames.begin(); jt != radioFrames.end(); jt++)
        {
            IRadioFrame *radioFrame = *jt;
            cPacket *receiverMacFrame = radio->receivePacket(radioFrame);
            RadioSignalReceptionDecision *decision = check_and_cast<RadioSignalReceptionDecision *>(receiverMacFrame->getControlInfo());
            if (decision->isReceptionSuccessful())
                successfulReceptionCount++;
            else
                failedReceptionCount++;
//            const ScalarRadioSignalReception *reception = check_and_cast<const ScalarRadioSignalReception *>(decision->getReception());
//            EV_DEBUG << std::setprecision(16) << "Radio decision arrival time = " << reception->getStartTime() << ", reception power = " << reception->getPower() << ", snr minimum = " << decision->getSNIR() << endl;
            delete receiverMacFrame->removeControlInfo();
            check_and_cast<cPacket *>(radioFrame)->encapsulate(receiverMacFrame);
        }
    }

    check_and_cast<RadioChannel *>(channel)->callFinish();
    EV_INFO << name << " radio channel received packets, successful reception count = " << successfulReceptionCount << ", failed reception count = " << failedReceptionCount << endl;
    delete channel;

    for (std::vector<IRadio *>::iterator it = radios.begin(); it != radios.end(); it++)
    {
        IRadio *radio = *it;
        delete radio->getAntenna()->getMobility();
        delete radio;
    }

    for (std::vector<IRadioFrame *>::iterator it = radioFrames.begin(); it != radioFrames.end(); it++)
    {
        IRadioFrame *radioFrame = *it;
        delete radioFrame;
    }
}

void testMultipleScalarRadiosWithAllRadioChannels(int radioCount, int frameCount, simtime_t duration, double playgroundSize)
{
    IRadioSignalPropagation *propagation = new ConstantSpeedRadioSignalPropagation(mps(SPEED_OF_LIGHT), 0);
    IRadioSignalAttenuation *attenuation = new ScalarRadioSignalFreeSpaceAttenuation(2);
    IRadioBackgroundNoise *backgroundNoise = new ScalarRadioBackgroundNoise(W(1E-14));
    IRadioChannel *radioChannel = new RadioChannel(propagation, attenuation, backgroundNoise, 1E-12, 10E-3, m(sNaN), m(sNaN));
    TIME(testMultipleScalarRadios("Sequential", radioChannel, radioCount, frameCount, duration, playgroundSize));

    propagation = new ConstantSpeedRadioSignalPropagation(mps(SPEED_OF_LIGHT), 0);
    attenuation = new ScalarRadioSignalFreeSpaceAttenuation(2);
    backgroundNoise = new ScalarRadioBackgroundNoise(W(1E-14));
    radioChannel = new MultiThreadedRadioChannel(propagation, attenuation, backgroundNoise, 1E-12, 10E-3, m(sNaN), m(sNaN), 3);
    TIME(testMultipleScalarRadios("Parallel", radioChannel, radioCount, frameCount, duration, playgroundSize));

//    propagation = new ConstantSpeedRadioSignalPropagation(mps(SPEED_OF_LIGHT), 0);
//    attenuation = new ScalarRadioSignalFreeSpaceAttenuation(2);
//    backgroundNoise = new ScalarRadioBackgroundNoise(W(1E-14));
//    radioChannel = new CUDARadioChannel(propagation, attenuation, backgroundNoise, 1E-12, 10E-3, m(sNaN), m(sNaN));
//    TIME(testMultipleScalarRadios("CUDA", radioChannel, radioCount, frameCount, duration, playgroundSize));
}

Define_Module(RadioTest);

void RadioTest::initialize(int stage)
{
    testIdealRadio();
    testScalarRadio();
//    testDimensionalRadio();
    testMultipleScalarRadiosWithAllRadioChannels(10, 100, 2, 100);
    testMultipleScalarRadiosWithAllRadioChannels(20, 200, 2, 100);
    testMultipleScalarRadiosWithAllRadioChannels(40, 400, 2, 100);
    testMultipleScalarRadiosWithAllRadioChannels(80, 800, 2, 100);
}
