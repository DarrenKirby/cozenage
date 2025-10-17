Booleans
========

Overview
--------

The standard boolean objects for true and false are written as #t and #f. Alternatively, they can be
written #true and #false, respectively. What really matters, though, are the objects that the Scheme
conditional expressions (if, cond, and, or, when, unless, do) treat as true or false. The phrase
“a true value” (or sometimes just “true”) means any object treated as true by the conditional
expressions, and the phrase “a false value” (or “false”) means any object treated as false by the
conditional expressions.

Of all the Scheme values, only #f counts as false in conditional expressions. All other Scheme
values, including #t, count as true.

Boolean constants evaluate to themselves, so they do not need to be quoted in programs. Unlike most
Schemes, Cozenage displays true and false as ``#true`` and ``#false`` by default in the REPL.

.. code-block:: scheme

    #t
    ;=> #true
    #false
    ;=> #false

Boolean procedures
------------------

.. _proc:not:

.. function:: (not obj)

   The not procedure returns ``#true`` if *obj* is false, and returns ``#false`` otherwise.

   :param obj: The object to test.
   :type obj1: any
   :return: #true or #false.
   :rtype: boolean

   **Example:**

   .. code-block:: scheme

      (not #t)
      ;=> #false
      (not 3)
      ;=> #false
      (not (list 3))
      ;=> #false
      (not #f)
      ;=> #true
      (not ’())
      ;=> #false
      (not (list))
      ;=> #false
      (not ’nil)
      ;=> #false
