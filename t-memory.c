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

/*!
 * @brief test of memory_init.
 */
static void t_memory_init(void)
{
	memory_t memory;
	memset( &memory, 0xcc, sizeof(memory_t) );
	memory_init( &memory );
	mu_assert_equal(    memory.address,  0L );
	mu_assert_ptr_null( memory.data );
	mu_assert_equal(    memory.length,   0L );
	mu_assert_equal(    memory.size,     0L );
}

/*!
 * @brief test of memory_allocate.
 */
static void t_memory_allocate(void)
{
	bool ret;
	memory_t memory;
	memory_init( &memory );

	/* memory_allocate() - failure (couldn't allocate) */
	{
		ret = memory_allocate( &memory, -1 );
		mu_assert_equal( ret, false );
	}
	/* memory_allocate() - suucess */
	{
		ret = memory_allocate( &memory, 10 );
		mu_assert_equal( ret,            true );
		mu_assert_equal( memory.address, 0L );
		mu_assert_ptr_not_null( memory.data );
		mu_assert_equal( memory.length,  10 );
		mu_assert_equal( memory.size,    0 );
	}
	/* memory_allocate() - failure (re-allocated) */
	{
		ret = memory_allocate( &memory, 10 );
		mu_assert_equal( ret, false );
	}

	free( memory.data );
}
	
/*!
 * @brief test of memory_clear.
 */
static void t_memory_clear(void)
{
	memory_t memory;
	memory_init( &memory );
	memory_allocate( &memory, 10 );

	memory.address = 2;
	memory.size = 3;
	memory_clear( &memory );
	mu_assert_ptr_not_null(  memory.data );
	mu_assert_not_equal(     memory.length,  0 );
	mu_assert_equal(         memory.address, 0L );
	mu_assert_equal(         memory.size,    0L );

	free( memory.data );
}

/*!
 * @brief test of memory_free.
 */
static void t_memory_free(void)
{
	bool ret;
	memory_t memory;
	memory_init( &memory );
	memory_allocate( &memory, 10 );

	/* memory_free() - success */
	{
		ret = memory_free( &memory );
		mu_assert_equal( ret,           true );
		mu_assert_ptr_null( memory.data );
		mu_assert_equal( memory.length, 0L );
	}
	/* memory_free() - failure */
	{
		ret = memory_free( &memory );
		mu_assert_equal( ret, false );
	}
}

void ts_memory(void)
{
	/* init */
	assert(verbose_out==NULL);//Error: verbose_out is not NULL
	t_stdin  = tmpfile();
	t_stdout = tmpfile();
	t_stderr = tmpfile();
	verbose_out = tmpfile();
	assert(t_stdin!=NULL||t_stdout!=NULL||t_stderr!=NULL);//Error: tmpfile() failure

	/* test */
	mu_run_test(t_memory_init);
	mu_run_test(t_memory_allocate);
	mu_run_test(t_memory_clear);
	mu_run_test(t_memory_free);

	/* cleanup */
	fclose( t_stdin  );
	fclose( t_stdout );
	fclose( t_stderr );
	t_stdin = NULL;
	t_stdout = NULL;
	t_stderr = NULL;
	verbose_out = NULL;
}

