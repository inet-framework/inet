
// BGP Version 4 - RFC 4271
// NOTIFICATION message is not implemented
// MinASOriginationIntervalTimer and MinRouteAdvertisementIntervalTimer are not implemented
// Optional UPDATE Msg Path Attributes are not implemented
// Optional Final State Machine events are not implemented
//
//3. Summary of Operation (Routes & routing Information Base) -- OK
//4. Msg Format :   Header, OPEN, UPDATE & KEEPALIVE -- OK 
//                          NOTIFICATION -- NOK
//5. Path Attributes: ORIGIN, AS_PATH, NEXT_HOP -- 0K
//                          MULTI_EXIT_DISC, LOCAL_PREF, ATOMIC_AGGRGATE, AGGREGATOR -- NOK
//6. BGP Error Handling -- NOK
//7. BGP Version Negotiation -- NOK
//8. Event for the BGP FSM -- OK exept optional
//9. UPDATE Message Handling:
//          Decision Process -- OK
//          Update-Send Process -- OK exept Controlling Routing Traffic Overhead
//10. BGP timers 
//          ConnectRetryTimer, Holdtimer, KeepAliveTimer -- OK
//          MinASOriginationIntervalTimer and MinRouteAdvertisementIntervalTimer -- NOK

package inet.networklayer.bgpv4;

simple BGPRouting
{
    parameters:
        string bgpConfigFile;
		
    gates:
        input fromTCP;
		output toTCP;
}
