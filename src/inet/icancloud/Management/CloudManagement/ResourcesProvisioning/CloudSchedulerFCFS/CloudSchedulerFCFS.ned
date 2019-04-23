// Module that implements a cloud scheduler
//
// This scheduler is a First Come First Serve
//
// @authors Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2014-12-11

       
package inet.icancloud.Management.CloudManagement.ResourcesProvisioning.CloudSchedulerFCFS;

import inet.icancloud.Management.CloudManagement.ResourcesProvisioning.CloudManagerScheduler;

module CloudSchedulerFCFS extends CloudManagerScheduler 
{
    parameters:
        
        @class(icancloud::CloudSchedulerFCFS);
        @display("i=msg/job");
        
}
