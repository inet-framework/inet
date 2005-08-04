//
// Copyright (C) 2005 Andras Varga
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
//

//
// Moves the node around a circle.
//
// @see ChannelControl
// @author Andras Varga
//
simple CircleMobility
    parameters:
        debug : bool, // debug switch
        cx: numeric const, // x coord of the center of the circle
        cy: numeric const, // y coord of the center of the circle
        r: numeric const, // radius of the circle
        speed: numeric const, // speed of the host (in m/s)
        startAngle: numeric const, // starting angle (degreees)
        updateInterval : numeric const; // time interval to update the hosts position
endsimple

