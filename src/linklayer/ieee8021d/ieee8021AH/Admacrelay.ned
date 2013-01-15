package inet.linklayer.ieee8021d.ieee8021AH;
//******************************************************
// @brief 802.1ah Admacrelay.
// Relay unit. Uses RSTP and MVRP modules and swtable information to determine the correct output gate for the incoming frames.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple Admacrelay
{
    parameters:
        bool verbose= default(false);   // sets the module verbosity     
    gates:
        output GatesOut[];	// To the backbone and the IComponents
        input GatesIn[];	// From the backbone and the IComponents
        inout RSTPGate;	// To the RSTP module
        inout MVRPGate;	// To the MVRP module
}
