/*!
 * @file
 * @brief unit test of 'main.c'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "CUnit/CUnit.h"

#include "verbose.h"
#include "bldump.h"

static int ts_main_init(void)
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

static int ts_main_cleanup(void)
{
	(void) fclose( t_stdin  );
	(void) fclose( t_stdout );
	(void) fclose( t_stderr );
	t_stdin = NULL;
	t_stdout = NULL;
	t_stderr = NULL;
	verbose_out = stdout;

	return 0;
}

/*!
 * @brief test bldump::help().
 */
static void tc_main(void)
{
	/* bldump (noopt) */
	int ret;
	char* argv[] = { "bldump" };

	ret = main( sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT_EQUAL( ret, EXIT_FAILURE );
}

CU_ErrorCode ts_main_regist(void)
{
	CU_TestInfo ts_main_cases[] = {
		{ "bldump",					tc_main },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "main", ts_main_init, ts_main_cleanup, ts_main_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

