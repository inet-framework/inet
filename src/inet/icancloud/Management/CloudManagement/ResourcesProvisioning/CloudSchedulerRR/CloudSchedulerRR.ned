// Module that implements a cloud scheduler
//
// This scheduler follows a policy round robin
//
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11

package inet.icancloud.Management.CloudManagement.ResourcesProvisioning.CloudSchedulerRR;

import inet.icancloud.Management.CloudManagement.ResourcesProvisioning.CloudManagerScheduler;

module CloudSchedulerRR extends CloudManagerScheduler
{
    parameters:		
        
        @class(icancloud::CloudSchedulerRR);
        @display("i=msg/job");
        
}
