/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 * ResortMUD 4.0 improvements copyright (C) 1999 by Peter Keeler            *
 * ------------------------------------------------------------------------ *
 * Rogue Winds 2.0 copyright (C) 2000 by Heather Dunn                       *
 * ------------------------------------------------------------------------ *
 *			    Main mud header file			    *
 ****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>

#ifdef WIN32
#include <winsock.h>
#include <sys/types.h>
#pragma warning( disable: 4018 4244 4761)
#define index strchr
#define rindex strrchr
#pragma warning( disable : 4550 )   /* NJG */
#define vsnprintf _vsnprintf  /* NJG */
#define snprintf _snprintf /* NJG */
#define lstat stat   /* NJG */
#pragma comment( lib, "ws2_32.lib" )   /* NJG */
#pragma comment( lib, "winmm.lib" ) /* NJG */
#else
#include <unistd.h>
#ifndef SYSV
#include <sys/cdefs.h>
#else
#include <re_comp.h>
#endif
#include <sys/time.h>
#endif

typedef int ch_ret;
typedef int obj_ret;

#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

typedef int sh_int;  /* Stupid, but this is how they like it */

#ifndef __cplusplus
typedef unsigned char bool;
#endif

/*
 * Structure types.
 */
typedef struct alias_data ALIAS_DATA;
typedef struct affect_data AFFECT_DATA;
typedef struct area_data AREA_DATA;
typedef struct ban_data BAN_DATA;
typedef struct extracted_char_data EXTRACT_CHAR_DATA;
typedef struct char_data CHAR_DATA;
typedef struct hunt_hate_fear HHF_DATA;
typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct exit_data EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data HELP_DATA;
typedef struct menu_data MENU_DATA;
typedef struct mob_index_data MOB_INDEX_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct pc_data PC_DATA;
typedef struct reset_data RESET_DATA;
typedef struct map_index_data MAP_INDEX_DATA;   /* maps */
typedef struct map_data MAP_DATA;   /* maps */
typedef struct room_index_data ROOM_INDEX_DATA;
typedef struct shop_data SHOP_DATA;
typedef struct reserve_data RESERVE_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct hour_min_sec HOUR_MIN_SEC;
typedef struct weather_data WEATHER_DATA;
typedef struct neighbor_data NEIGHBOR_DATA;  /* FB */
typedef struct mob_prog_data MPROG_DATA;
typedef struct mob_prog_act_list MPROG_ACT_LIST;
typedef struct editor_data EDITOR_DATA;
typedef struct timer_data TIMER;
typedef struct godlist_data GOD_DATA;
typedef struct system_data SYSTEM_DATA;
typedef struct smaug_affect SMAUG_AFF;
typedef struct who_data WHO_DATA;
typedef struct skill_type SKILLTYPE;
typedef struct social_type SOCIALTYPE;
typedef struct cmd_type CMDTYPE;
typedef struct killed_data KILLED_DATA;
typedef struct deity_data DEITY_DATA;
typedef struct wizent WIZENT;
typedef struct ignore_data IGNORE_DATA;
typedef struct immortal_host IMMORTAL_HOST;
typedef struct extended_bitvector EXT_BV;
typedef struct lcnv_data LCNV_DATA;
typedef struct olc_data OLC_DATA;   /* Tagith */
typedef struct material_data MATERIAL_DATA;  /* Scion */
typedef struct trail_data TRAIL_DATA;  /* Scion */
typedef struct spell_data SPELL_DATA;  /* Scion */
typedef struct clan_member_data CLAN_MEMBER; /* Scion */
typedef struct clan_data CLAN_DATA; /* Scion */
typedef struct bit_data BIT_DATA;   /* Scion */
typedef struct moon_data MOON_DATA; /* Keolah */
typedef struct species_data SPECIES_DATA;
typedef struct nation_data NATION_DATA;
typedef struct part_data PART_DATA;
typedef struct recipe_data RECIPE_DATA;
typedef struct ingred_data INGRED_DATA;
typedef struct mat_shop_data MAT_SHOP;
typedef struct generic_mob_data GENERIC_MOB;
typedef struct explored_area_data EXPLORED_AREA;
typedef struct mutation_data MUT_DATA;
typedef struct player_mutation_data PMUT_DATA;
typedef struct city_data CITY_DATA;
typedef struct construct_data CONSTRUCT_DATA;
typedef struct channel_data CHANNEL_DATA;
typedef struct channel_member_data CHANNEL_MEMBER;
typedef struct arena_data ARENA_DATA;
typedef struct arena_member_data ARENA_MEMBER;
typedef struct bet_data BET_DATA;
typedef struct new_help_data NEW_HELP_DATA;
typedef struct book_data BOOK_DATA;

/*
 * Function types.
 */
typedef void DO_FUN args( ( CHAR_DATA * ch, char *argument ) );
typedef bool SPEC_FUN args( ( CHAR_DATA * ch ) );
typedef ch_ret SPELL_FUN args( ( int sn, int wil, CHAR_DATA * ch, void *vo ) );

#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

/* 32bit bitvector defines */
#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * String and memory management parameters.
 */
/* Desperation move on my part- the crashing started with the new 30+ areas,
   the links seem to be dropping on almost everything, so... what the hell
   ... maybe the btree cant support as many levels as are being created
       shogar
*/
/*
#define MAX_KEY_HASH		 2048
*/
#define MAX_KEY_HASH		 4096
#define MAX_STRING_LENGTH	 4096 /* buf */
#define MAX_INPUT_LENGTH	 1024 /* arg */
#define MAX_INBUF_SIZE		 1024

#define HASHSTR   /* use string hashing */

#define	MAX_LAYERS		 8 /* maximum clothing layers */
#define MAX_NEST	       100  /* maximum container nesting */

/*
 * Color Alignment Parameters
 */
#define ALIGN_LEFT	1
#define ALIGN_CENTER	2
#define ALIGN_RIGHT	3

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH	       500000
#define MIN_EXP_WORTH		   20

#define MAX_REXITS		   20 /* Maximum exits allowed in 1 room */
#define MAX_SKILL		  800
#define MAX_RACE                   40  /* (was 19)  added more 3/25/99 Scion */
#define MAX_NPC_RACE		   91
#define MAX_MATERIALS		200   /* Object material types -- Scion */
#define MAX_MOONS		50
#define MAX_DEITY		   50
#define	MAX_HERB		   20
#define MAX_ALIAS                  20  /* alias - shogar */
#define	MAX_DISEASE		   20
#define MAX_WHERE_NAME             27
#define MAX_MOBS			10 /* Areas won't reset if more than this * areas mobs in the game */
#define MAX_OBJS			50 /* Areas won't reset if more than this * areas objs in the game */
#define	MAX_xBITS			32000

#define	SECONDS_PER_TICK			 70

#define PULSE_PER_SECOND			  4
#define PULSE_VIOLENCE				 (3 * PULSE_PER_SECOND)
#define PULSE_MOBILE				 (4 * PULSE_PER_SECOND)
#define PULSE_TICK		  (SECONDS_PER_TICK * PULSE_PER_SECOND)
#define PULSE_AREA				(60 * PULSE_PER_SECOND)
#define PULSE_TIME				(60 * 10 * PULSE_PER_SECOND)


/*
 * Stuff for area versions --Shaddai
 */
int area_version;
#define HAS_SPELL_INDEX     -1
#define AREA_VERSION_WRITE 1

/*
 * Command logging types.
 */
typedef enum
{
   LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM,
   LOG_WARN, LOG_ALL
} log_types;

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
   rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
   rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
   rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
   rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE, rCHAR_AND_OBJ_EXTRACTED = 128,
   rERROR = 255
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2
#define ECHOTAR_ROOM	3
#define ECHOTAR_AREA	4
#define ECHOTAR_WORLD	5

/* defines for new do_who */
#define WT_MORTAL	0
#define WT_DEADLY	1
#define WT_IMM		2
#define WT_GROUPED	3
#define WT_GROUPWHO	4
#define WT_GUEST	5
#define WT_STAFF	6
#define WT_ADMIN	7

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
#define INTBITS	32
#endif
#define XBM		31 /* extended bitmask   ( INTBITS - 1 )  */
#define RSV		5  /* right-shift value  ( sqrt(XBM+1) )  */
#define XBI		4  /* integers in an extended bitvector   */
#define MAX_BITS	XBI * INTBITS
/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
   int bits[XBI];
};

#ifdef I3
#include "i3.h"
#endif
#ifdef IMC
#include "imc.h"
#endif

/* abit and qbit struct */
struct bit_data
{
   int number;
   char desc[MAX_STRING_LENGTH];

   BIT_DATA *next;
   BIT_DATA *prev;
};

struct clan_member_data
{
   char *name;
   char *title;
   EXT_BV flags;
   int bank;
   CLAN_DATA *clan;

   CLAN_MEMBER *next;
   CLAN_MEMBER *prev;
};


struct clan_data
{
   char *name;
   char *title;
   char *info;
   int ext_sec;
   int int_sec;
   int bank;

   CLAN_MEMBER *first_member;
   CLAN_MEMBER *last_member;

   CLAN_DATA *next;
   CLAN_DATA *prev;
};


/*
 * do_who output structure -- Narn
 */
struct who_data
{
   WHO_DATA *prev;
   WHO_DATA *next;
   char *text;
   int type;
};

/*
 * Site ban structure.
 */
struct ban_data
{
   BAN_DATA *next;
   BAN_DATA *prev;
   char *name; /* Name of site/race banned */
   char *user; /* Name of user from site */
   char *ban_by;  /* Who banned this site */
   char *ban_time;   /* Time it was banned */
   int unban_date;   /* When ban expires */
   sh_int duration;  /* How long it is banned for */
   bool warn;  /* Echo on warn channel */
   bool prefix;   /* Use of *site */
   bool suffix;   /* Use of site* */
};



/*
 * Yeesh.. remind us of the old MERC ban structure? :)
 */
struct reserve_data
{
   RESERVE_DATA *next;
   RESERVE_DATA *prev;
   char *name;
};



/*
 * Time and weather stuff.
 */
typedef enum
{
   SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
   SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

struct time_info_data
{
   int hour;
   int day;
   int month;
   int year;
   int sunlight;
};

struct hour_min_sec
{
   int hour;
   int min;
   int sec;
   int manual;
};

/* Define maximum number of climate settings - FB */
#define MAX_CLIMATE 5

struct weather_data
{
/*    int			mmhg;
    int			change;
    int			sky;
    int			sunlight; */
   int temp;   /* temperature */
   int precip; /* precipitation */
   int wind;   /* umm... wind */
   int mana;   /* base amount of mana in the zone */
   int temp_vector;  /* vectors controlling */
   int precip_vector;   /* rate of change */
   int wind_vector;
   int mana_vector;
   int climate_temp; /* climate of the area */
   int climate_precip;
   int climate_wind;
   int climate_mana;
   NEIGHBOR_DATA *first_neighbor;   /* areas which affect weather sys */
   NEIGHBOR_DATA *last_neighbor;
   char *echo; /* echo string */
   int echo_color;   /* color for the echo */
};

struct neighbor_data
{
   NEIGHBOR_DATA *next;
   NEIGHBOR_DATA *prev;
   char *name;
   AREA_DATA *address;
};

/*
 * Connected state for a channel.
 */
typedef enum
{
   CON_GET_NAME = -100,
   CON_GET_OLD_PASSWORD,
   CON_CONFIRM_NEW_NAME,
   CON_GET_NEW_PASSWORD,
   CON_CONFIRM_NEW_PASSWORD,
   CON_READ_MOTD,
   CON_GET_WANT_RIPANSI,
   CON_TITLE,
   CON_PRESS_ENTER,
   CON_WAIT_1,
   CON_WAIT_2,
   CON_WAIT_3,
   CON_ACCEPTED,
   CON_GET_NEWBIE,
   CON_CHOOSE_SEX,
   CON_CHOOSE_SPECIES,
   CON_CHOOSE_NATION,
   CON_COPYOVER_RECOVER,

   CON_PLAYING = 0,
   CON_EDITING
} connection_types;

/*
 * Character substates
 */
typedef enum
{
   SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_BAN_DESC, SUB_OBJ_SHORT,
   SUB_OBJ_LONG, SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC,
   SUB_ROOM_EXTRA, SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT,
   SUB_HELP_EDIT, SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD,
   SUB_RESTRICTED, SUB_DEITYDESC,
   SUB_PROJ_DESC,
   /*
    * timer types ONLY below this point 
    */
   SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
   DESCRIPTOR_DATA *next;
   DESCRIPTOR_DATA *prev;
   DESCRIPTOR_DATA *snoop_by;
   CHAR_DATA *character;
   CHAR_DATA *original;
   char *host;
   int port;
   int descriptor;
   sh_int connected;
   sh_int idle;
   sh_int lines;
   sh_int scrlen;
   bool fcommand;
   char inbuf[MAX_INBUF_SIZE];
   char incomm[MAX_INPUT_LENGTH];
   char inlast[MAX_INPUT_LENGTH];
   int repeat;
   char *outbuf;
   unsigned long outsize;
   int outtop;
   char *pagebuf;
   unsigned long pagesize;
   int pagetop;
   char *pagepoint;
   char pagecmd;
   char pagecolor;
   char *user;
   int auth_fd;
   int auth_inc;
   int auth_state;
   int newstate;
   unsigned char prevcolor;
   OLC_DATA *olc; /* Tagith */
};



/*
 * Attribute bonus structures.
 */
struct str_app_type
{
   sh_int tohit;
   sh_int todam;
   sh_int carry;
   sh_int wield;
};

struct int_app_type
{
   sh_int learn;
};

struct wis_app_type
{
   sh_int practice;
};

struct dex_app_type
{
   sh_int defensive;
};

struct con_app_type
{
   sh_int hitp;
   sh_int shock;
};

struct cha_app_type
{
   sh_int charm;
};

struct lck_app_type
{
   sh_int luck;
};

/*
 * TO types for act.
 */
typedef enum
{ TO_ROOM, TO_NOTVICT, TO_VICT, TO_CHAR, TO_CANSEE } to_types;

/*
 * Real action "TYPES" for act.
 */
#define AT_BLACK	    0
#define AT_BLOOD	    1
#define AT_DGREEN           2
#define AT_ORANGE	    3
#define AT_DBLUE	    4
#define AT_PURPLE	    5
#define AT_CYAN	  	    6
#define AT_GREY		    7
#define AT_DGREY	    8
#define AT_RED		    9
#define AT_GREEN	   10
#define AT_YELLOW	   11
#define AT_BLUE		   12
#define AT_PINK		   13
#define AT_LBLUE	   14
#define AT_WHITE	   15
#define AT_BLINK	   16
#define AT_PLAIN	   AT_GREY
#define AT_ACTION	   AT_GREY
#define AT_SAY		   AT_LBLUE
#define AT_GOSSIP	   AT_GREEN
#define AT_YELL	           AT_GREEN
#define AT_TELL		   AT_RED
#define AT_WHISPER	   AT_DGREY
#define AT_HIT		   AT_WHITE
#define AT_HITME	   AT_YELLOW
#define AT_IMMORT	   AT_YELLOW
#define AT_HURT		   AT_RED
#define AT_FALLING	   AT_WHITE + AT_BLINK
#define AT_DANGER	   AT_RED + AT_BLINK
#define AT_MAGIC	   AT_BLUE
#define AT_CONSIDER	   AT_GREY
#define AT_REPORT	   AT_GREY
#define AT_POISON	   AT_GREEN
#define AT_SOCIAL	   AT_CYAN
#define AT_DYING	   AT_YELLOW
#define AT_DEAD		   AT_RED
#define AT_SKILL	   AT_GREEN
#define AT_CARNAGE	   AT_BLOOD
#define AT_DAMAGE	   AT_WHITE
#define AT_FLEE		   AT_YELLOW
#define AT_OBJECT	   AT_GREEN
#define AT_PERSON	   AT_PLAIN
#define AT_LIST		   AT_BLUE
#define AT_BYE		   AT_GREEN
#define AT_GOLD		   AT_YELLOW
#define AT_GTELL	   AT_BLUE
#define AT_NOTE		   AT_GREEN
#define AT_HUNGRY	   AT_ORANGE
#define AT_THIRSTY	   AT_BLUE
#define	AT_FIRE		   AT_RED
#define AT_FROST	   AT_LBLUE
#define AT_CHAOS	   AT_PURPLE
#define AT_SOBER	   AT_WHITE
#define AT_WEAROFF	   AT_YELLOW
#define AT_EXITS	   AT_WHITE
#define AT_SCORE	   AT_LBLUE
#define AT_RESET	   AT_DGREEN
#define AT_LOG		   AT_PURPLE
#define AT_DIEMSG	   AT_WHITE
#define AT_IGNORE	   AT_GREEN
#define AT_DIVIDER	   AT_PLAIN

#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 30

/*
 * Help table types.
 */
struct help_data
{
   HELP_DATA *next;
   HELP_DATA *prev;
   sh_int permit;
   char *keyword;
   char *text;
};

struct new_help_data
{
   NEW_HELP_DATA *next;
   NEW_HELP_DATA *prev;

   char *name;
   char *text;
   int permit;
};

struct book_data
{
   BOOK_DATA *next;
   BOOK_DATA *prev;

   char *title;
   char *text;
   char *name;
};

/*
 * Shop types.
 */

struct shop_data
{
   SHOP_DATA *next;  /* Next shop in list    */
   SHOP_DATA *prev;  /* Previous shop in list   */
   int room;   /* Vnum of shop room    */
   int profit_buy;
   int profit_sell;
   int gold;
   int type;   /* Only sell items of this type */
   int flag;   /* Only sell items with this flag */
   char *owner;   /* Allows player-owned shops  */
};

/* Mob program structures */
struct act_prog_data
{
   struct act_prog_data *next;
   void *vo;
};

struct mob_prog_act_list
{
   MPROG_ACT_LIST *next;
   char *buf;
   CHAR_DATA *ch;
   OBJ_DATA *obj;
   void *vo;
};

struct mob_prog_data
{
   MPROG_DATA *next;
   sh_int type;
   bool triggered;
   int resetdelay;
   char *arglist;
   char *comlist;
};

bool MOBtrigger;


struct channel_data
{
   char *name;
   char *owner;
   int flags;
   char *tag;
   char *div;
   char *clan;
   int permit;
   char *access;
   char *ban;
   char *info;

   CHANNEL_MEMBER *first_member;
   CHANNEL_MEMBER *last_member;

   CHANNEL_DATA *next;
   CHANNEL_DATA *prev;
};

struct channel_member_data
{
   CHAR_DATA *ch;
   int flags;

   CHANNEL_MEMBER *next;
   CHANNEL_MEMBER *prev;
};


struct explored_area_data
{
   EXT_BV rooms;
   int index;

   EXPLORED_AREA *next;
   EXPLORED_AREA *prev;
};

struct species_data
{
   char *name; /* Species name */
   sh_int skill_set;
   sh_int saving_poison_death;
   sh_int saving_wand;
   sh_int saving_para_petri;
   sh_int saving_breath;
   sh_int saving_spell_staff;
   int parts;
   int disadvantage;
   char *adj;

   SPECIES_DATA *next;
   SPECIES_DATA *prev;
};

struct nation_data
{
   char *name; /* Nation name */
   char *species; /* Nation's species */
   EXT_BV affected;  /* Default affect bitvectors    */
   sh_int hit;
   sh_int mana;
   int unarmed;   /* Percentage modifier to unarmed skill */
   sh_int resist;
   sh_int suscept;
   sh_int ac_plus;
   sh_int exp_multiplier;
   sh_int height;
   sh_int weight;
   sh_int metabolism;
   sh_int sector; /* Prefered sectortype */
   sh_int base_age;
   int flags;

   NATION_DATA *next;
   NATION_DATA *prev;

   sh_int str_mod;
   sh_int int_mod;
   sh_int wis_mod;
   sh_int dex_mod;
   sh_int con_mod;
   sh_int cha_mod;
   sh_int lck_mod;
   int parts;
   sh_int weapon;
   char *eyes;
   char *hair;
   char *skin_color;
   char *skin_type;
   char *extra_color;
   char *extra_type;
};

struct generic_mob_data
{
   NATION_DATA *nation;
   int chance;

   GENERIC_MOB *next;
   GENERIC_MOB *prev;
};

struct mutation_data
{
   int number;
   char *desc;
   char *gain_self;
   char *gain_other;
   char *lose_self;
   char *lose_other;
   AFFECT_DATA *affect;
   int rarity;

   MUT_DATA *next;
   MUT_DATA *prev;
};

struct player_mutation_data
{
   MUT_DATA *mut;
   int duration;

   PMUT_DATA *next;
   PMUT_DATA *prev;
};

/* Something under construction in a city */
struct construct_data
{
   int type;
   int builders;
   int done;
   int needed;
   int vnum;

   CONSTRUCT_DATA *next;
   CONSTRUCT_DATA *prev;
};


/* Data structure for the new city management code */
struct city_data
{
   AREA_DATA *area;
   char *name;
   int gold;
   int food;
   int wood;
   int iron;
   int builders;
   int farmers;
   int merchants;
   int miners;
   int peasants;
   int houses;
   int farms;
   int shops;
   int forests;
   int mines;

   CONSTRUCT_DATA *first_building;
   CONSTRUCT_DATA *last_building;

   CITY_DATA *next;
   CITY_DATA *prev;
};


struct recipe_data
{
   int number;
   int ingred1;   /* vnum of first ingredient */
   int ingred2;   /* vnum of second ingredient */
   int result; /* vnum of alchemy result */
   int flags;  /* does it need a fire/drinkcon/etc? */

   RECIPE_DATA *next;
   RECIPE_DATA *prev;
};

struct ingred_data
{
   int vnum;
   int rarity;
   int sector;
   int moisture;
   int elevation;
   int temp;
   int mana;
   int precip;

   INGRED_DATA *next;
   INGRED_DATA *prev;
};

struct deity_data
{
   int index;
   DEITY_DATA *next;
   DEITY_DATA *prev;
   char *filename;
   char *name;
   char *description;
   sh_int cost;
   char *message;
   int count;
   char *archmage;
   int arch_lvl;
   char *color;
};

/*
 * An affect.
 *
 * So limited... so few fields... should we add more?
 */
struct affect_data
{
   AFFECT_DATA *next;
   AFFECT_DATA *prev;
   sh_int type;
   sh_int duration;
   sh_int location;
   int modifier;
   EXT_BV bitvector;
};


/*
 * A SMAUG spell
 */
struct smaug_affect
{
   SMAUG_AFF *next;
   char *duration;
   sh_int location;
   char *modifier;
   int bitvector; /* this is the bit number */
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   5331
#define MOB_VNUM_VAMPIRE	   18
#define MOB_VNUM_ANIMATED_CORPSE   5
#define MOB_VNUM_POLY_WOLF	   10
#define MOB_VNUM_POLY_MIST	   11
#define MOB_VNUM_POLY_BAT	   12
#define MOB_VNUM_POLY_HAWK	   13
#define MOB_VNUM_POLY_CAT	   14
#define MOB_VNUM_POLY_DOVE	   15
#define MOB_VNUM_POLY_FISH	   16
#define MOB_VNUM_GENERIC	   17

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		  0   /* Auto set for mobs */
#define ACT_SENTINEL		  1   /* Stays in one room */
#define ACT_SCAVENGER		  2   /* Picks up objects  */
#define ACT_MAG_FAIL		  3   /* Tried to use spec, but pc was immune */
#define ACT_AGGRESSIVE		  5   /* Attacks PC's      */
#define ACT_STAY_AREA		  6   /* Won't leave area  */
#define ACT_WIMPY		  7   /* Flees when hurt   */
#define ACT_PET			  8   /* Auto set for pets */
#define ACT_TRAIN		  9   /* Can train PC's */
#define ACT_PRACTICE		 10   /* Can practice PC's */
#define ACT_PHYS_FAIL		 11   /* Tried to use physical attack, but PC was ethereal   */
#define ACT_DEADLY		     12  /* UNUSED  */
#define ACT_POLYSELF		 13
#define ACT_META_AGGR		 14   /* UNUSED   */
#define ACT_GUARDIAN		 15   /* Protects master   */
#define ACT_RUNNING		     16  /* Hunts quickly  */
#define ACT_NOWANDER		 17   /* Doesn't wander */
#define ACT_MOUNTABLE		 18   /* Can be mounted */
#define ACT_MOUNTED		     19  /* Is mounted     */
#define ACT_NO_CORPSE            20 /* Does not leave a corpse */
#define ACT_SECRETIVE		 21   /* actions aren't seen  */
#define ACT_NAMED	         22 /* Gets generated names */
#define ACT_MOBINVIS		 23   /* Like wizinvis  */
#define ACT_NOASSIST		 24   /* Doesn't assist mobs  */
#define ACT_AUTONOMOUS		 25   /* UNUSED */
#define ACT_PACIFIST         26  /* Doesn't ever fight   */
#define ACT_NOATTACK		 27   /* No physical attacks */
#define ACT_ANNOYING		 28   /* UNUSED */
#define ACT_PROTOTYPE		 30   /* A prototype mob   */
/* 26 acts */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
typedef enum
{
   AFF_BLIND, AFF_INVISIBLE, AFF_DETECT_EVIL, AFF_DETECT_INVIS,
   AFF_DETECT_MAGIC, AFF_DETECT_HIDDEN, AFF_HOLD, AFF_HOLY,
   AFF_FAERIE_FIRE, AFF_INFRARED, AFF_CURSE, AFF_FLAMING, AFF_POISON,
   AFF_PROTECT, AFF_PARALYSIS, AFF_SNEAK, AFF_HIDE, AFF_SLEEP, AFF_CHARM,
   AFF_FLYING, AFF_PASS_DOOR, AFF_FLOATING, AFF_TRUESIGHT, AFF_DETECTTRAPS,
   AFF_ANTHRO, AFF_HAS_ARTI, AFF_ANGEL, AFF_FEAR, AFF_BEAUTY,
   AFF_POSSESS, AFF_BERSERK, AFF_AQUA_BREATH, AFF_VOID,
   AFF_DREAMWORLD, AFF_HASTE, AFF_SLOW, AFF_DEAF, AFF_NONLIVING,
   AFF_COLDBLOOD, AFF_GUARD, AFF_GLOW, AFF_DARK, AFF_AQUATIC,
   AFF_DEMON, AFF_SILENT, AFF_PLAGUE, AFF_ROTTING, AFF_MOUNTABLE,
   AFF_MOUNTED, AFF_FELINE, AFF_NUISANCE, AFF_UNDEAD,
   AFF_CONSTRUCT, AFF_ETHEREAL, AFF_NO_CORPSE, AFF_UNHOLY, AFF_MAGICAL,
   MAX_AFFECTED_BY
} affected_by_types;


/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_LIGHT		  BV18
#define RIS_PSIONIC		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21
/* 21 RIS's*/

/*
 * Attack types
 */
typedef enum
{
   ATCK_BITE, ATCK_CLAWS, ATCK_TAIL, ATCK_STING, ATCK_PUNCH, ATCK_KICK,
   ATCK_TRIP, ATCK_BASH, ATCK_STUN, ATCK_GOUGE, ATCK_BACKSTAB, ATCK_FEED,
   ATCK_DRAIN, ATCK_FIREBREATH, ATCK_FROSTBREATH, ATCK_ACIDBREATH,
   ATCK_LIGHTNBREATH, ATCK_GASBREATH, ATCK_POISON, ATCK_NASTYPOISON, ATCK_GAZE,
   ATCK_BLINDNESS, ATCK_CAUSESERIOUS, ATCK_EARTHQUAKE, ATCK_CAUSECRITICAL,
   ATCK_CURSE, ATCK_FLAMESTRIKE, ATCK_HARM, ATCK_FIREBALL, ATCK_COLORSPRAY,
   ATCK_WEAKEN, ATCK_SPIRALBLAST, MAX_ATTACK_TYPE
} attack_types;

/*
 * Defense types
 */
typedef enum
{
   DFND_PARRY, DFND_DODGE, DFND_HEAL, DFND_CURELIGHT, DFND_CURESERIOUS,
   DFND_CURECRITICAL, DFND_DISPELMAGIC, DFND_DISPELEVIL, DFND_SANCTUARY,
   DFND_FIRESHIELD, DFND_SHOCKSHIELD, DFND_SHIELD, DFND_BLESS, DFND_STONESKIN,
   DFND_TELEPORT, DFND_MONSUM1, DFND_MONSUM2, DFND_MONSUM3, DFND_MONSUM4,
   DFND_DISARM, DFND_ICESHIELD, DFND_GRIP, DFND_TRUESIGHT, DFND_TUMBLE,
   MAX_DEFENSE_TYPE
} defense_types;

/*
 * Body parts
 */
#define PART_HEAD		  0
#define PART_ARMS		  1
#define PART_LEGS		  2
#define PART_HEART		  3
#define PART_BRAINS		  4
#define PART_GUTS		  5
#define PART_HANDS		  6
#define PART_FEET		  7
#define PART_FINGERS	  8
#define PART_EAR		  9
#define PART_EYE		  10
#define PART_LONG_TONGUE  11
#define PART_EYESTALKS	  12
#define PART_TENTACLES	  13
#define PART_FINS		  14
#define PART_WINGS		  15
#define PART_TAIL		  16
#define PART_SCALES		  17
#define PART_CLAWS		  18
#define PART_FANGS		  19
#define PART_HORNS		  20
#define PART_TUSKS		  21
#define PART_TAILATTACK	  22
#define PART_SHARPSCALES  23
#define PART_BEAK		  24
#define PART_HAUNCH		  25
#define PART_HOOVES		  26
#define PART_PAWS		  27
#define PART_FORELEGS	  28
#define PART_FEATHERS	  29
#define MAX_PARTS		  30

/* Conditions of parts */
#define PART_MISSING		0
#define PART_WELL		100
#define PART_BOUND		99
#define PART_BROKEN		50
#define PART_SEVERED		1

/* New part flags */
typedef enum
{
   BP_HEAD, BP_NECK, BP_CHEST, BP_STOMACH, BP_BACK, BP_NOSE,
   BP_TAIL, BP_RHAND, BP_LHAND, BP_RARM, BP_LARM, BP_RLEG,
   BP_LLEG, BP_RFOOT, BP_LFOOT, BP_REYE, BP_LEYE, BP_RWRIST,
   BP_LWRIST, BP_RANKLE, BP_LANKLE, BP_RHOOF, BP_LHOOF, BP_RPAW,
   BP_LPAW, BP_RWING, BP_LWING, BP_REAR, BP_LEAR, BP_HORN,
   BP_RHORN, BP_LHORN, BP_TONGUE, BP_FANGS, BP_TENTACLES, BP_BEAK,
   BP_RFLEG, BP_LFLEG, BP_RRLEG, BP_LRLEG, BP_ASS, BP_FACE,
   BP_RFIN, BP_LFIN, BP_DFIN, BP_PENIS, BP_VAGINA, BP_CLAWS
} part_bits;

/*
 * Autosave flags
 */
#define SV_DEATH		  BV00   /* Save on death */
#define SV_KILL			  BV01   /* Save when kill made */
#define SV_PASSCHG		  BV02   /* Save on password change */
#define SV_DROP			  BV03   /* Save on drop */
#define SV_PUT			  BV04   /* Save on put */
#define SV_GIVE			  BV05   /* Save on give */
#define SV_AUTO			  BV06   /* Auto save every x minutes (define in cset) */
#define SV_ZAPDROP		  BV07   /* Save when eq zaps */
#define SV_UNUSED		  BV08   /* Not used */
#define SV_GET			  BV09   /* Save on get */
#define SV_RECEIVE		  BV10   /* Save when receiving */
#define SV_IDLE			  BV11   /* Save when char goes idle */
#define SV_BACKUP		  BV12   /* Make backup of pfile on save */
#define SV_QUITBACKUP	  BV13   /* Backup on quit only --Blod */

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT	  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH	  BV08

/*
 * Flags for act_string -- Shaddai
 */
#define STRING_NONE               0
#define STRING_IMM                BV01


/*
 * old flags for conversion purposes -- will not conflict with the flags below
 */
#define OLD_SF_SAVE_HALF_DAMAGE	  BV18   /* old save for half damage */
#define OLD_SF_SAVE_NEGATES	  BV19   /* old save negates affect  */

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		  BV00   /* won't work without water  */
#define SF_EARTH		  BV01   /* won't work without plants */
#define SF_AIR			  BV02   /* won't work without air */
#define SF_ASTRAL		  BV03   /* dreamworld */
#define SF_AREA			  BV04   /* is an area spell      */
#define SF_DISTANT		  BV05   /* affects something far away  */
#define SF_REVERSE		  BV06
#define SF_NOSELF		  BV07   /* Can't target yourself!   */
#define SF_GROUND		  BV08   /* won't work unless on ground */
#define SF_ACCUMULATIVE		  BV09   /* is accumulative    */
#define SF_RECASTABLE		  BV10   /* can be refreshed      */
#define SF_NOSCRIBE		  BV11   /* cannot be scribed     */
#define SF_NOBREW		  BV12   /* cannot be brewed      */
#define SF_GROUPSPELL		  BV13   /* only affects group members  */
#define SF_OBJECT		  BV14   /* directed at an object   */
#define SF_CHARACTER		  BV15   /* directed at a character  */
#define SF_SECRETSKILL		  BV16   /* hidden unless learned   */
#define SF_PKSENSITIVE		  BV17   /* much harder for plr vs. plr   */
#define SF_STOPONFAIL		  BV18   /* stops spell on first failure */
#define SF_NOFIGHT		  BV19   /* stops if char fighting       */
#define SF_NODISPEL               BV20 /* stops spell from being dispelled */
#define SF_INSTANT		  BV21   /* spell is cast instantly */
typedef enum
{ SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
   SS_BREATH, SS_SPELL_STAFF
} save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)
#define SSAV_MASK		ALL_BITS & ~(BV11 | BV12 | BV13)

typedef enum
{ SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
   SD_POISON, SD_DRAIN
} spell_dam_types;

typedef enum
{ SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
   SA_DIVINATE, SA_OBSCURE, SA_CHANGE
} spell_act_types;

typedef enum
{ SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum
{ SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
   SC_LIFE, SC_DEATH, SC_ILLUSION
} spell_class_types;

typedef enum
{ SE_NONE, SE_NEGATE, SE_EIGHTHDAM, SE_QUARTERDAM, SE_HALFDAM,
   SE_3QTRDAM, SE_REFLECT, SE_ABSORB
} spell_save_effects;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum
{ SEX_NEUTRAL, SEX_MALE, SEX_FEMALE, SEX_HERMAPH } sex_types;

typedef enum
{
   BLDG_ROAD, BLDG_HOUSE, BLDG_SHOP, BLDG_WALL
} building_types;

typedef enum
{
   TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART, TRAP_TYPE_POISON_NEEDLE,
   TRAP_TYPE_POISON_DAGGER, TRAP_TYPE_POISON_ARROW, TRAP_TYPE_BLINDNESS_GAS,
   TRAP_TYPE_SLEEPING_GAS, TRAP_TYPE_FLAME, TRAP_TYPE_EXPLOSION,
   TRAP_TYPE_ACID_SPRAY, TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
   TRAP_TYPE_SEX_CHANGE
} trap_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_PART	     12
#define OBJ_VNUM_BLOOD		     17
#define OBJ_VNUM_PUDDLE		     55
#define OBJ_VNUM_BLOODSTAIN	     18
#define OBJ_VNUM_SCRAPS		     19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_SKIN		     23
#define OBJ_VNUM_SLICE		     24
#define OBJ_VNUM_SHOPPING_BAG	     25

#define OBJ_VNUM_FIRE		     30
#define OBJ_VNUM_TRAP		     31
#define OBJ_VNUM_PORTAL		     32

#define OBJ_VNUM_BLACK_POWDER	     33
#define OBJ_VNUM_SCROLL_SCRIBING     34
#define OBJ_VNUM_EMPTY_FLASK         35
#define OBJ_VNUM_NOTE		     36

#define OBJ_VNUM_BAG_OF_HOLDING		44
#define OBJ_VNUM_STATUE			45 /* Gargoyles turn into statues */
#define OBJ_VNUM_MAGIC_MARKER		46
#define OBJ_VNUM_WINE			48
#define OBJ_VNUM_BALL_DARK		49
#define OBJ_VNUM_MYSTIC_SHOVEL		52

#define OBJ_VNUM_KEY			47

#define OBJ_VNUM_CHAOS_CHEESE		17001

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
   ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
   ITEM_RADIO, ITEM_SCANNER, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
   ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_INSTRUMENT, ITEM_CONTAINER,
   ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
   ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
   ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
   ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
   ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_RUNE, ITEM_RUNEPOUCH,
   ITEM_SHEATH, ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER,
   ITEM_TINDER, ITEM_LOCKPICK, ITEM_LOCK, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
   ITEM_VEHICLE, ITEM_BANK, ITEM_MISSILE_WEAPON, ITEM_PROJECTILE,
   ITEM_QUIVER, ITEM_SHOVEL, ITEM_SALVE, ITEM_COOK, ITEM_KEYRING, ITEM_ODOR
} item_types;

#define MAX_ITEM_TYPE		     ITEM_ODOR

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
typedef enum
{
   ITEM_GLOW, ITEM_HUM, ITEM_DARK, ITEM_LOYAL, ITEM_EVIL, ITEM_INVIS, ITEM_MAGIC,
   ITEM_NODROP, ITEM_BLESS, ITEM_IN_USE, ITEM_HOVER, ITEM_NO_RESET,
   ITEM_NOREMOVE, ITEM_INVENTORY, ITEM_DREAMWORLD, ITEM_EXPLOSIVE,
   ITEM_FLAMMABLE, ITEM_NO_TAKE, ITEM_ORGANIC, ITEM_METAL, ITEM_RETURNING,
   ITEM_GEM, ITEM_DURABLE, ITEM_SHIELD, ITEM_TWO_HANDED,
   ITEM_HIDDEN, ITEM_POISONED, ITEM_COVERING, ITEM_DEATHROT, ITEM_BURIED,
   ITEM_PROTOTYPE, ITEM_NOLOCATE, ITEM_GROUNDROT, ITEM_ARTIFACT, ITEM_PLRBLD,
   MAX_ITEM_FLAG
} item_extra_flags;

#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		BV00
#define ITEM_WEAR_FINGER	BV01
#define ITEM_WEAR_NECK		BV02
#define ITEM_WEAR_BODY		BV03
#define ITEM_WEAR_HEAD		BV04
#define ITEM_WEAR_LEGS		BV05
#define ITEM_WEAR_FEET		BV06
#define ITEM_WEAR_HANDS		BV07
#define ITEM_WEAR_ARMS		BV08
#define ITEM_WEAR_SHIELD	BV09
#define ITEM_WEAR_ABOUT		BV10
#define ITEM_WEAR_WAIST		BV11
#define ITEM_WEAR_WRIST		BV12
#define ITEM_WEAR_HAND		BV13  /* formerly wield */
#define ITEM_UNUSED			BV14  /* formerly hold */
#define ITEM_WEAR_HOOVES	BV15  /* formerly dual-wield */
#define ITEM_WEAR_EARS		BV16
#define ITEM_WEAR_EYES		BV17
#define ITEM_UNUSED3		BV18  /* formerly missile_wield */
#define ITEM_WEAR_BACK		BV19
#define ITEM_WEAR_FACE		BV20
#define ITEM_WEAR_ANKLE		BV21
#define ITEM_WEAR_FLOAT		BV22
#define ITEM_WEAR_MAX		23

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
   APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
   APPLY_SEX, APPLY_UNUSED, APPLY_UNUSED2, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
   APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
   APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
   APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
   APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
   APPLY_WEAPONSPELL, APPLY_LCK, APPLY_FIRE_MAGIC,
   APPLY_EARTH_MAGIC, APPLY_WIND_MAGIC, APPLY_FROST_MAGIC,
   APPLY_LIGHTNING_MAGIC, APPLY_WATER_MAGIC, APPLY_DREAM_MAGIC,
   APPLY_SPEECH_MAGIC, APPLY_HEALING_MAGIC, APPLY_DEATH_MAGIC,
   APPLY_CHANGE_MAGIC, APPLY_TIME_MAGIC, APPLY_MOTION_MAGIC,
   APPLY_MIND_MAGIC, APPLY_ILLUSION_MAGIC, APPLY_SEEKING_MAGIC,
   APPLY_SECURITY_MAGIC, APPLY_CATALYSM_MAGIC, APPLY_VOID_MAGIC,
   APPLY_CLIMB, APPLY_GRIP, APPLY_NOTHING, APPLY_SCRIBE, APPLY_BREW,
   APPLY_WEARSPELL,
   APPLY_REMOVESPELL, APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN,
   APPLY_REMOVE, APPLY_DIG, APPLY_FULL, APPLY_THIRST, APPLY_DRUNK,
   APPLY_BLOOD, APPLY_COOK, APPLY_RECURRINGSPELL, APPLY_CONTAGIOUS,
   APPLY_EXT_AFFECT, APPLY_ODOR, APPLY_ROOMFLAG, APPLY_SECTORTYPE,
   APPLY_ROOMLIGHT, APPLY_TELEVNUM, APPLY_TELEDELAY, MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		   BV00
#define CONT_PICKPROOF		   BV01
#define CONT_CLOSED		   BV02
#define CONT_LOCKED		   BV03
#define CONT_EATKEY		   BV04

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_POLY		      3
#define ROOM_VNUM_HELL                6   /* for hell fix - shogar */
#define ROOM_VNUM_NEXUS		  20000
#define ROOM_VNUM_PKILL		  5000
#define ROOM_VNUM_SCHOOL	  100

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

#define ROOM_DARK		BV00
#define ROOM_NO_QUIT		BV01
#define ROOM_NO_MOB		BV02
#define ROOM_INDOORS		BV03
#define ROOM_SLIPPERY		BV04
#define ROOM_STICKY		BV05
#define ROOM_BURNING		BV06
#define ROOM_NO_MAGIC		BV07
#define ROOM_TUNNEL		BV08
#define ROOM_PRIVATE		BV09
#define ROOM_SAFE		BV10
#define ROOM_SOLITARY		BV11
#define ROOM_PET_SHOP		BV12
#define ROOM_NO_RECALL		BV13
#define ROOM_DONATION		BV14
#define ROOM_NODROPALL		BV15
#define ROOM_SILENCE		BV16
#define ROOM_LOGSPEECH		BV17
#define ROOM_NODROP		BV18
#define ROOM_NO_DREAM		BV19
#define ROOM_NO_SUMMON		BV20
#define ROOM_NO_ASTRAL		BV21
#define ROOM_NOFLOOR		BV24
#define ROOM_NOSUPPLICATE       BV25
#define ROOM_R26		BV26
#define ROOM_NOMISSILE		BV27
#define ROOM_AMPLIFY		BV28
#define ROOM_R29		BV29
#define ROOM_PROTOTYPE	     	BV30

#define RUNE_FIRE		BV00
#define RUNE_FROST		BV01
#define RUNE_SHOCK		BV02
#define RUNE_LIGHT		BV03
#define RUNE_PAIN		BV04
#define RUNE_ACID		BV05
#define RUNE_POISON		BV06
#define RUNE_TRAVEL		BV07
#define RUNE_LIFE		BV08
#define RUNE_WIND		BV09
#define RUNE_DISTRACT		BV10
#define RUNE_WARD		BV11


/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
   DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
   DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define PT_WATER	100
#define PT_AIR		200
#define PT_EARTH	300
#define PT_FIRE		400

/*
 * Push/pull types for exits					-Thoric
 * To differentiate between the current of a river, or a strong gust of wind
 */
typedef enum
{
   PULL_UNDEFINED, PULL_VORTEX, PULL_VACUUM, PULL_SLIP, PULL_ICE, PULL_MYSTERIOUS,
   PULL_CURRENT = PT_WATER, PULL_WAVE, PULL_WHIRLPOOL, PULL_GEYSER,
   PULL_WIND = PT_AIR, PULL_STORM, PULL_COLDWIND, PULL_BREEZE,
   PULL_LANDSLIDE = PT_EARTH, PULL_SINKHOLE, PULL_QUICKSAND, PULL_EARTHQUAKE,
   PULL_LAVA = PT_FIRE, PULL_HOTAIR
} dir_pulltypes;


#define MAX_DIR			DIR_SOUTHWEST  /* max for normal walking */
#define DIR_PORTAL		DIR_SOMEWHERE  /* portal direction    */


/*
 * Exit flags.			EX_RES# are reserved for use by the
 * Used in #ROOMS.		SMAUG development team
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_EATKEY		  BV09
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES1			  BV14
#define EX_RES2			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_NOFLEE	  	  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define EX_NEEDLE_TRAP		  BV27
#define MAX_EXFLAG		  27

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
   SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
   SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
   SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_LAVA,
   SECT_SWAMP, SECT_ICE, SECT_BEACH, SECT_MAX
} sector_types;

#define MAX_WEAR	52

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
   WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
   WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
   WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
   WEAR_HAND, WEAR_HAND2, WEAR_HOOVES, WEAR_EARS, WEAR_EYES,
   WEAR_UNUSED, WEAR_BACK, WEAR_FACE, WEAR_ANKLE_L, WEAR_ANKLE_R,
   WEAR_FLOAT, MAX_WEAR_OLD
} wear_locations;

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
   COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

/*
 * Positions.
 */
typedef enum
{
   POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_CHANNEL,
   POS_RESTING, POS_SITTING, POS_KNEELING, POS_SQUATTING, POS_SWIMMING,
   POS_FIGHTING, POS_STANDING, POS_MOUNTED, POS_FLYING, POS_SHOVE, POS_DRAG
} positions;

/*
 * ACT bits for players.
 */
typedef enum
{
   PLR_IS_NPC, PLR_LONG_EXIT, PLR_DEFENSE, PLR_AUTOEXIT, PLR_AUTOLOOT,
   PLR_NOHUNGER, PLR_BLANK, PLR_OUTCAST, PLR_BRIEF, PLR_COMBINE, PLR_PROMPT,
   PLR_TELNET_GA, PLR_HOLYLIGHT, PLR_WIZINVIS, PLR_ROOMVNUM, PLR_SILENCE,
   PLR_NO_EMOTE, PLR_ATTACKER, PLR_NO_TELL, PLR_LOG, PLR_DENY, PLR_FREEZE,
   PLR_THIEF, PLR_KILLER, PLR_LITTERBUG, PLR_ANSI, PLR_MULTI, PLR_OOC,
   PLR_SHOW_EXP, PLR_AUTOGOLD, PLR_AUTOMAP, PLR_AFK, PLR_INVISPROMPT,
   PLR_UNUSED, PLR_AUTOSEX, PLR_NO_CHANNEL, PLR_BOUNTY
} player_flags;

/*Bits for pc_data->flags. */
#define PCFLAG_MASK                BV00   /* illusion conceal aura */
#define PCFLAG_CHEAT               BV01   /* flag set if a char's levels are set */
#define PCFLAG_UNAUTHED		   BV02  /* has been counted in race/talent */
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_SHIELD	  	   BV11  /* mental shield */
#define PCFLAG_DIAGNOSE		   BV12
#define PCFLAG_HIGHGAG		   BV13
#define PCFLAG_HELPSTART	   BV15  /* Force new players to help start */
#define PCFLAG_UNUSED	 	   BV16
#define PCFLAG_NOBEEP		   BV17  /* stop people from beeping you -- Scion */
#define PCFLAG_ANONYMOUS	   BV18  /* Stop mortals' ranks and levels from showing up -- Scion */
#define PCFLAG_SPAR		   BV19  /* Will not fight to kill -keo */
#define PCFLAG_VAMPIRE		   BV20  /* Player is a vampire -keo */
#define PCFLAG_RW2		   BV21  /* Player has been updated for RW2 -keo */
#define PCFLAG_COUNTED		   BV22

typedef enum
{
   TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
   TIMER_APPLIED, TIMER_PKILLED, TIMER_ASUPRESSED, TIMER_NUISANCE,
   TIMER_MAGIC
} timer_types;

struct timer_data
{
   TIMER *prev;
   TIMER *next;
   DO_FUN *do_fun;
   int value;
   sh_int type;
   int count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_GENERAL		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_SPEECH		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_TWINK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_YELL		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
#define CHANNEL_HIGHGOD		   BV10
#define CHANNEL_PRIVATE		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_HIGH		   BV13
#define CHANNEL_OOC		   BV14
#define CHANNEL_PRAY		   BV15
#define CHANNEL_UNUSED2 	     BV16
#define CHANNEL_UNUSED3              BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_FOREIGN            BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_RACETALK           BV23
#define CHANNEL_WARN               BV24
#define CHANNEL_WHISPER		   BV25
#define CHANNEL_HUMOR		   BV26
#define	CHANNEL_GRATZ			BV27
#define CHANNEL_THINK			BV28

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_FREEKILL		    BV01
#define AFLAG_NOTELEPORT	    BV02
#define AFLAG_NOQUIT		    BV03
#define AFLAG_NOBYPASS		    BV04
#define AFLAG_NOTRAVEL		    BV05

/* Mud wide quests - Keolah Nov 30 2000 */
#define QUEST_ETERNAL_NIGHT		BV00
#define QUEST_TEMPEST			BV01
#define QUEST_EVIL			BV02

/* Command Permissions - Keolah Aug 16 2001 */
#define PERMIT_ADMIN		BV00
#define PERMIT_HELP		BV01
#define PERMIT_BUILD		BV02
#define PERMIT_HIBUILD		BV03
#define PERMIT_SECURITY		BV04
#define PERMIT_TALENT		BV05
#define PERMIT_ITEM		BV06
#define PERMIT_MISC		BV07
#define PERMIT_CHANNEL		BV08
#define PERMIT_MOB		BV09
#define PERMIT_PROG		BV11

/* Flags for recipes -keo */
#define RECIPE_FIRE		BV00
#define RECIPE_WATER		BV01
#define RECIPE_DRINKCON		BV02
#define RECIPE_CONTAINER	BV03
#define RECIPE_ICE		BV04
#define RECIPE_SUN		BV05
#define RECIPE_MOON		BV06
#define RECIPE_LIGHT		BV07
#define RECIPE_DARK		BV08
#define RECIPE_KNIFE		BV09
#define RECIPE_BLOOD		BV10

/* Mood flags -keo */
#define MOOD_RELAXED		BV00
#define MOOD_READY		BV01
#define MOOD_ANGRY		BV02
#define MOOD_AFRAID		BV03
#define MOOD_FRIENDY		BV04
#define MOOD_GUARD		BV05
#define MOOD_DEFENSE		BV06

/* Vehicle flags -keo */
#define VEHICLE_LAND		BV00
#define VEHICLE_SURFACE		BV01
#define VEHICLE_UNDERWATER	BV02
#define VEHICLE_AIR		BV03
#define	VEHICLE_SPACE		BV04
#define VEHICLE_UNDERGROUND	BV05
#define VEHICLE_CLIMB		BV06
#define VEHICLE_AIRTIGHT	BV07
#define VEHICLE_SAND		BV08
#define VEHICLE_ICE		BV09

/* Social flags -keo */
#define SOC_SLEEP			BV00
#define SOC_POLITE		BV01
#define SOC_RUDE			BV02
#define SOC_SEXUAL		BV03
#define SOC_VISUAL		BV04
#define SOC_AUDITORY		BV05
#define SOC_TOUCH			BV06
#define SOC_ANNOYING		BV06

/* Race flags */
#define NAT_NPC			BV00  /* Nation is NPC only */
#define NAT_NEWBIE		BV01  /* Newbie approved */

/* Arena flags */
#define ARENA_TEAM		BV00
#define ARENA_HALT		BV01
#define ARENA_DEBUG		BV02

/* Gladiator flags */
#define GLAD_RANDOM		BV00
#define GLAD_WAIT		BV01
#define GLAD_TOGGLE		BV02

typedef enum
{
   CL_LEADER, CL_INDUCT, CL_OUTCAST, CL_BANK, CL_FINANCE, CL_BESTOW,
   CL_SECRET, CL_KNOWLEDGE, CL_ALLOW
} clan_flags;

typedef enum
{
   LANG_KALORESE, LANG_ELVEN, LANG_DWARVEN, LANG_SUNRISE, LANG_ORCISH,
   LANG_DRAGON, LANG_ISLANDIC, LANG_DROW
} languages;

typedef enum
{
   SK_SMITH, SK_ALCHEMY, SK_TAILOR, SK_STEALTH, SK_STREET, SK_JEWEL,
   SK_NATURE, SK_MEDICINE, SK_MINING, SK_CHANNEL, SK_LINGUIST
} noncombat_skills;

typedef enum
{
   SK_COMBAT, SK_SWORD, SK_DAGGER, SK_AXE, SK_WHIP, SK_MACE, SK_WAND,
   SK_FIREARMS, SK_BOW, SK_CROSSBOW, SK_STAFF, SK_HALBERD, SK_SPEAR,
   SK_POLEARM, SK_CLAW, SK_FANG, SK_HEAD, SK_TAIL, SK_HAND, SK_KICK
} combat_skills;

typedef enum
{
   PHASE_NEW, PHASE_CRESCENT, PHASE_HALF, PHASE_GIBBOUS, PHASE_FULL
} moon_phases;

typedef enum
{
   MOON_DARK, MOON_HUNGER, MOON_BLOOD
} moon_affects;

typedef enum
{
   BANE_NONE, BANE_UNDEAD, BANE_DEMON, BANE_ELF, BANE_ORC, BANE_DRAGON,
   BANE_HUMAN, BANE_DWARF, BANE_FAERIE, BANE_GIANT, BANE_MINOTAUR,
   BANE_TROLL, BANE_HALFLING, BANE_ANIMAL, BANE_FELINE, BANE_CENTAUR,
   BANE_CONSTRUCT, BANE_ANGEL
} weapons_banes;

typedef enum
{
   BRAND_NONE, BRAND_POISON, BRAND_FLAMING, BRAND_VORPAL, BRAND_FROZEN,
   BRAND_ELEC, BRAND_ACID, BRAND_STORM, BRAND_RADIANT, BRAND_CRUSH,
   BRAND_DRAIN, BRAND_PSIONIC, BRAND_ANTI_MATTER, BRAND_SONIC,
   BRAND_DISTORT
} weapon_brands;

typedef enum
{
   SONG_NONE, SONG_PEACE, SONG_SILENCE, SONG_SLEEP, SONG_FLIGHT,
   SONG_CHARM, SONG_STRENGTH, SONG_SPEED, SONG_PROTECT,
   SONG_VIGOR, SONG_FIRE, SONG_THUNDER, SONG_WIND, SONG_RIVERS,
   SONG_SIGHT, SONG_SHADOWS, SONG_LIGHT, SONG_WINTER, SONG_LIFE,
   MAX_SONG
} speech_songs;

typedef enum
{
   TAL_FIRE, TAL_EARTH, TAL_WIND, TAL_FROST, TAL_LIGHTNING, TAL_WATER,
   TAL_DREAM, TAL_SPEECH, TAL_HEALING, TAL_DEATH, TAL_CHANGE, TAL_TIME,
   TAL_MOTION, TAL_MIND, TAL_ILLUSION, TAL_SEEKING, TAL_SECURITY,
   TAL_CATALYSM, TAL_VOID
} talents;

typedef enum
{
   MAG_NONE, MAG_FIRE, MAG_COLD, MAG_ELECTRICITY, MAG_ENERGY, MAG_ACID,
   MAG_POISON, MAG_DRAIN, MAG_PSIONIC, MAG_LIGHT, MAG_BLUNT, MAG_PIERCE,
   MAG_SLASH, MAG_WIND, MAG_WATER, MAG_EARTH, MAG_SOUND, MAG_DREAM,
   MAG_CHANGE, MAG_ANTIMATTER, MAG_ANTIMAGIC, MAG_HOLY, MAG_UNHOLY,
   MAG_TELEKINETIC, MAG_ETHER, MAG_TIME, MAG_SEEK, MAG_ALL
} magic_dam_types;

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
   MOB_INDEX_DATA *next;
   MOB_INDEX_DATA *next_sort;
   SPEC_FUN *spec_fun;
   MPROG_DATA *mudprogs;
   EXT_BV progtypes;
   char *player_name;
   char *short_descr;
   char *description;
   int vnum;
   sh_int count;
   sh_int killed;
   sh_int sex;
   EXT_BV act;
   EXT_BV affected_by;
   sh_int mobthac0;  /* Unused */
   sh_int ac;
   sh_int hitnodice;
   sh_int hitsizedice;
   sh_int hitplus;
   sh_int damnodice;
   sh_int damsizedice;
   sh_int damplus;
   sh_int numattacks;
   int gold;
   int exp;
   int xflags;
   int resistant;
   int immune;
   int susceptible;
   EXT_BV attacks;
   EXT_BV defenses;
   sh_int position;
   sh_int defposition;
   sh_int height;
   sh_int weight;
   sh_int race;
   sh_int hitroll;
   sh_int damroll;
   sh_int perm_str;
   sh_int perm_int;
   sh_int perm_wis;
   sh_int perm_dex;
   sh_int perm_con;
   sh_int perm_cha;
   sh_int perm_lck;
   sh_int saving_poison_death;
   sh_int saving_wand;
   sh_int saving_para_petri;
   sh_int saving_breath;
   sh_int saving_spell_staff;
};


struct hunt_hate_fear
{
   char *name;
   CHAR_DATA *who;
};

struct editor_data
{
   sh_int numlines;
   sh_int on_line;
   sh_int size;
   char line[49][81];
};

struct extracted_char_data
{
   EXTRACT_CHAR_DATA *next;
   CHAR_DATA *ch;
   ROOM_INDEX_DATA *room;
   ch_ret retcode;
   bool extract;
};

/*
 * One character (PC or NPC).
 */
struct char_data
{
   CHAR_DATA *next;
   CHAR_DATA *prev;
   CHAR_DATA *next_in_room;
   CHAR_DATA *prev_in_room;
   CHAR_DATA *master;
   CHAR_DATA *leader;
   CHAR_DATA *protected_by;   /* Scion */
   CHAR_DATA *reply;
   CHAR_DATA *retell;
   CHAR_DATA *switched;
   CHAR_DATA *mount;
   HHF_DATA *hunting;
   HHF_DATA *fearing;
   HHF_DATA *hating;
   SPEC_FUN *spec_fun;
   MPROG_ACT_LIST *mpact;
   int mpactnum;
   sh_int mpscriptpos;
   MOB_INDEX_DATA *pIndexData;
   DESCRIPTOR_DATA *desc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   OBJ_DATA *first_carrying;
   OBJ_DATA *last_carrying;
   ROOM_INDEX_DATA *in_room;
   OBJ_DATA *on;  /* furniture, stand on objects */
   ROOM_INDEX_DATA *was_in_room;
   int home_room; /* So sentinel mobs know how to go home after chasing down PCs */
   RESET_DATA *reset;   /* So mobs know what reset spawned them when they leave the game */
   PC_DATA *pcdata;
   DO_FUN *last_cmd;
   DO_FUN *prev_cmd; /* mapping */
   void *dest_buf;   /* This one is to assign to differen things */
   char *alloc_ptr;  /* Must str_dup and free this one */
   void *spare_ptr;
   int tempnum;
   EDITOR_DATA *editor;
   TIMER *first_timer;
   TIMER *last_timer;
   char *name;
   char *short_descr;
   char *description;
   sh_int substate;
   sh_int sex;
   sh_int race;
   sh_int talent[MAX_DEITY];  /* 'levels' of each talent the player has access to */
   sh_int curr_talent[MAX_DEITY];
   sh_int trust;
   int played;
   time_t logon;
   time_t save_time;
   sh_int timer;
   sh_int wait;
   sh_int hit;
   sh_int max_hit;
   sh_int base_hit;
   sh_int mana;
   sh_int max_mana;
   sh_int base_mana;
   sh_int move;
   sh_int max_move;
   sh_int base_move;
   sh_int numattacks;
   unsigned long gold;
   int exp;
   EXT_BV act;
   EXT_BV affected_by;
   EXT_BV no_affected_by;
   int carry_weight;
   int carry_number;
   int xflags;
   int resistant;
   int no_resistant;
   int immune;
   int no_immune;
   int susceptible;
   int no_susceptible;
   EXT_BV attacks;
   EXT_BV defenses;
   sh_int saving_poison_death;
   sh_int saving_wand;
   sh_int saving_para_petri;
   sh_int saving_breath;
   sh_int saving_spell_staff;
   sh_int barenumdie;
   sh_int baresizedie;
   sh_int mobthac0;
   sh_int hitroll;
   sh_int damroll;
   sh_int hitplus;
   sh_int damplus;
   sh_int position;
   sh_int height;
   sh_int weight;
   sh_int armor;
   sh_int wimpy;
   sh_int perm_str;
   sh_int perm_int;
   sh_int perm_wis;
   sh_int perm_dex;
   sh_int perm_con;
   sh_int perm_cha;
   sh_int perm_lck;
   sh_int mod_str;
   sh_int mod_int;
   sh_int mod_wis;
   sh_int mod_dex;
   sh_int mod_con;
   sh_int mod_cha;
   sh_int mod_lck;
   sh_int mental_state; /* simplified */
   sh_int emotional_state; /* simplified */
   int pagelen;   /* BUILD INTERFACE */
   int retran;
   int regoto;
   sh_int mobinvis;  /* Mobinvis level SB */
   int fur_pos;   /* furniture pos. */
   int recall; /* Where to recall to -- Scion */
   char *last_taken; /* Name of the last object picked up by mob/pc */
   BIT_DATA *first_abit;
   BIT_DATA *last_abit;
   char *species;
   NATION_DATA *nation;
   int singing;
   int shield;
   int mod_talent[MAX_DEITY];
   CHAR_DATA *last_hit; /* char that last damaged char */
   PART_DATA *first_part;  /* list of bodyparts */
   PART_DATA *last_part;
   OBJ_DATA *main_hand; /* wielded weapons etc */
   OBJ_DATA *off_hand;
   int mood;   /* anger, fear, etc */
   int encumberance; /* weight of items being worn */
   int speed;
   OBJ_DATA *in_obj; /* for vehicles */
};


struct killed_data
{
   int vnum;
   char count;
};


typedef struct alias_queue ALIAS_QUEUE;

struct alias_queue
{
   ALIAS_QUEUE *next;
   char *cmd;
};

/*
 * Data which only PC's have.
 */
struct pc_data
{
   ALIAS_DATA *first_alias;
   ALIAS_DATA *last_alias;
   ALIAS_QUEUE *alias_queue;
   int alias_used;   /* Alias used x times -- Scion */
   CHAR_DATA *pet;
   AREA_DATA *area;
   DEITY_DATA *deity;
   BIT_DATA *first_qbit;
   BIT_DATA *last_qbit;
   char *homepage;
   char *deity_name;
   char *pwd;
   char *bamfin;
   char *bamfout;
   char *filename;   /* For the safe mset name -Shaddai */
   char *rank;
   char *title;
   int version;
   int flags;  /* Whether the player is deadly and whatever else we add.      */
   int pkills; /* Number of pkills */
   int pdeaths;   /* Number of times pkilled (legally)  */
   int mkills; /* Number of mobs killed         */
   int mdeaths;   /* Number of deaths due to mobs       */

   long int restore_time;  /* The last time the char did a restore all */
   int r_range_lo;   /* room range */
   int r_range_hi;
   int m_range_lo;   /* mob range  */
   int m_range_hi;
   int o_range_lo;   /* obj range  */
   int o_range_hi;
   sh_int wizinvis;  /* wizinvis level */
   sh_int condition[MAX_CONDS];
   sh_int learned[MAX_SKILL];
   time_t release_date; /* Auto-helling.. Altrag */
   char *helled_by;
   char *bio;  /* Personal Bio */
   char *prompt;  /* User config prompts */
   char *fprompt; /* Fight prompts */
   char *subprompt;  /* Substate prompt */
   sh_int pagerlen;  /* For pager (NOT menus) */

   char *alias[MAX_ALIAS]; /* alias - shogar */
   char *alias_sub[MAX_ALIAS];

   char *outputprefix;
   char *outputsuffix;

   int memorize[8];  /* Locations that can be memorized with the memorize command -- Scion */

   unsigned long balance;  /* Bank balance -- Scion */
   int bounty; /* Cash reward for pkilling this char -- Scion */
   char *family;
   int gen;
   char *spouse;
   char *maiden_name;
   char *name_disguise;
   int age_adjust;
   CHAR_DATA *mindlink;
   sh_int weapon[20];
   int rkills; /* real kills */
   int rdeaths;   /* real deaths */
   char *were_race;
   int weapon_exp[20];
   int talent_exp[MAX_DEITY];
   int points; /* unspent points for talents */
   int inborn; /* talent that the player is naturally good at */
   EXT_BV perm_aff;  /* affects that dont come off automatically */
   int noncombat[20];
   int noncombat_exp[20];
   CHAR_DATA *consenting;
   char *parent;  /* the mother,father or sire of ch */
   char *auto_attack;
   EXPLORED_AREA *first_explored;   /* what rooms have been explored */
   EXPLORED_AREA *last_explored;
   PMUT_DATA *first_mutation;
   PMUT_DATA *last_mutation;
   CHAR_DATA *magiclink;
   char *type;
   int permissions;
   char *channels;
   char *eyes;
   char *hair;
   char *skin_color;
   char *skin_type;
   char *extra_color;
   char *extra_type;
#ifdef I3
   I3_CHARDATA *i3chardata;
#endif
#ifdef IMC
   IMC_CHARDATA *imcchardata;
#endif
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		18

struct liq_type
{
   char *liq_name;
   char *liq_color;
   sh_int liq_affect[3];
};


/*
 * Damage types from the attack_table[]
 */
typedef enum
{
   DAM_HIT, DAM_SLICE, DAM_STAB, DAM_SLASH, DAM_WHIP, DAM_CLAW,
   DAM_BLAST, DAM_POUND, DAM_CRUSH, DAM_GREP, DAM_BITE, DAM_PIERCE,
   DAM_SUCTION, DAM_BOLT, DAM_ARROW, DAM_DART, DAM_STONE, DAM_PEA
} damage_types;


/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
   EXTRA_DESCR_DATA *next; /* Next in list                     */
   EXTRA_DESCR_DATA *prev; /* Previous in list                 */
   char *keyword; /* Keyword in look/examine          */
   char *description;   /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct obj_index_data
{
   OBJ_INDEX_DATA *next;
   OBJ_INDEX_DATA *next_sort;
   EXTRA_DESCR_DATA *first_extradesc;
   EXTRA_DESCR_DATA *last_extradesc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   MPROG_DATA *mudprogs;   /* objprogs */
   EXT_BV progtypes; /* objprogs */
   char *name;
   char *short_descr;
   char *description;
   char *action_desc;
   int vnum;
   sh_int item_type;
   EXT_BV extra_flags;
   int tech;
   sh_int count;
   sh_int weight;
   int cost;
   int value[7];  /* Added one extra -- Scion */
   int serial;
   sh_int layers;
   EXT_BV parts;
};


/*
 * One object.
 */
struct obj_data
{
   OBJ_DATA *next;
   OBJ_DATA *prev;
   OBJ_DATA *next_content;
   OBJ_DATA *prev_content;
   OBJ_DATA *first_content;
   OBJ_DATA *last_content;
   OBJ_DATA *in_obj;
   CHAR_DATA *carried_by;
   EXTRA_DESCR_DATA *first_extradesc;
   EXTRA_DESCR_DATA *last_extradesc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   OBJ_INDEX_DATA *pIndexData;
   ROOM_INDEX_DATA *in_room;
   char *name;
   char *short_descr;
   char *description;
   char *action_desc;
   char *obj_by;  /* Who invoked/fashioned this object? -- Scion */
   sh_int item_type;
   sh_int mpscriptpos;
   EXT_BV extra_flags;
   MPROG_ACT_LIST *mpact;  /* mudprogs */
   int mpactnum;  /* mudprogs */
   sh_int wear_loc;
   sh_int weight;
   int cost;
   int condition; /* Damage to the object -- Scion */
   sh_int timer;
   int value[7];  /* Added one extra -- Scion */
   MATERIAL_DATA *material;
   sh_int count;  /* support for object grouping */
   int serial; /* serial number         */
   int size;
   int raw_mana;
   int mana;
   EXT_BV parts;  /* which parts the item is worn on */
   OBJ_DATA *gem; /* what gem is attached to it */
};

/* Data for a material type -- Scion */
struct material_data
{
   int number; /* Number to reference the material by */
   char *name; /* Namelist of raw material */
   char *short_descr;   /* Short desc of raw material */
   char *description;   /* Long desc of raw material */
   int weight; /* Weight modifier for finished product */
   int cost;   /* Cost modifier */
   int rarity; /* How often does this material load? (percentage) */
   int sector; /* Sectory type this ore can be found in */
   int race;   /* Race that generates this ore when killed (ie. dragon
                * scales */
   EXT_BV extra_flags;  /* Same flags as objects, they get transferred to finished product */

   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;

   MATERIAL_DATA *next;
   MATERIAL_DATA *prev;

   int magic;  /* magic absorbiency -keo */
};


struct mat_shop_data
{
   int room;
   int mat[5];

   MAT_SHOP *next;
   MAT_SHOP *prev;
};

struct arena_data
{
   char *name;
   int phase;
   int vnum;
   char *winner;
   int flags;

   ROOM_INDEX_DATA *room;

   ARENA_MEMBER *first_member;
   ARENA_MEMBER *last_member;

   ARENA_DATA *next;
   ARENA_DATA *prev;
};

struct arena_member_data
{
   char *name;
   char *owner;
   int wins;
   int kills;
   int played;
   int team;
   int age;
   int hp;
   int str;
   int con;
   int dex;
   int spd;
   int end;
   int skill;
   int flags;
   int score;
   int total;

   ARENA_DATA *arena;

   BET_DATA *first_bet;
   BET_DATA *last_bet;

   ARENA_MEMBER *next;
   ARENA_MEMBER *prev;
};

struct bet_data
{
   char *better;
   int wager;
   int odds;
   unsigned long winnings;

   BET_DATA *member;

   BET_DATA *next;
   BET_DATA *prev;
};

struct moon_data
{
   char *name;
   char *world;
   char *color;
   int rise;
   int set;
   int phase;
   int type;
   bool up;
   bool waning;

   MOON_DATA *next;
   MOON_DATA *prev;
};

struct part_data
{
   int loc; /* points to specific name of part in part_locs */
   int armor;  /* ac of the part */
   int cond;   /* hp of the part */
   int flags;  /* is it broken or missing? */

   PART_DATA *prev;
   PART_DATA *next;

   OBJ_DATA *obj; /* armor worn there */
   OBJ_DATA *obj2;   /* second layer of armor */
   PART_DATA *connect_to;
};

/*
 * Exit data.
 */
struct exit_data
{
   EXIT_DATA *prev;  /* previous exit in linked list  */
   EXIT_DATA *next;  /* next exit in linked list   */
   EXIT_DATA *rexit; /* Reverse exit pointer    */
   ROOM_INDEX_DATA *to_room;  /* Pointer to destination room   */
   char *keyword; /* Keywords for exit or door  */
   char *description;   /* Description of exit     */
   int vnum;   /* Vnum of room exit leads to */
   int rvnum;  /* Vnum of room in opposite dir  */
   int exit_info; /* door states & other flags  */
   int key; /* Key vnum       */
   sh_int vdir;   /* Physical "direction"    */
   sh_int distance;  /* how far to the next room   */
   sh_int pull;   /* pull of direction (current)   */
   sh_int pulltype;  /* type of pull (current, wind)  */
   int wall;   /* type of wall, if any    */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data
{
   RESET_DATA *next;
   RESET_DATA *prev;
   RESET_DATA *first_reset;
   RESET_DATA *last_reset;
   RESET_DATA *next_reset;
   RESET_DATA *prev_reset;
   char command;
   int extra;
   int arg1;
   int arg2;
   int arg3;
   bool active;
};

/*
 * Area definition.
 */
struct area_data
{
   AREA_DATA *next;
   AREA_DATA *prev;
   AREA_DATA *next_sort;
   AREA_DATA *prev_sort;
   AREA_DATA *next_sort_name; /* Used for alphanum. sort */
   AREA_DATA *prev_sort_name; /* Ditto, Fireblade */
   ROOM_INDEX_DATA *first_room;
   ROOM_INDEX_DATA *last_room;
   char *name;
   char *filename;
   int flags;
   sh_int status; /* h, 8/11 */
   sh_int age;
   sh_int nplayer;
   sh_int reset_frequency;
   int low_r_vnum;
   int hi_r_vnum;
   int low_o_vnum;
   int hi_o_vnum;
   int low_m_vnum;
   int hi_m_vnum;
   int low_soft_range;
   int hi_soft_range;
   int low_hard_range;
   int hi_hard_range;
   char *author;  /* Scryn */
   char *resetmsg;   /* Rennard */
   sh_int max_players;
   int mkills;
   int mdeaths;
   int pkills;
   int pdeaths;
   int gold_looted;
   int illegal_pk;
   int high_economy;
   int low_economy;
   WEATHER_DATA *weather;  /* FB */
   GENERIC_MOB *first_race;
   GENERIC_MOB *last_race;
   int nmob;   /* number of mobs in the area */
   CITY_DATA *city;
};


/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct system_data
{
   int alias_wait;   /* Command wait on aliases */
   int maxplayers;   /* Maximum players this boot   */
   int alltimemax;   /* Maximum players ever   */
   char *time_of_max;   /* Time of max ever */
   char *mud_name;   /* Name of mud */
   bool NO_NAME_RESOLVING; /* Hostnames are not resolved  */
   bool DENY_NEW_PLAYERS;  /* New players cannot connect  */
   sh_int max_sn; /* Max skills */
   int save_flags;   /* Toggles for saving conditions */
   sh_int save_frequency;  /* How old to autosave someone */
   sh_int save_pets; /* Do pets save? */
   sh_int ident_retries;   /* Number of times to retry broken pipes. */
   bool crashguard;  /* Toggles crashguard on or off.. no core dumps with it on! -- Scion */
   int morts_allowed;   /* Number of characters allowed at once for multiplaying -- Scion */
   int exp_base;
   int quest;  /* mud wide quests -keo */
   char *email;   /* support email */
};



/* For blood trails mostly, but could be very useful later -- Scion */
struct trail_data
{
   TRAIL_DATA *next;
   TRAIL_DATA *prev;

   char *name;
   NATION_DATA *race;
   bool blood;
   bool fly;
   char *graffiti;

   int from;
   int to;
   time_t age;
};

/*
 * Room type.
 */
struct room_index_data
{
   ROOM_INDEX_DATA *next;
   ROOM_INDEX_DATA *next_sort;
   RESET_DATA *first_reset;
   RESET_DATA *last_reset;
   RESET_DATA *last_mob_reset;
   RESET_DATA *last_obj_reset;
   ROOM_INDEX_DATA *next_aroom;  /* Rooms within an area */
   ROOM_INDEX_DATA *prev_aroom;
   CHAR_DATA *first_person;   /* people in the room  */
   CHAR_DATA *last_person; /*      ..    */
   OBJ_DATA *first_content;   /* objects on floor    */
   OBJ_DATA *last_content; /*      ..    */
   EXTRA_DESCR_DATA *first_extradesc;  /* extra descriptions */
   EXTRA_DESCR_DATA *last_extradesc;   /*      ..    */
   AREA_DATA *area;
   EXIT_DATA *first_exit;  /* exits from the room */
   EXIT_DATA *last_exit;   /*      ..    */
   AFFECT_DATA *first_affect; /* effects on the room */
   AFFECT_DATA *last_affect;  /*      ..    */
   TRAIL_DATA *first_trail;   /* Remember who goes through the room -- Scion */
   TRAIL_DATA *last_trail;
   MAP_DATA *map; /* maps */
   MPROG_ACT_LIST *mpact;  /* mudprogs */
   int mpactnum;  /* mudprogs */
   MPROG_DATA *mudprogs;   /* mudprogs */
   sh_int mpscriptpos;
   SHOP_DATA *pShop;
   char *name;
   char *description;
   int vnum;
   int room_flags;
   EXT_BV progtypes; /* mudprogs */
   sh_int sector_type;
   int tele_vnum;
   sh_int tele_delay;
   sh_int runes;  /* rune flags */
   sh_int virtual;   /* neccessary for bilvnums shogar */
   /*
    * Advanced sector types -- Scion 
    */
   int vegetation;
   int water;
   int elevation;
   int resources;
   int curr_vegetation;
   int curr_water;
   int curr_elevation;
   int curr_resources;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000 /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000   /* allows for 1000 attack types  */
#define TYPE_RACIAL		     3000   /* allows for 1000 herb types */
#define TYPE_DISEASE		     4000   /* allows for 1000 racial types */
#define TYPE_MAGIC		     5000   /* New magic system needs a special type -- Scion */
/*
 *  Target types.
 */
typedef enum
{
   TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
   TAR_OBJ_INV
} target_types;

typedef enum
{
   SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
   SKILL_HERB, SKILL_RACIAL, SKILL_DISEASE
} skill_types;



struct timerset
{
   int num_uses;
   struct timeval total_time;
   struct timeval min_time;
   struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
   char *name; /* Name of skill     */
   sh_int skill_level[MAX_DEITY];   /* x Level needed by talent   */
   sh_int race_level[MAX_RACE];  /* Racial abilities: level      */
   SPELL_FUN *spell_fun;   /* Spell pointer (for spells) */
   DO_FUN *skill_fun;   /* Skill pointer (for skills) */
   sh_int target; /* Legal targets     */
   sh_int minimum_position;   /* Position for caster / user */
   sh_int slot;   /* Slot for #OBJECT loading   */
   sh_int min_mana;  /* Minimum mana used    */
   sh_int beats;  /* Rounds required to use skill  */
   char *noun_damage;   /* Damage message    */
   char *msg_off; /* Wear off message     */
   sh_int guild;  /* Which guild the skill belongs to */
   sh_int min_level; /* x Minimum level to be able to cast */
   sh_int type;   /* Spell/Skill/Weapon/Tongue  */
   sh_int range;  /* Range of spell (rooms)  */
   int info;   /* x Spell action/class/etc   */
   int flags;  /* x Flags        */
   char *hit_char;   /* Success message to caster  */
   char *hit_vict;   /* Success message to victim  */
   char *hit_room;   /* Success message to room */
   char *hit_dest;   /* Success message to dest room  */
   char *miss_char;  /* Failure message to caster  */
   char *miss_vict;  /* Failure message to victim  */
   char *miss_room;  /* Failure message to room */
   char *die_char;   /* Victim death msg to caster */
   char *die_vict;   /* Victim death msg to victim */
   char *die_room;   /* Victim death msg to room   */
   char *imm_char;   /* Victim immune msg to caster   */
   char *imm_vict;   /* Victim immune msg to victim   */
   char *imm_room;   /* Victim immune msg to room  */
   char *dice; /* Dice roll         */
   int value;  /* x Misc value         */
   char saves; /* What saving spell applies  */
   char difficulty;  /* Difficulty of casting/learning */
   SMAUG_AFF *affects;  /* Spell affects, if any   */
   char *components; /* Spell components, if any   */
   char *teachers;   /* x Skill requires a special teacher */
   char participants;   /* # of required participants */
   struct timerset userec; /* Usage record         */
   int stat1;  /* First stat the skill cap depends on -- Scion */
   int stat2;  /* Second stat the skill cap depends on -- Scion */
   char *talent;  /* Name of the talent this skill belongs to (spells only) */
};


/*
 * So we can have different configs for different ports -- Shaddai
 */
extern int port;

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern sh_int gsn_detrap;
extern sh_int gsn_backstab;
extern sh_int gsn_circle;
extern sh_int gsn_dodge;
extern sh_int gsn_hide;
extern sh_int gsn_peek;
extern sh_int gsn_pick_lock;
extern sh_int gsn_locksmith;
extern sh_int gsn_scan;
extern sh_int gsn_sneak;
extern sh_int gsn_steal;
extern sh_int gsn_track;
extern sh_int gsn_throw;
extern sh_int gsn_search;
extern sh_int gsn_dig;
extern sh_int gsn_mount;
extern sh_int gsn_bashdoor;
extern sh_int gsn_berserk;

extern sh_int gsn_disarm;
extern sh_int gsn_parry;
extern sh_int gsn_rescue;

extern sh_int gsn_aid;

/* used to do specific lookups */
extern sh_int gsn_first_spell;
extern sh_int gsn_first_skill;
extern sh_int gsn_first_tongue;
extern sh_int gsn_top_sn;

/* auto spells -keo */
extern sh_int gsn_counterspell;

/* spells */
extern sh_int gsn_blindness;
extern sh_int gsn_charm_person;
extern sh_int gsn_weaken;
extern sh_int gsn_corruption;
extern sh_int gsn_curse;
extern sh_int gsn_poison;
extern sh_int gsn_sticky_flame;
extern sh_int gsn_sleep;
extern sh_int gsn_mummy_rot;
extern sh_int gsn_disease;

/* magic */
extern sh_int gsn_fire;
extern sh_int gsn_water;
extern sh_int gsn_mind;
extern sh_int gsn_wind;
extern sh_int gsn_earth;
extern sh_int gsn_seeking;
extern sh_int gsn_frost;
extern sh_int gsn_security;
extern sh_int gsn_motion;
extern sh_int gsn_lightning;
extern sh_int gsn_dream;
extern sh_int gsn_illusion;
extern sh_int gsn_death;

/* newer attack skills */
extern sh_int gsn_bash;
extern sh_int gsn_stun;

extern sh_int gsn_poison_weapon;
extern sh_int gsn_scribe;
extern sh_int gsn_brew;
extern sh_int gsn_climb;

extern sh_int gsn_grip;
extern sh_int gsn_swim;

extern sh_int gsn_tumble;

extern sh_int gsn_combine;
extern sh_int gsn_dye;
extern sh_int gsn_affix;
extern sh_int gsn_repair;
extern sh_int gsn_mix;


/*
 * Cmd flag names --Shaddai
 */
extern char *const cmd_flags[];
extern char *const permit_flags[];

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))


/*
 * Old-style Bit manipulation macros
 *
 * The bit passed is the actual value of the bit (Use the BV## defines)
 */
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))
#define CH(d)			((d)->original ? (d)->original : (d)->character)

/*
 * Macros for accessing virtually unlimited bitvectors.		-Thoric
 *
 * Note that these macros use the bit number rather than the bit value
 * itself -- which means that you can only access _one_ bit at a time
 *
 * This code uses an array of integers
 */

/*
 * The functions for these prototypes can be found in misc.c
 * They are up here because they are used by the macros below
 */
bool ext_is_empty args( ( EXT_BV * bits ) );
void ext_clear_bits args( ( EXT_BV * bits ) );
int ext_has_bits args( ( EXT_BV * var, EXT_BV * bits ) );
bool ext_same_bits args( ( EXT_BV * var, EXT_BV * bits ) );
void ext_set_bits args( ( EXT_BV * var, EXT_BV * bits ) );
void ext_remove_bits args( ( EXT_BV * var, EXT_BV * bits ) );
void ext_toggle_bits args( ( EXT_BV * var, EXT_BV * bits ) );

/*
 * Here are the extended bitvector macros:
 */
#define xIS_SET(var, bit)	((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)	((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)	(ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)	((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)	(ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)	((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)	(ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)	(ext_clear_bits(&(var)))
#define xIS_EMPTY(var)		(ext_is_empty(&(var)))
#define xHAS_BITS(var, bit)	(ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)	(ext_same_bits(&(var), &(bit)))

/*
 * Memory allocation macros.
 */

/* Added the +1 thing because we were going over our spending limit
   on memory -every- time CREATE got used. It was only by one, but once we
   got up over about 110 areas and started to have regular players, things
   were crashing us quite often. -- Scion */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), (sizeof(type)))))	\
    {								\
	perror("malloc failure.");\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
    if (!((result) = (type *) realloc ((result), (sizeof(type)) * (number))))\
    {								\
	perror("realloc failure");				\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)


#define DISPOSE(point) 						\
do								\
{								\
  if ((point))							\
    free(point);						\
  point = NULL;							\
} while(0)

#ifdef HASHSTR
#define STRALLOC(point)		str_alloc((point))
#define QUICKLINK(point)	quick_link((point))
#define QUICKMATCH(p1, p2)	(int) (p1) == (int) (p2)
#define STRFREE(point)						\
do								\
{								\
    if ((point) && str_free((point))==-1) 			\
      fprintf( stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
    point = NULL;							\
} while(0)
#else
#define STRALLOC(point)		str_dup((point))
#define QUICKLINK(point)	str_dup((point))
#define QUICKMATCH(p1, p2)	strcmp((p1), (p2)) == 0
#define STRFREE(point)						\
do								\
{								\
  if ((point))							\
    free((point));						\
  point = NULL;							\
} while(0)
#endif

/* double-linked list handling macros -Thoric */
/* Updated by Scion 8/6/1999 */
/*
#define LINK(link, first, last, next, prev)                     \
do                                                              \
{                                                               \
    if ( !(first) ) {                                           \
      (first)                   = (link);                       \
      (last)					= (link);						\
    } else                                                      \
      (last)->next              = (link);                       \
    (link)->next                = NULL;                         \
    if (first == link)											\
      (link)->prev 				= NULL;							\
    else														\
      (link)->prev              = (last);                       \
    (last)                      = (link);                       \
} while(0)

#define INSERT(link, insert, first, next, prev)                 \
do                                                              \
{                                                               \
    (link)->prev                = (insert)->prev;               \
    if ( !(insert)->prev )                                      \
      (first)                   = (link);                       \
    else                                                        \
      (insert)->prev->next      = (link);                       \
    (insert)->prev              = (link);                       \
    (link)->next                = (insert);                     \
} while(0)

#define UNLINK(link, first, last, next, prev)                   \
do                                                              \
{                                                               \
	if ( !(link)->prev ) {                                      \
      (first)                   = (link)->next;                 \
	  if ((first))												\
	  (first)->prev				= NULL;							\
	} else {                                                    \
      (link)->prev->next        = (link)->next;                 \
	}															\
	if ( !(link)->next ) {                                      \
      (last)                    = (link)->prev;                 \
	  if ((last))												\
	  (last)->next				= NULL;							\
	} else {                                                    \
      (link)->next->prev        = (link)->prev;                 \
	}															\
} while(0)
*/

/* double-linked list handling macros -Thoric */

#define LINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (last)->next		= (link);			\
    (link)->next		= NULL;				\
    (link)->prev		= (last);			\
    (last)			= (link);			\
} while(0)

#define INSERT(link, insert, first, next, prev)			\
do								\
{								\
    (link)->prev		= (insert)->prev;		\
    if ( !(insert)->prev )					\
      (first)			= (link);			\
    else							\
      (insert)->prev->next	= (link);			\
    (insert)->prev		= (link);			\
    (link)->next		= (insert);			\
} while(0)

#define UNLINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(link)->prev )					\
      (first)			= (link)->next;			\
    else							\
      (link)->prev->next	= (link)->next;			\
    if ( !(link)->next )					\
      (last)			= (link)->prev;			\
    else							\
      (link)->next->prev	= (link)->prev;			\
} while(0)

#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "CHECK_LINKS: last with NULL first!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "CHECK_LINKS: first with NULL last!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)


#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
	fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\n\r", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
#define IS_NPC(ch)		(xIS_SET((ch)->act, ACT_IS_NPC) && !(ch)->pcdata)
#define IS_NEWBIE(ch)		((ch)->pcdata && ((ch)->played + \
					(current_time - (ch)->logon)) / 3600 < 10)
#define IS_WRITING(ch)          (((ch)->desc && (ch)->desc->connected) ? TRUE : FALSE)
#define IS_AFFECTED(ch, sn)	(xIS_SET((ch)->affected_by, (sn)) \
				|| (!IS_NPC(ch)	                  \
				&& xIS_SET((ch)->pcdata->perm_aff, (sn))))
#define TALENT(ch, tal)		(IS_NPC(ch) ? 0 : \
				(((ch)->curr_talent[(tal)]/3 \
				+ (ch)->talent[(tal)] \
				+ ch->mod_talent[(tal)]) * \
				(ch->pcdata->inborn == tal ? 2 : 1) \
				+ (ch->pcdata->magiclink ? \
				ch->pcdata->magiclink->talent[(tal)] \
				: 0)))
#define IS_FIGHTING(ch)		((ch)->last_hit && \
				(ch)->last_hit != (ch) && \
				(ch)->in_room == (ch)->last_hit->in_room)
#define IS_SAME_PLANE(ch, vch)	((IS_AFFECTED((ch), AFF_DREAMWORLD) ? \
				IS_AFFECTED((vch), AFF_DREAMWORLD) : \
				!IS_AFFECTED((vch), AFF_DREAMWORLD)) ? \
				(IS_AFFECTED((ch), AFF_VOID) ? \
				IS_AFFECTED((vch), AFF_VOID) : \
				!IS_AFFECTED((vch), AFF_VOID)) : \
				1==2)

#define CAN_SEE_PLANE(ch,vch)   ((IS_AFFECTED((ch), AFF_DREAMWORLD) || \
                                IS_AFFECTED((ch), AFF_VOID)) || \
                                (!IS_AFFECTED((vch), AFF_DREAMWORLD) && \
                                !IS_AFFECTED((vch), AFF_VOID)))

#define CAN_CAST(ch)		(1==1)

#define IS_VAMPIRE(ch)		(!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_VAMPIRE))
#define IS_UNDEAD(ch)		(IS_AFFECTED(ch, AFF_UNDEAD))
#define IS_GARGOYLE(ch)

#define IS_CONSENTING(ch, victim) (!IS_NPC((ch)) && !IS_NPC((victim)) \
				&& (victim == ch \
				|| (IS_SET((victim)->pcdata->permissions, PERMIT_SECURITY) || \
				((ch)->pcdata->consenting \
				&& (ch)->pcdata->consenting == (victim)))))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)
#define GET_AC(ch)		(UMAX(0, (ch)->armor))
#define GET_HITROLL(ch)		((ch)->hitroll + (int)(get_curr_per(ch)/3) \
				    + (2-(abs((ch)->mental_state)/10)))

/* Thanks to Chriss Baeke for noticing damplus was unused */
#define GET_DAMROLL(ch)		((ch)->damroll                              \
				    +(ch)->damplus			    \
				    +(int)(get_curr_str(ch)/3)	    \
				    +(((ch)->mental_state > 5		    \
				    &&(ch)->mental_state < 15) ? 1 : 0) )

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS) && !IS_SET(               \
				    (ch)->in_room->room_flags,              \
				    ROOM_TUNNEL))

#define IS_UNDERWATER(ch)	((((ch)->in_room->sector_type == SECT_WATER_SWIM \
				|| (ch)->in_room->sector_type == SECT_WATER_NOSWIM \
				|| (ch)->in_room->sector_type == SECT_UNDERWATER) \
				&& !IS_AFFECTED((ch), AFF_FLYING)) \
				|| ((ch)->in_room->sector_type == SECT_OCEANFLOOR \
				|| (ch)->in_room->sector_type == SECT_LAVA))

#define NO_WEATHER_SECT(sect)  (  sect == SECT_INSIDE || 	           \
				  sect == SECT_UNDERWATER ||               \
                                  sect == SECT_OCEANFLOOR ||               \
                                  sect == SECT_UNDERGROUND )

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_DEVOTED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->deity)

#define IS_PKILL(ch)            (1==1)

#define CAN_PKILL(ch)           (1==1)

#define IS_SILENT(ch)		(!IS_NPC(ch) && (IS_AFFECTED(ch, AFF_SILENT) || \
				xIS_SET(ch->act, PLR_SILENCE) || \
				!can_use_bodypart(ch, BP_TONGUE) || \
				IS_SET(ch->in_room->room_flags, ROOM_SILENCE)))

#define WAIT_STATE(ch, npulse) ((ch)->wait=(UMAX((ch)->wait, (npulse - \
                        (int) TALENT((ch), TAL_TIME) / 50))))

#define EXIT(ch, door)		( get_exit( (ch)->in_room, door ) )

#define CAN_GO(ch, door)	(EXIT((ch),(door))			 \
				&& (EXIT((ch),(door))->to_room != NULL)  \
                          	&& !IS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define IS_FLOATING(ch)		( IS_AFFECTED((ch), AFF_FLYING) || IS_AFFECTED((ch), AFF_FLOATING) )

#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name )

#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name )

#define IS_VALID_DISEASE(sn)	( (sn) >=0 && (sn) < MAX_DISEASE	     \
				&& disease_table[(sn)]			     \
				&& disease_table[(sn)]->name )

#define IS_PACIFIST(ch)		(IS_NPC(ch) && xIS_SET(ch->act, ACT_PACIFIST))

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )
#define SPELL_DAMAGE(skill)	( ((skill)->info      ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->info >>  3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->info >>  6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->info >>  9) & 3 )
#define SPELL_SAVE(skill)	( ((skill)->info >> 11) & 7 )
#define SET_SDAM(skill, val)	( (skill)->info =  ((skill)->info & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->info =  ((skill)->info & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->info =  ((skill)->info & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->info =  ((skill)->info & SPOW_MASK) + (((val) & 3) << 9) )
#define SET_SSAV(skill, val)	( (skill)->info =  ((skill)->info & SSAV_MASK) + (((val) & 7) << 11) )

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		(IS_VALID_SN(dt) && \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE)
#define IS_COLD(dt)		(IS_VALID_SN(dt) && \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_COLD)
#define IS_ACID(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ACID )
#define IS_ELECTRICITY(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ELECTRICITY )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )

#define IS_DRAIN(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_DRAIN )

#define IS_POISON(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_POISON )

/*
 * Object macros.
 */
#define IS_OBJ_STAT(obj, stat)	(xIS_SET((obj)->extra_flags, (stat)))

/*
 * MudProg macros.						-Thoric
 */
#define HAS_PROG(what, prog)	((what) && xIS_SET((what)->progtypes,(prog)))

/*
 * Description macros.
 */
#define PERS(ch, looker)	( (ch)->name ? (can_see( (looker), (ch) ) ? \
( IS_NPC(ch) ? (xIS_SET((ch)->act, ACT_NAMED) ? \
(ch)->short_descr : aoran(themob(ch))) : \
TALENT((looker), TAL_SEEKING) >= TALENT((ch), TAL_ILLUSION) + TALENT((ch), TAL_CHANGE) ? (ch)->name : \
ch->pcdata->name_disguise ? ch->pcdata->name_disguise \
: (ch)->name) : "Someone") : "Nobody" )

#define log_string(txt)		( log_string_plus( (txt), LOG_NORMAL, 0 ))
#define dam_message(ch, victim, dam, dt, obj, part)	(new_dam_message((ch), (victim), (dam), (dt), (obj), (part)) )

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
   CMDTYPE *next;
   char *name;
   DO_FUN *do_fun;
   int flags;  /* Added for Checking interpret stuff -Shaddai */
   sh_int position;
   sh_int log;
   struct timerset userec;
   int lag_count; /* count lag flags for this cmd - FB */
   int permit; /* permission required */
};

struct alias_data
{
   ALIAS_DATA *next;
   ALIAS_DATA *prev;
   char *name;
   char *alias;
};

/*
 * Structure for a social in the socials table.
 */
struct social_type
{
   SOCIALTYPE *next;
   char *name;
   char *char_no_arg;
   char *others_no_arg;
   char *char_found;
   char *others_found;
   char *vict_found;
   char *char_auto;
   char *others_auto;
   char *adj;
   int flags;
};



/*
 * Global constants.
 */
extern time_t last_restore_all_time;
extern time_t boot_time;   /* this should be moved down */
extern HOUR_MIN_SEC *set_boot_time;
extern struct tm *new_boot_time;
extern time_t new_boot_time_t;

extern const struct str_app_type str_app[26];
extern const struct int_app_type int_app[26];
extern const struct wis_app_type wis_app[26];
extern const struct dex_app_type dex_app[26];
extern const struct con_app_type con_app[26];
extern const struct cha_app_type cha_app[26];
extern const struct lck_app_type lck_app[26];

extern char *const part_locs[];
extern const struct liq_type liq_table[LIQ_MAX];
extern char *const attack_table[18];
extern char *const part_cond_table[7]; /* Scion */
extern char *const part_name_singular[];  /* Scion */
extern char *const magic_table[29];

extern char **const s_message_table[20];
extern char **const p_message_table[20];

extern char *const skill_tname[];
extern sh_int const movement_loss[SECT_MAX];
extern char *const dir_name[];
extern char *const rev_dir_name[];
extern char *const where_name[];
extern const sh_int rev_dir[];
extern const int trap_door[];
extern char *const r_flags[];
extern char *const w_flags[];
extern char *const o_flags[];
extern char *const a_flags[];
extern char *const o_types[];
extern char *const a_types[];
extern char *const bp_flags[];
extern char *const rune_flags[];
extern char *const act_flags[];
extern char *const plr_flags[];
extern char *const pc_flags[];
extern char *const trap_flags[];
extern char *const ris_flags[];
extern char *const trig_flags[];
extern char *const part_flags[];
extern char *const npc_race[];
extern char *const area_flags[];
extern char *const container_flags[];  /* Tagith */
extern char *const ex_pmisc[];
extern char *const ex_pwater[];
extern char *const ex_pair[];
extern char *const ex_pearth[];
extern char *const ex_pfire[];

extern char *const temp_settings[]; /* FB */
extern char *const precip_settings[];
extern char *const wind_settings[];
extern char *const mana_settings[];
extern char *const preciptemp_msg[6][6];
extern char *const windtemp_msg[6][6];
extern char *const manatemp_msg[6][6];
extern char *const precip_msg[];
extern char *const wind_msg[];
extern char *const moon_phase[];
extern char *const clearday_msg[];
extern char *const cloudyday_msg[];
extern char *const talent_name[MAX_DEITY];
extern char *const talent_rank[MAX_DEITY][12];
extern char *const lang_names[];
extern char *const weapon_bane[];
extern char *const weapon_brand[];
extern char *const pos_names[];
extern char *const song_name[MAX_SONG];
extern char *const weapon_skill[];
extern char *const noncombat_skill[];

/*
 * Global variables.
 */
extern char *bigregex;
extern char *preg;

extern char *target_name;
extern char *ranged_target_name;
extern int numobjsloaded;
extern int nummobsloaded;
extern int physicalobjects;
extern int num_descriptors;
extern struct system_data sysdata;
extern int top_sn;
extern int top_vroom;
extern int top_herb;
extern AREA_DATA *stockobj;

extern CMDTYPE *command_hash[126];
NEW_HELP_DATA *help_hash[126];

extern SKILLTYPE *skill_table[MAX_SKILL];
extern SOCIALTYPE *social_index[27];
extern CHAR_DATA *cur_char;
extern ROOM_INDEX_DATA *cur_room;
extern bool cur_char_died;
extern ch_ret global_retcode;
extern SKILLTYPE *herb_table[MAX_HERB];
extern SKILLTYPE *disease_table[MAX_DISEASE];

extern int cur_obj;
extern int cur_obj_serial;
extern bool cur_obj_extracted;
extern obj_ret global_objcode;

extern HELP_DATA *first_help;
extern HELP_DATA *last_help;
extern SHOP_DATA *first_shop;
extern SHOP_DATA *last_shop;
extern BOOK_DATA *first_book;
extern BOOK_DATA *last_book;

extern CLAN_DATA *first_clan;
extern CLAN_DATA *last_clan;

extern CHANNEL_DATA *first_channel;
extern CHANNEL_DATA *last_channel;

extern BIT_DATA *first_abit;
extern BIT_DATA *last_abit;
extern BIT_DATA *first_qbit;
extern BIT_DATA *last_qbit;

extern BAN_DATA *first_ban;
extern BAN_DATA *last_ban;
extern RESERVE_DATA *first_reserved;
extern RESERVE_DATA *last_reserved;
extern CHAR_DATA *first_char;
extern CHAR_DATA *last_char;
extern DESCRIPTOR_DATA *first_descriptor;
extern DESCRIPTOR_DATA *last_descriptor;
extern OBJ_DATA *first_object;
extern OBJ_DATA *last_object;
extern DEITY_DATA *first_deity;
extern DEITY_DATA *last_deity;
extern AREA_DATA *first_area;
extern AREA_DATA *last_area;
extern AREA_DATA *first_build;
extern AREA_DATA *last_build;
extern AREA_DATA *first_asort;
extern AREA_DATA *last_asort;
extern AREA_DATA *first_bsort;
extern AREA_DATA *last_bsort;
extern AREA_DATA *first_area_name;  /*alphanum. sort */
extern AREA_DATA *last_area_name;   /* Fireblade */

extern MATERIAL_DATA *first_material;  /* Scion */
extern MATERIAL_DATA *last_material;   /* Scion */
extern MAT_SHOP *first_matshop;  /* keo */
extern MAT_SHOP *last_matshop;   /* keo */

extern MOON_DATA *first_moon;
extern MOON_DATA *last_moon;

extern RECIPE_DATA *first_recipe;
extern RECIPE_DATA *last_recipe;
extern INGRED_DATA *first_ingred;
extern INGRED_DATA *last_ingred;

extern SPECIES_DATA *first_species;
extern SPECIES_DATA *last_species;
extern NATION_DATA *first_nation;
extern NATION_DATA *last_nation;

extern MUT_DATA *first_mutation;
extern MUT_DATA *last_mutation;

extern OBJ_DATA *extracted_obj_queue;
extern EXTRACT_CHAR_DATA *extracted_char_queue;
extern OBJ_DATA *save_equipment[MAX_WEAR][MAX_LAYERS];
extern CHAR_DATA *quitting_char;
extern CHAR_DATA *loading_char;
extern CHAR_DATA *saving_char;
extern OBJ_DATA *all_obj;

extern CITY_DATA *first_city;
extern CITY_DATA *last_city;

extern ARENA_DATA *first_arena;
extern ARENA_DATA *last_arena;

extern char bug_buf[];
extern time_t current_time;
extern bool fLogAll;
extern FILE *fpReserve;
extern FILE *fpLOG;
extern char log_buf[];
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;
extern IMMORTAL_HOST *immortal_host_start;
extern IMMORTAL_HOST *immortal_host_end;
extern int weath_unit;
extern int rand_factor;
extern int climate_factor;
extern int neigh_factor;
extern int max_vector;

extern struct act_prog_data *mob_act_list;


/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( skill_notfound );
DECLARE_DO_FUN( do_aassign );
DECLARE_DO_FUN( do_adopt );
DECLARE_DO_FUN( do_aentrance );
DECLARE_DO_FUN( do_aexit );
DECLARE_DO_FUN( do_affected );
DECLARE_DO_FUN( do_affix );
DECLARE_DO_FUN( do_afk );
DECLARE_DO_FUN( do_aid );
DECLARE_DO_FUN( do_alias );
DECLARE_DO_FUN( do_allow );
DECLARE_DO_FUN( do_alter );
DECLARE_DO_FUN( do_ansi );
DECLARE_DO_FUN( do_antimatter );
DECLARE_DO_FUN( do_apply );
DECLARE_DO_FUN( do_archmages );
DECLARE_DO_FUN( do_areas );
DECLARE_DO_FUN( do_arena );
DECLARE_DO_FUN( do_aset );
DECLARE_DO_FUN( do_astat );
DECLARE_DO_FUN( do_at );
DECLARE_DO_FUN( do_atmob );
DECLARE_DO_FUN( do_atobj );
DECLARE_DO_FUN( do_attributes );
DECLARE_DO_FUN( do_auto );
DECLARE_DO_FUN( do_award );
DECLARE_DO_FUN( do_balefire );
DECLARE_DO_FUN( do_bamfin );
DECLARE_DO_FUN( do_bamfout );
DECLARE_DO_FUN( do_ban );
DECLARE_DO_FUN( do_bandage );
DECLARE_DO_FUN( do_bash );
DECLARE_DO_FUN( do_bashdoor );
DECLARE_DO_FUN( do_beep );
DECLARE_DO_FUN( do_berserk );
DECLARE_DO_FUN( do_bestow );
DECLARE_DO_FUN( do_bind );
DECLARE_DO_FUN( do_bio );
DECLARE_DO_FUN( do_bite );
DECLARE_DO_FUN( do_blast );
DECLARE_DO_FUN( do_blink );
DECLARE_DO_FUN( do_blizzard );
DECLARE_DO_FUN( do_bloodlet );
DECLARE_DO_FUN( do_body );
DECLARE_DO_FUN( do_bodybag );
DECLARE_DO_FUN( do_boneset );
DECLARE_DO_FUN( do_bookwrite );
DECLARE_DO_FUN( do_bounty );
DECLARE_DO_FUN( do_brainshock );
DECLARE_DO_FUN( do_brand );
DECLARE_DO_FUN( do_brew );
DECLARE_DO_FUN( do_bset );
DECLARE_DO_FUN( do_bstat );
DECLARE_DO_FUN( do_bug );
DECLARE_DO_FUN( do_build );
DECLARE_DO_FUN( do_burn );
DECLARE_DO_FUN( do_bury );
DECLARE_DO_FUN( do_buy );
DECLARE_DO_FUN( do_cast );
DECLARE_DO_FUN( do_cedit );
DECLARE_DO_FUN( do_chainlightning );
DECLARE_DO_FUN( do_change );
DECLARE_DO_FUN( do_channels );
DECLARE_DO_FUN( do_chat );
DECLARE_DO_FUN( do_check_vnums );
DECLARE_DO_FUN( do_chill );
DECLARE_DO_FUN( do_chop );
DECLARE_DO_FUN( do_city );
DECLARE_DO_FUN( do_circle );
DECLARE_DO_FUN( do_clan ); /* Scion */
DECLARE_DO_FUN( do_claw );
DECLARE_DO_FUN( do_climate ); /* FB */
DECLARE_DO_FUN( do_climb );
DECLARE_DO_FUN( do_close );
DECLARE_DO_FUN( do_cmdtable );
DECLARE_DO_FUN( do_cmenu );
DECLARE_DO_FUN( do_combine );
DECLARE_DO_FUN( do_commands );
DECLARE_DO_FUN( do_compare );
DECLARE_DO_FUN( do_config );
DECLARE_DO_FUN( do_connect );
DECLARE_DO_FUN( do_consent );
DECLARE_DO_FUN( do_consider );
DECLARE_DO_FUN( do_copyover );
DECLARE_DO_FUN( do_cook );
DECLARE_DO_FUN( do_credits );
DECLARE_DO_FUN( do_cset );
DECLARE_DO_FUN( do_cure );
DECLARE_DO_FUN( do_decay );
DECLARE_DO_FUN( do_deduct );
DECLARE_DO_FUN( do_deities );
DECLARE_DO_FUN( do_delay );
DECLARE_DO_FUN( do_deny );
DECLARE_DO_FUN( do_deposit );
DECLARE_DO_FUN( do_description );
DECLARE_DO_FUN( do_destro );
DECLARE_DO_FUN( do_destroy );
DECLARE_DO_FUN( do_detonate );
DECLARE_DO_FUN( do_detrap );
DECLARE_DO_FUN( do_dig );
DECLARE_DO_FUN( do_disarm );
DECLARE_DO_FUN( do_disconnect );
DECLARE_DO_FUN( do_dismiss );
DECLARE_DO_FUN( do_dismount );
DECLARE_DO_FUN( do_disown );
DECLARE_DO_FUN( do_dispel );
DECLARE_DO_FUN( do_distort );
DECLARE_DO_FUN( do_divorce );
DECLARE_DO_FUN( do_dmesg );
DECLARE_DO_FUN( do_down );
DECLARE_DO_FUN( do_drag );
DECLARE_DO_FUN( do_drain );
DECLARE_DO_FUN( do_dream );
DECLARE_DO_FUN( do_drink );
DECLARE_DO_FUN( do_drop );
DECLARE_DO_FUN( do_diagnose );
DECLARE_DO_FUN( do_dye );
DECLARE_DO_FUN( do_east );
DECLARE_DO_FUN( do_eat );
DECLARE_DO_FUN( do_echo );
DECLARE_DO_FUN( do_elbow );
DECLARE_DO_FUN( do_electrocute );
DECLARE_DO_FUN( do_emerge );
DECLARE_DO_FUN( do_emote );
DECLARE_DO_FUN( do_empty );
DECLARE_DO_FUN( do_enchant );
DECLARE_DO_FUN( do_engrave );
DECLARE_DO_FUN( do_enter );
DECLARE_DO_FUN( do_equipment );
DECLARE_DO_FUN( do_examine );
DECLARE_DO_FUN( do_exits );
DECLARE_DO_FUN( do_experience );
DECLARE_DO_FUN( do_family );
DECLARE_DO_FUN( do_fashion );
DECLARE_DO_FUN( do_feed );
DECLARE_DO_FUN( do_fill );
DECLARE_DO_FUN( do_fixchar );
DECLARE_DO_FUN( do_flamingrain );
DECLARE_DO_FUN( do_flare );
DECLARE_DO_FUN( do_flee );
DECLARE_DO_FUN( do_flood );
DECLARE_DO_FUN( do_fly );
DECLARE_DO_FUN( do_foldarea );
DECLARE_DO_FUN( do_follow );
DECLARE_DO_FUN( do_for );
DECLARE_DO_FUN( do_force );
DECLARE_DO_FUN( do_forceclose );
DECLARE_DO_FUN( do_foreign );
DECLARE_DO_FUN( do_fprompt );
DECLARE_DO_FUN( do_fquit );   /* Gorog */
DECLARE_DO_FUN( do_form_password );
DECLARE_DO_FUN( do_freeze );
DECLARE_DO_FUN( do_frostbite );
DECLARE_DO_FUN( do_funnel );
DECLARE_DO_FUN( do_get );
DECLARE_DO_FUN( do_genarea );
DECLARE_DO_FUN( do_geyser );
DECLARE_DO_FUN( do_give );
DECLARE_DO_FUN( do_glance );
DECLARE_DO_FUN( do_gold );
DECLARE_DO_FUN( do_goto );
DECLARE_DO_FUN( do_graffiti );
DECLARE_DO_FUN( do_group );
DECLARE_DO_FUN( do_grub );
DECLARE_DO_FUN( do_gtell );
DECLARE_DO_FUN( do_guard );
DECLARE_DO_FUN( do_gwhere );
DECLARE_DO_FUN( do_heal );
DECLARE_DO_FUN( do_headbutt );
DECLARE_DO_FUN( do_hell );
DECLARE_DO_FUN( do_hide );
DECLARE_DO_FUN( do_hire );
DECLARE_DO_FUN( do_hiscore );
DECLARE_DO_FUN( do_hiscoset );
DECLARE_DO_FUN( do_hit );
DECLARE_DO_FUN( do_holyblast );
DECLARE_DO_FUN( do_holylight );
DECLARE_DO_FUN( do_home );
DECLARE_DO_FUN( do_homepage );
DECLARE_DO_FUN( do_ide );
DECLARE_DO_FUN( do_idea );
DECLARE_DO_FUN( do_identify );
DECLARE_DO_FUN( do_immobilize );
DECLARE_DO_FUN( do_induct );
DECLARE_DO_FUN( do_info );
DECLARE_DO_FUN( do_ingredient );
DECLARE_DO_FUN( do_installarea );
DECLARE_DO_FUN( do_instaroom );
DECLARE_DO_FUN( do_instazone );
DECLARE_DO_FUN( do_introduce );
DECLARE_DO_FUN( do_inventory );
DECLARE_DO_FUN( do_invis );
DECLARE_DO_FUN( do_kick );
DECLARE_DO_FUN( do_knee );
DECLARE_DO_FUN( do_kneel );
DECLARE_DO_FUN( do_last );
DECLARE_DO_FUN( do_laws );
DECLARE_DO_FUN( do_leave );
DECLARE_DO_FUN( do_light );
DECLARE_DO_FUN( do_lightning );
DECLARE_DO_FUN( do_link );
DECLARE_DO_FUN( do_list );
DECLARE_DO_FUN( do_litterbug );
DECLARE_DO_FUN( do_land );
DECLARE_DO_FUN( do_load );
DECLARE_DO_FUN( do_loadarea );
DECLARE_DO_FUN( do_loadup );
DECLARE_DO_FUN( do_lock );
DECLARE_DO_FUN( do_locksmith );
DECLARE_DO_FUN( do_log );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_low_purge );
DECLARE_DO_FUN( do_lunge );
DECLARE_DO_FUN( do_magic );
DECLARE_DO_FUN( do_magma );
DECLARE_DO_FUN( do_manastorm );
DECLARE_DO_FUN( do_mailroom );
DECLARE_DO_FUN( do_makedeity );
DECLARE_DO_FUN( do_makeore );
DECLARE_DO_FUN( do_makerooms );
DECLARE_DO_FUN( do_makeshop );
DECLARE_DO_FUN( do_marry );
DECLARE_DO_FUN( do_massign );
DECLARE_DO_FUN( do_material );
DECLARE_DO_FUN( do_matshop );
DECLARE_DO_FUN( do_memory );
DECLARE_DO_FUN( do_meteor );
DECLARE_DO_FUN( do_mcreate );
DECLARE_DO_FUN( do_mdelete );
DECLARE_DO_FUN( do_mfind );
DECLARE_DO_FUN( do_mindflay );
DECLARE_DO_FUN( do_minvoke );
DECLARE_DO_FUN( do_mix );
DECLARE_DO_FUN( do_mlist );
DECLARE_DO_FUN( do_moblog );
DECLARE_DO_FUN( do_moon );
DECLARE_DO_FUN( do_morph );
DECLARE_DO_FUN( do_mount );
DECLARE_DO_FUN( do_motd );
DECLARE_DO_FUN( do_mset );
DECLARE_DO_FUN( do_mstat );
DECLARE_DO_FUN( do_mutation );
DECLARE_DO_FUN( do_mwhere );
DECLARE_DO_FUN( do_namegen );
DECLARE_DO_FUN( do_nation );
DECLARE_DO_FUN( do_neighbors );
DECLARE_DO_FUN( do_news );
DECLARE_DO_FUN( do_new_help );
DECLARE_DO_FUN( do_nightmare );
DECLARE_DO_FUN( do_nochannel );
DECLARE_DO_FUN( do_noemote );
DECLARE_DO_FUN( do_noresolve );
DECLARE_DO_FUN( do_north );
DECLARE_DO_FUN( do_northeast );
DECLARE_DO_FUN( do_northwest );
DECLARE_DO_FUN( do_notell );
DECLARE_DO_FUN( do_notitle );
DECLARE_DO_FUN( do_nuisance );
DECLARE_DO_FUN( do_oassign );
DECLARE_DO_FUN( do_ocopy );   /* tag */
DECLARE_DO_FUN( do_ocreate );
DECLARE_DO_FUN( do_odelete );
DECLARE_DO_FUN( do_ofind );
DECLARE_DO_FUN( do_ogrub );
DECLARE_DO_FUN( do_oinvoke );
DECLARE_DO_FUN( do_olist );
DECLARE_DO_FUN( do_ooedit );  /* Tag */
DECLARE_DO_FUN( do_omedit );  /* Tag */
DECLARE_DO_FUN( do_oredit );  /* Tag */
DECLARE_DO_FUN( do_open );
DECLARE_DO_FUN( do_ooc_say );
DECLARE_DO_FUN( do_order );
DECLARE_DO_FUN( do_oset );
DECLARE_DO_FUN( do_osearch );
DECLARE_DO_FUN( do_ostat );
DECLARE_DO_FUN( do_otransfer );
DECLARE_DO_FUN( do_outcast );
DECLARE_DO_FUN( do_outputprefix );
DECLARE_DO_FUN( do_outputsuffix );
DECLARE_DO_FUN( do_owhere );
DECLARE_DO_FUN( do_pacify );
DECLARE_DO_FUN( do_pager );
DECLARE_DO_FUN( do_password );
DECLARE_DO_FUN( do_plant );
DECLARE_DO_FUN( do_player_echo );
DECLARE_DO_FUN( do_pcrename );
DECLARE_DO_FUN( do_peace );
DECLARE_DO_FUN( do_permit );
DECLARE_DO_FUN( do_pick );
DECLARE_DO_FUN( do_piss );
DECLARE_DO_FUN( do_pound );
DECLARE_DO_FUN( do_poisoncloud );
DECLARE_DO_FUN( do_pose );
DECLARE_DO_FUN( do_practice );
DECLARE_DO_FUN( do_preserve );
DECLARE_DO_FUN( do_protect ); /* Scion */
DECLARE_DO_FUN( do_prompt );
DECLARE_DO_FUN( do_pry );
DECLARE_DO_FUN( do_pull );
DECLARE_DO_FUN( do_punch );
DECLARE_DO_FUN( do_purge );
DECLARE_DO_FUN( do_purify );
DECLARE_DO_FUN( do_push );
DECLARE_DO_FUN( do_put );
DECLARE_DO_FUN( do_pyrokinetics );
DECLARE_DO_FUN( do_abit );
DECLARE_DO_FUN( do_qbit );
DECLARE_DO_FUN( do_quaff );
DECLARE_DO_FUN( do_quake );
DECLARE_DO_FUN( do_qui );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_races );
DECLARE_DO_FUN( do_radio );
DECLARE_DO_FUN( do_rank );
DECLARE_DO_FUN( do_rap );
DECLARE_DO_FUN( do_rassign );
DECLARE_DO_FUN( do_rat );
DECLARE_DO_FUN( do_rdelete );
DECLARE_DO_FUN( do_ready );
DECLARE_DO_FUN( do_rebirth );
DECLARE_DO_FUN( do_reboo );
DECLARE_DO_FUN( do_reboot );
DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_recho );
DECLARE_DO_FUN( do_recipe );
DECLARE_DO_FUN( do_recite );
DECLARE_DO_FUN( do_redit );
DECLARE_DO_FUN( do_regoto );
DECLARE_DO_FUN( do_relax );
DECLARE_DO_FUN( do_remove );
DECLARE_DO_FUN( do_rent );
DECLARE_DO_FUN( do_repair );
DECLARE_DO_FUN( do_reply );
DECLARE_DO_FUN( do_report );
DECLARE_DO_FUN( do_rescue );
DECLARE_DO_FUN( do_reserve );
DECLARE_DO_FUN( do_reset );
DECLARE_DO_FUN( do_reshape );
DECLARE_DO_FUN( do_rest );
DECLARE_DO_FUN( do_restore );
DECLARE_DO_FUN( do_restoretime );
DECLARE_DO_FUN( do_resurrect );
DECLARE_DO_FUN( do_retell );
DECLARE_DO_FUN( do_retran );
DECLARE_DO_FUN( do_retrieve );
DECLARE_DO_FUN( do_return );
DECLARE_DO_FUN( do_revert );
DECLARE_DO_FUN( do_rgrub );
DECLARE_DO_FUN( do_rip );
DECLARE_DO_FUN( do_rfind );
DECLARE_DO_FUN( do_rlist );
DECLARE_DO_FUN( do_roleplay );
DECLARE_DO_FUN( do_roll );
DECLARE_DO_FUN( do_rset );
DECLARE_DO_FUN( do_rstat );
DECLARE_DO_FUN( do_rune );
DECLARE_DO_FUN( do_sandstorm );
DECLARE_DO_FUN( do_save );
DECLARE_DO_FUN( do_saveall );
DECLARE_DO_FUN( do_savearea );
DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_scan );
DECLARE_DO_FUN( do_scatter );
DECLARE_DO_FUN( do_score );
DECLARE_DO_FUN( do_scribe );
DECLARE_DO_FUN( do_search );
DECLARE_DO_FUN( do_sedit );
DECLARE_DO_FUN( do_seek );
DECLARE_DO_FUN( do_sell );
DECLARE_DO_FUN( do_set_boot_time );
DECLARE_DO_FUN( do_setdeity );
DECLARE_DO_FUN( do_set_book );
DECLARE_DO_FUN( do_set_help );
DECLARE_DO_FUN( do_setweather );
DECLARE_DO_FUN( do_sheathe );
DECLARE_DO_FUN( do_shield );
DECLARE_DO_FUN( do_shock );
DECLARE_DO_FUN( do_shoot );
DECLARE_DO_FUN( do_shops );
DECLARE_DO_FUN( do_shopset );
DECLARE_DO_FUN( do_shopstat );
DECLARE_DO_FUN( do_shove );
DECLARE_DO_FUN( do_showabit );
DECLARE_DO_FUN( do_showqbit );
DECLARE_DO_FUN( do_setabit );
DECLARE_DO_FUN( do_setqbit );
DECLARE_DO_FUN( do_showdeity );
DECLARE_DO_FUN( do_showweather );   /* FB */
DECLARE_DO_FUN( do_shutdow );
DECLARE_DO_FUN( do_shutdown );
DECLARE_DO_FUN( do_silence );
DECLARE_DO_FUN( do_sing );
DECLARE_DO_FUN( do_sit );
DECLARE_DO_FUN( do_skills );
DECLARE_DO_FUN( do_skin );
DECLARE_DO_FUN( do_slap );
DECLARE_DO_FUN( do_slash );
DECLARE_DO_FUN( do_slay );
DECLARE_DO_FUN( do_sleep );
DECLARE_DO_FUN( do_slookup );
DECLARE_DO_FUN( do_smite );
DECLARE_DO_FUN( do_smoke );
DECLARE_DO_FUN( do_sneak );
DECLARE_DO_FUN( do_snoop );
DECLARE_DO_FUN( do_sober );
DECLARE_DO_FUN( do_socials );
DECLARE_DO_FUN( do_sonics );
DECLARE_DO_FUN( do_soulfire );
DECLARE_DO_FUN( do_south );
DECLARE_DO_FUN( do_southeast );
DECLARE_DO_FUN( do_southwest );
DECLARE_DO_FUN( do_squat );
DECLARE_DO_FUN( do_species );
DECLARE_DO_FUN( do_spike );
DECLARE_DO_FUN( do_splash );
DECLARE_DO_FUN( do_split );
DECLARE_DO_FUN( do_sset );
DECLARE_DO_FUN( do_stab );
DECLARE_DO_FUN( do_stand );
DECLARE_DO_FUN( do_status );
DECLARE_DO_FUN( do_steal );
DECLARE_DO_FUN( do_steam );
DECLARE_DO_FUN( do_sting );
DECLARE_DO_FUN( do_storm );
DECLARE_DO_FUN( do_story );
DECLARE_DO_FUN( do_strew );
DECLARE_DO_FUN( do_strip );
DECLARE_DO_FUN( do_summon );
DECLARE_DO_FUN( do_sunstroke );
DECLARE_DO_FUN( do_switch );
DECLARE_DO_FUN( do_sworddance );
DECLARE_DO_FUN( do_showlayers );
DECLARE_DO_FUN( do_suicide );
DECLARE_DO_FUN( do_taint );
DECLARE_DO_FUN( do_talent_advance );
DECLARE_DO_FUN( do_tap );
DECLARE_DO_FUN( do_tail );
DECLARE_DO_FUN( do_tamp );
DECLARE_DO_FUN( do_telekinetics );
DECLARE_DO_FUN( do_tell );
DECLARE_DO_FUN( do_think );
DECLARE_DO_FUN( do_throw );
DECLARE_DO_FUN( do_time );
DECLARE_DO_FUN( do_timecmd );
DECLARE_DO_FUN( do_title );
DECLARE_DO_FUN( do_track );
DECLARE_DO_FUN( do_tradein );
DECLARE_DO_FUN( do_train );
DECLARE_DO_FUN( do_transfer );
DECLARE_DO_FUN( do_travel );
DECLARE_DO_FUN( do_turn );
DECLARE_DO_FUN( do_typo );
DECLARE_DO_FUN( do_unfoldarea );
DECLARE_DO_FUN( do_unhell );
DECLARE_DO_FUN( do_unholyblast );
DECLARE_DO_FUN( do_unlock );
DECLARE_DO_FUN( do_unnuisance );
DECLARE_DO_FUN( do_unsilence );
DECLARE_DO_FUN( do_up );
DECLARE_DO_FUN( do_users );
DECLARE_DO_FUN( do_value );
DECLARE_DO_FUN( do_ventriloquate );
DECLARE_DO_FUN( do_vines );
DECLARE_DO_FUN( do_vnums );
DECLARE_DO_FUN( do_void );
DECLARE_DO_FUN( do_vsearch );
DECLARE_DO_FUN( do_wail );
DECLARE_DO_FUN( do_wake );
DECLARE_DO_FUN( do_weapons );
DECLARE_DO_FUN( do_wear );
DECLARE_DO_FUN( do_weather );
DECLARE_DO_FUN( do_west );
DECLARE_DO_FUN( do_where );
DECLARE_DO_FUN( do_whip );
DECLARE_DO_FUN( do_whisper );
DECLARE_DO_FUN( do_who );
DECLARE_DO_FUN( do_whois );
DECLARE_DO_FUN( do_wield );
DECLARE_DO_FUN( do_wind );
DECLARE_DO_FUN( do_withdraw );
DECLARE_DO_FUN( do_wizhelp );
DECLARE_DO_FUN( do_wizlock );
DECLARE_DO_FUN( do_zones );

/* mob prog stuff */
DECLARE_DO_FUN( do_mp_close_passage );
DECLARE_DO_FUN( do_mp_damage );
DECLARE_DO_FUN( do_mp_log );
DECLARE_DO_FUN( do_mp_restore );
DECLARE_DO_FUN( do_mp_open_passage );
DECLARE_DO_FUN( do_mp_practice );
DECLARE_DO_FUN( do_mp_slay );
DECLARE_DO_FUN( do_mpaset );
DECLARE_DO_FUN( do_mpqset );
DECLARE_DO_FUN( do_mpadvance );
DECLARE_DO_FUN( do_mpaddpoints );
DECLARE_DO_FUN( do_mpasound );
DECLARE_DO_FUN( do_mpasupress );
DECLARE_DO_FUN( do_mpat );
DECLARE_DO_FUN( do_mpdream );
DECLARE_DO_FUN( do_mp_deposit );
DECLARE_DO_FUN( do_mp_fill_in );
DECLARE_DO_FUN( do_mp_withdraw );
DECLARE_DO_FUN( do_mpecho );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat );
DECLARE_DO_FUN( do_mpechozone );
DECLARE_DO_FUN( do_mpedit );
DECLARE_DO_FUN( do_mrange );
DECLARE_DO_FUN( do_opedit );
DECLARE_DO_FUN( do_orange );
DECLARE_DO_FUN( do_rpedit );
DECLARE_DO_FUN( do_mpforce );
DECLARE_DO_FUN( do_mpinvis );
DECLARE_DO_FUN( do_mpgoto );
DECLARE_DO_FUN( do_mpjunk );
DECLARE_DO_FUN( do_mpkill );
DECLARE_DO_FUN( do_mpmload );
DECLARE_DO_FUN( do_mpmset );
DECLARE_DO_FUN( do_mpnothing );
DECLARE_DO_FUN( do_mpoload );
DECLARE_DO_FUN( do_mposet );
DECLARE_DO_FUN( do_mppardon );
DECLARE_DO_FUN( do_mppeace );
DECLARE_DO_FUN( do_mppurge );
DECLARE_DO_FUN( do_mpstat );
DECLARE_DO_FUN( do_opstat );
DECLARE_DO_FUN( do_rpstat );
DECLARE_DO_FUN( do_mptransfer );
DECLARE_DO_FUN( do_mpnuisance );
DECLARE_DO_FUN( do_mpunnuisance );
DECLARE_DO_FUN( do_mpbodybag );
DECLARE_DO_FUN( do_mpmakeore );
DECLARE_DO_FUN( do_mpmakeitem );
DECLARE_DO_FUN( do_mpscatter );
DECLARE_DO_FUN( do_mpdelay );
DECLARE_DO_FUN( do_mpsellrand );
DECLARE_DO_FUN( do_mpsetquest );
DECLARE_DO_FUN( do_mpshowtalents );
DECLARE_DO_FUN( do_mpsound );
DECLARE_DO_FUN( do_mpsoundaround );
DECLARE_DO_FUN( do_mpsoundat );
DECLARE_DO_FUN( do_mpmusic );
DECLARE_DO_FUN( do_mpmusicaround );
DECLARE_DO_FUN( do_mpmusicat );

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN( spell_null );
DECLARE_SPELL_FUN( spell_notfound );
DECLARE_SPELL_FUN( spell_blindness );
DECLARE_SPELL_FUN( spell_cure_blindness );
DECLARE_SPELL_FUN( spell_cure_poison );
DECLARE_SPELL_FUN( spell_curse );
DECLARE_SPELL_FUN( spell_pigeon );
DECLARE_SPELL_FUN( spell_poison );
DECLARE_SPELL_FUN( spell_remove_curse );
DECLARE_SPELL_FUN( spell_sleep );
DECLARE_SPELL_FUN( spell_smaug );
DECLARE_SPELL_FUN( spell_portal );

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/"   /* Player files         */
#define BACKUP_DIR	"../backup/"   /* Backup Player files    */
#define GOD_DIR		"../gods/"  /* God Info Dir         */
#define DEITY_DIR	"../deity/" /* Deity data dir    */
#define BUILD_DIR       "../building/" /* Online building save dir     */
#define SYSTEM_DIR	"../system/"   /* Main system files    */
#define PROG_DIR	"mudprogs/" /* MUDProg files     */
#define CORPSE_DIR	"../corpses/"  /* Corpses        */
#ifdef WIN32
#define NULL_FILE	"nul" /* To reserve one stream        */
#else
#define NULL_FILE	"/dev/null" /* To reserve one stream        */
#endif
#define CLAN_DIR	"../clans/" /* Clan files -- Scion */
#define AREA_LIST	"area.lst"  /* List of areas     */
#define BAN_LIST        "ban.lst"   /* List of bans                 */
#define RESERVED_LIST	"reserved.lst" /* List of reserved names  */
#define GOD_LIST	"gods.lst"  /* List of gods         */
#define DEITY_LIST	"deity.lst" /* List of deities      */
#define	RACE_LIST	"race.lst"  /* List of races     */
#define CLAN_LIST	"clan.lst"  /* List of clans     */

#define SHUTDOWN_FILE	"shutdown.txt" /* For 'shutdown'  */
#define OBJ_FILE		"objects.dat"  /* For storing objects across reboots */
#define MOB_FILE		"mobs.dat"  /* For storing mobs across reboots */

#define RIPSCREEN_FILE	SYSTEM_DIR "mudrip.rip"
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"   /* Boot up error file  */
#define BUG_FILE	SYSTEM_DIR "bugs.txt"   /* For bug( )          */
#define PBUG_FILE	SYSTEM_DIR "pbugs.txt"  /* For 'bug' command   */
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"  /* For 'idea'       */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"  /* For 'typo'       */
#define RP_FILE		SYSTEM_DIR "rp.txt"  /* For 'rp'            */
#define LOG_FILE	SYSTEM_DIR "log.txt" /* For talking in logged rooms */
#define MOBLOG_FILE	SYSTEM_DIR "moblog.txt" /* For mplog messages  */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST" /* Wizlist       */
#define WHO_FILE	SYSTEM_DIR "WHO"  /* Who output file  */
#define WEBWHO_FILE	SYSTEM_DIR "WEBWHO"  /* WWW Who output file */
#define SKILL_FILE	SYSTEM_DIR "skills.dat" /* Skill table   */
#define HERB_FILE	SYSTEM_DIR "herbs.dat"  /* Herb table       */
#define TONGUE_FILE	SYSTEM_DIR "tongues.dat"   /* Tongue tables    */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"   /* Socials       */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat"  /* Commands      */
#define HELP_FILE		SYSTEM_DIR "help.dat"   /* Help Files      */
#define USAGE_FILE	SYSTEM_DIR "usage.txt"  /* How many people are on
                                              * every half hour - trying to
                                              * determine best reboot time */
#define ECONOMY_FILE	SYSTEM_DIR "economy.txt"   /* Gold looted, value of
                                                 * used potions/pills  */
#define RACEDIR 	"../races/"
#define COPYOVER_FILE	SYSTEM_DIR "copyover.dat"  /* for warm reboots    */
#define EXE_FILE	"../src/rmexe" /* executable path  */
#define MATERIAL_FILE SYSTEM_DIR "materials.dat"   /* for materials - Scion */
#define MATSHOP_FILE	SYSTEM_DIR "matshops.dat"
#define MOON_FILE	SYSTEM_DIR "moons.dat"  /* for moons -keo */
#define	RECIPE_FILE	SYSTEM_DIR "recipes.dat"
#define	INGRED_FILE	SYSTEM_DIR "ingreds.dat"
#define	SPECIES_FILE	SYSTEM_DIR "species.dat"
#define NATION_FILE	SYSTEM_DIR "nation.dat"
#define MUT_FILE	SYSTEM_DIR "mutations.dat"
#define CITY_FILE	SYSTEM_DIR "cities.dat"
#define CHANNEL_FILE	SYSTEM_DIR "channels.dat"
#define ARENA_FILE	SYSTEM_DIR "arenas.dat"
#define BOOK_FILE		SYSTEM_DIR "book.dat"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define DE	DEITY_DATA
#define SK	SKILLTYPE
#define PD	PART_DATA

/* act_comm.c */
bool circle_follow args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void add_follower args( ( CHAR_DATA * ch, CHAR_DATA * master ) );
void stop_follower args( ( CHAR_DATA * ch ) );
void die_follower args( ( CHAR_DATA * ch ) );
bool is_same_group args( ( CHAR_DATA * ach, CHAR_DATA * bch ) );
void send_rip_screen args( ( CHAR_DATA * ch ) );
void to_channel args( ( const char *argument, const char *verb, sh_int level ) );
char *obj_short args( ( OBJ_DATA * obj ) );
void init_profanity_checker args( ( void ) );
void init_signals args( ( void ) );
void do_crashguard args( ( void ) );
void sig_handler args( ( int sig ) );

/* act_info.c */
int get_door args( ( char *arg ) );
char *format_obj_to_char args( ( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort ) );
void show_list_to_char args( ( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing ) );
bool is_ignoring args( ( CHAR_DATA * ch, CHAR_DATA * ign_ch ) );
void show_race_line args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );

/* act_move.c */
void clear_vrooms args( ( void ) );
ED *find_door args( ( CHAR_DATA * ch, char *arg, bool quiet ) );
ED *get_exit args( ( ROOM_INDEX_DATA * room, sh_int dir ) );
ED *get_exit_to args( ( ROOM_INDEX_DATA * room, sh_int dir, int vnum ) );
ED *get_exit_num args( ( ROOM_INDEX_DATA * room, sh_int count ) );
ch_ret move_char args( ( CHAR_DATA * ch, EXIT_DATA * pexit, int fall ) );
sh_int encumbrance args( ( CHAR_DATA * ch, sh_int move ) );
bool will_fall args( ( CHAR_DATA * ch, int fall ) );
ch_ret pullcheck args( ( CHAR_DATA * ch, int pulse ) );
char *rev_exit args( ( sh_int vdir ) );

/* act_obj.c */

obj_ret damage_obj args( ( OBJ_DATA * obj ) );
sh_int get_obj_resistance args( ( OBJ_DATA * obj ) );
void obj_fall args( ( OBJ_DATA * obj, bool through ) );

/* act_wiz.c */
RID *find_location args( ( CHAR_DATA * ch, char *arg ) );
void echo_to_all args( ( sh_int AT_COLOR, char *argument, sh_int tar ) );
void get_reboot_string args( ( void ) );
struct tm *update_time args( ( struct tm * old_time ) );
void free_social args( ( SOCIALTYPE * social ) );
void add_social args( ( SOCIALTYPE * social ) );
void free_command args( ( CMDTYPE * command ) );
void unlink_command args( ( CMDTYPE * command ) );
void add_command args( ( CMDTYPE * command ) );

/* birth.c */
bool check_rebirth args( ( CHAR_DATA * ch, int i ) );

/* build.c */
int get_cmdflag args( ( char *flag ) );
char *flag_string args( ( int bitvector, char *const flagarray[] ) );
char *ext_flag_string args( ( EXT_BV * bitvector, char *const flagarray[] ) );
int get_mpflag args( ( char *flag ) );
int get_dir args( ( char *txt ) );
char *strip_cr args( ( char *str ) );
char *strip_crn args( ( char *str ) );
char *strip_crns args( ( char *str ) );

/* deity.c */
DE *get_deity args( ( char *name ) );
void load_deity args( ( void ) );
void save_deity args( ( DEITY_DATA * deity ) );

/* comm.c */
char *smaug_crypt( const char *pwd );
char *color_align( char *argument, int size, int align );
const char *const_color_align( const char *argument, int size, int align );
int color_str_len( char *argument );
int const_color_str_len( const char *argument );
int color_strnlen( char *argument, int maxlength );
int const_color_strnlen( const char *argument, int maxlength );
void close_socket args( ( DESCRIPTOR_DATA * dclose, bool force ) );
void write_to_buffer args( ( DESCRIPTOR_DATA * d, const char *txt, int length ) );
void write_to_pager args( ( DESCRIPTOR_DATA * d, const char *txt, int length ) );
void send_to_char args( ( const char *txt, CHAR_DATA * ch ) );
void send_to_char_color args( ( const char *txt, CHAR_DATA * ch ) );
void send_to_pager args( ( const char *txt, CHAR_DATA * ch ) );
void send_to_pager_color args( ( const char *txt, CHAR_DATA * ch ) );
void set_char_color args( ( sh_int AType, CHAR_DATA * ch ) );
void set_pager_color args( ( sh_int AType, CHAR_DATA * ch ) );
void ch_printf args( ( CHAR_DATA * ch, char *fmt, ... ) );
void ch_printf_color args( ( CHAR_DATA * ch, char *fmt, ... ) );
void pager_printf args( ( CHAR_DATA * ch, char *fmt, ... ) );
void pager_printf_color args( ( CHAR_DATA * ch, char *fmt, ... ) );
void act args( ( sh_int AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type ) );
void log_printf args( ( char *fmt, ... ) );
void copyover_recover args( ( void ) );

char *myobj args( ( OBJ_DATA * obj ) );
char *themob args( ( CHAR_DATA * mob ) );
char *obj_short args( ( OBJ_DATA * obj ) );

/* reset.c */
RD *make_reset args( ( char letter, int extra, int arg1, int arg2, int arg3 ) );
RD *add_reset( ROOM_INDEX_DATA * room, char letter, int extra, int arg1, int arg2, int arg3 );
void reset_area args( ( AREA_DATA * pArea ) );

/* db.c */
void show_file args( ( CHAR_DATA * ch, char *filename ) );
char *str_dup args( ( char const *str ) );
void boot_db args( ( void ) );
void area_update args( ( void ) );
void add_char args( ( CHAR_DATA * ch ) );
CD *create_mobile args( ( MOB_INDEX_DATA * pMobIndex ) );
OD *create_object args( ( OBJ_INDEX_DATA * pObjIndex, int level ) );
void clear_char args( ( CHAR_DATA * ch ) );
void free_char args( ( CHAR_DATA * ch ) );
char *get_extra_descr args( ( const char *name, EXTRA_DESCR_DATA * ed ) );
MID *get_mob_index args( ( int vnum ) );
OID *get_obj_index args( ( int vnum ) );
RID *get_room_index args( ( int vnum ) );
char fread_letter args( ( FILE * fp ) );
int fread_number args( ( FILE * fp ) );
EXT_BV fread_bitvector args( ( FILE * fp ) );
void fwrite_bitvector args( ( EXT_BV * bits, FILE * fp ) );
char *print_bitvector args( ( EXT_BV * bits ) );
char *fread_string args( ( FILE * fp ) );
char *fread_string_nohash args( ( FILE * fp ) );
void fread_to_eol args( ( FILE * fp ) );
char *fread_word args( ( FILE * fp ) );
char *fread_line args( ( FILE * fp ) );
int number_fuzzy args( ( int number ) );
int number_range args( ( int from, int to ) );
int number_percent args( ( void ) );
int number_door args( ( void ) );
int number_bits args( ( int width ) );
int number_mm args( ( void ) );
int dice args( ( int number, int size ) );
void smash_tilde args( ( char *str ) );
void hide_tilde args( ( char *str ) );
char *show_tilde args( ( char *str ) );
bool str_cmp args( ( const char *astr, const char *bstr ) );
bool str_prefix args( ( const char *astr, const char *bstr ) );
bool str_infix args( ( const char *astr, const char *bstr ) );
bool str_suffix args( ( const char *astr, const char *bstr ) );
char *capitalize args( ( const char *str ) );
char *strlower args( ( const char *str ) );
char *strupper args( ( const char *str ) );
char *aoran args( ( const char *str ) );
void append_file args( ( CHAR_DATA * ch, char *file, char *str ) );
void append_to_file args( ( char *file, char *str ) );
void bug args( ( const char *str, ... ) );
void log_string_plus args( ( const char *str, sh_int log_type, sh_int level ) );
RID *make_room args( ( int vnum ) );
OID *make_object args( ( int vnum, int cvnum, char *name ) );
MID *make_mobile args( ( int vnum, int cvnum, char *name ) );
ED *make_exit args( ( ROOM_INDEX_DATA * pRoomIndex, ROOM_INDEX_DATA * to_room, sh_int door ) );
void add_help args( ( HELP_DATA * pHelp ) );
void fix_area_exits args( ( AREA_DATA * tarea ) );
void load_area_file args( ( AREA_DATA * tarea, char *filename ) );
void randomize_exits args( ( ROOM_INDEX_DATA * room, sh_int maxdir ) );
void make_wizlist args( ( void ) );
void tail_chain args( ( void ) );
bool delete_room args( ( ROOM_INDEX_DATA * room ) );
bool delete_obj args( ( OBJ_INDEX_DATA * obj ) );
bool delete_mob args( ( MOB_INDEX_DATA * mob ) );
/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	args( ( MOB_INDEX_DATA *pMob ) );
void	obj_sort	args( ( OBJ_INDEX_DATA *pObj ) );
void	room_sort	args( ( ROOM_INDEX_DATA *pRoom ) );*/
void sort_area args( ( AREA_DATA * pArea, bool proto ) );
void sort_area_by_name args( ( AREA_DATA * pArea ) ); /* Fireblade */

/* build.c */
void start_editing args( ( CHAR_DATA * ch, char *data ) );
void stop_editing args( ( CHAR_DATA * ch ) );
void edit_buffer args( ( CHAR_DATA * ch, char *argument ) );
char *copy_buffer args( ( CHAR_DATA * ch ) );
bool can_rmodify args( ( CHAR_DATA * ch, ROOM_INDEX_DATA * room ) );
bool can_omodify args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
bool can_mmodify args( ( CHAR_DATA * ch, CHAR_DATA * mob ) );
bool can_medit args( ( CHAR_DATA * ch, MOB_INDEX_DATA * mob ) );
void free_reset args( ( AREA_DATA * are, RESET_DATA * res ) );
void free_area args( ( AREA_DATA * are ) );
void assign_area args( ( CHAR_DATA * ch ) );
EDD *SetRExtra args( ( ROOM_INDEX_DATA * room, char *keywords ) );
bool DelRExtra args( ( ROOM_INDEX_DATA * room, char *keywords ) );
EDD *SetOExtra args( ( OBJ_DATA * obj, char *keywords ) );
bool DelOExtra args( ( OBJ_DATA * obj, char *keywords ) );
EDD *SetOExtraProto args( ( OBJ_INDEX_DATA * obj, char *keywords ) );
bool DelOExtraProto args( ( OBJ_INDEX_DATA * obj, char *keywords ) );
void fold_area args( ( AREA_DATA * tarea, char *filename, bool install ) );
int get_otype args( ( char *type ) );
int get_atype args( ( char *type ) );
int get_aflag args( ( char *flag ) );
int get_oflag args( ( char *flag ) );
int get_wflag args( ( char *flag ) );
void init_area_weather args( ( void ) );
void save_weatherdata args( ( void ) );

/* bodyparts.c */
void hurt_part args( ( CHAR_DATA * ch, PART_DATA * part, int dam ) );
void check_bodyparts args( ( CHAR_DATA * ch ) );
PD *find_bodypart args( ( CHAR_DATA * ch, int loc ) );
bool has_bodypart args( ( CHAR_DATA * ch, int loc ) );
bool can_use_bodypart args( ( CHAR_DATA * ch, int loc ) );
OD *outer_layer args( ( PART_DATA * part ) );
char *hands args( ( CHAR_DATA * ch ) );
int find_part_name args( ( char *argument ) );
void show_equip args( ( CHAR_DATA * ch, CHAR_DATA * to ) );
void obj_affect_ch args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void obj_unaffect_ch args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
bool equip_char args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void unequip_char args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void break_part args( ( CHAR_DATA * ch, PART_DATA * part ) );

/* combat.c */
void die args( ( CHAR_DATA * ch ) );
void uncon args( ( CHAR_DATA * ch ) );
void gain_exp args( ( CHAR_DATA * ch, int gain ) );
bool lose_hp args( ( CHAR_DATA * ch, int amt ) );
bool lose_ep args( ( CHAR_DATA * ch, int amt ) );
void check_hit args( ( CHAR_DATA * ch, CHAR_DATA * victim, int part, int dam, int type ) );
void mob_attack args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
OD *get_weapon args( ( CHAR_DATA * ch, int skill ) );
bool direct_damage args( ( CHAR_DATA * ch, int amt ) );
void pain args( ( CHAR_DATA * ch, int dam ) );
void stop_fighting args( ( CHAR_DATA * ch, bool fBoth ) );

/* fight.c */
int max_fight args( ( CHAR_DATA * ch ) );
void violence_update args( ( void ) );
ch_ret multi_hit args( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) );
ch_ret projectile_hit args( ( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * wield, OBJ_DATA * projectile, sh_int dist ) );
sh_int ris_damage args( ( CHAR_DATA * ch, sh_int dam, int ris ) );
ch_ret damage args( ( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt ) );
void update_pos args( ( CHAR_DATA * victim ) );
void death_cry args( ( CHAR_DATA * ch ) );
void stop_hunting args( ( CHAR_DATA * ch ) );
void stop_hating args( ( CHAR_DATA * ch ) );
void stop_fearing args( ( CHAR_DATA * ch ) );
void start_hunting args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void start_hating args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void start_fearing args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool is_hunting args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool is_hating args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool is_fearing args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool is_safe args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool legal_loot args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
sh_int VAMP_AC args( ( CHAR_DATA * ch ) );
bool check_illegal_pk args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void raw_kill args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool can_astral args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );

/* makeobjs.c */
CD *generate_mob_nation args( ( char *argument ) );
CD *generate_mob args( ( NATION_DATA * nation ) );
void mob_to_area args( ( CHAR_DATA * mob, AREA_DATA * tarea ) );
void make_corpse args( ( CHAR_DATA * ch ) );
void make_blood args( ( CHAR_DATA * ch, int amt ) );
void make_bloodstain args( ( CHAR_DATA * ch ) );
void make_scraps args( ( OBJ_DATA * obj ) );
void make_fire args( ( ROOM_INDEX_DATA * in_room, sh_int timer ) );
OD *make_trap args( ( int v0, int v1, int v2, int v3 ) );
OD *create_money args( ( int amount ) );

/* misc.c */
void actiondesc args( ( CHAR_DATA * ch, OBJ_DATA * obj, void *vo ) );
EXT_BV meb args( ( int bit ) );
EXT_BV multimeb args( ( int bit, ... ) );

/* mud_comm.c */
char *mprog_type_to_name args( ( int type ) );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *strstr args( ( const char *s1, const char *s2 ) );
#endif

void mprog_wordlist_check args( ( char *arg, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void *vo, int type ) );
void mprog_percent_check args( ( CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void *vo, int type ) );
void mprog_act_trigger args( ( char *buf, CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj, void *vo ) );
void mprog_bribe_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch, int amount ) );
void mprog_entry_trigger args( ( CHAR_DATA * mob ) );
void mprog_give_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj ) );
void mprog_greet_trigger args( ( CHAR_DATA * mob ) );
void mprog_fight_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch ) );
void mprog_hitprcnt_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch ) );
void mprog_death_trigger args( ( CHAR_DATA * killer, CHAR_DATA * mob ) );
void mprog_random_trigger args( ( CHAR_DATA * mob ) );
void mprog_speech_trigger args( ( char *txt, CHAR_DATA * mob ) );
void mprog_script_trigger args( ( CHAR_DATA * mob ) );
void mprog_hour_trigger args( ( CHAR_DATA * mob ) );
void mprog_time_trigger args( ( CHAR_DATA * mob ) );
void progbug args( ( char *str, CHAR_DATA * mob ) );
void rset_supermob args( ( ROOM_INDEX_DATA * room ) );
void release_supermob args( (  ) );

/* player.c */
void set_title args( ( CHAR_DATA * ch, char *title ) );
bool add_mutation args( ( CHAR_DATA * ch, int i ) );
int get_height_weight args( ( CHAR_DATA * ch ) );

/* skills.c */
int learned args( ( CHAR_DATA * ch, int sn ) );
bool skill_available args( ( CHAR_DATA * ch, int sn ) );
int get_best_talent args( ( CHAR_DATA * ch, int sn ) );
int get_adept args( ( CHAR_DATA * ch, int sn ) );
bool can_use_skill args( ( CHAR_DATA * ch, int percent, int gsn ) );
bool check_skill args( ( CHAR_DATA * ch, char *command, char *argument ) );
void learn_from_success args( ( CHAR_DATA * ch, int sn ) );
void learn_from_failure args( ( CHAR_DATA * ch, int sn ) );
bool check_shieldwork args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_parry args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_dodge args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_tumble args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_grip args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void disarm args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void trip args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool mob_fire args( ( CHAR_DATA * ch, char *name ) );
CD *scan_for_victim args( ( CHAR_DATA * ch, EXIT_DATA * pexit, char *name ) );

/* ban.c */
int add_ban args( ( CHAR_DATA * ch, char *arg1, int time ) );
void show_bans args( ( CHAR_DATA * ch ) );
void save_banlist args( ( void ) );
void load_banlist args( ( void ) );
bool check_total_bans args( ( DESCRIPTOR_DATA * d ) );
bool check_bans args( ( CHAR_DATA * ch ) );
bool check_multi args( ( CHAR_DATA * ch ) );

/* handler.c */
void huh args( ( CHAR_DATA * ch ) );
DEITY_DATA *get_talent_by_index( int index );   /* Scion */
int get_char_worth args( ( CHAR_DATA * ch ) );  /* Scion */
int get_light_char args( ( CHAR_DATA * ch ) );  /* Scion */
int get_light_room args( ( ROOM_INDEX_DATA * room ) );   /* Scion */
int get_exp args( ( CHAR_DATA * ch ) );
int get_exp_worth args( ( CHAR_DATA * ch ) );
int exp_level args( ( CHAR_DATA * ch, sh_int level ) );
int talent_exp args( ( CHAR_DATA * ch, sh_int level, int tal ) );
int level_exp args( ( CHAR_DATA * ch, int exp ) );
sh_int get_trust args( ( CHAR_DATA * ch ) );
sh_int get_curr_str args( ( CHAR_DATA * ch ) );
sh_int get_curr_int args( ( CHAR_DATA * ch ) );
sh_int get_curr_wis args( ( CHAR_DATA * ch ) );
sh_int get_curr_wil args( ( CHAR_DATA * ch ) );
sh_int get_curr_dex args( ( CHAR_DATA * ch ) );
sh_int get_curr_con args( ( CHAR_DATA * ch ) );
sh_int get_curr_cha args( ( CHAR_DATA * ch ) );
sh_int get_curr_per args( ( CHAR_DATA * ch ) );
sh_int get_curr_lck args( ( CHAR_DATA * ch ) );
sh_int get_curr_end args( ( CHAR_DATA * ch ) );
bool can_take_proto args( ( CHAR_DATA * ch ) );
int can_carry_n args( ( CHAR_DATA * ch ) );
int can_carry_w args( ( CHAR_DATA * ch ) );
bool is_name args( ( const char *str, char *namelist ) );
bool is_name_prefix args( ( const char *str, char *namelist ) );
bool nifty_is_name args( ( char *str, char *namelist ) );
bool nifty_is_name_prefix args( ( char *str, char *namelist ) );
void affect_modify args( ( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd ) );
void affect_to_char args( ( CHAR_DATA * ch, AFFECT_DATA * paf ) );
void affect_remove args( ( CHAR_DATA * ch, AFFECT_DATA * paf ) );
void affect_strip args( ( CHAR_DATA * ch, int sn ) );
bool is_affected args( ( CHAR_DATA * ch, int sn ) );
void affect_join args( ( CHAR_DATA * ch, AFFECT_DATA * paf ) );
void obj_affect_join( OBJ_DATA * obj, AFFECT_DATA * paf );  /* Scion */
void char_from_room args( ( CHAR_DATA * ch ) );
void char_to_room args( ( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex ) );
OD *obj_to_char args( ( OBJ_DATA * obj, CHAR_DATA * ch ) );
void obj_from_char args( ( OBJ_DATA * obj ) );
int apply_ac args( ( OBJ_DATA * obj, int iWear ) );
OD *get_eq_char args( ( CHAR_DATA * ch, int iWear ) );
int count_obj_list( RESET_DATA * pReset, OBJ_INDEX_DATA * pObjIndex, OBJ_DATA * list );
void obj_from_room args( ( OBJ_DATA * obj ) );
OD *obj_to_room args( ( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex ) );
OD *obj_to_obj args( ( OBJ_DATA * obj, OBJ_DATA * obj_to ) );
void obj_from_obj args( ( OBJ_DATA * obj ) );
void extract_obj args( ( OBJ_DATA * obj ) );
void extract_exit args( ( ROOM_INDEX_DATA * room, EXIT_DATA * pexit ) );
void extract_room args( ( ROOM_INDEX_DATA * room ) );
void clean_room args( ( ROOM_INDEX_DATA * room ) );
void clean_obj args( ( OBJ_INDEX_DATA * obj ) );
void clean_mob args( ( MOB_INDEX_DATA * mob ) );
void clean_resets( ROOM_INDEX_DATA * room );
void extract_char args( ( CHAR_DATA * ch, bool fPull ) );
char *fur_pos_string args( ( int pos ) ); /* furniture */
char *pos_string args( ( CHAR_DATA * victim ) );   /* furniture */
int count_users args( ( OBJ_DATA * obj ) );  /* furniture */
CD *get_char_room args( ( CHAR_DATA * ch, char *argument ) );
CD *get_char_world args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_type args( ( OBJ_INDEX_DATA * pObjIndexData ) );
OD *get_obj_list args( ( CHAR_DATA * ch, char *argument, OBJ_DATA * list ) );
OD *get_obj_list_rev args( ( CHAR_DATA * ch, char *argument, OBJ_DATA * list ) );
OD *get_obj_carry args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_wear args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_vnum args( ( CHAR_DATA * ch, int vnum ) );
OD *get_obj_here args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_world args( ( CHAR_DATA * ch, char *argument ) );
int get_obj_number args( ( OBJ_DATA * obj ) );
int get_obj_weight args( ( OBJ_DATA * obj ) );
int get_real_obj_weight args( ( OBJ_DATA * obj ) );
bool room_is_dark args( ( ROOM_INDEX_DATA * pRoomIndex ) );
bool room_is_private args( ( ROOM_INDEX_DATA * pRoomIndex ) );
bool can_see args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool can_see_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
bool can_drop_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
char *item_type_name args( ( OBJ_DATA * obj ) );
char *affect_loc_name args( ( int location ) );
char *affect_bit_name args( ( EXT_BV * vector ) );
char *extra_bit_name args( ( EXT_BV * extra_flags ) );
char *pull_type_name args( ( int pulltype ) );
ch_ret check_for_trap args( ( CHAR_DATA * ch, OBJ_DATA * obj, int flag ) );
ch_ret check_room_for_traps args( ( CHAR_DATA * ch, int flag ) );
bool is_trapped args( ( OBJ_DATA * obj ) );
OD *get_trap args( ( OBJ_DATA * obj ) );
ch_ret spring_trap args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void throw_rider args( ( CHAR_DATA * ch ) );
void dump_passenger args( ( OBJ_DATA * obj ) );
void dump_container args( ( OBJ_DATA * obj ) );
void fix_char args( ( CHAR_DATA * ch ) );
void showaffect args( ( CHAR_DATA * ch, AFFECT_DATA * paf ) );
void set_cur_obj args( ( OBJ_DATA * obj ) );
bool obj_extracted args( ( OBJ_DATA * obj ) );
void queue_extracted_obj args( ( OBJ_DATA * obj ) );
void clean_obj_queue args( ( void ) );
void set_cur_char args( ( CHAR_DATA * ch ) );
bool char_died args( ( CHAR_DATA * ch ) );
void queue_extracted_char args( ( CHAR_DATA * ch, bool extract ) );
void clean_char_queue args( ( void ) );
void add_timer args( ( CHAR_DATA * ch, sh_int type, sh_int count, DO_FUN * fun, int value ) );
TIMER *get_timerptr args( ( CHAR_DATA * ch, sh_int type ) );
sh_int get_timer args( ( CHAR_DATA * ch, sh_int type ) );
void extract_timer args( ( CHAR_DATA * ch, TIMER * timer ) );
void remove_timer args( ( CHAR_DATA * ch, sh_int type ) );
bool in_hard_range args( ( CHAR_DATA * ch, AREA_DATA * tarea ) );
bool chance args( ( CHAR_DATA * ch, sh_int percent ) );
bool chance_attrib args( ( CHAR_DATA * ch, sh_int percent, sh_int attrib ) );
OD *clone_object args( ( OBJ_DATA * obj ) );
void split_obj args( ( OBJ_DATA * obj, int num ) );
void separate_obj args( ( OBJ_DATA * obj ) );
bool empty_obj args( ( OBJ_DATA * obj, OBJ_DATA * destobj, ROOM_INDEX_DATA * destroom ) );
OD *find_obj args( ( CHAR_DATA * ch, char *argument, bool carryonly ) );
bool ms_find_obj args( ( CHAR_DATA * ch ) );
void worsen_mental_state args( ( CHAR_DATA * ch, int mod ) );
void better_mental_state args( ( CHAR_DATA * ch, int mod ) );
void boost_economy args( ( AREA_DATA * tarea, int gold ) );
void lower_economy args( ( AREA_DATA * tarea, int gold ) );
void economize_mobgold args( ( CHAR_DATA * mob ) );
bool economy_has args( ( AREA_DATA * tarea, int gold ) );
void add_kill args( ( CHAR_DATA * ch, CHAR_DATA * mob ) );
int times_killed args( ( CHAR_DATA * ch, CHAR_DATA * mob ) );
void update_aris args( ( CHAR_DATA * ch ) );
AREA_DATA *get_area args( ( char *name ) );  /* FB */
OD *get_objtype args( ( CHAR_DATA * ch, sh_int type ) );
void learn_weapon args( ( CHAR_DATA * ch, int i ) );
void learn_noncombat args( ( CHAR_DATA * ch, int i ) );
NATION_DATA *find_nation args( ( char *name ) );

/* hiscore.c */
void adjust_hiscore args( ( char *argument, CHAR_DATA * ch, int i ) );
bool add_hiscore args( ( char *keyword, char *name, int score ) );
void load_hiscores args( ( void ) );

/* interp.c */
bool check_pos args( ( CHAR_DATA * ch, sh_int position ) );
void interpret args( ( CHAR_DATA * ch, char *argument, bool forced ) );
bool is_number args( ( char *arg ) );
int number_argument args( ( char *argument, char *arg ) );
char *one_argument args( ( char *argument, char *arg_first ) );
char *one_argument2 args( ( char *argument, char *arg_first ) );
char *one_argument_retain_case args( ( char *argument, char *arg_first ) );
ST *find_social args( ( char *command ) );
CMDTYPE *find_command args( ( char *command ) );
void hash_commands args( (  ) );
void start_timer args( ( struct timeval * stime ) );
time_t end_timer args( ( struct timeval * stime ) );
void send_timer args( ( struct timerset * vtime, CHAR_DATA * ch ) );
void update_userec args( ( struct timeval * time_used, struct timerset * userec ) );

/* magic.c */
bool process_spell_components args( ( CHAR_DATA * ch, int sn ) );
int ch_slookup args( ( CHAR_DATA * ch, const char *name ) );
int find_spell args( ( CHAR_DATA * ch, const char *name, bool know ) );
int find_skill args( ( CHAR_DATA * ch, const char *name, bool know ) );
int skill_lookup args( ( const char *name ) );
int herb_lookup args( ( const char *name ) );
int slot_lookup args( ( int slot ) );
int bsearch_skill args( ( const char *name, int first, int top ) );
int bsearch_skill_exact args( ( const char *name, int first, int top ) );
int bsearch_skill_prefix args( ( const char *name, int first, int top ) );
ch_ret obj_cast_spell args( ( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj ) );
int dice_parse args( ( CHAR_DATA * ch, char *exp ) );
SK *get_skilltype args( ( int sn ) );
bool check_manastorm args( ( CHAR_DATA * ch ) );

/* namegen.c */
char *random_name args( ( char *word, int lang ) );

/* talent.c */
CD *find_target args( ( CHAR_DATA * ch, char *argument, bool offense ) );
bool check_displacement args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_blur args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_antimagic args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void learn_talent args( ( CHAR_DATA * ch, int tal ) );
void mana_from_char args( ( CHAR_DATA * ch, int mana ) );
void use_magic args( ( CHAR_DATA * ch, int tal, int mana ) );
void magic_damage args( ( CHAR_DATA * victim, CHAR_DATA * ch, int dam, int type, int tal, bool dont_wait ) );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_GEM		1
#define OS_GROUND	2
int get_obj_room_vnum_recursive( OBJ_DATA * obj );
void save_char_obj args( ( CHAR_DATA * ch ) );
bool load_char_obj args( ( DESCRIPTOR_DATA * d, char *name, bool preload ) );
void set_alarm args( ( long seconds ) );
void requip_char args( ( CHAR_DATA * ch ) );
void fwrite_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest, sh_int os_type ) );
void fread_obj args( ( CHAR_DATA * ch, FILE * fp, sh_int os_type ) );
void de_equip_char args( ( CHAR_DATA * ch ) );
void re_equip_char args( ( CHAR_DATA * ch ) );
void read_char_mobile args( ( char *argument ) );
void write_char_mobile args( ( CHAR_DATA * ch, char *argument ) );
CHAR_DATA *fread_mobile args( ( FILE * fp ) );
void fwrite_mobile args( ( FILE * fp, CHAR_DATA * mob ) );

/* special.c */
SF *spec_lookup args( ( const char *name ) );
char *lookup_spec args( ( SPEC_FUN * special ) );

/* tables.c */
int get_skill args( ( char *skilltype ) );
char *spell_name args( ( SPELL_FUN * spell ) );
char *skill_name args( ( DO_FUN * skill ) );
void load_skill_table args( ( void ) );
void save_skill_table args( ( void ) );
void sort_skill_table args( ( void ) );
void remap_slot_numbers args( ( void ) );
void load_socials args( ( void ) );
void save_socials args( ( void ) );
void load_commands args( ( void ) );
void save_commands args( ( void ) );
SPELL_FUN *spell_function args( ( char *name ) );
DO_FUN *skill_function args( ( char *name ) );
void write_clan_file args( ( CLAN_DATA * clan, char filename[MAX_INPUT_LENGTH] ) );
void save_clans args( ( void ) );
void load_clans args( ( void ) );
void load_herb_table args( ( void ) );
void save_herb_table args( ( void ) );
void load_tongues args( ( void ) );
void save_bits( void );
void load_bits( void );


/* track.c */
void found_prey args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void hunt_victim args( ( CHAR_DATA * ch ) );

/* update.c */
void gain_condition args( ( CHAR_DATA * ch, int iCond, int value ) );
bool climate_affect args( ( CHAR_DATA * ch, int type ) );
void update_handler args( ( void ) );
void reboot_check args( ( time_t reset ) );
#if 0
void reboot_check args( ( char *arg ) );
#endif
void remove_portal args( ( OBJ_DATA * portal ) );
void weather_update args( ( void ) );

/* hashstr.c */
char *str_alloc args( ( char *str ) );
char *quick_link args( ( char *str ) );
int str_free args( ( char *str ) );
void show_hash args( ( int count ) );
char *hash_stats args( ( void ) );
char *check_hash args( ( char *str ) );
void hash_dump args( ( int hash ) );
void show_high_hash args( ( int top ) );

/* newscore.c */
char *get_race args( ( CHAR_DATA * ch ) );

/* bits.c */
BIT_DATA *find_abit( int number );
BIT_DATA *find_qbit( int number );
BIT_DATA *get_abit( CHAR_DATA * ch, int number );
BIT_DATA *get_qbit( CHAR_DATA * ch, int number );
void load_bits( void );
void save_bits( void );
void set_abit( CHAR_DATA * ch, int number );
void set_qbit( CHAR_DATA * ch, int number );
void remove_abit( CHAR_DATA * ch, int number );
void remove_qbit( CHAR_DATA * ch, int number );
void do_showabit( CHAR_DATA * ch, char *argument );
void do_showqbit( CHAR_DATA * ch, char *argument );

/* makezone.c */
int makezone( CHAR_DATA * ch, char note[MAX_STRING_LENGTH] );
//void spell_parse(CHAR_DATA *ch, char *argument);

#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 *
 *  New Build Interface Stuff Follows
 *
 */


/*
 *  Data for a menu page
 */
struct menu_data
{
   char *sectionNum;
   char *charChoice;
   int x;
   int y;
   char *outFormat;
   void *data;
   int ptrType;
   int cmdArgs;
   char *cmdString;
};

DECLARE_DO_FUN( do_redraw_page );
DECLARE_DO_FUN( do_refresh_page );
DECLARE_DO_FUN( do_pagelen );
DECLARE_DO_FUN( do_omenu );
DECLARE_DO_FUN( do_rmenu );
DECLARE_DO_FUN( do_mmenu );
DECLARE_DO_FUN( do_clear );

extern MENU_DATA room_page_a_data[];
extern MENU_DATA room_page_b_data[];
extern MENU_DATA room_page_c_data[];
extern MENU_DATA room_help_page_data[];

extern MENU_DATA mob_page_a_data[];
extern MENU_DATA mob_page_b_data[];
extern MENU_DATA mob_page_c_data[];
extern MENU_DATA mob_page_d_data[];
extern MENU_DATA mob_page_e_data[];
extern MENU_DATA mob_page_f_data[];
extern MENU_DATA mob_help_page_data[];

extern MENU_DATA obj_page_a_data[];
extern MENU_DATA obj_page_b_data[];
extern MENU_DATA obj_page_c_data[];
extern MENU_DATA obj_page_d_data[];
extern MENU_DATA obj_page_e_data[];
extern MENU_DATA obj_help_page_data[];

extern MENU_DATA control_page_a_data[];
extern MENU_DATA control_help_page_data[];

extern const char room_page_a[];
extern const char room_page_b[];
extern const char room_page_c[];
extern const char room_help_page[];

extern const char obj_page_a[];
extern const char obj_page_b[];
extern const char obj_page_c[];
extern const char obj_page_d[];
extern const char obj_page_e[];
extern const char obj_help_page[];

extern const char mob_page_a[];
extern const char mob_page_b[];
extern const char mob_page_c[];
extern const char mob_page_d[];
extern const char mob_page_e[];
extern const char mob_page_f[];
extern const char mob_help_page[];
extern const char *npc_sex[3];
extern const char *ris_strings[];

extern const char control_page_a[];
extern const char control_help_page[];

#define SH_INT 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5


#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19

#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4

#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST

/*
 * defines for use with this get_affect function
 */

#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30

/* furniture flags */
#define STAND_AT		BV00
#define STAND_ON		BV01
#define STAND_IN		BV02
#define SIT_AT			BV03
#define SIT_ON			BV04
#define SIT_IN			BV05
#define REST_AT			BV06
#define REST_ON			BV07
#define REST_IN			BV08
#define SLEEP_AT		BV09
#define SLEEP_ON		BV10
#define SLEEP_IN		BV11
#define PUT_AT			BV12
#define PUT_ON			BV13
#define PUT_IN			BV14
#define PUT_INSIDE		BV15

#define FURNITURE_UNUSED	-1
#define FURNITURE_NONE		0
#define ST_AT			BV00  /* Stand at */
#define ST_ON			BV01  /* Stand on */
#define ST_IN			BV02  /* Stand in */
#define SI_AT			BV03  /* Sit at      */
#define SI_ON			BV04  /* Sit on      */
#define SI_IN			BV05  /* Sit in      */
#define RE_AT			BV06  /* Rest at     */
#define RE_ON			BV07  /* Rest on     */
#define RE_IN			BV08  /* Rest in     */
#define SL_AT			BV09  /* Sleep at */
#define SL_ON			BV10  /* Sleep on */
#define SL_IN			BV11  /* Sleep in */
#define PT_AT			BV12  /* Put at      */
#define PT_ON			BV13  /* Put on      */
#define PT_IN			BV14  /* Put in      */
#define PT_INSIDE		BV15  /* Put inside  */

/* New spell sys, globally available function prototypes -- Scion */
void start_spell( CHAR_DATA * ch );
SPELL_DATA *get_spell( CHAR_DATA * ch );
void add_command_spell( CHAR_DATA * ch, char *cmd );

/*
 *   Map Structures
 */

/*DECLARE_DO_FUN( do_mapout 	);
DECLARE_DO_FUN( do_lookmap	);
*/
//struct  map_data   /* contains per-room data */
//{
//  int vnum;     /* which map this room belongs to */
//  int x;     /* horizontal coordinate */
//  int y;     /* vertical coordinate */
//  char entry;      /* code that shows up on map */
//};


//struct  map_index_data
//{
//  MAP_INDEX_DATA  *next;
//  int       vnum;          /* vnum of the map */
//  int             map_of_vnums[49][81];   /* room vnums aranged as a map */
//};


//MAP_INDEX_DATA *get_map_index(int vnum);
//void            init_maps();


/*
 * mudprograms stuff
 */
extern CHAR_DATA *supermob;

void oprog_speech_trigger( char *txt, CHAR_DATA * ch );
void oprog_random_trigger( OBJ_DATA * obj );
void oprog_wear_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
bool oprog_use_trigger( CHAR_DATA * ch, OBJ_DATA * obj, CHAR_DATA * vict, OBJ_DATA * targ, void *vo );
void oprog_remove_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_sac_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_damage_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_drop_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_zap_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
char *oprog_type_to_name( int type );

/*
 * MUD_PROGS START HERE
 * (object stuff)
 */
void oprog_greet_trigger( CHAR_DATA * ch );
void oprog_speech_trigger( char *txt, CHAR_DATA * ch );
void oprog_random_trigger( OBJ_DATA * obj );
void oprog_random_trigger( OBJ_DATA * obj );
void oprog_remove_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_sac_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_get_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_damage_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_drop_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_examine_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_zap_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_pull_trigger( CHAR_DATA * ch, OBJ_DATA * obj );
void oprog_push_trigger( CHAR_DATA * ch, OBJ_DATA * obj );


/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG      -2

typedef enum
{
   ACT_PROG, SPEECH_PROG, RAND_PROG, FIGHT_PROG, DEATH_PROG, HITPRCNT_PROG,
   ENTRY_PROG, GREET_PROG, ALL_GREET_PROG, GIVE_PROG, BRIBE_PROG, HOUR_PROG,
   TIME_PROG, WEAR_PROG, REMOVE_PROG, SAC_PROG, LOOK_PROG, EXA_PROG, ZAP_PROG,
   GET_PROG, DROP_PROG, DAMAGE_PROG, REPAIR_PROG, RANDIW_PROG, SPEECHIW_PROG,
   PULL_PROG, PUSH_PROG, SLEEP_PROG, REST_PROG, LEAVE_PROG, SCRIPT_PROG,
   USE_PROG
} prog_types;

/*
 * For backwards compatability
 */
#define RDEATH_PROG DEATH_PROG
#define ENTER_PROG  ENTRY_PROG
#define RFIGHT_PROG FIGHT_PROG
#define RGREET_PROG GREET_PROG
#define OGREET_PROG GREET_PROG

void rprog_leave_trigger( CHAR_DATA * ch );
void rprog_enter_trigger( CHAR_DATA * ch );
void rprog_sleep_trigger( CHAR_DATA * ch );
void rprog_rest_trigger( CHAR_DATA * ch );
void rprog_rfight_trigger( CHAR_DATA * ch );
void rprog_death_trigger( CHAR_DATA * killer, CHAR_DATA * ch );
void rprog_speech_trigger( char *txt, CHAR_DATA * ch );
void rprog_random_trigger( CHAR_DATA * ch );
void rprog_time_trigger( CHAR_DATA * ch );
void rprog_hour_trigger( CHAR_DATA * ch );
char *rprog_type_to_name( int type );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( char *buf, OBJ_DATA * mobj, CHAR_DATA * ch, OBJ_DATA * obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA * room, CHAR_DATA * ch, OBJ_DATA * obj, void *vo );
#endif

#define GET_ADEPT(ch, sn)	(get_adept((ch), (sn)))

#define LEARNED(ch,sn)	    (learned((ch), (sn)))

/* Structure and macros for using long bit vectors */
#define CHAR_SIZE sizeof(char)

typedef char *LONG_VECTOR;

#define LV_CREATE(vector, bit_length)					\
do									\
{									\
	int i;								\
	CREATE(vector, char, 1 + bit_length/CHAR_SIZE);			\
									\
	for(i = 0; i <= bit_length/CHAR_SIZE; i++)			\
		*(vector + i) = 0;					\
}while(0)

#define LV_IS_SET(vector, index)					\
	(*(vector + index/CHAR_SIZE) & (1 << index%CHAR_SIZE))

#define LV_SET_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) |= (1 << index%CHAR_SIZE))

#define LV_REMOVE_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) &= ~(1 << index%CHAR_SIZE))

#define LV_TOGGLE_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) ^= (1 << index%CHAR_SIZE))

#ifdef WIN32
void gettimeofday( struct timeval *tv, struct timezone *tz );
void kill_timer(  );

/* directory scanning stuff */

typedef struct dirent
{
   char *d_name;
};

typedef struct
{
   HANDLE hDirectory;
   WIN32_FIND_DATA Win32FindData;
   struct dirent dirinfo;
   char sDirName[MAX_PATH];
} DIR;


DIR *opendir( char *sDirName );
struct dirent *readdir( DIR * dp );
void closedir( DIR * dp );

/* --------------- Stuff for Win32 services ------------------ */
/*

   NJG:

   When "exit" is called to handle an error condition, we really want to
   terminate the game thread, not the whole process.

 */

#define exit(arg) Win32_Exit(arg)
void Win32_Exit( int exit_code );

#endif

#define send_to_char send_to_char_color
#define send_to_pager send_to_pager_color
