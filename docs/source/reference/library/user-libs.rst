User-provided libraries
=======================

Reusable code can be saved to a file and loaded from other scripts or from the
REPL using the ``import`` special form, which is documented elsewhere. See :ref:`import-form`.

Library names
-------------

A library is identified by a *library name*, a list of two parts. The first
part is the *collection*, a group of related libraries. The second is the
*library* itself. In the following example, ``testlib`` is the collection and
``test`` is the library:

.. code-block:: scheme

    (import (testlib test))

A single collection may contain many independently loadable libraries. The
import modifiers documented with ``import`` (``only``, ``except``, ``prefix``
and ``rename``) work with user-provided libraries too.

File layout
-----------

Each collection is a directory, and each library is a file in that directory
named after the library with an ``.sls`` extension. The library
``(testlib test)`` is therefore found at:

``<load path directory>/testlib/test.sls``

For example: ``~/.local/share/cozenage/testlib/test.sls``

The load path
-------------

When a library is imported, Cozenage searches these directories in order:

1. ``./lib/cozenage/``
2. ``../lib/cozenage/``
3. The directory named by the ``COZENAGE_LIB_PATH`` environment variable
4. ``$XDG_DATA_HOME/cozenage``, or ``~/.local/share/cozenage`` if
   ``XDG_DATA_HOME`` is unset
5. ``/usr/lib/cozenage`` (and ``/usr/lib64/cozenage`` on Linux only)
6. ``/usr/local/lib/cozenage``

The first match wins. If the same library exists in more than one of these
directories, only the earliest one is loaded.

Writing a library
-----------------

A library file contains a single ``define-library`` form:

.. code-block:: scheme

    (define-library (testlib test)
      (export circle-area cylinder-volume)
      (begin
        ;; Unexported helpers
        (define pi 3.14159265359)
        (define (square x) (* x x))

        ;; Exported procedures
        (define (circle-area radius)
          (* pi (square radius)))

        (define (cylinder-volume radius height)
          (* (circle-area radius) height))))

The form has three parts:

* The library name, which must match the file's location on the load path.
* An ``export`` clause listing the names the library makes available.
* A ``begin`` expression containing the library's code.

Only exported names are visible to the importing program. Helpers such as
``pi`` and ``square`` above stay private to the library and do not enter the
importer's environment.
