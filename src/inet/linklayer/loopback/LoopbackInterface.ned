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
// Author: Zoltan Bojthe
//


package inet.linklayer.loopback;

import inet.common.IHook;
import inet.linklayer.contract.ILoopbackInterface;


//
// Loopback interface. Complements the ~Loopback module.
//
module LoopbackInterface like ILoopbackInterface
{
    parameters:
        @class(InterfaceEntry);
        @display("i=block/ifcard;bgl=53");
        string interfaceTableModule;
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        *.interfaceTableModule = default(absPath(interfaceTableModule));
    gates:
        input upperLayerIn;
        output upperLayerOut;
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=100,100,row,50;is=s");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=400,100,row,50;is=s");
        }
        lo: Loopback {
            parameters:
                @display("p=250,200");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> outputHook[0].in if numOutputHooks > 0;
        upperLayerIn --> { @display("m=n"); } --> lo.upperLayerIn if numOutputHooks == 0;

        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }
        outputHook[numOutputHooks-1].out --> lo.upperLayerIn if numOutputHooks > 0;

        lo.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0;

        lo.upperLayerOut --> inputHook[0].in if numInputHooks > 0;

        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut if numInputHooks > 0;
}

