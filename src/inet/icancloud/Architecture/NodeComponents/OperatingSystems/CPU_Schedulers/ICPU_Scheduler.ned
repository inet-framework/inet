// Module that represents a Generic CPU Scheduler.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30


package inet.icancloud.Architecture.NodeComponents.OperatingSystems.CPU_Schedulers;

moduleinterface ICPU_Scheduler

{
    parameters:
        int numCPUs;			// Number of CPUs

        @display("i=block/cogwheel");
    gates:
        input fromOsGate;		// Input gate from Service Redirector
        input fromCPU[];					// Input gates from CPU

        output toOsGate;		// Output gate to Service Redirector
        output toCPU[];					// Output gates to CPU

}
