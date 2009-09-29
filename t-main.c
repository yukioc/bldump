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
 * @brief test "bldump"
 */
static void tc_main_help(void)
{
	int ret;
	char* argv[] = { "bldump" };

	ret = main( sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT_EQUAL( ret, EXIT_FAILURE );
}

/*!
 * @brief test "bldump -a"
 */
static void tc_main_hex(void)
{
	/* bldump -a */
	int ret;
	char* argv[] = { "bldump", "-a", t_tmpname };
	char* exp = "0123456789ABCDEFGHIJK";
	char act[80];

	fseek( t_stdout, 0, SEEK_SET );

	/* make input data */
	FILE* fp = fopen( t_tmpname, "wb" );
	assert( fp != NULL );
	fputs( exp, fp );
	fclose( fp );

	ret = main( sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT_EQUAL( ret, 0 );

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	fgets(act, sizeof(act), t_stdout);
	CU_ASSERT_NSTRING_EQUAL( act, "00000000: 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46", 57 );
	fgets(act, sizeof(act), t_stdout);
	CU_ASSERT_NSTRING_EQUAL( act, "00000010: 47 48 49 4a 4b", 24 );
}

/*!
 * @brief test "bldump --version"
 */
static void tc_main_ver(void)
{
	/* bldump --version */
	int ret;
	char* argv[] = { "bldump", "--version" };
	char  act[80];

	fseek( t_stdout, 0, SEEK_SET );
	ret = main( sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT_EQUAL( ret, 0 );

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	fgets(act, sizeof(act), t_stdout);
	CU_ASSERT_NSTRING_EQUAL( act, "bldump version ", 15 );
}

CU_ErrorCode ts_main_regist(void)
{
	CU_TestInfo ts_main_cases[] = {
		{ "bldump", tc_main_help },
		{ "bldump -a", tc_main_hex },
		{ "bldump --version", tc_main_ver },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "main", ts_main_init, ts_main_cleanup, ts_main_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

