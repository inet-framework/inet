package inet.linklayer.ieee8021d.ieee8021Q;
//******************************************************
// @brief 802.1Q cache
// Stores the Vid/MAC/Gate cache
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple Cache1Q
{
    parameters:
        @display("i=block/table2");
        double agingTime @unit("s")=default(30s);
        bool verbose=default(false);
}