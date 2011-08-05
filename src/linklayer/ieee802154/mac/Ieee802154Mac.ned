
package inet.linklayer.ieee802154.mac;

simple Ieee802154Mac
{
    parameters:
        // debug switch
        bool debug;					// debug switch
        // whether is PAN coordinator
        bool isPANCoor;					// whether is PAN coordinator
        // beacon order [1, 14]
        double BO;			// beacon order [1, 14]
        // superframe order [0, BO)
        double SO;			// superframe order [0, BO)
        double dataTransMode;			// 1: direct; 2: indirect; 3: GTS
        // name of optional external queue module
        string queueModule;				// name of optional external queue module
        string panCoorName;				// name of the PAN coordinator
        // transmit or receive in GTS
        bool isRecvGTS;					// transmit or receive in GTS
        // payload of data frames transmitted in GTS, copied from traffic module
        double gtsPayload;			// payload of data frames transmitted in GTS, copied from traffic module
        // whether acking for Data pkts received in GTS
        bool ack4Gts;					// whether acking for Data pkts received in GTS
        string address = default("auto");
        @display("i=block/layer");
    gates:
        input upperLayerIn; 			// from the upper layer
        output upperLayerOut; 		// to the upper layer
        input lowerLayerIn; 			// from the physical layer
        output lowerLayerOut; 			// to the physical layer
}
