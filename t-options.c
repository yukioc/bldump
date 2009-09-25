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

static int ts_opt_init(void)
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

static int ts_opt_cleanup(void)
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
 * @biref test bldump::options_clear().
 */
static void tc_options_clear(void)
{
	bool is;
	options_t opt;
	options_reset( &opt );

	/* normal */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_separator = (char*)malloc(1);
	opt.row_separator = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, true );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_separator );
	CU_ASSERT_PTR_NULL( opt.row_separator );

	/* error */
	opt.infile_name   = NULL;
	opt.outfile_name  = (char*)malloc(1);
	opt.col_separator = (char*)malloc(1);
	opt.row_separator = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_separator );
	CU_ASSERT_PTR_NULL( opt.row_separator );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = NULL;
	opt.col_separator = (char*)malloc(1);
	opt.row_separator = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_separator );
	CU_ASSERT_PTR_NULL( opt.row_separator );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_separator = NULL;
	opt.row_separator = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_separator );
	CU_ASSERT_PTR_NULL( opt.row_separator );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_separator = (char*)malloc(1);
	opt.row_separator = NULL;
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_separator );
	CU_ASSERT_PTR_NULL( opt.row_separator );
}

/*!
 * @biref test "bldump -".
 */
static void tc_opt_noopt(void)
{
	options_t opt;
	bool is;
	char* argv[] = { "bldump", "-" };
	options_reset( &opt );
	is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT( is == false );
}

/*!
 * @biref test "bldump".
 */
static void tc_opt_noarg(void)
{
	options_t opt;
	bool is;
	char* argv[] = { "bldump" };
	options_reset( &opt );
	is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
	CU_ASSERT( is == false );
}

/*!
 * @biref test "bldump -h".
 */
static void tc_opt_help(void)
{
	options_t opt;
	bool is;

	/* -h */
	{
		char* argv[] = { "bldump", "-h" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( is == false );
	}
	/* -? */
	{
		char* argv[] = { "bldump", "-?" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( is == false );
	}
	/* --help */
	{
		char* argv[] = { "bldump", "--help" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( is == false );
	}
}

/*!
 * @brief test infile and outfile
 */
static void tc_opt_infile_outfile(void)
{
	options_t opt;
	bool is;

	/* infile */
	{
		char* argv[] = { "bldump", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( is == true );
		CU_ASSERT( strcmp( opt.infile_name, "infile" ) == 0 );
		CU_ASSERT_PTR_NULL( opt.outfile_name );
	}

	/* infile outfile */
	{
		options_t opt;
		bool is;
		char* argv[] = { "bldump", "infile", "outfile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT( is == true );
		CU_ASSERT( strcmp( opt.infile_name, "infile" ) == 0 );
		CU_ASSERT( strcmp( opt.outfile_name, "outfile" ) == 0 );
	}
}

CU_ErrorCode ts_opt_regist(void)
{
	CU_TestInfo ts_opt_cases[] = {
		{ "options_clear()",					tc_options_clear },
		{ "options_load() : (noarg)",			tc_opt_noarg },
		{ "options_load() : -",					tc_opt_noopt },
		{ "options_load() : -h|-?|--help",		tc_opt_help },
		{ "options_load() : infile outfile",	tc_opt_infile_outfile },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "options", ts_opt_init, ts_opt_cleanup, ts_opt_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

