
#include "juce.h"


class SliderEx : public Slider
{
public:
	SliderEx(int aSnapSize=10);
	SliderEx(const String &componentName,int aSnapSize=10);
	void mouseDrag (const MouseEvent& e);
	double snapValue(double attemptedValue, DragMode dragMode);
	void setSnapSize(int snapSize);
	int getSnapSize();
private:
	bool snap;
	int snapSize;
};
