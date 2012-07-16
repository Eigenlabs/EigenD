/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#include <iomanip>
#include <sstream>

#include <piw/piw_tsd.h>

#include <lib_midi/control_mapper_gui.h>
#include <lib_midi/midi_gm.h>

#include "juce.h"
#include "JucerPopupDialogComponent.h"
#include "JucerGlobalSettingsComponent.h"
#include "JucerCellPopupComponent.h"
#include "JucerHelpComponent.h"

static const char *mapping_help_label = "Help on routing matrix";
static const char *mapping_help_text = ""
"This is parameter routing matrix window. It allows you to route expression signals from your Eigenharp to control different aspects of your instruments.\n"
"\n"
"You will see a table within this window. The top row contains the Eigenharp signals. The parameters that can be controlled are displayed in a column on the left hand side. By clicking on the different tabs you will see other types of control parameters.\n"
"\n"
"To enable control of a parameter, click once within the desired cell. A popup dialog will appear with a collection of options that are specific to this intersection in the routing matrix. Depending on the context, this dialog will contain a different set of options, only showing the ones that make sense.\n"
"\n"
"Routing configuration dialog\n"
"===============================\n"
"\n"
"This is the list of possible configuration parameters in the popup dialog:\n"
"\n"
"* Scale factor:\n"
"\n"
"This number is the amount by which EigenD multiplies the signal data. Clicking the 'Invert' checkbox allows you to reverse the behavior of the signal. Setting the number to a larger value will normally mean that it has a greater effect. The scale factor will also be displayed in the table cell when the popup dialog is not visible.\n"
"\n"
"* Bounds:\n"
"\n"
"This allows you to fine-tune the data that is generated from a control signal. 'Base' sets the starting point that corresponds to a neutral signal on the Eigenharp control, 'lo' allows you to configure the amount of travel below the neutral point and 'hi' does this for the amount of travel above. The default values of 100% for 'lo' and 'hi' indicate a maximum amount of travel.\n"
"\n"
"For instance, setting 'base' to 50% for the 'key yaw' control and touching the key in its resting position, will generate the middle value of the parameter that you're controlling. Moving the key into either direction allows you to decrease or increase that middle value. Without increasing the base value above 0%, it wouldn't be possible to decrease the value by moving the keys, it would only be possible to increase it. By decreasing the 'lo' or the 'hi' value, you reduce the effective range of data that the Eigenharp control will generate, but you will still benefit from the full physical movement range of that control. This can allow you to for instance set the lower level on volume messages to prevent it from ever going to an inaudible value, or to make sure that it never becomes too loud.\n"
"\n"
"Note that these settings are dependent on the data that the control signal generates, the 'key yaw' signal has a low and high travel distance from its resting point, the 'key pressure' signal only has a high travel distance (the amount of pressure).\n"
"\n"
"It's also important to realize that the 'bounds' settings are applied after the 'scale factor' settings, meaning that the 'invert' checkbox will switch the 'lo' and 'hi' settings around.\n"
"\n"
"* Always return to origin:\n"
"\n"
"Certain expression signals don't always send their origin position when you stop using them. The key yaw for instance, doesn't send out its resting position signal when you remove your finger from the key when it's still tilted. Depending on what you map the expression signal to, you might however always want this origin value to be sent, this checkbox allows you to do just that.\n"
"\n"
"* Data decimation:\n"
"\n"
"Eigenharp signals operate at a very fine level of detail. For certain instruments, this might generate too much data, too quickly. By increasing the data decimation rate, EigenD will remove all the data within the interval you specified, only sending out data each time the interval has passed. This can dramatically reduce the amount of information that instruments have to process, at the expense of fine-grained expressiveness. Note that this works together with the global data decimation setting, the one with the largest interval will have priority. If your local decimation value is smaller than the global one, its number will be coloured red instead of white.\n"
"\n"
"* Control scope:\n"
"\n"
"Certain Eigenharp controls can be tied to musical notes. When the plug-in is set to polyphonic MIDI mode, you can use the 'Per-note' setting to only send the data to the MIDI channel that corresponds to the musical note that was played by the same control. If the signal has no note information, for instance the breath pipe, the 'Per-note' setting will cause for no data to be sent. If you use the 'Global' setting, the data will be sent to all MIDI channels. The 'Fixed channel' setting allows you to force the data to always be sent to the midi channel you specified.\n"
"\n"
"* Resolution:\n"
"\n"
"MIDI CC messages are by default 7-bit, which means that only 128 possible values can be sent. The Eigenharp controls are much more detailed and by switching to 14-bit and selecting an appropriate secondary MIDI CC number, you can combine the resolution of both MIDI CC messages and leverage more of the detail of the Eigenharp.\n"
"\n"
"* Enabled:\n"
"\n"
"Clicking the 'Enabled' checkbox allows you to turn the mapping on or off without losing its settings.\n"
"\n"
"* Clear:\n"
"\n"
"To erase the routing for a particular cell, click on the 'Clear' button in the popup dialog.\n"
"\n"
"Global settings\n"
"===============================\n"
"\n"
"The 'Settings' button at the top left of the routing matrix opens the global settings dialog. This gives you control over the following global behavior:\n"
"\n"
"* Data decimation:\n"
"\n"
"In the next section you'll learn about the data decimation setting for each mapping. The minimum data decimation setting here takes precedence over the individual settings. If the individual setting is lower than the minimum data decimation value, the value in the global settings will be used instead. This makes it easy to quickly impose a global data decimation rate without having to reconfigure each individual mapping.\n"
"\n"
"* Send notes:\n"
"\n"
"When checked, 'note on' and 'note off' MIDI events will be sent. When unchecked, no note events will be sent. This can be useful when you're setting up the Eigenharp keys for control and not for music.\n"
"\n"
"* Send pitch bend:\n"
"\n"
"When checked, pitch bend MIDI messages will be sent. When unchecked, no pitch bend message will not be sent.\n" 
"\n" 
"* Active MIDI channel:\n"
"\n" 
"Selects the channel on which MIDI messages have to be sent. Values 1 to 16 select a single MIDI channel, the 'poly' setting will spread out MIDI messages over several MIDI channels.\n"
"\n"
"* Minimum poly channel:\n"
"\n" 
"Selects the minimum channel that will be used when the active MIDI channel is in poly mode.\n"
"\n"
"* Maximum poly channel:\n"
"\n" 
"Selects the maximum channel that will be used when the active MIDI channel is in poly mode.\n"
"\n"
"More about 'poly' mode\n"
"===============================\n"
"\n"
"MIDI has been designed to use a single device or instrument for each MIDI channel. It supports polyphony for 'note on' and 'note off' messages on a single channel, however only supports one control message that is note-specific: polyphonic aftertouch. This makes using a single MIDI channel very restrictive in terms of expressiveness, it's not even possible to individually bend the pitch of a note while keeping others unchanged.\n"
"\n"
"EigenD's poly mode allows you to use several MIDI channels for the same instrument. When a note is being played on the keyboard of the Eigenharp, it will pick the next MIDI channel that's available and reserve that channel for the key. If no notes are being played yet, the minimum poly channel will be used, otherwise it will cycle through the next channel numbers to find one that's not being used yet, up to and including the maximum poly channel. If no channels are available, EigenD will start using channels that are already in-use by other keys. With a maximum polyphony of 16 channels, it's however possible to configure EigenD so that duplicate keys per channel never occur in practice.\n"
"\n"
"When the control scope of a CC message mapping is set to per-note and the signal originates from a key, EigenD will retrieve the MIDI channel that belongs to that key and send the CC message only to that channel. Similarly, pitch bend messages will only be sent to the channel that corresponds to the note that is being shifted in pitch.\n"
"\n"
"This provides almost limitless expressiveness for each individual note that is played on the Eigenharp over MIDI.\n"
"\n"
"Key position signal\n"
"===============================\n"
"\n"
"Expression signal 16 is a special case that extracts the key position at initial key press and sends that value once. It's not a continuous signal, but can be used to trigger events. One example is to map '16: Key position' to 'Program Change' in the 'MIDI Behaviour' panel to change programs in your software synthesizer by pressing a particular key in the keygroup.\n"
"\n"
"Legato trigger\n"
"===============================\n"
"\n"
"The 'Legato trigger' row in the 'MIDI Behaviour' panel allows you to temporarily change the behaviour in which channels are selected in polyphonic MIDI mode. The 'legato trigger' is activated when the data of the expression signal exceeds 50% of the value. This can be fine-tuned with the configuration dialog to make the trigger more or less sensitive.\n"
"\n"
"When the trigger is activated, the channel selection in polyphonic MIDI mode will not cycle through the next available channels, but will use the same MIDI channel for all the notes that are played while the trigger is active. The first note after the trigger was activated will still select a new channel so that phrases will be isolated from chords that you are already playing.\n"
"\n"
"This allows a series of notes to always be sent out on the same MIDI channel, allowing a software instrument to treat them as legato-style playing with appropriate sounds.\n"
;


namespace midi
{

    /*
     * toolbar_t
     */

    toolbar_t::toolbar_t(toolbar_delegate_t *delegate): delegate_(delegate)
    {
        setStyle(juce::Toolbar::textOnly);
        setColour(juce::Toolbar::backgroundColourId,juce::Colours::black);
        setColour(juce::Toolbar::labelTextColourId,juce::Colours::white);
        setColour(juce::Toolbar::buttonMouseOverBackgroundColourId,juce::Colours::black);
        setColour(juce::Toolbar::buttonMouseDownBackgroundColourId,juce::Colours::darkgrey);
        addDefaultItems(*delegate_);
        for(int i=0; i<getNumItems(); ++i)
        {
            getItemComponent(i)->addListener(delegate_);
        }
    }


    /*
     * toolbar_button_t
     */

     toolbar_button_t::toolbar_button_t(int id, const juce::String &text): ToolbarButton(id,text,0,0)
     {
     }

     bool toolbar_button_t::getToolbarItemSizes(int depth, bool vert, int &pref, int &min, int &max)
     {
        pref=min=max=60;
        return true;
     }


    /*
     * eigen_dialog_t
     */

    struct eigen_dialog_t: juce::DocumentWindow
    {
        eigen_dialog_t(const juce::String &title, juce::Component *c):
            DocumentWindow(title, juce::Colours::black, juce::DocumentWindow::closeButton, true)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(c, true);
            centreAroundComponent(c, getWidth(), getHeight());
            setVisible(true);
            setResizable(true,true);
            setTopLeftPosition(180,170);
        }

        void closeButtonPressed()
        {
            setVisible(false);
        }
    };


    /*
     * mapping_help_t
     */

    struct mapping_help_t: HelpComponent
    {
        mapping_help_t()
        {
            set_title(mapping_help_label);
            set_text(mapping_help_text);
            setSize(740,600);
        }
        void buttonClicked(juce::Button *b)
        {
            if(b==get_ok_button())
            {
                getParentComponent()->setVisible(false);
            }
        }
    };


    /*
     * PopupDialogWindow
     */

    class PopupDialogWindow : public DialogWindow, public juce::Button::Listener
    {
        public:    
            PopupDialogWindow(modal_dialog_listener_t *listener, String name)
                : DialogWindow(name, Colour(0x00), true, true), listener_(listener), title_bg_label_(0), title_label_(0), close_button_(0)
            {
                addChildComponent(title_bg_label_ = new Label(T("title label background"), T("")));
                title_bg_label_->setVisible(true);
                title_bg_label_->setEditable(false, false, false);
                title_bg_label_->setColour(Label::backgroundColourId, Colour (0xcc666666));

                addChildComponent(title_label_ = new Label(T("title label"), name));
                title_label_->setAlwaysOnTop(true);
                title_label_->setVisible(true);
                title_label_->setFont (Font (13.0000f, Font::plain));
                title_label_->setJustificationType(Justification::horizontallyCentred);
                title_label_->setEditable(false, false, false);
                title_label_->setColour(Label::textColourId, Colour (0x99ffffff));
                title_label_->addMouseListener(this, false);

                addChildComponent(close_button_ = createTabBarCloseButton());
                close_button_->setAlwaysOnTop(true);
                close_button_->setTriggeredOnMouseDown(true);
                close_button_->setTooltip(T("Close popup"));
                close_button_->setVisible(true);
                close_button_->addListener(this);

                // this is the only way to not have a title bar
                setUsingNativeTitleBar(false);
                setTitleBarHeight(0);
                setAlwaysOnTop(true);
            }
            
            ~PopupDialogWindow()
            {
                delete close_button_;
                delete title_label_;
                delete title_bg_label_;
            }

            Button* createTabBarCloseButton()
            {
                const float thickness = 7.0f;
                const float indent = 0.0f;

                Path p;
                p.addEllipse (-10.0f, -10.0f, 120.0f, 120.0f);

                DrawablePath ellipse;
                ellipse.setPath (p);
                ellipse.setFill (Colour (0x99ffffff));

                p.clear();
                p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
                p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
                p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
                p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);    
                p.setUsingNonZeroWinding (false);	
                p.applyTransform(AffineTransform::rotation(3.141f/4.0f, 50.0f, 50.0f));

                DrawablePath dp;
                dp.setPath (p);
                dp.setFill (Colour (0xcc000000));

                DrawableComposite normalImage;
                normalImage.addAndMakeVisible (ellipse.createCopy());
                normalImage.addAndMakeVisible (dp.createCopy());

                dp.setFill (Colour (0x59000000));

                DrawableComposite overImage;
                overImage.addAndMakeVisible (ellipse.createCopy());
                overImage.addAndMakeVisible (dp.createCopy());

                DrawableButton* db = new DrawableButton (T("closePopup"), DrawableButton::ImageFitted);
                db->setImages (&normalImage, &overImage, 0);
                return db;
            }

            void resized()
            {
                DialogWindow::resized();

                if(title_bg_label_)
                {
                    title_bg_label_->setBounds(4, 4, getWidth()-8, 28);
                }

                if(title_label_)
                {
                    title_label_->setBounds(0, 5, getWidth(), 28);
                }

                if(close_button_)
                {
                    int size = 17;

                    close_button_->setSize(size,size);
                    close_button_->setCentrePosition(getWidth()-(size-1), size-1);
                }
            }

            void buttonClicked(Button *button)
            {
                if(button == close_button_)
                {
                    closeButtonPressed();
                }
            }

            void mouseDown(const MouseEvent &e)
            {
                if(title_label_==e.eventComponent)
                {
                    mouse_down_x_ = getX();
                    mouse_down_y_ = getY();
                }
                else
                {
                    DialogWindow::mouseDown(e);
                }
            }

            void mouseDrag(const MouseEvent &e)
            {
                if(title_label_==e.eventComponent)
                {
                    setTopLeftPosition(mouse_down_x_+e.getDistanceFromDragStartX(), mouse_down_y_+e.getDistanceFromDragStartY());
                }
                else
                {
                    DialogWindow::mouseDrag(e);
                }
            }

            void closeButtonPressed()
            {
                if(listener_)
                {
                    listener_->dialog_closed();
                }
                exitModalState(0);
                setVisible(false);
            }

            void inputAttemptWhenModal()
            {
                closeButtonPressed();
            }

            virtual const BorderSize<int> getBorderThickness()
            {
                // return null border size for no border
                return BorderSize<int>();
            }

        private:
            PopupDialogWindow (const PopupDialogWindow&);
            PopupDialogWindow& operator= (const PopupDialogWindow&);

            modal_dialog_listener_t *listener_;
            Label* title_bg_label_;
            Label* title_label_;
            Button* close_button_;
            int mouse_down_x_;
            int mouse_down_y_;
    };


    /*
     * mapping_delegate_t
     */

    enum mapping_delegate_item_id
    {
        id_settings = 1,
        id_clearall = 2,
        id_help = 3
    };

    void mapping_delegate_t::close()
    {
        if(help_)
        {
            delete help_;
            help_=0;
        }
    }

    void mapping_delegate_t::getAllToolbarItemIds(juce::Array<int> &ids)
    {
        ids.add(id_settings);
        ids.add(id_clearall);
        ids.add(flexibleSpacerId);
        ids.add(id_help);
    }

    void mapping_delegate_t::getDefaultItemSet(juce::Array<int> &ids)
    {
        getAllToolbarItemIds(ids);
    }

    juce::ToolbarItemComponent *mapping_delegate_t::createItem(const int id)
    {
        switch(id)
        {
            case id_clearall:
                return button_clearall_ = new toolbar_button_t(id,"Clear all");
            case id_settings:
                return button_settings_ = new toolbar_button_t(id,"Settings");
            case id_help:
                return button_help_ = new toolbar_button_t(id,"Help");
        }
        return 0;
    }

    void mapping_delegate_t::buttonClicked(juce::Button *b)
    {
        if(b==button_clearall_)
        {
            clearall();
        }
        if(b==button_settings_)
        {
            settings();
        }
        if(b==button_help_)
        {
            help();
        }
    }

    void mapping_delegate_t::change_settings(global_settings_t settings)
    {
        settings_functors_.change_settings_(settings);
    }

    global_settings_t mapping_delegate_t::get_settings()
    {
        return settings_functors_.get_settings_();
    }

    void mapping_delegate_t::settings_changed()
    {
        if(global_settings_)
        {
            ((GlobalSettingsComponent *)global_settings_)->updateComponent();
        }
    }

    unsigned mapping_delegate_t::get_midi_channel()
    {
        return settings_functors_.get_midi_channel_();
    }

    void mapping_delegate_t::set_midi_channel(unsigned channel)
    {
        settings_functors_.set_midi_channel_(channel);
    }

    unsigned mapping_delegate_t::get_min_channel()
    {
        return settings_functors_.get_min_channel_();
    }

    void mapping_delegate_t::set_min_channel(unsigned channel)
    {
        settings_functors_.set_min_channel_(channel);
    }

    unsigned mapping_delegate_t::get_max_channel()
    {
        return settings_functors_.get_max_channel_();
    }

    void mapping_delegate_t::set_max_channel(unsigned channel)
    {
        settings_functors_.set_max_channel_(channel);
    }

    void mapping_delegate_t::clearall()
    {
        if(!AlertWindow::showOkCancelBox(AlertWindow::NoIcon, "Please confirm ...", "Are you certain that you want to clear all the mappings in all tabs?", "No", "Yes"))
        {
           settings_functors_.clearall_();
        }
    }

    void mapping_delegate_t::settings()
    {
        PopupDialogWindow window(0, juce::String("routing matrix settings"));
        PopupDialogComponent dialog;
        GlobalSettingsComponent dialogContent;

        dialogContent.initialize(this);
        window.setContentNonOwned(&dialog, true);
        window.setSize(dialogContent.getWidth()+32, dialogContent.getHeight()+42);
        window.centreAroundComponent(button_settings_, dialog.getWidth(), dialog.getHeight());
        window.setVisible(true);
        dialog.setContentComponent(&dialogContent, 0, 20);
        dialogContent.setFocusOrder();
        dialogContent.setDialogWindow(&window);

        global_settings_ = &dialogContent;
        window.runModalLoop();
        global_settings_ = 0;
    }

    void mapping_delegate_t::help()
    {
        if(help_)
        {
            help_->setVisible(!help_->isVisible());
        }
        else
        {
            help_ = new eigen_dialog_t("Help", new mapping_help_t());
        }
    }


    /*
     * mapper_table_t
     */

    class mapper_table_t::ClearTabButton : public TextButton
    {
        public:    
            ClearTabButton(const String &buttonName): TextButton(buttonName) {}

        protected:
            void paintButton(Graphics &g, bool isMouseOverButton, bool isButtonDown)
            {
                if(isMouseOverButton)
                {
                    juce::Rectangle<int> bounds = g.getClipBounds();
                    g.setColour(Colour(0x10ffffff));
                    g.fillRect(0, 0, bounds.getWidth(), bounds.getHeight());

                    setColour(TextButton::textColourOffId, Colour(0xffffffff));
                }
                else
                {
                    setColour(TextButton::textColourOffId, Colour(0xffdddddd));
                }

                getLookAndFeel().drawButtonText(g, *this,
                        isMouseOverButton,
                        isButtonDown);
            }
    };


    /*
     * mapper_table_model_t
     */

    int mapper_table_model_t::getNumRows()
    {
        return mapper_.getNumRows();
    }


    /*
     * header_table_model_t
     */

    juce::Component *header_table_model_t::refreshComponentForCell(int row, int col, bool sel, juce::Component *existing)
    {
        mapper_yaxis_editor_t *e = (mapper_yaxis_editor_t *)existing;
        if(!e)
        {
            e = mapper_.create_yaxis_editor();
        }

        e->setup(row,col);
        return e;
    }

    void header_table_model_t::paintRowBackground(juce::Graphics& g, int row, int w, int h, bool sel)
    {
        mapper_.paintRowBackground(g, row, w, h, sel);
    }

    void header_table_model_t::listWasScrolled()
    {
        mapper_.table_header_->getViewport()->setViewPosition(0, mapper_.table_mapping_->getViewport()->getViewPositionY());
    }

    void header_table_model_t::selectedRowsChanged(int row)
    {
        if(!mapper_.initialized_) return;
        int selected = mapper_.table_header_->getLastRowSelected();
        if(-1==selected)
        {
            mapper_.table_mapping_->deselectAllRows();
        }
        else
        {
            mapper_.table_mapping_->selectRow(selected, true, true);
        }
    }

    int header_table_model_t::getNumRows()
    {
        return mapper_table_model_t::getNumRows()+1;
    }


    /*
     * mapping_table_model_t
     */

    juce::Component *mapping_table_model_t::refreshComponentForCell(int row, int col, bool sel, juce::Component *existing)
    {
        mapper_cell_editor_t *e = (mapper_cell_editor_t *)existing;
        if(!e)
        {
            e = mapper_.create_cell_editor(row, col);
        }

        e->setup(row,col);
        return e;
    }

    void mapping_table_model_t::listWasScrolled()
    {
        mapper_.table_header_->getViewport()->setViewPosition(0, mapper_.table_mapping_->getViewport()->getViewPositionY());
    }

    void mapping_table_model_t::paintRowBackground(juce::Graphics& g, int row, int w, int h, bool sel)
    {
        mapper_.paintRowBackground(g, row, w, h, sel);
    }

    void mapping_table_model_t::selectedRowsChanged(int row)
    {
        if(!mapper_.initialized_) return;
        int selected = mapper_.table_mapping_->getLastRowSelected();
        if(-1==selected)
        {
            mapper_.table_header_->deselectAllRows();
        }
        else
        {
            mapper_.table_header_->selectRow(selected, true, true);
        }
    }


    /*
     * mapper_cell_editor_t
     */

    mapper_cell_editor_t::mapper_cell_editor_t(mapper_table_t &mapper): mapper_(mapper), edit_control_scope_(true), edit_fixed_channel_(true), edit_resolution_(true), span_poly_(false), active_popup_(0), cell_popup_(0)
    {
        addAndMakeVisible(label_=new juce::Label(juce::String::empty,"0.0"));
        label_->addMouseListener(this,false);
        label_->setJustificationType(juce::Justification::centred);
        label_->setColour(juce::Label::textColourId,juce::Colours::white);
        label_->setColour(juce::Label::outlineColourId,juce::Colours::darkgrey);
    }

    void mapper_cell_editor_t::setup(int row, int col)
    {
        iparam_ = col;
        oparam_ = row;
        draw_text();
    }

    global_settings_t mapper_cell_editor_t::get_settings()
    {
        return mapper_.settings_functors_.get_settings_();
    }

    bool mapper_cell_editor_t::is_mapped()
    {
        return mapper_.mapping_functors_.is_mapped_(iparam_,oparam_);
    }

    mapping_info_t mapper_cell_editor_t::get_info()
    {
        return mapper_.mapping_functors_.get_info_(iparam_,oparam_);
    }

    void mapper_cell_editor_t::map(bool enabled, float scale, float lo, float base, float hi, bool return_to_base, float decimation, unsigned scope, unsigned channel, unsigned resolution, int sec, unsigned curve)
    {
        mapper_.mapping_functors_.map_(iparam_,mapping_info_t(oparam_,enabled,scale,lo,base,hi,return_to_base,decimation,scope,channel,resolution,sec,curve));
        draw_text();
    }

    void mapper_cell_editor_t::colour_cell(mapping_info_t &info, bool span)
    {
        juce::Colour txt;
        juce::Colour bg;
        if(!info.is_valid())
        {
            txt = juce::Colours::white;
            bg = juce::Colours::black;
        }
        else if(info.enabled_)
        {
            txt = juce::Colours::white;
            bg = (info.scale_>0.0) ? juce::Colours::green : juce::Colours::red;
        }
        else
        {
            txt = juce::Colours::grey;
            bg = juce::Colours::darkgrey;
        }

        if(span)
        {
            txt = txt.darker();
            bg = bg.darker();
        }

        label_->setColour(juce::Label::textColourId, txt);
        label_->setColour(juce::Label::backgroundColourId, bg);

        if(cell_popup_)
        {
            ((CellPopupComponent *)cell_popup_)->updateComponent();
        }
    }

    bool mapper_cell_editor_t::is_spanned()
    {
        if(span_poly_ && oparam_ > 0 && 0 == mapper_.settings_functors_.get_midi_channel_())
        {
            int poly_range = mapper_.settings_functors_.get_max_channel_() - mapper_.settings_functors_.get_min_channel_();
            for(int o = oparam_-1; o >= 0 && o >= oparam_-poly_range; --o)
            {
                mapping_info_t poly_info = mapper_.mapping_functors_.get_info_(iparam_,o);
                if(poly_info.is_valid() && PERNOTE_SCOPE == poly_info.scope_)
                {
                    return true;
                }
            }
        }

        return false;
    }

    void mapper_cell_editor_t::draw_text()
    {
        if(span_poly_ && oparam_ > 0 && 0 == mapper_.settings_functors_.get_midi_channel_())
        {
            int poly_range = mapper_.settings_functors_.get_max_channel_()-mapper_.settings_functors_.get_min_channel_();
            for(int o = oparam_-1; o >= 0 && o >= oparam_-poly_range; --o)
            {
                mapping_info_t poly_info = mapper_.mapping_functors_.get_info_(iparam_,o);
                if(poly_info.is_valid() && PERNOTE_SCOPE == poly_info.scope_)
                {
                    int channel = mapper_.settings_functors_.get_min_channel_()+(oparam_-o);
                    label_->setText(juce::String::formatted("%.1f (ch.%02d)", poly_info.scale_, channel), true);
                    colour_cell(poly_info, true);
                    return;
                }
            }
        }

        mapping_info_t info = get_info();
        if(info.is_valid())
        {
            if(span_poly_ && 0 == mapper_.settings_functors_.get_midi_channel_() && PERNOTE_SCOPE == info.scope_)
            {
                label_->setText(juce::String::formatted("%.1f (ch.%02d)", info.scale_, mapper_.settings_functors_.get_min_channel_()), true);
            }
            else
            {
                label_->setText(juce::String(info.scale_,1),true);
            }
        }
        else
        {
            label_->setText("",true);
        }

        colour_cell(info, false);
    }

    void mapper_cell_editor_t::mouseEnter(const juce::MouseEvent &e)
    {
        if(is_spanned())
        {
            return;
        }

        label_->setColour(juce::Label::outlineColourId,juce::Colour(0xffdddddd));
        label_->repaint();
    }

    void mapper_cell_editor_t::mouseExit(const juce::MouseEvent &e)
    {
        if(is_spanned())
        {
            return;
        }

        label_->setColour(juce::Label::outlineColourId,juce::Colours::darkgrey);
        label_->repaint();
    }

    void mapper_cell_editor_t::mouseUp(const juce::MouseEvent &e)
    {
        if(is_spanned())
        {
            return;
        }

        bool modal = isCurrentlyBlockedByAnotherModalComponent();
        juce::Component::mouseUp(e);
        if(!modal && e.mouseWasClicked() && piw::tsd_time() - mapper_.last_modal_dismissal_ > 500000)
        {
            if(!get_info().is_valid())
            {
                mapper_.default_mapping(*this);
            }

            PopupDialogWindow window(this, mapper_.get_cell_editor_name(*this));
            active_popup_ = &window;
            PopupDialogComponent dialog;
            CellPopupComponent dialogContent;

            dialogContent.initialize(this);
            window.setContentNonOwned(&dialog, true);
            window.setSize(dialogContent.getWidth()+32, dialogContent.getHeight()+42);
            window.centreAroundComponent(this, dialog.getWidth(), dialog.getHeight());
            window.setVisible(true);
            dialog.setContentComponent(&dialogContent, 0, 20);
            dialogContent.setFocusOrder();

            cell_popup_ = &dialogContent;
            window.runModalLoop();
            cell_popup_ = 0;
            active_popup_ = 0;
        }

        draw_text();
    }

    void mapper_cell_editor_t::close_popup()
    {
        if(active_popup_)
        {
            active_popup_->closeButtonPressed();
        }
    }

    void mapper_cell_editor_t::dialog_closed()
    {
        mapper_.last_modal_dismissal_ = piw::tsd_time();
    }

    void mapper_cell_editor_t::unmap()
    {
        close_popup();
        mapper_.mapping_functors_.unmap_(iparam_,oparam_);
    }


    /*
     * mapper_midi_cell_editor_t
     */

     void mapper_midi_cell_editor_t::setup(int row, int col)
     {
        mapper_cell_editor_t::setup(row+MIDI_CC_MAX, col);
     }


    /*
     * mapper_yaxis_editor_t
     */

    mapper_yaxis_editor_t::mapper_yaxis_editor_t(mapper_table_t &mapper): mapper_(mapper), label_(0), row_(-1)
    {
        addAndMakeVisible(label_=new juce::Label(juce::String::empty,""));
        label_->addMouseListener(this,false);
        label_->setJustificationType(juce::Justification::centredLeft);
        label_->setColour(juce::Label::textColourId,juce::Colours::white);
        label_->setFont(mapper_.font_);
    }

    void mapper_yaxis_editor_t::setup(int row, int col)
    {
        row_ = row;

        juce::String str(mapper_.getRowName(row));
        label_->setText(str,true);
        int current_width=mapper_.calculate_width(row);
        
        int row_header_size = std::max(mapper_.table_header_->getHeader().getColumnWidth(1),current_width);
        mapper_.table_header_->getHeader().setColumnWidth(1,row_header_size);
        mapper_.resized();
    }

    void mapper_yaxis_editor_t::resized()
    {
        label_->setBoundsInset(juce::BorderSize<int>(1));
    }

    void mapper_yaxis_editor_t::mouseUp(const juce::MouseEvent &e)
    {
        juce::Component::mouseUp(e);
        if(row_ >= 0 && e.mouseWasClicked())
        {
            if(mapper_.table_header_->isRowSelected(row_))
            {
                mapper_.table_header_->deselectRow(row_);
            }
            else
            {
                mapper_.table_header_->selectRow(row_, true, true);
            }
        }
    }


    /*
     * mapper_tablelistbox_t
     */

    mapper_tablelistbox_t::mapper_tablelistbox_t(const juce::String &componentName, juce::TableListBoxModel *model) : juce::TableListBox(componentName, model) { }

    void mapper_tablelistbox_t::mouseWheelMove(const juce::MouseEvent &, float, float)
    {
        // always receive the mouseWheelMove event from the containing mapper_table_t component, which will call delegatedMouseWheelMove
    }

    void mapper_tablelistbox_t::delegatedMouseWheelMove(const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
    {
        if(wheelIncrementX != 0 || wheelIncrementY != 0)
        {
            juce::TableListBox::mouseWheelMove(e, wheelIncrementX, wheelIncrementY);
        }
    }


    /*
     * mapper_table_t
     */

    mapper_table_t::mapper_table_t(settings_functors_t settings, mapping_functors_t mapping): settings_functors_(settings), mapping_functors_(mapping), header_table_model_(*this), mapping_table_model_(*this), font_(12.f), last_modal_dismissal_(0), initialized_(false)
    {
        addAndMakeVisible(table_header_ = new mapper_tablelistbox_t(juce::String::empty,&header_table_model_));
        table_header_->getViewport()->setScrollBarsShown(false, false);
        table_header_->setColour(juce::ListBox::backgroundColourId,juce::Colours::black);
        table_header_->setColour(juce::ListBox::outlineColourId,juce::Colours::grey);
        table_header_->setColour(juce::ListBox::textColourId,juce::Colours::white);
        table_header_->setOutlineThickness(0);
        table_header_->setHeaderHeight(0);
        table_header_->addMouseListener(this, true);

        addAndMakeVisible(table_mapping_ = new mapper_tablelistbox_t(juce::String::empty,&mapping_table_model_));
        table_mapping_->setColour(juce::ListBox::backgroundColourId,juce::Colours::black);
        table_mapping_->setColour(juce::ListBox::outlineColourId,juce::Colours::grey);
        table_mapping_->setColour(juce::ListBox::textColourId,juce::Colours::white);
        table_mapping_->setOutlineThickness(0);

        table_header_->getHeader().addColumn(juce::String::empty,1,110,110,500,juce::TableHeaderComponent::visible);

        for(unsigned i=1; i<=32; ++i)
        {
            juce::String name(mapping_functors_.get_name_(i).c_str());
            table_mapping_->getHeader().addColumn(name,i,std::max(80,font_.getStringWidth(name)+20),80,200,juce::TableHeaderComponent::visible|juce::TableHeaderComponent::resizable);
        }

        addAndMakeVisible(clear_tab_ = new ClearTabButton(T("clear tab button")));
        clear_tab_->setButtonText(T("Clear matrix"));
        clear_tab_->addListener(this);

        setBounds(0,0,600,400);
    }

    void mapper_table_t::initialize()
    {
        int header_width = 0;
        for(int i=0; i<getNumRows(); i++)
        {
            header_width = std::max(header_width, calculate_width(i));
            table_header_->getHeader().setColumnWidth(1,header_width);
        }
        resized();
        initialized_=true;
    }

    void mapper_table_t::paint(juce::Graphics& g)
    {
        juce::Rectangle<int> bounds = g.getClipBounds();
        g.setColour(Colour(0xffdddddd));
        g.fillRect(0, 0, bounds.getWidth(), 1);
        bounds.setTop(1);
        ColourGradient gradient(Colour(0xff404040), bounds.getWidth()/2, 0, Colour(0xff101010), bounds.getWidth()/2, bounds.getHeight(), false);
        gradient.addColour (0.9, Colour(0xff202020));
        g.setGradientFill(gradient);
        g.fillRect(bounds);
    }

    void mapper_table_t::resized()
    {
        int header_height = table_mapping_->getHeaderHeight();
        int first_column_width = table_header_->getHeader().getColumnWidth(1);
        table_header_->setBounds(0,header_height,first_column_width,getHeight()-header_height);
        table_mapping_->setBounds(table_header_->getWidth(),0,getWidth()-(table_header_->getWidth()),getHeight());

        clear_tab_->setBounds(0, 1, first_column_width, header_height-2);
    }

    void mapper_table_t::column_changed(int col)
    {
        juce::String name(mapping_functors_.get_name_(col).c_str());
        table_mapping_->getHeader().setColumnName(col,name);
        table_mapping_->getHeader().setColumnWidth(col,std::max(table_mapping_->getHeader().getColumnWidth(col), font_.getStringWidth(name)+20));
        table_mapping_->updateContent();
    }

    void mapper_table_t::buttonClicked(juce::Button* button)
    {
        if(button == clear_tab_)
        {
            if(!AlertWindow::showOkCancelBox(AlertWindow::NoIcon, "Please confirm ...", "Are you certain that you want to clear all the mappings in this tab?", "No", "Yes"))
            {
                mapping_functors_.clear_();
            }
        }
    }

    void mapper_table_t::paintRowBackground(juce::Graphics& g, int row, int w, int h, bool sel)
    {
        if(sel)
        {
            g.fillAll(juce::Colours::grey);
        }
    }

    const juce::String mapper_table_t::get_cell_editor_name(mapper_cell_editor_t &e)
    {
        std::stringstream oss;
        oss << "parameter ";
        oss << e.iparam_;
        oss << " for ";
        oss << e.oparam_;
        return juce::String(oss.str().c_str());
    }

    mapper_cell_editor_t* mapper_table_t::create_cell_editor(int row, int col)
    {
        mapper_cell_editor_t *editor = new mapper_cell_editor_t(*this);
        editor->edit_fixed_channel(false);
        editor->edit_resolution(false);
        editor->span_poly(true);
        return editor;
    }

    int mapper_table_t::calculate_width(int row)
    {
        juce::String str(getRowName(row));
        return font_.getStringWidth(str)+20;
    }

    void mapper_table_t::mouseWheelMove(const juce::MouseEvent &e, float wheelIncrementX, float wheelIncrementY)
    {
        if(wheelIncrementX != 0 || wheelIncrementY != 0)
        {
            table_mapping_->delegatedMouseWheelMove(e, wheelIncrementX, wheelIncrementY);
        }
    }


    /*
     * mapper_midi_cc_table_t
     */

    int mapper_midi_cc_table_t::getNumRows()
    {
        return MIDI_CC_MAX;
    }

    juce::String mapper_midi_cc_table_t::getRowName(int row)
    {
        std::stringstream oss;
        if(row>=getNumRows())
        {
            return juce::String();
        }
        else
        {
            oss << row;
            oss << ": CC ";
            if(row<MIDI_CC_MAX)
            {
                oss << midi::midi_cc[row];
            }
        }
        return juce::String(oss.str().c_str());
    }

    const juce::String mapper_midi_cc_table_t::get_cell_editor_name(mapper_cell_editor_t &e)
    {
        std::stringstream oss;
        oss << "parameter ";
        oss << e.iparam_;
        oss << " for midi ";
        oss << e.oparam_;
        return juce::String(oss.str().c_str());
    }

    void mapper_midi_cc_table_t::default_mapping(mapper_cell_editor_t &e)
    {
        e.map(true,1.f,1.f,0.f,1.f,false,10.f,GLOBAL_SCOPE,0,BITS_7,-1,CURVE_LINEAR);
    }

    mapper_cell_editor_t* mapper_midi_cc_table_t::create_cell_editor(int row, int col)
    {
        return new mapper_cell_editor_t(*this);
    }


    /*
     * mapper_midi_behaviour_table_t
     */

    int mapper_midi_behaviour_table_t::getNumRows()
    {
        return MIDI_STATUS_MAX+MIDI_EIGEND_MAX;
    }

    juce::String mapper_midi_behaviour_table_t::getRowName(int row)
    {
        std::stringstream oss;
        if(row>=getNumRows())
        {
            return juce::String();
        }
        else
        {
            if(row>=MIDI_STATUS_MAX)
            {
                oss << midi_eigend[row-MIDI_STATUS_MAX];
            }
            else
            {
                oss << midi_status[row];
            }
        }
        return juce::String(oss.str().c_str());
    }

    const juce::String mapper_midi_behaviour_table_t::get_cell_editor_name(mapper_cell_editor_t &e)
    {
        std::stringstream oss;
        oss << "parameter ";
        oss << e.iparam_;
        oss << " for ";
        int row=e.oparam_-MIDI_CC_MAX;
        if(row>=MIDI_STATUS_MAX)
        {
            oss << juce::String(midi_eigend[row-MIDI_STATUS_MAX].c_str()).toLowerCase();
        }
        else
        {
            oss << juce::String(midi_status[row].c_str()).toLowerCase();
        }
        return juce::String(oss.str().c_str());
    }

    mapper_cell_editor_t* mapper_midi_behaviour_table_t::create_cell_editor(int row, int col)
    {
        mapper_midi_cell_editor_t *editor = new mapper_midi_cell_editor_t(*this);
        editor->edit_resolution(false);
        if(row>=MIDI_STATUS_MAX)
        {
            editor->edit_control_scope(false);
            editor->edit_fixed_channel(false);
        }
 
        return editor;
    }

    void mapper_midi_behaviour_table_t::default_mapping(mapper_cell_editor_t &e)
    {
        e.map(true,1.f,1.f,0.f,1.f,false,10.f,GLOBAL_SCOPE,0,BITS_7,-1,CURVE_LINEAR);
    }
} // namespace midi
