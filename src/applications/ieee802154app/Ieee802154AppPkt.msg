
cplusplus {{
	using namespace std;
}}


packet Ieee802154AppPkt 
{
    string sourceName; 	// name of the source node
    string destName;		// name of the destination node
    simtime_t creationTime; // creation time of the packet
}
