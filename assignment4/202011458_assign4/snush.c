/*---------------------------------------------------------------------------*/
/* snush.c                                                                   */
/* 2020-11458 Kimsungjin                                                     */
/* This code contains main function of snush and signal handler              */
/*---------------------------------------------------------------------------*/

#include "util.h"
#include "token.h"
#include "dynarray.h"
#include "execute.h"
#include "lexsyn.h"
#include "snush.h"

/*
        //
        // TODO-start: global variables in snush.c
        //

        You may add global variables for handling background processes

        //
        // TODO-end: global variables in snush.c
        //
*/
int total_bg_cnt;
int *bg_array;
int *bg_array2;
int *bg_done_array;
int total_bg_done_cnt;

/*---------------------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/* cleanup:
 * Free global variables : bg_array, bg_array2, bg_done_array
 */
/*--------------------------------------------------------------*/
void cleanup() {
    /*
        //
        // TODO-start: cleanup() in snush.c
        //

        You need to free dynamically allocated data structures

        //
        // TODO-end: cleanup() in snush.c
        //
    */
   free(bg_array);
   free(bg_array2);
   free(bg_done_array);
}
/*---------------------------------------------------------------------------*/
/*--------------------------------------------------------------*/
/* check_bg_status:
 * Scan bg_done_array and print message to stdout of each done process.
 * Set the value of bg_done_array and total_bg_done_cnt to 0.
 */
/*--------------------------------------------------------------*/
void check_bg_status() {
    /*
        //
        // TODO-start: check_bg_status() in snush.c
        //

        The message "background process done" is not printed by the sigzombie_handler as soon as it is finished, but is printed here if there is any input at the command prompt.

        //
        // TODO-end: check_bg_status() in snush.c
        //
    */
   for(int i=0; i<total_bg_done_cnt; i++){
        printf("[%d] Done background process group\n", bg_done_array[i]);
        bg_done_array[i] = 0;
   }
   total_bg_done_cnt = 0;
}
/*---------------------------------------------------------------------------*/
/* Whenever a child process terminates, this handler handles all zombies. */
/*--------------------------------------------------------------*/
/* sigzombie_handler:
 * signo : signal number
 * Use global variables : bg_array, bg_array2, total_bg_cnt,
                          bg_done_array, total_bg_done_cnt
 */
/*--------------------------------------------------------------*/
static void sigzombie_handler(int signo) {
    pid_t pid;
    int stat;

    if (signo == SIGCHLD) {
        
        while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
            //
            // TODO-start: sigzombie_handler() in snush.c start 
            //
            for(int i=0; i<MAX_BG_PRO; i++){
                if(bg_array[i] == pid){
                    bg_array[i] = 0;
                    total_bg_cnt--;
                    if(bg_array2[i] == 1){
                        bg_done_array[total_bg_done_cnt++] = pid;
                    }
                    bg_array2[i] = 0;
                    break;
                }
            }
            //
            // TODO-end: sigzombie_handler() in snush.c end
            // 
        }

        if (pid < 0 && errno != ECHILD && errno != EINTR) {
            perror("waitpid");
        }
    }

    return;
}
/*---------------------------------------------------------------------------*/
static void shell_helper(const char *in_line) {
    DynArray_T oTokens;

    enum LexResult lexcheck;
    enum SyntaxResult syncheck;
    enum BuiltinType btype;
    int pcount;
    int ret_pgid; // background pid
    int is_background;

    oTokens = dynarray_new(0);
    if (oTokens == NULL) {
        error_print("Cannot allocate memory", FPRINTF);
        exit(EXIT_FAILURE);
    }

    lexcheck = lex_line(in_line, oTokens);
    switch (lexcheck) {
    case LEX_SUCCESS:
        if (dynarray_get_length(oTokens) == 0)
            return;

        /* dump lex result when DEBUG is set */
        dump_lex(oTokens);

        syncheck = syntax_check(oTokens);
        if (syncheck == SYN_SUCCESS) {
            btype = check_builtin(dynarray_get(oTokens, 0));
            if (btype == NORMAL) {
                is_background = check_bg(oTokens);

                pcount = count_pipe(oTokens);

                if (is_background) {
                    if (total_bg_cnt + pcount + 1 > MAX_BG_PRO) {
                        printf("Error: Total background processes "\
                                    "exceed the limit (%d).\n", MAX_BG_PRO);
                        return;
                    }
                }

                if (pcount > 0) {
                    ret_pgid = iter_pipe_fork_exec(pcount, oTokens, 
                                                is_background);
                }
                else {
                    ret_pgid = fork_exec(oTokens, is_background);
                }

                if (ret_pgid > 0) {
                    if (is_background == 1)
                        printf("[%d] Background process group running\n", 
                                    ret_pgid);
                }
                else {
                    printf("Invalid return value "\
                        "of external command execution\n");
                }
            }
            else {
                /* Execute builtin command */
                execute_builtin(oTokens, btype);
            }
        }

        /* syntax error cases */
        else if (syncheck == SYN_FAIL_NOCMD)
            error_print("Missing command name", FPRINTF);
        else if (syncheck == SYN_FAIL_MULTREDOUT)
            error_print("Multiple redirection of standard out", FPRINTF);
        else if (syncheck == SYN_FAIL_NODESTOUT)
            error_print("Standard output redirection without file name", 
                        FPRINTF);
        else if (syncheck == SYN_FAIL_MULTREDIN)
            error_print("Multiple redirection of standard input", FPRINTF);
        else if (syncheck == SYN_FAIL_NODESTIN)
            error_print("Standard input redirection without file name", 
                        FPRINTF);
        else if (syncheck == SYN_FAIL_INVALIDBG)
            error_print("Invalid use of background", FPRINTF);
        break;

    case LEX_QERROR:
        error_print("Unmatched quote", FPRINTF);
        break;

    case LEX_NOMEM:
        error_print("Cannot allocate memory", FPRINTF);
        break;

    case LEX_LONG:
        error_print("Command is too large", FPRINTF);
        break;

    default:
        error_print("lex_line needs to be fixed", FPRINTF);
        exit(EXIT_FAILURE);
    }

    /* Free memories allocated to tokens */
    dynarray_map(oTokens, free_token, NULL);
    dynarray_free(oTokens);
}
/*---------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    sigset_t sigset;
    char c_line[MAX_LINE_SIZE + 2];

    atexit(cleanup);

    /* Initialize variables for background processes */
    total_bg_cnt = 0;
    total_bg_done_cnt = 0;

    bg_array = calloc(MAX_BG_PRO, sizeof(int));
    bg_array2 = calloc(MAX_BG_PRO, sizeof(int));
    bg_done_array = calloc(MAX_BG_PRO, sizeof(int));

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGCHLD);
    sigaddset(&sigset, SIGQUIT);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
 
    /* Register signal handler */
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, sigzombie_handler);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    error_print(argv[0], SETUP);

    while (1) {
        check_bg_status();
        fprintf(stdout, "%% ");
        fflush(stdout);

        if (fgets(c_line, MAX_LINE_SIZE, stdin) == NULL) {
            printf("\n");
            exit(EXIT_SUCCESS);
        }        
        shell_helper(c_line);
    }

    return 0;
}
/*---------------------------------------------------------------------------*/