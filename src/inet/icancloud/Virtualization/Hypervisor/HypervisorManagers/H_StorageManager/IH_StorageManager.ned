//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_StorageManager;


moduleinterface IH_StorageManager
{
    parameters:
		// Number of Storage Servers
        int numStorageServers;										
        // Network file system parameters
        int requestSize_KB;		// Request size (in KB)
        //Parallel file system parameters
        int strideSize_KB;		// Stride Size (in KB)

		string storageCellType;
		
		double ioOverhead;
		
    gates:
        input fromVMStorageServers[];
        output toVMStorageServers[];
        input fromNodeStorageServers[numStorageServers];
        output toNodeStorageServers[numStorageServers];
        input fromNET_Manager;
        output toNET_Manager;
        input from_storageCell[];
        output to_storageCell[];
}

