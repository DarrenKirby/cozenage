Ports
=====

Overview
--------

In this interpreter, **ports** are the fundamental mechanism used for all input and output (I/O). A port is a first-class
object that represents a connection to some source of data (for input) or some destination for data (for output). Rather
than reading from or writing to files, strings, or devices directly, Cozenage programs interact with ports in a uniform
and abstract way.

This abstraction allows the same procedures—such as `read`, `display`, or `write`—to operate on many different kinds of
data sources and destinations without needing to know where the data ultimately comes from or goes to.

Ports as Scheme Objects
^^^^^^^^^^^^^^^^^^^^^^^

Ports are **primitive Scheme datatypes**. Like numbers, strings, lists, or characters, ports are values that can be:

* stored in variables,
* passed as arguments to procedures,
* returned as results from procedures, and
* inspected using predicates.

When printed at the REPL, ports have a readable external representation that describes their current state and role.
For example:

.. code-block:: scheme

    --> (current-output-port)
    #<open:text-file-port output-port 'stdout'>
    --> (current-input-port)
    #<open:text-file-port input-port 'stdin'>

This representation is intended for human inspection only. It shows:

* whether the port is currently **open** or closed,
* whether it is a **textual** or **binary** port,
* whether it is an **input** or **output** port, and
* what resource it is associated with (for example, `stdout`, `stdin`, or an in-memory buffer).

Programs should not attempt to parse or depend on this printed form.

Input Ports and Output Ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Every port is either an **input port**, an **output port**, or both. This determines which operations may be performed
on it:

* **Input ports** supply data to the program. Procedures such as `read`, `read-line`, `read-char`, and `read-u8` consume data from input ports.
* **Output ports** receive data from the program. Procedures such as `display`, `write`, `newline`, and their variants send data to output ports.

Attempting to read from an output-only port or write to an input-only port signals an error.

Textual Ports and Binary Ports
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Ports are also classified by the *kind of data* they carry:

**Textual ports** operate on characters and strings. They are used for:

* reading and writing text files,
* interacting with the user via the terminal,
* processing structured textual data such as Scheme expressions.

Procedures such as `read-char`, `read-line`, `display`, and `write` expect textual ports.

**Binary ports** operate on raw bytes, represented as exact integers in the range 0–255. They are used for:

* reading and writing binary files,
* handling non-textual data formats,
* working directly with bytevectors.

Procedures such as `read-u8` and `peek-u8` expect binary ports.

The distinction is important: textual ports perform character-based I/O, while binary ports perform byte-based I/O,
and the two are not interchangeable.

External Representations and Printed Output
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When Scheme objects are written to an output port, they are converted into an **external representation**: a sequence
of characters (for textual ports) or bytes (for binary ports) that represent the object.

Different output procedures use different conventions:

* `display` produces a *human-readable* representation. Strings are written without surrounding quotes, and characters are written as themselves.
* `write` produces a *machine-readable* representation. The output is suitable for reading back in using `read`.

For example:

.. code-block:: scheme

    --> (display "hello")
    hello
    --> (write "hello")
    "hello"

Both procedures write to the same kind of textual output port, but they choose different external representations
depending on their intended use.

File-Backed Ports
^^^^^^^^^^^^^^^^^^^^

A **file-backed port** is connected to a file in the filesystem. Opening a file for input or output creates a port
that reads from or writes to that file.

Examples include:

* `open-input-file` — creates a textual input port backed by a file,
* `open-output-file` — creates a textual output port backed by a file,
* `open-bin-input-file` and `open-bin-output-file` — binary variants.

The standard ports `stdin`, `stdout`, and `stderr` are also file-backed ports, typically connected to the terminal.

File-backed ports are commonly used for persistent data and interaction with the operating system.

Memory-Backed Ports
^^^^^^^^^^^^^^^^^^^

In addition to files, ports may also be backed by memory. These ports behave like ordinary ports, but read from or
write to in-memory buffers instead of external files.

Two important kinds of memory-backed ports are provided:

String ports
^^^^^^^^^^^^

String ports are textual ports backed by strings. An input string port reads characters from a given string, while an
output string port accumulates characters written to it.

Example:

.. code-block:: scheme

    --> (define p (open-input-string "Hello, world!"))
    --> p
    #<open:string-port input-port 'memory-backed'>
    --> (read-line p)
    "Hello, world!"

Output string ports are commonly used to capture output that would otherwise be printed.

Bytevector ports
^^^^^^^^^^^^^^^^

Bytevector ports are binary ports backed by bytevectors. They allow binary data to be read from or written to memory
using the same interface as file-backed binary ports.

Memory-backed ports are especially useful for testing, data transformation, and situations where I/O should not
interact with the filesystem.

Default Ports and Implicit I/O
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Many I/O procedures accept an optional port argument. When this argument is omitted, the procedure operates on a **default port**:

* input procedures use the current input port,
* output procedures use the current output port.

These defaults can be temporarily rebound using higher-level procedures such as `with-input-from-file` and
`with-output-to-file`, allowing code to be written in a port-agnostic style.

Summary
^^^^^^^

Ports provide a unified and flexible abstraction for input and output in Cozenage. By treating files, memory buffers,
and standard streams uniformly, ports allow programs to be written in a composable and modular way, while still
supporting both textual and binary data.

Understanding ports is essential to understanding how Cozenage programs communicate with the outside world.


Standard Ports and Predicates
-----------------------------

.. _proc:current-input-port:

.. function:: (current-input-port)

    Returns the current default input port. This is the port that input operations such as `read` and `read-line`
    will use when no explicit port argument is provided. In interactive use, this is typically connected to standard
    input (stdin).

    :return: The current default input port.
    :rtype: port

    **Example:**

    .. code-block:: scheme

      --> (current-input-port)
      #<input-port stdin>

.. _proc:current-output-port:

.. function:: (current-output-port)

    Returns the current default output port. This is the port that output operations such as `display` and `write` will
    use when no explicit port argument is provided. In interactive use, this is typically connected to standard
    output (stdout).

    :return: The current default output port.
    :rtype: port

    **Example:**

    .. code-block:: scheme

      --> (current-output-port)
      #<output-port stdout>

.. _proc:current-error-port:

.. function:: (current-error-port)

    Returns the current default error port. This port is intended for diagnostic or error messages and is typically
    connected to standard error (stderr).

    :return: The current default error port.
    :rtype: port

    **Example:**

    .. code-block:: scheme

      --> (current-error-port)
      #<output-port stderr>

Port Type Predicates
--------------------

The following predicates test whether an object is a port of a particular kind. These procedures are simple and closely
related, and are therefore documented together.

.. _proc:input-port?:
.. _proc:output-port?:
.. _proc:textual-port?:
.. _proc:binary-port?:

.. function:: (input-port? obj)
    (output-port? obj)
    (textual-port? obj)
    (binary-port? obj)

    Tests whether *obj* is a port of the specified type.

    :param obj: The object to test.
    :return: `#t` if the object is a port of the requested type; `#f` otherwise.
    :rtype: boolean

    **Examples:**

    .. code-block:: scheme

      --> (input-port? (current-input-port))
      #true
      --> (output-port? (current-input-port))
      #false

.. _proc:input-port-open?:

.. function:: (input-port-open? port)

    Returns `#t` if *port* is currently open for input operations. Closed ports cannot be read from.

    :param port: A port object.
    :type port: port
    :return: `#t` if the port is open for input; `#f` otherwise.
    :rtype: boolean

.. _proc:output-port-open?:

.. function:: (output-port-open? port)

    Returns `#t` if *port* is currently open for output operations. Closed ports cannot be written to.

    :param port: A port object.
    :type port: port
    :return: `#t` if the port is open for output; `#f` otherwise.
    :rtype: boolean


Opening and Closing Ports
-------------------------

.. _proc:open-input-file:

.. function:: (open-input-file path)

    Opens the file located at *path* for textual input and returns a new input port connected to that file.

    :param path: Filesystem path to the file to open.
    :type path: string
    :return: A textual input port associated with the file.
    :rtype: port

    **Example:**

    .. code-block:: scheme

      --> (define p (open-input-file "data.txt"))
      --> (read-line p)
      "first line"

.. _proc:open-bin-input-file:

.. function:: (open-bin-input-file path)

    Opens the file located at *path* for binary input and returns a new binary input port.

    :param path: Filesystem path to the file to open.
    :type path: string
    :return: A binary input port associated with the file.
    :rtype: port

.. _proc:open-output-file:

.. function:: (open-output-file path)

    Opens the file located at *path* for textual output. If the file already exists, output is appended to the end of
    the file rather than truncating it.

    :param path: Filesystem path to the file to open.
    :type path: string
    :return: A textual output port associated with the file.
    :rtype: port

    **Example:**

    .. code-block:: scheme

      --> (define p (open-output-file "log.txt"))
      --> (displayln "Hello" p)
      --> (close-port p)

.. _proc:open-bin-output-file:

.. function:: (open-bin-output-file path)

    Opens the file located at *path* for binary output. If the file already exists, output is appended to the end
    of the file.

    :param path: Filesystem path to the file to open.
    :type path: string
    :return: A binary output port associated with the file.
    :rtype: port

.. _proc:open-and-trunc-output-file:

.. function:: (open-and-trunc-output-file path)

    Opens the file located at *path* for textual output, truncating the file to zero length if it already exists.

    :param path: Filesystem path to the file to open.
    :type path: string
    :return: A textual output port associated with the file.
    :rtype: port

.. _proc:close-port:

.. function:: (close-port port)

    Closes *port* and releases the underlying system resource. After a port has been closed, no further input or output
    operations may be performed on it.

    :param port: The port to close.
    :type port: port
    :return: An unspecified value.

.. _proc:close-input-port:

.. function:: (close-input-port port)

    Closes an input port. This procedure is currently an alias for `close-port`.

    :param port: The input port to close.
    :type port: port

.. _proc:close-output-port:

.. function:: (close-output-port port)

    Closes an output port. This procedure is currently an alias for `close-port`.

    :param port: The output port to close.
    :type port: port


Input Operations
----------------

.. _proc:read:

.. function:: (read [port])

    Reads the next complete Scheme datum (S-expression) from *port* and returns it. If *port* is not provided, the
    current input port is used.

    Unlike R7RS, this implementation reads input line by line and continues reading until a complete and syntactically
    valid expression has been formed.

    :param port: Optional input port to read from.
    :type port: port
    :return: The next datum read from the input, or the EOF object if no more input is available.
    :rtype: object

    **Example:**

    .. code-block:: scheme

      --> (read)
      (+ 1 2)
      --> (+ 1 2)
      3

.. _proc:read-line:

.. function:: (read-line [port])

    Reads a single line of text from *port* and returns it as a string. The trailing newline character is not included
    in the returned string.

    :param port: Optional input port to read from.
    :type port: port
    :return: A string containing the next line of input.
    :rtype: string

.. _proc:read-lines:

.. function:: (read-lines [port])

    Reads all remaining lines from *port* and returns them as a list of strings, one string per line.

    :param port: Optional input port to read from.
    :type port: port
    :return: A list of strings.
    :rtype: list

.. _proc:read-char:

.. function:: (read-char [port])

    Reads the next character from *port* and advances the input position.

    :param port: Optional input port to read from.
    :type port: port
    :return: The next character from the input.
    :rtype: char

.. _proc:peek-char:

.. function:: (peek-char [port])

    Returns the next character from *port* without advancing the input position.

    :param port: Optional input port to peek from.
    :type port: port
    :return: The next character from the input.
    :rtype: char

.. _proc:read-u8:

.. function:: (read-u8 [port])

    Reads the next byte from a binary input port and advances the input position.

    :param port: Optional binary input port to read from.
    :type port: port
    :return: An exact integer in the range 0–255.
    :rtype: integer

.. _proc:peek-u8:

.. function:: (peek-u8 [port])

    Returns the next byte from a binary input port without advancing the input position.

    :param port: Optional binary input port to peek from.
    :type port: port
    :return: An exact integer in the range 0–255.
    :rtype: integer

.. _proc:eof-object:

.. function:: (eof-object)

    Returns a distinguished object used to represent end-of-file.

    :return: The EOF object.
    :rtype: eof-object

Output Operations
-----------------

.. _proc:display:

.. function:: (display obj [port])

    Writes a human-readable representation of *obj* to *port*. Strings are written without surrounding quotes. If
    *port* is not provided, the current output port is used.

    :param obj: The object to display.
    :param port: Optional output port to write to.
    :type port: port

.. _proc:displayln:

.. function:: (displayln obj [port])

    Like `display`, but automatically writes a newline after the object.

.. _proc:write:

.. function:: (write obj [port])

    Writes a machine-readable representation of *obj* to *port*. The output is suitable for later reading by `read`.

    :param obj: The object to write.
    :param port: Optional output port to write to.
    :type port: port

.. _proc:writeln:

.. function:: (writeln obj [port])

    Like `write`, but automatically writes a newline after the object.

.. _proc:newline:

.. function:: (newline [port])

    Writes an end-of-line character to *port*. If *port* is not provided, the current output port is used.

    :param port: Optional output port.
    :type port: port

String and Bytevector Ports
---------------------------

.. _proc:open-input-string:

.. function:: (open-input-string string)

    Creates a new textual input port that reads characters from *string*.

    :param string: The source string.
    :type string: string
    :return: A textual input port.
    :rtype: port

.. _proc:open-output-string:

.. function:: (open-output-string)

    Creates a new textual output port that accumulates written characters in memory.

    :return: A textual output port.
    :rtype: port

.. _proc:get-output-string:

.. function:: (get-output-string port)

    Returns the accumulated contents of an output string port as a string.

    :param port: An output string port.
    :type port: port
    :return: The accumulated output.
    :rtype: string

.. _proc:open-input-bytevector:

.. function:: (open-input-bytevector bytevector)

    Creates a new binary input port that reads bytes from *bytevector*.

    :param bytevector: A bytevector of unsigned 8-bit values.
    :type bytevector: bytevector
    :return: A binary input port.
    :rtype: port

.. _proc:open-output-bytevector:

.. function:: (open-output-bytevector)

    Creates a new binary output port that accumulates written bytes in memory.

    :return: A binary output port.
    :rtype: port

.. _proc:get-output-bytevector:

.. function:: (get-output-bytevector port)

    Returns the accumulated contents of an output bytevector port as a bytevector.

    :param port: An output bytevector port.
    :type port: port
    :return: The accumulated output bytes.
    :rtype: bytevector


High-Level Port Handlers
------------------------

.. _proc:call-with-port:

.. function:: (call-with-port port proc)

    Calls *proc* with *port* as its only argument. When *proc* returns, *port* is automatically closed, even if *proc*
    exits normally.

    :param port: A port to pass to the procedure.
    :type port: port
    :param proc: A procedure of one argument.
    :type proc: procedure
    :return: The value returned by *proc*.

    **Example:**

    .. code-block:: scheme

      --> (call-with-port (open-input-file "data.txt")
            (lambda (p)
              (read-line p)))
      "first line"

.. _proc:with-input-from-file:

.. function:: (with-input-from-file path thunk)

    Temporarily replaces the current input port with a port opened on the file at *path* while calling *thunk*. The
     port is closed and the previous input port is restored when *thunk* returns.

    :param path: Filesystem path to the file.
    :type path: string
    :param thunk: A procedure of zero arguments.
    :type thunk: procedure
    :return: The value returned by *thunk*.

.. _proc:with-output-to-file:

.. function:: (with-output-to-file path thunk)

    Temporarily replaces the current output port with a port opened on the file at *path* while calling *thunk*. The
    file is opened in append mode by default. The port is closed and the previous output port is restored when
    *thunk* returns.

    :param path: Filesystem path to the file.
    :type path: string
    :param thunk: A procedure of zero arguments.
    :type thunk: procedure
    :return: The value returned by *thunk*.


