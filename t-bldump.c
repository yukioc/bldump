/*!
 * @file
 * @brief unit test of 'bldump.c'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "CUnit/CUnit.h"

#include "verbose.h"
#include "bldump.h"

FILE *t_stdin, *t_stdout, *t_stderr;
char* t_tmpname  = "t-bldump.tmp" ;
char* t_tmpname2 = "t-bldump2.tmp" ;

static int ts_bldump_init(void)
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

static int ts_bldump_cleanup(void)
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
static void tc_help(void)
{
	/* help() function */
	long pos1, pos2;
	int ret;
	pos1 = ftell(t_stderr);
	ret=help();
	pos2 = ftell(t_stderr);

	CU_ASSERT_EQUAL( ret, EXIT_FAILURE);
	CU_ASSERT(pos2 - pos1 > 0); /* count of the help message */
}

/*!
 * @brief test bldump::strfree().
 */
static void tc_strfree(void)
{
	char* s;
	bool is;
   
	/* allocated memory */
	s = (char*)malloc(1);
	is = strfree(s);
	CU_ASSERT_EQUAL( is, true );

	/* NULL */ 
	s = NULL;
	is = strfree(s);
	CU_ASSERT_EQUAL( is, false );
}

/*!
 * @brief test bldump::bldump_setup().
 */
static void tc_bldump_setup(void)
{
	int is;
	memory_t  memory;
	file_t    infile;
	file_t    outfile;
	options_t opt;

	memory_init( &memory );
	file_reset( &infile );
	file_reset( &outfile );
	options_reset( &opt );

	opt.data_length  = 1;
	opt.data_columns = 16;

	/* make test file */
	{
		FILE* in;
		in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		(void) fputs( "hello", in );
		(void) fclose( in );
	}

	/* infile=not-exist */
	{
		opt.infile_name = tmpnam(NULL); /* not exist */
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_PTR_NULL( infile.ptr );
	}

	/* infile=exist, outfile=NULL(stdout) */
	{
		opt.infile_name  = t_tmpname;
		opt.outfile_name = NULL;
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_PTR_NOT_NULL( infile.ptr );
		CU_ASSERT_EQUAL( outfile.ptr, stdout  );
		CU_ASSERT_PTR_NOT_NULL( memory.data );
		(void) file_close( &infile );
		(void) file_close( &outfile );
		(void) memory_free( &memory );
	}

	/* infile=exist, outfile=can't open */
	{
		opt.infile_name  = t_tmpname;
		opt.outfile_name = "" ;
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_PTR_NULL( outfile.ptr );
		(void) file_close( &infile );
	}

	/* infile=exist, outfile=newfile */
	{
		opt.infile_name  = t_tmpname;
		opt.outfile_name = t_tmpname2;
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_PTR_NOT_NULL( infile.ptr );
		CU_ASSERT_PTR_NOT_NULL( outfile.ptr );
		(void) file_close( &infile );
		(void) file_close( &outfile );
		(void) memory_free( &memory );
	}

	opt.infile_name  = t_tmpname;
	opt.outfile_name = NULL;

	/* data_length=0(error) */
	{
		opt.data_length = 0;
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_PTR_NOT_NULL( infile.ptr );
		CU_ASSERT_PTR_NOT_NULL( outfile.ptr );
		CU_ASSERT_PTR_NULL( memory.data );
		(void) file_close( &infile );
		(void) file_close( &outfile );
		opt.data_length = 1;
	}

	/* data_columns=0(error) */
	{
		opt.data_columns = 0;
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_PTR_NOT_NULL( infile.ptr );
		CU_ASSERT_PTR_NOT_NULL( outfile.ptr );
		CU_ASSERT_PTR_NULL( memory.data );
		(void) file_close( &infile );
		(void) file_close( &outfile );
		opt.data_columns = 16;
	}

	{
		int ret;
		(void) memory_free( &memory );
		ret = remove( t_tmpname  );
		assert( ret == 0 );
		ret = remove( t_tmpname2 );
		assert( ret == 0 );
	}
}

/*!
 * @brief test of HEX format of bldump_write().
 */
static void tc_bldump_hex(void)
{
	file_t outfile;
	memory_t memory;
	options_t opt;

	file_reset( &outfile );
	memory_init( &memory );
	options_reset( &opt );

	(void) memory_allocate( &memory, 16 );
	strcpy( (char*)memory.data, "0123456789ABCDEF" );
	memory.size    = 16;
	memory.address = 0xAAAA5555;

	/* 30,31,32, .. 56,46 */
	{
		FILE* in;
		char buf[100];
		size_t reads;
		opt.output_type   = HEX;
		opt.show_address  = false;
		opt.data_length   = 1;
		opt.col_separator = ",";
		opt.row_separator = "\n";

		(void) file_open( &outfile, t_tmpname, "wb" );
		(void) bldump_write( &memory, &outfile, &opt );
		(void) file_close( &outfile );

		in = fopen( t_tmpname, "rb" );
		reads = fread( buf, 1, 100, in );
		CU_ASSERT_EQUAL( reads, 48 );
		CU_ASSERT_NSTRING_EQUAL( buf, "30,31,32,33,34,35,36,37,38,39,41,42,43,44,45,46\n", 48 );
	}

	/* 303132-333435- .. 434445-46 */
	{
		FILE* in;
		char buf[100];
		size_t reads;
		opt.output_type   = HEX;
		opt.show_address  = true;
		opt.data_length   = 5;
		opt.col_separator = "-";
		opt.row_separator = "\n";

		(void) file_open( &outfile, t_tmpname, "wb" );
		(void) bldump_write( &memory, &outfile, &opt );
		(void) file_close( &outfile );

		in = fopen( t_tmpname, "rb" );
		reads = fread( buf, 1, 100, in );
		CU_ASSERT_EQUAL( reads, 46 );
		CU_ASSERT_NSTRING_EQUAL( buf, "aaaa5555: 3031323334-3536373839-4142434445-46\n", 46 );
	}

}

/*!
 * @brief test of bldump_read().
 */
static void tc_bldump_read(void)
{
	int ret;
	options_t opt;
	memory_t memory;
	file_t infile;

	options_reset( &opt );
	memory_init( &memory );
	file_reset( &infile );
	memory_allocate( &memory, 30 );

	/* file size = 0 */
	{
		FILE* in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		fclose( in );

		(void)file_open( &infile, t_tmpname, "rt" );
		ret = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( ret, 1 );

		(void)file_close( &infile );
	}

	/* normal text */
	{
		FILE* in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		fputs( "The quick brown fox jumps over the lazy dog", in );
		fclose( in );

		(void)file_open( &infile, t_tmpname, "rt" );

		/* 1 */
		ret = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( ret,            0 );
		CU_ASSERT_EQUAL( memory.size,    30 );
		CU_ASSERT_EQUAL( memory.address, 0 );
		CU_ASSERT_NSTRING_EQUAL( memory.data, "The quick brown fox jumps over", 30 );

		/* 2 */
		ret = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( ret,            0 );
		CU_ASSERT_EQUAL( memory.size,    13 ); /* size is limited by file size */
		CU_ASSERT_EQUAL( memory.address, 30 );
		CU_ASSERT_NSTRING_EQUAL( memory.data, " the lazy dog", 13 ); /* notice: */

		/* 3 */
		ret = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( ret,            1 );
		CU_ASSERT_EQUAL( memory.size,    0 );
		CU_ASSERT_EQUAL( memory.address, 0 );

		(void)file_close( &infile );
	}

	remove( t_tmpname );
	memory_free( &memory );
}

/*!
 * @brief test of bldump_write().
 */
static void tc_bldump_write(void)
{
}

CU_ErrorCode ts_bldump_regist(void)
{
	CU_TestInfo ts_bldump_cases[] = {
		{ "help()",					tc_help },
		{ "strfree()",				tc_strfree },
		{ "bldump_setup()",			tc_bldump_setup },
		{ "bldump_read()",			tc_bldump_read },
		{ "bldump_write(HEX)",		tc_bldump_hex },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "bldump", ts_bldump_init, ts_bldump_cleanup, ts_bldump_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

