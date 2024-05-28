:orphan:

.. _dg:cha:ppp:

Point-to-Point Links
====================

Overview
--------

The modules of the PPP model can be found in the
``inet.linklayer.ppp`` package. The :ned:`Ppp` simple module performs
encapsulation of network datagrams into PPP frames and decapsulation of
the incoming PPP frames.

Sending PPP Frames
------------------

TODO how to send; accepted tags; example code

Receiving PPP Frames
--------------------

TODO tags PPP attaches to packets; example code

Extending the PPP Module
------------------------

TODO how (override handleMessage etc.)

Signals:
---------

Notifications are sent when transmission of a new PPP frame starts
(``NF_PP_TX_BEGIN``), finishes (``NF_PP_TX_END``), or when a PPP frame
is received (``NF_PP_RX_END``).