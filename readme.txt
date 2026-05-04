04.05.26 Added release and srelease(size optimized) build targets

01.05.2026
Its my attempt on small UI that is intended for a memory editor program for Linux that i develop.
Goal is to grow along with main program, so anything can change breaking everything.
Second goal is for it to be 100% Valgrind clear - not even still reachable memory, even in third party code.
I use external libraries as long as they are not leaking too. To date that is why XCB lib was introduced 
in code to walkaround XLib memory leak. So, no suppression files of Valgrind either.
It's my approach and i don't like to argue about it. It's over a year of work on an off, 
so i plan it to be that way.

(C) 2026 Anton Onuchin




