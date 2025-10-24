Time Library
============

The ``(scheme time)`` library provides access to time-related values.

R7RS procedures
---------------

.. _proc:current-seconds:

.. function:: (current-seconds)

   Returns an inexact number representing the current time on the International Atomic Time (TAI)
   scale. The value 0.0 represents midnight on January 1, 1970 TAI (equivalent to ten seconds before
   midnight Universal Time) and the value 1.0 represents one TAI second later. Neither high accuracy
   nor high precision are required; Cozenage returns Coordinated Universal Time plus 37, which is
   the number of leap seconds added to UTC since the epoch, as ofOctober 2025.

   :return: The current TAI time in seconds.
   :rtype: real

   **Example:**

   .. code-block:: scheme

      ;; Example: Timestamping with (current-second)

      ;; A simple logging procedure
      (define (log-message level message)
        (let ((tai-timestamp (current-second)))

          ;; We're just printing the raw TAI second value here.

          (display "[")
          (display tai-timestamp)
          (display "] [")
          (display level)
          (display "] ")
          (display message)
          (newline)))

      ;; Using the logger
      (log-message "INFO" "Interpreter boot sequence started.")
      ;; ... (imagine doing some work here) ...
      (log-message "WARN" "Could not find config file, using defaults.")
      ;; ... (more work) ...
      (log-message "INFO" "REPL is ready.")

      ;; Example output:

      [1761060904.3412] [INFO] Interpreter boot sequence started.
      [1761060904.3419] [WARN] Could not find config file, using defaults.
      [1761060904.3420] [INFO] REPL is ready.


.. _proc:current-jiffy:

.. function:: (current-jiffy)

    Returns the number of jiffies as an exact integer that have elapsed since an arbitrary,
    implementation-defined epoch. A jiffy is an implementation-defined fraction of a second which is
    defined by the return value of the jiffies-per-second procedure. The starting epoch is guaranteed
    to be constant during a run of the program, but may vary between runs. Cozenage uses
    ``CLOCK_MONOTONIC`` as the implementation-defined epoch, so it is essentially the time since
    boot of the currently running machine.

   :return: The current jiffy, as an exact integer.
   :rtype: integer



.. _proc:jiffies-per-second:

.. function:: (jiffies-per-second)

   Returns an exact integer representing the number of jiffies per SI second. This value is an
   implementation-specified constant. Cozenage uses 1 billion, ie: nanoseconds.

   :return: The jiffies-per-second constant, as an exact integer.
   :rtype: integer

   **Example:**

   .. code-block:: scheme

       ;; Example: Benchmarking with Jiffies

       ;; A helper procedure to time a function call.
       (define (time-it thunk)
         (let ((jps (jiffies-per-second))) ; Get the scale one time

           (let ((start-jiffies (current-jiffy)))

             ;; Call the procedure we're timing
             (let ((result (thunk)))

               (let ((end-jiffies (current-jiffy)))

                 ;; Calculate the difference
                 (let* ((elapsed-jiffies (- end-jiffies start-jiffies))

                        ;; Convert to (inexact) seconds for printing
                        (elapsed-seconds (/ (inexact elapsed-jiffies)
                                            (inexact jps))))

                   (display "Execution time: ")
                   (display elapsed-seconds)
                   (display " seconds")
                   (newline)

                   ;; Return the original result of the thunk
                   result)))))

       ;; Let's define a slow function to benchmark, like a recursive Fibonacci
       (define (slow-fib n)
         (if (<= n 1)
             n
             (+ (slow-fib (- n 1)) (slow-fib (- n 2)))))

       ;; Now, let's time it!
       (display "Running (slow-fib 30)...")
       (newline)

       (time-it (lambda () (slow-fib 30)))

       ;; Example output:

       Running (slow-fib 30)...
       Execution time: 0.048215 seconds
