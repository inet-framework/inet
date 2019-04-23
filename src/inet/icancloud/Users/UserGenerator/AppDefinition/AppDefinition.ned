// Module that implements an AppDefinition
//
// This module contains a vector of application definitions
//
// - A smart User Behavior.
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

package inet.icancloud.Users.UserGenerator.AppDefinition;

import inet.icancloud.Applications.Apps.AppBase;

module AppDefinition
{
    parameters:
        int appQuantity;
    submodules:
        application[appQuantity]: AppBase {
            parameters:
                @display("i=msg/job;p=98,129,row");
        }
}
