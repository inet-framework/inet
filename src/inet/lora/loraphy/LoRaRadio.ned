//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.lora.loraphy;

import inet.lora.loraphy.LoRaTransmitter;
import inet.lora.loraphy.LoRaReceiver;
import inet.physicallayer.common.packetlevel.Radio;
import inet.physicallayer.base.packetlevel.FlatRadioBase;
import inet.power.storage.IdealEpEnergyStorage;
module LoRaRadio extends FlatRadioBase
{
    parameters:
        antenna.typename = default("IsotropicAntenna");
        transmitter.typename = default("LoRaTransmitter");
        receiver.typename = default("LoRaReceiver");

        
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[radioModeChanged](type=long);
        @signal[listeningChanged];
        @signal[receptionStateChanged](type=long);
        @signal[transmissionStateChanged](type=long);
        @signal[receivedSignalPartChanged](type=long);
        @signal[transmittedSignalPartChanged](type=long);
        @signal[transmissionStarted];
        @signal[transmissionEnded];
        @signal[receptionStarted];
        @signal[receptionEnded];
        @signal[minSNIR];
        @signal[packetErrorRate];
        @signal[bitErrorRate];
        @signal[symbolErrorRate];
        @signal[droppedPacket](type=long);

        @statistic[radioMode](title="Radio mode"; source=radioModeChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[receptionState](title="Radio reception state"; source=receptionStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[transmissionState](title="Radio transmission state"; source=transmissionStateChanged; record=count,vector; interpolationmode=sample-hold);
        
        //transmitterType = default("Ieee802154UWBIRTransmitter");
        //receiverType = default("Ieee802154UWBIRReceiver");

        centerFrequency = 868 MHz;
        bandwidth = default(0.25 MHz);

        // 802.15.4-2006 (page 28)
        *.bitrate = default(250 kbps);

        // PHY Header, 802.15.4-2006 (page 43)
        // 4 octets Preamble
        // 1 octet SFD
        // 7 bit Frame length
        // 1 bit Reserved
        *.headerBitLength = (4*8 + 1*8 + 7 + 1) * 1 b;

        // RSSI sensitivity (ATmega256RFR2, page 566)
        receiver.energyDetection = default(-90dBm);

        // Lowest receiver sensitivity at SF 12, 
        receiver.sensitivity = default(-137dBm);

        // There is no fixed boundary, because of the
        // DSSS and the capture effect. Taking the sensitivity minus some
        // arbitrary value as an approximate guess.
        receiver.minInterferencePower = default(-120dBm);

        // Minimum SNIR
        // -8 dB results into 98% PER for a PSDU of 20 octets
        receiver.snirThreshold = default(-8 dB);

        //Default transmitter power set to maximum for cache limits
        transmitter.power = default(25.118mW);
        transmitter.preambleDuration = 0.001s;
        //*.energySourceModule = default(absPath(energySourceModule));
        //*.energySourceModule = default(absPath(energySourceModule));
        bool iAmGateway = default(false);
        @class(LoRaRadio); //originally it was @class(Radio);
        @display("bgb=215,413");
    submodules:
        IdealEpEnergyStorage: IdealEpEnergyStorage {
            @display("p=178,296");
        }

}
