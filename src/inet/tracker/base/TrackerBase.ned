//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.tracker.base;

//
// This is a base module for tracker simple modules. It simply provides the
// tracking target module for derived modules.
//
simple TrackerBase
{
    parameters:
        string trackingTargetModule = default("^"); // module path relative to the tracker which determines where the tracking should appear in the user interface, parent module by default
        string trackingSubjectModule = default("^"); // module path relative to the tracker which determines what part of the simulation model the tracker displays in the user interface, parent module by default
        @class(TrackerBase);
        @display("i=block/app");
}

