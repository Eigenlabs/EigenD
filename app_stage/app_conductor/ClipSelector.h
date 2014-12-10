#ifndef __CLIPSELECT__
#define __CLIPSELECT__

#include "juce.h"
#include "Clip.h"
class ClipSelector
{

public:
    ClipSelector(){};
    virtual ~ClipSelector(){};
    virtual void clipSelected(Clip*){};
};


#endif
