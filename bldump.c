/*!
 * @file
 * @brief bldump - simple decimal, hex dump to text file, CSV ..
 * @author yukio
 * @since 2009-09-20 
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
#ifndef VERSION
#define VERSION "0.x"
#endif
#define BUILD   __DATE__

const static char *usage[] = {
	"Usage: bldump [<options>] [<infile> [<outfile>]]",
	"",
	"  <infile>",
	"    dump file.",
	"",
	"  <outfile>",
	"    output file name. if not specified, output stdout.",
	"",
	/* container */
	"  -f <num>, --fields=<num>",
	"    The number of data fields of displaying at a line(default:16).",
	"",
	"  -l <num>, --length=<num>",
	"    The number of data bytes of displaying(default:1).",
	"",
	"  -r <order>, --reorder=<order>",
	"    Change byte-order of the input data.",
	"    <order> consists of 0-7.",
	"",
	/* inputs */
	"  -s <num>, --start-address=<num>",
	"    Skip <num> bytes from the beggining of the inputs.",
	"",
	"  -e <num>, --end-address=<num>",
	"    Stop reading data reached to the <num> address.",
	"",
	"  -S<hex>, --search=<hex>",
	"    Skip data to searching for <hex> pattern.",
	"",
	/* output */
	"  -i, --decimal",
	"    Displays decimal.",
	"",
	"  -u, --unsigned",
	"    Displays unsigned decimal.",
	"",
	"  -A, --ascii",
	"    Displays character.",
	"",
	"  -b, --binary",
	"    Outputs binary.",
	"",
	"  -a, --show-address",
	"    Displays data address preceded each line.",
	"    if not specified, doesn't display.",
	"",
	"  -d <str>, --delimitter=<str>",
	"    The field delimitter character(default:' ').",
	"",
	/* debug */
	"  -v <num>, --verbose=<num>",
	"    verbose mode(default:3).",
	"",
	/* others */
	"  -h -? --help",
	"    displays command line help message, and exit application.",
	"",
	"  --version",
	"    bldump version, build date.",
	""
};

/*** ifdef ***/
#ifdef CUNIT
#define DEBUG_ASSERT(c)		assert(c)
#else
#define DEBUG_ASSERT(c)
#endif
/* assert() is check always
 * DEBUG_ASSERT() is check in debug.
 */

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
	bool is_ok;

	options_t opt; 
	file_t    infile;
	file_t    outfile;
	memory_t  memory;

	if ( argc == 2 && strcmp("--version", argv[1]) == 0  ) {
		fprintf( (STDOUT)?STDOUT:stdout, "bldump version %s (%s)\n", VERSION, BUILD );
		return 0;
	}
#ifdef CUNIT
	/* run test */
	if ( argc == 2 && strcmp("--test", argv[1]) == 0  ) {
		unsigned int fails = 0;
		extern CU_ErrorCode ts_verbose_regist(void);
		extern CU_ErrorCode ts_opt_regist(void);
		extern CU_ErrorCode ts_memory_regist(void);
		extern CU_ErrorCode ts_file_regist(void);
		extern CU_ErrorCode ts_bldump_regist(void);
		extern CU_ErrorCode ts_main_regist(void);
		CU_ErrorCode cue;
		cue = CU_initialize_registry();
		if ( cue == CUE_SUCCESS ) cue = ts_verbose_regist();
		if ( cue == CUE_SUCCESS ) cue = ts_opt_regist();
		if ( cue == CUE_SUCCESS ) cue = ts_memory_regist();
		if ( cue == CUE_SUCCESS ) cue = ts_file_regist();
		if ( cue == CUE_SUCCESS ) cue = ts_bldump_regist();
		if ( cue == CUE_SUCCESS ) cue = ts_main_regist();
		/* use CUnit Basic I/F */
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_basic_run_tests();
		fails = CU_get_number_of_failures();
		CU_cleanup_registry();
		return (fails != 0) ? 1 : CU_get_error();
	}
	assert( t_stdin != NULL && t_stdout != NULL && t_stderr != NULL );
#endif


	/*** prepare ***/
	verbose_level = VERB_DEFAULT;
	if ( verbose_out == NULL ) verbose_out = stdout;

	options_reset( &opt );  
	memory_init( &memory );
	file_reset( &infile );
	file_reset( &outfile );

	/*** arguments ***/
	is_ok = options_load( &opt, argc, argv  );

	/*** set parameter. ***/
	if ( is_ok == true ) {
		is_ok = bldump_setup( &memory, &infile, &outfile, &opt );
	}

	/*** bldump ***/
	if ( is_ok == true ) {
		while( feof(infile.ptr)==0 ) {
			is_ok = bldump_read( &memory, &infile, &opt );
			if ( is_ok == false || memory.size == 0 ) {
				break;
			}

			is_ok = bldump_write( &memory, &outfile, &opt );
			if ( is_ok == false ) {
				break;
			}
		}
	}

	/*** dispose ***/
	(void)file_close( &infile );
	if ( memory.data != NULL ) {
		(void)memory_free( &memory );
	}
	(void)options_clear( &opt );

	return (is_ok == true) ? 0 : 1;
}

/**********
 * bldump *
 **********/

/*!
 * @brief setup parameter.
 * @param[out] memory
 * @param[out] infile
 * @param[out] outfile
 * @param[in] opt
 * @retval 0 success.
 * @retval 1 failure.
 */
bool bldump_setup( memory_t* memory, file_t* infile, file_t* outfile, options_t* opt )
{
	bool is;
	size_t size;

	/* infile */
	if ( file_open( infile, opt->infile_name, "rb" ) == false ) {
		(void)verbose_printf( VERB_ERR, "Error: can't open infile - %s\n", opt->infile_name );
		return false;
	}
	if ( opt->start_address > 0 ) {
		(void)file_seek( infile, opt->start_address ) ;
	}

	/* outfile */
	if ( opt->outfile_name == NULL ) {
		(void)verbose_printf( VERB_LOG, "bldump: output to `stdout\' insted of outfile.\n" );
		outfile->ptr    = STDOUT;
		outfile->length = 0;
	} else {
		if ( file_open( outfile, opt->outfile_name, "wb" ) == false ) {
			(void)verbose_printf( VERB_ERR, "Error: can't open outfile - %s\n", opt->outfile_name );
			return false;
		}
	}

	/* memory */
	if ( opt->data_length == 0 ) {
		(void)verbose_printf( VERB_ERR, "Error: wrong data_length=%d\n", opt->data_length );
		return false;
	}
	if ( opt->data_fields<= 0 ) {
		(void)verbose_printf( VERB_ERR, "Error: wrong data_fields=%d\n", opt->data_fields);
		return false;
	}
	size = (size_t) (opt->data_length * opt->data_fields);
	
	is = memory_allocate( memory, size );

	return is;
}

/*!
 * @brief read data.
 * @param[out] memory
 * @param[in] infile
 * @param[in] opt
 * @retval true success.
 * @retval false failure.
 */
bool bldump_read( memory_t* memory, file_t* infile, /*@unused@*/ options_t* opt )
{
	bool is;
	size_t nmemb;
	size_t limit;

	DEBUG_ASSERT( memory->length > 0 );

	memory_clear( memory );

	if ( opt->search_length > 0 ) {
		is = file_search( infile, memory, opt );
		if ( is == false ) {
			return true;
		}
	}

	nmemb = memory->length - memory->size;
	if ( opt->end_address != 0 ) {
		if ( infile->position >= opt->end_address ) {
			return false;
		}
		limit = opt->end_address - infile->position;
		if ( nmemb > limit ) {
			nmemb = limit;
			(void)verbose_printf( VERB_WARNING, "Warning: cut off the reading size less than end-address.\n" );
		}
	}

	is = file_read( infile, memory, nmemb );

	if ( is == false || memory->size == 0 ) {
		(void)verbose_printf( VERB_DEBUG, "bldump: file read failure.\n" );
	} else {
		/* reordering */
		if ( opt->data_order[0] != -1 ) {
			size_t i, j, k, idx = 0;
			for ( i=0; i<memory->size; i+=opt->data_length ) {
				uint64_t data = 0;
				for ( j=0, k=(opt->data_length-1)*8;
					j<opt->data_length; j++, k-=8 ) {
					size_t loc=i+opt->data_order[j];
					if ( loc < memory->size ) {
						data = data | (((uint64_t)memory->data[loc]) << k);
					}
				}

				for ( j=0, k=(opt->data_length-1)*8;
					(j<opt->data_length) && (i+j) < memory->length;
					j++, k-=8 ) {
					idx = i+j;
					memory->data[i+j] = (data_t) (data >> k);
				}
			}
			if ( idx >= memory->size ) {
				assert( idx <= memory->length );
				memory->size = idx;
			}
		}
	}
	return is;
}

/*!
 * @brief write data
 * @param[in] memory
 * @param[out] outfile
 * @param[in] opt
 * @retval true success.
 * @retval false failure.
 */
bool bldump_write( memory_t* memory, file_t* outfile, options_t* opt )
{
	/*** output ***/
	switch( opt->output_type )
	{
		case HEXADECIMAL:
		default:
			write_hex( memory, outfile, opt );
			break;
		case UDECIMAL:
		case DECIMAL:
			write_dec( memory, outfile, opt );
			break;
		case BINARY:
			file_write( outfile, memory );
			break;
		case ASCII:
			to_printable( memory );
			write_hex( memory, outfile, opt );
			break;
	}
	return true ;
}

/*!
 * @brief print hex data.
 * @param[in] memory read dump data.
 * @param[out] file file pointer.
 * @param[in] opt
 */
void write_hex( memory_t* memory, file_t* outfile, options_t* opt )
{
	size_t i, j;
	size_t data_len = opt->data_length;

	DEBUG_ASSERT( opt->col_delimitter != NULL );
	DEBUG_ASSERT( opt->row_delimitter != NULL );

	/*** output address  ***/
	if ( opt->show_address == true ) {
		fprintf( outfile->ptr, "%08lx: ", (unsigned long)memory->address );
	}

	for ( i = 0; i < memory->size; ) {
		/*** column delimitter ***/
		if ( opt->col_delimitter != NULL && i != 0 ) {
			(void)fputs( opt->col_delimitter, outfile->ptr );
		}

		/*** output data ***/
		for ( j = 0; j < data_len; j++ ) {
			fprintf( outfile->ptr, opt->output_format, (unsigned int) memory->data[i] );
			++i;
			if ( i >= memory->size ) {
				break;
			}
		}
	}

	(void)fputs( opt->row_delimitter, outfile->ptr ); /* line separater */
}

/*!
 * @brief print decimal.
 * @param[in] memory read dump data.
 * @param[out] file file pointer.
 * @param[in] opt
 */
void write_dec( memory_t* memory, file_t* outfile, options_t* opt )
{
	size_t i, j;
	size_t data_len = opt->data_length;
	int64_t data;

	/*** output address ***/
	if ( opt->show_address == true ) {
		fprintf( outfile->ptr, "%08lx: ", (unsigned long)memory->address );
	}

	for ( i = 0; i < memory->size; ) {
		/*** column delimitter ***/
		if ( opt->col_delimitter != NULL && i != 0 ) {
			(void)fputs( opt->col_delimitter, outfile->ptr );
		}

		/*** output data ***/
		data = 0;
		for ( j = 0; j < data_len; j++ ) {
			data = (data << 8) | memory->data[i];
			++i;
			if ( i >= memory->size ) {
				break;
			}
		}
		if ( opt->output_type == DECIMAL ) {
			int s = ((int)sizeof(data) - (int)data_len) * 8;
			data = (data << s) >> s; /* expanded zero */
		}
		(void)fprintf( outfile->ptr, opt->output_format, (long long int)data );
	}
	(void)fputs( opt->row_delimitter, outfile->ptr ); /* line separater */
}

void to_printable( memory_t* memory )
{
	size_t i;
	for ( i = 0; i < memory->size; i++ )
	{
		if ( isprint( memory->data[i] ) == 0 ) {
			memory->data[i] = (data_t) '.';
		}
	}
}

/***********
 * options *
 ***********/

/*! 
 * @brief reset options.
 * @param[in] opt option data.
 */
void options_reset( options_t* opt )
{
	memset( opt, 0, sizeof(options_t) );

	/*** file ***/
	opt->infile_name    = NULL;
	opt->outfile_name   = NULL;

	/*** input ***/
	opt->start_address  = 0;
	opt->end_address    = 0;

	/*** container ***/
	opt->data_length    = 0;
	opt->data_fields    = 0;
	opt->data_order[0]  = -1;

	/*** outfile ***/
	opt->output_type    = HEXADECIMAL;
	opt->output_format  = NULL;
	opt->show_address   = false;
	opt->col_delimitter  = NULL;
	opt->row_delimitter  = NULL;
}

/*! 
 * @brief clear options.
 * @param[in,out] opt option data.
 */
bool options_clear( options_t* opt )
{
	bool retval = true;
	if ( strfree( opt->infile_name ) == true ) {
		opt->infile_name = NULL;
	} else {
		retval = false;
	}
	if ( strfree( opt->outfile_name ) == true ) {
		opt->outfile_name = NULL;
	} else {
		retval = false;
	}
	if ( strfree( opt->col_delimitter ) == true ) {
		opt->col_delimitter = NULL;
	} else {
		retval = false;
	}
	if ( strfree( opt->row_delimitter ) == true ) {
		opt->row_delimitter = NULL;
	} else {
		retval = false;
	}

	return retval;
}

/*!
 * @brief load options from CLI arguments.
 * @param[out] opt option parameter.
 * @param[in] argc arguments data count.
 * @param[in] argv arguments data.
 * @retval true success.
 * @retval false failure.
 */
bool options_load( options_t* opt, int argc, char* argv[] )
{
	int i;
	size_t a; /* for macro */
	char *sub;

#define strlcmp(l,r) (strncmp(l,r,strlen(l)))
#define ARG_FLAG(s) (strcmp(s,argv[i])==0)
#define ARG_SPARAM(s) (strcmp(s,argv[i])==0 && (i+1) < argc && ((sub=argv[++i])!=NULL))
#define ARG_LPARAM(s) (strlcmp(s,argv[i])==0 && (a=strlen(s))<strlen(argv[i]) && ((sub=&argv[i][a])!=NULL))
	for (i = 1; i < argc; i++) {
		/* help */
		if (ARG_FLAG("-?") || ARG_FLAG("-h") || ARG_FLAG("--help")) {
			(void) help();
			return false;

		/* input */
		} else if ( ARG_SPARAM("-s") || ARG_LPARAM("--start-address=") ) {
			opt->start_address = (size_t)strtoul( sub, NULL, 0 );
		} else if ( ARG_SPARAM("-e") || ARG_LPARAM("--end-address=") ) {
			opt->end_address = (size_t)strtoul( sub, NULL, 0 );
		} else if ( ARG_SPARAM("-S") || ARG_LPARAM("--search=") ) {
			char fmt[64];
			size_t length = strlen( sub );
			opt->search_pattern = (uint64_t) strtoull( sub, (char**)NULL, 16 );
			if ( (length & 1) != 0 ) {
				(void)verbose_printf( VERB_WARNING, "Warning: pattern should be byte align, append 4 bit for stuffing.\n" );
				length++;
				opt->search_pattern = opt->search_pattern << 4;
			}
			opt->search_length  = (int)(length * 4uL); /* number of bits */
			/*@i@*/ sprintf( fmt, "bldump: set searching pattern - pat=0x%%0%dllx, size=%%ld \n", length );
			(void)verbose_printf( VERB_LOG, fmt, opt->search_pattern, opt->search_length );

		/* memory */
		} else if ( ARG_SPARAM("-l") || ARG_LPARAM("--length=") ) {
			if ( opt->data_length != 0 ) {
				(void)verbose_printf( VERB_ERR, "Error: can't set opt -r and -l at once.\n" );
				return false;
			}
			opt->data_length = (size_t)strtoul( sub, NULL, 0 );
		} else if ( ARG_SPARAM("-f") || ARG_LPARAM("--fields=") ) {
			opt->data_fields= (int)strtoul( sub, NULL, 0 );
		} else if ( ARG_SPARAM("-r") || ARG_LPARAM("--reorder=") ) {
			uint64_t order;
			size_t j, k;
			if ( opt->data_length != 0 ) {
				(void)verbose_printf( VERB_ERR, "Error: can't set opt -r and -l at once.\n" );
				return false;
			}
			order            = (uint64_t) strtoull( sub, NULL, 16 );
			opt->data_length = (size_t)strlen( sub );
			if ( opt->data_length <= 1 || opt->data_length > 8 ) {
				(void)verbose_printf( VERB_ERR, "Error: order string is too long or <= 1 - len=%d\n", opt->data_length );
				return false;
			}
			for ( j=0, k=(opt->data_length*4)-4; j < opt->data_length; j++, k-=4 ) {
				opt->data_order[j] = (int)((order >> k) & 0xFuLL);
				if ( opt->data_order[j] >= (int)opt->data_length ) {
					(void)verbose_printf( VERB_ERR, "Error: data reordering pattern is out of range - %x\n", opt->data_order[i] );
					return false;
				}
			}
			(void)verbose_printf( VERB_DEBUG, "bldump: set order len=%d pat=", opt->data_length );
			for ( j=0; j<opt->data_length; j++ ) (void)verbose_printf( VERB_DEBUG, "%2d ", opt->data_order[j] );
			(void)verbose_printf( VERB_DEBUG, "\n" );

		/* output */
		} else if ( ARG_FLAG("-a") || ARG_FLAG("--show-address") ) {
			opt->show_address = true;
		} else if ( ARG_SPARAM("-d") || ARG_LPARAM("--delimitter=") ) {
			opt->col_delimitter = strclone( sub );
		} else if ( ARG_FLAG("-i") || ARG_FLAG("--decimal") ) {
			opt->output_type = DECIMAL;
			opt->output_format = "%lld";
		} else if ( ARG_FLAG("-u") || ARG_FLAG("--unsigned") ) {
			opt->output_type = UDECIMAL;
			opt->output_format = "%llu";
		} else if ( ARG_FLAG("-b") || ARG_FLAG("--binary") ) {
			opt->output_type = BINARY;
		} else if ( ARG_FLAG("-A") || ARG_FLAG("--ascii") ) {
			opt->output_type = ASCII;
			opt->output_format = "%c";

		/* debug */
		} else if ( ARG_SPARAM("-v") || ARG_LPARAM("--verbose=") ) {
			verbose_level = (unsigned int)strtoul( sub, NULL, 0 ); 

		/* error */
		} else if ( argv[i][0] == '-' ) {
			(void)verbose_printf( VERB_ERR, "Error: unsupported option - %s\n", argv[i] );
			return false;
		} else {
			break;
		}
	}

	/* file name */
	if ( argc - i > 0 ) {
		assert( strlen(argv[i]) != 0 );
		opt->infile_name = strclone( argv[i] );
		i++;
	} else {
		(void)verbose_printf( VERB_ERR, "Error: not found argument - infile\n" );
		return false;
	}

	if ( argc - i > 0 ) {
		assert( strlen(argv[i]) != 0 );
		opt->outfile_name = strclone( argv[i] );
		i++;
	} else {
		opt->outfile_name = NULL;
	}

	/* delimitter */
	if ( opt->col_delimitter == NULL ) {
		opt->col_delimitter  = strclone( " " );
	}
	if ( opt->row_delimitter == NULL ) {
		opt->row_delimitter  = strclone( "\n" );
	}

	if ( opt->data_length == 0 ) {
		opt->data_length = 1;
	}
	if ( opt->data_fields == 0 ) {
		opt->data_fields = 16;
	}
	if ( opt->output_format == NULL ) {
		opt->output_format = "%02x";
	}

	return true;
}

/**********
 * memory *
 **********/

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
		(void)verbose_printf( VERB_ERR, "Error: memory is already allocated.\n" );
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
			(void)verbose_printf( VERB_ERR, "Error: memory allocation failure\n" );
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
		(void)verbose_printf( VERB_ERR, "Error: memory is not allocated\n" );
		retval = false;
	}
	memory->data = NULL;
	memory->length = 0;

	return retval;
}

/********
 * file *
 ********/

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
bool file_open( file_t* file, const char* name, const char* mode )
{
	/*** check parameter ***/
	if ( name == NULL || strlen( name ) == 0 || mode == NULL ) {
		return false;
	}

	/*** file open ***/
	file->name = strclone( name );
	file->ptr  = fopen( name, mode );
	if ( file->ptr == NULL ) {
		return false;
	}

	/*** get file length ***/
	if ( mode[0] == 'r' ) { /* read mode */
		(void)fseek( file->ptr, 0, SEEK_END );
		file->length = (size_t) ftell( file->ptr );
		(void)fseek( file->ptr, 0, SEEK_SET);
	}

	(void)verbose_printf( VERB_LOG,
		"bldump: open file - name=%s, ptr=0x%x, length=%d, pos=0x%x\n",
		(file->name == NULL) ? "(NULL)" : file->name,
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
	bool retval = true;

	if ( (file->ptr == STDOUT) || (file->ptr == STDERR) || (file->ptr == STDIN) ) {
	} else if ( file->ptr != NULL ) {
		(void)fclose( file->ptr );
	} else {
		retval = false;
	}
	if ( file->name != NULL ) {
		free( file->name );
	} else {
		retval = false;
	}

	file_reset( file );

	return retval;
}

/*!
 * @brief read data.
 * @param[in] file file pointer.
 * @param[out] memory write dump data.
 * @retval true  success.
 * @retval false failure, cannot read then memory isnot updated.
 */
bool file_read( file_t* file, memory_t* memory, size_t nmemb )
{
	const size_t size  = 1;
	size_t       reads = 0;
	size_t       pos   = 0;
	bool         is    = false;

	assert( nmemb <= memory->length - memory->size ); /* nmemb must be set to 'memory' memory area. */

	if ( feof(file->ptr) == 1 ) {
		is = true;
	} else if ( nmemb == 0 ) {
		is = false;
	} else {
		pos   = file->position;
		reads = fread( &memory->data[memory->size], size, nmemb, file->ptr );
	
		(void)verbose_printf( VERB_LOG, "bldump: fread - ret=%d, size=%d nmemb=%d fp=%x mem=%x\n", reads, size, nmemb, file->ptr, memory->data );

		if ( reads == 0 ) {
			if ( feof(file->ptr) == 1 ) {
				is = true;
			}
		} else {
			if ( memory->size == 0 ) {
				memory->address = pos;
			}
			file->position += reads;
			memory->size   += reads;
			is = true;
		}
	}

	return is;
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
 * @brief search the pattern in file.
 * @param[in] file   file information.
 * @param[in] search searching information.
 * @retval true found target pattern.
 * @retval false not found (proberbly reached EOF).
 */
bool file_search( file_t* file, memory_t* memory, options_t* opt )
{
	int i, j;
	uint64_t mask = (uint64_t)((0x1uLL << opt->search_length) - 0x1uLL);
	uint64_t read = 0;
	int search_bytes = opt->search_length/8;

	DEBUG_ASSERT( memory->size == 0 );
	DEBUG_ASSERT( opt->search_length > 0 );

	(void)verbose_printf( VERB_TRACE, "bldump: file_search - mask=0x%llx pat=0x%llx\n",
		mask, opt->search_pattern );

	if ( feof(file->ptr) != 0 ) {
		(void)verbose_printf( VERB_TRACE, "bldump: detected EOF on file searching.\n" );
		return false;
	}

	/*** charge buffer ***/
	for ( i = 0; i < search_bytes; i++ ) {
		if ( (opt->end_address > 0) && (opt->end_address <= (size_t)ftell(file->ptr)) ) {
			(void)verbose_printf( VERB_TRACE, "bldump: detected end address on file searching.\n" );
			return false;
		}

		read = (read << 8) | ((uint8_t)fgetc(file->ptr));

		if ( feof(file->ptr) != 0 ) {
			(void)verbose_printf( VERB_TRACE, "bldump: detected EOF on file searching.\n" );
			return false;
		}
	}

	/*** search for pattern ***/
	while ( (read & mask) != opt->search_pattern ) {
		if ( opt->end_address > 0 && opt->end_address <= (size_t)ftell(file->ptr) ) {
			(void)verbose_printf( VERB_TRACE, "bldump: detected end address on file searching.\n" );
			return false;
		}

		read = (read << 8) | ((uint8_t)fgetc(file->ptr));

		if ( feof(file->ptr) != 0 ) {
			(void)verbose_printf( VERB_TRACE, "bldump: detected EOF on file searching.\n" );
			return false;
		}
	};

	/*** read file and write to memory ***/
	assert( (read & mask) == opt->search_pattern );

	file->position  = (size_t) ftell(file->ptr);
	memory->address = file->position - search_bytes;
	
	for ( j=0, i=search_bytes-1; i >= 0; i--, j++ ) {
		if ( i < (int)memory->length ) {
			memory->data[j] = (data_t)(opt->search_pattern >> (i*8));
		}
	}
	memory->size = (size_t)search_bytes;

	return true;
}

/*!
 * @brief display help message.
 */
int help(void) /*{{{*/
{
	int i, size;
	size = (int)(sizeof(usage)/sizeof(const char*));
	for( i=0; i<size; i++ ) fprintf(STDERR, "%s\n", usage[i] );
	return EXIT_FAILURE;
}
/*}}}*/

/*!
 * @brief clone string.
 * @param[in] str string to clone.
 * @retval string memory pointer that point new memory and set string same as 'str'.
 */
char* strclone( const char* str ) /*{{{*/
{
	char* retval;
	DEBUG_ASSERT( str != NULL );

	retval = (char*)malloc( strlen(str)+1 );
	if ( retval != NULL ) {
		strcpy( retval, str );
	}
	return retval;
}
/*}}}*/

/*!
 * @brief free memory of string.
 * @param[in] str string pointer.
 * @retval true success.
 * @retval false failure.
 */
bool strfree( char* str ) /*{{{*/
{
	bool retval = true;
	if ( str != NULL ) {
		free( str );
	} else {
		retval = false;
	}
	return retval;
}
/*}}}*/


/* vim:fdm=marker:
 */
