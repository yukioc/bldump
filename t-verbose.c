/*!
 * @file
 * @brief unit test of 'verbose.c'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "munit.h"
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
static void t_verbose_level(void)
{
	mu_assert( verbose_level == VERB_DEFAULT );
}

/*!
 * @brief test of 'verbose_printf()'.
 */
static void t_verbose_printf(void)
{
	assert( verbose_level == VERB_DEFAULT );

	mu_assert( verbose_printf( VERB_EMERG, "This messege is effective\n" ) == 26 ); 
	mu_assert( verbose_printf( VERB_DEFAULT, "This messege is effective\n" ) == 26 );
	mu_assert( verbose_printf( VERB_DEBUG, "This message is not effective\n" ) == 0 );
}

/*!
 * @brief test of 'verbose_die()'.
 */
static void t_verbose_die(void)
{
	/*** pre-conditinons ***/
	assert( verbose_level == VERB_DEFAULT );
	assert( t_exit_code == -1 );
	assert( t_exit_count == 0 );

	/*** test ***/
	mu_assert( verbose_die( "This messege is effective\n" ) == 26 );
	mu_assert( t_exit_count == 1 );
	mu_assert( t_exit_code  == EXIT_FAILURE );

	verbose_level = VERB_EMERG;
	mu_assert( verbose_die( "This message is not effective\n" ) == 0 );

	/*** dispose ***/
	verbose_level = VERB_DEFAULT;

	/*** post-conditions ***/
	/* nothing */
}

/*!
 * @brief test registry of 'verbose'.
 *
 * This test registry contains test suites of
 * verbose level, printf, die.
 */
void ts_verbose(void)
{
	/* init */
	assert( verbose_out == NULL );
	verbose_out = tmpfile();
	assert(verbose_out!=NULL);//Error: tmpfile() failure

	/* test */
	mu_run_test(t_verbose_level);
	mu_run_test(t_verbose_printf);
	mu_run_test(t_verbose_die);

	/* cleanup */
	assert(verbose_out!=NULL&&verbose_out!=stdout);
	(void)fclose( verbose_out );
	verbose_out = NULL;
}

