//
//  Copyright (C) Mirco Musolesi University College London
//  m.musolesi@cs.ucl.ac.uk
//  Mirco Musolesi
//  Department of Computer Science - University College London
//  Gower Street London WC1E 6BT United Kingdom
//  Email: m.musolesi@cs.ucl.ac.uk
//  Phone: +44 20 7679 0391 Fax: +44 20 7387 1397
//  Web: http://www.cs.ucl.ac.uk/staff/m.musolesi
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.mobility.models;

//
// Mobility Patterns Generator for ns-2 simulator
// Based on a Community Based Mobility Model
//
// Mobilty based on the code released by Mirco Musolesi:
// http://portal.acm.org/citation.cfm?id=1132983.1132990
// "A community based mobility model for ad hoc network research"
// Ported to Omnet++ by Leonardo Maccari for the PAF-FPE project, see pervacy.eu
// leonardo.maccari@unitn.it
//
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

