/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/* Portable interfaces to OS-specific functions
 *
 * This file defines interfaces to certain functions that must be called
 * from portable code, but which must have system-specific implementations.
 */

#ifndef GLK_TADS_TADS2_OS
#define GLK_TADS_TADS2_OS

#include "common/system.h"
#include "glk/tads/osfrobtads.h"
#include "glk/tads/tads2/lib.h"
#include "glk/tads/tads2/appctx.h"

namespace Glk {
namespace TADS {
namespace TADS2 {


/* ------------------------------------------------------------------------ */
/*
 *   A note on character sets:
 *   
 *   Except where noted, all character strings passed to and from the
 *   osxxx functions defined herein use the local operating system
 *   representation.  On a Windows machine localized to Eastern Europe,
 *   for example, the character strings passed to and from the osxxx
 *   functions would use single-byte characters in the Windows code page
 *   1250 representation.
 *   
 *   Callers that use multiple character sets must implement mappings to
 *   and from the local character set when calling the osxxx functions.
 *   The osxxx implementations are thus free to ignore any issues related
 *   to character set conversion or mapping.
 *   
 *   The osxxx implementations are specifically not permitted to use
 *   double-byte Unicode as the native character set, nor any other
 *   character set where a null byte could appear as part of a non-null
 *   character.  In particular, callers may assume that null-terminated
 *   strings passed to and from the osxxx functions contain no embedded
 *   null bytes.  Multi-byte character sets (i.e., character sets with
 *   mixed single-byte and double-byte characters) may be used as long as
 *   a null byte is never part of any multi-byte character, since this
 *   would guarantee that a null byte could always be taken as a null
 *   character without knowledge of the encoding or context.  
 */

/* ------------------------------------------------------------------------ */
/*
 *   "Far" Pointers.  Most platforms can ignore this.  For platforms with
 *   mixed-mode addressing models, where pointers of different sizes can
 *   be used within a single program and hence some pointers require
 *   qualification to indicate that they use a non-default addressing
 *   model, the keyword OSFAR should be defined to the appropriate
 *   compiler-specific extension keyword.
 *   
 *   If you don't know what I'm talking about here, you should just ignore
 *   it, because your platform probably doesn't have anything this
 *   sinister.  As of this writing, this applies only to MS-DOS, and then
 *   only to 16-bit implementations that must interact with other 16-bit
 *   programs via dynamic linking or other mechanisms.  
 */


/* ------------------------------------------------------------------------ */
/*
 *   ANSI C99 exact-size integer types.
 *   
 *   C99 defines a set of integer types with exact bit sizes, named intXX_t
 *   for a signed integer with XX bits, and uintXX_t for unsigned.  XX can be
 *   8, 16, 32, or 64.  TADS uses the 16- and 32-bit sizes, so each platform
 *   is responsible for defining the following types:
 *   
 *.    int16_t   - a signed integer type storing EXACTLY 16 bits
 *.    uint16_t  - an unsigned integer type storing EXACTLY 16 bits
 *.    int32_t   - a signed integer type storing EXACTLY 32 bits
 *.    uint32_t  - an unsigned integer type storing EXACTLY 32 bits
 *   
 *   Many modern compilers provide definitions for these types via the
 *   standard header stdint.h.  Where stdint.h is provided, the platform code
 *   can merely #include <stdint.h>.
 *   
 *   For compilers where stdint.h isn't available, you must provide suitable
 *   typedefs.  Note that the types must be defined with the exact bit sizes
 *   specified; it's not sufficient to use a bigger type, because we depend
 *   in some cases on overflow and sign extension behavior at the specific
 *   bit size.
 */


/* ------------------------------------------------------------------------ */
/*
 *   Thread-local storage (TLS).
 *   
 *   When TADS is compiled with threading support, it requires some variables
 *   to be "thread-local".  This means that the variables have global scope
 *   (so they're not stored in "auto" variables on the stack), but each
 *   thread has a private copy of each such variable.
 *   
 *   Nearly all systems that support threads also support thread-local
 *   storage.  Like threading support itself, though, TLS support is at
 *   present implemented only in non-portable OS APIs rather than standard C
 *   language features.  TLS is a requirement if TADS is compiled with
 *   threading, but it's not needed for non-threaded builds.  TADS only
 *   requires threading at present (version 3.1) for its network features;
 *   since these features are optional, systems that don't have threading and
 *   TLS support will simply need to disable the network features, which will
 *   allow all of the threading and TLS definitions in osifc to be omitted.
 *   
 *   There appear to be two common styles of TLS programming models.  The
 *   first provides non-standard compiler syntax for declarative creation of
 *   thread-local variables.  The Microsoft (on Windows) and Gnu compilers
 *   (on Linux and Unix) do this: they provide custom storage class modifiers
 *   for declaring thread locals (__declspec(thread) for MSVC, __thread for
 *   gcc).  Compilers that support declarative thread locals handle the
 *   implementation details through code generation, so the program merely
 *   needs to add the special TLS storage class qualifier to an otherwise
 *   ordinary global variable declaration, and then can access the thread
 *   local as though it were an ordinary global.
 *   
 *   The second programming model is via explicit OS API calls to create,
 *   initialize, and access thread locals.  pthreads provides such an API, as
 *   does Win32.  In fact, when you use the declarative syntax with MSVC or
 *   gcc, the compiler generates the appropriate API calls, but the details
 *   are transparent to the program; in contrast, when using pthreads
 *   directly, the program must actively call the relevant APIs.
 *   
 *   It's probably the case that every system that has compiler-level support
 *   for declarative thread local creation also has procedural APIs, so the
 *   simplest way to abstract the platform differences would be to do
 *   everything in terms of APIs.  However, it seems likely that compilers
 *   with declarative syntax might be able to generate more efficient code,
 *   since optimizers always benefit from declarative information.  So we'd
 *   like to use declarative syntax whenever it's available, but fall back on
 *   explicit API calls when it's not.  So our programming model is a union
 *   of the two styles:
 *   
 *   1. For each thread local, declare the thread local:
 *.      OS_DECL_TLS(char *, my_local);
 *   
 *   2. At main program startup (for the main thread only), initialize each
 *   thread local:
 *.      os_tls_create(my_local);
 *   
 *   3. Never get or set the value of a thread local directly; instead, use
 *   the get/set functions:
 *.      char *x = os_tls_get(char *, my_local);
 *.      os_tls_set(my_local, "hello");
 *   
 *   One key feature of this implementation is that each thread local is
 *   stored as a (void *) value.  We do it this way to allow a simple direct
 *   mapping to the pthreads APIs, since that's going to be the most common
 *   non-declarative implementation.  This means that a thread local variable
 *   can contain any pointer type, but *only* a pointer type.  The standard
 *   pattern for dealing with anything more ocmplex is the same as in
 *   pthreads: gather up the data into a structure, malloc() an instance of
 *   that structure at entry to each thread (including the main thread), and
 *   os_tls_set() the variable to contain a pointer to that structure.  From
 *   then on, use os_tls_set(my_struct *, my_local)->member to access the
 *   member variables in the structure.  And finally, each thread must delete
 *   the structure at thread exit.
 */

/*   
 *   
 *   Declare a thread local.
 *   
 *   - For compilers that support declarative TLS variables, the local OS
 *   headers should use the compiler support by #defining OS_DECL_TLS to the
 *   appropriate local declarative keyword.
 *   
 *   - For systems without declarative TLS support but with TLS APIs, the
 *   global declared by this macro actually stores the slot ID (what pthreads
 *   calls the "key") for the variable.  This macro should therefore expand
 *   to a declaration of the appropriate API type for a slot ID; for example,
 *   on pthreads, #define OS_DECL_TLS(t, v) pthread_key_t v.
 *   
 *   - For builds with no thread support, simply #define this to declare the
 *   variable as an ordinary global: #define OS_DECL_TLS(t, v) t v.
 */
/* #define OS_DECL_TLS(typ, varname)  __thread typ varname */

/*
 *   For API-based systems without declarative support in the compiler, the
 *   main program startup code must explicitly create a slot for each thread-
 *   local variable by calling os_tls_create().  The API returns a slot ID,
 *   which is shared among threads and therefore can be stored in an ordinary
 *   global variable.  OS_DECL_TLS will have declared the global variable
 *   name in this case as an ordinary global of the slot ID type.  The
 *   os_tls_create() macro should therefore expand to a call to the slot
 *   creation API, storing the new slot ID in the global.
 *   
 *   Correspondingly, before the main thread exits, it should delete each
 *   slot it created, b calling os_tls_delete().
 *   
 *   For declarative systems, there's no action required here, so these
 *   macros can be defined to empty.
 */
/* #define os_tls_create(varname)  pthread_key_create(&varname, NULL) */
/* #define os_tls_delete(varname)  pthread_key_delete(varname) */


/*
 *   On API-based systems, each access to get or set the thread local
 *   requires an API call, using the slot ID stored in the actual global to
 *   get the per-thread instance of the variable's storage.
 *.    #define os_tls_get(typ, varname) ((typ)pthread_getspecific(varname))
 *.    #define os_tls_set(varname, val) pthread_setspecific(varname, val)
 *   
 *   On declarative systems, the global variable itself is the thread local,
 *   so get/set can be implemented as direct access to the variable.
 *.    #define os_tls_get(typ, varname) varname
 *.    #define os_tls_set(varname, val) varname = (val)
 */

/*
 *   Common TLS definitions - declarative thread locals
 *   
 *   For systems with declarative TLS support in the compiler, the OS header
 *   can #define OS_DECLARATIVE_TLS to pick up suitable definitions for the
 *   os_tls_xxx() macros.  The OS header must separately define OS_DECL_TLS
 *   as appropriate for the local system.
 */
#ifdef OS_DECLARATIVE_TLS
#define os_tls_create(varname)
#define os_tls_delete(varname)
#define os_tls_get(typ, varname) varname
#define os_tls_set(varname, val) varname = (val)
#endif

/*
 *   Common TLS definitions - pthreads
 *   
 *   For pthreads systems without declarative TLS support in the compiler,
 *   the OS header can simply #define OS_PTHREAD_TLS to pick up the standard
 *   definitions below. 
 */
#ifdef OS_PTHREAD_TLS
#include <pthread.h>
#define OS_DECL_TLS(typ, varname) pthread_key_t varname
#define os_tls_create(varname) pthread_key_create(&varname, NULL)
#define os_tls_delete(varname) pthread_key_delete(varname)
#define os_tls_get(typ, varname) ((typ)pthread_getspecific(varname))
#define os_tls_set(varname, val) pthread_setspecific(varname, val)
#endif


/* ------------------------------------------------------------------------ */
/*
 *   <time.h> definitions.
 *   
 *   os_time() should act like Unix time(), returning the number of seconds
 *   elapsed since January 1, 1970 at midnight UTC.
 *   
 *   The original Unix <time.h> package defined time_t as a 32-bit signed
 *   int, and many subsequent C compilers on other platforms followed suit.
 *   A signed 32-bit time_t has the well-known year-2038 problem; some later
 *   C compilers tried to improve matters by using an unsigned 32-bit time_t
 *   instead, but for many purposes this is even worse since it can't
 *   represent any date before 1/1/1970.  *Most* modern compilers solve the
 *   problem once and for all (for 300 billion years in either direction of
 *   1/1/1970, anyway - enough to represent literally all of eternity in most
 *   current cosmological models) by defining time_t as a signed 64-bit int.
 *   But some compilers stubbornly stick to the old 32-bit time_t even in
 *   newer versions, for the sake of compatibility with older code that might
 *   be lax about mixing time_t's with ordinary int's.  E.g., MSVC2003 does
 *   this.  Fortunately, some of these compilers (such as MSVC2003 again)
 *   also define a parallel, transitional set of 64-bit time functions that
 *   you can use by replacing all references to the standard time_t and
 *   related names with the corresponding 64-bit names.
 *   
 *   We'd really like to use a 64-bit time_t wherever we can - the TADS
 *   release cycle can be a bit slow, and we don't want 2038 to sneak up on
 *   us and catch us unawares.  So for those compilers that offer a choice of
 *   32 or 64 bits, we'd like to select the 64 bit version.  To facilitate
 *   this, we define covers here for the time.h types and functions that we
 *   use.  On platforms where the regular time_t is already 64 bits, or where
 *   there's no 64-bit option at all, you can simply do nothing - the
 *   defaults defined here use the standard time_t typedef and functions, so
 *   that's what you'll get if you don't define these in the OS-specific
 *   headers for your platform.  For compilers that provide both a 32-bit
 *   time_t and a 64-bit other_time_t, the OS headers should #define these
 *   macros in terms of those compiler-specific 64-bit names.
 */
#ifndef os_time_t
# define os_time_t        TimeDate
# define os_gmtime(t)     gmtime(t)
# define os_localtime(t)  localtime(t)
# define os_time(t)       time(t)
#endif

/*
 *   Initialize the time zone.  This routine is meant to take care of any
 *   work that needs to be done prior to calling localtime() and other
 *   time-zone-dependent routines in the run-time library.  For DOS and
 *   Windows, we need to call the run-time library routine tzset() to set up
 *   the time zone from the environment; most systems shouldn't need to do
 *   anything in this routine.  It's sufficient to call this once during the
 *   process lifetime, since it's meant to perform static initialization that
 *   lasts as long as the process is running.
 */
#ifndef os_tzset
void os_tzset(void);
#endif

/*
 *   Higher-precision time.  This retrieves the same time information as
 *   os_time() (i.e., the elapsed time since the standard Unix Epoch, January
 *   1, 1970 at midnight UTC), but retrieves it with the highest precision
 *   available on the local system, up to nanosecond precision.  If less
 *   precision is available, that's fine; just return the time to the best
 *   precision available, but expressed in terms of the number of
 *   nanoseconds.  For example, if you can retrieve milliseconds, you can
 *   convert that to nanoseconds by multiplying by 1,000,000.
 *   
 *   On return, fills in '*seconds' with the number of whole seconds since
 *   the Epoch, and fills in '*nanoseconds' with the fractional portion,
 *   expressed in nanosceconds.  Note that '*nanoseconds' is merely the
 *   fractional portion of the time, so 0 <= *nanoseconds < 1000000000.
 */
void os_time_ns(os_time_t *seconds, long *nanoseconds);

/*
 *   Get the local time zone name, as a location name in the IANA zoneinfo
 *   database.  For example, locations using US Pacific Time should return
 *   "America/Los_Angeles".
 *   
 *   Returns true if successful, false if not.  If the local operating system
 *   doesn't have a way to obtain this information, or if it's not available
 *   in the local machine's configuration, this returns false.
 *   
 *   The zoneinfo database is also known as the Olson or TZ (timezone)
 *   database; it's widely used on Unix systems as the definitive source of
 *   local time zone settings.  See http://www.iana.org/time-zones for more
 *   information.
 *   
 *   On many Unix systems, the TZ environment variable contains the zoneinfo
 *   zone name when its first character is ':'.  Windows uses a proprietary
 *   list of time zone names that can be mapped to zoneinfo names via a
 *   hand-coded list (such a list is maintained in the Unicode CLDR; our
 *   Windows implementation uses the CLDR list to generate the mapping).
 *   MacOS X uses zoneinfo keys directly; /etc/localtime is a link to the
 *   zoneinfo file for the local zone as set via the system preferences.
 *   
 *   os_tzset() must be invoked at some point before this routine is called.
 */
int os_get_zoneinfo_key(char *buf, size_t buflen);

/*
 *   Get a description of the local time zone.  Fills in '*info' with the
 *   available information.  Returns true on success, false on failure.
 *   
 *   See osstzprs.h/.c for a portable implementation of a parser for
 *   POSIX-style TZ strings.  That can serve as a full implementation of this
 *   function for systems that use the POSIX TZ environment variable syntax
 *   to specify the timezone.  (That routine simply parses a string from any
 *   source, so it can be used to parse the TZ syntax even on systems where
 *   the string comes from somewhere other than the TZ environment variable.)
 *   
 *   os_tzset() must be invoked at some point before this routine is called.
 *   
 *   The following two structures are used for the return information:
 *   
 *   os_tzrule_t - Timezone Rule structure.  This describes a rule for an
 *   annual transition between daylight savings time and standard time in a
 *   time zone.  Most timezones that have recurring standard/daylight changes
 *   require two of these rules, one for switching to daylight time in the
 *   spring and one for switching to standard time in the fall.
 *   
 *   os_tzinfo_t - Timezone Information structure.  This describes a
 *   timezone's clock settings, name(s), and rules for recurring annual
 *   changes between standard time and daylight time, if applicable.
 */
struct os_tzrule_t
{
    /* 
     *   Day of year, 1-365, NEVER counting Feb 29; set to 0 if not used.
     *   Corresponds to the "J" format in Unix TZ strings.  (Called "Julian
     *   day" in the POSIX docs, thus the "J", even though it's a bit of a
     *   misnomer.)(Because of the invariance of the mapping from J-number to
     *   date, this is just an obtuse way of specifying a month/day date.
     *   But even so, we'll let the OS layer relay this back to us in
     *   J-number format and count on the portable caller to work out the
     *   date, rather than foisting that work on each platform
     *   implementation.)
     */
    int jday;

    /*
     *   Day of year, 1-366, counting Feb 29 on leap years; set to 0 if not
     *   used; ignored if 'jday' is nonzero.  This corresponds to the Julian
     *   day sans "J" in TZ strings (almost - that TZ format uses 0-365 as
     *   its range, so bump it up by one when parsing a TZ string).  This
     *   format is even more obtuse than the J-day format, in that it doesn't
     *   even have an invariant month/day mapping (not after day 59, anyway -
     *   day 60 is either February 29 or March 1, depending on the leapness
     *   of the year, and every day after that is similarly conditional).  As
     *   far as I can tell, no one uses this option, so I'm not sure why it
     *   exists.  The zoneinfo source format doesn't have a way to represent
     *   it, which says to me that no one has ever used it in a statutory DST
     *   start/end date definition in the whole history of time zones around
     *   the world, since the whole history of time zones around the world is
     *   exactly what the zoneinfo database captures in exhaustive and
     *   painstaking detail.  If anyone had ever used it in defining a time
     *   zone, zoneinfo would have an option for it.  My guess is that it's a
     *   fossilized bug from some early C RTL that's been retained out of an
     *   abundance of caution vis-a-vis compatibility, and was entirely
     *   replaced in practice by the J-number format as soon as someone
     *   noticed the fiddly leap year behavior.  But for the sake of
     *   completeness...
     */
    int yday;
    
    /* 
     *   The month (1-12), week of the month, and day of the week (1-7 for
     *   Sunday to Saturday).  Week 1 is the first week in which 'day'
     *   occurs, week 2 is the second, etc.; week 5 is the last occurrence of
     *   'day' in the month.  These fields are used for "second Sunday in
     *   March" types of rules.  Set these to zero if they're not used;
     *   they're ignored in any case if 'jday' or 'yday' are non-zero.
     */
    int month;
    int week;
    int day;

    /* time of day, in seconds after midnight (e.g., 2AM is 120 == 2*60*60) */
    int time;
};
struct os_tzinfo_t
{
    /*
     *   The local offset from GMT, in seconds, for standard time and
     *   daylight time in this zone.  These values are positive for zones
     *   east of GMT and negative for zones west: New York standard time
     *   (EST) is 5 hours west of GMT, so its offset is -5*60*60.
     *   
     *   Set both of these fields (if possible) regardless of whether
     *   standard or daylight time is currently in effect in the zone.  The
     *   caller will select which offset to use based on the start/end rules,
     *   or based on the 'is_dst' flag if no rules are available.
     *   
     *   If it's only possible to determine the current wall clock offset, be
     *   it standard or daylight time, and it's not possible to determine the
     *   time difference between the two, simply set both of these to the
     *   current offset.  This information isn't available from the standard
     *   C library, and many OS APIs also lack it.  
     */
    int32_t std_ofs;
    int32_t dst_ofs;

    /*
     *   The abbreviations for the local zone's standard time and daylight
     *   time, respectively, when displaying date/time values.  E.g., "EST"
     *   and "EDT" for US Eastern Time.  If the zone doesn't observe daylight
     *   time (it's on standard time year round), set dst_abbr to an empty
     *   string.
     *   
     *   As with std_ofs and dst_ofs, you can set both of these to the same
     *   string if it's only possible to determine the one that's currently
     *   in effect.
     */
    char std_abbr[16];
    char dst_abbr[16];

    /*
     *   The ongoing rules for switching between daylight and standard time
     *   in this zone, if available.  'dst_start' is the date when daylight
     *   savings starts, 'dst_end' is the date when standard time resumes.
     *   Set all fields to 0 if the start/stop dates aren't available, or the
     *   zone is on standard time year round.
     */
    struct os_tzrule_t dst_start;
    struct os_tzrule_t dst_end;

    /* 
     *   True -> the zone is CURRENTLY on daylight savings time; false means
     *   it's currently on standard time.
     *   
     *   This is only used if the start/end rules aren't specified.  In the
     *   absence of start/end rules, there's no way to know when the current
     *   standard/daylight phase ends, so we'll have to assume that the
     *   current mode is in effect permanently.  In this case, the caller
     *   will use only be able to use the offset and abbreviation for the
     *   current mode and will have to ignore the other one.
     */
    int is_dst;
};
int os_get_timezone_info(struct os_tzinfo_t *info);


/*
 *   Get the current system high-precision timer.  This function returns a
 *   value giving the wall-clock ("real") time in milliseconds, relative to
 *   any arbitrary zero point.  It doesn't matter what this value is relative
 *   to -- the only important thing is that the values returned by two
 *   different calls should differ by the number of actual milliseconds that
 *   have elapsed between the two calls.  This might be the number of
 *   milliseconds since the computer was booted, since the current user
 *   logged in, since midnight of the previous night, since the program
 *   started running, since 1-1-1970, etc - it doesn't matter what the epoch
 *   is, so the implementation can use whatever's convenient on the local
 *   system.
 *   
 *   True millisecond precision isn't required.  Each implementation should
 *   simply use the best precision available on the system.  If your system
 *   doesn't have any kind of high-precision clock, you can simply use the
 *   time() function and multiply the result by 1000 (but see the note below
 *   about exceeding 32-bit precision).
 *   
 *   However, it *is* required that the return value be in *units* of
 *   milliseconds, even if your system clock doesn't have that much
 *   precision; so on a system that uses its own internal clock units, this
 *   routine must multiply the clock units by the appropriate factor to yield
 *   milliseconds for the return value.
 *   
 *   It is also required that the values returned by this function be
 *   monotonically increasing.  In other words, each subsequent call must
 *   return a value that is equal to or greater than the value returned from
 *   the last call.  On some systems, you must be careful of two special
 *   situations.
 *   
 *   First, the system clock may "roll over" to zero at some point; for
 *   example, on some systems, the internal clock is reset to zero at
 *   midnight every night.  If this happens, you should make sure that you
 *   apply a bias after a roll-over to make sure that the value returned from
 *   this return continues to increase despite the reset of the system clock.
 *   
 *   Second, a 32-bit signed number can only hold about twenty-three days
 *   worth of milliseconds.  While it seems unlikely that a TADS game would
 *   run for 23 days without a break, it's certainly reasonable to expect
 *   that the computer itself may run this long without being rebooted.  So,
 *   if your system uses some large type (a 64-bit number, for example) for
 *   its high-precision timer, you may want to store a zero point the very
 *   first time this function is called, and then always subtract this zero
 *   point from the large value returned by the system clock.  If you're
 *   using time(0)*1000, you should use this technique, since the result of
 *   time(0)*1000 will almost certainly not fit in 32 bits in most cases.  
 */
long os_get_sys_clock_ms(void);


/* ------------------------------------------------------------------------ */
/*
 *   Hardware Configuration.  Define the following functions appropriately
 *   for your hardware.  For efficiency, these functions should be defined
 *   as macros if possible.
 *   
 *   Note that these hardware definitions are independent of the OS, at
 *   least to the extent that your OS can run on multiple types of
 *   hardware.  So, rather than combining these definitions into your
 *   osxxx.h header file, we recommend that you put these definitions in a
 *   separate h_yyy.h header file, which can be configured into os.h with
 *   an appropriate "_M_yyy" preprocessor symbol.  Refer to os.h for
 *   details of configuring the hardware include file.  
 */

/* 
 *   Round a size up to worst-case alignment boundary.  For example, on a
 *   platform where the largest type must be aligned on a 4-byte boundary,
 *   this should round the value up to the next higher mutliple of 4 and
 *   return the result.  
 */
/* size_t osrndsz(size_t siz); */

/* 
 *   Round a pointer up to worst-case alignment boundary. 
 */
/* void *osrndpt(void *ptr); */

/* 
 *   Read an unaligned portable unsigned 2-byte value, returning an int
 *   value.  The portable representation has the least significant byte
 *   first, so the value 0x1234 is represented as the byte 0x34, followed
 *   by the byte 0x12.
 *   
 *   The source value must be treated as unsigned, but the result is
 *   signed.  This is significant on 32- and 64-bit platforms, because it
 *   means that the source value should never be sign-extended to 32-bits.
 *   For example, if the source value is 0xffff, the result is 65535, not
 *   -1.  
 */
/* int osrp2(unsigned char *p); */

/* 
 *   Read an unaligned portable signed 2-byte value, returning int.  This
 *   differs from osrp2() in that this function treats the source value as
 *   signed, and returns a signed result; hence, on 32- and 64-bit
 *   platforms, the result must be sign-extended to the int size.  For
 *   example, if the source value is 0xffff, the result is -1.  
 */
/* int osrp2s(unsigned char *p); */

/* 
 *   Write unsigned int to unaligned portable 2-byte value.  The portable
 *   representation stores the low-order byte first in memory, so
 *   oswp2(0x1234) should result in storing a byte value 0x34 in the first
 *   byte, and 0x12 in the second byte.  
 */
/* void oswp2(unsigned char *p, unsigned int i); */

/*
 *   Write signed int to unaligned portable 2-byte value.  Negative values
 *   must be stored in two's complement notation.  E.g., -1 is stored as
 *   FF.FF, -32768 is stored as 00.80 (little-endian).  
 *   
 *   Virtually all modern hardware uses two's complement notation as the
 *   native representation, which makes this routine a trivial synonym of
 *   osrp2() (i.e., #define oswp2s(p,i) oswp2(p,i)).  We distinguish the
 *   signed version on the extremely off chance that TADS is ever ported to
 *   wacky hardware with a different representation for negative integers
 *   (one's complement, sign bit, etc).  
 */
/* void oswp2s(unsigned char *p, int i); */

/* 
 *   Read an unaligned unsigned portable 4-byte value, returning long.  The
 *   underlying value should be considered signed, and the result is signed.
 *   The portable representation stores the bytes starting with the least
 *   significant: the value 0x12345678 is stored with 0x78 in the first byte,
 *   0x56 in the second byte, 0x34 in the third byte, and 0x12 in the fourth
 *   byte.
 */
/* unsigned long osrp4(unsigned char *p); */

/*
 *   Read an unaligned signed portable 4-byte value, returning long. 
 */
/* long osrp4s(unsigned char *p); */

/* 
 *   Write an unsigned long to an unaligned portable 4-byte value.  The
 *   portable representation stores the low-order byte first in memory, so
 *   0x12345678 is written to memory as 0x78, 0x56, 0x34, 0x12.  
 */
/* void oswp4(unsigned char *p, unsigned long l); */

/*
 *   Write a signed long, using little-endian byte order and two's complement
 *   notation for negative numbers.  This is a trivial synonym for oswp4()
 *   for all platforms with native two's complement arithmetic (which is
 *   virtually all modern platforms).  See oswp2s() for more discussion.
 */
/* void oswp4s(unsigned char *p, long l); */

/*
 *   For convenience and readability, the 1-byte integer (signed and
 *   unsigned) equivalents of the above.
 */
#define osrp1(p) (*(unsigned char *)(p))
#define osrp1s(p) (*(signed char *)(p))
#define oswp1(p, b) (*(unsigned char *)(p) = (b))
#define oswp1s(p, b) (*(signed char *)(p) = (b))


/* ------------------------------------------------------------------------ */
/*
 *   varargs va_copy() extension.
 *   
 *   On some compilers, va_list is a reference type.  This means that if a
 *   va_list value is passed to a function that uses va_arg() to step through
 *   the referenced arguments, the caller's copy of the va_list might be
 *   updated on return.  This is problematic in cases where the caller needs
 *   to use the va_list again in another function call, since the va_list is
 *   no longer pointing to the first argument for the second call.  C99 has a
 *   solution in the form of the va_copy() macro.  Unfortunately, this isn't
 *   typically available in pre-C99 compilers, and isn't standard in *any*
 *   C++ version.  We thus virtualize it here in a macro.
 *   
 *   os_va_copy() has identical semantics to C99 va_copy().  A matching call
 *   to os_va_copy_end() must be made for each call to os_va_copy() before
 *   the calling function returns; this has identical semantics to C99
 *   va_end().
 *   
 *   Because our semantics are identical to the C99 version, we provide a
 *   default definition here for compilers that define va_copy().  Platform
 *   headers must provide suitable definitions only if their compilers don't
 *   have va_copy().  We also provide a definition for GCC compilers that
 *   define the private __va_copy macro, which also has the same semantics.
 */
#ifdef va_copy
# define os_va_copy(dst, src) va_copy(dst, src)
# define os_va_copy_end(dst)  va_end(dst)
#else
# if defined(__GNUC__) && defined(__va_copy)
#  define os_va_copy(dst, src) __va_copy(dst, src)
#  define os_va_copy_end(dst)  va_end(dst)
# endif
#endif



/* ------------------------------------------------------------------------ */
/*
 *   Platform Identifiers.  You must define the following macros in your
 *   osxxx.h header file:
 *   
 *   OS_SYSTEM_NAME - a string giving the system identifier.  This string
 *   must contain only characters that are valid in a TADS identifier:
 *   letters, numbers, and underscores; and must start with a letter or
 *   underscore.  For example, on MS-DOS, this string is "MSDOS".
 *   
 *   OS_SYSTEM_LDESC - a string giving the system descriptive name.  This
 *   is used in messages displayed to the user.  For example, on MS-DOS,
 *   this string is "MS-DOS".  
 */


/* ------------------------------------------------------------------------ */
/*
 *   Message Linking Configuration.  You should #define ERR_LINK_MESSAGES
 *   in your osxxx.h header file if you want error messages linked into
 *   the application.  Leave this symbol undefined if you want an external
 *   message file. 
 */


/* ------------------------------------------------------------------------ */
/*
 *   Program Exit Codes.  These values are used for the argument to exit()
 *   to conform to local conventions.  Define the following values in your
 *   OS-specific header:
 *   
 *   OSEXSUCC - successful completion.  Usually defined to 0.
 *.  OSEXFAIL - failure.  Usually defined to 1.  
 */


/* ------------------------------------------------------------------------ */
/*
 *   Basic memory management interface.  These functions are merely
 *   documented here, but no prototypes are defined, because most
 *   platforms #define macros for these functions and types, mapping them
 *   to malloc or other system interfaces.  
 */

/*
 *   Theoretical maximum osmalloc() size.  This may be less than the
 *   capacity of the argument to osmalloc() on some systems.  For example,
 *   on segmented architectures (such as 16-bit x86), memory is divided into
 *   segments, so a single memory allocation can allocate only a subset of
 *   the total addressable memory in the system.  This value thus specifies
 *   the maximum amount of memory that can be allocated in one chunk.
 *   
 *   Note that this is an architectural maximum for the hardware and
 *   operating system.  It doesn't have anything to do with the total amount
 *   of memory actually available at run-time.
 *   
 *   #define OSMALMAX to a constant long value with theoretical maximum
 *   osmalloc() argument value.  For a platform with a flat (unsegmented)
 *   32-bit memory space, this is usually 0xffffffff; for 16-bit platforms,
 *   this is usually 0xffff.  
 */
/* #define OSMALMAX 0xffffffff */

/*   
 *   Allocate a block of memory of the given size in bytes.  The actual
 *   allocation may be larger, but may be no smaller.  The block returned
 *   should be worst-case aligned (i.e., suitably aligned for any type).
 *   Return null if the given amount of memory is not available.  
 */
/* void *osmalloc(size_t siz); */

/*
 *   Free memory previously allocated with osmalloc().  
 */
/* void osfree(void *block); */

/* 
 *   Reallocate memory previously allocated with osmalloc() or
 *   osrealloc(), changing the block's size to the given number of bytes.
 *   If necessary, a new block at a different address can be allocated, in
 *   which case the data from the original block is copied (the lesser of
 *   the old block size and the new size is copied) to the new block, and
 *   the original block is freed.  If the new size is less than the old
 *   size, this need not do anything at all, since the returned block can
 *   be larger than the new requested size.  If the block cannot be
 *   enlarged to the requested size, return null.  
 */
/* void *osrealloc(void *block, size_t siz); */


/* ------------------------------------------------------------------------ */
/*
 *   Basic file I/O interface.  These functions are merely documented here,
 *   but no prototypes are defined, because most platforms #define macros for
 *   these functions and types, mapping them to stdio or other system I/O
 *   interfaces.  
 *   
 *   When writing a file, writes might or might not be buffered in
 *   application memory; this is up to the OS implementation, which can
 *   perform buffering according to local conventions and what's most
 *   efficient.  However, it shouldn't make any difference to the caller
 *   whether writes are buffered or not - the OS implementation must take
 *   care that any buffering is invisible to the app.  (Porters: note that
 *   the basic C stdio package has the proper behavior here, so you'll get
 *   the correct semantics if you use a simple stdio implementation.)
 *   
 *   Write buffering might be visible to *other* apps, though.  In
 *   particular, another process might not see data written to a file (with
 *   osfwb(), os_fprint(), etc) immediately, since the write functions might
 *   hold the written bytes in an internal memory buffer rather than sending
 *   them to the OS.  Any internal buffers are guaranteed to be flushed to
 *   the OS upon calling osfcls() or osfflush().  Note that it's never
 *   *necessary* to call osfflush(), because buffered data will always be
 *   flushed on closing the file with osfcls().  However, if you want other
 *   apps to be able to see updates immediately, you can use osfflush() to
 *   ensure that buffers are flushed to a file before you close it.
 *   
 *   You can also use osfflush() to check for buffered write errors.  When
 *   you use osfwb() or other write functions to write data, they will return
 *   a success indication even if the data was only copied into a buffer.
 *   This means that a write that appeared to succeed might actually fail
 *   later, when the buffer is flushed.  The only way to know for sure is to
 *   explicitly flush buffers using osfflush(), and check the result code.
 *   If the original write function and a subsequent osfflush() *both* return
 *   success indications, then the write has definitely succeeded.  
 */


/*
 *   Define the following values in your OS header to indicate local
 *   file/path syntax conventions:
 *   
 *   OSFNMAX - integer indicating maximum length of a filename
 *   
 *   OSPATHCHAR - character giving the normal path separator character
 *.  OSPATHALT - string giving other path separator characters
 *.  OSPATHURL - string giving path separator characters for URL conversions
 *.  OSPATHSEP - directory separator for PATH-style environment variables
 *.  OSPATHPWD - string giving the special path representing the current
 *.              working directory; for Unix or Windows, this is "."
 *   
 *   OSPATHURL is a little different: this specifies the characters that
 *   should be converted to URL-style separators when converting a path from
 *   local notation to URL notation.  This is usually the same as the union
 *   of OSPATHCHAR and OSPATHALT, but need not be; for example, on DOS, the
 *   colon (':') is a path separator for most purposes, but is NOT a path
 *   character for URL conversions.
 */

/*
 *   Define the type osfildef as the appropriate file handle structure for
 *   your osfxxx functions.  This type is always used as a pointer, but
 *   the value is always obtained from an osfopxxx call, and is never
 *   synthesized by portable code, so you can use essentially any type
 *   here that you want.
 *   
 *   For platforms that use C stdio functions to implement the osfxxx
 *   functions, osfildef can simply be defined as FILE.
 */
/* typedef FILE osfildef; */


/*
 *   File types.
 *   
 *   These are symbols of the form OSFTxxxx defining various content types,
 *   somewhat aking to MIME types.  These were mainly designed for the old
 *   Mac OS (versions up to OS 9), where the file system stored a type tag
 *   with each file's metadata.  The type tags were used for things like
 *   filtering file selector dialogs and setting file-to-app associations in
 *   the desktop shell.
 *   
 *   Our OSFTxxx symbols are abstract file types that we define, for types
 *   used within the TADS family of applications.  They give us a common,
 *   cross-platform reference point for each type we use.  Each port where
 *   file types are meaningful then maps our abstract type IDs to the
 *   corresponding port-specific type IDs.  In practice, this has never been
 *   used anywhere other than the old Mac OS ports; in fact, it's not even
 *   used in the modern Mac OS (OS X and later), since Apple decided to stop
 *   fighting the tide and start using filename suffixes for this sort of
 *   tagging, like everyone else always has.
 *   
 *   For the list of file types, see osifctyp.h 
 */


/*
 *   Local newline convention.
 *   
 *   Because of the pernicious NIH ("Not Invented Here") cultures of the
 *   major technology vendors, basically every platform out there has its own
 *   unique way of expressing newlines in text files.  Unix uses LF (ASCII
 *   10); Mac uses CR (ASCII 13); DOS and Windows use CR-LF pairs.  In the
 *   past there were heaven-only-knows how many other conventions in use, but
 *   fortunately these three have the market pretty well locked up at this
 *   point.  But we do still have to worry about these three.
 *   
 *   Our strategy on input is to be open to just about anything whenever
 *   possible.  So, when we're reading something that we believe to be a text
 *   file, we'll treat all of these as line endings: CR, LF, CR-LF, and
 *   LF-CR.  It's pretty safe to do this; if we have a CR and LF occurring
 *   adjacently, it's almost certain that they're intended to be taken
 *   together as a single newline sequence.  Likewise, if there's a lone CR
 *   or LF, it's rare for it to mean anything other than a newline.
 *   
 *   On output, though, we can't be as loose.  The problem is that other
 *   applications on our big three platforms *don't* tend to aim for the same
 *   flexibility we do on input: other apps usually expect exactly the local
 *   conventions on input, and don't always work well if they don't get it.
 *   So it's important that when we're writing a text file, we write newlines
 *   in the local convention.  This means that we sometimes need to know what
 *   the local convention actually is.  That's where this definition comes
 *   in.
 *   
 *   Each port must define OS_NEWLINE_SEQ as an ASCII string giving the local
 *   newline sequence to write on output.  For example, DOS defines it as
 *   "\r\n" (CR-LF).  Always define it as a STRING (not a character
 *   constant), even if it's only one character long.
 *   
 *   (Note that some compilers use wacky mappings for \r and \n.  Some older
 *   Mac compilers, for example, defined \n as CR and \r as LF, because of
 *   the Mac convention where newline is represented as CR in a text file.
 *   If there's any such variability on your platform, you can always use the
 *   octal codes to be unambiguous: \012 for LF and \015 for CR.)  
 */
/* #define OS_NEWLINE_SEQ  "\r\n" */



/* 
 *   Open text file for reading.  This opens the file with read-only access;
 *   we're not allowed to write to the file using this handle.  Returns NULL
 *   on error.
 *   
 *   A text file differs from a binary file in that some systems perform
 *   translations to map between C conventions and local file system
 *   conventions; for example, on DOS, the stdio library maps the DOS CR-LF
 *   newline convention to the C-style '\n' newline format.  On many systems
 *   (Unix, for example), there is no distinction between text and binary
 *   files.
 *   
 *   On systems that support file sharing and locking, this should open the
 *   file in "shared read" mode - this means that other processes are allowed
 *   to simultaneously read from the file, but no other processs should be
 *   allowed to write to the file as long as we have it open.  If another
 *   process already has the file open with write access, this routine should
 *   return failure, since we can't take away the write privileges the other
 *   process already has and thus we can't guarantee that other processes
 *   won't write to the file while we have it open.  
 */
/* osfildef *osfoprt(const char *fname, os_filetype_t typ); */

/*
 *   Open a text file for "volatile" reading: we open the file with read-only
 *   access, and we explicitly accept instability in the file's contents due
 *   to other processes simultaneously writing to the file.  On systems that
 *   support file sharing and locking, the file should be opened in "deny
 *   none" mode, meaning that other processes can simultaneously open the
 *   file for reading and/or writing even while have the file open.  
 */
/* osfildef *osfoprtv(const char *fname, os_filetype_t typ); */

/* 
 *   Open text file for writing; returns NULL on error.  If the file already
 *   exists, this truncates the file to zero length, deleting any existing
 *   contents.
 */
/* osfildef *osfopwt(const char *fname, os_filetype_t typ); */

/*
 *   Open text file for reading and writing, keeping the file's existing
 *   contents if the file already exists or creating a new file if no such
 *   file exists.  Returns NULL on error. 
 */
/* osfildef *osfoprwt(const char *fname, os_filetype_t typ); */

/* 
 *   Open text file for reading/writing.  If the file already exists,
 *   truncate the existing contents to zero length.  Create a new file if it
 *   doesn't already exist.  Return null on error.  
 */
/* osfildef *osfoprwtt(const char *fname, os_filetype_t typ); */

/* 
 *   Open binary file for writing; returns NULL on error.  If the file
 *   exists, this truncates the existing contents to zero length.
 */
/* osfildef *osfopwb(const char *fname, os_filetype_t typ); */

/* 
 *   Open source file for reading - use the appropriate text or binary
 *   mode.  
 */
/* osfildef *osfoprs(const char *fname, os_filetype_t typ); */

/* 
 *   Open binary file for reading; returns NULL on error.  
 */
/* osfildef *osfoprb(const char *fname, os_filetype_t typ); */

/*
 *   Open binary file for 'volatile' reading; returns NULL on error.
 *   ("Volatile" means that we'll accept writes from other processes while
 *   reading, so the file should be opened in "deny none" mode or the
 *   equivalent, to the extent that the local system supports file sharing
 *   modes.)  
 */
/* osfildef *osfoprbv(const char *fname, os_filetype_t typ); */

/* 
 *   Open binary file for random-access reading/writing.  If the file already
 *   exists, keep the existing contents; if the file doesn't already exist,
 *   create a new empty file.
 *   
 *   The caller is allowed to perform any mixture of read and write
 *   operations on the returned file handle, and can seek around in the file
 *   to read and write at random locations.
 *   
 *   If the local file system supports file sharing or locking controls, this
 *   should generally open the file in something equivalent to "exclusive
 *   write, shared read" mode ("deny write" in DENY terms), so that other
 *   processes can't modify the file at the same time we're modifying it (but
 *   it doesn't bother us to have other processes reading from the file while
 *   we're working on it, as long as they don't mind that we could change
 *   things on the fly).  It's not absolutely necessary to assert these
 *   locking semantics, but if there's an option to do so this is preferred.
 *   Stricter semantics (such as "exclusive" or "deny all" mode) are better
 *   than less strict semantics.  Less strict semantics are dicey, because in
 *   that case the caller has no way of knowing that another process could be
 *   modifying the file at the same time, and no way (through osifc) of
 *   coordinating that activity.  If less strict semantics are implemented,
 *   the caller will basically be relying on luck to avoid corruptions due to
 *   writing by other processes.
 *   
 *   Return null on error.  
 */
/* osfildef *osfoprwb(const char *fname, os_filetype_t typ); */

/* 
 *   Open binary file for random-access reading/writing.  If the file already
 *   exists, truncate the existing contents (i.e., delete the contents of the
 *   file, resetting it to a zero-length file).  Create a new file if it
 *   doesn't already exist.  The caller is allowed to perform any mixture of
 *   read and write operations on the returned handle, and can seek around in
 *   the file to read and write at random locations.
 *   
 *   The same comments regarding sharing/locking modes for osfoprwb() apply
 *   here as well.
 *   
 *   Return null on error.  
 */
/* osfildef *osfoprwtb(const char *fname, os_filetype_t typ); */

/*
 *   Duplicate a file handle.  Returns a new osfildef* handle that accesses
 *   the same open file as an existing osfildef* handle.  The new handle is
 *   independent of the original handle, with its own seek position,
 *   buffering, etc.  The new handle and the original handle must each be
 *   closed separately when the caller is done with them (closing one doesn't
 *   close the other).  The effect should be roughly the same as the Unix
 *   dup() function.
 *   
 *   On success, returns a new, non-null osfildef* handle duplicating the
 *   original handle.  Returns null on failure.
 *   
 *   'mode' is a simplified stdio fopen() mode string.  The first
 *   character(s) indicate the access type: "r" for read access, "w" for
 *   write access, or "r+" for read/write access.  Note that "w+" mode is
 *   specifically not defined, since the fopen() handling of "w+" is to
 *   truncate any existing file contents, which is not desirable when
 *   duplicating a handle.  The access type can optionally be followed by "t"
 *   for text mode, "s" for source file mode, or "b" for binary mode, with
 *   the same meanings as for the various osfop*() functions.  The default is
 *   't' for text mode if none of these are specified.
 *   
 *   If the osfop*() functions are implemented in terms of stdio FILE*
 *   objects, this can be implemented as fdopen(dup(fileno(orig)), mode), or
 *   using equivalents if the local stdio library uses different names for
 *   these functions.  Note that "s" (source file format) isn't a stdio mode,
 *   so implementations must translate it to the appropriate "t" or "b" mode.
 *   (For that matter, "t" and "b" modes aren't universally supported either,
 *   so some implementations may have to translate these, or more likely
 *   simply remove them, as most platforms don't distinguish text and binary
 *   modes anyway.)
 */
osfildef *osfdup(osfildef *orig, const char *mode);

/* 
 *   Set a file's type information.  This is primarily for implementations on
 *   Mac OS 9 and earlier, where the file system keeps file-type metadata
 *   separate from the filename.  On such systems, this can be used to set
 *   the type metadata after a file is created.  The system should map the
 *   os_filetype_t values to the actual metadata values on the local system.
 *   On most systems, there's no such thing as file-type metadata, in which
 *   case this function should simply be stubbed out with an empty function.
 */
void os_settype(const char *f, os_filetype_t typ);

/* open the error message file for reading */
osfildef *oserrop(const char *arg0);

/* 
 *   Get a line of text from a text file.  Uses fgets semantics.  
 */
/* char *osfgets(char *buf, size_t len, osfildef *fp); */

/* 
 *   Write a line of text to a text file.  Uses fputs semantics.  
 */
/* void osfputs(const char *buf, osfildef *fp); */

/*
 *   Write to a text file.  os_fprintz() takes a null-terminated string,
 *   while os_fprint() takes an explicit separate length argument that might
 *   not end with a null terminator.  
 */
void os_fprintz(osfildef *fp, const char *str);
void os_fprint(osfildef *fp, const char *str, size_t len);

/* 
 *   Write bytes to file.  Return 0 on success, non-zero on error.
 */
/* int osfwb(osfildef *fp, const void *buf, int bufl); */

/*
 *   Flush buffered writes to a file.  This ensures that any bytes written to
 *   the file (with osfwb(), os_fprint(), etc) are actually sent out to the
 *   operating system, rather than being buffered in application memory for
 *   later writing.
 *   
 *   Note that this routine only guarantees that we write through to the
 *   operating system.  This does *not* guarantee that the data will actually
 *   be committed to the underlying physical storage device.  Such a
 *   guarantee is hard to come by in general, since most modern systems use
 *   multiple levels of software and hardware buffering - the OS might buffer
 *   some data in system memory, and the physical disk drive might itself
 *   buffer data in its own internal cache.  This routine thus isn't good
 *   enough, for example, to protect transactional data that needs to survive
 *   a power failure or a serious system crash.  What this routine *does*
 *   ensure is that buffered data are written through to the OS; in
 *   particular, this ensures that another process that's reading from the
 *   same file will see all updates we've made up to this point.
 *   
 *   Returns 0 on success, non-zero on error.  Errors can occur for any
 *   reason that they'd occur on an ordinary write - a full disk, a hardware
 *   failure, etc.  
 */
/* int osfflush(osfildef *fp); */

/* 
 *   Read a character from a file.  Provides the same semantics as fgetc().
 */
/* int osfgetc(osfildef *fp); */

/* 
 *   Read bytes from file.  Return 0 on success, non-zero on error.  
 */
/* int osfrb(osfildef *fp, void *buf, int bufl); */

/* 
 *   Read bytes from file and return the number of bytes read.  0
 *   indicates that no bytes could be read. 
 */
/* size_t osfrbc(osfildef *fp, void *buf, size_t bufl); */

/* 
 *   Get the current seek location in the file.  The first byte of the
 *   file has seek position 0.  
 */
/* long osfpos(osfildef *fp); */

/* 
 *   Seek to a location in the file.  The first byte of the file has seek
 *   position 0.  Returns zero on success, non-zero on error.
 *   
 *   The following constants must be defined in your OS-specific header;
 *   these values are used for the "mode" parameter to indicate where to
 *   seek in the file:
 *   
 *   OSFSK_SET - set position relative to the start of the file
 *.  OSFSK_CUR - set position relative to the current file position
 *.  OSFSK_END - set position relative to the end of the file 
 */
/* int osfseek(osfildef *fp, long pos, int mode); */

/* 
 *   Close a file.
 *   
 *   If the OS implementation uses buffered writes, this routine guarantees
 *   that any buffered data are flushed to the underlying file.  So, it's not
 *   necessary to call osfflush() before calling this routine.  However,
 *   since this function doesn't return any error indication, a caller could
 *   use osfflush() first to check for errors on any final buffered writes.  
 */
/* void osfcls(osfildef *fp); */

/* 
 *   Delete a file.  Returns zero on success, non-zero on error. 
 */
/* int osfdel(const char *fname); */

/*
 *   Rename/move a file.  This should apply the usual C rename() behavior.
 *   Renames the old file to the new name, which may be in a new directory
 *   location if supported on the local system; moves across devices,
 *   volumes, file systems, etc may or may not be supported according to the
 *   local system's rules.  If the new file already exists, results are
 *   undefined.  Returns true on success, false on failure.
 */
/* int os_rename_file(const char *oldname, const char *newname); */


/* ------------------------------------------------------------------------ */
/*
 *   File "stat()" information - mode, size, time stamps 
 */

/* 
 *   Test access to a file - i.e., determine if the file exists.  Returns
 *   zero if the file exists, non-zero if not.  (The semantics may seem
 *   backwards, but this is consistent with the conventions used by most of
 *   the other osfxxx calls: zero indicates success, non-zero indicates an
 *   error.  If the file exists, "accessing" it was successful, so osfacc
 *   returns zero; if the file doesn't exist, accessing it gets an error,
 *   hence a non-zero return code.)  
 */
/* int osfacc(const char *fname) */

/*
 *   Get a file's mode and attribute flags.  This retrieves information on
 *   the given file equivalent to the st_mode member of the 'struct stat'
 *   data returned by the Unix stat() family of functions, as well as some
 *   extra system-specific attributes.  On success, fills in *mode (if mode
 *   is non-null) with the mode information as a bitwise combination of
 *   OSFMODE_xxx values, fills in *attr (if attr is non-null) with a
 *   combination of OSFATTR_xxx attribute flags, and returns true; on
 *   failure, simply returns false.  Failure can occur if the file doesn't
 *   exist, can't be accessed due to permissions, etc.
 *   
 *   Note that 'mode' and/or 'attr' can be null if the caller doesn't need
 *   that information.  Implementations must check these parameters for null
 *   pointers and skip returning the corresponding information if null.
 *   
 *   If the file in 'fname' is a symbolic link, the behavior depends upon
 *   'follow_links'.  If 'follow_links' is true, the function should resolve
 *   the link reference (and if that points to another link, the function
 *   resolves that link as well, and so on) and return information on the
 *   object the link points to.  Otherwise, the function returns information
 *   on the link itself.  This only applies for symbolic links (not for hard
 *   links), and only if the underlying OS and file system support this
 *   distinction; if the OS transparently resolves links and doesn't allow
 *   retrieving information about the link itself, 'follow_links' can be
 *   ignored.  Likewise, hard links (on systems that support them) are
 *   generally indistinguishable from regular files, so this function isn't
 *   expected to do anything special with them.
 *   
 *   The '*mode' value returned is a bitwise combination of OSFMODE_xxx flag.
 *   Many of the flags are mutually exclusive; for example, "file" and
 *   "directory" should never be combined.  It's also possible for '*mode' to
 *   be zero for a valid file; this means that the file is of some special
 *   type on the local system that doesn't fit any of the OSFMODE_xxx types.
 *   (If any ports do encounter such cases, we can add OSFMODE_xxx types to
 *   accommodate new types.  The list below isn't meant to be final; it's
 *   just what we've encountered so far on the platforms where TADS has
 *   already been ported.)
 *   
 *   The OSFMODE_xxx values are left for the OS to define so that they can be
 *   mapped directly to the OS API's equivalent constants, if desired.  This
 *   makes the routine easy to write, since you can simply set *mode directly
 *   to the mode information the OS returns from its stat() or equivalent.
 *   However, note that these MUST be defined as bit flags - that is, each
 *   value must be exactly a power of 2.  Windows and Unix-like systems
 *   follow this practice, as do most "stat()" functions in C run-time
 *   libraries, so this usually works automatically if you map these
 *   constants to OS or C library values.  However, if a port defines its own
 *   values for these, take care that they're all powers of 2.
 *   
 *   Obviously, a given OS might not have all of the file types listed here.
 *   If any OSFMODE_xxx values aren't applicable on the local OS, you can
 *   simply define them as zero since they'll never be returned.
 *   
 *   Notes on attribute flags:
 *   
 *   OSFATTR_HIDDEN means that the file is conventionally hidden by default
 *   in user interface views or listings, but is still fully accessible to
 *   the user.  Hidden files are also usually excluded by default from
 *   wildcard patterns in commands ("rm *.*").  On Unix, a hidden file is one
 *   whose name starts with "."; on Windows, it's a file with the HIDDEN bit
 *   in its file attributes.  On systems where this concept exists, the user
 *   can still manipulate these files as normal by naming them explicitly,
 *   and can typically make them appear in UI views or directory listings via
 *   a preference setting or command flag (e.g., "ls -a" on Unix).  The
 *   "hidden" flag is explicitly NOT a security or permissions mechanism, and
 *   it doesn't protect the file against intentional access by a user; it's
 *   merely a convenience designed to reduce clutter by excluding files
 *   maintained by the OS or by an application (such as preference files,
 *   indices, caches, etc) from casual folder browsing, where a user is
 *   typically only concerned with her own document files.  On systems where
 *   there's no such naming convention or attribute metadata, this flag will
 *   never appear.
 *   
 *   OSFATTR_SYSTEM is similar to 'hidden', but means that the file is
 *   specially marked as an operating system file.  This is mostly a
 *   DOS/Windows concept, where it corresponds to the SYSTEM bit in the file
 *   attributes; this flag will probably never appear on other systems.  The
 *   distinction between 'system' and 'hidden' is somewhat murky even on
 *   Windows; most 'system' file are also marked as 'hidden', and in
 *   practical terms in the user interface, 'system' files are treated the
 *   same as 'hidden'.
 *   
 *   OSFATTR_READ means that the file is readable by this process.
 *   
 *   OSFATTR_WRITE means that the file is writable by this process.
 */
/* int osfmode(const char *fname, int follow_links, */
/*             unsigned long *mode, unsigned long *attr); */

/* file mode/type constants */
/* #define OSFMODE_FILE    - regular file */
/* #define OSFMODE_DIR     - directory */
/* #define OSFMODE_BLK     - block-mode device */
/* #define OSFMODE_CHAR    - character-mode device */
/* #define OSFMODE_PIPE    - pipe/FIFO/other character-oriented IPC */
/* #define OSFMODE_SOCKET  - network socket */
/* #define OSFMODE_LINK    - symbolic link */

/* file attribute constants */
/* #define OSFATTR_HIDDEN  - hidden file */
/* #define OSFATTR_SYSTEM  - system file */
/* #define OSFATTR_READ    - the file is readable by this process */
/* #define OSFATTR_WRITE   - the file is writable by this process */

struct os_file_stat_t {
    /* 
     *   Size of the file, in bytes.  For platforms lacking 64-bit types, we
     *   split this into high and low 32-bit portions.  Platforms where the
     *   native stat() or equivalent only returns a 32-bit file size can
     *   simply set sizehi to zero, since sizelo can hold the entire size
     *   value.
     */
    uint32_t sizelo;
    uint32_t sizehi;

    /* 
     *   Creation time, modification time, and last access time.  If the file
     *   system doesn't keep information on one or more of these, use
     *   (os_time_t)0 to indicate that the timestamp isn't available.  It's
     *   fine to return any subset of these.  Per the standard C stat(),
     *   these should be expressed as seconds after the Unix Epoch.
     */
    os_time_t cre_time;
    os_time_t mod_time;
    os_time_t acc_time;

    /* file mode, using the same flags as returned from osfmode() */
    unsigned long mode;

    /* file attributes, using the same flags as returned from osfmode() */
    unsigned long attrs;
};


/*
 *   Get stat() information.  This fills in the portable os_file_stat
 *   structure with the requested file information.  Returns true on success,
 *   false on failure (file not found, permissions error, etc).
 *   
 *   'follow_links' has the same meaning as for osfmode().
 */
int os_file_stat(const char *fname, int follow_links, os_file_stat_t *s);

/*
 *   Manually resolve a symbolic link.  If the local OS and file system
 *   support symbolic links, and the given filename is a symbolic link (in
 *   which case osfmode(fname, FALSE, &m, &a) will set OSFMODE_LINK in the
 *   mode bits), this fills in 'target' with the name of the link target
 *   (i.e., the object that the link in 'fname' points to).  This should
 *   return a fully qualified file system path.  Returns true on success,
 *   false on failure.
 *   
 *   This should only resolve a single level of indirection.  If the link
 *   target of 'fname' is itself a link to a second target, this should only
 *   resolve the single reference from 'fname' to its direct direct.  Callers
 *   that wish to resolve the final target of a chain of link references must
 *   iterate until the returned path doesn't refer to a link.
 */
int os_resolve_symlink(const char *fname, char *target, size_t target_size);


/* ------------------------------------------------------------------------ */
/*
 *   Get a list of root directories.  If 'buf' is non-null, fills in 'buf'
 *   with a list of strings giving the root directories for the local,
 *   file-oriented devices on the system.  The strings are each null
 *   terminated and are arranged consecutively in the buffer, with an extra
 *   null terminator after the last string to mark the end of the list.
 *   
 *   The return value is the length of the buffer required to hold the
 *   results.  If the caller's buffer is null or is too short, the routine
 *   should return the full length required, and leaves the contents of the
 *   buffer undefined; the caller shouldn't expect any contents to be filled
 *   in if the return value is greater than buflen.  Both 'buflen' and the
 *   return value include the null terminators, including the extra null
 *   terminator at the end of the list.  If an error occurs, or the system
 *   has no concept of a root directory, returns zero.
 *   
 *   Each result string should be expressed using the syntax for the root
 *   directory on a device.  For example, on Windows, "C:\" represents the
 *   root directory on the C: drive.
 *   
 *   "Local" means a device is mounted locally, as opposed to being merely
 *   visible on the network via some remote node syntax; e.g., on Windows
 *   this wouldn't include any UNC-style \\SERVER\SHARE names, and on VMS it
 *   excludes any SERVER:: nodes.  It's up to each system how to treat
 *   virtual local devices, i.e., those that look synctactically like local
 *   devices but are actually mounted network devices, such as Windows mapped
 *   network drives; we recommend including them if it would take extra work
 *   to filter them out, and excluding them if it would take extra work to
 *   include them.  "File-oriented" means that the returned devices are
 *   accessed via file systems, not as character devices or raw block
 *   devices; so this would exclude /dev/xxx devices on Unix and things like
 *   CON: and LPT1: on Windows.
 *   
 *   Examples ("." represents a null byte):
 *   
 *   Windows: C:\.D:\.E:\..
 *   
 *   Unix example: /..
 */
size_t os_get_root_dirs(char *buf, size_t buflen);


/* ------------------------------------------------------------------------ */
/*
 *   Open a directory.  This begins an enumeration of a directory's contents.
 *   'dirname' is a relative or absolute path to a directory.  On success,
 *   returns true, and 'handle' is set to a port-defined handle value that's
 *   used in subsequent calls to os_read_dir() and os_close_dir().  Returns
 *   false on failure.
 *   
 *   If the routine succeeds, the caller must eventually call os_close_dir()
 *   to release the resources associated with the handle.
 */
/* typedef <local system type> osdirhdl_t; */
int os_open_dir(const char *dirname, /*OUT*/osdirhdl_t *handle);

/*
 *   Read the next file in a directory.  'handle' is a handle value obtained
 *   from a call to os_open_dir().  On success, returns true and fills in
 *   'fname' with the next filename; the handle is also internally updated so
 *   that the next call to this function will retrieve the next file, and so
 *   on until all files have been retrieved.  If an error occurs, or there
 *   are no more files in the directory, returns false.
 *   
 *   The filename returned is the root filename only, without the path.  The
 *   caller can build the full path by calling os_build_full_path() or
 *   os_combine_paths() with the original directory name and the returned
 *   filename as parameters.
 *   
 *   This routine lists all objects in the directory that are visible to the
 *   corresponding native API, and is non-recursive.  The listing should thus
 *   include subdirectory objects, but not the contents of subdirectories.
 *   Implementations are encouraged to simply return all objects returned
 *   from the corresponding native directory scan API; there's no need to do
 *   any filtering, except perhaps in cases where it's difficult or
 *   impossible to represent an object in terms of the osifc APIs (e.g., it
 *   might be reasonable to exclude files without names).  System relative
 *   links, such as the Unix/DOS "." and "..", specifically should be
 *   included in the listing.  For unusual objects that don't fit into the
 *   os_file_stat() taxonomy or that otherwise might create confusion for a
 *   caller, err on the side of full disclosure (i.e., just return everything
 *   unfiltered); if necessary, we can extend the os_file_stat() taxonomy or
 *   add new osifc APIs to create a portable abstraction to handle whatever
 *   is unusual or potentially confusing about the native object.  For
 *   example, Unix implementations should feel free to return symbolic link
 *   objects, including dangling links, since we have the portable
 *   os_resolve_symlink() that lets the caller examine the meaning of the
 *   link object.
 */
int os_read_dir(osdirhdl_t handle, char *fname, size_t fname_size);

/*
 *   Close a directory handle.  This releases the resources associated with a
 *   directory search started with os_open_dir().  Every successful call to
 *   os_open_dir() must have a matching call to os_close_dir().  As usual for
 *   open/close protocols, the handle is invalid after calling this function,
 *   so no more calls to os_read_dir() may be made with the handle.
 */
void os_close_dir(osdirhdl_t handle);


/* ------------------------------------------------------------------------ */
/*
 *   NB - this routine is DEPRECATED as of TADS 2.5.16/3.1.1.  Callers should
 *   use os_open_dir(), os_read_dir(), os_close_dir() instead.
 *   
 *   Find the first file matching a given pattern.  The returned context
 *   pointer is a pointer to whatever system-dependent context structure is
 *   needed to continue the search with the next file, and is opaque to the
 *   caller.  The caller must pass the context pointer to the next-file
 *   routine.  The caller can optionally cancel a search by calling the
 *   close-search routine with the context pointer.  If the return value is
 *   null, it indicates that no matching files were found.  If a file was
 *   found, outbuf will be filled in with its name, and isdir will be set to
 *   true if the match is a directory, false if it's a file.  If pattern is
 *   null, all files in the given directory should be returned; otherwise,
 *   pattern is a string containing '*' and '?' as wildcard characters, but
 *   not containing any directory separators, and all files in the given
 *   directory matching the pattern should be returned.
 *   
 *   Important: because this routine may allocate memory for the returned
 *   context structure, the caller must either call os_find_next_file until
 *   that routine returns null, or call os_find_close() to cancel the search,
 *   to ensure that the os code has a chance to release the allocated memory.
 *   
 *   'outbuf' should be set on output to the name of the matching file,
 *   without any path information.
 *   
 *   'outpathbuf' should be set on output to full path of the matching file.
 *   If possible, 'outpathbuf' should use the same relative or absolute
 *   notation that the search criteria used on input.  For example, if dir =
 *   "resfiles", and the file found is "MyPic.jpg", outpathbuf should be set
 *   to "resfiles/MyPic.jpg" (or appropriate syntax for the local platform).
 *   Similarly, if dir = "/home/tads/resfiles", outpath buf should be
 *   "/home/tads/resfiles/MyPic.jpg".  The result should always conform to
 *   correct local conventions, which may require some amount of manipulation
 *   of the filename; for example, on the Mac, if dir = "resfiles", the
 *   result should be ":resfiles:MyPic.jpg" (note the added leading colon) to
 *   conform to Macintosh relative path notation.
 *   
 *   Note that 'outpathbuf' may be null, in which case the caller is not
 *   interested in the full path information.  
 */
/*   
 *   Note the following possible ways this function may be called:
 *   
 *   dir = "", pattern = filename - in this case, pattern is the name of a
 *   file or directory in the current directory.  filename *might* be a
 *   relative path specified by the user (on a command line, for example);
 *   for instance, on Unix, it could be something like "resfiles/jpegs".
 *   
 *   dir = path, pattern = filname - same as above, but this time the
 *   filename or directory pattern is relative to the given path, rather
 *   than to the current directory.  For example, we could have dir =
 *   "/games/mygame" and pattern = "resfiles/jpegs".
 *   
 *   dir = path, pattern = 0 (NULL) - this should search for all files in
 *   the given path.  The path might be absolute or it might be relative.
 *   
 *   dir = path, pattern = "*" - this should have the same result as when
 *   pattern = 0.
 *   
 *   dir = path, pattern = "*.ext" - this should search for all files in
 *   the given path whose names end with ".ext".
 *   
 *   dir = path, pattern = "abc*" - this should search for all files in
 *   the given path whose names start with "abc".
 *   
 *   All of these combinations are possible because callers, for
 *   portability, must generally not manipulate filenames directly;
 *   instead, callers obtain paths and search strings from external
 *   sources, such as from the user, and present them to this routine with
 *   minimal manipulation.  
 */
void *os_find_first_file(const char *dir,
                         char *outbuf, size_t outbufsiz, int *isdir,
                         char *outpathbuf, size_t outpathbufsiz);

/*
 *   Implementation notes for porting os_find_first_file:
 *   
 *   The algorithm for this routine should go something like this:
 *   
 *   - If 'path' is null, create a variable real_path and initialize it
 *   with the current directory.  Otherwise, copy path to real_path.
 *   
 *   - If 'pattern' contains any directory separators ("/" on Unix, for
 *   example), change real_path so that it reflects the additional leading
 *   subdirectories in the path in 'pattern', and remove the leading path
 *   information from 'pattern'.  For example, on Unix, if real_path
 *   starts out as "./subdir", and pattern is "resfiles/jpegs", change
 *   real_path to "./subdir/resfiles", and change pattern to "jpegs".
 *   Take care to add and remove path separators as needed to keep the
 *   path strings well-formed.
 *   
 *   - Begin a search using appropriate OS API's for all files in
 *   real_path.
 *   
 *   - Check each file found.  Skip any files that don't match 'pattern',
 *   treating "*" as a wildcard that matches any string of zero or more
 *   characters, and "?" as a wildcard that matches any single character
 *   (or matches nothing at the end of a string).  For example:
 *   
 *.  "*" matches anything
 *.  "abc?" matches "abc", "abcd", "abce", "abcf", but not "abcde"
 *.  "abc???" matches "abc", "abcd", "abcde", "abcdef", but not "abcdefg"
 *.  "?xyz" matches "wxyz", "axyz", but not "xyz" or "abcxyz"
 *   
 *   - Return the first file that matches, if any, by filling in 'outbuf'
 *   and 'isdir' with appropriate information.  Before returning, allocate
 *   a context structure (which is entirely for your own use, and opaque
 *   to the caller) and fill it in with the information necessary for
 *   os_find_next_file to get the next matching file.  If no file matches,
 *   return null.  
 */


/*
 *   Find the next matching file, continuing a search started with
 *   os_find_first_file().  Returns null if no more files were found, in
 *   which case the search will have been automatically closed (i.e.,
 *   there's no need to call os_find_close() after this routine returns
 *   null).  Returns a non-null context pointer, which is to be passed to
 *   this function again to get the next file, if a file was found.
 *   
 *   'outbuf' and 'outpathbuf' are filled in with the filename (without
 *   path) and full path (relative or absolute, as appropriate),
 *   respectively, in the same manner as they do for os_find_first_file().
 *   
 *   Implementation note: if os_find_first_file() allocated memory for the
 *   search context, this routine must free the memory if it returs null,
 *   because this indicates that the search is finished and the caller
 *   need not call os_find_close().  
 */
void *os_find_next_file(void *ctx, char *outbuf, size_t outbufsiz,
                        int *isdir, char *outpathbuf, size_t outpathbufsiz);

/*
 *   Cancel a search.  The context pointer returned by the last call to
 *   os_find_first_file() or os_find_next_file() is the parameter.  There
 *   is no need to call this function if find-first or find-next returned
 *   null, since they will have automatically closed the search.
 *   
 *   Implementation note: if os_find_first_file() allocated memory for the
 *   search context, this routine should release the memory.  
 */
void os_find_close(void *ctx);

/*
 *   Special filename classification 
 */
enum os_specfile_t
{
    /* not a special file */
    OS_SPECFILE_NONE,

    /* 
     *   current directory link - this is a file like the "." file on Unix
     *   or DOS, which is a special link that simply refers to itself 
     */
    OS_SPECFILE_SELF,

    /* 
     *   parent directory link - this is a file like the ".." file on Unix
     *   or DOS, which is a special link that refers to the parent
     *   directory 
     */
    OS_SPECFILE_PARENT
};

/*
 *   Determine if the given filename refers to a special file.  Returns the
 *   appropriate enum value if so, or OS_SPECFILE_NONE if not.  The given
 *   filename must be a root name - it must not contain a path prefix.  The
 *   purpose here is to classify the results from os_find_first_file() and
 *   os_find_next_file() to identify the special relative links, so callers
 *   can avoid infinite recursion when traversing a directory tree.
 */
enum os_specfile_t os_is_special_file(const char *fname);

/* ------------------------------------------------------------------------ */
/* 
 *   Convert string to all-lowercase. 
 */
char *os_strlwr(char *s);


/* ------------------------------------------------------------------------ */
/*
 *   Character classifications for quote characters.  os_squote() returns
 *   true if its argument is any type of single-quote character;
 *   os_dquote() returns true if its argument is any type of double-quote
 *   character; and os_qmatch(a, b) returns true if a and b are matching
 *   open- and close-quote characters.
 *   
 *   These functions allow systems with extended character codes with
 *   weird quote characters (such as the Mac) to match the weird
 *   characters, so that users can use the extended quotes in input.
 *   
 *   These are usually implemented as macros.  The most common
 *   implementation simply returns true for the standard ASCII quote
 *   characters:
 *   
 *   #define os_squote(c) ((c) == '\'')
 *.  #define os_dquote(c) ((c) == '"')
 *.  #define os_qmatch(a, b) ((a) == (b))
 *   
 *   These functions take int arguments to allow for the possibility of
 *   Unicode input.  
 */
/* int os_squote(int c); */
/* int os_dquote(int c); */
/* int os_qmatch(int a, int b); */


/* ------------------------------------------------------------------------ */
/*
 *   Special file and directory locations
 */

/*
 *   Get the full filename (including directory path) to the executable
 *   file, given the argv[0] parameter passed into the main program.  This
 *   fills in the buffer with a null-terminated string that can be used in
 *   osfoprb(), for example, to open the executable file.
 *   
 *   Returns non-zero on success.  If it's not possible to determine the
 *   name of the executable file, returns zero.
 *   
 *   Some operating systems might not provide access to the executable file
 *   information, so non-trivial implementation of this routine is optional;
 *   if the necessary information is not available, simply implement this to
 *   return zero.  If the information is not available, callers should offer
 *   gracefully degraded functionality if possible.  
 */
int os_get_exe_filename(char *buf, size_t buflen, const char *argv0);

/*
 *   Get a special directory path.  Returns the selected path, in a format
 *   suitable for use with os_build_full_path().  The main program's argv[0]
 *   parameter is provided so that the system code can choose to make the
 *   special paths relative to the program install directory, but this is
 *   entirely up to the system implementation, so the argv[0] parameter can
 *   be ignored if it is not needed.
 *   
 *   The 'id' parameter selects which special path is requested; this is one
 *   of the constants defined below.  If the id is not understood, there is
 *   no way of signalling an error to the caller; this routine can fail with
 *   an assert() in such cases, because it indicates that the OS layer code
 *   is out of date with respect to the calling code.
 *   
 *   This routine can be implemented using one of the strategies below, or a
 *   combination of these.  These are merely suggestions, though, and systems
 *   are free to ignore these and implement this routine using whatever
 *   scheme is the best fit to local conventions.
 *   
 *   - Relative to argv[0].  Some systems use this approach because it keeps
 *   all of the TADS files together in a single install directory tree, and
 *   doesn't require any extra configuration information to find the install
 *   directory.  Since we base the path name on the executable that's
 *   actually running, we don't need any environment variables or parameter
 *   files or registry entries to know where to look for related files.
 *   
 *   - Environment variables or local equivalent.  On some systems, it is
 *   conventional to set some form of global system parameter (environment
 *   variables on Unix, for example) for this sort of install configuration
 *   data.  In these cases, this routine can look up the appropriate
 *   configuration variables in the system environment.
 *   
 *   - Hard-coded paths.  Some systems have universal conventions for the
 *   installation configuration of compiler-like tools, so the paths to our
 *   component files can be hard-coded based on these conventions.
 *   
 *   - Hard-coded default paths with environment variable overrides.  Let the
 *   user set environment variables if they want, but use the standard system
 *   paths as hard-coded defaults if the variables aren't set.  This is often
 *   the best choice; users who expect the standard system conventions won't
 *   have to fuss with any manual settings or even be aware of them, while
 *   users who need custom settings aren't stuck with the defaults.
 */
void os_get_special_path(char *buf, size_t buflen,
                         const char *argv0, int id);

/* 
 *   TADS 3 system resource path.  This path is used to load system
 *   resources, such as character mapping files and error message files.  
 */
#define OS_GSP_T3_RES       1

/* 
 *   TADS 3 compiler - system headers.  This is the #include path for the
 *   header files included with the compiler. 
 */
#define OS_GSP_T3_INC       2

/*
 *   TADS 3 compiler - system library source code.  This is the path to the
 *   library source files that the compiler includes in every compilation by
 *   default (such as _main.t). 
 */
#define OS_GSP_T3_LIB       3

/*
 *   TADS 3 compiler - user library path list.  This is a list of directory
 *   paths, separated by the OSPATHSEP character, that should be searched for
 *   user library files.  The TADS 3 compiler uses this as an additional set
 *   of locations to search after the list of "-Fs" options and before the
 *   OS_GSP_T3_LIB directory.
 *   
 *   This path list is intended for the user's use, so no default value is
 *   needed.  The value should be user-configurable using local conventions;
 *   on Unix, for example, this might be handled with an environment
 *   variable.  
 */
#define OS_GSP_T3_USER_LIBS 4

/*
 *   TADS 3 interpreter - application data path.  This is the directory where
 *   we should store things like option settings: data that we want to store
 *   in a file, global to all games.  Depending on local system conventions,
 *   this can be a global shared directory for all users, or can be a
 *   user-specific directory. 
 */
#define OS_GSP_T3_APP_DATA 5

/*
 *   TADS 3 interpreter - system configuration files.  This is used for files
 *   that affect all games, and generally all users on the system, so it
 *   should be in a central location.  On Windows, for example, we simply
 *   store these files in the install directory containing the intepreter
 *   binary.  
 */
#define OS_GSP_T3_SYSCONFIG  6

/*
 *   System log files.  This is the directory for system-level status, debug,
 *   and error logging files.  (Note that we're NOT talking about in-game
 *   transcript logging per the SCRIPT command.  SCRIPT logs are usually sent
 *   to files selected by the user via a save-file dialog, so these don't
 *   need a special location.)
 */
#define OS_GSP_LOGFILE  7


/* 
 *   Seek to the resource file embedded in the current executable file,
 *   given the main program's argv[0].
 *   
 *   On platforms where the executable file format allows additional
 *   information to be attached to an executable, this function can be used
 *   to find the extra information within the executable.
 *   
 *   The 'typ' argument gives a resource type to find.  This is an arbitrary
 *   string that the caller uses to identify what type of object to find.
 *   The "TGAM" type, for example, is used by convention to indicate a TADS
 *   compiled GAM file.  
 */
osfildef *os_exeseek(const char *argv0, const char *typ);


/* ------------------------------------------------------------------------ */
/*
 *   Load a string resource.  Given a string ID number, load the string
 *   into the given buffer.
 *   
 *   Returns zero on success, non-zero if an error occurs (for example,
 *   the buffer is too small, or the requested resource isn't present).
 *   
 *   Whenever possible, implementations should use an operating system
 *   mechanism for loading the string from a user-modifiable resource
 *   store; this will make localization of these strings easier, since the
 *   resource store can be modified without the need to recompile the
 *   application.  For example, on the Macintosh, the normal system string
 *   resource mechanism should be used to load the string from the
 *   application's resource fork.
 *   
 *   When no operating system mechanism exists, the resources can be
 *   stored as an array of strings in a static variable; this isn't ideal,
 *   because it makes it much more difficult to localize the application.
 *   
 *   Resource ID's are application-defined.  For example, for TADS 2,
 *   "res.h" defines the resource ID's.  
 */
int os_get_str_rsc(int id, char *buf, size_t buflen);


/* ------------------------------------------------------------------------ */
/*
 *   Look for a file in the "standard locations": current directory, program
 *   directory, PATH-like environment variables, etc.  The actual standard
 *   locations are specific to each platform; the implementation is free to
 *   use whatever conventions are appropriate to the local system.  On
 *   systems that have something like Unix environment variables, it might be
 *   desirable to define a TADS-specific variable (TADSPATH, for example)
 *   that provides a list of directories to search for TADS-related files.
 *   
 *   On return, fill in 'buf' with the full filename of the located copy of
 *   the file (if a copy was indeed found), in a format suitable for use with
 *   the osfopxxx() functions; in other words, after this function returns,
 *   the caller should be able to pass the contents of 'buf' to an osfopxxx()
 *   function to open the located file.
 *   
 *   Returns true (non-zero) if a copy of the file was located, false (zero)
 *   if the file could not be found in any of the standard locations.  
 */
int os_locate(const char *fname, int flen, const char *arg0,
              char *buf, size_t bufsiz);


/* ------------------------------------------------------------------------ */
/*
 *   Create and open a temporary file.  The file must be opened to allow
 *   both reading and writing, and must be in "binary" mode rather than
 *   "text" mode, if the system makes such a distinction.  Returns null on
 *   failure.
 *   
 *   If 'fname' is non-null, then this routine should create and open a file
 *   with the given name.  When 'fname' is non-null, this routine does NOT
 *   need to store anything in 'buf'.  Note that the routine shouldn't try
 *   to put the file in a special directory or anything like that; just open
 *   the file with the name exactly as given.
 *   
 *   If 'fname' is null, this routine must choose a file name and fill in
 *   'buf' with the chosen name; if possible, the file should be in the
 *   conventional location for temporary files on this system, and should be
 *   unique (i.e., it shouldn't be the same as any existing file).  The
 *   filename stored in 'buf' is opaque to the caller, and cannot be used by
 *   the caller except to pass to osfdel_temp().  On some systems, it may
 *   not be possible to determine the actual filename of a temporary file;
 *   in such cases, the implementation may simply store an empty string in
 *   the buffer.  (The only way the filename would be unavailable is if the
 *   implementation uses a system API that creates a temporary file, and
 *   that API doesn't return the name of the created temporary file.  In
 *   such cases, we don't need the name; the only reason we need the name is
 *   so we can pass it to osfdel_temp() later, but since the system is going
 *   to delete the file automatically, osfdel_temp() doesn't need to do
 *   anything and thus doesn't need the name.)
 *   
 *   After the caller is done with the file, it should close the file (using
 *   osfcls() as normal), then the caller MUST call osfdel_temp() to delete
 *   the temporary file.
 *   
 *   This interface is intended to take advantage of systems that have
 *   automatic support for temporary files, while allowing implementation on
 *   systems that don't have any special temp file support.  On systems that
 *   do have automatic delete-on-close support, this routine should use that
 *   system-level support, because it helps ensure that temp files will be
 *   deleted even if the caller fails to call osfdel_temp() due to a
 *   programming error or due to a process or system crash.  On systems that
 *   don't have any automatic delete-on-close support, this routine can
 *   simply use the same underlying system API that osfoprwbt() normally
 *   uses (although this routine must also generate a name for the temp file
 *   when the caller doesn't supply one).
 *   
 *   This routine can be implemented using ANSI library functions as
 *   follows: if 'fname' is non-null, return fopen(fname,"w+b"); otherwise,
 *   set buf[0] to '\0' and return tmpfile().  
 */
osfildef *os_create_tempfile(const char *fname, char *buf);

/*
 *   Delete a temporary file - this is used to delete a file created with
 *   os_create_tempfile().  For most platforms, this can simply be defined
 *   the same way as osfdel().  For platforms where the operating system or
 *   file manager will automatically delete a file opened as a temporary
 *   file, this routine should do nothing at all, since the system will take
 *   care of deleting the temp file.
 *   
 *   Callers are REQUIRED to call this routine after closing a file opened
 *   with os_create_tempfile().  When os_create_tempfile() is called with a
 *   non-null 'fname' argument, the same value should be passed as 'fname' to
 *   this function.  When os_create_tempfile() is called with a null 'fname'
 *   argument, then the buffer passed in the 'buf' argument to
 *   os_create_tempfile() must be passed as the 'fname' argument here.  In
 *   other words, if the caller explicitly names the temporary file to be
 *   opened in os_create_tempfile(), then that same filename must be passed
 *   here to delete the named file; if the caller lets os_create_tempfile()
 *   generate a filename, then the generated filename must be passed to this
 *   routine.
 *   
 *   If os_create_tempfile() is implemented using ANSI library functions as
 *   described above, then this routine can also be implemented with ANSI
 *   library calls as follows: if 'fname' is non-null and fname[0] != '\0',
 *   then call remove(fname); otherwise do nothing.  
 */
int osfdel_temp(const char *fname);

/*
 *   Get the temporary file path.  This should fill in the buffer with a
 *   path prefix (suitable for strcat'ing a filename onto) for a good
 *   directory for a temporary file, such as the swap file.  
 */
void os_get_tmp_path(char *buf);

/* 
 *   Generate a name for a temporary file.  This constructs a random file
 *   path in the system temp directory that isn't already used by an existing
 *   file.
 *   
 *   On systems with long filenames, this can be implemented by selecting a
 *   GUID-strength random name (such as 32 random hex digits) with a decent
 *   random number generator.  That's long enough that the odds of a
 *   collision are essentially zero.  On systems that only support short
 *   filenames, the odds of a collision are non-zero, so the routine should
 *   actually check that the chosen filename doesn't exist.
 *   
 *   Optionally, before returning, this routine *may* create (and close) an
 *   empty placeholder file to "reserve" the chosen filename.  This isn't
 *   required, and on systems with long filenames it's usually not necessary
 *   because of the negligible chance of a collision.  On systems with short
 *   filenames, a placeholder can be useful to prevent a subsequent call to
 *   this routine, or a separate process, from using the same filename before
 *   the caller has had a chance to use the returned name to create the
 *   actual temp file.
 *   
 *   Returns true on success, false on failure.  This can fail if there's no
 *   system temporary directory defined, or the temp directory is so full of
 *   other files that we can't find an unused filename.  
 */
int os_gen_temp_filename(char *buf, size_t buflen);


/* ------------------------------------------------------------------------ */
/*
 *   Basic directory/folder management routines
 */

/*
 *   Switch to a new working directory.
 *   
 *   This is meant to behave similarly to the Unix concept of a working
 *   directory, in that it sets the base directory assumed for subsequent
 *   file operations (e.g., the osfopxx() functions, osfdel(), etc - anything
 *   that takes a filename or directory name as an argument).  The working
 *   directory applies to filenames specified with relative paths in the
 *   local system notation.  File operations on filenames specified with
 *   absolute paths, of course, ignore the working directory.
 */
void os_set_pwd(const char *dir);

/*
 *   Switch the working directory to the directory containing the given
 *   file.  Generally, this routine should only need to parse the filename
 *   enough to determine the part that's the directory path, then use
 *   os_set_pwd() to switch to that directory.  
 */
void os_set_pwd_file(const char *filename);

/*
 *   Create a directory.  This creates a new directory/folder with the given
 *   name, which may be given as a relative or absolute path.  Returns true
 *   on success, false on failure.
 *   
 *   If 'create_parents' is true, and the directory has mulitiple path
 *   elements, this routine should create each enclosing parent that doesn't
 *   already exist.  For example, if the path is specified as "a/b/c", and
 *   there exists a folder "a" in the working directory, but "a" is empty,
 *   this should first create "b" and then create "c".  If an error occurs
 *   creating any parent, the routine should simply stop and return failure.
 *   (Optionally, the routine may attempt to behave atomically by undoing any
 *   parent folder creations it accomplished before failing on a nested
 *   folder, but this isn't required.  To reduce the chances of a failure
 *   midway through the operation, the routine might want to scan the
 *   filename before starting to ensure that it contains only valid
 *   characters, since an invalid character is the most likely reason for a
 *   failure part of the way through.)
 *   
 *   We recommend making the routine flexible in terms of the notation it
 *   accepts; e.g., on Unix, "/dir/sub/folder" and "/dir/sub/folder/" should
 *   be considered equivalent.
 */
int os_mkdir(const char *dir, int create_parents);

/*
 *   Remove a directory.  Returns true on success, false on failure.
 *   
 *   If the directory isn't already empty, this routine fails.  That is, the
 *   routine does NOT recursively delete the contents of a non-empty
 *   directory.  It's up to the caller to delete any contents before removing
 *   the directory, if that's the caller's intention.  (Note to implementors:
 *   most native OS APIs to remove directories fail by default if the
 *   directory isn't empty, so it's usually safe to implement this simply by
 *   calling the native API.  However, if your system's version of this API
 *   can remove a non-empty directory, you MUST add an extra test before
 *   removing the directory to ensure it's empty, and return failure if it's
 *   not.  For the purposes of this test, "empty" should of course ignore any
 *   special objects that are automatically or implicitly present in all
 *   directories, such as the Unix "." and ".." relative links.)
 */
int os_rmdir(const char *dir);


/* ------------------------------------------------------------------------ */
/*
 *   Filename manipulation routines
 */

/* apply a default extension to a filename, if it doesn't already have one */
void os_defext(char *fname, const char *ext);

/* unconditionally add an extention to a filename */
void os_addext(char *fname, const char *ext);

/* remove the extension from a filename */
void os_remext(char *fname);

/*
 *   Compare two file names/paths for syntactic equivalence.  Returns true if
 *   the names are equivalent names according to the local file system's
 *   syntax conventions, false if not.  This does a syntax-only comparison of
 *   the paths, without looking anything up in the file system.  This means
 *   that a false return doesn't guarantee that the paths don't point to the
 *   same file.
 *   
 *   This routine DOES make the following equivalences:
 *   
 *   - if the local file system is insensitive to case, the names are
 *   compared ignoring case
 *   
 *   - meaningless path separator difference are ignored: on Unix, "a/b" ==
 *   "a//b" == "a/b/"; on Windows, "a/b" == "a\\b"
 *   
 *   - relative links that are strictly structural or syntactic are applied;
 *   for example, on Unix or Windows, "a/./b" == "a/b" = "a/b/c/..".  This
 *   only applies for special relative links that can be resolved without
 *   looking anything up in the file system.
 *   
 *   This DOES NOT do the following:
 *   
 *   - it doesn't apply working directories/volums to relative paths
 *   
 *   - it doesn't follow symbolic links in the file system
 */
int os_file_names_equal(const char *a, const char *b);

/*
 *   Get a pointer to the root name portion of a filename.  This is the part
 *   of the filename after any path or directory prefix.  For example, on
 *   Unix, given the string "/home/mjr/deep.gam", this function should return
 *   a pointer to the 'd' in "deep.gam".  If the filename doesn't appear to
 *   have a path prefix, it should simply return the argument unchanged.
 *   
 *   IMPORTANT: the returned pointer MUST point into the original 'buf'
 *   string, and the contents of that buffer must NOT be modified.  The
 *   return value must point into the same buffer because there are no
 *   allowances for the alternatives.  In particular, (a) you can't return a
 *   pointer to newly allocated memory, because callers won't free it, so
 *   doing so would cause a memory leak; and (b) you can't return a pointer
 *   to an internal static buffer, because callers might call this function
 *   more than once and still rely on a value returned on an older call,
 *   which would be invalid if a static buffer could be overwritten on each
 *   call.  For these reasons, it's required that the return value point to a
 *   position within the original string passed in 'buf'.  
 */
char *os_get_root_name(const char *buf);

/*
 *   Determine whether a filename specifies an absolute or relative path.
 *   This is used to analyze filenames provided by the user (for example,
 *   in a #include directive, or on a command line) to determine if the
 *   filename can be considered relative or absolute.  This can be used,
 *   for example, to determine whether to search a directory path for a
 *   file; if a given filename is absolute, a path search makes no sense.
 *   A filename that doesn't specify an absolute path can be combined with
 *   a path using os_build_full_path().
 *   
 *   Returns true if the filename specifies an absolute path, false if
 *   not.  
 */
int os_is_file_absolute(const char *fname);

/*
 *   Extract the path from a filename.  Fills in pathbuf with the path
 *   portion of the filename.  If the filename has no path, the pathbuf
 *   should be set appropriately for the current directory (on Unix or DOS,
 *   for example, it can be set to an empty string).
 *   
 *   The result can end with a path separator character or not, depending on
 *   local OS conventions.  Paths extracted with this function can only be
 *   used with os_build_full_path(), so the conventions should match that
 *   function's.
 *   
 *   Unix examples:
 *   
 *.   /home/mjr/deep.gam -> /home/mjr
 *.   games/deep.gam -> games
 *.   deep.gam -> [empty string]
 *   
 *   Mac examples:
 *   
 *    :home:mjr:deep.gam -> :home:mjr
 *.   Hard Disk:games:deep.gam -> Hard Disk:games
 *.   Hard Disk:deep.gam -> Hard Disk:
 *.   deep.gam -> [empty string]
 *   
 *   VMS examples:
 *
 *.   SYS$DISK:[mjr.games]deep.gam -> SYS$DISK:[mjr.games]
 *.   SYS$DISK:[mjr.games] -> SYS$DISK:[mjr]
 *.   deep.gam -> [empty string]
 *   
 *   Note in the last example that we've retained the trailing colon in the
 *   path, whereas we didn't in the others; although the others could also
 *   retain the trailing colon, it's required only for the last case.  The
 *   last case requires the colon because it would otherwise be impossible to
 *   determine whether "Hard Disk" was a local subdirectory or a volume name.
 *   
 */
void os_get_path_name(char *pathbuf, size_t pathbuflen, const char *fname);

/*
 *   Build a full path name, given a path and a filename.  The path may have
 *   been specified by the user, or may have been extracted from another file
 *   via os_get_path_name().  This routine must take care to add path
 *   separators as needed, but also must take care not to add too many path
 *   separators.
 *   
 *   This routine should reformat the path into canonical format to the
 *   extent possible purely through syntactic analysis.  For example, special
 *   relative links, such as Unix "." and "..", should be resolved; for
 *   example, combining "a/./b/c" with ".." on Unix should yield "a/b".
 *   However, symbolic links that require looking up names in the file system
 *   should NOT be resolved.  We don't want to perform any actual file system
 *   lookups because might want to construct hypothetical paths that don't
 *   necessarily relate to files on the local system.
 *   
 *   Note that relative path names may require special care on some
 *   platforms.  In particular, if the source path is relative, the result
 *   should also be relative.  For example, on the Macintosh, a path of
 *   "games" and a filename "deep.gam" should yield ":games:deep.gam" - note
 *   the addition of the leading colon to make the result path relative.
 *   
 *   Note also that the 'filename' argument is not only allowed to be an
 *   ordinary file, possibly qualified with a relative path, but is also
 *   allowed to be a subdirectory.  The result in this case should be a path
 *   that can be used as the 'path' argument to a subsequent call to
 *   os_build_full_path; this allows a path to be built in multiple steps by
 *   descending into subdirectories one at a time.
 *   
 *   Unix examples:
 *   
 *.   /home/mjr + deep.gam -> /home/mjr/deep.gam"
 *.   /home/mjr + .. -> /home
 *.   /home/mjr + ../deep.gam -> /home/deep.gam
 *.   /home/mjr/ + deep.gam -> /home/mjr/deep.gam"
 *.   games + deep.gam -> games/deep.gam"
 *.   games/ + deep.gam -> games/deep.gam"
 *.   /home/mjr + games/deep.gam -> /home/mjr/games/deep.gam"
 *.   games + scifi/deep.gam -> games/scifi/deep.gam"
 *.   /home/mjr + games -> /home/mjr/games"
 *   
 *   Mac examples:
 *   
 *.   Hard Disk: + deep.gam -> Hard Disk:deep.gam
 *.   :games: + deep.gam -> :games:deep.gam
 *.   :games:deep + ::test.gam -> :games:test.gam
 *.   games + deep.gam -> :games:deep.gam
 *.   Hard Disk: + :games:deep.gam -> Hard Disk:games:deep.gam
 *.   games + :scifi:deep.gam -> :games:scifi:deep.gam
 *.   Hard Disk: + games -> Hard Disk:games
 *.   Hard Disk:games + scifi -> Hard Disk:games:scifi
 *.   Hard Disk:games:scifi + deep.gam -> Hard Disk:games:scifi:deep.gam
 *.   Hard Disk:games + :scifi:deep.gam -> Hard Disk:games:scifi:deep.gam
 *   
 *   VMS examples:
 *   
 *.   [home.mjr] + deep.gam -> [home.mjr]deep.gam
 *.   [home.mjr] + [-]deep.gam -> [home]deep.gam
 *.   mjr.dir + deep.gam -> [.mjr]deep.gam
 *.   [home]mjr.dir + deep.gam -> [home.mjr]deep.gam
 *.   [home] + [.mjr]deep.gam -> [home.mjr]deep.gam
 */
void os_build_full_path(char *fullpathbuf, size_t fullpathbuflen,
                        const char *path, const char *filename);

/*
 *   Combine a path and a filename to form a full path to the file.  This is
 *   *almost* the same as os_build_full_path(), but if the 'filename' element
 *   is a special relative link, such as Unix '.' or '..', this preserves
 *   that special link in the final name.
 *   
 *   Unix examples:
 *   
 *.    /home/mjr + deep.gam -> /home/mjr/deep.gam
 *.    /home/mjr + . -> /home/mjr/.
 *.    /home/mjr + .. -> /home/mjr/..
 *   
 *   Mac examples:
 *   
 *.    Hard Disk:games + deep.gam -> HardDisk:games:deep.gam
 *.    Hard Disk:games + :: -> HardDisk:games::
 *   
 *   VMS exmaples:
 *   
 *.    [home.mjr] + deep.gam -> [home.mjr]deep.gam
 *.    [home.mjr] + [-] -> [home.mjr.-]
 */
void os_combine_paths(char *fullpathbuf, size_t pathbuflen,
                      const char *path, const char *filename);


/*
 *   Get the absolute, fully qualified filename for a file.  This fills in
 *   'result_buf' with the absolute path to the given file, taking into
 *   account the current working directory and any other implied environment
 *   information that affects the way the file system would resolve the given
 *   file name to a specific file on disk if we opened the file now using
 *   this name.
 *   
 *   The returned path should be in absolute path form, meaning that it's
 *   independent of the current working directory or any other environment
 *   settings.  That is, this path should still refer to the same file even
 *   if the working directory changes.
 *   
 *   Note that it's valid to get the absolute path for a file that doesn't
 *   exist, or for a path with directory components that don't exist.  For
 *   example, a caller might generate the absolute path for a file that it's
 *   about to create, or a hypothetical filename for path comparison
 *   purposes.  The function should succeed even if the file or any path
 *   components don't exist.  If the file is in relative format, and any path
 *   elements don't exist but are syntactically well-formed, the result
 *   should be the path obtained from syntactically combining the working
 *   directory with the relative path.
 *   
 *   On many systems, a given file might be reachable through more than one
 *   absolute path.  For example, on Unix it might be possible to reach a
 *   file through symbolic links to the file itself or to parent directories,
 *   or hard links to the file.  It's up to the implementation to determine
 *   which path to use in such cases.
 *   
 *   On success, returns true.  If it's not possible to resolve the file name
 *   to an absolute path, the routine copies the original filename to the
 *   result buffer exactly as given, and returns false.  
 */
int os_get_abs_filename(char *result_buf, size_t result_buf_size,
                        const char *filename);

/*
 *   Get the relative version of the given filename path 'filename', relative
 *   to the given base directory 'basepath'.  Both paths must be given in
 *   absolute format.
 *   
 *   Returns true on success, false if it's not possible to rewrite the path
 *   in relative terms.  For example, on Windows, it's not possible to
 *   express a path on the "D:" drive as relative to a base path on the "C:"
 *   drive, since each drive letter has an independent root folder; there's
 *   no namespace entity enclosing a drive letter's root folder.  On
 *   Unix-like systems where the entire namespace has a single hierarchical
 *   root, it should always be possible to express any path relative to any
 *   other.
 *   
 *   The result should be a relative path that can be combined with
 *   'basepath' using os_build_full_path() to reconstruct a path that
 *   identifies the same file as the original 'filename' (it's not important
 *   that this procedure would result in the identical string - it just has
 *   to point to the same file).  If it's not possible to express the
 *   filename relative to the base path, fill in 'result_buf' with the
 *   original filename and return false.
 *   
 *   Windows examples:
 *   
 *.    c:\mjr\games | c:\mjr\games\deep.gam  -> deep.gam
 *.    c:\mjr\games | c:\mjr\games\tads\deep.gam  -> tads\deep.gam
 *.    c:\mjr\games | c:\mjr\tads\deep.gam  -> ..\tads\deep.gam
 *.    c:\mjr\games | d:\deep.gam  ->  d:\deep.gam (and return false)
 *   
 *   Mac OS examples:
 *   
 *.    Mac HD:mjr:games | Mac HD:mjr:games:deep.gam -> deep.gam
 *.    Mac HD:mjr:games | Mac HD:mjr:games:tads:deep.gam -> :tads:deep.gam
 *.    Mac HD:mjr:games | Ext Disk:deep.gam -> Ext Disk:deep.gam (return false)
 *   
 *   VMS examples:
 *   
 *.    SYS$:[mjr.games] | SYS$:[mjr.games]deep.gam -> deep.gam
 *.    SYS$:[mjr.games] | SYS$:[mjr.games.tads]deep.gam -> [.tads]deep.gam
 *.    SYS$:[mjr.games] | SYS$:[mjr.tads]deep.gam -> [-.tads]deep.gam
 *.    SYS$:[mjr.games] | DISK$:[mjr]deep.gam -> DISK$[mjr]deep.gam (ret false)
 */
int os_get_rel_path(char *result_buf, size_t result_buf_size,
                    const char *basepath, const char *filename);

/*
 *   Determine if the given file is in the given directory.  Returns true if
 *   so, false if not.  'filename' is a relative or absolute file name;
 *   'path' is a relative or absolute directory path, such as one returned
 *   from os_get_path_name().
 *   
 *   If 'include_subdirs' is true, the function returns true if the file is
 *   either directly in the directory 'path', OR it's in any subdirectory of
 *   'path'.  If 'include_subdirs' is false, the function returns true only
 *   if the file is directly in the given directory.
 *   
 *   If 'match_self' is true, the function returns true if 'filename' and
 *   'path' are the same directory; otherwise it returns false in this case.
 *   
 *   This routine is allowed to return "false negatives" - that is, it can
 *   claim that the file isn't in the given directory even when it actually
 *   is.  The reason is that it's not always possible to determine for sure
 *   that there's not some way for a given file path to end up in the given
 *   directory.  In contrast, a positive return must be reliable.
 *   
 *   If possible, this routine should fully resolve the names through the
 *   file system to determine the path relationship, rather than merely
 *   analyzing the text superficially.  This can be important because many
 *   systems have multiple ways to reach a given file, such as via symbolic
 *   links on Unix; analyzing the syntax alone wouldn't reveal these multiple
 *   pathways.
 *   
 *   SECURITY NOTE: If possible, implementations should fully resolve all
 *   symbolic links, relative paths (e.g., Unix ".."), etc, before rendering
 *   judgment.  One important application for this routine is to determine if
 *   a file is in a sandbox directory, to enforce security restrictions that
 *   prevent a program from accessing files outside of a designated folder.
 *   If the implementation fails to resolve symbolic links or relative paths,
 *   a malicious program or user could bypass the security restriction by,
 *   for example, creating a symbolic link within the sandbox directory that
 *   points to the root folder.  Implementations can avoid this loophole by
 *   converting the file and directory names to absolute paths and resolving
 *   all symbolic links and relative notation before comparing the paths.
 */
int os_is_file_in_dir(const char *filename, const char *path,
                      int include_subdirs, int match_self);


/* ------------------------------------------------------------------------ */
/*
 *   Convert an OS filename path to URL-style format.  This isn't a true URL
 *   conversion; rather, it simply expresses a filename in Unix-style
 *   notation, as a series of path elements separated by '/' characters.
 *   Unlike true URLs, we don't use % encoding or a scheme prefix (file://,
 *   etc).
 *   
 *   The result path never ends in a trailing '/', unless the entire result
 *   path is "/".  This is for consistency; even if the source path ends with
 *   a local path separator, the result doesn't.
 *   
 *   If the local file system syntax uses '/' characters as ordinary filename
 *   characters, these must be replaced with some other suitable character in
 *   the result, since otherwise they'd be taken as path separators when the
 *   URL is parsed.  If possible, the substitution should be reversible with
 *   respect to os_cvt_dir_url(), so that the same URL read back in on this
 *   same platform will produce the same original filename.  One particular
 *   suggestion is that if the local system uses '/' to delimit what would be
 *   a filename extension on other platforms, replace '/' with '.', since
 *   this will provide reversibility as well as a good mapping if the URL is
 *   read back in on another platform.
 *   
 *   The local equivalents of "." and "..", if they exist, are converted to
 *   "." and ".." in the URL notation.
 *   
 *   Examples:
 *   
 *.   Windows: images\rooms\startroom.jpg -> images/rooms/startroom.jpg
 *.   Windows: ..\startroom.jpg -> ../startroom.jpg
 *.   Mac:     :images:rooms:startroom.jpg -> images/rooms/startroom.jpg
 *.   Mac:     ::startroom.jpg -> ../startroom.jpg
 *.   VMS:     [.images.rooms]startroom.jpg -> images/rooms/startroom.jpg
 *.   VMS:     [-.images]startroom.jpg -> ../images/startroom.jpg
 *.   Unix:    images/rooms/startroom.jpg -> images/rooms/startroom.jpg
 *.   Unix:    ../images/startroom.jpg -> ../images/startroom.jpg
 *   
 *   If the local name is an absolute path in the local file system (e.g.,
 *   Unix /file, Windows C:\file), translate as follows.  If the local
 *   operating system uses a volume or device designator (Windows C:, VMS
 *   SYS$DISK:, etc), make the first element of the path the exact local
 *   syntax for the device designator: /C:/ on Windows, /SYS$DISK:/ on VMS,
 *   etc.  Include the local syntax for the device prefix.  For a system like
 *   Unix with a unified file system root ("/"), simply start with the root
 *   directory.  Examples:
 *   
 *.    Windows:  C:\games\deep.gam         -> /C:/games/deep.gam
 *.    Windows:  C:games\deep.gam          -> /C:./games/deep.gam
 *.    Windows:  \\SERVER\DISK\games\deep.gam -> /\\SERVER/DISK/games/deep.gam
 *.    Mac OS 9: Hard Disk:games:deep.gam  -> /Hard Disk:/games/deep.gam
 *.    VMS:      SYS$DISK:[games]deep.gam  -> /SYS$DISK:/games/deep.gam
 *.    Unix:     /games/deep.gam           -> /games/deep.gam
 *   
 *   Rationale: it's effectively impossible to create a truly portable
 *   representation of an absolute path.  Operating systems are too different
 *   in the way they represent root paths, and even if that were solvable, a
 *   root path is essentially unusable across machines anyway because it
 *   creates a dependency on the contents of a particular machine's disk.  So
 *   if we're called upon to translate an absolute path, we can forget about
 *   trying to be truly portable and instead focus on round-trip fidelity -
 *   i.e., making sure that applying os_cvt_url_dir() to our result recovers
 *   the exact original path, assuming it's done on the same operating
 *   system.  The approach outlined above should achieve round-trip fidelity
 *   when a local path is converted to a URL and back on the same machine,
 *   since the local URL-to-path converter should recognize its own special
 *   type of local absolute path prefix.  It also produces reasonable results
 *   on other platforms - see the os_cvt_url_dir() comments below for
 *   examples of the decoding results for absolute paths moved to new
 *   platforms.  The result when a device-rooted absolute path is encoded on
 *   one machine and then decoded on another will generally be a local path
 *   with a root on the default device/volume and an outermost directory with
 *   a name based on the original machine's device/volume name.  This
 *   obviously won't reproduce the exact original path, but since that's
 *   impossible anyway, this is probably as good an approximation as we can
 *   create.
 *   
 *   Character sets: the input could be in local or UTF-8 character sets.
 *   The implementation shouldn't care, though - just treat bytes in the
 *   range 0-127 as plain ASCII, and everything else as opaque.  I.e., do not
 *   quote or otherwise modify characters outside the 0-127 range.  
 */
void os_cvt_dir_url(char *result_buf, size_t result_buf_size,
                    const char *src_path);

/*
 *   Convert a URL-style path into a filename path expressed in the local
 *   file system's syntax.  Fills in result_buf with a file path, constructed
 *   using the local file system syntax, that corresponds to the path in
 *   src_url expressed in URL-style syntax.  Examples:
 *   
 *   images/rooms/startroom.jpg -> 
 *.   Windows   -> images\rooms\startroom.jpg
 *.   Mac OS 9  -> :images:rooms:startroom.jpg
 *.   VMS       -> [.images.rooms]startroom.jpg
 *   
 *   The source format isn't a true URL; it's simply a series of path
 *   elements separated by '/' characters.  Unlike true URLs, our input
 *   format doesn't use % encoding and doesn't have a scheme (file://, etc).
 *   (Any % in the source is treated as an ordinary character and left as-is,
 *   even if it looks like a %XX sequence.  Anything that looks like a scheme
 *   prefix is left as-is, with any // treated as path separators.
 *   
 *   images/file%20name.jpg ->
 *.   Windows   -> images\file%20name.jpg
 *   
 *   file://images/file.jpg ->
 *.   Windows   -> file_\\images\file.jpg
 *   
 *   Any characters in the path that are invalid in the local file system
 *   naming rules are converted to "_", unless "_" is itself invalid, in
 *   which case they're converted to "X".  One exception is that if '/' is a
 *   valid local filename character (rather than a path separator as it is on
 *   Unix and Windows), it can be used as the replacement for the character
 *   that os_cvt_dir_url uses as its replacement for '/', so that this
 *   substitution is reversible when a URL is generated and then read back in
 *   on this same platform.
 *   
 *   images/file:name.jpg ->
 *.   Windows   -> images\file_name.jpg
 *.   Mac OS 9  -> :images:file_name.jpg
 *.   Unix      -> images/file:name.jpg
 *   
 *   The path elements "." and ".." are specifically defined as having their
 *   Unix meanings: "." is an alias for the preceding path element, or the
 *   working directory if it's the first element, and ".." is an alias for
 *   the parent of the preceding element.  When these appear as path
 *   elements, this routine translates them to the appropriate local
 *   conventions.  "." may be translated simply by removing it from the path,
 *   since it reiterates the previous path element.  ".." may be translated
 *   by removing the previous element - HOWEVER, if ".." appears as the first
 *   element, it has to be retained and translated to the equivalent local
 *   notation, since it will have to be applied later, when the result_buf
 *   path is actually used to open a file, at which point it will combined
 *   with the working directory or another base path.
 *   
 *.  /images/../file.jpg -> [Windows] file.jpg
 *.  ../images/file.jpg ->
 *.   Windows  -> ..\images\file.jpg
 *.   Mac OS 9 -> ::images:file.jpg
 *.   VMS      -> [-.images]file.jpg
 *   
 *   If the URL path is absolute (starts with a '/'), the routine inspects
 *   the path to see if it was created by the same OS, according to the local
 *   rules for converting absolute paths in os_cvt_dir_url() (see).  If so,
 *   we reverse the encoding done there.  If it doesn't appear that the name
 *   was created by the same operating system - that is, if reversing the
 *   encoding doesn't produce a valid local filename - then we create a local
 *   absolute path as follows.  If the local system uses device/volume
 *   designators, we start with the current working device/volume or some
 *   other suitable default volume.  We then add the first element of the
 *   path, if any, as the root directory name, applying the usual "_" or "X"
 *   substitution for any characters that aren't allowed in local names.  The
 *   rest of the path is handled in the usual fashion.
 *   
 *.  /images/file.jpg ->
 *.    Windows -> \images\file.jpg
 *.    Unix    -> /images/file.jpg
 *   
 *.  /c:/images/file.jpg ->
 *.    Windows -> c:\images\file.jpg
 *.    Unix    -> /c:/images/file.jpg
 *.    VMS     -> SYS$DISK:[c__.images]file.jpg
 *   
 *.  /Hard Disk:/images/file.jpg ->
 *.    Windows -> \Hard Disk_\images\file.jpg
 *.    Unix    -> SYS$DISK:[Hard_Disk_.images]file.jpg
 *   
 *   Note how the device/volume prefix becomes the top-level directory when
 *   moving a path across machines.  It's simply not possible to reconstruct
 *   the exact original path in such cases, since device/volume syntax rules
 *   have little in common across systems.  But this seems like a good
 *   approximation in that (a) it produces a valid local path, and (b) it
 *   gives the user a reasonable basis for creating a set of folders to mimic
 *   the original source system, if they want to use that approach to port
 *   the data rather than just changing the paths internally in the source
 *   material.
 *   
 *   Character sets: use the same rules as for os_cvt_dir_url().  
 */
void os_cvt_url_dir(char *result_buf, size_t result_buf_size,
                    const char *src_url);


/* ------------------------------------------------------------------------ */
/*
 *   Get a suitable seed for a random number generator; should use the system
 *   clock or some other source of an unpredictable and changing seed value.
 */
void os_rand(long *val);

/*
 *   Generate random bytes for use in seeding a PRNG (pseudo-random number
 *   generator).  This is an extended version of os_rand() for PRNGs that use
 *   large seed vectors containing many bytes, rather than the simple 32-bit
 *   seed that os_rand() assumes.
 *   
 *   As with os_rand(), this function isn't meant to be used directly as a
 *   random number source for ongoing use - instead, this is intended mostly
 *   for seeding a PRNG, which will then be used as the primary source of
 *   random numbers.  The big problem with using this routine directly as a
 *   randomness source is that some implementations might rely heavily on
 *   environmental randomness, such as the real-time clock or OS usage
 *   statistics.  Such sources tend to provide reasonable entropy from one
 *   run to the next, but not within a single session, as the underlying data
 *   sources don't change rapidly enough.
 *   
 *   Ideally, this routine should generate *truly* random bytes obtained from
 *   hardware sources.  Not all systems can provide that, though, so true
 *   randomness isn't guaranteed.  Here are the suggested implementation
 *   options, in descending order of desirability:
 *   
 *   1.  Use a hardware source of true randomness, such as a /dev/rand type
 *   of device.  However, note that this call should return reasonably
 *   quickly, so always use a non-blocking source.  Some Unix /dev/rand
 *   devices, for example, can block indefinitely to allow sufficient entropy
 *   to accumulate.
 *   
 *   2. Use a cryptographic random number source provided by the OS.  Some
 *   systems provide this as an API service.  If going this route, be sure
 *   that the OS generator is itself "seeded" with some kind of true
 *   randomness source, as it defeats the whole purpose if you always return
 *   a fixed pseudo-random sequence each time the program runs.
 *   
 *   3. Use whatever true random sources are available locally to seed a
 *   software pseudo-random number generator, then generate bytes from your
 *   PRNG.  Some commonly available sources of true randomness are a
 *   high-resolution timer, the system clock, the current process ID,
 *   logged-in user ID, environment variables, uninitialized pages of memory,
 *   the IP address; each of these sources might give you a few bits of
 *   entropy at best, so the best bet is to use an ensemble.  You could, for
 *   example, concatenate a bunch of this type of information together and
 *   calculate an MD5 or SHA1 hash to mix the bits more thoroughly.  For the
 *   PRNG, use a cryptographic generator.  (You could use the ISAAC generator
 *   from TADS 3, as that's a crypto PRNG, but it's probably better to use a
 *   different generator here since TADS 3 is going to turn around and use
 *   this function's output to seed ISAAC - seeding one ISAAC instance with
 *   another ISAAC's output seems likely to magnify any weaknesses in the
 *   ISAAC algorithm.)  Note that this option is basically the DIY version of
 *   option 2.  Option 2 is better because the OS probably has access to
 *   better sources of true randomness than an application does.  
 */
void os_gen_rand_bytes(unsigned char *buf, size_t len);


/* ------------------------------------------------------------------------ */
/*
 *   Display routines.
 *   
 *   Our display model is a simple stdio-style character stream.
 *   
 *   In addition, we provide an optional "status line," which is a
 *   non-scrolling area where a line of text can be displayed.  If the status
 *   line is supported, text should only be displayed in this area when
 *   os_status() is used to enter status-line mode (mode 1); while in status
 *   line mode, text is written to the status line area, otherwise (mode 0)
 *   it's written to the normal main text area.  The status line is normally
 *   shown in a different color to set it off from the rest of the text.
 *   
 *   The OS layer can provide its own formatting (word wrapping in
 *   particular) if it wants, in which case it should also provide pagination
 *   using os_more_prompt().  
 */

/*
 *   OS_MAXWIDTH - the maximum width of a line of text.  Most platforms use
 *   135 for this, but you can use more or less as appropriate.  If you use
 *   OS-level line wrapping, then the true width of a text line is
 *   irrelevant, and the portable code will use this merely for setting its
 *   internal buffer sizes.
 *   
 *   This must be defined in the os_xxx.h header file for each platform.
 */
/*#define OS_MAXWIDTH 135 - example only: define for real in os_xxx.h header*/

/*
 *   Print a string on the console.  These routines come in two varieties:
 *   
 *   os_printz - write a NULL-TERMINATED string
 *.  os_print - write a COUNTED-LENGTH string, which may not end with a null
 *   
 *   These two routines are identical except that os_printz() takes a string
 *   which is terminated by a null byte, and os_print() instead takes an
 *   explicit length, and a string that may not end with a null byte.
 *   
 *   os_printz(str) may be implemented as simply os_print(str, strlen(str)).
 *   
 *   The string is written in one of three ways, depending on the status mode
 *   set by os_status():
 *   
 *   status mode == 0 -> write to main text window
 *.  status mode == 1 -> write to status line
 *.  anything else -> do not display the text at all
 *   
 *   Implementations are free to omit any status line support, in which case
 *   they should simply suppress all output when the status mode is anything
 *   other than zero.
 *   
 *   The following special characters must be recognized in the displayed
 *   text:
 *   
 *   '\n' - newline: end the current line and move the cursor to the start of
 *   the next line.  If the status line is supported, and the current status
 *   mode is 1 (i.e., displaying in the status line), then two special rules
 *   apply to newline handling: newlines preceding any other text should be
 *   ignored, and a newline following any other text should set the status
 *   mode to 2, so that all subsequent output is suppressed until the status
 *   mode is changed with an explicit call by the client program to
 *   os_status().
 *   
 *   '\r' - carriage return: end the current line and move the cursor back to
 *   the beginning of the current line.  Subsequent output is expected to
 *   overwrite the text previously on this same line.  The implementation
 *   may, if desired, IMMEDIATELY clear the previous text when the '\r' is
 *   written, rather than waiting for subsequent text to be displayed.
 *   
 *   All other characters may be assumed to be ordinary printing characters.
 *   The routine need not check for any other special characters.
 *   
 */
void os_printz(const char *str);
void os_print(const char *str, size_t len);

/*
 *   Print to the debugger console.  These routines are for interactive
 *   debugger builds only: they display the given text to a separate window
 *   within the debugger UI (separate from the main game command window)
 *   where the debugger displays status information specific to the debugging
 *   session (such as compiler/build output, breakpoint status messages,
 *   etc).  For example, TADS Workbench on Windows displays these messages in
 *   its "Debug Log" window.
 *   
 *   These routines only need to be implemented for interactive debugger
 *   builds, such as TADS Workbench on Windows.  These can be omitted for
 *   regular interpreter builds.  
 */
void os_dbg_printf(const char *fmt, ...);
void os_dbg_vprintf(const char *fmt, va_list args);

/*
 *   Allocating sprintf and vsprintf.  These work like the regular C library
 *   sprintf and vsprintf funtions, but they allocate a return buffer that's
 *   big enough to hold the result, rather than formatting into a caller's
 *   buffer.  This protects against buffer overruns and ensures that the
 *   result isn't truncated.
 *   
 *   On return, '*bufptr' is filled in with a pointer to a buffer allocated
 *   with osmalloc().  This buffer contains the formatted string result.  The
 *   caller is responsible for freeing the buffer by calling osfree().
 *   *bufptr can be null on return if an error occurs.
 *   
 *   The return value is the number of bytes written to the allocated buffer,
 *   not including the null terminator.  If an error occurs, the return value
 *   is -1 and *bufptr is undefined.
 *   
 *   Many modern C libraries provide equivalents of these, usually called
 *   asprintf() and vasprintf(), respectively.  
 */
/* int os_asprintf(char **bufptr, const char *fmt, ...); */
int os_vasprintf(char **bufptr, const char *fmt, va_list ap);


/* 
 *   Set the status line mode.  There are three possible settings:
 *   
 *   0 -> main text mode.  In this mode, all subsequent text written with
 *   os_print() and os_printz() is to be displayed to the main text area.
 *   This is the normal mode that should be in effect initially.  This mode
 *   stays in effect until an explicit call to os_status().
 *   
 *   1 -> statusline mode.  In this mode, text written with os_print() and
 *   os_printz() is written to the status line, which is usually rendered as
 *   a one-line area across the top of the terminal screen or application
 *   window.  In statusline mode, leading newlines ('\n' characters) are to
 *   be ignored, and any newline following any other character must change
 *   the mode to 2, as though os_status(2) had been called.
 *   
 *   2 -> suppress mode.  In this mode, all text written with os_print() and
 *   os_printz() must simply be ignored, and not displayed at all.  This mode
 *   stays in effect until an explicit call to os_status().  
 */
void os_status(int stat);

/* get the status line mode */
int os_get_status();

/* 
 *   Set the score value.  This displays the given score and turn counts on
 *   the status line.  In most cases, these values are displayed at the right
 *   edge of the status line, in the format "score/turns", but the format is
 *   up to the implementation to determine.  In most cases, this can simply
 *   be implemented as follows:
 *   
 *.  void os_score(int score, int turncount)
 *.  {
 *.     char buf[40];
 *.     sprintf(buf, "%d/%d", score, turncount);
 *.     os_strsc(buf);
 *.  }
 */
void os_score(int score, int turncount);

/* display a string in the score area in the status line */
void os_strsc(const char *p);

/* clear the screen */
void oscls(void);

/* redraw the screen */
void os_redraw(void);

/* flush any buffered display output */
void os_flush(void);

/*
 *   Update the display - process any pending drawing immediately.  This
 *   only needs to be implemented for operating systems that use
 *   event-driven drawing based on window invalidations; the Windows and
 *   Macintosh GUI's both use this method for drawing window contents.
 *   
 *   The purpose of this routine is to refresh the display prior to a
 *   potentially long-running computation, to avoid the appearance that the
 *   application is frozen during the computation delay.
 *   
 *   Platforms that don't need to process events in the main thread in order
 *   to draw their window contents do not need to do anything here.  In
 *   particular, text-mode implementations generally don't need to implement
 *   this routine.
 *   
 *   This routine doesn't absolutely need a non-empty implementation on any
 *   platform, but it will provide better visual feedback if implemented for
 *   those platforms that do use event-driven drawing.  
 */
void os_update_display();


/* ------------------------------------------------------------------------ */
/*
 *   Set text attributes.  Text subsequently displayed through os_print() and
 *   os_printz() are to be displayed with the given attributes.
 *   
 *   'attr' is a (bitwise-OR'd) combination of OS_ATTR_xxx values.  A value
 *   of zero indicates normal text, with no extra attributes.  
 */
void os_set_text_attr(int attr);

/* attribute code: bold-face */
#define OS_ATTR_BOLD     0x0001

/* attribute code: italic */
#define OS_ATTR_ITALIC   0x0002

/*
 *   Abstract attribute codes.  Each platform can choose a custom rendering
 *   for these by #defining them before this point, in the OS-specific header
 *   (osdos.h, osmac.h, etc).  We provide *default* definitions in case the
 *   platform doesn't define these.
 *   
 *   For compatibility with past versions, we treat HILITE, EM, and BOLD as
 *   equivalent.  Platforms that can display multiple kinds of text
 *   attributes (boldface and italic, say) should feel free to use more
 *   conventional HTML mappings, such as EM->italic and STRONG->bold.  
 */

/* 
 *   "Highlighted" text, as appropriate to the local platform.  On most
 *   text-mode platforms, the only kind of rendering variation possible is a
 *   brighter or intensified color.  If actual bold-face is available, that
 *   can be used instead.  This is the attribute used for text enclosed in a
 *   TADS2 "\( \)" sequence.  
 */
#ifndef OS_ATTR_HILITE
# define OS_ATTR_HILITE  OS_ATTR_BOLD
#endif

/* HTML <em> attribute - by default, map this to bold-face */
#ifndef OS_ATTR_EM
# define OS_ATTR_EM      OS_ATTR_BOLD
#endif

/* HTML <strong> attribute - by default, this has no effect */
#ifndef OS_ATTR_STRONG
# define OS_ATTR_STRONG  0
#endif


/* ------------------------------------------------------------------------ */
/*
 *   Colors.
 *   
 *   There are two ways of encoding a color.  First, a specific color can be
 *   specified as an RGB (red-green-blue) value, with discreet levels for
 *   each component's intensity, ranging from 0 to 255.  Second, a color can
 *   be "parameterized," which doesn't specify a color in absolute terms but
 *   rather specified one of a number of pre-defined *types* of colors;
 *   these pre-defined types can be chosen by the OS implementation, or, on
 *   some systems, selected by the user via a preferences mechanism.
 *   
 *   The os_color_t type encodes a color in 32 bits.  The high-order 8 bits
 *   of a color value give the parameterized color identifier, or are set to
 *   zero to indicate an RGB color.  An RGB color is encoded in the
 *   low-order 24 bits, via the following formula:
 *   
 *   (R << 16) + (G << 8) + B
 *   
 *   R specifies the intensity of the red component of the color, G green,
 *   and B blue.  Each of R, G, and B must be in the range 0-255.  
 */
typedef unsigned long os_color_t;

/* encode an R, G, B triplet into an os_color_t value */
#define os_rgb_color(r, g, b) (((r) << 16) + ((g) << 8) + (b))

/* 
 *   Determine if a color is given as an RGB value or as a parameterized
 *   color value.  Returns true if the color is given as a parameterized
 *   color (one of the OS_COLOR_xxx values), false if it's given as an
 *   absolute RGB value.  
 */
#define os_color_is_param(color) (((color) & 0xFF000000) != 0)

/* get the red/green/blue components of an os_color_t value */
#define os_color_get_r(color) ((int)(((color) >> 16) & 0xFF))
#define os_color_get_g(color) ((int)(((color) >> 8) & 0xFF))
#define os_color_get_b(color) ((int)((color) & 0xFF))

/*
 *   Parameterized color codes.  These are os_color_t values that indicate
 *   colors by type, rather than by absolute RGB values.  
 */

/* 
 *   "transparent" - applicable to backgrounds only, this specifies that the
 *   current screen background color should be used 
 */
#define OS_COLOR_P_TRANSPARENT ((os_color_t)0x01000000)

/* "normal text" color (as set via user preferences, if applicable) */
#define OS_COLOR_P_TEXT        ((os_color_t)0x02000000)

/* normal text background color (from user preferences) */
#define OS_COLOR_P_TEXTBG      ((os_color_t)0x03000000)

/* "status line" text color (as set via user preferences, if applicable) */
#define OS_COLOR_P_STATUSLINE  ((os_color_t)0x04000000)

/* status line background color (from user preferences) */
#define OS_COLOR_P_STATUSBG    ((os_color_t)0x05000000)

/* input text color (as set via user preferences, if applicable) */
#define OS_COLOR_P_INPUT       ((os_color_t)0x06000000)

/*
 *   Set the text foreground and background colors.  This sets the text
 *   color for subsequent os_printf() and os_vprintf() calls.
 *   
 *   The background color can be OS_COLOR_TRANSPARENT, in which case the
 *   background color is "inherited" from the current screen background.
 *   Note that if the platform is capable of keeping old text for
 *   "scrollback," then the transparency should be a permanent attribute of
 *   the character - in other words, it should not be mapped to the current
 *   screen color in the scrollback buffer, because doing so would keep the
 *   current screen color even if the screen color changes in the future. 
 *   
 *   Text color support is optional.  If the platform doesn't support text
 *   colors, this can simply do nothing.  If the platform supports text
 *   colors, but the requested color or attributes cannot be displayed, the
 *   implementation should use the best available approximation.  
 */
void os_set_text_color(os_color_t fg, os_color_t bg);

/*
 *   Set the screen background color.  This sets the text color for the
 *   background of the screen.  If possible, this should immediately redraw
 *   the main text area with this background color.  The color is given as an
 *   OS_COLOR_xxx value.
 *   
 *   If the platform is capable of redisplaying the existing text, then any
 *   existing text that was originally displayed with 'transparent'
 *   background color should be redisplayed with the new screen background
 *   color.  In other words, the 'transparent' background color of previously
 *   drawn text should be a permanent attribute of the character - the color
 *   should not be mapped on display to the then-current background color,
 *   because doing so would lose the transparency and thus retain the old
 *   screen color on a screen color change.  
 */
void os_set_screen_color(os_color_t color);


/* ------------------------------------------------------------------------ */
/* 
 *   os_plain() - Use plain ascii mode for the display.  If possible and
 *   necessary, turn off any text formatting effects, such as cursor
 *   positioning, highlighting, or coloring.  If this routine is called,
 *   the terminal should be treated as a simple text stream; users might
 *   wish to use this mode for things like text-to-speech converters.
 *   
 *   Purely graphical implementations that cannot offer a textual mode
 *   (such as Mac OS or Windows) can ignore this setting.
 *   
 *   If this routine is to be called, it must be called BEFORE os_init().
 *   The implementation should set a flag so that os_init() will know to
 *   set up the terminal for plain text output.  
 */
#ifndef os_plain
/* 
 *   some platforms (e.g. Mac OS) define this to be a null macro, so don't
 *   define a prototype in those cases 
 */
void os_plain(void);
#endif

/*
 *   Set the game title.  The output layer calls this routine when a game
 *   sets its title (via an HTML <title> tag, for example).  If it's
 *   convenient to do so, the OS layer can use this string to set a window
 *   caption, or whatever else makes sense on each system.  Most
 *   character-mode implementations will provide an empty implementation,
 *   since there's not usually any standard way to show the current
 *   application title on a character-mode display.  
 */
void os_set_title(const char *title);

/*
 *   Show the system-specific MORE prompt, and wait for the user to respond.
 *   Before returning, remove the MORE prompt from the screen.
 *   
 *   This routine is only used and only needs to be implemented when the OS
 *   layer takes responsibility for pagination; this will be the case on
 *   most systems that use proportionally-spaced (variable-pitch) fonts or
 *   variable-sized windows, since on such platforms the OS layer must do
 *   most of the formatting work, leaving the standard output layer unable
 *   to guess where pagination should occur.
 *   
 *   If the portable output formatter handles the MORE prompt, which is the
 *   usual case for character-mode or terminal-style implementations, this
 *   routine is not used and you don't need to provide an implementation.
 *   Note that HTML TADS provides an implementation of this routine, because
 *   the HTML renderer handles line breaking and thus must handle
 *   pagination.  
 */
void os_more_prompt();

/*
 *   Interpreter Class Configuration.
 *   
 *   If this is a TEXT-ONLY interpreter: DO NOT define USE_HTML.
 *   
 *   If this is a MULTIMEDIA (HTML TADS) intepreter: #define USE_HTML
 *   
 *   (This really should be called something like OS_USE_HTML - the USE_ name
 *   is for historical reasons.  This purpose of this macro is to configure
 *   the tads 2 VM-level output formatter's line breaking and MORE mode
 *   behavior.  In HTML mode, the VM-level formatter knows that it's feeding
 *   its output to a page layout engine, so the VM-level output is just a
 *   text stream.  In plain-text mode, the VM formatter *is* the page layout
 *   engine, so it needs to do all of the word wrapping and MORE prompting
 *   itself.  The tads 3 output layer does NOT use this macro for its
 *   equivalent configuration, but instead has different .cpp files for the
 *   different modes, and you simply link in the one for the configuration
 *   you want.)  
 */
/* #define USE_HTML */


/*
 *   Enter HTML mode.  This is only used when the run-time is compiled
 *   with the USE_HTML flag defined.  This call instructs the renderer
 *   that HTML sequences should be parsed; until this call is made, the
 *   renderer should not interpret output as HTML.  Non-HTML
 *   implementations do not need to define this routine, since the
 *   run-time will not call it if USE_HTML is not defined.  
 */
void os_start_html(void);

/* exit HTML mode */
void os_end_html(void);

/*
 *   Global variables with the height and width (in character cells - rows
 *   and columns) of the main text display area into which os_printf
 *   displays.  The height and width are given in text lines and character
 *   columns, respectively.  The portable code can use these values to
 *   format text for display via os_printf(); for example, the caller can
 *   use the width to determine where to put line breaks.
 *   
 *   These values are only needed for systems where os_printf() doesn't
 *   perform its own word-wrap formatting.  On systems such as the Mac,
 *   where os_printf() performs word wrapping, these sizes aren't really
 *   important because the portable code doesn't need to perform any real
 *   formatting.
 *   
 *   These variables reflect the size of the "main text area," which is the
 *   area of the screen excluding the status line and any "banner" windows
 *   (as created with the os_banner_xxx() interfaces).
 *   
 *   The OS code must initialize these variables during start-up, and must
 *   adjust them whenever the display size is changed by user action or
 *   other external events (for example, if we're running inside a terminal
 *   window, and the user resizes the window, the OS code must recalculate
 *   the layout and adjust these accordingly).  
 */
extern int G_os_pagelength;
extern int G_os_linewidth;

/*
 *   Global flag that tells the output formatter whether to count lines
 *   that it's displaying against the total on the screen so far.  If this
 *   variable is true, lines are counted, and the screen is paused with a
 *   [More] message when it's full.  When not in MORE mode, lines aren't
 *   counted.  This variable should be set to false when displaying text
 *   that doesn't count against the current page, such as status line
 *   information.
 *   
 *   This flag should not be modified by OS code.  Instead, the output
 *   formatter will set this flag according to its current state; the OS
 *   code can use this flag to determine whether or not to display a MORE
 *   prompt during os_printf()-type operations.  Note that this flag is
 *   normally interesting to the OS code only when the OS code itself is
 *   handling the MORE prompt.  
 */
extern int G_os_moremode;

/*
 *   Global buffer containing the name of the byte-code file (the "game
 *   file") loaded into the VM.  This is used only where applicable, which
 *   generally means in TADS Interpreter builds.  In other application
 *   builds, this is simply left empty.  The application is responsible for
 *   setting this during start-up (or wherever else the byte-code filename
 *   becomes known or changes).  
 */
extern char G_os_gamename[OSFNMAX];

/*
 *   Set non-stop mode.  This tells the OS layer that it should disable any
 *   MORE prompting it would normally do.
 *   
 *   This routine is needed only when the OS layer handles MORE prompting; on
 *   character-mode platforms, where the prompting is handled in the portable
 *   console layer, this can be a dummy implementation.  
 */
void os_nonstop_mode(int flag);

/* 
 *   Update progress display with current info, if appropriate.  This can
 *   be used to provide a status display during compilation.  Most
 *   command-line implementations will just ignore this notification; this
 *   can be used for GUI compiler implementations to provide regular
 *   display updates during compilation to show the progress so far.  
 */
/* void os_progress(const char *fname, unsigned long linenum); */

/* 
 *   Set busy cursor.  If 'flag' is true, provide a visual representation
 *   that the system or application is busy doing work.  If 'flag' is
 *   false, remove any visual "busy" indication and show normal status.
 *   
 *   We provide a prototype here if your osxxx.h header file does not
 *   #define a macro for os_csr_busy.  On many systems, this function has
 *   no effect at all, so the osxxx.h header file simply #define's it to
 *   do an empty macro.  
 */
#ifndef os_csr_busy
void os_csr_busy(int flag);
#endif


/* ------------------------------------------------------------------------ */
/*
 *   User Input Routines
 */

/*
 *   Ask the user for a filename, using a system-dependent dialog or other
 *   mechanism.  Returns one of the OS_AFE_xxx status codes (see below).
 *   
 *   prompt_type is the type of prompt to provide -- this is one of the
 *   OS_AFP_xxx codes (see below).  The OS implementation doesn't need to
 *   pay any attention to this parameter, but it can be used if desired to
 *   determine the type of dialog to present if the system provides
 *   different types of dialogs for different types of operations.
 *   
 *   file_type is one of the OSFTxxx codes for system file type.  The OS
 *   implementation is free to ignore this information, but can use it to
 *   filter the list of files displayed if desired; this can also be used
 *   to apply a default suffix on systems that use suffixes to indicate
 *   file type.  If OSFTUNK is specified, it means that no filtering
 *   should be performed, and no default suffix should be applied.  
 */
int os_askfile(const char *prompt, char *fname_buf, int fname_buf_len,
               int prompt_type, os_filetype_t file_type);

/* 
 *   os_askfile status codes 
 */

/* success */
#define OS_AFE_SUCCESS  0 

/* 
 *   Generic failure - this is largely provided for compatibility with
 *   past versions, in which only zero and non-zero error codes were
 *   meaningful; since TRUE is defined as 1 on most platforms, we assume
 *   that 1 is probably the generic non-zero error code that most OS
 *   implementations have traditionally used.  In addition, this can be
 *   used to indicate any other error for which there is no more specific
 *   error code.  
 */
#define OS_AFE_FAILURE  1

/* user cancelled */
#define OS_AFE_CANCEL   2

/* 
 *   os_askfile prompt types
 *   
 *   Important note: do not change these values when porting TADS.  These
 *   values can be used by games, so they must be the same on all
 *   platforms.  
 */
#define OS_AFP_OPEN    1     /* choose an existing file to open for reading */
#define OS_AFP_SAVE    2          /* choose a filename for saving to a file */


/* 
 *   Read a string of input.  Fills in the buffer with a null-terminated
 *   string containing a line of text read from the standard input.  The
 *   returned string should NOT contain a trailing newline sequence.  On
 *   success, returns 'buf'; on failure, including end of file, returns a
 *   null pointer.  
 */
unsigned char *os_gets(unsigned char *buf, size_t bufl);

/*
 *   Read a string of input with an optional timeout.  This behaves like
 *   os_gets(), in that it allows the user to edit a line of text (ideally
 *   using the same editing keys that os_gets() does), showing the line of
 *   text under construction during editing.  This routine differs from
 *   os_gets() in that it returns if the given timeout interval expires
 *   before the user presses Return (or the local equivalent).
 *   
 *   If the user presses Return before the timeout expires, we store the
 *   command line in the given buffer, just as os_gets() would, and we return
 *   OS_EVT_LINE.  We also update the display in the same manner that
 *   os_gets() would, by moving the cursor to a new line and scrolling the
 *   displayed text as needed.
 *   
 *   If a timeout occurs before the user presses Return, we store the command
 *   line so far in the given buffer, statically store the cursor position,
 *   insert mode, buffer text, and anything else relevant to the editing
 *   state, and we return OS_EVT_TIMEOUT.
 *   
 *   If the implementation does not support the timeout operation, this
 *   routine should simply return OS_EVT_NOTIMEOUT immediately when called;
 *   the routine should not allow the user to perform any editing if the
 *   timeout is not supported.  Callers must use the ordinary os_gets()
 *   routine, which has no timeout capabilities, if the timeout is not
 *   supported.
 *   
 *   When we return OS_EVT_TIMEOUT, the caller is responsible for doing one
 *   of two things.
 *   
 *   The first possibility is that the caller performs some work that doesn't
 *   require any display operations (in other words, the caller doesn't
 *   invoke os_printf, os_getc, or anything else that would update the
 *   display), and then calls os_gets_timeout() again.  In this case, we will
 *   use the editing state that we statically stored before we returned
 *   OS_EVT_TIMEOUT to continue editing where we left off.  This allows the
 *   caller to perform some computation in the middle of user command editing
 *   without interrupting the user - the extra computation is transparent to
 *   the user, because we act as though we were still in the midst of the
 *   original editing.
 *   
 *   The second possibility is that the caller wants to update the display.
 *   In this case, the caller must call os_gets_cancel() BEFORE making any
 *   display changes.  Then, the caller must do any post-input work of its
 *   own, such as updating the display mode (for example, closing HTML font
 *   tags that were opened at the start of the input).  The caller is now
 *   free to do any display work it wants.
 *   
 *   If we have information stored from a previous call that was interrupted
 *   by a timeout, and os_gets_cancel(TRUE) was never called, we will resume
 *   editing where we left off when the cancelled call returned; this means
 *   that we'll restore the cursor position, insertion state, and anything
 *   else relevant.  Note that if os_gets_cancel(FALSE) was called, we must
 *   re-display the command line under construction, but if os_gets_cancel()
 *   was never called, we will not have to make any changes to the display at
 *   all.
 *   
 *   Note that when resuming an interrupted editing session (interrupted via
 *   os_gets_cancel()), the caller must re-display the prompt prior to
 *   invoking this routine.
 *   
 *   Note that we can return OS_EVT_EOF in addition to the other codes
 *   mentioned above.  OS_EVT_EOF indicates that an error occurred reading,
 *   which usually indicates that the application is being terminated or that
 *   some hardware error occurred reading the keyboard.  
 *   
 *   If 'use_timeout' is false, the timeout should be ignored.  Without a
 *   timeout, the function behaves the same as os_gets(), except that it will
 *   resume editing of a previously-interrupted command line if appropriate.
 *   (This difference is why the timeout is optional: a caller might not need
 *   a timeout, but might still want to resume a previous input that did time
 *   out, in which case the caller would invoke this routine with
 *   use_timeout==FALSE.  The regular os_gets() would not satisfy this need,
 *   because it cannot resume an interrupted input.)
 *   
 *   Note that a zero timeout has the same meaning as for os_get_event(): if
 *   input is available IMMEDIATELY, return the input, otherwise return
 *   immediately with the OS_EVT_TIMEOUT result code.  
 */
int os_gets_timeout(unsigned char *buf, size_t bufl,
                    unsigned long timeout_in_milliseconds, int use_timeout);

/*
 *   Cancel an interrupted editing session.  This MUST be called if any
 *   output is to be displayed after a call to os_gets_timeout() returns
 *   OS_EVT_TIMEOUT.
 *   
 *   'reset' indicates whether or not we will forget the input state saved
 *   by os_gets_timeout() when it last returned.  If 'reset' is true, we'll
 *   clear the input state, so that the next call to os_gets_timeout() will
 *   start with an empty input buffer.  If 'reset' is false, we will retain
 *   the previous input state, if any; this means that the next call to
 *   os_gets_timeout() will re-display the same input buffer that was under
 *   construction when it last returned.
 *   
 *   This routine need not be called if os_gets_timeout() is to be called
 *   again with no other output operations between the previous
 *   os_gets_timeout() call and the next one.
 *   
 *   Note that this routine needs only a trivial implementation when
 *   os_gets_timeout() is not supported (i.e., the function always returns
 *   OS_EVT_NOTIMEOUT).  
 */
void os_gets_cancel(int reset);

/* 
 *   Read a character from the keyboard.  For extended keystrokes, this
 *   function returns zero, and then returns the CMD_xxx code for the
 *   extended keystroke on the next call.  For example, if the user presses
 *   the up-arrow key, the first call to os_getc() should return 0, and the
 *   next call should return CMD_UP.  Refer to the CMD_xxx codes below.
 *   
 *   os_getc() should return a high-level, translated command code for
 *   command editing.  This means that, where a functional interpretation of
 *   a key and the raw key-cap interpretation both exist as CMD_xxx codes,
 *   the functional interpretation should be returned.  For example, on Unix,
 *   Ctrl-E is conventionally used in command editing to move to the end of
 *   the line, following Emacs key bindings.  Hence, os_getc() should return
 *   CMD_END for this keystroke, rather than a single 0x05 character (ASCII
 *   Ctrl-E), because CMD_END is the high-level command code for the
 *   operation.
 *   
 *   The translation ability of this function allows for system-dependent key
 *   mappings to functional meanings.  
 */
int os_getc(void);

/*
 *   Read a character from the keyboard, following the same protocol as
 *   os_getc() for CMD_xxx codes (i.e., when an extended keystroke is
 *   encountered, os_getc_raw() returns zero, then returns the CMD_xxx code
 *   on the subsequent call).
 *   
 *   This function differs from os_getc() in that this function returns the
 *   low-level, untranslated key code whenever possible.  This means that,
 *   when a functional interpretation of a key and the raw key-cap
 *   interpretation both exist as CMD_xxx codes, this function returns the
 *   key-cap interpretation.  For the Unix Ctrl-E example in the comments
 *   describing os_getc() above, this function should return 5 (the ASCII
 *   code for Ctrl-E), because the ASCII control character interpretation is
 *   the low-level key code.
 *   
 *   This function should return all control keys using their ASCII control
 *   codes, whenever possible.  Similarly, this function should return ASCII
 *   27 for the Escape key, if possible.  
 *   
 *   For keys for which there is no portable ASCII representation, this
 *   should return the CMD_xxx sequence.  So, this function acts exactly the
 *   same as os_getc() for arrow keys, function keys, and other special keys
 *   that have no ASCII representation.  This function returns a
 *   non-translated version ONLY when an ASCII representation exists - in
 *   practice, this means that this function and os_getc() vary only for CTRL
 *   keys and Escape.  
 */
int os_getc_raw(void);


/* wait for a character to become available from the keyboard */
void os_waitc(void);

/*
 *   Constants for os_getc() when returning commands.  When used for
 *   command line editing, special keys (arrows, END, etc.)  should cause
 *   os_getc() to return 0, and return the appropriate CMD_ value on the
 *   NEXT call.  Hence, os_getc() must keep the appropriate information
 *   around statically for the next call when a command key is issued.
 *   
 *   The comments indicate which CMD_xxx codes are "translated" codes and
 *   which are "raw"; the difference is that, when a particular keystroke
 *   could be interpreted as two different CMD_xxx codes, one translated
 *   and the other raw, os_getc() should always return the translated
 *   version of the key, and os_getc_raw() should return the raw version.
 */
#define CMD_UP    1                        /* move up/up arrow (translated) */
#define CMD_DOWN  2                    /* move down/down arrow (translated) */
#define CMD_RIGHT 3                  /* move right/right arrow (translated) */
#define CMD_LEFT  4                    /* move left/left arrow (translated) */
#define CMD_END   5              /* move cursor to end of line (translated) */
#define CMD_HOME  6            /* move cursor to start of line (translated) */
#define CMD_DEOL  7                   /* delete to end of line (translated) */
#define CMD_KILL  8                      /* delete entire line (translated) */
#define CMD_DEL   9                /* delete current character (translated) */
#define CMD_SCR   10                 /* toggle scrollback mode (translated) */
#define CMD_PGUP  11                                /* page up (translated) */
#define CMD_PGDN  12                              /* page down (translated) */
#define CMD_TOP   13                            /* top of file (translated) */
#define CMD_BOT   14                         /* bottom of file (translated) */
#define CMD_F1    15                               /* function key F1 (raw) */
#define CMD_F2    16                               /* function key F2 (raw) */
#define CMD_F3    17                               /* function key F3 (raw) */
#define CMD_F4    18                               /* function key F4 (raw) */
#define CMD_F5    19                               /* function key F5 (raw) */
#define CMD_F6    20                               /* function key F6 (raw) */
#define CMD_F7    21                               /* function key F7 (raw) */
#define CMD_F8    22                               /* function key F8 (raw) */
#define CMD_F9    23                               /* function key F9 (raw) */
#define CMD_F10   24                              /* function key F10 (raw) */
#define CMD_CHOME 25                                  /* control-home (raw) */
#define CMD_TAB   26                                    /* tab (translated) */
#define CMD_SF2   27                                      /* shift-F2 (raw) */
/* not used (obsolete) - 28 */
#define CMD_WORD_LEFT  29      /* word left (ctrl-left on dos) (translated) */
#define CMD_WORD_RIGHT 30    /* word right (ctrl-right on dos) (translated) */
#define CMD_WORDKILL 31                   /* delete word right (translated) */
#define CMD_EOF   32                                   /* end-of-file (raw) */
#define CMD_BREAK 33     /* break (Ctrl-C or local equivalent) (translated) */
#define CMD_INS   34                                    /* insert key (raw) */


/*
 *   ALT-keys - add alphabetical code to CMD_ALT: ALT-A == CMD_ALT + 0,
 *   ALT-B == CMD_ALT + 1, ALT-C == CMD_ALT + 2, etc
 *   
 *   These keys are all raw (untranslated).  
 */
#define CMD_ALT   128                                  /* start of ALT keys */


/* ------------------------------------------------------------------------ */
/*
 *   Event information structure for os_get_event.  The appropriate union
 *   member should be filled in, depending on the type of event that
 *   occurs. 
 */
union os_event_info_t
{
    /* 
     *   OS_EVT_KEY - this returns the one or two characters of the
     *   keystroke.  If the key is an extended key, so that os_getc() would
     *   return a two-character sequence for the keystroke, the first
     *   character should be zero and the second the extended key code.
     *   Otherwise, the first character should simply be the ASCII key code.
     *   
     *   The key code here is the "raw" keycode, equivalent to the codes
     *   returned by os_getc_raw().  Note in particular that this means that
     *   CTRL and Escape keys are presented as one-byte ASCII control
     *   characters, not as two-byte CMD_xxx sequences.  
     *   
     *   For multi-byte character sets (Shift-JIS, for example), note that
     *   os_get_event() must NOT return a complete two-byte character here.
     *   The two bytes here are exclusively used to represent special
     *   CMD_xxx keys (such as arrow keys and function keys).  For a
     *   keystroke that is represented in a multi-byte character set using
     *   more than one byte, os_get_event() must return a series of events.
     *   Return an ordinary OS_EVT_KEY for the first byte of the sequence,
     *   putting the byte in key[0]; then, return the second byte as a
     *   separate OS_EVT_KEY as the next event; and so on for any additional
     *   bytes.  This will allow callers that are not multibyte-aware to
     *   treat multi-byte characters as though they were sequences of
     *   one-byte characters.  
     */
    int key[2];

    /*
     *   OS_EVT_HREF - this returns the text of the HREF as a
     *   null-terminated string.  
     */
    char href[256];

    /* command ID (for OS_EVT_COMMAND) */
    int cmd_id;
};
typedef union os_event_info_t os_event_info_t;

/*
 *   Event types for os_get_event 
 */

/* invalid/no event */
#define OS_EVT_NONE      0x0000

/* OS_EVT_KEY - user typed a key on the keyboard */
#define OS_EVT_KEY       0x0001

/* OS_EVT_TIMEOUT - no event occurred before the timeout elapsed */
#define OS_EVT_TIMEOUT   0x0002

/* 
 *   OS_EVT_HREF - user clicked on a <A HREF> link.  This only applies to
 *   the HTML-enabled run-time. 
 */
#define OS_EVT_HREF      0x0003

/* 
 *   OS_EVT_NOTIMEOUT - caller requested a timeout, but timeout is not
 *   supported by this version of the run-time 
 */
#define OS_EVT_NOTIMEOUT 0x0004

/*
 *   OS_EVT_EOF - an error occurred reading the event.  This generally
 *   means that the application is quitting or we can no longer read from
 *   the keyboard or terminal. 
 */
#define OS_EVT_EOF       0x0005

/* 
 *   OS_EVT_LINE - user entered a line of text on the keyboard.  This event
 *   is not returned from os_get_event(), but rather from os_gets_timeout().
 */
#define OS_EVT_LINE      0x0006


/*
 *   Get an input event.  The event types are shown above.  If use_timeout is
 *   false, this routine should simply wait until one of the events it
 *   recognizes occurs, then return the appropriate information on the event.
 *   If use_timeout is true, this routine should return OS_EVT_TIMEOUT after
 *   the given number of milliseconds elapses if no event occurs first.
 *   
 *   This function is not obligated to obey the timeout.  If a timeout is
 *   specified and it is not possible to obey the timeout, the function
 *   should simply return OS_EVT_NOTIMEOUT.  The trivial implementation thus
 *   checks for a timeout, returns an error if specified, and otherwise
 *   simply waits for the user to press a key.
 *   
 *   A timeout value of 0 does *not* mean that there's no timeout (i.e., it
 *   doesn't mean we should wait indefinitely) - that's specified by passing
 *   FALSE for use_timeout.  A zero timeout also doesn't meant that the
 *   function should unconditionally return OS_EVT_TIMEOUT.  Instead, a zero
 *   timeout specifically means that IF an event is available IMMEDIATELY,
 *   without blocking the thread, we should return that event; otherwise we
 *   should immediately return a timeout event.  
 */
int os_get_event(unsigned long timeout_in_milliseconds, int use_timeout,
                 os_event_info_t *info);


/* ------------------------------------------------------------------------ */
/*
 *   Extended os_get_event() codes.
 *   
 *   THESE ARE NOT USED in the basic osifc implementation - these are only
 *   used if the interpreter supports the "extended" interface defined in
 *   osifcext.h.  
 */

/*
 *   System menu command event.  Some interpreters (such as HTML TADS for
 *   Windows) provide menu commands for common system-level game verbs -
 *   SAVE, RESTORE, UNDO, QUIT.  By default, these commands are returned as
 *   literal command strings ("save", "restore", etc) via os_gets(), as
 *   though the user had typed the commands at the keyboard.  The extended OS
 *   interface allows the program to receive these commands via
 *   os_get_event().  When a command is enabled for os_get_event() via the
 *   extended OS interface, and the player selects the command via a menu (or
 *   toolbar button, etc) during a call to os_get_event(), os_get_event()
 *   returns this event code, with the menu ID stored in the cmd_id field of
 *   the event structure.  
 */
#define OS_EVT_COMMAND   0x0100

/* command IDs for OS_EVT_COMMAND */
#define OS_CMD_NONE      0x0000     /* invalid command ID, for internal use */
#define OS_CMD_SAVE      0x0001                                /* save game */
#define OS_CMD_RESTORE   0x0002                             /* restore game */
#define OS_CMD_UNDO      0x0003                           /* undo last turn */
#define OS_CMD_QUIT      0x0004                                /* quit game */
#define OS_CMD_CLOSE     0x0005                    /* close the game window */
#define OS_CMD_HELP      0x0006                           /* show game help */

/* highest command ID used in this version of the interface */
#define OS_CMD_LAST      0x0006


/* ------------------------------------------------------------------------ */
/*
 *   Ask for input through a dialog.
 *   
 *   'prompt' is a text string to display as a prompting message.  For
 *   graphical systems, this message should be displayed in the dialog;
 *   for text systems, this should be displayed on the terminal after a
 *   newline.
 *   
 *   'standard_button_set' is one of the OS_INDLG_xxx values defined
 *   below, or zero.  If this value is zero, no standard button set is to
 *   be used; the custom set of buttons defined in 'buttons' is to be used
 *   instead.  If this value is non-zero, the appropriate set of standard
 *   buttons, with labels translated to the local language if possible, is
 *   to be used.
 *   
 *   'buttons' is an array of strings to use as button labels.
 *   'button_count' gives the number of entries in the 'buttons' array.
 *   'buttons' and 'button_count' are ignored if 'standard_button_set' is
 *   non-zero, since a standard set of buttons is used instead.  If
 *   'buttons' and 'button_count' are to be used, each entry contains the
 *   label of a button to show.  
 */
/*   
 *   An ampersand ('&') character in a label string indicates that the
 *   next character after the '&' is to be used as the short-cut key for
 *   the button, if supported.  The '&' should NOT be displayed in the
 *   string; instead, the character should be highlighted according to
 *   local system conventions.  On Windows, for example, the short-cut
 *   character should be shown underlined; on a text display, the response
 *   might be shown with the short-cut character enclosed in parentheses.
 *   If there is no local convention for displaying a short-cut character,
 *   then the '&' should simply be removed from the displayed text.  
 *   
 *   The short-cut key specified by each '&' character should be used in
 *   processing responses.  If the user presses the key corresponding to a
 *   button's short-cut, the effect should be the same as if the user
 *   clicked the button with the mouse.  If local system conventions don't
 *   allow for short-cut keys, any short-cut keys can be ignored.
 *   
 *   'default_index' is the 1-based index of the button to use as the
 *   default.  If this value is zero, there is no default response.  If
 *   the user performs the appropriate system-specific action to select
 *   the default response for the dialog, this is the response that is to
 *   be selected.  On Windows, for example, pressing the "Return" key
 *   should select this item.
 *   
 *   'cancel_index' is the 1-based index of the button to use as the
 *   cancel response.  If this value is zero, there is no cancel response.
 *   This is the response to be used if the user cancels the dialog using
 *   the appropriate system-specific action.  On Windows, for example,
 *   pressing the "Escape" key should select this item.  
 */
/*
 *   icon_id is one of the OS_INDLG_ICON_xxx values defined below.  If
 *   possible, an appropriate icon should be displayed in the dialog.
 *   This can be ignored in text mode, and also in GUI mode if there is no
 *   appropriate system icon.
 *   
 *   The return value is the 1-based index of the response selected.  If
 *   an error occurs, return 0.  
 */
int os_input_dialog(int icon_id, const char *prompt, int standard_button_set,
                    const char **buttons, int button_count,
                    int default_index, int cancel_index);

/*
 *   Standard button set ID's 
 */

/* OK */
#define OS_INDLG_OK            1

/* OK, Cancel */
#define OS_INDLG_OKCANCEL      2

/* Yes, No */
#define OS_INDLG_YESNO         3

/* Yes, No, Cancel */
#define OS_INDLG_YESNOCANCEL   4

/*
 *   Dialog icons 
 */

/* no icon */
#define OS_INDLG_ICON_NONE     0

/* warning */
#define OS_INDLG_ICON_WARNING  1

/* information */
#define OS_INDLG_ICON_INFO     2

/* question */
#define OS_INDLG_ICON_QUESTION 3

/* error */
#define OS_INDLG_ICON_ERROR    4



/* ------------------------------------------------------------------------ */
/* 
 *   OS main entrypoint 
 */
int os0main(int oargc, char **oargv,
            int (*mainfn)(int, char **, char *), 
            const char *before, const char *config);

/* 
 *   new-style OS main entrypoint - takes an application container context 
 */
int os0main2(int oargc, char **oargv,
             int (*mainfn)(int, char **, struct appctxdef *, char *),
             const char *before, const char *config,
             struct appctxdef *appctx);

/*
 *   OBSOLETE - Get filename from startup parameter, if possible; returns
 *   true and fills in the buffer with the parameter filename on success,
 *   false if no parameter file could be found.
 *   
 *   (This was used until TADS 2.2.5 for the benefit of the Mac interpreter,
 *   and interpreters on systems with similar desktop shells, to allow the
 *   user to launch the terp by double-clicking on a saved game file.  The
 *   terp would read the launch parameters, discover that a saved game had
 *   been used to invoke it, and would then stash away the saved game info
 *   for later retrieval from this function.  This functionality was replaced
 *   in 2.2.5 with a command-line parameter: the terp now uses the desktop
 *   launch data to synthesize a suitable argv[] vectro to pass to os0main()
 *   or os0main2().  This function should now simply be stubbed out - it
 *   should simply return FALSE.)  
 */
int os_paramfile(char *buf);

/* 
 *   Initialize.  This should be called during program startup to
 *   initialize the OS layer and check OS-specific command-line arguments.
 *   
 *   If 'prompt' and 'buf' are non-null, and there are no arguments on the
 *   given command line, the OS code can use the prompt to ask the user to
 *   supply a filename, then store the filename in 'buf' and set up
 *   argc/argv to give a one-argument command string.  (This mechanism for
 *   prompting for a filename is obsolescent, and is retained for
 *   compatibility with a small number of existing implementations only;
 *   new implementations should ignore this mechanism and leave the
 *   argc/argv values unchanged.)  
 */
int os_init(int *argc, char *argv[], const char *prompt,
            char *buf, int bufsiz);

/*
 *   Termination functions.  There are three main termination functions,
 *   described individually below; here's a brief overview of the
 *   relationship among the functions.  The important thing to realize is
 *   that these functions have completely independent purposes; they should
 *   never call one another, and they should never do any of the work that's
 *   intended for the others.
 *   
 *   os_uninit() is meant to undo the effects of os_init().  On many
 *   systems, os_init() has some global effect, such as setting the terminal
 *   to some special input or output mode.  os_uninit's purpose is to undo
 *   these global effects, returning the terminal mode (and whatever else)
 *   to the conditions they were in at program startup.  Portable callers
 *   are meant to call this routine at some point before terminating if they
 *   ever called os_init().  Note that this routine DOES NOT terminate the
 *   program - it should simply undo anything that os_init() did and return,
 *   to let the caller do any additional termination work of its own.
 *   
 *   os_expause() optionally pauses before termination, to allow the user to
 *   acknowledge any text the program displays just before exiting.  This
 *   doesn't have to do anything at all, but it's useful on systems where
 *   program termination will do something drastic like close the window:
 *   without a pause, the user wouldn't have a chance to read any text the
 *   program displayed after the last interactive input, because the window
 *   would abruptly disappear moments after the text was displayed.  For
 *   systems where termination doesn't have such drastic effects, there's no
 *   need to do anything in this routine.  Because it's up to this routine
 *   whether or not to pause, this routine must display a prompt if it's
 *   going to pause for user input - the portable caller obviously can't do
 *   so, because the caller doesn't know if the routine is going to pause or
 *   not (so if the caller displayed a prompt assuming the routine would
 *   pause, the prompt would show up even on systems where there actually is
 *   no pause, which would be confusing).  This routine DOES NOT terminate
 *   the program; it simply pauses if necessary to allow the user to
 *   acknowledge the last bit of text the program displayed, then returns to
 *   allow the caller to carry on with its own termination work.
 *   
 *   os_term() is meant to perform the same function as the C standard
 *   library routine exit(): this actually terminates the program, exiting
 *   to the operating system.  This routine is not meant to return to its
 *   caller, because it's supposed to exit the program directly.  For many
 *   systems, this routine can simply call exit(); the portable code calls
 *   this routine instead of calling exit() directly, because some systems
 *   have their own OS-specific way of terminating rather than using exit().
 *   This routine MUST NOT undo the effects of os_init(), because callers
 *   might not have ever called os_init(); callers are required to call
 *   os_uninit() if they ever called os_init(), before calling os_term(), so
 *   this routine can simply assume that any global modes set by os_init()
 *   have already been undone by the time this is called.  
 */

/*
 *   Uninitialize.  This is called prior to progam termination to reverse
 *   the effect of any changes made in os_init().  For example, if
 *   os_init() put the terminal in raw mode, this should restore the
 *   previous terminal mode.  This routine should not terminate the
 *   program (so don't call exit() here) - the caller might have more
 *   processing to perform after this routine returns.  
 */
void os_uninit(void);

/* 
 *   Pause prior to exit, if desired.  This is meant to be called by
 *   portable code just before the program is to be terminated; it can be
 *   implemented to show a prompt and wait for user acknowledgment before
 *   proceeding.  This is useful for implementations that are using
 *   something like a character-mode terminal window running on a graphical
 *   operating system: this gives the implementation a chance to pause
 *   before exiting, so that the window doesn't just disappear
 *   unceremoniously.
 *   
 *   This is allowed to do nothing at all.  For regular character-mode
 *   systems, this routine usually doesn't do anything, because when the
 *   program exits, the terminal will simply return to the OS command
 *   prompt; none of the text displayed just before the program exited will
 *   be lost, so there's no need for any interactive pause.  Likewise, for
 *   graphical systems where the window will remain open, even after the
 *   program exits, until the user explicitly closes the window, there's no
 *   need to do anything here.
 *   
 *   If this is implemented to pause, then this routine MUST show some kind
 *   of prompt to let the user know we're waiting.  In the simple case of a
 *   text-mode terminal window on a graphical OS, this should simply print
 *   out some prompt text ("Press a key to exit...") and then wait for the
 *   user to acknowledge the prompt (by pressing a key, for example).  For
 *   graphical systems, the prompt could be placed in the window's title
 *   bar, or status-bar, or wherever is appropriate for the OS.  
 */
void os_expause(void);

/* 
 *   Terminate.  This should exit the program with the given exit status.
 *   In general, this should be equivalent to the standard C library
 *   exit() function, but we define this interface to allow the OS code to
 *   do any necessary pre-termination cleanup.  
 */
void os_term(int status);

/* 
 *   Install/uninstall the break handler.  If possible, the OS code should
 *   set (if 'install' is true) or clear (if 'install' is false) a signal
 *   handler for keyboard break signals (control-C, etc, depending on
 *   local convention).  The OS code should set its own handler routine,
 *   which should note that a break occurred with an internal flag; the
 *   portable code uses os_break() from time to time to poll this flag.  
 */
void os_instbrk(int install);

/*
 *   Check for user break ("control-C", etc) - returns true if a break is
 *   pending, false if not.  If this returns true, it should "consume" the
 *   pending break (probably by simply clearing the OS code's internal
 *   break-pending flag).  
 */
int os_break(void);

/*
 *   Sleep for a given interval.  This should simply pause for the given
 *   number of milliseconds, then return.  On multi-tasking systems, this
 *   should use a system API to suspend the current process for the desired
 *   delay; on single-tasking systems, this can simply sit in a wait loop
 *   until the desired interval has elapsed.  
 */
void os_sleep_ms(long delay_in_milliseconds);

/*
 *   Yield CPU; returns TRUE if user requested an interrupt (a "control-C"
 *   type of operation to abort the entire program), FALSE to continue.
 *   Portable code should call this routine from time to time during lengthy
 *   computations that don't involve any UI operations; if practical, this
 *   routine should be invoked roughly every 10 to 100 milliseconds.
 *   
 *   The purpose of this routine is to support "cooperative multitasking"
 *   systems, such as pre-X MacOS, where it's necessary for each running
 *   program to call the operating system explicitly in order to yield the
 *   CPU from time to time to other concurrently running programs.  On
 *   cooperative multitasking systems, a program can only lose control of
 *   the CPU by making specific system calls, usually related to GUI events;
 *   a program can never lose control of the CPU asynchronously.  So, a
 *   program that performs lengthy computations without any UI interaction
 *   can cause the rest of the system to freeze up until the computations
 *   are finished; but if a compute-intensive program explicitly yields the
 *   CPU from time to time, it allows other programs to remain responsive.
 *   Yielding the CPU at least every 100 milliseconds or so will generally
 *   allow the UI to remain responsive; yielding more frequently than every
 *   10 ms or so will probably start adding noticeable overhead.
 *   
 *   On single-tasking systems (such as MS-DOS), there's only one program
 *   running at a time, so there's no need to yield the CPU; on virtually
 *   every modern system, the OS automatically schedules CPU time without
 *   the running programs having any say in the matter, so again there's no
 *   need for a program to yield the CPU.  For systems where this routine
 *   isn't needed, the system header should simply #define os_yield to
 *   something like "((void)0)" - this will allow the compiler to completely
 *   ignore calls to this routine for systems where they aren't needed.
 *   
 *   Note that this routine is NOT meant to provide scheduling hinting to
 *   modern systems with true multitasking, so a trivial implementation is
 *   fine for any modern system.  
 */
#ifndef os_yield
int os_yield(void);
#endif

/*
 *   Set the default saved-game extension.  This routine will NOT be called
 *   when we're using the standard saved game extension; this routine will be
 *   invoked only if we're running as a stand-alone game, and the game author
 *   specified a non-standard saved-game extension when creating the
 *   stand-alone game.
 *   
 *   This routine is not required if the system does not use the standard,
 *   semi-portable os0.c implementation.  Even if the system uses the
 *   standard os0.c implementation, it can provide an empty routine here if
 *   the system code doesn't need to do anything special with this
 *   information.
 *   
 *   The extension is specified as a null-terminated string.  The extension
 *   does NOT include the leading period.  
 */
void os_set_save_ext(const char *ext);

/* 
 *   Get the saved game extension previously set with os_set_save_ext().
 *   Returns null if no custom extension has been set.
 */
const char *os_get_save_ext();


/* ------------------------------------------------------------------------*/
/*
 *   Translate a character from the HTML 4 Unicode character set to the
 *   current character set used for display.  Takes an HTML 4 character
 *   code and returns the appropriate local character code.
 *   
 *   The result buffer should be filled in with a null-terminated string
 *   that should be used to represent the character.  Multi-character
 *   results are possible, which may be useful for certain approximations
 *   (such as using "(c)" for the copyright symbol).
 *   
 *   Note that we only define this prototype if this symbol isn't already
 *   defined as a macro, which may be the case on some platforms.
 *   Alternatively, if the function is already defined (for example, as an
 *   inline function), the defining code can define OS_XLAT_HTML4_DEFINED,
 *   in which case we'll also omit this prototype.
 *   
 *   Important: this routine provides the *default* mapping that is used
 *   when no external character mapping file is present, and for any named
 *   entities not defined in the mapping file.  Any entities in the
 *   mapping file, if used, will override this routine.
 *   
 *   A trivial implementation of this routine (that simply returns a
 *   one-character result consisting of the original input character,
 *   truncated to eight bits if necessary) can be used if you want to
 *   require an external mapping file to be used for any game that
 *   includes HTML character entities.  The DOS version implements this
 *   routine so that games will still look reasonable when played with no
 *   mapping file present, but other systems are not required to do this.  
 */
#ifndef os_xlat_html4
# ifndef OS_XLAT_HTML4_DEFINED
void os_xlat_html4(unsigned int html4_char,
                   char *result, size_t result_buf_len);
# endif
#endif

/*
 *   Generate a filename for a character-set mapping file.  This function
 *   should determine the current native character set in use, if
 *   possible, then generate a filename, according to system-specific
 *   conventions, that we should attempt to load to get a mapping between
 *   the current native character set and the internal character set
 *   identified by 'internal_id'.
 *   
 *   The internal character set ID is a string of up to 4 characters.
 *   
 *   On DOS, the native character set is a DOS code page.  DOS code pages
 *   are identified by 3- or 4-digit identifiers; for example, code page
 *   437 is the default US ASCII DOS code page.  We generate the
 *   character-set mapping filename by appending the internal character
 *   set identifier to the DOS code page number, then appending ".TCP" to
 *   the result.  So, to map between ISO Latin-1 (internal ID = "La1") and
 *   DOS code page 437, we would generate the filename "437La1.TCP".
 *   
 *   Note that this function should do only two things.  First, determine
 *   the current native character set that's in use.  Second, generate a
 *   filename based on the current native code page and the internal ID.
 *   This function is NOT responsible for figuring out the mapping or
 *   anything like that -- it's simply where we generate the correct
 *   filename based on local convention.
 *   
 *   'filename' is a buffer of at least OSFNMAX characters.
 *   
 *   'argv0' is the executable filename from the original command line.
 *   This parameter is provided so that the system code can look for
 *   mapping files in the original TADS executables directory, if desired.
 */
void os_gen_charmap_filename(char *filename, char *internal_id,
                             char *argv0);

/*
 *   Receive notification that a character mapping file has been loaded.
 *   The caller doesn't require this routine to do anything at all; this
 *   is purely for the system-dependent code's use so that it can take
 *   care of any initialization that it must do after the caller has
 *   loaded a charater mapping file.  'id' is the character set ID, and
 *   'ldesc' is the display name of the character set.  'sysinfo' is the
 *   extra system information string that is stored in the mapping file;
 *   the interpretation of this information is up to this routine.
 *   
 *   For reference, the Windows version uses the extra information as a
 *   code page identifier, and chooses its default font character set to
 *   match the code page.  On DOS, the run-time requires the player to
 *   activate an appropriate code page using a DOS command (MODE CON CP
 *   SELECT) prior to starting the run-time, so this routine doesn't do
 *   anything at all on DOS. 
 */
void os_advise_load_charmap(const char *id, const char *ldesc, const char *sysinfo);

/*
 *   Generate the name of the character set mapping table for Unicode
 *   characters to and from the given local character set.  Fills in the
 *   buffer with the implementation-dependent name of the desired
 *   character set map.  See below for the character set ID codes.
 *   
 *   For example, on Windows, the implementation would obtain the
 *   appropriate active code page (which is simply a Windows character set
 *   identifier number) from the operating system, and build the name of
 *   the Unicode mapping file for that code page, such as "CP1252".  On
 *   Macintosh, the implementation would look up the current script system
 *   and return the name of the Unicode mapping for that script system,
 *   such as "ROMAN" or "CENTEURO".
 *   
 *   If it is not possible to determine the specific character set that is
 *   in use, this function should return "asc7dflt" (ASCII 7-bit default)
 *   as the character set identifier on an ASCII system, or an appropriate
 *   base character set name on a non-ASCII system.  "asc7dflt" is the
 *   generic character set mapping for plain ASCII characters.
 *   
 *   The given buffer must be at least 32 bytes long; the implementation
 *   must limit the result it stores to 32 bytes.  (We use a fixed-size
 *   buffer in this interface for simplicity, and because there seems no
 *   need for greater flexibility in the interface; a character set name
 *   doesn't carry very much information so shouldn't need to be very
 *   long.  Note that this function doesn't generate a filename, but
 *   simply a mapping name; in practice, a map name will be used to
 *   construct a mapping file name.)
 *   
 *   Because this function obtains the Unicode mapping name, there is no
 *   need to specify the internal character set to be used: the internal
 *   character set is Unicode.  
 */
/*
 *   Implementation note: when porting this routine, the convention that
 *   you use to name your mapping files is up to you.  You should simply
 *   choose a convention for this implementation, and then use the same
 *   convention for packaging the mapping files for your OS release.  In
 *   most cases, the best convention is to use the names that the Unicode
 *   consortium uses in their published cross-mapping listings, since
 *   these listings can be used as the basis of the mapping files that you
 *   include with your release.  For example, on Windows, the convention
 *   is to use the code page number to construct the map name, as in
 *   CP1252 or CP1250.  
 */
void os_get_charmap(char *mapname, int charmap_id);

/*
 *   Character map for the display (i.e., for the user interface).  This
 *   is the character set which is used for input read from the keyboard,
 *   and for output displayed on the monitor or terminal.  
 */
#define OS_CHARMAP_DISPLAY     1

/* 
 *   Character map for mapping filename strings.  This should identify the
 *   character set currently in use on the local system for filenames
 *   (i.e., for strings identifying objects in the local file system),
 *   providing a suitable name for use in opening a mapping file.
 *   
 *   On many platforms, the entire system uses only one character set at
 *   the OS level, so the file system character set is the same as the
 *   display character set.  Some systems define a particular character
 *   set for file system use, though, because different users might be
 *   running applications on terminals that display different character
 *   sets.  
 */
#define OS_CHARMAP_FILENAME    2

/*
 *   Default character map for file contents.  On most systems, this will
 *   be the same as display.  On some systems, it won't be possible to
 *   know in general what character set files use; in fact, this isn't
 *   possible anywhere, since a file might have been copied without
 *   translation from a different type of computer using a different
 *   character set.  So, this isn't meant to provide a reliable choice of
 *   character set for any arbitrary file; it's simply meant to be a good
 *   guess that most files on this system are likely to use.  
 */
#define OS_CHARMAP_FILECONTENTS  3

/*
 *   Default character map for the command line.  This is the maping we use
 *   to interpret command line arguments passed to our main() or equivalent.
 *   On most systems, this will be the same as the display character set.
 */
#define OS_CHARMAP_CMDLINE     4


/* ------------------------------------------------------------------------ */
/*
 *   External Banner Interface.  This interface provides the ability to
 *   divide the display window into multiple sub-windows, each with its own
 *   independent contents.
 *   
 *   To determine where a new banner is displayed, we look at the banners as
 *   a tree, rooted at the "main window," the special banner that the system
 *   automatically creates initially for the main game text.  We start by
 *   allocating the entire display (or the entire application window, if
 *   we're running on a GUI system) to the main window.  We then traverse
 *   the tree, starting with the root window's children.  For each child
 *   window, we allocate space for the child out of the parent window's
 *   area, according to the child's alignment and size settings, and deduct
 *   this space from the parent window's size.  We then lay out the children
 *   of the child.
 *   
 *   For each banner window, we take its requested space out of the parent
 *   window's area by starting at the edge of the parent window rectangle as
 *   indicated by the banner's alignment, and taking the requested `width
 *   (for a left/right banner) or height (for a top/bottom banner), limiting
 *   to the available width/height in the parent window's space.  Give the
 *   banner the full extent of the parent's space in its other dimension (so
 *   a left/right banner gets the full height of the parent space, and a
 *   top/bottom banner gets the full width).
 *   
 *   Note that the layout proceeds exclusively down the tree (i.e., from the
 *   root to children to grandchildren, and so on).  It *appears* that a
 *   child affects its parent, because of the deduction step: a child
 *   acquires screen space by carving out a chunk of its parent.  The right
 *   way to think about this, though, is that the parent's full area is the
 *   union of the parent window and all of its children; when viewed this
 *   way, the parent's full area is fully determined the instant the parent
 *   is laid out, and never changes as its children are laid out.  Note in
 *   particular that a child can never make a parent larger; the only thing
 *   a child can do to a parent is carve out a chunk of the parent for
 *   itself, which doesn't affect the boundaries of the union of the parent
 *   plus its children.
 *   
 *   Note also that if the banner has a border, and the implementation
 *   actually draws borders, the border must be drawn for the *full* area of
 *   the banner, as defined above.  For example, suppose we have two
 *   borders: banner A is a child of the main window, is top-aligned, and
 *   has a border.  Banner B is a child of banner A, right-aligned, with no
 *   border.  Obviously, without considering banner B, banner A's space runs
 *   across the entire width of the main window, so its border (at the
 *   bottom of its area) runs across the entire width of the main window.
 *   Banner B carves out some space from A's right side for itself, so
 *   banner A's actual on-screen area runs from the left edge of the main
 *   window to banner B's left edge.  However, even though banner A itself
 *   no longer runs the full width of the main window, banner A's *full*
 *   area - that is, the union of banner A's on-screen area and all of its
 *   children's full areas - does still run the entire width of the main
 *   window, hence banner A's border must still run the full width of the
 *   main window.  The simple way of looking at this is that a banner's
 *   border is always to be drawn exactly the same way, regardless of
 *   whether or not the banner has children - simply draw the banner as it
 *   would be drawn if the banner had no children.
 *   
 *   Each time a banner is added or removed, we must recalculate the layout
 *   of the remaining banners and main text area.  The os_banner_xxx()
 *   implementation is responsible for this layout refiguring.
 *   
 *   The entire external banner window interface is optional, although the
 *   functions must at least be defined as dummies to avoid linker errors
 *   when building.  If a platform doesn't implement this feature,
 *   os_banner_create() should simply return null, and the other routines
 *   can do nothing.  
 */

/* 
 *   Create a banner window.  'info' gives the desired parameters for the new
 *   banner.
 *   
 *   Note that certain requested parameter settings might or might not be
 *   respected, depending on the capabilities of the platform and user
 *   preferences.  os_banner_getinfo() can be used after creation to
 *   determine which parameter settings are actually used in the new banner.
 *   
 *   'parent' gives the parent of this banner; this is the banner handle of
 *   another banner window, or null.  If 'parent' is null, then the new
 *   banner is a child of the main window, which the system creates
 *   automatically at startup and which contains the main input/output
 *   transcript.  The new banner's on-screen area is carved out of the
 *   parent's space, according to the alignment and size settings of the new
 *   window, so this determines how the window is laid out on the screen.
 *   
 *   'where' is OS_BANNER_FIRST to make the new window the first child of its
 *   parent; OS_BANNER_LAST to make it the last child of its parent;
 *   OS_BANNER_BEFORE to insert it immediately before the existing banner
 *   identified by handle in 'other'; or OS_BANNER_AFTER to insert
 *   immediately after 'other'.  When BEFORE or AFTER is used, 'other' must
 *   be another child of the same parent; if it is not, the routine should
 *   act as though 'where' were given as OS_BANNER_LAST.
 *   
 *   'other' is a banner handle for an existing banner window.  This is used
 *   to specify the relative position among children of the new banner's
 *   parent, if 'where' is either OS_BANNER_BEFORE or OS_BANNER_AFTER.  If
 *   'where' is OS_BANNER_FIRST or OS_BANNER_LAST, 'other' is ignored.
 *   
 *   'wintype' is the type of the window.  This is one of the
 *   OS_BANNER_TYPE_xxx codes indicating what kind of window is desired.
 *   
 *   'align' is the banner's alignment, given as an OS_BANNER_ALIGN_xxx
 *   value.  Top/bottom banners are horizontal: they run across the full
 *   width of the existing main text area.  Left/right banners are vertical:
 *   they run down the full height of the existing main text area.
 *   
 *   'siz' is the requested size of the new banner.  The meaning of 'siz'
 *   depends on the value of 'siz_units', which can be OS_BANNER_SIZE_PCT to
 *   set the size as a percentage of the REMAINING space, or
 *   OS_BANNER_SIZE_ABS to set an absolute size in the "natural" units of the
 *   window.  The natural units vary by window type: for text and text grid
 *   windows, this is in rows/columns of '0' characters in the default font
 *   for the window.  Note that when OS_BANNER_SIZE_ABS is used in a text or
 *   text grid window, the OS implementation MUST add the space needed for
 *   margins and borders when determining the actual pixel size of the
 *   window; in other words, the window should be large enough that it can
 *   actually display the given number or rows or columns.
 *   
 *   The size is interpreted as a width or height according to the window's
 *   orientation.  For a TOP or BOTTOM banner, the size is the height; for a
 *   LEFT or RIGHT banner, the size is the width.  A banner has only one
 *   dimension's size given, since the other dimension's size is determined
 *   automatically by the layout rules.
 *   
 *   Note that the window's size can be changed later using
 *   banner_size_to_contents() or banner_set_size().
 *   
 *   'style' is a combination of OS_BANNER_STYLE_xxx flags - see below.  The
 *   style flags give the REQUESTED style for the banner, which might or
 *   might not be respected, depending on the platform's capabilities, user
 *   preferences, and other factors.  os_banner_getinfo() can be used to
 *   determine which style flags are actually used.
 *   
 *   Returns the "handle" to the new banner window, which is an opaque value
 *   that is used in subsequent os_banner_xxx calls to operate on the window.
 *   Returns null if the window cannot be created.  An implementation is not
 *   required to support this functionality at all, and can subset it if it
 *   does support it (for example, an implementation could support only
 *   top/bottom-aligned banners, but not left/right-aligned), so callers must
 *   be prepared for this routine to return null.  
 */
void *os_banner_create(void *parent, int where, void *other, int wintype,
                       int align, int siz, int siz_units,
                       unsigned long style);


/*
 *   insertion positions 
 */
#define OS_BANNER_FIRST   1
#define OS_BANNER_LAST    2
#define OS_BANNER_BEFORE  3
#define OS_BANNER_AFTER   4

/*
 *   banner types 
 */

/* 
 *   Normal text stream window.  This is a text stream that behaves
 *   essentially like the main text window: text is displayed to this
 *   through os_banner_disp(), always in a stream-like fashion by adding new
 *   text to the end of any exiting text.
 *   
 *   Systems that use proportional fonts should usually simply use the same
 *   font they use by default in the main text window.  However, note that
 *   the OS_BANNER_STYLE_TAB_ALIGN style flag might imply that a fixed-pitch
 *   font should be used even when proportional fonts are available, because
 *   a fixed-pitch font will allow the calling code to rely on using spaces
 *   to align text within the window.  
 */
#define OS_BANNER_TYPE_TEXT       1

/* 
 *   "Text grid" window.  This type of window is similar to an normal text
 *   window (OS_BANNER_TYPE_TEXT), but is guaranteed to arrange its text in
 *   a regular grid of character cells, all of the same size.  This means
 *   that the output position can be moved to an arbitrary point within the
 *   window at any time, so the calling program can precisely control the
 *   layout of the text in the window.
 *   
 *   Because the output position can be moved to arbitrary positions in the
 *   window, it is possible to overwrite text previously displayed.  When
 *   this happens, the old text is completely obliterated by the new text,
 *   leaving no trace of the overwritten text.
 *   
 *   In order to guarantee that character cells are all the same size, this
 *   type of window does not allow any text attributes.  The implementation
 *   should simply ignore any attempts to change text attributes in this
 *   type of window.  However, colors can be used to the same degree they
 *   can be used in an ordinary text window.
 *   
 *   To guarantee the regular spacing of character cells, all
 *   implementations must use fixed-pitch fonts for these windows.  This
 *   applies even to platforms where proportional fonts are available.  
 */
#define OS_BANNER_TYPE_TEXTGRID   2


/* 
 *   banner alignment types 
 */
#define OS_BANNER_ALIGN_TOP       0
#define OS_BANNER_ALIGN_BOTTOM    1
#define OS_BANNER_ALIGN_LEFT      2
#define OS_BANNER_ALIGN_RIGHT     3

/*
 *   size units 
 */
#define OS_BANNER_SIZE_PCT  1
#define OS_BANNER_SIZE_ABS  2


/* 
 *   banner style flags 
 */

/* 
 *   The banner has a visible border; this indicates that a line is to be
 *   drawn to separate the banner from the adjacent window or windows
 *   "inside" the banner.  So, a top-aligned banner will have its border
 *   drawn along its bottom edge; a left-aligned banner will show a border
 *   along its right edge; and so forth.
 *   
 *   Note that character-mode platforms generally do NOT respect the border
 *   style, since doing so takes up too much screen space.  
 */
#define OS_BANNER_STYLE_BORDER     0x00000001

/*
 *   The banner has a vertical/horizontal scrollbar.  Character-mode
 *   platforms generally do not support scrollbars.  
 */
#define OS_BANNER_STYLE_VSCROLL    0x00000002
#define OS_BANNER_STYLE_HSCROLL    0x00000004

/* 
 *   Automatically scroll the banner vertically/horizontally whenever new
 *   text is displayed in the window.  In other words, whenever
 *   os_banner_disp() is called, scroll the window so that the text that the
 *   new cursor position after the new text is displayed is visible in the
 *   window.
 *   
 *   Note that this style is independent of the presence of scrollbars.
 *   Even if there are no scrollbars, we can still scroll the window's
 *   contents programmatically.
 *   
 *   Implementations can, if desired, keep an internal buffer of the
 *   window's contents, so that the contents can be recalled via the
 *   scrollbars if the text displayed in the banner exceeds the space
 *   available in the banner's window on the screen.  If the implementation
 *   does keep such a buffer, we recommend the following method for managing
 *   this buffer.  If the AUTO_VSCROLL flag is not set, then the banner's
 *   contents should be truncated at the bottom when the contents overflow
 *   the buffer; that is, once the banner's internal buffer is full, any new
 *   text that the calling program attempts to add to the banner should
 *   simply be discarded.  If the AUTO_VSCROLL flag is set, then the OLDEST
 *   text should be discarded instead, so that the most recent text is
 *   always retained.  
 */
#define OS_BANNER_STYLE_AUTO_VSCROLL 0x00000008
#define OS_BANNER_STYLE_AUTO_HSCROLL 0x00000010

/*
 *   Tab-based alignment is required/supported.  On creation, this is a hint
 *   to the implementation that is sometimes necessary to determine what
 *   kind of font to use in the new window, for non-HTML platforms.  If this
 *   flag is set on creation, the caller is indicating that it wants to use
 *   <TAB> tags to align text in the window.
 *   
 *   Character-mode implementations that use a single font with fixed pitch
 *   can simply ignore this.  These implementations ALWAYS have a working
 *   <TAB> capability, because the portable output formatter provides <TAB>
 *   interpretation for a fixed-pitch window.
 *   
 *   Full HTML TADS implementations can also ignore this.  HTML TADS
 *   implementations always have full <TAB> support via the HTML
 *   parser/renderer.
 *   
 *   Text-only implementations on GUI platforms (i.e., implementations that
 *   are not based on the HTML parser/renderer engine in HTML TADS, but
 *   which run on GUI platforms with proportionally-spaced text) should use
 *   this flag to determine the font to display.  If this flag is NOT set,
 *   then the caller doesn't care about <TAB>, and the implementation is
 *   free to use a proportionally-spaced font in the window if desired.
 *   
 *   When retrieving information on an existing banner, this flag indicates
 *   that <TAB> alignment is actually supported on the window.  
 */
#define OS_BANNER_STYLE_TAB_ALIGN 0x00000020

/*
 *   Use "MORE" mode in this window.  By default, a banner window should
 *   happily allow text to overflow the vertical limits of the window; the
 *   only special thing that should happen on overflow is that the window
 *   should be srolled down to show the latest text, if the auto-vscroll
 *   style is set.  With this flag, though, a banner window acts just like
 *   the main text window: when the window fills up vertically, we show a
 *   MORE prompt (using appropriate system conventions), and wait for the
 *   user to indicate that they're ready to see more text.  On most systems,
 *   the user acknowledges a MORE prompt by pressing a key or scrolling with
 *   the mouse, but it's up to the system implementor to decide what's
 *   appropriate for the system.
 *   
 *   Note that MORE mode in ANY banner window should generally override all
 *   other user input focus.  In other words, if the game in the main window
 *   would like to read a keystroke from the user, but one of the banner
 *   windows is pausing with a MORE prompt, any keyboard input should be
 *   directed to the banner paused at the MORE prompt, not to the main
 *   window; the main window should not receive any key events until the MORE
 *   prompt has been removed.
 *   
 *   This style requires the auto-vscroll style.  Implementations should
 *   assume auto-vscroll when this style is set.  This style can be ignored
 *   with text grid windows.  
 */
#define OS_BANNER_STYLE_MOREMODE  0x00000040

/*
 *   This banner is a horizontal/vertical "strut" for sizing purposes.  This
 *   means that the banner's content size is taken into account when figuring
 *   the content size of its *parent* banner.  If the banner has the same
 *   orientation as the parent, its content size is added to its parent's
 *   internal content size to determine the parent's overall content size.
 *   If the banner's orientation is orthogonal to the parent's, then the
 *   parent's overall content size is the larger of the parent's internal
 *   content size and this banner's content size.  
 */
#define OS_BANNER_STYLE_HSTRUT    0x00000080
#define OS_BANNER_STYLE_VSTRUT    0x00000100


/* 
 *   Delete a banner.  This removes the banner from the display, which
 *   requires recalculating the entire screen's layout to reallocate this
 *   banner's space to other windows.  When this routine returns, the banner
 *   handle is invalid and can no longer be used in any os_banner_xxx
 *   function calls.  
 *   
 *   If the banner has children, the children will no longer be displayed,
 *   but will remain valid in memory until deleted.  A child window's
 *   display area always comes out of its parent's space, so once the parent
 *   is gone, a child has no way to acquire any display space; resizing the
 *   child won't help, since it simply has no way to obtain any screen space
 *   once its parent has been deleted.  Even though the window's children
 *   will become invisible, their banner handles will remain valid; the
 *   caller is responsible for explicitly deleting the children even after
 *   deleting their parent.  
 */
void os_banner_delete(void *banner_handle);

/*
 *   "Orphan" a banner.  This tells the osifc implementation that the caller
 *   wishes to sever all of its ties with the banner (as part of program
 *   termination, for example), but that the calling program does not
 *   actually require that the banner's on-screen display be immediately
 *   removed.
 *   
 *   The osifc implementation can do one of two things:
 *   
 *   1.  Simply call os_banner_delete().  If the osifc implementation
 *   doesn't want to do anything extra with the banner, it can simply delete
 *   the banner, since the caller has no more use for it.
 *   
 *   2.  Take ownership of the banner.  If the osifc implementation wishes
 *   to continue displaying the final screen configuration after a program
 *   has terminated, it can simply take over the banner and leave it on the
 *   screen.  The osifc subsystem must eventually delete the banner itself
 *   if it takes this routine; for example, if the osifc subsystem allows
 *   another client program to be loaded into the same window after a
 *   previous program has terminated, it would want to delete any orphaned
 *   banners from the previous program when loading a new program.  
 */
void os_banner_orphan(void *banner_handle);

/*
 *   Banner information structure.  This is filled in by the system-specific
 *   implementation in os_banner_getinfo().  
 */
struct os_banner_info_t
{
    /* alignment */
    int align;

    /* style flags - these indicate the style flags actually in use */
    unsigned long style;

    /* 
     *   Actual on-screen size of the banner, in rows and columns.  If the
     *   banner is displayed in a proportional font or can display multiple
     *   fonts of different sizes, this is approximated by the number of "0"
     *   characters in the window's default font that will fit in the
     *   window's display area.  
     */
    int rows;
    int columns;

    /*
     *   Actual on-screen size of the banner in pixels.  This is meaningful
     *   only for full HTML interpreter; for text-only interpreters, these
     *   are always set to zero.
     *   
     *   Note that even if we're running on a GUI operating system, these
     *   aren't meaningful unless this is a full HTML interpreter.  Text-only
     *   interpreters should always set these to zero, even on GUI OS's.  
     */
    int pix_width;
    int pix_height;

    /* 
     *   OS line wrapping flag.  If this is set, the window uses OS-level
     *   line wrapping because the window uses a proportional font, so the
     *   caller does not need to (and should not) perform line breaking in
     *   text displayed in the window.
     *   
     *   Note that OS line wrapping is a PERMANENT feature of the window.
     *   Callers can note this information once and expect it to remain
     *   fixed through the window's lifetime.  
     */
    int os_line_wrap;
};
typedef struct os_banner_info_t os_banner_info_t;

/* 
 *   Get information on the banner - fills in the information structure with
 *   the banner's current settings.  Note that this should indicate the
 *   ACTUAL properties of the banner, not the requested properties; this
 *   allows callers to determine how the banner is actually displayed, which
 *   depends upon the platform's capabilities and user preferences.
 *   
 *   Returns true if the information was successfully obtained, false if
 *   not.  This can return false if the underlying OS window has already
 *   been closed by a user action, for example.  
 */
int os_banner_getinfo(void *banner_handle, os_banner_info_t *info);

/* 
 *   Get the character width/height of the banner, for layout purposes.  This
 *   gives the size of the banner in character cells.
 *   
 *   These are not meaningful when the underlying window uses a proportional
 *   font or varying fonts of different sizes.  When the size of text varies
 *   in the window, the OS layer is responsible for word-wrapping and other
 *   layout, in which case these simply return zero.
 *   
 *   Note that these routines might appear to be redundant with the 'rows'
 *   and 'columns' information returned from os_banner_getinfo(), but these
 *   have two important distinctions.  First, these routines return only the
 *   width and height information, so they can be implemented with less
 *   overhead than os_banner_getinfo(); this is important because formatters
 *   might need to call these routines frequently while formatting text.
 *   Second, these routines are not required to return an approximation for
 *   windows using proportional fonts, as os_banner_getinfo() does; these can
 *   simply return zero when a proportional font is in use.  
 */
int os_banner_get_charwidth(void *banner_handle);
int os_banner_get_charheight(void *banner_handle);

/* clear the contents of a banner */
void os_banner_clear(void *banner_handle);

/* 
 *   Display output on a banner.  Writes the output to the window on the
 *   display at the current output position.
 *   
 *   The following special characters should be recognized and handled:
 *   
 *   '\n' - newline; move output position to the start of the next line.
 *   
 *   '\r' - move output position to start of current line; subsequent text
 *   overwrites any text previously displayed on the current line.  It is
 *   permissible to delete the old text immediately on seeing the '\r',
 *   rather than waiting for additional text to actually overwrite it.
 *   
 *   All other characters should simply be displayed as ordinary printing
 *   text characters.  Note that tab characters should not be passed to this
 *   routine, but if they are, they can simply be treated as ordinary spaces
 *   if desired.  Other control characters (backspace, escape, etc) should
 *   never be passed to this routine; the implementation is free to ignore
 *   any control characters not listed above.
 *   
 *   If any text displayed here overflows the current boundaries of the
 *   window on the screen, the text MUST be "clipped" to the current window
 *   boundaries; in other words, anything this routine tries to display
 *   outside of the window's on-screen rectangle must not actually be shown
 *   on the screen.
 *   
 *   Text overflowing the display boundaries MUST also be retained in an
 *   internal buffer.  This internal buffer can be limited to the actual
 *   maximum display size of the terminal screen or application window, if
 *   desired.  It is necessary to retain clipped text, because this allows a
 *   window to be expanded to the size of its contents AFTER the contents
 *   have already been displayed.
 *   
 *   If the banner does its own line wrapping, it must indicate this via the
 *   os_line_wrap flag in the os_banner_getinfo() return data.  If the
 *   banner doesn't indicate this flag, then it must not do any line
 *   wrapping at all, even if the caller attempts to write text beyond the
 *   right edge of the window - any text overflowing the width of the window
 *   must simply be clipped.
 *   
 *   Text grid banners must ALWAYS clip - these banners should never perform
 *   any line wrapping.  
 */
void os_banner_disp(void *banner_handle, const char *txt, size_t len);

/*
 *   Set the text attributes in a banner, for subsequent text displays.
 *   'attr' is a (bitwise-OR'd) combination of OS_ATTR_xxx values. 
 */
void os_banner_set_attr(void *banner_handle, int attr);

/* 
 *   Set the text color in a banner, for subsequent text displays.  The 'fg'
 *   and 'bg' colors are given as RGB or parameterized colors; see the
 *   definition of os_color_t for details.
 *   
 *   If the underlying renderer is HTML-enabled, then this should not be
 *   used; the appropriate HTML code should simply be displayed to the
 *   banner instead.  
 */
void os_banner_set_color(void *banner_handle, os_color_t fg, os_color_t bg);

/* 
 *   Set the screen color in the banner - this is analogous to the screen
 *   color in the main text area.
 *   
 *   If the underlying renderer is HTML-enabled, then this should not be
 *   used; the HTML <BODY> tag should be used instead.  
 */
void os_banner_set_screen_color(void *banner_handle, os_color_t color);

/* flush output on a banner */
void os_banner_flush(void *banner_handle);

/*
 *   Set the banner's size.  The size has the same meaning as in
 *   os_banner_create().
 *   
 *   'is_advisory' indicates whether the sizing is required or advisory only.
 *   If this flag is false, then the size should be set as requested.  If
 *   this flag is true, it means that the caller intends to call
 *   os_banner_size_to_contents() at some point, and that the size being set
 *   now is for advisory purposes only.  Platforms that support
 *   size-to-contents may simply ignore advisory sizing requests, although
 *   they might want to ensure that they have sufficient off-screen buffer
 *   space to keep track of the requested size of display, so that the
 *   information the caller displays in preparation for calling
 *   size-to-contents will be retained.  Platforms that do not support
 *   size-to-contents should set the requested size even when 'is_advisory'
 *   is true.  
 */
void os_banner_set_size(void *banner_handle, int siz, int siz_units,
                        int is_advisory);

/* 
 *   Set the banner to the size of its current contents.  This can be used
 *   to set the banner's size after some text (or other material) has been
 *   displayed to the banner, so that the size can be set according to the
 *   banner's actual space requirements.
 *   
 *   This changes the banner's "requested size" to match the current size.
 *   Subsequent calls to os_banner_getinfo() will thus indicate a requested
 *   size according to the size set here.  
 */
void os_banner_size_to_contents(void *banner_handle);

/* 
 *   Turn HTML mode on/off in the banner window.  If the underlying renderer
 *   doesn't support HTML, these have no effect.  
 */
void os_banner_start_html(void *banner_handle);
void os_banner_end_html(void *banner_handle);

/*
 *   Set the output coordinates in a text grid window.  The grid window is
 *   arranged into character cells numbered from row zero, column zero for
 *   the upper left cell.  This function can only be used if the window was
 *   created with type OS_BANNER_TYPE_TEXTGRID; the request should simply be
 *   ignored by other window types.
 *   
 *   Moving the output position has no immediate effect on the display, and
 *   does not itself affect the "content size" for the purposes of
 *   os_banner_size_to_contents().  This simply sets the coordinates where
 *   any subsequent text is displayed.  
 */
void os_banner_goto(void *banner_handle, int row, int col);


/* ------------------------------------------------------------------------ */
/*
 *   Get system information.  'code' is a SYSINFO_xxx code, which
 *   specifies what type of information to get.  The 'param' argument's
 *   meaning depends on which code is selected.  'result' is a pointer to
 *   an integer that is to be filled in with the result value.  If the
 *   code is not known, this function should return FALSE.  If the code is
 *   known, the function should fill in *result and return TRUE.
 */
int os_get_sysinfo(int code, void *param, long *result);

/* determine if systemInfo is supported - os_get_sysinfo never gets this */
#define SYSINFO_SYSINFO   1

/* get interpreter version number - os_get_sysinfo never gets this */
#define SYSINFO_VERSION   2

/* get operating system name - os_get_sysinfo never gets this */
#define SYSINFO_OS_NAME   3

/* 
 *   Can the system process HTML directives?  returns 1 if so, 0 if not.
 *   Note that if this returns false, then all of the codes below from
 *   JPEG to LINKS are implicitly false as well, since TADS can only use
 *   images, sounds, and links through HTML. 
 */
#define SYSINFO_HTML      4

/* can the system display JPEG's?  1 if yes, 0 if no */
#define SYSINFO_JPEG      5

/* can the system display PNG's?  1 if yes, 0 if no */
#define SYSINFO_PNG       6

/* can the system play WAV's?  1 if yes, 0 if no */
#define SYSINFO_WAV       7

/* can the system play MIDI's?  1 if yes, 0 if no */
#define SYSINFO_MIDI      8

/* can the system play MIDI and WAV's simultaneously?  yes=1, no=0 */
#define SYSINFO_WAV_MIDI_OVL  9

/* can the system play multiple WAV's simultaneously?  yes=1, no=0 */
#define SYSINFO_WAV_OVL   10

/*
 *   GENERAL NOTES ON PREFERENCE SETTINGS:
 *   
 *   Several of the selectors below refer to the preference settings.  We're
 *   talking about user-settable options to control various aspects of the
 *   interpreter.  The conventional GUI for this kind of thing is a dialog
 *   box reachable through a menu command named something like "Options" or
 *   "Preferences".  A couple of general notes about these:
 *   
 *   1.  The entire existence of a preferences mechanism is optional -
 *   interpreter writers aren't required to implement anything along these
 *   lines.  In some cases the local platforms might not have any suitable
 *   conventions for a preferences UI (e.g., character-mode console
 *   applications), and in other cases the terp developer might just want to
 *   omit a prefs mechanism because of the work involved to implement it, or
 *   to keep the UI simpler.
 *   
 *   2.  If a given SYSINFO_PREF_xxx selector refers to a preference item
 *   that's not implemented in the local interpreter, the terp should simply
 *   return a suitable default result.  For example, if the interpreter
 *   doesn't have a preference item to allow the user to turn sounds off, the
 *   selector SYSINFO_PREF_SOUNDS should return 1 to indicate that the user
 *   has not in fact turned off sounds (because there's no way to do so).
 *   
 *   3.  The various SYSINFO_PREF_xxx selectors are purely queries - they're
 *   NOT a mechanism for enforcing the preferences.  For example, if the
 *   interpreter provides a "Sounds On/Off" option, it's up to the terp to
 *   enforce it the Off setting by ignoring any sound playback requests.  The
 *   game isn't under any obligation to query any of the preferences or to
 *   alter its behavior based on their settings - you should expect that the
 *   game will go on trying to play sounds even when "Sounds Off" is selected
 *   in the preferences.  The purpose of these SYSINFO selectors is to let
 *   the game determine the current presentation status, but *only if it
 *   cares*.  For example, the game might determine whether or not sounds are
 *   actually being heard just before playing a sound effect that's important
 *   to the progress of the game, so that it can provide a visual alternative
 *   if the effect won't be heard.  
 */

/* 
 *   Get image preference setting - 1 = images can be displayed, 0 = images
 *   are not being displayed because the user turned off images in the
 *   preferences.  This is, of course, irrelevant if images can't be
 *   displayed at all.
 *   
 *   See the general notes on preferences queries above.  
 */
#define SYSINFO_PREF_IMAGES  11

/*
 *   Get digitized sound effect (WAV) preference setting - 1 = sounds can be
 *   played, 0 = sounds are not being played because the user turned off
 *   sounds in the preferences.
 *   
 *   See the general notes on preferences queries above.  
 */
#define SYSINFO_PREF_SOUNDS  12

/*
 *   Get music (MIDI) preference setting - 1 = music can be played, 0 = music
 *   is not being played because the user turned off music in the
 *   preferences.
 *   
 *   See the general notes on preferences queries above.  
 */
#define SYSINFO_PREF_MUSIC   13

/*
 *   Get link display preference setting - 0 = links are not being displayed
 *   because the user set a preference item that suppresses all links (which
 *   doesn't actually hide them, but merely displays them and otherwise
 *   treats them as ordinary text).  1 = links are to be displayed normally.
 *   2 = links can be displayed temporarily by the user by pressing a key or
 *   some similar action, but aren't being displayed at all times.  
 *   
 *   See the general note on preferences queries above.  
 */
#define SYSINFO_PREF_LINKS   14

/* can the system play MPEG sounds of any kind? */
#define SYSINFO_MPEG         15

/* can the system play MPEG audio 2.0 layer I/II/III sounds? */
#define SYSINFO_MPEG1        16
#define SYSINFO_MPEG2        17
#define SYSINFO_MPEG3        18

/* 
 *   is the system *currently* in HTML mode?  os_get_sysinfo never gets
 *   this code, since the portable output layer keeps track of this 
 */
#define SYSINFO_HTML_MODE    19

/* 
 *   Does the system allow following external URL links of the various
 *   types?  These return true if the system is capable of following these
 *   types of hypertext links, false if not.  Note that, if the system is
 *   capable of following these links, these should return true regardless
 *   of any current mode settings; in particular, these should not be
 *   sensitive to the current HTML mode or the current link display mode,
 *   since the question is not whether a link now displayed can be
 *   followed by the user, but rather whether the system has the
 *   capability to follow these types of links at all.  
 */
#define SYSINFO_LINKS_HTTP   20
#define SYSINFO_LINKS_FTP    21
#define SYSINFO_LINKS_NEWS   22
#define SYSINFO_LINKS_MAILTO 23
#define SYSINFO_LINKS_TELNET 24

/* is PNG transparency supported? */
#define SYSINFO_PNG_TRANS    25

/* is PNG alpha blending supported? */
#define SYSINFO_PNG_ALPHA    26

/* is the Ogg Vorbis audio format supported? */
#define SYSINFO_OGG          27

/* can the system display MNG's? */
#define SYSINFO_MNG          28

/* can the system display MNG's with transparency? */
#define SYSINFO_MNG_TRANS    29

/* can the system display MNG's with alpha blending? */
#define SYSINFO_MNG_ALPHA    30

/* can we display highlighted text in its own appearance? */
#define SYSINFO_TEXT_HILITE  31

/* 
 *   Can we display text colors?  This returns a SYSINFO_TXC_xxx code
 *   indicating the level of color support.
 *   
 *   The os_xxx interfaces don't presently support anything beyond the ANSI
 *   colors; however, HTML-enabled interpreters generally support full RGB
 *   colors, so we call this out as a separate level.  
 */
#define SYSINFO_TEXT_COLORS  32

/* no text color support */
#define SYSINFO_TXC_NONE      0

/* parameterized color names only (OS_COLOR_P_TEXT, etc) */
#define SYSINFO_TXC_PARAM     1

/* 
 *   we support only the basic ANSI colors, foreground control only (white,
 *   black, blue, red, green, yellow, cyan, magenta) 
 */
#define SYSINFO_TXC_ANSI_FG   2

/* ANSI colors, foreground and background */
#define SYSINFO_TXC_ANSI_FGBG 3

/* full RGB support */
#define SYSINFO_TXC_RGB       4

/* are the os_banner_xxx() interfaces supported? */
#define SYSINFO_BANNERS      33

/* Interpreter Class - this returns one of the SYSINFO_ICLASS_xxx codes */
#define SYSINFO_INTERP_CLASS 34

/* 
 *   Interpreter class: Character-mode Text-Only.  Interpreters of this class
 *   use a single, fixed-pitch font to display all text, and use the
 *   text-only HTML subset, and cannot display graphics.
 */
#define SYSINFO_ICLASS_TEXT    1

/* 
 *   Interpreter class: Text-Only GUI.  Interpreters of this class are
 *   traditional text-only interpreters running on graphical operating
 *   systems.  These interpreters might use multiple fonts (for example, they
 *   might display highlighted text in boldface), and might use
 *   proportionally-spaced text for some windows.  These interpreters support
 *   the text-only HTML subset, and cannot display graphics.
 *   
 *   Text-only GUI interpreters act essentially the same as character-mode
 *   text-only interpreters, from the perspective of the client program.  
 */
#define SYSINFO_ICLASS_TEXTGUI 2

/*
 *   Interpreter class: HTML.  Interpreters of this class can display
 *   graphics and sounds, can display multiple fonts and font sizes, can use
 *   proportional fonts, and support the full HTML TADS markup language for
 *   formatting.  
 */
#define SYSINFO_ICLASS_HTML    3

/*
 *   Audio fade information.
 *   
 *   SYSINFO_AUDIO_FADE: basic fade-in and fade-out support.  Interpreters
 *   that don't support audio fade at all should return 0.  Interpreters that
 *   support fades should return a bitwise OR'd combination of
 *   SYSINFO_AUDIOFADE_xxx flags below indicating which formats can be used
 *   with fades.
 *   
 *   SYSINFO_AUDIO_CROSSFADE: cross-fades are supported (i.e., simultaneous
 *   play of overlapping tracks, one fading out while the other fades in).
 *   If cross-fades aren't supported, return 0.  If they're supported, return
 *   a combination of SYSINFO_AUDIOFADE_xxx flags indicating which formats
 *   can be used with cross-fades.  
 */
#define SYSINFO_AUDIO_FADE       35
#define SYSINFO_AUDIO_CROSSFADE  36

/* 
 *   Specific audio fading features.  These are bit flags that can be
 *   combined to indicate the fading capabilities of the interpreter.  
 */
#define SYSINFO_AUDIOFADE_MPEG  0x0001          /* supported for MPEG audio */
#define SYSINFO_AUDIOFADE_OGG   0x0002          /* supported for Ogg Vorbis */
#define SYSINFO_AUDIOFADE_WAV   0x0004                 /* supported for WAV */
#define SYSINFO_AUDIOFADE_MIDI  0x0008                /* supported for MIDI */


/* ------------------------------------------------------------------------ */
/*
 *   Integer division operators.  For any compiler that follows ANSI C
 *   rules, no definitions are required for these routine, since the
 *   standard definitions below will work properly.  However, if your
 *   compiler does not follow ANSI standards with respect to integer
 *   division of negative numbers, you must provide implementations of
 *   these routines that produce the correct results.
 *   
 *   Division must "truncate towards zero," which means that any
 *   fractional part is dropped from the result.  If the result is
 *   positive, the result must be the largest integer less than the
 *   algebraic result: 11/3 yields 3.  If the result is negative, the
 *   result must be the smallest integer less than the result: (-11)/3
 *   yields -3.
 *   
 *   The remainder must obey the relationship (a/b)*b + a%b == a, for any
 *   integers a and b (b != 0).
 *   
 *   If your compiler does not obey the ANSI rules for the division
 *   operators, make the following changes in your osxxx.h file
 *   
 *   - define the symbol OS_NON_ANSI_DIVIDE in the osxxx.h file
 *   
 *   - either define your own macros for os_divide_long() and
 *   os_remainder_long(), or put actual prototypes for these functions
 *   into your osxxx.h file and write appropriate implementations of these
 *   functions in one of your osxxx.c or .cpp files.
 */
/* long os_divide_long(long a, long b);    // returns (a/b) with ANSI rules */
/* long os_remainder_long(long a, long b); // returns (a%b) with ANSI rules */

/* standard definitions for any ANSI compiler */
#ifndef OS_NON_ANSI_DIVIDE
#define os_divide_long(a, b)     ((a) / (b))
#define os_remainder_long(a, b)  ((a) % (b))
#endif

/* ------------------------------------------------------------------------ */
/*
 *   Special "switch" statement optimization flags.  These definitions are
 *   OPTIONAL - if a platform doesn't provide these definitions, suitable
 *   code that's fully portable will be used.
 *   
 *   On some compilers, the performance of a "switch" statement can be
 *   improved by fully populating the switch with all possible "case"
 *   values.  When the compiler can safely assume that every possible "case"
 *   value is specifically called out in the switch, the compiler can
 *   generate somewhat faster code by omitting any range check for the
 *   controlling expression of the switch: a range check is unnecessary
 *   because the compiler knows that the value can never be outside the
 *   "case" table.
 *   
 *   This type of optimization doesn't apply to all compilers.  When a given
 *   platform's compiler can be coerced to produce faster "switch"
 *   statements, though, there might be some benefit in defining these
 *   symbols according to local platform rules.
 *   
 *   First, #define OS_FILL_OUT_CASE_TABLES if you want this type of switch
 *   optimization at all.  This symbol is merely a flag, so it doesn't need
 *   a value - #defining it is enough to activate the special code.  If you
 *   don't define this symbol, then the code that cares about this will
 *   simply generate ordinary switches, leaving holes in the case table and
 *   using "default:" to cover them.  If the platform's osxxx.h header does
 *   #define OS_FILL_OUT_CASE_TABLES, then the portable code will know to
 *   fill out case tables with all possible alternatives where it's possible
 *   and potentially beneficial to do so.
 *   
 *   Second, if you #define OS_FILL_OUT_CASE_TABLES, you MUST ALSO #define
 *   OS_IMPOSSIBLE_DEFAULT_CASE.  The value for this symbol must be some
 *   code to insert into a "switch" statement at the point where a
 *   "default:" case would normally go.  On some compilers, special
 *   extensions allow the program to explicitly indicate within a switch
 *   that all possible cases are covered, so that the compiler doesn't have
 *   to be relied upon to infer this for itself (which would be impossible
 *   for it to do in many cases anyway).  You can use an empty definition
 *   for this symbol if your compiler doesn't have any special construct for
 *   indicating a fully-populated case table.
 *   
 *   Note that *most* switch statements in portable code won't care one way
 *   or the other about these definitions.  There's a time/space trade-off
 *   in fully populating a switch's case table, so only the most
 *   time-critical code will bother trying.  
 */



/* ------------------------------------------------------------------------ */
/*
 *   TADS 2 swapping configuration.  Define OS_DEFAULT_SWAP_ENABLED to 0
 *   if you want swapping turned off, 1 if you want it turned on.  If we
 *   haven't defined a default swapping mode yet, turn swapping on by
 *   default.  
 */
#ifndef OS_DEFAULT_SWAP_ENABLED
# define OS_DEFAULT_SWAP_ENABLED   1
#endif

/*
 *   If the system "long description" (for the banner) isn't defined, make
 *   it the same as the platform ID string.  
 */
#ifndef OS_SYSTEM_LDESC
# define OS_SYSTEM_LDESC  OS_SYSTEM_NAME
#endif

/*
 *   TADS 2 Usage Lines
 *   
 *   If the "usage" lines (i.e., the descriptive lines of text describing
 *   how to run the various programs) haven't been otherwise defined,
 *   define defaults for them here.  Some platforms use names other than
 *   tc, tr, and tdb for the tools (for example, on Unix they're usually
 *   tadsc, tadsr, and tadsdb), so the usage lines should be adjusted
 *   accordingly; simply define them earlier than this point (in this file
 *   or in one of the files included by this file, such as osunixt.h) for
 *   non-default text.  
 */
#ifndef OS_TC_USAGE
# define OS_TC_USAGE  "usage: tc [options] file"
#endif
#ifndef OS_TR_USAGE
# define OS_TR_USAGE  "usage: tr [options] file"
#endif
#ifndef OS_TDB_USAGE
# define OS_TDB_USAGE  "usage: tdb [options] file"
#endif

/*
 *   Ports can define a special TDB startup message, which is displayed
 *   after the copyright/version banner.  If it's not defined at this
 *   point, define it to an empty string.  
 */
#ifndef OS_TDB_STARTUP_MSG
# define OS_TDB_STARTUP_MSG ""
#endif

/*
 *   If a system patch sub-level isn't defined, define it here as zero.
 *   The patch sub-level is used on some systems where a number of ports
 *   are derived from a base port (for example, a large number of ports
 *   are based on the generic Unix port).  For platforms like the Mac,
 *   where the porting work applies only to that one platform, this
 *   sub-level isn't meaningful.
 */
#ifndef OS_SYSTEM_PATCHSUBLVL
# define OS_SYSTEM_PATCHSUBLVL  "0"
#endif

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
