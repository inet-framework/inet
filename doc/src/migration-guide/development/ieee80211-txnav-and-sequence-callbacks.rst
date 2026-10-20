IEEE 802.11 TXNAV and Sequence Callbacks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Custom ``IRx`` implementations must provide these operations:

.. code-block:: c++

   simtime_t getTxnavRemaining() const override;
   void successfulFrameTransmitted(simtime_t ppduEnd, simtime_t durationField) override;
   void setContentionBlocked(bool blocked) override;

Keep TXNAV separate from received NAV. Replace its endpoint with ``ppduEnd + durationField`` after successful holder transmission. Cancel TXNAV for zero or expired fields. Preserve TXNAV after TXOP end. Exclude contention while TXNAV remains active. Keep ``isMediumFree()`` independent of TXNAV for the CTS policy and radio configuration.

Honor HCF's separate contention permission through ``setContentionBlocked()``. HCF blocks contention after internal-collision handling and restores it after channel release and TXOP end. This permission prevents a second grant during an initial response wait, when successful TXNAV does not yet exist. Preserve the CTS medium query.

Custom single-protection admission must compare the complete exchange cost strictly against TXNAV at the decision before SIFS. Exclude the initial SIFS from that exchange cost. Include the initial SIFS in the separate positive TXOP limit check. For example, a 238 us DATA/ACK exchange passes with 246.67 us TXNAV and at least 248 us of positive TXOP time. This example uses 10 us initial SIFS. The timer sample instant follows INET's stated interpretation of IEEE Std 802.11-2024, Clause 10.23.2.8.

Custom sequence callbacks must provide these signatures:

.. code-block:: c++

   void frameSequenceStarted() override;
   bool transmitFrame(Packet *packet, simtime_t ifs) override;

Emit the start observation from ``frameSequenceStarted()``. The handler installs its context and initializes the sequence before this call. Return true after submission to Tx. Return false before submission to end the sequence normally. The handler destroys the refused step before finish observers inspect history. An empty preparation or immediate refusal therefore produces one start and one finish with valid contexts.

TxOpFs publishes the selected DATA ACK policy before execution. HCF consumes that policy without a second DATA-submission query. Custom ACK policies must not require effects from that removed query. Their method signatures remain unchanged.
