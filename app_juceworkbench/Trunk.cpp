/*
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Trunk.h"

TrunkDrawingTool::TrunkDrawingTool(int x, int y, float zoomFactor, bool append)
{
    zoomFactor_=zoomFactor;
    orientation_=Trunk::HORIZONTAL;
    orientationSet_=false;
    movedOffLastPoint_=false;
    appendTrunkOrientation_=0;
    appendTrunkX_=0;
    appendTrunkY_=0;
    joiningCornerType_=0;
    showJoiningCorner_=append;
    addPoint(x,y);
    mx_=x;
    my_=y;
}

void TrunkDrawingTool::setJoiningCornerParameters(int tOrientation,int tx,int ty)
{
    appendTrunkOrientation_=tOrientation;
    appendTrunkX_=tx;
    appendTrunkY_=ty;
}

int TrunkDrawingTool::getLowerBound(float zoomFactor)
{
    int n =getNumSections();
    int lowerBound=0;
    if (n>0)
    {
        TrunkSection* test=getSection(0);
        if(test->getOrientation()==Trunk::HORIZONTAL)
        {
            if (test->getWidth()<56*zoomFactor)
            {
                lowerBound=1;
                pic::logmsg()<<"Horizontal trunk width < critical value and is ignored";
            }
        }
        else
        {
            if (test->getHeight()<56*zoomFactor)
            {
                lowerBound=1;
                pic::logmsg()<<"Vertical trunk height < critical value and is ignored";
            }
        }

        delete test;
    }
    return lowerBound;
}

void TrunkDrawingTool::draw(Graphics& g)
{

    if(showJoiningCorner_)
    {
        if(pointsX_.size()>=1)
        {
            if (pointsX_.size()==1)
            {
                joiningCornerType_=getInitialJoiningCornerType();
            }
            drawCorner(g,pointsX_[0],pointsY_[0],joiningCornerType_);
        }
    }

    unsigned msize=2;
    if (pointsX_.size()>=msize)
    {
        for(unsigned i =0; i<pointsX_.size()-1;i++)
        {
            drawTrunk(g,pointsX_[i], pointsY_[i],pointsX_[i+1], pointsY_[i+1]);
            drawCorner(g,pointsX_[i+1], pointsY_[i+1], getCornerType(i) );
        }
    }
    drawTrunk(g,pointsX_.back(),pointsY_.back(), currentX_, currentY_);
}

void TrunkDrawingTool::getRectangle(juce::Rectangle <int>&u)
{
      int minX=20000;
      int maxX=0;
      int minY=20000;
      int maxY=0;
    unsigned n =pointsX_.size();
//    juce::Point<float> points[n+1];
    for(unsigned i =0; i<n;i++)
    {
        if(pointsX_[i]<minX)
        {
            minX=pointsX_[i];
        }
        if(pointsY_[i]<minY)
        {
            minY=pointsY_[i];
        }
        if(pointsX_[i]>maxX)
        {
            maxX=pointsX_[i];
        }
        if(pointsY_[i]>maxY)
        {
            maxY=pointsY_[i];
        }
    }
    if(mx_<minX)
    {
        minX=mx_;
    }
    if(my_<minY)
    {
        minY=my_;
    }
    if(mx_>maxX)
    {
        maxX=mx_;
    }
    if(my_>maxY)
    {
        maxY=my_;
    }

    u=juce::Rectangle<int>::leftTopRightBottom(minX,minY,maxX,maxY);
}

int TrunkDrawingTool::getJoiningCornerPosX()
{
    return pointsX_[0];
}

int TrunkDrawingTool::getJoiningCornerPosY()
{
    return pointsY_[0];
}

int TrunkDrawingTool::getInitialJoiningCornerType()
{       
    if (appendTrunkOrientation_==Trunk::HORIZONTAL)
    {
        if(appendTrunkX_<pointsX_[0])
        {
            if(pointsY_[0]>currentY_)
            {
                return TrunkCorner::WEST_NORTH;
            }
            else
            {
                return TrunkCorner::WEST_SOUTH;
            }
        }
        else
        {
            if(pointsY_[0]>currentY_)
            {
                return TrunkCorner::EAST_NORTH;
            }
            else
            {
                return TrunkCorner::EAST_SOUTH;
            }
        }

    }
    else
    {
        if (appendTrunkY_<pointsY_[0])
        {
              if(pointsX_[0]>currentX_)
              {
                  return TrunkCorner::NORTH_WEST;
              }
              else
              {
                  return TrunkCorner::NORTH_EAST;
              }
            
        }
        else 
        {
            if(pointsX_[0]>currentX_)
            {
                return TrunkCorner::SOUTH_WEST;
            }
            else
            {
                return TrunkCorner::SOUTH_EAST;
            }
        }
    }

    return 0;
}
int TrunkDrawingTool::getJoiningCornerType(int orientation, int x, int y)
{       
    if (orientation==Trunk::HORIZONTAL)
    {
        if(x<pointsX_[0])
        {
            if(pointsY_[0]>pointsY_[1])
            {
                return TrunkCorner::WEST_NORTH;
            }
            else
            {
                return TrunkCorner::WEST_SOUTH;
            }
        }
        else
        {
            if(pointsY_[0]>pointsY_[1])
            {
                return TrunkCorner::EAST_NORTH;
            }
            else
            {
                return TrunkCorner::EAST_SOUTH;
            }
        }

    }
    else
    {
        if (y<pointsY_[0])
        {
              if(pointsX_[0]>pointsX_[1])
              {
                  return TrunkCorner::NORTH_WEST;
              }
              else
              {
                  return TrunkCorner::NORTH_EAST;
              }
            
        }
        else 
        {
            if(pointsX_[0]>pointsX_[1])
            {
                return TrunkCorner::SOUTH_WEST;
            }
            else
            {
                return TrunkCorner::SOUTH_EAST;
            }
        }
    }

    return 0;
}

int TrunkDrawingTool::getCornerType(unsigned i)
{
    int x2;
    int y2;
    if(i==(pointsX_.size()-2))
    {
       x2=currentX_;
       y2=currentY_;
    }
    else
    {
        x2=pointsX_[i+2];
        y2=pointsY_[i+2];
    }

    if(pointsY_[i+1]==pointsY_[i])
    {
        if (pointsX_[i+1]>pointsX_[i])
        {
            if (y2<pointsY_[i+1])
            {
                return TrunkCorner::WEST_NORTH;
            }
            else if(y2>pointsY_[i+1])
            {
                return TrunkCorner::WEST_SOUTH;
            }

        }
        else if(pointsX_[i+1]<pointsX_[i])
        {
            if (y2<pointsY_[i+1])
            {
                return TrunkCorner::EAST_NORTH;
            }
            else if(y2>pointsY_[i+1])
            {
                return TrunkCorner::EAST_SOUTH;
            }

        }
    }
    else if(pointsX_[i+1]==pointsX_[i])
    {
        if (pointsY_[i+1]>pointsY_[i])
        {
            if (x2<pointsX_[i+1])
            {
                return TrunkCorner::NORTH_WEST;
            }
            else if(x2>pointsX_[i+1])
            {
                return TrunkCorner::NORTH_EAST;
            }

        }
        else if(pointsY_[i+1]<pointsY_[i])
        {
            
            if (x2<pointsX_[i+1])
            {
                return TrunkCorner::SOUTH_WEST;
            }
            else if(x2>pointsX_[i+1])
            {
                return TrunkCorner::SOUTH_EAST;
            }
        }
    }

    return 0;
}

void TrunkDrawingTool::drawCorner(Graphics& g, int x, int y, int cornerType)
{
    // XXX
    if(cornerType!=0)
    {
        Path p=getCornerPath(((float)x)/zoomFactor_,((float)y)/zoomFactor_,cornerType);
        g.setColour(Colour(0xBAAAAFAA));
        g.fillPath(p,AffineTransform::scale(zoomFactor_,zoomFactor_));
        g.setColour (Colour(0xff817e7e));
        float lineThickness=2.0f*zoomFactor_;
        g.strokePath(p,PathStrokeType(lineThickness),AffineTransform::scale(zoomFactor_,zoomFactor_));
    }
}

Path TrunkDrawingTool::getCornerPath(int x, int y, int cornerType)
{
    return getTrueCorner(x,y,cornerType);
}

void TrunkDrawingTool::drawTrunk(Graphics& g,int x1,int y1,int x2,int y2)
{
    x1=(float)x1/zoomFactor_;
    x2=(float)x2/zoomFactor_;
    y1=(float)y1/zoomFactor_;
    y2=(float)y2/zoomFactor_;

    int length;
    int endSpacing=10;
    int halfThickness=25;
    int thickness=2*halfThickness;

    if (x1==x2)
    {
        length=abs(y2-y1)-2*endSpacing;
        if (y2<y1)
        {
            drawVerticalTrunkBody(g, zoomFactor_, length,thickness, x1-halfThickness, y2+endSpacing,false);
        }
        else
        {
            drawVerticalTrunkBody(g, zoomFactor_, length,thickness, x1-halfThickness, y1+endSpacing,false);
        }
    }
    else if(y1==y2)
    {
        length=abs(x2-x1)-2*endSpacing;
        if (x2<x1)
        {
            drawHorizontalTrunkBody(g, zoomFactor_, length, thickness, x2+endSpacing, y1-halfThickness,false);
        }
        else
        {
            drawHorizontalTrunkBody(g, zoomFactor_, length, thickness, x1+endSpacing, y1-halfThickness,false);
        }
    }
//    g.drawLine(x1,y1,x2,y2, 2.0);

}

int TrunkDrawingTool::getNumSections()
{
    return pointsX_.size()-1;
}   

int TrunkDrawingTool:: getNumCorners()
{
    return pointsX_.size()-2;
}
int TrunkDrawingTool::getCornerX(unsigned index)
{
    return pointsX_[index+1];
}
int TrunkDrawingTool:: getCornerY(unsigned index)
{
    return pointsY_[index+1];
}


TrunkSection* TrunkDrawingTool::getSection(unsigned i)
{
    return new TrunkSection(pointsX_[i], pointsY_[i],pointsX_[i+1],pointsY_[i+1], zoomFactor_);
}

void TrunkDrawingTool::addPoint(int x, int y)
{
    pointsX_.push_back(x);
    pointsY_.push_back(y);
}

void TrunkDrawingTool::removePoint()
{
    pointsX_.pop_back();
    pointsY_.pop_back();
}

int TrunkDrawingTool:: getLastX()
{
    return pointsX_.back();
}

int TrunkDrawingTool:: getLastY()
{
    return pointsY_.back();
}

void TrunkDrawingTool::setCurrentPoint(int x, int y)
{
    mx_=x;
    my_=y;
    int dx=x-getLastX();
    int dy=y-getLastY();


    if (!orientationSet_) 
    {
        if(abs(dx)>abs(dy))
        {
            orientation_=Trunk::HORIZONTAL;
        }
        else
        {
            orientation_=Trunk::VERTICAL;
        }
        orientationSet_=true;
    }
   
    if(!dragOrientationChanged(x,y))
    {
        if (orientation_==Trunk::HORIZONTAL)
        {
            currentX_=x;
            currentY_=getLastY();
        }
        else
        {
           currentX_= getLastX();
           currentY_=y;
        }
        setMovedOffLastPoint();
        
        if(movedOffLastPoint_ && sameAsLastPoint(currentX_,currentY_, getStandardTolerance()))
        {
            if (pointsX_.size()>1)
            {
                removePoint();
                if(orientation_==Trunk::HORIZONTAL)
                {
                    orientation_=Trunk::VERTICAL;
                }
                else
                {
                    orientation_=Trunk::HORIZONTAL;
                }
            }
        }

    }
}

void TrunkDrawingTool::setMovedOffLastPoint()
{
    if (!movedOffLastPoint_ && !sameAsLastPoint(currentX_,currentY_,getStandardTolerance()))
    {
        movedOffLastPoint_=true;
    }
}

bool TrunkDrawingTool::sameAsLastPoint(int x,int y, int tol)
{
    return ((x>getLastX()-tol) && (x<getLastX()+tol)) && ((y>getLastY()-tol) && (y<getLastY()+tol));
}

int TrunkDrawingTool::getStandardTolerance()
{
    return std::max(20,(int)(56*zoomFactor_));
}
int TrunkDrawingTool::getEndDragTolerance()
{
    return std::max(20,(int)(70*zoomFactor_));
}

bool TrunkDrawingTool::dragOrientationChanged(int x, int y)
{
    int tol=std::max(20,(int)(56*zoomFactor_));
    if(orientationSet_)
    {
        if(orientation_==Trunk::HORIZONTAL)
        {
            if(abs(y-getLastY())>tol)
            // XXX test
            //if((abs(y-getLastY()))>tol &&(abs(x-getLastX())>tol))
            {
                if((pointsX_.size()==1)&&(abs(x-getLastX())<tol))
                {
                    return false;
                }
                else
                {
                    pic::logmsg()<<"****** direction changed to vertical: abs(y-getLastY())="<<abs(y-getLastY())<<" tol="<<tol;
                    pic::logmsg() <<"Trunk Drag vertice added = "<<x<< ","<<getLastY();
                    addPoint(x, getLastY());
                    movedOffLastPoint_=false;
                    orientation_=Trunk::VERTICAL;
                    return true;
                }
            }
        }
        else if (orientation_==Trunk::VERTICAL)
        {
            if(abs(x-getLastX())>tol)
            //XXX test
            //if((abs(x-getLastX())>tol)&&(abs(y-getLastY())>tol))
            {
		    if((pointsY_.size()==1)&&(abs(y-getLastY())<tol))
		    {
			return false;
		    }
		    else
		    {
			pic::logmsg()<<"****** direction changed to horizontal: abs(x-getLastX())="<<abs(x-getLastX())<<" tol="<<tol;
			pic::logmsg() <<"Trunk Drag vertice added = "<<getLastX()<< ","<<y; 
			addPoint(getLastX(),y);
			movedOffLastPoint_=false;
			orientation_=Trunk::HORIZONTAL;

			return true;
		    }   
             }
        }
    }
    return false;
}

void TrunkDrawingTool::endDrag(int x, int y)
{
    pic::logmsg()<<"TrunkDrawingTool:endDrag  dx="<<(abs(x-getLastX()))*zoomFactor_<<" dy="<<(abs(y-getLastY()))*zoomFactor_;
    if(!sameAsLastPoint(x,y, getEndDragTolerance()))
    {
        pic::logmsg()<<" point added";
        addPoint(currentX_, currentY_);    
    }
}

TrunkSection::TrunkSection(int x1, int y1,int x2, int y2, float zoomFactor)
{
    zoomFactor_=zoomFactor;
    x1_=x1;
    y1_=y1;
    x2_=x2;
    y2_=y2;
}

int TrunkSection:: getOrientation()
{
    if (x1_==x2_)
    {
        return Trunk::VERTICAL;
    }
    else
    {
        return Trunk::HORIZONTAL;
    }
}


int TrunkSection::getX()
{
    if (getOrientation()==Trunk::HORIZONTAL)
    {
        if (x2_<x1_)
        {
            return x2_;
        }
        return x1_;
    }
    else
    {
        return x1_;
    }

}

int TrunkSection::getY()
{
    if (getOrientation()==Trunk::HORIZONTAL)
    {
        return y1_;
    }
    else
    {
        if(y2_<y1_)
        {
            return y2_;
        }
        return y1_;
    }
}

int TrunkSection::getDirection()
{
    if (getOrientation()==Trunk::HORIZONTAL)
    {
        if (x2_<x1_)
        {
            return Trunk::RIGHT_TO_LEFT;
        }
        return Trunk::LEFT_TO_RIGHT;;
    }
    else
    {
        if(y2_<y1_)
        {
            return Trunk::BOTTOM_TO_TOP;
        }
        else
        {
            return Trunk::TOP_TO_BOTTOM;
        }
    }
}

int TrunkSection::getReverseDirection()
{
    if (getOrientation()==Trunk::HORIZONTAL)
    {
        if (x2_<x1_)
        {
            return Trunk::LEFT_TO_RIGHT;
        }
        return Trunk::RIGHT_TO_LEFT;
    }
    else
    {
        if(y2_<y1_)
        {
            return Trunk::TOP_TO_BOTTOM;
        }
        else
        {
            return Trunk::BOTTOM_TO_TOP;
        }
    }
}
int TrunkSection::getWidth()
{
    if(getOrientation()==Trunk::HORIZONTAL)
    {
        return abs(x2_-x1_);
    }
    else
    {
        return 50*zoomFactor_;
    }
}

int TrunkSection::getHeight()
{
    if(getOrientation()==Trunk::HORIZONTAL)
    {
        return 50*zoomFactor_;
    }
    else
    {
        return abs(y2_-y1_);
    }
}

int TrunkSection::getSectionLength()
{
    if(getOrientation()==Trunk::HORIZONTAL)
    {
        return getWidth();
    }
    else
    {
        return getHeight();
    }
}

TrunkCorner::TrunkCorner(PropertyStore* p, float zoomFactor)
{
    pic::logmsg()<<"new trunkCorner";
    props_=p;
    zoomFactor_=zoomFactor;
    assemblyId_=props_->get_string("AssemblyId");
    trueX_=props_->get_number("x");
    trueY_=props_->get_number("y");;
    int x=ceil(((float)trueX_*zoomFactor_)-0.5);
    int y=ceil(((float)trueY_*zoomFactor_)-0.5);
    trueWidth_=props_->get_number("width");
    trueHeight_=props_->get_number("height");
    move_init_=false;
    setBounds(x,y,ceil(((float)zoomFactor*trueWidth_)-0.5), ceil(((float)zoomFactor*trueHeight_)-0.5));
    mouseOver_=false;
    lassoed_=false;
    foregrounded_=false;
}

void TrunkCorner::doSetLassoed(bool lassoed)
{
    pic::logmsg()<<"TrunkCorner::doSetLassoed";
    lassoed_=lassoed;
    repaint();
}

void TrunkCorner::bringToFront()
{
   getMainPanel()->bringToFrontAssembly(props_->get_string("AssemblyId"));
}

void TrunkCorner::doBringToFront()
{
   if(foregrounded_)
   {
        toFront(false);
   }
   else
   {
       toBehind(getMainPanel()->getDisplayLayer());
   }

}

void TrunkCorner:: paint (Graphics& g)
{
    Colour col;
    if(mouseOver_||lassoed_)
    {
        col=Colour(mouseOverTrunkCornerColour);
    }
    else
    {
        col=Colour(trunkCornerColour);
    }

    int cornerType=props_->get_number("cornerType");
    Path p=getCornerPath(0.5*trueWidth_,0.5*trueHeight_,cornerType);
    g.setColour(col);
    g.fillPath(p,AffineTransform::scale(zoomFactor_,zoomFactor_));
    g.setColour (Colour(0xff817e7e));
    float lineThickness=2.0f*zoomFactor_;
    g.strokePath(p,PathStrokeType(lineThickness),AffineTransform::scale(zoomFactor_,zoomFactor_));
}

void TrunkCorner::changed(PropertyStore* p)
{
    if(p!=0)
    {
        pic::logmsg()<<"TrunkCorner::changed";
        trueX_=p->get_number("x");
        trueY_=p->get_number("y");
        int x=ceil(((float)trueX_*zoomFactor_)-0.5);
        int y=ceil(((float)trueY_*zoomFactor_)-0.5);
        setBounds(x,y,ceil((zoomFactor_*(float)trueWidth_)-0.5), ceil((zoomFactor_*(float)trueHeight_)-0.5));
        props_=p;
    }
}

void TrunkCorner::mouseMove (const MouseEvent& e)
{
//    setCursor(e);

}

void TrunkCorner::mouseEnter (const MouseEvent& e)
{
    setCursor(e);
    int tool=getMainPanel()->getTool();
    if(tool==ToolManager::MOVETOOL||tool==ToolManager::DELETETOOL||tool==ToolManager::TRUNKTOOL)
    {
        setMouseOver(true);
    }
    if(tool==ToolManager::TRUNKTOOL)
    {
        setMouseCursor(getMainPanel()->getCursor(ToolManager::TRUNK_EXISTING));
    }
}

void TrunkCorner::mouseExit (const MouseEvent& e)
{
    int tool=getMainPanel()->getTool();
    if(tool==ToolManager::MOVETOOL||tool==ToolManager::DELETETOOL||tool==ToolManager::TRUNKTOOL)
    {
        setMouseOver(false);
    }
}

void TrunkCorner::setMouseOver(bool over)
{
    getMainPanel()->mouseOverAssembly(props_->get_string("AssemblyId"),over);
}

void TrunkCorner::doSetMouseOver(bool over)
{
    mouseOver_=over;
    repaint();
}

void TrunkCorner::mouseDown(const MouseEvent& e)
{
  	mouseDownX=getX();
	mouseDownY=getY();
    if(getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->mouseDown(e.getEventRelativeTo(getMainPanel()));
    }   
    else if(getMainPanel()->getTool()==ToolManager::MOVETOOL)
    {
        bringToFront();
    }
}

void TrunkCorner::mouseDrag (const MouseEvent& e)
{
    if(getMainPanel()->getTool()==ToolManager::MOVETOOL)
    {
        if(!move_init_)
        {
            getMainPanel()->initAssemblyMove(props_->get_string("AssemblyId"));
            move_init_=true;
        }
        doMove(e);
        getMainPanel()->thingMoved();;
    }
    else if (getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
       getMainPanel()->mouseDrag(e.getEventRelativeTo(getMainPanel()));
    }

}
void TrunkCorner::mouseUp (const MouseEvent& e)
{
    move_init_=false;
 	ModifierKeys m=e.mods;
	if (m.isPopupMenu())
	{
        getMainPanel()->showTools(e);
	}
    else if (getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->mouseUp(e.getEventRelativeTo(getMainPanel()));
    }
    else
    {
//        getMainPanel()->stopAutoDrag();
        getMainPanel()->endThingMoved();
        getMainPanel()->setAssemblyProps(props_->get_string("assemblyId"));
        
        if (e.mouseWasClicked())
        {
            doMouseClick(e);
        }
    }
    // XXX toBack/toFront etc
    // selected ?
    String assemblyId=props_->get_string("assemblyId");
    if(foregrounded_)
    {
        //setForegrounded(true,foregrounded_);
        // XXX
        getMainPanel()->revertForegroundAssembly(assemblyId);
    }
    else
    {
        getMainPanel()->toBackAssembly(assemblyId);
    }


}

void TrunkCorner::doMouseClick(const MouseEvent& e)
{
    if(getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->doMouseClick(e.getEventRelativeTo(getMainPanel()));
    }

    else if(getMainPanel()->getTool()==ToolManager::DELETETOOL) 
    {
        bool doDelete=true;
        if(getMainPanel()->requiresDeleteConfirmation("Trunk"))
        {
            DeleteTrunkConfirmation* da=new DeleteTrunkConfirmation();
            DialogWindow::showModalDialog("Delete trunk",da,this,Colour(0xffababab),true);
            if(da->dontShowAgain_&& da->okPressed_)
            {
               getMainPanel()->setDeleteConfirmationRequired("Trunk"); 
            }

            doDelete=da->okPressed_;
            delete da;
        }

        if(doDelete)
        {
            getMainPanel()->deleteAssembly(props_->get_string("assemblyId"));
        }
    }
}
 
Path TrunkCorner::getCornerPath(int x, int y, int cornerType)
{
    return getTrueCorner(x,y,cornerType);
}

void TrunkCorner::doZoom(float zoomFactor)
{
    zoomFactor_=zoomFactor;
    int x=ceil(((float)trueX_*zoomFactor)-0.5);
    int y=ceil(((float)trueY_*zoomFactor)-0.5);
    setBounds(x,y,ceil(((float)zoomFactor*trueWidth_)-0.5), ceil(((float)zoomFactor*trueHeight_)-0.5));
    repaint();
}

void TrunkCorner::doAssemblyMove(int dx, int dy)
{
    int x=getX()+dx;
    int y=getY()+dy;
    trueX_=floor(((float)x/zoomFactor_)+0.5);
    trueY_=floor(((float)y/zoomFactor_)+0.5);

    setTopLeftPosition(x, y);
}

void TrunkCorner::doAssemblySetProps()
{
   if (props_!=0)
   {
        props_->set_number("x",trueX_);
        props_->set_number("y",trueY_);
        save_props();
   }
}

void TrunkCorner::doSetForegrounded(bool foregrounded)
{
    foregrounded_=foregrounded;
    if(foregrounded)
    {
        pic::logmsg()<<"trunk corner foregrounded";
        setAlwaysOnTop(true);
        getMainPanel()->getDisplayLayer()->toBack();
    }
    else
    {
        setAlwaysOnTop(false);
        toBehind(getMainPanel()->getDisplayLayer());
    }
}

void TrunkCorner::doMove(const MouseEvent& e)
{
    int x=mouseDownX+e.getDistanceFromDragStartX();
    int y=mouseDownY+e.getDistanceFromDragStartY();

    x=getMainPanel()->conditionX(x,getWidth());
    y=getMainPanel()->conditionY(y,getHeight());

    int dx=x-getX();
    int dy=y-getY();
    getMainPanel()->moveAssembly(props_->get_string("assemblyId"),dx,dy);
    //autoDragIfRequired(x,y, e.x+getX(), e.y+getY());
}

void TrunkCorner::setCursor(const MouseEvent& e)
{
    
    setMouseCursor(getMainPanel()->getCursor());
}

PropertyStore* TrunkCorner::getProps()
{
    return props_;    
}

void TrunkCorner::save_props()
{
    pic::logmsg()<<"trunkCorner:save_props";
    String id =props_->get_string("id");
    getMainPanel()->save_props(id);
}


String TrunkCorner::getAssemblyId()
{
    return assemblyId_;
}

Trunk::Trunk(PropertyStore* props, float zoomFactor, int orientation)
{
    props_=props;
    zoomFactor_=zoomFactor;
    orientation_=orientation;
    trueWidth_=props_->get_number("width");
    trueHeight_=props_->get_number("height");
    trueX_=props_->get_number("x");
    trueY_=props_->get_number("y");
    int x=ceil(((float)trueX_*zoomFactor_)-0.5);
    int y=ceil(((float)trueY_*zoomFactor_)-0.5);
    setBounds(x,y,ceil(((float)zoomFactor*trueWidth_)-0.5), ceil(((float)zoomFactor*trueHeight_)-0.5));
    mouseMode=0;
    selected_=String::empty;
    foregrounded_=String::empty;
    deleteOnDragEnd_=false;
    move_init_=false;
    extending_=false;
    mouseOver_=false;
    inEZone_=false;
    lassoed_=false;

    String listname;
    for (int i=0;i<props_->list_count();i++)
    {
        listname=props_->get_listkey(i);

        if (listname.startsWith("input"))
        {
            String wire_id=props_->get_list(listname,0);
            String aperture_in=props_->get_list(listname,1);
            String aperture_out=props_->get_list(listname,2);
            createInput(wire_id, aperture_in.getIntValue());
            createOutput(wire_id,aperture_out.getIntValue());
        }
    }
}

Trunk::~Trunk()
{
    pic::logmsg()<<"Trunk destructor";
}

void Trunk::lasso(bool lassoed)
{
    setLassoed(lassoed);
}


int Trunk::getMidX()
{
    return getX()+0.5*getWidth();
}

int Trunk::getMidY()
{
    return getY()+0.5*getHeight();
}

void Trunk::changed(PropertyStore* props)
{
    if(props!=0)
    {
        props_=props;
        trueX_=props_->get_number("x");
        trueY_=props_->get_number("y");
        trueWidth_=props_->get_number("width");
        trueHeight_=props_->get_number("height");

        //  if they exist already set the positions
        //  if they dont exist - create them 
        String listname;
        for (int i=0;i<props_->list_count();i++)
        {
            listname=props_->get_listkey(i);

            if (listname.startsWith("input"))
            {
                String wire_id=props_->get_list(listname,0);
                String aperture_in=props_->get_list(listname,1);
                String aperture_out=props_->get_list(listname,2);

                TrunkInput* t=getInput(wire_id);
                if(t!=0)
                {
                    //std::cout<<"setting trunk input aperture index"<<std::endl;
                    t->setApertureIndex(aperture_in.getIntValue());
                    t=getOutput(wire_id);
                    t->setApertureIndex(aperture_out.getIntValue());
                }
                else
                {
                    //std::cout<<"creating trunk inputs and outputs"<<std::endl;
                    createInput(wire_id, aperture_in.getIntValue());
                    createOutput(wire_id,aperture_out.getIntValue());
                }
            }
        }

        //  if inputs exist which are not in the props - remove them
        std::vector<String> v;
        for (int i =getNumChildComponents();--i>=0;)
        {
            TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
            if (t!=0)
            {
                if (t->isInput())
                {
                    String inputName="input"+String(t->getID().hashCode());
                    if(!props_->has_list(inputName))
                    {
                        //std::cout<<this<<" props_ does not have list named "<<inputName.toUTF8()<<std::endl;
                        v.push_back(t->getID());
                    }
                }
            }
        } 

        for(std::vector<String>::iterator i=v.begin();i!=v.end();i++)
        {
            setSelected(false,(*i));
            removeWire((*i));
        }
        int x=ceil(((float)trueX_*zoomFactor_)-0.5);
        int y=ceil(((float)trueY_*zoomFactor_)-0.5);

        setBounds(x,y,ceil((zoomFactor_*(float)trueWidth_)-0.5), ceil((zoomFactor_*(float)trueHeight_)-0.5));

        repositionInputs(false);
    }
}


void Trunk::setInputTopLeftPosition(TrunkInput* t, int x, int y)
{
    std::vector<TrunkInput*> inputs;
    getInputsLike(t,inputs);
    for(std::vector<TrunkInput*>::iterator i=inputs.begin();i<inputs.end();i++)
    {
        (*i)->setPos(x,y);
    }
}

void Trunk::saveInputProps(TrunkInput* t)
{
    std::vector<TrunkInput*> inputs;
    getInputsLike(t,inputs);
    for(std::vector<TrunkInput*>::iterator i=inputs.begin();i<inputs.end();i++)
    {
      pic::logmsg()<<"save_props for input "<<std::string((*i)->getID().toUTF8());
      //std::cout<<"save_props for input "<<(*i)->getID().toUTF8()<<std::endl;
      (*i)->save_props();
    }
}

void Trunk::setInputIndex(TrunkInput* t, int index)
{
    std::vector<TrunkInput*> inputs;
    getInputsLike(t,inputs);
    for(std::vector<TrunkInput*>::iterator i=inputs.begin();i<inputs.end();i++)
    {
      pic::logmsg()<<"setIndex for input "<<std::string((*i)->getID().toUTF8())<<" to "<<index;
      (*i)->setApertureIndex(index);
    }

}

bool Trunk::isRevConnect(TrunkInput* t)
{
    String id=t->getID();
    Wire* w=getMainPanel()->getWireById(id);
    if(w!=0)
    {
        return w->isRevConnect();
    }
    else
    {
        pic::logmsg()<<"Trunk::isRevConnect() w=0";
        return 0;
    }
}

int Trunk::getRouteHash(TrunkInput* t)
{
    String id=t->getID();
    Wire* w=getMainPanel()->getWireById(id);
    if(w!=0)
    {
        return w->getRouteHash();
    }
    else
    {
        //pic::logmsg()<<"Trunk::getRouteHash() w=0";
        return 0;
    }
}

void Trunk::getInputsLike(TrunkInput* model, std::vector<TrunkInput*>& inputs)
{
    int hash =getRouteHash(model);
    if (model->isInput())
    {
        for (int i =getNumChildComponents();--i>=0;)
        {
            TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
            if(t==model)
            {
                inputs.push_back(t);
            }
            else if(isRevConnect(t) && !isRevConnect(model))
            {
                if (t!=0 && t->isOutput())
                {
                    if(hash==getRouteHash(t))
                    {
                        inputs.push_back(t);
                    }
                }
            }
            else
            {
                if (t!=0 && t->isInput())
                {
                    if(hash==getRouteHash(t))
                    {
                        inputs.push_back(t);
                    }
                }
            }
        }
    }
    else if (model->isOutput())
    {
        for (int i =getNumChildComponents();--i>=0;)
        {
            TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
            if(t==model)
            {
                inputs.push_back(t);
            }

            else if(isRevConnect(t) && !isRevConnect(model))
            {
                 if (t!=0 && t->isInput())
                {
                    if(hash==getRouteHash(t))
                    {
                        inputs.push_back(t);
                    }
                }
            }
            else
            {
                if (t!=0 && t->isOutput())
                {
                    if(hash==getRouteHash(t))
                    {
                        inputs.push_back(t);
                    }
                }
            }
        }
    }
}

int Trunk::getOrientation()
{
    return orientation_;
}

void Trunk::paint (Graphics& g)
{
    drawTrunkWires(g);
    // XXX if all wires are hidden - dont draw the trunk body
    // but needs to be done for whole trunk assembly
    drawTrunkBody(g,zoomFactor_, getTrueTrunkLength(),getTrueTrunkThickness(), orientation_,0,0,mouseOver_||lassoed_);

    // show fullextent of this object -useful for testing
//    g.fillAll(Colour(0xaac84b4b));
}

void Trunk::drawTrunkWires(Graphics& g)
{
    String id;
    std::set<int> drawnRoutes;
    std::set<int>::iterator iter;

    //draw the wires in the trunk
    int count=0;
    int neg=-1;
    float m=25;
    float pos=m;
    float selectedPos=m;
    int selectedHash=0;

    float sep=4.0;
    TrunkInput* tin;

    for (int i =getNumChildComponents();--i>=0;)
    {
        TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
        if (t!=0 && t->isOutput())
        {
            id=t->getID();
            if (id!="toBeDeleted")
            {
                // only draw if havent drawn one with this hash yet
                int hash=getRouteHash(t);
                if(id==selected_)
                {
                    selectedHash=hash;
                }
                iter=drawnRoutes.find(hash);
                if (iter==drawnRoutes.end())
                {
                    MainComponent* mc=getMainPanel();
                    Wire* w=mc->getWireById(id);
                    if((w!=0) && !w->isHidden())
                    {
                        t->setVisible(true);
                        tin=getInput(id);
                        if (tin!=0)
                        {
                            tin->setVisible(true);
                        }
                        Colour col =getWireColour(id);
                        pos=pos+(neg*count*sep);
                        count++;        
                        neg=neg*-1;
                        if ((pos<10) ||(pos>(40)))
                        {
                            pos=m;
                        }
                        
                        drawTrunkWire(g,id,pos*zoomFactor_,col);
                        drawnRoutes.insert(hash);
                        if (hash==selectedHash)
                        {
                            selectedPos=pos;
                        }
                    }
                    else
                    {
                        t->setVisible(false);
                        tin=getInput(id);
                        if (tin!=0)
                        {
                            tin->setVisible(false);
                        }

                    }
                }
            }
        }
    }

    drawnRoutes.clear();

    if (selected_.isNotEmpty())
    {
        drawTrunkWire(g,selected_,selectedPos*zoomFactor_,Colour(selectedWireColour));
    }
}

Colour Trunk::getWireColour(String wireId)
{
    MainComponent* mc=getMainPanel();
    if (mc!=0)
    {
        Wire* w= mc->getWireById(wireId);
        if (w!=0)
        {
            return w->getWireColour();
        }
    }

    if(wireId.contains("Loose"))
    {
        return Colour(looseWireColour);
    }
    return Colour(wireColour);
}

void Trunk::drawTrunkWire(Graphics& g, String id, float pos, const Colour col )
{
    float xIn=getXinInternal(id)-getX();
    float yIn=getYinInternal(id)-getY();
    float xOut=getXoutInternal(id)-getX();
    float yOut=getYoutInternal(id)-getY();
    float xxOut=0.0;
    float xxIn=0.0;
    float yyOut=0.0;
    float yyIn=0.0;

    g.setColour (col);
    if (orientation_==HORIZONTAL)
    {
        g.drawLine(xIn,yIn,xIn,pos,4.0f*zoomFactor_);
        if(xIn<xOut)
        {
            xxIn=xIn-2.0*zoomFactor_;
            xxOut=xOut+2.0*zoomFactor_;
        }
        else
        {
            xxIn=xIn+2.0*zoomFactor_;
            xxOut=xOut-2.0*zoomFactor_;
        }
        g.drawLine(xxIn,pos,xxOut,pos,4.0f*zoomFactor_);
        g.drawLine(xOut,pos,xOut,yOut,4.0f*zoomFactor_);
    }
    else
    {
        g.drawLine(xIn,yIn,pos,yIn,4.0f*zoomFactor_);
        if(yIn<yOut)
        {
            yyIn=yIn-2.0*zoomFactor_;
            yyOut=yOut+2.0*zoomFactor_;
        }
        else
        {
            yyIn=yIn+2.0*zoomFactor_;
            yyOut=yOut-2.0*zoomFactor_;
        }

        g.drawLine(pos,yyIn,pos,yyOut,4.0f*zoomFactor_);
        g.drawLine(pos,yOut,xOut,yOut,4.0f*zoomFactor_);
    }
}


void Trunk::resized()
{
}

void Trunk::mouseMove (const MouseEvent& e)
{
    int tool=getMainPanel()->getTool();
    if(tool==ToolManager::TRUNKTOOL)
    {
        checkEZone(e);
    }
}

void Trunk::checkEZone(const MouseEvent& e)
{
    if(inExtendZone(e) && !inEZone_)
    {
        pic::logmsg()<<" enter ExtendZone";
        inEZone_=true;
        if (orientation_==HORIZONTAL)
        {
            setMouseCursor(getMainPanel()->getCursor(ToolManager::TRUNK_RESIZE_VERT));
        }
        else
        {
            setMouseCursor(getMainPanel()->getCursor(ToolManager::TRUNK_RESIZE_HOR));
        }

    }
    else if(!inExtendZone(e) && inEZone_)
    {
        pic::logmsg()<<" exit ExtendZone";

        inEZone_=false;
        if (orientation_==HORIZONTAL)
        {
            setMouseCursor(getMainPanel()->getCursor(ToolManager::TRUNK_RESIZE_HOR));
        }
        else
        {
            setMouseCursor(getMainPanel()->getCursor(ToolManager::TRUNK_RESIZE_VERT));
        }

    }
}

void Trunk::mouseEnter (const MouseEvent& e)
{
    setCursor(e);
    int tool=getMainPanel()->getTool();
    if(tool==ToolManager::MOVETOOL||tool==ToolManager::DELETETOOL||tool==ToolManager::TRUNKTOOL)
    {
        setMouseOver(true);
    }
    if(tool==ToolManager::TRUNKTOOL)
    {
        checkEZone(e);
        if(!inEZone_)
        {
            if (orientation_==HORIZONTAL)
            {
                setMouseCursor(getMainPanel()->getCursor(ToolManager::TRUNK_RESIZE_HOR));
            }
            else
            {
                setMouseCursor(getMainPanel()->getCursor(ToolManager::TRUNK_RESIZE_VERT));
            }
        }
    }   
}

void Trunk::mouseExit (const MouseEvent& e)
{
    int tool=getMainPanel()->getTool();
    if(tool==ToolManager::MOVETOOL||tool==ToolManager::DELETETOOL||tool==ToolManager::TRUNKTOOL)
    {
        setMouseOver(false);
    }

    inEZone_=false;
    setMouseCursor(getMainPanel()->getCursor());
}

void Trunk::setMouseOver(bool over)
{
    getMainPanel()->mouseOverAssembly(props_->get_string("AssemblyId"),over);
}

void Trunk::setLassoed(bool lassoed)
{
    pic::logmsg()<<"Trunk setLassoed "<<lassoed<<" id="<<std::string(get_id().toUTF8());
    getMainPanel()->lassoAssembly(props_->get_string("AssemblyId"),lassoed);
}

void Trunk::doSetMouseOver(bool over)
{
    mouseOver_=over;
    repaint();
}

void Trunk::doSetLassoed(bool lassoed)
{
    lassoed_=lassoed;
    repaint();
}

void Trunk::mouseDown (const MouseEvent& e)
{
  	mouseDownX=getX();
	mouseDownY=getY();
    mouseDownWidth=getWidth();
    mouseDownHeight=getHeight();

    if(getMainPanel()->getTool()==ToolManager::TRUNKTOOL)
    {
        trunkToolMouseDown(e);
    }

    else if(getMainPanel()->getTool()==ToolManager::MOVETOOL)
    {
        moveToolMouseDown(e);
    }

    else if(getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->mouseDown(e.getEventRelativeTo(getMainPanel()));
    }

}
void Trunk::trunkToolMouseDown(const MouseEvent& e)
{
    if (orientation_==HORIZONTAL)
    {
        if(e.x>0.5*getWidth())
        {
            mouseMode=RESIZINGRIGHT;
        }
        else
        {
            mouseMode=RESIZINGLEFT;
        }
    }
    else
    {
        if(e.y>0.5*getHeight())
        {
            mouseMode=RESIZINGBOTTOM;
        }
        else
        {
            mouseMode=RESIZINGTOP;
        }
    }
}

void Trunk::moveToolMouseDown(const MouseEvent& e)
{
    mouseMode=MOVING;
    setMouseCursor(getMainPanel()->getCursor());
    if(lassoed_)
    {
        getMainPanel()->initSelectedItemMove();
        bringToFront();
    }
    else
    {
        getMainPanel()->addToSelectionBasedOnModifiers(this,e.mods);
        bringToFront();
        if(!move_init_) 
        {
            getMainPanel()->clearWireMoveList();
            getMainPanel()->initRoutingElementMove(this);
            move_init_=true;
        }
    }
}

void Trunk::bringToFront()
{
   getMainPanel()->bringToFrontAssembly(props_->get_string("AssemblyId"));
}

void Trunk::doBringToFront()
{
   if(foregrounded_.isNotEmpty())
   {
        toFront(false);
   }
   else
   {
       toBehind(getMainPanel()->getDisplayLayer());
   }

}

bool Trunk::isFirstSection()
{
    return getAssemblyIndex()==0;
}

bool Trunk::isLastSection()
{
    return getAssemblyIndex()==getMaxAssemblyIndex();
}

int Trunk::getAssemblyIndex()
{
    return props_->get_number("assemblyIndex");
}

int Trunk::getAssemblySize()
{
    return props_->get_number("assemblySize");
}

int Trunk::getMaxAssemblyIndex()
{
    return props_->get_number("assemblySize")-1;
}

int Trunk::getAppendX(const MouseEvent& e)
{
    if (getOrientation()==Trunk::HORIZONTAL)
    {
        if(e.x>((float)getWidth()*0.5f))
        {
            return trueX_+trueWidth_+10;
        }
        else
        {
            return trueX_-10;
        }
    }
    else
    {
        return trueX_+((float)trueWidth_*0.5f);
    }
}

int Trunk::getAppendY(const MouseEvent& e)
{
    int y=0;
    if (getOrientation()==Trunk::VERTICAL)
    {
        if(e.y>((float)getHeight()*0.5f))
        {
            y=trueY_+trueHeight_+10;
            return y;
        }
        else
        {
            y=trueY_-10;
            return y;
        }
    }
    else
    {
        y=trueY_+((float)trueHeight_*0.5f);
        return y;
    }
}


bool Trunk::inExtendZone(const MouseEvent& e)
{
    float c=20.0f*zoomFactor_;
    if (isAssemblyEnd())
    {
         int direction=getDirection();
         if (getOrientation()==Trunk::HORIZONTAL)
         {
             if (isFirstSection())
             {
                 if(direction==Trunk::LEFT_TO_RIGHT)
                 {
                     if(isLastSection())
                     {
                         return (e.x<c)||(e.x>(getWidth()-c));
                     }
                     return e.x<c;
                 }
                 else
                 {
                     if(isLastSection())
                     {
                        return(e.x>(getWidth()-c))||(e.x<c);  
                     }
                     return e.x>(getWidth()-c);
                 }
             }
             else
             {
                 if(direction==Trunk::LEFT_TO_RIGHT)
                 {
                     return e.x>(getWidth()-c);
                 }
                 else
                 {
                     return e.x<c;
                 }
             }
         }
         else   //vertical
         {
             if(isFirstSection())
             {
                 if(direction==Trunk::TOP_TO_BOTTOM)
                 {
                     if(isLastSection())
                     {
                        return (e.y<c)||(e.y>(getHeight()-c));
                     }
                     return e.y<c;
                 }
                 else
                 {
                     if(isLastSection())
                     {
                        return (e.y>(getHeight()-c))||(e.y<c);
                     }
                     return e.y>(getHeight()-c);
                 }
             }
             else
             {
                 if(direction==Trunk::TOP_TO_BOTTOM)
                 {
                     return e.y>(getHeight()-c);
                 }
                 else
                 {
                     return e.y<c;
                 }
             }
         }
    }

    return false;
}

bool Trunk::isPerpendicularMovement(const MouseEvent& e)
{

    if(getOrientation()==Trunk::HORIZONTAL)
    {
        if(abs(e.getDistanceFromDragStartY())>abs(2*e.getDistanceFromDragStartX()) )    
        {
            return true;
        }
    }
    else
    {
        if(abs(e.getDistanceFromDragStartX())>abs(2*e.getDistanceFromDragStartY()) )    
        {
            return true;
        }
    }
    return false;
}

void Trunk::mouseDrag (const MouseEvent& e)
{
//    pic::logmsg()<<"Trunk:mouseDrag";
    if(getMainPanel()->getTool()==ToolManager::TRUNKTOOL)
    {
        // extending means extending the assembly
        if(extending_)
        {
            getMainPanel()->extendTrunkMouseDrag(this,e);
        }

        else
        {
            bool test1=inExtendZone(e);
            bool test2=isPerpendicularMovement(e);
            pic::logmsg() <<"inExtendZone="<<test1<<"  perpendicularMovement="<<test2;

            if(test1 && test2)
            {
                 pic::logmsg()<<"in extend zone and perpendicular movement";
                 mouseMode=EXTEND;
                 extending_=true;
                 getMainPanel()->startExtendTrunkMouseDrag(e);
            }
            else
            {
                if(!move_init_) 
                {
                    getMainPanel()->clearWireMoveList();
                    getMainPanel()->initRoutingElementMove(this);
                    move_init_=true;
                }

                doResize(e);
                getMainPanel()->thingMoved();
            }
        }
    }

    else if(getMainPanel()->getTool()==ToolManager::MOVETOOL)
    {

        if(lassoed_)
        {
            doLassoMove(e);
        }
        else
        {
            doMove(e);
            getMainPanel()->thingMoved();
        }
    }

    else if (getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
       getMainPanel()->mouseDrag(e.getEventRelativeTo(getMainPanel()));
    }

}

String Trunk::getAssemblyId()
{
    return props_->get_string("assemblyId");
}

void Trunk::doResize(const MouseEvent& e)
{
//    pic::logmsg() <<"Trunk doResize";
    String assemblyId=props_->get_string("assemblyId");
    int direction=props_->get_number("direction");
    int assemblyIndex=props_->get_number("assemblyIndex");

    if (getOrientation()==Trunk::HORIZONTAL)
    {
        if (mouseMode==RESIZINGRIGHT)
        {
           int dx= resizeRight(mouseDownWidth+e.getDistanceFromDragStartX());
           getMainPanel()->moveSubAssemblyX(assemblyId, assemblyIndex, direction,Trunk::RIGHT, dx); 
           autoDragIfRequired(getX(),getY(),e.x+getX(),e.y+getY());
        }

        else if (mouseMode==RESIZINGLEFT)
        {
           int dx=resizeLeft(mouseDownX+e.getDistanceFromDragStartX(),mouseDownWidth-e.getDistanceFromDragStartX());
           getMainPanel()->moveSubAssemblyX(assemblyId, assemblyIndex, direction,Trunk::LEFT, dx); 
           autoDragIfRequired(getX(),getY(),e.x+getX(),e.y+getY());
        }
    }
    else
    {
       if(mouseMode==RESIZINGBOTTOM)
       {
            int dy=resizeBottom(mouseDownHeight+e.getDistanceFromDragStartY());
            getMainPanel()->moveSubAssemblyY(assemblyId, assemblyIndex, direction, Trunk::BOTTOM,dy); 
            autoDragIfRequired(getX(),getY(),e.x+getX(),e.y+getY());
       }
       else if(mouseMode==RESIZINGTOP)
       {
            int dy=resizeTop(mouseDownY+e.getDistanceFromDragStartY(),mouseDownHeight-e.getDistanceFromDragStartY());
            getMainPanel()->moveSubAssemblyY(assemblyId, assemblyIndex, direction,Trunk::TOP, dy); 
            autoDragIfRequired(getX(),getY(),e.x+getX(),e.y+getY());
       }
    }
}

void Trunk::doMove(const MouseEvent& e)
{
    if (mouseMode==MOVING)
    {
        int x=mouseDownX+e.getDistanceFromDragStartX();
        int y=mouseDownY+e.getDistanceFromDragStartY();

        x=getMainPanel()->conditionX(x,getWidth());
        y=getMainPanel()->conditionY(y,getHeight());
 
        int dx=x-getX();
        int dy=y-getY();
        getMainPanel()->moveAssembly(props_->get_string("assemblyId"),dx,dy);
        autoDragIfRequired(getX(),getY(), e.x+getX(), e.y+getY());
    }
}

void Trunk::doLassoMove(const MouseEvent&e)
{
    if (mouseMode==MOVING)
    {
//        pic::logmsg()<<"doLassoMove";
        int x=mouseDownX+e.getDistanceFromDragStartX();
        int y=mouseDownY+e.getDistanceFromDragStartY();

        x=getMainPanel()->conditionX(x,getWidth());
        y=getMainPanel()->conditionY(y,getHeight());
 
        int dx=x-getX();
        int dy=y-getY();
        getMainPanel()->moveSelectedGroup(dx,dy);
        autoDragIfRequired(getX(),getY(), e.x+getX(), e.y+getY());
    }
}

void Trunk::doAssemblyMove(int dx, int dy)
{
//    pic::logmsg()<<"doAssemblyMove";
    int x=getX()+dx;
    int y=getY()+dy;
//    trueX_=(float)x/zoomFactor_;
//    trueY_=(float)y/zoomFactor_;
    trueX_=toTrue(x);
    trueY_=toTrue(y);
    setTopLeftPosition(x, y);
}

int Trunk::toTrue(int displayVal)
{
    return floor(((float)displayVal/zoomFactor_)+0.5);    
}

void Trunk::doSelectedGroupMove(int dx, int dy)
{
//    pic::logmsg()<<"doSelectedGroupMove";
    getMainPanel()->toFrontAssembly(props_->get_string("assemblyId"));
    getMainPanel()->moveAssembly(props_->get_string("assemblyId"),dx,dy);
}
void Trunk::doSetPositionProps()
{
//    pic::logmsg()<<"doSetPositionProps";
    getMainPanel()->setAssemblyProps(props_->get_string("assemblyId"));
}

void Trunk::initMove()
{
    getMainPanel()->initRoutingElementMove(this);
}

void Trunk::doAssemblySetProps()
{
    if (props_!=0)
    {
        int x=getX();
        //props_->set_number("x",(float)x/zoomFactor_);
        props_->set_number("x",toTrue(x));
        int y =getY();
        //props_->set_number("y",(float)y/zoomFactor_);
        props_->set_number("y",toTrue(y));
        props_->set_number("width",trueWidth_);
        props_->set_number("height",trueHeight_);
        //std::cout<<"doAssemblySetProps"<<std::endl;
        save_props();
    }
}

bool Trunk::isAssemblyNeighbour(Trunk* t)
{
   PropertyStore* props2=t->getProps();
   String aId1 = props_->get_string("AssemblyId");
   String aId2=props2->get_string("AssemblyId");
   int index1=props_->get_number("AssemblyIndex");
   int index2=props2->get_number("AssemblyIndex");
   return (aId1==aId2) && ((index2>=index1-1) && (index2<=index1+1));
}

void Trunk::autoDragIfRequired(int x, int y, int mx, int my)
{
    pic::logmsg() <<"autoDragIfRequired x="<<x<< " y="<<y;
    if (!getMainPanel()->inCentralViewPort(mx,my))
    {
        getMainPanel()->autoComponentDrag(x,y,this);
    }
    else
    {
        getMainPanel()->stopAutoDrag();
    }
}

void Trunk::mouseUp (const MouseEvent& e)
{
    move_init_=false;
 	ModifierKeys m=e.mods;
	if (m.isPopupMenu())
	{
        getMainPanel()->showTools(e);
	}
    else if(e.mouseWasClicked())
    {
        doMouseClick(e);
    }
    else if (getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->mouseUp(e.getEventRelativeTo(getMainPanel()));
    }

    else
    {
        if(mouseMode==EXTEND)
        {
            getMainPanel()->extendTrunkMouseUp(this,e);
        }

        if(lassoed_)
        {   
            getMainPanel()->endThingMoved();
            getMainPanel()->stopAutoDrag();
            getMainPanel()->setSelectedGroupProps();
            getMainPanel()->doToolChangeIfRequired();
        }
        else
        {
            getMainPanel()->endThingMoved();
            getMainPanel()->stopAutoDrag();
            getMainPanel()->setAssemblyProps(props_->get_string("assemblyId"));
            getMainPanel()->doToolChangeIfRequired();
        }

        getMainPanel()->clearSelectedItemsIfOne();
        setSelected(!(selected_.isEmpty()),selected_);
    }

    if(deleteOnDragEnd_)
    {
        pic::logmsg()<< "deleteOnDragEnd_=true";
        getMainPanel()->deleteAssemblyElement(this);
    }

    extending_=false;
    mouseMode=0;
}

void Trunk::doMouseClick(const MouseEvent& e)
{
    pic::logmsg()<<"click on trunk "<<std::string(get_id().toUTF8());
    if(getMainPanel()->getTool()==ToolManager::ZOOMTOOL)
    {
        getMainPanel()->doMouseClick(e.getEventRelativeTo(getMainPanel()));
    }

    else if(getMainPanel()->getTool()==ToolManager::DELETETOOL) 
    {
        deleteToolMouseClick(e);
    }
    else if(getMainPanel()->getTool()==ToolManager::SELECTTOOL)
    {
        selectToolMouseClick(e);
    }

    else if(getMainPanel()->getTool()==ToolManager::POINTERTOOL)
    {
        pointerToolMouseClick(e);
    }
}

void Trunk::deleteToolMouseClick(const MouseEvent& e)
{
    //XXX testing single element deletion
 	ModifierKeys m=e.mods;
    if(m.isShiftDown())
    {
        if (isAssemblyEnd())
        {
            getMainPanel()->deleteAssemblyElement(this);
        }
    }
    else
    {
        bool doDelete=true;
        if(getMainPanel()->requiresDeleteConfirmation("Trunk"))
        {
            DeleteTrunkConfirmation* da=new DeleteTrunkConfirmation();
            DialogWindow::showModalDialog("Delete trunk",da,this,Colour(0xffababab),true);
            if(da->dontShowAgain_&& da->okPressed_)
            {
               getMainPanel()->setDeleteConfirmationRequired("Trunk"); 
            }

            doDelete=da->okPressed_;
            delete da;
        }

        if(doDelete)
        {
            getMainPanel()->deleteRoutingElement(this);
        }
    }
}

void Trunk::selectToolMouseClick(const MouseEvent& e)
{
    ModifierKeys m=e.mods;
    if(lassoed_ && !(m.isCommandDown()||m.isShiftDown()))
    {
        getMainPanel()->deselect(this);
    }
    else
    {
          getMainPanel()->addToSelectionBasedOnModifiers(this,m);
    }
}

void Trunk::pointerToolMouseClick(const MouseEvent& e)
{
    selectToolMouseClick(e);
}

bool Trunk::isAssemblyEnd()
{
    return (props_->get_number("assemblyIndex")==0)||(props_->get_number("assemblyIndex")==props_->get_number("assemblySize")-1);
}

String Trunk::get_id()
{
    return String(props_->get_string("id"));
}

int Trunk::getDirection()
{
    return props_->get_number("direction");
}

String Trunk::get_hash(String wireId)
{
    String hash=get_id();
    TrunkInput* input=getInput(wireId);
    if(input!=0)
    {
        hash =hash+String(input->getApertureIndex());
    }
    TrunkInput* output=getOutput(wireId);
    if(output!=0)
    {
        hash=hash+"X"+String(output->getApertureIndex());
    }
    
    return hash;
}

String Trunk::get_reverse_hash(String wireId)
{
    String hash=get_id();
    TrunkInput* output=getOutput(wireId);
    if(output!=0)
    {
        hash=hash+String(output->getApertureIndex());
    }
 
    TrunkInput* input=getInput(wireId);
    if(input!=0)
    {
        hash =hash+"X"+String(input->getApertureIndex());
    }
   
    return hash;
}

PropertyStore* Trunk::getProps()
{
    return props_;
}

void Trunk::setOutput(String id, int x, int y)
{
    pic::logmsg()<<"Trunk setOutput "<<x <<"  " << y;
    TrunkInput* t=getOutput(id);
    TrunkInput* tin=getInput(id);
    if (t!=0 && tin!=0)
    {
        if(!t->checkRemove2(tin->getApertureIndex(),x,y))
        {
            t->setPos(x,y,true);
        }
    }
    else
    {
        pic::logmsg()<< "attempt to set output on trunk which is not in route";
    }
}

void Trunk::setInput(String id, int x, int y)
{
    pic::logmsg()<<"Trunk setInput "<<x <<"  " << y;
    TrunkInput* t=getInput(id);
    TrunkInput* tout=getOutput(id);
    if (t!=0 && tout!=0)
    {
        if(!t->checkRemove2(tout->getApertureIndex(),x,y))
        {
            t->setPos(x,y, true);
        }
    }
}
void Trunk::createInput(String id,int apertureIndex )
{
    TrunkInput * tIn=new TrunkInput(this,id,apertureIndex,zoomFactor_, true);
    addAndMakeVisible(tIn);
}

void Trunk::createOutput(String id,int apertureIndex )
{
    TrunkInput * tOut=new TrunkInput(this,id,apertureIndex,zoomFactor_,false);
    addAndMakeVisible(tOut);
}

void Trunk::removeWire(String id)
{
    getMainPanel()->removeWireFromRoutingElement(id, this);
}

void Trunk::toggleSelected(String id)
{
    if(selected_==id)
    {
        selected_=String();
    }
    else
    {
        selected_=id;
    }
    repaint();
}

void Trunk::setSelected(bool selected, String id)
{
    String assemblyId=props_->get_string("assemblyId");
    if (selected==true)
    {
        selected_=id;
        getMainPanel()->toFrontAssembly(assemblyId);
    }
    else
    {
        selected_=String();
        if(foregrounded_.isNotEmpty())
        {
            setForegrounded(true,foregrounded_);
        }
        else
        {
            getMainPanel()->toBackAssembly(assemblyId);
        }
    }

    repaint();
}

void Trunk::setForegrounded(bool foregrounded, String id)
{
    String assemblyId=props_->get_string("assemblyId");
    getMainPanel()->foregroundAssembly(assemblyId,foregrounded,id);
}

void Trunk::doRevertForegrounded()
{
    if(foregrounded_.isNotEmpty())
    {
        setAlwaysOnTop(true);
        getMainPanel()->getDisplayLayer()->toBack();
    }
    else
    {
        setAlwaysOnTop(false);
        toBehind(getMainPanel()->getDisplayLayer());
    }
}

void Trunk::doSetForegrounded(bool foregrounded, String id)
{
    if(foregrounded)
    {
//        pic::logmsg()<<"trunk foregrounded";
        foregrounded_=id;
    // XXX temporary just bring to front - long term need to only foreground the relevant wires
 
        setAlwaysOnTop(true);
        getMainPanel()->getDisplayLayer()->toBack();
    }
    else
    {
        foregrounded_=String::empty;
        setAlwaysOnTop(false);
        toBehind(getMainPanel()->getDisplayLayer());
    }
}



void Trunk::repositionInputs(bool save)
{
    for (int i =getNumChildComponents();--i>=0;)
    {
        TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
        if (t!=0)
        {
          t->reposition(save);
        }
    }
}

void Trunk::zoomInputs()
{
    for (int i =getNumChildComponents();--i>=0;)
    {
        TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
        if (t!=0)
        {
            t->doZoom(zoomFactor_);
        }
    }
}

void Trunk::autoResizeLeft(int dx)
{
    String assemblyId=props_->get_string("assemblyId");
    int direction=props_->get_number("direction");
    int assemblyIndex=props_->get_number("assemblyIndex");

    int minWidth=getMinWidth();
    int x=getX()+dx;
    int newWidth=getWidth()-dx;

    if (newWidth>=minWidth)
    {
        setBounds(x,getY(),newWidth, getHeight());
        //trueX_=(float)x/zoomFactor_;
        trueX_=toTrue(x);
        //trueWidth_=(float)newWidth/zoomFactor_;
        trueWidth_=toTrue(newWidth);
        repositionInputs(true);
        mouseDownX=mouseDownX+dx;
        mouseDownWidth=mouseDownWidth-dx;
        getMainPanel()->moveSubAssemblyX(assemblyId, assemblyIndex, direction, Trunk::LEFT,-dx); 
    }
    else
    {
        doAtResizeLimit();
    }
}

void Trunk::doAtResizeLimit()
{
    if(isAssemblyEnd())
    {
        deleteOnDragEnd_=true;
    }

    getMainPanel()->stopAutoDrag();
}

int Trunk::getMinWidth()
{
    int minWidth=36;;
    if(!isAssemblyEnd())
    {
        minWidth=(getTrueBorderWidth()*2)+(getTrueApertureWidth()*6);
    }
    return minWidth*zoomFactor_;  
}

int Trunk::getMinHeight()
{
    int minHeight=36;
    if(!isAssemblyEnd())
    {
        minHeight=(getTrueBorderWidth()*2)+(getTrueApertureWidth()*6);
    }
    return minHeight*zoomFactor_;
}


int Trunk::resizeLeft(int x, int newWidth)
{
    int dx=0;
    int minWidth=getMinWidth();
    pic::logmsg()<<"resizeLeft: newWidth="<<newWidth<<" minWidth="<<minWidth;

    if (newWidth>=minWidth)
    {
        dx=newWidth-getWidth();
        deleteOnDragEnd_=false;
        setBounds(x,getY(),newWidth, getHeight());
        //trueX_=(float)x/zoomFactor_;
        trueX_=toTrue(x);
        //trueWidth_=(float)newWidth/zoomFactor_;
        trueWidth_=toTrue(newWidth);
        repositionInputs(true);
    }
    else
    {
        if(isAssemblyEnd())
        {
            dx=minWidth-getWidth();
            setBounds(x,getY(),minWidth, getHeight());
            //trueX_=(float)x/zoomFactor_;
            trueX_=toTrue(x);
            //trueWidth_=(float)minWidth/zoomFactor_;
            trueWidth_=toTrue(minWidth);
            repositionInputs(true);
            pic::logmsg()<<"newWidth<minWidth";
            deleteOnDragEnd_=true;
        }
    }

    return dx;
}

void Trunk::autoResizeTop(int dy)
{
    pic::logmsg()<<"autoResizeTop: dy="<<dy;
    String assemblyId=props_->get_string("assemblyId");
    int direction=props_->get_number("direction");
    int assemblyIndex=props_->get_number("assemblyIndex");

    int minHeight=getMinHeight();
    int y=getY()+dy;
    int newHeight=getHeight()-dy;;
    if (newHeight>=minHeight)
    {
        setBounds(getX(),y,getWidth(), newHeight);
        //trueY_=(float)y/zoomFactor_;
        trueY_=toTrue(y);
        //trueHeight_=(float)newHeight/zoomFactor_;
        trueHeight_=toTrue(newHeight);
        repositionInputs(true);
        mouseDownY=mouseDownY+dy;
        mouseDownHeight=mouseDownHeight-dy;
        getMainPanel()->moveSubAssemblyY(assemblyId, assemblyIndex, direction, Trunk::TOP,-dy); 

    }
    else
    {
        doAtResizeLimit();
    }

}

int Trunk::resizeTop(int y, int newHeight)
{
    int dy=0;
    int minHeight=getMinHeight();
    pic::logmsg()<<"resizeTop: newHeight="<<newHeight<<" minHeight="<<minHeight;
    if (newHeight>minHeight)
    {
        deleteOnDragEnd_=false;
        dy=newHeight-getHeight();
        setBounds(getX(),y,getWidth(), newHeight);
        //trueY_=(float)y/zoomFactor_;
        trueY_=toTrue(y);
        //trueHeight_=(float)newHeight/zoomFactor_;
        trueHeight_=toTrue(newHeight);
        repositionInputs(true);
    }
    else
    {
        if(isAssemblyEnd())
        {
 
            dy=minHeight-getHeight();
            setBounds(getX(),y,getWidth(), minHeight);
            //trueY_=(float)y/zoomFactor_;
            trueY_=toTrue(y);
            //trueHeight_=(float)minHeight/zoomFactor_;
            trueHeight_=toTrue(minHeight);
            repositionInputs(true);
            pic::logmsg()<<"newHeight<minHeight";
            deleteOnDragEnd_=true;
        }
    }

    return dy;
}

void Trunk::autoResizeRight(int dx)
{
    String assemblyId=props_->get_string("assemblyId");
    int direction=props_->get_number("direction");
    int assemblyIndex=props_->get_number("assemblyIndex");
    int minWidth=getMinWidth();
    int newWidth=getWidth()+dx;

    pic::logmsg()<<"autoResizeRight newWidth="<<newWidth<<" minWidth="<<minWidth;
    if (newWidth>=minWidth)
    {
        setBounds(getX(),getY(),newWidth,getHeight());
        //trueWidth_=(float)newWidth/zoomFactor_;
        trueWidth_=toTrue(newWidth);
        repositionInputs(true);
        mouseDownWidth=mouseDownWidth+dx;
        getMainPanel()->moveSubAssemblyX(assemblyId, assemblyIndex, direction, Trunk::RIGHT,dx); 

    }
    else
    {
        doAtResizeLimit();
    }
}

int Trunk::resizeRight(int newWidth)
{
    int dx=0;
    int minWidth=getMinWidth();
    pic::logmsg()<<"resizeRight: newWidth="<<newWidth<<" minWidth="<<minWidth;
    if (newWidth>minWidth)
    {
        deleteOnDragEnd_=false;
        dx=newWidth-getWidth();
        setBounds(getX(),getY(),newWidth,getHeight());
        //trueWidth_=(float)newWidth/zoomFactor_;
        trueWidth_=toTrue(newWidth);
        repositionInputs(true);
    }
    else
    {
        dx=minWidth-getWidth();
        setBounds(getX(),getY(),minWidth,getHeight());
        //trueWidth_=(float)minWidth/zoomFactor_;
        trueWidth_=toTrue(minWidth);
        repositionInputs(true);

        pic::logmsg()<<"newWidth<minWidth";
        if (isAssemblyEnd())
        {
            deleteOnDragEnd_=true;
        }
    }

    return dx;
}

void Trunk::autoResizeBottom(int dy)
{
    pic::logmsg()<<"autoResizeBottom: dy="<<dy;
    String assemblyId=props_->get_string("assemblyId");
    int direction=props_->get_number("direction");
    int assemblyIndex=props_->get_number("assemblyIndex");

    int minHeight=getMinHeight();
    int newHeight=getHeight()+dy;
    if (newHeight>=minHeight)
    {
        setBounds(getX(),getY(),getWidth(),newHeight);
        //trueHeight_=(float)newHeight/zoomFactor_;
        trueHeight_=toTrue(newHeight);
        repositionInputs(true);
        mouseDownHeight=mouseDownHeight+dy;
        getMainPanel()->moveSubAssemblyY(assemblyId, assemblyIndex, direction, Trunk::BOTTOM,dy); 
    }
    else
    {
        doAtResizeLimit();
    }
}

int Trunk::resizeBottom(int newHeight)
{
    int dy=0;
    int minHeight=getMinHeight();
    pic::logmsg()<<"resizeBottom: newHeight="<<newHeight<<" minHeight="<<minHeight;
    if (newHeight>minHeight)
    {
        deleteOnDragEnd_=false;
        dy=newHeight-getHeight();
        setBounds(getX(),getY(),getWidth(),newHeight);
        //trueHeight_=(float)newHeight/zoomFactor_;
        trueHeight_=toTrue(newHeight);
        repositionInputs(true);
    }
    else
    {
        dy=minHeight-getHeight();
        setBounds(getX(),getY(),getWidth(),minHeight);
        //trueHeight_=(float)minHeight/zoomFactor_;
        trueHeight_=toTrue(minHeight);
        repositionInputs(true);

        pic::logmsg()<<"newHeight<minHeight";
        if (isAssemblyEnd())
        {
            deleteOnDragEnd_=true;
        }
    }
    return dy;
}

int Trunk:: getXinInternal(String id)
{
    TrunkInput* t =getInput(id);
    if (t!=0)
    {
        return getX()+t->getConnectionX();
    }
    else
    {
        pic::logmsg() <<"Did not find TrunkInput for id "<<std::string(id.toUTF8());
        return getX();
    }
}

int Trunk:: getYinInternal(String id)
{
    TrunkInput* t =getInput(id);
    if (t!=0)
    {
        return getY()+t->getConnectionY();
    }
    else
    {
        pic::logmsg() <<"Did not find TrunkInput for id "<<std::string(id.toUTF8());
        return getY();
    }
}

int Trunk:: getXoutInternal(String id)
{
    TrunkInput* t = getOutput(id);
    if (t!=0)
    {
        return getX()+t->getConnectionX();
    }
    else
    {
        pic::logmsg() <<"Did not find TrunkOutput for id "<<std::string(id.toUTF8());
        return getRight();
    }
}

int Trunk::getYoutInternal(String id)
{
    TrunkInput* t = getOutput(id);
    if (t!=0)
    {
        return getY()+t->getConnectionY();
    }
    else
    {
        pic::logmsg() <<"Did not find TrunkOutput for id "<<std::string(id.toUTF8());
        return getY();
    }
}
int Trunk:: getXin(String id)
{
    TrunkInput* t =getInput(id);
    if (t!=0)
    {
        return trueX_+t->getTrueConnectionX();
    }
    else
    {
        pic::logmsg() <<"Did not find TrunkInput for id "<<std::string(id.toUTF8());
        return trueX_;
    }
}

int Trunk:: getYin(String id)
{
    TrunkInput* t =getInput(id);
    if (t!=0)
    {
        return trueY_+t->getTrueConnectionY();
    }
    else
    {
        pic::logmsg() <<"Did not find TrunkInput for id "<<std::string(id.toUTF8());
        return trueY_;
    }
}

int Trunk:: getXout(String id)
{
    TrunkInput* t = getOutput(id);
    if (t!=0)
    {
        return trueX_+t->getTrueConnectionX();
    }
    else
    {
        pic::logmsg() <<"Did not find TrunkOutput for id "<<std::string(id.toUTF8());
        return trueX_+trueWidth_;
    }
}

int Trunk::getYout(String id)
{
    TrunkInput* t = getOutput(id);
    if (t!=0)
    {
        return trueY_+t->getTrueConnectionY();

    }
    else
    {
        pic::logmsg() <<"Did not find TrunkOutput for id "<<std::string(id.toUTF8());
        return trueY_;
    }
}

Path Trunk::getPathFrom(RoutingElement* other, String id)
{
    Path p=getMainPanel()->getWirePath(other->getXout(id),other->getYout(id),getXin(id),getYin(id),1.1f);
    return p;
}

void Trunk::addId(String id, int input, int output)
{
    if(output==-1000)
    {
        output=1000;
    }
    pic::logmsg()<<"Trunk addId "<<std::string(id.toUTF8())<<" input="<<input<<" output="<<output;
    //std::cout<<"Trunk addId "<<id.toUTF8()<<" input="<<input<<" output="<<output<<std::endl;
    String listname="input"+String(id.hashCode());
    if(!props_->has_list(listname))
    {
        pic::logmsg() <<"Trunk addId "<<std::string(id.toUTF8());
        createInput(id,input);//1
        createOutput(id,output);//-1

        props_->set_list(listname,0,id);
        props_->set_list(listname,1,String(input));
        props_->set_list(listname,2,String(output));

        //std::cout<<"calling save props"<<std::endl;
        save_props();
    }
    //std::set<String>::iterator wireIter;  
    //wireIter=wireIds_.find(id);
    //if(wireIter==wireIds_.end())
    //{
    wireIds_.insert(id);
    //}
}

TrunkInput* Trunk::getInput(String id)
{
//    std::cout<<"Trunk getInput"<<id.toUTF8()<<std::endl;
    for (int i =getNumChildComponents();--i>=0;)
    {
        TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
        if (t!=0)
        {
            if (t->getID().equalsIgnoreCase(id) && t->isInput())
            {
                return t;
            }
        }
    } 
    return 0;
}

TrunkInput* Trunk::getOutput(String id)
{
    for (int i =getNumChildComponents();--i>=0;)
    {
        TrunkInput* const t =dynamic_cast <TrunkInput *>(getChildComponent (i));
        if (t!=0)
        {
            if (t->getID().equalsIgnoreCase(id) && t->isOutput())
            {
                return t;
            }
        }
    } 
    return 0;
}

void Trunk::removeId(String id)
{
    pic::logmsg()<<"Trunk removeId "<<std::string(id.toUTF8());
    if(id==selected_)
    {
        selected_=String::empty;
    }
    TrunkInput * t=getInput(id);
    String listname="input"+String(id.hashCode());
    if(t!=0)
    {
        if (props_->has_list(listname))
        {
            pic::logmsg()<< "remove_list input"<<std::string(id.toUTF8());
            //std::cout<<this<< " remove_list input"<<id.toUTF8()<<"props_ does have list named "<<listname.toUTF8() <<" calls save_props"<<std::endl;
            props_->remove_list(listname);
            save_props();
        }


        if (t->isDragging())
        {
            pic::logmsg()<<"t is dragging";
            t->removeOnDragEnd();
        }
        else
        {
            doRemoveInput(t);
        }
    }

    t=getOutput(id);
    if (t!=0)
    {
        if(t->isDragging())
        {
            pic::logmsg()<<"t is dragging";
            t->removeOnDragEnd();
        }
        else
        {
            doRemoveInput(t);
        }
    }

    int n=wireIds_.erase(id);
    pic::logmsg()<<"removed "<<n<<" ids from wireIds_";
    bool fg=false;
    for (std::set<String>::iterator i =wireIds_.begin();i!=wireIds_.end();i++)
    {
        Wire* w=getMainPanel()->getWireById((*i));
        if(w!=0)
        {
            if(w->isForegrounded())
            {
                fg=true;
                break;
            }
        }
        else
        {
            pic::logmsg()<<std::string((*i).toUTF8())<<" in wireIds_ but wire not found";
        }
    }
    if(fg==false)
    {
        setForegrounded(false,id);
    }
}

void Trunk::doRemoveInput(TrunkInput* t)
{
    if (t->isInput())
    {
        pic::logmsg() <<"doRemoveTrunkInput:  input  id="<<std::string(t->getID().toUTF8());
    }
    else
    {
        pic::logmsg() <<"doRemoveTrunkInput:  output  id="<<std::string(t->getID().toUTF8());
    }
    removeChildComponent(t);
    delete(t);

}

void Trunk::doAutoDrag(int dx,int dy)
{
    if(mouseMode==MOVING)
    {
        mouseDownX=mouseDownX+dx;
        mouseDownY=mouseDownY+dy;
        int x=getX()+dx;
        int y=getY()+dy;
        x=getMainPanel()->conditionX(x,getWidth());
        y=getMainPanel()->conditionY(y,getHeight());

        if(lassoed_)
        {
            getMainPanel()->moveSelectedGroup(dx,dy);

        }
        else
        {
            getMainPanel()->moveAssembly(props_->get_string("assemblyId"),dx,dy);
        } 
    }

    else if(mouseMode==RESIZINGBOTTOM)
    {
        autoResizeBottom(dy);
    }
    else if(mouseMode==RESIZINGTOP)
    {
        autoResizeTop(dy);
    }
    else if(mouseMode==RESIZINGRIGHT)
    {
        autoResizeRight(dx);
    }
    else if(mouseMode==RESIZINGLEFT)
    {
        autoResizeLeft(dx);
    }
}

void Trunk::doZoom(float zoomFactor)
{
    zoomFactor_=zoomFactor;

    int x=ceil(((float)trueX_*zoomFactor)-0.5);
    int y=ceil(((float)trueY_*zoomFactor)-0.5);
    setBounds(x,y,ceil(((float)zoomFactor*trueWidth_)-0.5), ceil(((float)zoomFactor*trueHeight_)-0.5));

    zoomInputs();
    repaint();
}

bool Trunk::onLeftBorder(int x)
{
    if (x>0 && x <5)
    {
        return true;
    }
    return false;
}

bool Trunk::onRightBorder(int x)
{
    if (x >getWidth()-5 && x<getWidth())
    {
        return true;
    }
    return false;
}

void Trunk::setCursor(const MouseEvent& e)
{
    setMouseCursor(getMainPanel()->getCursor());
}

int Trunk::getTool()
{
    return getMainPanel()->getTool();
}

void Trunk::save_props()
{
    pic::logmsg() <<"trunk save_props";
    //std::cout <<"trunk save_props"<<std::endl;
    String id =props_->get_string("id");

    getMainPanel()->save_props(id);
}

float Trunk::getYInputRange()
{
    if(orientation_==HORIZONTAL)
    {
        return getHeight()-(8.0f*zoomFactor_);
    }
    else
    {
        return getHeight();
    }
}

float Trunk::getXInputRange()
{
    if (orientation_==HORIZONTAL)
    {
        return getWidth();
    }
    else
    {
        return getWidth()-8.0f*(zoomFactor_);
    }
}

int Trunk::getNumSections()
{
    float bx=getTrueBorderWidth();
    float dx=getTrueApertureWidth();
    float sectionLength=5.0f*dx;

    return (getTrueTrunkLength()-(2.0f*bx)-dx)/sectionLength;
}

int Trunk::getApertureIndex(int x, int y, bool endsAllowed)
{ 
    // convert to true x,y 
    int trueX=toTrue(x);
    int trueY=toTrue(y);

    if(orientation_==HORIZONTAL)
    {
        return doGetApertureIndex(trueX,trueY, endsAllowed);
    }
    else
    {
        return doGetApertureIndex(trueY,trueX,endsAllowed);
    }
}

int Trunk::doGetApertureIndex(int trueLng, int trueLat, bool endsAllowed)
{
    int lng=trueLng;
    int lat=trueLat;
    float  bx=getTrueBorderWidth();
    float dx=getTrueApertureWidth();
    float sectionLength=5.0f*dx;

    int sectionNum=0;

    if (orientation_==HORIZONTAL)
    {
        pic::logmsg()<<"doGetApertureIndex HORIZONTAL: lng="<<lng<<" lat="<<lat<<" endsAllowed="<<endsAllowed;

        if(lng<2*bx)
        {
            if(endsAllowed)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
        else if(lng>getTrueTrunkLength()-2*bx)
        {
            if(endsAllowed)
            {
                return 1000;
            }
            else
            {
                int numApertures=int((getTrueTrunkLength()-dx-2*bx)/sectionLength);
                if (((getTrueTrunkLength()-(2*bx)-dx)-(numApertures*sectionLength))>=(2*dx))
                {
                   numApertures++;
                }
                return numApertures;
            }
        }

        else if(lat<0.5f*(getTrueTrunkThickness()))
        {
             int numApertures=int((getTrueTrunkLength()-dx-2*bx)/sectionLength);
             if (((getTrueTrunkLength()-(2*bx)-dx)-(numApertures*sectionLength))>=(2*dx))
             {
                numApertures++;
             }

             sectionNum=int(((lng-bx-dx)/sectionLength)+0.5);
             sectionNum++;
             pic::logmsg()<<"Horizontal a) numApertures="<<numApertures<<" sectionNum="<<sectionNum;
             if(sectionNum>numApertures)
             {
                sectionNum=numApertures;
                pic::logmsg()<<"adjusted to "<<sectionNum;
             }
        }
        else
        {
            int numApertures=int((getTrueTrunkLength()-dx-2*bx)/sectionLength);
            sectionNum=-1*int(((lng-bx-(3.5f*dx))/sectionLength)+0.5);
            sectionNum--;
            pic::logmsg()<<"Horizontal b) numApertures="<<numApertures<<" sectionNum="<<sectionNum;
            if(abs(sectionNum)>numApertures)
            {
               sectionNum=-1*numApertures;
               pic::logmsg()<<"adjusted to "<<sectionNum;
            }
        }
    }

    else  // Vertical 
    {
        if(lng<bx)
        {
            if(endsAllowed)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
        else if(lng>getTrueTrunkLength()-bx)
        {
            if (endsAllowed)
            {
                return 1000;
            }
            else
            {
                 int numApertures=int((getTrueTrunkLength()-dx-2*bx)/sectionLength);
                 if (((getTrueTrunkLength()-(2*bx)-dx)-(numApertures*sectionLength))>=(2*dx))
                 {
                    numApertures++;
                 }
                 return numApertures;
            }
        }


        else if(lat<0.5f*(getTrueTrunkThickness()))
        {
            int numApertures=int((getTrueTrunkLength()-dx-2*bx)/sectionLength);
            sectionNum=-1*int(((lng-bx-(3.5f*dx))/sectionLength)+0.5);
            sectionNum--;
            pic::logmsg()<<"Vertical a) numApertures="<<numApertures<<" sectionNum="<<sectionNum;
            if(abs(sectionNum)>numApertures)
            {
                sectionNum=-1*numApertures;
                pic::logmsg()<<"adjusted to "<<sectionNum;
            }
        }
        else
        {
             int numApertures=int((getTrueTrunkLength()-dx-2*bx)/sectionLength);
             if (((getTrueTrunkLength()-(2*bx)-dx)-(numApertures*sectionLength))>=(2*dx))
             {
                numApertures++;
             }

            sectionNum=int(((lng-bx-dx)/sectionLength)+0.5);
            sectionNum++;
            pic::logmsg()<<"Vertical b) numApertures="<<numApertures<<" sectionNum="<<sectionNum;
            if(sectionNum>numApertures)
            {
               sectionNum=numApertures;
               pic::logmsg()<<"adjusted to "<<sectionNum;
            }
        }
    }
    return sectionNum;
}

int Trunk::getApertureX(int index)
{
      // XXX trunkinputs should deal with true x and y
      if (orientation_==HORIZONTAL)
      {
        //return getApertureLong(index)*zoomFactor_;
        return getApertureLong(index);
      }
      else
      {
        //return getApertureLat(index)*zoomFactor_;
        return getApertureLat(index);
      }
}

int Trunk::getApertureY(int index)
{
      // XXX trunkinputs should deal with true x and y
      if (orientation_==HORIZONTAL)
      {
          //return getApertureLat(index)*zoomFactor_;
          return getApertureLat(index);
      }
      else
      {
          //return getApertureLong(index)*zoomFactor_;
          return getApertureLong(index);
      }
}

int Trunk::getApertureLong(int index)
{
      float  bx=getTrueBorderWidth();
      float dx=getTrueApertureWidth();;
      float sectionLength=5.0f*dx;
      if (index==0)
      {
        return 0;
      }
      else if (index==1000)
      {
        return getTrueTrunkLength()-8;
      }
      
      else if (index>0)
      {    
          index--;
          return bx+(1.5f*dx)+(index*sectionLength)-4;
      }
      else
      {
          index++;
          return bx+(4.0f*dx)+(-1*index*sectionLength)-4;
      }
}

int Trunk::getApertureLat(int index)
{
    if (orientation_==HORIZONTAL)
    {
        if (index==0)
        {
            return 0.5f*getTrueTrunkThickness()-4;
        }

        else if(index==1000)
        {
            return 0.5f*getTrueTrunkThickness()-4;
        }
        else if (index<0)
        {
            return getTrueTrunkThickness()-8.0f;
        }
        return 0;
    }
    else
    {
        if (index==0)
        {
            return 0.5f*getTrueTrunkThickness()-4;
        }

        else if(index==1000)
        {
            return 0.5f*getTrueTrunkThickness()-4;
        }

        else if(index<0)
        {
            return 0;
        }
        else
        {
            return getTrueTrunkThickness()-8.0f;
        }
    }
}

int Trunk::getTrueTrunkThickness()
{
    if (orientation_==Trunk::HORIZONTAL)
    {
        return trueHeight_;
    }
    else
    {
        return trueWidth_;
    }
}

int Trunk::getTrueTrunkLength()
{
    if(orientation_==Trunk::HORIZONTAL)
    {
        return trueWidth_;
    }
    else
    {
        return trueHeight_;
    }
}

float Trunk::getTrueApertureWidth()
{
    return 16.0f;
}

float Trunk::getTrueBorderWidth()
{
    return 10.0f;
}

int Trunk::getInputAt(int x, int y)
{
    return getApertureIndex(x,y,true);
}

int Trunk::getOutputAt(int x, int y)
{
    return getApertureIndex(x,y, true);
}

Path getTrueCorner(int trueX, int trueY, int cornerType)
{
    Path p;
    int x=trueX;
    int y=trueY;
    int ht=15;

    if (cornerType==TrunkCorner::WEST_NORTH)
    {
        p.startNewSubPath(x-ht,y-ht);
        p.quadraticTo(x,y-23, x+ht,y-ht);
        p.quadraticTo(x+9, y+9,x-ht, y+ht);
        p.quadraticTo(x-23,y, x-ht,y-ht);
        p.closeSubPath();
    }

    else if(cornerType==TrunkCorner::WEST_SOUTH)
    {
        p.startNewSubPath(x-ht,y+ht);
        p.quadraticTo(x-23,y, x-ht,y-ht);
        p.quadraticTo(x+9, y-9,x+ht, y+ht);
        p.quadraticTo(x,y+23, x-ht,y+ht);
        p.closeSubPath();
    }

    else if(cornerType==TrunkCorner::EAST_NORTH)
    {
        p.startNewSubPath(x+ht,y-ht);
        p.quadraticTo(x+23,y, x+ht,y+ht);
        p.quadraticTo(x-9, y+9,x-ht, y-ht);
        p.quadraticTo(x,y-23, x+ht,y-ht);
        p.closeSubPath();
    }

    else if(cornerType==TrunkCorner::EAST_SOUTH)
    {
        p.startNewSubPath(x+ht,y+ht);
        p.quadraticTo(x,y+23, x-ht,y+ht);
        p.quadraticTo(x-9, y-9,x+ht, y-ht);
        p.quadraticTo(x+23,y, x+ht,y+ht);
        p.closeSubPath();
    }

    else
    {
        p.startNewSubPath(x+ht,y+ht);
        p.addEllipse(x-ht,y-ht,2*ht, 2*ht);
        p.closeSubPath();
    }

    return p;

}
