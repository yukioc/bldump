/*!
 * @file
 * @brief unit test of 'bldump.c'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "munit.h"
#include "verbose.h"
#include "bldump.h"

extern FILE *t_stdin, *t_stdout, *t_stderr;
extern char* t_tmpname;

/*!
 * @brief test of file_reset.
 */
static void t_file_reset(void)
{
	file_t file;

	memset( &file, 0xcc, sizeof(file_t) );
	file_reset( &file );
	mu_assert_ptr_null( file.ptr  );
	mu_assert_ptr_null( file.name );
	mu_assert_equal( file.position, 0L );
	mu_assert_equal( file.length,   0L );
}

/*!
 * @brief test of file_open.
 */
static void t_file_open(void)
{
	bool is;
	file_t file;

	file_reset( &file );

	/* file_open() - check NULL */
	{
		is = file_open( &file, NULL, "rt" );
		mu_assert_equal( is, false );
		mu_assert_ptr_null( file.name );
	}
	/* file_open() - non existing file */
	{
		char *non_existing = tmpnam(NULL);
		is = file_open( &file, non_existing, "rt" ); /* non-existing file */
		mu_assert_equal( is, false );
		mu_assert_string_equal( file.name, non_existing );
	}
	/* file_open() - normal file */
	{
		is = file_open( &file, t_tmpname, "rt" );
		mu_assert_equal( is, true );
		mu_assert_string_equal( file.name, t_tmpname );
		mu_assert_ptr_not_null( file.ptr );
		mu_assert_equal( file.position, 0L );
		mu_assert_equal( file.length,   5 );

		fclose( file.ptr );
		file.ptr = NULL;
	}

	assert( file.ptr == NULL );
}

/*!
 * @brief test of file_close.
 */
static void t_file_close(void)
{
	bool is;
	file_t file;

	file_reset( &file );
	file_open( &file, t_tmpname, "rt" );

	/* file_close() - success */
	{
		is = file_close( &file );
		mu_assert_equal( is, true );
		mu_assert_ptr_null( file.ptr );
	}
	/* file_close() - failure */
	{
		is = file_close( &file );
		mu_assert_equal( is, false );
	}
}

/*!
 * @brief test of file_seek.
 */
static void t_file_seek(void)
{
	int val;
	file_t file;

	file_reset( &file );
	file_open( &file, t_tmpname, "rt" );

	/* file_seek() - failure */
	{
		val = file_seek( &file, -1 );
		mu_assert_not_equal( val, 0 );
	}
	/* file_seek() - success */
	{
		val = file_seek( &file, 3 );
		mu_assert_equal( val, 0 );
		mu_assert_equal( file.position, 3 );
	}

	file_close( &file );
}

/*!
 * @brief test of file_read.
 */
static void t_file_read(void)
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
	mu_assert_equal( is,             true );
	mu_assert_equal( file.position,  3 );
	mu_assert_equal( memory.address, 1 );
	mu_assert_equal( memory.size,    2 );

	is = file_read( &file, &memory, 2 );
	mu_assert_equal( is,             true );
	mu_assert_equal( file.position,  5 );
	mu_assert_equal( memory.address, 1 ); /* not changed */

	is = file_read( &file, &memory, 0 );
	mu_assert_equal( is, false );
	
	file_close( &file );
	memory_free( &memory );
}

/*!
 * @brief test of file_write.
 */
static void t_file_write(void)
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
	mu_assert( file.length == 3 );
	file_close( &file );

	memory_free( &memory );

	{
		FILE* in=fopen(t_tmpname,"rt");
		char t[10];
		val = fread( t, 1, 10, in );
		mu_assert_equal( val, 3 );
		mu_assert_nstring_equal( t, "foo", 3 );
		fclose(in);
	}
}

/*!
 * @brief test of file_search.
 */
static void t_file_search(void)
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
		mu_assert_equal( is,             true );
		mu_assert_equal( file.position,  4 );
		mu_assert_equal( memory.address, 2 );
		mu_assert_equal( memory.size,    2 );
		mu_assert_equal( memory.data[0], 0x6c );
		mu_assert_equal( memory.data[1], 0x6c );
	}

	/* detect EOF on charging */
	{
		memory_clear( &memory );
		opt.search_pattern = 0x6c6c;
		opt.search_length  = 16;
		is = file_search( &file, &memory, &opt );
		mu_assert_equal( is, false );
	}

	/* detect EOF before charging */
	{
		memory_clear( &memory );
		is = file_search( &file, &memory, &opt );
		mu_assert_equal( is, false );
	}

	/* detect EOF on searching */
	{
		memory_clear( &memory );
		file_close( &file );
		file_open( &file, t_tmpname, "rt" ); /* hello */

		opt.search_pattern = 0xAAAA; //not existing
		opt.search_length  = 16;
		is = file_search( &file, &memory, &opt );
		mu_assert_equal( is, false );
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
		mu_assert_equal( is, false );
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
		mu_assert_equal( is, false );
	}

	file_close( &file );
	memory_free( &memory );
}


void ts_file(void)
{
	/* init */
	assert(verbose_out == NULL ); //Error: verbose_out is not NULL
	t_stdin  = tmpfile();
	t_stdout = tmpfile();
	t_stderr = tmpfile();
	verbose_out = tmpfile();
	assert(t_stdin!=NULL||t_stdout!=NULL||t_stderr!=NULL);//Error: tmpfile() failure

	/* make test file */
	{
		FILE* in;
		in = fopen( t_tmpname, "wt" );
		assert( in != NULL );
		fputs( "hello", in );
		fclose( in );
	}

	/* test */
	mu_run_test(t_file_reset);
	mu_run_test(t_file_open);
	mu_run_test(t_file_close);
	mu_run_test(t_file_seek);
	mu_run_test(t_file_read);
	mu_run_test(t_file_search);
	mu_run_test(t_file_write); // this test overwrite t_tmpname file.

	/* cleanup */
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
	return;
}

