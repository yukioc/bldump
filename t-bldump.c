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

static int ts_bldump_cleanup(void)
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

	CU_ASSERT_EQUAL( ret, EXIT_FAILURE );
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

	opt.data_length = 1;
	opt.data_fields = 16;

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
		CU_ASSERT_EQUAL( outfile.ptr, t_stdout  );
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

	/* start_address=1 */
	{
		opt.infile_name  = t_tmpname;
		opt.outfile_name = NULL;
		opt.start_address = 1;
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_PTR_NOT_NULL( infile.ptr );
		CU_ASSERT_EQUAL( infile.position, 1 );
		CU_ASSERT_EQUAL( outfile.ptr, t_stdout  );
		CU_ASSERT_PTR_NOT_NULL( memory.data );
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

	/* data_fields=0(error) */
	{
		opt.data_fields = 0;
		is = bldump_setup( &memory, &infile, &outfile, &opt );
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_PTR_NOT_NULL( infile.ptr );
		CU_ASSERT_PTR_NOT_NULL( outfile.ptr );
		CU_ASSERT_PTR_NULL( memory.data );
		(void) file_close( &infile );
		(void) file_close( &outfile );
		opt.data_fields = 16;
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
 * @brief test of bldump_read().
 */
static void tc_bldump_read(void)
{
	bool is;
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
		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is, true );

		(void)file_close( &infile );
	}

	/* make test file */
	{
		FILE* in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		fputs( "The quick brown fox jumps over the lazy dog", in );
		fclose( in );
	}

	/* normal text */
	{
		(void)file_open( &infile, t_tmpname, "rt" );

		/* 1 */
		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is,             true );
		CU_ASSERT_EQUAL( memory.size,    30 );
		CU_ASSERT_EQUAL( memory.address, 0 );
		CU_ASSERT_NSTRING_EQUAL( memory.data, "The quick brown fox jumps over", 30 );

		/* 2 */
		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is,             true );
		CU_ASSERT_EQUAL( memory.size,    13 ); /* size is limited by file size */
		CU_ASSERT_EQUAL( memory.address, 30 );
		CU_ASSERT_NSTRING_EQUAL( memory.data, " the lazy dog", 13 ); /* notice: */

		/* 3 */
		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is,             true );
		CU_ASSERT_EQUAL( memory.size,    0 );
		CU_ASSERT_EQUAL( memory.address, 0 );

		(void)file_close( &infile );
	}

	/* end_address */
	{
		(void)file_open( &infile, t_tmpname, "rt" );

		opt.end_address = 34;

		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( memory.size, 30 );

		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( memory.size, 4 );
		CU_ASSERT_NSTRING_EQUAL( memory.data, " the", 4 );

		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_EQUAL( memory.size, 0 );

		(void)file_close( &infile );
	}


	remove( t_tmpname );
	memory_free( &memory );
}

/*!
 * @brief test of HEXADECIMAL format of bldump_write().
 */
static void tc_bldump_hexadecimal(void)
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
		opt.output_type    = HEXADECIMAL;
		opt.output_format  = "%02x";
		opt.show_address   = false;
		opt.data_length    = 1;
		opt.col_delimitter = ",";
		opt.row_delimitter = "\n";

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
		opt.output_type    = HEXADECIMAL;
		opt.output_format  = "%02x";
		opt.show_address   = true;
		opt.data_length    = 5;
		opt.col_delimitter = "-";
		opt.row_delimitter = "\n";

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
 * @brief test of DECIMAL format of bldump_write().
 */
static void tc_bldump_decimal(void)
{
	int i;
	file_t outfile;
	memory_t memory;
	options_t opt;
	char data[16] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* 72623859790382856 */
		0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88  /* -9114578090645354616 */
	} ;
	file_reset( &outfile );
	memory_init( &memory );
	options_reset( &opt );

	(void) memory_allocate( &memory, 16 );
	for ( i=0; i<16; i++ ) memory.data[i] = (data_t) data[i];
	memory.size    = 16;
	memory.address = 0xAAAA5555;

	/* 1,2,3, .. -121,-120 */
	{
		FILE* in;
		char buf[100];
		size_t reads;
		opt.output_type    = DECIMAL;
		opt.output_format  = "%lld";
		opt.show_address   = false;
		opt.data_length    = 1;
		opt.col_delimitter = ",";
		opt.row_delimitter = "\n";

		(void) file_open( &outfile, t_tmpname, "wb" );
		(void) bldump_write( &memory, &outfile, &opt );
		(void) file_close( &outfile );

		in = fopen( t_tmpname, "rb" );
		reads = fread( buf, 1, 100, in );
		CU_ASSERT_EQUAL( reads, 56 );
		CU_ASSERT_NSTRING_EQUAL( buf, "1,2,3,4,5,6,7,8,-127,-126,-125,-124,-123,-122,-121,-120\n", 56 );
		//printf( "reads=%d buf=%s\n", reads, buf );
	}

	/* aaaa5555: 72623859790382856
	 * 00000008: -9114578090645354616
	 */
	{
		FILE* in;
		char buf[100];
		size_t reads;
		opt.output_type    = DECIMAL;
		opt.output_format  = "%lld";
		opt.show_address   = true;
		opt.data_length    = 8;
		opt.col_delimitter = " ";
		opt.row_delimitter = "\n";

		(void) file_open( &outfile, t_tmpname, "wb" );
		(void) bldump_write( &memory, &outfile, &opt );
		(void) file_close( &outfile );

		in = fopen( t_tmpname, "rb" );
		reads = fread( buf, 1, 100, in );
		CU_ASSERT_EQUAL( reads, 49 );
		CU_ASSERT_NSTRING_EQUAL( buf, "aaaa5555: 72623859790382856 -9114578090645354616\n", 49 );
		//printf( "reads=%d buf=%s\n", reads, buf );
	}
}

/*!
 * @brief test of unsigned decimal format of bldump_write().
 */
static void tc_bldump_udecimal(void)
{
	int i;
	file_t outfile;
	memory_t memory;
	options_t opt;
	char data[16] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88
	} ;
	file_reset( &outfile );
	memory_init( &memory );
	options_reset( &opt );

	(void) memory_allocate( &memory, 16 );
	for ( i=0; i<16; i++ ) memory.data[i] = (data_t) data[i];
	memory.size    = 16;
	memory.address = 0xAAAA5555;

	/* 1,2,3, ... 135,136 */
	{
		FILE* in;
		char buf[100];
		size_t reads;
		opt.output_type    = UDECIMAL;
		opt.output_format  = "%llu";
		opt.show_address   = false;
		opt.data_length    = 1;
		opt.col_delimitter = ",";
		opt.row_delimitter = "\n";

		(void) file_open( &outfile, t_tmpname, "wb" );
		(void) bldump_write( &memory, &outfile, &opt );
		(void) file_close( &outfile );

		in = fopen( t_tmpname, "rb" );
		reads = fread( buf, 1, 100, in );
		CU_ASSERT_EQUAL( reads, 48 );
		CU_ASSERT_NSTRING_EQUAL( buf, "1,2,3,4,5,6,7,8,129,130,131,132,133,134,135,136\n", 48 );
		//printf( "reads=%d buf=%s\n", reads, buf );
	}

	/* 72623859790382856 -9114578090645354616 */
	{
		FILE* in;
		char buf[100];
		size_t reads;
		opt.output_type    = UDECIMAL;
		opt.output_format  = "%llu";
		opt.show_address   = false;
		opt.data_length    = 8;
		opt.col_delimitter = " ";
		opt.row_delimitter = "\n";

		(void) file_open( &outfile, t_tmpname, "wb" );
		(void) bldump_write( &memory, &outfile, &opt );
		(void) file_close( &outfile );

		in = fopen( t_tmpname, "rb" );
		reads = fread( buf, 1, 100, in );
		CU_ASSERT_EQUAL( reads, 38 );
		CU_ASSERT_NSTRING_EQUAL( buf, "72623859790382856 9332165983064197000\n", 38 );
		//printf( "reads=%d buf=%s\n", reads, buf );
	}
}

/*!
 * @brief test of BINARY format of bldump_write().
 */
static void tc_bldump_binary(void)
{
	int i;
	file_t outfile;
	memory_t memory;
	options_t opt;
	char data[3] = { 0x01, 0x02, 0x03 } ;

	file_reset( &outfile );
	memory_init( &memory );
	options_reset( &opt );

	(void) memory_allocate( &memory, 7 );
	memset( memory.data, 0xcc, memory.length );
	for ( i=0; i<3; i++ ) memory.data[i] = (data_t) data[i];
	memory.size = 3;

	/* 0x01, 0x02, 0x03 */
	{
		FILE* in;
		char buf[100];
		size_t reads;
		opt.output_type    = BINARY;

		(void) file_open( &outfile, t_tmpname, "wb" );
		(void) bldump_write( &memory, &outfile, &opt );
		(void) file_close( &outfile );

		in = fopen( t_tmpname, "rb" );
		reads = fread( buf, 1, 100, in );
		CU_ASSERT_EQUAL( reads, 3 );
		CU_ASSERT_EQUAL( buf[0], 0x01 );
		CU_ASSERT_EQUAL( buf[1], 0x02 );
		CU_ASSERT_EQUAL( buf[2], 0x03 );
		//printf( "reads=%d buf=%s", reads, buf );
	}
}

/*!
 * @brief test of reordering of bldump_read().
 */
static void tc_bldump_reorder(void)
{
	bool is;
	file_t infile;
	memory_t memory;
	options_t opt;
	char data[] = { 0x80, 0x81, 0x82, 0x83, 0x84 } ;

	file_reset( &infile );
	memory_init( &memory );
	options_reset( &opt );

	/* make test file */
	{
		FILE* in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		(void)fwrite( data, 1, (int)sizeof(data)/sizeof(char), in);
		fclose( in );
	}

	(void) memory_allocate( &memory, 16 );

	{
		(void)file_open( &infile, t_tmpname, "rt" );

		opt.data_order[0] = 2;
		opt.data_order[1] = 1;
		opt.data_order[2] = 0;
		opt.data_length   = 3;

		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( memory.size, 5 );
		/* 80,81,82 -> 82,81,80 */
		CU_ASSERT_EQUAL( memory.data[0], 0x82 );
		CU_ASSERT_EQUAL( memory.data[1], 0x81 );
		CU_ASSERT_EQUAL( memory.data[2], 0x80 );
		/* 83,84,(00) -> 00,84 */
		CU_ASSERT_EQUAL( memory.data[3], 0x00 );
		CU_ASSERT_EQUAL( memory.data[4], 0x84 );
		(void)file_close( &infile );
	}

	{
		(void)file_open( &infile, t_tmpname, "rt" );

		opt.data_order[0] = 0;
		opt.data_order[1] = 1;
		opt.data_order[2] = 2;
		opt.data_order[3] = 3;
		opt.data_order[4] = 4;
		opt.data_order[5] = 0;
		opt.data_order[6] = 1;
		opt.data_length   = 7;

		is = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_EQUAL( memory.size, 6 );
		CU_ASSERT_EQUAL( memory.data[0], 0x80 );
		CU_ASSERT_EQUAL( memory.data[1], 0x81 );
		CU_ASSERT_EQUAL( memory.data[2], 0x82 );
		CU_ASSERT_EQUAL( memory.data[3], 0x83 );
		CU_ASSERT_EQUAL( memory.data[4], 0x84 );
		CU_ASSERT_EQUAL( memory.data[5], 0x80 );
		CU_ASSERT_EQUAL( memory.data[6], 0x81 );

		(void)file_close( &infile );
	}


}

CU_ErrorCode ts_bldump_regist(void)
{
	CU_TestInfo ts_bldump_cases[] = {
		{ "help()"                    , tc_help }         , 
		{ "strfree()"                 , tc_strfree }      , 
		{ "bldump_setup()"            , tc_bldump_setup } , 
		{ "bldump_read()"             , tc_bldump_read }  , 
		{ "bldump_read(reordering)"   , tc_bldump_reorder } ,
		{ "bldump_write(HEXADECIMAL)" , tc_bldump_hexadecimal }   , 
		{ "bldump_write(DECIMAL)"     , tc_bldump_decimal }   , 
		{ "bldump_write(UDECIMAL)"    , tc_bldump_udecimal }   , 
		{ "bldump_write(BINARY)"      , tc_bldump_binary }   , 
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "bldump", ts_bldump_init, ts_bldump_cleanup, ts_bldump_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

