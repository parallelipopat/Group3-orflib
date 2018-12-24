/** 
@file  defines.hpp
@brief Library-wide version numbers, macro, and constant definitions
*/

#ifndef ORF_DEFINES_HPP
#define ORF_DEFINES_HPP

/** version string */
#ifdef _DEBUG
#define ORF_VERSION_STRING "0.10.0-debug"
#else
#define ORF_VERSION_STRING "0.10.0"
#endif

/** version numbers */
#define ORF_VERSION_MAJOR 0
#define ORF_VERSION_MINOR 10
#define ORF_VERSION_REVISION 0

/** Macro for namespaces */
#define BEGIN_NAMESPACE(x)	namespace x {
#define END_NAMESPACE(x)	}

/** Macro for Extern C */
#define BEGIN_EXTERN_C  extern "C" {
#define END_EXTERN_C    }

/** number of days in a year */
#define DAYS_PER_YEAR 365.25

/** number of seconds in a day */
#define SECS_PER_DAY 86400. //(24.*60*60)
#define SECS_PER_DAY_LONG 86400L

/** number of seconds in a year */
#define SECS_PER_YEAR (SECS_PER_DAY*DAYS_PER_YEAR)

/** number of seconds in an hour */
#define SECS_PER_HOUR 3600.

/** e */
#define M_E 2.71828182845904523536
/** log2(e) */
#define M_LOG2E 1.44269504088896340736
/** log10(e) */
#define M_LOG10E 0.434294481903251827651
/** ln(2) */
#define M_LN2 0.693147180559945309417
/** ln(10) */
#define M_LN10 2.30258509299404568402
/** pi */
#define M_PI 3.14159265358979323846
/** pi/2 */
#define M_PI_2 1.57079632679489661923
/** pi/4 */
#define M_PI_4 0.785398163397448309616
/** 1/pi */
#define M_1_PI 0.318309886183790671538
/** 2/pi */
#define M_2_PI 0.636619772367581343076
/** 2/sqrt(pi) */
#define M_2_SQRTPI 1.12837916709551257390
/** sqrt(2) */
#define M_SQRT2 1.41421356237309504880
/** 1/sqrt(2) */
#define M_SQRT1_2 0.707106781186547524401
/** sqrt(pi) */
#define M_SQRTPI  1.77245385090551602792981
/** 1/sqrt(pi) */
#define M_1_SQRTPI  0.564189583547756286948
/** 1/sqrt(2*pi) */
#define M_1_SQRT2PI  0.398942280401432678

#endif // ORF_DEFINES_HPP
