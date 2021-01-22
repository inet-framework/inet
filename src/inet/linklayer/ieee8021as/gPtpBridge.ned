//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package inet.linklayer.ieee8021as;

import inet.node.ethernet.EthernetSwitch;


module GPtpBridge extends EthernetSwitch
{
    parameters:
        // 11 - Grandmaster, 12 - Bridge, 13 - Slave
        int gPtpNodeType = 12;
        eth[*].typename = "EthernetInterfaceGPTP";
    submodules:
        tableGPTP: TableGPTP {
            @display("p=920,100");
        }
        clockGPTP: Clock {
            @display("p=100,601");
        }
}

