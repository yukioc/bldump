/*!
 * @file
 * @brief unit test of CLI.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "CUnit/CUnit.h"

#include "verbose.h"
#include "bldump.h"

int ts_cli_init(void)
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

int ts_cli_cleanup(void)
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

/*!
 * @biref test "bldump [-]".
 */
void tc_cli_wrong(void)
{
	int ret;

	/* - */
	{
		char* argv[] = { "bldump", "-" };
		ret = main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == EXIT_FAILURE );
	}
	/* no argument */
	{
		char* argv[] = { "bldump" };
		ret = main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == EXIT_FAILURE );
	}
}


/*!
 * @biref test "bldump -h".
 */
void tc_cli_help(void)
{
	int ret;
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
}

/*!
 * @brief test infile
 */
void tc_cli_infile(void)
{
	int ret;

	/* non-existing */
	{
		char* argv[] = { "bldump", "non-existing.tmp" };
		ret = main( sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == EXIT_FAILURE );
	}

}

CU_ErrorCode ts_cli_regist(void)
{
	CU_TestInfo ts_cli_cases[] = {
		{ "bldump [-]",				tc_cli_wrong },
		{ "bldump (-h|-?|--help)",	tc_cli_help },
		{ "bldump infile",			tc_cli_infile },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "CLI", ts_cli_init, ts_cli_cleanup, ts_cli_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

