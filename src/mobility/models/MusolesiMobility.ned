//

package inet.mobility.models;
import inet.mobility.IMobility;

//
// This is a package generating Mobility as described by Musolesi in
// http://portal.acm.org/citation.cfm?id=1132983.1132990 
// (A community based mobility model for ad hoc network research)
// Code for off-line traces has been produced by Musolesi, Leonardo Maccari
// ported it to Omnet++

simple MusolesiMobility extends MobilityBase
{
    parameters:
      @display("i=block/cogwheel_s");
      @class(MusolesiMobility);
      @statistic[blocksHistogram](title="Blocks histogram"; record=histogram, vector);
      @statistic[walkedMeters](title="Walked Meters"; record=sum);
      @statistic[blockChanges](title="Block changes"; record=sum);
      
      bool obstacleAvoidance = default(false);

      double minHostSpeed = default(0.1);
      double maxHostSpeed = default(10);
      double connectionThreshold = default(0.1); 
      // below this treshold there is no connection
      double recordStartTime @unit("s") = default(10s); // time when we begin recording statistics

      int numberOfRows = default(10);
      int numberOfColumns = default(10);
      double rewiringProb = default(0.1);
      double rewiringPeriod @unit("s") = default(0); // rewiring interval
      double reshufflePeriod @unit("s") = default(0); // reshuffling interval
      int numberOfGroups = default(10); 
      bool girvanNewmanOn = default(false);
      string targetChoice = default("pseudodeterministic");
      bool recordStatistics = default(false);
      double drift = default(0.1); 
      double expmean = default(1); 

      bool reshufflePositionsOnly = default(false); // do not update group
                            //composition, only the position of the group

      bool RWP = default(false); // disable group movements, just a RWP. This
                    //has been added just to perform statistical comparisons

      int numHosts;
      double hcmm = default(0); // do we use Boldrini/Conti/Passarella? what is
                               // the probability of going back home?

      double x = default(-1); // start x coordinate (-1 = display string position, or random if it's missing)
      double y = default(-1); // start y coordinate (-1 = display string position, or random if it's missing)
      double updateInterval @unit("s") = default(100ms); // time interval to update the hosts position      
    
}

