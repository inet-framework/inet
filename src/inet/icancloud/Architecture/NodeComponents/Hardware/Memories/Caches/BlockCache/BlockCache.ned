// Module that implements a Generic Cache
//
// nullptr memory.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.Hardware.Memories.Caches.BlockCache;

import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.Caches.ICache;

simple BlockCache like ICache

{
    parameters:
    @class(icancloud::BlockCache);
        int numInputs;	// Number of inputs
        double hitRatio;	// Cache hit ratio [0..1]

    gates:
        input fromInput[];	// Input gates from imputs
        input fromOutput;		// Input gate from output

        output toInput[];		// Output gates to inputs
        output toOutput;		// Output gate to output

}

