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
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, true );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_delimitter );
	CU_ASSERT_PTR_NULL( opt.row_delimitter );

	/* error */
	opt.infile_name   = NULL;
	opt.outfile_name  = (char*)malloc(1);
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_delimitter );
	CU_ASSERT_PTR_NULL( opt.row_delimitter );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = NULL;
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_delimitter );
	CU_ASSERT_PTR_NULL( opt.row_delimitter );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_delimitter = NULL;
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_delimitter );
	CU_ASSERT_PTR_NULL( opt.row_delimitter );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = NULL;
	is = options_clear( &opt );
	CU_ASSERT_EQUAL( is, false );
	CU_ASSERT_PTR_NULL( opt.infile_name );
	CU_ASSERT_PTR_NULL( opt.outfile_name );
	CU_ASSERT_PTR_NULL( opt.col_delimitter );
	CU_ASSERT_PTR_NULL( opt.row_delimitter );
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
	CU_ASSERT_EQUAL( is, false );
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
	CU_ASSERT_EQUAL( is, false );
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
		CU_ASSERT_EQUAL( is, false );
	}
	/* -? */
	{
		char* argv[] = { "bldump", "-?" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, false );
	}
	/* --help */
	{
		char* argv[] = { "bldump", "--help" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, false );
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
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_STRING_EQUAL( opt.infile_name, "infile" );
		CU_ASSERT_PTR_NULL( opt.outfile_name );
	}

	/* infile outfile */
	{
		options_t opt;
		bool is;
		char* argv[] = { "bldump", "infile", "outfile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_STRING_EQUAL( opt.infile_name, "infile" );
		CU_ASSERT_STRING_EQUAL( opt.outfile_name, "outfile" );
	}
}

/*!
 * @brief test -l, --length
 */
static void tc_opt_length(void)
{
	options_t opt;
	bool is;

	/* -l */
	{
		char* argv[] = { "bldump", "-l", "3", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.data_length, 3 );
		CU_ASSERT_PTR_NULL( opt.outfile_name );
	}

	/* --length */
	{
		char* argv[] = { "bldump", "--length=4", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.data_length, 4 );
	}

	/* -l --length (error for conflict of other options) */
	{
		char* argv[] = { "bldump", "-l", "3", "--length=4", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); /* duplicated option */
		CU_ASSERT_EQUAL( is, false );
	}

}

/*!
 * @brief test -f, --fields
 */
static void tc_opt_fields(void)
{
	options_t opt;
	bool is;

	/* -f */
	{
		char* argv[] = { "bldump", "-f", "5", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.data_fields, 5 );
		CU_ASSERT_PTR_NULL( opt.outfile_name );
	}

	/* --fields */
	{
		char* argv[] = { "bldump", "--fields=6", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.data_fields, 6 );
	}
}

/*!
 * @brief test -a, --show-address
 */
static void tc_opt_address(void)
{
	options_t opt;
	bool is;

	/* -a */
	{
		char* argv[] = { "bldump", "-a", "infile" };
		options_reset( &opt );
		CU_ASSERT_EQUAL( opt.show_address, false );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.show_address, true );
		CU_ASSERT_PTR_NULL( opt.outfile_name );
	}

	/* --show_address */
	{
		char* argv[] = { "bldump", "--show-address", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.show_address, true );
	}
}

/*!
 * @brief test -s, --start-address
 */
static void tc_opt_start(void)
{
	options_t opt;
	bool is;

	/* -s */
	{
		char* argv[] = { "bldump", "-s", "1", "infile" };
		options_reset( &opt );
		CU_ASSERT_EQUAL( opt.start_address, 0 );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.start_address, 1 );
	}

	/* --start_address */
	{
		char* argv[] = { "bldump", "--start-address=2", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.start_address, 2 );
	}
}

/*!
 * @brief test -v, --verbose
 */
static void tc_opt_verbose(void)
{
	options_t opt;
	bool is;

	/* -v */
	{
		char* argv[] = { "bldump", "-v", "3" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_EQUAL( verbose_level, 3 );
	}

	/* --verboes */
	{
		char* argv[] = { "bldump", "--verbose=4", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( verbose_level, 4 );
	}
}

/*!
 * @brief test -d, --delimitter
 */
static void tc_opt_delimitter(void)
{
	options_t opt;
	bool is;

	/* -d */
	{
		char* argv[] = { "bldump", "-d", ",", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_STRING_EQUAL( opt.col_delimitter, "," );
	}

	/* --delimitter */
	{
		char* argv[] = { "bldump", "--delimitter=\t", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_STRING_EQUAL( opt.col_delimitter, "\t" );
	}
}

/*!
 * @brief test -i, --decimal
 */
static void tc_opt_dec(void)
{
	options_t opt;
	bool is;

	/* -i */
	{
		char* argv[] = { "bldump", "-i", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.output_type, DECIMAL );
	}

	/* --decimal */
	{
		char* argv[] = { "bldump", "--decimal", "infile" };
		options_reset( &opt );
		is = options_load( &opt, sizeof(argv)/sizeof(char*), argv ); 
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( opt.output_type, DECIMAL );
	}
}

CU_ErrorCode ts_opt_regist(void)
{
	CU_TestInfo ts_opt_cases[] = {
		{ "options_clear()", tc_options_clear },
		{ "options_load( bldump )", tc_opt_noarg },
		{ "options_load( bldump - )", tc_opt_noopt },
		{ "options_load( bldump -h|-?|--help )", tc_opt_help },
		{ "options_load( bldump infile outfile )", tc_opt_infile_outfile },
		{ "options_load( bldump -l|--length infile )", tc_opt_length },
		{ "options_load( bldump -f|--fields infile )", tc_opt_fields },
		{ "options_load( bldump -a|--show-address )", tc_opt_address },
		{ "options_load( bldump -s|--start-address )", tc_opt_start },
		{ "options_load( bldump -v|--verbose )", tc_opt_verbose },
		{ "options_load( bldump -d|--delimitter )", tc_opt_delimitter },
		{ "options_load( bldump -i|--decimal )", tc_opt_dec },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "options", ts_opt_init, ts_opt_cleanup, ts_opt_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

