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
 * @biref test "bldump -".
 */
void tc_cli_noopt(void)
{
	options_t opt;
	int ret;
	char* argv[] = { "bldump", "-" };
	options_reset( &opt );
	ret = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT( ret == 1 );
}

/*!
 * @biref test "bldump".
 */
void tc_cli_noarg(void)
{
	options_t opt;
	int ret;
	char* argv[] = { "bldump" };
	options_reset( &opt );
	ret = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT( ret == 1 );
}

/*!
 * @biref test "bldump -h".
 */
void tc_cli_help(void)
{
	options_t opt;
	int ret;

	/* -h */
	{
		char* argv[] = { "bldump", "-h" };
		options_reset( &opt );
		ret = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == 1 );
	}
	/* -? */
	{
		char* argv[] = { "bldump", "-?" };
		options_reset( &opt );
		ret = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == 1 );
	}
	/* --help */
	{
		char* argv[] = { "bldump", "--help" };
		options_reset( &opt );
		ret = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( ret == 1 );
	}
}

/*!
 * @brief test infile
 */
void tc_cli_infile(void)
{
	options_t opt;
	int ret;
	char* argv[] = { "bldump", "infile" };
	options_reset( &opt );
	ret = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT( ret == 0 );
	CU_ASSERT( strcmp( opt.infile_name, "infile" ) == 0 );
}

CU_ErrorCode ts_cli_regist(void)
{
	CU_TestInfo ts_cli_cases[] = {
		{ "bldump",					tc_cli_noarg },
		{ "bldump -",				tc_cli_noopt },
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

