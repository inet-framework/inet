IEEE 802.11 Radio Command Deferral
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Tx is the component that transmits frames for medium access control (MAC).
The MAC keeps a radio command while Tx retains an accepted transmission.
This includes the short interframe space (SIFS) before a response.
The hybrid coordination function (HCF) and distributed coordination function (DCF)
release the pending command after the response completes.
This prevents radio reconfiguration before the accepted response completes.

Custom implementations of ``ITx`` need the new method to compile.
Implement the query with this signature:

.. code-block:: c++

   [[nodiscard]] bool hasTransmission() const override;

1. Return true after Tx accepts a frame, including any wait before transmission starts.
2. Keep the return value true while Tx transmits the frame.
3. Clear the accepted transmission state before Tx calls ``ICallback::transmissionComplete()``.
4. Return false when Tx holds no accepted transmission.

The completion callback can release a pending command, so it must observe the cleared state.
For example, Tx accepts an acknowledgment (ACK) frame with a SIFS delay.
A radio command arrives during that delay, when ``hasTransmission()`` returns true.
The MAC keeps the command until the ACK completes.
Tx clears its state before the callback, so the coordination function can release the command.
