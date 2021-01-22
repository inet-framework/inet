//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

package inet.linklayer.ieee8021as;

simple Clock
{
    string clockType = default("ConstantDrift");
    int clockDrift = default(0);
}
