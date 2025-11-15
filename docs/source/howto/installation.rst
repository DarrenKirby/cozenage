How to Build and Install Cozenage
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

Cozenage requires one of `GNU Readline <https://www.gnu.org/software/readline/>`_,
or \*BSD libedit installed for the REPL. It requires `ICU <https://icu.unicode.org/>`_ for Unicode.
It requires `libgc <https://www.hboehm.info/gc/>`_ for garbage collection.

CMake build
-----------

.. code-block:: bash

    $ make

This will create a build/ directory in the source root which contains all the intermediate object
files. The main ``cozenage`` binary will be placed in the top-level of the source tree. The library modules will be
placed in a ``lib/`` subdirectory of the source tree. On macOS systems, these modules will be suffixed with ``.dylib``
extensions. All other systems will suffix them with the ``.so`` extension.

The default build specifies an -02 optimized binary. You can specify a non-optimized build with debugging symbols
by running:

.. code-block:: bash

    $ make DEBUG=ON

.. tip::

    By default, the Cozenage build prefers Gnu Readline for its extended and configurable tab-completion
    facilities. On Linux, GNU readline is typically the only readline installed. On BSD or macOS systems,
    however, libedit is the default system-provided readline library, but GNU readline can also be installed
    by the user. To force linking against libedit even if GNU readline is installed, you can pass a flag to
    make:

    .. code-block:: bash

        $ make USE_LIBEDIT=ON


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

.. tip::

    The unified Makefile uses GNU-extensions that will cause an error on most \*BSD systems that have standard make
    installed. On such systems, replace all the ``make`` commands above with ``gmake``, if GNU Make is installed. If not,
    CMake must be used manually to build. For example, from the top of the source directory:

    .. code-block::

        $ mkdir build
        $ cd build
        $ cmake ..
        $ make
        $ mv cozenage ..

Portability
-----------

Cozenage is regularly tested on macOS, Linux, and FreeBSD. I do not have any Windows systems, so
Windows support is unknown, and likely broken. If you would like to help with this please see
`this issue <https://github.com/DarrenKirby/cozenage/issues/1>`_.
