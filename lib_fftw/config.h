
#include <picross/pic_config.h>

#ifdef PI_WINDOWS
//#define FFTW_DLL 1
//#define COMPILING_FFTW 1
#include <config_win.h>
#endif

#ifdef PI_LINUX
#include <config_linux.h>
#endif

#ifdef PI_MACOSX
#include <config_mac.h>
#endif
