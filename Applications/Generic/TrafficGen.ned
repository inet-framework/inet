// $Header$
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// Simulation of Transport and application level
// traffic generators for testing purposes



//
// Transport Application with Protocol set
//
module TrafficGenerator
    parameters: 
        GeneratorType: string,
        tcpProtocol : bool, 
        nodename : string,
        nodenr, 
        generationTime, 
        generationSize, // (bit)
        burstPackets : numeric const;
    gates: 
        in: in; 
        out: out; 
    submodules: 
        appIn: AppIn; 
            parameters: 
                nodename = nodename;
            display: "p=97,98;b=40,24,rect";
        appOut: GeneratorType like GeneratorAppOut; 
            parameters: 
                nodename = nodename,
                nodenr = nodenr,
                tcpProtocol = tcpProtocol, 
                generationTime = generationTime, 
                generationSize = generationSize,
                burstPackets = burstPackets;
            display: "p=145,100;b=40,24,rect";
    connections: 
        in --> appIn.in; 
        appOut.out --> out; 
    display: "p=10,10;b=203,147,rect";
endmodule 

simple AppIn
    parameters: 
        nodename : string;
    gates: 
        in: in; 
endsimple 

simple GeneratorAppOut
    parameters: 
        tcpProtocol : bool, 
        nodename : string,
        nodenr, 
        generationTime,
        generationSize, // (bit)
        burstPackets : numeric const;
    gates: 
        out: out; 
endsimple 

