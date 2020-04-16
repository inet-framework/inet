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

package inet.environment.common;

import inet.environment.contract.IGround;
import inet.environment.contract.IObjectCache;
import inet.environment.contract.IPhysicalEnvironment;

//
// The propagation of communication signals, the movement of communicating
// agents, or battery exhaustion depend on the surrounding physical environment.
// For example, signals can be absorbed by objects, can pass through objects,
// can be refracted by surfaces, can be reflected from surfaces, etc.
//
// This module represents the physical environment. The physical environment
// defines a set of physical objects. The physical objects are loaded from an
// XML configuration file. The file must contain exactly one <config> root
// element. Under the root element there can be multiple of the following
// elements.
//
//  - <object>
//    This element defines a physical object.
//
//    - id:
//       This optional attribute specifies a globally unique integer identifier
//       among all objects. This value might be used to find a specific object
//       from C++ models.
//
//    - name:
//       This optional attribute specifies the name of the object. The name need
//       not be unique, and it might be still useful on the graphical user interface.
//
//    - shape:
//       TODO
//
//    - position:
//       This mandatory attribute specifies the position of the object. The value
//       constists of a type followed by 3 coordinates (X, Y, and Z) separated
//       by spaces. The type specifier determines which predefined point of the
//       shape is specified by the coordinates. The supported types are min, max,
//       and center. The coordinates refer to the respective point of the shape's
//       bounding box without applying orientation.
//
//    - orientation:
//       This optional attribute specifies the orientation of the object relative
//       to the default orientation of the shape. The value contains 3 rotation
//       Euler angles (heading, elevation and bank) respectively. The rotation
//       is done around the center of the shape's bounding box.
//
//    - material:
//       This mandatory attribute specifies the material of the object. The value
//       contains the name of a predefined material, or refers to a material
//       defined in the same file. In the latter case the value contains the
//       identifier or the name of the material.
//
//    - line-width:
//       This optional attribute specifies the line width for the outline of the
//       object's surface. The value 0 means the object will not have an outline.
//       The default value is 1.
//
//    - line-color:
//       This optional attribute specifies the color for the outline of the
//       object's surface. The value either contains a predefined color name or
//       3 integers (red, green, and blue) between 0 and 255 separated by spaces.
//       The default value is black.
//
//    - fill-color:
//       This optional attribute specifies the color for filling the object's
//       surface. The value is either a predefined color name or 3 integers (red,
//       green, and blue) between 0 and 255 separated by spaces. The default
//       value is white.
//
//    - opacity:
//       This optional attribute specifies the opacity of the object. The value
//       is a number between 0 and 1, the former means completely transparent,
//       the latter means completely opaque. The default value is 1.
//
//    - tags:
//       This optional attribute attaches multiple tags, separated by spaces, to
//       the object. They are primarily useful for hiding objects in the graphical
//       user interface. The default value is empty.
//
//    - texture:
//       This optional attribute specifies an image that is drawn on the faces of
//       the object. The default value specifies no texture.
//
//  - <shape>
//    This element defines a shape that might be used by multiple physical objects.
//
//    - id:
//       This mandatory attribute specifies a globally unique integer identifier
//       among all shapes. This value might be used in the shape attribute of
//       object elements.
//
//    - type:
//       This mandatory attribute determines the type of the shape. Valid values
//       are sphere, cuboid, prism, and polyhedron.
//
//    - size:
//       This attribute is mandatory for cuboid shapes, it specifies the size in
//       meters. The value contains 3 coordinates (X, Y, and Z sizes) separated
//       by spaces.
//
//    - radius:
//       This attribute is mandatory for sphere shapes, it specifies the radius
//       in meters.
//
//    - height:
//       This attribute is mandatory for prism shapes, it specifies the height
//       in meters.
//
//    - points:
//       This attribute is mandatory for prism and polyhedron shapes, it contains
//       a sequence of numbers separated by spaces. The value specifies the base
//       polygon for prisms using 2 coordinates per point, or it specifies the
//       vertices for polyhedrons using 3 coordinates per point.
//
//  - <material>
//    This element defines a material that might be used by multiple physical objects.
//
//    - id:
//       This mandatory attribute specifies a globally unique integer identifier
//       among all materials. This value might be used in the material attribute
//       of object elements.
//
//    - name:
//       This optional attribute specifies the name of the material. This value
//       might be used in the material attribute of object elements to refer to
//       this material. The name need not be unique, and it might be still useful
//       on the graphical user interface.
//
//    - resistivity:
//       This mandatory attribute specifies the resistivity of the material in
//       ohm per meter. Resistivity is used to compute the dielectric loss
//       tangent of the material.
//
//    - relativePermittivity:
//       This mandatory attribute specifies the relative permittivity of the
//       material as a unitless quantity. Relative permittivity is used to
//       compute the dielectric loss tangent and the refractive index of the
//       material.
//
//    - relativePermeability:
//       This mandatory attribute specifies the relative permeability of the
//       material as a unitless quantity. Relative permeability is used to
//       compute the refractive index of the material.
//
module PhysicalEnvironment like IPhysicalEnvironment
{
    parameters:
        string coordinateSystemModule = default(""); // module path of the geographic coordinate system module
        double temperature @unit(K) = default(293K);
        double spaceMinX @unit(m) = default(nan m); // lower bound of x coordinates in space, NaN means determined by physical objects, NaN by default
        double spaceMinY @unit(m) = default(nan m); // lower bound of y coordinates in space, NaN means determined by physical objects, NaN by default
        double spaceMinZ @unit(m) = default(nan m); // lower bound of z coordinates in space, NaN means determined by physical objects, NaN by default
        double spaceMaxX @unit(m) = default(nan m); // upper bound of x coordinates in space, NaN means determined by physical objects, NaN by default
        double spaceMaxY @unit(m) = default(nan m); // upper bound of y coordinates in space, NaN means determined by physical objects, NaN by default
        double spaceMaxZ @unit(m) = default(nan m); // upper bound of z coordinates in space, NaN means determined by physical objects, NaN by default
        xml config = default(xml("<config/>"));
        @display("i=misc/town");
        @class(PhysicalEnvironment);

    submodules:
        objectCache: <default("")> like IObjectCache if typename != "" {
            parameters:
                @display("p=100,100");
        }
        ground: <default("")> like IGround if typename != "" {
            parameters:
                @display("p=100,200");
        }
}

