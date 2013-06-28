

#include "screen.h"
#include "view.h"
#include "radar.h"
#include "walker.h"
#include "smooth.h"
#include "input.h"
#include "util.h"
#include "level_data.h"

/* Changelog
 * 	8/8/02: Zardus: added scrolling-by-minimap
 * 		Zardus: added scrolling-by-keyboard
 */

#define OK 4 //this function was successful, continue normal operation

#include <string>
using namespace std;
#include <stdlib.h>
#define MINIMUM_TIME 0


#define S_LEFT 1
#define S_RIGHT 245
#define S_UP 1
#define S_DOWN 188

#define VERSION_NUM (char) 8 // save scenario type info
#define SCROLLSIZE 8

#define NUM_BACKGROUNDS PIX_MAX

#define PIX_LEFT   (S_RIGHT+18)
#define PIX_TOP    (S_UP+79)
#define PIX_OVER   4
//#define PIX_DOWN   ((PIX_MAX/PIX_OVER)+1)
#define PIX_DOWN   4
#define PIX_RIGHT  (PIX_LEFT+(PIX_OVER*GRID_SIZE))
#define PIX_BOTTOM (PIX_TOP+(PIX_DOWN*GRID_SIZE))

#define L_D(x) ((S_UP+7)+8*x)
#define L_W(x) (x*8 + 9)
#define L_H(x) (x*8)


int toInt(const std::string& s);

bool yes_or_no_prompt(const char* title, const char* message, bool default_value);
void popup_dialog(const char* title, const char* message);
void timed_dialog(const char* message, float delay_seconds = 3.0f);

enum ModeEnum {TERRAIN, OBJECT, SELECT};

void set_screen_pos(screen *myscreen, Sint32 x, Sint32 y);
walker * some_hit(Sint32 x, Sint32 y, walker  *ob, LevelData* data);
char get_random_matching_tile(Sint32 whatback);

Sint32 display_panel(screen* myscreen, LevelData* level, ModeEnum mode);
void info_box(walker  *target, screen * myscreen);
void set_facing(walker *target, screen *myscreen);
void set_name(walker  *target, screen * myscreen);
void scenario_options(screen * myscreen);

extern screen *myscreen;  // global for scen?

// Zardus: our prefs object from view.cpp
extern options * theprefs;

extern Sint32 *mymouse;
Uint8 *mykeyboard;
//scenario *myscen = new scenario;
Uint32 currentlevel = 1;
char scen_name[10];
char grid_name[10];

unsigned char scenpalette[768];
Sint32 backcount=0, forecount = 0;
Sint32 myorder = ORDER_LIVING;
char currentteam = 0;
Sint32 event = 1;  // need to redraw?
Sint32 campaignchanged = 0;  // has campaign changed?
Sint32 levelchanged = 0;  // has level changed?
Sint32 cyclemode = 1;      // for color cycling
Sint32 grid_aligned = 1;  // aligned by grid, default is on
//buffers: PORT: changed start_time to start_time_s to avoid conflict with
//input.cpp
Sint32 start_time_s; // for timer ops


Sint32 backgrounds[] = {
                         PIX_GRASS1, PIX_GRASS2, PIX_GRASS_DARK_1, PIX_GRASS_DARK_2,
                         //PIX_GRASS_DARK_B1, PIX_GRASS_DARK_BR, PIX_GRASS_DARK_R1, PIX_GRASS_DARK_R2,
                         PIX_BOULDER_1, PIX_GRASS_DARK_LL, PIX_GRASS_DARK_UR, PIX_GRASS_RUBBLE,

                         PIX_GRASS_LIGHT_LEFT_TOP, PIX_GRASS_LIGHT_1,
                         PIX_GRASS_LIGHT_RIGHT_TOP, PIX_WATER1,

                         PIX_WATERGRASS_U, PIX_WATERGRASS_D,
                         PIX_WATERGRASS_L, PIX_WATERGRASS_R,

                         PIX_DIRTGRASS_UR1, PIX_DIRT_1, PIX_DIRT_1, PIX_DIRTGRASS_LL1,
                         PIX_DIRTGRASS_LR1, PIX_DIRT_DARK_1, PIX_DIRT_DARK_1, PIX_DIRTGRASS_UL1,

                         PIX_DIRTGRASS_DARK_UR1, PIX_DIRTGRASS_DARK_LL1,
                         PIX_DIRTGRASS_DARK_LR1, PIX_DIRTGRASS_DARK_UL1,

                         PIX_JAGGED_GROUND_1, PIX_JAGGED_GROUND_2,
                         PIX_JAGGED_GROUND_3, PIX_JAGGED_GROUND_4,

                         PIX_PATH_1, PIX_PATH_2, PIX_PATH_3, PIX_PATH_4,
                         PIX_COBBLE_1, PIX_COBBLE_2, PIX_COBBLE_3, PIX_COBBLE_4,

                         //PIX_WALL2, PIX_WALL3, PIX_WALL4, PIX_WALL5,

                         PIX_WALL4, PIX_WALL_ARROW_GRASS,
                         PIX_WALL_ARROW_FLOOR, PIX_WALL_ARROW_GRASS_DARK,

                         PIX_WALL2, PIX_WALL3, PIX_H_WALL1, PIX_WALL_LL,

                         PIX_WALLSIDE_L, PIX_WALLSIDE_C, PIX_WALLSIDE_R, PIX_WALLSIDE1,

                         PIX_WALLSIDE_CRACK_C1, PIX_WALLSIDE_CRACK_C1,
                         PIX_TORCH1, PIX_VOID1,

                         //PIX_VOID1, PIX_FLOOR1, PIX_VOID1, PIX_VOID1,

                         PIX_CARPET_SMALL_TINY, PIX_CARPET_M2, PIX_PAVEMENT1, PIX_FLOOR1,

                         //PIX_PAVEMENT1, PIX_PAVEMENT2, PIX_PAVEMENT3, PIX_PAVEMENT3,
                         PIX_FLOOR_PAVEL, PIX_FLOOR_PAVEU, PIX_FLOOR_PAVED, PIX_FLOOR_PAVED,

                         PIX_WALL_LL,
                         PIX_WALLTOP_H,
                         PIX_PAVESTEPS1,
                         PIX_BRAZIER1,

                         PIX_PAVESTEPS2L, PIX_PAVESTEPS2, PIX_PAVESTEPS2R, PIX_PAVESTEPS1,
                         //PIX_TORCH1, PIX_TORCH2, PIX_TORCH3, PIX_TORCH3,

                         PIX_COLUMN1, PIX_COLUMN2, PIX_COLUMN2, PIX_COLUMN2,

                         PIX_TREE_T1, PIX_TREE_T1, PIX_TREE_T1, PIX_TREE_T1,
                         PIX_TREE_ML, PIX_TREE_M1, PIX_TREE_MT, PIX_TREE_MR,
                         PIX_TREE_B1, PIX_TREE_B1, PIX_TREE_B1, PIX_TREE_B1,

                         PIX_CLIFF_BACK_L, PIX_CLIFF_BACK_1, PIX_CLIFF_BACK_2, PIX_CLIFF_BACK_R,
                         PIX_CLIFF_LEFT, PIX_CLIFF_BOTTOM, PIX_CLIFF_TOP, PIX_CLIFF_RIGHT,
                         PIX_CLIFF_LEFT, PIX_CLIFF_TOP_L, PIX_CLIFF_TOP_R, PIX_CLIFF_RIGHT,
                     };

Sint32 rowsdown = 0;
Sint32 maxrows = ((sizeof(backgrounds)/4) / 4);
text *scentext;

bool save_level_and_map(screen* ascreen);

bool does_campaign_exist(const std::string& campaign_id)
{
    std::list<std::string> ls = list_campaigns();
    for(std::list<std::string>::iterator e = ls.begin(); e != ls.end(); e++)
    {
        if(campaign_id == *e)
            return true;
    }
    
    return false;
}

bool create_new_campaign(const std::string& campaign_id)
{
    // Delete the temp directory
    cleanup_unpacked_campaign();
    
    // Create the necessities in the temp directory
    create_dir(get_user_path() + "temp/");
    create_dir(get_user_path() + "temp/pix");
    create_dir(get_user_path() + "temp/scen");
    create_dir(get_user_path() + "temp/sound");
    create_new_pix(get_user_path() + "temp/icon.pix", 32, 32);
    create_new_campaign_descriptor(get_user_path() + "temp/campaign.yaml");
    create_new_scen_file(get_user_path() + "temp/scen/scen1.fss", "scen0001");
    // Create the map file (grid)
    create_new_map_pix(get_user_path() + "temp/pix/scen0001.pix", 40, 60);
    
    bool result = repack_campaign(campaign_id);
    if(!result)
        return result;
    
    cleanup_unpacked_campaign();
    return true;
}

void importCampaignPicker()
{
    
}

void shareCampaign(screen* myscreen)
{
    
}

void resmooth_map(LevelData* data)
{
    data->mysmoother.smooth();
}

void clear_terrain(LevelData* data)
{
    int w = data->grid.w;
    int h = data->grid.h;
    
    memset(data->grid.data, 1, w*h);
    resmooth_map(data);
    
    myscreen->viewob[0]->myradar->update();
}


class SimpleButton
{
public:
    SDL_Rect area;
    std::string label;
    bool remove_border;
    int color;
    bool centered;
    
    SimpleButton(const std::string& label, int x, int y, unsigned int w, unsigned int h, bool remove_border = false, int color = DARK_BLUE);
    
    void draw(screen* myscreen, text* mytext);
    bool contains(int x, int y) const;
};


SimpleButton::SimpleButton(const std::string& label, int x, int y, unsigned int w, unsigned int h, bool remove_border, int color)
    : label(label), remove_border(remove_border), color(color), centered(false)
{
    area.x = x;
    area.y = y;
    area.w = w;
    area.h = h;
}

void SimpleButton::draw(screen* myscreen, text* mytext)
{
    myscreen->draw_button(area.x, area.y, area.x + area.w - 1, area.y + area.h - 1, (remove_border? 0 : 1), 1);
    if(centered)
        mytext->write_xy(area.x + area.w/2 - 3*label.size(), area.y + area.h/2 - 2, label.c_str(), color, 1);
    else
        mytext->write_xy(area.x + 2, area.y + area.h/2 - 2, label.c_str(), color, 1);
}

bool SimpleButton::contains(int x, int y) const
{
    return (area.x <= x && x < area.x + area.w
            && area.y <= y && y < area.y + area.h);
}



bool prompt_for_string_block(text* mytext, const std::string& message, std::list<std::string>& result)
{
    int max_chars = 40;
    int max_lines = 8;
    
    int w = max_chars*6;
    int h = max_lines*10;
    int x = 160 - w/2;
    int y = 100 - h/2;
    
    // Background
    myscreen->draw_button(x - 5, y - 20, x + w + 10, y + h + 10, 1);
    
    unsigned char forecolor = DARK_BLUE;
    //unsigned char backcolor = 13;
    

	clear_keyboard();
	clear_key_press_event();
	clear_text_input_event();
	enable_keyrepeat();
    #ifdef USE_SDL2
    SDL_StartTextInput();
    #endif
    
    if(result.size() == 0)
        result.push_back("");
    
    std::list<std::string>::iterator s = result.begin();
    size_t cursor_pos = 0;
    size_t current_line = 0;
    
    bool cancel = false;
    bool done = false;
	while (!done)
	{
        
        // TODO: Need swipe controls for touch input
        // TODO: Needs a button for done
        if(query_key_press_event())
        {
            char c = query_key();
            clear_key_press_event();
            
            if (c == SDLK_RETURN)
            {
                std::string rest_of_line = s->substr(cursor_pos);
                s->erase(cursor_pos);
                s++;
                s = result.insert(s, rest_of_line);
                current_line++;
                cursor_pos = 0;
            }
            else if (c == SDLK_BACKSPACE)
            {
                // At the beginning of the line?
                if(cursor_pos == 0)
                {
                    // Deleting a line break
                    // Not at the first line?
                    if(result.size() > 1 && current_line > 0)
                    {
                        // Then move up into the previous line, copying the old line
                        current_line--;
                        std::string old_line = *s;
                        s = result.erase(s);
                        s--;
                        cursor_pos = s->size();
                        // Append the old line
                        *s += old_line;
                    }
                }
                else
                {
                    // Delete previous character
                    cursor_pos--;
                    s->erase(cursor_pos, 1);
                }
            }
        }
        
        if(mykeyboard[KEYSTATE_ESCAPE])
        {
            while(mykeyboard[KEYSTATE_ESCAPE])
                get_input_events(WAIT);
            
            done = true;
            break;
        }
        if(mykeyboard[KEYSTATE_DELETE])
        {
            if(cursor_pos < s->size())
                s->erase(cursor_pos, 1);
            
            while(mykeyboard[KEYSTATE_DELETE])
                get_input_events(WAIT);
        }
        if(mykeyboard[KEYSTATE_UP])
        {
            if(current_line > 0)
            {
                current_line--;
                s--;
                if(s->size() < cursor_pos)
                    cursor_pos = s->size();
            }
            
            while(mykeyboard[KEYSTATE_UP])
                get_input_events(WAIT);
        }
        if(mykeyboard[KEYSTATE_DOWN])
        {
            if(current_line+1 < result.size())
            {
                current_line++;
                s++;
            }
            else  // At the bottom already
                cursor_pos = s->size();
            
            if(s->size() < cursor_pos)
                cursor_pos = s->size();
            
            while(mykeyboard[KEYSTATE_DOWN])
                get_input_events(WAIT);
        }
        if(mykeyboard[KEYSTATE_LEFT])
        {
            if(cursor_pos > 0)
                cursor_pos--;
            else if(current_line > 0)
            {
                current_line--;
                s--;
                cursor_pos = s->size();
            }
            
            while(mykeyboard[KEYSTATE_LEFT])
                get_input_events(WAIT);
        }
        if(mykeyboard[KEYSTATE_RIGHT])
        {
            cursor_pos++;
            if(cursor_pos > s->size())
            {
                if(current_line+1 < result.size())
                {
                    // Go to next line
                    current_line++;
                    s++;
                    cursor_pos = 0;
                }
                else  // No next line
                    cursor_pos = s->size();
            }
            
            while(mykeyboard[KEYSTATE_RIGHT])
                get_input_events(WAIT);
        }
        
        if(query_text_input_event())
        {
            char* temptext = query_text_input();
            
            if(temptext != NULL)
            {
                s->insert(cursor_pos, temptext);
                cursor_pos += strlen(temptext);
            }
        }
		
        clear_text_input_event();
        myscreen->draw_button(x - 5, y - 20, x + w + 10, y + h + 10, 1);
        mytext->write_xy(x, y - 13, message.c_str(), BLACK, 1);
        myscreen->hor_line(x, y - 5, w, BLACK);
        
        int offset = 0;
        if(current_line > 3)
            offset = (current_line - 3)*10;
        int j = 0;
        for(std::list<std::string>::iterator e = result.begin(); e != result.end(); e++)
        {
            int ypos = y + j*10 - offset;
            if(y <= ypos && ypos <= y + h)
                mytext->write_xy(x, ypos, e->c_str(), forecolor, 1);
            j++;
        }
        myscreen->ver_line(x + cursor_pos*6, y + current_line*10 - 2 - offset, 10, RED);
		myscreen->buffer_to_screen(0, 0, 320, 200);
		
		myscreen->clearfontbuffer(x,y,w,h);
        
		// Wait for a key to be pressed ..
		while (!query_key_press_event() && !query_text_input_event())
			get_input_events(WAIT);
	}

    #ifdef USE_SDL2
    SDL_StopTextInput();
    #endif
	disable_keyrepeat();
	clear_keyboard();
    myscreen->clearfontbuffer();
    
    return !cancel;
}

bool prompt_for_string(text* mytext, const std::string& message, std::string& result)
{
    int max_chars = 29;
    
    int x = 58;
    int y = 60;
    int w = max_chars*6;
    int h = 10;
    
    myscreen->draw_button(x - 5, y - 20, x + w + 10, y + h + 10, 1);
    
    char* str = mytext->input_string_ex(x, y, max_chars, message.c_str(), result.c_str());
    myscreen->clearfontbuffer();
    
    if(str == NULL)
        return false;
    
    result = str;
    return true;
}

bool button_showing(const std::list<std::pair<SimpleButton*, std::set<SimpleButton*> > >& ls, SimpleButton* elem)
{
    for(std::list<std::pair<SimpleButton*, std::set<SimpleButton*> > >::const_iterator e = ls.begin(); e != ls.end(); e++)
    {
        const std::set<SimpleButton*>& s = e->second;
        if(s.find(elem) != s.end())
            return true;
    }
    return false;
}

// Wouldn't spatial partitioning be nice?  Too bad!
bool mouse_on_menus(int mx, int my, const set<SimpleButton*>& menu_buttons, const std::list<std::pair<SimpleButton*, std::set<SimpleButton*> > >& current_menu)
{
    for(set<SimpleButton*>::const_iterator e = menu_buttons.begin(); e != menu_buttons.end(); e++)
    {
        if((*e)->contains(mx, my))
            return true;
    }
    
    for(std::list<std::pair<SimpleButton*, std::set<SimpleButton*> > >::const_iterator e = current_menu.begin(); e != current_menu.end(); e++)
    {
        const set<SimpleButton*>& s = e->second;
        for(set<SimpleButton*>::const_iterator f = s.begin(); f != s.end(); f++)
        {
            if((*f)->contains(mx, my))
                return true;
        }
    }
    
    return false;
}

bool activate_sub_menu_button(int mx, int my, std::list<std::pair<SimpleButton*, std::set<SimpleButton*> > >& current_menu, SimpleButton& button, bool is_in_top_menu = false)
{
    // Make sure it is showing
    if(!button.contains(mx, my) || (!is_in_top_menu && !button_showing(current_menu, &button)))
        return false;
    
    while (mymouse[MOUSE_LEFT])
        get_input_events(WAIT);
    
    if(current_menu.size() > 0)
    {
        // Close menu if already open
        if(current_menu.back().first == &button)
        {
            current_menu.pop_back();
            myscreen->clearfontbuffer();
            return false;
        }
        
        myscreen->clearfontbuffer();
        
        // Remove all menus up to the parent
        while(current_menu.size() > 0)
        {
            std::set<SimpleButton*>& s = current_menu.back().second;
            if(s.find(&button) == s.end())
                current_menu.pop_back();
            else
                return true; // Open this menu
        }
    }
    
    // No parent!
    return is_in_top_menu;
}

bool activate_menu_choice(int mx, int my, std::list<std::pair<SimpleButton*, std::set<SimpleButton*> > >& current_menu, SimpleButton& button, bool is_in_top_menu = false)
{
    // Make sure it is showing
    if(!button.contains(mx, my) || (!is_in_top_menu && !button_showing(current_menu, &button)))
        return false;
    
    while (mymouse[MOUSE_LEFT])
        get_input_events(WAIT);
    
    // Close menu
    myscreen->clearfontbuffer();
    current_menu.clear();
    return true;
}

class LevelEditorData
{
public:
    CampaignData* campaign;
    LevelData* level;
    
    LevelEditorData();
    ~LevelEditorData();
    
    bool loadCampaign(const std::string& id);
    bool reloadCampaign();
    
    bool loadLevel(int id);
    bool reloadLevel();
    
    bool saveCampaignAs(const std::string& id);
    bool saveCampaign();
    
    bool saveLevelAs(int id);
    bool saveLevel();
};

LevelEditorData::LevelEditorData()
    : campaign(new CampaignData("org.openglad.gladiator")), level(new LevelData(1))
{
    
}

LevelEditorData::~LevelEditorData()
{
    delete campaign;
    delete level;
}

bool LevelEditorData::loadCampaign(const std::string& id)
{
    campaign->id = id;
    return campaign->load();
}

bool LevelEditorData::reloadCampaign()
{
    return campaign->load();
}


bool LevelEditorData::loadLevel(int id)
{
    level->id = id;
    return level->load();
}

bool LevelEditorData::reloadLevel()
{
    return level->load();
}


bool LevelEditorData::saveCampaignAs(const std::string& id)
{
    campaign->id = id;
    bool result = campaign->save();
    
    // Remount for consistency in PhysFS
    std::string old_campaign = get_mounted_campaign();
    unmount_campaign_package(old_campaign);
    mount_campaign_package(old_campaign);
    
    return result;
}

bool LevelEditorData::saveCampaign()
{
    bool result = campaign->save();
    
    // Remount for consistency in PhysFS
    std::string old_campaign = get_mounted_campaign();
    unmount_campaign_package(old_campaign);
    mount_campaign_package(old_campaign);
    
    return result;
}


bool LevelEditorData::saveLevelAs(int id)
{
    level->id = id;
    char buf[20];
    snprintf(buf, 20, "scen%d", id);
    level->grid_file = buf;
    
    std::string old_campaign = get_mounted_campaign();
    unpack_campaign(old_campaign);
    bool result = level->save();
    if(result)
        result = repack_campaign(old_campaign);
    cleanup_unpacked_campaign();
    
    // Remount for consistency in PhysFS
    unmount_campaign_package(old_campaign);
    mount_campaign_package(old_campaign);
    
    return result;
}

// Recursively get the connected levels
void get_connected_level_exits(int current_level, const std::list<int>& levels, std::set<int>& connected, std::list<std::string>& problems)
{
    // Stopping condition
    if(connected.find(current_level) != connected.end())
        return;
    
    connected.insert(current_level);
    
    // Load level
    LevelData d(current_level);
    if(!d.load())
    {
        char buf[40];
        snprintf(buf, 40, "Level %d failed to load.", current_level);
        problems.push_back(buf);
        return;
    }
    
    // Get the exits
    std::set<int> exits;
    oblink* fx = d.fxlist;
    while(fx != NULL)
    {
        if(fx->ob != NULL)
        {
            if(fx->ob->query_order() == ORDER_TREASURE && fx->ob->query_family() == FAMILY_EXIT && fx->ob->stats != NULL)
                exits.insert(fx->ob->stats->level);
        }
        
        fx = fx->next;
    }
    
    // With no exits, we'll progress directly to the next sequential level
    if(exits.size() == 0)
    {
        // Does the next sequential level exist?
        bool has_next = false;
        for(std::list<int>::const_iterator e = levels.begin(); e != levels.end(); e++)
        {
            if(current_level+1 == *e)
            {
                has_next = true;
                break;
            }
        }
        
        if(has_next)
        {
            exits.insert(current_level+1);
        }
        else
        {
            char buf[40];
            snprintf(buf, 40, "Level %d has no exits.", current_level);
            problems.push_back(buf);
            return;
        }
    }
    
    // Recursively call on exits
    for(std::set<int>::iterator e = exits.begin(); e != exits.end(); e++)
    {
        get_connected_level_exits(*e, levels, connected, problems);
    }
}

bool LevelEditorData::saveLevel()
{
    char buf[20];
    snprintf(buf, 20, "scen%d", level->id);
    level->grid_file = buf;
    
    std::string old_campaign = get_mounted_campaign();
    unpack_campaign(old_campaign);
    bool result = level->save();
    if(result)
        result = repack_campaign(get_mounted_campaign());
    cleanup_unpacked_campaign();
    
    // Remount for consistency in PhysFS
    unmount_campaign_package(old_campaign);
    mount_campaign_package(old_campaign);
    
    return result;
}

bool are_objects_outside_area(LevelData* level, int x, int y, int w, int h)
{
    x *= GRID_SIZE;
    y *= GRID_SIZE;
    w *= GRID_SIZE;
    h *= GRID_SIZE;
    
	oblink* here;

	here = level->oblist;
	while(here)
	{
		if(here->ob && (x > here->ob->xpos || here->ob->xpos >= x + w || y > here->ob->ypos || here->ob->ypos >= y + h))
		{
		    return true;
		}
		
		here = here->next;
	}

	here = level->fxlist;
	while(here)
	{
		if(here->ob && (x > here->ob->xpos || here->ob->xpos >= x + w || y > here->ob->ypos || here->ob->ypos >= y + h))
		{
		    return true;
		}
		
		here = here->next;
	}

	here = level->weaplist;
	while(here)
	{
		if(here->ob && (x > here->ob->xpos || here->ob->xpos >= x + w || y > here->ob->ypos || here->ob->ypos >= y + h))
		{
		    return true;
		}
		
		here = here->next;
	}
	
	return false;
}



Sint32 level_editor()
{
    static LevelEditorData data;
    
	Sint32 i,j;
	Sint32 extra;
	Sint32 windowx, windowy;
	walker  *newob;
	Sint32 mx, my;
	char mystring[80];
	short count;
    
    // Initialize palette for cycling
    load_and_set_palette("our.pal", scenpalette);
	
	scentext = new text(myscreen);
	
	// Set the un-set text to empty ..
	for (i=0; i < 60; i ++)
		myscreen->scentext[i][0] = 0;
    
    if(data.reloadCampaign())
        Log("Loaded campaign data successfully.\n");
    else
        Log("Failed to load campaign data.\n");
    
    std::string old_campaign = get_mounted_campaign();
    if(old_campaign.size() > 0)
        unmount_campaign_package(old_campaign);
    mount_campaign_package(data.campaign->id);
    

    std::list<int> levels = list_levels();
    if(levels.size() > 0)
    {
        if(data.loadLevel(levels.front()))
        {
            Log("Loaded level data successfully.\n");
        }
        else
            Log("Failed to load level data.\n");
    }
    else
        Log("Campaign has no valid levels!\n");

	myscreen->clearfontbuffer();
	event = 1;  // Redraw right away
	
	// Minimap
	radar myradar(myscreen->viewob[0], myscreen, 0);
	myradar.start(data.level);
	
	// GUI
	using std::set;
	using std::pair;
	using std::list;
	
	// File menu
	SimpleButton fileButton("File", 0, 0, 30, 15);
	SimpleButton fileCampaignButton("Campaign >", 0, fileButton.area.y + fileButton.area.h, 65, 15, true);
	SimpleButton fileLevelButton("Level >", 0, fileCampaignButton.area.y + fileCampaignButton.area.h, 65, 15, true);
	SimpleButton fileQuitButton("Exit", 0, fileLevelButton.area.y + fileLevelButton.area.h, 65, 15, true);
	
	// File > Campaign submenu
	SimpleButton fileCampaignImportButton("Import...", fileCampaignButton.area.x + fileCampaignButton.area.w, fileCampaignButton.area.y, 65, 15, true);
	SimpleButton fileCampaignShareButton("Share...", fileCampaignImportButton.area.x, fileCampaignImportButton.area.y + fileCampaignImportButton.area.h, 65, 15, true);
	SimpleButton fileCampaignNewButton("New", fileCampaignImportButton.area.x, fileCampaignShareButton.area.y + fileCampaignShareButton.area.h, 65, 15, true);
	SimpleButton fileCampaignLoadButton("Load...", fileCampaignImportButton.area.x, fileCampaignNewButton.area.y + fileCampaignNewButton.area.h, 65, 15, true);
	SimpleButton fileCampaignSaveButton("Save", fileCampaignImportButton.area.x, fileCampaignLoadButton.area.y + fileCampaignLoadButton.area.h, 65, 15, true);
	SimpleButton fileCampaignSaveAsButton("Save As...", fileCampaignImportButton.area.x, fileCampaignSaveButton.area.y + fileCampaignSaveButton.area.h, 65, 15, true);
	
	// File > Level submenu
	SimpleButton fileLevelNewButton("New", fileLevelButton.area.x + fileLevelButton.area.w, fileLevelButton.area.y, 65, 15, true);
	SimpleButton fileLevelLoadButton("Load...", fileLevelNewButton.area.x, fileLevelNewButton.area.y + fileLevelNewButton.area.h, 65, 15, true);
	SimpleButton fileLevelSaveButton("Save", fileLevelNewButton.area.x, fileLevelLoadButton.area.y + fileLevelLoadButton.area.h, 65, 15, true);
	SimpleButton fileLevelSaveAsButton("Save As...", fileLevelNewButton.area.x, fileLevelSaveButton.area.y + fileLevelSaveButton.area.h, 65, 15, true);
	
	// Campaign menu
	SimpleButton campaignButton("Campaign", fileButton.area.x + fileButton.area.w, 0, 55, 15);
	SimpleButton campaignInfoButton("Info...", campaignButton.area.x, campaignButton.area.y + campaignButton.area.h, 59, 15, true);
	SimpleButton campaignProfileButton("Profile >", campaignButton.area.x, campaignInfoButton.area.y + campaignInfoButton.area.h, 59, 15, true);
	SimpleButton campaignDetailsButton("Details >", campaignButton.area.x, campaignProfileButton.area.y + campaignProfileButton.area.h, 59, 15, true);
	SimpleButton campaignValidateButton("Validate", campaignButton.area.x, campaignDetailsButton.area.y + campaignDetailsButton.area.h, 59, 15, true);
	
	// Campaign > Profile submenu
	SimpleButton campaignProfileTitleButton("Title...", campaignProfileButton.area.x + campaignProfileButton.area.w, campaignProfileButton.area.y, 95, 15, true);
	SimpleButton campaignProfileDescriptionButton("Description...", campaignProfileTitleButton.area.x, campaignProfileTitleButton.area.y + campaignProfileTitleButton.area.h, 95, 15, true);
	SimpleButton campaignProfileIconButton("Icon...", campaignProfileTitleButton.area.x, campaignProfileDescriptionButton.area.y + campaignProfileDescriptionButton.area.h, 95, 15, true);
	SimpleButton campaignProfileAuthorsButton("Authors...", campaignProfileTitleButton.area.x, campaignProfileIconButton.area.y + campaignProfileIconButton.area.h, 95, 15, true);
	SimpleButton campaignProfileContributorsButton("Contributors...", campaignProfileTitleButton.area.x, campaignProfileAuthorsButton.area.y + campaignProfileAuthorsButton.area.h, 95, 15, true);
	
	// Campaign > Details submenu
	SimpleButton campaignDetailsVersionButton("Version...", campaignDetailsButton.area.x + campaignDetailsButton.area.w, campaignDetailsButton.area.y, 113, 15, true);
	SimpleButton campaignDetailsSuggestedPowerButton("Suggested power...", campaignDetailsVersionButton.area.x, campaignDetailsVersionButton.area.y + campaignDetailsVersionButton.area.h, 113, 15, true);
	SimpleButton campaignDetailsFirstLevelButton("First level...", campaignDetailsVersionButton.area.x, campaignDetailsSuggestedPowerButton.area.y + campaignDetailsSuggestedPowerButton.area.h, 113, 15, true);
	
	
	// Level menu
	SimpleButton levelButton("Level", campaignButton.area.x + campaignButton.area.w, 0, 40, 15);
	SimpleButton levelInfoButton("Info...", levelButton.area.x, levelButton.area.y + levelButton.area.h, 100, 15, true);
	SimpleButton levelTitleButton("Title...", levelButton.area.x, levelInfoButton.area.y + levelInfoButton.area.h, 100, 15, true);
	SimpleButton levelDescriptionButton("Description...", levelButton.area.x, levelTitleButton.area.y + levelTitleButton.area.h, 100, 15, true);
	SimpleButton levelMapSizeButton("Map size...", levelButton.area.x, levelDescriptionButton.area.y + levelDescriptionButton.area.h, 100, 15, true);
	SimpleButton levelResmoothButton("Resmooth terrain", levelButton.area.x, levelMapSizeButton.area.y + levelMapSizeButton.area.h, 100, 15, true);
	
	
	// Mode menu
	ModeEnum mode = TERRAIN;
	SimpleButton modeButton("Mode (Terrain)", levelButton.area.x + levelButton.area.w, 0, 90, 15);
	SimpleButton modeTerrainButton("Terrain", modeButton.area.x, modeButton.area.y + modeButton.area.h, 47, 15, true);
	SimpleButton modeObjectButton("Object", modeButton.area.x, modeTerrainButton.area.y + modeTerrainButton.area.h, 47, 15, true);
	SimpleButton modeSelectButton("Select", modeButton.area.x, modeObjectButton.area.y + modeObjectButton.area.h, 47, 15, true);
	
	// Top menu
	set<SimpleButton*> menu_buttons;
	menu_buttons.insert(&fileButton);
	menu_buttons.insert(&campaignButton);
	menu_buttons.insert(&levelButton);
	menu_buttons.insert(&modeButton);
	
	// The active menu buttons
	list<pair<SimpleButton*, set<SimpleButton*> > > current_menu;
	

	//******************************
	// Keyboard loop
	//******************************
    
    float cycletimer = 0.0f;
	grab_mouse();
	mykeyboard = query_keyboard();
	Uint32 last_ticks = SDL_GetTicks();
	Uint32 start_ticks = last_ticks;

	//
	// This is the main program loop
	//
	bool done = false;
	while(!done)
	{
		// Reset the timer count to zero ...
		reset_timer();

		if (myscreen->end)
		{
		    done = true;
			break;
		}

		//buffers: get keys and stuff
		get_input_events(POLL);

		// Zardus: COMMENT: I went through and replaced dumbcounts with get_input_events.
        
        if(query_key_press_event() && mykeyboard[KEYSTATE_ESCAPE])
        {
            if((!levelchanged && !campaignchanged)
                || yes_or_no_prompt("Exit", "Quit without saving?", false))
            {
                done = true;
                break;
            }
            
            myscreen->clearfontbuffer();
            event = 1;
            
            // Wait until release
            while (mykeyboard[KEYSTATE_ESCAPE])
                get_input_events(WAIT);
        }
        
		// Delete all with ^D
		if (mykeyboard[KEYSTATE_d] && mykeyboard[KEYSTATE_LCTRL])
		{
		    data.level->delete_objects();
			levelchanged = 1;
			event = 1;
		}

		// Change teams ..
		if (mykeyboard[KEYSTATE_0])
		{
			currentteam = 0;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_1])
		{
			currentteam = 1;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_2])
		{
			currentteam = 2;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_3])
		{
			currentteam = 3;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_4])
		{
			currentteam = 4;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_5])
		{
			currentteam = 5;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_6])
		{
			currentteam = 6;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_7])
		{
			currentteam = 7;
			event = 1;
		}

		// Toggle grid alignment
		if (mykeyboard[KEYSTATE_g])
		{
			grid_aligned = (grid_aligned+1)%3;
			event = 1;
			while (mykeyboard[KEYSTATE_g])
				//buffers: dumbcount++;
				get_input_events(WAIT);
		}

		if (mykeyboard[KEYSTATE_KP_MULTIPLY]) // options menu
		{
			release_mouse();
			//scenario_options(myscreen);
			grab_mouse();
			event = 1; // redraw screen
		}

		// Save scenario
		if(mykeyboard[KEYSTATE_s] && mykeyboard[KEYSTATE_LCTRL])
		{
		    bool saved = false;
		    if(levelchanged)
            {
                if(data.saveLevel())
                {
                    event = 1;
                    levelchanged = 0;
                    saved = true;
                }
                else
                {
                    timed_dialog("Failed to save level.");
                    event = 1;
                }
            }
            if(campaignchanged)
            {
                if(data.saveCampaign())
                {
                    event = 1;
                    campaignchanged = 0;
                    saved = true;
                }
                else
                {
                    timed_dialog("Failed to save campaign.");
                    event = 1;
                }
            }
            
            if(saved)
            {
                timed_dialog("Saved.");
                event = 1;
            }
            else if(!levelchanged && !campaignchanged)
            {
                timed_dialog("No changes to save.");
                event = 1;
            }
		}  // end of saving routines

		// Enter scenario text ..
		if (mykeyboard[KEYSTATE_t])
		{
#define TEXT_DOWN(x)  (14+((x)*7))
   #define TL 4
			//gotoxy(1, 1);
			myscreen->draw_button(0, 10, 200, 200, 2, 1);
			myscreen->buffer_to_screen(0, 0, 320, 200);
			scentext->write_xy(TL, TEXT_DOWN(0), "Enter new scenario text;", DARK_BLUE, 1);
			scentext->write_xy(TL, TEXT_DOWN(1), " PERIOD (.) alone to end.", DARK_BLUE, 1);
			scentext->write_xy(TL, TEXT_DOWN(2), "*--------*---------*---------*", DARK_BLUE, 1);
			myscreen->buffer_to_screen(0, 0, 320, 200);
			myscreen->scentextlines = 0;
			count = 2;
			for (i=0; i < 23; i++)
				if (strlen(myscreen->scentext[i]))
					scentext->write_xy(TL, TEXT_DOWN(i+3), myscreen->scentext[i], DARK_BLUE, 1);
			myscreen->buffer_to_screen(0, 0, 320, 200);
			extra = 1;
			for (i=0; i < 60; i++)
			{
				count++;
				mystring[0] = 0;
				if (! (count%26) )
				{
					myscreen->draw_box(TL, TEXT_DOWN(3), 196, 196, 27, 1, 1);
					myscreen->buffer_to_screen(0, 0, 320, 200);
					for (j=0; j < 23; j++)
					{
						count = j+(23*extra);
						if (count < 60)
							if (strlen(myscreen->scentext[count]))
								scentext->write_xy(TL, TEXT_DOWN(j+3), myscreen->scentext[count], DARK_BLUE, 1);
					}
					count = 3;
					extra++;
					myscreen->buffer_to_screen(0, 0, 320, 200);
				}
				char* new_text = scentext->input_string(TL, TEXT_DOWN(count), 30, myscreen->scentext[i]);
				if(new_text == NULL)
                    new_text = myscreen->scentext[i];
				strcpy(mystring, new_text);
				strcpy(myscreen->scentext[i], mystring);
				if (!strcmp(".", mystring)) // says end ..
				{
					i = 70;
					myscreen->draw_box(0, 10, 200, 200, 0, 1, 1);
					myscreen->buffer_to_screen(0, 0, 320, 200);
					myscreen->scentext[i][0] = 0;
					event = 1;
				}
				else
					myscreen->scentextlines++;
			}
			myscreen->draw_box(0, 10, 200, 200, 0, 1, 1);
			myscreen->buffer_to_screen(0, 0, 320, 200);
			event = 1;
		}

		// Display the scenario help..
		if (mykeyboard[KEYSTATE_SLASH] && (mykeyboard[KEYSTATE_RSHIFT] || mykeyboard[KEYSTATE_LSHIFT]))
		{
			//read_scenario(myscreen);
			myscreen->clearfontbuffer();
			event = 1;
		}

		// Change level of current guy being placed ..
		if (mykeyboard[KEYSTATE_RIGHTBRACKET])
		{
			currentlevel++;
			//while (mykeyboard[KEYSTATE_RIGHTBRACKET])
			//  dumbcount++;
			event = 1;
		}
		if (mykeyboard[KEYSTATE_LEFTBRACKET] && currentlevel > 1)
		{
			currentlevel--;
			//while (mykeyboard[KEYSTATE_LEFTBRACKET])
			//  dumbcount++;
			event = 1;
		}

		// Change between generator and living orders
		if (mykeyboard[KEYSTATE_o])        // this is letter o
		{
			if (myorder == ORDER_LIVING)
			{
				myorder = ORDER_GENERATOR;
				forecount = FAMILY_TENT;
			}
			else if (myorder == ORDER_GENERATOR)
				myorder = ORDER_SPECIAL;   // for placing team guys ..
			else if (myorder == ORDER_SPECIAL)
			{
				myorder = ORDER_TREASURE;
				forecount = FAMILY_DRUMSTICK;
			}
			else if (myorder == ORDER_TREASURE)
				myorder = ORDER_WEAPON;
			else if (myorder == ORDER_WEAPON)
				myorder = ORDER_LIVING;
			mode = OBJECT;
            modeButton.label = "Mode (Object)";
			event = 1; // change score panel
			while (mykeyboard[KEYSTATE_o])
				get_input_events(WAIT);
		}

		// Slide tile selector down ..
		if (mykeyboard[KEYSTATE_DOWN])
		{
			rowsdown++;
			event = 1;
			if (rowsdown >= maxrows)
				rowsdown -= maxrows;
			display_panel(myscreen, data.level, mode);
			while (mykeyboard[KEYSTATE_DOWN])
				get_input_events(WAIT);
		}

		// Slide tile selector up ..
		if (mykeyboard[KEYSTATE_UP])
		{
			rowsdown--;
			event = 1;
			if (rowsdown < 0)
				rowsdown += maxrows;
			if (rowsdown <0 || rowsdown >= maxrows) // bad case
				rowsdown = 0;
			display_panel(myscreen, data.level, mode);
			while (mykeyboard[KEYSTATE_UP])
				get_input_events(WAIT);
		}

		// Smooth current map, F5
		if (mykeyboard[KEYSTATE_F5])
		{
		    resmooth_map(data.level);
			while (mykeyboard[KEYSTATE_F5])
				get_input_events(WAIT);
			event = 1;
			levelchanged = 1;
		}

		// Change to new palette ..
		if (mykeyboard[KEYSTATE_F9])
		{
			load_and_set_palette("our.pal", scenpalette);
			while (mykeyboard[KEYSTATE_F9])
				get_input_events(WAIT);
		}

		// Mouse stuff ..
		mymouse = query_mouse();

		// Scroll the screen ..
		// Zardus: ADD: added scrolling by keyboard
		// Zardus: PORT: disabled mouse scrolling
		if ((mykeyboard[KEYSTATE_KP_8] || mykeyboard[KEYSTATE_KP_7] || mykeyboard[KEYSTATE_KP_9]) // || mymouse[MOUSE_Y]< 2)
		        && data.level->topy >= 0) // top of the screen
        {
            event = 1;
			data.level->add_draw_pos(0, -SCROLLSIZE);
        }
		if ((mykeyboard[KEYSTATE_KP_2] || mykeyboard[KEYSTATE_KP_1] || mykeyboard[KEYSTATE_KP_3]) // || mymouse[MOUSE_Y]> 198)
		        && data.level->topy <= (GRID_SIZE*data.level->grid.h)-18) // scroll down
        {
            event = 1;
			data.level->add_draw_pos(0, SCROLLSIZE);
        }
		if ((mykeyboard[KEYSTATE_KP_4] || mykeyboard[KEYSTATE_KP_7] || mykeyboard[KEYSTATE_KP_1]) // || mymouse[MOUSE_X]< 2)
		        && data.level->topx >= 0) // scroll left
        {
            event = 1;
			data.level->add_draw_pos(-SCROLLSIZE, 0);
        }
		if ((mykeyboard[KEYSTATE_KP_6] || mykeyboard[KEYSTATE_KP_3] || mykeyboard[KEYSTATE_KP_9]) // || mymouse[MOUSE_X] > 318)
		        && data.level->topx <= (GRID_SIZE*data.level->grid.w)-18) // scroll right
        {
            event = 1;
			data.level->add_draw_pos(SCROLLSIZE, 0);
        }

		if (mymouse[MOUSE_LEFT])       // put or remove the current guy
		{
			event = 1;
			mx = mymouse[MOUSE_X];
			my = mymouse[MOUSE_Y];
            
            // Clicking on menu items
            if(mouse_on_menus(mx, my, menu_buttons, current_menu))
            {
                // FILE
                if(activate_sub_menu_button(mx, my, current_menu, fileButton, true))
                {
                    set<SimpleButton*> s;
                    s.insert(&fileCampaignButton);
                    s.insert(&fileLevelButton);
                    s.insert(&fileQuitButton);
                    current_menu.push_back(std::make_pair(&fileButton, s));
                }
                // Campaign >
                else if(activate_sub_menu_button(mx, my, current_menu, fileCampaignButton))
                {
                    set<SimpleButton*> s;
                    s.insert(&fileCampaignImportButton);
                    s.insert(&fileCampaignShareButton);
                    s.insert(&fileCampaignNewButton);
                    s.insert(&fileCampaignLoadButton);
                    s.insert(&fileCampaignSaveButton);
                    s.insert(&fileCampaignSaveAsButton);
                    current_menu.push_back(std::make_pair(&fileCampaignButton, s));
                }
                else if(activate_menu_choice(mx, my, current_menu, fileCampaignImportButton))
                {
                    bool cancel = false;
                    if(levelchanged)
                    {
                        cancel = !yes_or_no_prompt("Import", "Discard unsaved level changes?", false);
                    }
                    
                    if(campaignchanged)
                    {
                        cancel = !yes_or_no_prompt("Import", "Discard unsaved campaign changes?", false);
                    }
                    
                    if(!cancel)
                    {
                        popup_dialog("Import Campaign", "Not yet implemented.");
                        importCampaignPicker();
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileCampaignShareButton))
                {
                    bool cancel = false;
                    if(levelchanged)
                    {
                        if(yes_or_no_prompt("Share", "Save level first?", false))
                        {
                            if(data.saveLevel())
                            {
                                timed_dialog("Level saved.");
                                event = 1;
                                levelchanged = 0;
                            }
                            else
                            {
                                timed_dialog("Save failed.");
                                event = 1;
                                
                                cancel = true;
                            }
                        }
                    }
                    
                    if(campaignchanged)
                    {
                        if(yes_or_no_prompt("Share", "Save campaign first?", false))
                        {
                            if(data.saveCampaign())
                            {
                                timed_dialog("Campaign saved.");
                                event = 1;
                                campaignchanged = 0;
                            }
                            else
                            {
                                timed_dialog("Save failed.");
                                event = 1;
                                
                                cancel = true;
                            }
                        }
                    }
                    
                    if(!cancel)
                    {
                        popup_dialog("Share Campaign", "Not yet implemented.");
                        shareCampaign(myscreen);
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileCampaignNewButton))
                {
                    // Confirm if unsaved
                    bool cancel = false;
                    if (levelchanged)
                    {
                        cancel = !yes_or_no_prompt("New Campaign", "Discard unsaved changes?", false);
                    }
                    
                    
                    if(!cancel)
                    {
                        // Ask for campaign ID
                        std::string campaign = "com.example.new_campaign";
                        if(prompt_for_string(scentext, "New Campaign", campaign))
                        {
                            // TODO: Check if campaign already exists and prompt the user to overwrite
                            if(does_campaign_exist(campaign) && !yes_or_no_prompt("Overwrite?", "Overwrite existing campaign with that ID?", false))
                            {
                                cancel = true;
                            }
                            
                            if(!cancel)
                            {
                                if(create_new_campaign(campaign))
                                {
                                    
                                    // Load campaign data for the editor
                                    if(data.loadCampaign(campaign))
                                    {
                                        // Mount new campaign
                                        unmount_campaign_package(get_mounted_campaign());
                                        mount_campaign_package(campaign);
                                        
                                        // Load first scenario
                                        levels = list_levels();
                                        
                                        if(levels.size() > 0)
                                        {
                                            data.loadLevel(levels.front());
                                            // Update minimap
                                            myradar.update(data.level);
                                            timed_dialog("Campaign created.");
                                            campaignchanged = 0;
                                            levelchanged = 0;
                                        }
                                        else
                                        {
                                            timed_dialog("Campaign has no scenarios!");
                                            event = 1;
                                        }
                                    }
                                    else
                                    {
                                        timed_dialog("Failed to load new campaign.");
                                        event = 1;
                                    }
                                }
                                else
                                {
                                    timed_dialog("Failed to create new campaign.");
                                    event = 1;
                                }
                            }
                        }
                        
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileCampaignLoadButton))
                {
                    // TODO: Use campaign picker here
                    std::string campaign = "com.example.new_campaign";
                    if(prompt_for_string(scentext, "Load Campaign", campaign))
                    {
                        if(data.loadCampaign(campaign))
                        {
                            unmount_campaign_package(get_mounted_campaign());
                            mount_campaign_package(campaign);
                            
                            timed_dialog("Campaign loaded.");
                            campaignchanged = 0;
                            event = 1;
                        }
                        else
                        {
                            timed_dialog("Failed to load campaign.");
                            event = 1;
                        }
                        
                        myradar.update(data.level);
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileCampaignSaveButton))
                {
                    if(data.saveCampaign())
                    {
                        timed_dialog("Campaign saved.");
                        campaignchanged = 0;
                        event = 1;
                    }
                    else
                    {
                        timed_dialog("Failed to save campaign.");
                        event = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileCampaignSaveAsButton))
                {
                    // TODO: Use campaign picker
                    std::string campaign = data.campaign->id;
                    if(prompt_for_string(scentext, "Save Campaign As", campaign))
                    {
                        if(data.saveCampaignAs(campaign))
                        {
                            timed_dialog("Campaign saved.");
                            campaignchanged = 0;
                            event = 1;
                        }
                        else
                        {
                            timed_dialog("Failed to save campaign.");
                            event = 1;
                        }
                    }
                }
                // Level >
                else if(activate_sub_menu_button(mx, my, current_menu, fileLevelButton))
                {
                    set<SimpleButton*> s;
                    s.insert(&fileLevelNewButton);
                    s.insert(&fileLevelLoadButton);
                    s.insert(&fileLevelSaveButton);
                    s.insert(&fileLevelSaveAsButton);
                    current_menu.push_back(std::make_pair(&fileLevelButton, s));
                }
                else if(activate_menu_choice(mx, my, current_menu, fileLevelNewButton))
                {
                    // New level
                    data.level->clear();
                    data.level->create_new_grid();
                    myradar.update(data.level);
                    levelchanged = 1;
                    event = 1;
                }
                else if(activate_menu_choice(mx, my, current_menu, fileLevelLoadButton))
                {
                    // Confirm if unsaved
                    bool cancel = false;
                    if (levelchanged)
                    {
                        cancel = !yes_or_no_prompt("Load Level", "Discard unsaved changes?", false);
                    }
                    
                    if(!cancel)
                    {
                        // TODO: Use level picker here
                        char buf[20];
                        snprintf(buf, 20, "%d", data.level->id);
                        
                        std::string level = buf;
                        if(prompt_for_string(scentext, "Load Level (num)", level))
                        {
                            if(data.loadLevel(atoi(level.c_str())))
                            {
                                timed_dialog("Level loaded.");
                                levelchanged = 0;
                                event = 1;
                            }
                            else
                            {
                                timed_dialog("Failed to load level.");
                                event = 1;
                            }
                            
                            myradar.update(data.level);
                            event = 1;
                        }
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileLevelSaveButton))
                {
                    if(data.saveLevel())
                    {
                        timed_dialog("Level saved.");
                        event = 1;
                        levelchanged = 0;
                    }
                    else
                    {
                        timed_dialog("Save failed.");
                        event = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileLevelSaveAsButton))
                {
                    // TODO: It would be nice to use the level browser for this
                    
                    char buf[20];
                    snprintf(buf, 20, "%d", data.level->id);
                    
                    std::string level = buf;
                    if(prompt_for_string(scentext, "Save Level (num)", level))
                    {
                        if(data.saveLevelAs(atoi(level.c_str())))
                        {
                            timed_dialog("Level saved.");
                            event = 1;
                            levelchanged = 0;
                        }
                        else
                        {
                            timed_dialog("Save failed.");
                            event = 1;
                        }
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, fileQuitButton))
                {
                    done = true;
                    break;
                }
                // CAMPAIGN
                else if(activate_sub_menu_button(mx, my, current_menu, campaignButton, true))
                {
                    set<SimpleButton*> s;
                    s.insert(&campaignInfoButton);
                    s.insert(&campaignProfileButton);
                    s.insert(&campaignDetailsButton);
                    s.insert(&campaignValidateButton);
                    current_menu.push_back(std::make_pair(&campaignButton, s));
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignInfoButton))
                {
                    char buf[512];
                    snprintf(buf, 512, "%s\nID: %s\nTitle: %s\nVersion: %s\nAuthors: %s\nContributors: %s\nSugg. Power: %d\nFirst level: %d", 
                                (campaignchanged? "(unsaved)" : ""), data.campaign->id.c_str(), data.campaign->title.c_str(), data.campaign->version.c_str(), data.campaign->authors.c_str(), data.campaign->contributors.c_str(), data.campaign->suggested_power, data.campaign->first_level);
                    popup_dialog("Campaign Info", buf);
                }
                // Profile >
                else if(activate_sub_menu_button(mx, my, current_menu, campaignProfileButton))
                {
                    set<SimpleButton*> s;
                    s.insert(&campaignProfileTitleButton);
                    s.insert(&campaignProfileDescriptionButton);
                    s.insert(&campaignProfileIconButton);
                    s.insert(&campaignProfileAuthorsButton);
                    s.insert(&campaignProfileContributorsButton);
                    current_menu.push_back(std::make_pair(&campaignProfileButton, s));
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignProfileTitleButton))
                {
                    std::string title = data.campaign->title;
                    if(prompt_for_string(scentext, "Campaign Title", title))
                    {
                        data.campaign->title = title;
                        campaignchanged = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignProfileDescriptionButton))
                {
                    std::list<std::string> desc = data.campaign->description;
                    if(prompt_for_string_block(scentext, "Campaign Description", desc))
                    {
                        data.campaign->description = desc;
                        campaignchanged = 1;
                    }
                    event = 1;
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignProfileIconButton))
                {
                    popup_dialog("Edit Icon", "Not yet implemented.");
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignProfileAuthorsButton))
                {
                    std::string authors = data.campaign->authors;
                    if(prompt_for_string(scentext, "Campaign Authors", authors))
                    {
                        data.campaign->authors = authors;
                        campaignchanged = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignProfileContributorsButton))
                {
                    std::string contributors = data.campaign->contributors;
                    if(prompt_for_string(scentext, "Campaign Contributors", contributors))
                    {
                        data.campaign->contributors = contributors;
                        campaignchanged = 1;
                    }
                }
                // Details >
                else if(activate_sub_menu_button(mx, my, current_menu, campaignDetailsButton))
                {
                    set<SimpleButton*> s;
                    s.insert(&campaignDetailsVersionButton);
                    s.insert(&campaignDetailsSuggestedPowerButton);
                    s.insert(&campaignDetailsFirstLevelButton);
                    current_menu.push_back(std::make_pair(&campaignDetailsButton, s));
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignDetailsVersionButton))
                {
                    std::string version = data.campaign->version;
                    if(prompt_for_string(scentext, "Campaign Version", version))
                    {
                        data.campaign->version = version;
                        campaignchanged = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignDetailsSuggestedPowerButton))
                {
                    char buf[20];
                    snprintf(buf, 20, "%d", data.campaign->suggested_power);
                    std::string power = buf;
                    if(prompt_for_string(scentext, "Suggested Power", power))
                    {
                        data.campaign->suggested_power = toInt(power);
                        campaignchanged = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignDetailsFirstLevelButton))
                {
                    char buf[20];
                    snprintf(buf, 20, "%d", data.campaign->first_level);
                    std::string level = buf;
                    if(prompt_for_string(scentext, "First Level", level))
                    {
                        data.campaign->first_level = toInt(level);
                        campaignchanged = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, campaignValidateButton))
                {
                    std::list<int> levels = list_levels();
                    std::set<int> connected;
                    std::list<std::string> problems;
                    
                    // Are the levels all connected to the first level?
                    int current_level = data.campaign->first_level;
                    get_connected_level_exits(current_level, levels, connected, problems);
                    
                    for(std::list<int>::iterator e = levels.begin(); e != levels.end(); e++)
                    {
                        if(connected.find(*e) == connected.end())
                        {
                            char buf[40];
                            snprintf(buf, 40, "Level %d is not connected.", *e);
                            problems.push_back(buf);
                        }
                    }
                    
                    // Get ready to show the user the problems
                    char buf[512];
                    if(problems.size() == 0)
                    {
                        snprintf(buf, 512, "No problems!");
                    }
                    else
                    {
                        // Only show the first 6 problems and "More problems..."
                        if(problems.size() > 6)
                        {
                            int num_over = problems.size() - 6;
                            while(problems.size() > 6)
                                problems.pop_back();
                            char buf[40];
                            snprintf(buf, 40, "%d more problems...", num_over);
                            problems.push_back(buf);
                        }
                        
                        // Put all the problems together for the printer
                        buf[0] = '\0';
                        for(std::list<std::string>::iterator e = problems.begin(); e != problems.end(); e++)
                        {
                            if(e->size() + strlen(buf) + 1 >= 512)
                                break;
                            strcat(buf, e->c_str());
                            strcat(buf, "\n");
                        }
                    }
                    
                    // Show user the problems
                    popup_dialog("Validate Campaign", buf);
                }
                // LEVEL
                else if(activate_sub_menu_button(mx, my, current_menu, levelButton, true))
                {
                    set<SimpleButton*> s;
                    s.insert(&levelInfoButton);
                    s.insert(&levelTitleButton);
                    s.insert(&levelDescriptionButton);
                    s.insert(&levelMapSizeButton);
                    s.insert(&levelResmoothButton);
                    current_menu.push_back(std::make_pair(&levelButton, s));
                }
                else if(activate_menu_choice(mx, my, current_menu, levelInfoButton))
                {
                    char buf[512];
                    snprintf(buf, 512, "%s\nID number: %d\nTitle: %s\nSize: %ux%u",
                             (levelchanged? "(unsaved)" : ""), data.level->id, data.level->title.c_str(), data.level->grid.w, data.level->grid.h);
                    popup_dialog("Level Info", buf);
                }
                else if(activate_menu_choice(mx, my, current_menu, levelTitleButton))
                {
                    std::string title = data.level->title;
                    if(prompt_for_string(scentext, "Level Title", title))
                    {
                        data.level->title = title;
                        levelchanged = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, levelDescriptionButton))
                {
                    std::list<std::string> desc = data.level->description;
                    if(prompt_for_string_block(scentext, "Level Description", desc))
                    {
                        data.level->description = desc;
                        levelchanged = 1;
                    }
                    event = 1;
                }
                else if(activate_menu_choice(mx, my, current_menu, levelMapSizeButton))
                {
                    // Using two prompts sequentially
                    
                    char buf[20];
                    snprintf(buf, 20, "%u", data.level->grid.w);
                    std::string width = buf;
                    snprintf(buf, 20, "%u", data.level->grid.h);
                    std::string height = buf;
                    
                    if(prompt_for_string(scentext, "Map Width", width))
                    {
                        int w = toInt(width);
                        int h;
                        
                        if(prompt_for_string(scentext, "Map Height", height))
                        {
                            h = toInt(height);
                            
                            // Validate here so we can tell the user
                            // Size is limited to one byte in the file format
                            if(w < 3 || h < 3 || w > 255 || h > 255)
                            {
                                char buf[200];
                                snprintf(buf, 200, "Can't resize grid to %dx%d\n", w, h);
                                if(w < 3)
                                    strcat(buf, "Width is too small.\n");
                                if(h < 3)
                                    strcat(buf, "Height is too small.\n");
                                if(w > 255)
                                    strcat(buf, "Width is too big (max 255).\n");
                                if(h > 255)
                                    strcat(buf, "Height is too big (max 255).\n");
                                
                                popup_dialog("Resize Map", buf);
                            }
                            else
                            {
                                if((w >= data.level->grid.w && h >= data.level->grid.h)
                                    || !are_objects_outside_area(data.level, 0, 0, w, h)
                                    || yes_or_no_prompt("Resize Map", "Delete objects outside of map?", false))
                                {
                                    // Now change it
                                    data.level->resize_grid(w, h);
                                    
                                    // Reset the minimap
                                    myradar.start(data.level);
                                    myradar.update(data.level);
                                    
                                    char buf[30];
                                    snprintf(buf, 30, "Resized map to %ux%u", data.level->grid.w, data.level->grid.h);
                                    timed_dialog(buf);
                                    event = 1;
                                    levelchanged = 1;
                                }
                                else
                                {
                                    timed_dialog("Resize canceled.");
                                    event = 1;
                                }
                            }
                        }
                        else
                        {
                            timed_dialog("Resize canceled.");
                            event = 1;
                        }
                    }
                    else
                    {
                        timed_dialog("Resize canceled.");
                        event = 1;
                    }
                }
                else if(activate_menu_choice(mx, my, current_menu, levelResmoothButton))
                {
                    resmooth_map(data.level);
                    levelchanged = 1;
                    event = 1;
                }
                // MODE
                else if(activate_sub_menu_button(mx, my, current_menu, modeButton, true))
                {
                    set<SimpleButton*> s;
                    s.insert(&modeTerrainButton);
                    s.insert(&modeObjectButton);
                    s.insert(&modeSelectButton);
                    current_menu.push_back(std::make_pair(&modeButton, s));
                }
                else if(activate_menu_choice(mx, my, current_menu, modeTerrainButton))
                {
                    mode = TERRAIN;
                    modeButton.label = "Mode (Terrain)";
                }
                else if(activate_menu_choice(mx, my, current_menu, modeObjectButton))
                {
                    mode = OBJECT;
                    modeButton.label = "Mode (Object)";
                }
                else if(activate_menu_choice(mx, my, current_menu, modeSelectButton))
                {
                    mode = SELECT;
                    modeButton.label = "Mode (Select)";
                }
            }
            else
            {
                // No menu click
                if(current_menu.size() > 0)
                {
                    myscreen->clearfontbuffer();  // Erase menu text that isn't there anymore
                    current_menu.clear();
                }
                
                // Zardus: ADD: can move map by clicking on minimap
                if (mx > myscreen->viewob[0]->endx - myradar.xview - 4
                        && my > myscreen->viewob[0]->endy - myradar.yview - 4
                        && mx < myscreen->viewob[0]->endx - 4 && my < myscreen->viewob[0]->endy - 4)
                {
                    mx -= myscreen->viewob[0]->endx - myradar.xview - 4;
                    my -= myscreen->viewob[0]->endy - myradar.yview - 4;

                    // Zardus: above set_screen_pos doesn't take into account that minimap scrolls too. This one does.
                    data.level->set_draw_pos(myradar.radarx * GRID_SIZE + mx * GRID_SIZE - 160,
                                    myradar.radary * GRID_SIZE + my * GRID_SIZE - 100);
                }
                else if ( (mx >= S_LEFT) && (mx <= S_RIGHT) &&
                          (my >= S_UP) && (my <= S_DOWN) )      // in the main window
                {
                    windowx = mymouse[MOUSE_X] + data.level->topx - myscreen->viewob[0]->xloc; // - S_LEFT
                    if (grid_aligned==1)
                        windowx -= (windowx%GRID_SIZE);
                    windowy = mymouse[MOUSE_Y] + data.level->topy - myscreen->viewob[0]->yloc; // - S_UP
                    if (grid_aligned==1)
                        windowy -= (windowy%GRID_SIZE);
                    if (mykeyboard[KEYSTATE_i]) // get info on current object
                    {
                        newob = data.level->add_ob(ORDER_LIVING, FAMILY_ELF);
                        newob->setxy(windowx, windowy);
                        if (some_hit(windowx, windowy, newob, data.level))
                            info_box(newob->collide_ob,myscreen);
                        data.level->remove_ob(newob,0);
                        continue;
                    }  // end of info mode
                    if (mykeyboard[KEYSTATE_f]) // set facing of current object
                    {
                        newob = data.level->add_ob(ORDER_LIVING, FAMILY_ELF);
                        newob->setxy(windowx, windowy);
                        if (some_hit(windowx, windowy, newob, data.level))
                        {
                            set_facing(newob->collide_ob,myscreen);
                            levelchanged = 1;
                        }
                        data.level->remove_ob(newob,0);
                        continue;
                    }  // end of set facing

                    if (mykeyboard[KEYSTATE_r]) // (re)name the current object
                    {
                        newob = data.level->add_ob(ORDER_LIVING, FAMILY_ELF);
                        newob->setxy(windowx, windowy);
                        if (some_hit(windowx, windowy, newob, data.level))
                        {
                            set_name(newob->collide_ob,myscreen);
                            levelchanged = 1;
                        }
                        data.level->remove_ob(newob,0);
                    }  // end of info mode
                    else if (mode == OBJECT)
                    {
                        levelchanged = 1;
                        newob = data.level->add_ob(myorder, forecount);
                        newob->setxy(windowx, windowy);
                        newob->team_num = currentteam;
                        newob->stats->level = currentlevel;
                        newob->dead = 0; // just in case
                        newob->collide_ob = 0;
                        if ( (grid_aligned==1) && some_hit(windowx, windowy, newob, data.level))
                        {
                            if (mykeyboard[KEYSTATE_LCTRL] &&    // are we holding the erase?
                                    newob->collide_ob )                    // and hit a guy?
                            {
                                data.level->remove_ob(newob->collide_ob,0);
                                while (mymouse[MOUSE_LEFT])
                                {
                                    mymouse = query_mouse();
                                }
                                levelchanged = 1;
                            } // end of deleting guy
                            if (newob)
                            {
                                data.level->remove_ob(newob,0);
                                newob = NULL;
                            }
                        }  // end of failure to put guy
                        else if (grid_aligned == 2)
                        {
                            newob->draw(myscreen->viewob[0]);
                            myscreen->buffer_to_screen(0, 0, 320, 200);
                            start_time_s = query_timer();
                            while ( mymouse[MOUSE_LEFT] && (query_timer()-start_time_s) < 36 )
                            {
                                mymouse = query_mouse();
                            }
                            levelchanged = 1;
                        }
                        if (mykeyboard[KEYSTATE_LCTRL] && newob)
                        {
                            data.level->remove_ob(newob,0);
                            newob = NULL;
                        }
                        //       while (mymouse[MOUSE_LEFT])
                        //         mymouse = query_mouse();
                    }  // end of putting a guy
                    if (mode == TERRAIN)
                    {
                        windowx /= GRID_SIZE;  // get the map position ..
                        windowy /= GRID_SIZE;
                        // Set to our current selection
                        data.level->grid.data[windowy*(data.level->grid.w)+windowx] = get_random_matching_tile(backcount);
                        levelchanged = 1;
                        if (!mykeyboard[KEYSTATE_LCTRL]) // smooth a few squares, if not control
                        {
                            for (i=windowx-1; i <= windowx+1; i++)
                                for (j=windowy-1; j <=windowy+1; j++)
                                    if (i >= 0 && i < data.level->grid.w &&
                                            j >= 0 && j < data.level->grid.h)
                                        data.level->mysmoother.smooth(i, j);
                        }
                        
                        //myscreen->viewob[0]->myradar->update(data.level);
                        myradar.update(data.level);
                    }  // end of setting grid square
                } // end of main window
                //    if ( (mx >= PIX_LEFT) && (mx <= PIX_RIGHT) &&
                //        (my >= PIX_TOP) && (my <= PIX_BOTTOM) ) // grid menu
                if (mx >= S_RIGHT && my >= PIX_TOP && my <= PIX_BOTTOM)
                {
                    //windowx = (mx - PIX_LEFT) / GRID_SIZE;
                    windowx = (mx-S_RIGHT) / GRID_SIZE;
                    windowy = (my - PIX_TOP) / GRID_SIZE;
                    backcount = backgrounds[ (windowx + ((windowy+rowsdown) * PIX_OVER))
                                             % (sizeof(backgrounds)/4)];
                    backcount %= NUM_BACKGROUNDS;
                    mode = TERRAIN;
                    modeButton.label = "Mode (Terrain)";
                } // end of background grid window
            }

		}      // end of left mouse button

		if (mymouse[MOUSE_RIGHT])      // cycle through things ...
		{
			event = 1;
			if (mode == OBJECT)
			{
				if (myorder == ORDER_LIVING)
					forecount = (forecount+1) % NUM_FAMILIES;
				else if (myorder == ORDER_TREASURE)
					forecount = (forecount+1) % (MAX_TREASURE+1);
				else if (myorder == ORDER_GENERATOR)
					forecount = (forecount+1) % 4;
				else if (myorder == ORDER_WEAPON)
					forecount = (forecount+1) % (FAMILY_DOOR+1); // use largest weapon
				else
					forecount = 0;
			} // end of if object mode
			if (mode == TERRAIN)
			{
				windowx = mymouse[MOUSE_X] + data.level->topx - myscreen->viewob[0]->xloc; // - S_LEFT
				windowx -= (windowx%GRID_SIZE);
				windowy = mymouse[MOUSE_Y] + data.level->topy - myscreen->viewob[0]->yloc; // - S_UP
				windowy -= (windowy%GRID_SIZE);
				windowx /= GRID_SIZE;
				windowy /= GRID_SIZE;
				backcount = data.level->grid.data[windowy*(data.level->grid.w)+windowx];
			}
			while (mymouse[MOUSE_RIGHT])
			{
				mymouse = query_mouse();
			}
		}

		// Now perform color cycling if selected
		if (cyclemode)
		{
		    cycletimer -= (start_ticks - last_ticks)/1000.0f;
		    if(cycletimer <= 0)
            {
                cycletimer = 0.5f;
                cycle_palette(scenpalette, WATER_START, WATER_END, 1);
                cycle_palette(scenpalette, ORANGE_START, ORANGE_END, 1);
            }
			event = 1;
		}
		
		// Redraw screen
		if (event)
		{
		    myscreen->clearscreen();
			data.level->draw(myscreen);
			myradar.draw(data.level);
			for(set<SimpleButton*>::iterator e = menu_buttons.begin(); e != menu_buttons.end(); e++)
                (*e)->draw(myscreen, scentext);
            for(list<pair<SimpleButton*, set<SimpleButton*> > >::iterator e = current_menu.begin(); e != current_menu.end(); e++)
            {
                set<SimpleButton*>& s = e->second;
                for(set<SimpleButton*>::iterator f = s.begin(); f != s.end(); f++)
                    (*f)->draw(myscreen, scentext);
            }
			display_panel(myscreen, data.level, mode);
			myscreen->refresh();
			
            event = 0;
		}
        
        SDL_Delay(10);
        
	    last_ticks = start_ticks;
	    start_ticks = SDL_GetTicks();

	}
	
	// Reset the screen position so it doesn't ruin the main menu
    data.level->set_draw_pos(0, 0);
    // Update the screen's position
    data.level->draw(myscreen);
    // Clear the background
    myscreen->clearscreen();
    
    unmount_campaign_package(data.campaign->id);
    mount_campaign_package(old_campaign);
    
    delete scentext;
    
	return OK;
}

Sint32 display_panel(screen* myscreen, LevelData* level, ModeEnum mode)
{
	char message[50];
	Sint32 i, j; // for loops
	//   static Sint32 family=-1, hitpoints=-1, score=-1, act=-1;
	static Sint32 numobs = myscreen->numobs;
	static Sint32 lm = 245;
	Sint32 curline = 0;
	Sint32 whichback;
	static char treasures[20][NUM_FAMILIES] =
	    { "BLOOD", "DRUMSTICK", "GOLD", "SILVER",
	      "MAGIC", "INVIS", "INVULN", "FLIGHT",
	      "EXIT", "TELEPORTER", "LIFE GEM", "KEY", "SPEED", "CC",
	    };
	static char weapons[20][NUM_FAMILIES] =
	    { "KNIFE", "ROCK", "ARROW", "FIREBALL",
	      "TREE", "METEOR", "SPRINKLE", "BONE",
	      "BLOOD", "BLOB", "FIRE ARROW", "LIGHTNING",
	      "GLOW", "WAVE 1", "WAVE 2", "WAVE 3",
	      "PROTECTION", "HAMMER", "DOOR",
	    };

	static char livings[NUM_FAMILIES][20] =
	    {  "SOLDIER", "ELF", "ARCHER", "MAGE",
	       "SKELETON", "CLERIC", "ELEMENTAL",
	       "FAERIE", "L SLIME", "S SLIME", "M SLIME",
	       "THIEF", "GHOST", "DRUID", "ORC",
	       "ORC CAPTAIN", "BARBARIAN", "ARCHMAGE",
	       "GOLEM", "G SKELETON", "TOWER1",
	    };

	// Hide the mouse ..
	//release_mouse();

	// Draw the bounding box
	myscreen->draw_button(lm-4, L_D(-1)+4, 315, L_D(7)-2, 1, 1);

	// Get team number ..
	sprintf(message, "%d:", currentteam);
	if (myorder == ORDER_LIVING)
		strcat(message, livings[forecount]);
	else if (myorder == ORDER_GENERATOR)
		switch (forecount)      // who are we?
		{
			case FAMILY_TENT:
				strcat(message, "TENT");
				break;
			case FAMILY_TOWER:
				strcat(message, "TOWER");
				break;
			case FAMILY_BONES:
				strcat(message, "BONEPILE");
				break;
			case FAMILY_TREEHOUSE:
				strcat(message, "TREEHOUSE");
				break;
			default:
				strcat(message, "GENERATOR");
				break;
		}
	else if (myorder == ORDER_SPECIAL)
		strcat(message, "PLAYER");
	else if (myorder == ORDER_TREASURE)
		strcat(message, treasures[forecount]);
	else if (myorder == ORDER_WEAPON)
		strcat(message, weapons[forecount]);
	else
		strcat(message, "UNKNOWN");
	scentext->write_xy(lm, L_D(curline++), message, DARK_BLUE, 1);

	// Level display
	sprintf(message, "LVL: %u", currentlevel);
	//myscreen->fastbox(lm,L_D(curline),55,7,27, 1);
	scentext->write_xy(lm, L_D(curline++), message, DARK_BLUE, 1);

	// Is grid alignment on?
	//myscreen->fastbox(lm, L_D(curline),65, 7, 27, 1);
	if (grid_aligned==1)
		scentext->write_xy(lm, L_D(curline++), "ALIGN: ON", DARK_BLUE, 1);
	else if (grid_aligned==2)
		scentext->write_xy(lm, L_D(curline++), "ALIGN: STACK", DARK_BLUE, 1);
	else
		scentext->write_xy(lm, L_D(curline++), "ALIGN: OFF", DARK_BLUE, 1);

	numobs = myscreen->numobs;
	//myscreen->fastbox(lm,L_D(curline),55,7,27, 1);
	sprintf(message, "OB: %d", numobs);
	scentext->write_xy(lm,L_D(curline++),message, DARK_BLUE, 1);

	// Show the background grid ..
	myscreen->putbuffer(lm+40, PIX_TOP-16, GRID_SIZE, GRID_SIZE,
	                    0, 0, 320, 200, myscreen->pixdata[backcount].data);

	//   rowsdown = (NUM_BACKGROUNDS / 4) + 1;
	//   rowsdown = 0; // hack for now
	for (i=0; i < PIX_OVER; i++)
	{
		for (j=0; j < 4; j++)
		{
			//myscreen->back[i]->draw( S_RIGHT+(i*8), S_UP+100);
			//myscreen->back[0]->draw(64, 64);
			whichback = (i+(j+rowsdown)*4) % (sizeof(backgrounds)/4);
			myscreen->putbuffer(S_RIGHT+i*GRID_SIZE, PIX_TOP+j*GRID_SIZE,
			                    GRID_SIZE, GRID_SIZE,
			                    0, 0, 320, 200,
			                    myscreen->pixdata[ backgrounds[whichback] ].data);
		}
	}
	myscreen->draw_box(S_RIGHT, PIX_TOP,
	                   S_RIGHT+4*GRID_SIZE, PIX_TOP+4*GRID_SIZE, 0, 0, 1);
	myscreen->buffer_to_screen(0, 0, 320, 200);
	// Restore the mouse
	//grab_mouse();

	return 1;
}


void set_screen_pos(screen *myscreen, Sint32 x, Sint32 y)
{
	myscreen->topx = x;
	myscreen->topy = y;
	event = 1;
}


void remove_first_ob(screen *master)
{
	oblink  *here;

	here = master->oblist;

	while (here)
	{
		if (here->ob)
		{
			delete here->ob;
			return;
		}
		else
			here = here->next;
	}
}

Sint32 save_map_file(char  * filename, screen *master)
{
	// File data in form:
	// <# of frames>      1 byte
	// <x size>                   1 byte
	// <y size>                   1 byte
	// <pixie data>               <x*y*frames> bytes

	char numframes, x, y;
	//  char  *newpic;
	string fullpath(filename);
	SDL_RWops  *outfile;

	// Create the full pathname for the pixie file
	fullpath += ".pix";

	lowercase (fullpath);

	if ( (outfile = open_write_file("temp/pix/", fullpath.c_str())) == NULL )
	{
		master->draw_button(30, 30, 220, 60, 1, 1);
		scentext->write_xy(32, 32, "Error in saving map file", DARK_BLUE, 1);
		scentext->write_xy(32, 42, fullpath.c_str(), DARK_BLUE, 1);
		scentext->write_xy(32, 52, "Press SPACE to continue", DARK_BLUE, 1);
		master->buffer_to_screen(0, 0, 320, 200);
		while (!mykeyboard[KEYSTATE_SPACE])
			get_input_events(WAIT);
		return 0;
	}

	x = master->grid.w;
	y = master->grid.h;
	numframes = 1;
	SDL_RWwrite(outfile, &numframes, 1, 1);
	SDL_RWwrite(outfile, &x, 1, 1);
	SDL_RWwrite(outfile, &y, 1, 1);

	SDL_RWwrite(outfile, master->grid.data, 1, (x*y));

	SDL_RWclose(outfile);        // Close the data file
	return 1;

} // End of map-saving routine

char get_random_matching_tile(Sint32 whatback)
{
	Sint32 i;

	i = random(4);  // max # of types of any particular ..

	switch (whatback)
	{
		case PIX_GRASS1:
			switch (i)
			{
				case 0:
					return PIX_GRASS1;
				case 1:
					return PIX_GRASS2;
				case 2:
					return PIX_GRASS3;
				case 3:
					return PIX_GRASS4;
				default:
					return PIX_GRASS1;
			}
			//break;
		case PIX_GRASS_DARK_1:
			switch (i)
			{
				case 0:
					return PIX_GRASS_DARK_1;
				case 1:
					return PIX_GRASS_DARK_2;
				case 2:
					return PIX_GRASS_DARK_3;
				case 3:
					return PIX_GRASS_DARK_4;
				default:
					return PIX_GRASS_DARK_1;
			}
			//break;
		case PIX_GRASS_DARK_B1:
		case PIX_GRASS_DARK_B2:
			switch (i)
			{
				case 0:
				case 1:
					return PIX_GRASS_DARK_B1;
				case 2:
				case 3:
				default:
					return PIX_GRASS_DARK_B2;
			}
			//break;
		case PIX_GRASS_DARK_R1:
		case PIX_GRASS_DARK_R2:
			switch (i)
			{
				case 0:
				case 1:
					return PIX_GRASS_DARK_R1;
				case 2:
				case 3:
				default:
					return PIX_GRASS_DARK_R2;
			}
			//break;
		case PIX_WATER1:
			switch (i)
			{
				case 0:
					return PIX_WATER1;
				case 1:
					return PIX_WATER2;
				case 2:
					return PIX_WATER3;
				default:
					return PIX_WATER1;
			}
			//break;
		case PIX_PAVEMENT1:
			switch (random(12))
			{
				case 0:
					return PIX_PAVEMENT1;
				case 1:
					return PIX_PAVEMENT2;
				case 2:
					return PIX_PAVEMENT3;
				default:
					return PIX_PAVEMENT1;
			}
			//break;
		case PIX_COBBLE_1:
			switch (random(i))
			{
				case 0:
					return PIX_COBBLE_1;
				case 1:
					return PIX_COBBLE_2;
				case 2:
					return PIX_COBBLE_3;
				case 3:
					return PIX_COBBLE_4;
				default:
					return PIX_COBBLE_1;
			}
			//break;
		case PIX_BOULDER_1:
			switch (random(i))
			{
				case 0:
					return PIX_BOULDER_1;
				case 1:
					return PIX_BOULDER_2;
				case 2:
					return PIX_BOULDER_3;
				case 3:
					return PIX_BOULDER_4;
				default:
					return PIX_BOULDER_1;
			}
			//break;
		case PIX_JAGGED_GROUND_1:
			switch (i)
			{
				case 0:
					return PIX_JAGGED_GROUND_1;
				case 1:
					return PIX_JAGGED_GROUND_2;
				case 2:
					return PIX_JAGGED_GROUND_3;
				case 3:
					return PIX_JAGGED_GROUND_4;
				default:
					return PIX_JAGGED_GROUND_1;
			}
		default:
			return whatback;
	}
}

// Copy of collide from obmap; used manually .. :(
Sint32 check_collide(Sint32 x,  Sint32 y,  Sint32 xsize,  Sint32 ysize,
                   Sint32 x2, Sint32 y2, Sint32 xsize2, Sint32 ysize2)
{
	if (x < x2)
	{
		if (y < y2)
		{
			if (x2 - x < xsize &&
			        y2 - y < ysize)
				return 1;
		}
		else // y >= y2
		{
			if (x2 - x < xsize &&
			        y - y2 < ysize2)
				return 1;
		}
	}
	else // x >= x2
	{
		if (y < y2)
		{
			if (x - x2 < xsize2 &&
			        y2 - y < ysize)
				return 1;
		}
		else // y >= y2
		{
			if (x - x2 < xsize2 &&
			        y - y2 < ysize2)
				return 1;
		}
	}
	return 0;
}

// The old-fashioned hit check ..
walker * some_hit(Sint32 x, Sint32 y, walker  *ob, LevelData* data)
{
	oblink  *here;

	here = data->oblist;

	while (here)
	{
		if (here->ob && here->ob != ob)
			if (check_collide(x, y, ob->sizex, ob->sizey,
			                  here->ob->xpos, here->ob->ypos,
			                  here->ob->sizex, here->ob->sizey) )
			{
				ob->collide_ob = here->ob;
				return here->ob;
			}
		here = here->next;
	}

	// Also check the fx list ..
	here = data->fxlist;
	while (here)
	{
		if (here->ob && here->ob != ob)
			if (check_collide(x, y, ob->sizex, ob->sizey,
			                  here->ob->xpos, here->ob->ypos,
			                  here->ob->sizex, here->ob->sizey) )
			{
				ob->collide_ob = here->ob;
				return here->ob;
			}
		here = here->next;
	}

	// Also check the weapons list ..
	here = data->weaplist;
	while (here)
	{
		if (here->ob && !here->ob->dead && here->ob != ob)
			if (check_collide(x, y, ob->sizex, ob->sizey,
			                  here->ob->xpos, here->ob->ypos,
			                  here->ob->sizex, here->ob->sizey) )
			{
				ob->collide_ob = here->ob;
				return here->ob;
			}
		here = here->next;
	}

	ob->collide_ob = NULL;
	return NULL;
}

// Display info about the target object ..
#define INFO_DOWN(x) (25+7*x)
void info_box(walker  *target,screen * myscreen)
{
	text *infotext = new text(myscreen);
	Sint32 linesdown = 0;
	Sint32 lm = 25+32;
	char message[80];
	treasure  *teleporter, *temp;

	static const char *orders[] =
	    { "LIVING", "WEAPON", "TREASURE", "GENERATOR", "FX", "SPECIAL", };
	static const char *livings[] =
	    { "SOLDIER", "ELF", "ARCHER", "MAGE",
	      "SKELETON", "CLERIC", "ELEMENTAL",
	      "FAERIE", "L-SLIME", "S-SLIME",
	      "M-SLIME", "THIEF", "GHOST",
	      "DRUID",
	    };
	static const char *treasures[] =
	    { "BLOODSTAIN", "DRUMSTICK: FOOD",
	      "GOLD BAR", "SILVER BAR",
	      "MAGIC POTION", "INVISIBILITY POTION",
	      "INVULNERABILITY POTION",
	      "FLIGHT POTION", "EXIT", "TELEPORTER",
	      "LIFE GEM", "KEY", "SPEED", "CC",
	    };

	release_mouse();
	myscreen->draw_button(20+32, 20, 220+32, 170, 1, 1);

	infotext->write_xy(lm, INFO_DOWN(linesdown++), "INFO TEXT", DARK_BLUE,1);
	linesdown++;

	if (strlen(target->stats->name)) // it has a name
	{
		sprintf(message, "Name    : %s", target->stats->name);
		infotext->write_xy(lm, INFO_DOWN(linesdown++), message, DARK_BLUE,1);
	}

	sprintf(message, "Order   : %s", orders[(int)target->query_order()] );
	infotext->write_xy(lm, INFO_DOWN(linesdown++), message, DARK_BLUE,1);

	if (target->query_order() == ORDER_LIVING)
		sprintf(message, "Family  : %s",
		        livings[(int)target->query_family()] );
	else if (target->query_order() == ORDER_TREASURE)
		sprintf(message, "Family  : %s",
		        treasures[(int)target->query_family()] );
	else
		sprintf(message, "Family  : %d", target->query_family());
	infotext->write_xy(lm, INFO_DOWN(linesdown++), message, DARK_BLUE,1);

	sprintf(message, "Team Num: %d", target->team_num);
	infotext->write_xy(lm, INFO_DOWN(linesdown++), message, DARK_BLUE, 1);

	sprintf(message, "Position: %dx%d (%dx%d)", target->xpos, target->ypos,
	        (target->xpos/GRID_SIZE), (target->ypos/GRID_SIZE) );
	infotext->write_xy(lm, INFO_DOWN(linesdown++), message, DARK_BLUE,1);

	if (target->query_order() == ORDER_TREASURE &&
	        target->query_family()== FAMILY_EXIT)
		sprintf(message, "Exits to: Level %d", target->stats->level);
	else if (target->query_order() == ORDER_TREASURE &&
	         target->query_family() == FAMILY_TELEPORTER)
	{
		sprintf(message, "Group # : %d", target->stats->level);
		infotext->write_xy(lm, INFO_DOWN(linesdown++), message, DARK_BLUE,1);
		temp = (treasure  *) target;
		teleporter = (treasure  *) temp->find_teleport_target();
		if (!teleporter || teleporter == target)
			infotext->write_xy(lm, INFO_DOWN(linesdown++), "Goes to : Itself!", DARK_BLUE,1);
		else
		{
			sprintf(message, "Goes to : %dx%d (%dx%d)", teleporter->xpos,
			        teleporter->ypos, teleporter->xpos/GRID_SIZE, teleporter->ypos/GRID_SIZE);
		}
	}
	else
		sprintf(message, "Level   : %d", target->stats->level);
	infotext->write_xy(lm, INFO_DOWN(linesdown++), message, DARK_BLUE,1);

	linesdown++;
	infotext->write_xy(lm, INFO_DOWN(linesdown++),
	                   "PRESS ESC TO EXIT", DARK_BLUE,1);

	myscreen->buffer_to_screen(0, 0, 320, 200);
	grab_mouse();

	// Wait for press and release of ESC
	while (!mykeyboard[KEYSTATE_ESCAPE])
		get_input_events(WAIT);
	while (mykeyboard[KEYSTATE_ESCAPE])
		get_input_events(WAIT);
}

// Set the stats->name value of a walker ..
void set_name(walker  *target, screen * master)
{
	char newname[11];
	char oldname[20];
	char buffer[200];

	//gotoxy(1,20);
	master->draw_button(30, 30, 220, 70, 1, 1);
	sprintf(buffer, "Renaming object");
	scentext->write_xy(32, 32, buffer, DARK_BLUE, 1);
	sprintf(buffer, "Enter '.' to not change.");
	scentext->write_xy(32, 42, buffer, DARK_BLUE, 1);

	if (strlen(target->stats->name))
	{
		sprintf(buffer, "Current name: %s", target->stats->name);
		strcpy(oldname, target->stats->name);
	}
	else
	{
		sprintf(buffer, "Current name: NOT SET");
		strcpy(oldname, "NOT SET");
	}
	scentext->write_xy(32, 52, buffer, DARK_BLUE, 1);
	scentext->write_xy(32, 62, "    New name:", DARK_BLUE, 1);

	master->buffer_to_screen(0, 0, 320, 200);

	// wait for key release
	while (mykeyboard[KEYSTATE_r])
		get_input_events(WAIT);
    
    char* new_text = scentext->input_string(115, 62, 9, oldname);
    if(new_text == NULL)
        new_text = oldname;
	strcpy(newname, new_text);
	newname[10] = 0;

	if (strcmp(newname, ".")) // didn't type '.'
		strcpy(target->stats->name, newname);

	info_box(target,master);

}

void scenario_options(screen *myscreen)
{
	static text opt_text(myscreen);
	Uint8 *opt_keys = query_keyboard();
	short lm, tm;
	char message[80];

	lm = 55;
	tm = 45;

#define OPT_LD(x) (short) (tm + (x*8) )
while (!opt_keys[KEYSTATE_ESCAPE])
        {


	myscreen->draw_button(lm-5, tm-5, 260, 160, 2, 1);

	opt_text.write_xy(lm, OPT_LD(0), "SCENARIO OPTIONS", DARK_BLUE, 1);

	if (myscreen->scenario_type & SCEN_TYPE_CAN_EXIT)
		opt_text.write_xy(lm, OPT_LD(2), "Can Always Exit (E)         : Yes", DARK_BLUE, 1);
	else
		opt_text.write_xy(lm, OPT_LD(2), "Can Always Exit (E)         : No ", DARK_BLUE, 1);

	if (myscreen->scenario_type & SCEN_TYPE_GEN_EXIT)
		opt_text.write_xy(lm, OPT_LD(3), " Kill Generators to Exit (G): Yes", DARK_BLUE, 1);
	else
		opt_text.write_xy(lm, OPT_LD(3), " Kill Generators to Exit (G): No ", DARK_BLUE, 1);

	if (myscreen->scenario_type & SCEN_TYPE_SAVE_ALL)
		opt_text.write_xy(lm, OPT_LD(4), " Must Save Named NPC's (N)  : Yes", DARK_BLUE, 1);
	else
		opt_text.write_xy(lm, OPT_LD(4), " Must Save Named NPC's (N)  : No ", DARK_BLUE, 1);

	sprintf(message, " Level Par Value (+,-)      : %d ", myscreen->par_value);
	opt_text.write_xy(lm, OPT_LD(5), message, DARK_BLUE, 1);


	myscreen->buffer_to_screen(0, 0, 320, 200);

	get_input_events(WAIT);
	if (opt_keys[KEYSTATE_e]) // toggle exit mode
	{
		if (myscreen->scenario_type & SCEN_TYPE_CAN_EXIT) // already set
			myscreen->scenario_type -= SCEN_TYPE_CAN_EXIT;
		else
			myscreen->scenario_type += SCEN_TYPE_CAN_EXIT;
	}
	if (opt_keys[KEYSTATE_g]) // toggle exit mode -- generators
	{
		if (myscreen->scenario_type & SCEN_TYPE_GEN_EXIT) // already set
			myscreen->scenario_type -= SCEN_TYPE_GEN_EXIT;
		else
			myscreen->scenario_type += SCEN_TYPE_GEN_EXIT;
	}
	if (opt_keys[KEYSTATE_n]) // toggle fail mode -- named guys
	{
		if (myscreen->scenario_type & SCEN_TYPE_SAVE_ALL) // already set
			myscreen->scenario_type -= SCEN_TYPE_SAVE_ALL;
		else
			myscreen->scenario_type += SCEN_TYPE_SAVE_ALL;
	}
	if (opt_keys[KEYSTATE_KP_MINUS]) // lower the par value
	{
		if (myscreen->par_value > 1)
			myscreen->par_value--;
	}
	if (opt_keys[KEYSTATE_KP_PLUS]) // raise the par value
	{
		myscreen->par_value++;
	}
}

while (opt_keys[KEYSTATE_ESCAPE])
	get_input_events(WAIT); // wait for key release

	myscreen->clearfontbuffer(lm-5, tm-5, 260-(lm-5), 160-(tm-5));
}

// Set an object's facing ..
void set_facing(walker *target, screen *myscreen)
{
	Uint8 *setkeys = query_keyboard();

	if (target)
		target = target;  // dummy code

	myscreen->draw_dialog(100, 50, 220, 170, "Set Facing");
	myscreen->buffer_to_screen(0, 0, 320, 200);

	while (setkeys[KEYSTATE_f])
		get_input_events(WAIT);

}