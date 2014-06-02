
#include "SliderEx.h"



SliderEx::SliderEx(int aSnapSize)
{
	snapSize=aSnapSize;
}

SliderEx::SliderEx(const String& componentName,int aSnapSize) : Slider(componentName)
{
	snapSize=aSnapSize;
}

void SliderEx::setSnapSize(int newSnapSize)
{ 
	snapSize=newSnapSize;
} 

int SliderEx::getSnapSize()
{
	return snapSize;
}


void SliderEx::mouseDrag (const MouseEvent& e)
{
    snap=e.mods.isShiftDown();
	Slider::mouseDrag(e);
}


double SliderEx::snapValue(double attemptedValue, DragMode dragMode)
{
	if (!snap || dragMode==notDragging)
	{
		return Slider::snapValue(attemptedValue, dragMode);
	}
	int v=((int) (attemptedValue/snapSize) ) * snapSize;
	return Slider::snapValue(v, dragMode);
	
}

