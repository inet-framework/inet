//
// This module defines a virtual machine messages controller
//
// All the messages that a virtual machine generates are controlled by this module. It has two main functionalites:
//   - The messages has a commId parameter. This parameter is setted by the applications. For example, when an application is distributed 
//     it is used to identify the rank of the source/destination process. This module, storage the original value for this parameter
//     and it is replaced with the position of the application at the gates vector. This is in order to know which is the owner of the message if 
//     it is responsed with a request.
//   - The second functionality (under developement) is the migration manager. It control the quantity of messages that are out of 
//     application in order to block the out messages and wait for the incoming messages to disconnect gates and reconnect to the destination
//     host when it would be possible
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2012-10-23
//


package inet.icancloud.Virtualization.VirtualMachines.VMManagement.VmMsgController;

simple VmMsgController
{
     @class(icancloud::VmMsgController);
    gates:
        input fromOSApps[];
	    input fromApps[];
        
        output toOSApps[];
        output toApps[];
}
