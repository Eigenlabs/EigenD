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

#include "jucer_DocumentEditorComponent.h"
#include "../Project/jucer_ProjectContentComponent.h"
#include "../Application/jucer_Application.h"


//==============================================================================
DocumentEditorComponent::DocumentEditorComponent (OpenDocumentManager::Document* doc)
    : document (doc)
{
    IntrojucerApp::getApp().openDocumentManager.addListener (this);
}

DocumentEditorComponent::~DocumentEditorComponent()
{
    IntrojucerApp::getApp().openDocumentManager.removeListener (this);
}

void DocumentEditorComponent::documentAboutToClose (OpenDocumentManager::Document* closingDoc)
{
    if (document == closingDoc)
    {
        jassert (document != nullptr);

        if (ProjectContentComponent* pcc = findParentComponentOfClass<ProjectContentComponent>())
            pcc->hideDocument (document);
    }
}

void DocumentEditorComponent::setEditedState (bool /*hasBeenEdited*/)
{
    if (ProjectContentComponent* pcc = findParentComponentOfClass<ProjectContentComponent>())
        pcc->updateMainWindowTitle();
}
