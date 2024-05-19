#ifndef JUSTANOTHERCATGIRL_HEADERS_TTYCOLOR
#define JUSTANOTHERCATGIRL_HEADERS_TTYCOLOR

/// This header is a collection of macros and escape sequences for ANSII terminals.
/// It has common tempalte for abbreviation <Letter1><Letter2(optional)><3 letters>.
/// * First letter: C for Color (text color), B for Backdround (bg color), T for Text (text mode)
/// * Second letter: B for Bright, N for uNset (only in combination with T as 1st letter)
/// * 3 last letters can be checked in comments after each definition

#include "utility.h"


#define PAINT_STRING(string, args...) ("\x1b[" arg1 ";" arg2 ";" ... "m" string ARES)

#define ARES "\x1b[0m" // All RESet

// resular text colors
#define CBLK "\x1b[30m" // black
#define CRED "\x1b[31m" // red
#define CGRN "\x1b[32m" // green
#define CYLW "\x1b[33m" // yellow
#define CBLU "\x1b[34m" // blue
#define CMAG "\x1b[35m" // magenta
#define CCYN "\x1b[36m" // cyan
#define CWHT "\x1b[37m" // white
#define CDEF "\x1b[39m" // default
#define CBLK "\x1b[30m" // black

// regular background colors 
#define BBLK "\x1b[40m" // black
#define BRED "\x1b[41m" // red
#define BGRN "\x1b[42m" // green
#define BYLW "\x1b[43m" // yellow
#define BBLU "\x1b[44m" // blue
#define BMAG "\x1b[45m" // magenta
#define BCYN "\x1b[46m" // cyan
#define BWHT "\x1b[47m" // white
#define BDEF "\x1b[49m" // default
#define BBLK "\x1b[40m" // black


// bright text colors
#define CBBLK "\x1b[90m" // bright black
#define CBRED "\x1b[91m" // bright red
#define CBGRN "\x1b[92m" // bright green
#define CBYLW "\x1b[93m" // bright yellow
#define CBBLU "\x1b[94m" // bright blue
#define CBMAG "\x1b[95m" // bright magenta
#define CBCYN "\x1b[96m" // bright cyan
#define CBWHT "\x1b[97m" // bright white

// bright baclground colors 
#define BBBLK "\x1b[100m" // bright black
#define BBRED "\x1b[101m" // bright red
#define BBGRN "\x1b[102m" // bright green
#define BBYLW "\x1b[103m" // bright yellow
#define BBBLU "\x1b[104m" // bright blue
#define BBMAG "\x1b[105m" // bright magenta
#define BBCYN "\x1b[106m" // bright cyan
#define BBWHT "\x1b[107m" // bright white


// text style set
#define TSBLD "\x1b[1m" // bold
#define TSDIM "\x1b[2m" // dim/faint
#define TSITL "\x1b[3m" // italic
#define TSUND "\x1b[4m" // underline
#define TSBNK "\x1b[5m" // blinking
#define TSINV "\x1b[7m" // inverse/reverse
#define TSHID "\x1b[8m" // hidden/invisible
#define TSSTR "\x1b[9m" // strikethrough

// text style unset
#define TNBLD "\x1b[22m" // bold
#define TNDIM "\x1b[22m" // dim/faint
#define TNITL "\x1b[23m" // italic
#define TNUND "\x1b[24m" // underline
#define TNBNK "\x1b[25m" // blinking
#define TNINV "\x1b[27m" // inverse/reverse
#define TNHID "\x1b[28m" // hidden/invisible
#define TNSTR "\x1b[29m" // strikethrough

#endif //JUSTANOTHERCATGIRL_HEADERS_TTYCOLOR

