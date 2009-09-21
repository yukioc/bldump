/*!
 * @file
 * @brief unit test of 'bldump.c'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "CUnit/CUnit.h"

#include "verbose.h"
#include "bldump.h"

FILE *t_stdin, *t_stdout, *t_stderr;

int ts_bldump_init(void)
{
	if ( verbose_out != stdout ) {
		fprintf( stderr, "Error: verbose_out is not stdout\n" );
		return 1;
	}

	t_stdin  = tmpfile();
	t_stdout = tmpfile();
	t_stderr = tmpfile();
	verbose_out = tmpfile();

	if ( t_stdin == NULL || t_stdout == NULL || t_stderr == NULL ) {
		fprintf( stderr, "Error: tmpfile() failure - errno = %d.\n", errno );
		return 1;
	}

	return 0;
}

int ts_bldump_cleanup(void)
{
	fclose( t_stdin  );
	fclose( t_stdout );
	fclose( t_stderr );
	t_stdin = NULL;
	t_stdout = NULL;
	t_stderr = NULL;
	verbose_out = stdout;

	return 0;
}

void tc_bldump_options(void)
{
	int ret;

	t_exit_code  = -1;
	t_exit_count = 0;

	/* -h */
	{
		char* argv[] = { "bldump", "-h" };
		ret = main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == EXIT_FAILURE );
	}
	/* -? */
	{
		char* argv[] = { "bldump", "-?" };
		ret = main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == EXIT_FAILURE );
	}
	/* --help */
	{
		char* argv[] = { "bldump", "--help" };
		ret = main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == EXIT_FAILURE );
	}
	/* - */
	{
		char* argv[] = { "bldump", "-" };
		ret = main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( t_exit_code == EXIT_FAILURE );
		CU_ASSERT( t_exit_count == 1 );
	}
}

/*!
 * @brief test bldump::help().
 */
void tc_bldump_help(void)
{
	long pos1, pos2;
	int ret;

	pos1 = ftell(t_stderr);
	help();
	pos2 = ftell(t_stderr);

	CU_ASSERT(t_exit_count == 1);
	CU_ASSERT(pos2 - pos1 > 0); /* count of the help message */
}

CU_ErrorCode ts_bldump_regist(void)
{
	CU_TestInfo ts_bldump_cases[] = {
		{ "options",	tc_bldump_options },
		{ "help()",		tc_bldump_help },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "bldump", ts_bldump_init, ts_bldump_cleanup, ts_bldump_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

