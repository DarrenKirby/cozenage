How to Install Cozenage
=======================

Get the Source
--------------

Once ``Cozenage`` reaches a stable, 1.0.0 release, tar archives will be made available, but for now,
use git to download the source tree:

.. code-block::
    $ git clone https://github.com/DarrenKirby/cozenage.git

``Cozenage`` provides a unified Makefile which uses either CMake or GNU-make for the build process.
There is no install target for now, but I will add one upon the stable release. For now, just run
the binary from the source tree.

CMake build
-----------

.. code-block:: console
    $ make

This will create a build/ directory in the source root which contains all the intermediate object
files, and place the ``cozenage`` binary at the top-level of the source tree.

GNU make build
--------------

If you do not have Cmake installed, or prefer not to use it, you can run:

.. code-block:: console
    $ make nocmake

Building the testrunner
-----------------------

Cozenage comes with an (incomplete) set of end-to-end tests. To build this suite, and run the tests:

.. code-block:: console
    $ make test
    $ ./run_tests

The tests require the `Criterion framework <https://criterion.readthedocs.io/en/master/>` _
to build and run.

Cleaning and rebuilding the source tree
---------------------------------------

Running

.. code-block:: console
    $ make clean

will remove the cozenage binary, the run_tests binary (if it exists), and remove the build/ object
directory. You can also run

.. code-block:: console
    $ make rebuild

which is shorthand for

.. code-block:: console
    $ make clean
    $ make
