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

package inet.tracker.integrated;

import inet.tracker.contract.IDataLinkTracker;
import inet.tracker.contract.IIntegratedTracker;
import inet.tracker.contract.INetworkPathTracker;
import inet.tracker.contract.IPhysicalLinkTracker;
import inet.tracker.contract.ITransportPathTracker;

//
// This module integrates all communication trackers into a single module. It
// supports disabling any submodule tracker by providing an empty string as
// its type.
//
module IntegratedTracker like IIntegratedTracker
{
    parameters:
        @display("i=block/app");
        *.trackingSubjectModule = default("^.^");
    submodules:
        physicalLinkTracker: <default("PhysicalLinkTracker")> like IPhysicalLinkTracker if typename != "" {
            parameters:
                @display("p=150,100");
        }
        dataLinkTracker: <default("DataLinkTracker")> like IDataLinkTracker if typename != "" {
            parameters:
                @display("p=150,200");
        }
        networkPathTracker: <default("NetworkPathTracker")> like INetworkPathTracker if typename != "" {
            parameters:
                @display("p=150,300");
        }
        transportPathTracker: <default("TransportPathTracker")> like ITransportPathTracker if typename != "" {
            parameters:
                @display("p=150,400");
        }
}

