//
// Copyright (C) 2014 OpenSim Ltd.
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

package inet.physicallayer.wireless.common.errormodel.packetlevel;

simple NeuralNetworkErrorModelTrainingDatasetGenerator
{
    parameters:
        string filenameFormat = default("results/%c_%r.csv");
        string radioModule = default("^.radio");
        int repeatCount = default(100);
        int packetCount;
        double bitrate @unit (bps);
        volatile int packetLength @unit(B);
        volatile double backgroundNoisePowerMean @unit(W);
        volatile double backgroundNoisePowerStddev @unit(W);
        volatile int numInterferingSignals = default(0);
        volatile double interferingSignalNoisePowerMean @unit(W) = default(0W);
        volatile double interferingSignalNoisePowerStddev @unit(W) = default(0W);
        @display("i=block/cogwheel");
}

