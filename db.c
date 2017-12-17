/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 * 			Database management module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
#include <dirent.h>
#else
#define strcasecmp strcmp
#endif
#include "mud.h"


extern int _filbuf args( ( FILE * ) );
extern char *munch_colors( char *word );

#if defined(KEY)
#undef KEY
#endif

void init_supermob(  );

extern void load_world( CHAR_DATA * ch );

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

extern SPECIES_DATA find_species( char *name );
extern void load_new_helps args( ( void ) );
extern void load_books args( ( void ) );

/*
 * Globals.
 */

WIZENT *first_wiz;
WIZENT *last_wiz;

time_t last_restore_all_time = 0;

HELP_DATA *first_help;
HELP_DATA *last_help;

SHOP_DATA *first_shop;
SHOP_DATA *last_shop;

CHANNEL_DATA *first_channel;
CHANNEL_DATA *last_channel;

OBJ_DATA *extracted_obj_queue;
EXTRACT_CHAR_DATA *extracted_char_queue;

char bug_buf[2 * MAX_INPUT_LENGTH];
CHAR_DATA *first_char;
CHAR_DATA *last_char;
char *help_greeting;
char log_buf[2 * MAX_INPUT_LENGTH];

OBJ_DATA *first_object;
OBJ_DATA *last_object;
TIME_INFO_DATA time_info;
WEATHER_DATA weather_info;

int weath_unit;   /* global weather param */
int rand_factor;
int climate_factor;
int neigh_factor;
int max_vector;

int cur_qobjs;
int cur_qchars;
int nummobsloaded;
int numobjsloaded;
int physicalobjects;

MAP_INDEX_DATA *first_map; /* maps */

FILE *fpLOG;

/* thief */
sh_int gsn_detrap;
sh_int gsn_dodge;
sh_int gsn_hide;
sh_int gsn_peek;
sh_int gsn_pick_lock;
sh_int gsn_locksmith;
sh_int gsn_sneak;
sh_int gsn_steal;
sh_int gsn_poison_weapon;

/* thief & warrior */
sh_int gsn_disarm;
sh_int gsn_parry;
sh_int gsn_stun;
sh_int gsn_bashdoor;
sh_int gsn_grip;
sh_int gsn_tumble;

/* other   */
sh_int gsn_aid;
sh_int gsn_track;
sh_int gsn_search;
sh_int gsn_dig;
sh_int gsn_mount;
sh_int gsn_bite;
sh_int gsn_claw;
sh_int gsn_sting;
sh_int gsn_tail;
sh_int gsn_scribe;
sh_int gsn_skin;
sh_int gsn_brew;
sh_int gsn_climb;
sh_int gsn_scan;
sh_int gsn_swim;

/* blacksmithy stuff -- Scion */
sh_int gsn_fashion;
sh_int gsn_sharpen;
sh_int gsn_engrave;
sh_int gsn_combine;
sh_int gsn_spike;
sh_int gsn_dye;
sh_int gsn_affix;
sh_int gsn_repair;
sh_int gsn_mix;

/* automatic spells -keo */
sh_int gsn_counterspell;

/* spells */
sh_int gsn_weaken;
sh_int gsn_blindness;
sh_int gsn_charm_person;
sh_int gsn_corruption;
sh_int gsn_curse;
sh_int gsn_poison;
sh_int gsn_sleep;
sh_int gsn_sticky_flame;
sh_int gsn_disease;
sh_int gsn_mummy_rot;

/* magic */
sh_int gsn_fire;
sh_int gsn_water;
sh_int gsn_mind;
sh_int gsn_wind;
sh_int gsn_earth;
sh_int gsn_seeking;
sh_int gsn_frost;
sh_int gsn_security;
sh_int gsn_motion;
sh_int gsn_lightning;
sh_int gsn_dream;
sh_int gsn_illusion;
sh_int gsn_death;
sh_int gsn_invoke;

/* for searching */
sh_int gsn_first_spell;
sh_int gsn_first_skill;
sh_int gsn_top_sn;

/*
 * Locals.
 */
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

AREA_DATA *first_area;
AREA_DATA *last_area;
AREA_DATA *first_area_name;   /*Used for alphanum. sort */
AREA_DATA *last_area_name;
AREA_DATA *first_build;
AREA_DATA *last_build;
AREA_DATA *first_asort;
AREA_DATA *last_asort;
AREA_DATA *first_bsort;
AREA_DATA *last_bsort;

SYSTEM_DATA sysdata;

MATERIAL_DATA *first_material;   /* Scion */
MATERIAL_DATA *last_material; /* Scion */
MAT_SHOP *first_matshop;   /* keo */
MAT_SHOP *last_matshop;

MOON_DATA *first_moon;  /* Keolah */
MOON_DATA *last_moon;   /* Keolah */

RECIPE_DATA *first_recipe;
RECIPE_DATA *last_recipe;
INGRED_DATA *first_ingred;
INGRED_DATA *last_ingred;

SPECIES_DATA *first_species;
SPECIES_DATA *last_species;
NATION_DATA *first_nation;
NATION_DATA *last_nation;

MUT_DATA *first_mutation;
MUT_DATA *last_mutation;

int top_affect;
int top_area;
int top_ed;
int top_exit;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_shop;
int top_vroom;
int top_help;

/*
 * Semi-locals.
 */
bool fBootDb;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];



/*
 * Local booting procedures.
 */
void init_mm args( ( void ) );
void check_all_shops( void );

void boot_log args( ( const char *str, ... ) );
void load_area args( ( FILE * fp ) );
void load_author args( ( AREA_DATA * tarea, FILE * fp ) );
void load_economy args( ( AREA_DATA * tarea, FILE * fp ) );
void load_resetmsg args( ( AREA_DATA * tarea, FILE * fp ) );   /* Rennard */
void load_flags args( ( AREA_DATA * tarea, FILE * fp ) );
void load_mobiles args( ( AREA_DATA * tarea, FILE * fp ) );
void load_objects args( ( AREA_DATA * tarea, FILE * fp ) );
void load_resets args( ( AREA_DATA * tarea, FILE * fp ) );
void load_rooms args( ( AREA_DATA * tarea, FILE * fp ) );
void load_shops args( ( AREA_DATA * tarea, FILE * fp ) );
void load_specials args( ( AREA_DATA * tarea, FILE * fp ) );
void load_ranges args( ( AREA_DATA * tarea, FILE * fp ) );
void load_climate args( ( AREA_DATA * tarea, FILE * fp ) ); /* FB */
void load_neighbor args( ( AREA_DATA * tarea, FILE * fp ) );
bool load_systemdata args( ( SYSTEM_DATA * sys ) );
void load_banlist args( ( void ) );
void load_version args( ( AREA_DATA * tarea, FILE * fp ) );
void load_reserved args( ( void ) );
void load_materials args( ( void ) );  /* Scion */
void load_matshops args( ( void ) );
void load_moons args( ( void ) );
void load_recipes args( ( void ) );
void load_ingreds args( ( void ) );
void load_species args( ( void ) );
void load_nations args( ( void ) );
void load_mutations args( ( void ) );
void load_cities args( ( void ) );
void load_channels args( ( void ) );
void load_arenas args( ( void ) );
void initialize_economy args( ( void ) );
void fix_exits args( ( void ) );
void sort_reserved args( ( RESERVE_DATA * pRes ) );
void init_area_weather args( ( void ) );
void load_weatherdata args( ( void ) );
/*
 * External booting function
 */
void renumber_put_resets( ROOM_INDEX_DATA * room );
void wipe_resets( ROOM_INDEX_DATA * room );

/*
 * MUDprogram locals
 */

int mprog_name_to_type args( ( char *name ) );
MPROG_DATA *mprog_file_read args( ( char *f, MPROG_DATA * mprg, MOB_INDEX_DATA * pMobIndex ) );
/* int 		oprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *oprog_file_read args( ( char *f, MPROG_DATA * mprg, OBJ_INDEX_DATA * pObjIndex ) );
/* int 		rprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *rprog_file_read args( ( char *f, MPROG_DATA * mprg, ROOM_INDEX_DATA * pRoomIndex ) );
void load_mudprogs args( ( AREA_DATA * tarea, FILE * fp ) );
void load_objprogs args( ( AREA_DATA * tarea, FILE * fp ) );
void load_roomprogs args( ( AREA_DATA * tarea, FILE * fp ) );
void mprog_read_programs args( ( FILE * fp, MOB_INDEX_DATA * pMobIndex ) );
void oprog_read_programs args( ( FILE * fp, OBJ_INDEX_DATA * pObjIndex ) );
void rprog_read_programs args( ( FILE * fp, ROOM_INDEX_DATA * pRoomIndex ) );


void shutdown_mud( char *reason )
{
   FILE *fp;

   if( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
   {
      fprintf( fp, "%s\n", reason );
      fclose( fp );
   }
}


/*
 * Big mama top level function.
 */
void boot_db( void )
{
   sh_int wear, x;

   show_hash( 32 );
   unlink( BOOTLOG_FILE );
   boot_log( "---------------------[ Boot Log ]--------------------" );

   log_string( "Loading commands" );
   load_commands(  );

   log_string( "Loading sysdata configuration..." );

   /*
    * default values 
    */
   sysdata.save_frequency = 20;  /* minutes */
   sysdata.save_pets = 0;
   sysdata.quest = 0;
   sysdata.email = "";
   sysdata.save_flags = SV_DEATH | SV_PASSCHG | SV_AUTO | SV_PUT | SV_DROP | SV_GIVE | SV_UNUSED | SV_ZAPDROP | SV_IDLE;
   if( !load_systemdata( &sysdata ) )
   {
      log_string( "Not found.  Creating new configuration." );
      sysdata.alltimemax = 0;
      sysdata.mud_name = str_dup( "(Name not set)" );
   }

   log_string( "Loading socials" );
   load_socials(  );

   log_string( "Loading skill table" );
   load_skill_table(  );
   sort_skill_table(  );
   remap_slot_numbers(  ); /* must be after the sort */

   gsn_first_spell = 0;
   gsn_first_skill = 0;
   gsn_top_sn = top_sn;

   for( x = 0; x < top_sn; x++ )
      if( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
         gsn_first_spell = x;
      else if( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
         gsn_first_skill = x;

   log_string( "Loading herb table" );
   load_herb_table(  );

   log_string( "Loading clans" );
   load_clans(  );

   log_string( "Loading channels" );
   load_channels(  );

   log_string( "Loading bits" );
   load_bits(  );

   log_string( "Loading help files" );
   load_new_helps(  );

   log_string( "Loading books" );
   load_books(  );
   


   fBootDb = TRUE;

   nummobsloaded = 0;
   numobjsloaded = 0;
   physicalobjects = 0;
   sysdata.maxplayers = 0;
   first_object = NULL;
   last_object = NULL;
   first_char = NULL;
   last_char = NULL;
   first_area = NULL;
   first_area_name = NULL; /*Used for alphanum. sort */
   last_area_name = NULL;
   last_area = NULL;
   first_build = NULL;
   last_area = NULL;
   first_shop = NULL;
   last_shop = NULL;
   first_asort = NULL;
   last_asort = NULL;
   extracted_obj_queue = NULL;
   extracted_char_queue = NULL;
   cur_qobjs = 0;
   cur_qchars = 0;
   cur_char = NULL;
   cur_obj = 0;
   cur_obj_serial = 0;
   cur_char_died = FALSE;
   cur_obj_extracted = FALSE;
   cur_room = NULL;
   quitting_char = NULL;
   loading_char = NULL;
   saving_char = NULL;
   first_ban = NULL;
   last_ban = NULL;

   weath_unit = 10;
   rand_factor = 2;
   climate_factor = 1;
   neigh_factor = 3;
   max_vector = weath_unit * 3;

   for( wear = 0; wear < MAX_WEAR; wear++ )
      for( x = 0; x < MAX_LAYERS; x++ )
         save_equipment[wear][x] = NULL;



   /*
    * Init random number generator.
    */
   log_string( "Initializing random number generator" );
   init_mm(  );

   /*
    * Set time and weather.
    */
   {
      long lhour, lday, lmonth;

      log_string( "Setting time and weather" );

      lhour = ( current_time - 650336715 ) / ( PULSE_TICK / PULSE_PER_SECOND );
      time_info.hour = lhour % 24;
      lday = lhour / 24;
      time_info.day = lday % 35;
      lmonth = lday / 35;
      time_info.month = lmonth % 12;
      time_info.year = lmonth / 12;

      if( time_info.hour < 5 )
         time_info.sunlight = SUN_DARK;
      else if( time_info.hour < 6 )
         time_info.sunlight = SUN_RISE;
      else if( time_info.hour < 19 )
         time_info.sunlight = SUN_LIGHT;
      else if( time_info.hour < 20 )
         time_info.sunlight = SUN_SET;
      else
         time_info.sunlight = SUN_DARK;

      /*
       * weather_info.change  = 0;
       * weather_info.mmhg = 960;
       * if ( time_info.month >= 7 && time_info.month <=12 )
       * weather_info.mmhg += number_range( 1, 50 );
       * else
       * weather_info.mmhg += number_range( 1, 80 );
       * 
       * if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
       * else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
       * else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
       * else                                  weather_info.sky = SKY_CLOUDLESS;
       */
   }

   /*
    * Assign gsn's for skills which need them.
    */
   {
      log_string( "Assigning gsn's" );
      ASSIGN_GSN( gsn_detrap, "detrap" );
      ASSIGN_GSN( gsn_tumble, "tumble" );
      ASSIGN_GSN( gsn_dodge, "dodge" );
      ASSIGN_GSN( gsn_hide, "hide" );
      ASSIGN_GSN( gsn_peek, "peek" );
      ASSIGN_GSN( gsn_pick_lock, "pick lock" );
      ASSIGN_GSN( gsn_locksmith, "locksmith" );
      ASSIGN_GSN( gsn_sneak, "sneak" );
      ASSIGN_GSN( gsn_steal, "steal" );
      ASSIGN_GSN( gsn_poison_weapon, "poison weapon" );
      ASSIGN_GSN( gsn_disarm, "disarm" );
      ASSIGN_GSN( gsn_parry, "parry" );
      ASSIGN_GSN( gsn_stun, "stun" );
      ASSIGN_GSN( gsn_bashdoor, "doorbash" );
      ASSIGN_GSN( gsn_grip, "grip" );
      ASSIGN_GSN( gsn_aid, "aid" );
      ASSIGN_GSN( gsn_track, "track" );
      ASSIGN_GSN( gsn_search, "search" );
      ASSIGN_GSN( gsn_dig, "dig" );
      ASSIGN_GSN( gsn_mount, "mount" );
      ASSIGN_GSN( gsn_scribe, "scribe" );
      ASSIGN_GSN( gsn_brew, "brew" );
      ASSIGN_GSN( gsn_climb, "climb" );
      ASSIGN_GSN( gsn_scan, "scan" );
      ASSIGN_GSN( gsn_swim, "swim" );
      ASSIGN_GSN( gsn_repair, "repair" );
      ASSIGN_GSN( gsn_mix, "mix" );
      ASSIGN_GSN( gsn_blindness, "blindness" );
      ASSIGN_GSN( gsn_mummy_rot, "mummy rot" );
      ASSIGN_GSN( gsn_disease, "disease" );
      ASSIGN_GSN( gsn_weaken, "weaken" );
      ASSIGN_GSN( gsn_corruption, "transmutation" );
      ASSIGN_GSN( gsn_charm_person, "charm person" );
      ASSIGN_GSN( gsn_curse, "curse" );
      ASSIGN_GSN( gsn_poison, "poison" );
      ASSIGN_GSN( gsn_sticky_flame, "sticky flame" );
      ASSIGN_GSN( gsn_sleep, "sleep" );
      ASSIGN_GSN( gsn_counterspell, "counterspell" );
      ASSIGN_GSN( gsn_fashion, "fashion" );
      ASSIGN_GSN( gsn_sharpen, "sharpen" );
      ASSIGN_GSN( gsn_engrave, "engrave" );
      ASSIGN_GSN( gsn_combine, "combine" );
      ASSIGN_GSN( gsn_spike, "spike" );
      ASSIGN_GSN( gsn_dye, "dye" );
      ASSIGN_GSN( gsn_affix, "affix" );
	  ASSIGN_GSN( gsn_skin, "skin" );

      /*
       * magic 
       */
      ASSIGN_GSN( gsn_fire, "fire magic" );
      ASSIGN_GSN( gsn_water, "water magic" );
      ASSIGN_GSN( gsn_mind, "mind magic" );
      ASSIGN_GSN( gsn_earth, "earth magic" );
      ASSIGN_GSN( gsn_wind, "wind magic" );
      ASSIGN_GSN( gsn_seeking, "seeking magic" );
      ASSIGN_GSN( gsn_frost, "frost magic" );
      ASSIGN_GSN( gsn_security, "security magic" );
      ASSIGN_GSN( gsn_motion, "motion magic" );
      ASSIGN_GSN( gsn_lightning, "lightning magic" );
      ASSIGN_GSN( gsn_dream, "dream magic" );
      ASSIGN_GSN( gsn_illusion, "illusion magic" );
      ASSIGN_GSN( gsn_death, "death magic" );
	  ASSIGN_GSN( gsn_invoke, "invoke" );
   }

   /*
    * MUST be done before loading the areas 
    */
   log_string( "Loading nations" );
   load_nations(  );
   log_string( "Loading cities" );
   load_cities(  );

   /*
    * Read in all the area files.
    */
   {
      FILE *fpList;

      log_string( "Reading in area files..." );
      if( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
      {
         perror( AREA_LIST );
         shutdown_mud( "Unable to open area list" );
         exit( 1 );
      }

      for( ;; )
      {
         strcpy( strArea, fread_word( fpList ) );
         if( strArea[0] == '$' )
            break;

         load_area_file( last_area, strArea );

      }
      fclose( fpList );
   }

   log_string( "Loading high scores..." );
   load_hiscores(  );

   /*
    *   initialize supermob.
    *    must be done before reset_area!
    *
    */
   init_supermob(  );

   
   /*
   * Loading up the housing
   */
      log_string("Loading Housing System, Home Accessories Data,"
                       " and Home Auctioning System");
            load_homedata();
            load_accessories();
            load_homebuy();
   /*
    * Fix up exits.
    * Declare db booting over.
    * Reset all areas once.
    * Load up the notes file.
    */
   {
      log_string( "Fixing exits" );
      fix_exits(  );
      fBootDb = FALSE;
      log_string( "Initializing economy" );
      initialize_economy(  );
      log_string( "Loading materials" );
      load_materials(  );
      log_string( "Loading world" );
      load_world( NULL );
      log_string( "Loading deities" );
      load_deity(  );
      log_string( "Loading bans" );
      load_banlist(  );
      log_string( "Loading reserved names" );
      load_reserved(  );
      log_string( "Loading material shops" );
      load_matshops(  );
      log_string( "Loading moons" );
      load_moons(  );
      log_string( "Loading recipes" );
      load_recipes(  );
      log_string( "Loading ingredients" );
      load_ingreds(  );
      log_string( "Loading species" );
      load_species(  );
      log_string( "Loading mutations" );
      load_mutations(  );
      log_string( "Loading arenas" );
      load_arenas(  );
      MOBtrigger = TRUE;
   }

   log_string( "Checking shops" );
   check_all_shops( );

   /*
    * Initialize area weather data 
    */
   load_weatherdata(  );
   init_area_weather(  );

   return;
}



/*
 * Load an 'area' header line.
 */
void load_area( FILE * fp )
{
   AREA_DATA *pArea;

   CREATE( pArea, AREA_DATA, 1 );
   pArea->first_room = pArea->last_room = NULL;
   pArea->name = fread_string_nohash( fp );
   pArea->author = STRALLOC( "unknown" );
   pArea->filename = str_dup( strArea );
   pArea->age = 15;
   pArea->nplayer = 0;
   pArea->low_r_vnum = 0;
   pArea->low_o_vnum = 0;
   pArea->low_m_vnum = 0;
   pArea->hi_r_vnum = 0;
   pArea->hi_o_vnum = 0;
   pArea->hi_m_vnum = 0;
   pArea->low_soft_range = 0;
   pArea->hi_soft_range = 0;
   pArea->low_hard_range = 0;
   pArea->hi_hard_range = 0;

   /*
    * initialize weather data - FB 
    */
   CREATE( pArea->weather, WEATHER_DATA, 1 );
   pArea->weather->temp = 0;
   pArea->weather->precip = 0;
   pArea->weather->wind = 0;
   pArea->weather->mana = 0;
   pArea->weather->temp_vector = 0;
   pArea->weather->precip_vector = 0;
   pArea->weather->wind_vector = 0;
   pArea->weather->mana_vector = 0;
   pArea->weather->climate_temp = 2;
   pArea->weather->climate_precip = 2;
   pArea->weather->climate_wind = 2;
   pArea->weather->climate_mana = 2;
   pArea->weather->first_neighbor = NULL;
   pArea->weather->last_neighbor = NULL;
   pArea->weather->echo = NULL;
   pArea->weather->echo_color = AT_GREY;
   area_version = 0;
   LINK( pArea, first_area, last_area, next, prev );
   top_area++;
   return;
}


/* Load the version number of the area file if none exists, then it
 * is set to version 0 when #AREA is read in which is why we check for
 * the #AREA here.  --Shaddai
 */

void load_version( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "Load_author: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   area_version = fread_number( fp );
   return;
}

/*
 * Load an author section. Scryn 2/1/96
 */
void load_author( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "Load_author: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   if( tarea->author )
      STRFREE( tarea->author );
   tarea->author = fread_string( fp );
   return;
}

/*
 * Load an economy section. Thoric
 */
void load_economy( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "Load_economy: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   tarea->high_economy = fread_number( fp );
   tarea->low_economy = fread_number( fp );
   return;
}

/* Reset Message Load, Rennard */
void load_resetmsg( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "Load_resetmsg: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   if( tarea->resetmsg )
      DISPOSE( tarea->resetmsg );
   tarea->resetmsg = fread_string_nohash( fp );
   return;
}

/*
 * Load area flags. Narn, Mar/96
 */
void load_flags( AREA_DATA * tarea, FILE * fp )
{
   char *ln;
   int x1, x2;

   if( !tarea )
   {
      bug( "Load_flags: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }
   ln = fread_line( fp );
   x1 = x2 = 0;
   sscanf( ln, "%d %d", &x1, &x2 );
   tarea->flags = x1;
   tarea->reset_frequency = x2;
   if( x2 )
      tarea->age = x2;

   /*
    * Link it up to the city 
    */
   if( tarea->reset_frequency )
   {
      int i;
      CITY_DATA *city;

      i = 0;
      city = first_city;
      while( city )
      {
         if( ++i == tarea->reset_frequency )
         {
            tarea->city = city;
            city->area = tarea;
            return;
         }
         city = city->next;
      }
   }
}


/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA * ch )
{
   LINK( ch, first_char, last_char, next, prev );
}


/*
 * Load a mob section.
 */
void load_mobiles( AREA_DATA * tarea, FILE * fp )
{
   MOB_INDEX_DATA *pMobIndex;
   char *ln;
   int x1, x2, x3, x4, x5, x6, x7, x8;

   if( !tarea )
   {
      bug( "Load_mobiles: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   for( ;; )
   {
      char buf[MAX_STRING_LENGTH];
      int vnum;
      char letter;
      int iHash;
      bool oldmob;
      bool tmpBootDb;

      letter = fread_letter( fp );
      if( letter != '#' )
      {
         bug( "Load_mobiles: # not found." );
         if( fBootDb )
         {
            shutdown_mud( "# not found" );
            exit( 1 );
         }
         else
            return;
      }

      vnum = fread_number( fp );
      if( vnum == 0 )
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;

      if( get_mob_index( vnum ) )
      {
         if( tmpBootDb )
         {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
            shutdown_mud( "duplicate vnum" );
            exit( 1 );
         }
         else
         {
            pMobIndex = get_mob_index( vnum );
            sprintf( buf, "Cleaning mobile: %d", vnum );
            log_string_plus( buf, LOG_BUILD, 0 );
            clean_mob( pMobIndex );
            oldmob = TRUE;
         }
      }
      else
      {
         oldmob = FALSE;
         CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
      }
      fBootDb = tmpBootDb;

      pMobIndex->vnum = vnum;
      if( fBootDb )
      {
         if( !tarea->low_m_vnum )
            tarea->low_m_vnum = vnum;
         if( vnum > tarea->hi_m_vnum )
            tarea->hi_m_vnum = vnum;
      }
      pMobIndex->player_name = fread_string( fp );
      pMobIndex->short_descr = fread_string( fp );
      pMobIndex->description = fread_string( fp );

      pMobIndex->description[0] = UPPER( pMobIndex->description[0] );

      pMobIndex->act = fread_bitvector( fp );
      xSET_BIT( pMobIndex->act, ACT_IS_NPC );
      pMobIndex->affected_by = fread_bitvector( fp );
      /*
       * was align 
       */ fread_number( fp );
      letter = fread_letter( fp );
      /*
       * was level 
       */ fread_number( fp );
      pMobIndex->mobthac0 = fread_number( fp );
      pMobIndex->ac = fread_number( fp );
      pMobIndex->hitnodice = fread_number( fp );
      /*
       * 'd'      
       */ fread_letter( fp );
      pMobIndex->hitsizedice = fread_number( fp );
      /*
       * '+'      
       */ fread_letter( fp );
      pMobIndex->hitplus = fread_number( fp );
      pMobIndex->damnodice = fread_number( fp );
      /*
       * 'd'      
       */ fread_letter( fp );
      pMobIndex->damsizedice = fread_number( fp );
      /*
       * '+'      
       */ fread_letter( fp );
      pMobIndex->damplus = fread_number( fp );
      pMobIndex->gold = fread_number( fp );
      pMobIndex->exp = fread_number( fp );
      pMobIndex->position = fread_number( fp );
      fread_number( fp );
      pMobIndex->sex = fread_number( fp );

      if( letter != 'S' && letter != 'C' )
      {
         bug( "Load_mobiles: vnum %d: letter '%c' not S or C.", vnum, letter );
         shutdown_mud( "bad mob data" );
         exit( 1 );
      }
      if( letter == 'C' )  /* Realms complex mob   -Thoric */
      {
         pMobIndex->perm_str = fread_number( fp );
         pMobIndex->perm_int = fread_number( fp );
         pMobIndex->perm_wil = fread_number( fp );
         pMobIndex->perm_dex = fread_number( fp );
         pMobIndex->perm_con = fread_number( fp );
         pMobIndex->perm_per = fread_number( fp );
         pMobIndex->perm_lck = fread_number( fp );
         pMobIndex->skinamount = fread_number( fp );
         pMobIndex->hide_type = fread_number( fp );
         pMobIndex->saving_poison_death = fread_number( fp );
         pMobIndex->saving_wand = fread_number( fp );
         pMobIndex->saving_para_petri = fread_number( fp );
         pMobIndex->saving_breath = fread_number( fp );
         pMobIndex->saving_spell_staff = fread_number( fp );
         ln = fread_line( fp );
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
         sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
         pMobIndex->race = x1;
         pMobIndex->height = x3;
         pMobIndex->weight = x4;
         pMobIndex->numattacks = x7;

#ifndef XBI
         ln = fread_line( fp );
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
         sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
         pMobIndex->hitroll = x1;
         pMobIndex->damroll = x2;
         pMobIndex->xflags = x3;
         pMobIndex->resistant = x4;
         pMobIndex->immune = x5;
         pMobIndex->susceptible = x6;
         pMobIndex->attacks = x7;
         pMobIndex->defenses = x8;
#else
         pMobIndex->hitroll = fread_number( fp );
         pMobIndex->damroll = fread_number( fp );
         pMobIndex->xflags = fread_number( fp );
         pMobIndex->resistant = fread_number( fp );
         pMobIndex->immune = fread_number( fp );
         pMobIndex->susceptible = fread_number( fp );
         pMobIndex->attacks = fread_bitvector( fp );
         pMobIndex->defenses = fread_bitvector( fp );
#endif
      }
      else
      {
         pMobIndex->perm_str = 100;
         pMobIndex->perm_dex = 100;
         pMobIndex->perm_int = 100;
         pMobIndex->perm_wil = 100;
         pMobIndex->perm_per = 100;
         pMobIndex->perm_con = 100;
         pMobIndex->perm_lck = 100;
         pMobIndex->skinamount = 0;
         pMobIndex->hide_type = 0;
         pMobIndex->race = 0;
         pMobIndex->xflags = 0;
         pMobIndex->resistant = 0;
         pMobIndex->immune = 0;
         pMobIndex->susceptible = 0;
         pMobIndex->numattacks = 0;
#ifdef XBI
         xCLEAR_BITS( pMobIndex->attacks );
         xCLEAR_BITS( pMobIndex->defenses );
#else
         pMobIndex->attacks = 0;
         pMobIndex->defenses = 0;
#endif
      }

      letter = fread_letter( fp );
      if( letter == '>' )
      {
         ungetc( letter, fp );
         mprog_read_programs( fp, pMobIndex );
      }
      else
         ungetc( letter, fp );

      if( !oldmob )
      {
         iHash = vnum % MAX_KEY_HASH;
         pMobIndex->next = mob_index_hash[iHash];
         mob_index_hash[iHash] = pMobIndex;
         top_mob_index++;
      }
   }



   return;
}



/*
 * Load an obj section.
 */
void load_objects( AREA_DATA * tarea, FILE * fp )
{
   OBJ_INDEX_DATA *pObjIndex;
   char letter;
   char *ln;
   int x1, x2, x3, x4, x5, x6, x7;

   if( !tarea )
   {
      bug( "Load_objects: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   for( ;; )
   {
      char buf[MAX_STRING_LENGTH];
      int vnum;
      int iHash;
      bool tmpBootDb;
      bool oldobj;

      letter = fread_letter( fp );
      if( letter != '#' )
      {
         bug( "Load_objects: # not found." );
         if( fBootDb )
         {
            shutdown_mud( "# not found" );
            exit( 1 );
         }
         else
            return;
      }

      vnum = fread_number( fp );
      if( vnum == 0 )
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if( get_obj_index( vnum ) )
      {
         if( tmpBootDb )
         {
            bug( "Load_objects: vnum %d duplicated.", vnum );
            shutdown_mud( "duplicate vnum" );
            exit( 1 );
         }
         else
         {
            pObjIndex = get_obj_index( vnum );
            sprintf( buf, "Cleaning object: %d", vnum );
            log_string_plus( buf, LOG_BUILD, 0 );
            clean_obj( pObjIndex );
            oldobj = TRUE;
         }
      }
      else
      {
         oldobj = FALSE;
         CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
      }
      fBootDb = tmpBootDb;

      pObjIndex->vnum = vnum;
      if( fBootDb )
      {
         if( !tarea->low_o_vnum )
            tarea->low_o_vnum = vnum;
         if( vnum > tarea->hi_o_vnum )
            tarea->hi_o_vnum = vnum;
      }
      pObjIndex->name = fread_string( fp );
      pObjIndex->short_descr = fread_string( fp );
      pObjIndex->description = fread_string( fp );
      pObjIndex->action_desc = fread_string( fp );

      pObjIndex->description[0] = UPPER( pObjIndex->description[0] );

      pObjIndex->item_type = fread_number( fp );
      pObjIndex->extra_flags = fread_bitvector( fp );

      ln = fread_line( fp );
      x1 = x2 = 0;
      sscanf( ln, "%d %d", &x1, &x2 );
      pObjIndex->tech = x1;
      pObjIndex->layers = x2;
      pObjIndex->parts = fread_bitvector( fp );

      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = x5 = x6 = 0;
      sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
      pObjIndex->value[0] = x1;
      pObjIndex->value[1] = x2;
      pObjIndex->value[2] = x3;
      pObjIndex->value[3] = x4;
      pObjIndex->value[4] = x5;
      pObjIndex->value[5] = x6;
      pObjIndex->value[6] = x7;
      pObjIndex->weight = fread_number( fp );
      pObjIndex->weight = UMAX( 1, pObjIndex->weight );
      pObjIndex->cost = fread_number( fp );
      /*
       * pObjIndex->rent         = 
       */ fread_number( fp );
      /*
       * unused 
       */
      if( area_version == 1 )
      {
         switch ( pObjIndex->item_type )
         {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
               pObjIndex->value[1] = skill_lookup( fread_word( fp ) );
               pObjIndex->value[2] = skill_lookup( fread_word( fp ) );
               pObjIndex->value[3] = skill_lookup( fread_word( fp ) );
               break;
            case ITEM_STAFF:
            case ITEM_WAND:
               pObjIndex->value[3] = skill_lookup( fread_word( fp ) );
               break;
            case ITEM_SALVE:
               pObjIndex->value[4] = skill_lookup( fread_word( fp ) );
               pObjIndex->value[5] = skill_lookup( fread_word( fp ) );
               break;
         }
      }
      for( ;; )
      {
         letter = fread_letter( fp );

         if( letter == 'A' )
         {
            AFFECT_DATA *paf;

            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = fread_number( fp );
            if( paf->location == APPLY_WEAPONSPELL
                || paf->location == APPLY_WEARSPELL
                || paf->location == APPLY_REMOVESPELL
                || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL )
               paf->modifier = slot_lookup( fread_number( fp ) );
            else
               paf->modifier = fread_number( fp );
            xCLEAR_BITS( paf->bitvector );
            LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
            top_affect++;
         }

         else if( letter == 'E' )
         {
            EXTRA_DESCR_DATA *ed;

            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword = fread_string( fp );
            ed->description = fread_string( fp );
            LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
            top_ed++;
         }
         else if( letter == '>' )
         {
            ungetc( letter, fp );
            oprog_read_programs( fp, pObjIndex );
         }

         else
         {
            ungetc( letter, fp );
            break;
         }
      }

      /*
       * Translate spell "slot numbers" to internal "skill numbers."
       */
      if( area_version == 0 )
         switch ( pObjIndex->item_type )
         {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
               pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
               pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
               pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
               break;

            case ITEM_STAFF:
            case ITEM_WAND:
               pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
               break;
            case ITEM_SALVE:
               pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
               pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
               break;
         }

      if( !oldobj )
      {
         iHash = vnum % MAX_KEY_HASH;
         pObjIndex->next = obj_index_hash[iHash];
         obj_index_hash[iHash] = pObjIndex;
         top_obj_index++;
      }
   }

   return;
}

/*
 * Load a reset section.
 */
void load_resets( AREA_DATA * tarea, FILE * fp )
{
   ROOM_INDEX_DATA *pRoomIndex = NULL;
   ROOM_INDEX_DATA *roomlist;
   bool not01 = FALSE;
   int count = 0;

   if( !tarea )
   {
      bug( "%s", "Load_resets: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   if( !tarea->first_room )
   {
      bug( "%s: No #ROOMS section found. Cannot load resets.", __FUNCTION__ );
      if( fBootDb )
      {
         shutdown_mud( "No #ROOMS" );
         exit( 1 );
      }
      else
         return;
   }

   for( ;; )
   {
      EXIT_DATA *pexit;
      char letter;
      int extra, arg1, arg2, arg3;

      if( ( letter = fread_letter( fp ) ) == 'S' )
         break;

      if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }

      extra = fread_number( fp );
      if( letter == 'M' || letter == 'O' )
         extra = 0;
      arg1 = fread_number( fp );
      arg2 = fread_number( fp );
      arg3 = ( letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
      fread_to_eol( fp );
      ++count;

      /*
       * Validate parameters.
       * We're calling the index functions for the side effect.
       */
      switch ( letter )
      {
         default:
            bug( "%s: bad command '%c'.", __FUNCTION__, letter );
            if( fBootDb )
               boot_log( "%s: %s (%d) bad command '%c'.", __FUNCTION__, tarea->filename, count, letter );
            return;

         case 'M':
            if( get_mob_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) 'M': mobile %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg1 );

            if( ( pRoomIndex = get_room_index( arg3 ) ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) 'M': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg3 );
            else
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            break;

         case 'O':
            if( get_obj_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg1 );

            if( ( pRoomIndex = get_room_index( arg3 ) ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg3 );
            else
            {
               if( !pRoomIndex )
                  bug( "%s: Unable to add room reset - room not found.", __FUNCTION__ );
               else
                  add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            }
            break;

         case 'P':
            if( get_obj_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg1 );
            if( arg3 > 0 )
            {
               if( get_obj_index( arg3 ) == NULL && fBootDb )
                  boot_log( "%s: %s (%d) 'P': destination object %d doesn't exist.", __FUNCTION__, tarea->filename, count,
                            arg3 );
               if( extra > 1 )
                  not01 = TRUE;
            }
            if( !pRoomIndex )
               bug( "%s: Unable to add room reset - room not found.", __FUNCTION__ );
            else
            {
               if( arg3 == 0 )
                  arg3 = OBJ_VNUM_MONEY_ONE; // This may look stupid, but for some reason it works.
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            }
            break;

         case 'G':
         case 'E':
            if( get_obj_index( arg1 ) == NULL && fBootDb )
               boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, tarea->filename, count, letter, arg1 );
            if( !pRoomIndex )
               bug( "%s: Unable to add room reset - room not found.", __FUNCTION__ );
            else
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            break;

         case 'T':
            if( IS_SET( extra, TRAP_OBJ ) )
               bug( "%s: Unable to add legacy object trap reset. Must be converted manually.", __FUNCTION__ );
            else
            {
               if( !( pRoomIndex = get_room_index( arg3 ) ) )
                  bug( "%s: Unable to add trap reset - room not found.", __FUNCTION__ );
               else
                  add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            }
            break;

         case 'H':
            bug( "%s: Unable to convert legacy hide reset. Must be converted manually.", __FUNCTION__ );
            break;

         case 'D':
            if( !( pRoomIndex = get_room_index( arg1 ) ) )
            {
               bug( "%s: 'D': room %d doesn't exist.", __FUNCTION__, arg1 );
               bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'D': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg1 );
               break;
            }

            if( arg2 < 0 || arg2 > MAX_DIR + 1
                || !( pexit = get_exit( pRoomIndex, arg2 ) ) || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
            {
               bug( "%s: 'D': exit %d not door.", __FUNCTION__, arg2 );
               bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'D': exit %d not door.", __FUNCTION__, tarea->filename, count, arg2 );
            }

            if( arg3 < 0 || arg3 > 2 )
            {
               bug( "%s: 'D': bad 'locks': %d.", __FUNCTION__, arg3 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'D': bad 'locks': %d.", __FUNCTION__, tarea->filename, count, arg3 );
            }
            add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            break;

         case 'R':
            if( !( pRoomIndex = get_room_index( arg1 ) ) && fBootDb )
               boot_log( "%s: %s (%d) 'R': room %d doesn't exist.", __FUNCTION__, tarea->filename, count, arg1 );
            else
               add_reset( pRoomIndex, letter, extra, arg1, arg2, arg3 );
            if( arg2 < 0 || arg2 > 10 )
            {
               bug( "%s: 'R': bad exit %d.", __FUNCTION__, arg2 );
               if( fBootDb )
                  boot_log( "%s: %s (%d) 'R': bad exit %d.", __FUNCTION__, tarea->filename, count, arg2 );
               break;
            }
            break;
      }
   }
   if( !not01 )
   {
      for( roomlist = tarea->first_room; roomlist; roomlist = roomlist->next_aroom )
         renumber_put_resets( roomlist );
   }
   return;
}

void load_room_reset( ROOM_INDEX_DATA * room, FILE * fp )
{
   EXIT_DATA *pexit;
   char letter;
   int extra, arg1, arg2, arg3;
   bool not01 = FALSE;
   int count = 0;

   letter = fread_letter( fp );
   extra = fread_number( fp );
   if( letter == 'M' || letter == 'O' )
      extra = 0;
   arg1 = fread_number( fp );
   arg2 = fread_number( fp );
   arg3 = ( letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
   fread_to_eol( fp );
   ++count;

   /*
    * Validate parameters.
    * We're calling the index functions for the side effect.
    */
   switch ( letter )
   {
      default:
         bug( "%s: bad command '%c'.", __FUNCTION__, letter );
         if( fBootDb )
            boot_log( "%s: %s (%d) bad command '%c'.", __FUNCTION__, room->area->filename, count, letter );
         return;

      case 'M':
         if( get_mob_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) 'M': mobile %d doesn't exist.", __FUNCTION__, room->area->filename, count, arg1 );
         break;

      case 'O':
         if( get_obj_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, room->area->filename, count, letter,
                      arg1 );
         break;

      case 'P':
         if( get_obj_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, room->area->filename, count, letter,
                      arg1 );

         if( arg3 <= 0 )
            arg3 = OBJ_VNUM_MONEY_ONE; // This may look stupid, but for some reason it works.
         if( get_obj_index( arg3 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) 'P': destination object %d doesn't exist.", __FUNCTION__, room->area->filename, count,
                      arg3 );
         if( extra > 1 )
            not01 = TRUE;
         break;

      case 'G':
      case 'E':
         if( get_obj_index( arg1 ) == NULL && fBootDb )
            boot_log( "%s: %s (%d) '%c': object %d doesn't exist.", __FUNCTION__, room->area->filename, count, letter,
                      arg1 );
         break;

      case 'T':
      case 'H':
         break;

      case 'D':
         if( arg2 < 0 || arg2 > MAX_DIR + 1
             || !( pexit = get_exit( room, arg2 ) ) || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
         {
            bug( "%s: 'D': exit %d not door.", __FUNCTION__, arg2 );
            bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3 );
            if( fBootDb )
               boot_log( "%s: %s (%d) 'D': exit %d not door.", __FUNCTION__, room->area->filename, count, arg2 );
         }

         if( arg3 < 0 || arg3 > 2 )
         {
            bug( "%s: 'D': bad 'locks': %d.", __FUNCTION__, arg3 );
            if( fBootDb )
               boot_log( "%s: %s (%d) 'D': bad 'locks': %d.", __FUNCTION__, room->area->filename, count, arg3 );
         }
         break;

      case 'R':
         if( arg2 < 0 || arg2 > 10 )
         {
            bug( "%s: 'R': bad exit %d.", __FUNCTION__, arg2 );
            if( fBootDb )
               boot_log( "%s: %s (%d) 'R': bad exit %d.", __FUNCTION__, room->area->filename, count, arg2 );
            break;
         }
         break;
   }
   add_reset( room, letter, extra, arg1, arg2, arg3 );

   if( !not01 )
      renumber_put_resets( room );
   return;
}

/*
 * Load a room section.
 */
void load_rooms( AREA_DATA * tarea, FILE * fp )
{
   ROOM_INDEX_DATA *pRoomIndex;
   char buf[MAX_STRING_LENGTH];
   char *ln;

   if( !tarea )
   {
      bug( "Load_rooms: no #AREA seen yet." );
      shutdown_mud( "No #AREA" );
      exit( 1 );
   }

   tarea->first_room = tarea->last_room = NULL;

   for( ;; )
   {
      int vnum;
      char letter;
      int door;
      int iHash;
      bool tmpBootDb;
      bool oldroom;
      int x1, x2, x3, x4, x5, x6, x7, x8;

      letter = fread_letter( fp );
      if( letter != '#' )
      {
         bug( "Load_rooms: # not found." );
         if( fBootDb )
         {
            shutdown_mud( "# not found" );
            exit( 1 );
         }
         else
            return;
      }

      vnum = fread_number( fp );
      if( vnum == 0 )
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if( get_room_index( vnum ) != NULL )
      {
         if( tmpBootDb )
         {
            bug( "Load_rooms: vnum %d duplicated.", vnum );
            shutdown_mud( "duplicate vnum" );
            exit( 1 );
         }
         else
         {
            pRoomIndex = get_room_index( vnum );
            sprintf( buf, "Cleaning room: %d", vnum );
            log_string_plus( buf, LOG_BUILD, 0 );
            clean_room( pRoomIndex );
            oldroom = TRUE;
         }
      }
      else
      {
         oldroom = FALSE;
         CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
         pRoomIndex->first_person = NULL;
         pRoomIndex->last_person = NULL;
         pRoomIndex->first_content = NULL;
         pRoomIndex->last_content = NULL;
         pRoomIndex->first_trail = NULL;
         pRoomIndex->last_trail = NULL;
      }

      fBootDb = tmpBootDb;
      pRoomIndex->area = tarea;
      pRoomIndex->vnum = vnum;
      pRoomIndex->pShop = NULL;
      pRoomIndex->first_extradesc = NULL;
      pRoomIndex->last_extradesc = NULL;
      pRoomIndex->first_trail = NULL;
      pRoomIndex->last_trail = NULL;

      if( fBootDb )
      {
         if( !tarea->low_r_vnum )
            tarea->low_r_vnum = vnum;
         if( vnum > tarea->hi_r_vnum )
            tarea->hi_r_vnum = vnum;
      }
      pRoomIndex->name = fread_string( fp );
      pRoomIndex->description = fread_string( fp );

      /*
       * Area number         fread_number( fp ); 
       */
      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = x5 = x6 = 0;
      sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );

      pRoomIndex->room_flags = x2;
      pRoomIndex->sector_type = x3;
      pRoomIndex->tele_delay = x4;
      pRoomIndex->tele_vnum = x5;
      pRoomIndex->runes = x6;

      /*
       * advanced sector types -- Scion 
       */
      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
      sscanf( ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
      pRoomIndex->vegetation = x1;
      pRoomIndex->curr_vegetation = x2;
      pRoomIndex->water = x3;
      pRoomIndex->curr_water = x4;
      pRoomIndex->elevation = x5;
      pRoomIndex->curr_elevation = x6;
      pRoomIndex->resources = x7;
      pRoomIndex->curr_resources = x8;


      /*
       * Resources regenerate a little on reboots 
       */
      if( pRoomIndex->curr_resources < pRoomIndex->resources )
         pRoomIndex->curr_resources += 25;

      if( pRoomIndex->curr_vegetation < pRoomIndex->vegetation )
         pRoomIndex->curr_vegetation += 25;


      if( pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX )
      {
         bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum, pRoomIndex->sector_type );
         pRoomIndex->sector_type = 1;
      }
      pRoomIndex->first_exit = NULL;
      pRoomIndex->last_exit = NULL;

      for( ;; )
      {
         letter = fread_letter( fp );

         if( letter == 'S' )
            break;

         if( letter == 'D' )
         {
            EXIT_DATA *pexit;
            int locks;

            door = fread_number( fp );
            if( door < 0 || door > 10 )
            {
               bug( "Fread_rooms: vnum %d has bad door number %d.", vnum, door );
               if( fBootDb )
                  exit( 1 );
            }
            else
            {
               pexit = make_exit( pRoomIndex, NULL, door );
               pexit->description = fread_string( fp );
               pexit->keyword = fread_string( fp );
               pexit->exit_info = 0;
               ln = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = 0;
               sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );

               locks = x1;
               pexit->key = x2;
               pexit->vnum = x3;
               pexit->vdir = door;
               pexit->distance = x4;
               pexit->pulltype = x5;
               pexit->pull = x6;

               switch ( locks )
               {
                  case 1:
                     pexit->exit_info = EX_ISDOOR;
                     break;
                  case 2:
                     pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
                     break;
                  default:
                     pexit->exit_info = locks;
               }
            }
         }
         else if( letter == 'E' )
         {
            EXTRA_DESCR_DATA *ed;

            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword = fread_string( fp );
            ed->description = fread_string( fp );
            LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc, next, prev );
            top_ed++;
         }
         else if( letter == 'R' )
            load_room_reset( pRoomIndex, fp );
         else if( letter == 'M' )   /* ignore maps */
         {
            fread_number( fp );
            fread_number( fp );
            fread_number( fp );
            fread_letter( fp );
         }
/*		MAP_DATA *map;
		MAP_INDEX_DATA *map_index;
		int i, j;

		CREATE( map, MAP_DATA, 1);
                map->vnum                     = fread_number( fp );
                map->x                        = fread_number( fp );
                map->y                        = fread_number( fp );
		map->entry		      = fread_letter( fp );

                pRoomIndex->map               = map;
		if(  (map_index = get_map_index(map->vnum)) == NULL  )
		{
                     CREATE( map_index, MAP_INDEX_DATA, 1);
		     map_index->vnum = map->vnum;
		     map_index->next = first_map;
                     first_map       = map_index;
		     for (i = 0; i <  49; i++) {
			     for (j = 0; j <  79; j++) {
			       map_index->map_of_vnums[i][j] = -1; */
         /*
          * map_index->map_of_ptrs[i][j] = NULL; 
          */
         /*
          * }
          * }
          * }
          * if( (map->y <0) || (map->y >48) )
          * {
          * bug("Map y coord out of range.  Room %d\n\r", map->y);
          * 
          * }
          * if( (map->x <0) || (map->x >78) )
          * {
          * bug("Map x coord out of range.  Room %d\n\r", map->x);
          * 
          * }
          * if(  (map->x >0)
          * &&(map->x <80)
          * &&(map->y >0)
          * &&(map->y <48) )
          * map_index->map_of_vnums[map->y][map->x]=pRoomIndex->vnum;
          * }
          */
         else if( letter == '>' )
         {
            ungetc( letter, fp );
            rprog_read_programs( fp, pRoomIndex );
         }
         else
         {
            bug( "Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum, letter );
            shutdown_mud( "Room flag not DES" );
            exit( 1 );
         }

      }

      if( !oldroom )
      {
         iHash = vnum % MAX_KEY_HASH;
         pRoomIndex->next = room_index_hash[iHash];
         room_index_hash[iHash] = pRoomIndex;
         LINK( pRoomIndex, tarea->first_room, tarea->last_room, next_aroom, prev_aroom );
         top_room++;
      }
   }

   return;
}



/*
 * Load a shop section.
 */
void load_shops( AREA_DATA * tarea, FILE * fp )
{
   SHOP_DATA *pShop;

   for( ;; )
   {
      ROOM_INDEX_DATA *pRoomIndex;

      CREATE( pShop, SHOP_DATA, 1 );
      pShop->room = fread_number( fp );
      if( pShop->room == 0 )
      {
         DISPOSE( pShop );
         break;
      }
      pShop->profit_buy = fread_number( fp );
      pShop->profit_sell = fread_number( fp );
      pShop->gold = fread_number( fp );
      pShop->type = fread_number( fp );
      pShop->flag = fread_number( fp );
      pShop->owner = fread_string( fp );
      fread_to_eol( fp );
      pRoomIndex = get_room_index( pShop->room );
      pRoomIndex->pShop = pShop;

      if( !first_shop )
         first_shop = pShop;
      else
         last_shop->next = pShop;
      pShop->next = NULL;
      pShop->prev = last_shop;
      last_shop = pShop;
      top_shop++;
   }
   return;
}

/*
 * Load spec proc declarations.
 * Modified to allow race resets as well. -keo
 */
void load_specials( AREA_DATA * tarea, FILE * fp )
{
   NATION_DATA *nation;
   GENERIC_MOB *genmob;

   for( ;; )
   {
      MOB_INDEX_DATA *pMobIndex;
      char letter;

      switch ( letter = fread_letter( fp ) )
      {
         default:
            bug( "Load_specials: letter '%c' not *MNS.", letter );
            exit( 1 );

         case 'S':
            return;

         case '*':
            break;

         case 'M':
            pMobIndex = get_mob_index( fread_number( fp ) );
            pMobIndex->spec_fun = spec_lookup( fread_word( fp ) );
            if( pMobIndex->spec_fun == 0 )
            {
               bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
               exit( 1 );
            }
            break;
         case 'N':
            nation = find_nation( fread_string( fp ) );
            if( !nation )
            {
               bug( "Load_specials: Nation not found!", 0 );
               break;
            }
            CREATE( genmob, GENERIC_MOB, 1 );
            genmob->nation = nation;
            LINK( genmob, tarea->first_race, tarea->last_race, next, prev );
            break;
      }

      fread_to_eol( fp );
   }
}


/*
 * Load soft / hard area ranges.
 */
void load_ranges( AREA_DATA * tarea, FILE * fp )
{
   int x1, x2, x3, x4;
   char *ln;

   if( !tarea )
   {
      bug( "Load_ranges: no #AREA seen yet." );
      shutdown_mud( "No #AREA" );
      exit( 1 );
   }

   for( ;; )
   {
      ln = fread_line( fp );

      if( ln[0] == '$' )
         break;

      x1 = x2 = x3 = x4 = 0;
      sscanf( ln, "%d %d %d %d", &x1, &x2, &x3, &x4 );

      tarea->low_soft_range = tarea->prev ? tarea->prev->low_soft_range + 1 : 0;
      tarea->hi_soft_range = x2;
      tarea->low_hard_range = x3;
      tarea->hi_hard_range = x4;
   }
   return;

}

/* Checks a neighbors entry for an area pointer, util for do_neighbors -- Scion */
bool dupe_neighbor( AREA_DATA * tarea, AREA_DATA * carea )
{
   NEIGHBOR_DATA *neighbor;

   for( neighbor = tarea->weather->first_neighbor; neighbor; neighbor = neighbor->next )
   {
      if( !strcmp( neighbor->name, carea->name ) )
         return TRUE;
   }
   return FALSE;
}

/* Finds all the neighbors of all the zones in the game and sets them in the climate info -- Scion */
void do_neighbors( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *room;
   int i, vnum;
   AREA_DATA *tarea;
   EXIT_DATA *pexit;
   NEIGHBOR_DATA *new;
   int lrange;
   int trange;

   send_to_char( "Clearing existing neighbor data...\r\n", ch );
   for( tarea = first_area; tarea; tarea = tarea->next )
   {
      new = tarea->weather->first_neighbor;
      while( new )
      {
         UNLINK( new, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
         new = tarea->weather->first_neighbor;
      }
   }

   for( tarea = first_area; tarea; tarea = tarea->next )
   {
      trange = tarea->hi_r_vnum;
      lrange = tarea->low_r_vnum;

      pager_printf( ch, "Finding neighbors for area: %s\r\n", tarea->filename );

      for( vnum = lrange; vnum <= trange; vnum++ )
      {
         if( ( room = get_room_index( vnum ) ) == NULL )
            continue;
         for( i = 0; i < MAX_DIR + 1; i++ )
         {
            if( ( pexit = get_exit( room, i ) ) == NULL )
               continue;
            if( pexit->to_room->area != tarea && !dupe_neighbor( tarea, pexit->to_room->area ) )
            {
               pager_printf( ch, "To: %-20.20s %s\n\r", pexit->to_room->area->filename, pexit->to_room->area->name );
               pager_printf( ch, "%15d %-30.30s -> %15d (%s)\n\r", vnum, room->name, pexit->vnum, dir_name[i] );

               CREATE( new, NEIGHBOR_DATA, 1 );
               new->next = NULL;
               new->prev = NULL;
               new->address = NULL;
               new->name = STRALLOC( pexit->to_room->area->name );;
               LINK( new, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
            }
         }
      }
   }
   send_to_char( "Done.\r\n", ch );
}

/*
 * Load climate information for the area
 * Last modified: July 13, 1997
 * Fireblade
 */
void load_climate( AREA_DATA * tarea, FILE * fp )
{
   if( !tarea )
   {
      bug( "load_climate: no #AREA seen yet" );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   tarea->weather->climate_temp = fread_number( fp );
   tarea->weather->climate_precip = fread_number( fp );
   tarea->weather->climate_wind = fread_number( fp );
   tarea->weather->climate_mana = fread_number( fp );

   return;
}

/*
 * Load data for a neghboring weather system
 * Last modified: July 13, 1997
 * Fireblade
 */
void load_neighbor( AREA_DATA * tarea, FILE * fp )
{
   NEIGHBOR_DATA *new;

   if( !tarea )
   {
      bug( "load_neighbor: no #AREA seen yet." );
      if( fBootDb )
      {
         shutdown_mud( "No #AREA" );
         exit( 1 );
      }
      else
         return;
   }

   CREATE( new, NEIGHBOR_DATA, 1 );
   new->next = NULL;
   new->prev = NULL;
   new->address = NULL;
   new->name = fread_string( fp );
   LINK( new, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );

   return;
}


/*
 * Go through all areas, and set up initial economy based on mob
 */
void initialize_economy( void )
{
   AREA_DATA *tarea;
   MOB_INDEX_DATA *mob;
   int idx, gold;

   for( tarea = first_area; tarea; tarea = tarea->next )
   {
      /*
       * skip area if they already got some gold 
       */
      if( tarea->high_economy > 0 || tarea->low_economy > 10000 )
         continue;
      gold = 100000000;
      boost_economy( tarea, gold );
      for( idx = tarea->low_m_vnum; idx < tarea->hi_m_vnum; idx++ )
         if( ( mob = get_mob_index( idx ) ) != NULL )
            boost_economy( tarea, mob->gold * 10 );
   }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
   ROOM_INDEX_DATA *pRoomIndex;
   EXIT_DATA *pexit, *pexit_next, *rev_exit;
   int iHash;

   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
      for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      {
         bool fexit;

         fexit = FALSE;
         for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
         {
            pexit_next = pexit->next;
            pexit->rvnum = pRoomIndex->vnum;
            if( pexit->vnum <= 0 || ( pexit->to_room = get_room_index( pexit->vnum ) ) == NULL )
            {
               if( fBootDb )
                  boot_log( "Fix_exits: room %d, exit %s leads to bad vnum (%d)",
                            pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum );

               bug( "Deleting %s exit in room %d", dir_name[pexit->vdir], pRoomIndex->vnum );
               extract_exit( pRoomIndex, pexit );
            }
            else
               fexit = TRUE;
         }
/*	    if ( !fexit ) -- Leave the damn room flags alone -- Scion
	      SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
*/ }
   }

   /*
    * Set all the rexit pointers   -Thoric 
    */
   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
      for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      {
         for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
         {
            if( pexit->to_room && !pexit->rexit )
            {
               rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
               if( rev_exit )
               {
                  pexit->rexit = rev_exit;
                  rev_exit->rexit = pexit;
               }
            }
         }
      }
   }

   return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
EXIT_DATA *get_exit_number( ROOM_INDEX_DATA * room, int xit )
{
   EXIT_DATA *pexit;
   int count;

   count = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
      if( ++count == xit )
         return pexit;
   return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA ** xit1, EXIT_DATA ** xit2 )
{
   int d1, d2;

   d1 = ( *xit1 )->vdir;
   d2 = ( *xit2 )->vdir;

   if( d1 < d2 )
      return -1;
   if( d1 > d2 )
      return 1;
   return 0;
}

void sort_exits( ROOM_INDEX_DATA * room )
{
   EXIT_DATA *pexit; /* *texit *//* Unused */
   EXIT_DATA *exits[MAX_REXITS];
   int x, nexits;

   nexits = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
   {
      exits[nexits++] = pexit;
      if( nexits > MAX_REXITS )
      {
         bug( "sort_exits: more than %d exits in room... fatal", nexits );
         return;
      }
   }
   qsort( &exits[0], nexits, sizeof( EXIT_DATA * ), ( int ( * )( const void *, const void * ) )exit_comp );
   for( x = 0; x < nexits; x++ )
   {
      if( x > 0 )
         exits[x]->prev = exits[x - 1];
      else
      {
         exits[x]->prev = NULL;
         room->first_exit = exits[x];
      }
      if( x >= ( nexits - 1 ) )
      {
         exits[x]->next = NULL;
         room->last_exit = exits[x];
      }
      else
         exits[x]->next = exits[x + 1];
   }
}

void randomize_exits( ROOM_INDEX_DATA * room, sh_int maxdir )
{
   EXIT_DATA *pexit;
   int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
   int vdirs[MAX_REXITS];

   nexits = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
      vdirs[nexits++] = pexit->vdir;

   for( d0 = 0; d0 < nexits; d0++ )
   {
      if( vdirs[d0] > maxdir )
         continue;
      count = 0;
      while( vdirs[( d1 = number_range( d0, nexits - 1 ) )] > maxdir || ++count > 5 );
      if( vdirs[d1] > maxdir )
         continue;
      door = vdirs[d0];
      vdirs[d0] = vdirs[d1];
      vdirs[d1] = door;
   }
   count = 0;
   for( pexit = room->first_exit; pexit; pexit = pexit->next )
      pexit->vdir = vdirs[count++];

   sort_exits( room );
}


/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
   AREA_DATA *pArea;

   for( pArea = first_area; pArea; pArea = pArea->next )
   {
/*	CHAR_DATA *pch; */
      int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

      if( ( reset_age == -1 && pArea->age == -1 ) || ++pArea->age < ( reset_age - 1 ) )
         continue;

      /*
       * Check for PC's.
       */
/*	if ( pArea->nplayer > 0 && pArea->age == (reset_age-1) )
	{
	    char buf[MAX_STRING_LENGTH]; */

      /*
       * Rennard 
       */
/*	    if ( pArea->resetmsg )
		sprintf( buf, "%s\n\r", pArea->resetmsg );
	    else
		strcpy( buf, "You hear some squeaking sounds...\n\r" );
	    for ( pch = first_char; pch; pch = pch->next )
	    {
		if ( !IS_NPC(pch)
		&&   IS_AWAKE(pch)
		&&   pch->in_room
		&&   pch->in_room->area == pArea
		&&   (pch->desc
		&&    pch->desc->connected == CON_PLAYING ) )
		{
		    set_char_color( AT_RESET, pch );
		    send_to_char( buf, pch );
		}
	    }


		continue;
	}*/

      /*
       * Check age and reset.
       * Note: Mud Academy resets every 3 minutes (not 15).
       */

/* NEVER RESET if the total number of objects is less than the number of areas * 75  -- Scion */
      if( ( pArea->nplayer == 0 || pArea->age >= reset_age ) &&
          ( ( physicalobjects < ( top_area * MAX_OBJS ) ) || ( nummobsloaded < ( top_area * MAX_MOBS ) ) ) )
      {
         ROOM_INDEX_DATA *pRoomIndex;

/* thought I took this spammy log complicating i/o generating crap out once
	shogar

	    fprintf( stderr, "Resetting: %s\n", pArea->filename );
*/
         reset_area( pArea );

         if( reset_age == -1 )
            pArea->age = -1;
         else
            pArea->age = number_range( 0, reset_age / 5 );

         pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );

         if( pRoomIndex != NULL && pArea == pRoomIndex->area && pArea->reset_frequency == 0 )
            pArea->age = 15 - 3;
      }
   }
   return;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA * pMobIndex )
{
   CHAR_DATA *mob;

   if( !pMobIndex )
   {
      bug( "Create_mobile: NULL pMobIndex." );
      return NULL;
   }

   CREATE( mob, CHAR_DATA, 1 );
   clear_char( mob );
   mob->pIndexData = pMobIndex;
   mob->in_obj = NULL;
   mob->editor = NULL;

   /*
    * Generate randomized names if applicable -- Scion 
    */


   mob->description = QUICKLINK( pMobIndex->description );
   mob->spec_fun = pMobIndex->spec_fun;
   mob->mpscriptpos = 0;
   mob->act = pMobIndex->act;

   if( xIS_SET( mob->act, ACT_MOBINVIS ) )
      mob->mobinvis = 1000;

   mob->affected_by = pMobIndex->affected_by;
   mob->mood = MOOD_RELAXED;
   mob->last_hit = NULL;
   if( pMobIndex->sex == 0 )
      mob->sex = number_range( 1, 2 );
   else
      mob->sex = pMobIndex->sex;

   if( mob->height == 0 )
      mob->height = 500;
   if( mob->weight == 0 )
      mob->weight = 2000;
   mob->armor = pMobIndex->ac;

   /*
    * lets put things back the way they used to be! -Thoric 
    */
   mob->gold = pMobIndex->gold;
   mob->exp = pMobIndex->exp;
   mob->position = POS_STANDING;
   mob->barenumdie = pMobIndex->damnodice;
   mob->baresizedie = pMobIndex->damsizedice;
   mob->mobthac0 = pMobIndex->mobthac0;
   mob->hitplus = pMobIndex->hitplus;
   mob->damplus = pMobIndex->damplus;

   mob->perm_str = pMobIndex->perm_str;
   mob->perm_dex = pMobIndex->perm_dex;
   mob->perm_wil = pMobIndex->perm_wil;
   mob->perm_int = pMobIndex->perm_int;
   mob->perm_con = pMobIndex->perm_con;
   mob->perm_per = pMobIndex->perm_per;
   mob->perm_lck = pMobIndex->perm_lck;
   mob->skinamount = pMobIndex->skinamount;
   mob->hitroll = pMobIndex->hitroll;
   mob->damroll = pMobIndex->damroll;
   mob->race = pMobIndex->race;
   mob->xflags = pMobIndex->xflags;
   mob->saving_poison_death = pMobIndex->saving_poison_death;
   mob->saving_wand = pMobIndex->saving_wand;
   mob->saving_para_petri = pMobIndex->saving_para_petri;
   mob->saving_breath = pMobIndex->saving_breath;
   mob->saving_spell_staff = pMobIndex->saving_spell_staff;
   mob->height = pMobIndex->height;
   mob->weight = pMobIndex->weight;

   /*
    * Must be done after the race is set 
    */
   if( xIS_SET( mob->act, ACT_NAMED ) )
   {
      char buf[MAX_STRING_LENGTH];
      char name[MAX_STRING_LENGTH];

      if( mob->race == 1 || mob->race == 4 )
         strcpy( name, random_name( name, LANG_ELVEN ) );
      else if( mob->race == 2 )
         strcpy( name, random_name( name, LANG_DWARVEN ) );
      else if( mob->race == 11 )
         strcpy( name, random_name( name, LANG_DROW ) );
      else if( mob->race == 35 )
         strcpy( name, random_name( name, LANG_DRAGON ) );
      else
         strcpy( name, random_name( name, LANG_KALORESE ) );

      sprintf( buf, pMobIndex->player_name, name );
      mob->name = STRALLOC( buf );

      sprintf( buf, pMobIndex->short_descr, name );
      mob->short_descr = STRALLOC( buf );
   }
   else
   {
      mob->name = QUICKLINK( pMobIndex->player_name );
      mob->short_descr = QUICKLINK( pMobIndex->short_descr );
   }

   /*
    * base hp's on con -keo 
    */
   if( !pMobIndex->hitnodice )
      mob->max_hit = UMAX( get_curr_con( mob ) * 30, 100 );
   else
      mob->max_hit = pMobIndex->hitnodice * number_range( 1, pMobIndex->hitsizedice ) + pMobIndex->hitplus;
   mob->hit = mob->max_hit;
   mob->move = get_curr_end( mob ) * 30;
   mob->max_move = mob->move;

   /*
    * Give 'em a brand new body! -- Scion 
    */
   check_bodyparts( mob );

   mob->resistant = pMobIndex->resistant;
   mob->immune = pMobIndex->immune;
   mob->susceptible = pMobIndex->susceptible;
   mob->attacks = pMobIndex->attacks;
   mob->defenses = pMobIndex->defenses;
   mob->numattacks = pMobIndex->numattacks;
   mob->home_room = -1;

   /*
    * Perhaps add this to the index later --Shaddai
    */
   xCLEAR_BITS( mob->no_affected_by );
   mob->no_resistant = 0;
   mob->no_immune = 0;
   mob->no_susceptible = 0;

   /*
    * Insert in list.
    */
   add_char( mob );
   pMobIndex->count++;
   nummobsloaded++;
   return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA * pObjIndex, int level )
{
   OBJ_DATA *obj;

   if( !pObjIndex )
   {
      bug( "Create_object: NULL pObjIndex." );
      return NULL;
   }

   CREATE( obj, OBJ_DATA, 1 );

   obj->pIndexData = pObjIndex;
   obj->in_room = NULL;
   obj->wear_loc = -1;
   obj->count = 1;
   cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
   obj->serial = obj->pIndexData->serial = cur_obj_serial;

   obj->name = QUICKLINK( pObjIndex->name );
   obj->short_descr = QUICKLINK( pObjIndex->short_descr );
   obj->description = QUICKLINK( pObjIndex->description );
   obj->action_desc = QUICKLINK( pObjIndex->action_desc );
   obj->obj_by = NULL;
   obj->item_type = pObjIndex->item_type;
   obj->extra_flags = pObjIndex->extra_flags;
   obj->parts = pObjIndex->parts;
   obj->value[0] = pObjIndex->value[0];
   obj->value[1] = pObjIndex->value[1];
   obj->value[2] = pObjIndex->value[2];
   obj->value[3] = pObjIndex->value[3];
   obj->value[4] = pObjIndex->value[4];
   obj->value[5] = pObjIndex->value[5];
   obj->value[6] = pObjIndex->value[6];
   obj->weight = pObjIndex->weight;
   obj->mana = 0;
   obj->raw_mana = 0;
   obj->cost = pObjIndex->cost;
   obj->condition = obj->weight * 50;
   obj->material = NULL;
   obj->gem = NULL;
   obj->size = 66;

   /*
    * Mess with object properties.
    */
   switch ( obj->item_type )
   {
      default:
         bug( "Read_object: vnum %d bad type: %d.", pObjIndex->vnum, obj->item_type );
         break;

      case ITEM_LIGHT:
      case ITEM_TREASURE:
      case ITEM_FURNITURE:
      case ITEM_RADIO:
      case ITEM_SCANNER:
      case ITEM_TRASH:
      case ITEM_CONTAINER:
      case ITEM_DRINK_CON:
      case ITEM_KEY:
      case ITEM_KEYRING:
      case ITEM_ODOR:
      case ITEM_INSTRUMENT:
      case ITEM_VEHICLE:
      case ITEM_BANK:
         break;
      case ITEM_COOK:
      case ITEM_FOOD:
         /*
          * optional food condition (rotting food)    -Thoric
          * value1 is the max condition of the food
          * value4 is the optional initial condition
          */
         if( obj->value[4] )
            obj->timer = obj->value[4];
         else
            obj->timer = obj->value[1];
         break;
      case ITEM_BOAT:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
      case ITEM_FOUNTAIN:
      case ITEM_BLOOD:
      case ITEM_BLOODSTAIN:
      case ITEM_SCRAPS:
      case ITEM_PIPE:
      case ITEM_HERB_CON:
      case ITEM_HERB:
      case ITEM_INCENSE:
      case ITEM_FIRE:
      case ITEM_BOOK:
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
      case ITEM_BUTTON:
      case ITEM_DIAL:
      case ITEM_RUNE:
      case ITEM_RUNEPOUCH:
      case ITEM_SHEATH:
      case ITEM_TRAP:
      case ITEM_MAP:
      case ITEM_PORTAL:
      case ITEM_PAPER:
      case ITEM_PEN:
      case ITEM_TINDER:
      case ITEM_LOCKPICK:
      case ITEM_LOCK:
      case ITEM_DISEASE:
      case ITEM_OIL:
      case ITEM_FUEL:
      case ITEM_QUIVER:
      case ITEM_SHOVEL:
      case ITEM_PROJECTILE:
      case ITEM_CARVINGKNIFE:
         break;

      case ITEM_SALVE:
         obj->value[3] = number_fuzzy( obj->value[3] );
         break;

      case ITEM_SCROLL:
         obj->value[0] = number_fuzzy( obj->value[0] );
         break;

      case ITEM_WAND:
      case ITEM_STAFF:
         obj->value[0] = number_fuzzy( obj->value[0] );
         obj->value[1] = number_fuzzy( obj->value[1] );
         obj->value[2] = obj->value[1];
         break;

      case ITEM_WEAPON:
      case ITEM_MISSILE_WEAPON:
         if( obj->value[2] )
            obj->value[2] *= obj->value[2];
         else
            obj->value[2] = number_range( 20, 50 );
         break;

      case ITEM_ARMOR:
         if( obj->value[1] == 0 )
            obj->value[1] = obj->value[0];
         break;

      case ITEM_POTION:
      case ITEM_PILL:
         obj->value[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
         break;

      case ITEM_MONEY:
         obj->value[0] = obj->cost;
         if( obj->value[0] == 0 )
            obj->value[0] = 1;
         break;
   }

   LINK( obj, first_object, last_object, next, prev );
   ++pObjIndex->count;
   ++numobjsloaded;
   ++physicalobjects;

   return obj;
}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA * ch )
{
   int i;

   ch->editor = NULL;
   ch->hunting = NULL;
   ch->fearing = NULL;
   ch->hating = NULL;
   ch->name = NULL;
   ch->short_descr = NULL;
   ch->description = NULL;
   ch->next = NULL;
   ch->prev = NULL;
   ch->reply = NULL;
   ch->retell = NULL;
   ch->first_carrying = NULL;
   ch->last_carrying = NULL;
   ch->next_in_room = NULL;
   ch->prev_in_room = NULL;
   ch->switched = NULL;
   ch->first_affect = NULL;
   ch->last_affect = NULL;
   ch->prev_cmd = NULL; /* maps */
   ch->last_cmd = NULL;
   ch->dest_buf = NULL;
   ch->alloc_ptr = NULL;
   ch->spare_ptr = NULL;
   ch->mount = NULL;
   xCLEAR_BITS( ch->affected_by );
   ch->logon = current_time;
   ch->armor = 0;
   ch->position = POS_STANDING;
   ch->hit = 20;
   ch->max_hit = 20;
   ch->mana = 100;
   ch->max_mana = 100;
   ch->move = 100;
   ch->max_move = 100;
   ch->height = 0;
   ch->weight = 0;
   ch->xflags = 0;
   ch->race = 0;

   for( i = 0; i < MAX_DEITY; i++ )
   {
      ch->talent[i] = -1;
      ch->curr_talent[i] = -1;
   }

   ch->barenumdie = 1;
   ch->baresizedie = 4;
   ch->substate = 0;
   ch->tempnum = 0;
   ch->perm_str = 23;
   ch->perm_dex = 23;
   ch->perm_int = 23;
   ch->perm_wil = 23;
   ch->perm_per = 23;
   ch->perm_con = 23;
   ch->perm_lck = 23;
   ch->skinamount = 0;
   ch->mod_str = 0;
   ch->mod_dex = 0;
   ch->mod_int = 0;
   ch->mod_wis = 0;
   ch->mod_cha = 0;
   ch->mod_con = 0;
   ch->mod_lck = 0;
   ch->pagelen = 24; /* BUILD INTERFACE */
   return;
}

/*
 * Free a character.
 */
void free_char( CHAR_DATA * ch )
{
   AFFECT_DATA *paf;
   TIMER *timer;
   MPROG_ACT_LIST *mpact, *mpact_next;

   if( !ch )
   {
      bug( "Free_char: null ch!" );
      return;
   }

   if( ch->desc )
      bug( "Free_char: char still has descriptor." );

   while( ( paf = ch->last_affect ) != NULL )
      affect_remove( ch, paf );

   while( ( timer = ch->first_timer ) != NULL )
      extract_timer( ch, timer );

   if( ch->editor )
      stop_editing( ch );

   STRFREE( ch->name );
   STRFREE( ch->short_descr );
   STRFREE( ch->description );

   stop_hunting( ch );
   stop_hating( ch );
   stop_fearing( ch );

   if( ch->pcdata )
   {

      STRFREE( ch->pcdata->filename );
      STRFREE( ch->pcdata->deity_name );
      DISPOSE( ch->pcdata->pwd );   /* no hash */
      DISPOSE( ch->pcdata->bamfin );   /* no hash */
      DISPOSE( ch->pcdata->bamfout );  /* no hash */
      DISPOSE( ch->pcdata->rank );
      STRFREE( ch->pcdata->title );
      STRFREE( ch->pcdata->bio );
      DISPOSE( ch->pcdata->homepage ); /* no hash */
      STRFREE( ch->pcdata->prompt );
      STRFREE( ch->pcdata->fprompt );
      if( ch->pcdata->helled_by )
         STRFREE( ch->pcdata->helled_by );
      if( ch->pcdata->subprompt )
         STRFREE( ch->pcdata->subprompt );
#ifdef IMC
      imc_freechardata( ch );
#endif
#ifdef I3
      free_i3chardata( ch );
#endif
      DISPOSE( ch->pcdata );
   }

   STRFREE( ch->last_taken );
   STRFREE( ch->species );

   for( mpact = ch->mpact; mpact; mpact = mpact_next )
   {
      mpact_next = mpact->next;
      DISPOSE( mpact->buf );
      DISPOSE( mpact );
   }

   DISPOSE( ch );
   return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA * ed )
{
   for( ; ed; ed = ed->next )
      if( is_name( name, ed->keyword ) )
         return ed->description;

   return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
   MOB_INDEX_DATA *pMobIndex;

   if( vnum < 0 )
      vnum = 0;

   for( pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex; pMobIndex = pMobIndex->next )
      if( pMobIndex->vnum == vnum )
         return pMobIndex;

/*    if ( fBootDb )
	bug( "Get_mob_index: bad vnum %d.", vnum );
*/
   return NULL;
}



/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
   OBJ_INDEX_DATA *pObjIndex;

   if( vnum < 0 )
      vnum = 0;

   for( pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex; pObjIndex = pObjIndex->next )
      if( pObjIndex->vnum == vnum )
         return pObjIndex;

/*    if ( fBootDb )
	bug( "Get_obj_index: bad vnum %d.", vnum );
*/
   return NULL;
}



/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
   ROOM_INDEX_DATA *pRoomIndex;

   if( vnum < 0 )
      vnum = 0;

   for( pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      if( pRoomIndex->vnum == vnum )
         return pRoomIndex;

   if( fBootDb )
      bug( "Get_room_index: bad vnum %d.", vnum );

   return NULL;
}



/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */


/*
 * Read a letter from a file.
 */
char fread_letter( FILE * fp )
{
   char c;

   do
   {
      if( feof( fp ) )
      {
         bug( "fread_letter: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         return '\0';
      }
      c = getc( fp );
   }
   while( isspace( (int)c ) );

   return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE * fp )
{
   int number;
   bool sign;
   char c;

   do
   {
      if( feof( fp ) )
      {
         bug( "fread_number: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         return 0;
      }
      c = getc( fp );
   }
   while( isspace( (int)c ) );

   number = 0;

   sign = FALSE;
   if( c == '+' )
   {
      c = getc( fp );
   }
   else if( c == '-' )
   {
      sign = TRUE;
      c = getc( fp );
   }

   if( !isdigit( (int)c ) )
   {
      bug( "Fread_number: bad format. (%c)", c );
      if( fBootDb )
         exit( 1 );
      return 0;
   }

   while( isdigit( (int)c ) )
   {
      if( feof( fp ) )
      {
         bug( "fread_number: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         return number;
      }
      number = number * 10 + c - '0';
      c = getc( fp );
   }

   if( sign )
      number = 0 - number;

   if( c == '|' )
      number += fread_number( fp );
   else if( c != ' ' )
      ungetc( c, fp );

   return number;
}

/*
 * custom str_dup using create					-Thoric
 */
char *str_dup( char const *str )
{
   static char *ret;
   int len;

   if( !str )
      return NULL;

   len = strlen( str ) + 1;

   CREATE( ret, char, len );
   strcpy( ret, str );
   return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *plast;
   char c;
   int ln;

   plast = buf;
   buf[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if( feof( fp ) )
      {
         bug( "fread_string: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         return STRALLOC( "" );
      }
      c = getc( fp );
   }
   while( isspace( (int)c ) );

   if( ( *plast++ = c ) == '~' )
      return STRALLOC( "" );

   for( ;; )
   {
      if( ln >= ( MAX_STRING_LENGTH - 1 ) )
      {
         bug( "fread_string: string too long" );
         *plast = '\0';
         return STRALLOC( buf );
      }
      switch ( *plast = getc( fp ) )
      {
         default:
            plast++;
            ln++;
            break;

         case EOF:
            bug( "Fread_string: EOF" );
            if( fBootDb )
               exit( 1 );
            *plast = '\0';
            return STRALLOC( buf );
            break;

         case '\n':
            plast++;
            ln++;
            *plast++ = '\r';
            ln++;
            break;

         case '\r':
            break;

         case '~':
            *plast = '\0';
            return STRALLOC( buf );
      }
   }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *plast;
   char c;
   int ln;

   plast = buf;
   buf[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if( feof( fp ) )
      {
         bug( "fread_string_no_hash: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         return str_dup( "" );
      }
      c = getc( fp );
   }
   while( isspace( (int)c ) );

   if( ( *plast++ = c ) == '~' )
      return str_dup( "" );

   for( ;; )
   {
      if( ln >= ( MAX_STRING_LENGTH - 1 ) )
      {
         bug( "fread_string_no_hash: string too long" );
         *plast = '\0';
         return str_dup( buf );
      }
      switch ( *plast = getc( fp ) )
      {
         default:
            plast++;
            ln++;
            break;

         case EOF:
            bug( "Fread_string_no_hash: EOF" );
            if( fBootDb )
               exit( 1 );
            *plast = '\0';
            return str_dup( buf );
            break;

         case '\n':
            plast++;
            ln++;
            *plast++ = '\r';
            ln++;
            break;

         case '\r':
            break;

         case '~':
            *plast = '\0';
            return str_dup( buf );
      }
   }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE * fp )
{
   char c;

   do
   {
      if( feof( fp ) )
      {
         bug( "fread_to_eol: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         return;
      }
      c = getc( fp );
   }
   while( c != '\n' && c != '\r' );

   do
   {
      c = getc( fp );
   }
   while( c == '\n' || c == '\r' );

   ungetc( c, fp );
   return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE * fp )
{
   static char line[MAX_STRING_LENGTH];
   char *pline;
   char c;
   int ln;

   pline = line;
   line[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if( feof( fp ) )
      {
         bug( "fread_line: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         strcpy( line, "" );
         return line;
      }
      c = getc( fp );
   }
   while( isspace( (int)c ) );

   ungetc( c, fp );
   do
   {
      if( feof( fp ) )
      {
         bug( "fread_line: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         *pline = '\0';
         return line;
      }
      c = getc( fp );
      *pline++ = c;
      ln++;
      if( ln >= ( MAX_STRING_LENGTH - 1 ) )
      {
         bug( "fread_line: line too long" );
         break;
      }
   }
   while( c != '\n' && c != '\r' );

   do
   {
      c = getc( fp );
   }
   while( c == '\n' || c == '\r' );

   ungetc( c, fp );
   *pline = '\0';
   return line;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE * fp )
{
   static char word[MAX_INPUT_LENGTH];
   char *pword;
   char cEnd;

   do
   {
      if( feof( fp ) )
      {
         bug( "fread_word: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         word[0] = '\0';
         return word;
      }
      cEnd = getc( fp );
   }
   while( isspace( (int)cEnd ) );

   if( cEnd == '\'' || cEnd == '"' )
   {
      pword = word;
   }
   else
   {
      word[0] = cEnd;
      pword = word + 1;
      cEnd = ' ';
   }

   for( ; pword < word + MAX_INPUT_LENGTH; pword++ )
   {
      if( feof( fp ) )
      {
         bug( "fread_word: EOF encountered on read.\n\r" );
         if( fBootDb )
            exit( 1 );
         *pword = '\0';
         return word;
      }
      *pword = getc( fp );
      if( cEnd == ' ' ? isspace( (int)*pword ) : *pword == cEnd )
      {
         if( cEnd == ' ' )
            ungetc( *pword, fp );
         *pword = '\0';
         return word;
      }
   }

   bug( "Fread_word: word too long" );
   exit( 1 );
   return NULL;
}

void do_memory( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int hash;

   set_char_color( AT_PLAIN, ch );
   argument = one_argument( argument, arg );
   send_to_char_color( "\n\r&GSystem Memory [arguments - hash, check, showhigh]\n\r", ch );
   ch_printf_color( ch, "&gAffects: &G%5d     &gAreas:   &G%5d\n\r", top_affect, top_area );
   ch_printf_color( ch, "&gExtDes:  &G%5d     &gExits:   &G%5d\n\r", top_ed, top_exit );
   ch_printf_color( ch, "&gHelps:   &G%5d     &gResets:  &G%5d\n\r", top_help, top_reset );
   ch_printf_color( ch, "&gIdxMobs: &G%5d     &gMobiles: %s%5d\n\r",
                    top_mob_index, ( MAX_MOBS * top_area >= nummobsloaded ? "&G" : "&R" ), nummobsloaded );
   ch_printf_color( ch, "&gIdxObjs: &G%5d     &gObjs:    %s%5d(%d)\n\r",
                    top_obj_index, ( MAX_OBJS * top_area >= numobjsloaded ? "&G" : "&R" ), numobjsloaded, physicalobjects );
   ch_printf_color( ch, "&gRooms:   &G%5d     &gShops:   &G%5d\n\r", top_room, top_shop );
   ch_printf_color( ch, "&gCurOq's: &G%5d     &gCurCq's: &G%5d\n\r", cur_qobjs, cur_qchars );
   ch_printf_color( ch, "&gPlayers: &G%5d     &gMaxplrs: &G%5d\n\r", num_descriptors, sysdata.maxplayers );
   ch_printf_color( ch, "&gMaxEver: &G%5d     &gTopsn:   &G%5d(%d)\n\r", sysdata.alltimemax, top_sn, MAX_SKILL );
   ch_printf_color( ch, "&gMaxEver was recorded on:  &G%s\n\r\n\r", sysdata.time_of_max );
/* Ntanel - Codebase version ID */
   ch_printf_color( ch, "&wVersion ID:  &WCalareyMUD 3.0 Revision 1\n\r" );
   ch_printf_color( ch, "&wDev. Date:   &WJune 14, 2004\n\r\n\r" );

   if( !str_cmp( arg, "check" ) )
   {
#ifdef HASHSTR
      send_to_char( check_hash( argument ), ch );
#else
      send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
      return;
   }
   if( !str_cmp( arg, "showhigh" ) )
   {
#ifdef HASHSTR
      show_high_hash( atoi( argument ) );
#else
      send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
      return;
   }
   if( argument[0] != '\0' )
      hash = atoi( argument );
   else
      hash = -1;
   if( !str_cmp( arg, "hash" ) )
   {
#ifdef HASHSTR
      ch_printf( ch, "Hash statistics:\n\r%s", hash_stats(  ) );
      if( hash != -1 )
         hash_dump( hash );
#else
      send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
   }
   return;
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
   switch ( number_bits( 2 ) )
   {
      case 0:
         number -= 1;
         break;
      case 3:
         number += 1;
         break;
   }

   return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
/*    int power;
    int number;*/

   if( ( to = to - from + 1 ) <= 1 )
      return from;

/*    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm( ) & (power - 1) ) >= to )
	;

    return from + number;*/
   return ( number_mm(  ) % to ) + from;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
/*    int percent;

    while ( ( percent = number_mm( ) & (128-1) ) > 99 )
	;

    return 1 + percent;*/
   return number_mm(  ) % 100;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
   int door;

   while( ( door = number_mm(  ) & ( 16 - 1 ) ) > 9 )
      ;

   return door;
/*    return number_mm() & 10; */
}



int number_bits( int width )
{
   return number_mm(  ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];

void init_mm(  )
{
   int *piState;
   int iState;

   piState = &rgiState[2];

   piState[-2] = 55 - 55;
   piState[-1] = 55 - 24;

   piState[0] = ( ( int )current_time ) & ( ( 1 << 30 ) - 1 );
   piState[1] = 1;
   for( iState = 2; iState < 55; iState++ )
   {
      piState[iState] = ( piState[iState - 1] + piState[iState - 2] ) & ( ( 1 << 30 ) - 1 );
   }
   return;
}



int number_mm( void )
{
   int *piState;
   int iState1;
   int iState2;
   int iRand;

   piState = &rgiState[2];
   iState1 = piState[-2];
   iState2 = piState[-1];
   iRand = ( piState[iState1] + piState[iState2] ) & ( ( 1 << 30 ) - 1 );
   piState[iState1] = iRand;
   if( ++iState1 == 55 )
      iState1 = 0;
   if( ++iState2 == 55 )
      iState2 = 0;
   piState[-2] = iState1;
   piState[-1] = iState2;
   return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
   int idice;
   int sum;

   switch ( size )
   {
      case 0:
         return 0;
      case 1:
         return number;
   }

   for( idice = 0, sum = 0; idice < number; idice++ )
      sum += number_range( 1, size );

   return sum;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
   for( ; *str != '\0'; str++ )
      if( *str == '~' )
         *str = '-';

   return;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
   for( ; *str != '\0'; str++ )
      if( *str == '~' )
         *str = HIDDEN_TILDE;

   return;
}

char *show_tilde( char *str )
{
   static char buf[MAX_STRING_LENGTH];
   char *bufptr;

   bufptr = buf;
   for( ; *str != '\0'; str++, bufptr++ )
   {
      if( *str == HIDDEN_TILDE )
         *bufptr = '~';
      else
         *bufptr = *str;
   }
   *bufptr = '\0';

   return buf;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
   if( !astr )
   {
      bug( "Str_cmp: null astr, %s bstr.", bstr );
      if( bstr )
         fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
      return TRUE;
   }

   if( !bstr )
   {
      bug( "Str_cmp: null bstr, %s astr.", astr );
      if( astr )
         fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
      return TRUE;
   }

   for( ; *astr || *bstr; astr++, bstr++ )
   {
      if( LOWER( *astr ) != LOWER( *bstr ) )
         return TRUE;
   }

   return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
   if( !astr )
   {
      bug( "Strn_cmp: null astr." );
      return TRUE;
   }

   if( !bstr )
   {
      bug( "Strn_cmp: null bstr." );
      return TRUE;
   }

   for( ; *astr; astr++, bstr++ )
   {
      if( LOWER( *astr ) != LOWER( *bstr ) )
         return TRUE;
   }

   return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
   int sstr1;
   int sstr2;
   int ichar;
   char c0;

   if( ( c0 = LOWER( astr[0] ) ) == '\0' )
      return FALSE;

   sstr1 = strlen( astr );
   sstr2 = strlen( bstr );

   for( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
      if( c0 == LOWER( bstr[ichar] ) && !str_prefix( astr, bstr + ichar ) )
         return FALSE;

   return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
   int sstr1;
   int sstr2;

   sstr1 = strlen( astr );
   sstr2 = strlen( bstr );
   if( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
      return FALSE;
   else
      return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
   static char strcap[MAX_STRING_LENGTH];
   int i;

   for( i = 0; str[i] != '\0'; i++ )
   {
      if( i && str[i - 1] != '&' && str[i - 1] != '^' )  /* color fix - shogar */
         strcap[i] = LOWER( str[i] );
      else
         strcap[i] = str[i];
   }
   strcap[i] = '\0';
   strcap[0] = UPPER( strcap[0] );
   return strcap;
}


/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
   static char strlow[MAX_STRING_LENGTH];
   int i;

   for( i = 0; str[i] != '\0'; i++ )
      strlow[i] = LOWER( str[i] );
   strlow[i] = '\0';
   return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
   static char strup[MAX_STRING_LENGTH];
   int i;

   for( i = 0; str[i] != '\0'; i++ )
      strup[i] = UPPER( str[i] );
   strup[i] = '\0';
   return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel			-Thoric
 */
bool isavowel( char letter )
{
   char c;

   c = LOWER( letter );
   if( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
      return TRUE;
   else
      return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
char *aoran( const char *str )
{
   static char temp[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];

   if( !str )
   {
      bug( "Aoran(): NULL str" );
      return "";
   }

   strcpy( buf, str );
   strcpy( buf, munch_colors( buf ) );

   /*
    * If the string begins with 'some', don't touch it. As in 'some marbles' -- Scion 
    */
   if( LOWER( str[0] ) == 's' && LOWER( str[1] ) == 'o' && LOWER( str[2] ) == 'm' && LOWER( str[3] ) == 'e' )
   {
      strcpy( temp, buf );
      return temp;
   }

   if( isavowel( buf[0] ) || ( strlen( buf ) > 1 && LOWER( buf[0] ) == 'y' && !isavowel( buf[1] ) ) )
      strcpy( temp, "an " );
   else
      strcpy( temp, "a " );
   strcat( temp, str );
   return temp;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA * ch, char *file, char *str )
{
   FILE *fp;

   if( IS_NPC( ch ) || str[0] == '\0' )
      return;

   fclose( fpLOG );
   fpLOG = NULL;  /* redhat 5.1 fix - shogar */
   if( ( fp = fopen( file, "a" ) ) == NULL )
   {
      perror( file );
      send_to_char( "Could not open the file!\n\r", ch );
   }
   else
   {
      fprintf( fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
      fclose( fp );
   }

   fpLOG = fopen( NULL_FILE, "r" );
   return;
}

/*
 * Append a string to a file.
 */
void append_to_file( char *file, char *str )
{
   FILE *fp;

   if( ( fp = fopen( file, "a" ) ) == NULL )
      perror( file );
   else
   {
      fprintf( fp, "%s\n", str );
      fclose( fp );
   }

   return;
}


/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
   char buf[MAX_STRING_LENGTH];
   FILE *fp;
   struct stat fst;

   if( fpArea != NULL )
   {
      int iLine;
      int iChar;

      if( fpArea == stdin )
      {
         iLine = 0;
      }
      else
      {
         iChar = ftell( fpArea );
         fseek( fpArea, 0, 0 );
         for( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
         {
            while( getc( fpArea ) != '\n' )
               ;
         }
         fseek( fpArea, iChar, 0 );
      }

      sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
      log_string( buf );

      if( stat( SHUTDOWN_FILE, &fst ) != -1 )   /* file exists */
      {
         if( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
         {
            fprintf( fp, "[*****] %s\n", buf );
            fclose( fp );
            fp = NULL;  /* redhat 5.1 fix - shogar */
         }
      }
   }

   strcpy( buf, "[*****] BUG: " );
   {
      va_list param;

      va_start( param, str );
      vsprintf( buf + strlen( buf ), str, param );
      va_end( param );
   }
   log_string( buf );

   fclose( fpLOG );
   fpLOG = NULL;  /* redhat 5.1 fix - shogar */
   if( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
   {
      fprintf( fp, "%s\n", buf );
      fclose( fp );
   }
   fpLOG = fopen( NULL_FILE, "r" );

   return;
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log( const char *str, ... )
{
   char buf[MAX_STRING_LENGTH];
   FILE *fp;
   va_list param;

   strcpy( buf, "[*****] BOOT: " );
   va_start( param, str );
   vsprintf( buf + strlen( buf ), str, param );
   va_end( param );
   log_string( buf );

   fclose( fpLOG );
   fpLOG = NULL;  /* redhat 5.1 fix - shogar */
   if( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
   {
      fprintf( fp, "%s\n", buf );
      fclose( fp );
   }
   fpLOG = fopen( NULL_FILE, "r" );

   return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA * ch, char *filename )
{
   FILE *fp;
   char buf[MAX_STRING_LENGTH];
   int c;
   int num = 0;

   fclose( fpReserve );
   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      while( !feof( fp ) )
      {
         while( ( buf[num] = fgetc( fp ) ) != EOF
                && buf[num] != '\n' && buf[num] != '\r' && num < ( MAX_STRING_LENGTH - 2 ) )
            num++;
         c = fgetc( fp );
         if( ( c != '\n' && c != '\r' ) || c == buf[num] )
            ungetc( c, fp );
         buf[num++] = '\n';
         buf[num++] = '\r';
         buf[num] = '\0';
         send_to_pager_color( buf, ch );
         num = 0;
      }
      /*
       * Thanks to stu <sprice@@ihug.co.nz> from the mailing list in pointing
       * *  This out. 
       */
      fclose( fp );
   }
   fpReserve = fopen( NULL_FILE, "r" );
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg( CHAR_DATA * ch, char *argument )
{
   set_pager_color( AT_LOG, ch );
   show_file( ch, BOOTLOG_FILE );
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, sh_int log_type, sh_int level )
{
   char *strtime;
   int offset;

   strtime = ctime( &current_time );
   strtime[strlen( strtime ) - 1] = '\0';
   fprintf( stderr, "%s :: %s\n", strtime, str );
   if( strncmp( str, "Log ", 4 ) == 0 )
      offset = 4;
   else
      offset = 0;
   switch ( log_type )
   {
      default:
         to_channel( str + offset, "Log", PERMIT_HIBUILD );
         break;
      case LOG_BUILD:
         to_channel( str + offset, "Build", PERMIT_BUILD );
         break;
      case LOG_COMM:
         to_channel( str + offset, "Comm", PERMIT_CHANNEL );
         break;
      case LOG_WARN:
         to_channel( str + offset, "Warn", PERMIT_SECURITY );
         break;
      case LOG_ALL:
         break;
   }
   return;
}

/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type( char *name )
{
   if( !str_cmp( name, "in_file_prog" ) )
      return IN_FILE_PROG;
   if( !str_cmp( name, "act_prog" ) )
      return ACT_PROG;
   if( !str_cmp( name, "speech_prog" ) )
      return SPEECH_PROG;
   if( !str_cmp( name, "rand_prog" ) )
      return RAND_PROG;
   if( !str_cmp( name, "fight_prog" ) )
      return FIGHT_PROG;
   if( !str_cmp( name, "hitprcnt_prog" ) )
      return HITPRCNT_PROG;
   if( !str_cmp( name, "death_prog" ) )
      return DEATH_PROG;
   if( !str_cmp( name, "entry_prog" ) )
      return ENTRY_PROG;
   if( !str_cmp( name, "greet_prog" ) )
      return GREET_PROG;
   if( !str_cmp( name, "all_greet_prog" ) )
      return ALL_GREET_PROG;
   if( !str_cmp( name, "give_prog" ) )
      return GIVE_PROG;
   if( !str_cmp( name, "bribe_prog" ) )
      return BRIBE_PROG;
   if( !str_cmp( name, "time_prog" ) )
      return TIME_PROG;
   if( !str_cmp( name, "hour_prog" ) )
      return HOUR_PROG;
   if( !str_cmp( name, "wear_prog" ) )
      return WEAR_PROG;
   if( !str_cmp( name, "remove_prog" ) )
      return REMOVE_PROG;
   if( !str_cmp( name, "sac_prog" ) )
      return SAC_PROG;
   if( !str_cmp( name, "look_prog" ) )
      return LOOK_PROG;
   if( !str_cmp( name, "exa_prog" ) )
      return EXA_PROG;
   if( !str_cmp( name, "zap_prog" ) )
      return ZAP_PROG;
   if( !str_cmp( name, "get_prog" ) )
      return GET_PROG;
   if( !str_cmp( name, "drop_prog" ) )
      return DROP_PROG;
   if( !str_cmp( name, "damage_prog" ) )
      return DAMAGE_PROG;
   if( !str_cmp( name, "greet_prog" ) )
      return GREET_PROG;
   if( !str_cmp( name, "randiw_prog" ) )
      return RANDIW_PROG;
   if( !str_cmp( name, "speechiw_prog" ) )
      return SPEECHIW_PROG;
   if( !str_cmp( name, "pull_prog" ) )
      return PULL_PROG;
   if( !str_cmp( name, "push_prog" ) )
      return PUSH_PROG;
   if( !str_cmp( name, "sleep_prog" ) )
      return SLEEP_PROG;
   if( !str_cmp( name, "rest_prog" ) )
      return REST_PROG;
   if( !str_cmp( name, "rfight_prog" ) )
      return FIGHT_PROG;
   if( !str_cmp( name, "enter_prog" ) )
      return ENTRY_PROG;
   if( !str_cmp( name, "leave_prog" ) )
      return LEAVE_PROG;
   if( !str_cmp( name, "rdeath_prog" ) )
      return DEATH_PROG;
   if( !str_cmp( name, "script_prog" ) )
      return SCRIPT_PROG;
   if( !str_cmp( name, "use_prog" ) )
      return USE_PROG;
   return ( ERROR_PROG );
}

MPROG_DATA *mprog_file_read( char *f, MPROG_DATA * mprg, MOB_INDEX_DATA * pMobIndex )
{

   char MUDProgfile[MAX_INPUT_LENGTH];
   FILE *progfile;
   char letter;
   MPROG_DATA *mprg_next, *mprg2;
   bool done = FALSE;

   sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

   progfile = fopen( MUDProgfile, "r" );
   if( !progfile )
   {
      bug( "Mob: %d couldn't open mudprog file", pMobIndex->vnum );
      exit( 1 );
   }

   mprg2 = mprg;
   switch ( letter = fread_letter( progfile ) )
   {
      case '>':
         break;
      case '|':
         bug( "empty mudprog file." );
         exit( 1 );
         break;
      default:
         bug( "in mudprog file syntax error." );
         exit( 1 );
         break;
   }

   while( !done )
   {
      mprg2->type = mprog_name_to_type( fread_word( progfile ) );
      switch ( mprg2->type )
      {
         case ERROR_PROG:
            bug( "mudprog file type error" );
            exit( 1 );
            break;
         case IN_FILE_PROG:
            bug( "mprog file contains a call to file." );
            exit( 1 );
            break;
         default:
            xSET_BIT( pMobIndex->progtypes, mprg2->type );
            mprg2->arglist = fread_string( progfile );
            mprg2->comlist = fread_string( progfile );
            switch ( letter = fread_letter( progfile ) )
            {
               case '>':
                  CREATE( mprg_next, MPROG_DATA, 1 );
                  mprg_next->next = mprg2;
                  mprg2 = mprg_next;
                  break;
               case '|':
                  done = TRUE;
                  break;
               default:
                  bug( "in mudprog file syntax error." );
                  exit( 1 );
                  break;
            }
            break;
      }
   }
   fclose( progfile );
   return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_mudprogs( AREA_DATA * tarea, FILE * fp )
{
   MOB_INDEX_DATA *iMob;
   MPROG_DATA *original;
   MPROG_DATA *working;
   char letter;
   int value;

   for( ;; )
      switch ( letter = fread_letter( fp ) )
      {
         default:
            bug( "Load_mudprogs: bad command '%c'.", letter );
            exit( 1 );
            break;
         case 'S':
         case 's':
            fread_to_eol( fp );
            return;
         case '*':
            fread_to_eol( fp );
            break;
         case 'M':
         case 'm':
            value = fread_number( fp );
            if( ( iMob = get_mob_index( value ) ) == NULL )
            {
               bug( "Load_mudprogs: vnum %d doesnt exist", value );
               exit( 1 );
            }

            /*
             * Go to the end of the prog command list if other commands
             * exist 
             */

            if( ( original = iMob->mudprogs ) != NULL )
               for( ; original->next; original = original->next );

            CREATE( working, MPROG_DATA, 1 );
            if( original )
               original->next = working;
            else
               iMob->mudprogs = working;
            working = mprog_file_read( fread_word( fp ), working, iMob );
            working->next = NULL;
            fread_to_eol( fp );
            break;
      }

   return;

}

/* This procedure is responsible for reading any in_file MUDprograms.
 */

void mprog_read_programs( FILE * fp, MOB_INDEX_DATA * pMobIndex )
{
   MPROG_DATA *mprg;
   char letter;
   bool done = FALSE;

   if( ( letter = fread_letter( fp ) ) != '>' )
   {
      bug( "Load_mobiles: vnum %d MUDPROG char", pMobIndex->vnum );
      exit( 1 );
   }
   CREATE( mprg, MPROG_DATA, 1 );
   pMobIndex->mudprogs = mprg;

   while( !done )
   {
      mprg->type = mprog_name_to_type( fread_word( fp ) );
      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "Load_mobiles: vnum %d MUDPROG type.", pMobIndex->vnum );
            exit( 1 );
            break;
         case IN_FILE_PROG:
            mprg = mprog_file_read( fread_string( fp ), mprg, pMobIndex );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
               case '>':
                  CREATE( mprg->next, MPROG_DATA, 1 );
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol( fp );
                  done = TRUE;
                  break;
               default:
                  bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
                  exit( 1 );
                  break;
            }
            break;
         default:
            xSET_BIT( pMobIndex->progtypes, mprg->type );
            mprg->arglist = fread_string( fp );
            fread_to_eol( fp );
            mprg->comlist = fread_string( fp );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
               case '>':
                  CREATE( mprg->next, MPROG_DATA, 1 );
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol( fp );
                  done = TRUE;
                  break;
               default:
                  bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
                  exit( 1 );
                  break;
            }
            break;
      }
   }

   return;

}



/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA *oprog_file_read( char *f, MPROG_DATA * mprg, OBJ_INDEX_DATA * pObjIndex )
{

   char MUDProgfile[MAX_INPUT_LENGTH];
   FILE *progfile;
   char letter;
   MPROG_DATA *mprg_next, *mprg2;
   bool done = FALSE;

   sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

   progfile = fopen( MUDProgfile, "r" );
   if( !progfile )
   {
      bug( "Obj: %d couldnt open mudprog file", pObjIndex->vnum );
      exit( 1 );
   }

   mprg2 = mprg;
   switch ( letter = fread_letter( progfile ) )
   {
      case '>':
         break;
      case '|':
         bug( "empty objprog file." );
         exit( 1 );
         break;
      default:
         bug( "in objprog file syntax error." );
         exit( 1 );
         break;
   }

   while( !done )
   {
      mprg2->type = mprog_name_to_type( fread_word( progfile ) );
      switch ( mprg2->type )
      {
         case ERROR_PROG:
            bug( "objprog file type error" );
            exit( 1 );
            break;
         case IN_FILE_PROG:
            bug( "objprog file contains a call to file." );
            exit( 1 );
            break;
         default:
            xSET_BIT( pObjIndex->progtypes, mprg2->type );
            mprg2->arglist = fread_string( progfile );
            mprg2->comlist = fread_string( progfile );
            switch ( letter = fread_letter( progfile ) )
            {
               case '>':
                  CREATE( mprg_next, MPROG_DATA, 1 );
                  mprg_next->next = mprg2;
                  mprg2 = mprg_next;
                  break;
               case '|':
                  done = TRUE;
                  break;
               default:
                  bug( "in objprog file syntax error." );
                  exit( 1 );
                  break;
            }
            break;
      }
   }
   fclose( progfile );
   return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_objprogs( AREA_DATA * tarea, FILE * fp )
{
   OBJ_INDEX_DATA *iObj;
   MPROG_DATA *original;
   MPROG_DATA *working;
   char letter;
   int value;

   for( ;; )
      switch ( letter = fread_letter( fp ) )
      {
         default:
            bug( "Load_objprogs: bad command '%c'.", letter );
            exit( 1 );
            break;
         case 'S':
         case 's':
            fread_to_eol( fp );
            return;
         case '*':
            fread_to_eol( fp );
            break;
         case 'M':
         case 'm':
            value = fread_number( fp );
            if( ( iObj = get_obj_index( value ) ) == NULL )
            {
               bug( "Load_objprogs: vnum %d doesnt exist", value );
               exit( 1 );
            }

            /*
             * Go to the end of the prog command list if other commands
             * exist 
             */

            if( ( original = iObj->mudprogs ) != NULL )
               for( ; original->next; original = original->next );

            CREATE( working, MPROG_DATA, 1 );
            if( original )
               original->next = working;
            else
               iObj->mudprogs = working;
            working = oprog_file_read( fread_word( fp ), working, iObj );
            working->next = NULL;
            fread_to_eol( fp );
            break;
      }

   return;

}

/* This procedure is responsible for reading any in_file OBJprograms.
 */

void oprog_read_programs( FILE * fp, OBJ_INDEX_DATA * pObjIndex )
{
   MPROG_DATA *mprg;
   char letter;
   bool done = FALSE;

   if( ( letter = fread_letter( fp ) ) != '>' )
   {
      bug( "Load_objects: vnum %d OBJPROG char", pObjIndex->vnum );
      exit( 1 );
   }
   CREATE( mprg, MPROG_DATA, 1 );
   pObjIndex->mudprogs = mprg;

   while( !done )
   {
      mprg->type = mprog_name_to_type( fread_word( fp ) );
      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "Load_objects: vnum %d OBJPROG type.", pObjIndex->vnum );
            exit( 1 );
            break;
         case IN_FILE_PROG:
            mprg = oprog_file_read( fread_string( fp ), mprg, pObjIndex );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
               case '>':
                  CREATE( mprg->next, MPROG_DATA, 1 );
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol( fp );
                  done = TRUE;
                  break;
               default:
                  bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
                  exit( 1 );
                  break;
            }
            break;
         default:
            xSET_BIT( pObjIndex->progtypes, mprg->type );
            mprg->arglist = fread_string( fp );
            fread_to_eol( fp );
            mprg->comlist = fread_string( fp );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
               case '>':
                  CREATE( mprg->next, MPROG_DATA, 1 );
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol( fp );
                  done = TRUE;
                  break;
               default:
                  bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
                  exit( 1 );
                  break;
            }
            break;
      }
   }

   return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
MPROG_DATA *rprog_file_read( char *f, MPROG_DATA * mprg, ROOM_INDEX_DATA * RoomIndex )
{

   char MUDProgfile[MAX_INPUT_LENGTH];
   FILE *progfile;
   char letter;
   MPROG_DATA *mprg_next, *mprg2;
   bool done = FALSE;

   sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

   progfile = fopen( MUDProgfile, "r" );
   if( !progfile )
   {
      bug( "Room: %d couldnt open roomprog file", RoomIndex->vnum );
      exit( 1 );
   }

   mprg2 = mprg;
   switch ( letter = fread_letter( progfile ) )
   {
      case '>':
         break;
      case '|':
         bug( "empty roomprog file." );
         exit( 1 );
         break;
      default:
         bug( "in roomprog file syntax error." );
         exit( 1 );
         break;
   }

   while( !done )
   {
      mprg2->type = mprog_name_to_type( fread_word( progfile ) );
      switch ( mprg2->type )
      {
         case ERROR_PROG:
            bug( "roomprog file type error" );
            exit( 1 );
            break;
         case IN_FILE_PROG:
            bug( "roomprog file contains a call to file." );
            exit( 1 );
            break;
         default:
            xSET_BIT( RoomIndex->progtypes, mprg2->type );
            mprg2->arglist = fread_string( progfile );
            mprg2->comlist = fread_string( progfile );
            switch ( letter = fread_letter( progfile ) )
            {
               case '>':
                  CREATE( mprg_next, MPROG_DATA, 1 );
                  mprg_next->next = mprg2;
                  mprg2 = mprg_next;
                  break;
               case '|':
                  done = TRUE;
                  break;
               default:
                  bug( "in roomprog file syntax error." );
                  exit( 1 );
                  break;
            }
            break;
      }
   }
   fclose( progfile );
   return mprg2;
}

/* Load a ROOMprogram section from the area file.
 */
void load_roomprogs( AREA_DATA * tarea, FILE * fp )
{
   ROOM_INDEX_DATA *iRoom;
   MPROG_DATA *original;
   MPROG_DATA *working;
   char letter;
   int value;

   for( ;; )
      switch ( letter = fread_letter( fp ) )
      {
         default:
            bug( "Load_objprogs: bad command '%c'.", letter );
            exit( 1 );
            break;
         case 'S':
         case 's':
            fread_to_eol( fp );
            return;
         case '*':
            fread_to_eol( fp );
            break;
         case 'M':
         case 'm':
            value = fread_number( fp );
            if( ( iRoom = get_room_index( value ) ) == NULL )
            {
               bug( "Load_roomprogs: vnum %d doesnt exist", value );
               exit( 1 );
            }

            /*
             * Go to the end of the prog command list if other commands
             * exist 
             */

            if( ( original = iRoom->mudprogs ) != NULL )
               for( ; original->next; original = original->next );

            CREATE( working, MPROG_DATA, 1 );
            if( original )
               original->next = working;
            else
               iRoom->mudprogs = working;
            working = rprog_file_read( fread_word( fp ), working, iRoom );
            working->next = NULL;
            fread_to_eol( fp );
            break;
      }

   return;

}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

void rprog_read_programs( FILE * fp, ROOM_INDEX_DATA * pRoomIndex )
{
   MPROG_DATA *mprg;
   char letter;
   bool done = FALSE;

   if( ( letter = fread_letter( fp ) ) != '>' )
   {
      bug( "Load_rooms: vnum %d ROOMPROG char", pRoomIndex->vnum );
      exit( 1 );
   }
   CREATE( mprg, MPROG_DATA, 1 );
   pRoomIndex->mudprogs = mprg;

   while( !done )
   {
      mprg->type = mprog_name_to_type( fread_word( fp ) );
      switch ( mprg->type )
      {
         case ERROR_PROG:
            bug( "Load_rooms: vnum %d ROOMPROG type.", pRoomIndex->vnum );
            exit( 1 );
            break;
         case IN_FILE_PROG:
            mprg = rprog_file_read( fread_string( fp ), mprg, pRoomIndex );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
               case '>':
                  CREATE( mprg->next, MPROG_DATA, 1 );
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol( fp );
                  done = TRUE;
                  break;
               default:
                  bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
                  exit( 1 );
                  break;
            }
            break;
         default:
            xSET_BIT( pRoomIndex->progtypes, mprg->type );
            mprg->arglist = fread_string( fp );
            fread_to_eol( fp );
            mprg->comlist = fread_string( fp );
            fread_to_eol( fp );
            switch ( letter = fread_letter( fp ) )
            {
               case '>':
                  CREATE( mprg->next, MPROG_DATA, 1 );
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol( fp );
                  done = TRUE;
                  break;
               default:
                  bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
                  exit( 1 );
                  break;
            }
            break;
      }
   }

   return;

}


/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
   Narn, May/96
   Don't ask me why they return bool.. :).. oh well.. -- Alty
*/
bool delete_room( ROOM_INDEX_DATA * room )
{
   int hash;
   ROOM_INDEX_DATA *prev, *limbo = get_room_index( ROOM_VNUM_LIMBO );
   OBJ_DATA *o;
   CHAR_DATA *ch;
   EXTRA_DESCR_DATA *ed;
   EXIT_DATA *ex;
   MPROG_ACT_LIST *mpact;
   MPROG_DATA *mp;
   TRAIL_DATA *trail;

   while( ( ch = room->first_person ) != NULL )
   {
      if( !IS_NPC( ch ) )
      {
         char_from_room( ch );
         char_to_room( ch, limbo );
      }
      else
         extract_char( ch, TRUE );
   }
   while( ( o = room->first_content ) != NULL )
      extract_obj( o );
   wipe_resets( room );
   while( ( ed = room->first_extradesc ) != NULL )
   {
      room->first_extradesc = ed->next;
      STRFREE( ed->keyword );
      STRFREE( ed->description );
      DISPOSE( ed );
      --top_ed;
   }
   while( ( trail = room->first_trail ) != NULL )
   {
      room->first_trail = trail->next;
      DISPOSE( trail->next );
      DISPOSE( trail->prev );
      DISPOSE( trail );
   }
   while( ( ex = room->first_exit ) != NULL )
      extract_exit( room, ex );
   while( ( mpact = room->mpact ) != NULL )
   {
      room->mpact = mpact->next;
      DISPOSE( mpact->buf );
      DISPOSE( mpact );
   }
   while( ( mp = room->mudprogs ) != NULL )
   {
      room->mudprogs = mp->next;
      STRFREE( mp->arglist );
      STRFREE( mp->comlist );
      DISPOSE( mp );
   }
/*  if (room->map)
  {
    MAP_INDEX_DATA *mapi;

    if ((mapi = get_map_index(room->map->vnum)) != NULL)
      if (room->map->x > 0 && room->map->x < 80 &&
          room->map->y > 0 && room->map->y < 48)
        mapi->map_of_vnums[room->map->y][room->map->x] = -1;
    DISPOSE(room->map);
  }*/
   STRFREE( room->name );
   STRFREE( room->description );

   hash = room->vnum % MAX_KEY_HASH;
   if( room == room_index_hash[hash] )
      room_index_hash[hash] = room->next;
   else
   {
      for( prev = room_index_hash[hash]; prev; prev = prev->next )
         if( prev->next == room )
            break;
      if( prev )
         prev->next = room->next;
      else
         bug( "delete_room: room %d not in hash bucket %d.", room->vnum, hash );
   }
   DISPOSE( room );
   --top_room;
   return TRUE;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA * obj )
{
   int hash;
   OBJ_INDEX_DATA *prev;
   OBJ_DATA *o, *o_next;
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *af;
   MPROG_DATA *mp;

   /*
    * Remove references to object index 
    */
   for( o = first_object; o; o = o_next )
   {
      o_next = o->next;
      if( o->pIndexData == obj )
         extract_obj( o );
   }
   while( ( ed = obj->first_extradesc ) != NULL )
   {
      obj->first_extradesc = ed->next;
      STRFREE( ed->keyword );
      STRFREE( ed->description );
      DISPOSE( ed );
      --top_ed;
   }
   while( ( af = obj->first_affect ) != NULL )
   {
      obj->first_affect = af->next;
      DISPOSE( af );
      --top_affect;
   }
   while( ( mp = obj->mudprogs ) != NULL )
   {
      obj->mudprogs = mp->next;
      STRFREE( mp->arglist );
      STRFREE( mp->comlist );
      DISPOSE( mp );
   }
   STRFREE( obj->name );
   STRFREE( obj->short_descr );
   STRFREE( obj->description );
   STRFREE( obj->action_desc );

   hash = obj->vnum % MAX_KEY_HASH;
   if( obj == obj_index_hash[hash] )
      obj_index_hash[hash] = obj->next;
   else
   {
      for( prev = obj_index_hash[hash]; prev; prev = prev->next )
         if( prev->next == obj )
            break;
      if( prev )
         prev->next = obj->next;
      else
         bug( "delete_obj: object %d not in hash bucket %d.", obj->vnum, hash );
   }
   DISPOSE( obj );
   --top_obj_index;
   return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA * mob )
{
   int hash;
   MOB_INDEX_DATA *prev;
   CHAR_DATA *ch, *ch_next;
   MPROG_DATA *mp;

   for( ch = first_char; ch; ch = ch_next )
   {
      ch_next = ch->next;
      if( ch->pIndexData == mob )
         extract_char( ch, TRUE );
   }
   while( ( mp = mob->mudprogs ) != NULL )
   {
      mob->mudprogs = mp->next;
      STRFREE( mp->arglist );
      STRFREE( mp->comlist );
      DISPOSE( mp );
   }

   STRFREE( mob->player_name );
   STRFREE( mob->short_descr );
   STRFREE( mob->description );

   hash = mob->vnum % MAX_KEY_HASH;
   if( mob == mob_index_hash[hash] )
      mob_index_hash[hash] = mob->next;
   else
   {
      for( prev = mob_index_hash[hash]; prev; prev = prev->next )
         if( prev->next == mob )
            break;
      if( prev )
         prev->next = mob->next;
      else
         bug( "delete_mob: mobile %d not in hash bucket %d.", mob->vnum, hash );
   }
   DISPOSE( mob );
   --top_mob_index;
   return TRUE;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( int vnum )
{
   ROOM_INDEX_DATA *pRoomIndex;
   int iHash;

   CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
   pRoomIndex->first_person = NULL;
   pRoomIndex->last_person = NULL;
   pRoomIndex->first_content = NULL;
   pRoomIndex->last_content = NULL;
   pRoomIndex->first_reset = pRoomIndex->last_reset = NULL;
   pRoomIndex->first_trail = NULL;
   pRoomIndex->last_trail = NULL;
   pRoomIndex->first_extradesc = NULL;
   pRoomIndex->last_extradesc = NULL;
   pRoomIndex->area = NULL;
   pRoomIndex->vnum = vnum;
   pRoomIndex->name = STRALLOC( "a void" );
   pRoomIndex->description = STRALLOC( "" );
   pRoomIndex->room_flags = ROOM_PROTOTYPE;
   pRoomIndex->sector_type = 1;
   pRoomIndex->first_exit = NULL;
   pRoomIndex->last_exit = NULL;

   iHash = vnum % MAX_KEY_HASH;
   pRoomIndex->next = room_index_hash[iHash];
   room_index_hash[iHash] = pRoomIndex;
   top_room++;

   return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, char *name )
{
   OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
   char buf[MAX_STRING_LENGTH];
   int iHash;

   if( cvnum > 0 )
      cObjIndex = get_obj_index( cvnum );
   else
      cObjIndex = NULL;
   CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
   pObjIndex->vnum = vnum;
   pObjIndex->name = STRALLOC( name );
   pObjIndex->first_affect = NULL;
   pObjIndex->last_affect = NULL;
   pObjIndex->first_extradesc = NULL;
   pObjIndex->last_extradesc = NULL;
   if( !cObjIndex )
   {
      sprintf( buf, "A newly created %s", name );
      pObjIndex->short_descr = STRALLOC( buf );
      sprintf( buf, "Some god dropped a newly created %s here.", name );
      pObjIndex->description = STRALLOC( buf );
      pObjIndex->action_desc = STRALLOC( "" );
      pObjIndex->short_descr[0] = LOWER( pObjIndex->short_descr[0] );
      pObjIndex->description[0] = UPPER( pObjIndex->description[0] );
      pObjIndex->item_type = ITEM_TRASH;
      xCLEAR_BITS( pObjIndex->extra_flags );
      xCLEAR_BITS( pObjIndex->parts );
      xSET_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
      pObjIndex->tech = 0;
      pObjIndex->value[0] = 0;
      pObjIndex->value[1] = 0;
      pObjIndex->value[2] = 0;
      pObjIndex->value[3] = 0;
      pObjIndex->value[4] = 0;
      pObjIndex->value[5] = 0;
      pObjIndex->value[6] = 0;
      pObjIndex->weight = 1;
      pObjIndex->cost = 0;
   }
   else
   {
      EXTRA_DESCR_DATA *ed, *ced;
      AFFECT_DATA *paf, *cpaf;

      pObjIndex->short_descr = QUICKLINK( cObjIndex->short_descr );
      pObjIndex->description = QUICKLINK( cObjIndex->description );
      pObjIndex->action_desc = QUICKLINK( cObjIndex->action_desc );
      pObjIndex->item_type = cObjIndex->item_type;
      pObjIndex->extra_flags = cObjIndex->extra_flags;
      xSET_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
      pObjIndex->parts = cObjIndex->parts;
      pObjIndex->tech = cObjIndex->tech;
      pObjIndex->value[0] = cObjIndex->value[0];
      pObjIndex->value[1] = cObjIndex->value[1];
      pObjIndex->value[2] = cObjIndex->value[2];
      pObjIndex->value[3] = cObjIndex->value[3];
      pObjIndex->value[4] = cObjIndex->value[4];
      pObjIndex->value[5] = cObjIndex->value[5];
      pObjIndex->value[6] = cObjIndex->value[6];
      pObjIndex->weight = cObjIndex->weight;
      pObjIndex->cost = cObjIndex->cost;
      for( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
      {
         CREATE( ed, EXTRA_DESCR_DATA, 1 );
         ed->keyword = QUICKLINK( ced->keyword );
         ed->description = QUICKLINK( ced->description );
         LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
         top_ed++;
      }
      for( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = cpaf->type;
         paf->duration = cpaf->duration;
         paf->location = cpaf->location;
         paf->modifier = cpaf->modifier;
         paf->bitvector = cpaf->bitvector;
         LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
         top_affect++;
      }
   }
   pObjIndex->count = 0;
   iHash = vnum % MAX_KEY_HASH;
   pObjIndex->next = obj_index_hash[iHash];
   obj_index_hash[iHash] = pObjIndex;
   top_obj_index++;

   return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( int vnum, int cvnum, char *name )
{
   MOB_INDEX_DATA *pMobIndex, *cMobIndex;
   char buf[MAX_STRING_LENGTH];
   int iHash;

   if( cvnum > 0 )
      cMobIndex = get_mob_index( cvnum );
   else
      cMobIndex = NULL;
   CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
   pMobIndex->vnum = vnum;
   pMobIndex->count = 0;
   pMobIndex->killed = 0;
   pMobIndex->player_name = STRALLOC( name );
   if( !cMobIndex )
   {
      sprintf( buf, "A newly created %s", name );
      pMobIndex->short_descr = STRALLOC( buf );
      sprintf( buf, "Some god abandoned a newly created %s here.\n\r", name );
      pMobIndex->description = STRALLOC( "" );
      pMobIndex->short_descr[0] = LOWER( pMobIndex->short_descr[0] );
      pMobIndex->description[0] = UPPER( pMobIndex->description[0] );
      xCLEAR_BITS( pMobIndex->act );
      xSET_BIT( pMobIndex->act, ACT_IS_NPC );
      xSET_BIT( pMobIndex->act, ACT_PROTOTYPE );
      xCLEAR_BITS( pMobIndex->affected_by );
      pMobIndex->spec_fun = NULL;
      pMobIndex->mudprogs = NULL;
      xCLEAR_BITS( pMobIndex->progtypes );
      pMobIndex->mobthac0 = 0;
      pMobIndex->ac = 0;
      pMobIndex->hitnodice = 0;
      pMobIndex->hitsizedice = 0;
      pMobIndex->hitplus = 0;
      pMobIndex->damnodice = 0;
      pMobIndex->damsizedice = 0;
      pMobIndex->damplus = 0;
      pMobIndex->gold = 0;
      pMobIndex->exp = 0;
      pMobIndex->position = POS_STANDING;
      pMobIndex->sex = 0;
      pMobIndex->perm_str = 13;
      pMobIndex->perm_dex = 13;
      pMobIndex->perm_int = 13;
      pMobIndex->perm_wil = 13;
      pMobIndex->perm_per = 13;
      pMobIndex->perm_con = 13;
      pMobIndex->perm_lck = 13;
      pMobIndex->skinamount = 0;
      pMobIndex->race = 0;
      pMobIndex->xflags = 0;
      pMobIndex->resistant = 0;
      pMobIndex->immune = 0;
      pMobIndex->susceptible = 0;
      pMobIndex->numattacks = 0;
      xCLEAR_BITS( pMobIndex->attacks );
      xCLEAR_BITS( pMobIndex->defenses );
   }
   else
   {
      pMobIndex->short_descr = QUICKLINK( cMobIndex->short_descr );
      pMobIndex->description = QUICKLINK( cMobIndex->description );
      pMobIndex->act = cMobIndex->act;
      xSET_BIT( pMobIndex->act, ACT_PROTOTYPE );
      pMobIndex->affected_by = cMobIndex->affected_by;
      pMobIndex->spec_fun = cMobIndex->spec_fun;
      pMobIndex->mudprogs = NULL;
      xCLEAR_BITS( pMobIndex->progtypes );
      pMobIndex->mobthac0 = cMobIndex->mobthac0;
      pMobIndex->ac = cMobIndex->ac;
      pMobIndex->hitnodice = cMobIndex->hitnodice;
      pMobIndex->hitsizedice = cMobIndex->hitsizedice;
      pMobIndex->hitplus = cMobIndex->hitplus;
      pMobIndex->damnodice = cMobIndex->damnodice;
      pMobIndex->damsizedice = cMobIndex->damsizedice;
      pMobIndex->damplus = cMobIndex->damplus;
      pMobIndex->gold = cMobIndex->gold;
      pMobIndex->exp = cMobIndex->exp;
      pMobIndex->position = cMobIndex->position;
      pMobIndex->sex = cMobIndex->sex;
      pMobIndex->perm_str = cMobIndex->perm_str;
      pMobIndex->perm_dex = cMobIndex->perm_dex;
      pMobIndex->perm_int = cMobIndex->perm_int;
      pMobIndex->perm_wil = cMobIndex->perm_wil;
      pMobIndex->perm_per = cMobIndex->perm_per;
      pMobIndex->perm_con = cMobIndex->perm_con;
      pMobIndex->perm_lck = cMobIndex->perm_lck;
      pMobIndex->skinamount = cMobIndex->skinamount;
      pMobIndex->race = cMobIndex->race;
      pMobIndex->xflags = cMobIndex->xflags;
      pMobIndex->resistant = cMobIndex->resistant;
      pMobIndex->immune = cMobIndex->immune;
      pMobIndex->susceptible = cMobIndex->susceptible;
      pMobIndex->numattacks = cMobIndex->numattacks;
      pMobIndex->attacks = cMobIndex->attacks;
      pMobIndex->defenses = cMobIndex->defenses;
   }
   iHash = vnum % MAX_KEY_HASH;
   pMobIndex->next = mob_index_hash[iHash];
   mob_index_hash[iHash] = pMobIndex;
   top_mob_index++;

   return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA * pRoomIndex, ROOM_INDEX_DATA * to_room, sh_int door )
{
   EXIT_DATA *pexit, *texit;
   bool broke;

   CREATE( pexit, EXIT_DATA, 1 );
   pexit->vdir = door;
   pexit->rvnum = pRoomIndex->vnum;
   pexit->to_room = to_room;
   pexit->distance = 1;
   if( to_room )
   {
      pexit->vnum = to_room->vnum;
      texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );
      if( texit ) /* assign reverse exit pointers */
      {
         texit->rexit = pexit;
         pexit->rexit = texit;
      }
   }
   broke = FALSE;
   for( texit = pRoomIndex->first_exit; texit; texit = texit->next )
      if( door < texit->vdir )
      {
         broke = TRUE;
         break;
      }
   if( !pRoomIndex->first_exit )
      pRoomIndex->first_exit = pexit;
   else
   {
      /*
       * keep exits in incremental order - insert exit into list 
       */
      if( broke && texit )
      {
         if( !texit->prev )
            pRoomIndex->first_exit = pexit;
         else
            texit->prev->next = pexit;
         pexit->prev = texit->prev;
         pexit->next = texit;
         texit->prev = pexit;
         top_exit++;
         return pexit;
      }
      pRoomIndex->last_exit->next = pexit;
   }
   pexit->next = NULL;
   pexit->prev = pRoomIndex->last_exit;
   pRoomIndex->last_exit = pexit;
   top_exit++;
   return pexit;
}

void fix_area_exits( AREA_DATA * tarea )
{
   ROOM_INDEX_DATA *pRoomIndex;
   EXIT_DATA *pexit, *rev_exit;
   int rnum;
   bool fexit;

   for( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
   {
      if( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
         continue;

      fexit = FALSE;
      for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
      {
         fexit = TRUE;
         pexit->rvnum = pRoomIndex->vnum;
         if( pexit->vnum <= 0 )
            pexit->to_room = NULL;
         else
            pexit->to_room = get_room_index( pexit->vnum );
      }
/*	if ( !fexit ) -- Leave the damn room flags alone -- Scion
	  SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
 */ }


   for( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
   {
      if( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
         continue;

      for( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
      {
         if( pexit->to_room && !pexit->rexit )
         {
            rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
            if( rev_exit )
            {
               pexit->rexit = rev_exit;
               rev_exit->rexit = pexit;
            }
         }
      }
   }
}

void load_area_file( AREA_DATA * tarea, char *filename )
{
/*    FILE *fpin;
    what intelligent person stopped using fpArea?????
    if fpArea isn't being used, then no filename or linenumber
    is printed when an error occurs during loading the area..
    (bug uses fpArea)
      --TRI  */

   if( fBootDb )
      tarea = last_area;
   if( !fBootDb && !tarea )
   {
      bug( "Load_area: null area!" );
      return;
   }

   if( ( fpArea = fopen( filename, "r" ) ) == NULL )
   {
      perror( filename );
      bug( "load_area: error loading file (can't open)" );
      bug( filename );
      return;
   }
   area_version = 0;
   for( ;; )
   {
      char *word;

      if( fread_letter( fpArea ) != '#' )
      {
         bug( tarea->filename );
         bug( "load_area: # not found." );
/*	    exit( 1 ); */
      }

      word = fread_word( fpArea );

      if( word[0] == '$' )
         break;
      else if( !str_cmp( word, "AREA" ) )
      {
         if( fBootDb )
         {
            load_area( fpArea );
            tarea = last_area;
         }
         else
         {
            DISPOSE( tarea->name );
            tarea->name = fread_string_nohash( fpArea );
         }
      }
      else if( !str_cmp( word, "AUTHOR" ) )
         load_author( tarea, fpArea );
      else if( !str_cmp( word, "FLAGS" ) )
         load_flags( tarea, fpArea );
      else if( !str_cmp( word, "RANGES" ) )
         load_ranges( tarea, fpArea );
      else if( !str_cmp( word, "ECONOMY" ) )
         load_economy( tarea, fpArea );
      else if( !str_cmp( word, "RESETMSG" ) )
         load_resetmsg( tarea, fpArea );
      /*
       * Rennard 
       */
      else if( !str_cmp( word, "MOBILES" ) )
         load_mobiles( tarea, fpArea );
      else if( !str_cmp( word, "MUDPROGS" ) )
         load_mudprogs( tarea, fpArea );
      else if( !str_cmp( word, "OBJECTS" ) )
         load_objects( tarea, fpArea );
      else if( !str_cmp( word, "OBJPROGS" ) )
         load_objprogs( tarea, fpArea );
      else if( !str_cmp( word, "RESETS" ) )
         load_resets( tarea, fpArea );
      else if( !str_cmp( word, "ROOMS" ) )
         load_rooms( tarea, fpArea );
      else if( !str_cmp( word, "SHOPS" ) )
         load_shops( tarea, fpArea );
      else if( !str_cmp( word, "SPECIALS" ) )
         load_specials( tarea, fpArea );
      else if( !str_cmp( word, "CLIMATE" ) )
         load_climate( tarea, fpArea );
      else if( !str_cmp( word, "NEIGHBOR" ) )
         load_neighbor( tarea, fpArea );
      else if( !str_cmp( word, "VERSION" ) )
         load_version( tarea, fpArea );
      else
      {
         bug( tarea->filename );
         bug( "load_area: bad section name." );
         if( !fBootDb )
/*	      exit( 1 );
	    else
*/
         {
            fclose( fpArea );
            return;
         }
      }
   }
   fclose( fpArea );
   /*
    * redhat 5.1 fix, appears fclose does not null the file pointer , but
    * needs it null if you reuse it. - shogar 
    */
   fpArea = NULL;

   if( tarea )
   {
      if( fBootDb )
      {
         sort_area_by_name( tarea );   /* 4/27/97 */
         sort_area( tarea, FALSE );
      }
      fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d Mobs: %5d - %d\n",
               tarea->filename,
               tarea->low_r_vnum, tarea->hi_r_vnum,
               tarea->low_o_vnum, tarea->hi_o_vnum, tarea->low_m_vnum, tarea->hi_m_vnum );
      if( !tarea->author )
         tarea->author = STRALLOC( "" );
      SET_BIT( tarea->status, AREA_LOADED );
   }
   else
      fprintf( stderr, "(%s)\n", filename );
}

void load_reserved( void )
{
   RESERVE_DATA *res;
   FILE *fp;

   if( !( fp = fopen( SYSTEM_DIR RESERVED_LIST, "r" ) ) )
      return;

   for( ;; )
   {
      if( feof( fp ) )
      {
         bug( "Load_reserved: no $ found." );
         fclose( fp );
         return;
      }
      CREATE( res, RESERVE_DATA, 1 );
      res->name = fread_string_nohash( fp );
      if( *res->name == '$' )
         break;
      sort_reserved( res );
   }
   DISPOSE( res->name );
   DISPOSE( res );
   fclose( fp );
   return;
}

void sort_reserved( RESERVE_DATA * pRes )
{
   RESERVE_DATA *res = NULL;

   if( !pRes )
   {
      bug( "Sort_reserved: NULL pRes" );
      return;
   }

   pRes->next = NULL;
   pRes->prev = NULL;

   for( res = first_reserved; res; res = res->next )
   {
      if( strcasecmp( pRes->name, res->name ) > 0 )
      {
         INSERT( pRes, res, first_reserved, next, prev );
         break;
      }
   }

   if( !res )
   {
      LINK( pRes, first_reserved, last_reserved, next, prev );
   }

   return;
}


/*
 * Sort areas by name alphanumercially
 *      - 4/27/97, Fireblade
 */
void sort_area_by_name( AREA_DATA * pArea )
{
   AREA_DATA *temp_area;

   if( !pArea )
   {
      bug( "Sort_area_by_name: NULL pArea" );
      return;
   }
   for( temp_area = first_area_name; temp_area; temp_area = temp_area->next_sort_name )
   {
      if( strcmp( pArea->name, temp_area->name ) < 0 )
      {
         INSERT( pArea, temp_area, first_area_name, next_sort_name, prev_sort_name );
         break;
      }
   }
   if( !temp_area )
   {
      LINK( pArea, first_area_name, last_area_name, next_sort_name, prev_sort_name );
   }
   return;
}

/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA * pArea, bool proto )
{
   AREA_DATA *area = NULL;
   AREA_DATA *first_sort, *last_sort;
   bool found;

   if( !pArea )
   {
      bug( "Sort_area: NULL pArea" );
      return;
   }

   if( proto )
   {
      first_sort = first_bsort;
      last_sort = last_bsort;
   }
   else
   {
      first_sort = first_asort;
      last_sort = last_asort;
   }

   found = FALSE;
   pArea->next_sort = NULL;
   pArea->prev_sort = NULL;

   if( !first_sort )
   {
      pArea->prev_sort = NULL;
      pArea->next_sort = NULL;
      first_sort = pArea;
      last_sort = pArea;
      found = TRUE;
   }
   else
      for( area = first_sort; area; area = area->next_sort )
         if( pArea->low_r_vnum < area->low_r_vnum )
         {
            if( !area->prev_sort )
               first_sort = pArea;
            else
               area->prev_sort->next_sort = pArea;
            pArea->prev_sort = area->prev_sort;
            pArea->next_sort = area;
            area->prev_sort = pArea;
            found = TRUE;
            break;
         }

   if( !found )
   {
      pArea->prev_sort = last_sort;
      pArea->next_sort = NULL;
      last_sort->next_sort = pArea;
      last_sort = pArea;
   }

   if( proto )
   {
      first_bsort = first_sort;
      last_bsort = last_sort;
   }
   else
   {
      first_asort = first_sort;
      last_asort = last_sort;
   }
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA * ch, int low, int high, bool proto, bool shownl, char *loadst, char *notloadst )
{
   AREA_DATA *pArea, *first_sort;
   int count, loaded;

   count = 0;
   loaded = 0;
   set_pager_color( AT_PLAIN, ch );
   if( proto )
      first_sort = first_bsort;
   else
      first_sort = first_asort;
   for( pArea = first_sort; pArea; pArea = pArea->next_sort )
   {
      if( IS_SET( pArea->status, AREA_DELETED ) )
         continue;
      if( pArea->low_r_vnum < low )
         continue;
      if( pArea->hi_r_vnum > high )
         break;
      if( IS_SET( pArea->status, AREA_LOADED ) )
         loaded++;
      else if( !shownl )
         continue;
      pager_printf( ch, "%-15s| Rooms: %5d - %-5d"
                    " Objs: %5d - %-5d Mobs: %5d - %-5d%s\n\r",
                    ( pArea->filename ? pArea->filename : "(invalid)" ),
                    pArea->low_r_vnum, pArea->hi_r_vnum,
                    pArea->low_o_vnum, pArea->hi_o_vnum,
                    pArea->low_m_vnum, pArea->hi_m_vnum, IS_SET( pArea->status, AREA_LOADED ) ? loadst : notloadst );
      count++;
   }
   pager_printf( ch, "Areas listed: %d  Loaded: %d\n\r", count, loaded );
   return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */
void do_vnums( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int low, high;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   low = 0;
   high = 1048575999;
   if( arg1[0] != '\0' )
   {
      low = atoi( arg1 );
      if( arg2[0] != '\0' )
         high = atoi( arg2 );
   }
   show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int low, high;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   low = 0;
   high = 1048575999;
   if( arg1[0] != '\0' )
   {
      low = atoi( arg1 );
      if( arg2[0] != '\0' )
         high = atoi( arg2 );
   }
   show_vnums( ch, low, high, FALSE, TRUE, "", " X" );
}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_newzones( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int low, high;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   low = 0;
   high = 1048575999;
   if( arg1[0] != '\0' )
   {
      low = atoi( arg1 );
      if( arg2[0] != '\0' )
         high = atoi( arg2 );
   }
   show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}

/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
   FILE *fp;
   char filename[MAX_INPUT_LENGTH];

   sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

   fclose( fpReserve );
   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_sysdata: fopen" );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#SYSTEM\n" );
      fprintf( fp, "MudName	     %s~\n", sys.mud_name );
      fprintf( fp, "Highplayers    %d\n", sys.alltimemax );
      fprintf( fp, "Highplayertime %s~\n", sys.time_of_max );
      fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING );
      fprintf( fp, "Saveflags      %d\n", sys.save_flags );
      fprintf( fp, "Savefreq       %d\n", sys.save_frequency );
      fprintf( fp, "PetSave	     %d\n", sys.save_pets );
      fprintf( fp, "IdentTries     %d\n", sys.ident_retries );
      fprintf( fp, "Crashguard	%d\n", sys.crashguard );
      fprintf( fp, "MultiLimit	%d\n", sys.morts_allowed );
      fprintf( fp, "Expbase		%d\n", sys.exp_base );
      fprintf( fp, "Quest		%d\n", sys.quest );
      fprintf( fp, "Email 		%s~\n", sys.email );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
   }
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}


void fread_sysdata( SYSTEM_DATA * sys, FILE * fp )
{
   char *word;
   bool fMatch;

   sys->time_of_max = NULL;
   sys->mud_name = NULL;
   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'C':
            KEY( "Crashguard", sys->crashguard, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !sys->time_of_max )
                  sys->time_of_max = str_dup( "(not recorded)" );
               if( !sys->mud_name )
                  sys->mud_name = str_dup( "(Name Not Set)" );
               return;
            }
            KEY( "Email", sys->email, fread_string( fp ) );
            KEY( "Expbase", sys->exp_base, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Highplayers", sys->alltimemax, fread_number( fp ) );
            KEY( "Highplayertime", sys->time_of_max, fread_string_nohash( fp ) );
            break;

         case 'I':
            KEY( "IdentTries", sys->ident_retries, fread_number( fp ) );
            break;

         case 'M':
            KEY( "MudName", sys->mud_name, fread_string_nohash( fp ) );
            KEY( "MultiLimit", sys->morts_allowed, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Nameresolving", sys->NO_NAME_RESOLVING, fread_number( fp ) );
            break;

         case 'P':
            KEY( "PetSave", sys->save_pets, fread_number( fp ) );
            break;

         case 'Q':
            KEY( "Quest", sys->quest, fread_number( fp ) );
            break;

         case 'S':
            KEY( "Saveflags", sys->save_flags, fread_number( fp ) );
            KEY( "Savefreq", sys->save_frequency, fread_number( fp ) );
            break;

      }


      if( !fMatch )
      {
         bug( "Fread_sysdata: no match: %s", word );
      }
   }
}

void load_matshops( void )
{
   FILE *fp;
   MAT_SHOP *matshop;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( MATSHOP_FILE, "r" ) ) == NULL )
   {
      bug( "Load_matshops: no matshops found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#MATSHOP", word ) )
            {
               MAT_SHOP *new_matshop;

               CREATE( new_matshop, MAT_SHOP, 1 );

               if( !first_matshop )
               {
                  first_matshop = new_matshop;
                  last_matshop = new_matshop;
               }
               else
               {
                  last_matshop->next = new_matshop;
                  new_matshop->prev = last_matshop;
                  last_matshop = new_matshop;
               }
               matshop = new_matshop;
               matshop->room = 0;
               matshop->mat[1] = 0;
               matshop->mat[2] = 0;
               matshop->mat[3] = 0;
               matshop->mat[4] = 0;
               matshop->mat[0] = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'M':
            KEY( "Mat1", matshop->mat[1], fread_number( fp ) );
            KEY( "Mat2", matshop->mat[2], fread_number( fp ) );
            KEY( "Mat3", matshop->mat[3], fread_number( fp ) );
            KEY( "Mat4", matshop->mat[4], fread_number( fp ) );
            KEY( "Mat5", matshop->mat[0], fread_number( fp ) );
            break;
         case 'R':
            KEY( "Room", matshop->room, fread_number( fp ) );
            break;
         default:
            bug( "Unknown matshop keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void load_materials( void )
{
   FILE *fp;
   MATERIAL_DATA *material;
// AFFECT_DATA *paf;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( MATERIAL_FILE, "r" ) ) == NULL )
   {
      bug( "Load_materials: no materials found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#MATERIAL", word ) )
            {
               {
                  MATERIAL_DATA *new_material;

                  CREATE( new_material, MATERIAL_DATA, 1 );

                  if( !first_material )
                  {  /* Ok, get this new material added into the list */
                     first_material = new_material;
                     last_material = new_material;
                     new_material->number = 1;
                  }
                  else if( first_material->number != 1 )
                  {
                     first_material->prev = material;
                     material->next = first_material;
                     material->prev = NULL;
                     material->number = 1;
                     first_material = material;
                  }
                  else
                  {
                     material = first_material;
                     while( material->next )
                     {
                        if( material->number != material->next->number - 1 )
                        {  /* and keep the numbers sequential */
                           material->next->prev = new_material;
                           new_material->next = material->next;
                           new_material->prev = material;
                           material->next = new_material;
                           new_material->number = new_material->prev->number + 1;
                           break;
                        }
                        material = material->next;
                     }
                     if( !new_material->prev )
                     {
                        last_material->next = new_material;
                        new_material->prev = last_material;
                        last_material = new_material;
                        new_material->number = new_material->prev->number + 1;
                     }
                  }
                  material = new_material;
                  material->name = str_dup( "New Material" );
                  material->short_descr = str_dup( "a piece of material" );
                  material->description = str_dup( "A nondescript piece of material lies on the floor here." );
                  material->weight = 1;
                  material->rarity = 0;
                  material->sector = 5;
                  material->magic = 100;
                  material->race = -1;
                  material->skin = -1;
                  xCLEAR_BITS( material->extra_flags );
                  material->first_affect = NULL;
               }
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
            break;
         case 'A':
            if( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;
               int pafmod;

               CREATE( paf, AFFECT_DATA, 1 );
               if( !strcmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
/*				else
				{
				    int sn;

				    sn = skill_lookup( fread_word( fp ) );
				    if ( sn < 0 )
						bug( "Load_materials: unknown skill.", 0 );
				    else
						paf->type = sn;
				} */
               paf->duration = fread_number( fp );
               pafmod = fread_number( fp );
               paf->location = fread_number( fp );
               paf->bitvector = fread_bitvector( fp );
               if( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_STRIPSN
                   || paf->location == APPLY_REMOVESPELL || paf->location == APPLY_RECURRINGSPELL )
                  paf->modifier = slot_lookup( pafmod );
               else
                  paf->modifier = pafmod;
               LINK( paf, material->first_affect, material->last_affect, next, prev );
               fMatch = TRUE;
            }
            break;
         case 'C':
            KEY( "Cost", material->cost, fread_number( fp ) );
            break;
         case 'E':
            KEY( "Extra", material->extra_flags, fread_bitvector( fp ) );
            break;
         case 'L':
            KEY( "Long", material->description, fread_string( fp ) );
            break;
         case 'M':
            KEY( "Magic", material->magic, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", material->name, fread_string( fp ) );
            break;
         case 'R':
            KEY( "Race", material->race, fread_number( fp ) );
            KEY( "Rarity", material->rarity, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Sector", material->sector, fread_number( fp ) );
            KEY( "Short", material->short_descr, fread_string( fp ) );
            KEY( "Skin", material->skin, fread_number( fp ) );
            break;
         case 'W':
            KEY( "Weight", material->weight, fread_number( fp ) );
            break;
         default:
            bug( "Unknown material keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void load_ingreds( void )
{
   FILE *fp;
   INGRED_DATA *ingred;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( INGRED_FILE, "r" ) ) == NULL )
   {
      bug( "Load_ingreds: no ingredients found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#INGRED", word ) )
            {
               INGRED_DATA *new_ingred;

               CREATE( new_ingred, INGRED_DATA, 1 );

               if( !first_ingred )
               {
                  first_ingred = new_ingred;
                  last_ingred = new_ingred;
               }
               else
               {
                  last_ingred->next = new_ingred;
                  new_ingred->prev = last_ingred;
                  last_ingred = new_ingred;
               }
               ingred = new_ingred;
               ingred->sector = 0;
               ingred->mana = 0;
               ingred->moisture = 0;
               ingred->elevation = 0;
               ingred->precip = 0;
               ingred->temp = 0;
               ingred->rarity = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'E':
            KEY( "Elevation", ingred->elevation, fread_number( fp ) );
            break;
         case 'M':
            KEY( "Mana", ingred->mana, fread_number( fp ) );
            KEY( "Moisture", ingred->moisture, fread_number( fp ) );
            break;
         case 'P':
            KEY( "Precip", ingred->precip, fread_number( fp ) );
            break;
         case 'R':
            KEY( "Rarity", ingred->rarity, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Sector", ingred->sector, fread_number( fp ) );
            break;
         case 'T':
            KEY( "Temp", ingred->temp, fread_number( fp ) );
            break;
         case 'V':
            KEY( "Vnum", ingred->vnum, fread_number( fp ) );
            break;
         default:
            bug( "Unknown ingredient keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void load_mutations( void )
{
   FILE *fp;
   MUT_DATA *mut;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( MUT_FILE, "r" ) ) == NULL )
   {
      bug( "Load_mutations: no mutations found!", 0 );
      return;
   }
   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#MUTATION", word ) )
            {
               MUT_DATA *new_mut;

               CREATE( new_mut, MUT_DATA, 1 );

               if( !first_mutation )
               {
                  first_mutation = new_mut;
                  last_mutation = new_mut;
               }
               else
               {
                  last_mutation->next = new_mut;
                  new_mut->prev = last_mutation;
                  last_mutation = new_mut;
               }
               mut = new_mut;
               mut->number = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'D':
            KEY( "Desc", mut->desc, fread_string( fp ) );
            break;
         case 'G':
            KEY( "GainOther", mut->gain_other, fread_string( fp ) );
            KEY( "GainSelf", mut->gain_self, fread_string( fp ) );
            break;
         case 'L':
            KEY( "LoseOther", mut->lose_other, fread_string( fp ) );
            KEY( "LoseSelf", mut->lose_self, fread_string( fp ) );
         case 'N':
            KEY( "Number", mut->number, fread_number( fp ) );
            break;
         default:
            bug( "Unknown mutation keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void load_channels( void )
{
   FILE *fp;
   CHANNEL_DATA *chan;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( CHANNEL_FILE, "r" ) ) == NULL )
   {
      bug( "Load_channels: no channels found!", 0 );
      return;
   }
   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#CHANNEL", word ) )
            {
               CHANNEL_DATA *new_chan;

               CREATE( new_chan, CHANNEL_DATA, 1 );

               if( !first_channel )
               {
                  first_channel = new_chan;
                  last_channel = new_chan;
               }
               else
               {
                  last_channel->next = new_chan;
                  new_chan->prev = last_channel;
                  last_channel = new_chan;
               }
               chan = new_chan;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'A':
            KEY( "Access", chan->access, fread_string( fp ) );
            break;
         case 'B':
            KEY( "Ban", chan->ban, fread_string( fp ) );
            break;
         case 'C':
            KEY( "Clan", chan->clan, fread_string( fp ) );
            break;
         case 'D':
            KEY( "Div", chan->div, fread_string( fp ) );
            break;
         case 'F':
            KEY( "Flags", chan->flags, fread_number( fp ) );
            break;
         case 'I':
            KEY( "Info", chan->info, fread_string( fp ) );
            break;
         case 'N':
            KEY( "Name", chan->name, fread_string( fp ) );
            break;
         case 'O':
            KEY( "Owner", chan->owner, fread_string( fp ) );
            break;
         case 'P':
            KEY( "Permit", chan->permit, fread_number( fp ) );
            break;
         case 'T':
            KEY( "Tag", chan->tag, fread_string( fp ) );
            break;
         default:
            bug( "Unknown channel keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void load_recipes( void )
{
   FILE *fp;
   RECIPE_DATA *recipe;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( RECIPE_FILE, "r" ) ) == NULL )
   {
      bug( "Load_recipes: no recipes found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#RECIPE", word ) )
            {
               RECIPE_DATA *new_recipe;

               CREATE( new_recipe, RECIPE_DATA, 1 );

               if( !first_recipe )
               {
                  first_recipe = new_recipe;
                  last_recipe = new_recipe;
               }
               else
               {
                  last_recipe->next = new_recipe;
                  new_recipe->prev = last_recipe;
                  last_recipe = new_recipe;
               }
               recipe = new_recipe;
               recipe->ingred1 = 0;
               recipe->ingred2 = 0;
               recipe->result = 0;
               recipe->flags = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'F':
            KEY( "First", recipe->ingred1, fread_number( fp ) );
            KEY( "Flags", recipe->flags, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Number", recipe->number, fread_number( fp ) );
            break;
         case 'R':
            KEY( "Result", recipe->result, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Second", recipe->ingred2, fread_number( fp ) );
            break;
         default:
            bug( "Unknown recipe keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void load_cities( void )
{
   FILE *fp;
   CITY_DATA *city;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( CITY_FILE, "r" ) ) == NULL )
   {
      bug( "Load_cities: no cities found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#CITY", word ) )
            {
               CITY_DATA *new_city;

               CREATE( new_city, CITY_DATA, 1 );

               if( !first_city )
               {
                  first_city = new_city;
                  last_city = new_city;
               }
               else
               {
                  last_city->next = new_city;
                  new_city->prev = last_city;
                  last_city = new_city;
               }
               city = new_city;
               city->peasants = 0;
               city->builders = 0;
               city->farmers = 0;
               city->merchants = 0;
               city->miners = 0;
               city->houses = 0;
               city->farms = 0;
               city->shops = 0;
               city->forests = 0;
               city->mines = 0;
               city->food = 0;
               city->gold = 0;
               city->wood = 0;
               city->iron = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'B':
            KEY( "Builders", city->builders, fread_number( fp ) );
            break;
         case 'F':
            KEY( "Farmers", city->farmers, fread_number( fp ) );
            KEY( "Farms", city->farms, fread_number( fp ) );
            KEY( "Food", city->food, fread_number( fp ) );
            KEY( "Forests", city->forests, fread_number( fp ) );
            break;
         case 'G':
            KEY( "Gold", city->gold, fread_number( fp ) );
            break;
         case 'H':
            KEY( "Houses", city->houses, fread_number( fp ) );
            break;
         case 'I':
            KEY( "Iron", city->iron, fread_number( fp ) );
            break;
         case 'M':
            KEY( "Merchants", city->merchants, fread_number( fp ) );
            KEY( "Miners", city->miners, fread_number( fp ) );
            KEY( "Mines", city->mines, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", city->name, fread_string( fp ) );
            break;
         case 'P':
            KEY( "Peasants", city->peasants, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Shops", city->shops, fread_number( fp ) );
            break;
         case 'W':
            KEY( "Wood", city->wood, fread_number( fp ) );
            break;
         default:
            bug( "Unknown city keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void load_arenas( void )
{
   FILE *fp;
   ARENA_DATA *arena;
   ARENA_MEMBER *member;
   BET_DATA *bet;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( ARENA_FILE, "r" ) ) == NULL )
   {
      bug( "Load_arenas: no arenas found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#ARENA", word ) )
            {
               ARENA_DATA *new_arena;

               CREATE( new_arena, ARENA_DATA, 1 );

               if( !first_arena )
               {
                  first_arena = new_arena;
                  last_arena = new_arena;
               }
               else
               {
                  last_arena->next = new_arena;
                  new_arena->prev = last_arena;
                  last_arena = new_arena;
               }
               arena = new_arena;
               arena->phase = 1;
               arena->vnum = 0;
               arena->room = NULL;
               arena->flags = 0;
            }
            else if( !str_cmp( "#MEMBER", word ) )
            {
               ARENA_MEMBER *new_member;

               if( !arena )
               {
                  bug( "#MEMBER tag without #ARENA first! Aborting.", 0 );
                  done = TRUE;
                  continue;
               }

               CREATE( new_member, ARENA_MEMBER, 1 );

               if( !arena->first_member )
               {
                  arena->first_member = new_member;
                  arena->last_member = new_member;
               }
               else
               {
                  arena->last_member->next = new_member;
                  new_member->prev = arena->last_member;
                  arena->last_member = new_member;
               }
               new_member->arena = arena;
               new_member->hp = 5;
               new_member->kills = 0;
               new_member->wins = 0;
               new_member->played = 0;
               new_member->str = 5;
               new_member->con = 5;
               new_member->dex = 5;
               new_member->spd = 5;
               new_member->skill = 0;
               new_member->flags = 0;
               new_member->age = 0;
               member = new_member;
            }
            else if( !str_cmp( "#BET", word ) )
            {
               BET_DATA *new_bet;

               if( !member )
               {
                  bug( "#BET tag without #MEMBER first! Aborting.", 0 );
                  done = TRUE;
                  continue;
               }

               CREATE( new_bet, BET_DATA, 1 );

               if( !member->first_bet )
               {
                  member->first_bet = new_bet;
                  member->last_bet = new_bet;
               }
               else
               {
                  member->last_bet->next = new_bet;
                  new_bet->prev = member->last_bet;
                  member->last_bet = new_bet;
               }
               bet = new_bet;
               bet->wager = 0;
               bet->odds = 0;
               bet->winnings = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'A':
            KEY( "Age", member->age, fread_number( fp ) );
            KEY( "Auto", member->flags, fread_number( fp ) );
            break;
         case 'B':
            KEY( "Better", bet->better, fread_string( fp ) );
            break;
         case 'C':
            KEY( "Con", member->con, fread_number( fp ) );
            break;
         case 'D':
            KEY( "Dex", member->dex, fread_number( fp ) );
            break;
         case 'F':
            KEY( "Flags", arena->flags, fread_number( fp ) );
            break;
         case 'H':
            KEY( "HP", member->hp, fread_number( fp ) );
            break;
         case 'K':
            KEY( "Kills", member->kills, fread_number( fp ) );
            break;
         case 'M':
            KEY( "Member_name", member->name, fread_string( fp ) );
            break;
         case 'N':
            KEY( "Name", arena->name, fread_string( fp ) );
            break;
         case 'O':
            KEY( "Odds", bet->odds, fread_number( fp ) );
            KEY( "Owner", member->owner, fread_string( fp ) );
            break;
         case 'P':
            KEY( "Phase", arena->phase, fread_number( fp ) );
            KEY( "Played", member->played, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Skill", member->skill, fread_number( fp ) );
            KEY( "Score", member->score, fread_number( fp ) );
            KEY( "Spd", member->spd, fread_number( fp ) );
            KEY( "Str", member->str, fread_number( fp ) );
            break;
         case 'T':
            KEY( "Team", member->team, fread_number( fp ) );
            KEY( "Total", member->total, fread_number( fp ) );
            break;
         case 'W':
            KEY( "Wager", bet->wager, fread_number( fp ) );
            KEY( "Winner", arena->winner, fread_string( fp ) );
            KEY( "Winnings", bet->winnings, fread_number( fp ) );
            KEY( "Wins", member->wins, fread_number( fp ) );
            break;
         case 'V':
            KEY( "Vnum", arena->vnum, fread_number( fp ) );
            break;
         default:
            bug( "Unknown arena keyword: %s", word );
      }
      arena->room = get_room_index( arena->vnum );
   }
   fclose( fp );
   return;
}


void load_moons( void )
{
   FILE *fp;
   MOON_DATA *moon;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( MOON_FILE, "r" ) ) == NULL )
   {
      bug( "Load_moons: no moons found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#MOON", word ) )
            {
               MOON_DATA *new_moon;

               CREATE( new_moon, MOON_DATA, 1 );

               if( !first_moon )
               {
                  first_moon = new_moon;
                  last_moon = new_moon;
               }
               else
               {
                  last_moon->next = new_moon;
                  new_moon->prev = last_moon;
                  last_moon = new_moon;
               }
               moon = new_moon;
               moon->name = str_dup( "newmoon" );
               moon->color = str_dup( "white" );
               moon->world = str_dup( "Nowhere" );
               moon->phase = 0;
               moon->rise = 16;
               moon->set = 6;
               moon->type = 0;
               moon->up = FALSE;
               moon->waning = FALSE;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'C':
            KEY( "Color", moon->color, fread_string( fp ) );
            break;
         case 'N':
            KEY( "Name", moon->name, fread_string( fp ) );
            break;
         case 'P':
            KEY( "Phase", moon->phase, fread_number( fp ) );
            break;
         case 'R':
            KEY( "Rise", moon->rise, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Set", moon->set, fread_number( fp ) );
            break;
         case 'T':
            KEY( "Type", moon->type, fread_number( fp ) );
            break;
         case 'W':
            KEY( "World", moon->world, fread_string( fp ) );
            break;
         default:
            bug( "Unknown moon keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}


void load_species( void )
{
   FILE *fp;
   SPECIES_DATA *species;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( SPECIES_FILE, "r" ) ) == NULL )
   {
      bug( "Load_species: no species found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#SPECIES", word ) )
            {
               SPECIES_DATA *new_species;

               CREATE( new_species, SPECIES_DATA, 1 );

               if( !first_species )
               {
                  first_species = new_species;
                  last_species = new_species;
               }
               else
               {
                  last_species->next = new_species;
                  new_species->prev = last_species;
                  last_species = new_species;
               }
               species = new_species;
               species->name = str_dup( "newspecies" );
               species->adj = str_dup( "new" );
               species->skill_set = 0;
               species->parts = 0;
               species->disadvantage = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'A':
            KEY( "Adjective", species->adj, fread_string( fp ) );
            break;
         case 'D':
            KEY( "Disadvantage", species->disadvantage, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", species->name, fread_string( fp ) );
            break;
         case 'P':
            KEY( "Part", species->parts, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Skillset", species->skill_set, fread_number( fp ) );
            break;
         default:
            bug( "Unknown species keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}


void load_nations( void )
{
   FILE *fp;
   NATION_DATA *nation = NULL;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( NATION_FILE, "r" ) ) == NULL )
   {
      bug( "Load_nations: no nations found!", 0 );
      return;
   }

   while( !done )
   {
      if( feof( fp ) )
      {
         break;
      }
      else
      {
         word = fread_word( fp );
      }

      switch ( UPPER( word[0] ) )
      {
         case '#':
            if( !str_cmp( "#NATION", word ) )
            {
               NATION_DATA *new_nation;

               CREATE( new_nation, NATION_DATA, 1 );

               if( !first_nation )
               {
                  first_nation = new_nation;
                  last_nation = new_nation;
               }
               else
               {
                  last_nation->next = new_nation;
                  new_nation->prev = last_nation;
                  last_nation = new_nation;
               }
               nation = new_nation;
               nation->name = str_dup( "newnation" );
               nation->hit = 0;
               nation->mana = 0;
               nation->sector = 0;
               nation->resist = 0;
               nation->suscept = 0;
               nation->unarmed = 0;
               nation->ac_plus = 0;
               nation->exp_multiplier = 0;
               nation->height = 66;
               nation->weight = 150;
               nation->parts = 0;
               nation->weapon = 0;
               nation->metabolism = 1;
               nation->base_age = 1;
               nation->str_mod = 0;
               nation->int_mod = 0;
               nation->wis_mod = 0;
               nation->dex_mod = 0;
               nation->con_mod = 0;
               nation->cha_mod = 0;
               nation->lck_mod = 0;
               nation->eyes = NULL;
               nation->hair = NULL;
               nation->skin_color = NULL;
               nation->skin_type = NULL;
               nation->extra_color = NULL;
               nation->extra_type = NULL;
               nation->flags = 0;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'A':
            KEY( "AC_plus", nation->ac_plus, fread_number( fp ) );
            /*
             * convert old system -keo 
             */
            if( nation->ac_plus < 0 )
               nation->ac_plus = abs( nation->ac_plus / 10 );
            KEY( "Affect", nation->affected, fread_bitvector( fp ) );
            break;
         case 'B':
            KEY( "Base_age", nation->base_age, fread_number( fp ) );
            KEY( "Bodyparts", nation->parts, fread_number( fp ) );
            break;
         case 'C':
            KEY( "Cha_mod", nation->cha_mod, fread_number( fp ) );
            KEY( "Con_mod", nation->con_mod, fread_number( fp ) );
            break;
         case 'D':
            KEY( "Dex_mod", nation->dex_mod, fread_number( fp ) );
            break;
         case 'E':
            KEY( "Extra_color", nation->extra_color, fread_string( fp ) );
            KEY( "Extra_type", nation->extra_type, fread_string( fp ) );
            KEY( "Eyes", nation->eyes, fread_string( fp ) );
         case 'H':
            KEY( "Hair", nation->hair, fread_string( fp ) );
            KEY( "Height", nation->height, fread_number( fp ) );
            KEY( "Hit", nation->hit, fread_number( fp ) );
            break;
         case 'I':
            KEY( "Int_mod", nation->int_mod, fread_number( fp ) );
            break;
         case 'L':
            KEY( "Lck_mod", nation->lck_mod, fread_number( fp ) );
            break;
         case 'M':
            KEY( "Metabolism", nation->metabolism, fread_number( fp ) );
            KEY( "Mana", nation->mana, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", nation->name, fread_string( fp ) );
            KEY( "NPC_only", nation->flags, fread_number( fp ) );
            break;
         case 'R':
            KEY( "Resist", nation->resist, fread_number( fp ) );
         case 'S':
            KEY( "Skin_color", nation->skin_color, fread_string( fp ) );
            KEY( "Skin_type", nation->skin_type, fread_string( fp ) );
            KEY( "Species", nation->species, fread_string( fp ) );
            KEY( "Sector", nation->sector, fread_number( fp ) );
            KEY( "Str_mod", nation->str_mod, fread_number( fp ) );
            KEY( "Suscept", nation->suscept, fread_number( fp ) );
            break;
         case 'U':
            KEY( "Unarmed", nation->unarmed, fread_number( fp ) );
            break;
         case 'W':
            KEY( "Weapon", nation->weapon, fread_number( fp ) );
            KEY( "Weight", nation->weight, fread_number( fp ) );
            KEY( "Wis_mod", nation->wis_mod, fread_number( fp ) );
            break;
         default:
            bug( "Unknown nation keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}


/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA * sys )
{
   char filename[MAX_INPUT_LENGTH];
   FILE *fp;
   bool found;

   found = FALSE;
   sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_sysdata_file: # not found." );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SYSTEM" ) )
         {
            fread_sysdata( sys, fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_sysdata_file: bad section." );
            break;
         }
      }
      fclose( fp );
   }

   return found;
}

/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   AREA_DATA *pArea;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   bool room, mob, obj, all, area_conflict;
   int low_range, high_range;

   room = FALSE;
   mob = FALSE;
   obj = FALSE;
   all = FALSE;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Please specify room, mob, object, or all as your first argument.\n\r", ch );
      return;
   }

   if( !str_cmp( arg1, "room" ) )
      room = TRUE;

   else if( !str_cmp( arg1, "mob" ) )
      mob = TRUE;

   else if( !str_cmp( arg1, "object" ) )
      obj = TRUE;

   else if( !str_cmp( arg1, "all" ) )
      all = TRUE;
   else
   {
      send_to_char( "Please specify room, mob, or object as your first argument.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      send_to_char( "Please specify the low end of the range to be searched.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Please specify the high end of the range to be searched.\n\r", ch );
      return;
   }

   low_range = atoi( arg2 );
   high_range = atoi( argument );

   if( low_range < 1 || low_range > 1048576000 )
   {
      send_to_char( "Invalid argument for bottom of range.\n\r", ch );
      return;
   }

   if( high_range < 1 || high_range > 1048576000 )
   {
      send_to_char( "Invalid argument for top of range.\n\r", ch );
      return;
   }

   if( high_range < low_range )
   {
      send_to_char( "Bottom of range must be below top of range.\n\r", ch );
      return;
   }

   if( all )
   {
      sprintf( buf, "room %d %d", low_range, high_range );
      do_check_vnums( ch, buf );
      sprintf( buf, "mob %d %d", low_range, high_range );
      do_check_vnums( ch, buf );
      sprintf( buf, "object %d %d", low_range, high_range );
      do_check_vnums( ch, buf );
      return;
   }
   set_char_color( AT_PLAIN, ch );

   for( pArea = first_asort; pArea; pArea = pArea->next_sort )
   {
      area_conflict = FALSE;
      if( IS_SET( pArea->status, AREA_DELETED ) )
         continue;
      else if( room )
      {
         if( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_r_vnum ) && ( low_range <= pArea->hi_r_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_r_vnum ) && ( high_range >= pArea->low_r_vnum ) )
            area_conflict = TRUE;
      }

      if( mob )
      {
         if( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
            area_conflict = TRUE;
         if( ( low_range >= pArea->low_m_vnum ) && ( low_range <= pArea->hi_m_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_m_vnum ) && ( high_range >= pArea->low_m_vnum ) )
            area_conflict = TRUE;
      }

      if( obj )
      {
         if( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_o_vnum ) && ( low_range <= pArea->hi_o_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_o_vnum ) && ( high_range >= pArea->low_o_vnum ) )
            area_conflict = TRUE;
      }

      if( area_conflict )
      {
         sprintf( buf, "Conflict:%-15s| ", ( pArea->filename ? pArea->filename : "(invalid)" ) );
         if( room )
            sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, pArea->hi_r_vnum );
         if( mob )
            sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, pArea->hi_m_vnum );
         if( obj )
            sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, pArea->hi_o_vnum );

         strcat( buf, buf2 );
         send_to_char( buf, ch );
      }
   }
   for( pArea = first_bsort; pArea; pArea = pArea->next_sort )
   {
      area_conflict = FALSE;
      if( IS_SET( pArea->status, AREA_DELETED ) )
         continue;
      else if( room )
      {
         if( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_r_vnum ) && ( low_range <= pArea->hi_r_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_r_vnum ) && ( high_range >= pArea->low_r_vnum ) )
            area_conflict = TRUE;
      }

      if( mob )
      {
         if( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
            area_conflict = TRUE;
         if( ( low_range >= pArea->low_m_vnum ) && ( low_range <= pArea->hi_m_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_m_vnum ) && ( high_range >= pArea->low_m_vnum ) )
            area_conflict = TRUE;
      }

      if( obj )
      {
         if( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
            area_conflict = TRUE;

         if( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
            area_conflict = TRUE;

         if( ( low_range >= pArea->low_o_vnum ) && ( low_range <= pArea->hi_o_vnum ) )
            area_conflict = TRUE;

         if( ( high_range <= pArea->hi_o_vnum ) && ( high_range >= pArea->low_o_vnum ) )
            area_conflict = TRUE;
      }

      if( area_conflict )
      {
         sprintf( buf, "Conflict:%-15s| ", ( pArea->filename ? pArea->filename : "(invalid)" ) );
         if( room )
            sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, pArea->hi_r_vnum );
         if( mob )
            sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, pArea->hi_m_vnum );
         if( obj )
            sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, pArea->hi_o_vnum );

         strcat( buf, buf2 );
         send_to_char( buf, ch );
      }
   }

/*
    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range)
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range)
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range)
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }

    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range)
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range)
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range)
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  sprintf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }
*/
   return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
   return;
}


/*
 * Initialize the weather for all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void init_area_weather(  )
{
   AREA_DATA *pArea;
   NEIGHBOR_DATA *neigh;
   NEIGHBOR_DATA *next_neigh;

   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      int cf;

      /*
       * init temp and temp vector 
       */
      cf = pArea->weather->climate_temp - 2;
      pArea->weather->temp = number_range( -weath_unit, weath_unit ) + cf * number_range( 0, weath_unit );
      pArea->weather->temp_vector = cf + number_range( -rand_factor, rand_factor );

      /*
       * init precip and precip vector 
       */
      cf = pArea->weather->climate_precip - 2;
      pArea->weather->precip = number_range( -weath_unit, weath_unit ) + cf * number_range( 0, weath_unit );
      pArea->weather->precip_vector = cf + number_range( -rand_factor, rand_factor );

      /*
       * init wind and wind vector 
       */
      cf = pArea->weather->climate_wind - 2;
      pArea->weather->wind = number_range( -weath_unit, weath_unit ) + cf * number_range( 0, weath_unit );
      pArea->weather->wind_vector = cf + number_range( -rand_factor, rand_factor );

      /*
       * init mana and mana vector 
       */
      cf = pArea->weather->climate_mana - 2;
      pArea->weather->mana = number_range( -weath_unit, weath_unit ) + cf * number_range( 0, weath_unit );
      pArea->weather->mana_vector = cf + number_range( -rand_factor, rand_factor );

      /*
       * check connections between neighbors 
       */
      for( neigh = pArea->weather->first_neighbor; neigh; neigh = next_neigh )
      {
         AREA_DATA *tarea;
         NEIGHBOR_DATA *tneigh;

         /*
          * get the address if needed 
          */
         if( !neigh->address )
            neigh->address = get_area( neigh->name );

         /*
          * area does not exist 
          */
         if( !neigh->address )
         {
            tneigh = neigh;
            next_neigh = tneigh->next;
            UNLINK( tneigh, pArea->weather->first_neighbor, pArea->weather->last_neighbor, next, prev );
            STRFREE( tneigh->name );
            DISPOSE( tneigh );
            fold_area( pArea, pArea->filename, FALSE );
            continue;
         }

         /*
          * make sure neighbors both point to each other 
          */
         tarea = neigh->address;
         for( tneigh = tarea->weather->first_neighbor; tneigh; tneigh = tneigh->next )
         {
            if( !strcmp( pArea->name, tneigh->name ) )
               break;
         }

         if( !tneigh )
         {
            CREATE( tneigh, NEIGHBOR_DATA, 1 );
            tneigh->name = STRALLOC( pArea->name );
            LINK( tneigh, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
            fold_area( tarea, tarea->filename, FALSE );
         }

         tneigh->address = pArea;

         next_neigh = neigh->next;
      }
   }

   return;
}

/*
 * Load weather data from appropriate file in system dir
 * Last Modified: July 24, 1997
 * Fireblade
 */
void load_weatherdata(  )
{
   char filename[MAX_INPUT_LENGTH];
   FILE *fp;

   sprintf( filename, "%sweather.dat", SYSTEM_DIR );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );

         if( letter != '#' )
         {
            bug( "load_weatherdata: # not found" );
            return;
         }

         word = fread_word( fp );

         if( !str_cmp( word, "RANDOM" ) )
            rand_factor = fread_number( fp );
         else if( !str_cmp( word, "CLIMATE" ) )
            climate_factor = fread_number( fp );
         else if( !str_cmp( word, "NEIGHBOR" ) )
            neigh_factor = fread_number( fp );
         else if( !str_cmp( word, "UNIT" ) )
            weath_unit = fread_number( fp );
         else if( !str_cmp( word, "MAXVECTOR" ) )
            max_vector = fread_number( fp );
         else if( !str_cmp( word, "END" ) )
         {
            fclose( fp );
            break;
         }
         else
         {
            bug( "load_weatherdata: unknown field" );
            fclose( fp );
            break;
         }
      }
   }

   return;
}

/*
 * Write data for global weather parameters
 * Last Modified: July 24, 1997
 * Fireblade
 */
void save_weatherdata(  )
{
   char filename[MAX_INPUT_LENGTH];
   FILE *fp;

   sprintf( filename, "%sweather.dat", SYSTEM_DIR );

   if( ( fp = fopen( filename, "w" ) ) != NULL )
   {
      fprintf( fp, "#RANDOM %d\n", rand_factor );
      fprintf( fp, "#CLIMATE %d\n", climate_factor );
      fprintf( fp, "#NEIGHBOR %d\n", neigh_factor );
      fprintf( fp, "#UNIT %d\n", weath_unit );
      fprintf( fp, "#MAXVECTOR %d\n", max_vector );
      fprintf( fp, "#END\n" );
      fclose( fp );
   }
   else
   {
      bug( "save_weatherdata: could not open file" );
   }

   return;
}
