//
// Copyright (C) 2015 Felix Weinrank
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.trafficgen;

import inet.applications.trafficgen.ITrafficgen;

simple TrafficgenSimple like ITrafficgen
{
    parameters:
        int id;
        string name									= default("trafficgen");
       	int priority								= default(512);		
       	int packetCount								= default(-1);		// -1 = infinite
        volatile int packetSize @unit("B")			= default(1024B);	
        volatile double packetInterval @unit("s")	= default(0ms);	
        volatile int packetBurstSize				= default(1);
    	double startTime @unit("s")					= default(1s);
    	double stopTime @unit("s")					= default(-1s);		// -1 = infiniter
    	bool reliable								= default(true);
    	bool ordered								= default(true);
    	
    	@display("i=block/plug");
    	@signal[sentPkt](type=cPacket);
    	@statistic[sentPkt](title="packets sent"; source=sentPkt; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    	
    gates:
        input generatorIn @labels(trafficGen/up);
        output generatorOut @labels(trafficGen/down);	
}