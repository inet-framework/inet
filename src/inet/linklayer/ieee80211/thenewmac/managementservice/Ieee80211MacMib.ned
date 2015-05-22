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
        bool excludeUnencrypted;
        int fragmentationThreshold;
        string groupAddresses;
        int longRetryLimit;
        double maxReceiveLifetime;
        double maxTransmitMsduLifetime;
        double mediumOccupancyLimit;
        bool privacyInvoked;
        bool mReceiveDTIMs;
        int cfpPeriod;
        double cfpMaxDuration;
        double authenticationResponseTimeout;
        int rtsThreshold;
        int shortRetryLimit;
        int wepDefaultKeyId;
        int currentChannelNumber;
        int currentSet;
        int currentPattern;
        int currentIndex;
        
    gates:
        output mres;
}
