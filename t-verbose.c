/*!
 * @file
 * @brief unit test of 'verbose.c'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "CUnit/CUnit.h"

#include "verbose.h"

/* interanl functin/variables for 'tp_verbose_die' */
int   t_exit_code  = -1;
int   t_exit_count = 0;
void  t_exit(int code)
{
	t_exit_code = code;
	t_exit_count++;
}

/*!
 * @brief test of 'verbose_level'.
 */
static void tc_verbose_level(void)
{
	CU_ASSERT( verbose_level == VERB_DEFAULT );
}

/*!
 * @brief test of 'verbose_printf()'.
 */
static void tc_verbose_printf(void)
{
	assert( verbose_level == VERB_DEFAULT );

	CU_ASSERT( verbose_printf( VERB_EMERG, "This messege is effective\n" ) == 26 ); 
	CU_ASSERT( verbose_printf( VERB_DEFAULT, "This messege is effective\n" ) == 26 );
	CU_ASSERT( verbose_printf( VERB_DEBUG, "This message is not effective\n" ) == 0 );
}

/*!
 * @brief test of 'verbose_die()'.
 */
static void tc_verbose_die(void)
{
	/*** pre-conditinons ***/
	assert( verbose_level == VERB_DEFAULT );
	assert( t_exit_code == -1 );
	assert( t_exit_count == 0 );

	/*** test ***/
	CU_ASSERT( verbose_die( "This messege is effective\n" ) == 26 );
	CU_ASSERT( t_exit_count == 1 );
	CU_ASSERT( t_exit_code  == EXIT_FAILURE );

	verbose_level = VERB_EMERG;
	CU_ASSERT( verbose_die( "This message is not effective\n" ) == 0 );

	/*** dispose ***/
	verbose_level = VERB_DEFAULT;

	/*** post-conditions ***/
	/* nothing */
}


#if 0
static FILE* tmpout = NULL;
#endif

/*!
 * @brief The suite initialization function.
 * @retval zero on success.
 * @retval non-zero otherwise.
 */
static int ts_verbose_init(void)
{
	FILE* tmp;

	assert( verbose_out = stdout );

	tmp = tmpfile();

	if ( tmp == NULL ) {
		fprintf( stderr, "Error: tmpfile() failure - errorno = %d\n.", errno );
		return 1;
	} else {
		verbose_out = tmp;
	}
	return 0;
}

/*!
 * @brief The suite cleanup function.
 * @retval zero on success.
 * @retval non-zero otherwise.
 */
static int ts_verbose_cleanup(void)
{
	if ( verbose_out != stdout ) (void)fclose( verbose_out );
	verbose_out = stdout;
	return 0;
}

/*!
 * @brief test registry of 'verbose'.
 * @return registration result of suites.
 *
 * This test registry contains test suites of
 * verbose level, printf, die.
 */
CU_ErrorCode ts_verbose_regist(void)
{
	CU_TestInfo ts_verbose_cases[] = {
		{ "verbose_level"    , tc_verbose_level }  , 
		{ "verbose_printf()" , tc_verbose_printf } , 
		{ "verbose_die()"    , tc_verbose_die }    , 
		CU_TEST_INFO_NULL
	};

	CU_SuiteInfo suites[] = {
		{ "verbose", ts_verbose_init, ts_verbose_cleanup, ts_verbose_cases },
		CU_SUITE_INFO_NULL
	};

	return CU_register_suites( suites );
}

