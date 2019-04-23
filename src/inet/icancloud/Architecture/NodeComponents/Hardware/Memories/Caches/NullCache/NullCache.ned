// Module that implements a Generic Cache
//
// nullptr memory.
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.Hardware.Memories.Caches.NullCache;

import inet.icancloud.Architecture.NodeComponents.Hardware.Memories.Caches.ICache;

simple NullCache like ICache

{
    parameters:
    @class(icancloud::NullCache);
        int numInputs;	// Number of inputs		
		
    gates:
        input fromInput[];	// Input gates from imputs
        input fromOutput;		// Input gate from output

        output toInput[];		// Output gates to inputs
        output toOutput;		// Output gate to output

}

