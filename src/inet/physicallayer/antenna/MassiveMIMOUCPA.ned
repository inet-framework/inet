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

package inet.physicallayer.antenna;

import inet.physicallayer.base.packetlevel.AntennaBase;



//
// This antenna model describes the well-known dipole antenna or doublet. It
// consists of two identical conductive elements, which are bilaterally
// symmetrical. 
// 
module MassiveMIMOUCPA extends AntennaBase
{
    parameters:
        double length @unit(m); // the length of the single antenna
        double distance; // distance between elements
        double freq;
        double phiz; // the steering angle
        int M;  //number of circular rings
        string energySourceModule;
        double interval @unit(s)=default(2s);


        @class(MassiveMIMOUCPA);
        @signal[radioModeChangedSignal](type="long");
        @signal[receptionStateChangedSignal](type="long");
        @signal[transmissionStateChangedSignal](type="long");
        @signal[receivedSignalPartChangedSignal](type="long");
        //@statistic[guadagno](title="guadagno"; source="gain"; record=vector; interpolationmode=none);
    submodules:
}