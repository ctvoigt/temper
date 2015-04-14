/*
 * Standalone temperature logger
 *
 */

#include <stdio.h>
#include <time.h>
#include "argtable2.h"
#include "pcsensor.h"

int main(int argc, char **argv){
    /* Define the allowable command line options, collecting them in argtable[] */
    struct arg_lit *vers  = arg_lit0(NULL,"version",    "print version information and exit");
    struct arg_lit *help  = arg_lit0(NULL,"help",       "print this help and exit");
    struct arg_end *end   = arg_end(20);
    void* argtable[] = {help,vers,end};
    const char* progname = "temper";
    int nerrors_argpars;


    /* verify the argtable[] entries were allocated sucessfully */
    if (arg_nullcheck(argtable) != 0) {
        /* NULL entries were detected, some allocations must have failed */
        printf("%s: insufficient memory\n",progname);
        return 101;
    }

    /* Parse the command line as defined by argtable[] */
    nerrors_argpars = arg_parse(argc,argv,argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout,argtable,"\n");
        printf("Read Data from TEMPer sensors.\n\n");
        arg_print_glossary(stdout,argtable,"  %-10s %s\n");
        printf("\n Without params all TEMPer sensors are read.\n");
        return 0;
        }


    /* special case: '--version' takes precedence error reporting */
    if (vers->count > 0) {
        printf("TEMPer\n");
        return 0;
        }

    /* If the parser returned any errors then display them and exit */
    if (nerrors_argpars > 0) {
        /* Display the error details contained in the arg_end struct.*/
        //arg_print_errors(stdout,end,progname);
        //printf("Try '%s --help' for more information.\n",progname);
        //return 100;
        }

/*	
TODO
Include Code here
	1. Read Commandline parameters
	2. Help Text
	3. Pass paramters run_sensor
Further TODO
	Implement different output formats, alongside CSV a better human readable version.
	Implement selection of a special device.
*/
    return read_temper();
}
