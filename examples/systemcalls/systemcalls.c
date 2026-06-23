#include "systemcalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int outcome = system(cmd);
    if (outcome != 0) {
        return false;
    }
    return true;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    /*
    verify command[0] is valid - otherwise, abort early
    */
    struct stat path_stat;
    if (stat(command[0], &path_stat) != 0) {
        perror("ERROR: invalid path provided\n");
        return false;
    }

    fflush(stdout);
    pid_t fork_pid = fork();
    if (fork_pid < 0) {
        perror("ERROR: fork() failed\n");
        return false;
    }

    // find the child
    if (fork_pid == 0) {
        // FOR DEBUGGING:
        // for(int i = 0; i < count+1; i++) {
        //     printf("command[%d]: %s\n", i, command[i]);
        // }
        
        execv(command[0], command);
        // printf("returning false now due to execev() failure case\n\n");
        // return false;
    } else {
        int execv_status;
        wait(&execv_status);
        if (WIFEXITED(execv_status)) {
            /* 
            exit code is actually what needs to be checked - not simply status
            -> errors can still exit with a normal status
            */ 
            int exit_code = WEXITSTATUS(execv_status);
            printf("child exited normally with status: %d\n", exit_code);
            if (exit_code != 0) {
                perror("ERROR: execv did not exit with a normal exit status\n");
                return false;
            }
        } else {
            /*
            still need this exit case - an abnormal status definitely indicates problems
            */
            perror("ERROR: execv failed\n");
            return false;
        }
    }

    
    va_end(args);

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    // command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    // Source - https://stackoverflow.com/a/13784315
    // Posted by tmyklebu, modified by community. See post 'Timeline' for change history
    // Retrieved 2026-06-22, License - CC BY-SA 3.0

    int kidpid;
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { perror("open"); abort(); }
    fflush(stdout);
    switch (kidpid = fork()) {
    case -1: perror("fork"); abort();
    case 0:
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("ERROR: dup2 failed\n"); abort(); }
        close(fd);
        execv(command[0], command); perror("ERROR: execv failed\n"); abort();
    default:
        close(fd);
        int execv_status;
        wait(&execv_status);
        if (WIFEXITED(execv_status)) {
            /* 
            exit code is actually what needs to be checked - not simply status
            -> errors can still exit with a normal status
            */ 
            int exit_code = WEXITSTATUS(execv_status);
            printf("child exited normally with status: %d\n", exit_code);
            if (exit_code != 0) {
                perror("ERROR: execv did not exit with a normal exit status\n");
                return false;
            }
        } else {
            /*
            still need this exit case - an abnormal status definitely indicates problems
            */
            perror("ERROR: execv failed\n");
            return false;
        }
    }


    va_end(args);

    return true;
}
