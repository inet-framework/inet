// Module that implements a General user behavior.
//
// This module contains the following subModules:
//
// - A general user behaviour
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30
package inet.icancloud.Users.Profiles.GeneralUser;

simple GeneralUser
{
    parameters:
        bool printVMs = default (true);
        bool printJobs = default (true);
        @class(icancloud::GeneralUser);
        @display("i=abstract/person");
        
}
