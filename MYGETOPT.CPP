/*--- MYGETOPT.CPP - code to implement UNIX getopt,
 * used to parse command lines from argc and argv.
 */
/*LINTLIBRARY*/
#include <string.h>
#ifdef MSDOS
#include <io.h>
#endif

#ifndef NULL
#define NULL    0
#endif
#define EOF     (-1)

#define GETOPTERR(s,c)

int     myopterr = 1;
int     myoptind = 1;
int     myoptopt;
char    *myoptarg;

int
mygetopt(int argc,char **argv, char *opts)
{
        static int sp = 1;
        register int c;
        register char *cp;

        if(sp == 1)
                if(myoptind >= argc || (argv[myoptind][0] != '/' &&
                   argv[myoptind][0] != '-') || argv[myoptind][1] == '\0')
                        return(EOF);
                else if(strcmp(argv[myoptind], "--") == 0) {
                        myoptind++;
                        return(EOF);
                }
        myoptopt = c = argv[myoptind][sp];
        if(c == ':' || (cp=strchr(opts, c)) == NULL) {
                GETOPTERR(": illegal option -- ", c);
                if(argv[myoptind][++sp] == '\0') {
                        myoptind++;
                        sp = 1;
                }
                return('?');
        }
        if(*++cp == ':') {
                if(argv[myoptind][sp+1] != '\0')
                        myoptarg = &argv[myoptind++][sp+1];
                else if(++myoptind >= argc) {
                        GETOPTERR(": option requires an argument -- ", c);
                        sp = 1;
                        return('?');
                } else
                        myoptarg = argv[myoptind++];
                sp = 1;
        } else {
                if(argv[myoptind][++sp] == '\0') {
                        sp = 1;
                        myoptind++;
                }
                myoptarg = NULL;
        }
        return(c);
}
