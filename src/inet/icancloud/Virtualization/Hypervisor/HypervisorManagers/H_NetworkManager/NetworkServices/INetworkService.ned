// Module that represents a Generic Network Service.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Virtualization.Hypervisor.HypervisorManagers.H_NetworkManager.NetworkServices;

moduleinterface INetworkService

{
    parameters:
        string localIP;						// Local IP address

    gates:
        input fromNetManager;				// Input gate from Service Redirector
        input fromNetTCP;					// Input gate from Network (TCP)

        output toNetManager;				// Output gate to Service Redirector
        output toNetTCP;					// Output gate to Network (TCP)

}
