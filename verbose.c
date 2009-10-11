/*!
 * @file
 * @brief output verbose message.
 * @author yukio
 *
 * $Id$
 */

#include <verbose.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef CUNIT
void t_exit(int);
#define EXIT t_exit
#else
#define EXIT exit
#endif

/*!
 * @brief priority level to output.
 */
unsigned int verbose_level = VERB_DEFAULT;

/*!
 * @brief output stream.
 * default stream is NULL that's mean no-displaying.
 */
FILE* verbose_out = NULL;

/*!
 * @brief verbose vprintf.
 * @param[in] level effective level.
 * @param[in] fmt output text format.
 * @param[in] ap arguments.
 */
static int verbose_vprintf( unsigned int level, const char *fmt, va_list ap ) 
{
	int ret=0;
	if ( verbose_level >= level && verbose_out != NULL ) {
		ret=vfprintf( verbose_out, fmt, ap );
	}
	return ret;
}

/*!
 * @brief verbose printf.
 * @param[in] level output level.
 * @param[in] fmt   output format.
 */
int verbose_printf( unsigned int level, const char *fmt, ... ) 
{
	int ret;
	va_list ap;
	va_start( ap, fmt );
	ret=verbose_vprintf( level, fmt, ap );
	va_end( ap );
	return ret;
}

/*!
 * @brief output message and exit application.
 */
int verbose_die( const char* fmt, ... )
{
	int ret=0;
	va_list ap;
	va_start( ap, fmt );
	ret=verbose_vprintf( VERB_ERR, fmt, ap );
	va_end( ap );
	EXIT( EXIT_FAILURE );
	return ret;
}

