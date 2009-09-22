/*!
 * @file
 * @brief bldump - simple decimal, hex dump to text file, CSV ..
 * @author yukio
 * @since 2009-09-20 
 * @date 2009-09-20 Initial commit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#ifdef CUNIT
#include "CUnit/Basic.h"
#endif

#include "verbose.h"
#include "bldump.h"

/*** interface ***/
#define VERSION		"Version 0.0x"
#define DATE		"(" __DATE__ ")"
#define COPYRIGHT	"by yukio"
#define ABOUT		"[bldump] simple decimal, hex dump with Binary, Text, CSV.. " VERSION " " DATE " " COPYRIGHT "\n\n"

const char *usage[] = {
	"Usage: bldump [options] [infile [outfile]]",
	"",
	"  infile  dump file.",
	"  outfile output(stdout).",
	"",
	"options:",
	"  -h -? --help",
	"    this help.",
	""
};

/*** macro ***/
#define die verbose_die
#define min(a,b) ((a)>(b)?(b):(a))

/*** CUnit ***/
#ifdef CUNIT
#define STDIN	t_stdin
#define STDOUT	t_stdout
#define STDERR	t_stderr
#define EXIT	t_exit
#else
#define STDIN	stdin
#define STDOUT	stdout
#define STDERR	stderr
#define EXIT	exit
#endif

/*!
 * @brief bldump main function.
 * @retval 0 normal termination.
 */
int main( int argc, char* argv[] )
{
	int ret;

	options_t opt;
	file_t    infile;
	memory_t  memory;

	options_reset( &opt );
	file_reset( &infile );
	memory_init( &memory );

#ifdef CUNIT
	/* run test */
	if ( argc == 2 && strcmp("--test", argv[1]) == 0  ) {
		extern CU_ErrorCode ts_verbose_regist(void);
		extern CU_ErrorCode ts_bldump_regist(void);
		extern CU_ErrorCode ts_cli_regist(void);
		CU_ErrorCode cue;
		cue = CU_initialize_registry();
		if ( cue == CUE_SUCCESS ) cue == ts_verbose_regist();
		if ( cue == CUE_SUCCESS ) cue == ts_bldump_regist();
		if ( cue == CUE_SUCCESS ) cue == ts_cli_regist();
		/* use CUnit Basic I/F */
		{
			unsigned int fails;
			CU_basic_set_mode(CU_BRM_VERBOSE);
			CU_basic_run_tests();
			fails = CU_get_number_of_failures();
			CU_cleanup_registry();
		}
		return CU_get_error();
	}
#endif

	/*** arguments ***/
	{
		int i;
		size_t a; /* for macro */
		char *sub;
		char *endp;

#define strlcmp(l,r) (strncmp(l,r,strlen(l)))
#define ARG_FLAG(s) (strcmp(s,argv[i])==0)
#define ARG_SPARAM(s) (strcmp(s,argv[i])==0 && (i+1) < argc && (sub=argv[++i]))
#define ARG_LPARAM(s) (strlcmp(s,argv[i])==0 && (a=strlen(s))<strlen(argv[i]) && (sub=&argv[i][a]))
		for (i = 1; i < argc; i++) {
			if (ARG_FLAG("-?") || ARG_FLAG("-h") || ARG_FLAG("--help")) {
				return help();
			} else if ( argv[i][0] == '-' ) {
				verbose_printf( VERB_ERR, "Error: unsupported option - %s\n", argv[i] );
				return EXIT_FAILURE;
			} else {
				break;
			}
		}

		if ( argc - i > 0 ) {
			opt.infile_name = argv[i++];
		} else {
			verbose_printf( VERB_ERR, "Error: not found argument - infile\n" );
			return EXIT_FAILURE;
		}

		if ( argc - i > 0 ) {
			opt.outfile_name = argv[i++];
		} else {
			opt.outfile_name = NULL;
		}
	}

	/*** set parameter. ***/
	{	
		/*** infile ***/
		if ( file_open( &infile, opt.infile_name, "rb" ) == false ) {
			verbose_printf( VERB_ERR, "Error: can't open infile - %s\n", opt.infile_name );
			return EXIT_FAILURE;
		}
	}

	return 0;
}

/*! 
 * @brief reset options.
 * @param[in] opt option data.
 */
void options_reset( options_t* opt )
{
	memset( opt, 0, sizeof(options_t) );

	//file
	opt->infile_name    = NULL;
	opt->outfile_name   = NULL;
}

//##############################################################################
// memory
//##############################################################################

/*!
 * @brief initialize memory_t.
 * @param[in] memory memory data.
 */
void memory_init( memory_t* memory )
{
	memset( memory, 0, sizeof(memory_t) );
}

/*!
 * @brief allocate memoly.
 * @param[in] memory memory data.
 * @param[in] length size of memory.
 * @retval true allocated memory.
 * @retval false couldn't allocate.
 */
bool memory_allocate( memory_t* memory, size_t length )
{
	bool retval;
	data_t* mem;

	if ( memory->data != NULL ) {
		verbose_printf( VERB_ERR, "Error: memory is already allocated.\n" );
		retval = false;
	}
	else {
		(void)verbose_printf( VERB_TRACE, "bldump: allocated memory %d byte\n", length );

		mem = (data_t*) malloc((size_t) length);

		if ( mem != NULL ) {
			memory->data    = mem;
			memory->length  = length;
			memory->size    = 0;
			memory->address = 0;
			retval = true;
		} else {
			verbose_printf( VERB_ERR, "Error: memory allocation failure\n" );
			retval = false;
		}
	}
	return retval;
}

/*!
 * @brief clear memory.
 * @param[in] memory memory data.
 */
void memory_clear( memory_t* memory )
{
	assert( memory->data != NULL );
	assert( memory->length > 0 );

	memory->size    = 0;
	memory->address = 0;
}

/*!
 * @brief free memory.
 * @param[in] memory memory data.
 * @retval true free memory.
 * @retval false not free because memory is not allocated.
 */
bool memory_free( memory_t* memory )
{
	bool retval;

	if ( memory->data != NULL )
	{
		free( memory->data );
		retval = true;
	}
	else {
		verbose_printf( VERB_ERR, "Error: memory is not allocated\n" );
		retval = false;
	}
	memory->data = NULL;
	memory->length = 0;

	return retval;
}

//##############################################################################
// file
//##############################################################################

/*!
 * @brief reset parameter of file.
 *
 * Before call this function, call file_close() to close file.
 * 
 * @param[in] file file data.
 * @see file_close
 */
void file_reset( file_t* file )
{
	memset( file, 0, sizeof(file_t) );

	file->ptr      = NULL;
	file->name     = NULL;
	file->position = 0L;
	file->length   = 0L;
}

/*!
 * @brief open file.
 * @param[in] file file data.
 * @param[in] name file name.
 * @param[in] mode file mode.
 * @retval true  opened file of 'name'.
 * @retval false couldn't opend.
 */
bool file_open( file_t* file, char* name, const char* mode )
{
	// file open
	file->name = name;
	if ( file->name == NULL || strlen( file->name ) == 0 ) {
		return false;
	}

	file->ptr = fopen( file->name, mode );
	if ( file->ptr == NULL ) {
		return false;
	}

	// get file length
	if ( mode[0] == 'r' ) { //read mode
		fseek( file->ptr, 0, SEEK_END );
		file->length = (size_t) ftell( file->ptr );
		fseek( file->ptr, 0, SEEK_SET);
	}

	(void)verbose_printf( VERB_LOG,
		"bldump: open file - name=%s, ptr=0x%x, length=%d, pos=0x%x\n",
		file->name,
		file->ptr,
		file->length,
		file->position );

	return true;
}

/*!
 * @brief file seek position.
 * @param[in] file file pointer.
 * @param[in] offset offset address.
 * @retval 0 success.
 * @retval none-zero failure.
 */
int file_seek( file_t* file, size_t offset )
{
	int retval;
	retval = fseek( file->ptr, (long)offset, SEEK_SET );
	if ( retval == 0 ) {
		file->position = offset;
	} else {
		(void)verbose_printf( VERB_ERR, "Error: fseek error - 0x%x\n", offset );
	}

	return retval;
}

/*!
 * @brief close file.
 * @param[in] file data.
 * @retval true  closed file.
 * @retval false couldn't close.
 */
bool file_close( file_t* file )
{
	bool retval;
	if ( file->ptr != NULL ) {
		fclose( file->ptr );
		file_reset( file );
		retval = true;
	}
	else {
		retval = false;
	}
	return retval;
}

/*!
 * @brief read data.
 * @param[in] file file pointer.
 * @param[out] memory write dump data.
 * @retval true success.
 * @retval false failture, cannot read because writable area is missing.
 */
bool file_read( file_t* file, memory_t* memory, size_t nmemb )
{
	const size_t size  = 1;
	size_t       reads = 0;

	assert( nmemb <= memory->length - memory->size );
		// nmemb must be set to 'memory' memory area.

	if ( nmemb <= 0 ) {
		assert( memory->length > 0 ); //! @test write buffer must be allocated. 
		assert( nmemb == 0 ); //! @test always length>=size 
		return false;
	}
	if ( memory->size == 0 ) {
		memory->address = file->position;
	}

	reads = fread( &memory->data[memory->size], size, nmemb, file->ptr );
	
	(void)verbose_printf( VERB_LOG, "bldump: fread - ret=%d, size=%d nmemb=%d fp=%x mem=%x\n", reads, size, nmemb, file->ptr, memory->data );

	file->position += reads;
	memory->size   += reads;

	return true;
}

/*!
 * @brief write data.
 * @param[out] file file pointer.
 * @param[in] memory read dump data.
 */
void file_write( file_t* file, memory_t* memory )
{
	size_t nmemb = memory->size;
	nmemb = fwrite( memory->data, 1, nmemb, file->ptr );
	file->length += nmemb;
}

/*!
 * @brief display help message.
 */
int help(void) //{{{
{
	int i, size;
	size = (int)(sizeof(usage)/sizeof(const char*));
	for( i=0; i<size; i++ ) fprintf(STDERR, "%s\n", usage[i] );
	return EXIT_FAILURE;
}

//}}}

/* vim:fdm=marker:
 */
