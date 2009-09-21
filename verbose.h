/*!
 * @file
 * @brief verbosely printf 
 * @author yukio
 */
/* vim: ts=4 sw=4 sts=0 fenc=utf-8 ff=unix:
 * $Id$
 */

#ifndef __verbose_h__
#define __verbose_h__

#include <stdio.h>

#if 0
#define VERBOSE_TEST		/*!< verbose::test() を有効にする */
#endif

enum verbose_level_t {
	VERB_DEFAULT = 6,	//!< verbose_levelデフォルト値
	VERB_UNUSE   = 0,	//!< 使用しない
	VERB_EMERG   = 0,	//!< system is unusable
	VERB_ALERT   = 1,	//!< action must be taken immediately
	VERB_CRIT    = 2,	//!< critical conditions
	VERB_ERR     = 3,	//!< error conditions
	VERB_WARNING = 4,	//!< warning conditions
	VERB_NOTICE  = 5,	//!< normal but significant conditions
	VERB_INFO    = 6,	//!< informational
	VERB_DEBUG   = 7,	//!< debug level messages
	VERB_LOG     = 7,	//!< ログ出力
	VERB_TRACE   = 9	//!< 内部トレース出力
};

extern unsigned int   verbose_level;
extern FILE* verbose_out;

extern int   verbose_printf( unsigned int level, const char *fmt, ... );
extern int   verbose_die( const char* fmt, ... );

#ifdef CUNIT
extern int t_exit_count;
extern int t_exit_code;
extern void t_exit(int);
#endif

#endif /* __verbose_h__ */

