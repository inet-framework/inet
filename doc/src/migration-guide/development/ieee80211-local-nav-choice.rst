IEEE 802.11 Local NAV Choice
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Tx transmits MAC frames and can update the local network allocation vector (NAV). The NAV records a
reservation of the medium. Both ``ITx::transmitFrame()`` overloads require an explicit
``updateLocalNav`` argument. Custom implementations must use these signatures:

.. code-block:: c++

   void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
           bool updateLocalNav, ITx::ICallback *callback) override;
   void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
           simtime_t ifs, bool updateLocalNav, ITx::ICallback *callback) override;

Pass ``true`` for DCF holder frames. Pass ``false`` for HCF holder frames and recipient responses. For example, HCF publishes successful RTS and DATA fields through shared TXNAV. DCF retains the local NAV update. The transmitted duration field
remains in each response. The response no longer extends the recipient's local NAV. Received
reservations still prevent a recipient CTS when the medium is busy.

The immediate overload forwards the caller's choice with zero interframe space:

.. code-block:: c++

   transmitFrame(packet, header, SIMTIME_ZERO, updateLocalNav, callback);

1. Retain the choice with the accepted frame through its delay and transmission.
2. Save the completed frame's choice before the completion callback.
3. Clear the retained state before that callback.
4. Apply the local NAV update after the callback only when the saved choice is true.

.. code-block:: c++

   auto completedUpdateLocalNav = updateLocalNav;
   auto completedCallback = txCallback;
   auto completedFrame = frame;
   auto duration = completedFrame->peekAtFront<Ieee80211MacHeader>()->getDurationField();
   frame = nullptr;
   txCallback = nullptr;
   updateLocalNav = false;
   completedCallback->transmissionComplete(completedFrame,
       completedFrame->peekAtFront<Ieee80211MacHeader>());
   delete completedFrame;
   if (completedUpdateLocalNav)
       rx->frameTransmitted(duration);

The callback can accept another transmission with a different choice. For example, a completed
recipient CTS must retain ``false`` even if the callback accepts a holder frame with ``true``. Do
not use default arguments on these virtual methods.
