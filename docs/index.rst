.. Binocle documentation master file, created by
   sphinx-quickstart on Fri Jun 21 11:27:56 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Binocle's documentation
==================================

This is the main documentation of Binocle, a C engine mainly aimed at game development.

The previous incarnation was a C++ engine with way more features than this one, but I wanted to get back to the basics and trim everything down to a more manageable framework without all the bloat that C++ carries around.

It's born out of the need for the following features:

- Cross-platform compilation (macOS, Windows, iOS, Android, Web)
- OpenGL ES 2 support (but you can use any variant of OpenGL as long as it's supported by your hardware)

Nothing too fancy, but still something I always need when I make 2D or 3D games and prototypes.

.. warning::
   The API is evolving all the time but the core is pretty stable. I keep adding and tweaking stuff based on my needs, so things may change without notice.


.. toctree::
   :maxdepth: 2
   :caption: Contents:
   :glob:

   features
   install
   build
   apiref



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

