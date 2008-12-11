//
// Copyright (C) 2005 Andras Varga
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



package inet.base;

//
// Using NotificationBoard, modules can now notify each other about
// "events" such as routing table changes, interface status changes (up/down),
// interface configuration changes, wireless handovers, changes in the state
// of the wireless channel, mobile node position changes, etc.
//
// The NotificationBoard has exactly one instance within a host or router
// model, and acts as a intermediary between module where state changes
// can occur and modules which are interested in learning about those
// changes.
//
// NotificationBoard is accessed via direct C++ method calls (not
// message exchange). Modules can "subscribe" to categories of changes
// (e.g. "routing table changed" or "radio channel became empty").
// When such a change occurs, the corresponding module (e.g. the RoutingTable
// or the physical layer module) will let NotificationBoard know, and
// it will disseminate this information to all interested modules.
//
simple NotificationBoard
{
    parameters:
        @display("i=block/control");
}

