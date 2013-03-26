#include <algorithm>   // min,max
#include "TCPVegas.h"
#include "TCP.h"


Register_Class(TCPVegas);

TCPVegasStateVariables::TCPVegasStateVariables()
{
    ssthresh = 65535;
    v_recoverypoint = 0;
//    v_last_cwnd_action = 0;
    v_cwnd_changed = 0;
    
    v_baseRTT = 0x7fffffff;
	
	v_inc_flag = true;
	v_incr_ss = false;
	v_incr = 0;
	
	v_worried = 0;

	v_begseq = 0;
	v_begtime = 0;
	v_cntRTT = 0;
	v_sumRTT = 0.0;
	v_rtt_timeout = 1000.0;

	v_sendtime = NULL;
	v_transmits = NULL;
	v_maxwnd = 0;
}

TCPVegasStateVariables::~TCPVegasStateVariables()
{
    delete [] v_sendtime;
    delete [] v_transmits;
}

std::string TCPVegasStateVariables::info() const
{
    std::stringstream out;
    out << TCPBaseAlgStateVariables::info();
    out << " ssthresh=" << ssthresh;
    return out.str();
}

std::string TCPVegasStateVariables::detailedInfo() const
{
    std::stringstream out;
    out << TCPBaseAlgStateVariables::detailedInfo();
    out << "ssthresh = " << ssthresh << "\n";
	out << "baseRTT = " << v_baseRTT << "\n";
    return out.str();
}


TCPVegas::TCPVegas() : TCPBaseAlg(),
  state((TCPVegasStateVariables *&)TCPAlgorithm::state)
{
}
 
// Check rtt timer to see if should rtx or not (when an ack is received)
bool TCPVegas::checkRTTTimer() {

	// Check if time since oldest segment (snd_una) was sent exceeds
	// Vegas timeout
    simtime_t tSent = state->v_sendtime[(state->snd_una - (state->iss+1)) % state->v_maxwnd];      
	simtime_t currentTime = simTime();

	simtime_t elapse = currentTime - tSent;
	if(elapse >= state->v_rtt_timeout)	
		return true;
	else
		return false;	
}


// Same as TCPReno
void TCPVegas::recalculateSlowStartThreshold()
{
    // RFC 2581, page 4:
    // "When a TCP sender detects segment loss using the retransmission
    // timer, the value of ssthresh MUST be set to no more than the value
    // given in equation 3:
    //
    //   ssthresh = max (FlightSize / 2, 2*SMSS)            (3)
    //
    // As discussed above, FlightSize is the amount of outstanding data in
    // the network."

    // set ssthresh to flight size/2, but at least 2 SMSS
    // (the formula below practically amounts to ssthresh=cwnd/2 most of the time)
    uint32 flight_size = std::min(state->snd_cwnd, state->snd_wnd); // FIXME TODO - Does this formula computes the amount of outstanding data?
    // uint32 flight_size = state->snd_max - state->snd_una;
    state->ssthresh = std::max(flight_size/2, 2*state->snd_mss);
    if (ssthreshVector)
    	ssthreshVector->record(state->ssthresh);
}


//Process rexmit timer
void TCPVegas::processRexmitTimer(TCPEventCode& event)
{

	TCPBaseAlg::processRexmitTimer(event);

	if (event==TCP_E_ABORT)
        return;

    recalculateSlowStartThreshold();
	// Vegas: when rtx timeout: cwnd = 2*smss, instead of 1*smss (Reno)
    state->snd_cwnd = 2 * state->snd_mss;
    
	if (cwndVector) cwndVector->record(state->snd_cwnd);
    tcpEV << "RXT Timeout in Vegas: resetting cwnd to " << state->snd_cwnd << "\n"
          << ", ssthresh=" << state->ssthresh << "\n";
    
	state->v_cwnd_changed = simTime(); // Save time when cwdn changes due to rtx

	state->afterRto = true;
	++state->v_transmits[(state->snd_una - (state->iss+1)) % state->v_maxwnd];
	conn->retransmitOneSegment(true); //retransmit one segment from snd_una
        
}


void TCPVegas::receivedDataAck(uint32 firstSeqAcked) 
{
	TCPBaseAlg::receivedDataAck(firstSeqAcked);

	//FIXME: sometimes the v_sendtime is uninitialized here!!! ---> crash
if (state->v_sendtime == NULL) {
    EV << "Received ACK, but v_sendtime is NULL";
}
else
{
	simtime_t tSent = state->v_sendtime[(firstSeqAcked - (state->iss+1)) % state->v_maxwnd];    
	simtime_t currentTime = simTime();

    uint32 v_beta = 4 * state->snd_mss; // Value in bytes is needed, instead ofnum. packets
	uint32 v_alpha = 2 * state->snd_mss;
	uint32 v_gamma = 1 * state->snd_mss;	
	
	if (firstSeqAcked == state->iss+1) { // Inicialization
		state->v_baseRTT = currentTime - tSent;
		state->v_sa = state->v_baseRTT * 8;
		state->v_sd = state->v_baseRTT;
		state->v_rtt_timeout = ((state->v_sa / 4.0) + state->v_sd) / 2.0;
		tcpEV << "Vegas: initialization" << "\n";
	}
	

 	// Once per RTT
	if (state->snd_una > state->v_begseq) {		
		simtime_t newRTT;
		if(state->v_cntRTT > 0) {
			newRTT = state->v_sumRTT / state->v_cntRTT;
			tcpEV << "Vegas: newRTT (state->v_sumRTT / state->v_cntRTT) calculated: " << state->v_sumRTT / state->v_cntRTT << "\n";
		}
		else {
			newRTT = currentTime - state->v_begtime;
			tcpEV << "Vegas: newRTT calculated: " << newRTT << "\n";		
		}
		state->v_sumRTT = 0.0;
		state->v_cntRTT = 0;
 
	
    	// decide if incr/decr cwnd
		if (newRTT > 0) {

			// rttLen: bytes transmitted since a segment is sent and its ack is received
			uint32 rttLen = state->snd_nxt - state->v_begseq; 

        	
			// If only one packet in transit: update baseRTT
			if (rttLen <= state->snd_mss)
    	    	state->v_baseRTT = newRTT;
    	   		  
        	// actual = rttLen/(current_rtt)
        	uint32 actual = rttLen/newRTT;
        
			// expected = (current window size)/baseRTT
			uint32 expected;
			uint32 acked = state->snd_una - firstSeqAcked;
			expected = (uint32)((state->snd_nxt - firstSeqAcked) + std::min(state->snd_mss - acked, (uint32)0)) / state->v_baseRTT;
		   
			// diff = expected - actual
			uint32 diff;
			double baseRTT_double = conn->convertSimtimeToTS(state->v_baseRTT); 
			// convert v_baseRTT to double
			diff = (uint32)(expected - actual) * (baseRTT_double/1000) + 0.5;
	
			tcpEV << "Vegas: expected: " << expected << "\n"
          		<< ", actual =" << actual << "\n"
	      		<< ", diff =" << diff << "\n";
		   
			// Slow start
			state->v_incr_ss = false; // reset
	    	if (state->snd_cwnd < state->ssthresh) { //Slow start
				tcpEV << "Vegas: Slow Start: " << "\n";
		    	
				// cwnd modification during slow-start only every 2 rtt
		    	state->v_inc_flag = !state->v_inc_flag; 
		    	if (!state->v_inc_flag) {
 			    	state->v_incr = 0;				
				}
            	else {
					if (diff > v_gamma) {
						/* When actual rate falls below expected rate a certain value (gamma threshold)
						Vegas changes from slow start to linear incr/decr*/
   	  		        	
   	  		       		recalculateSlowStartThreshold(); // to enter again in cong. avoidance  	  		       		
						state->snd_cwnd -= (state->snd_cwnd / 8);
   	  		       	  
   	  		       		if (state->snd_cwnd < 2*state->snd_mss)
  	  				    	state->snd_cwnd = 2*state->snd_mss;
 				      
                    	state->v_incr = 0;
 				      
 				    	if (cwndVector) cwndVector->record(state->snd_cwnd);
 				      
					} else {
                		state->v_incr = state->snd_mss; //incr 1 segment
						state->v_incr_ss = true;
					}
				}
        	} // end slow start
       		else {
            	// Cong. avoidance
				tcpEV << "Vegas: Congestion avoidance: " << "\n";
			
            	if (diff > v_beta) {
					state->v_incr = -state->snd_mss;
				}
				else if (diff < v_alpha) {
					state->v_incr = state->snd_mss;
				}
				else
					state->v_incr = 0;
			} // end cong. avoidance			
		} 
		// register beqseq and begtime values for next rtt
		state->v_begtime = currentTime;
		state->v_begseq = state->snd_nxt;

	} // end 'once per rtt' section
		

	// if cwnd > ssthresh, no incr
    if (state->v_incr_ss && state->snd_cwnd >= state->ssthresh) {
		state->v_incr = 0;
		tcpEV << "Vegas: surpass ssthresh during slow-start: no cwnd incr. " << "\n";	
	}
		   
	//incr/decr cwnd
	if (state->v_incr > 0) {
		state->snd_cwnd += state->v_incr;

		if (cwndVector) cwndVector->record(state->snd_cwnd);
		tcpEV << "Vegas: incr cwnd linearly, to " << state->snd_cwnd << "\n";
	}
	else if (state->v_incr < 0) {
		state->snd_cwnd += state->v_incr;
		if (state->snd_cwnd < 2*state->snd_mss)
  	  		state->snd_cwnd = 2*state->snd_mss;
					  
		if (cwndVector) cwndVector->record(state->snd_cwnd);
		tcpEV << "Vegas: decr cwnd linearly, to " << state->snd_cwnd << "\n";
	}

	int num_transmits = state->v_transmits[(firstSeqAcked - (state->iss+1)) % state->v_maxwnd];
	// reset v_sendtime for acked segments
	uint32 range = state->snd_una - firstSeqAcked;
	for(uint32 k = firstSeqAcked; k < state->snd_una && range > 0; k++, range--) {
		state->v_sendtime[(k - (state->iss+1)) % state->v_maxwnd] = -1;
		state->v_transmits[(k - (state->iss+1)) % state->v_maxwnd] = 0;
	}

		    
	// update vegas fine-grained timeout value (retransmitted packets do not count)
	if (tSent != 0 && num_transmits == 1) {
		simtime_t newRTT = currentTime - tSent;
		state->v_sumRTT += newRTT;
		++state->v_cntRTT;

		if (newRTT > 0) {
			if(newRTT < state->v_baseRTT)
				state->v_baseRTT = newRTT;

			simtime_t n = newRTT - state->v_sa/8;
			state->v_sa += n;
			n = n < 0 ? -n : n;
			n -= state->v_sd / 4;
			state->v_sd += n;
			state->v_rtt_timeout = ((state->v_sa / 4) + state->v_sd) / 2;
			state->v_rtt_timeout += (state->v_rtt_timeout / 16);
			tcpEV << "Vegas: new v_rtt_timeout = " << state->v_rtt_timeout << "\n";		
		}
	}


	// check 1st and 2nd ack adter a rtx
    if (state->v_worried > 0){
		state->v_worried -= state->snd_mss;
		bool expired = checkRTTTimer();
		// added comprobation to check that received ACK do not acks all outstanding data. If not, 
		// TCPConnection::retransmitOneSegment will fail: ASSERT(bytes!=0), line 839), because bytes = snd_max-snd_una
		if (expired && (state->snd_max - state->snd_una > 0)) {
			state->dupacks = DUPTHRESH;
			++state->v_transmits[(state->snd_una - (state->iss+1)) % state->v_maxwnd];
			tcpEV << "Vegas: retransmission (v_rtt_timeout) " << "\n";
			conn->retransmitOneSegment(false); //retransmit one segment from snd_una	
		}
		else
			state->v_worried = 0;
	}

} // Closes if v_sendtime != NULL
		
	//Try to send more data
    sendData(false);
}


void TCPVegas::receivedDuplicateAck()
{
	TCPBaseAlg::receivedDuplicateAck();

	
	simtime_t tSent = state->v_sendtime[(state->snd_una - (state->iss+1)) % state->v_maxwnd];      
	simtime_t currentTime = simTime();
	int num_transmits = state->v_transmits[(state->snd_una - (state->iss+1)) % state->v_maxwnd];

	// check Vegas timeout
	bool expired = checkRTTTimer();
	

	// rtx if Vegas timeout || 3 dupacks
	if (expired || state->dupacks == DUPTHRESH){ //DUPTHRESH = 3

			uint32 win = std::min(state->snd_cwnd, state->snd_wnd);
			
			state->v_worried = std::min((uint32)2*state->snd_mss, state->snd_nxt - state->snd_una);

			if (num_transmits > 1)
				state->v_rtt_timeout *= 2; // exp. Backoff
			else
				state->v_rtt_timeout += (state->v_rtt_timeout / 8.0);
						
			
			// Vegas reduces cwnd if retransmitted segment rtx. was sent after last cwnd reduction
			if (state->v_cwnd_changed < tSent) {
				win = win/state->snd_mss;
				
				if (win <= 3)
					win = 2;
				else if (num_transmits > 1)
					win = win/2; //win <<= 1
				else win -= win/4; // win -= (win >>2)

				state->snd_cwnd = win * state->snd_mss + 3 * state->snd_mss;
        		
        		state->v_cwnd_changed = currentTime;

        		if (cwndVector)
            		cwndVector->record(state->snd_cwnd);
				
				// reset rtx. timer
				restartRexmitTimer();

			}
			
			// retransmit one segment from snd_una
			++state->v_transmits[(state->snd_una - (state->iss+1)) % state->v_maxwnd];
        	conn->retransmitOneSegment(false); 

			if(num_transmits == 1) {
				state->dupacks = DUPTHRESH;			
			}
		
		
	}
	//else if dupacks > duphtresh, cwnd+1
	else if (state->dupacks > DUPTHRESH) { // DUPTHRESH = 3
		state->snd_cwnd += state->snd_mss;
        if (cwndVector) cwndVector->record(state->snd_cwnd);
	}
	
	// try to send more data
    sendData(false);
}

void TCPVegas::dataSent(uint32 fromseq) {

	TCPBaseAlg::dataSent(fromseq);
	
	// If 1st packet, initialization
	if (fromseq == state->iss+1) {
		// rcv_wnd: max capacity of the receiver buffer
		state->v_maxwnd = state->rcv_wnd; 
		
		state->v_sendtime = new simtime_t[state->v_maxwnd];
		state->v_transmits = new int[state->v_maxwnd]; 
		for(int i = 0; i < state->v_maxwnd; i++) {
			state->v_sendtime[i] = -1;
			state->v_transmits[i] = 0;		
		}
	}

	
	// save time when packet is sent
	// fromseq is the seq number of the 1st sent byte
	// we need this value, based on iss=0 (to store it the right way on the vector),
	// but iss is not a constant value (ej: iss=0), so it needs to be detemined each time 
	// (this is why it is used: fromseq-state->iss)

	simtime_t sendtime = simTime();
	for(uint32 i = fromseq; i < state->snd_max; i++) {
		int index = (i - (state->iss+1)) % state->v_maxwnd;
		state->v_sendtime[index] = sendtime;
		++state->v_transmits[index];
	}
	
	
}


