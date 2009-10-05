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
/*** enum ***/
typedef enum {
	HEXADECIMAL = 0, DECIMAL, UDECIMAL, BINARY
} OUTPUT_TYPE;

typedef struct {
	char*        infile_name;  /*!< <infile> */
	char*        outfile_name; /*!< <outfile> */

	/* input */
	size_t		start_address; /*!< -s : start reading address(skip bytes). */
	size_t		end_address;   /*!< -l : end reading address */

	/* container */
	int			data_fields;   /*!< -f : input data fields. */
	int			data_length;   /*!< -l : input data length. */
	int			data_order[8]; /*!< -r : byte order of input data */

	/* output */
	bool        show_address;   /*!< -a : data address. */
	char*       col_delimitter; /*!< -d : delimitter of outputting column. */
	char*       row_delimitter; /*!< delimitter of outputting row. */
	OUTPUT_TYPE output_type;    /*!< argument -d, -u, -b */
	char*		output_format;  /*!< output format. */

} options_t;

/*** file_t ***/
typedef struct {
	FILE* ptr;       /*!< input file pointer */
	char* name;      /*!< input file name */
	size_t position; /*!< start address to input */
	size_t length;   /*!< input file length */
} file_t;

/*** memory_t ***/
typedef unsigned char data_t;

typedef struct {
	size_t address; /*!< start address. */
	data_t* data;   /*!< data buffer pointer. */
	size_t length;  /*!< data buffer length. */
	size_t size;    /*!< valid size. */
} memory_t;


/***********************
 * Function assignment *
 ***********************/
int main( int argc, char* argv[] );
int help(void);

bool bldump_setup( memory_t* memory, file_t* infile, file_t* outfile, options_t* opt );
bool bldump_read( memory_t* memory, file_t* infile, options_t* opt );
bool bldump_write( memory_t* memory, file_t* outfile, options_t* opt );
void write_hex( memory_t* memory, file_t* file, options_t* opt );
void write_dec( memory_t* memory, file_t* outfile, options_t* opt );

/*** options ***/
void options_reset( /*@out@*/ options_t* opt );
bool options_load( options_t* opt, int argc, char* argv[] );
bool options_clear( options_t* opt );

/*** memory ***/
void memory_init( /*@out@*/ memory_t* memory );
bool memory_allocate( memory_t* memory, size_t length );
void memory_clear( memory_t* memory );
bool memory_free( memory_t* memory );

/*** file ***/
void file_reset( /*@out@*/ file_t* file );
bool file_open( file_t* file, const char* name, const char* mode );
bool file_close( file_t* file );
bool file_read( file_t* file, memory_t* memory, size_t nmemb );
void file_write( file_t* file, memory_t* memory );

/*** utility ***/
char* strclone( const char* str );
bool strfree( char* str );

#ifdef CUNIT
extern FILE *t_stdin, *t_stdout, *t_stderr;
extern char* t_tmpname;
#endif

#endif

