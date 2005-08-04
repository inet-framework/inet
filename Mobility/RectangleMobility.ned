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
// Moves the node around a rectangle.
//
// @see ChannelControl
// @author Andras Varga
//
simple RectangleMobility
    parameters:
        debug : bool, // debug switch
        x1: numeric const, // rectangle is denoted by the (x1,y1) and (x2,y2) coords
        y1: numeric const,
        x2: numeric const,
        y2: numeric const,
        startPos: numeric const, // in range [0.0,4.0): topleft=0, topright=1, bottomright=2, bottomleft=3
        speed: numeric const, // speed of the host (in m/s)
        updateInterval : numeric const; // time interval to update the hosts position
endsimple

