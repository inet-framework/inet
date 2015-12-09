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

package inet.linklayer.ieee80211.mac;

// References:
//   [1] ftp://ftp-sop.inria.fr/planete/ivs/papers/mswim04.pdf
//   [2] http://research.microsoft.com/en-us/projects/mesh/mobicom01.pdf
//    "However, there is a clear threshold region in the 40ms-60ms
//    range, depending on the degree of mobility, beyond which
//    there is little performance change but below which there is a
//    noticeable drop."
//
simple AARFRateControl like IRateControl
{
    parameters:
        double interval @unit("s") = default(50ms); // The rate (unconditionally) increases after each time interval (the default interval is unspecified [2]).
        int increaseThreshold = default(10); // Number of successful transmissions needed to increase the rate
        int decreaseThreshold = default(2); // Number of consecutive unsuccessful transmissions (in the sense that ACK does not arrive within timeout) 
                                            // needed to increase the rate
        double increaseThresholdFactor = default(2); // When the transmission of probing packet fails, we multiply by "increaseThresholdFactor" the increaseThreshold.
        int maxIncreaseThreshold = default(50); // Upper bound for increaseThreshold.
        @display("i=block/control");
}
