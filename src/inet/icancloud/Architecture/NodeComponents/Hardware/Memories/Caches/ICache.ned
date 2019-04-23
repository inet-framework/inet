// Module that represents a Generic Cache
//
// @author Alberto N&uacute;&ntilde;ez Covarrubias
// @date 2010-11-30

package inet.icancloud.Architecture.NodeComponents.Hardware.Memories.Caches;

moduleinterface ICache

{
    parameters:
        int numInputs;	// Number of inputs

    gates:
        input fromInput[];	// Input gates from imputs
        input fromOutput;		// Input gate from output

        output toInput[];		// Output gates to inputs
        output toOutput;		// Output gate to output

}

