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

package inet.linklayer.ieee80211.thenewmac.managementservice;

// 
// Declarations of MIB attributes exported from this process
//
simple Ieee80211MacMib
{
    parameters:
        string macmibPackage = default("^.^.macmib");
        
        string authenticationAlgorithms @enum("open_system", "shared_key");
        bool excludeUnencrypted = default(false);
        int fragmentationThreshold = default(2346);
        string groupAddresses = default("");
        int longRetryLimit = default(4);
        int maxReceiveLifetime = default(512); // Kusec
        int maxTransmitMsduLifetime = default(512); // Kusec
        int mediumOccupancyLimit = default(100); // Kusec
        bool privacyInvoked = default(false);
        bool mReceiveDTIMs = default(true);
        int cfpPeriod = default(1);
        int cfpMaxDuration = default(200); // Kusec
        int authenticationResponseTimeout = default(512); // Kusec
        int rtsThreshold = default(3000);
        int shortRetryLimit = default(7);
        int wepDefaultKeyId = default(0);
        int currentChannelNumber = default(0);
        int currentSet = default(0);
        int currentPattern = default(0);
        int currentIndex = default(0);
//        
//        bool cfPollable;
//        string macAddress;
//        string manufacturerId;
//        bool privacyOptionImplemented;
//        string productId;
//        string aStationId;
//        int wepKayMappingLength;
        
    gates:
        output mres;
        inout getSet;
}
