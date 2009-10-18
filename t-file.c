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

extern FILE *t_stdin, *t_stdout, *t_stderr;
extern char* t_tmpname;

static int ts_file_init(void)
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

	/* make test file */
	{
		FILE* in;
		in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		fputs( "hello", in );
		fclose( in );
	}

	return 0;
}

static int ts_file_cleanup(void)
{
	int val;

	fclose( t_stdin  );
	fclose( t_stdout );
	fclose( t_stderr );
	t_stdin = NULL;
	t_stdout = NULL;
	t_stderr = NULL;
	verbose_out = NULL;

	val = remove( t_tmpname );
	assert( val == 0 );

	return 0;
}

/*!
 * @brief test of file_reset.
 */
static void tc_file_reset(void)
{
	file_t file;

	memset( &file, 0xcc, sizeof(file_t) );
	file_reset( &file );
	CU_ASSERT_PTR_NULL( file.ptr  );
	CU_ASSERT_PTR_NULL( file.name );
	CU_ASSERT_EQUAL( file.position, 0L );
	CU_ASSERT_EQUAL( file.length,   0L );
}

/*!
 * @brief test of file_open.
 */
static void tc_file_open(void)
{
	bool is;
	file_t file;

	file_reset( &file );

	/* file_open() - check NULL */
	{
		is = file_open( &file, NULL, "rt" );
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_PTR_NULL( file.name );
	}
	/* file_open() - non existing file */
	{
		char *non_existing = tmpnam(NULL);
		is = file_open( &file, non_existing, "rt" ); /* non-existing file */
		CU_ASSERT_EQUAL( is, false );
		CU_ASSERT_STRING_EQUAL( file.name, non_existing );
	}
	/* file_open() - normal file */
	{
		is = file_open( &file, t_tmpname, "rt" );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_STRING_EQUAL( file.name, t_tmpname );
		CU_ASSERT_PTR_NOT_NULL( file.ptr );
		CU_ASSERT_EQUAL( file.position, 0L );
		CU_ASSERT_EQUAL( file.length,   5 );

		fclose( file.ptr );
		file.ptr = NULL;
	}

	assert( file.ptr == NULL );
}

/*!
 * @brief test of file_close.
 */
static void tc_file_close(void)
{
	bool is;
	file_t file;

	file_reset( &file );
	file_open( &file, t_tmpname, "rt" );

	/* file_close() - success */
	{
		is = file_close( &file );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_PTR_NULL( file.ptr );
	}
	/* file_close() - failure */
	{
		is = file_close( &file );
		CU_ASSERT_EQUAL( is, false );
	}
}

/*!
 * @brief test of file_seek.
 */
static void tc_file_seek(void)
{
	int val;
	file_t file;

	file_reset( &file );
	file_open( &file, t_tmpname, "rt" );

	/* file_seek() - failure */
	{
		val = file_seek( &file, -1 );
		CU_ASSERT_NOT_EQUAL( val, 0 );
	}
	/* file_seek() - success */
	{
		val = file_seek( &file, 3 );
		CU_ASSERT_EQUAL( val, 0 );
		CU_ASSERT_EQUAL( file.position, 3 );
	}

	file_close( &file );
}

/*!
 * @brief test of file_read.
 */
static void tc_file_read(void)
{
	bool is;
	file_t file;
	memory_t memory;

	file_reset( &file );
	file_open( &file, t_tmpname, "rt" );

	memory_init( &memory );
	memory_allocate( &memory, 10 );
	file_open( &file, t_tmpname, "rt" ); /* "hello" */
	file_seek( &file, 1 );

	is = file_read( &file, &memory, 2 );
	CU_ASSERT_EQUAL( is,             true );
	CU_ASSERT_EQUAL( file.position,  3 );
	CU_ASSERT_EQUAL( memory.address, 1 );
	CU_ASSERT_EQUAL( memory.size,    2 );

	is = file_read( &file, &memory, 2 );
	CU_ASSERT_EQUAL( is,             true );
	CU_ASSERT_EQUAL( file.position,  5 );
	CU_ASSERT_EQUAL( memory.address, 1 ); /* not changed */

	is = file_read( &file, &memory, 0 );
	CU_ASSERT_EQUAL( is, false );
	
	file_close( &file );
	memory_free( &memory );
}

/*!
 * @brief test of file_write.
 */
static void tc_file_write(void)
{
	file_t file;
	memory_t memory;
	char* s = "foo";
	size_t val;

	memory_init( &memory );
	memory_allocate( &memory, 3 );
	strncpy( (char*)memory.data, s, 3 );
	memory.size = 3;

	file_reset( &file );
	file_open( &file, t_tmpname, "wt" );
	file_write( &file, &memory );
	CU_ASSERT( file.length == 3 );
	file_close( &file );

	memory_free( &memory );

	{
		FILE* in=fopen(t_tmpname,"rt");
		char t[10];
		val = fread( t, 1, 10, in );
		CU_ASSERT_EQUAL( val, 3 );
		CU_ASSERT_NSTRING_EQUAL( t, "foo", 3 );
		fclose(in);
	}
}

/*!
 * @brief test of file_search.
 */
static void tc_file_search(void)
{
	bool is;
	file_t file;
	memory_t memory;
	options_t opt;

	options_reset( &opt );

	file_reset( &file );
	file_open( &file, t_tmpname, "rt" ); /* hello */

	memory_init( &memory );
	memory_allocate( &memory, 10 );
	

	/* search 'll' */
	{
		memory_clear( &memory );
		opt.search_pattern = 0x6c6c;
		opt.search_length  = 16;

		is = file_search( &file, &memory, &opt );
		CU_ASSERT_EQUAL( is,             true );
		CU_ASSERT_EQUAL( file.position,  4 );
		CU_ASSERT_EQUAL( memory.address, 2 );
		CU_ASSERT_EQUAL( memory.size,    2 );
		CU_ASSERT_EQUAL( memory.data[0], 0x6c );
		CU_ASSERT_EQUAL( memory.data[1], 0x6c );
	}

	/* detect EOF on charging */
	{
		memory_clear( &memory );
		opt.search_pattern = 0x6c6c;
		opt.search_length  = 16;
		is = file_search( &file, &memory, &opt );
		CU_ASSERT_EQUAL( is, false );
	}

	/* detect EOF before charging */
	{
		memory_clear( &memory );
		is = file_search( &file, &memory, &opt );
		CU_ASSERT_EQUAL( is, false );
	}

	/* detect EOF on searching */
	{
		memory_clear( &memory );
		file_close( &file );
		file_open( &file, t_tmpname, "rt" ); /* hello */

		opt.search_pattern = 0xAAAA; //not existing
		opt.search_length  = 16;
		is = file_search( &file, &memory, &opt );
		CU_ASSERT_EQUAL( is, false );
	}

	/* detect end_address on charging */
	{
		memory_clear( &memory );
		file_close( &file );
		file_open( &file, t_tmpname, "rt" ); /* hello */

		opt.search_pattern = 0xAAAA; //not existing
		opt.search_length  = 16;
		opt.end_address    = 1;
		is = file_search( &file, &memory, &opt );
		CU_ASSERT_EQUAL( is, false );
	}

	/* detect end_address on charging */
	{
		memory_clear( &memory );
		file_close( &file );
		file_open( &file, t_tmpname, "rt" ); /* hello */

		opt.search_pattern = 0xAAAA; //not existing
		opt.search_length  = 16;
		opt.end_address    = 3;
		is = file_search( &file, &memory, &opt );
		CU_ASSERT_EQUAL( is, false );
	}

	file_close( &file );
	memory_free( &memory );
}


CU_ErrorCode ts_file_regist(void)
{
	CU_TestInfo ts_file_cases[] = {
		{ "file_reset()" , tc_file_reset } , 
		{ "file_open()"  , tc_file_open }  , 
		{ "file_close()" , tc_file_close } , 
		{ "file_seek()"  , tc_file_seek }  , 
		{ "file_read()"  , tc_file_read }  , 
		{ "file_search()", tc_file_search } ,
		{ "file_write()" , tc_file_write } ,  // this test overwrite t_tmpname file.
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "file", ts_file_init, ts_file_cleanup, ts_file_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

