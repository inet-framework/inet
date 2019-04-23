//
// Module that know all the real ips from nodes and know all the virtual machines that each node has allocated.
// 
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11

package inet.icancloud.Management.NetworkManager;

simple NetworkManager
{
    parameters:
    @class(icancloud::NetworkManager);
    	@display("i=icons/Redirector");
    	@class(icancloud::NetworkManager);
}
