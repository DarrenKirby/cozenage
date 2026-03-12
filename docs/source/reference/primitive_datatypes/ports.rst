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


Port Procedures
---------------

Default Ports
^^^^^^^^^^^^^

.. _proc:current-input-port:

current-input-port
******************

.. function:: (current-input-port)

    Returns the current default input port. Unless rebound, this is the
    standard input port ``stdin``.

    :return: The current input port.
    :rtype: input-port

    **Example:**

    .. code-block:: scheme

        --> (current-input-port)
        #<open:text-file-port input-port 'stdin'>


.. _proc:current-output-port:

current-output-port
*******************

.. function:: (current-output-port)

    Returns the current default output port. Unless rebound, this is the
    standard output port ``stdout``.

    :return: The current output port.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (current-output-port)
        #<open:text-file-port output-port 'stdout'>


.. _proc:current-error-port:

current-error-port
******************

.. function:: (current-error-port)

    Returns the current default error port. Unless rebound, this is the
    standard error port ``stderr``.

    :return: The current error port.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (current-error-port)
        #<open:text-file-port output-port 'stderr'>

Port Type Predicates
^^^^^^^^^^^^^^^^^^^^

.. _proc:input-port?:

input-port?
***********

.. function:: (input-port? obj)

    Returns ``#t`` if *obj* is an input port, ``#f`` otherwise. Returns ``#f``
    for any non-port argument.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is an input port, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (input-port? (current-input-port))
        #t
        --> (input-port? (current-output-port))
        #f
        --> (input-port? "not a port")
        #f


.. _proc:output-port?:

output-port?
************

.. function:: (output-port? obj)

    Returns ``#t`` if *obj* is an output port, ``#f`` otherwise. Returns ``#f``
    for any non-port argument.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is an output port, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (output-port? (current-output-port))
        #t
        --> (output-port? (current-input-port))
        #f
        --> (output-port? 42)
        #f


.. _proc:textual-port?:

textual-port?
*************

.. function:: (textual-port? obj)

    Returns ``#t`` if *obj* is a textual port, ``#f`` otherwise. Textual ports
    operate on characters and strings; this includes text file-backed ports and
    string ports. Binary ports (bytevector-backed or binary file-backed) return
    ``#f``, as do non-port arguments.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is a textual port, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (textual-port? (current-output-port))
        #t
        --> (textual-port? (open-input-string "hello"))
        #t
        --> (textual-port? (open-bin-input-file "data.bin"))
        #f


.. _proc:binary-port?:

binary-port?
************

.. function:: (binary-port? obj)

    Returns ``#t`` if *obj* is a binary port, ``#f`` otherwise. Binary ports
    operate on raw bytes; this includes binary file-backed ports and
    bytevector ports. Textual ports (string-backed or text file-backed) return
    ``#f``, as do non-port arguments.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is a binary port, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (binary-port? (open-bin-input-file "data.bin"))
        #t
        --> (binary-port? (current-input-port))
        #f
        --> (binary-port? (open-input-string "hello"))
        #f


.. _proc:input-port-open?:

input-port-open?
****************

.. function:: (input-port-open? obj)

    Returns ``#t`` if *obj* is an input port and is currently open, ``#f``
    otherwise. Returns ``#f`` for closed input ports, non-input ports, and
    non-port arguments.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is an open input port, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-file "data.txt"))
        --> (input-port-open? p)
        #t
        --> (close-input-port p)
        --> (input-port-open? p)
        #f


.. _proc:output-port-open?:

output-port-open?
*****************

.. function:: (output-port-open? obj)

    Returns ``#t`` if *obj* is an output port and is currently open, ``#f``
    otherwise. Returns ``#f`` for closed output ports, non-output ports, and
    non-port arguments.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is an open output port, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-file "out.txt"))
        --> (output-port-open? p)
        #t
        --> (close-output-port p)
        --> (output-port-open? p)
        #f

Opening and Closing Ports
^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:open-input-file:

open-input-file
***************

.. function:: (open-input-file string)

    Opens the file named by *string* for reading and returns a textual input
    port capable of delivering character data from that file. Signals a
    file-error if the file does not exist or cannot be opened.

    :param string: The path to the file to open.
    :type string: string
    :return: A textual input port backed by the named file.
    :rtype: input-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-file "data.txt"))
        --> p
        #<open:text-file-port input-port '/home/user/data.txt'>
        --> (input-port? p)
        #t
        --> (textual-port? p)
        #t


.. _proc:open-bin-input-file:

open-bin-input-file
*******************

.. function:: (open-bin-input-file string)

    Opens the file named by *string* for reading and returns a binary input
    port capable of delivering raw byte data from that file. Signals a
    file-error if the file does not exist or cannot be opened.

    :param string: The path to the file to open.
    :type string: string
    :return: A binary input port backed by the named file.
    :rtype: input-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-bin-input-file "data.bin"))
        --> p
        #<open:binary-file-port input-port '/home/user/data.bin'>
        --> (binary-port? p)
        #t


.. _proc:open-output-file:

open-output-file
****************

.. function:: (open-output-file string [mode])

    Opens the file named by *string* for writing and returns a textual output
    port capable of writing character data to that file. If the file does not
    exist it is created. If the file already exists, output is **appended** to
    it — the existing contents are preserved. Signals a file-error if the file
    cannot be opened.

    An optional *mode* string may be supplied to override the default file
    opening mode. This is an advanced option and in most cases should be
    omitted.

    .. note::

        This differs from R7RS, where opening an existing output file has
        unspecified consequences. Here, appending is the explicit default. Use
        ``open-and-trunc-output-file`` if you need to overwrite an existing
        file from the beginning.

    :param string: The path to the file to open.
    :type string: string
    :param mode: Optional file mode string. Defaults to ``"a"`` (append).
    :type mode: string
    :return: A textual output port backed by the named file.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-file "out.txt"))
        --> p
        #<open:text-file-port output-port '/home/user/out.txt'>
        --> (output-port? p)
        #t
        --> (textual-port? p)
        #t


.. _proc:open-bin-output-file:

open-bin-output-file
********************

.. function:: (open-bin-output-file string [mode])

    Opens the file named by *string* for writing and returns a binary output
    port capable of writing raw byte data to that file. If the file does not
    exist it is created. If the file already exists, output is **appended** to
    it — the existing contents are preserved. Signals a file-error if the file
    cannot be opened.

    An optional *mode* string may be supplied to override the default file
    opening mode. This is an advanced option and in most cases should be
    omitted.

    .. note::

        This differs from R7RS, where opening an existing output file has
        unspecified consequences. Here, appending is the explicit default. Use
        ``open-and-trunc-output-file`` if you need to overwrite an existing
        file from the beginning.

    :param string: The path to the file to open.
    :type string: string
    :param mode: Optional file mode string. Defaults to ``"a"`` (append).
    :type mode: string
    :return: A binary output port backed by the named file.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-bin-output-file "data.bin"))
        --> p
        #<open:binary-file-port output-port '/home/user/data.bin'>
        --> (binary-port? p)
        #t


.. _proc:open-and-trunc-output-file:

open-and-trunc-output-file
**************************

.. function:: (open-and-trunc-output-file string [mode])

    Opens the file named by *string* for writing and returns a textual output
    port capable of writing character data to that file. If the file does not
    exist it is created. If the file already exists, it is **truncated to zero
    length** before writing begins — all existing contents are discarded.
    Signals a file-error if the file cannot be opened.

    An optional *mode* string may be supplied to override the default file
    opening mode. This is an advanced option and in most cases should be
    omitted.

    :param string: The path to the file to open.
    :type string: string
    :param mode: Optional file mode string. Defaults to ``"w"`` (write/truncate).
    :type mode: string
    :return: A textual output port backed by the named file.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-and-trunc-output-file "out.txt"))
        --> (display "first line" p)
        --> (close-port p)
        --> (define p2 (open-and-trunc-output-file "out.txt"))
        --> (display "replaced" p2)
        --> (close-port p2)

.. _proc:open-and-trunc-bin-output-file:

open-and-trunc-bin-output-file
******************************

.. function:: (open-and-trunc-bin-output-file string [mode])

    Opens the file named by *string* for writing and returns a binary output
    port capable of writing raw byte data to that file. If the file does not
    exist it is created. If the file already exists, it is **truncated to zero
    length** before writing begins — all existing contents are discarded.
    Signals a file-error if the file cannot be opened.

    An optional *mode* string may be supplied to override the default file
    opening mode. This is an advanced option and in most cases should be
    omitted.

    :param string: The path to the file to open.
    :type string: string
    :param mode: Optional file mode string. Defaults to ``"w"`` (write/truncate).
    :type mode: string
    :return: A binary output port backed by the named file.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-and-trunc-bin-output-file "data.bin"))
        --> (binary-port? p)
        #t
        --> (output-port? p)
        #t
        --> (close-port p)
        --> (define p2 (open-and-trunc-bin-output-file "data.bin"))
        --> (write-u8 42 p2)
        --> (close-port p2)

.. _proc:open-output-string:

open-output-string
******************

.. function:: (open-output-string)

    Creates and returns a textual output port backed by an in-memory string
    buffer. Characters written to this port are accumulated in the buffer and
    may be retrieved as a string using ``get-output-string``.

    :return: A textual output port backed by an in-memory string buffer.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-string))
        --> p
        #<open:string-port output-port 'memory-backed'>
        --> (display "hello, " p)
        --> (display "world" p)
        --> (get-output-string p)
        "hello, world"


.. _proc:open-input-string:

open-input-string
*****************

.. function:: (open-input-string string)

    Takes *string* and returns a textual input port that delivers characters
    from it. The port reads from a private copy of the string's contents at
    the time of the call; subsequent modifications to *string* have no effect
    on the port.

    :param string: The string whose contents will be read from the port.
    :type string: string
    :return: A textual input port backed by an in-memory string buffer.
    :rtype: input-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-string "hello, world"))
        --> p
        #<open:string-port input-port 'memory-backed'>
        --> (read-char p)
        #\h
        --> (read-line p)
        "ello, world"


.. _proc:get-output-string:

get-output-string
*****************

.. function:: (get-output-string port)

    Returns a string consisting of all characters written to *port* so far, in
    the order they were written. *port* must be a textual output port created
    by ``open-output-string``; it is an error to pass any other kind of port.
    The port remains open and usable after this call — subsequent writes
    continue to accumulate.

    :param port: A textual output port created by ``open-output-string``.
    :type port: output-port
    :return: The characters accumulated in *port* as a string.
    :rtype: string

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-string))
        --> (display "hello, " p)
        --> (display "world" p)
        --> (get-output-string p)
        "hello, world"
        --> (display "!" p)
        --> (get-output-string p)
        "hello, world!"


.. _proc:open-output-bytevector:

open-output-bytevector
**********************

.. function:: (open-output-bytevector)

    Creates and returns a binary output port backed by an in-memory bytevector
    buffer. Bytes written to this port are accumulated in the buffer and may
    be retrieved as a u8 bytevector using ``get-output-bytevector``.

    :return: A binary output port backed by an in-memory bytevector buffer.
    :rtype: output-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-bytevector))
        --> p
        #<open:bytevector-port output-port 'memory-backed'>
        --> (write-u8 72 p)
        --> (write-u8 105 p)
        --> (get-output-bytevector p)
        #u8(72 105)


.. _proc:open-input-bytevector:

open-input-bytevector
*********************

.. function:: (open-input-bytevector bytevector)

    Takes a u8 *bytevector* and returns a binary input port that delivers bytes
    from it. The port reads from a private copy of the bytevector's contents at
    the time of the call; subsequent modifications to *bytevector* have no
    effect on the port. It is an error if *bytevector* is not a u8 bytevector.

    :param bytevector: The u8 bytevector whose contents will be read from the port.
    :type bytevector: bytevector
    :return: A binary input port backed by an in-memory bytevector buffer.
    :rtype: input-port

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-bytevector #u8(72 101 108 108 111)))
        --> p
        #<open:bytevector-port input-port 'memory-backed'>
        --> (read-u8 p)
        72
        --> (read-u8 p)
        101


.. _proc:get-output-bytevector:

get-output-bytevector
*********************

.. function:: (get-output-bytevector port)

    Returns a u8 bytevector consisting of all bytes written to *port* so far,
    in the order they were written. *port* must be a binary output port created
    by ``open-output-bytevector``; it is an error to pass any other kind of
    port. The port remains open and usable after this call — subsequent writes
    continue to accumulate.

    :param port: A binary output port created by ``open-output-bytevector``.
    :type port: output-port
    :return: The bytes accumulated in *port* as a u8 bytevector.
    :rtype: bytevector

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-bytevector))
        --> (write-u8 72 p)
        --> (write-u8 105 p)
        --> (get-output-bytevector p)
        #u8(72 105)
        --> (write-u8 33 p)
        --> (get-output-bytevector p)
        #u8(72 105 33)


.. _proc:close-port:

close-port
**********

.. function:: (close-port port)

    Closes *port*, releasing any resources associated with it and rendering it
    incapable of delivering or accepting data. If *port* is already closed,
    this procedure has no effect. Signals an error if *port* is not a port.

    :param port: The port to close.
    :type port: port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-file "data.txt"))
        --> (input-port-open? p)
        #t
        --> (close-port p)
        --> (input-port-open? p)
        #f
        --> (close-port p)

Input Operations
^^^^^^^^^^^^^^^^

.. _proc:read-line:

read-line
*********

.. function:: (read-line [port])

    Reads the next line of text from the textual input *port*, or from the
    current input port if *port* is omitted. Returns a string containing all
    characters up to, but not including, the line ending. The line ending
    itself is consumed but not included in the returned string.

    Line endings recognised are: a linefeed (``\n``), a carriage return
    (``\r``), or a carriage-return/linefeed sequence (``\r\n``).

    If end-of-file is encountered before any characters are read, an
    end-of-file object is returned. If end-of-file is encountered after some
    characters have been read but before a line ending, the characters read so
    far are returned as a string. Signals an error if *port* is not open for
    input.

    :param port: A textual input port. Defaults to the current input port.
    :type port: input-port
    :return: A string containing the next line of text, or an end-of-file object.

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-string "hello\nworld\n"))
        --> (read-line p)
        "hello"
        --> (read-line p)
        "world"
        --> (read-line p)
        #<eof>


.. _proc:read-lines:

read-lines
**********

.. function:: (read-lines [port])

    Reads all remaining lines of text from the textual input *port*, or from
    the current input port if *port* is omitted, until end-of-file is
    encountered. Returns a list of strings, one per line, in the order they
    were read. Line endings are consumed but not included in the returned
    strings. If the port is already at end-of-file, an empty list is returned.
    Signals an error if *port* is not open for input.

    :param port: A textual input port. Defaults to the current input port.
    :type port: input-port
    :return: A list of strings, one per line.
    :rtype: list

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-string "one\ntwo\nthree\n"))
        --> (read-lines p)
        ("one" "two" "three")
        --> (read-lines (open-input-string ""))
        ()

.. _proc:read:

read
****

.. function:: (read [port])

    Reads and returns the next complete Scheme datum (S-expression) from the
    textual input *port*, or from the current input port if *port* is omitted.
    If the port is at end-of-file before any input is read, an end-of-file
    object is returned. Signals an error if *port* is not open for input.

    .. note::

        This procedure differs from R7RS in an important way. Rather than
        reading from a continuous stream, ``read`` is **line-oriented**: it
        reads one line at a time and accumulates input until a complete,
        syntactically valid S-expression has been formed. If the input so far
        is incomplete — for example, an opening parenthesis has not yet been
        matched — ``read`` simply reads another line and tries again. This
        makes ``read`` behave naturally at an interactive prompt, where input
        arrives one line at a time.

    :param port: A textual input port. Defaults to the current input port.
    :type port: input-port
    :return: The next complete Scheme datum, or an end-of-file object.

    **Example:**

    .. code-block:: scheme

        --> (read (open-input-string "42"))
        42
        --> (read (open-input-string "(1 2 3)"))
        (1 2 3)
        --> (define p (open-input-string "\"hello\""))
        --> (read p)
        "hello"

    At an interactive prompt, multi-line expressions are read naturally:

    .. code-block:: scheme

        --> (read)
        (define x
        ...   (+ 1 2))
        (define x (+ 1 2))

.. _proc:read-string:

read-string
***********

.. function:: (read-string k [port])

    Reads the next *k* characters from the textual input *port*, or from the
    current input port if *port* is omitted, and returns them as a newly
    allocated string. Characters are read in left-to-right order. If fewer
    than *k* characters are available before end-of-file, only those available
    are returned. If no characters are available before end-of-file, an
    end-of-file object is returned. It is an error if *k* is not a positive
    exact integer.

    :param k: The number of characters to read.
    :type k: integer
    :param port: A textual input port. Defaults to the current input port.
    :type port: input-port
    :return: A string of up to *k* characters, or an end-of-file object.
    :rtype: string

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-string "hello, world"))
        --> (read-string 5 p)
        "hello"
        --> (read-string 5 p)
        ", wor"
        --> (read-string 100 p)
        "ld"
        --> (read-string 5 p)
        #<eof>


.. _proc:read-char:

read-char
*********

.. function:: (read-char [port])

    Reads and returns the next character from the textual input *port*, or
    from the current input port if *port* is omitted, advancing the port
    position past that character. If no characters are available, an
    end-of-file object is returned. Signals an error if *port* is not open
    for input, or if *port* is a binary port.

    :param port: A textual input port. Defaults to the current input port.
    :type port: input-port
    :return: The next character, or an end-of-file object.
    :rtype: character

    **Example:**

    .. code-block::

        --> (define p (open-input-string "héllo"))
        --> (read-char p)
        #\h
        --> (read-char p)
        #\é
        --> (read-char p)
        #\l


.. _proc:read-u8:

read-u8
*******

.. function:: (read-u8 [port])

    Reads and returns the next byte from the binary input *port*, or from the
    current input port if *port* is omitted, advancing the port position past
    that byte. The byte is returned as an exact integer in the range 0–255.
    If no bytes are available, an end-of-file object is returned. Signals an
    error if *port* is not open for input.

    :param port: A binary input port. Defaults to the current input port.
    :type port: input-port
    :return: The next byte as an exact integer in the range 0–255, or an
             end-of-file object.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-bytevector #u8(72 101 108 108 111)))
        --> (read-u8 p)
        72
        --> (read-u8 p)
        101
        --> (read-u8 p)
        108

.. _proc:read-bytevector:

read-bytevector
***************

.. function:: (read-bytevector k [port])

    Reads the next *k* bytes from the binary input *port*, or from the current
    input port if *port* is omitted, and returns them as a newly allocated u8
    bytevector. Bytes are read in left-to-right order. If fewer than *k* bytes
    are available before end-of-file, only those available are returned. If no
    bytes are available before end-of-file, an end-of-file object is returned.
    It is an error if *k* is not a positive exact integer.

    :param k: The number of bytes to read.
    :type k: integer
    :param port: A binary input port. Defaults to the current input port.
    :type port: input-port
    :return: A u8 bytevector of up to *k* bytes, or an end-of-file object.
    :rtype: bytevector

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-bytevector #u8(1 2 3 4 5)))
        --> (read-bytevector 3 p)
        #u8(1 2 3)
        --> (read-bytevector 10 p)
        #u8(4 5)
        --> (read-bytevector 3 p)
        #<eof>


.. _proc:read-bytevector!:

read-bytevector!
****************

.. function:: (read-bytevector! bytevector [port [start [end]]])

    Reads bytes from the binary input *port*, or from the current input port
    if *port* is omitted, writing them directly into *bytevector* beginning at
    index *start* (inclusive) and stopping at index *end* (exclusive). *start*
    defaults to ``0`` and *end* defaults to the length of *bytevector*. Returns
    the number of bytes actually read as an exact integer. If no bytes are
    available before end-of-file, an end-of-file object is returned. It is an
    error if *port* is not a binary input port, or if *start* or *end* are out
    of range.

    Unlike ``read-bytevector``, this procedure reuses an existing bytevector
    rather than allocating a new one, making it suitable for use in
    performance-sensitive or allocation-conscious contexts.

    :param bytevector: The u8 bytevector to read bytes into.
    :type bytevector: bytevector
    :param port: A binary input port. Defaults to the current input port.
    :type port: input-port
    :param start: Index of the first byte position to write. Defaults to ``0``.
    :type start: integer
    :param end: Index past the last byte position to write. Defaults to the
                length of *bytevector*.
    :type end: integer
    :return: The number of bytes read, or an end-of-file object.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

        --> (define bv (make-bytevector 5 0))
        --> (define p (open-input-bytevector #u8(1 2 3 4 5)))
        --> (read-bytevector! bv p)
        5
        --> bv
        #u8(1 2 3 4 5)
        --> (define bv2 (make-bytevector 5 0))
        --> (define p2 (open-input-bytevector #u8(1 2 3 4 5)))
        --> (read-bytevector! bv2 p2 1 4)
        3
        --> bv2
        #u8(0 1 2 3 0)

.. _proc:peek-char:

peek-char
*********

.. function:: (peek-char [port])

    Returns the next character available from the textual input *port*, or
    from the current input port if *port* is omitted, without advancing the
    port position. A subsequent call to ``read-char`` on the same port will
    return the same character. If no characters are available, an end-of-file
    object is returned. Signals an error if *port* is not open for input, or
    if *port* is a binary port.

    :param port: A textual input port. Defaults to the current input port.
    :type port: input-port
    :return: The next character without consuming it, or an end-of-file object.
    :rtype: character

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-string "hello"))
        --> (peek-char p)
        #\h
        --> (peek-char p)
        #\h
        --> (read-char p)
        #\h
        --> (read-char p)
        #\e


.. _proc:peek-u8:

peek-u8
*******

.. function:: (peek-u8 [port])

    Returns the next byte available from the binary input *port*, or from the
    current input port if *port* is omitted, without advancing the port
    position. The byte is returned as an exact integer in the range 0–255. A
    subsequent call to ``read-u8`` on the same port will return the same byte.
    If no bytes are available, an end-of-file object is returned. Signals an
    error if *port* is not open for input, or if *port* is a textual port.

    :param port: A binary input port. Defaults to the current input port.
    :type port: input-port
    :return: The next byte without consuming it as an exact integer in the
             range 0–255, or an end-of-file object.
    :rtype: integer

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-bytevector #u8(1 2 3)))
        --> (peek-u8 p)
        1
        --> (peek-u8 p)
        1
        --> (read-u8 p)
        1
        --> (read-u8 p)
        2

Output Operations
^^^^^^^^^^^^^^^^^

.. _proc:display:

display
*******

.. function:: (display obj [port])

    Writes a human-readable representation of *obj* to the textual output
    *port*, or to the current output port if *port* is omitted. The
    representation is intended for human consumption rather than machine
    reading:

    * Strings are written without surrounding quotation marks.
    * Characters are written as themselves, not in ``#\`` notation.
    * Symbols are written without escaping.

    Returns an unspecified value. Signals an error if *port* is not open for
    output.

    :param obj: The object to write.
    :param port: A textual output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (display "hello")
        hello
        --> (display #\A)
        A
        --> (display '(1 "two" #\3))
        (1 two 3)
        --> (display 42)
        42


.. _proc:displayln:

displayln
*********

.. function:: (displayln obj [port])

    Identical to ``display``, but appends a newline after writing *obj*.
    Provided as a convenience to avoid the common pattern of pairing
    ``display`` with ``newline``. Returns an unspecified value. Signals an
    error if *port* is not open for output.

    :param obj: The object to write.
    :param port: A textual output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (displayln "hello")
        hello
        --> (displayln "first")
        first
        --> (displayln "second")
        second


.. _proc:write:

write
*****

.. function:: (write obj [port])

    Writes a machine-readable external representation of *obj* to the textual
    output *port*, or to the current output port if *port* is omitted. The
    representation is intended to be readable back in by ``read``:

    * Strings are enclosed in quotation marks, with backslash and quote
      characters escaped.
    * Characters are written in ``#\`` notation.
    * Symbols containing non-ASCII characters are escaped with vertical lines.

    Returns an unspecified value. Signals an error if *port* is not open for
    output.

    :param obj: The object to write.
    :param port: A textual output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (write "hello")
        "hello"
        --> (write #\A)
        #\A
        --> (write '(1 "two" #\3))
        (1 "two" #\3)
        --> (write 42)
        42


.. _proc:writeln:

writeln
*******

.. function:: (writeln obj [port])

    Identical to ``write``, but appends a newline after writing *obj*.
    Provided as a convenience to avoid the common pattern of pairing ``write``
    with ``newline``. Returns an unspecified value. Signals an error if *port*
    is not open for output.

    :param obj: The object to write.
    :param port: A textual output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (writeln "hello")
        "hello"
        --> (writeln '(1 2 3))
        (1 2 3)

.. _proc:write-char:

write-char
**********

.. function:: (write-char char [port])

    Writes *char* to the textual output *port*, or to the current output port
    if *port* is omitted. The character itself is written, not its external
    representation — for example, ``#\A`` is written as ``A``. Returns an
    unspecified value. Signals an error if *port* is not open for output, or
    if *port* is a binary port.

    :param char: The character to write.
    :type char: character
    :param port: A textual output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block::

        --> (write-char #\H)
        H
        --> (write-char #\λ)
        λ
        --> (define p (open-output-string))
        --> (write-char #\A p)
        --> (write-char #\B p)
        --> (get-output-string p)
        "AB"


.. _proc:write-string:

write-string
************

.. function:: (write-string string [port [start [end]]])

    Writes the characters of *string* from index *start* (inclusive) to *end*
    (exclusive) to the textual output *port*, or to the current output port if
    *port* is omitted. *start* defaults to ``0`` and *end* defaults to the
    length of *string*. Characters are written in left-to-right order. Returns
    an unspecified value. Signals an error if *port* is not open for output.

    :param string: The string to write.
    :type string: string
    :param port: A textual output port. Defaults to the current output port.
    :type port: output-port
    :param start: Index of the first character to write. Defaults to ``0``.
    :type start: integer
    :param end: Index past the last character to write. Defaults to the length
                of *string*.
    :type end: integer
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (write-string "hello, world")
        hello, world
        --> (write-string "hello, world" (current-output-port) 7)
        world
        --> (write-string "hello, world" (current-output-port) 0 5)
        hello


.. _proc:write-u8:

write-u8
********

.. function:: (write-u8 byte [port])

    Writes *byte* to the binary output *port*, or to the current output port
    if *port* is omitted. *byte* must be an exact integer in the range 0–255.
    Returns an unspecified value. Signals an error if *port* is not open for
    output, or if *port* is a textual port.

    :param byte: The byte value to write.
    :type byte: integer
    :param port: A binary output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-bytevector))
        --> (write-u8 72 p)
        --> (write-u8 105 p)
        --> (get-output-bytevector p)
        #u8(72 105)


.. _proc:write-bytevector:

write-bytevector
****************

.. function:: (write-bytevector bytevector [port [start [end]]])

    Writes the bytes of *bytevector* from index *start* (inclusive) to *end*
    (exclusive) to the binary output *port*, or to the current output port if
    *port* is omitted. *start* defaults to ``0`` and *end* defaults to the
    length of *bytevector*. Bytes are written in left-to-right order. Returns
    an unspecified value. Signals an error if *port* is not open for output,
    or if *port* is a textual port.

    :param bytevector: The u8 bytevector to write.
    :type bytevector: bytevector
    :param port: A binary output port. Defaults to the current output port.
    :type port: output-port
    :param start: Index of the first byte to write. Defaults to ``0``.
    :type start: integer
    :param end: Index past the last byte to write. Defaults to the length of
                *bytevector*.
    :type end: integer
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-bytevector))
        --> (write-bytevector #u8(1 2 3 4 5) p)
        --> (get-output-bytevector p)
        #u8(1 2 3 4 5)
        --> (define p2 (open-output-bytevector))
        --> (write-bytevector #u8(1 2 3 4 5) p2 1 4)
        --> (get-output-bytevector p2)
        #u8(2 3 4)

.. _proc:newline:

newline
*******

.. function:: (newline [port])

    Writes a newline character to the textual output *port*, or to the current
    output port if *port* is omitted. Returns an unspecified value. Signals an
    error if *port* is not open for output, or if *port* is a binary port.

    :param port: A textual output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-string))
        --> (write-string "line one" p)
        --> (newline p)
        --> (write-string "line two" p)
        --> (get-output-string p)
        "line one\nline two"



Miscellaneous I/O Procedures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:eof-object:

eof-object
**********

.. function:: (eof-object)

    Returns an end-of-file object. This is the same object returned by input
    procedures such as ``read``, ``read-char``, and ``read-u8`` when the end
    of an input source is reached.

    :return: An end-of-file object.

    **Example:**

    .. code-block:: scheme

        --> (eof-object)
        #<eof>
        --> (eof-object? (eof-object))
        #t
        --> (eof-object? (read (open-input-string "")))
        #t


.. _proc:read-error?:

read-error?
***********

.. function:: (read-error? obj)

    Returns ``#t`` if *obj* is a read error object, ``#f`` otherwise. Read
    errors are signalled by input procedures such as ``read-char`` and
    ``read-u8`` when an I/O error occurs during reading. Returns ``#f`` for
    any non-error argument.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is a read error, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (read-error? (read-char (open-input-string "hi")))
        #f
        --> (file-error? "not an error")
        #f


.. _proc:file-error?:

file-error?
***********

.. function:: (file-error? obj)

    Returns ``#t`` if *obj* is a file error object, ``#f`` otherwise. File
    errors are signalled by procedures such as ``open-input-file`` and
    ``open-output-file`` when a file cannot be opened or closed. Returns
    ``#f`` for any non-error argument.

    :param obj: The object to test.
    :return: ``#t`` if *obj* is a file error, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (file-error? "not an error")
        #f


.. _proc:flush-output-port:

flush-output-port
*****************

.. function:: (flush-output-port [port])

    Flushes any buffered output from *port* to the underlying file or device,
    or flushes the current output port if *port* is omitted. Returns an
    unspecified value. Signals an error if *port* is not open for output.

    For memory-backed ports (string and bytevector ports), this procedure has
    no effect, as they do not buffer output in the same way as file-backed
    ports.

    :param port: An output port. Defaults to the current output port.
    :type port: output-port
    :return: Unspecified.

    **Example:**

    .. code-block:: scheme

        --> (define p (open-output-file "out.txt"))
        --> (display "hello" p)
        --> (flush-output-port p)
        --> (close-port p)


.. _proc:char-ready?:

char-ready?
***********

.. function:: (char-ready? [port])

    Returns ``#t`` if a character is immediately available from the textual
    input *port*, or from the current input port if *port* is omitted, without
    blocking. If ``#t`` is returned, the next call to ``read-char`` on that
    port is guaranteed not to hang. Returns ``#f`` if no character is
    immediately available. If the port is at end-of-file, returns ``#t``.
    Signals an error if *port* is not an open textual input port.

    String ports are always ready and always return ``#t``. For file-backed
    ports, readiness is determined by checking the underlying C library buffer
    and, if empty, querying the operating system via ``select()``.

    :param port: A textual input port. Defaults to the current input port.
    :type port: input-port
    :return: ``#t`` if a character is ready, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-string "hello"))
        --> (char-ready? p)
        #t
        --> (read-char p)
        #\h


.. _proc:u8-ready?:

u8-ready?
*********

.. function:: (u8-ready? [port])

    Returns ``#t`` if a byte is immediately available from the binary input
    *port*, or from the current input port if *port* is omitted, without
    blocking. If ``#t`` is returned, the next call to ``read-u8`` on that port
    is guaranteed not to hang. Returns ``#f`` if no byte is immediately
    available. If the port is at end-of-file, returns ``#t``. Signals an error
    if *port* is not an open binary input port.

    Bytevector ports are always ready and always return ``#t``. For
    file-backed binary ports, readiness is determined by checking the
    underlying C library buffer and, if empty, querying the operating system
    via ``select()``.

    :param port: A binary input port. Defaults to the current input port.
    :type port: input-port
    :return: ``#t`` if a byte is ready, ``#f`` otherwise.
    :rtype: boolean

    **Example:**

    .. code-block:: scheme

        --> (define p (open-input-bytevector #u8(1 2 3)))
        --> (u8-ready? p)
        #t
        --> (read-u8 p)
        1


High-Level Port Handlers
^^^^^^^^^^^^^^^^^^^^^^^^

.. _proc:call-with-port:

call-with-port
**************

.. function:: (call-with-port port proc)

    Calls *proc* with *port* as its sole argument. When *proc* returns — whether
    normally or with an error — *port* is closed automatically and the value
    yielded by *proc* is returned. It is an error if *proc* does not accept
    exactly one argument.

    .. note::

        This differs from R7RS, where a port is left open if *proc* does not
        return normally. Here, the port is always closed on return.

    :param port: The port to pass to *proc*.
    :type port: port
    :param proc: A procedure accepting exactly one argument.
    :type proc: procedure
    :return: The value(s) returned by *proc*.

    **Example:**

    .. code-block:: scheme

        --> (call-with-port (open-input-file "data.txt")
        ...   (lambda (p)
        ...     (read-line p)))
        "first line of data.txt"


.. _proc:call-with-input-file:

call-with-input-file
********************

.. function:: (call-with-input-file string proc)

    Opens the file named by *string* for textual input, then calls *proc* with
    the resulting input port as its sole argument. The port is closed when
    *proc* returns, whether normally or with an error. Returns the value
    yielded by *proc*. Signals a file-error if the file cannot be opened. It
    is an error if *proc* does not accept exactly one argument.

    :param string: The path to the file to open for input.
    :type string: string
    :param proc: A procedure accepting exactly one argument.
    :type proc: procedure
    :return: The value(s) returned by *proc*.

    **Example:**

    .. code-block:: scheme

        --> (call-with-input-file "data.txt"
        ...   (lambda (p)
        ...     (read-line p)))
        "first line of data.txt"


.. _proc:call-with-output-file:

call-with-output-file
*********************

.. function:: (call-with-output-file string proc)

    Opens the file named by *string* for textual output, then calls *proc*
    with the resulting output port as its sole argument. The port is closed
    when *proc* returns, whether normally or with an error. Returns the value
    yielded by *proc*. Signals a file-error if the file cannot be opened. It
    is an error if *proc* does not accept exactly one argument.

    If the file does not exist it is created. If the file already exists,
    output is **appended** to it — existing contents are preserved.

    :param string: The path to the file to open for output.
    :type string: string
    :param proc: A procedure accepting exactly one argument.
    :type proc: procedure
    :return: The value(s) returned by *proc*.

    **Example:**

    .. code-block:: scheme

        --> (call-with-output-file "out.txt"
        ...   (lambda (p)
        ...     (display "hello, world" p)))


.. _proc:with-input-from-file:

with-input-from-file
********************

.. function:: (with-input-from-file string thunk)

    Opens the file named by *string* for textual input and temporarily rebinds
    the current input port to the new port for the dynamic extent of the call.
    *thunk* is then called with no arguments. When *thunk* returns, the port is
    closed and the previous default input port is restored, whether *thunk*
    returned normally or with an error. Returns the value yielded by *thunk*.
    Signals a file-error if the file cannot be opened. It is an error if
    *thunk* does not accept zero arguments.

    While *thunk* is running, procedures that implicitly read from the current
    input port — such as ``read`` and ``read-line`` called without a port
    argument — will read from the file.

    :param string: The path to the file to open for input.
    :type string: string
    :param thunk: A procedure accepting no arguments.
    :type thunk: procedure
    :return: The value(s) returned by *thunk*.

    **Example:**

    .. code-block:: scheme

        --> (with-input-from-file "data.txt"
        ...   (lambda ()
        ...     (read-line)))
        "first line of data.txt"


.. _proc:with-output-to-file:

with-output-to-file
*******************

.. function:: (with-output-to-file string thunk)

    Opens the file named by *string* for textual output and temporarily
    rebinds the current output port to the new port for the dynamic extent of
    the call. *thunk* is then called with no arguments. When *thunk* returns,
    the port is closed and the previous default output port is restored,
    whether *thunk* returned normally or with an error. Returns the value
    yielded by *thunk*. Signals a file-error if the file cannot be opened. It
    is an error if *thunk* does not accept zero arguments.

    While *thunk* is running, procedures that implicitly write to the current
    output port — such as ``display`` and ``write`` called without a port
    argument — will write to the file.

    If the file does not exist it is created. If the file already exists,
    output is **appended** to it — existing contents are preserved.

    :param string: The path to the file to open for output.
    :type string: string
    :param thunk: A procedure accepting no arguments.
    :type thunk: procedure
    :return: The value(s) returned by *thunk*.

    **Example:**

    .. code-block:: scheme

        --> (with-output-to-file "out.txt"
        ...   (lambda ()
        ...     (display "hello, world")))

