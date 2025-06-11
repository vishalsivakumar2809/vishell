# vishell

# About:
```vishell``` is a custom-built Shell implemented in C. The program starts with ```initialize()``` which sets ```signal_callback_handler()``` to ensure the shell doesn't crash when CTRL+C or CTRL+V has been used. After doing so, an infinite loop begins, which looks for an input. This input is tokenized. After so, it checks whether the command is ```pwd```, ```cd```, or an ```exit``` call, and handles them accordingly. Otherwise, it handles input/output redirection, and uses ```fork()``` and ```pipe()``` to create and execute child processes to execute the command. 

Support currently exists for basic Unix commands such as ```cd```, ```pwd```, ```ls```, ```echo```, ```exit```.
# Demo:

# References:
https://man.archlinux.org/man/getcwd.2.en

https://people.cs.rutgers.edu/~pxk/416/notes/c-tutorials/times.html

https://jameshfisher.com/2017/02/05/how-do-i-use-execve-in-c/

https://stackoverflow.com/questions/47857369/stop-signal-propagation-in-child-process-making-exec

https://man7.org/linux/man-pages/man2/signal.2.html

https://man7.org/linux/man-pages/man2/getrusage.2.html
