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

#define JUCE_MODULE_AVAILABLE_juce_audio_basics          1
#define JUCE_MODULE_AVAILABLE_juce_audio_devices         1
#define JUCE_MODULE_AVAILABLE_juce_audio_formats         1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors      1
#define JUCE_MODULE_AVAILABLE_juce_audio_utils           1
#define JUCE_MODULE_AVAILABLE_juce_core                  1
#define JUCE_MODULE_AVAILABLE_juce_cryptography          1
#define JUCE_MODULE_AVAILABLE_juce_data_structures       1
#define JUCE_MODULE_AVAILABLE_juce_events                1
#define JUCE_MODULE_AVAILABLE_juce_graphics              1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics            1
#define JUCE_MODULE_AVAILABLE_juce_gui_extra             1

#define JUCE_ASIO 1
#define JUCE_WASAPI 1
#define JUCE_DIRECTSOUND 1
#define JUCE_ALSA 1
#define JUCE_JACK 0
#define JUCE_USE_ANDROID_OPENSLES 0
#define JUCE_USE_CDREADER 0
#define JUCE_USE_CDBURNER 0

#define JUCE_USE_FLAC 0
#define JUCE_USE_OGGVORBIS 0
#define JUCE_USE_MP3AUDIOFORMAT 0
#define JUCE_USE_WINDOWS_MEDIA_FORMAT 0

#ifndef PI_LINUX
#define JUCE_PLUGINHOST_VST 1
#define JUCE_PLUGINHOST_VST3 1
#define JUCE_VST3_CAN_REPLACE_VST2 0
#define JUCE_PLUGINHOST_AU 1
#endif

// #define JUCE_FORCE_DEBUG 1
// #define JUCE_LOG_ASSERTIONS 1
// #define JUCE_CHECK_MEMORY_LEAKS 1
#define JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES 1
// #define JUCE_CATCH_UNHANDLED_EXCEPTIONS 1

#define JUCE_USE_COREIMAGE_LOADER 0
#define JUCE_USE_DIRECTWRITE 0

#define JUCE_ENABLE_REPAINT_DEBUGGING 0
#define JUCE_USE_XSHM 1
#define JUCE_USE_XRENDER 0
#define JUCE_USE_XCURSOR 0

#define JUCE_WEB_BROWSER 0

#define JUCE_DIRECTSHOW 0
#define JUCE_MEDIAFOUNDATION 0
#define JUCE_QUICKTIME 0
#define JUCE_USE_CAMERA 0

#define DONT_SET_USING_JUCE_NAMESPACE 1

#ifdef PI_MACOSX_8664
#define JUCE_SUPPORT_CARBON 0
#else
#define JUCE_SUPPORT_CARBON 1
#endif

#define NDEBUG 1
