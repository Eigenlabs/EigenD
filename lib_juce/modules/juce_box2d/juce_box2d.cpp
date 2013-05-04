/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#if defined (__JUCE_BOX2D_JUCEHEADER__) && ! JUCE_AMALGAMATED_INCLUDE
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "juce_box2d.h"

#include "box2d/Collision/b2BroadPhase.cpp"
#include "box2d/Collision/b2CollideCircle.cpp"
#include "box2d/Collision/b2CollideEdge.cpp"
#include "box2d/Collision/b2CollidePolygon.cpp"
#include "box2d/Collision/b2Collision.cpp"
#include "box2d/Collision/b2Distance.cpp"
#include "box2d/Collision/b2DynamicTree.cpp"
#include "box2d/Collision/b2TimeOfImpact.cpp"
#include "box2d/Collision/Shapes/b2ChainShape.cpp"
#include "box2d/Collision/Shapes/b2CircleShape.cpp"
#include "box2d/Collision/Shapes/b2EdgeShape.cpp"
#include "box2d/Collision/Shapes/b2PolygonShape.cpp"
#include "box2d/Common/b2BlockAllocator.cpp"
#include "box2d/Common/b2Draw.cpp"
#include "box2d/Common/b2Math.cpp"
#include "box2d/Common/b2Settings.cpp"
#include "box2d/Common/b2StackAllocator.cpp"
#include "box2d/Common/b2Timer.cpp"
#include "box2d/Dynamics/b2Body.cpp"
#include "box2d/Dynamics/b2ContactManager.cpp"
#include "box2d/Dynamics/b2Fixture.cpp"
#include "box2d/Dynamics/b2Island.cpp"
#include "box2d/Dynamics/b2World.cpp"
#include "box2d/Dynamics/b2WorldCallbacks.cpp"
#include "box2d/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
#include "box2d/Dynamics/Contacts/b2CircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2Contact.cpp"
#include "box2d/Dynamics/Contacts/b2ContactSolver.cpp"
#include "box2d/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
#include "box2d/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2PolygonContact.cpp"
#include "box2d/Dynamics/Joints/b2DistanceJoint.cpp"
#include "box2d/Dynamics/Joints/b2FrictionJoint.cpp"
#include "box2d/Dynamics/Joints/b2GearJoint.cpp"
#include "box2d/Dynamics/Joints/b2Joint.cpp"
#include "box2d/Dynamics/Joints/b2MouseJoint.cpp"
#include "box2d/Dynamics/Joints/b2PrismaticJoint.cpp"
#include "box2d/Dynamics/Joints/b2PulleyJoint.cpp"
#include "box2d/Dynamics/Joints/b2RevoluteJoint.cpp"
#include "box2d/Dynamics/Joints/b2RopeJoint.cpp"
#include "box2d/Dynamics/Joints/b2WeldJoint.cpp"
#include "box2d/Dynamics/Joints/b2WheelJoint.cpp"
#include "box2d/Rope/b2Rope.cpp"

namespace juce
{
#include "utils/juce_Box2DRenderer.cpp"
}
