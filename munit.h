#ifndef __munit_h__
#define __munit_h__
#include <stdio.h>
#include <string.h>
#define mu_failed(file,line,expr) printf( "%s:%u: failed assertion `%s'\n",file,line,expr)
#define mu_tested(test,passed) printf( "Test: %-25s ... %s\n",test,(passed)?"passed":"FAILED")
#define mu_assert(expr) do{mu_nassert++;if(!(expr)){++mu_nfail;mu_failed(__FILE__,__LINE__,#expr);}}while(0)
#define mu_run_test(test) do{int f=mu_nfail;++mu_ntest;test();mu_tested(#test,(f==mu_nfail));}while(0)
#define mu_show_failures() do{printf("### Failed %d of %d asserts (%d tests).\n",mu_nfail,mu_nassert,mu_ntest);}while(0)
extern int mu_nfail;
extern int mu_ntest;
extern int mu_nassert;
#define mu_assert_equal(a,b) mu_assert(a==b)
#define mu_assert_not_equal(a,b) mu_assert(a!=b)
#define mu_assert_ptr_null(a) mu_assert(a==NULL)
#define mu_assert_ptr_not_null(a) mu_assert(a!=NULL)
#define mu_assert_nstring_equal(a,b,n) mu_assert(strncmp((const char*)a,(const char*)b,n)==0)
#define mu_assert_string_equal(a,b) mu_assert(strcmp((const char*)a,(const char*)b)==0)
#endif /* __munit_h__ */
