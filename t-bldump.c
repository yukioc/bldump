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
char* t_tmpname = "t-bldump.tmp" ;

int ts_bldump_init(void)
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

int ts_bldump_cleanup(void)
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
 * @brief test bldump::help().
 */
void tc_help(void)
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
 * @brief test of memory_*.
 */
void tc_memory_x(void)
{
	bool ret;
	memory_t memory;
	
	/* memory_init() */
	{
		memset( &memory, 0xcc, sizeof(memory_t) );
		memory_init( &memory );
		CU_ASSERT_EQUAL(    memory.address,  0L );
		CU_ASSERT_PTR_NULL( memory.data );
		CU_ASSERT_EQUAL(    memory.length,   0L );
		CU_ASSERT_EQUAL(    memory.size,     0L );
	}

	/* memory_allocate() - failure (couldn't allocate) */
	{
		ret = memory_allocate( &memory, -1 );
		CU_ASSERT_EQUAL( ret, false );
	}
	/* memory_allocate() - suucess */
	{
		ret = memory_allocate( &memory, 10 );
		CU_ASSERT_EQUAL( ret,            true );
		CU_ASSERT_EQUAL( memory.address, 0L );
		CU_ASSERT_PTR_NOT_NULL( memory.data );
		CU_ASSERT_EQUAL( memory.length,  10 );
		CU_ASSERT_EQUAL( memory.size,    0 );
	}
	/* memory_allocate() - failure (re-allocated) */
	{
		ret = memory_allocate( &memory, 10 );
		CU_ASSERT_EQUAL( ret, false );
	}
	
	/* memory_clear() */
	{
		memory.address = 2;
		memory.size = 3;
		memory_clear( &memory );
		CU_ASSERT_PTR_NOT_NULL(  memory.data );
		CU_ASSERT_NOT_EQUAL(     memory.length,  0 );
		CU_ASSERT_EQUAL(         memory.address, 0L );
		CU_ASSERT_EQUAL(         memory.size,    0L );
	}

	/* memory_free() - success */
	{
		ret = memory_free( &memory );
		CU_ASSERT_EQUAL( ret,           true );
		CU_ASSERT_PTR_NULL( memory.data );
		CU_ASSERT_EQUAL( memory.length, 0L );
	}
	/* memory_free() - failure */
	{
		ret = memory_free( &memory );
		CU_ASSERT_EQUAL( ret, false );
	}
}

/*!
 * @brief test of file_*.
 */
void tc_file_x(void)
{
	bool is;
	int val;
	file_t file;

	/* file_reset() */
	{
		memset( &file, 0xcc, sizeof(file_t) );
		file_reset( &file );
		CU_ASSERT_PTR_NULL( file.ptr  );
		CU_ASSERT_PTR_NULL( file.name );
		CU_ASSERT_EQUAL( file.position, 0L );
		CU_ASSERT_EQUAL( file.length,   0L );
	}

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

	/* make test file */
	{
		FILE* in;
	  	in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		fputs( "hello", in );
		fclose( in );
	}

	/* file_open() - normal file */
	{
		is = file_open( &file, t_tmpname, "rt" );
		CU_ASSERT_EQUAL( is, true );
		CU_ASSERT_STRING_EQUAL( file.name, t_tmpname );
		CU_ASSERT_PTR_NOT_NULL( file.ptr );
		CU_ASSERT_EQUAL( file.position, 0L );
		CU_ASSERT_EQUAL( file.length,   5 );
	}

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

	/* file_read */
	{
		memory_t memory;

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

	/* file_write */
	{
		memory_t memory;
		char* s = "foo";
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
			fread( t, 10, 1, in );
			CU_ASSERT_EQUAL( strlen( t ), 3 );
			CU_ASSERT_STRING_EQUAL( t, "foo" );
			fclose(in);
		}
	}

	val = remove( t_tmpname );
	assert( val == 0 );
}

/*!
 * @brief test of bldump_read().
 */
void tc_bldump_read(void)
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

		file_open( &infile, t_tmpname, "rt" );
		ret = bldump_read( &memory, &infile, &opt );
		CU_ASSERT_EQUAL( ret, 1 );

		file_close( &infile );
	}

	/* normal text */
	{
		FILE* in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		fputs( "The quick brown fox jumps over the lazy dog", in );
		fclose( in );

		file_open( &infile, t_tmpname, "rt" );

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

		file_close( &infile );
	}

	memory_free( &memory );
}

/*!
 * @brief test of bldump_write().
 */
void tc_bldump_write(void)
{
}

CU_ErrorCode ts_bldump_regist(void)
{
	CU_TestInfo ts_bldump_cases[] = {
		{ "help()",					tc_help },
		{ "memory_*()",				tc_memory_x },
		{ "file_*()",				tc_file_x },
		{ "bldump_read()",			tc_bldump_read },
		{ "bldump_write()",			tc_bldump_write },
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "bldump", ts_bldump_init, ts_bldump_cleanup, ts_bldump_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

