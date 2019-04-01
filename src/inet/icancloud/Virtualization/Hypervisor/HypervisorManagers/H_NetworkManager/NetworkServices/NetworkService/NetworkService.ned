// Module that implements a Generic Network Service.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.NetworkServices.NetworkService;

import inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.NetworkServices.INetworkService;

simple NetworkService like INetworkService

{
    parameters:
        @class(icancloud::NetworkService);
        string componentName = default ("Generic_NIC");		// Name of the component (memorization)
        string localIP;										// Local IP address

    gates:
        input fromNetManager;								// Input gate from Service Redirector
        input fromNetTCP;									// Input gate from Network (TCP)

        output toNetManager;								// Output gate to Service Redirector
        output toNetTCP;									// Output gate to Network (TCP)

}
