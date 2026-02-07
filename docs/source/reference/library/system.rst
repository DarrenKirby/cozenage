Base System Library
===================

The system library provides the necessary primitives for Cozenage programs to inspect and interact with
the host operating system. It offers a comprehensive suite of tools for process management, user
authentication, and system telemetry. By exposing low-level POSIX functionality—such as process IDs,
environment variables, and user/group identities—the library allows developers to write "system-aware"
scripts that can adapt to their environment, manage permissions, and execute external shell commands.

Whether you are auditing system resources with cpu-count and uptime, navigating the directory tree with
chdir, or verifying security contexts with is-root?, this module serves as the primary bridge between the
Cozenage REPL and the underlying Unix-like kernel.

Process Information
-------------------

.. _proc:get-pid:

.. function:: (get-pid)

    Returns the process ID (PID) of the calling process.

    :return: The process ID.
    :rtype: integer

    Example

    .. code-block:: scheme

        --> (get-pid)
        12345

.. _proc:get-ppid:

.. function:: (get-ppid)

    Returns the process ID of the parent of the calling process.

    :return: The parent process ID.
    :rtype: integer

Environment Variables
---------------------

.. _proc:get-env-var:

.. function:: (get-env-var string)

    Returns the value of the environment variable string. If the variable is unset, returns #false.

    :param string: The name of the environment variable.
    :type string: string
    :return: The value of the variable or #false.
    :rtype: string or boolean

    Example

    .. code-block:: scheme

    --> (get-env-var "SHELL")
    "/bin/bash"
    --> (get-env-var "NON_EXISTENT")
    #false

.. _proc:get-env-vars:

.. function:: (get-env-vars)

    Returns an association list of all environment variables in the running process. Each element is a pair of (variable . value).

    :return: An alist of environment variables.
    :rtype: list

.. _proc:get-home:

.. function:: (get-home)

    Returns the path to the current user's home directory.

    :return: The home directory path.
    :rtype: string

.. _proc:get-path:

.. function:: (get-path)

    Returns a list of directories in the current user's PATH environment variable, ordered according to the shell's search priority.

    :return: A list of directory paths.
    :rtype: list

User and Group IDs
------------------

.. _proc:get-uid:

.. function:: (get-uid)

    Returns the real user ID of the calling process.

    :return: The user ID.
    :rtype: integer

.. _proc:get-gid:

.. function:: (get-gid)

    Returns the real group ID of the calling process.

    :return: The group ID.
    :rtype: integer

.. _proc:get-euid:

.. function:: (get-euid)

    Returns the effective user ID of the calling process.

    :return: The effective user ID.
    :rtype: integer

.. _proc:get-egid:

.. function:: (get-egid)

    Returns the effective group ID of the calling process.

    :return: The effective group ID.
    :rtype: integer

.. _proc:set-uid-bang:

.. function:: (set-uid! n)

    Sets the real user ID of the currently running process to n.

    :param n: The new user ID.
    :type n: integer
    :return: #true on success, otherwise returns an OS error.
    :rtype: boolean

.. _proc:set-gid-bang:

.. function:: (set-gid! n)

    Sets the real group ID of the currently running process to n.

    :param n: The new group ID.
    :type n: integer
    :return: #true on success, otherwise returns an OS error.
    :rtype: boolean

User and Group Information
--------------------------

.. _proc:get-username:

.. function:: (get-username)

    Returns the username associated with the effective UID of the running process.

    :return: The username string, or #false if it cannot be determined.
    :rtype: string or boolean

.. _proc:get-groups:

.. function:: (get-groups)

    Returns an association list of all groups associated with the current process's effective UID. Each element is a pair of (gid . "groupname").

    :return: An alist of group IDs and names.
    :rtype: list

.. _proc:is-root-p:

.. function:: (is-root?)

    Returns #true if the process is running with root privileges (UID 0).

    :return: #true or #false.
    :rtype: boolean


Working Directory and Permissions
---------------------------------

.. _proc:get-cwd:

.. function:: (get-cwd)

    Returns the current working directory of the process.

    :return: The absolute path of the current directory.
    :rtype: string

.. _proc:chdir:

.. function:: (chdir path)

    Changes the current working directory to path.

    :param path: The target directory path.
    :type path: string
    :return: #true on success, or an OS error.
    :rtype: boolean

.. _proc:chmod:

.. function:: (chmod path mode)

    Changes the permission bits of the file at path.

    :param path: The path to the file.
    :type path: string
    :param mode: The bitmask for the new permissions (typically provided as an octal integer).
    :type mode: integer
    :return: #true on success.
    :rtype: boolean

    Example

    .. code-block:: scheme

      --> (chmod "script.sh" #o755)
        #true


System Metadata and Telemetry
-----------------------------

.. _proc:uname:

.. function:: (uname)

    Returns a 5-tuple (list) containing system and hardware platform information. The fields are: (sysname nodename release version machine).

    :return: A list of 5 strings.
    :rtype: list

.. _proc:uptime:

.. function:: (uptime)

    Returns system uptime and load average statistics. Return value is a list which contains:
        1. Total uptime in seconds (integer).
        2. A human-readable uptime string.
        3. A list of three floats representing the 1, 5, and 15-minute load averages.


    :return: The results list.
    :rtype: list

    Example

    .. code-block:: scheme

      --> (uptime)
        (2738231 "up 31 days 16:37" (0.15 0.10 0.05))

.. _proc:cpu-count:

.. function:: (cpu-count)

    Returns the number of logical processors currently configured on the system.

    :return: The number of CPUs.
    :rtype: integer

.. _proc:get-hostname:

.. function:: (get-hostname)

    Returns the system's network hostname.

    :return: The hostname string.
    :rtype: string

Process Control
---------------

.. _proc:system:

.. function:: (system command)

    Forks a new process and executes command using /bin/sh. This procedure blocks until the command completes.

    :param command: The shell command to execute.
    :type command: string
    :return: The exit status of the command.
    :rtype: integer

.. _proc:sleep:

.. function:: (sleep n)

    Suspends the execution of the calling process for n seconds.

    :param n: The number of seconds to sleep.
    :type n: integer
    :return: Unspecified.
    :rtype: void
