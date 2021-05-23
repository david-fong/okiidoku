#ifndef HPP_BUILDFLAG
#define HPP_BUILDFLAG

/**
 * Whether or not ansi-escape sequences should be emitted within output
 * to dim out harsh, non-essential text.
 */
#define USE_ANSI_ESC true

/**
 * Whether or not to enable windows ansi escape code handling.
 */
#define WINDOWS_ANSI (USE_ANSI_ESC && false)

#endif
