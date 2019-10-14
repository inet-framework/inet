//
// Copyright (C) 2019 Marcel Marek
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

package inet.queueing;

//
// TODO auto-generated module
//
simple RedMarker extends RedDropper
{
    parameters:
        @class(RedMarker);
        int packetCapacity = default(120);
        double mark = default(0.0);
        @signal[markingProb](type=double);
        @statistic[marked](title="marking Prob"; source="markingProb"; record=mean,vector(count); interpolationmode=none);
}

