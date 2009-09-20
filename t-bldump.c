/*!
 * @file
 * @brief unit test of 'bldump.c'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "CUnit/CUnit.h"

#include "bldump.h"

FILE *t_stdin, *t_stdout, *t_stderr;
int t_exit_count;
int t_exit_code;
void t_exit(int c) {
	t_exit_count++;
	t_exit_code = c;
}

int ts_bldump_init(void)
{
	t_stdin  = tmpfile();
	t_stdout = tmpfile();
	t_stderr = tmpfile();
	t_exit_count = 0;

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
	return 0;
}

void tc_bldump_options(void)
{
	int ret;

	t_exit_count = 0;
	/* -h */
	{
		char* argv[] = { "bldump", "-h" };
		main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( t_exit_count == 1 );
		CU_ASSERT( t_exit_code == EXIT_FAILURE );
	}
	/* -? */
	{
		char* argv[] = { "bldump", "-?" };
		main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( t_exit_count == 2 );
		CU_ASSERT( t_exit_code == EXIT_FAILURE );
	}
	/* --help */
	{
		char* argv[] = { "bldump", "--help" };
		main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( t_exit_count == 3 );
		CU_ASSERT( t_exit_code == EXIT_FAILURE );
	}
	
}

/*!
 * @brief test bldump::help().
 */
void tc_bldump_help(void)
{
	long stderr_pos;

	t_exit_count = 0;
	stderr_pos = ftell(t_stderr);
	help();
	stderr_pos = ftell(t_stderr);

	CU_ASSERT(t_exit_count == 1);
	CU_ASSERT(stderr_pos > 0);
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

