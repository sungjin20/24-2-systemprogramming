# SNU CS M1522.00800 : System Programming
## Lab 4 : A Unix Shell


#### Please note that late submission for assignment 4 is NOT allowed.


Purpose
-------

A Unix shell is a program that makes the facilities of the operating system available to interactive users. There are several popular Unix shells: `sh` (the Bourne shell), `csh` (the C shell), and `bash` (the Bourne Again shell) are a few.

The purpose of this assignment is to help you learn about Unix processes, low-level input/output, and signals. It will also give you ample opportunity to define software modules; in that sense the assignment is a capstone for the course.


Your Task
---------

Your task in this assignment is to create a program named `snush`. **If your program name isn't `snush`, you cannot get any score**. Your program should be a minimal but realistic interactive Unix shell.


Building a Program
------------------

Makefile for building a program is already prepared. What you need to do is just build a program with `make` command. The submission you submit should of course also be able to be compiled using the make command. You cannot receive 0 points if it is not compiled with 'make'

Initialization and Termination
------------------------------

Your program should print a **percent sign and a space (% )** before each such line. To facilitate your debugging and our testing, set `export DEBUG=1` before you run your snush. For clean running of your shell do `unset DEBUG`. Make sure the DEBUG environment variable is set by checking with `echo $DEBUG`.

Your program should terminate when the user types Ctrl-d or issues the `exit` command. (See also the section below entitled "Signal Handling.")


Interactive Operation
---------------------

After start-up processing, your program repeatedly should perform these actions:

*   Print a prompt, which consists of a percent sign followed by a space, to the standard output stream.
*   Read a line from the standard input stream.
*   Lexically analyze the line to form an array of tokens.
*   Syntactically analyze the token array.
*   Form a command line for execution.
*   Execute the command.

Lexical Analysis
----------------

Informally, a _token_ should be a word. More formally, a token should consist of a sequence of non-white-space characters that are separated from other tokens by white-space characters. There should be two exceptions:

*   The special characters '>', and '<' should form separate tokens.
*   Strings enclosed in double quotes (") or single quotes(') should form part or all of a single token. Special characters inside of strings should not form separate tokens.

Your program should assume that no line of the standard input stream contains more than 1023 characters; the terminating newline character is included in that count. In other words, your program should assume that a string composed from a line of input can fit in an array of characters of length 1024. If a line of the standard input stream is longer than 1023 characters, then your program need not handle it properly; but it should not corrupt memory.

Syntactic Analysis
------------------

A _command_ should be a sequence of tokens, the first of which specifies the command name.

Execution
---------

Your program should interpret four shell built-in commands:

`cd [_dir_]`

Your program should change its working directory to `_dir_`, or to the HOME directory if `_dir_` is omitted.

`exit`

Your program should exit with exit status 0.

Note that those built-in commands should neither read from the standard input stream nor write to the standard output stream. Your program should print an error message if there is any file redirection with those built-in commands.

If the command is not a built-in command, then your program should consider the command name to be the name of a file that contains code to be executed. Your program should fork a child process and pass the file name, along with its arguments, to the `execvp` system call. If the attempt to execute the file fails, then your program should print an error message indicating the reason for the failure.

Process Handling
----------------

*   All child processes created by your program should run in the foreground, meaning the shell should wait for these processes to complete before accepting new commands.
*   You are required to use wait to ensure each foreground child process finishes execution properly, allowing your program to manage resources and maintain process control effectively.

Signal Handling
---------------

**\[NOTE\] Ctrl-d represents EOF, not a signal. Do NOT make a signal handler for Ctrl-d.**

When the user types Ctrl-c, Linux sends a SIGINT signal to the parent process and its children. Upon receiving a SIGINT signal:

*   The parent process should ignore the SIGINT signal.
*   A child process should not necessarily ignore the SIGINT signal. That is, unless the child process itself (beyond the control of parent process) has installed a handler for SIGINT signals, the child process should terminate.

Redirection
-----------

You are going to implement redirection of standard input and standard output.

*   The special character '<' and '>' should form separate token in lexical analysis.
*   The '<' token should indicate that the following token is a name of a file. Your program should redirect the command's standard input to that file. It should be an error to redirect a command's standard input stream more than once.
*   The '>' token should indicate that the following token is a name of a file. Your program should redirect the command's standard output to that file. It should be an error to redirect a command's standard output stream more than once.
*   If the standard input stream is redirected to a file that does not exist, then your program should print an appropriate error message.
*   If the standard output stream is redirected to a file that does not exist, then your program should create it. If the standard output stream is redirected to a file that already exists, then your program should destroy the file's contents and rewrite the file from scratch. 

Pipe
-----------

You are going to implement pipe between commands to redirect output from one command to the input of another.

*   The special character | should be treated as a separate token during lexical analysis.
*   The | token should indicate that the standard output of the command before | should be redirected to the standard input of the command after |.
*   Multiple pipes should be supported, allowing chaining of commands (e.g., cmd1 | cmd2 | cmd3).



Error Handling
--------------

Return value/error checking is mandatory when doing the assignment to ensure proper error handling

**Your program should handle all user errors. It should be impossible for the user's input to cause your program to crash.**


Logistics
---------

Develop on lab machines. Use your favorite edtor to create source code. Use `make` to automate the build process. Use `gdb` to debug.

We also provide the [interface](dynarray.h) and [implementation](dynarray.c) of the `dynarray` ADT. You are welcome to use that ADT in your program.

Your `readme` file should contain:

*   Your name and the name and the student ID.
*   (Optionally) An indication of how much time you spent doing the assignment.
*   (Optionally) Your assessment of the assignment.
*   (Optionally) Any information that will help us to grade your work in the most favorable light. In particular you should describe all known bugs.

Submission
----------

Your submission should be one gzipped tar file whose name is

YourStudentID\_assign4.tar.gz

When you submit the code, submit just one copy to the submission link if you work in a team.

Your submission need to include the following files:

*   Your source code files. (If you used `dynarray` ADT, then submit the `dynarray.h` and `dynarray.c` files as well.)
*   `Makefile`. The first dependency rule should build your entire program. The `Makefile` should maintain object (.o) files to allow for partial builds, and encode the dependencies among the files that comprise your program. As always, use the `gcc800` command to build.
*   A `readme` file.

Your submission file should look like this:

202412345\_assign4.tar.gz 

    202412345\_assign4

        your\_source\_code.c (can be any name or multiple files)

        your\_header.h (can be any name or multiple files)

        Makefile

        readme



Grading
-------

If your submission file does not contain the expected files, or your code cannot be compiled  with `gcc800`, we cannot give you any points. Please double check before you submit.



| Test                                      | Score |
|-------------------------------------------|-------|
| Redirection input test                    | 5     |
| Redirection output test                   | 10     |
| Finding execution file test               | 5     |
| Single pipe test                          | 10     |
| Multiple pipe test                        | 12    |
| Multiple pipe and output redirection test | 20    |
| Slow pipe test                            | 12    |
| Multiple slow pipe and output redirection test | 22 |
| Interrupt on single process test          | 2     |
| Interrupt on multiple process test        | 2     |
| **Total**                                 | **100** |

Note: The total score for all tests before the Background process test is 100 points. 

Extra Credit (extra 30 points)
-------------------------------------------------------------------------

To earn extra credit, implement background process support by enabling processes to run in the background, adding them to a job list, and handling their termination efficiently.
You must pass test11 for the Background test to receive the 30 points extra credit.

| Test                                      | Score |
|-------------------------------------------|-------|
| Redirection input test                    | 5     |
| Redirection output test                   | 10     |
| Finding execution file test               | 5     |
| Single pipe test                          | 10     |
| Multiple pipe test                        | 12    |
| Multiple pipe and output redirection test | 20    |
| Slow pipe test                            | 12    |
| Multiple slow pipe and output redirection test | 22 |
| Interrupt on single process test          | 2     |
| Interrupt on multiple process test        | 2     |
| **Background process test (Extra Credit)**    | **30** |
| **Total**                                 | **130** |