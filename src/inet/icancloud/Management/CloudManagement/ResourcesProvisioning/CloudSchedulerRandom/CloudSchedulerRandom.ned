// Module that implements a cloud scheduler
//
// This scheduler is a random policy. It selects a random node between all the data center to get the 
// requested resources.
//
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11
        
package inet.icancloud.Management.CloudManagement.ResourcesProvisioning.CloudSchedulerRandom;

import inet.icancloud.Management.CloudManagement.ResourcesProvisioning.CloudManagerScheduler;

module CloudSchedulerRandom extends CloudManagerScheduler
{
    parameters:		
        
        @class(icancloud::CloudSchedulerRandom);
        @display("i=msg/job");
        
}
