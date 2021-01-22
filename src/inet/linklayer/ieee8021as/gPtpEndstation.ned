//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package inet.linklayer.ieee8021as;

import inet.node.inet.StandardHost;


module GPtpEndstation extends StandardHost
{
    parameters:
        eth[*].typename = "EthernetInterfaceGPTP";
    submodules:
        clockGPTP: Clock {
            @display("p=125,738");
        }
        tableGPTP: TableGPTP {
            @display("p=558,259");
        }
}

