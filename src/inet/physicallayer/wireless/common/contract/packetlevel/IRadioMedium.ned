//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The medium model describes the shared physical medium where communication
// takes place. It keeps track of radios, noise sources, ongoing transmissions,
// background noise, and other ongoing noises. The medium computes when, where
// and how transmissions and noises arrive at receivers.
//
moduleinterface IRadioMedium
{
    parameters:
        @display("i=misc/sun");
        @signal[radioAdded](type=cModule);    // (type=IRadio)
        @signal[radioRemoved](type=cModule);    // (type=IRadio)
        @signal[signalAdded](type=cObject);    // (type=ITransmission)
        @signal[signalRemoved](type=cObject);    // (type=ITransmission)
        @signal[signalDepartureStarted](type=cObject);    // (type=ITransmission)
        @signal[signalDepartureEnded](type=cObject);    // (type=ITransmission)
        @signal[signalArrivalStarted](type=cObject);    // (type=IReception)
        @signal[signalArrivalEnded](type=cObject);    // (type=IReception)
}

