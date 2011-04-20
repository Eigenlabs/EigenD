/*
    This file contains settings that you might want to explicitly apply to 
    your Juce build.

    These flags enable or disable juce features - if you're linking to juce as
    a library, then to change them, you'd need to alter your juce_Config.h file and
    recompile the juce lib. But because we're using the amalgamated file, you can
    just include this file before including your juce_amalgamated.cpp file to
    have the same effect.

    If you leave any of these commented-out, they'll take on the default value 
    assigned to them in juce_Config.h, so to force them on or off, just set them
    to an explicit 0 or 1 in here.
*/

#include <picross/pic_config.h>

#define JUCE_QUICKTIME  0
#define JUCE_FORCE_DEBUG  0
#define JUCE_LOG_ASSERTIONS 0
#define JUCE_ASIO  1
#define JUCE_WASAPI  1
#define JUCE_ALSA  1
#define JUCE_JACK  0
#define JUCE_QUICKTIME  0
#define JUCE_OPENGL  0
#define JUCE_USE_FLAC  0
#define JUCE_USE_OGGVORBIS  0
#define JUCE_USE_CDBURNER  0
#define JUCE_ENABLE_REPAINT_DEBUGGING  0
#define JUCE_USE_XINERAMA  0
#define JUCE_USE_XSHM  1
#define JUCE_INCLUDE_PNGLIB_CODE      1
#ifndef PI_LINUX
#define JUCE_PLUGINHOST_VST  1
#define JUCE_PLUGINHOST_AU  1
#endif
#define JUCE_CHECK_MEMORY_LEAKS  0
#define JUCE_CATCH_UNHANDLED_EXCEPTIONS  1
#define JUCE_STRINGS_ARE_UNICODE  1
#define DONT_SET_USING_JUCE_NAMESPACE 1
#define JUCE_SUPPORT_CARBON 1
#define NDEBUG 1
