/*---------------------------------------------------------------------------*/
/* execute.c                                                                 */
/* Author: Jongki Park, Kyoungsoo Park                                       */
/*---------------------------------------------------------------------------*/

#include "dynarray.h"
#include "token.h"
#include "util.h"
#include "lexsyn.h"
#include "snush.h"
#include "execute.h"

extern volatile int bg_array_idx;
extern int *bg_array;
extern int bg_cnt;

/*---------------------------------------------------------------------------*/
void redout_handler(char *fname) {
	//
	// TODO-start: redout_handler() in execute.c
	// 
	int fd;

	fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
	if (fd < 0) {
		error_print(NULL, PERROR);
		exit(EXIT_FAILURE);
	}
	else {
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
	//
	// TODO-end redout_handler() in execute.c
	// 
}
/*---------------------------------------------------------------------------*/
void redin_handler(char *fname) {
	int fd;

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		error_print(NULL, PERROR);
		exit(EXIT_FAILURE);
	}
	else {
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
}
/*---------------------------------------------------------------------------*/
int build_command_partial(DynArray_T oTokens, int start, 
						int end, char *args[]) {
	int i, ret = 0, redin = FALSE, redout = FALSE, cnt = 0;
	struct Token *t;

	/* Build command */
	for (i = start; i < end; i++) {

		t = dynarray_get(oTokens, i);

		if (t->token_type == TOKEN_WORD) {
			if (redin == TRUE) {
				redin_handler(t->token_value);
				redin = FALSE;
			}
			else if (redout == TRUE) {
				redout_handler(t->token_value);
				redout = FALSE;
			}
			else
				args[cnt++] = t->token_value;
		}
		else if (t->token_type == TOKEN_REDIN)
			redin = TRUE;
		else if (t->token_type == TOKEN_REDOUT)
			redout = TRUE;
	}
	args[cnt] = NULL;

#ifdef DEBUG
	for (i = 0; i < cnt; i++)
	{
		if (args[i] == NULL)
			printf("CMD: NULL\n");
		else
			printf("CMD: %s\n", args[i]);
	}
	printf("END\n");
#endif
	return ret;
}
/*---------------------------------------------------------------------------*/
int build_command(DynArray_T oTokens, char *args[]) {
	return build_command_partial(oTokens, 0, 
								dynarray_get_length(oTokens), args);
}
/*---------------------------------------------------------------------------*/
void execute_builtin(DynArray_T oTokens, enum BuiltinType btype) {
	int ret;
	char *dir = NULL;
	struct Token *t1;

	switch (btype) {
	case B_EXIT:
		if (dynarray_get_length(oTokens) == 1) {
			// printf("\n");
			dynarray_map(oTokens, free_token, NULL);
			dynarray_free(oTokens);

			exit(EXIT_SUCCESS);
		}
		else
			error_print("exit does not take any parameters", FPRINTF);

		break;

	case B_CD:
		if (dynarray_get_length(oTokens) == 1) {
			dir = getenv("HOME");
			if (dir == NULL) {
				error_print("cd: HOME variable not set", FPRINTF);
				break;
			}
		}
		else if (dynarray_get_length(oTokens) == 2) {
			t1 = dynarray_get(oTokens, 1);
			if (t1->token_type == TOKEN_WORD)
				dir = t1->token_value;
		}

		if (dir == NULL) {
			error_print("cd takes one parameter", FPRINTF);
			break;
		}
		else {
			ret = chdir(dir);
			if (ret < 0)
				error_print(NULL, PERROR);
		}
		break;

	default:
		error_print("Bug found in execute_builtin", FPRINTF);
		exit(EXIT_FAILURE);
	}
}
/*---------------------------------------------------------------------------*/
/* Important Notice!! 
	Add "signal(SIGINT, SIG_DFL);" after fork
*/
int fork_exec(DynArray_T oTokens, int is_background) {
	//
	// TODO-START: fork_exec() in execute.c 
	// 
	int ret = 1;

	char *args[MAX_ARGS_CNT] = {NULL};
	int pid = fork();
	
	if(pid == 0){ // child process
		signal(SIGINT, SIG_DFL);
		build_command(oTokens, args);
		execvp(args[0], args);
	}
	else{ // parent process
		if(!is_background){
			waitpid(pid, NULL, 0);
		}
		else{
			for(int i=0; i<MAX_BG_PRO; i++){
                if(bg_array[i] == 0){
                    bg_array[i] = pid;
                    bg_array_idx++;
                    break;
                }
            }
		}
	}
	ret = pid;
	
	return ret;
	//
	// TODO-END: fork_exec() in execute.c
	//
}
/*---------------------------------------------------------------------------*/
/* Important Notice!! 
	Add "signal(SIGINT, SIG_DFL);" after fork
*/
int iter_pipe_fork_exec(int pcount, DynArray_T oTokens, int is_background) {
	//
	// TODO-START: fork_exec() in execute.c 
	// 
	int ret = 1;

	int pipefd[2];
	int pids[pcount + 1];
	
	struct Token *t;
	int prev = 0;
	int process_cnt = 0;
	int in_fd = STDIN_FILENO;
	for (int i = 0; i < dynarray_get_length(oTokens); i++){
		t = dynarray_get(oTokens, i);
		if (t->token_type == TOKEN_PIPE){
			char *args[MAX_ARGS_CNT];
			if(pipe(pipefd) == -1) {
				error_print("pipe failed", PERROR);
				exit(EXIT_FAILURE);
			}
			pids[process_cnt] = fork();

			if(pids[process_cnt] == 0){ // child process
				signal(SIGINT, SIG_DFL);
				build_command_partial(oTokens, prev, i, args);
				if (in_fd != STDIN_FILENO) {
					dup2(in_fd, STDIN_FILENO);
					close(in_fd);
				}

				dup2(pipefd[1], STDOUT_FILENO);
				close(pipefd[1]);

				close(pipefd[0]);

				execvp(args[0], args);
			}
			else{ // parent process
				if (in_fd != STDIN_FILENO) {
					close(in_fd);
				}
				close(pipefd[1]);
				in_fd = pipefd[0];
			}
			
			process_cnt += 1;
			prev = i + 1;
		}
	}
	// start processing last command
	char *args[MAX_ARGS_CNT];
	pids[process_cnt] = fork();
	if(pids[process_cnt] == 0){ // child process
		signal(SIGINT, SIG_DFL);
		build_command_partial(oTokens, prev, dynarray_get_length(oTokens), args);
		dup2(in_fd, STDIN_FILENO);
		close(in_fd);
		close(pipefd[0]);

		execvp(args[0], args);
	}
	else{ // parent process
		close(in_fd);
		close(pipefd[1]);
	}

	if(!is_background){
		for(int i=0; i<pcount + 1; i++){
			waitpid(pids[i], NULL, 0);
		}
	}
	else{
		int j=0;
		for(int i=0; i<MAX_BG_PRO; i++){
			if(bg_array[i] == 0){
				bg_array[i] = pids[j];
				bg_array_idx++;
				if(j == pcount){
					break;
				}
				else{
					j++;
				}
			}
		}
	}

	ret = pids[0];

	return ret;
	//
	// TODO-END: fork_exec() in execute.c
	//
}
/*---------------------------------------------------------------------------*/