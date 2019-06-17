Step 6. Counting to Infinity (Two-node loop instability)
========================================================

Goals
-----

TODO: The link breaks, and in the RIP updates the hop count gets
gradually larger -> counting to infinity it reaches 16 (unusable) but it
takes lots of minutes

The model
---------

TODO: how this can be prevented (SplitHorizon, etc)

TODO: the network and the config

Results
-------

TODO: video

Rip convergence takes 220 seconds (from link break at 50s to no routes
to host3 at 270s). Note the incrementally increasing hop count
(indicated on the route in parentheses), i.e. counting to infinity



   .. video:: media/step6_1.mp4



   <!--internal video recording, zoom 0.77, playback speed 1, no animation speed-->

Rip start, link break, counting to infinity, ping packets:



   .. video:: media/step6_2.mp4

Ping packets go back and forth between the two routers, indicating the
presence of the routing loop. The ping packet times out after 8 hops,
and then it's dropped.

TODO: what happens

TODO: screenshots of routing tables / RIP packets
