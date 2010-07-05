//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#ifdef ADDONS_MANAGER

#include "states_screens/dialogs/addons_loading.hpp"

#include <pthread.h>

#include "guiengine/engine.hpp"
#include "guiengine/widgets.hpp"
#include "input/input_manager.hpp"
#include "utils/translation.hpp"
#include "addons/addons.hpp"
#include "addons/network.hpp"
#include "states_screens/addons_screen.hpp"
#include "utils/string_utils.hpp"
#include "io/file_manager.hpp"

using namespace GUIEngine;
using namespace irr::gui;

// ------------------------------------------------------------------------------------------------------

AddonsLoading::AddonsLoading(Addons * id, const float w, const float h) :
        ModalDialog(w, h)
{
    this->addons = id;
    m_can_install = false;
    m_percent_update = false;
    pthread_mutex_init(&mutex_can_install, NULL);
    core::rect< s32 > area_right(10, 0,  m_area.getWidth()/2, m_area.getHeight());

    core::rect< s32 > area_left(m_area.getWidth()/2 + 20, 0, m_area.getWidth(), m_area.getHeight());
    /*Init the icon here to be able to load a single image*/
    icon = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO, false, /*focusbale*/ false);

    /* Next and previous button*/
    m_next = new IconButtonWidget();
    m_next->setImage("gui/next_addons.png");
    m_next->m_x = area_right.UpperLeftCorner.X + 100;
    m_next->m_y = area_right.UpperLeftCorner.Y +20 ;
    m_next->m_w = 75;
    m_next->m_properties[PROP_ID] = "next";
    m_next->m_h = 40;
    m_next->setParent(m_irrlicht_window);
    m_children.push_back(m_next);
    m_next->add();

    m_previous = new IconButtonWidget();
    m_previous->setImage("gui/back_addons.png");
    m_previous->m_x = area_right.UpperLeftCorner.X + 20;
    m_previous->m_y = area_right.UpperLeftCorner.Y +20 ;
    m_previous->m_w = 75;
    m_previous->m_properties[PROP_ID] = "previous";
    m_previous->m_h = 40;
    m_previous->setParent(m_irrlicht_window);
    m_children.push_back(m_previous);
    m_previous->add();

    name = new LabelWidget();
    name->m_text = StringUtils::insertValues(_("Name: %i"), this->addons->GetName().c_str());
    name->m_x = area_left.UpperLeftCorner.X;
    name->m_y = area_left.UpperLeftCorner.Y;
    name->m_w = area_left.getWidth();
    name->m_h = area_left.getHeight()/6;
    name->setParent(m_irrlicht_window);

    m_children.push_back(name);
    name->add();

    description = new LabelWidget();
    description->m_properties[PROP_WORD_WRAP] = "true";
    description->m_x = area_left.UpperLeftCorner.X;
    description->m_y = area_left.UpperLeftCorner.Y + area_left.getHeight()/6;
    description->m_w = area_left.getWidth();
    description->m_h = area_left.getHeight()/3;
    description->setParent(m_irrlicht_window);
    description->m_text = StringUtils::insertValues(_("Description: %i"), this->addons->GetDescription().c_str());
    description->add();
    m_children.push_back(description);
    
    version = new LabelWidget();
    version->m_x = area_left.UpperLeftCorner.X;
    version->m_y = area_left.UpperLeftCorner.Y + area_left.getHeight()/6 + area_left.getHeight()/3;
    version->m_text = StringUtils::insertValues(_("Version: %i"), this->addons->GetVersionAsStr().c_str());
    version->m_w = area_left.getWidth();
    version->m_h = area_left.getHeight()/3;
    version->setParent(m_irrlicht_window);

    m_children.push_back(version);
    version->add();

    m_progress = new LabelWidget();
    m_progress->m_x = 180;
    m_progress->m_y = m_area.getHeight()-45;
    m_progress->m_text = "";
    m_progress->m_w = m_area.getWidth() - 180;
    m_progress->m_h = 25;
    m_progress->setParent(m_irrlicht_window);

    m_children.push_back(m_progress);
    m_progress->add();
    
    this->loadInfo();
}
void AddonsLoading::loadInfo()
{

    core::rect< s32 > area_right(10, 0,  m_area.getWidth()/2, m_area.getHeight());

    core::rect< s32 > area_left(m_area.getWidth()/2 + 20, 0, m_area.getWidth(), m_area.getHeight());

    /*I think we can wait a little to have the icon ?*/
    download("icon/" + this->addons->GetIcon(), this->addons->GetName() + ".png");
    icon->setImage(std::string(file_manager->getConfigDir() + "/" +  this->addons->GetName() + ".png").c_str(), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    icon->m_x = area_right.UpperLeftCorner.X;
    icon->m_y = area_right.UpperLeftCorner.Y;
    icon->m_w = m_area.getWidth()/2;
    icon->m_h = m_area.getHeight();
    icon->setParent(m_irrlicht_window);
    m_children.push_back(icon);
    icon->add();



    name->setText(StringUtils::insertValues(_("Name: %i"), this->addons->GetName().c_str()));
    description->setText(StringUtils::insertValues(_("Description: %i"), this->addons->GetDescription().c_str()));
    version->setText(StringUtils::insertValues(_("Version: %i"), this->addons->GetVersionAsStr().c_str()));



    m_back_button = new ButtonWidget();
    /*m_back_button->setLabel(std::string("Back").c_str());*/
    m_back_button->m_text = _("Back");
    m_back_button->m_properties[PROP_ID] = "cancel";
    m_back_button->m_x = 20;
    m_back_button->m_y = m_area.getHeight()-45;
    m_back_button->setParent(m_irrlicht_window);
    m_children.push_back(m_back_button);
    m_back_button->m_w = 150;
    m_back_button->m_h = 35;
    m_back_button->add();

    this->install_button = new ButtonWidget();
    /*m_back_button->setLabel(std::string("Back").c_str());*/
    if(this->addons->IsInstalled() == "yes")
        this->install_button->m_text = _("Uninstall");
    else
        this->install_button->m_text = _("Install");
    this->install_button->m_properties[PROP_ID] = "install";
    this->install_button->m_x = m_area.getWidth()-170;
    this->install_button->m_y = m_area.getHeight()-45;
    this->install_button->setParent(m_irrlicht_window);
    m_children.push_back(this->install_button);
    this->install_button->m_w = 150;
    this->install_button->m_h = 35;
    this->install_button->add();

}
// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation AddonsLoading::processEvent(const std::string& eventSource)
{
    if(eventSource == "cancel")
    {
        //input_manager->setMode(InputManager::MENU);
        dismiss();
        //return GUIEngine::EVENT_BLOCK;
    }
    else if(eventSource == "next")
    {
        this->addons->NextType(this->addons->GetType());
        this->loadInfo();
    }
    else if(eventSource == "previous")
    {
        this->addons->PreviousType(this->addons->GetType());
        this->loadInfo();
    }
    if(eventSource == "install")
    {
        m_back_button->setDeactivated();
        m_next->setDeactivated();
        m_previous->setDeactivated();
        this->install_button->setDeactivated();
        m_percent_update = true;
        pthread_t thread;
        pthread_create(&thread, NULL, &AddonsLoading::startInstall, this);
    }
    return GUIEngine::EVENT_LET;
}
// ------------------------------------------------------------------------------------------------------
void AddonsLoading::onUpdate(float delta)
{

    pthread_mutex_lock(&(this->mutex_can_install));
    if(m_can_install)
    {
        this->close();
    }
    if(m_percent_update)
    {
        m_progress->setText(std::string(StringUtils::toString(addons->getDownloadState()) + "\% downloaded").c_str());
    }
    pthread_mutex_unlock(&(mutex_can_install));
}

// ------------------------------------------------------------------------------------------------------
void AddonsLoading::close()
{

    GUIEngine::Screen* curr_screen = GUIEngine::getCurrentScreen();
	pthread_mutex_lock(&(((AddonsScreen*)curr_screen)->mutex));
	((AddonsScreen*)curr_screen)->can_load_list = true;
	pthread_mutex_unlock(&(((AddonsScreen*)curr_screen)->mutex));
    dismiss();
}
// ------------------------------------------------------------------------------------------------------

void * AddonsLoading::startInstall(void* pthis)
{
    AddonsLoading * obj = (AddonsLoading*)pthis;
    if(obj->addons->IsInstalled() == "yes")
    {
    std::cout << obj->addons->IsInstalled() << std::endl;
        obj->addons->UnInstall();
    }
    else
    {
        obj->addons->Install();
    }
    pthread_mutex_lock(&(obj->mutex_can_install));
    obj->m_can_install = true;
    obj->m_percent_update = false;
    pthread_mutex_unlock(&(obj->mutex_can_install));
    return NULL;
}
// ------------------------------------------------------------------------------------------------------

void * AddonsLoading::downloadIcon(void* pthis)
{
/*
    AddonsLoading * obj = (AddonsLoading*)pthis;
    download("icon/" + obj->addons->GetIcon(), obj->addons->GetName() + ".png");
    obj->icon->setImage(std::string(file_manager->getConfigDir() + "/" +  obj->addons->GetName() + ".png").c_str(), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    obj->icon->setImage(std::string(file_manager->getConfigDir() + "/" +  obj->addons->GetName() + ".png").c_str(), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    obj->icon->x = 0;
    obj->icon->y = 0;
    obj->icon->w = obj->m_area.getWidth()/2;
    obj->icon->h = obj->m_area.getHeight();
    obj->icon->setParent(obj->m_irrlicht_window);
    obj->m_children.push_back(obj->icon);
    obj->icon->add();*/
    return NULL;
}
#endif
