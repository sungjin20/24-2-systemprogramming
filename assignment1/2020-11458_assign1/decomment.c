// Kim SungJin, Assignment1, decomment.c

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/* This is skeleton code for reading characters from 
standard input (e.g., a file or console input) one by one until 
the end of the file (EOF) is reached. It keeps track of the current 
line number and is designed to be extended with additional 
functionality, such as processing or transforming the input data. 
In this specific task, the goal is to implement logic that removes 
C-style comments from the input. */


// Print out to stdout the input starting with ' and ending with ' and return whether EOF found
int print_char(int *ich, char *ch, int *line_cur){
  // ich: int type variable to store character input from getchar() (abbreviation of int character)
  // ch: character that comes from casting (char) on ich (abbreviation of character)
  // line_cur: current line number (abbreviation of current line)
  printf("\'");
  while(1){
    *ich = getchar();
    if(*ich == EOF){
      return 1;
    }

    *ch = (char)*ich;
    printf("%c", *ch); // print all kinds of characters

    if(*ch == '\\'){ // Handling backslash
      *ich = getchar();
      *ch = (char)*ich;
      printf("%c", *ch);
      continue;
    }

    if(*ch == '\n') (*line_cur)++;
    if(*ch == '\'') break;
  }
  return 0;
}

// Print out to stdout the input starting with " and ending with " and return whether EOF found
int print_str(int *ich, char *ch, int *line_cur){
  // ich: int type variable to store character input from getchar() (abbreviation of int character)
  // ch: character that comes from casting (char) on ich (abbreviation of character)
  // line_cur: current line number (abbreviation of current line)
  printf("\"");
  while(1){
    *ich = getchar();
    if(*ich == EOF){
      return 1; // got_eof = 1
    }

    *ch = (char)*ich;
    printf("%c", *ch); // print all kinds of characters

    if(*ch == '\\'){ // Handling backslash
      *ich = getchar();
      *ch = (char)*ich;
      printf("%c", *ch);
      continue;
    }

    if(*ch == '\n') (*line_cur)++;
    if(*ch == '\"') break;
  }
  return 0; // got_eof = 0
}

// Print out to stdout one space when input is single line comment
void decomment_sl(int *ich, char *ch){
  // ich: int type variable to store character input from getchar() (abbreviation of int character)
  // ch: character that comes from casting (char) on ich (abbreviation of character)
  printf(" ");
  while(1){
    *ich = getchar();
    if(*ich == EOF){
      break;
    }
    *ch = (char)*ich;
    if(*ch == '\n'){
      printf("%c", *ch);
      break;
    }
  }
}

// Print out to stdout one space and newlines when input is multi-line comment. Print error to stderr when unterminated comment found.
int decomment_ml(int *ich, char *ch, int *line_cur, int *line_com){
  // ich: int type variable to store character input from getchar() (abbreviation of int character)
  // ch: character that comes from casting (char) on ich (abbreviation of character)
  // line_cur & line_com: current line number and comment line number (abbreviation of current line and comment line)
  printf(" ");
  *line_com = *line_cur;
  int breakpoint = 0; // Indicate that found the end of the comment
  while(1){
    *ich = getchar();
    if(*ich == EOF){
      fprintf(stderr, "Error: line %d: unterminated comment\n", *line_com);
      return 1; // got_eof = 1
    }

    *ch = (char)*ich;
    
    // Checks whether it indicates the end of a comment
    while(*ch == '*'){
      *ich = getchar();  
      *ch = (char)*ich;
      if(*ch == '/') breakpoint = 1;
    }
    if(breakpoint) break;

    if(*ch == '\n'){
      printf("%c", *ch);
      (*line_cur)++;
    }
  }
  return 0; // got_eof = 0
}




// decomment from stdin and print result to stdout and print error to stderr
int main(void)
{
  // ich: int type variable to store character input from getchar() (abbreviation of int character)
  int ich;
  // line_cur & line_com: current line number and comment line number (abbreviation of current line and comment line)
  int line_cur, line_com;
  // ch: character that comes from casting (char) on ich (abbreviation of character)
  char ch;

  line_cur = 1;
  line_com = -1;

  // This while loop reads all characters from standard input one by one
  while (1) {
    int got_eof = 0;

    ich = getchar();
    if (ich == EOF)
      break;

    ch = (char)ich;
    // TODO: Implement the decommenting logic
    
    if(ch == '\''){ // Character constant state
      got_eof = print_char(&ich, &ch, &line_cur);
    }

    else if(ch == '\"'){ // String state
      got_eof = print_str(&ich, &ch, &line_cur);
    }

    else if(ch == '/'){ // Maybe comment state
      ich = getchar();  
      ch = (char)ich;

      if(ch == '/'){ // Single line comment state
         decomment_sl(&ich, &ch);
      }

      else if(ch == '*'){ // Multi line comment state
         if(decomment_ml(&ich, &ch, &line_cur, &line_com) != 0){
          return(EXIT_FAILURE);
         }
      }

      else{ // Normal state
        printf("/");
        
        if(ch == '\''){ // Character constant state
          got_eof = print_char(&ich, &ch, &line_cur);
        }

        else if(ch == '\"'){ // String state
          got_eof = print_str(&ich, &ch, &line_cur);
        }

        else{ // Normal state
          printf("%c", ch);
        }
      }
    }

    else{ // Normal state
      printf("%c", ch);
    }

    if (ch == '\n')
      line_cur++;
    if (got_eof)
      break;
  }
  
  return(EXIT_SUCCESS);
}