#include <gem.h>
#include "popup.h"

#if 0
/* Structure for passing menu data */
typedef struct _menu
{
	OBJECT *mn_tree;		/* Object tree of the menu */
	WORD mn_menu;			/* Parent of the menu items*/
	WORD mn_item;			/* Starting menu item */
	WORD mn_scroll;			/* scroll flag for the menu*/
	WORD mn_keystate;		/* Key State */
} MENU;


/* Structure for the Menu Settings */
typedef struct _mn_set
{
	LONG Display;			/* The display delay */
	LONG Drag;				/* The drag delay */
	LONG Delay;				/* The Arrow Delay */
	LONG Speed;				/* The scroll speed delay */
	WORD Height;			/* The menu scroll height */
} MN_SET;
#endif

static short obj_num_children(OBJECT *tree, short obj)
{
	short head = tree[obj].ob_head;
	short next = head;
	short count = 0;

	while (next != obj)
	{
		next = tree[next].ob_next;
		count++;
	}

	return count;
}

static short max(const short a, const short b)
{
    return (a > b ? a : b);
}

static short min(const short a, const short b)
{
	return (a < b ? a : b);
}

static MN_SET mn_set =
{
	0,			/* submenu display delay */
	0,			/* submenu drag display */
	0,			/* single click scroll delay */
	0,			/* continuous scroll delay */
	5			/* number of displayed items */
};

short do_popup(MENU *pm, OBJECT *dial, short originator)
{
	short x, y;
	short button, state;
	short exit_obj;
	OBJECT *popup = pm->mn_tree;
	OBJECT *o = &popup[pm->mn_menu];

	const short max_items = mn_set.height;
    short num_items = obj_num_children(popup, ROOT);
    short dsp_items = min(max_items, num_items);


	char upstr[] = " \x01 ";
	char dnstr[] = " \x02 ";

	wind_update(BEG_UPDATE);
	objc_offset(dial, originator, &x, &y);

	o->ob_x = (x + dial[originator].ob_width / 2) - o->ob_width / 2;

	short first;
	short last;
	char *first_str;
	char *last_str;

	first = o->ob_head;
	last = o->ob_tail;

	do
	{
		if (pm->mn_item != first)
		{
			first_str = popup[pm->mn_item - 1].ob_spec.free_string;
			popup[pm->mn_item - 1].ob_spec.free_string = upstr;
			popup->ob_head = pm->mn_item - 1;
		}
		else
		{
			popup->ob_head = pm->mn_item;				
		}
		

		if (pm->mn_item + dsp_items - 1 < last)
		{
			last_str = popup[pm->mn_item + dsp_items - 1].ob_spec.free_string;
			popup[pm->mn_item + dsp_items - 1].ob_spec.free_string = dnstr;
			
			popup->ob_tail = pm->mn_item + dsp_items - 1;
			popup[pm->mn_item + dsp_items - 1].ob_next = ROOT;
			popup->ob_height = popup[first].ob_height * dsp_items;
			o->ob_y = (y + dial[originator].ob_height / 2) - o->ob_height / 2;
		}
		else
		{
			popup->ob_tail = last;
		}
		
		
		objc_draw(popup, ROOT, MAX_DEPTH,
				o->ob_x, o->ob_y,
				o->ob_width, o->ob_height);		
		
		exit_obj = form_do(popup, ROOT) & 0x7fff;

		popup[exit_obj].ob_state &= ~OS_SELECTED;
		
		/*
		* undo object tree mods we did above
		*/
		if (pm->mn_item != first)
		{
			popup[pm->mn_item - 1].ob_spec.free_string = first_str;
			popup->ob_head = first;

			if (exit_obj == pm->mn_item - 1)
			{
				pm->mn_item--;
				continue;
			}
		}

		if (max_items < last)
		{
			popup[pm->mn_item + dsp_items - 1].ob_spec.free_string = last_str;
			popup->ob_tail = last;

			if (pm->mn_item + dsp_items - 1 != last)
				popup[pm->mn_item + dsp_items - 1].ob_next = pm->mn_item + dsp_items;
			
			if (exit_obj == pm->mn_item + dsp_items - 1)
			{
				pm->mn_item++;
				continue;
			}
		}

	} while (exit_obj == pm->mn_item - 1 || exit_obj == pm->mn_item + dsp_items - 1);

	wind_update(END_UPDATE);

	
	return exit_obj;
}
