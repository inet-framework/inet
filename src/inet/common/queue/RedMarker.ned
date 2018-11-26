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

package inet.common.queue;


import inet.common.queue.RedDropper;
//
// TODO auto-generated module
//
simple RedMarker extends RedDropper
{
    parameters:
        @class(RedMarker);

        string marks = default("0");  // average packet rate for calculations when queue is empty
        @signal[markingProb](type=double);
        @statistic[marked](title="marking Prob"; source="markingProb"; record=mean,vector(count); interpolationmode=none);
}
