//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//




import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::lora;

//
// Message kinds for sending commands to the radio.
//
enum LoRaRadioCommandCode
{
    LORA_RADIO_C_CONFIGURE = 1;
}

//
// Control info attached to a configure command that is sent to the ~Radio.
//
class LoRaConfigureRadioCommand
{
    int radioMode = -1;                  // new radio mode or -1 if not set.
    W power;                    // new default transmission power in the range (0, +infinity) or NaN if not set.
    bps bitrate;              // new default bitrate in the range (0, +infinity) or NaN if not set.
    //IModulationPtr modulation = nullptr; // new default modulation or nullptr if not set.
    Hz carrierFrequency;       // new default carrier frequency in the range (0, +infinity) or NaN if not set.
    Hz bandwidth;              // new default bandwidth in the rage (0, +infinity) or NaN if not set.
}

//
// Control info attached to a mac frame that is sent down to the ~Radio.
//
class LoRaTransmissionRequest
{
    W power;              // override default transmission power in the range (0, +infinity) or NaN if not set.
    bps bitrate;        // override default bitrate in the range (0, +infinity) or NaN if not set.
    Hz carrierFrequency; // override default carrier frequency in the range (0, +infinity) or NaN if not set.
    Hz bandwidth;        // override default bandwidth in the rage (0, +infinity) or NaN if not set.
}

//
// Control info attached to a mac frame that is sent up from the ~Radio.
//
class LoRaReceptionIndication
{
    //int bitErrorCount = -1;       // number of erroneous bits in the range [0, +infinity) or -1 if unknown.
    //int symbolErrorCount = -1;    // number of erroneous symbols in the range [0, +infinity) or -1 if unknown.
    //double packetErrorRate = NaN; // packet error rate (probability) in the range [0, 1] or NaN if unknown.
    //double bitErrorRate = NaN;    // bit error rate (probability) in the range [0, 1] or NaN if unknown.
    //double symbolErrorRate = NaN; // symbol error rate (probability) in the range [0, 1] or NaN if unknown.
    W minRSSI;           // minimum receive signal strength indication in the range (0, +infinity) or NaN if unknown.
    double minSNIR;         // minimum signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown.
    W LoRaTP;
    Hz LoRaCF;
    int LoRaSF;
    Hz LoRaBW;
    int LoRaCR;
}
