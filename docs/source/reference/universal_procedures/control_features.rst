Control Feature Procedures
==========================

Overview
--------

This section documents procedures that do not operate on a specific data type
but instead provide general-purpose facilities for program control, execution,
and convenience.

**Control and Evaluation**

``eval`` and ``apply`` are the core meta-programming facilities inherited from
Scheme's Lisp heritage. ``eval`` allows programs to construct and evaluate
expressions at runtime, bridging the gap between data and code — a natural
consequence of Scheme's homoiconicity, where code and data share the same list
representation. ``apply`` allows a procedure to be called with its arguments
supplied as a list, which is invaluable when the number of arguments is not
known until runtime. Both are standard R7RS procedures, though this
implementation's ``eval`` deviates in being unary — evaluation always takes
place in the global environment.

**System Interface**

``load`` reads and evaluates a file of Scheme source code, providing the
primary mechanism for organising programs across multiple files. ``exit``
terminates the interpreter with an optional status code, following the Unix
convention that zero denotes success and non-zero denotes failure. The boolean
shorthand — ``#t`` for success, ``#f`` for failure — is a convenience that
maps naturally onto Scheme idioms. ``command-line`` provides access to the
arguments passed to a script, enabling programs to behave differently based on
their invocation context.

**Polymorphic Convenience Procedures**

``len``, ``idx``, and ``rev`` are non-standard extensions that provide a
uniform interface across the ordered and compound types: lists, vectors,
bytevectors, and strings. Rather than remembering which type-specific procedure
applies in a given context — ``string-length`` versus ``vector-length`` versus
``list-length`` — these procedures dispatch automatically on the type of their
argument. They are particularly useful in generic or polymorphic code where the
specific sequence type is not known in advance, and in interactive use where
brevity is valued. Code that requires the clarity of explicit type dispatch
should prefer the type-specific procedures.

Procedure Documentation
-----------------------

.. _proc:eval:

.. function:: (eval expr)

    Evaluates *expr* as a Scheme expression in the global environment and
    returns the result. *expr* may be any object that has a valid external
    representation as an expression, including a quoted list representing a
    procedure call.

    .. note::

        This implementation's ``eval`` is unary and always evaluates in the
        global environment. R7RS specifies a second argument for the
        environment in which evaluation takes place (e.g.
        ``(interaction-environment)``), but first-class environments are not
        currently supported. Any environment argument passed will be silently
        ignored.

    :param expr: The expression to evaluate.
    :type expr: any
    :return: The result of evaluating *expr*.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (eval '(+ 1 2))
      3
      --> (eval '(define x 42))
      --> x
      42
      --> (eval (list '+ 1 2 3))
      6
      --> (let ((op '+)) (eval (list op 10 20)))
      30


.. _proc:apply:

.. function:: (apply proc arg1 ... args)

    Calls *proc* with the elements of the list formed by appending any
    individual *arg1* ... arguments to the final *args* list as its actual
    arguments. The final argument must be a proper list; all preceding
    arguments after *proc* are prepended to it individually.

    ``apply`` is implemented as a tail call: the result is returned as a
    tail-call sentinel to the interpreter's evaluator for further evaluation,
    meaning it participates correctly in tail-call optimisation and does not
    consume additional stack depth.

    :param proc: The procedure to call.
    :type proc: procedure
    :param arg1: Zero or more individual arguments to prepend to *args*.
    :type arg1: any
    :param args: A proper list of the remaining arguments to pass to *proc*.
    :type args: list
    :return: The result of calling *proc* with the assembled argument list.
    :rtype: any

    **Example:**

    .. code-block:: scheme

      --> (apply + '(1 2 3))
      6
      --> (apply + 1 2 '(3 4))
      10
      --> (apply string '(#\h #\e #\l #\l #\o))
      "hello"
      --> (apply max '(3 1 4 1 5 9 2 6))
      9
      --> (apply map (list char-upcase '("hello" "world")))
      ("HELLO" "WORLD")

.. _proc:load:

.. function:: (load filename)

    Reads and evaluates Scheme expressions and definitions from the file named
    by *filename* sequentially in the global environment. Returns ``#t`` on
    success, or ``#f`` if an error is encountered during loading, in which
    case a message is printed to standard error.

    .. note::

        R7RS specifies an optional second environment argument to ``load``.
        This implementation always loads into the global environment; no
        environment argument is supported.

    :param filename: The path to a file containing Scheme source code.
    :type filename: string
    :return: ``#t`` on success, ``#f`` on error.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

      --> (load "mylib.scm")
      #t
      --> (load "init.scm")
      #f


.. _proc:exit:

.. function:: (exit [code])

    Terminates the running program. If *code* is omitted, exits with status
    ``0`` (success). If *code* is a boolean, ``#t`` maps to exit status ``0``
    and ``#f`` maps to exit status ``1``. If *code* is an integer, it is
    passed directly to the system as the exit status code. If running in the
    REPL, the session history is saved before exiting.

    :param code: An optional exit status. A boolean ``#t`` exits with ``0``,
                 ``#f`` exits with ``1``, and an integer exits with that value
                 directly.
    :type code: boolean or integer
    :return: This procedure does not return.

    **Example:**

    .. code-block:: scheme

      --> (exit)
      --> (exit #t)
      --> (exit #f)
      --> (exit 42)


.. _proc:command-line:

.. function:: (command-line)

    Returns a list of command-line arguments passed to the script being
    interpreted. The zeroth element is always the script name. When called
    from the REPL, returns the empty list.

    Script arguments must be separated from interpreter arguments using
    ``--``, so that the interpreter can distinguish between arguments intended
    for itself and those intended for the script. For example:

    .. code-block:: shell

      $ cozenage myscript.scm -- arg1 arg2

    :return: A list of strings representing the command-line arguments, or
             the empty list when called from the REPL.
    :rtype: list

    **Example:**

    .. code-block:: scheme

      --> (command-line)
      ()

    When invoked as ``cozenage myscript.scm -- foo bar``, within
    ``myscript.scm``:

    .. code-block:: scheme

      --> (command-line)
      ("myscript.scm" "foo" "bar")


.. _proc:len:

.. function:: (len obj)

    Returns the length of *obj* as an exact integer. Accepts lists, vectors,
    bytevectors, strings, sets, and hashes. For strings, returns the number
    of characters (Unicode code points), not the number of underlying bytes.
    For sets and hashes, returns the number of members or key-value pairs
    respectively. Raises an error if *obj* is not a compound or sequence type.

    .. note::

        This is a non-standard convenience procedure. For type-specific length
        procedures, see ``list-length``, ``vector-length``, ``string-length``,
        and ``bytevector-length``.

    :param obj: A compound or sequence object.
    :type obj: list, vector, bytevector, string, set, or hash
    :return: The length of *obj*.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

      --> (len '(1 2 3))
      3
      --> (len #(1 2 3))
      3
      --> (len "café")
      4
      --> (len #{1 2 3})
      3
      --> (len #["a" 1 "b" 2])
      2
      --> (len "")
      0


.. _proc:idx:

.. function:: (idx seq i)
              (idx seq start end)
              (idx seq start end step)

    Polymorphic subscript and slicing procedure. With a single index *i*,
    returns the element of *seq* at that position. With *start* and *end*,
    returns a new sequence containing elements from *start* (inclusive) to
    *end* (exclusive). With *step*, returns every *step*-th element in that
    range. Accepts lists, vectors, bytevectors, and strings. All indices are
    zero-based.

    With a single index, delegates to the type-specific ref procedure for
    each type (``list-ref``, ``vector-ref``, ``bytevector-ref``,
    ``string-ref``). Slicing is supported for lists and vectors; bytevectors
    and strings support single-element access only.

    .. note::

        This is a non-standard convenience procedure.

    :param seq: A sequence to subscript or slice.
    :type seq: list, vector, bytevector, or string
    :param i: A zero-based index for single-element access.
    :type i: integer
    :param start: The index of the first element to include in a slice.
    :type start: integer
    :param end: The index past the last element to include in a slice.
    :type end: integer
    :param step: The stride between selected elements. Defaults to ``1``.
    :type step: integer
    :return: The selected element or a new sequence containing the selected
             elements.
    :rtype: any, list, or vector

    **Example:**

    .. code-block:: scheme

      --> (idx '(a b c d) 1)
      b
      --> (idx #(10 20 30 40 50) 2)
      30
      --> (idx '(a b c d e) 1 4)
      (b c d)
      --> (idx #(0 1 2 3 4 5 6) 0 7 2)
      #(0 2 4 6)
      --> (idx "hello" 1)
      #\e


.. _proc:rev:

.. function:: (rev seq)

    Returns a new sequence containing the elements of *seq* in reverse order.
    Accepts lists, vectors, bytevectors, and strings. For strings, reversal
    is grapheme-aware: multi-codepoint grapheme clusters (such as accented
    characters and emoji with combining sequences) are kept intact rather than
    having their constituent code points reversed independently.

    .. note::

        This is a non-standard convenience procedure. For the type-specific
        list procedure, see ``reverse``.

    :param seq: A sequence to reverse.
    :type seq: list, vector, bytevector, or string
    :return: A new sequence with the elements of *seq* in reverse order.
    :rtype: list, vector, bytevector, or string

    **Example:**

    .. code-block:: scheme

      --> (rev '(1 2 3))
      (3 2 1)
      --> (rev #(1 2 3))
      #(3 2 1)
      --> (rev "hello")
      "olleh"
      --> (rev "café")
      "éfac"

