/*!
 * @file
 * @brief bldump - simple decimal, hex dump to text file, CSV ..
 * @author yukioc
 * @since 2009-09-20 
 * @date 2009-09-20 Initial commit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CUNIT
#include "CUnit/Basic.h"
#endif
#include "bldump.h"

#define VERSION		"Version 0.01"
#define DATE		"(" __DATE__ ")"
#define COPYRIGHT	"by yukio"
#define ABOUT		"[bldump] simple decimal, hex dump to text file, CSV .. " VERSION " " DATE " " COPYRIGHT "\n\n"

const char *usage[] = {
	"Usage: bldump [options] [infile [outfile]]",
	"",
	"  infile  dump file.",
	"  outfile output(stdout).",
	"",
	"options:",
	"  -h -? --help",
	"    this help.",
	""
};

#ifdef CUNIT
FILE *t_stdin, *t_stdout, *t_stderr;
void t_exit(int);

#define STDIN	t_stdin
#define STDOUT	t_stdout
#define STDERR	t_stderr
#define EXIT	t_exit
#else
#define STDIN	stdin
#define STDOUT	stdout
#define STDERR	stderr
#define EXIT	exit
#endif

/*!
 * @brief bldump main function.
 * @retvalue 0 normal termination.
 */
int main( int argc, char* argv[] ) {
	int ret;

#ifdef CUNIT
	/* run test */
	if ( argc == 1 && strncmp("bldump-test", argv[0], 11)  ) {
		extern int ts_bldump_regist();
		CU_ErrorCode cue;
		cue = CU_initialize_registry();
		if ( cue == CUE_SUCCESS ) cue == ts_bldump_regist();
		/* use CUnit Basic I/F */
		{
			unsigned int fails;
			CU_basic_set_mode(CU_BRM_VERBOSE);
			CU_basic_run_tests();
			fails = CU_get_number_of_failures();
			CU_cleanup_registry();
		}
		return CU_get_error();
	}
#endif

	/*** arguments ***/
	{
		int i;
		size_t a; /* for macro */
		char *sub;
		char *endp;

#define strlcmp(l,r) (strncmp(l,r,strlen(l)))
#define ARG_FLAG(s) (strcmp(s,argv[i])==0)
#define ARG_SPARAM(s) (strcmp(s,argv[i])==0 && (i+1) < argc && (sub=argv[++i]))
#define ARG_LPARAM(s) (strlcmp(s,argv[i])==0 && (a=strlen(s))<strlen(argv[i]) && (sub=&argv[i][a]))
		for (i = 1; i < argc; i++) {
			if (ARG_FLAG("-?") || ARG_FLAG("-h") || ARG_FLAG("--help")) {
				help();
			}
		}
	}

	return 0;
}

/*!
 * @brief display help message.
 */
void help(void) //{{{
{
	int i, size;
	size = (int)(sizeof(usage)/sizeof(const char*));
	for( i=0; i<size; i++ ) fprintf(STDERR, "%s\n", usage[i] );
	EXIT( EXIT_FAILURE );
}

//}}}
/* vim:fdm=marker:
 */
