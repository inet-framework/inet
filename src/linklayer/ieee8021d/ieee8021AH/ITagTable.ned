package inet.linklayer.ieee8021d.ieee8021AH;
//******************************************************
// @brief 802.1ah I-Component cache.
// Stores the ISid/CMAC/BMAC and the Gate/ISid/SVid/CMAC asociation.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple ITagTable
{
    parameters:
		double agingTime @unit("s") = default(30s); // max idle time for table entries
                                     // (when it expires, entry is removed from the table)
        @display("i=block/table2");
}