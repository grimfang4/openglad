//
// __interrupt routines library
// for Gladiator and associated files
//
// Created:  02-04-95
//
#include "input.h"
// Z's script: #include <dos.h>
// Z's script: #include <conio.h>
// Z's script: #include <i86.h>
#include <stdio.h>
#include <time.h>

unsigned long timer_count=0;
unsigned long timer_control=0;
unsigned long start_time=0;
unsigned long reset_value=0;
time_t starttime, endtime;
tm tmbuffer;
// Z: might need these two later
// dostime_t newtime;
// dosdate_t newdate;
long keyboard_grabbed=0;
long timer_grabbed=0;
long quit(long arg1);

unsigned short raw_key;
short key_press_event = 0;    // used to signed key-press
char key_list[MAXKEYS+JOYKEYS];

long mouse_state[MSTATE];
long mouse_buttons;

long joy_state[JSTATE];

// Zardus: PORT: the __stuff seems to freak it out: void (__far __interrupt *old_timer_isr)();
// same here: void (__far __interrupt *old_keyboard_isr)();


void change_time(unsigned long new_count)
{
}

// Zardus: PORT: g++ isn't a bit __stuff fan: void __interrupt increment_timer()
// Put this in for now:
void increment_timer()
{
// Theirs:
//  if (!(timer_control%DIVISOR))
//  {
//    old_timer_isr();
//    basecall++;
//  }
  timer_count++;
  timer_control++;
}

void grab_timer()
{
  if (timer_grabbed)
    return;
  timer_grabbed = 1;
// Zardus: PORT: seem to be dos things:  old_timer_isr = _dos_getvect(TIME_KEEPER_INT);
// Same here:  _dos_setvect(TIME_KEEPER_INT,increment_timer);

}

void release_timer()
{
  if (!timer_grabbed)
    return;
  timer_grabbed = 0;
// Zardus: dos stuff:  _dos_setvect(TIME_KEEPER_INT,old_timer_isr);
}

void reset_timer()
{
	reset_value = SDL_GetTicks();
}

long query_timer()
{
//  return timer_count;
	return (SDL_GetTicks() - reset_value) / 10;
}

unsigned long query_timer_control()
{
  return timer_control;
}


//
// Input routine (for handling all events and then setting the appropriate vars)
//

void get_input_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			// Key pressed or released:
			case SDL_KEYDOWN:
				key_list[event.key.keysym.sym] = 1;
				raw_key = event.key.keysym.sym;
				key_press_event = 1;
				break;
			case SDL_KEYUP:
				key_list[event.key.keysym.sym] = 0;
				break;

			// Mouse event
			case SDL_MOUSEMOTION:
				mouse_state[MOUSE_X] = event.motion.x;
				mouse_state[MOUSE_Y] = event.motion.y;
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
					mouse_state[MOUSE_LEFT] = 0;
				if (event.button.button == SDL_BUTTON_RIGHT)
					mouse_state[MOUSE_RIGHT] = 0;
				//mouse_state[MOUSE_LEFT] = SDL_BUTTON(SDL_BUTTON_LEFT);
				//printf ("LMB: %d",  SDL_BUTTON(SDL_BUTTON_LEFT));
				//mouse_state[MOUSE_RIGHT] = SDL_BUTTON(SDL_BUTTON_RIGHT);
				//printf ("RMB: %d",  SDL_BUTTON(SDL_BUTTON_RIGHT));
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
					mouse_state[MOUSE_LEFT] = 1;
				if (event.button.button == SDL_BUTTON_RIGHT)
					mouse_state[MOUSE_RIGHT] = 1;
				break;
			case SDL_QUIT:
				//buffers: PORT: the quit function is not avialiable to the scen app so we don't try to call it if we compile scen
				#ifndef SCEN
					quit(1);
				#endif
				break;
			default:
				break;
		}
	}
}


//
//Keyboard routines
//

void grab_keyboard()
{
}

void release_keyboard()
{
}

// Zardus: __stuff again. will replace temporarily: void __interrupt key_int()
void key_int()
{
  // above fetches a keypress from keyboard, and then
  // reenables both __interrupts and the keyboard control
  //remember cli/sti
}

int query_key()
{
  return raw_key;
}

//
// Set the keyboard array to all zeros, the
// virgin state, nothing depressed
//
void clear_keyboard()
{
	int i = 0;
	for (i = 0; i < MAXKEYS; i++)
		key_list[i] = 0;
}

char * query_keyboard()
{
	get_input_events();
	return key_list;
}

void wait_for_key(unsigned char somekey)
{
	// First wait for key press .. 
	while (!key_list[somekey])
		get_input_events();

	// And now for the key to be released ..
	while (key_list[somekey])
		get_input_events();
}

short query_key_press_event()
{

  return key_press_event;
}

void clear_key_press_event()
{
  key_press_event = 0;
}

short query_key_code(short code)
{
  return key_list[code];
}


//
// Mouse routines
//

void grab_mouse()
{
}

void release_mouse()
{
}

void reset_mouse()
{
}

long * query_mouse()
{
	// The mouse_state thing is set using get_input_events, though
	// it should probably get its own function
	get_input_events();
	return mouse_state;
}

long * query_joy()
{
  //joyasm();
  //joy_state[JOY_X] = (long) joyx;
  //joy_state[JOY_Y] = (long) joyy;
  
  //return joy_state;
}
