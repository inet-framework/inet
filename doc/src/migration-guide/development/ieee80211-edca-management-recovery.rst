IEEE 802.11 EDCA Management Recovery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The shared ``hcf.edca.mgmtAndNonQoSRecoveryProcedure`` module has moved to
``hcf.edca.edcaf[i].mgmtAndNonQoSRecoveryProcedure``. Each procedure now
updates its own EDCAF's contention window and management retry state.

Replace the old configuration path with
``hcf.edca.edcaf[*].mgmtAndNonQoSRecoveryProcedure`` to apply a setting to
all access categories, or select ``edcaf[3]`` for the current management
classification (voice). Update signal subscriptions and result paths in the
same way. Filter observations by source/AC; an ancestor subscription receives
independent recovery streams, including each instance's initialization sample.

C++ callers must use ``Edcaf::getMgmtAndNonQoSRecoveryProcedure()`` on the
affected EDCAF. The former ``Edca`` getter is removed. For internal collisions,
select the losing EDCAF, which need not be the channel owner.
