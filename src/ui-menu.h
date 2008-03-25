/*
 * Copyright (c) 2007 Pete Mack and others
 * This code released under the Gnu Public License. See www.fsf.org
 * for current GPL license details. Addition permission granted to
 * incorporate modifications in all Angband variants as defined in the
 * Angband variants FAQ. See rec.games.roguelike.angband for FAQ.
 */

#ifndef INCLUDED_UI_MENU_H
#define INCLUDED_UI_MENU_H

/* ============= Constants ============ */

/* Colors for interactive menus */
enum {
	CURS_UNKNOWN = 0,		/* Use gray; dark blue for cursor */
	CURS_KNOWN = 1			/* Use white; light blue for cursor */
};
static const byte curs_attrs[2][2] =
{
	{TERM_SLATE, TERM_BLUE},		/* Greyed row */
	{TERM_WHITE, TERM_L_BLUE}		/* Valid row */
};

/* Standard menu orderings */
extern const char default_choice[];		/* 1234567890A-Za-z */
extern const char lower_case[];			/* abc..z */
extern const char upper_case[];			/* ABC..Z */



/* Forward declare */
/* RISC OS already has a menu_item and menu_flags in system library */
#ifdef RISCOS
#define menu_item ang_menu_item
#define menu_flags ang_menu_flags
#endif

typedef struct menu_item menu_item;
typedef struct menu_type menu_type;
typedef struct menu_skin menu_skin;
typedef struct menu_iter menu_iter;


/* ================= PANEL ============ */
typedef struct panel_type panel_type;

/*
 * An event target bound to a particular screen area.
 * A Panel is a (rectangular) (sub)region, possibly containing a set of event
 * listeners, and responsible for maintaining its own internal layout and
 * dispatching.  A Panel has ownership of a Region, is a Container for 
 * Event Listeners, and an Event Target for mouse events).
 * Potential examples include: 
 *  - menu
 *  - window
 *  - map
 */
struct panel_type
{
	event_target target;
	void (*refresh)(menu_type *);
	region boundary;
};

/* ================== MENUS ================= */

/*
 * Performs an action on object with an optional environment label 
 * Member function of "menu_iter" VTAB
 */
typedef void (*action_f)(void *object, const char *name);

/* 
 * Displays a single row in a menu
 * Driver function for populating a single menu row.
 * Member function for "menu_iter" VTAB
 */
typedef void (*display_row_f) (menu_type *menu, int pos,
			       bool cursor, int row, int col, int width);

/* 
 * Driver function for displaying a page of rows.
 * Member function of "menu_skin" VTAB
 */
typedef void (*display_list_f)(menu_type *menu, int cursor, int *top, region *);


/* Primitive menu item with bound action */
typedef struct menu_action
{
	int id;			/* Object id used to define macros &c */
	const char *name;	/* Name of the action */
	action_f action;	/* Action to perform, if any */
	void *data;		/* Local environment for the action, if required */
} menu_action;


/* Decorated menu item with bound action */
struct menu_item
{
	menu_action act; /* Base type */
	char sel;        /* Character used for selection, if special-purpose */
	                 /* bindings are desired. */
	int flags;	 /* State of the menu item.  See menu flags below */
};


/* TODO: menu registry */
/*
  Together, these classes define the constant properties of
  the various menu classes.

  A menu consists of:
   - menu_iter, which describes how to handle the type of "list" that's 
     being displayed as a menu
   - a menu_skin, which describes the layout of the menu on the screen.
   - various bits and bobs of other data (e.g. the actual list of entries)
 */


/* Flags for menu appearance & behavior */
/* ==================================== */

/* Tags are associated with the view, not the element */
#define MN_REL_TAGS 0x0100 
/* No tags -- movement key and mouse browsing only */
#define MN_NO_TAGS  0x0200 
/* Tags to be generated by the display function */
#define MN_PVT_TAGS  0x0400 
/* Tag selections can be made regardless of the case of the key pressed. 
 * i.e. 'a' activates the line tagged 'A'. */
#define MN_CASELESS_TAGS 0x0800 

/* double tap (or keypress) for selection; single tap is cursor movement */
#define MN_DBL_TAP 0x1000 
/* Do not invoke the specified action; menu selection only */
#define MN_NO_ACT 0x2000 
/* No cursor movement */
#define MN_NO_CURSOR 0x8000 


/* Row flags in action_menu structure. */
#define MN_DISABLED   0x0100000 /* Neither action nor selection is permitted */
#define MN_GRAYED     0x0200000 /* Row is displayed with CURS_UNKNOWN colors */
#define MN_GREYED     0x0200000 /* Row is displayed with CURS_UNKNOWN colors */
#define MN_SELECTED   0x0400000 /* Row is currently selected */
#define MN_SELECTABLE 0x0800000 /* Row is permitted to be selected */
#define MN_HIDDEN     0x1000000 /* Row is hidden, but may be selected via */
                                /* key-binding.  */

/* Identifier for the type of menu layout to use */
typedef enum
{
	/* Skins */
	MN_SCROLL       = 0x0000, /* Ordinary scrollable single-column list */
	MN_COLUMNS      = 0x0002, /* multicolumn view */
	MN_NATIVE       = 0x0003, /* Not implemented -- OS menu */
	MN_KEY_ONLY     = 0x0004, /* No display */
	MN_USER	        = 0x0005  /* Anonymous, user defined. */
} skin_id;

/* Class functions for menu layout */
struct menu_skin 
{
	/* Identifier from the above list */
	skin_id id;
	/* Determines the cursor index given a (mouse) location */
	int (*get_cursor)(int row, int col, int n, int top, region *loc);
	/* Displays the current list of visible menu items */
	display_list_f display_list;
	/* Specifies the relative menu item given the state of the menu */
	char (*get_tag)(menu_type *menu, int pos);
	/* Superclass pointer. Not currently used */
	const menu_skin *super;
};


/* Identifiers for canned row iterator implementations */
typedef enum
{
	/* An empty slate */
	MN_NULL	= 0x0,

	/* A simple list of actions with an associated name and id.
	   An array of menu_action */
	MN_ACTIONS  = 0x1, 

	/* Slightly more sophisticated, a list of menu items that also 
	   allows per-item flags and a "selection" character to be specified.
	   An array of menu_item */
	MN_ITEMS   = 0x2,

	/* A list of strings to be selected from - no associated actions.
	   An array of const char * */
	MN_STRINGS = 0x3
} menu_iter_id;


/* Class functions for menu row-level accessor functions */
struct menu_iter 
{
	/* Type identifier from above set */
	menu_iter_id id;
	/* Optional selection tag function */
	char (*get_tag)(menu_type *menu, int oid);
	/* Optional validity checker.  All rows are assumed valid if not present. */
 	/* To support "hidden" items, it uses 3-level logic: 0 = no  1 = yes 2 = hide */
	int (*valid_row)(menu_type *menu, int oid);
	/* Displays a menu row at specified location */
	display_row_f display_row;
	/* Handler function called for selection or command key events */
	bool (*row_handler)(char cmd, void *db, int oid);
};


/* A menu defines either an action
 * or db row event
 */
struct menu_type
{
	/* menu inherits from panel */
	event_target target;
	void (*refresh)();
	region boundary;

	/* set of commands that may be performed on a menu item */
	const char *cmd_keys;

	/* Public variables */
	const char *title;
	const char *prompt;

	/* Keyboard shortcuts for menu selection */
	/* IMPORTANT: this cannot intersect with cmd_keys */
	const char *selections; 

	int flags;              /* Flags specifying the behavior of this menu. */
	int filter_count;       /* number of rows in current view */
	int *filter_list;       /* optional filter (view) of menu objects */

	int count;              /* number of rows in underlying data set */
	void *menu_data;  /* the data used to access rows. */

  	/* auxiliary browser help function */
	void (*browse_hook)(int oid, void *db, const region *loc);

	/* These are "protected" - not visible for canned menu classes, */
	/* The per-row functions  */
	const menu_iter *row_funcs;


	/* State variables for the menu */
	int cursor;             /* Currently selected row */
	int top;                /* Position in list for partial display */
	region active;          /* Subregion actually active for selection */

	/* helper functions for layout information. */
	const menu_skin *skin;  /* Defines menu appearance */
};


/* 
 * Select a row from a menu.
 * 
 * Arguments:
 *  object_list - optional ordered subset of menu OID.  If NULL, cursor is used for OID
 *  cursor - current (absolute) position in menu.  Modified on selection.
 *  loc - location to display the menu.
 * Return: A command key; cursor is set to the corresponding row.
 * (This is a stand-in for a menu event)
 * reserved commands are 0xff for selection and ESCAPE for escape.
 */
ui_event_data menu_select(menu_type *menu, int *cursor, int no_handle);

/* TODO: This belongs in the VTAB */
bool menu_layout(menu_type *menu, const region *loc);

/* accessor & utility functions */
void menu_set_filter(menu_type *menu, const int object_list[], int n);
void menu_release_filter(menu_type *menu);
void menu_set_id(menu_type *menu, int id);


/* Initialize a menu given a pointer to a skin and an iterator */
bool menu_init2(menu_type *menu, const menu_skin *skin,
	       const menu_iter *iter, const region *loc);

/* Initialise a menu block given skin and iterator IDs */
bool menu_init(menu_type *menu, skin_id skin, menu_iter_id iter, const region *loc);


void menu_refresh(menu_type *menu);

/* Menu VTAB registry */
const menu_iter *find_menu_iter(menu_iter_id iter_id);
const menu_skin *find_menu_skin(skin_id skin_id);

void add_menu_skin(const menu_skin *skin, skin_id id);
void add_menu_iter(const menu_iter *skin, menu_iter_id id);

#endif /* INCLUDED_UI_MENU_H */

