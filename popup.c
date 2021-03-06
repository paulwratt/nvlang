#include <stdlib.h>
#include <gem.h>
#include "popup.h"

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

short do_popup(MENU *pm, short x, short y)
{
	short exit_obj;
	OBJECT *popup = pm->mn_tree;

	const short max_items = mn_set.height;
    short num_items = obj_num_children(popup, ROOT);
    short dsp_items = min(max_items, num_items);


	char upstr[] = " \x01 ";
	char dnstr[] = " \x02 ";

	wind_update(BEG_UPDATE);
	
	popup->ob_x = x;
	popup->ob_y = y;

	short first;
	short last;
	char *first_str = NULL;
	char *last_str = NULL;
	short mn_item_adjust = 0;

	first = popup->ob_head;
	last = popup->ob_tail;
	short up_arrow;
	short down_arrow;

	do
	{
		up_arrow = down_arrow = -1;

		pm->mn_item += mn_item_adjust;
		mn_item_adjust = 0;

		if (pm->mn_item != first)
		{
			/*
			 * pm->mn_item is not the first item to be displayed, but further down the menu.
			 * Thus we need to add an up arrow menu button. We save the menu text
			 * of the menu item immediately before pm->mn_item and temporarily
			 * replace it with our up arrow string
			 */

			first_str = popup[pm->mn_item - 1].ob_spec.free_string;
			popup[pm->mn_item - 1].ob_spec.free_string = upstr;

			up_arrow = pm->mn_item - 1;
			popup->ob_head = up_arrow;
		}
		else
		{
			popup->ob_head = pm->mn_item;				
		}
		

		if (pm->mn_item + dsp_items - 1 + (up_arrow > 0 ? 1 : 0) < last)
		{
			/*
			 * now take care about the lower end of the popup. If the last item displayed is not
			 * the last item of the menu, we temporarily replace its menu text with our down arrow
			 * text to enable menu scrolling
			 */
			down_arrow = pm->mn_item + dsp_items - 1 - (up_arrow > 0 ? 1 : 0);

			last_str = popup[down_arrow].ob_spec.free_string;
			popup[down_arrow].ob_spec.free_string = dnstr;
			
			popup->ob_tail = down_arrow;
			
			popup[down_arrow].ob_next = ROOT;
		}
		else
		{
			popup->ob_tail = last;
		}

		popup->ob_height = popup[first].ob_height * dsp_items;
		
		short ob_y = 0;
		
		/*
		 * adjust objects in menu
		 */
		for (int i = popup->ob_head; i <= popup->ob_tail; i++)
		{
			popup[i].ob_y = ob_y;
			ob_y += popup[i].ob_height;
		}

		/*
		 * draw menu
		 */
		objc_draw(popup, ROOT, MAX_DEPTH,
				  popup->ob_x, popup->ob_y,
				  popup->ob_width, popup->ob_height);		
		
		/*
		 * FIXME: must go away once scrolling has been sorted out
		 */
		exit_obj = form_do(popup, ROOT) & 0x7fff;

		popup[exit_obj].ob_state &= ~OS_SELECTED;
		
		/*
		* undo object tree mods we did above
		*/
		if (pm->mn_item != first)
		{
			popup[up_arrow].ob_spec.free_string = first_str;		/* restore saved menu text */
			popup->ob_head = first;

		}

		if (pm->mn_item + dsp_items - 1 < last)
		{
			/*
			 * restore saved menu text of down arrow object
			 */
			popup[down_arrow].ob_spec.free_string = last_str;
			popup->ob_tail = last;

			popup[down_arrow].ob_next = pm->mn_item + dsp_items - 1;
			
			//if (up_arrow > 0)
			//	mn_item_adjust += 1;
		}
		
		if (exit_obj == up_arrow)
		{
			if (exit_obj == first + 1)
			{
				mn_item_adjust -= 2;
			}
			else
			{
				mn_item_adjust -= 1;
			}
		}
		else if (exit_obj == down_arrow)
		{
			mn_item_adjust += 1;
		}
	} while (exit_obj == up_arrow || exit_obj == down_arrow);

	wind_update(END_UPDATE);

	return exit_obj;
}
