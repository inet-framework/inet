PCP of the 802.1Q Tag Inserters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The 802.1Q tag inserters, :ned:`Ieee8021qTagEpdHeaderInserter` and
:ned:`Ieee8021qTagTpidHeaderInserter`, take the PCP of their tag in three steps
now, and each step has a parameter:

1. :par:`mapUserPriorityReqToPcpReq` (``true`` by default): a
   :cpp:`UserPriorityReq` becomes the PCP request of the tag type of the
   inserter, unless the packet has one already. :par:`removeUserPriorityReq`
   (``false`` by default) removes the :cpp:`UserPriorityReq` from the packet in
   this step.
2. The PCP comes from the PCP request, :cpp:`CPcpReq` or :cpp:`SPcpReq`, or from
   :par:`defaultPcp`. The inserter writes it into the tag and adds the PCP
   indication, :cpp:`CPcpInd` or :cpp:`SPcpInd`. A :par:`defaultPcp` of -1 leaves
   the PCP unspecified: the tag gets PCP 0, and the packet gets no PCP
   indication.
3. :par:`mapPcpIndToUserPriorityInd` (``true`` by default): the PCP indication
   becomes the :cpp:`UserPriorityInd` of the packet, which, for example,
   :ned:`EthernetPriorityQueue` classifies by.

The :par:`defaultUserPriority` parameter is removed. Set :par:`defaultPcp`
instead. With :par:`mapPcpIndToUserPriorityInd`, the frame gets the same user
priority indication as before:

.. code-block:: ini

   # before
   **.qTagHeaderInserter.defaultUserPriority = 5

   # after
   **.qTagHeaderInserter.defaultPcp = 5

A model that relied on one of these behaviors gets different results:

- A user priority request takes precedence over :par:`defaultPcp`. Before,
  :par:`defaultPcp` overrode it.
- A PCP that comes from the user priority is indicated as a PCP. Before, a
  :ned:`PcpClassifier` or :ned:`PcpTrafficClassClassifier` below the inserter
  found no PCP for the frame and used its :par:`defaultGateIndex`. Now it uses
  the PCP.
- The user priority indication carries the PCP of the tag. Before, it carried
  the requested user priority, also when a PCP request overrode it. In a node
  that inserts a C-tag and an S-tag, it is the PCP of the S-tag, whose inserter
  runs last. To indicate the PCP of the C-tag, set
  :par:`mapPcpIndToUserPriorityInd` of the S-tag inserter to ``false``.
- The inserters of INET 4.7 removed the user priority request, so the layers
  below did not get it. To keep that, set :par:`removeUserPriorityReq` to
  ``true``.
- A PCP request, a user priority request that the inserter uses, or a
  :par:`defaultPcp` outside [0, 7] stops the simulation with an error, and so
  does :par:`removeUserPriorityReq` without :par:`mapUserPriorityReqToPcpReq`.
  Before, the inserter put such a value into the tag, and the serializer cut it
  to the three bits of the PCP field.
