/* DO NOT modify this file.
 * This is the provided parsing facility. 
 * You can call parse() to divide the user command line into useful pieces. */

#include "parse.h"
#include "shell.h"

/* Helper Functions */
static int parse_token(char *p_tok, char *argv[], int index, Cmd_aux *aux, int *offset);
static int parse_redirect_token(char *p_tok, Cmd_aux *aux);

/*********
 * Command Parsing Functions
 *********/

void parse(char *cmd_line, char *argv[], char *argv2[], Cmd_aux *aux) {
  /* Step 1: Only work on a copy of the original command */ 
  char *p_tok = NULL;
  char buffer[MAXLINE] = {0};
  strncpy(buffer, cmd_line, MAXLINE);


  /* Step 2: Tokenize the inputs (space delim) and parse */
  p_tok = strtok(buffer, " ");
  if(p_tok == NULL) {
    return;
  }
  int index = 0;
  int offset = 0;

  aux->control = NONE;
  while(p_tok != NULL) {
    if (offset == 0)
      index = parse_token(p_tok, argv, index, aux, &offset); 
    else{
      index = parse_token(p_tok, argv2, index, aux, &offset); 
    }
    p_tok = strtok(NULL, " ");
  }
}

/* Parse the next token from the original command */
static int parse_token(char *p_tok, char *argv[], int index, Cmd_aux *aux, int* offset) {
  char *arg = NULL;
  argv[index-(*offset)] = NULL;
  if(p_tok == NULL) {
    return index;
  }

  if(*p_tok == '>' || *p_tok == '<') {
    if(parse_redirect_token(p_tok, aux) != 0) {
      return index;
    }
  }

  else if(strncmp(p_tok, "&&", 2) == 0) {
    *offset = index;
    aux->control = AND;
  }
  else if(strncmp(p_tok, "||", 2) == 0) {
    *offset = index;
    aux->control = OR;
  }

  else if(*p_tok == '&') {
    aux->is_bg = 1;
  }
  else {
    arg = malloc(strlen(p_tok) + 1);
    snprintf(arg, strlen(p_tok) + 1, "%s", p_tok);
    argv[index- (*offset)] = arg;
    index++;
  }

  return index;
}


/* Parse the next token from the original command */
static int parse_redirect_token(char *p_tok, Cmd_aux *aux) {
  char *filename = NULL;
  if(*p_tok == '>') {
    if(strncmp(p_tok, ">>", 2) == 0) {
      aux->is_append = 1;
    } 
    else {
      aux->is_append = 0;
    }
    p_tok = strtok(NULL, " ");
    if(p_tok != NULL) {
      filename = malloc(strlen(p_tok) + 1);
      snprintf(filename, strlen(p_tok) + 1, "%s", p_tok);
      aux->out_file = filename;
    }
    else {
      return -1;
    }
  }
  else if(*p_tok == '<') {
    p_tok = strtok(NULL, " ");
    if(p_tok != NULL) {
      filename = malloc(strlen(p_tok) + 1);
      snprintf(filename, strlen(p_tok) + 1, "%s", p_tok);
      aux->in_file = filename;
    }
    else {
      return -1;
    }
  }

  return 0;
}

/*********
 * String Processing Helpers
 *********/

/* Returns 1 if string is all whitespace, else 0 */
int is_whitespace(char *str) {
  while(isspace(*str)) {
    ++str;
  }
  return *str == '\0';
}

/*********
 * Initialization Functions
 *********/

void initialize_argv(char *argv[]) {
  int i = 0;
  for(i = 0; i < MAXARGS; i++) {
    argv[i] = NULL;
  }
}

void initialize_aux(Cmd_aux *aux) {
  aux->in_file = NULL;
  aux->out_file = NULL;
  aux->is_append = -1;
  aux->is_bg = 0;
  aux->control = 0;
}

void free_argv(char *argv[]) {
  int i;

  for(i = 0; argv[i]; i++) {
    free(argv[i]);
    argv[i] = NULL;
  }
}

void free_options(char **cmd, char *argv[], char *argv2[], Cmd_aux *aux) {
  if(*cmd) {
    free(*cmd);
    *cmd = NULL;
  }

  free_argv(argv);
  free_argv(argv2);

  if(aux->in_file) {
    free(aux->in_file);
    aux->in_file = NULL;
  }
  if(aux->out_file) {
    free(aux->out_file);
    aux->out_file = NULL;
  }
}

/*********
 * Debug Functions
 *********/
void debug_print_parse(char *cmdline, char *argv[], char *argv2[], Cmd_aux *aux, char *loc) {
  int i = 0;
  printf("\n-------------\n");
  printf("- %s\n", loc);
  printf("-------------\n");

  if(cmdline) {
    printf("cmdline = %s\n", cmdline);
  }

  if(argv != NULL) {
    for(i = 0; argv[i]; i++) {
      printf("argv[%d] == %s\n", i, argv[i]);
    }
  }
  if(argv2 != NULL) {
    for(i = 0; argv2[i]; i++) {
      printf("argv2[%d] == %s\n", i, argv2[i]);
    }
  }
  if(aux) {
    printf("Aux:\n");
    printf("in_file   = %s\n", aux->in_file);
    printf("out_file  = %s\n", aux->out_file);
    printf("is_bg     = %d\n", aux->is_bg);
    printf("is_append = %d\n", aux->is_append);
    printf("control   = %d\n", aux->control);
  }
  printf("-------------\n");
  return;
}
