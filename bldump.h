/*!
 * @file
 * @brief bldump header file.
 */

#ifndef __bldump_h__
#define __bldump_h__

typedef struct {
	char*		infile_name;	/*!< argument infile */
	char*		outfile_name;	/*!< argument outfile */

} options_t;

int main( int argc, char* argv[] );
int help(void);

#ifdef CUNIT
extern FILE *t_stdin, *t_stdout, *t_stderr;
#endif

#endif

