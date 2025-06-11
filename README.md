# vishell <p align="center"> <img src="https://i.pinimg.com/originals/a6/67/3a/a6673a6f9bf48588b4b9bbff57b59a21.gif" width="50"/> </p>

```vishell``` is my custom-built shell implemented in C, offering Unix-like functionality and process control. It supports command execution, I/O redirection, simple piping, and background process handling.

# Key Features:
Built-in commands: ```cd```, ```pwd```, ```exit```
Support for external commands like ```ls```, ```echo```, etc.
Input (<) and output (>) redirection
Basic piping (cmd1 | cmd2)
Background process execution with &
Signal handling for Ctrl+C (SIGINT) and Ctrl+Z (SIGTSTP) — prevents the shell itself from terminating or suspending

# How it Works:
The program begins with ```initialize()```, which sets up signal handlers via ```signal_callback_handler()``` to gracefully handle user interruptions.

It enters an infinite loop to:
1. Read and tokenize user input
2. Handle built-in commands (```cd```, ```pwd```, ```exit```)
3. Manage redirection (<, >) and piping (|)
4. Spawn child processes using ```fork()``` and execute commands with ```execvp()``` or ```execve()```
5. Track background and suspended processes

# Demo:

# References:
https://man.archlinux.org/man/getcwd.2.en

https://people.cs.rutgers.edu/~pxk/416/notes/c-tutorials/times.html

https://jameshfisher.com/2017/02/05/how-do-i-use-execve-in-c/

https://stackoverflow.com/questions/47857369/stop-signal-propagation-in-child-process-making-exec

https://man7.org/linux/man-pages/man2/signal.2.html

https://man7.org/linux/man-pages/man2/getrusage.2.html
