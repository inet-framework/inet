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

package inet.physicallayer.errormodel.packetlevel;

simple Ieee80211RadioErrorModelEvaluator
{
    parameters:
        string snirsFilename; // TODO replace with an object read by the built-in CSV functions once available
        string persFilename;
        string radioModule = default("^.radio");
        int repeatCount = default(100);
        @display("i=block/cogwheel");
}

