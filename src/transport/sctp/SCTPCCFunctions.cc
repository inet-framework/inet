//
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "SCTPAssociation.h"


void SCTPAssociation::initCCParameters(SCTPPathVariables* path)
{
	path->cwnd = (int32)min(4 * path->pmtu, max(2 * path->pmtu, 4380));
	path->pathCwnd->record(path->cwnd);
	path->cwndAdjustmentTime = simulation.getSimTime();
	path->ssthresh = state->peerRwnd;
	path->pathSsthresh->record(path->ssthresh);
}



void SCTPAssociation::cwndUpdateAfterSack(bool rtxNecessary, SCTPPathVariables* path)
{
	if (rtxNecessary == true && state->fastRecoveryActive == false) 
	{
		for (SCTPPathMap::iterator iter=sctpPathMap.begin(); iter!=sctpPathMap.end(); iter++)
		{	
			path=iter->second;
			if (path->requiresRtx) 
			{
				path->ssthresh = (int32)max(path->cwnd / 2,  4 * path->pmtu);
				path->pathSsthresh->record(path->ssthresh);;
				path->cwnd = path->ssthresh;
				path->pathCwnd->record(path->cwnd);
				path->partialBytesAcked = 0;
				/* get and store the time when CWND was adjusted here */
				path->cwndAdjustmentTime = simulation.getSimTime();
				
				if (state->fastRecoverySupported) 
				{
					state->highestTsnAcked = state->lastTsnAck;
					for (SCTPQueue::PayloadQueue::iterator pq=retransmissionQ->payloadQueue.begin(); pq!=retransmissionQ->payloadQueue.end(); pq++)
					{
						if (pq->second->hasBeenAcked == true && tsnGt(pq->second->tsn, state->highestTsnAcked)) 
						{
							state->highestTsnAcked = pq->second->tsn;
						}
					}
					/* this can ONLY become TRUE, when Fast Recovery IS supported */
					state->fastRecoveryActive = true;
					state->fastRecoveryExitPoint = state->highestTsnAcked;
					 
					sctpEV3<<"===> ENTERING FAST RECOVERY.....Exit Point == "<< state->fastRecoveryExitPoint<<"\n";
					
					
				} /* end: if (fast_recovery_supported */
			}	  /* end: if (path_requires_rtx... 	*/
		}		  /* end: for (counter...  			*/
	} 			  /* end: if (rtx_necessary			*/    
	
}

void SCTPAssociation::cwndUpdateAfterCwndTimeout(SCTPPathVariables* path)
{
	/* 
	 *  When the association does not transmit data on a given transport address
	 *  within an RTO, the cwnd of the transport address SHOULD be adjusted to 2*MTU.
	 */

	path->cwnd = (int32)min(4 * path->pmtu, max(2 * path->pmtu, 4380));
	path->pathCwnd->record(path->cwnd);
	 
	sctpEV3<<"CWND timer run off: readjusting CWND(path=="<<path->remoteAddress<<") = "<<path->cwnd<<"\n";
	
	path->cwndAdjustmentTime = simulation.getSimTime();
}

void SCTPAssociation::cwndUpdateAfterRtxTimeout(SCTPPathVariables* path)
{
	/* implement optional FAST RECOVERY */
	if (state->fastRecoveryActive == false) 
	{
		path->ssthresh = (int32)max(path->cwnd / 2, 4 * path->pmtu);

		path->pathSsthresh->record(path->ssthresh);
		path->cwnd = path->pmtu;
		path->pathCwnd->record(path->cwnd);
		path->partialBytesAcked = 0;

		path->cwndAdjustmentTime = simulation.getSimTime();

	}
}


void SCTPAssociation::cwndUpdateMaxBurst(SCTPPathVariables* path)
{
	if(path->cwnd > ((path->outstandingBytes + (uint32)sctpMain->par("maxBurst") * path->pmtu))) 
	{
		path->cwnd = path->outstandingBytes + ((uint32)sctpMain->par("maxBurst") * path->pmtu);
		path->pathCwnd->record(path->cwnd); 
		path->cwndAdjustmentTime = simulation.getSimTime();
	}
}


void SCTPAssociation::cwndUpdateBytesAcked(uint32 ackedBytes, uint32 osb, bool ctsnaAdvanced, IPvXAddress pathId, uint32 pathOsb, uint32 newOsb)
{
	SCTPPathVariables* path=getPath(pathId);	
	
	 
	sctpEV3<<simulation.getSimTime()<<" fcAdjustCounters: path="<<pathId<<" cwnd="<<path->cwnd<<" ssthresh="<<path->ssthresh<<" pathOsbBefore="<<pathOsb<<" osbBefore="<<osb<<" ackedBytes="<<ackedBytes<<"\n";
	
	
	if (path->cwnd <=path->ssthresh) 
	{	
		// SLOW START 
		for (SCTPPathMap::iterator iter=sctpPathMap.begin(); iter!=sctpPathMap.end(); iter++)
		{
			iter->second->partialBytesAcked = 0;
		}

		 
		sctpEV3<<"ctsnaAdvanced="<<ctsnaAdvanced<<"  fastRecoveryActive="<<state->fastRecoveryActive<<"\n";
		sctpEV3<<simulation.getSimTime()<<" primaryPath="<<state->primaryPathIndex<<", pathOsb="<<pathOsb<<", cwnd="<<path->cwnd<<"\n";
		if ((ctsnaAdvanced == true && pathOsb >= path->cwnd))
		{
			path->cwndAdjustmentTime = simulation.getSimTime();
			
			path->cwnd += (int32)min(path->pmtu, ackedBytes);
			path->pathCwnd->record(path->cwnd);
			 
			sctpEV3<<simulation.getSimTime()<<" SLOW START: Setting CWND of path "<<pathId<<" to "<<path->cwnd<<" bytes. Osb = "<<pathOsb<<"\n";
			
		}
		
	} 
	else 
	{
		// CONGESTION AVOIDANCE 
		path->partialBytesAcked += ackedBytes;
	sctpEV3<<"partialBytesAcked="<<path->partialBytesAcked<<"  cwnd="<<path->cwnd<<"  osb="<<osb<<" advanced="<<ctsnaAdvanced<<"  fastRecoveryActive="<<state->fastRecoveryActive<<"\n";
		if ((path->partialBytesAcked  >= path->cwnd) &&
			(osb >= path->cwnd) && (ctsnaAdvanced == true)) 
		{		
			path->cwnd += path->pmtu;
			path->pathCwnd->record(path->cwnd);
			path->partialBytesAcked = 
				((path->cwnd < path->partialBytesAcked) ?
					(path->partialBytesAcked - path->cwnd) : 0);

			/* get and store the time when CWND was adjusted here */
			path->cwndAdjustmentTime = simulation.getSimTime();
			
			 
			sctpEV3<<"CONG.AVOID.: Setting CWND of path "<<pathId<<" to "<<path->cwnd<<" bytes\n";
			
		}
	}
			

	if (newOsb == 0) path->partialBytesAcked = 0;
}

