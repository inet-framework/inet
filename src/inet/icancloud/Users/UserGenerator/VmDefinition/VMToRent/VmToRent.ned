//
// Module that implements a virtual machine definition that tenants can hire.
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

package inet.icancloud.Users.UserGenerator.VmDefinition.VMToRent;

simple VmToRent
{
   parameters:
       @class(icancloud::VmToRent);
       string name;
       int quantity;
       
}