//
// Copyright (C) 2007 Peterpaul Klein Haneveld
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

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Moves a tractor through a field with a certain amount of rows.
// Since the tractor also moves around the field, the tractor travels
// the number of rows PLUS one rows. Consider the following piece of
// ascii-art for rows=2.
//
// >>>>>>>>>>>>>>>v
// ^              v
// ^              v
// <<<<<<<<<<<<<<<<
// v              ^
// v              ^
// >>>>>>>>>>>>>>>^
//
// NOTE: Does not yet support 3-dimensional movement.
// @author Peterpaul Klein Haneveld
//
simple TractorMobility extends MovingMobilityBase
{
    parameters:
        double speed @unit(mps);
        double x1 @unit(m);
        double y1 @unit(m);
        double x2 @unit(m);
        double y2 @unit(m);
        int rowCount; // the number of rows on the field
        @class(TractorMobility);
}
