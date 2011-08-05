//
// AnnotationManager - manages annotations on the OMNeT++ canvas
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
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

package inet.world.annotations;

//
// Manages annotations on the OMNeT++ canvas
//
// An xml example:
//    <annotations>
//      <line color="#F00" shape="16,0 8,13.8564" />
//      <poly color="#0F0" shape="16,64 8,77.8564 -8,77.8564 -16,64 -8,50.1436 8,50.1436" />
//    </annotations>
//
simple AnnotationManager
{
    parameters:
        volatile bool draw = default(false);  // draw annotations?
        xml annotations = default(xml("<annotations/>")); // annotations to add at startup
        @display("i=msg/paperclip");
        @labels(node);
}

