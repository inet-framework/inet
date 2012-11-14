//
// Copyright 2012 OpenSim Ltd.
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// @author Zoltan Bojthe
//


// package module_.lib;


//
// a simple queue for prevents "channel is currently busy with an ongoing transmission"
// on an output channel connected to "out" gate
//
simple QQ
{
    gates:
        input in;
        output out;
}

