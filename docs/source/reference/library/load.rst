Scheme Load Library
===================

Overview
--------

The ``(scheme load)`` library exports a single name: the ``load`` procedure, which takes a filename as a string, and
reads and evaluates the Scheme code in the file. Upon success, the procedure will return ``#true``. Upon failure,
the procedure will print the error message to the standard error stream, and return ``#false``.

Load Procedures
---------------

.. _proc:load:

.. function:: (load string)

   Returns ``#true`` if the file specified by *string* is found and evaluated without error, and ``#false`` otherwise.

   :param string: The file to load.
   :type char: string
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

       --> (import (scheme load))
         #true
       --> atom?
        Value error:  Unbound symbol: 'atom?'
       --> (load "./scheme/little_schemer.scm")
         #true
       --> atom?
         <lambda 'atom?'>
       --> (atom? "foo")
         #true

