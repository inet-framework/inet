//
// Rack that contains a set of blades
//
package inet.icancloud.Architecture.Aggregation;

module Rack
{
    parameters:
        @display("i=old/ipc");
        int numBlades;		// Number of blades.
        int nodesPerBlade;	// Number of nodes per blade
        
    gates:
        inout ethg[numBlades];

    submodules:
        blade[numBlades]: Blade {
            parameters:
                numNodes = nodesPerBlade;
                @display("i=inet.icancloud.nodeBoard/node_board;p=136,119,row,70");
        }

    connections:

        // Connections between Nodes and Switch
        for i=0..numBlades-1 {
            blade[i].ethg <--> ethg++;
        }
}


