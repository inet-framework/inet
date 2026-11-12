//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.physicallayer.wireless.common.radio.packetlevel.Radio;

//
// The radio of one component carrier of a base station sector or a UE. A node with carrier
// aggregation has one such radio per carrier, under one MAC entity. A TDD carrier is one radio
// whose transmitter and receiver share the frequency; an FDD carrier is one radio whose
// transmitter and receiver are tuned to different frequencies: a base station transmits on the
// downlink frequency and listens on the uplink frequency, a UE the other way round. The radios
// of one band share an antenna through ~SharedAntennaRef.
//
module CellularRadio extends Radio
{
    parameters:
        bool isBaseStation; // Whether the radio belongs to a base station sector or to a UE
        double downlinkFrequency @unit(Hz); // Center frequency of the downlink carrier
        double uplinkFrequency @unit(Hz) = default(downlinkFrequency); // Center frequency of the uplink carrier; the same as the downlink frequency for TDD
        double subcarrierSpacing @unit(Hz) = default(30kHz); // Subcarrier spacing of the numerology, 15 kHz times a power of 2
        int numPrbs = default(273); // Number of PRBs of the carrier
        initialRadioMode = default("transceiver");
        signalAnalogRepresentation = default("dimensional");
        allowConcurrentTransmissions = default(isBaseStation);
        allowConcurrentReceptions = default(isBaseStation);
        antenna.typename = default("SharedAntennaRef");
        transmitter.typename = default("CellularTransmitter");
        receiver.typename = default("CellularReceiver");
        transmitter.centerFrequency = default(this.isBaseStation ? this.downlinkFrequency : this.uplinkFrequency);
        receiver.centerFrequency = default(this.isBaseStation ? this.uplinkFrequency : this.downlinkFrequency);
        *.subcarrierSpacing = default(this.subcarrierSpacing);
        *.numPrbs = default(this.numPrbs);
}
