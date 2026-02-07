Base File Library
=================

Overview
--------

The file library in Cozenage provides a comprehensive interface for interacting with the filesystem, bridging the gap between
high-level Scheme logic and low-level system calls. It offers a robust set of predicates to identify file types—ranging from
regular files and directories to specialized Unix entities like symbolic links, sockets, and FIFOs. By leveraging these tools,
developers can write portable and defensive code that verifies the nature of a path before attempting operations, ensuring that
the interpreter handles external data gracefully.

Beyond simple identification, the library supports essential filesystem manipulations such as creating and removing directories
or unlinking files. It also exposes detailed metadata through a powerful stat interface, providing access to file sizes, ownership,
and high-precision timestamps (atime, mtime, and ctime) in both machine-readable and human-readable formats. Whether you are
building a build-system tool, a simple file navigator, or a data-logging application, this library provides the necessary
primitives to manage the environment surrounding your Cozenage programs.


File, Device, and Pipe Predicates
---------------------------------


.. _proc:reg-file-p:

.. function:: (reg-file? path)

    Returns #true if the file at path exists and is a regular file.

    :param path: The path to the file.
    :type path: string
    :return: #true or #false.
    :rtype: boolean

    Example

    .. code-block:: scheme

          --> (reg-file? "/etc/passwd")
            #true
          --> (reg-file? "/bin")
            #false

.. _proc:directory-p:

.. function:: (directory? path)

    Returns #true if the file at path exists and is a directory.

    :param path: The path to the directory.
    :type path: string
    :return: #true or #false.
    :rtype: boolean

    Example

    .. code-block:: scheme

        --> (directory? "/tmp")
        #true

.. _proc:symlink-p:

.. function:: (symlink? path)

    Returns #true if the file at path exists and is a symbolic link.

    :param path: The path to the file.
    :type path: string
    :return: #true or #false.
    :rtype: boolean


.. _proc:char-device-p:

.. function:: (char-device? path)

    Returns #true if the file at path exists and is a character device.

    :param path: The path to the file.
    :type path: string
    :return: #true or #false.
    :rtype: boolean

.. _proc:block-device-p:

.. function:: (block-device? path)

    Returns #true if the file at path exists and is a block device.

    :param path: The path to the file.
    :type path: string
    :return: #true or #false.
    :rtype: boolean

.. _proc:fifo-p:

.. function:: (fifo? path)

    Returns #true if the file at path exists and is a FIFO (named pipe).

    :param path: The path to the file.
    :type path: string
    :return: #true or #false.
    :rtype: boolean

.. _proc:socket-p:

.. function:: (socket? path)

    Returns #true if the file at path exists and is a Unix domain socket.

    :param path: The path to the file.
    :type path: string
    :return: #true or #false.
    :rtype: boolean


.. _proc:file-exists-p:

.. function:: (file-exists? path)

    Returns #true if a file or directory exists at path.

    :param path: The path to check.
    :type path: string
    :return: #true or #false.
    :rtype: boolean


Basic File Operations
---------------------

.. _proc:mkdir:

.. function:: (mkdir path)

    Creates the directory named by path. If path is relative, it is created in the current working directory. Default permissions are 0755.

    :param path: The name of the directory to create.
    :type path: string
    :return: #true on success.
    :rtype: boolean

.. _proc:rmdir-bang:

.. function:: (rmdir! path)

    Removes the directory pointed to by path. The directory must be empty.

    :param path: The path to the directory to remove.
    :type path: string
    :return: #true on success.
    :rtype: boolean

.. _proc:unlink-bang:

.. function:: (unlink! path)

    Unlinks (and possibly deletes) the file pointed to by path.

    :param path: The path to the file.
    :type path: string
    :return: #true on success.
    :rtype: boolean

File Metadata and Permissions
-----------------------------

.. _proc:stat:

.. function:: (stat path)

    Returns an association list containing the status information for the file at path.

    :param path: The path to the file.
    :type path: string
    :return: An alist of file attributes (e.g., size, mode, uid, gid).
    :rtype: list

    **Example**

    .. code-block:: scheme

          --> (stat "README.md")
            ((type . "regular") (st_size . 1240) (st_mode . "-rw-r--r--") ...)

.. _proc:file-size:

.. function:: (file-size path)

    Returns the size in bytes of the file pointed to by path.

    :param path: The path to the file.
    :type path: string
    :return: The size in bytes.
    :rtype: integer

.. _proc:file-readable-p:

.. function:: (file-readable? path)

    Returns #true if the currently running process has read permissions for the file or directory.

    :param path: The path to check.
    :type path: string
    :return: #true or #false.
    :rtype: boolean

Extended Timestamp Procedures
-----------------------------

.. _proc:file-mtime:

.. function:: (file-mtime path)

    Returns a list representing the last modified time of the file.

    :param path: The path to the file.
    :type path: string
    :return: A list containing (seconds nanoseconds human-readable-string).
    :rtype: list

    Example

    .. code-block:: scheme

      --> (file-mtime "main.c")
        (1706728331 387617529 "2026-01-31 19:12:11.387617529 PST")

.. _proc:file-atime:

.. function:: (file-atime path)

    Returns a list representing the last access time (atime) of the file pointed to by path.

    :param path: The path to the file.
    :type path: string
    :return: A list containing (seconds nanoseconds human-readable-string).
    :rtype: list

.. _proc:file-ctime:

.. function:: (file-ctime path)

    Returns a list representing the last status change time (ctime) of the file pointed to by path. Note that on Unix systems, this typically refers to metadata changes (like permissions) rather than file creation.

    :param path: The path to the file.
    :type path: string
    :return: A list containing (seconds nanoseconds human-readable-string).
    :rtype: list


Permission Predicates
---------------------

.. _proc:file-writable-p:

.. function:: (file-writable? path)

    Returns #true if the currently running process has write permissions for the file or directory pointed to by path.

    :param path: The path to check.
    :type path: string
    :return: #true or #false.
    :rtype: boolean

    Example

    .. code-block:: scheme

        --> (file-writable? "/etc/shadow")
        #false
        --> (file-writable? "/tmp/my-temp-file")
        #true

.. _proc:file-executable-p:

.. function:: (file-executable? path)

    Returns #true if the currently running process has execute permissions for the file or directory pointed to by path. For directories, "execute" permission allows the process to enter or search the directory.

    :param path: The path to check.
    :type path: string
    :return: #true or #false.
    :rtype: boolean
