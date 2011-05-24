package inet.underTest.wpan.Isabel.Base;
simple Display
{
    parameters:
        bool debug; 			// debug switch
        double transmitterPower @unit("mW");
        double carrierFrequency @unit("Hz");
        double sat @unit("dBm");
        double sensitivity @unit("dBm");
        double alpha;
}
