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

#include "../jucer_Headers.h"
#include "jucer_Application.h"
#include "jucer_MainWindow.h"
#include "jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "../Project/jucer_NewProjectWizard.h"
#include "../Utility/jucer_JucerTreeViewBase.h"

ScopedPointer<ApplicationCommandManager> commandManager;


//==============================================================================
MainWindow::MainWindow()
    : DocumentWindow (IntrojucerApp::getApp().getApplicationName(),
                      Colour::greyLevel (0.6f),
                      DocumentWindow::allButtons,
                      false)
{
    setUsingNativeTitleBar (true);
    createProjectContentCompIfNeeded();

   #if ! JUCE_MAC
    setMenuBar (IntrojucerApp::getApp().menuModel);
   #endif

    setResizable (true, false);
    centreWithSize (800, 600);

    // Register all the app commands..
    commandManager->registerAllCommandsForTarget (this);
    commandManager->registerAllCommandsForTarget (getProjectContentComponent());

    // update key mappings..
    {
        commandManager->getKeyMappings()->resetToDefaultMappings();

        ScopedPointer <XmlElement> keys (getGlobalProperties().getXmlValue ("keyMappings"));

        if (keys != nullptr)
            commandManager->getKeyMappings()->restoreFromXml (*keys);

        addKeyListener (commandManager->getKeyMappings());
    }

    // don't want the window to take focus when the title-bar is clicked..
    setWantsKeyboardFocus (false);

    //getPeer()->setCurrentRenderingEngine (0);
    getLookAndFeel().setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);

    setResizeLimits (600, 500, 32000, 32000);
}

MainWindow::~MainWindow()
{
   #if ! JUCE_MAC
    setMenuBar (nullptr);
   #endif

    removeKeyListener (commandManager->getKeyMappings());

    // save the current size and position to our settings file..
    getGlobalProperties().setValue ("lastMainWindowPos", getWindowStateAsString());

    clearContentComponent();
    currentProject = nullptr;
}

void MainWindow::createProjectContentCompIfNeeded()
{
    if (getProjectContentComponent() == nullptr)
    {
        clearContentComponent();
        setContentOwned (IntrojucerApp::getApp().createProjectContentComponent(), false);
        jassert (getProjectContentComponent() != nullptr);
    }
}

void MainWindow::makeVisible()
{
    restoreWindowPosition();
    setVisible (true);
    addToDesktop();  // (must add before restoring size so that fullscreen will work)
    restoreWindowPosition();

    getContentComponent()->grabKeyboardFocus();
}

ProjectContentComponent* MainWindow::getProjectContentComponent() const
{
    return dynamic_cast <ProjectContentComponent*> (getContentComponent());
}

void MainWindow::closeButtonPressed()
{
    IntrojucerApp::getApp().mainWindowList.closeWindow (this);
}

bool MainWindow::closeProject (Project* project)
{
    jassert (project == currentProject && project != nullptr);

    if (project == nullptr)
        return true;

    project->getStoredProperties().setValue (getProjectWindowPosName(), getWindowStateAsString());

    ProjectContentComponent* const pcc = getProjectContentComponent();

    if (pcc != nullptr)
    {
        pcc->saveTreeViewState();
        pcc->saveOpenDocumentList();
        pcc->hideEditor();
    }

    if (! IntrojucerApp::getApp().openDocumentManager.closeAllDocumentsUsingProject (*project, true))
        return false;

    FileBasedDocument::SaveResult r = project->saveIfNeededAndUserAgrees();

    if (r == FileBasedDocument::savedOk)
    {
        setProject (nullptr);
        return true;
    }

    return false;
}

bool MainWindow::closeCurrentProject()
{
    return currentProject == nullptr || closeProject (currentProject);
}

void MainWindow::setProject (Project* newProject)
{
    createProjectContentCompIfNeeded();
    getProjectContentComponent()->setProject (newProject);
    currentProject = newProject;
    getProjectContentComponent()->updateMainWindowTitle();
    commandManager->commandStatusChanged();
}

void MainWindow::restoreWindowPosition()
{
    String windowState;

    if (currentProject != nullptr)
        windowState = currentProject->getStoredProperties().getValue (getProjectWindowPosName());

    if (windowState.isEmpty())
        windowState = getGlobalProperties().getValue ("lastMainWindowPos");

    restoreWindowStateFromString (windowState);
}

bool MainWindow::canOpenFile (const File& file) const
{
    return file.hasFileExtension (Project::projectFileExtension)
             || IntrojucerApp::getApp().openDocumentManager.canOpenFile (file);
}

bool MainWindow::openFile (const File& file)
{
    createProjectContentCompIfNeeded();

    if (file.hasFileExtension (Project::projectFileExtension))
    {
        ScopedPointer <Project> newDoc (new Project (file));

        if (newDoc->loadFrom (file, true)
             && closeCurrentProject())
        {
            setProject (newDoc.release());
            return true;
        }
    }
    else if (file.exists())
    {
        return getProjectContentComponent()->showEditorForFile (file, true);
    }

    return false;
}

bool MainWindow::isInterestedInFileDrag (const StringArray& filenames)
{
    for (int i = filenames.size(); --i >= 0;)
        if (canOpenFile (filenames[i]))
            return true;

    return false;
}

void MainWindow::filesDropped (const StringArray& filenames, int mouseX, int mouseY)
{
    for (int i = filenames.size(); --i >= 0;)
    {
        const File f (filenames[i]);

        if (canOpenFile (f) && openFile (f))
            break;
    }
}

bool MainWindow::shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                                       StringArray& files, bool& canMoveFiles)
{
    if (TreeView* tv = dynamic_cast <TreeView*> (sourceDetails.sourceComponent.get()))
    {
        Array<JucerTreeViewBase*> selected;

        for (int i = tv->getNumSelectedItems(); --i >= 0;)
            if (JucerTreeViewBase* b = dynamic_cast <JucerTreeViewBase*> (tv->getSelectedItem(i)))
                selected.add (b);

        if (selected.size() > 0)
        {
            for (int i = selected.size(); --i >= 0;)
            {
                const File f (selected.getUnchecked(i)->getDraggableFile());

                if (f.existsAsFile())
                    files.add (f.getFullPathName());
            }

            canMoveFiles = false;
            return files.size() > 0;
        }
    }

    return false;
}

void MainWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();

    if (getProjectContentComponent() != nullptr)
        getProjectContentComponent()->updateMissingFileStatuses();

    IntrojucerApp::getApp().openDocumentManager.reloadModifiedFiles();
}

void MainWindow::updateTitle (const String& documentName)
{
    String name (IntrojucerApp::getApp().getApplicationName());

    if (currentProject != nullptr)
        name << " - " << currentProject->getDocumentTitle();

    if (documentName.isNotEmpty())
        name << " - " << documentName;

    setName (name);
}

void MainWindow::showNewProjectWizard()
{
    jassert (currentProject == nullptr);
    setContentOwned (NewProjectWizard::createComponent(), true);
    makeVisible();
}

//==============================================================================
ApplicationCommandTarget* MainWindow::getNextCommandTarget()
{
    return nullptr;
}

void MainWindow::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { CommandIDs::closeWindow };

    commands.addArray (ids, numElementsInArray (ids));
}

void MainWindow::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::closeWindow:
        result.setInfo ("Close Window", "Closes the current window", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier, 0));
        break;

    default:
        break;
    }
}

bool MainWindow::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::closeWindow:
            closeButtonPressed();
            break;

        default:
            return false;
    }

    return true;
}


//==============================================================================
MainWindowList::MainWindowList()
{
}

void MainWindowList::forceCloseAllWindows()
{
    windows.clear();
}

bool MainWindowList::askAllWindowsToClose()
{
    saveCurrentlyOpenProjectList();

    while (windows.size() > 0)
    {
        if (! windows[0]->closeCurrentProject())
            return false;

        windows.remove (0);
    }

    return true;
}

void MainWindowList::createWindowIfNoneAreOpen()
{
    if (windows.size() == 0)
        createNewMainWindow()->makeVisible();
}

void MainWindowList::closeWindow (MainWindow* w)
{
    jassert (windows.contains (w));

   #if ! JUCE_MAC
    if (windows.size() == 1)
    {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }
    else
   #endif
    {
        if (w->closeCurrentProject())
        {
            windows.removeObject (w);
            saveCurrentlyOpenProjectList();
        }
    }
}

void MainWindowList::openDocument (OpenDocumentManager::Document* doc, bool grabFocus)
{
    MainWindow* w = getOrCreateFrontmostWindow();
    w->getProjectContentComponent()->showDocument (doc, grabFocus);
}

bool MainWindowList::openFile (const File& file)
{
    for (int i = windows.size(); --i >= 0;)
    {
        MainWindow* const w = windows.getUnchecked(i);

        if (w->getProject() != nullptr && w->getProject()->getFile() == file)
        {
            w->toFront (true);
            return true;
        }
    }

    if (file.hasFileExtension (Project::projectFileExtension))
    {
        ScopedPointer <Project> newDoc (new Project (file));

        if (newDoc->loadFrom (file, true))
        {
            MainWindow* const w = getOrCreateEmptyWindow();
            w->setProject (newDoc);

            newDoc.release()->setChangedFlag (false);

            w->makeVisible();
            avoidSuperimposedWindows (w);

            jassert (w->getProjectContentComponent() != nullptr);
            w->getProjectContentComponent()->reloadLastOpenDocuments();

            return true;
        }
    }
    else if (file.exists())
    {
        MainWindow* const w = getOrCreateFrontmostWindow();
        return w->openFile (file);
    }

    return false;
}

MainWindow* MainWindowList::createNewMainWindow()
{
    MainWindow* const w = new MainWindow();
    windows.add (w);
    w->restoreWindowPosition();
    avoidSuperimposedWindows (w);
    return w;
}

MainWindow* MainWindowList::getOrCreateFrontmostWindow()
{
    if (windows.size() == 0)
    {
        MainWindow* w = createNewMainWindow();
        avoidSuperimposedWindows (w);
        w->makeVisible();
        return w;
    }

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        MainWindow* mw = dynamic_cast <MainWindow*> (Desktop::getInstance().getComponent (i));
        if (windows.contains (mw))
            return mw;
    }

    return windows.getLast();
}

MainWindow* MainWindowList::getOrCreateEmptyWindow()
{
    if (windows.size() == 0)
        return createNewMainWindow();

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        MainWindow* mw = dynamic_cast <MainWindow*> (Desktop::getInstance().getComponent (i));
        if (windows.contains (mw) && mw->getProject() == nullptr)
            return mw;
    }

    return createNewMainWindow();
}

void MainWindowList::avoidSuperimposedWindows (MainWindow* const mw)
{
    for (int i = windows.size(); --i >= 0;)
    {
        MainWindow* const other = windows.getUnchecked(i);

        const Rectangle<int> b1 (mw->getBounds());
        const Rectangle<int> b2 (other->getBounds());

        if (mw != other
             && std::abs (b1.getX() - b2.getX()) < 3
             && std::abs (b1.getY() - b2.getY()) < 3
             && std::abs (b1.getRight()  - b2.getRight()) < 3
             && std::abs (b1.getBottom() - b2.getBottom()) < 3)
        {
            int dx = 40, dy = 30;

            if (b1.getCentreX() >= mw->getScreenBounds().getCentreX())   dx = -dx;
            if (b1.getCentreY() >= mw->getScreenBounds().getCentreY())   dy = -dy;

            mw->setBounds (b1.translated (dx, dy));
        }
    }
}

void MainWindowList::saveCurrentlyOpenProjectList()
{
    Array<File> projects;

    Desktop& desktop = Desktop::getInstance();
    for (int i = 0; i < desktop.getNumComponents(); ++i)
    {
        MainWindow* const mw = dynamic_cast <MainWindow*> (desktop.getComponent(i));

        if (mw != nullptr && mw->getProject() != nullptr)
            projects.add (mw->getProject()->getFile());
    }

    getAppSettings().setLastProjects (projects);
}

void MainWindowList::reopenLastProjects()
{
    Array<File> projects (getAppSettings().getLastProjects());

    for (int i = 0; i < projects.size(); ++ i)
        openFile (projects.getReference(i));
}

void MainWindowList::sendLookAndFeelChange()
{
    for (int i = windows.size(); --i >= 0;)
        windows.getUnchecked(i)->sendLookAndFeelChange();
}
