# LiCpp - Header-only library to "Let it Crash" in C++

Name: LiC++ - Let it Crash in C++

Copyright: Boost Software License - Version 1.0 - August 17th, 2003

Author: Christoph Woskowski, cwo_at_zuehlke.com

Description:
This is a header only library, enabling to implement the Erlang "Let it Crash"
paradigm of handling errors in C++. It uses the Message Passing example from
the book "C++ Concurrency in Action" by Anthony Williams in order to allow
message based communication between threads. It also uses the Thread Safe List
example from the same book for managing threads in a thread register.
Basically this library enables to start, pause, resume and kill threads, while
allowing them to communication via message queues. It also allows to link
threads for monitoring purposes. If one thread is dying (getting killed or
killing itself by throwing an exception) the monitoring thread gets notified
and may react by replacing the dead one. See the following link for more
information regarding "Let it Crash" (LiC):

http://blog.zuehlke.com/en/is-it-safe-to-let-it-crash/
