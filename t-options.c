/*!
 * @file
 * @brief unit test of CLI.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "munit.h"
#include "verbose.h"
#include "bldump.h"

/*!
 * @biref test bldump::options_clear().
 */
static void t_options_clear(void)
{
	bool is;
	options_t opt;
	options_reset( &opt );

	/* normal */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	mu_assert_equal( is, true );
	mu_assert_ptr_null( opt.infile_name );
	mu_assert_ptr_null( opt.outfile_name );
	mu_assert_ptr_null( opt.col_delimitter );
	mu_assert_ptr_null( opt.row_delimitter );

	/* error */
	opt.infile_name   = NULL;
	opt.outfile_name  = (char*)malloc(1);
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	mu_assert_equal( is, false );
	mu_assert_ptr_null( opt.infile_name );
	mu_assert_ptr_null( opt.outfile_name );
	mu_assert_ptr_null( opt.col_delimitter );
	mu_assert_ptr_null( opt.row_delimitter );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = NULL;
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	mu_assert_equal( is, false );
	mu_assert_ptr_null( opt.infile_name );
	mu_assert_ptr_null( opt.outfile_name );
	mu_assert_ptr_null( opt.col_delimitter );
	mu_assert_ptr_null( opt.row_delimitter );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_delimitter = NULL;
	opt.row_delimitter = (char*)malloc(1);
	is = options_clear( &opt );
	mu_assert_equal( is, false );
	mu_assert_ptr_null( opt.infile_name );
	mu_assert_ptr_null( opt.outfile_name );
	mu_assert_ptr_null( opt.col_delimitter );
	mu_assert_ptr_null( opt.row_delimitter );

	/* error */
	opt.infile_name   = (char*)malloc(1);
	opt.outfile_name  = (char*)malloc(1);
	opt.col_delimitter = (char*)malloc(1);
	opt.row_delimitter = NULL;
	is = options_clear( &opt );
	mu_assert_equal( is, false );
	mu_assert_ptr_null( opt.infile_name );
	mu_assert_ptr_null( opt.outfile_name );
	mu_assert_ptr_null( opt.col_delimitter );
	mu_assert_ptr_null( opt.row_delimitter );
}

/*!
 * @biref test "bldump -".
 */
static void t_opt_noopt(void)
{
	options_t opt;
	bool is;
	char* argv[] = { "bldump", "-" };
	options_reset( &opt );
	is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
	mu_assert_equal( is, false );
}

/*!
 * @biref test "bldump".
 */
static void t_opt_noarg(void)
{
	options_t opt;
	bool is;
	char* argv[] = { "bldump" };
	options_reset( &opt );
	is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
	mu_assert_equal( is, false );
}

/*!
 * @biref test "bldump -h".
 */
static void t_opt_help(void)
{
	options_t opt;
	bool is;

	/* -h */
	{
		char* argv[] = { "bldump", "-h" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, false );
	}
	/* -? */
	{
		char* argv[] = { "bldump", "-?" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, false );
	}
	/* --help */
	{
		char* argv[] = { "bldump", "--help" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, false );
	}
}

/*!
 * @brief test infile and outfile
 */
static void t_opt_infile_outfile(void)
{
	options_t opt;
	bool is;

	/* infile */
	{
		char* argv[] = { "bldump", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_string_equal( opt.infile_name, "infile" );
		mu_assert_ptr_null( opt.outfile_name );
	}

	/* infile outfile */
	{
		options_t opt;
		bool is;
		char* argv[] = { "bldump", "infile", "outfile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_string_equal( opt.infile_name, "infile" );
		mu_assert_string_equal( opt.outfile_name, "outfile" );
	}
}

/*!
 * @brief test -l, --length
 */
static void t_opt_length(void)
{
	options_t opt;
	bool is;

	/* -l */
	{
		char* argv[] = { "bldump", "-l", "3", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.data_length, 3 );
		mu_assert_ptr_null( opt.outfile_name );
	}

	/* --length */
	{
		char* argv[] = { "bldump", "--length=4", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.data_length, 4 );
	}

	/* -l --length (error for conflict of other options) */
	{
		char* argv[] = { "bldump", "-l", "3", "--length=4", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); /* duplicated option */
		mu_assert_equal( is, false );
	}

}

/*!
 * @brief test -f, --fields
 */
static void t_opt_fields(void)
{
	options_t opt;
	bool is;

	/* -f */
	{
		char* argv[] = { "bldump", "-f", "5", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.data_fields, 5 );
		mu_assert_ptr_null( opt.outfile_name );
	}

	/* --fields */
	{
		char* argv[] = { "bldump", "--fields=6", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.data_fields, 6 );
	}
}

/*!
 * @brief test -a, --show-address
 */
static void t_opt_address(void)
{
	options_t opt;
	bool is;

	/* -a */
	{
		char* argv[] = { "bldump", "-a", "infile" };
		options_reset( &opt );
		mu_assert_equal( opt.show_address, false );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.show_address, true );
		mu_assert_ptr_null( opt.outfile_name );
	}

	/* --show_address */
	{
		char* argv[] = { "bldump", "--show-address", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.show_address, true );
	}
}

/*!
 * @brief test -s, --start-address
 */
static void t_opt_start(void)
{
	options_t opt;
	bool is;

	/* -s */
	{
		char* argv[] = { "bldump", "-s", "1", "infile" };
		options_reset( &opt );
		mu_assert_equal( opt.start_address, 0 );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.start_address, 1 );
	}

	/* --start_address */
	{
		char* argv[] = { "bldump", "--start-address=2", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.start_address, 2 );
	}
}

/*!
 * @brief test -e, --end-address
 */
static void t_opt_end(void)
{
	options_t opt;
	bool is;

	/* -e */
	{
		char* argv[] = { "bldump", "-e", "1", "infile" };
		options_reset( &opt );
		mu_assert_equal( opt.end_address, 0 );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.end_address, 1 );
	}

	/* --end_address */
	{
		char* argv[] = { "bldump", "--end-address=2", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.end_address, 2 );
	}
}
/*!
 * @brief test -v, --verbose
 */
static void t_opt_verbose(void)
{
	options_t opt;
	bool is;

	/* -v */
	{
		char* argv[] = { "bldump", "-v", "3" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, false );
		mu_assert_equal( verbose_level, 3 );
	}

	/* --verboes */
	{
		char* argv[] = { "bldump", "--verbose=4", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( verbose_level, 4 );
	}
}

/*!
 * @brief test -d, --delimitter
 */
static void t_opt_delimitter(void)
{
	options_t opt;
	bool is;

	/* -d */
	{
		char* argv[] = { "bldump", "-d", ",", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_string_equal( opt.col_delimitter, "," );
	}

	/* --delimitter */
	{
		char* argv[] = { "bldump", "--delimitter=\t", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_string_equal( opt.col_delimitter, "\t" );
	}
}

/*!
 * @brief test -i, --decimal
 */
static void t_opt_dec(void)
{
	options_t opt;
	bool is;

	/* -i */
	{
		char* argv[] = { "bldump", "-i", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.output_type, DECIMAL );
		mu_assert_string_equal( opt.output_format, "%lld" );
	}

	/* --decimal */
	{
		char* argv[] = { "bldump", "--decimal", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.output_type, DECIMAL );
	}
}

/*!
 * @brief test -u, --unsigned
 */
static void t_opt_udec(void)
{
	options_t opt;
	bool is;

	/* -u */
	{
		char* argv[] = { "bldump", "-u", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.output_type, UDECIMAL );
		mu_assert_string_equal( opt.output_format, "%llu" );
	}

	/* --unsigned */
	{
		char* argv[] = { "bldump", "--decimal", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.output_type, DECIMAL );
	}
}

/*!
 * @brief test -b, --binary
 */
static void t_opt_bin(void)
{
	options_t opt;
	bool is;

	/* -b */
	{
		char* argv[] = { "bldump", "-b", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.output_type, BINARY );
	}

	/* --binary */
	{
		char* argv[] = { "bldump", "--binary", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.output_type, BINARY );
	}
}

/*!
 * @brief test -r, --reorder
 */
static void t_opt_reorder(void)
{
	options_t opt;
	bool is;

	/* -r */
	{
		char* argv[] = { "bldump", "-r", "10", "infile" };
		options_reset( &opt );
		mu_assert_equal( opt.data_order[0], -1 );

		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.data_length, 2 );
		mu_assert_equal( opt.data_order[0], 1 );
		mu_assert_equal( opt.data_order[1], 0 );
	}

	/* --reoder */
	{
		char* argv[] = { "bldump", "--reorder=012", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.data_length, 3 );
		mu_assert_equal( opt.data_order[0], 0 );
		mu_assert_equal( opt.data_order[1], 1 );
		mu_assert_equal( opt.data_order[2], 2 );
	}

	/* -r 0 -r 0 (error) */
	{
		char* argv[] = { "bldump", "-r", "10", "-r", "10", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, false );
	}

	/* -r 012345678 (error) */
	{
		char* argv[] = { "bldump", "-r", "012345678", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, false );
	}

	/* -r 31 (error) */
	{
		char* argv[] = { "bldump", "-r", "31", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, false );
	}
}

/*!
 * @brief test -S, --search
 */
static void t_opt_search(void)
{
	options_t opt;
	bool is;

	/* -S */
	{
		char* argv[] = { "bldump", "-S", "0123", "infile" };
		options_reset( &opt );

		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.search_pattern, 0x0123 );
		mu_assert_equal( opt.search_length,  16 );
	}

	/* --search */
	{
		char* argv[] = { "bldump", "--search=234567", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( is, true );
		mu_assert_equal( opt.search_pattern, 0x234567 );
		mu_assert_equal( opt.search_length,  24 );
	}

	/* -S 1 (error) */
	{
		char* argv[] = { "bldump", "-S", "1", "infile" };
		options_reset( &opt );
		is = options_load( &opt, (int)(sizeof(argv)/sizeof(char*)), argv ); 
		mu_assert_equal( opt.search_pattern, 0x10 );
		mu_assert_equal( opt.search_length,  8 );
		//printf("search: len=%d, pat=%x\n", opt.search_length, opt.search_pattern );
	}
}

void ts_opt(void)
{
	/* init */
	assert(verbose_out==NULL); //Error: verbose_out is not NULL
	t_stdin  = tmpfile();
	t_stdout = tmpfile();
	t_stderr = tmpfile();
	verbose_out = tmpfile();
	assert(t_stdin!=NULL||t_stdout!=NULL||t_stderr!=NULL);//Error: tmpfile() failure
	
	/* test */
	mu_run_test(t_options_clear);
	mu_run_test(t_opt_noarg);          //options_load( bldump )
	mu_run_test(t_opt_noopt);          //options_load( bldump - )
	mu_run_test(t_opt_help);           //options_load( bldump -h|-?|--help )
	mu_run_test(t_opt_infile_outfile); //options_load( bldump infile outfile )
	mu_run_test(t_opt_length);         //options_load( bldump -l|--length infile )
	mu_run_test(t_opt_fields);         //options_load( bldump -f|--fields infile )
	mu_run_test(t_opt_address);        //options_load( bldump -a|--show-address )
	mu_run_test(t_opt_start);          //options_load( bldump -s|--start-address )
	mu_run_test(t_opt_verbose);        //options_load( bldump -v|--verbose )
	mu_run_test(t_opt_delimitter);     //options_load( bldump -d|--delimitter )
	mu_run_test(t_opt_dec);            //options_load( bldump -i|--decimal )
	mu_run_test(t_opt_udec);           //options_load( bldump -u|--unsigned)
	mu_run_test(t_opt_bin);            //options_load( bldump -b|--binary)
	mu_run_test(t_opt_end);            //options_load( bldump -e|--end-address)
	mu_run_test(t_opt_reorder);        //options_load( bldump -r|--reorder)
	mu_run_test(t_opt_search);         //options_load( bldump -S|--search)

	/* cleanup */
	fclose( t_stdin  );
	fclose( t_stdout );
	fclose( t_stderr );
	t_stdin = NULL;
	t_stdout = NULL;
	t_stderr = NULL;
	verbose_out = NULL;
}

