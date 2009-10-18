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

static int ts_memory_init(void)
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

static int ts_memory_cleanup(void)
{
	fclose( t_stdin  );
	fclose( t_stdout );
	fclose( t_stderr );
	t_stdin = NULL;
	t_stdout = NULL;
	t_stderr = NULL;
	verbose_out = NULL;

	return 0;
}

/*!
 * @brief test of memory_init.
 */
static void tc_memory_init(void)
{
	memory_t memory;
	memset( &memory, 0xcc, sizeof(memory_t) );
	memory_init( &memory );
	CU_ASSERT_EQUAL(    memory.address,  0L );
	CU_ASSERT_PTR_NULL( memory.data );
	CU_ASSERT_EQUAL(    memory.length,   0L );
	CU_ASSERT_EQUAL(    memory.size,     0L );
}

/*!
 * @brief test of memory_allocate.
 */
static void tc_memory_allocate(void)
{
	bool ret;
	memory_t memory;
	memory_init( &memory );

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

	free( memory.data );
}
	
/*!
 * @brief test of memory_clear.
 */
static void tc_memory_clear(void)
{
	memory_t memory;
	memory_init( &memory );
	memory_allocate( &memory, 10 );

	memory.address = 2;
	memory.size = 3;
	memory_clear( &memory );
	CU_ASSERT_PTR_NOT_NULL(  memory.data );
	CU_ASSERT_NOT_EQUAL(     memory.length,  0 );
	CU_ASSERT_EQUAL(         memory.address, 0L );
	CU_ASSERT_EQUAL(         memory.size,    0L );

	free( memory.data );
}

/*!
 * @brief test of memory_free.
 */
static void tc_memory_free(void)
{
	bool ret;
	memory_t memory;
	memory_init( &memory );
	memory_allocate( &memory, 10 );

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

CU_ErrorCode ts_memory_regist(void)
{
	CU_TestInfo ts_memory_cases[] = {
		{ "memory_init()"     , tc_memory_init }     , 
		{ "memory_allocate()" , tc_memory_allocate } , 
		{ "memory_clear()"    , tc_memory_clear }    , 
		{ "memory_free()"     , tc_memory_free }     , 
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "memory", ts_memory_init, ts_memory_cleanup, ts_memory_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

