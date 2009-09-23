/*!
 * @file
 * @brief bldump header file.
 * @author yukio
 * @since 2009-09-20 
 */

#ifndef __bldump_h__
#define __bldump_h__

#include <stdbool.h>
#include <stdint.h>

/******************
 * Data structure *
 ******************/
typedef struct {
	char*		infile_name;	/*!< argument infile */
	char*		outfile_name;	/*!< argument outfile */

	//container
	int			data_length;
	int			data_columns;

} options_t;

/*** file_t ***/
typedef struct {
	FILE* ptr;		//!< input file pointer
	char* name;		//!< input file name
	size_t position;	//!< start address to input
	size_t length;	//!< input file length
} file_t;

/*** memory_t ***/
typedef unsigned char data_t;

typedef struct {
	size_t address;	//!< start address.
	data_t* data;	//!< data buffer pointer.
	size_t length;	//!< data buffer length.
	size_t size;	//!< valid size.
} memory_t;


/***********************
 * Function assignment *
 ***********************/
int main( int argc, char* argv[] );
int help(void);

/*** options ***/
void options_reset( options_t* opt );
int  options_load( options_t* opt, int argc, char* argv[] );

/*** memory ***/
void memory_init( memory_t* memory );
bool memory_allocate( memory_t* memory, size_t length );
void memory_clear( memory_t* memory );
bool memory_free( memory_t* memory );

/*** file ***/
void file_reset( file_t* file );
bool file_open( file_t* file, char* name, const char* mode );
bool file_close( file_t* file );
bool file_read( file_t* file, memory_t* memory, size_t nmemb );
void file_write( file_t* file, memory_t* memory );

#ifdef CUNIT
extern FILE *t_stdin, *t_stdout, *t_stderr;
#endif

#endif

