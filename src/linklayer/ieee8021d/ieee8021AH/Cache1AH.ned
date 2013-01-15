package inet.linklayer.ieee8021d.ieee8021AH;
//******************************************************
// @brief 802.1ah B-Component cache.
// Stores the Vid/MAC/Gate cache
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple Cache1AH
{
    parameters:
        @display("i=block/table2");
        bool verbose = default(false);
        double agingTime @unit("s") = default(30s);
        xml ISIDConfig = default(xmldoc("ISIDConfig.xml"));
}
