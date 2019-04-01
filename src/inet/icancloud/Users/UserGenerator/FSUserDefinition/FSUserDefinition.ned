//
// Module that implements a file system configuration given by a tenant for executing jobs
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

package inet.icancloud.Users.UserGenerator.FSUserDefinition;

simple FSUserDefinition
{
   parameters:
       @class(icancloud::FSUserDefinition);
       string type;
       string route;
}        
