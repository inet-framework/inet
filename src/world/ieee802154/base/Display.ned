package inet.world.ieee802154.base;

// This class displays the communication radius of a host and provides
// the interference distance.
//
simple Display
{
    parameters:
        bool debug;        // debug switch
        double transmitterPower @unit("mW");
        double carrierFrequency @unit("Hz");
        double sat @unit("dBm");
        double sensitivity @unit("dBm");
        double alpha;
}
