/* Do not modify this file */

#ifndef PARSE_H
#define PARSE_H

#include <ctype.h> /* isspace */

/* Types */
typedef enum control_enum {NONE, AND, OR} Control;
typedef struct cmd_aux_struct{
    char *in_file;
    char *out_file;
    int is_append;
    int is_bg;
    Control control;
}Cmd_aux;

/* The record that keeps additional information about the user command.
 * - in_file: input file name specified by user command (with <); 
 *            it should be NULL if no redirection (with <);
 * - out_file: output file name specified by user command (with > or >>), 
 *            it should be NULL if no redirection (with > or >>);  
 * - is_append: int flag to specify how to change the output file:
 *               --  1 : the output file should be appended
 *               --  0 : the output file should be erased
 *               --  -1: no output file specified
 * - is_bg: int flag to speficy whether the job is background:
 *           -- 1 : background job
 *           -- 0 : foreground job
 * - control: control operator in user command; can be AND (&&), OR(||) or None
 */

/* Command Parsing Functions */
void parse(char *cmd_line, char *argv[], char *argv2[], Cmd_aux *aux);
/* The required function to parse the user command into useful pieces.
 * - cmd_line: the line typed in by user WITHOUT the ending \n.
 * - argv: the array of NULL terminated char pointers. 
 *   	-- argv[0] should be either a built-in command, or the name of the
 *   	   program to be loaded and executed;
 *   	-- the remainder of argv[] should be the list of arguments used 
 *   	   to run the command/file
 *   	-- argv[] must have NULL as its last member after all arguments
 * - argv2: the same format as argv; only used when the command line has 
 *          two commands connected with a control operator.
 * - aux: the pointer to a Cmd_aux record which should be filled with
 *        the other information regarding the task as defined for Cmd_aux 
 */

/* String Processing Functions */
int is_whitespace(char *str);

/* Initialization & Free Functions */
void initialize_argv(char *argv[]);
void initialize_aux(Cmd_aux *aux);
void free_options(char **cmd, char *argv[], char *argv2[], Cmd_aux *aux);
void free_argv(char *argv[]);

/* Debug Functions */
void debug_print_parse(char *cmdline, char *argv[], char *argv2[], Cmd_aux *aux, char *loc);

#endif /*PARSE_H*/
