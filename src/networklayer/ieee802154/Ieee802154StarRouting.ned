package inet.underTest.wpan.networklayer.Ieee802154StarRouting;
simple Ieee802154StarRouting
{
    parameters:
        // debug switch
        bool debug; 				// debug switch
        bool isPANCoor;
    gates:
        input uppergateIn; 			// from the upper layer
        output uppergateOut; 		// to the upper layer
        input lowergateIn; 			// from the physical layer
        output lowergateOut; 			// to the physical layer
}
