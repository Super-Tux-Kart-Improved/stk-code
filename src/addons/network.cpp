//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
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
#include <curl/curl.h>
#include <stdio.h>
#include <string>
#include "addons/network.hpp"
#include "config/user_config.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/addons_screen.hpp"

#include "io/file_manager.hpp"

// ------------------------------------------------------------------------------------------------------
bool download(std::string file, std::string save, int * progress_data)
{
	CURL *session = curl_easy_init();
	std::cout << "Downloading: " << std::string(UserConfigParams::m_server_addons.toString() + "/" + file) << std::endl;
	curl_easy_setopt(session, CURLOPT_URL, std::string(UserConfigParams::m_server_addons.toString() + "/" + file).c_str());
	FILE * fout;
	if(save != "")
	    fout = fopen(std::string(file_manager->getConfigDir() + std::string("/") + save).c_str(), "w");
    else
    	fout = fopen(std::string(file_manager->getConfigDir() + std::string("/") + file).c_str(), "w");
	
	
	//from and out
	curl_easy_setopt(session,  CURLOPT_WRITEDATA, fout);
	curl_easy_setopt(session,  CURLOPT_WRITEFUNCTION, fwrite);
	
	//init the mutex for the progress function
    pthread_mutex_init(&download_mutex, NULL);
    
	curl_easy_setopt(session,  CURLOPT_PROGRESSFUNCTION, &progressDownload);
	//needed, else, the progress function doesn't work
	curl_easy_setopt(session,  CURLOPT_NOPROGRESS, 0);
	
	//to update the progress bar
	curl_easy_setopt(session,  CURLOPT_PROGRESSDATA, progress_data);
	
	int succes = curl_easy_perform(session);
	
	//close the file where we downloaded the content
	fclose(fout);
	
	//stop curl
	curl_easy_cleanup(session);
	
	if(succes == 0)
	{
    	std::cout << "Download successfull" << std::endl;
    	return true;
	}
	else
	{
	    std::cout << "Download failed... check your network connexion" << std::endl;
	    return false;
    }
}
int progressDownload (void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    int progress = dlnow/dltotal*100;
    if(isnan(dlnow/dltotal*100))
        progress = 0;
    pthread_mutex_lock(&download_mutex);
    if(clientp != NULL)
    {
        int * progress_data = (int*)clientp;
        *progress_data = progress;
    }
    pthread_mutex_unlock(&download_mutex);
    std::cout << "Download progress: " << progress << "%" << std::endl;
    return 0;
}
#endif
