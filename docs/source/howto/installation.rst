How to Install Cozenage
=======================

Get the Source
--------------

Once Cozenage reaches a stable, 1.0.0 release, tar archives will be made available, but for now,
use git to download the source tree:

.. code-block:: bash

    $ git clone https://github.com/DarrenKirby/cozenage.git

Cozenage provides a unified Makefile which uses either CMake or GNU-make for the build process.
There is no install target for now, but I will add one upon the stable release. For now, just run
the binary from the source tree.

Install dependencies
--------------------

Cozenage requires one of `GNU Readline <>`_,
or /*BSD libedit installed for the REPL. It requires `ICU <https://icu.unicode.org/>`_ for Unicode.
It requires `libgc <https://www.hboehm.info/gc/>`_ for garbage collection.

CMake build
-----------

.. code-block:: bash

    $ make

This will create a build/ directory in the source root which contains all the intermediate object
files, and place the ``cozenage`` binary at the top-level of the source tree.

GNU make build
--------------

If you do not have Cmake installed, or prefer not to use it, you can run:

.. code-block:: bash

    $ make nocmake

Building the testrunner
-----------------------

Cozenage comes with an (incomplete) set of end-to-end tests. To build this suite, and run the tests:

.. code-block:: bash

    $ make test
    $ ./run_tests

The tests require the `Criterion framework <https://criterion.readthedocs.io/en/master/>`_
to build and run.

Cleaning and rebuilding the source tree
---------------------------------------

Running

.. code-block:: bash

    $ make clean

will remove the cozenage binary, the run_tests binary (if it exists), and remove the build/ object
directory. You can also run

.. code-block:: bash

    $ make rebuild

which is shorthand for

.. code-block:: bash

    $ make clean
    $ make

Portability
-----------

Cozenage is regularly tested on macOS, Linux, and FreeBSD. I do not have any Windows systems, so
Windows support is unknown, and likely broken. If you would like to help with this please see
`this issue <https://github.com/DarrenKirby/cozenage/issues/1>`_.
