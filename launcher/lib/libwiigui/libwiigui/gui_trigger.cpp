/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_trigger.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include <wiidrc/wiidrc.h>

static int scrollDelay = 0;

/**
 * Constructor for the GuiTrigger class.
 */
GuiTrigger::GuiTrigger()
{
	chan = -1;
	memset(&wpaddata, 0, sizeof(WPADData));
	memset(&pad, 0, sizeof(PADData));
	wpad = &wpaddata;
	drc = NULL;
	drc_btns_d = 0;
	drc_btns_h = 0;
}

/**
 * Destructor for the GuiTrigger class.
 */
GuiTrigger::~GuiTrigger()
{
}

/**
 * Sets a simple trigger. Requires:
 * - Element is selected
 * - Trigger button is pressed
 */
void GuiTrigger::SetSimpleTrigger(s32 ch, u32 wiibtns, u16 gcbtns, u16 drcbtns)
{
	type = TRIGGER_SIMPLE;
	chan = ch;
	wpaddata.btns_d = wiibtns;
	pad.btns_d = gcbtns;
	drc_btns_d = drcbtns;
}

/**
 * Sets a held trigger. Requires:
 * - Element is selected
 * - Trigger button is pressed and held
 */
void GuiTrigger::SetHeldTrigger(s32 ch, u32 wiibtns, u16 gcbtns, u16 drcbtns)
{
	type = TRIGGER_HELD;
	chan = ch;
	wpaddata.btns_h = wiibtns;
	pad.btns_h = gcbtns;
	drc_btns_h = drcbtns;
}

/**
 * Sets a button trigger. Requires:
 * - Trigger button is pressed
 */
void GuiTrigger::SetButtonOnlyTrigger(s32 ch, u32 wiibtns, u16 gcbtns, u16 drcbtns)
{
	type = TRIGGER_BUTTON_ONLY;
	chan = ch;
	wpaddata.btns_d = wiibtns;
	pad.btns_d = gcbtns;
	drc_btns_d = drcbtns;
}

/**
 * Sets a button trigger. Requires:
 * - Trigger button is pressed
 * - Parent window is in focus
 */
void GuiTrigger::SetButtonOnlyInFocusTrigger(s32 ch, u32 wiibtns, u16 gcbtns, u16 drcbtns)
{
	type = TRIGGER_BUTTON_ONLY_IN_FOCUS;
	chan = ch;
	wpaddata.btns_d = wiibtns;
	pad.btns_d = gcbtns;
	drc_btns_d = drcbtns;
}

/****************************************************************************
 * WPAD_Stick
 *
 * Get X/Y value from Wii Joystick (classic, nunchuk) input
 ***************************************************************************/

s8 GuiTrigger::WPAD_Stick(u8 right, int axis)
{
	float mag = 0.0;
	float ang = 0.0;

	switch (wpad->exp.type)
	{
		case WPAD_EXP_NUNCHUK:
			if (right == 0)
			{
				mag = wpad->exp.nunchuk.js.mag;
				ang = wpad->exp.nunchuk.js.ang;
			}
			break;

		case WPAD_EXP_GUITARHERO3:
			if (right == 0)
			{
				mag = wpad->exp.gh3.js.mag;
				ang = wpad->exp.gh3.js.ang;
			}
			break;

		case WPAD_EXP_CLASSIC:
			if (right == 0)
			{
				mag = wpad->exp.classic.ljs.mag;
				ang = wpad->exp.classic.ljs.ang;
			}
			else
			{
				mag = wpad->exp.classic.rjs.mag;
				ang = wpad->exp.classic.rjs.ang;
			}
			break;

		default:
			break;
	}

	/* calculate x/y value (angle need to be converted into radian) */
	if (mag > 1.0) mag = 1.0;
	else if (mag < -1.0) mag = -1.0;
	double val;

	if(axis == 0) // x-axis
		val = mag * sin((PI * ang)/180.0f);
	else // y-axis
		val = mag * cos((PI * ang)/180.0f);

	return (s8)(val * 128.0f);
}

bool GuiTrigger::Left()
{
	u32 wiibtn = WPAD_BUTTON_LEFT;

	if (wpad->exp.type == EXP_CLASSIC)
		wiibtn |= WPAD_CLASSIC_BUTTON_LEFT;
	else if (wpad->exp.type == EXP_GUITAR_HERO_3)
		wiibtn |= WPAD_GUITAR_HERO_3_BUTTON_RED;

	// Check DRC input
	bool drc_btn_down = false;
	bool drc_btn_held = false;
	if (drc) {
		drc_btn_down = (drc->button & WIIDRC_BUTTON_LEFT) != 0;
		drc_btn_held = drc_btn_down || (drc->xAxisL < -PADCAL) || (drc->xAxisR < -PADCAL);
	}

	if(((wpad->btns_d | wpad->btns_h) & wiibtn)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_LEFT
			|| pad.stickX < -PADCAL
			|| WPAD_Stick(0,0) < -PADCAL
			|| drc_btn_held)
	{
		if(wpad->btns_d & wiibtn
			|| pad.btns_d & PAD_BUTTON_LEFT
			|| drc_btn_down)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Right()
{
	u32 wiibtn = WPAD_BUTTON_RIGHT;

	if (wpad->exp.type == EXP_CLASSIC)
		wiibtn |= WPAD_CLASSIC_BUTTON_RIGHT;
	else if (wpad->exp.type == EXP_GUITAR_HERO_3)
		wiibtn |= WPAD_GUITAR_HERO_3_BUTTON_YELLOW;

	// Check DRC input
	bool drc_btn_down = false;
	bool drc_btn_held = false;
	if (drc) {
		drc_btn_down = (drc->button & WIIDRC_BUTTON_RIGHT) != 0;
		drc_btn_held = drc_btn_down || (drc->xAxisL > PADCAL) || (drc->xAxisR > PADCAL);
	}

	if(((wpad->btns_d | wpad->btns_h) & wiibtn)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_RIGHT
			|| pad.stickX > PADCAL
			|| WPAD_Stick(0,0) > PADCAL
			|| drc_btn_held)
	{
		if(wpad->btns_d & wiibtn
			|| pad.btns_d & PAD_BUTTON_RIGHT
			|| drc_btn_down)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Up()
{
	u32 wiibtn = WPAD_BUTTON_UP;

	if (wpad->exp.type == EXP_CLASSIC)
		wiibtn |= WPAD_CLASSIC_BUTTON_UP;
	else if (wpad->exp.type == EXP_GUITAR_HERO_3)
		wiibtn |= WPAD_GUITAR_HERO_3_BUTTON_STRUM_UP;

	// Check DRC input
	bool drc_btn_down = false;
	bool drc_btn_held = false;
	if (drc) {
		drc_btn_down = (drc->button & WIIDRC_BUTTON_UP) != 0;
		drc_btn_held = drc_btn_down || (drc->yAxisL > PADCAL) || (drc->yAxisR > PADCAL);
	}

	if(((wpad->btns_d | wpad->btns_h) & wiibtn)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_UP
			|| pad.stickY > PADCAL
			|| WPAD_Stick(0,1) > PADCAL
			|| drc_btn_held)
	{
		if(wpad->btns_d & wiibtn
			|| pad.btns_d & PAD_BUTTON_UP
			|| drc_btn_down)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Down()
{
	u32 wiibtn = WPAD_BUTTON_DOWN;

	if (wpad->exp.type == EXP_CLASSIC)
		wiibtn |= WPAD_CLASSIC_BUTTON_DOWN;
	else if (wpad->exp.type == EXP_GUITAR_HERO_3)
		wiibtn |= WPAD_GUITAR_HERO_3_BUTTON_STRUM_DOWN;

	// Check DRC input
	bool drc_btn_down = false;
	bool drc_btn_held = false;
	if (drc) {
		drc_btn_down = (drc->button & WIIDRC_BUTTON_DOWN) != 0;
		drc_btn_held = drc_btn_down || (drc->yAxisL < -PADCAL) || (drc->yAxisR < -PADCAL);
	}

	if(((wpad->btns_d | wpad->btns_h) & wiibtn)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_DOWN
			|| pad.stickY < -PADCAL
			|| WPAD_Stick(0,1) < -PADCAL
			|| drc_btn_held)
	{
		if(wpad->btns_d & wiibtn
			|| pad.btns_d & PAD_BUTTON_DOWN
			|| drc_btn_down)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}