Transport Connection Visualizer Icons
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:cpp:`TransportConnectionCanvasVisualizer::createConnectionEndFigure()` and
:cpp:`TransportConnectionOsgVisualizer::createConnectionEndNode()` take the
index of the color and the letter of the connection as a second parameter:

.. code-block:: c++

   virtual LabeledIconFigure *createConnectionEndFigure(tcp::TcpConnection *tcpConnection, int index) const;
   virtual osg::Node *createConnectionEndNode(tcp::TcpConnection *tcpConnection, int index) const;

A subclass outside INET that overrides the one-parameter form no longer
compiles if the override is marked ``override``, and is no longer called
otherwise; add the parameter and take the color and the letter from
``index``. The constructors of :cpp:`TransportConnectionCanvasVisualization`
and :cpp:`TransportConnectionOsgVisualization` require the index as their
last parameter, and the constructor of :cpp:`TransportConnectionVisualization`
has a new last parameter ``index`` with the default -1.
