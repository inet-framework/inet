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

package inet.physicallayer.radio.detailed;

import inet.physicallayer.radio.common.RadioBase;

// Physical layer module which knows how to initialize the Deciders
// and AnalogueModels from the modules directory.
// See the classes documentation for details.
simple DetailedRadio extends RadioBase
{
    parameters:
        bool recordStats = default(false); //enable/disable tracking of statistics (eg. cOutvectors)
        int headerLength @unit(bit) = default(0); //defines the length of the phy header (/preamble)

        bool usePropagationDelay;		//Should transmission delay be simulated?
        double thermalNoise @unit(dBm);	//the strength of the thermal noise [dBm]
        bool useThermalNoise;			//should thermal noise be considered?

        xml analogueModels; 			//Specification of the analogue models to use and their parameters
        xml decider;					//Specification of the decider to use and its parameters

        double sensitivity @unit(dBm);	//The sensitivity of the physical layer [dBm]
        double txPower @unit(mW);       //The actual transmission power
        double maxTXPower @unit(mW);	//The maximum transimission power of the physical layer [mW]

        //# switch times [s]:
        double timeRXToTX @unit(s) = default(0); // Elapsed time to switch from receive to send state
        double timeRXToSleep @unit(s) = default(0); // Elapsed time to switch from receive to sleep state

        double timeTXToRX @unit(s) = default(0); // Elapsed time to switch from send to receive state
        double timeTXToSleep @unit(s) = default(0); // Elapsed time to switch from send to sleep state

        double timeSleepToRX @unit(s) = default(0); // Elapsed time to switch from sleep to receive state
        double timeSleepToTX @unit(s) = default(0); // Elapsed time to switch from sleep to send state

        int initialRadioState = default(0);   // State the radio is initially in (0=RX, 1=TX, 2=Sleep) 

        double radioMinAtt = default(1.0); //radios gain factor (attenuation) while receiving
        double radioMaxAtt = default(0.0); //radios gain factor (attenuation) while not receiving

        int nbRadioChannels = default(1);  // Number of available radio channels. Defaults to single channel radio.
        int initialRadioChannel = default(0);  // Initial radio channel.

        @class(DetailedRadio);
}
