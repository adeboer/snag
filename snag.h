/* snag.h */

#define cfile "/etc/snag.conf"

extern char *sword[];
extern int lineno;

int snagdf();
int snaginfo();

/* thresher returns a Nagios result, 0=OK, 1=WARNING, 2=CRITICAL, 3=UNDEFINED
 * based on the thing "s" having the specified value.
 */
int thresher(char *s, long val);

/* gthresher returns a bitmask used internally by snag, where the 1 bit
 * means a WARNING condition is true, the 2 bit means a CRITICAL condition
 * is true, and the 4 bit means an UNDEFINED condition is true.  All bits
 * clear means we don't know of anything wrong at the moment.  Arguments
 * are the same as thresher above.  Internal-format bitmasks can be OR-ed
 * together and ultimately normalized by gnormalize.  The highest bit set
 * is trump.
 */
int gthresher(char *s, long val);

/* normalize internal format to Nagios external format */
int gnormalize(int c);

/* initialize an internal hash table */
void hinit(int showdefaults);

/* add a rule to the internal hash table */
void hashadd(char *s, long lcrit, long lwarn, long hwarn, long hcrit);

/* open the config file, return 1 if ok, 0 if no config file */
int openconfig();

/* close the config file, especially pre-exec */
void closeconfig();

/* set up for external commands */
void setupcmd();

/* set up an external command */
void startcmd(char *name, char *cmdline);

/* clean up external commands */
void cleancmd();
