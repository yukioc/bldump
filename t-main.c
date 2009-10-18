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
	if ( verbose_out != NULL ) {
		fprintf( stderr, "Error: verbose_out is not NULL\n" );
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
	verbose_out = NULL;

	return 0;
}

/*!
 * @brief test "bldump"
 */
static void tc_main_help(void)
{
	int ret;
	char* argv[] = { "bldump" };

	ret = main( (int)(sizeof(argv)/sizeof(char*)), argv ); 
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
	char* s;

	fseek( t_stdout, 0, SEEK_SET );

	/* make input data */
	{
		FILE* fp = fopen( t_tmpname, "wb" );
		assert( fp != NULL );
		fputs( exp, fp );
		fclose( fp );
	}

	ret = main( (int)(sizeof(argv)/sizeof(char*)), argv ); 
	CU_ASSERT_EQUAL( ret, 0 );

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	s = fgets(act, (int)(sizeof(act)), t_stdout);
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "00000000: 30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46", 57 );
	s = fgets(act, (int)(sizeof(act)), t_stdout);
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "00000010: 47 48 49 4a 4b", 24 );
}

/*!
 * @brief test "bldump -r 3210"
 */
static void tc_main_reorder(void)
{
	int ret;
	char* argv[] = { "bldump", "-f", "1", "-r" , "3210", t_tmpname };
	char* exp = "01234567";
	char act[80];
	char* s;

	fseek( t_stdout, 0, SEEK_SET );

	/* make input data */
	{
		FILE* fp = fopen( t_tmpname, "wb" );
		assert( fp != NULL );
		fputs( exp, fp );
		fclose( fp );
	}

	ret = main( (int)(sizeof(argv)/sizeof(char*)), argv ); 
	CU_ASSERT_EQUAL( ret, 0 );

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	s = fgets(act, (int)(sizeof(act)), t_stdout);
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "33323130", 8 );
	s = fgets(act, (int)(sizeof(act)), t_stdout);
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "37363534", 8 );
}

/*!
 * @brief test "bldump -d , "
 */
static void tc_main_csv(void)
{
	/* bldump -d , */
	int ret;
	char* argv[] = { "bldump", "-i", "-d", ",", t_tmpname };
	char* exp = "0123456789ABCDEFGHIJK";
	char act[80];
	char* s;

	fseek( t_stdout, 0, SEEK_SET );

	/* make input data */
	{
		FILE* fp = fopen( t_tmpname, "wb" );
		assert( fp != NULL );
		fputs( exp, fp );
		fclose( fp );
	}

	ret = main( (int)(sizeof(argv)/sizeof(char*)), argv ); 
	CU_ASSERT_EQUAL( ret, 0 );

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	s = fgets(act, (int)(sizeof(act)), t_stdout);
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70", 47 );
	s = fgets(act, (int)(sizeof(act)), t_stdout);
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "71,72,73,74,75", 14 );
}

/*!
 * @brief test "bldump -l 2 -f 1 -a -S FF"
 */
static void tc_main_search(void)
{
	int ret;
	char act[80];
	char* argv[] = { "bldump", "-l", "2", "-f", "1", "-a", "-S", "FF", t_tmpname };
	char exp[] = {
		0x01, 0x02, 0xFF, 0x04, 0xBB, 0xFF, 0x07, 0x08,
		0xFF, 0xBB, 0x0B, 0xFF, 0x0D, 0x0E, 0xFB
	};
	char* s;

	fseek( t_stdout, 0, SEEK_SET );

	/* make input data */
	{
		FILE* fp = fopen( t_tmpname, "wb" );
		assert( fp != NULL );
		(void)fwrite( exp, 1, sizeof(exp), fp );
		fclose( fp );
	}

	ret = main( (int)(sizeof(argv)/sizeof(char*)), argv ); 
	CU_ASSERT_EQUAL( ret, 0 );

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	s = fgets( act, (int)(sizeof(act)), t_stdout );
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "00000002: ff04", 14 );
	s = fgets( act, (int)(sizeof(act)), t_stdout );
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "00000005: ff07", 14 );
	s = fgets( act, (int)(sizeof(act)), t_stdout );
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "00000008: ffbb", 14 );

	remove( t_tmpname );
}

/*!
 * @brief test "bldump -A -d ''"
 */
static void tc_main_ascii(void)
{
	/* bldump -A -d '' */
	int ret;
	char* argv[] = { "bldump", "-A", "-d", "", "-l", "4", "-f", "1", t_tmpname };
	char* exp = "12\r4\n6";
	char act[80];
	char* s;

	/* make input data */
	FILE* fp = fopen( t_tmpname, "wb" );
	assert( fp != NULL );
	fputs( exp, fp );
	fclose( fp );

	fseek( t_stdout, 0, SEEK_SET );

	ret = main( (int)(sizeof(argv)/sizeof(char*)), argv ); 

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	s = fgets( act, (int)(sizeof(act)), t_stdout );
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "12.4", 4 );
	s = fgets( act, (int)(sizeof(act)), t_stdout );
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, ".6", 2 );

	remove( t_tmpname );
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
	char* s;

	fseek( t_stdout, 0, SEEK_SET );
	ret = main( (int)(sizeof(argv)/sizeof(char*)), argv ); 
	CU_ASSERT_EQUAL( ret, 0 );

	fflush( t_stdout );
	fseek( t_stdout, 0, SEEK_SET );
	s = fgets(act, (int)(sizeof(act)), t_stdout);
	assert( s == act );
	CU_ASSERT_NSTRING_EQUAL( act, "bldump version ", 15 );
}

CU_ErrorCode ts_main_regist(void)
{
	CU_TestInfo ts_main_cases[] = {
		{ "bldump", tc_main_help },
		{ "bldump -a", tc_main_hex },
		{ "bldump -i -d ,", tc_main_csv },
		{ "bldump -r 3210", tc_main_reorder },
		{ "bldump -l 2 -f 1 -a -S FF", tc_main_search },
		{ "bldump -A -d '' -l 4 -f 1", tc_main_ascii },
		{ "bldump --version", tc_main_ver },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "main", ts_main_init, ts_main_cleanup, ts_main_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

