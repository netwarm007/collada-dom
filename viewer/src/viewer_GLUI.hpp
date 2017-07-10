/*
 * Copyright 2006 Sony Computer Entertainment Inc.
 *
 * Licensed under the MIT Open Source License, for details please see license.txt or the website
 * http://www.opensource.org/licenses/mit-license.php
 *
 */
#include "viewer_base.pch.h"

//EXPERIMENTAL
#include "../external-libs/GLUI/trunk/include/GL/glui.h"

COLLADA_(namespace)
{
	static void UI_callback(GLUI_Control*);

	static struct UI : protected GLUI //SINGLETON
	{
	public: 
		
		using GLUI::show; 
		using GLUI::hide;
		using GLUI::activate_control;

		GLUI_CommandLine *site;

		void Init(int COLLADA_viewer)
		{	
			#ifdef NDEBUG
			#error Discuss merging GLUI_SUBWINDOW/GLUI_SUBWINDOW_X.
			#endif
			//HACK: Emulating create_glui_subwindow.
			init("UI",GLUI_SUBWINDOW|GLUI_SUBWINDOW_TOP,0,0,COLLADA_viewer);
			main_panel->set_int_val(GLUI_PANEL_EMBOSSED);
			link_this_to_parent_last(&GLUI_Master.gluis);

			set_main_gfx_window(COLLADA_viewer);	
	
			site = new GLUI_CommandLine(this,"",0,0,UI_callback);
		}

		void Callback(GLUI_Control *c)
		{
			if(c==site)
			{
				//Timing issues.
				//return site->scroll_history(-1);		
				COLLADA_viewer_go = site->get_text();
				return;		
			}
		}

	}UI;

	static void UI_callback(GLUI_Control *c){ UI.Callback(c); }
}