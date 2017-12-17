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
 *		       Online Building and Editing Module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* from handler.c */
extern MATERIAL_DATA *material_lookup( int number );
extern MOON_DATA *find_moon( char *name );
extern SPECIES_DATA *find_species( char *name );
extern NATION_DATA *find_nation( char *name );
extern RECIPE_DATA *find_recipe( int i );
extern INGRED_DATA *find_ingred( int i );
extern MAT_SHOP *find_matshop( int i );
extern MUT_DATA *find_mutation( int i );

/* from interp.c */
extern bool check_social( CHAR_DATA * ch, char *command, char *argument );

/* from db.c */
extern void load_matshops args( ( void ) );

/* from makeobjs.c */
extern void make_randart args( ( OBJ_DATA * obj ) );

char *sprint_reset( RESET_DATA * pReset, short *num );

extern int top_affect;
extern int top_reset;
extern int top_ed;
extern bool fBootDb;
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

char *const fur_pos[] = {
   "stand_at", "stand_on", "stand_in",
   "sit_at", "sit_on", "sit_in",
   "reset_at", "rest_on", "rest_in",
   "sleep_at", "sleep_on", "sleep_in",
   "put_at", "put_on", "put_in"
};

char *const v_flags[] = { "land", "surface", "underwater", "air", "space", "underground", "climb",
   "airtight", "sand", "ice"
};

char *const ex_pmisc[] = { "undefined", "vortex", "vacuum", "slip", "ice", "mysterious" };

char *const ex_pwater[] = { "current", "wave", "whirlpool", "geyser" };

char *const ex_pair[] = { "wind", "storm", "coldwind", "breeze" };

char *const ex_pearth[] = { "landslide", "sinkhole", "quicksand", "earthquake" };

char *const ex_pfire[] = { "lava", "hotair" };


char *const ex_flags[] = {
   "isdoor", "closed", "locked", "secret", "swim",
   "pickproof", "fly", "climb", "dig", "eatkey",
   "nopassdoor", "hidden", "passage", "portal", "r14",
   "r15", "can_climb", "can_enter", "can_leave", "auto",
   "noflee", "searchable", "bashed", "bashproof", "nomob",
   "window", "can_look", "needle_trap", "r28", "r29",
   "r30", "r31"
};

char *const ex_pulltype[] = {
	"none", "vortex", "vacuum", "slip", "ice", "mysterious", "currect - water", "wave",
	"whirlpool", "geyser", "wind", "storm", "coldwind", "breeze", "landslide", "sinkhole",
	"quicksand", "earthquake", "lava", "hotair"
	};

char *const r_flags[] = {
   "dark", "noquit", "nomob", "indoors", "slippery", "sticky", "burning",
   "nomagic", "tunnel", "private", "safe", "solitary", "petshop", "norecall",
   "donation", "nodropall", "silence", "logspeech", "nodrop", "nodream",
   "nosummon", "notravel", "teleport", "teleshowdesc", "nofloor",
   "nosupplicate", "r26", "nomissile", "amplify", "r29", "prototype", "r31"
};

char *const bp_flags[] = {
   "head", "neck", "chest", "stomach", "back", "nose", "tail", "rhand",
   "lhand", "rarm", "larm", "rleg", "lleg",
   "rfoot", "lfoot", "reye", "leye", "rwrist",
   "lwrist", "rankle", "lankle", "rhoof", "lhoof",
   "rpaw", "lpaw", "rwing", "lwing", "rear",
   "lear", "horn", "rhorn", "lhorn", "tongue", "fangs",
   "tentacles", "beak", "rfleg", "lfleg", "rrleg",
   "lrleg", "waist", "face", "rfin", "lfin", "dfin",
   "groin", "claws"
};

char *const o_flags[] = {
   "glowing", "humming", "dark", "loyal", "evil", "invis", "magic", "nodrop", "bless",
   "in_use", "hovering", "noreset", "noremove", "inventory",
   "dreamworld", "explosive", "flammable", "notake", "organic",
   "metal", "returning", "gem", "durable", "shield", "twohanded",
   "hidden", "poisoned", "covering", "deathrot", "buried", "prototype",
   "nolocate", "groundrot", "artifact", "plrbld"
};

char *const container_flags[] = {
   "closeable", "pickproof", "closed", "locked", "eatkey", "r1", "r2", "r3",
   "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
   "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
   "r27"
};

char *const rune_flags[] = {
   "fire", "frost", "shock", "light", "pain", "acid", "poison", "travel",
   "life", "wind", "distract", "ward"
};

char *const w_flags[] = {
   "take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
   "shield", "about", "waist", "wrist", "hold", "_hold2_", "hooves", "ears", "eyes",
   "r4", "back", "face", "ankle", "floating", "r5", "r6",
   "r7", "r8", "r9", "r10", "r11", "r12", "r13"
};

char *const area_flags[] = {
   "nopkill", "freekill", "noteleport", "noquit", "nobypass", "notravel",
   "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16",
   "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
   "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

char *const o_types[] = {
   "none", "light", "scroll", "wand", "staff", "weapon", "radio",
   "scanner", "ore", "armor", "potion", "_worn", "furniture", "trash",
   "instrument", "container", "_note", "drinkcon", "key", "food", "money",
   "pen", "boat", "corpse", "corpse_pc", "fountain", "pill", "blood",
   "bloodstain", "scraps", "pipe", "herbcon", "herb", "incense", "fire",
   "book", "switch", "lever", "pullchain", "button", "dial", "rune",
   "runepouch", "sheath", "trap", "map", "portal", "paper", "tinder",
   "lockpick", "lock", "disease", "oil", "fuel", "vehicle", "bank",
   "missileweapon", "projectile", "quiver", "shovel", "salve", "cook",
   "keyring", "carvingknife", "odor"
};

char *const a_types[] = {
   "none", "strength", "dexterity", "intelligence", "willpower", "constitution",
   "sex", "unused", "unused2", "age", "height", "weight", "mana", "hit", "move",
   "gold", "experience", "armor", "hitroll", "damroll", "save_poison", "save_rod",
   "save_para", "save_breath", "save_spell", "perception", "affected", "resistant",
   "immune", "susceptible", "weaponspell", "luck", "fire", "earth", "wind",
   "frost", "lightning", "water", "dream", "speech", "healing", "death",
   "change", "time", "motion", "mind", "illusion", "seeking", "security",
   "catalysm", "void", "stun", "climb", "grip", "scribe", "brew",
   "wearspell", "removespell", "emotion", "mentalstate",
   "stripsn", "remove", "dig", "full", "thirst", "drunk", "blood", "cook",
   "recurringspell", "contagious", "xaffected", "odor", "roomflag", "sectortype",
   "roomlight", "televnum", "teledelay"
};

char *const a_flags[] = {
   "blind", "invisible", "detect_align", "detect_invis", "detect_magic",
   "detect_hidden", "hold", "holy", "faerie_fire", "infravision", "curse",
   "flaming", "poison", "protect", "_paralysis", "sneak", "hide", "sleep",
   "charm", "flying", "pass_door", "feather_fall", "truesight",
   "detect_traps", "scrying", "has_artifact", "angel", "fear",
   "beauty", "possess", "berserk", "breathing", "void",
   "dreamworld", "haste", "slow", "deaf", "nonliving", "coldblood",
   "guard", "glow", "dark", "aquatic", "demon", "silent", "plague",
   "rotting", "mountable", "mounted", "feline", "hellcurse", "undead",
   "construct", "ethereal", "nocorpse", "unholy", "magical"
};

char *const act_flags[] = {
   "npc", "sentinel", "scavenger", "attack_failed", "r4", "aggressive", "stayarea",
   "wimpy", "pet", "train", "practice", "unused", "deadly", "polyself",
   "meta_aggr", "guardian", "running", "nowander", "mountable", "mounted",
   "nocorpse", "secretive", "named", "mobinvis", "noassist", "autonomous",
   "pacifist", "noattack", "annoying", "r30", "prototype", "r32"
};

char *const pc_flags[] = {
   "auramask", "nopkill", "unauthed", "norecall", "nointro", "gag",
   "retired", "guest", "nosummon", "pager", "notitled", "mindshield",
   "diagnose", "highgag", "watch", "nstart", "r19", "nobeep", "anonymous",
   "spar", "vampire", "rw2",
   "counted", "buildwalk", "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

char *const plr_flags[] = {
   "npc", "longexit", "defense", "autoexits", "autoloot", "nohunger",
   "blank", "outcast", "brief", "combine", "prompt", "telnet_ga", "holylight",
   "wizinvis", "roomvnum", "silence", "noemote", "attacker", "notell", "log",
   "deny", "freeze", "thief", "killer", "litterbug", "ansi", "multi",
   "ooc", "showexp", "autogold", "automap", "idle", "invisprompt",
   "unused", "autosex", "nochan", "bounty"
};

char *const trap_flags[] = {
   "room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
   "unlock", "north", "south", "east", "r1", "west", "up", "down", "examine",
   "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
   "r14", "r15"
};

char *const cmd_flags[] = {
   "possessed", "polymorphed", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
   "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
   "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30"
};

char *const wear_locs[] = {
   "light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
   "feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
   "wield", "hold", "dual_wield", "ears", "eyes", "missile_wield", "back",
   "face", "ankle1", "ankle2", "float"
};

char *const ris_flags[] = {
   "fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
   "poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
   "plus3", "plus4", "light", "psionic", "magic", "paralysis", "r1", "r2",
   "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10"
};

char *const trig_flags[] = {
   "up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
   "d_down", "door", "container", "open", "close", "passage", "oload", "mload",
   "teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
   "rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
   "showroomdesc", "autoreturn", "r2", "r3"
};

char *const part_flags[] = {
   "head", "arms", "legs", "chest", "neck", "stomach", "hands", "feet",
   "fingers", "ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins",
   "wings", "tail", "scales", "claws", "fangs", "horns", "tusks",
   "tailattack", "sharpscales", "beak", "haunches", "hooves", "paws",
   "forelegs", "feathers", "r1", "r2"
};

char *const attack_flags[] = {
   "bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
   "gouge", "backstab", "feed", "drain", "firebreath", "frostbreath",
   "acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
   "blindness", "causeserious", "earthquake", "causecritical", "curse",
   "flamestrike", "harm", "fireball", "colorspray", "weaken", "r1"
};

char *const defense_flags[] = {
   "parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
   "dispelmagic", "dispelevil", "sanctuary", "fireshield", "shockshield",
   "shield", "bless", "stoneskin", "teleport", "monsum1", "monsum2", "monsum3",
   "monsum4", "disarm", "iceshield", "grip", "truesight", "r4", "r5", "r6", "r7",
   "r8", "r9", "r10", "r11", "r12"
};

char *const moon_flags[] = {
   "dark", "hunger", "blood"
};

char *const recipe_flags[] = {
   "fire", "water", "drinkcon", "container", "ice", "sun", "moon", "light",
   "dark", "knife", "blood", "r12", "r13", "r14", "r15", "r16", "r17", "r18",
   "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28",
   "r29", "r30", "r31"
};

/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 * 
 */
char *const mprog_flags[] = {
   "act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
   "allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
   "look", "exa", "zap", "get", "drop", "damage", "unused1", "randiw",
   "speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use"
};


char *flag_string( int bitvector, char *const flagarray[] )
{
   static char buf[MAX_STRING_LENGTH];
   int x;

   buf[0] = '\0';
   for( x = 0; x < 32; x++ )
   {
      if( IS_SET( bitvector, 1 << x ) )
      {
         strcat( buf, flagarray[x] );
         strcat( buf, " " );
      }
   }

   if( ( x = strlen( buf ) ) > 0 )
      buf[--x] = '\0';

   return buf;
}

char *ext_flag_string( EXT_BV * bitvector, char *const flagarray[] )
{
   static char buf[MAX_STRING_LENGTH];
   int x;

   buf[0] = '\0';
   for( x = 0; x < MAX_BITS; x++ )
   {
      if( xIS_SET( *bitvector, x ) )
      {
         strcat( buf, flagarray[x] );
         strcat( buf, " " );
      }
   }
   if( ( x = strlen( buf ) ) > 0 )
      buf[--x] = '\0';

   return buf;
}

int get_otype( char *type )
{
   int x;

   for( x = 0; x < ( sizeof( o_types ) / sizeof( o_types[0] ) ); x++ )
      if( !str_cmp( type, o_types[x] ) )
         return x;
   return -1;
}

int get_aflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( a_flags ) / sizeof( a_flags[0] ) ); x++ )
      if( !str_cmp( flag, a_flags[x] ) )
         return x;
   return -1;
}

int get_recipe_flag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( recipe_flags ) / sizeof( recipe_flags[0] ) ); x++ )
      if( !str_cmp( flag, recipe_flags[x] ) )
         return x;
   return -1;
}

int get_moon_flag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( moon_flags ) / sizeof( moon_flags[0] ) ); x++ )
      if( !str_cmp( flag, moon_flags[x] ) )
         return x;
   return -1;
}


int get_trapflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( trap_flags ) / sizeof( trap_flags[0] ) ); x++ )
      if( !str_cmp( flag, trap_flags[x] ) )
         return x;
   return -1;
}

int get_atype( char *type )
{
   int x;

   for( x = 0; x < ( sizeof( a_types ) / sizeof( a_types[0] ) ); x++ )
      if( !str_cmp( type, a_types[x] ) )
         return x;
   return -1;
}

int get_npc_race( char *type )
{
   int x;

   for( x = 0; x < MAX_NPC_RACE; x++ )
      if( !str_cmp( type, npc_race[x] ) )
         return x;
   return -1;
}

int get_fur_pos( char *type )
{
   int x;

   for( x = 0; x < ( sizeof( fur_pos ) / sizeof( fur_pos[0] ) ); x++ )
      if( !str_cmp( type, fur_pos[x] ) )
         return x;
   return -1;
}

int get_vflag( char *type )
{
   int x;

   for( x = 0; x < ( sizeof( v_flags ) / sizeof( v_flags[0] ) ); x++ )
      if( !str_cmp( type, v_flags[x] ) )
         return x;
   return -1;
}

int get_wearloc( char *type )
{
   int x;

   for( x = 0; x < ( sizeof( wear_locs ) / sizeof( wear_locs[0] ) ); x++ )
      if( !str_cmp( type, wear_locs[x] ) )
         return x;
   return -1;
}

int get_exflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( ex_flags ) / sizeof( ex_flags[0] ) ); x++ )
      if( !str_cmp( flag, ex_flags[x] ) )
         return x;
   return -1;
}

int get_pulltype( char *type )
{
   int x;

   if( !str_cmp( type, "none" ) || !str_cmp( type, "clear" ) )
      return 0;

   for( x = 0; x < ( sizeof( ex_pmisc ) / sizeof( ex_pmisc[0] ) ); x++ )
      if( !str_cmp( type, ex_pmisc[x] ) )
         return x;

   for( x = 0; x < ( sizeof( ex_pwater ) / sizeof( ex_pwater[0] ) ); x++ )
      if( !str_cmp( type, ex_pwater[x] ) )
         return x + PT_WATER;
   for( x = 0; x < ( sizeof( ex_pair ) / sizeof( ex_pair[0] ) ); x++ )
      if( !str_cmp( type, ex_pair[x] ) )
         return x + PT_AIR;
   for( x = 0; x < ( sizeof( ex_pearth ) / sizeof( ex_pearth[0] ) ); x++ )
      if( !str_cmp( type, ex_pearth[x] ) )
         return x + PT_EARTH;
   for( x = 0; x < ( sizeof( ex_pfire ) / sizeof( ex_pfire[0] ) ); x++ )
      if( !str_cmp( type, ex_pfire[x] ) )
         return x + PT_FIRE;
   return -1;
}

int get_rflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( r_flags ) / sizeof( r_flags[0] ) ); x++ )
      if( !str_cmp( flag, r_flags[x] ) )
         return x;
   return -1;
}

int get_runeflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( rune_flags ) / sizeof( rune_flags[0] ) ); x++ )
      if( !str_cmp( flag, rune_flags[x] ) )
         return x;
   return -1;
}

int get_mpflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( mprog_flags ) / sizeof( mprog_flags[0] ) ); x++ )
      if( !str_cmp( flag, mprog_flags[x] ) )
         return x;
   return -1;
}

int get_bpflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( bp_flags ) / sizeof( bp_flags[0] ) ); x++ )
      if( !str_cmp( flag, bp_flags[x] ) )
         return x;
   return -1;
}

int get_oflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( o_flags ) / sizeof( o_flags[0] ) ); x++ )
      if( !str_cmp( flag, o_flags[x] ) )
         return x;
   return -1;
}

int get_areaflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( area_flags ) / sizeof( area_flags[0] ) ); x++ )
      if( !str_cmp( flag, area_flags[x] ) )
         return x;
   return -1;
}

int get_wflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( w_flags ) / sizeof( w_flags[0] ) ); x++ )
      if( !str_cmp( flag, w_flags[x] ) )
         return x;
   return -1;
}

int get_actflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( act_flags ) / sizeof( act_flags[0] ) ); x++ )
      if( !str_cmp( flag, act_flags[x] ) )
         return x;
   return -1;
}

int get_pcflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( pc_flags ) / sizeof( pc_flags[0] ) ); x++ )
      if( !str_cmp( flag, pc_flags[x] ) )
         return x;
   return -1;
}

int get_plrflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( plr_flags ) / sizeof( plr_flags[0] ) ); x++ )
      if( !str_cmp( flag, plr_flags[x] ) )
         return x;
   return -1;
}

int get_risflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( ris_flags ) / sizeof( ris_flags[0] ) ); x++ )
      if( !str_cmp( flag, ris_flags[x] ) )
         return x;
   return -1;
}

/*
 * For use with cedit --Shaddai
 */

int get_cmdflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( cmd_flags ) / sizeof( cmd_flags[0] ) ); x++ )
      if( !str_cmp( flag, cmd_flags[x] ) )
         return x;
   return -1;
}

int get_trigflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( trig_flags ) / sizeof( trig_flags[0] ) ); x++ )
      if( !str_cmp( flag, trig_flags[x] ) )
         return x;
   return -1;
}

int get_partflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( part_flags ) / sizeof( part_flags[0] ) ); x++ )
      if( !str_cmp( flag, part_flags[x] ) )
         return x;
   return -1;
}

int get_attackflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( attack_flags ) / sizeof( attack_flags[0] ) ); x++ )
      if( !str_cmp( flag, attack_flags[x] ) )
         return x;
   return -1;
}

int get_defenseflag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( defense_flags ) / sizeof( defense_flags[0] ) ); x++ )
      if( !str_cmp( flag, defense_flags[x] ) )
         return x;
   return -1;
}

/*
 * Remove carriage returns from a line
 */
char *strip_cr( char *str )
{
   static char newstr[MAX_STRING_LENGTH];
   int i, j;

   for( i = j = 0; str[i] != '\0'; i++ )
      if( str[i] != '\r' )
      {
         newstr[j++] = str[i];
      }
   newstr[j] = '\0';
   return newstr;
}

/* Remove crs and newlines from a line -- Scion */
char *strip_crn( char *str )
{
   static char newstr[MAX_STRING_LENGTH];
   int i, j;

   for( i = j = 0; str[i] != '\0'; i++ )
   {
      if( str[i] != '\r' && str[i] != '\n' )
      {
         newstr[j++] = str[i];
      }
   }
   newstr[j] = '\0';
   return newstr;
}

/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
   int len;
   char last;
   char *strptr;

   strptr = str;

   len = strlen( str );
   if( len )
      last = strptr[len - 1];
   else
      last = '\0';

   for( ; *str != '\0'; str++ )
   {
      if( *str == '~' )
         *str = '-';
   }
   if( len )
      strptr[len - 1] = last;

   return;
}


void start_editing( CHAR_DATA * ch, char *data )
{
   EDITOR_DATA *edit;
   sh_int lines, size, lpos;
   char c;

   if( !ch->desc )
   {
      bug( "Fatal: start_editing: no desc", 0 );
      return;
   }
   if( ch->substate == SUB_RESTRICTED )
      bug( "NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED", 0 );

   act( AT_PLAIN, "$n begins writing.", ch, NULL, NULL, TO_ROOM );
   set_char_color( AT_GREEN, ch );
   send_to_char( "Begin entering your text now (/? = help /s = save /c = clear /l = list)\n\r", ch );
   send_to_char( "-----------------------------------------------------------------------\n\r> ", ch );
   if( ch->editor )
      stop_editing( ch );

   CREATE( edit, EDITOR_DATA, 1 );
   edit->numlines = 0;
   edit->on_line = 0;
   edit->size = 0;
   size = 0;
   lpos = 0;
   lines = 0;
   if( !data )
      bug( "editor: data is NULL!\n\r", 0 );
   else
      for( ;; )
      {
         c = data[size++];
         if( c == '\0' )
         {
            edit->line[lines][lpos] = '\0';
            break;
         }
         else if( c == '\r' );
         else if( c == '\n' || lpos > 78 )
         {
            edit->line[lines][lpos] = '\0';
            ++lines;
            lpos = 0;
         }
         else
            edit->line[lines][lpos++] = c;
         if( lines >= 49 || size > 4096 )
         {
            edit->line[lines][lpos] = '\0';
            break;
         }
      }
   if( lpos > 0 && lpos < 78 && lines < 49 )
   {
      edit->line[lines][lpos] = '~';
      edit->line[lines][lpos + 1] = '\0';
      ++lines;
      lpos = 0;
   }
   edit->numlines = lines;
   edit->size = size;
   edit->on_line = lines;
   ch->editor = edit;
   ch->desc->connected = CON_EDITING;
}

char *copy_buffer( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char tmp[100];
   sh_int x, len;

   if( !ch )
   {
      bug( "copy_buffer: null ch", 0 );
      return STRALLOC( "" );
   }

   if( !ch->editor )
   {
      bug( "copy_buffer: null editor", 0 );
      return STRALLOC( "" );
   }

   buf[0] = '\0';
   for( x = 0; x < ch->editor->numlines; x++ )
   {
      strcpy( tmp, ch->editor->line[x] );
      len = strlen( tmp );
      if( tmp != NULL && tmp[len - 1] == '~' )
         tmp[len - 1] = '\0';
      else
         strcat( tmp, "\n\r" );
      smush_tilde( tmp );
      strcat( buf, tmp );
   }
   return STRALLOC( buf );
}

void stop_editing( CHAR_DATA * ch )
{
   set_char_color( AT_PLAIN, ch );
   DISPOSE( ch->editor );
   ch->editor = NULL;
   send_to_char( "Done.\n\r", ch );
   ch->dest_buf = NULL;
   ch->spare_ptr = NULL;
   ch->substate = SUB_NONE;
   if( !ch->desc )
   {
      bug( "Fatal: stop_editing: no desc", 0 );
      return;
   }
   act( AT_PLAIN, "$n finishes writing.", ch, NULL, NULL, TO_ROOM );
   ch->desc->connected = CON_PLAYING;
}


void do_goto( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   CHAR_DATA *fch;
   CHAR_DATA *fch_next;
   ROOM_INDEX_DATA *in_room;
   AREA_DATA *pArea;
   int vnum;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Goto where?\n\r", ch );
      return;
   }
   if( ( location = find_location( ch, arg ) ) == NULL )
   {
      vnum = atoi( arg );
      if( vnum < 0 || get_room_index( vnum ) )
      {
         send_to_char( "You cannot find that...\n\r", ch );
         return;
      }
      if( vnum < 1 || IS_NPC( ch ) )
      {
         send_to_char( "No such location.\n\r", ch );
         return;
      }
      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to create rooms.\n\r", ch );
         return;
      }
      location = make_room( vnum );
      if( !location )
      {
         bug( "Goto: make_room failed", 0 );
         return;
      }
      location->area = ch->pcdata->area;
      set_char_color( AT_WHITE, ch );
      send_to_char( "Waving your hand, you form order from swirling chaos,\n\rand step into a new reality...\n\r", ch );
   }

   if( room_is_private( location ) )
   {
      if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
      {
         send_to_char( "That room is private right now.\n\r", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\n\r", ch );
      }
   }

   in_room = ch->in_room;
   if( ch->position != POS_STANDING )
      do_stand( ch, "" );

   act( AT_MAGIC, ch->pcdata->bamfout, ch, NULL, NULL, TO_ROOM );

   ch->regoto = ch->in_room->vnum;
   char_from_room( ch );
   if( ch->mount )
   {
      char_from_room( ch->mount );
      char_to_room( ch->mount, location );
   }
   char_to_room( ch, location );

   act( AT_MAGIC, ch->pcdata->bamfin, ch, NULL, NULL, TO_ROOM );

   do_look( ch, "auto" );

   if( ch->in_room == in_room )
      return;
   for( fch = in_room->first_person; fch; fch = fch_next )
   {
      fch_next = fch->next_in_room;
      if( fch->master == ch && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
      {
         act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
         do_goto( fch, argument );
      }
/* Experimental change by Gorog so imm's personal mobs follow them */
      else if( IS_NPC( fch ) && fch->master == ch )
      {
         char_from_room( fch );
         char_to_room( fch, location );
      }
   }
   return;
}

void do_mset( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char outbuf[MAX_STRING_LENGTH];
   int num, size, plus;
   char char1, char2;
   CHAR_DATA *victim;
   int value;
   int minattr, maxattr, minskin, maxskin;
   bool lockvictim;
   char *origarg = argument;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mob's can't mset\n\r", ch );
      return;
   }

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_MOB_DESC:
         if( !ch->dest_buf )
         {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_mset: sub_mob_desc: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
         }
         victim = ch->dest_buf;
         if( char_died( victim ) )
         {
            send_to_char( "Your victim died!\n\r", ch );
            stop_editing( ch );
            return;
         }
         STRFREE( victim->description );
         victim->description = copy_buffer( ch );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->description );
            victim->pIndexData->description = QUICKLINK( victim->description );
         }
         stop_editing( ch );
         ch->substate = ch->tempnum;
         return;
   }

   victim = NULL;
   lockvictim = FALSE;
   smash_tilde( argument );

   if( ch->substate == SUB_REPEATCMD )
   {
      victim = ch->dest_buf;

      if( char_died( victim ) )
      {
         send_to_char( "Your victim died!\n\r", ch );
         victim = NULL;
         argument = "done";
      }
      if( argument[0] == '\0' || !str_cmp( argument, " " ) || !str_cmp( argument, "stat" ) )
      {
         if( victim )
            do_mstat( ch, victim->name );
         else
            send_to_char( "No victim selected.  Type '?' for help.\n\r", ch );
         return;
      }
      if( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
      {
         send_to_char( "Mset mode off.\n\r", ch );
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
         if( ch->pcdata && ch->pcdata->subprompt )
         {
            STRFREE( ch->pcdata->subprompt );
            ch->pcdata->subprompt = NULL;
         }
         return;
      }
   }
   if( victim )
   {
      lockvictim = TRUE;
      strcpy( arg1, victim->name );
      argument = one_argument( argument, arg2 );
      strcpy( arg3, argument );
   }
   else
   {
      lockvictim = FALSE;
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      strcpy( arg3, argument );
   }

   if( !str_cmp( arg1, "on" ) )
   {
      send_to_char( "Syntax: mset <victim|vnum> on.\n\r", ch );
      return;
   }

   if( arg1[0] == '\0' || ( arg2[0] == '\0' && ch->substate != SUB_REPEATCMD ) || !str_cmp( arg1, "?" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
      {
         if( victim )
            send_to_char( "Syntax: <field>  <value>\n\r", ch );
         else
            send_to_char( "Syntax: <victim> <field>  <value>\n\r", ch );
      }
      else
         send_to_char( "Syntax: mset <victim> <field>  <value>\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Field being one of:\n\r", ch );
      send_to_char( "  str int wil dex con per end sex\n\r", ch );
      send_to_char( "  gold hp mana move practice race\n\r", ch );
      send_to_char( "  hitroll damroll armor affected\n\r", ch );
      send_to_char( "  thirst drunk full blood flags weight\n\r", ch );
      send_to_char( "  pos defpos part (see BODYPARTS)\n\r", ch );
      send_to_char( "  sav1 sav2 sav4 sav4 sav5 (see SAVINGTHROWS)\n\r", ch );
      send_to_char( "  resistant immune susceptible (see RIS)\n\r", ch );
      send_to_char( "  attack defense numattacks species\n\r", ch );
      send_to_char( "  name short long description title spec\n\r", ch );
      send_to_char( "  age qp qpa favor deity\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "skinamount - set amount 0 to 4\n\r", ch);
      send_to_char( " &GHideype: \n\r", ch);
      send_to_char( "     &G1)Scraps,    2)Scrawny Pelt, 3)Scrawny Fur, 4)Scrawny Hide, 5)Rough Pelt, 6)Rough Fur\r\n", ch );
      send_to_char( "     &G7)Rough Hide, 8)Thin Pelt, 9)Thin Fur, 10)Thin Hide, 11)Pelt, 12)Fur, 13)Hide, 14)Tough Pelt\r\n", ch );
      send_to_char( "    &G15)Tough Fur, 16)Tough Hide, 17)Fine Pelt, 18)Fine Fur, 19)Fine Hide, 20)Fancy Pelt\r\n", ch );
      send_to_char( "    &G21)Fancy Fur, 22)Fancy Hide\r\n&w\n\r", ch);
      send_to_char( "\n\r", ch );
      send_to_char( "For editing index/prototype mobiles:\n\r", ch );
      send_to_char( "  hitnumdie hitsizedie hitplus (hit points)\n\r", ch );
      send_to_char( "  damnumdie damsizedie damplus (damage roll)\n\r", ch );
      send_to_char( "To toggle area flag: aloaded\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "No one like that in all the realms.\n\r", ch );
      return;
   }

   if( lockvictim )
      ch->dest_buf = victim;

   minattr = 1;
   maxattr = 1000;
   minskin = 0;
   maxskin = 4;
   
   if( !str_cmp( arg2, "on" ) )
   {
      CHECK_SUBRESTRICTED( ch );
      ch_printf( ch, "Mset mode on. (Editing %s).\n\r", victim->name );
      ch->substate = SUB_REPEATCMD;
      ch->dest_buf = victim;
      if( ch->pcdata )
      {
         if( ch->pcdata->subprompt )
            STRFREE( ch->pcdata->subprompt );
         if( IS_NPC( victim ) )
            sprintf( buf, "<&CMset &W#%d&w> %%i", victim->pIndexData->vnum );
         else
            sprintf( buf, "<&CMset &W%s&w> %%i", victim->name );
         ch->pcdata->subprompt = STRALLOC( buf );
      }
      return;
   }
   value = is_number( arg3 ) ? atoi( arg3 ) : -1;

   if( atoi( arg3 ) < -1 && value == -1 )
      value = atoi( arg3 );
   
    if( !str_cmp( arg2, "skinamount" ) )
   {
      if( value < minskin || value > maxskin )
      {
          ch_printf( ch, "Skinning Amount range if %d to %d. \n\r", minskin, maxskin );
          victim->skinamount = 0;
          return;
      }
      victim->skinamount = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->skinamount = value;
      return;
   }
       if( !str_cmp( arg2, "hidetype" ) )
   {
      if( value < 0 || value > SKIN_MAX )
      {
          ch_printf( ch, "Hide type range if %d to %d. \n\r", 0, SKIN_MAX );
          victim->hide_type = 0;
          return;
      }
      victim->hide_type = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hide_type = value;
      return;
   }
   if( !str_cmp( arg2, "str" ) )
   {
      if( value < minattr || value > maxattr )
      {
         ch_printf( ch, "Strength range is %d to %d.\n\r", minattr, maxattr );
         return;
      }
      victim->perm_str = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_str = value;
      return;
   }

   if( !str_cmp( arg2, "int" ) )
   {
      if( value < minattr || value > maxattr )
      {
         ch_printf( ch, "Intelligence range is %d to %d.\n\r", minattr, maxattr );
         return;
      }
      victim->perm_int = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_int = value;
      return;
   }

   if( !str_cmp( arg2, "wil" ) )
   {
      if( value < minattr || value > maxattr )
      {
         ch_printf( ch, "Willpower range is %d to %d.\n\r", minattr, maxattr );
         return;
      }
      victim->perm_wil = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_wil = value;
      return;
   }

   if( !str_cmp( arg2, "dex" ) )
   {
      if( value < minattr || value > maxattr )
      {
         ch_printf( ch, "Dexterity range is %d to %d.\n\r", minattr, maxattr );
         return;
      }
      victim->perm_dex = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_dex = value;
      return;
   }

   if( !str_cmp( arg2, "con" ) )
   {
      if( value < minattr || value > maxattr )
      {
         ch_printf( ch, "Constitution range is %d to %d.\n\r", minattr, maxattr );
         return;
      }
      victim->perm_con = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_con = value;
      return;
   }

   if( !str_cmp( arg2, "per" ) )
   {
      if( value < minattr || value > maxattr )
      {
         ch_printf( ch, "Perception range is %d to %d.\n\r", minattr, maxattr );
         return;
      }
      victim->perm_per = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_per = value;
      return;
   }

   if( !str_cmp( arg2, "end" ) )
   {
      if( value < minattr || value > maxattr )
      {
         ch_printf( ch, "Endurance range is %d to %d.\n\r", minattr, maxattr );
         return;
      }
      victim->perm_lck = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->perm_lck = value;
      return;
   }

   if( !str_cmp( arg2, "height" ) )
   {
      victim->height = URANGE( 10, value, 1200 );
      return;
   }

   if( !str_cmp( arg2, "weight" ) )
   {
      victim->weight = URANGE( 10, value, 5000 );
      return;
   }

   if( !str_cmp( arg2, "age" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPCs.\n\r", ch );
         return;
      }
      victim->pcdata->age_adjust = URANGE( -100000, value, 100000 );
      return;
   }

   if( !str_cmp( arg2, "sav1" ) )
   {
      if( value < -30 || value > 30 )
      {
         send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
         return;
      }
      victim->saving_poison_death = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_poison_death = value;
      return;
   }

   if( !str_cmp( arg2, "sav2" ) )
   {
      if( value < -30 || value > 30 )
      {
         send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
         return;
      }
      victim->saving_wand = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_wand = value;
      return;
   }

   if( !str_cmp( arg2, "sav3" ) )
   {
      if( value < -30 || value > 30 )
      {
         send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
         return;
      }
      victim->saving_para_petri = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_para_petri = value;
      return;
   }

   if( !str_cmp( arg2, "sav4" ) )
   {
      if( value < -30 || value > 30 )
      {
         send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
         return;
      }
      victim->saving_breath = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_breath = value;
      return;
   }

   if( !str_cmp( arg2, "sav5" ) )
   {
      if( value < -30 || value > 30 )
      {
         send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
         return;
      }
      victim->saving_spell_staff = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->saving_spell_staff = value;
      return;
   }

   if( !str_cmp( arg2, "sex" ) )
   {
      if( value < 0 || value > 3 )
      {
         send_to_char( "Sex range is 0 to 3.\n\r", ch );
         return;
      }
      victim->sex = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->sex = value;
      return;
   }

   if( !str_cmp( arg2, "species" ) )
   {
      victim->nation = find_nation( arg3 );
      if( victim->nation )
      {
         STRFREE( victim->species );
         victim->species = STRALLOC( victim->nation->name );
      }
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on mobs.\n\r", ch );
         return;
      }
      STRFREE( victim->pcdata->type );
      victim->pcdata->type = STRALLOC( arg3 );
      return;
   }

   if( !str_cmp( arg2, "inborn" ) )
   {
      int i;

      i = atoi( arg3 );
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on mobs.\n\r", ch );
         return;
      }
      if( i < -1 || i > MAX_DEITY )
      {
         send_to_char( "Out of range.\n\r", ch );
         return;
      }
      victim->pcdata->inborn = i;
      return;
   }

   if( !str_cmp( arg2, "race" ) )
   {
      value = get_npc_race( arg3 );
      if( value < 0 )
         value = atoi( arg3 );
      if( !IS_NPC( victim ) && ( value < 0 || value >= MAX_RACE ) )
      {
         ch_printf( ch, "Race range is 0 to %d.\n", MAX_RACE - 1 );
         return;
      }
      if( IS_NPC( victim ) && ( value < 0 || value >= MAX_NPC_RACE ) )
      {
         ch_printf( ch, "Race range is 0 to %d.\n", MAX_NPC_RACE - 1 );
         return;
      }
      victim->race = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->race = value;
      return;
   }

   if( !str_cmp( arg2, "armor" ) )
   {
      if( value < 0 || value > 3000 )
      {
         send_to_char( "AC range is 0 to 3000.\n\r", ch );
         return;
      }
      victim->armor = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->ac = value;
      return;
   }

   if( !str_cmp( arg2, "numattacks" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Not on PC's.\n\r", ch );
         return;
      }

      if( value < 0 || value > 20 )
      {
         send_to_char( "Attacks range is 0 to 20.\n\r", ch );
         return;
      }
      victim->numattacks = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->numattacks = value;
      return;
   }

   if( !str_cmp( arg2, "gold" ) )
   {
      victim->gold = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->gold = value;
      return;
   }

   if( !str_cmp( arg2, "hitroll" ) )
   {
      victim->hitroll = URANGE( 0, value, 85 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitroll = victim->hitroll;
      return;
   }

   if( !str_cmp( arg2, "damroll" ) )
   {
      victim->damroll = URANGE( 0, value, 65 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damroll = victim->damroll;
      return;
   }

   if( !str_cmp( arg2, "hp" ) )
   {
      if( value < 1 || value > 32700 )
      {
         send_to_char( "Hp range is 1 to 32,700 hit points.\n\r", ch );
         return;
      }
      victim->max_hit = value;
      return;
   }

   if( !str_cmp( arg2, "mana" ) )
   {
      if( value < 0 || value > 30000 )
      {
         send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
         return;
      }
      victim->max_mana = value;
      return;
   }

   
   
   if( !str_cmp( arg2, "move" ) )
   {
      if( value < 0 || value > 30000 )
      {
         send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
         return;
      }
      victim->max_move = value;
      return;
   }

   if( !str_cmp( arg2, "password" ) )
   {
      char *pwdnew;
      char *p;

      if( IS_NPC( victim ) )
      {
         send_to_char( "Mobs don't have passwords.\n\r", ch );
         return;
      }

      if( strlen( arg3 ) < 5 )
      {
         send_to_char( "New password must be at least five characters long.\n\r", ch );
         return;
      }

      /*
       * No tilde allowed because of player file format.
       */
      pwdnew = smaug_crypt( arg3 );
      for( p = pwdnew; *p != '\0'; p++ )
      {
         if( *p == '~' )
         {
            send_to_char( "New password not acceptable, try again.\n\r", ch );
            return;
         }
      }

      DISPOSE( victim->pcdata->pwd );
      victim->pcdata->pwd = str_dup( pwdnew );
      if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
         save_char_obj( victim );
      send_to_char( "Ok.\n\r", ch );
      ch_printf( victim, "Your password has been changed by %s.\n\r", ch->name );
      return;
   }

   if( !str_cmp( arg2, "rank" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPC's.\n\r", ch );
         return;
      }
      smash_tilde( argument );
      DISPOSE( victim->pcdata->rank );
      if( !argument || argument[0] == '\0' || !str_cmp( argument, "none" ) )
         victim->pcdata->rank = str_dup( "" );
      else
         victim->pcdata->rank = str_dup( argument );
      send_to_char( "Ok.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "mentalstate" ) )
   {
      if( value < -100 || value > 100 )
      {
         send_to_char( "Value must be in range -100 to +100.\n\r", ch );
         return;
      }
      victim->mental_state = value;
      return;
   }

   if( !str_cmp( arg2, "emotion" ) )
   {
      if( value < -100 || value > 100 )
      {
         send_to_char( "Value must be in range -100 to +100.\n\r", ch );
         return;
      }
      victim->emotional_state = value;
      return;
   }

   if( !str_cmp( arg2, "thirst" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPC's.\n\r", ch );
         return;
      }

      if( value < -1 || value > 100 )
      {
         send_to_char( "Thirst range is -1 to 100.\n\r", ch );
         return;
      }

      victim->pcdata->condition[COND_THIRST] = value;
      return;
   }

   if( !str_cmp( arg2, "drunk" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPC's.\n\r", ch );
         return;
      }

      if( value < 0 || value > 100 )
      {
         send_to_char( "Drunk range is 0 to 100.\n\r", ch );
         return;
      }

      victim->pcdata->condition[COND_DRUNK] = value;
      return;
   }

   if( !str_cmp( arg2, "full" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPC's.\n\r", ch );
         return;
      }

      if( value < -1 || value > 100 )
      {
         send_to_char( "Full range is -1 to 100.\n\r", ch );
         return;
      }

      victim->pcdata->condition[COND_FULL] = value;
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Not on PC's.\n\r", ch );
         return;
      }

      if( arg3 == NULL || arg3[0] == '\0' )
      {
         send_to_char( "Names can not be set to an empty string.\n\r", ch );
         return;
      }

      STRFREE( victim->name );
      victim->name = STRALLOC( arg3 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
      {
         STRFREE( victim->pIndexData->player_name );
         victim->pIndexData->player_name = QUICKLINK( victim->name );
      }
      return;
   }

   if( !str_cmp( arg2, "short" ) )
   {
      STRFREE( victim->short_descr );
      victim->short_descr = STRALLOC( arg3 );
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
      {
         STRFREE( victim->pIndexData->short_descr );
         victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
      }
      return;
   }

   if( !str_cmp( arg2, "description" ) )
   {
      if( arg3[0] )
      {
         STRFREE( victim->description );
         victim->description = STRALLOC( arg3 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->description );
            victim->pIndexData->description = QUICKLINK( victim->description );
         }
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_MOB_DESC;
      ch->dest_buf = victim;
      start_editing( ch, victim->description );
      return;
   }

   if( !str_cmp( arg2, "title" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPC's.\n\r", ch );
         return;
      }

      set_title( victim, arg3 );
      return;
   }

   if( !str_cmp( arg2, "spec" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Not on PC's.\n\r", ch );
         return;
      }

      if( !str_cmp( arg3, "none" ) )
      {
         victim->spec_fun = NULL;
         send_to_char( "Special function removed.\n\r", ch );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->spec_fun = victim->spec_fun;
         return;
      }

      if( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
      {
         send_to_char( "No such spec fun.\n\r", ch );
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->spec_fun = victim->spec_fun;
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      bool pcflag;
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> flags <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         pcflag = FALSE;
         argument = one_argument( argument, arg3 );
         value = IS_NPC( victim ) ? get_actflag( arg3 ) : get_plrflag( arg3 );

         if( !IS_NPC( victim ) && ( value < 0 || value > MAX_BITS ) )
         {
            pcflag = TRUE;
            value = get_pcflag( arg3 );
         }
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
         {
            if( !IS_NPC( victim ) && value == ACT_PROTOTYPE )
               send_to_char( "You cannot change the prototype flag.\n\r", ch );
            else if( IS_NPC( victim ) && value == ACT_IS_NPC )
               send_to_char( "If that could be changed, it would cause many problems.\n\r", ch );
            else
            {
               if( pcflag )
                  TOGGLE_BIT( victim->pcdata->flags, 1 << value );
               else
               {
                  xTOGGLE_BIT( victim->act, value );
                  /*
                   * NPC check added by Gorog 
                   */
                  if( IS_NPC( victim ) && value == ACT_PROTOTYPE )
                     victim->pIndexData->act = victim->act;
               }
            }
         }
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->act = victim->act;
      return;
   }

   if( !str_cmp( arg2, "affected" ) )
   {

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> affected <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_aflag( arg3 );
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            xTOGGLE_BIT( victim->affected_by, value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->affected_by = victim->affected_by;
      return;
   }

   if( !str_cmp( arg2, "permaff" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Not on NPC's.\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_aflag( arg3 );
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            xTOGGLE_BIT( victim->pcdata->perm_aff, value );
      }
      return;
   }

   /*
    * save some more finger-leather for setting RIS stuff
    */
   if( !str_cmp( arg2, "r" ) )
   {
      sprintf( outbuf, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "i" ) )
   {
      sprintf( outbuf, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "s" ) )
   {
      sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "ri" ) )
   {
      sprintf( outbuf, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      sprintf( outbuf, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "rs" ) )
   {
      sprintf( outbuf, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "is" ) )
   {
      sprintf( outbuf, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "ris" ) )
   {
      sprintf( outbuf, "%s resistant %s", arg1, arg3 );
      do_mset( ch, outbuf );
      sprintf( outbuf, "%s immune %s", arg1, arg3 );
      do_mset( ch, outbuf );
      sprintf( outbuf, "%s susceptible %s", arg1, arg3 );
      do_mset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "resistant" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> resistant <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            TOGGLE_BIT( victim->resistant, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->resistant = victim->resistant;
      return;
   }

   if( !str_cmp( arg2, "immune" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> immune <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            TOGGLE_BIT( victim->immune, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->immune = victim->immune;
      return;
   }

   if( !str_cmp( arg2, "susceptible" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> susceptible <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            TOGGLE_BIT( victim->susceptible, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->susceptible = victim->susceptible;
      return;
   }

   if( !str_cmp( arg2, "part" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> part <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_partflag( arg3 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            TOGGLE_BIT( victim->xflags, 1 << value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->xflags = victim->xflags;
      return;
   }

   if( !str_cmp( arg2, "attack" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "You can only modify a mobile's attacks.\n\r", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> attack <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_attackflag( arg3 );
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            xTOGGLE_BIT( victim->attacks, value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->attacks = victim->attacks;
      return;
   }

   if( !str_cmp( arg2, "defense" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "You can only modify a mobile's defenses.\n\r", ch );
         return;
      }
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: mset <victim> defense <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_defenseflag( arg3 );
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            xTOGGLE_BIT( victim->defenses, value );
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->defenses = victim->defenses;
      return;
   }

   if( !str_cmp( arg2, "weight" ) )
   {
      if( value < 0 || value > 10000 )
      {
         send_to_char( "Invalid weight. Try something reasonable.\r\n", ch );
         return;
      }
      victim->weight = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->weight = victim->weight;
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "pos" ) )
   {
      if( value < 0 || value > POS_STANDING )
      {
         ch_printf( ch, "Position range is 0 to %d.\n\r", POS_STANDING );
         return;
      }
      victim->position = value;
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->position = victim->position;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   /*
    * save some finger-leather
    */
   if( !str_cmp( arg2, "hitdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      sscanf( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
      sprintf( outbuf, "%s hitnumdie %d", arg1, num );
      do_mset( ch, outbuf );

      sprintf( outbuf, "%s hitsizedie %d", arg1, size );
      do_mset( ch, outbuf );

      sprintf( outbuf, "%s hitplus %d", arg1, plus );
      do_mset( ch, outbuf );
      return;
   }
   /*
    * save some more finger-leather
    */
   if( !str_cmp( arg2, "damdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      sscanf( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
      sprintf( outbuf, "%s damnumdie %d", arg1, num );
      do_mset( ch, outbuf );
      sprintf( outbuf, "%s damsizedie %d", arg1, size );
      do_mset( ch, outbuf );
      sprintf( outbuf, "%s damplus %d", arg1, plus );
      do_mset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "hitnumdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      if( value < 0 || value > 32767 )
      {
         send_to_char( "Number of hitpoint dice range is 0 to 30000.\n\r", ch );
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitnodice = value;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "hitsizedie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      if( value < 0 || value > 32767 )
      {
         send_to_char( "Hitpoint dice size range is 0 to 30000.\n\r", ch );
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitsizedice = value;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "hitplus" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      if( value < 0 || value > 32767 )
      {
         send_to_char( "Hitpoint bonus range is 0 to 30000.\n\r", ch );
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->hitplus = value;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "damnumdie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      if( value < 0 || value > 100 )
      {
         send_to_char( "Number of damage dice range is 0 to 100.\n\r", ch );
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damnodice = value;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "damsizedie" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      if( value < 0 || value > 100 )
      {
         send_to_char( "Damage dice size range is 0 to 100.\n\r", ch );
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damsizedice = value;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "damplus" ) )
   {
      if( !IS_NPC( victim ) )
      {
         send_to_char( "Mobiles only.\n\r", ch );
         return;
      }
      if( value < 0 || value > 1000 )
      {
         send_to_char( "Damage bonus range is 0 to 1000.\n\r", ch );
         return;
      }

      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         victim->pIndexData->damplus = value;
      send_to_char( "Done.\n\r", ch );
      return;

   }


   if( !str_cmp( arg2, "aloaded" ) )
   {
      if( IS_NPC( victim ) )
      {
         send_to_char( "Player Characters only.\n\r", ch );
         return;
      }

      if( !IS_SET( victim->pcdata->area->status, AREA_LOADED ) )
      {
         SET_BIT( victim->pcdata->area->status, AREA_LOADED );
         send_to_char( "Your area set to LOADED!\n\r", victim );
         if( ch != victim )
            send_to_char( "Area set to LOADED!\n\r", ch );
         return;
      }
      else
      {
         REMOVE_BIT( victim->pcdata->area->status, AREA_LOADED );
         send_to_char( "Your area set to NOT-LOADED!\n\r", victim );
         if( ch != victim )
            send_to_char( "Area set to NON-LOADED!\n\r", ch );
         return;
      }
   }

   /*
    * Generate usage message.
    */
   if( ch->substate == SUB_REPEATCMD )
   {
      ch->substate = SUB_RESTRICTED;
      interpret( ch, origarg, FALSE );
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_mset;
   }
   else
      do_mset( ch, "" );
   return;
}


void do_oset( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char outbuf[MAX_STRING_LENGTH];
   OBJ_DATA *obj, *tmpobj;
   EXTRA_DESCR_DATA *ed;
   bool lockobj;
   char *origarg = argument;

   int value, tmp;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mob's can't oset\n\r", ch );
      return;
   }

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;

      case SUB_OBJ_EXTRA:
         if( !ch->dest_buf )
         {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_oset: sub_obj_extra: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
         }
         /*
          * hopefully the object didn't get extracted...
          * if you're REALLY paranoid, you could always go through
          * the object and index-object lists, searching through the
          * extra_descr lists for a matching pointer...
          */
         ed = ch->dest_buf;
         STRFREE( ed->description );
         ed->description = copy_buffer( ch );
         tmpobj = ch->spare_ptr;
         stop_editing( ch );
         ch->dest_buf = tmpobj;
         ch->substate = ch->tempnum;
         return;

      case SUB_OBJ_LONG:
         if( !ch->dest_buf )
         {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_oset: sub_obj_long: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
         }
         obj = ch->dest_buf;
         if( obj && obj_extracted( obj ) )
         {
            send_to_char( "Your object was extracted!\n\r", ch );
            stop_editing( ch );
            return;
         }
         STRFREE( obj->description );
         obj->description = copy_buffer( ch );
         if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         {
            STRFREE( obj->pIndexData->description );
            obj->pIndexData->description = QUICKLINK( obj->description );
         }
         tmpobj = ch->spare_ptr;
         stop_editing( ch );
         ch->substate = ch->tempnum;
         ch->dest_buf = tmpobj;
         return;
   }

   obj = NULL;
   smash_tilde( argument );

   if( ch->substate == SUB_REPEATCMD )
   {
      obj = ch->dest_buf;
      if( obj && obj_extracted( obj ) )
      {
         send_to_char( "Your object was extracted!\n\r", ch );
         obj = NULL;
         argument = "done";
      }
      if( argument[0] == '\0' || !str_cmp( argument, " " ) || !str_cmp( argument, "stat" ) )
      {
         if( obj )
            do_ostat( ch, obj->name );
         else
            send_to_char( "No object selected.  Type '?' for help.\n\r", ch );
         return;
      }
      if( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
      {
         send_to_char( "Oset mode off.\n\r", ch );
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
         if( ch->pcdata && ch->pcdata->subprompt )
         {
            STRFREE( ch->pcdata->subprompt );
            ch->pcdata->subprompt = NULL;
         }
         return;
      }
   }
   if( obj )
   {
      lockobj = TRUE;
      strcpy( arg1, obj->name );
      argument = one_argument( argument, arg2 );
      strcpy( arg3, argument );
   }
   else
   {
      lockobj = FALSE;
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, arg2 );
      strcpy( arg3, argument );
   }

   if( !str_cmp( arg1, "on" ) )
   {
      send_to_char( "Syntax: oset <object|vnum> on.\n\r", ch );
      return;
   }

   if( arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp( arg1, "?" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
      {
         if( obj )
            send_to_char( "Syntax: <field>  <value>\n\r", ch );
         else
            send_to_char( "Syntax: <object> <field>  <value>\n\r", ch );
      }
      else
         send_to_char( "Syntax: oset <object> <field>  <value>\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Field being one of:\n\r", ch );
      send_to_char( "  flags tech weight cost condition timer\n\r", ch );
      send_to_char( "  name short long ed rmed actiondesc\n\r", ch );
      send_to_char( "  type value0 value1 value2 value3 value4 value5\n\r", ch );
      send_to_char( "  affect rmaffect layers size mana\n\r", ch );
      send_to_char( "For corpses:             \n\r", ch );
 /*     send_to_char( "   skinamount\n\r", ch);
      send_to_char( "     &G1)Scraps,    2)Scrawny Pelt, 3)Scrawny Fur, 4)Scrawny Hide, 5)Rough Pelt, 6)Rough Fur\r\n", ch );
      send_to_char( "     &G7)Rough Hide, 8)Thin Pelt, 9)Thin Fur, 10)Thin Hide, 11)Pelt, 12)Fur, 13)Hide, 14)Tough Pelt\r\n", ch );
      send_to_char( "    &G15)Tough Fur, 16)Tough Hide, 17)Fine Pelt, 18)Fine Fur, 19)Fine Hide, 20)Fancy Pelt\r\n", ch );
      send_to_char( "    &G21)Fancy Fur, 22)Fancy Hide\r\n&w", ch);  */ //don't really need this yet I don't think    
      send_to_char( "For weapons:             For armor:\n\r", ch );
      send_to_char( "  weapontype               ac\n\r", ch );
      send_to_char( "For scrolls, potions and pills:\n\r", ch );
      send_to_char( "  spell1 spell2 spell3\n\r", ch );
      send_to_char( "For wands and staves:\n\r", ch );
      send_to_char( "  spell maxcharges charges\n\r", ch );
      send_to_char( "For containers:          For levers and switches:\n\r", ch );
      send_to_char( "  cflags key capacity      tflags\n\r", ch );
      return;
   }

   if( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "There is nothing like that in all the realms.\n\r", ch );
      return;
   }
   if( lockobj )
      ch->dest_buf = obj;
   else
      ch->dest_buf = NULL;

   separate_obj( obj );
   value = atoi( arg3 );

   if( !str_cmp( arg2, "on" ) )
   {
      ch_printf( ch, "Oset mode on. (Editing '%s' vnum %d).\n\r", obj->name, obj->pIndexData->vnum );
      ch->substate = SUB_REPEATCMD;
      ch->dest_buf = obj;
      if( ch->pcdata )
      {
         if( ch->pcdata->subprompt )
            STRFREE( ch->pcdata->subprompt );
         sprintf( buf, "<&COset &W#%d&w> %%i", obj->pIndexData->vnum );
         ch->pcdata->subprompt = STRALLOC( buf );
      }
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      bool proto = FALSE;

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         proto = TRUE;
      STRFREE( obj->name );
      obj->name = STRALLOC( arg3 );
      if( proto )
      {
         STRFREE( obj->pIndexData->name );
         obj->pIndexData->name = QUICKLINK( obj->name );
      }
      return;
   }

   if( !str_cmp( arg2, "short" ) )
   {
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( arg3 );
         STRFREE( obj->pIndexData->short_descr );
         obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
      }
      else
         /*
          * Feature added by Narn, Apr/96 
          * * If the item is not proto, add the word 'rename' to the keywords
          * * if it is not already there.
          */
      {
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( arg3 );
         if( str_infix( "rename", obj->name ) )
         {
            sprintf( buf, "%s %s", obj->name, "rename" );
            STRFREE( obj->name );
            obj->name = STRALLOC( buf );
         }
      }
      return;
   }

   if( !str_cmp( arg2, "long" ) )
   {
      if( arg3[0] )
      {
         if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         {
            STRFREE( obj->description );
            obj->description = STRALLOC( arg3 );
            STRFREE( obj->pIndexData->description );
            obj->pIndexData->description = QUICKLINK( obj->description );
            return;
         }
         STRFREE( obj->description );
         obj->description = STRALLOC( arg3 );
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      if( lockobj )
         ch->spare_ptr = obj;
      else
         ch->spare_ptr = NULL;
      ch->substate = SUB_OBJ_LONG;
      ch->dest_buf = obj;
      start_editing( ch, obj->description );
      return;
   }

   if( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
   {
      obj->value[0] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[0] = value;
      return;
   }

   if( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
   {
      obj->value[1] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[1] = value;
      return;
   }

   if( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
   {
      if( obj->item_type == ITEM_FURNITURE )
      {
         argument = one_argument( argument, arg3 );
         while( arg3[0] )
         {
            value = get_fur_pos( arg3 );
            if( value < 0 )
            {
               send_to_char( "stand_ sleep_ sit_ rest_ ( in at on )\n\r", ch );
               return;
            }
            TOGGLE_BIT( obj->value[2], 1 << value );
            if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            {
               TOGGLE_BIT( obj->pIndexData->value[2], 1 << value );
               obj->value[2] = obj->pIndexData->value[2];
            }
            argument = one_argument( argument, arg3 );
         }
         return;
      }
      if( obj->item_type == ITEM_VEHICLE )
      {
         argument = one_argument( argument, arg3 );
         while( arg3[0] )
         {
            value = get_vflag( arg3 );
            if( value < 0 )
            {
               send_to_char( "land surface underwater air space underground\n\r", ch );
               return;
            }
            TOGGLE_BIT( obj->value[2], 1 << value );
            if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            {
               TOGGLE_BIT( obj->pIndexData->value[2], 1 << value );
               obj->value[2] = obj->pIndexData->value[2];
            }
            argument = one_argument( argument, arg3 );
         }
         return;
      }
      obj->value[2] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         obj->pIndexData->value[2] = value;
         if( obj->item_type == ITEM_WEAPON && value != 0 )
            obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];
      }
      return;
   }

   if( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
   {
      obj->value[3] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[3] = value;
      return;
   }

   if( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
   {
      obj->value[4] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[4] = value;
      return;
   }

   if( !str_cmp( arg2, "value6" ) || !str_cmp( arg2, "v6" ) )
   {
      obj->value[6] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[6] = value;
      return;
   }

   if( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
   {
      obj->value[5] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[5] = value;
      return;
   }

   if( !str_cmp( arg2, "mana" ) )
   {
      obj->mana = value;
      return;
   }

   if( !str_cmp( arg2, "size" ) )
   {
      obj->size = value;
      return;
   }

   if( !str_cmp( arg2, "type" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: oset <object> type <type>\n\r", ch );
         return;
      }
      value = get_otype( argument );
      if( value < 1 )
      {
         ch_printf( ch, "Unknown type: %s\n\r", arg3 );
         return;
      }
      obj->item_type = ( sh_int ) value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->item_type = obj->item_type;
      return;
   }

   if( !str_cmp( arg2, "parts" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: oset <object> parts <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_bpflag( arg3 );
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
            xTOGGLE_BIT( obj->parts, value );
      }
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->parts = obj->parts;
      return;
   }


   if( !str_cmp( arg2, "flags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: oset <object> flags <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_oflag( arg3 );
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
         {
            xTOGGLE_BIT( obj->extra_flags, value );
            if( value == ITEM_PROTOTYPE )
               obj->pIndexData->extra_flags = obj->extra_flags;
         }
      }
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->extra_flags = obj->extra_flags;
      return;
   }

   if( !str_cmp( arg2, "tech" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: oset <object> tech <level>\n\r", ch );
         return;
      }

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->tech = value;
      return;
   }

   if( !str_cmp( arg2, "weight" ) )
   {
      obj->weight = value;
      obj->condition = number_fuzzy( obj->weight * 50 );
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         obj->pIndexData->weight = value;
      }
      return;
   }
/*       if( !str_cmp( arg2, "skinamount" ) )
   {
      if( value < 0 || value > 4 )
      {
          ch_printf( ch, "Skinning Amount range if %d to %d. \n\r", 0, 4 );
          obj->skinamount = 0;
          return;
      }
      obj->skinamount = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         obj->pIndexData->skinamount = value;
      }
      return;
   } */
   if( !str_cmp( arg2, "cost" ) )
   {
      obj->cost = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->cost = value;
      return;
   }

   if( !str_cmp( arg2, "layers" ) )
   {
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->layers = value;
      else
         send_to_char( "Item must have prototype flag to set this value.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "randart" ) )
   {
      make_randart( obj );
      return;
   }

   if( !str_cmp( arg2, "timer" ) )
   {
      obj->timer = value;
      return;
   }

   if( !str_cmp( arg2, "actiondesc" ) )
   {
      if( strstr( arg3, "%n" ) || strstr( arg3, "%d" ) || strstr( arg3, "%l" ) )
      {
         send_to_char( "Illegal characters!\n\r", ch );
         return;
      }
      STRFREE( obj->action_desc );
      obj->action_desc = STRALLOC( arg3 );
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         STRFREE( obj->pIndexData->action_desc );
         obj->pIndexData->action_desc = QUICKLINK( obj->action_desc );
      }
      return;
   }


   /*
    * Crash fix and name support by Shaddai 
    */
   if( !str_cmp( arg2, "affect" ) )
   {
      AFFECT_DATA *paf;
      sh_int loc;
      int bitv;

      argument = one_argument( argument, arg2 );
      if( arg2 == NULL || arg2[0] == '\0' || !argument || argument[0] == 0 )
      {
         send_to_char( "Usage: oset <object> affect <field> <value>\n\r", ch );
         return;
      }
      loc = get_atype( arg2 );
      if( loc < 1 )
      {
         ch_printf( ch, "Unknown field: %s\n\r", arg2 );
         return;
      }
      if( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
      {
         bitv = 0;
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg3 );
            if( loc == APPLY_AFFECT )
               value = get_aflag( arg3 );
            else
               value = get_risflag( arg3 );
            if( value < 0 || value > 31 )
               ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
               SET_BIT( bitv, 1 << value );
         }
         if( !bitv )
            return;
         value = bitv;
      }
      else
      {
         one_argument( argument, arg3 );
         if( loc == APPLY_WEARSPELL && !is_number( arg3 ) )
         {
            value = bsearch_skill_exact( arg3, gsn_first_spell, gsn_first_skill - 1 );
            if( value == -1 )
            {
/*		    printf("%s\n\r", arg3);	*/
               send_to_char( "Unknown spell name.\n\r", ch );
               return;
            }
         }
         else
            value = atoi( arg3 );
      }
      CREATE( paf, AFFECT_DATA, 1 );
      paf->type = -1;
      paf->duration = -1;
      paf->location = loc;
      paf->modifier = value;
      xCLEAR_BITS( paf->bitvector );
      paf->next = NULL;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         LINK( paf, obj->pIndexData->first_affect, obj->pIndexData->last_affect, next, prev );
      else
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
      ++top_affect;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "rmaffect" ) )
   {
      AFFECT_DATA *paf;
      sh_int loc, count;

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: oset <object> rmaffect <affect#>\n\r", ch );
         return;
      }
      loc = atoi( argument );
      if( loc < 1 )
      {
         send_to_char( "Invalid number.\n\r", ch );
         return;
      }

      count = 0;

      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         OBJ_INDEX_DATA *pObjIndex;

         pObjIndex = obj->pIndexData;
         for( paf = pObjIndex->first_affect; paf; paf = paf->next )
         {
            if( ++count == loc )
            {
               UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
               DISPOSE( paf );
               send_to_char( "Removed.\n\r", ch );
               --top_affect;
               return;
            }
         }
         send_to_char( "Not found.\n\r", ch );
         return;
      }
      else
      {
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( ++count == loc )
            {
               UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
               DISPOSE( paf );
               send_to_char( "Removed.\n\r", ch );
               --top_affect;
               return;
            }
         }
         send_to_char( "Not found.\n\r", ch );
         return;
      }
   }

   if( !str_cmp( arg2, "ed" ) )
   {
      if( arg3 == NULL || arg3[0] == '\0' )
      {
         send_to_char( "Syntax: oset <object> ed <keywords>\n\r", ch );
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      if( obj->timer )
      {
         send_to_char( "It's not safe to edit an extra description on an object with a timer.\n\rTurn it off first.\n\r",
                       ch );
         return;
      }
      if( obj->item_type == ITEM_PAPER )
      {
         send_to_char( "You can not add an extra description to a note paper at the moment.\n\r", ch );
         return;
      }
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         ed = SetOExtraProto( obj->pIndexData, arg3 );
      else
         ed = SetOExtra( obj, arg3 );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      if( lockobj )
         ch->spare_ptr = obj;
      else
         ch->spare_ptr = NULL;
      ch->substate = SUB_OBJ_EXTRA;
      ch->dest_buf = ed;
      start_editing( ch, ed->description );
      return;
   }

   if( !str_cmp( arg2, "rmed" ) )
   {
      if( arg3 == NULL || arg3[0] == '\0' )
      {
         send_to_char( "Syntax: oset <object> rmed <keywords>\n\r", ch );
         return;
      }
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      {
         if( DelOExtraProto( obj->pIndexData, arg3 ) )
            send_to_char( "Deleted.\n\r", ch );
         else
            send_to_char( "Not found.\n\r", ch );
         return;
      }
      if( DelOExtra( obj, arg3 ) )
         send_to_char( "Deleted.\n\r", ch );
      else
         send_to_char( "Not found.\n\r", ch );
      return;
   }
   /*
    * save some finger-leather
    */
   if( !str_cmp( arg2, "ris" ) )
   {
      sprintf( outbuf, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      sprintf( outbuf, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      sprintf( outbuf, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "r" ) )
   {
      sprintf( outbuf, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "i" ) )
   {
      sprintf( outbuf, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }
   if( !str_cmp( arg2, "s" ) )
   {
      sprintf( outbuf, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "ri" ) )
   {
      sprintf( outbuf, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      sprintf( outbuf, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "rs" ) )
   {
      sprintf( outbuf, "%s affect resistant %s", arg1, arg3 );
      do_oset( ch, outbuf );
      sprintf( outbuf, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   if( !str_cmp( arg2, "is" ) )
   {
      sprintf( outbuf, "%s affect immune %s", arg1, arg3 );
      do_oset( ch, outbuf );
      sprintf( outbuf, "%s affect susceptible %s", arg1, arg3 );
      do_oset( ch, outbuf );
      return;
   }

   /*
    * Make it easier to set special object values by name than number
    *                  -Thoric
    */
   tmp = -1;
   switch ( obj->item_type )
   {
      case ITEM_ARMOR:
         if( !str_cmp( arg2, "ac" ) )
            tmp = 1;
         break;
      case ITEM_SALVE:
         if( !str_cmp( arg2, "maxdoses" ) )
            tmp = 1;
         if( !str_cmp( arg2, "doses" ) )
            tmp = 2;
         if( !str_cmp( arg2, "delay" ) )
            tmp = 3;
         if( !str_cmp( arg2, "spell1" ) )
            tmp = 4;
         if( !str_cmp( arg2, "spell2" ) )
            tmp = 5;
         if( tmp >= 4 && tmp <= 5 )
            value = skill_lookup( arg3 );
         break;
      case ITEM_SCROLL:
      case ITEM_POTION:
      case ITEM_PILL:
         if( !str_cmp( arg2, "spell1" ) )
            tmp = 1;
         if( !str_cmp( arg2, "spell2" ) )
            tmp = 2;
         if( !str_cmp( arg2, "spell3" ) )
            tmp = 3;
         if( tmp >= 1 && tmp <= 3 )
            value = skill_lookup( arg3 );
         break;
      case ITEM_STAFF:
      case ITEM_WAND:
         if( !str_cmp( arg2, "spell" ) )
         {
            tmp = 3;
            value = skill_lookup( arg3 );
         }
         if( !str_cmp( arg2, "maxcharges" ) )
            tmp = 1;
         if( !str_cmp( arg2, "charges" ) )
            tmp = 2;
         break;
      case ITEM_CONTAINER:
         if( !str_cmp( arg2, "capacity" ) )
            tmp = 0;
         if( !str_cmp( arg2, "cflags" ) )
            tmp = 1;
         if( !str_cmp( arg2, "key" ) )
            tmp = 2;
         break;
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
      case ITEM_BUTTON:
         if( !str_cmp( arg2, "tflags" ) )
         {
            tmp = 0;
            value = get_trigflag( arg3 );
         }
         break;
   }
   if( tmp >= 0 && tmp <= 3 )
   {
      obj->value[tmp] = value;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         obj->pIndexData->value[tmp] = value;
      return;
   }

   /*
    * Generate usage message.
    */
   if( ch->substate == SUB_REPEATCMD )
   {
      ch->substate = SUB_RESTRICTED;
      interpret( ch, origarg, FALSE );
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_oset;
   }
   else
      do_oset( ch, "" );
   return;
}

/* Material shop editor -- Keolah */
void do_matshop( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   MAT_SHOP *matshop;
   MAT_SHOP *new_matshop;

   if( ch->curr_talent[TAL_CHANGE] < 102 )
   {
      send_to_char( "Nothing happens.\n\r", ch );
      return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: matshop create <vnum>\r\n", ch );
      send_to_char( "        matshop list\r\n", ch );
      send_to_char( "        matshop show <vnum>\r\n", ch );
      send_to_char( "        matshop set <vnum> <field> <value>\r\n", ch );
      send_to_char( "        matshop save\r\n", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  room mat1 mat2 mat3 mat4 mat5 \r\n", ch );
      return;
   }

   if( !strcmp( arg1, "load" ) )
   {
      send_to_char( "Loading matshops...\n\r", ch );
      load_matshops(  );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_species )
      {
         send_to_char( "No material shops have been defined.\r\n", ch );
         return;
      }
      matshop = first_matshop;
      send_to_char( "Currently defined material shops:\r\n", ch );
      while( matshop )
      {
         sprintf( buf, "%d\r\n", matshop->room );
         send_to_char( buf, ch );
         if( matshop == last_matshop )
            break;
         matshop = matshop->next;
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( matshop = find_matshop( atoi( arg2 ) ) ) )
      {
         send_to_char( "There is no material shop with that vnum.\r\n", ch );
         return;
      }
      sprintf( buf, "Material shop %d:\r\n", matshop->room );
      send_to_char( buf, ch );
      sprintf( buf, "Materials sold: %d %d %d %d %d\r\n",
               matshop->mat[1], matshop->mat[2], matshop->mat[3], matshop->mat[4], matshop->mat[0] );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
      CREATE( new_matshop, MAT_SHOP, 1 );

      if( !first_matshop )
      {
         first_matshop = new_matshop;
         last_matshop = new_matshop;
         new_matshop->next = NULL;
         new_matshop->prev = NULL;
      }
      else
      {
         last_matshop->next = new_matshop;
         new_matshop->prev = last_matshop;
         last_matshop = new_matshop;
         new_matshop->next = NULL;
      }
      matshop = new_matshop;
      if( arg2 != NULL )
         matshop->room = atoi( arg2 );
      else
         matshop->room = 0;
      matshop->mat[1] = 0;
      matshop->mat[2] = 0;
      matshop->mat[3] = 0;
      matshop->mat[4] = 0;
      matshop->mat[0] = 0;
      sprintf( buf, "matshop %d created.\r\n", matshop->room );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "set" ) )
   {
      matshop = find_matshop( atoi( arg2 ) );
      if( !matshop )
      {
         send_to_char( "Unknown material shop.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "room" ) )
      {
         matshop->room = atoi( arg4 );
         return;
      }
      if( !strcmp( arg3, "mat1" ) )
      {
         matshop->mat[1] = atoi( arg4 );
         return;
      }
      if( !strcmp( arg3, "mat2" ) )
      {
         matshop->mat[2] = atoi( arg4 );
         return;
      }
      if( !strcmp( arg3, "mat3" ) )
      {
         matshop->mat[3] = atoi( arg4 );
         return;
      }
      if( !strcmp( arg3, "mat4" ) )
      {
         matshop->mat[4] = atoi( arg4 );
         return;
      }
      if( !strcmp( arg3, "mat5" ) )
      {
         matshop->mat[0] = atoi( arg4 );
         return;
      }
   }

   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( MATSHOP_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open matshop file!", 0 );
         return;
      }

      for( matshop = first_matshop; matshop; matshop = matshop->next )
      {
         fprintf( fp, "\n#MATSHOP\n" );
         fprintf( fp, "Room   %d\n", matshop->room );
         fprintf( fp, "Mat1   %d\n", matshop->mat[1] );
         fprintf( fp, "Mat2   %d\n", matshop->mat[2] );
         fprintf( fp, "Mat3   %d\n", matshop->mat[3] );
         fprintf( fp, "Mat4   %d\n", matshop->mat[4] );
         fprintf( fp, "Mat5   %d\n", matshop->mat[0] );
         if( matshop == last_matshop )
            break;
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      send_to_char( "matshops saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}

/* Material editor -- Scion */
void do_material( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char arg5[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   MATERIAL_DATA *material;
   int i = 0;

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: material create\r\n", ch );
      send_to_char( "        material delete <number>\r\n", ch );
      send_to_char( "        material list\r\n", ch );
      send_to_char( "        material show <number>\r\n", ch );
      send_to_char( "        material find <name>\n\r", ch );
      send_to_char( "        material set <number> <field> <value>\r\n\r\n", ch );
      send_to_char( "        material save\r\n", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  name short long weight flags\r\n", ch );
      send_to_char( "  affect sector race rarity\r\n", ch );
      send_to_char( " &GHideType\r\n", ch);
      send_to_char( "     &G1)Scraps,    2)Scrawny Pelt, 3)Scrawny Fur, 4)Scrawny Hide, 5)Rough Pelt, 6)Rough Fur\r\n", ch );
      send_to_char( "     &G7)Rough Hide, 8)Thin Pelt, 9)Thin Fur, 10)Thin Hide, 11)Pelt, 12)Fur, 13)Hide, 14)Tough Pelt\r\n", ch );
      send_to_char( "    &G15)Tough Fur, 16)Tough Hide, 17)Fine Pelt, 18)Fine Fur, 19)Fine Hide, 20)Fancy Pelt\r\n", ch );
      send_to_char( "    &G21)Fancy Fur, 22)Fancy Hide\r\n&w", ch);
      send_to_char( "  magic\n\r", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {  /* Let's go for the easy one first :P */
      if( !first_material )
      {
         send_to_char( "No material types have been defined.\r\n", ch );
         return;
      }
      material = first_material;
      send_to_char( "Currently defined materials:\r\n", ch );
      while( material )
      {
         sprintf( buf, "%3d: %s\r\n", material->number, material->short_descr );
         send_to_char( buf, ch );
         material = material->next;
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      AFFECT_DATA *paf;

      if( !is_number( arg2 ) )
      {
         send_to_char( "That is not a number.\r\n", ch );
         return;
      }

      i = atoi( arg2 );
      if( !( material = material_lookup( i ) ) )
      {
         send_to_char( "There is no material with that number.\r\n", ch );
         return;
      }
      sprintf( buf, "Material #%d: %s\r\n", material->number, material->short_descr );
      send_to_char( buf, ch );
      sprintf( buf, "Namelist: %s\r\n", material->name );
      send_to_char( buf, ch );
      sprintf( buf, "Long description: %s\r\n", material->description );
      send_to_char( buf, ch );
      sprintf( buf, "Cost: %d  Weight: %d\r\n", material->cost, material->weight );
      send_to_char( buf, ch );
      sprintf( buf, "Magic absorbtion: %d per tick\n\r", material->magic );
      send_to_char( buf, ch );
      sprintf( buf, "Flags: %s\r\n", ext_flag_string( &material->extra_flags, o_flags ) );
      send_to_char( buf, ch );
      sprintf( buf, "Rarity: %d percent\r\n", material->rarity );
      send_to_char( buf, ch );
      sprintf( buf, "Sector: %d  Race: %d %s\r\n", material->sector, material->race, npc_race[material->race] );
      send_to_char( buf, ch );
      sprintf( buf, "Hidetype: %d\r\n", material->skin );
      send_to_char( buf, ch );
      for( paf = material->first_affect; paf; paf = paf->next )
      {
         sprintf( buf, "Affects %s by %d.\r\n", affect_loc_name( paf->location ), paf->modifier );
         send_to_char( buf, ch );
      }
      return;
   }

   if( !strcmp( arg1, "find" ) )
   {
      send_to_char( "Searching for materials...\n\r", ch );
      material = first_material;
      while( material )
      {
         if( !material )
            break;
         if( !str_cmp( material->name, ( char * )arg2 ) )
         {
            sprintf( buf, "Material #%d: %s\r\n", material->number, material->short_descr );
            send_to_char( buf, ch );
         }
         material = material->next;
      }
      return;
   }

   if( !strcmp( arg1, "delete" ) )
   {
      if( !is_number( arg2 ) )
      {
         send_to_char( "That is not a number.\r\n", ch );
         return;
      }
      i = atoi( arg2 );
      material = first_material;
      while( material )
      {
         if( material->number == i )
         {
            sprintf( buf, "Material \"%s\" deleted.\r\n", material->name );
            if( material == first_material )
               first_material = material->next;
            if( material == last_material )
               last_material = material->prev;
            if( material->prev )
               material->prev->next = material->next;
            if( material->next )
               material->next->prev = material->prev;
            STRFREE( material->name );
            STRFREE( material->short_descr );
            STRFREE( material->description );
            DISPOSE( material );
            send_to_char( buf, ch );
            return;
         }
         if( material )
            material = material->next;
      }
      send_to_char( "There does not appear to be such a material.\r\n", ch );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
      MATERIAL_DATA *new_material;

      CREATE( new_material, MATERIAL_DATA, 1 );

      if( !first_material )
      {  /* Ok, get this new material added into the list */
         first_material = new_material;
         last_material = new_material;
         new_material->number = 1;
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
      material->name = str_dup( "material" );
      material->short_descr = str_dup( "a piece of material" );
      material->description = str_dup( "A nondescript piece of material lies on the floor here." );
      material->weight = 1;
      material->rarity = 0;
      material->magic = 100;
      material->sector = -1;
      material->race = -1;
      material->skin = -1;
      xCLEAR_BITS( material->extra_flags );
      material->first_affect = NULL;
      sprintf( buf, "Material %d created.\r\n", material->number );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "set" ) )
   {
      if( !is_number( arg2 ) )
      {
         send_to_char( "Unknown material number.\r\n", ch );
         return;
      }
      material = material_lookup( atoi( arg2 ) );
      if( !material )
      {
         send_to_char( "Unknown material number.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "name" ) )
      {
         material->name = str_dup( arg4 );
         send_to_char( "Material name set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "short" ) )
      {
         material->short_descr = str_dup( arg4 );
         send_to_char( "Material short description set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "long" ) )
      {
         material->description = str_dup( arg4 );
         send_to_char( "Material long description set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "magic" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         material->magic = atoi( arg4 );
         send_to_char( "Material magic set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "weight" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         material->weight = atoi( arg4 );
         send_to_char( "Material weight set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "cost" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         material->cost = atoi( arg4 );
         send_to_char( "Material cost set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "flags" ) )
      {
         while( strlen( arg4 ) > 1 )
         {
            strcpy( arg4, one_argument( arg4, arg5 ) );
            i = get_oflag( arg5 );
            if( i < 0 || i > MAX_BITS )
            {
               send_to_char( "Unknown flag.\r\n", ch );
               return;
            }
            xTOGGLE_BIT( material->extra_flags, i );
         }
         send_to_char( "Flag set.\r\n", ch );
         return;
      }

      if( !strcmp( arg3, "affect" ) )
      {
         AFFECT_DATA *paf;
         sh_int loc;
         int bitv;
         int value;

         argument = one_argument( argument, arg4 );
         if( arg4 == NULL || arg4[0] == '\0' || !argument || argument[0] == 0 )
         {
            send_to_char( "Usage: material set <number> affect <field> <value>\n\r", ch );
            return;
         }
         loc = get_atype( arg4 );
         if( loc < 1 )
         {
            ch_printf( ch, "Unknown field: %s\n\r", arg4 );
            return;
         }
         if( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
         {
            bitv = 0;
            while( argument[0] != '\0' )
            {
               argument = one_argument( argument, arg5 );
               if( loc == APPLY_AFFECT )
                  value = get_aflag( arg5 );
               else
                  value = get_risflag( arg5 );
               if( value < 0 || value > 31 )
                  ch_printf( ch, "Unknown flag: %s\n\r", arg5 );
               else
                  SET_BIT( bitv, 1 << value );
            }
            if( !bitv )
               return;
            value = bitv;
         }
         else
         {
            one_argument( argument, arg5 );
            if( loc == APPLY_WEARSPELL && !is_number( arg5 ) )
            {
               value = bsearch_skill_exact( arg5, gsn_first_spell, gsn_first_skill - 1 );
               if( value == -1 )
               {
                  /*
                   * printf("%s\n\r", arg5); 
                   */
                  send_to_char( "Unknown spell name.\n\r", ch );
                  return;
               }
            }
            else
               value = atoi( arg5 );
         }
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = loc;
         paf->modifier = value;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, material->first_affect, material->last_affect, next, prev );
         send_to_char( "Done.\n\r", ch );
         return;
      }

      if( !str_cmp( arg3, "rmaffect" ) )
      {
         AFFECT_DATA *paf;
         sh_int loc, count;

         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "Usage: material set <number> rmaffect <affect#>\n\r", ch );
            return;
         }
         loc = atoi( argument );
         if( loc < 1 )
         {
            send_to_char( "Invalid number.\n\r", ch );
            return;
         }

         count = 0;

         for( paf = material->first_affect; paf; paf = paf->next )
         {
            if( ++count == loc )
            {
               UNLINK( paf, material->first_affect, material->last_affect, next, prev );
               DISPOSE( paf );
               send_to_char( "Removed.\n\r", ch );
               --top_affect;
               return;
            }
         }
         send_to_char( "Not found.\n\r", ch );
         return;
      }

      if( !strcmp( arg3, "sector" ) )
      {
         if( arg4[0] != '-' && !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         if( atoi( arg4 ) >= SECT_MAX || atoi( arg4 ) < -1 )
         {
            send_to_char( "That is not a valid sector type.\r\n", ch );
            return;
         }
         material->sector = atoi( arg4 );
         send_to_char( "Sector type set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "hidetype" ) )
      {
          if( arg4[0] != '-' && !is_number( arg4 ) )
          {
              send_to_char( "That is not a number.\r\n", ch );
              return;
          }
          if( atoi( arg4 ) >= SKIN_MAX || atoi( arg4 ) < -1 )
          {
              send_to_char( "That is not a valid skin type. \r\n", ch );
              return;
          }
          material->skin = atoi( arg4 );
          send_to_char( "Hide type set.\r\n", ch);
          return;
      }
      if( !strcmp( arg3, "race" ) )
      {
         if( arg4[0] != '-' && !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         if( atoi( arg4 ) >= MAX_NPC_RACE || atoi( arg4 ) < -1 )
         {
            send_to_char( "That is not a valid race.\r\n", ch );
            return;
         }
         material->race = atoi( arg4 );
         send_to_char( "Race type set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "rarity" ) )
      {
         if( atoi( arg4 ) < 0 || atoi( arg4 ) > 100 )
         {
            material->rarity = 0;
            send_to_char( "That is not a valid percentage.\r\n", ch );
            return;
         }
         material->rarity = atoi( arg4 );
         send_to_char( "Material rarity set.\r\n", ch );
         return;
      }
   }
   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;
      AFFECT_DATA *paf;

      if( ( fp = fopen( MATERIAL_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open material file!", 0 );
         return;
      }

      for( material = first_material; material; material = material->next )
      {
         fprintf( fp, "\n#MATERIAL\n" );
         fprintf( fp, "Name	%s~\n", material->name );
         fprintf( fp, "Short	%s~\n", material->short_descr );
         fprintf( fp, "Long	%s~\n", material->description );
         fprintf( fp, "Weight	%d\n", material->weight );
         fprintf( fp, "Magic	%d\n", material->magic );
         fprintf( fp, "Cost	%d\n", material->cost );
         fprintf( fp, "Rarity	%d\n", material->rarity );
         fprintf( fp, "Sector	%d\n", material->sector );
         fprintf( fp, "Race	%d\n", material->race );
         fprintf( fp, "Skin    %d\n", material->skin );
         fprintf( fp, "Extra	%s\n", print_bitvector( &material->extra_flags ) );
/* save affects */
         for( paf = material->first_affect; paf; paf = paf->next )
         {
            if( paf->type < 0 || paf->type >= top_sn )
            {
               fprintf( fp, "Affect       %d %d %d %d %s\n",
                        paf->type,
                        paf->duration,
                        ( ( paf->location == APPLY_WEAPONSPELL
                            || paf->location == APPLY_WEARSPELL
                            || paf->location == APPLY_REMOVESPELL
                            || paf->location == APPLY_STRIPSN
                            || paf->location == APPLY_RECURRINGSPELL )
                          && IS_VALID_SN( paf->modifier ) )
                        ? skill_table[paf->modifier]->slot : paf->modifier,
                        paf->location, print_bitvector( &paf->bitvector ) );
            }
            else
               fprintf( fp, "AffectData   '%s' %d %d %d %s\n",
                        skill_table[paf->type]->name,
                        paf->duration,
                        ( ( paf->location == APPLY_WEAPONSPELL
                            || paf->location == APPLY_WEARSPELL
                            || paf->location == APPLY_REMOVESPELL
                            || paf->location == APPLY_STRIPSN
                            || paf->location == APPLY_RECURRINGSPELL )
                          && IS_VALID_SN( paf->modifier ) )
                        ? skill_table[paf->modifier]->slot : paf->modifier,
                        paf->location, print_bitvector( &paf->bitvector ) );
         }
         /*
          * end affect saving 
          */
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      send_to_char( "Materials saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}

/* Alchemical ingredient editor -- Keolah */
void do_ingredient( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   INGRED_DATA *ingred;
   INGRED_DATA *new_ingred;
   OBJ_INDEX_DATA *pObjIndex;

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: ingred create\r\n", ch );
      send_to_char( "        ingred delete <number>\r\n", ch );
      send_to_char( "        ingred list\r\n", ch );
      send_to_char( "        ingred show <number>\r\n", ch );
      send_to_char( "        ingred set <number> <field> <value>\r\n", ch );
      send_to_char( "        ingred save\r\n", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  vnum sector rarity\r\n", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_ingred )
      {
         send_to_char( "No alchemical ingredients have been defined.\r\n", ch );
         return;
      }
      ingred = first_ingred;
      send_to_char( "Currently defined ingredients:\r\n", ch );
      if( arg2 != NULL && !str_cmp( arg2, "name" ) )
      {
         while( ingred )
         {
            sprintf( buf, "%d: %s\r\n",
                     ingred->vnum,( pObjIndex = get_obj_index( ingred->vnum ) ) ? pObjIndex->short_descr : "???" );
            send_to_char( buf, ch );
            ingred = ingred->next;
         }
      }
      else
      {
         while( ingred )
         {
            sprintf( buf, "%d\r\n", ingred->vnum );
            send_to_char( buf, ch );
            ingred = ingred->next;
         }
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( ingred = find_ingred( atoi( arg2 ) ) ) )
      {
         send_to_char( "There is no ingredient with that vnum.\r\n", ch );
         return;
      }
      sprintf( buf, "Ingredient %d:\r\n", ingred->vnum );
      send_to_char( buf, ch );
      sprintf( buf, "Sector: %d\r\n", ingred->sector );
      send_to_char( buf, ch );
      sprintf( buf, "Rarity: %d percent\r\n", ingred->rarity );
      send_to_char( buf, ch );
      sprintf( buf, "Moisture:    %d   Elevation:     %d\r\n", ingred->moisture, ingred->elevation );
      send_to_char( buf, ch );
      sprintf( buf, "Temperature: %d   Precipitation: %d\r\n", ingred->temp, ingred->precip );
      send_to_char( buf, ch );
      sprintf( buf, "Mana:        %d\n\r", ingred->mana );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
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
      if( arg2 != NULL )
         ingred->vnum = atoi( arg2 );
      else
         ingred->vnum = 1;
      ingred->sector = 0;
      ingred->mana = 0;
      ingred->moisture = 0;
      ingred->elevation = 0;
      ingred->precip = 0;
      ingred->temp = 0;
      ingred->rarity = 0;
      sprintf( buf, "ingred %d created.\r\n", ingred->vnum );
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "set" ) )
   {
      ingred = find_ingred( atoi( arg2 ) );
      if( !ingred )
      {
         send_to_char( "Unknown alchemical ingredient.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "sector" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         ingred->sector = atoi( arg4 );
         send_to_char( "ingred sector set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "vnum" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         ingred->vnum = atoi( arg4 );
         send_to_char( "ingred vnum set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "rarity" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         ingred->rarity = atoi( arg4 );
         send_to_char( "ingred rarity set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "moisture" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         ingred->moisture = atoi( arg4 );
         send_to_char( "ingred moisture set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "elevation" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         ingred->elevation = atoi( arg4 );
         send_to_char( "ingred elevation set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "mana" ) )
      {
         ingred->mana = atoi( arg4 );
         send_to_char( "ingred mana set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "precip" ) )
      {
         ingred->precip = atoi( arg4 );
         send_to_char( "ingred precip set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "temp" ) )
      {
         ingred->temp = atoi( arg4 );
         send_to_char( "ingred temp set.\r\n", ch );
         return;
      }
   }

   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( INGRED_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open ingredient file!", 0 );
         return;
      }

      for( ingred = first_ingred; ingred; ingred = ingred->next )
      {
         fprintf( fp, "\n#INGRED\n" );
         fprintf( fp, "Vnum        %d\n", ingred->vnum );
         fprintf( fp, "Sector      %d\n", ingred->sector );
         fprintf( fp, "Rarity      %d\n", ingred->rarity );
         fprintf( fp, "Mana        %d\n", ingred->mana );
         fprintf( fp, "Temp        %d\n", ingred->temp );
         fprintf( fp, "Moisture    %d\n", ingred->moisture );
         fprintf( fp, "Elevation   %d\n", ingred->elevation );
         fprintf( fp, "Precip      %d\n", ingred->precip );
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      send_to_char( "ingredients saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}


/* Alchemical recipe editor -- Keolah */
void do_recipe( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char arg5[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   RECIPE_DATA *recipe;
   RECIPE_DATA *new_recipe;
   OBJ_INDEX_DATA *pObjIndex;

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   argument = one_argument( argument, arg4 );
   strcpy( arg5, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: recipe create\r\n", ch );
      send_to_char( "        recipe delete <number>\r\n", ch );
      send_to_char( "        recipe list [name]\r\n", ch );
      send_to_char( "        recipe show <number>\r\n", ch );
      send_to_char( "        recipe set <number> <field> <value>\r\n", ch );
      send_to_char( "        recipe find <vnum>\n\r", ch );
      send_to_char( "        recipe save\r\n", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  first second result flags\r\n", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_recipe )
      {
         send_to_char( "No alchemical recipes have been defined.\r\n", ch );
         return;
      }
      recipe = first_recipe;
      send_to_char( "Currently defined recipes:\r\n", ch );
      if( arg2 != NULL && !str_cmp( arg2, "name" ) )
      {
         while( recipe )
         {
            ch_printf( ch, "%d: %s + ", recipe->number, ( pObjIndex = get_obj_index( recipe->ingred1 ) ) ? pObjIndex->short_descr : "???" );
            ch_printf( ch, "%s = ", ( pObjIndex = get_obj_index( recipe->ingred2 ) ) ? pObjIndex->short_descr : "???" );
            ch_printf( ch, "%s\r\n", ( pObjIndex = get_obj_index( recipe->result ) ) ? pObjIndex->short_descr : "???" );
            recipe = recipe->next;
         }
      }
      else
      {
         while( recipe )
         {
            sprintf( buf, "%d: %d + %d = %d\r\n", recipe->number, recipe->ingred1, recipe->ingred2, recipe->result );
            send_to_char( buf, ch );
            recipe = recipe->next;
         }
      }
      return;
   }

   if( !strcmp( arg1, "find" ) )
   {
      int i = atoi( arg2 );

      if( !first_recipe )
      {
         send_to_char( "No alchemical recipes have been defined.\n\r", ch );
         return;
      }
      recipe = first_recipe;
      send_to_char( "Searching for recipes...\r\n", ch );
      if( arg3 != NULL && !str_cmp( arg3, "name" ) )
      {
         while( recipe )
         {
            if( recipe->result == i || recipe->ingred1 == i || recipe->ingred2 == i )
            {
               ch_printf( ch, "%d: %s + ", recipe->number, ( pObjIndex = get_obj_index( recipe->ingred1 ) ) ? pObjIndex->short_descr : "???" );
               ch_printf( ch, "%s = ", ( pObjIndex = get_obj_index( recipe->ingred2 ) ) ? pObjIndex->short_descr : "???" );
               ch_printf( ch, "%s\r\n", ( pObjIndex = get_obj_index( recipe->result ) ) ? pObjIndex->short_descr : "???" );
            }
            recipe = recipe->next;
         }
      }
      else
      {
         while( recipe )
         {
            if( recipe->result == i || recipe->ingred1 == i || recipe->ingred2 == i )
            {
               sprintf( buf, "%d: %d + %d = %d\r\n", recipe->number, recipe->ingred1, recipe->ingred2, recipe->result );
               send_to_char( buf, ch );
            }
            recipe = recipe->next;
         }
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( recipe = find_recipe( atoi( arg2 ) ) ) )
      {
         send_to_char( "There is no recipe with that number.\r\n", ch );
         return;
      }
      sprintf( buf, "Recipe #%d:\r\n", recipe->number );
      send_to_char( buf, ch );
      sprintf( buf, "First ingredient: %s (%d)\r\n",
               ( pObjIndex = get_obj_index( recipe->ingred1 ) ) ? pObjIndex->short_descr : "???", recipe->ingred1 );
      send_to_char( buf, ch );
      sprintf( buf, "Second ingredient: %s (%d)\r\n",
               ( pObjIndex = get_obj_index( recipe->ingred2 ) ) ? pObjIndex->short_descr : "???", recipe->ingred2 );
      send_to_char( buf, ch );
      sprintf( buf, "Result: %s (%d)\r\n",
               ( pObjIndex = get_obj_index( recipe->result ) ) ? pObjIndex->short_descr : "???", recipe->result );
      send_to_char( buf, ch );
      sprintf( buf, "Flags: %s\r\n", flag_string( recipe->flags, recipe_flags ) );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
      CREATE( new_recipe, RECIPE_DATA, 1 );

      if( !first_recipe )
      {
         first_recipe = new_recipe;
         last_recipe = new_recipe;
         new_recipe->number = 1;
      }
      else
      {
         new_recipe->number = last_recipe->number + 1;
         last_recipe->next = new_recipe;
         new_recipe->prev = last_recipe;
         last_recipe = new_recipe;
      }
      recipe = new_recipe;
      recipe->ingred1 = 0;
      recipe->ingred2 = 0;
      recipe->result = 0;
      recipe->flags = 0;
      sprintf( buf, "recipe %d created.\r\n", recipe->number );
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "set" ) )
   {
      recipe = find_recipe( atoi( arg2 ) );
      if( !recipe )
      {
         send_to_char( "Unknown alchemical recipe.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "first" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         recipe->ingred1 = atoi( arg4 );
         send_to_char( "recipe first ingredient set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "second" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         recipe->ingred2 = atoi( arg4 );
         send_to_char( "recipe second ingredient set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "result" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         recipe->result = atoi( arg4 );
         send_to_char( "recipe result set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "flags" ) )
      {
         while( strlen( arg4 ) > 1 )
         {
            int i;

            strcpy( arg4, one_argument( arg4, arg5 ) );
            i = get_recipe_flag( arg5 );
            if( i < 0 || i > MAX_BITS )
            {
               send_to_char( "Unknown flag.\r\n", ch );
               return;
            }
            TOGGLE_BIT( recipe->flags, 1 << i );
         }
         send_to_char( "Flag set.\r\n", ch );
         return;
      }
   }

   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( RECIPE_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open recipe file!", 0 );
         return;
      }

      for( recipe = first_recipe; recipe; recipe = recipe->next )
      {
         fprintf( fp, "\n#RECIPE\n" );
         fprintf( fp, "Number     %d\n", recipe->number );
         fprintf( fp, "First      %d\n", recipe->ingred1 );
         fprintf( fp, "Second     %d\n", recipe->ingred2 );
         fprintf( fp, "Result     %d\n", recipe->result );
         fprintf( fp, "Flags      %d\n", recipe->flags );
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      send_to_char( "recipes saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}


/* Moon editor -- Keolah */
void do_moon( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   MOON_DATA *moon;
   MOON_DATA *new_moon;

   if( !IS_SET( ch->pcdata->permissions, PERMIT_ADMIN ) )
   {
      check_social( ch, "moon", argument );
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: moon create\r\n", ch );
      send_to_char( "        moon delete <name>\r\n", ch );
      send_to_char( "        moon list\r\n", ch );
      send_to_char( "        moon show <name>\r\n", ch );
      send_to_char( "        moon set <name> <field> <value>\r\n", ch );
      send_to_char( "        moon save\r\n", ch );
      send_to_char( "        moon [up|down] <name>\n\r", ch );
      send_to_char( "        moon random\n\r", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  name color world type\r\n", ch );
      send_to_char( "  phase set rise\n\r", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_moon )
      {
         send_to_char( "No moons have been defined.\r\n", ch );
         return;
      }
      moon = first_moon;
      send_to_char( "Currently defined moons:\r\n", ch );
      while( moon )
      {
         sprintf( buf, "%s\r\n", moon->name );
         send_to_char( buf, ch );
         moon = moon->next;
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( moon = find_moon( arg2 ) ) )
      {
         send_to_char( "There is no moon with that name.\r\n", ch );
         return;
      }
      sprintf( buf, "Moon %s:\r\n", moon->name );
      send_to_char( buf, ch );
      sprintf( buf, "Color: %s&w\r\n", moon->color );
      send_to_char( buf, ch );
      sprintf( buf, "World: %s\r\n", moon->world );
      send_to_char( buf, ch );
      sprintf( buf, "Rising time: %d  Setting time: %d\r\n", moon->rise, moon->set );
      send_to_char( buf, ch );
      sprintf( buf, "Current phase: %d \r\n", moon->phase );
      send_to_char( buf, ch );
      sprintf( buf, "Currently up: %d  Currently waning: %d\n\r", moon->up, moon->waning );
      send_to_char( buf, ch );
      sprintf( buf, "Type: %d\r\n", moon->type );
      send_to_char( buf, ch );
      return;
   }

/* make this work sometime -keo */
/*        if (!strcmp(arg1, "delete")) {
                moon=first_moon;
                while (moon) {
                        if (!str_cmp(moon->name, arg2)) {
                                sprintf(buf, "moon %s deleted.\r\n", moon->name);
                                if (moon==first_moon)
                                        first_moon=moon->next;
                                if (moon==last_moon)
                                        last_moon=moon->prev;
                                if (moon->prev)
                                        moon->prev->next=moon->next;
                                if (moon->next)
                                        moon->next->prev=moon->prev;
                                STRFREE(moon->name);
                                STRFREE(moon->color);
                                STRFREE(moon->world);
                                DISPOSE(moon);
                                send_to_char(buf, ch);
                                return;
                        }
                        if (moon)
                                moon=moon->next;
                }
                send_to_char("There does not appear to be such a moon.\r\n", ch);
                return;
        } */

   if( !strcmp( arg1, "create" ) )
   {
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
      if( arg2 != NULL )
         moon->name = str_dup( arg2 );
      else
         moon->name = str_dup( "newmoon" );
      moon->color = str_dup( "white" );
      moon->world = str_dup( "Nowhere" );
      moon->phase = PHASE_NEW;
      moon->rise = 16;
      moon->set = 6;
      moon->type = 0;
      moon->up = FALSE;
      moon->waning = FALSE;
      sprintf( buf, "moon %s created.\r\n", moon->name );
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "up" ) )
   {
      moon = find_moon( arg2 );
      moon->up = TRUE;
      sprintf( buf, "moon %s now in the sky.\n\r", moon->name );
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "down" ) )
   {
      moon = find_moon( arg2 );
      moon->up = FALSE;
      sprintf( buf, "moon %s no longer in the sky.\n\r", moon->name );
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "set" ) )
   {
      moon = find_moon( arg2 );
      if( !moon )
      {
         send_to_char( "Unknown moon name.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "name" ) )
      {
         moon->name = str_dup( arg4 );
         send_to_char( "moon name set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "color" ) )
      {
         moon->color = str_dup( arg4 );
         send_to_char( "moon color set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "world" ) )
      {
         moon->world = str_dup( arg4 );
         send_to_char( "moon world set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "rise" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         moon->rise = atoi( arg4 );
         send_to_char( "moon rising time set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "set" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         moon->set = atoi( arg4 );
         send_to_char( "moon setting time set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "phase" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         moon->phase = atoi( arg4 );
         send_to_char( "moon phase set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "type" ) )
      {
         int i;
         i = get_moon_flag( arg4 );
         if( i < 0 || i > MAX_BITS )
         {
            send_to_char( "Unknown flag.\r\n", ch );
            return;
         }
         moon->type = i;
         send_to_char( "moon type set.\r\n", ch );
         return;
      }
   }

   if( !strcmp( arg1, "random" ) )
   {
      moon = first_moon;
      while( moon )
      {
         moon->phase = number_range( 0, 4 );
         moon->waning = number_range( 0, 1 );
         moon->rise = number_range( 0, 23 );
         moon->set = number_range( 0, 23 );
         moon = moon->next;
      }
      send_to_char( "moons randomized\n\r", ch );
      return;
   }

   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( MOON_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open moon file!", 0 );
         return;
      }

      for( moon = first_moon; moon; moon = moon->next )
      {
         fprintf( fp, "\n#MOON\n" );
         fprintf( fp, "Name       %s~\n", moon->name );
         fprintf( fp, "Color      %s~\n", moon->color );
         fprintf( fp, "World      %s~\n", moon->world );
         fprintf( fp, "Phase      %d\n", moon->phase );
         fprintf( fp, "Set        %d\n", moon->set );
         fprintf( fp, "Rise       %d\n", moon->rise );
         fprintf( fp, "Type       %d\n", moon->type );
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      send_to_char( "moons saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}

/* Species editor -- Keolah */
void do_species( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   SPECIES_DATA *species;
   SPECIES_DATA *new_species;
   int value;

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: species create <name>\r\n", ch );
      send_to_char( "        species list\r\n", ch );
      send_to_char( "        species show <name>\r\n", ch );
      send_to_char( "        species set <name> <field> <value>\r\n", ch );
      send_to_char( "        species save\r\n", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  name skill part \r\n", ch );
      send_to_char( "  adj disadvantage \r\n", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_species )
      {
         send_to_char( "No species have been defined.\r\n", ch );
         return;
      }
      species = first_species;
      send_to_char( "Currently defined species:\r\n", ch );
      while( species )
      {
         sprintf( buf, "%s\r\n", species->name );
         send_to_char( buf, ch );
         species = species->next;
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( species = find_species( arg2 ) ) )
      {
         send_to_char( "There is no species with that name.\r\n", ch );
         return;
      }
      sprintf( buf, "Species %s:\r\n", species->name );
      send_to_char( buf, ch );
      sprintf( buf, "Adjective: %s\r\n", species->adj );
      send_to_char( buf, ch );
      sprintf( buf, "Skill set: %d\r\n", species->skill_set );
      send_to_char( buf, ch );
      send_to_char( "Body parts: ", ch );
      send_to_char( flag_string( species->parts, part_flags ), ch );
      send_to_char( "\r\n", ch );
      sprintf( buf, "Disadvantage: %d\r\n", species->disadvantage );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
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
      if( arg2 != NULL )
         species->name = str_dup( arg2 );
      else
         species->name = str_dup( "newspecies" );
      species->parts = 0;
      species->skill_set = 0;
      species->disadvantage = 0;
      species->adj = str_dup( "new" );
      sprintf( buf, "species %s created.\r\n", species->name );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "set" ) )
   {
      species = find_species( arg2 );
      if( !species )
      {
         send_to_char( "Unknown species name.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "name" ) )
      {
         species->name = str_dup( arg4 );
         send_to_char( "species name set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "adj" ) )
      {
         species->adj = str_dup( arg4 );
         send_to_char( "species adjective set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "skillset" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         species->skill_set = atoi( arg4 );
         send_to_char( "species skill set set.\r\n", ch );
         return;
      }
      if( !str_cmp( arg3, "part" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "Usage: species <species> part <flags>...\n\r", ch );
            return;
         }
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg4 );
            value = get_partflag( arg4 );
            if( value < 0 || value > 31 )
               ch_printf( ch, "Unknown flag: %s\n\r", arg4 );
            else
               TOGGLE_BIT( species->parts, 1 << value );
         }
         send_to_char( "species bodyparts set.\n\r", ch );
         return;
      }
      if( !strcmp( arg3, "disadvantage" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         species->disadvantage = atoi( arg4 );
         send_to_char( "species disadvantage set.\r\n", ch );
         return;
      }
   }

   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( SPECIES_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open species file!", 0 );
         return;
      }

      for( species = first_species; species; species = species->next )
      {
         fprintf( fp, "\n#SPECIES\n" );
         fprintf( fp, "Name       %s~\n", species->name );
         fprintf( fp, "Skillset   %d\n", species->skill_set );
         fprintf( fp, "Part       %d\n", species->parts );
         fprintf( fp, "Disadvantage	%d\n", species->disadvantage );
         fprintf( fp, "Adjective	%s~\n", species->adj );
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      send_to_char( "species saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}


/* Nation editor -- Keolah */
void do_nation( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   NATION_DATA *nation;
   NATION_DATA *new_nation;
   int value;

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: nation create <name>\r\n", ch );
      send_to_char( "        nation list\r\n", ch );
      send_to_char( "        nation show <name>\r\n", ch );
      send_to_char( "        nation set <name> <field> <value>\r\n", ch );
      send_to_char( "        nation save\r\n", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  name species affected hit mana armor\r\n", ch );
      send_to_char( "  resist suscept age weapon\r\n", ch );
      send_to_char( "  height weight metabolism npc part\r\n", ch );
      send_to_char( "  str int wil dex con per end\r\n", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_nation )
      {
         send_to_char( "No nations have been defined.\r\n", ch );
         return;
      }
      nation = first_nation;
      send_to_char( "Currently defined nations:\r\n", ch );
      while( nation )
      {
         sprintf( buf, "%s\r\n", nation->name );
         send_to_char( buf, ch );
         nation = nation->next;
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( nation = find_nation( arg2 ) ) )
      {
         send_to_char( "There is no nation with that name.\r\n", ch );
         return;
      }
      sprintf( buf, "&gNation &G%s:\r\n", nation->name );
      send_to_char( buf, ch );
      if( nation->species )
      {
         sprintf( buf, "&gSpecies: &G%s\r\n", nation->species );
         send_to_char( buf, ch );
      }
      else
      {
         send_to_char( "&gNo species set.\r\n", ch );
      }
      sprintf( buf, "&gHitpoints: &G%d   &gMana: &G%d\r\n", nation->hit, nation->mana );
      send_to_char( buf, ch );
      sprintf( buf, "&gSTR: &G%d, &gINT: &G%d, &gWIL: &G%d, &gDEX: &G%d, &gCON: &G%d, &gPER: &G%d, &gEND: &G%d\r\n",
               nation->str_mod, nation->int_mod, nation->wis_mod, nation->dex_mod,
               nation->con_mod, nation->cha_mod, nation->lck_mod );
      send_to_char( buf, ch );

      send_to_char( "&gAffected by: &G", ch );
      send_to_char( affect_bit_name( &nation->affected ), ch );
      send_to_char( "\n\r", ch );

      send_to_char( "&gResistant to: &G", ch );
      send_to_char( flag_string( nation->resist, ris_flags ), ch );
      send_to_char( "\n\r", ch );

      send_to_char( "&gSusceptible to: &G", ch );
      send_to_char( flag_string( nation->suscept, ris_flags ), ch );
      send_to_char( "\n\r", ch );

      send_to_char( "&gBody parts: &G", ch );
      send_to_char( flag_string( nation->parts, part_flags ), ch );
      send_to_char( "\r\n", ch );
      sprintf( buf, "&gUnarmed bonus: &G%d   &gArmor class: &G%d\r\n", nation->unarmed, nation->ac_plus );
      send_to_char( buf, ch );
      sprintf( buf, "&gNon-magical talent proficiency: &G%s\r\n", weapon_skill[nation->weapon] );
      send_to_char( buf, ch );
      sprintf( buf, "&gHeight: &G%d  &gMetabolism: &G%d   &gBase age: &G%d\r\n",
               nation->height, nation->metabolism, nation->base_age );
      send_to_char( buf, ch );
      sprintf( buf, "&gPrefered sectortype: &G%d   &gFlags: &G%d\r\n", nation->sector, nation->flags );
      send_to_char( buf, ch );
      ch_printf( ch, "&gEyes: &G%s\n\r&gHair: &G%s\n\r", nation->eyes, nation->hair );
      ch_printf( ch, "&gSkin - Type: &G%s\n\r&gColors: &G%s\n\r", nation->skin_type, nation->skin_color );
      ch_printf( ch, "&gExtra - Type: &G%s\n\r&gColors: &G%s\n\r", nation->extra_type, nation->extra_color );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
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
      if( arg2 != NULL )
         nation->name = str_dup( arg2 );
      else
         nation->name = str_dup( "newnation" );
      nation->hit = 0;
      nation->mana = 0;
      nation->sector = 0;
      nation->resist = 0;
      nation->suscept = 0;
      nation->unarmed = 0;
      nation->ac_plus = 0;
      nation->parts = 0;
      nation->exp_multiplier = 0;
      nation->height = 66;
      nation->metabolism = 1;
      nation->base_age = 1;
      nation->flags = 0;
      nation->str_mod = 0;
      nation->int_mod = 0;
      nation->wis_mod = 0;
      nation->dex_mod = 0;
      nation->con_mod = 0;
      nation->cha_mod = 0;
      nation->lck_mod = 0;
      sprintf( buf, "nation %s created.\r\n", nation->name );
      send_to_char( buf, ch );
      return;
   }

   if( !strcmp( arg1, "set" ) )
   {
      nation = find_nation( arg2 );
      if( !nation )
      {
         send_to_char( "Unknown nation name.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "name" ) )
      {
         nation->name = str_dup( arg4 );
         send_to_char( "nation name set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "species" ) )
      {
         nation->species = str_dup( arg4 );

         send_to_char( "nation species set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "eyes" ) )
      {
         nation->eyes = str_dup( arg4 );
         send_to_char( "nation eye colors set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "hair" ) )
      {
         nation->hair = str_dup( arg4 );
         send_to_char( "nation hair colors set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "skincolor" ) )
      {
         nation->skin_color = str_dup( arg4 );
         send_to_char( "nation skin colors set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "skintype" ) )
      {
         nation->skin_type = str_dup( arg4 );
         send_to_char( "nation skin type set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "extracolor" ) )
      {
         nation->extra_color = str_dup( arg4 );
         send_to_char( "nation extra colors set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "extratype" ) )
      {
         nation->extra_type = str_dup( arg4 );
         send_to_char( "nation extra type set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "hit" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         nation->hit = atoi( arg4 );
         send_to_char( "nation hitpoints set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "weapon" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         if( atoi( arg4 ) >= 20 )
         {
            send_to_char( "Out of range.\n\r", ch );
            return;
         }
         nation->weapon = atoi( arg4 );
         send_to_char( "nation weapon skill set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "mana" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         nation->mana = atoi( arg4 );
         send_to_char( "nation mana points set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "sector" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         nation->sector = atoi( arg4 );
         send_to_char( "nation sectortype set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "height" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         nation->height = atoi( arg4 );
         send_to_char( "nation height set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "unarmed" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         nation->unarmed = atoi( arg4 );
         send_to_char( "nation unarmed bonus set.\r\n", ch );
         return;
      }
      if( !str_cmp( arg3, "part" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "Usage: nation <nation> part <flags>...\n\r", ch );
            return;
         }
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg4 );
            value = get_partflag( arg4 );
            if( value < 0 || value > 31 )
               ch_printf( ch, "Unknown flag: %s\n\r", arg4 );
            else
               TOGGLE_BIT( nation->parts, 1 << value );
         }
         send_to_char( "nation bodyparts set\n\r", ch );
         return;
      }
      if( !strcmp( arg3, "metabolism" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         nation->metabolism = atoi( arg4 );
         send_to_char( "nation metabolism set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "age" ) )
      {
         if( !is_number( arg4 ) )
         {
            send_to_char( "That is not a number.\r\n", ch );
            return;
         }
         nation->base_age = atoi( arg4 );
         send_to_char( "nation starting age set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "armor" ) )
      {
         nation->ac_plus = atoi( arg4 );
         send_to_char( "nation armor class bonus set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "str" ) )
      {
         nation->str_mod = atoi( arg4 );
         send_to_char( "nation strength mod set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "int" ) )
      {
         nation->int_mod = atoi( arg4 );
         send_to_char( "nation intelligence mod set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "wil" ) )
      {
         nation->wis_mod = atoi( arg4 );
         send_to_char( "nation willpower mod set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "dex" ) )
      {
         nation->dex_mod = atoi( arg4 );
         send_to_char( "nation dexterity mod set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "con" ) )
      {
         nation->con_mod = atoi( arg4 );
         send_to_char( "nation constitution mod set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "per" ) )
      {
         nation->cha_mod = atoi( arg4 );
         send_to_char( "nation perception mod set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "end" ) )
      {
         nation->lck_mod = atoi( arg4 );
         send_to_char( "nation endurance mod set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "npc" ) )
      {
         if( IS_SET( nation->flags, NAT_NPC ) )
         {
            REMOVE_BIT( nation->flags, NAT_NPC );
            send_to_char( "nation is now visible to players.\r\n", ch );
         }
         else
         {
            SET_BIT( nation->flags, NAT_NPC );
            send_to_char( "nation is no longer visible to players.\r\n", ch );
         }
         return;
      }
      if( !strcmp( arg3, "newbie" ) )
      {
         if( IS_SET( nation->flags, NAT_NEWBIE ) )
         {
            REMOVE_BIT( nation->flags, NAT_NEWBIE );
            send_to_char( "nation is no longer newbie approved.\r\n", ch );
         }
         else
         {
            SET_BIT( nation->flags, NAT_NEWBIE );
            send_to_char( "nation is now newbie approved.\r\n", ch );
         }
         return;
      }
      if( !str_cmp( arg3, "affected" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "Usage: nation <nation> affected <flags>...\n\r", ch );
            return;
         }
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg4 );
            value = get_aflag( arg4 );
            if( value < 0 || value > MAX_BITS )
               ch_printf( ch, "Unknown flag: %s\n\r", arg4 );
            else
               xTOGGLE_BIT( nation->affected, value );
         }
         send_to_char( "nation affects set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "resist" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "Usage: nation <nation> resist <flags>...\n\r", ch );
            return;
         }
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg4 );
            value = get_risflag( arg4 );
            if( value < 0 || value > 31 )
               ch_printf( ch, "Unknown flag: %s\n\r", arg4 );
            else
               TOGGLE_BIT( nation->resist, 1 << value );
         }
         send_to_char( "nation resistances set.\n\r", ch );
         return;
      }

      if( !str_cmp( arg3, "suscept" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "Usage: nation <nation> suscept <flags>...\n\r", ch );
            return;
         }
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg4 );
            value = get_risflag( arg4 );
            if( value < 0 || value > 31 )
               ch_printf( ch, "Unknown flag: %s\n\r", arg4 );
            else
               TOGGLE_BIT( nation->suscept, 1 << value );
         }
         send_to_char( "nation susceptibilities set.\n\r", ch );
         return;
      }
   }
   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( NATION_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open nation file!", 0 );
         return;
      }

      for( nation = first_nation; nation; nation = nation->next )
      {
         fprintf( fp, "\n#NATION\n" );
         fprintf( fp, "Name       %s~\n", nation->name );
         fprintf( fp, "Species    %s~\n", nation->species );
         fprintf( fp, "Hit        %d\n", nation->hit );
         fprintf( fp, "Mana       %d\n", nation->mana );
         fprintf( fp, "Height     %d\n", nation->height );
         fprintf( fp, "Base_age	%d\n", nation->base_age );
         fprintf( fp, "Metabolism	%d\n", nation->metabolism );
         fprintf( fp, "AC_plus	%d\n", nation->ac_plus );
         fprintf( fp, "NPC_only	%d\n", nation->flags );
         fprintf( fp, "Unarmed	%d\n", nation->unarmed );
         fprintf( fp, "Sector	%d\n", nation->sector );
         fprintf( fp, "Resist	%d\n", nation->resist );
         fprintf( fp, "Suscept	%d\n", nation->suscept );
         fprintf( fp, "Weapon	%d\n", nation->weapon );
         fprintf( fp, "Bodyparts	%d\n", nation->parts );
         fprintf( fp, "Affect	%s\n", print_bitvector( &nation->affected ) );
         fprintf( fp, "Str_mod	%d\n", nation->str_mod );
         fprintf( fp, "Int_mod    %d\n", nation->int_mod );
         fprintf( fp, "Wis_mod    %d\n", nation->wis_mod );
         fprintf( fp, "Dex_mod    %d\n", nation->dex_mod );
         fprintf( fp, "Con_mod    %d\n", nation->con_mod );
         fprintf( fp, "Cha_mod    %d\n", nation->cha_mod );
         fprintf( fp, "Lck_mod    %d\n", nation->lck_mod );
         if( nation->eyes )
            fprintf( fp, "Eyes    %s~\n", nation->eyes );
         if( nation->hair )
            fprintf( fp, "Hair    %s~\n", nation->hair );
         if( nation->skin_color )
            fprintf( fp, "Skin_color    %s~\n", nation->skin_color );
         if( nation->skin_type )
            fprintf( fp, "Skin_type    %s~\n", nation->skin_type );
         if( nation->extra_color )
            fprintf( fp, "Extra_color    %s~\n", nation->extra_color );
         if( nation->extra_type )
            fprintf( fp, "Extra_type    %s~\n", nation->extra_type );
      }
      fprintf( fp, "#END\n" );

      fclose( fp );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}


void do_races( CHAR_DATA * ch, char *argument )
{
   int i;
   NATION_DATA *nation;
   SPECIES_DATA *species;
   char buf[MAX_STRING_LENGTH];

   if( !first_nation )
   {
      send_to_char( "No nations have been defined.\r\n", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      i = 0;
      species = first_species;
      send_to_char( "These are the species in existance:\r\n", ch );
      while( species )
      {
         ch_printf( ch, "%s\n\r", species->name );
         species = species->next;
      }
   }
   else if( !str_cmp( argument, "count" ) )
   {
      nation = first_nation;
      i = 0;
      while( nation )
      {
         i++;
         nation = nation->next;
      }
      ch_printf( ch, "&cThere are &C%d &craces in existance.\n\r", i );
   }
   else
   {
      nation = first_nation;
      ch_printf( ch, "&gNations of species &G%s &gavailable:\r\n", argument );
      while( nation )
      {
         if( !str_cmp( nation->species, argument )
             && ( !IS_SET( nation->flags, NAT_NPC ) || IS_SET( ch->pcdata->permissions, PERMIT_MISC ) ) )
         {
            sprintf( buf, "&G%s%s\r\n", nation->name, IS_SET( nation->flags, NAT_NPC ) ? " &z(hidden)" : "" );
            send_to_char( buf, ch );
         }
         nation = nation->next;
      }
   }
}

void do_rfind( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *room;
   int hash;
   bool found;

   found = FALSE;
   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
   {
      for( room = room_index_hash[hash]; room; room = room->next )
      {
         if( nifty_is_name( argument, room->name ) )
         {
            ch_printf( ch, "[%d] %s in %s\n\r", room->vnum, room->name, room->area->name );
            found = TRUE;
         }
      }
   }
   if( !found )
      send_to_char( "No matching rooms found.\n\r", ch );
}


/* Mutation editor -- Keolah */
void do_mutation( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   MUT_DATA *mut;
   MUT_DATA *new_mut;

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   strcpy( arg4, argument );

   send_to_char( "&w", ch );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: mut create\r\n", ch );
      send_to_char( "        mut delete <number>\r\n", ch );
      send_to_char( "        mut list\r\n", ch );
      send_to_char( "        mut show <number>\r\n", ch );
      send_to_char( "        mut set <number> <field> <value>\r\n", ch );
      send_to_char( "        mut save\r\n", ch );
      send_to_char( "Where <field> is one of:\r\n", ch );
      send_to_char( "  desc gain lose rarity\r\n", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_mutation )
      {
         send_to_char( "No mutations have been defined.\r\n", ch );
         return;
      }
      mut = first_mutation;
      send_to_char( "Currently defined mutations:\r\n", ch );
      while( mut )
      {
         sprintf( buf, "%d: %s\r\n", mut->number, mut->desc );
         send_to_char( buf, ch );
         mut = mut->next;
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( mut = find_mutation( atoi( arg2 ) ) ) )
      {
         send_to_char( "There is no mutation with that number.\r\n", ch );
         return;
      }
      sprintf( buf, "Mutation #%d:\r\n", mut->number );
      send_to_char( buf, ch );
      ch_printf( ch, "Desc: %s\n\r", mut->desc );
      ch_printf( ch, "GainSelf: %s\n\r", mut->gain_self );
      ch_printf( ch, "GainOthers: %s\n\r", mut->gain_other );
      ch_printf( ch, "LoseSelf: %s\n\r", mut->lose_self );
      ch_printf( ch, "LoseOthers: %s\n\r", mut->lose_other );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
      CREATE( new_mut, MUT_DATA, 1 );

      if( !first_mutation )
      {
         first_mutation = new_mut;
         last_mutation = new_mut;
         new_mut->number = 1;
      }
      else
      {
         new_mut->number = last_mutation->number + 1;
         last_mutation->next = new_mut;
         new_mut->prev = last_mutation;
         last_mutation = new_mut;
      }
      mut = new_mut;
      sprintf( buf, "mutation %d created.\r\n", mut->number );
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "set" ) )
   {
      mut = find_mutation( atoi( arg2 ) );
      if( !mut )
      {
         send_to_char( "Unknown mutation.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "desc" ) )
      {
         STRFREE( mut->desc );
         arg4[0] = UPPER( arg4[0] );
         mut->desc = STRALLOC( arg4 );
         return;
      }
      if( !strcmp( arg3, "gainself" ) )
      {
         STRFREE( mut->gain_self );
         arg4[0] = UPPER( arg4[0] );
         mut->gain_self = STRALLOC( arg4 );
         return;
      }
      if( !strcmp( arg3, "gainother" ) )
      {
         STRFREE( mut->gain_other );
         arg4[0] = UPPER( arg4[0] );
         mut->gain_other = STRALLOC( arg4 );
         return;
      }
      if( !strcmp( arg3, "loseself" ) )
      {
         STRFREE( mut->lose_self );
         arg4[0] = UPPER( arg4[0] );
         mut->lose_self = STRALLOC( arg4 );
         return;
      }
      if( !strcmp( arg3, "loseother" ) )
      {
         STRFREE( mut->lose_other );
         arg4[0] = UPPER( arg4[0] );
         mut->lose_other = STRALLOC( arg4 );
         return;
      }
   }

   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( MUT_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open mutation file!", 0 );
         return;
      }

      for( mut = first_mutation; mut; mut = mut->next )
      {
         fprintf( fp, "\n#MUTATION\n" );
         fprintf( fp, "Number     %d\n", mut->number );
         fprintf( fp, "Desc       %s~\n", mut->desc );
         fprintf( fp, "GainSelf	%s~\n", mut->gain_self );
         fprintf( fp, "GainOther	%s~\n", mut->gain_other );
         fprintf( fp, "LoseSelf	%s~\n", mut->lose_self );
         fprintf( fp, "LoseOther	%s~\n", mut->lose_other );
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      send_to_char( "mutations saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}



/* Pre-make rooms in an area, by Triskal */
void do_makerooms( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   AREA_DATA *pArea;
   int max = 0;
   sh_int vnum;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( ch->curr_talent[TAL_VOID] < 103 )
   {
      send_to_char( "Nothing happens.\n\r", ch );
      return;
   }

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Make rooms using what vnums?\n\r", ch );
      return;
   }

   vnum = atoi( arg1 );
   max = atoi( arg2 );

   if( ( location = find_location( ch, arg1 ) ) == NULL )
   {
      if( vnum < 0 || get_room_index( vnum ) )
      {
         send_to_char( "You cannot find that...\n\r", ch );
         return;
      }
   }


   if( !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) || vnum < 1 || IS_NPC( ch ) || !ch->pcdata->area )
   {
      send_to_char( "No such location.\n\r", ch );
      return;
   }


   if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
   {
      send_to_char( "You must have an assigned area to create rooms.\n\r", ch );
      return;
   }

   max = atoi( arg2 );

   while( vnum <= max )
   {
      if( vnum < pArea->low_r_vnum || vnum > pArea->hi_r_vnum )
      {
         send_to_char( "That room is not within your assigned range.\n\r", ch );
         return;
      }

      location = make_room( vnum );
      vnum += 1;

      if( !location )
      {
         bug( "Makerooms: make_room failed", 0 );
         return;
      }
      location->area = ch->pcdata->area;
   }


   set_char_color( AT_WHITE, ch );
   send_to_char( "You snap your fingers and the world unfolds before you..\n\r", ch );
   return;
}


/*
 * Obsolete Merc room editing routine
 */
void do_rset( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   int value;
   bool proto;

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   strcpy( arg3, argument );

   if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
   {
      send_to_char( "Syntax: rset <location> <field> value\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Field being one of:\n\r", ch );
      send_to_char( "  flags sector\n\r", ch );
      return;
   }

   if( ( location = find_location( ch, arg1 ) ) == NULL )
   {
      send_to_char( "No such location.\n\r", ch );
      return;
   }


   if( !is_number( arg3 ) )
   {
      send_to_char( "Value must be numeric.\n\r", ch );
      return;
   }
   value = atoi( arg3 );

   /*
    * Set something.
    */
   if( !str_cmp( arg2, "flags" ) )
   {
      /*
       * Protect from messing up prototype flag
       */
      if( IS_SET( location->room_flags, ROOM_PROTOTYPE ) )
         proto = TRUE;
      else
         proto = FALSE;
      location->room_flags = value;
      if( proto )
         SET_BIT( location->room_flags, ROOM_PROTOTYPE );
      return;
   }

   if( !str_cmp( arg2, "sector" ) )
   {
      location->sector_type = value;
      return;
   }

   /*
    * Generate usage message.
    */
   do_rset( ch, "" );
   return;
}

/*
 * Returns value 0 - 9 based on directional text.
 */
int get_dir( char *txt )
{
   int edir;
   char c1, c2;

   if( !str_cmp( txt, "northeast" ) )
      return DIR_NORTHEAST;
   if( !str_cmp( txt, "northwest" ) )
      return DIR_NORTHWEST;
   if( !str_cmp( txt, "southeast" ) )
      return DIR_SOUTHEAST;
   if( !str_cmp( txt, "southwest" ) )
      return DIR_SOUTHWEST;
   if( !str_cmp( txt, "somewhere" ) )
      return 10;

   c1 = txt[0];
   if( c1 == '\0' )
      return 0;
   c2 = txt[1];
   edir = 0;
   switch ( c1 )
   {
      case 'n':
         switch ( c2 )
         {
            default:
               edir = 0;
               break;   /* north */
            case 'e':
               edir = 6;
               break;   /* ne   */
            case 'w':
               edir = 7;
               break;   /* nw   */
         }
         break;
      case '0':
         edir = 0;
         break;   /* north */
      case 'e':
      case '1':
         edir = 1;
         break;   /* east  */
      case 's':
         switch ( c2 )
         {
            default:
               edir = 2;
               break;   /* south */
            case 'e':
               edir = 8;
               break;   /* se   */
            case 'w':
               edir = 9;
               break;   /* sw   */
         }
         break;
      case '2':
         edir = 2;
         break;   /* south */
      case 'w':
      case '3':
         edir = 3;
         break;   /* west  */
      case 'u':
      case '4':
         edir = 4;
         break;   /* up    */
      case 'd':
      case '5':
         edir = 5;
         break;   /* down  */
      case '6':
         edir = 6;
         break;   /* ne   */
      case '7':
         edir = 7;
         break;   /* nw   */
      case '8':
         edir = 8;
         break;   /* se   */
      case '9':
         edir = 9;
         break;   /* sw   */
      case '?':
         edir = 10;
         break;   /* somewhere */
   }
   return edir;
}

void do_redit( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA *location, *tmp;
   EXTRA_DESCR_DATA *ed;
   EXIT_DATA *xit, *texit;
   int value;
   int edir, ekey, evnum;
   char *origarg = argument;

   set_char_color( AT_PLAIN, ch );
   if( !ch->desc )
   {
      send_to_char( "You have no descriptor.\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_ROOM_DESC:
         location = ch->dest_buf;
         if( !location )
         {
            bug( "redit: sub_room_desc: NULL ch->dest_buf", 0 );
            location = ch->in_room;
         }
         STRFREE( location->description );
         location->description = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         return;
      case SUB_ROOM_EXTRA:
         ed = ch->dest_buf;
         if( !ed )
         {
            bug( "redit: sub_room_extra: NULL ch->dest_buf", 0 );
            stop_editing( ch );
            return;
         }
         STRFREE( ed->description );
         ed->description = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         return;
   }

   location = ch->in_room;

   smash_tilde( argument );
   argument = one_argument( argument, arg );
   if( ch->substate == SUB_REPEATCMD )
   {
      if( arg[0] == '\0' )
      {
         do_rstat( ch, "" );
         return;
      }
      if( !str_cmp( arg, "done" ) || !str_cmp( arg, "off" ) )
      {
         send_to_char( "Redit mode off.\n\r", ch );
         if( ch->pcdata && ch->pcdata->subprompt )
         {
            STRFREE( ch->pcdata->subprompt );
            ch->pcdata->subprompt = NULL;
         }
         ch->substate = SUB_NONE;
         return;
      }
   }
   if( arg[0] == '\0' || !str_cmp( arg, "?" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
         send_to_char( "Syntax: <field> value\n\r", ch );
      else
         send_to_char( "Syntax: redit <field> value\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Field being one of:\n\r", ch );
      send_to_char( "  name desc ed rmed\n\r", ch );
      send_to_char( "  exit bexit exdesc exflags exname exkey\n\r", ch );
      send_to_char( "  flags sector teledelay televnum runes\n\r", ch );
      send_to_char( "  rlist exdistance pulltype pull push\n\r", ch );
      send_to_char( "  vegetation water elevation resources\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "name" ) )
   {
      if( argument[0] == '\0' )
      {
         send_to_char( "Set the room name.  A very brief single line room description.\n\r", ch );
         send_to_char( "Usage: redit name <Room summary>\n\r", ch );
         return;
      }
      STRFREE( location->name );
      location->name = STRALLOC( argument );
      return;
   }

   if( !str_cmp( arg, "desc" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_ROOM_DESC;
      ch->dest_buf = location;
      start_editing( ch, location->description );
      return;
   }

   /*
    * Crash fix and name support by Shaddai 
    */
   if( !str_cmp( arg, "affect" ) )
   {
      AFFECT_DATA *paf;
      sh_int loc;
      int bitv;

      argument = one_argument( argument, arg2 );
      if( arg2 == NULL || arg2[0] == '\0' || !argument || argument[0] == 0 )
      {
         send_to_char( "Usage: redit affect <field> <value>\n\r", ch );
         return;
      }
      loc = get_atype( arg2 );
      if( loc < 1 )
      {
         ch_printf( ch, "Unknown field: %s\n\r", arg2 );
         return;
      }
      if( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
      {
         bitv = 0;
         while( argument[0] != '\0' )
         {
            argument = one_argument( argument, arg3 );
            if( loc == APPLY_AFFECT )
               value = get_aflag( arg3 );
            else
               value = get_risflag( arg3 );
            if( value < 0 || value > 31 )
               ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
               SET_BIT( bitv, 1 << value );
         }
         if( !bitv )
            return;
         value = bitv;
      }
      else
      {
         one_argument( argument, arg3 );
         if( loc == APPLY_WEARSPELL && !is_number( arg3 ) )
         {
            value = bsearch_skill_exact( arg3, gsn_first_spell, gsn_first_skill - 1 );
            if( value == -1 )
            {
/*		    printf("%s\n\r", arg3);	*/
               send_to_char( "Unknown spell name.\n\r", ch );
               return;
            }
         }
         else
            value = atoi( arg3 );
      }
      CREATE( paf, AFFECT_DATA, 1 );
      paf->type = -1;
      paf->duration = -1;
      paf->location = loc;
      paf->modifier = value;
      xCLEAR_BITS( paf->bitvector );
      paf->next = NULL;
      LINK( paf, location->first_affect, location->last_affect, next, prev );
      ++top_affect;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "rmaffect" ) )
   {
      AFFECT_DATA *paf;
      sh_int loc, count;

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: redit rmaffect <affect#>\n\r", ch );
         return;
      }
      loc = atoi( argument );
      if( loc < 1 )
      {
         send_to_char( "Invalid number.\n\r", ch );
         return;
      }

      count = 0;

      for( paf = location->first_affect; paf; paf = paf->next )
      {
         if( ++count == loc )
         {
            UNLINK( paf, location->first_affect, location->last_affect, next, prev );
            DISPOSE( paf );
            send_to_char( "Removed.\n\r", ch );
            --top_affect;
            return;
         }
      }
      send_to_char( "Not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "ed" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Create an extra description.\n\r", ch );
         send_to_char( "You must supply keyword(s).\n\r", ch );
         return;
      }
      CHECK_SUBRESTRICTED( ch );
      ed = SetRExtra( location, argument );
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_ROOM_EXTRA;
      ch->dest_buf = ed;
      start_editing( ch, ed->description );
      return;
   }

   if( !str_cmp( arg, "rmed" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Remove an extra description.\n\r", ch );
         send_to_char( "You must supply keyword(s).\n\r", ch );
         return;
      }
      if( DelRExtra( location, argument ) )
         send_to_char( "Deleted.\n\r", ch );
      else
         send_to_char( "Not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "rlist" ) )
   {
      RESET_DATA *pReset;
      char *rbuf;
      short num;

      if( !location->first_reset )
      {
         send_to_char( "This room has no resets to list.\n\r", ch );
         return;
      }
      num = 0;
      for( pReset = location->first_reset; pReset; pReset = pReset->next )
      {
         num++;
         if( !( rbuf = sprint_reset( pReset, &num ) ) )
            continue;
         send_to_char( rbuf, ch );
      }
      return;
   }

   if( !str_cmp( arg, "flags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Toggle the room flags.\n\r", ch );
         send_to_char( "Usage: redit flags <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_rflag( arg2 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
         else
         {
            TOGGLE_BIT( location->room_flags, 1 << value );
         }
      }
      return;
   }

   if( !str_cmp( arg, "runes" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Toggle the rune flags.\n\r", ch );
         send_to_char( "Usage: redit runes <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_rflag( arg2 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown rune: %s\n\r", arg2 );
         else
         {
            TOGGLE_BIT( location->runes, 1 << value );
         }
      }
      return;
   }

   if( !str_cmp( arg, "teledelay" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Set the delay of the teleport. (0 = off).\n\r", ch );
         send_to_char( "Usage: redit teledelay <value>\n\r", ch );
         return;
      }
      location->tele_delay = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "televnum" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Set the vnum of the room to teleport to.\n\r", ch );
         send_to_char( "Usage: redit televnum <vnum>\n\r", ch );
         return;
      }
      location->tele_vnum = atoi( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "sector" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Set the sector type.\n\r", ch );
         send_to_char( "Usage: redit sector <value>\n\r", ch );
         return;
      }
      location->sector_type = atoi( argument );
      if( location->sector_type < 0 || location->sector_type >= SECT_MAX )
      {
         location->sector_type = 1;
         send_to_char( "Out of range\n\r.", ch );
      }
      else
         send_to_char( "Done.\n\r", ch );
      return;
   }

   /*
    * advanced sector types -- Scion 
    */
   if( !str_cmp( arg, "vegetation" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Set the vegetation level.\r\n", ch );
         send_to_char( "Usage: redit vegetation <value>\r\n", ch );
         return;
      }

      location->vegetation = atoi( argument );
      location->curr_vegetation = location->vegetation;
      if( location->vegetation < -100 || location->vegetation > 100 )
      {
         location->vegetation = 0;
         location->curr_vegetation = 0;
         send_to_char( "Out of range.\r\n", ch );
      }
      else
         send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * advanced sector types -- Scion 
    */
   if( !str_cmp( arg, "water" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Set the water level.\r\n", ch );
         send_to_char( "Usage: redit water <value>\r\n", ch );
         return;
      }

      location->water = atoi( argument );
      location->curr_water = location->water;
      if( location->water < -100 || location->water > 100 )
      {
         location->water = 0;
         location->curr_water = 0;
         send_to_char( "Out of range.\r\n", ch );
      }
      else
         send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * advanced sector types -- Scion 
    */
   if( !str_cmp( arg, "elevation" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Set the elevation level.\r\n", ch );
         send_to_char( "Usage: redit elevation <value>\r\n", ch );
         return;
      }

      location->elevation = atoi( argument );
      location->curr_elevation = location->elevation;
      if( location->elevation < -100 || location->elevation > 100 )
      {
         location->elevation = 0;
         location->curr_elevation = 0;
         send_to_char( "Out of range.\r\n", ch );
      }
      else
         send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * advanced sector types -- Scion 
    */
   if( !str_cmp( arg, "resources" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Set the resource level.\r\n", ch );
         send_to_char( "Usage: redit resources <value>\r\n", ch );
         return;
      }

      location->resources = atoi( argument );
      location->curr_resources = location->resources;
      if( location->resources < -100 || location->resources > 100 )
      {
         location->resources = 0;
         location->curr_resources = 0;
         send_to_char( "Out of range.\r\n", ch );
      }
      else
         send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "exkey" ) )
   {
      argument = one_argument( argument, arg2 );
      argument = one_argument( argument, arg3 );
      if( arg2[0] == '\0' || arg3[0] == '\0' )
      {
         send_to_char( "Usage: redit exkey <dir> <key vnum>\n\r", ch );
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      value = atoi( arg3 );
      if( !xit )
      {
         send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
         return;
      }
      xit->key = value;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "exname" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '\0' )
      {
         send_to_char( "Change or clear exit keywords.\n\r", ch );
         send_to_char( "Usage: redit exname <dir> [keywords]\n\r", ch );
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( !xit )
      {
         send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
         return;
      }
      STRFREE( xit->keyword );
      xit->keyword = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "exflags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Toggle or display exit flags.\n\r", ch );
         send_to_char( "Usage: redit exflags <dir> <flag> [flag]...\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg2 );
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( !xit )
      {
         send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
         return;
      }
      if( argument[0] == '\0' )
      {
         sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\n\r[ ", xit->vdir, xit->keyword, xit->key );
         for( value = 0; value <= MAX_EXFLAG; value++ )
         {
            if( IS_SET( xit->exit_info, 1 << value ) )
            {
               strcat( buf, ex_flags[value] );
               strcat( buf, " " );
            }
         }
         strcat( buf, "]\n\r" );
         send_to_char( buf, ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         value = get_exflag( arg2 );
         if( value < 0 || value > MAX_EXFLAG )
            ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
         else
            TOGGLE_BIT( xit->exit_info, 1 << value );
      }
      return;
   }




   if( !str_cmp( arg, "exit" ) )
   {
      bool addexit, numnotdir;

      argument = one_argument( argument, arg2 );
      argument = one_argument( argument, arg3 );
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "Create, change or remove an exit.\n\r", ch );
         send_to_char( "Usage: redit exit <dir> [room] [flags] [key] [keywords]\n\r", ch );
         return;
      }
      addexit = numnotdir = FALSE;
      switch ( arg2[0] )
      {
         default:
            edir = get_dir( arg2 );
            break;
         case '+':
            edir = get_dir( arg2 + 1 );
            addexit = TRUE;
            break;
         case '#':
            edir = atoi( arg2 + 1 );
            numnotdir = TRUE;
            break;
      }
      if( arg3 == NULL || arg3[0] == '\0' )
         evnum = 0;
      else
         evnum = atoi( arg3 );
      if( numnotdir )
      {
         if( ( xit = get_exit_num( location, edir ) ) != NULL )
            edir = xit->vdir;
      }
      else
         xit = get_exit( location, edir );
      if( !evnum )
      {
         if( xit )
         {
            extract_exit( location, xit );
            send_to_char( "Exit removed.\n\r", ch );
            return;
         }
         send_to_char( "No exit in that direction.\n\r", ch );
         return;
      }
      if( evnum < 1 || evnum > MAX_VNUM )
      {
         send_to_char( "Invalid room number.\n\r", ch );
         return;
      }
      if( ( tmp = get_room_index( evnum ) ) == NULL )
      {
         send_to_char( "Non-existant room.\n\r", ch );
         return;
      }
      if( addexit || !xit )
      {
         if( numnotdir )
         {
            send_to_char( "Cannot add an exit by number, sorry.\n\r", ch );
            return;
         }
         if( addexit && xit && get_exit_to( location, edir, tmp->vnum ) )
         {
            send_to_char( "There is already an exit in that direction leading to that location.\n\r", ch );
            return;
         }
         xit = make_exit( location, tmp, edir );
         xit->keyword = STRALLOC( "" );
         xit->description = STRALLOC( "" );
         xit->key = -1;
         xit->exit_info = 0;
         act( AT_GREEN, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
      }
      else
         act( AT_GREEN, "Something is different...", ch, NULL, NULL, TO_ROOM );
      if( xit->to_room != tmp )
      {
         xit->to_room = tmp;
         xit->vnum = evnum;
         texit = get_exit_to( xit->to_room, rev_dir[edir], location->vnum );
         if( texit )
         {
            texit->rexit = xit;
            xit->rexit = texit;
         }
      }
      argument = one_argument( argument, arg3 );
      if( arg3 != NULL && arg3[0] != '\0' )
         xit->exit_info = atoi( arg3 );
      if( argument && argument[0] != '\0' )
      {
         one_argument( argument, arg3 );
         ekey = atoi( arg3 );
         if( ekey != 0 || arg3[0] == '0' )
         {
            argument = one_argument( argument, arg3 );
            xit->key = ekey;
         }
         if( argument && argument[0] != '\0' )
         {
            STRFREE( xit->keyword );
            xit->keyword = STRALLOC( argument );
         }
      }
      send_to_char( "Done.\n\r", ch );
      return;
   }

   /*
    * Twisted and evil, but works           -Thoric
    * Makes an exit, and the reverse in one shot.
    */
   if( !str_cmp( arg, "bexit" ) )
   {
      EXIT_DATA *xit, *rxit;
      char tmpcmd[MAX_INPUT_LENGTH];
      ROOM_INDEX_DATA *tmploc;
      int vnum, exnum;
      char rvnum[MAX_INPUT_LENGTH];
      bool numnotdir;

      argument = one_argument( argument, arg2 );
      argument = one_argument( argument, arg3 );
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "Create, change or remove a two-way exit.\n\r", ch );
         send_to_char( "Usage: redit bexit <dir> [room] [flags] [key] [keywords]\n\r", ch );
         return;
      }
      numnotdir = FALSE;
      switch ( arg2[0] )
      {
         default:
            edir = get_dir( arg2 );
            break;
         case '#':
            numnotdir = TRUE;
            edir = atoi( arg2 + 1 );
            break;
         case '+':
            edir = get_dir( arg2 + 1 );
            break;
      }
      tmploc = location;
      exnum = edir;
      if( numnotdir )
      {
         if( ( xit = get_exit_num( tmploc, edir ) ) != NULL )
            edir = xit->vdir;
      }
      else
         xit = get_exit( tmploc, edir );
      rxit = NULL;
      vnum = 0;
      rvnum[0] = '\0';
      if( xit )
      {
         vnum = xit->vnum;
         if( arg3[0] != '\0' )
            sprintf( rvnum, "%d", tmploc->vnum );
         if( xit->to_room )
            rxit = get_exit( xit->to_room, rev_dir[edir] );
         else
            rxit = NULL;
      }
      sprintf( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
      do_redit( ch, tmpcmd );
      if( numnotdir )
         xit = get_exit_num( tmploc, exnum );
      else
         xit = get_exit( tmploc, edir );
      if( !rxit && xit )
      {
         vnum = xit->vnum;
         if( arg3[0] != '\0' )
            sprintf( rvnum, "%d", tmploc->vnum );
         if( xit->to_room )
            rxit = get_exit( xit->to_room, rev_dir[edir] );
         else
            rxit = NULL;
      }
      if( vnum )
      {
         sprintf( tmpcmd, "%d redit exit %d %s %s", vnum, rev_dir[edir], rvnum, argument );
         do_at( ch, tmpcmd );
      }
      return;
   }

   if( !str_cmp( arg, "pulltype" ) || !str_cmp( arg, "pushtype" ) )
   {
      int pt;

      argument = one_argument( argument, arg2 );
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         ch_printf( ch, "Set the %s between this room, and the destination room.\n\r", arg );
         ch_printf( ch, "Usage: redit %s <dir> <type>\n\r", arg );
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         if( ( pt = get_pulltype( argument ) ) == -1 )
            ch_printf( ch, "Unknown pulltype: %s.  (See help PULLTYPES)\n\r", argument );
         else
         {
            xit->pulltype = pt;
            send_to_char( "Done.\n\r", ch );
            return;
         }
      }
      send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "pull" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "Set the 'pull' between this room, and the destination room.\n\r", ch );
         send_to_char( "Usage: redit pull <dir> <force (0 to 100)>\n\r", ch );
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         xit->pull = URANGE( -100, atoi( argument ), 100 );
         send_to_char( "Done.\n\r", ch );
         return;
      }
      send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "push" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "Set the 'push' away from the destination room in the opposite direction.\n\r", ch );
         send_to_char( "Usage: redit push <dir> <force (0 to 100)>\n\r", ch );
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         xit->pull = URANGE( -100, -( atoi( argument ) ), 100 );
         send_to_char( "Done.\n\r", ch );
         return;
      }
      send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "exdistance" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "Set the distance (in rooms) between this room, and the destination room.\n\r", ch );
         send_to_char( "Usage: redit exdistance <dir> [distance]\n\r", ch );
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         xit->distance = URANGE( 1, atoi( argument ), 50 );
         send_to_char( "Done.\n\r", ch );
         return;
      }
      send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "exdesc" ) )
   {
      argument = one_argument( argument, arg2 );
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "Create or clear a description for an exit.\n\r", ch );
         send_to_char( "Usage: redit exdesc <dir> [description]\n\r", ch );
         return;
      }
      if( arg2[0] == '#' )
      {
         edir = atoi( arg2 + 1 );
         xit = get_exit_num( location, edir );
      }
      else
      {
         edir = get_dir( arg2 );
         xit = get_exit( location, edir );
      }
      if( xit )
      {
         STRFREE( xit->description );
         if( !argument || argument[0] == '\0' )
            xit->description = STRALLOC( "" );
         else
         {
            sprintf( buf, "%s\n\r", argument );
            xit->description = STRALLOC( buf );
         }
         send_to_char( "Done.\n\r", ch );
         return;
      }
      send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
      return;
   }

   /*
    * Generate usage message.
    */
   if( ch->substate == SUB_REPEATCMD )
   {
      ch->substate = SUB_RESTRICTED;
      interpret( ch, origarg, FALSE );
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_redit;
   }
   else
      do_redit( ch, "" );
   return;
}

void do_ocreate( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int vnum, cvnum;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobiles cannot create.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );

   vnum = is_number( arg ) ? atoi( arg ) : -1;

   if( vnum == -1 || !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage:  ocreate <vnum> [copy vnum] <item name>\n\r", ch );
      return;
   }

   if( vnum < 1 || vnum > 1048576000 )
   {
      send_to_char( "Vnum out of range.\n\r", ch );
      return;
   }

   one_argument( argument, arg2 );
   cvnum = atoi( arg2 );
   if( cvnum != 0 )
      argument = one_argument( argument, arg2 );
   if( cvnum < 1 )
      cvnum = 0;

   if( get_obj_index( vnum ) )
   {
      send_to_char( "An object with that number already exists.\n\r", ch );
      return;
   }

   if( IS_NPC( ch ) )
      return;
   pObjIndex = make_object( vnum, cvnum, argument );
   if( !pObjIndex )
   {
      send_to_char( "Error.\n\r", ch );
      log_string( "do_ocreate: make_object failed." );
      return;
   }
   obj = create_object( pObjIndex, 0 );
   obj_to_char( obj, ch );
   act( AT_PURPLE, "$n makes arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
   ch_printf_color( ch,
                    "&YYou make arcane gestures, and open your hands to reveal %s!\n\rObjVnum:  &W%d   &YKeywords:  &W%s\n\r",
                    pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name );
}

void do_mcreate( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *mob;
   int vnum, cvnum;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobiles cannot create.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );

   vnum = is_number( arg ) ? atoi( arg ) : -1;

   if( vnum == -1 || !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage:  mcreate <vnum> [cvnum] <mobile name>\n\r", ch );
      return;
   }

   if( vnum < 1 || vnum > 1048576000 )
   {
      send_to_char( "Vnum out of range.\n\r", ch );
      return;
   }

   one_argument( argument, arg2 );
   cvnum = atoi( arg2 );
   if( cvnum != 0 )
      argument = one_argument( argument, arg2 );
   if( cvnum < 1 )
      cvnum = 0;

   if( get_mob_index( vnum ) )
   {
      send_to_char( "A mobile with that number already exists.\n\r", ch );
      return;
   }

   if( IS_NPC( ch ) )
      return;

   pMobIndex = make_mobile( vnum, cvnum, argument );
   if( !pMobIndex )
   {
      send_to_char( "Error.\n\r", ch );
      log_string( "do_mcreate: make_mobile failed." );
      return;
   }
   mob = create_mobile( pMobIndex );
   char_to_room( mob, ch->in_room );
   act( AT_YELLOW, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
   ch_printf_color( ch,
                    "&YYou wave your arms about, and %s appears at your command!\n\rMobVnum:  &W%d   &YKeywords:  &W%s\n\r",
                    pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name );
}

/*
 * Simple but nice and handy line editor.			-Thoric
 */
void edit_buffer( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   EDITOR_DATA *edit;
   char cmd[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   sh_int x, line, max_buf_lines;
   bool save;

   if( ( d = ch->desc ) == NULL )
   {
      send_to_char( "You have no descriptor.\n\r", ch );
      return;
   }

   if( d->connected != CON_EDITING )
   {
      send_to_char( "You can't do that!\n\r", ch );
      bug( "Edit_buffer: d->connected != CON_EDITING", 0 );
      return;
   }

   if( ch->substate <= SUB_PAUSE )
   {
      send_to_char( "You can't do that!\n\r", ch );
      bug( "Edit_buffer: illegal ch->substate (%d)", ch->substate );
      d->connected = CON_PLAYING;
      return;
   }

   if( !ch->editor )
   {
      send_to_char( "You can't do that!\n\r", ch );
      bug( "Edit_buffer: null editor", 0 );
      d->connected = CON_PLAYING;
      return;
   }

   edit = ch->editor;
   save = FALSE;
   max_buf_lines = 24;

   if( ch->substate == SUB_MPROG_EDIT || ch->substate == SUB_HELP_EDIT )
      max_buf_lines = 1000;

   if( argument[0] == '/' || argument[0] == '\\' )
   {
      one_argument( argument, cmd );
      if( !str_cmp( cmd + 1, "?" ) )
      {
         send_to_char( "Editing commands\n\r---------------------------------\n\r", ch );
         send_to_char( "/l              list buffer\n\r", ch );
         send_to_char( "/c              clear buffer\n\r", ch );
         send_to_char( "/d [line]       delete line\n\r", ch );
         send_to_char( "/f              format text, needs /g first\n\r", ch );
         send_to_char( "/g <line>       goto line\n\r", ch );
         send_to_char( "/i <line>       insert line\n\r", ch );
         send_to_char( "/r <old> <new>  global replace\n\r", ch );
         send_to_char( "/a              abort editing\n\r", ch );
         if( IS_SET( ch->pcdata->permissions, PERMIT_HELP ) )
            send_to_char( "/! <command>    execute command (do not use another editing command)\n\r", ch );
         send_to_char( "/s              save buffer\n\r\n\r> ", ch );
         return;
      }
      if( !str_cmp( cmd + 1, "c" ) )
      {
         memset( edit, '\0', sizeof( EDITOR_DATA ) );
         edit->numlines = 0;
         edit->on_line = 0;
         send_to_char( "Buffer cleared.\n\r> ", ch );
         return;
      }
      /*
       * added format command - shogar 
       */
      if( !str_cmp( cmd + 1, "f" ) )
      {
         char cword[MAX_INPUT_LENGTH];
         char cline[MAX_INPUT_LENGTH + 80];
         char *sptr, *lwptr, *wptr, *tptr;
         int x, linecnt = -1, spacercnt = 0;

         pager_printf( ch, "Reformating...\n\r" );
         for( x = edit->on_line; x < edit->numlines; x++ )
         {
            if( linecnt < 0 )
            {
               linecnt = x;
               lwptr = edit->line[linecnt];
            }
            tptr = edit->line[x];
            while( *tptr )
            {
               if( !isspace( (int)*tptr ) )
                  break;
               tptr++;
            }
            if( !*tptr )
            {
               if( !spacercnt )
               {
                  linecnt++;
                  lwptr = edit->line[linecnt];
                  *lwptr = 0;
                  linecnt++;
                  spacercnt++;
                  lwptr = edit->line[linecnt];
               }
               continue;
            }
            spacercnt = 0;
            strcpy( cline, edit->line[x] );
            sptr = cline;
            *lwptr = 0;
            while( *sptr )
            {
               wptr = cword;
               while( isspace( (int)*sptr ) && *sptr )
                  sptr++;
               while( !isspace( (int)*sptr ) && *sptr )
                  *wptr++ = *sptr++;
               *wptr = 0;

               if( !*cword )
               {
                  sptr = cline;
                  *sptr = 0;
                  continue;
               }
               if( ( strlen( edit->line[linecnt] ) + strlen( cword ) + 1 ) > 79 )
               {
                  if( edit->numlines >= max_buf_lines )
                  {
                     send_to_pager( "Buffer is full.\n\r> ", ch );
                     return;
                  }
                  else
                  {
                     strcpy( edit->line[edit->numlines], "" );
                     edit->numlines++;
                     linecnt++;
                     lwptr = edit->line[linecnt];
                     *lwptr = 0;

                  }
               }
               strcat( lwptr, cword );
               lwptr += strlen( cword );
               strcat( lwptr, " " );
               lwptr++;
            }
         }
         if( linecnt != -1 )
         {
            for( x = linecnt + 1; x < edit->numlines; x++ )
            {
               strcpy( edit->line[x], "" );
            }
            edit->numlines = linecnt + 2;
            edit->on_line = linecnt + 1;
         }
         pager_printf( ch, "Reformating done...\n\r> " );
         return;

      }
      if( !str_cmp( cmd + 1, "r" ) )
      {
         char word1[MAX_INPUT_LENGTH];
         char word2[MAX_INPUT_LENGTH];
         char *sptr, *wptr, *lwptr;
         int x, count, wordln, word2ln, lineln;

         sptr = one_argument( argument, word1 );
         sptr = one_argument( sptr, word1 );
         sptr = one_argument( sptr, word2 );
         if( word1[0] == '\0' || word2[0] == '\0' )
         {
            send_to_char( "Need word to replace, and replacement.\n\r> ", ch );
            return;
         }
         if( strcmp( word1, word2 ) == 0 )
         {
            send_to_char( "Done.\n\r> ", ch );
            return;
         }
         count = 0;
         wordln = strlen( word1 );
         word2ln = strlen( word2 );
         ch_printf( ch, "Replacing all occurrences of %s with %s...\n\r", word1, word2 );
         for( x = edit->on_line; x < edit->numlines; x++ )
         {
            lwptr = edit->line[x];
            while( ( wptr = strstr( lwptr, word1 ) ) != NULL )
            {
               ++count;
               lineln = sprintf( buf, "%s%s", word2, wptr + wordln );
               if( lineln + wptr - edit->line[x] > 79 )
                  buf[lineln] = '\0';
               strcpy( wptr, buf );
               lwptr = wptr + word2ln;
            }
         }
         ch_printf( ch, "Found and replaced %d occurrence(s).\n\r> ", count );
         return;
      }

      if( !str_cmp( cmd + 1, "i" ) )
      {
         if( edit->numlines >= max_buf_lines )
            send_to_char( "Buffer is full.\n\r> ", ch );
         else
         {
            if( argument[2] == ' ' )
               line = atoi( argument + 2 ) - 1;
            else
               line = edit->on_line;
            if( line < 0 )
               line = edit->on_line;
            if( line < 0 || line > edit->numlines )
               send_to_char( "Out of range.\n\r> ", ch );
            else
            {
               for( x = ++edit->numlines; x > line; x-- )
                  strcpy( edit->line[x], edit->line[x - 1] );
               strcpy( edit->line[line], "" );
               send_to_char( "Line inserted.\n\r> ", ch );
            }
         }
         return;
      }
      if( !str_cmp( cmd + 1, "d" ) )
      {
         if( edit->numlines == 0 )
            send_to_char( "Buffer is empty.\n\r> ", ch );
         else
         {
            if( argument[2] == ' ' )
               line = atoi( argument + 2 ) - 1;
            else
               line = edit->on_line;
            if( line < 0 )
               line = edit->on_line;
            if( line < 0 || line > edit->numlines )
               send_to_char( "Out of range.\n\r> ", ch );
            else
            {
               if( line == 0 && edit->numlines == 1 )
               {
                  memset( edit, '\0', sizeof( EDITOR_DATA ) );
                  edit->numlines = 0;
                  edit->on_line = 0;
                  send_to_char( "Line deleted.\n\r> ", ch );
                  return;
               }
               for( x = line; x < ( edit->numlines - 1 ); x++ )
                  strcpy( edit->line[x], edit->line[x + 1] );
               strcpy( edit->line[edit->numlines--], "" );
               if( edit->on_line > edit->numlines )
                  edit->on_line = edit->numlines;
               send_to_char( "Line deleted.\n\r> ", ch );
            }
         }
         return;
      }
      if( !str_cmp( cmd + 1, "g" ) )
      {
         if( edit->numlines == 0 )
            send_to_char( "Buffer is empty.\n\r> ", ch );
         else
         {
            if( argument[2] == ' ' )
               line = atoi( argument + 2 ) - 1;
            else
            {
               send_to_char( "Goto what line?\n\r> ", ch );
               return;
            }
            if( line < 0 )
               line = edit->on_line;
            if( line < 0 || line > edit->numlines )
               send_to_char( "Out of range.\n\r> ", ch );
            else
            {
               edit->on_line = line;
               ch_printf( ch, "(On line %d)\n\r> ", line + 1 );
            }
         }
         return;
      }
      if( !str_cmp( cmd + 1, "l" ) )
      {
         if( edit->numlines == 0 )
            send_to_char( "Buffer is empty.\n\r> ", ch );
         else
         {
            send_to_char( "------------------\n\r", ch );
            for( x = 0; x < edit->numlines; x++ )
               ch_printf( ch, "%2d> %s\n\r", x + 1, edit->line[x] );
            send_to_char( "------------------\n\r> ", ch );
         }
         return;
      }
      if( !str_cmp( cmd + 1, "a" ) )
      {
         send_to_char( "\n\rAborting... ", ch );
         stop_editing( ch );
         return;
      }
      if( IS_SET( ch->pcdata->permissions, PERMIT_HELP ) && !str_cmp( cmd + 1, "!" ) )
      {
         DO_FUN *last_cmd;
         int substate = ch->substate;

         last_cmd = ch->last_cmd;
         ch->substate = SUB_RESTRICTED;
         interpret( ch, argument + 3, FALSE );
         ch->substate = substate;
         ch->last_cmd = last_cmd;
         set_char_color( AT_GREEN, ch );
         send_to_char( "\n\r> ", ch );
         return;
      }
      if( !str_cmp( cmd + 1, "s" ) )
      {
         d->connected = CON_PLAYING;
         if( !ch->last_cmd )
            return;
         ( *ch->last_cmd ) ( ch, "" );
         return;
      }
   }

   if( edit->size + strlen( argument ) + 1 >= MAX_STRING_LENGTH - 1 )
      send_to_char( "You buffer is full.\n\r", ch );
   else
   {
      if( strlen( argument ) > 79 )
      {
         strncpy( buf, argument, 79 );
         buf[79] = 0;
         send_to_char( "(Long line trimmed)\n\r> ", ch );
      }
      else
         strcpy( buf, argument );
      strcpy( edit->line[edit->on_line++], buf );
      if( edit->on_line > edit->numlines )
         edit->numlines++;
      if( edit->numlines > max_buf_lines )
      {
         edit->numlines = max_buf_lines;
         send_to_char( "Buffer full.\n\r", ch );
         save = TRUE;
      }
   }

   if( save )
   {
      d->connected = CON_PLAYING;
      if( !ch->last_cmd )
         return;
      ( *ch->last_cmd ) ( ch, "" );
      return;
   }
   send_to_char( "> ", ch );
}

void assign_area( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char taf[1024];
   AREA_DATA *tarea, *tmp;
   bool created = FALSE;

   if( IS_NPC( ch ) )
      return;
   if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
   {
      tarea = ch->pcdata->area;
      sprintf( taf, "%s.are", capitalize( ch->name ) );
      if( !tarea )
      {
         for( tmp = first_build; tmp; tmp = tmp->next )
            if( !str_cmp( taf, tmp->filename ) )
            {
               tarea = tmp;
               break;
            }
      }
      if( !tarea )
      {
         sprintf( buf, "Creating area entry for %s", ch->name );
         log_string_plus( buf, LOG_BUILD, 0 );
         CREATE( tarea, AREA_DATA, 1 );
         LINK( tarea, first_build, last_build, next, prev );
         tarea->first_room = tarea->last_room = NULL;
         sprintf( buf, "{PROTO} %s's area in progress", ch->name );
         tarea->name = str_dup( buf );
         tarea->filename = str_dup( taf );
         sprintf( buf2, "%s", ch->name );
         tarea->author = STRALLOC( buf2 );
         tarea->age = 0;
         tarea->nplayer = 0;
         tarea->resetmsg = str_dup( "Void" );

         CREATE( tarea->weather, WEATHER_DATA, 1 );   /* FB */
         tarea->weather->temp = 0;
         tarea->weather->precip = 0;
         tarea->weather->wind = 0;
         tarea->weather->temp_vector = 0;
         tarea->weather->precip_vector = 0;
         tarea->weather->wind_vector = 0;
         tarea->weather->climate_temp = 2;
         tarea->weather->climate_precip = 2;
         tarea->weather->climate_wind = 2;
         tarea->weather->first_neighbor = NULL;
         tarea->weather->last_neighbor = NULL;
         tarea->weather->echo = NULL;
         tarea->weather->echo_color = AT_GREY;

         created = TRUE;
      }
      else
      {
         sprintf( buf, "Updating area entry for %s", ch->name );
         log_string_plus( buf, LOG_BUILD, 0 );
      }
      tarea->low_r_vnum = ch->pcdata->r_range_lo;
      tarea->low_o_vnum = ch->pcdata->o_range_lo;
      tarea->low_m_vnum = ch->pcdata->m_range_lo;
      tarea->hi_r_vnum = ch->pcdata->r_range_hi;
      tarea->hi_o_vnum = ch->pcdata->o_range_hi;
      tarea->hi_m_vnum = ch->pcdata->m_range_hi;
      ch->pcdata->area = tarea;
      if( created )
         sort_area( tarea, TRUE );
   }
}

void do_aassign( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   AREA_DATA *tarea, *tmp;

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      send_to_char( "Syntax: aassign <filename.are>\n\r", ch );
      return;
   }

   if( !str_cmp( "none", argument ) || !str_cmp( "null", argument ) || !str_cmp( "clear", argument ) )
   {
      ch->pcdata->area = NULL;
      assign_area( ch );
      if( !ch->pcdata->area )
         send_to_char( "Area pointer cleared.\n\r", ch );
      else
         send_to_char( "Originally assigned area restored.\n\r", ch );
      return;
   }

   sprintf( buf, "%s", argument );
   tarea = NULL;

   for( tmp = first_area; tmp; tmp = tmp->next )
      if( !str_cmp( buf, tmp->filename ) )
      {
         tarea = tmp;
         break;
      }

   if( !tarea )
      for( tmp = first_build; tmp; tmp = tmp->next )
         if( !str_cmp( buf, tmp->filename ) )
         {
            tarea = tmp;
            break;
         }

   if( !tarea )
   {
      send_to_char( "No such area.  Use 'zones'.\n\r", ch );
      return;
   }
   ch->pcdata->area = tarea;
   ch_printf( ch, "Assigning you: %s\n\r", tarea->name );
   return;
}


EXTRA_DESCR_DATA *SetRExtra( ROOM_INDEX_DATA * room, char *keywords )
{
   EXTRA_DESCR_DATA *ed;

   for( ed = room->first_extradesc; ed; ed = ed->next )
   {
      if( is_name( keywords, ed->keyword ) )
         break;
   }
   if( !ed )
   {
      CREATE( ed, EXTRA_DESCR_DATA, 1 );
      LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
      ed->keyword = STRALLOC( keywords );
      ed->description = STRALLOC( "" );
      top_ed++;
   }
   return ed;
}

bool DelRExtra( ROOM_INDEX_DATA * room, char *keywords )
{
   EXTRA_DESCR_DATA *rmed;

   for( rmed = room->first_extradesc; rmed; rmed = rmed->next )
   {
      if( is_name( keywords, rmed->keyword ) )
         break;
   }
   if( !rmed )
      return FALSE;
   UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
   STRFREE( rmed->keyword );
   STRFREE( rmed->description );
   DISPOSE( rmed );
   top_ed--;
   return TRUE;
}

EXTRA_DESCR_DATA *SetOExtra( OBJ_DATA * obj, char *keywords )
{
   EXTRA_DESCR_DATA *ed;

   for( ed = obj->first_extradesc; ed; ed = ed->next )
   {
      if( is_name( keywords, ed->keyword ) )
         break;
   }
   if( !ed )
   {
      CREATE( ed, EXTRA_DESCR_DATA, 1 );
      LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
      ed->keyword = STRALLOC( keywords );
      ed->description = STRALLOC( "" );
      top_ed++;
   }
   return ed;
}

bool DelOExtra( OBJ_DATA * obj, char *keywords )
{
   EXTRA_DESCR_DATA *rmed;

   for( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
   {
      if( is_name( keywords, rmed->keyword ) )
         break;
   }
   if( !rmed )
      return FALSE;
   UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
   STRFREE( rmed->keyword );
   STRFREE( rmed->description );
   DISPOSE( rmed );
   top_ed--;
   return TRUE;
}

EXTRA_DESCR_DATA *SetOExtraProto( OBJ_INDEX_DATA * obj, char *keywords )
{
   EXTRA_DESCR_DATA *ed;

   for( ed = obj->first_extradesc; ed; ed = ed->next )
   {
      if( is_name( keywords, ed->keyword ) )
         break;
   }
   if( !ed )
   {
      CREATE( ed, EXTRA_DESCR_DATA, 1 );
      LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
      ed->keyword = STRALLOC( keywords );
      ed->description = STRALLOC( "" );
      top_ed++;
   }
   return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA * obj, char *keywords )
{
   EXTRA_DESCR_DATA *rmed;

   for( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
   {
      if( is_name( keywords, rmed->keyword ) )
         break;
   }
   if( !rmed )
      return FALSE;
   UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
   STRFREE( rmed->keyword );
   STRFREE( rmed->description );
   DISPOSE( rmed );
   top_ed--;
   return TRUE;
}

void fold_area( AREA_DATA * tarea, char *filename, bool install )
{
   RESET_DATA *pReset, *tReset, *gReset;
   ROOM_INDEX_DATA *room;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   MPROG_DATA *mprog;
   GENERIC_MOB *genmob;
   EXIT_DATA *xit;
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *paf;
   SHOP_DATA *pShop;
   NEIGHBOR_DATA *neigh;
   char buf[MAX_STRING_LENGTH];
   FILE *fpout;
   int vnum;
   int val0, val1, val2, val3, val4, val5;
   bool complexmob;

/*    sprintf( buf, "Saving %s...", tarea->filename );
    log_string_plus( buf, LOG_BUILD, 0 ); */

   sprintf( buf, "%s.bak", filename );
   rename( filename, buf );
   fclose( fpReserve );
   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      bug( "fold_area: fopen", 0 );
      perror( filename );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }

   fprintf( fpout, "#AREA   %s~\n\n\n\n", tarea->name );
   fprintf( fpout, "#VERSION %d\n", AREA_VERSION_WRITE );
   fprintf( fpout, "#AUTHOR %s~\n\n", tarea->author );
   fprintf( fpout, "#RANGES\n" );
   fprintf( fpout, "%d %d %d %d\n", tarea->low_soft_range,
            tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
   fprintf( fpout, "$\n\n" );
   if( tarea->resetmsg )   /* Rennard */
      fprintf( fpout, "#RESETMSG %s~\n\n", tarea->resetmsg );
   if( tarea->reset_frequency )
      fprintf( fpout, "#FLAGS\n%d %d\n\n", tarea->flags, tarea->reset_frequency );
   else
      fprintf( fpout, "#FLAGS\n%d\n\n", tarea->flags );

   fprintf( fpout, "#ECONOMY %d %d\n\n", tarea->high_economy, tarea->low_economy );

   /*
    * Climate info - FB 
    */
   fprintf( fpout, "#CLIMATE %d %d %d %d\n\n", tarea->weather->climate_temp,
            tarea->weather->climate_precip, tarea->weather->climate_wind, tarea->weather->climate_mana );

   /*
    * neighboring weather systems - FB 
    */
   for( neigh = tarea->weather->first_neighbor; neigh; neigh = neigh->next )
      fprintf( fpout, "#NEIGHBOR %s~\n\n", neigh->name );

   /*
    * save mobiles 
    */
   fprintf( fpout, "#MOBILES\n" );
   for( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
   {
      if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
         continue;
      if( install )
         xREMOVE_BIT( pMobIndex->act, ACT_PROTOTYPE );
      if( pMobIndex->perm_str != 13 || pMobIndex->perm_int != 13
          || pMobIndex->perm_wil != 13 || pMobIndex->perm_dex != 13
          || pMobIndex->perm_con != 13 || pMobIndex->perm_per != 13
          || pMobIndex->perm_lck != 13
          || pMobIndex->hitroll != 0 || pMobIndex->damroll != 0
          || pMobIndex->race != 0
          || !xIS_EMPTY( pMobIndex->attacks )
          || !xIS_EMPTY( pMobIndex->defenses )
          || pMobIndex->height != 0 || pMobIndex->weight != 0 || pMobIndex->xflags != 0 || pMobIndex->numattacks != 0 )
         complexmob = TRUE;
      else
         complexmob = FALSE;
      fprintf( fpout, "#%d\n", vnum );
      fprintf( fpout, "%s~\n", pMobIndex->player_name );
      fprintf( fpout, "%s~\n", pMobIndex->short_descr );
      fprintf( fpout, "%s~\n", strip_cr( pMobIndex->description ) );
      fprintf( fpout, "%s ", print_bitvector( &pMobIndex->act ) );
      fprintf( fpout, "%s %d %c\n", print_bitvector( &pMobIndex->affected_by ), 0, complexmob ? 'C' : 'S' );

      fprintf( fpout, "%d %d %d ", 0, pMobIndex->mobthac0, pMobIndex->ac );
      fprintf( fpout, "%dd%d+%d ", pMobIndex->hitnodice, pMobIndex->hitsizedice, pMobIndex->hitplus );
      fprintf( fpout, "%dd%d+%d\n", pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus );
      fprintf( fpout, "%d %d\n", pMobIndex->gold, pMobIndex->exp );
/* Need to convert to new positions correctly on loadup sigh -Shaddai */
      fprintf( fpout, "%d %d %d\n", pMobIndex->position + 100, pMobIndex->defposition + 100, pMobIndex->sex );
      if( complexmob )
      {
         fprintf( fpout, "%d %d %d %d %d %d %d\n",
                  pMobIndex->perm_str,
                  pMobIndex->perm_int,
                  pMobIndex->perm_wil, pMobIndex->perm_dex, pMobIndex->perm_con, pMobIndex->perm_per, pMobIndex->perm_lck );
         fprintf( fpout, "%d %d %d %d %d\n",
                  pMobIndex->saving_poison_death,
                  pMobIndex->saving_wand,
                  pMobIndex->saving_para_petri, pMobIndex->saving_breath, pMobIndex->saving_spell_staff );
         fprintf( fpout, "%d %d %d %d %d %d %d\n",
                  pMobIndex->race, 0, pMobIndex->height, pMobIndex->weight, 0, 0, pMobIndex->numattacks );
         fprintf( fpout, "%d %d %d %d %d %d %s ",
                  pMobIndex->hitroll,
                  pMobIndex->damroll,
                  pMobIndex->xflags,
                  pMobIndex->resistant, pMobIndex->immune, pMobIndex->susceptible, print_bitvector( &pMobIndex->attacks ) );
         fprintf( fpout, "%s\n", print_bitvector( &pMobIndex->defenses ) );
      }
      if( pMobIndex->mudprogs )
      {
         for( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
            fprintf( fpout, "> %s %s~\n%s~\n",
                     mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr( mprog->comlist ) );
         fprintf( fpout, "|\n" );
      }
   }
   fprintf( fpout, "#0\n\n\n" );
   if( install && vnum < tarea->hi_m_vnum )
      tarea->hi_m_vnum = vnum - 1;

   /*
    * save objects 
    */
   fprintf( fpout, "#OBJECTS\n" );
   for( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
   {
      if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
         continue;
      if( install )
         xREMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
      fprintf( fpout, "#%d\n", vnum );
      fprintf( fpout, "%s~\n", pObjIndex->name );
      fprintf( fpout, "%s~\n", pObjIndex->short_descr );
      fprintf( fpout, "%s~\n", pObjIndex->description );
      fprintf( fpout, "%s~\n", pObjIndex->action_desc );
      if( pObjIndex->layers )
         fprintf( fpout, "%d %s %d %d\n", pObjIndex->item_type,
                  print_bitvector( &pObjIndex->extra_flags ), pObjIndex->tech, pObjIndex->layers );
      else
         fprintf( fpout, "%d %s %d\n", pObjIndex->item_type, print_bitvector( &pObjIndex->extra_flags ), pObjIndex->tech );
      fprintf( fpout, "%s\n", print_bitvector( &pObjIndex->parts ) );

      val0 = pObjIndex->value[0];
      val1 = pObjIndex->value[1];
      val2 = pObjIndex->value[2];
      val3 = pObjIndex->value[3];
      val4 = pObjIndex->value[4];
      val5 = pObjIndex->value[5];
      switch ( pObjIndex->item_type )
      {
         case ITEM_PILL:
         case ITEM_POTION:
         case ITEM_SCROLL:
            if( IS_VALID_SN( val1 ) )
            {
               if( AREA_VERSION_WRITE == 0 )
                  val1 = skill_table[val1]->slot;
               else
                  val1 = HAS_SPELL_INDEX;
            }
            if( IS_VALID_SN( val2 ) )
            {
               if( AREA_VERSION_WRITE == 0 )
                  val2 = skill_table[val2]->slot;
               else
                  val1 = HAS_SPELL_INDEX;
            }
            if( IS_VALID_SN( val3 ) )
            {
               if( AREA_VERSION_WRITE == 0 )
                  val3 = skill_table[val3]->slot;
               else
                  val1 = HAS_SPELL_INDEX;
               break;
            }
         case ITEM_STAFF:
         case ITEM_WAND:
            if( IS_VALID_SN( val3 ) )
            {
               if( AREA_VERSION_WRITE == 0 )
                  val3 = skill_table[val3]->slot;
               else
                  val3 = HAS_SPELL_INDEX;
               break;
            }
         case ITEM_SALVE:
            if( IS_VALID_SN( val4 ) )
            {
               if( AREA_VERSION_WRITE == 0 )
                  val4 = skill_table[val4]->slot;
               else
                  val4 = HAS_SPELL_INDEX;

               if( IS_VALID_SN( val5 ) )
               {
                  if( AREA_VERSION_WRITE == 0 )
                     val5 = skill_table[val5]->slot;
                  else
                     val5 = HAS_SPELL_INDEX;
                  break;
               }
            }
      }
      if( val4 || val5 )
         fprintf( fpout, "%d %d %d %d %d %d\n", val0, val1, val2, val3, val4, val5 );
      else
         fprintf( fpout, "%d %d %d %d\n", val0, val1, val2, val3 );

      fprintf( fpout, "%d %d %d\n", pObjIndex->weight, pObjIndex->cost, 0 );

      if( AREA_VERSION_WRITE > 0 )
         switch ( pObjIndex->item_type )
         {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
               fprintf( fpout, "'%s' '%s' '%s'\n",
                        IS_VALID_SN( pObjIndex->value[1] ) ?
                        skill_table[pObjIndex->value[1]]->name : "NONE",
                        IS_VALID_SN( pObjIndex->value[2] ) ?
                        skill_table[pObjIndex->value[2]]->name : "NONE",
                        IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
               break;
            case ITEM_STAFF:
            case ITEM_WAND:
               fprintf( fpout, "'%s'\n",
                        IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );

               break;
            case ITEM_SALVE:
               fprintf( fpout, "'%s' '%s'\n",
                        IS_VALID_SN( pObjIndex->value[4] ) ?
                        skill_table[pObjIndex->value[4]]->name : "NONE",
                        IS_VALID_SN( pObjIndex->value[5] ) ? skill_table[pObjIndex->value[5]]->name : "NONE" );
               break;
         }

      for( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
         fprintf( fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr( ed->description ) );

      for( paf = pObjIndex->first_affect; paf; paf = paf->next )
         fprintf( fpout, "A\n%d %d\n", paf->location,
                  ( ( paf->location == APPLY_WEAPONSPELL
                      || paf->location == APPLY_WEARSPELL
                      || paf->location == APPLY_REMOVESPELL
                      || paf->location == APPLY_STRIPSN
                      || paf->location == APPLY_RECURRINGSPELL )
                    && IS_VALID_SN( paf->modifier ) ) ? skill_table[paf->modifier]->slot : paf->modifier );

      if( pObjIndex->mudprogs )
      {
         for( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
            fprintf( fpout, "> %s %s~\n%s~\n",
                     mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr( mprog->comlist ) );
         fprintf( fpout, "|\n" );
      }
   }
   fprintf( fpout, "#0\n\n\n" );
   if( install && vnum < tarea->hi_o_vnum )
      tarea->hi_o_vnum = vnum - 1;

   /*
    * save rooms   
    */
   fprintf( fpout, "#ROOMS\n" );
   for( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) == NULL )
         continue;
      if( install )
      {
         CHAR_DATA *victim, *vnext;
         OBJ_DATA *obj, *obj_next;

         /*
          * remove prototype flag from room 
          */
         REMOVE_BIT( room->room_flags, ROOM_PROTOTYPE );
         /*
          * purge room of (prototyped) mobiles 
          */
         for( victim = room->first_person; victim; victim = vnext )
         {
            vnext = victim->next_in_room;
            if( IS_NPC( victim ) )
               extract_char( victim, TRUE );
         }
         /*
          * purge room of (prototyped) objects 
          */
         for( obj = room->first_content; obj; obj = obj_next )
         {
            obj_next = obj->next_content;
            extract_obj( obj );
         }
      }
      fprintf( fpout, "#%d\n", vnum );
      fprintf( fpout, "%s~\n", room->name );
      fprintf( fpout, "%s~\n", strip_cr( room->description ) );
      if( ( room->tele_delay > 0 && room->tele_vnum > 0 ) || room->runes > 0 )
         fprintf( fpout, "0 %d %d %d %d %d\n", room->room_flags,
                  room->sector_type, room->tele_delay, room->tele_vnum, room->runes );
      else
         fprintf( fpout, "0 %d %d\n", room->room_flags, room->sector_type );

      /*
       * Scion -- advanced sector types 
       */

/*	switch( room->sector_type )
	{
	    default:
			break;
	    case SECT_AIR:
			room->vegetation = 0;
			room->curr_vegetation = 0;
			room->water = 0;
			room->curr_water = 0;
			room->elevation = 100;
			room->curr_elevation = 100;
			room->resources = 0;
			room->curr_resources = 0;
			break;
	    case SECT_INSIDE:
			room->vegetation = 0;
			room->curr_vegetation = 0;
			room->water = 0;
			room->curr_water = 0;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 0;
			room->curr_resources = 0;
			break;
	    case SECT_FIELD:
			room->vegetation = 20;
			room->curr_vegetation = 20;
			room->water = 0;
			room->curr_water = 0;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 70;
			room->curr_resources = 70;
			break;
	    case SECT_UNDERGROUND:
			room->vegetation = 0;
			room->curr_vegetation = 0;
			room->water = 20;
			room->curr_water = 20;
			room->elevation = -100;
			room->curr_elevation = -100;
			room->resources = 100;
			room->curr_resources = 100;
			break;
	    case SECT_FOREST:
			room->vegetation = 90;
			room->curr_vegetation = 90;
			room->water = 30;
			room->curr_water = 30;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 100;
			room->curr_resources = 100;
			break;
	    case SECT_CITY:
			room->vegetation = 0;
			room->curr_vegetation = 0;
			room->water = 0;
			room->curr_water = 0;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 30;
			room->curr_resources = 30;
			break;
	    case SECT_DESERT:
			room->vegetation = 10;
			room->curr_vegetation = 10;
			room->water = 0;
			room->curr_water = 0;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 20;
			room->curr_resources = 20;
			break;
	    case SECT_HILLS:
			room->vegetation = 30;
			room->curr_vegetation = 30;
			room->water = 0;
			room->curr_water = 0;
			room->elevation = 30;
			room->curr_elevation = 30;
			room->resources = 90;
			room->curr_resources = 90;
			break;
	    case SECT_WATER_SWIM:
			room->vegetation = 0;
			room->curr_vegetation = 0;
			room->water = 40;
			room->curr_water = 40;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 100;
			room->curr_resources = 100;
			break;
	    case SECT_WATER_NOSWIM:
			room->vegetation = 0;
			room->curr_vegetation = 0;
			room->water = 80;
			room->curr_water = 80;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 100;
			room->curr_resources = 100;
			break;
	    case SECT_MOUNTAIN:
			room->vegetation =20;
			room->curr_vegetation = 20;
			room->water = 0;
			room->curr_water = 0;
			room->elevation = 70;
			room->curr_elevation = 70;
			room->resources = 100;
			room->curr_resources = 100;
			break;
	    case SECT_UNDERWATER:
			room->vegetation = 0;
			room->curr_vegetation = 0;
			room->water = 100;
			room->curr_water = 100;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 0;
			room->curr_resources = 0;
			break;
	    case SECT_OCEANFLOOR:
			room->vegetation = 30;
			room->curr_vegetation = 30;
			room->water = 100;
			room->curr_water = 100;
			room->elevation = 0;
			room->curr_elevation = 0;
			room->resources = 100;
			room->curr_resources = 100;
			break;
	}
*/
      fprintf( fpout, "%d %d %d %d %d %d %d %d\n",
               room->vegetation, room->curr_vegetation,
               room->water, room->curr_water, room->elevation, room->curr_elevation, room->resources, room->curr_resources );

      for( xit = room->first_exit; xit; xit = xit->next )
      {
         if( IS_SET( xit->exit_info, EX_PORTAL ) ) /* don't fold portals */
            continue;
         fprintf( fpout, "D%d\n", xit->vdir );
         fprintf( fpout, "%s~\n", strip_cr( xit->description ) );
         fprintf( fpout, "%s~\n", strip_cr( xit->keyword ) );
         if( xit->distance > 1 || xit->pull )
            fprintf( fpout, "%d %d %d %d %d %d\n",
                     xit->exit_info & ~EX_BASHED, xit->key, xit->vnum, xit->distance, xit->pulltype, xit->pull );
         else
            fprintf( fpout, "%d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum );
      }
      for( pReset = room->first_reset; pReset; pReset = pReset->next )
      {
         switch ( pReset->command ) /* extra arg1 arg2 arg3 */
         {
            default:
            case '*':
               break;
            case 'm':
            case 'M':
            case 'o':
            case 'O':
               fprintf( fpout, "R %c %d %d %d %d\n", UPPER( pReset->command ),
                        pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );

               for( tReset = pReset->first_reset; tReset; tReset = tReset->next_reset )
               {
                  switch ( tReset->command )
                  {
                     case 'p':
                     case 'P':
                     case 'e':
                     case 'E':
                        fprintf( fpout, "  R %c %d %d %d %d\n", UPPER( tReset->command ),
                                 tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
                        if( tReset->first_reset )
                        {
                           for( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
                           {
                              if( gReset->command != 'p' && gReset->command != 'P' )
                                 continue;
                              fprintf( fpout, "    R %c %d %d %d %d\n", UPPER( gReset->command ),
                                       gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
                           }
                        }
                        break;

                     case 'g':
                     case 'G':
                        fprintf( fpout, "  R %c %d %d %d\n", UPPER( tReset->command ),
                                 tReset->extra, tReset->arg1, tReset->arg2 );
                        if( tReset->first_reset )
                        {
                           for( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
                           {
                              if( gReset->command != 'p' && gReset->command != 'P' )
                                 continue;
                              fprintf( fpout, "    R %c %d %d %d %d\n", UPPER( gReset->command ),
                                       gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
                           }
                        }
                        break;

                     case 't':
                     case 'T':
                     case 'h':
                     case 'H':
                        fprintf( fpout, "  R %c %d %d %d %d\n", UPPER( tReset->command ),
                                 tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
                        break;
                  }
               }
               break;

            case 'd':
            case 'D':
            case 't':
            case 'T':
            case 'h':
            case 'H':
               fprintf( fpout, "R %c %d %d %d %d\n", UPPER( pReset->command ),
                        pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
               break;

            case 'r':
            case 'R':
               fprintf( fpout, "R %c %d %d %d\n", UPPER( pReset->command ), pReset->extra, pReset->arg1, pReset->arg2 );
               break;
         }
      }
      for( ed = room->first_extradesc; ed; ed = ed->next )
         fprintf( fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr( ed->description ) );

      if( room->map )   /* maps */
      {
/*#ifdef OLDMAPS
	   fprintf( fpout, "M\n" );
	   fprintf( fpout, "%s~\n", strip_cr( room->map )	);
#else
	   fprintf( fpout, "M %d %d %d %c\n",	room->map->vnum,
						room->map->x,
						room->map->y,
						room->map->entry );
#endif*/
      }
      if( room->mudprogs )
      {
         for( mprog = room->mudprogs; mprog; mprog = mprog->next )
            fprintf( fpout, "> %s %s~\n%s~\n",
                     mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr( mprog->comlist ) );
         fprintf( fpout, "|\n" );
      }
      fprintf( fpout, "S\n" );
   }
   fprintf( fpout, "#0\n\n\n" );
   if( install && vnum < tarea->hi_r_vnum )
      tarea->hi_r_vnum = vnum - 1;

   /*
    * save shops 
    */
   fprintf( fpout, "#SHOPS\n" );
   for( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) == NULL )
         continue;
      if( ( pShop = room->pShop ) == NULL )
         continue;
      fprintf( fpout, " %d   %d %d %d %d %d %s~\n",
               pShop->room, pShop->profit_buy, pShop->profit_sell, pShop->gold, pShop->type, pShop->flag, pShop->owner );
   }
   fprintf( fpout, "0\n\n\n" );

   /*
    * save specials 
    */
   fprintf( fpout, "#SPECIALS\n" );
   for( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
   {
      if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
         continue;
      if( !pMobIndex->spec_fun )
         continue;
      fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, lookup_spec( pMobIndex->spec_fun ) );
   }
   /*
    * save generic mobs 
    */
   for( genmob = tarea->first_race; genmob; genmob = genmob->next )
   {
      fprintf( fpout, "N	%s~\n", genmob->nation->name );
   }
   fprintf( fpout, "S\n\n\n" );

   /*
    * END 
    */
   fprintf( fpout, "#$\n" );
   fclose( fpout );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}

void do_savearea( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *tarea;
   char filename[256];

   if( IS_NPC( ch ) || !ch->pcdata || ( argument[0] == '\0' && !ch->pcdata->area ) )
   {
      send_to_char( "You don't have an assigned area to save.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
      tarea = ch->pcdata->area;
   else
   {
      bool found;

      if( !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
      {
         send_to_char( "You can only save your own area.\n\r", ch );
         return;
      }
      for( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, argument ) )
         {
            found = TRUE;
            break;
         }
      if( !found )
      {
         send_to_char( "Area not found.\n\r", ch );
         return;
      }
   }

   if( !tarea )
   {
      send_to_char( "No area to save.\n\r", ch );
      return;
   }

/* Ensure not wiping out their area with save before load - Scryn 8/11 */
   if( !IS_SET( tarea->status, AREA_LOADED ) )
   {
      send_to_char( "Your area is not loaded!\n\r", ch );
      return;
   }

   sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
   send_to_char( "Saving area...\n\r", ch );
   fold_area( tarea, filename, FALSE );
   send_to_char( "Done.\n\r", ch );
}

void do_genarea( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   AREA_DATA *tarea;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Hell no, mobs are NOT allowed to build zones!\r\n", ch );
      return;
   }

   if( ch->pcdata->r_range_lo == 0 )
   {
      send_to_char( "You need to have a room range assigned first.\r\n", ch );
      return;
   }

   tarea = ch->pcdata->area;

   if( ch->pcdata->r_range_hi - ch->pcdata->r_range_lo < 99 )
   {
      send_to_char( "Your room range is less than 100 rooms.  Please have it reset.\r\n", ch );
      return;
   }

   if( IS_SET( tarea->status, AREA_LOADED ) )
   {
      send_to_char( "Your area is already loaded.\n\r", ch );
      return;
   }

   assign_area( ch );
   sprintf( buf, "Generating rooms for %s", ch->name );
   log_string_plus( buf, LOG_BUILD, 0 );
   if( makezone( ch, argument ) == FALSE )
   {
      sprintf( buf, "Room generation failed for %s", ch->name );
      log_string_plus( buf, LOG_BUILD, 0 );
   }
}


void do_loadarea( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *tarea;
   char filename[256];
   int tmp;

   if( IS_NPC( ch ) || !ch->pcdata || ( argument[0] == '\0' && !ch->pcdata->area ) )
   {
      send_to_char( "You don't have an assigned area to load.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
      tarea = ch->pcdata->area;
   else
   {
      bool found;

      if( !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
      {
         send_to_char( "You can only load your own area.\n\r", ch );
         return;
      }
      for( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, argument ) )
         {
            found = TRUE;
            break;
         }
      if( !found )
      {
         send_to_char( "Area not found.\n\r", ch );
         return;
      }
   }

   if( !tarea )
   {
      send_to_char( "No area to load.\n\r", ch );
      return;
   }

/* Stops char from loading when already loaded - Scryn 8/11 */
   if( IS_SET( tarea->status, AREA_LOADED ) )
   {
      send_to_char( "Your area is already loaded.\n\r", ch );
      return;
   }
   sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
   send_to_char( "Loading...\n\r", ch );
   load_area_file( tarea, filename );
   send_to_char( "Linking exits...\n\r", ch );
   fix_area_exits( tarea );
   if( tarea->first_room )
   {
      tmp = tarea->nplayer;
      tarea->nplayer = 0;
      send_to_char( "Resetting area...\n\r", ch );
      reset_area( tarea );
      tarea->nplayer = tmp;
   }
   send_to_char( "Done.\n\r", ch );
}

/*
 * Dangerous command.  Can be used to install an area that was either:
 *   (a) already installed but removed from area.lst
 *   (b) designed offline
 * The mud will likely crash if:
 *   (a) this area is already loaded
 *   (b) it contains vnums that exist
 *   (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.		-Thoric
 */
void do_unfoldarea( CHAR_DATA * ch, char *argument )
{

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Unfold what?\n\r", ch );
      return;
   }

   fBootDb = TRUE;
   load_area_file( last_area, argument );
   fBootDb = FALSE;
   return;
}


void do_foldarea( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *tarea;
   bool all = FALSE;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Fold what?\n\r", ch );
      return;
   }

   if( !str_cmp( argument, "everything" ) )
      all = TRUE;

   for( tarea = first_area; tarea; tarea = tarea->next )
   {
      if( !str_cmp( tarea->filename, argument ) || all == TRUE )
      {
         send_to_char( "Folding area...\n\r", ch );
         fold_area( tarea, tarea->filename, FALSE );
      }
   }
   send_to_char( "Done.\n\r", ch );
   return;
}

extern int top_area;

void write_area_list(  )
{
   AREA_DATA *tarea;
   FILE *fpout;

   fpout = fopen( AREA_LIST, "w" );
   if( !fpout )
   {
      bug( "FATAL: cannot open area.lst for writing!\n\r", 0 );
      return;
   }
   fprintf( fpout, "help.are\n" );
   for( tarea = first_area; tarea; tarea = tarea->next )
      fprintf( fpout, "%s\n", tarea->filename );
   fprintf( fpout, "$\n" );
   fclose( fpout );
}

/*
 * A complicated to use command as it currently exists.		-Thoric
 * Once area->author and area->name are cleaned up... it will be easier
 */
void do_installarea( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *tarea;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int num;
   DESCRIPTOR_DATA *d;

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: installarea <filename> [Area title]\n\r", ch );
      return;
   }

   for( tarea = first_build; tarea; tarea = tarea->next )
   {
      if( !str_cmp( tarea->filename, arg ) )
      {
         if( argument && argument[0] != '\0' )
         {
            DISPOSE( tarea->name );
            tarea->name = str_dup( argument );
         }

         /*
          * Fold area with install flag -- auto-removes prototype flags 
          */
         send_to_char( "Saving and installing file...\n\r", ch );
         fold_area( tarea, tarea->filename, TRUE );

         /*
          * Remove from prototype area list 
          */
         UNLINK( tarea, first_build, last_build, next, prev );

         /*
          * Add to real area list 
          */
         LINK( tarea, first_area, last_area, next, prev );

         /*
          * Fix up author if online 
          */
         for( d = first_descriptor; d; d = d->next )
            if( d->character && d->character->pcdata && d->character->pcdata->area == tarea )
            {
               /*
                * remove area from author 
                */
               d->character->pcdata->area = NULL;
               /*
                * clear out author vnums  
                */
               d->character->pcdata->r_range_lo = 0;
               d->character->pcdata->r_range_hi = 0;
               d->character->pcdata->o_range_lo = 0;
               d->character->pcdata->o_range_hi = 0;
               d->character->pcdata->m_range_lo = 0;
               d->character->pcdata->m_range_hi = 0;
            }

         top_area++;
         send_to_char( "Writing area.lst...\n\r", ch );
         write_area_list(  );
         send_to_char( "Resetting new area.\n\r", ch );
         num = tarea->nplayer;
         tarea->nplayer = 0;
         reset_area( tarea );
         tarea->nplayer = num;
         send_to_char( "Renaming author's building file.\n\r", ch );
         sprintf( buf, "%s%s.installed", BUILD_DIR, tarea->filename );
         sprintf( arg, "%s%s", BUILD_DIR, tarea->filename );
         rename( arg, buf );
         send_to_char( "Done.\n\r", ch );
         return;
      }
   }
   send_to_char( "No such area exists.\n\r", ch );
   return;
}

void do_astat( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *tarea;
   GENERIC_MOB *genmob;
   bool proto, found;
   int pdeaths = 0, pkills = 0, mdeaths = 0, mkills = 0;
   found = FALSE;
   proto = FALSE;

   set_char_color( AT_PLAIN, ch );

   if( !str_cmp( "summary", argument ) )
   {
      for( tarea = first_area; tarea; tarea = tarea->next )
      {
         pdeaths += tarea->pdeaths;
         mdeaths += tarea->mdeaths;
         pkills += tarea->pkills;
         mkills += tarea->mkills;
      }
      ch_printf_color( ch, "&WTotal pdeaths:      &w%d\n\r", pdeaths );
      ch_printf_color( ch, "&WTotal pkills:       &w%d\n\r", pkills );
      ch_printf_color( ch, "&WTotal mdeaths:      &w%d\n\r", mdeaths );
      ch_printf_color( ch, "&WTotal mkills:       &w%d\n\r", mkills );
      return;
   }

   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( tarea->filename, argument ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
      for( tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, argument ) )
         {
            found = TRUE;
            proto = TRUE;
            break;
         }

   if( !found )
   {
      if( argument && argument[0] != '\0' )
      {
         send_to_char( "Area not found.  Check 'zones'.\n\r", ch );
         return;
      }
      else
      {
         for( tarea = first_build; tarea; tarea = tarea->next )
            if( tarea == ch->in_room->area )
               proto = TRUE;  /* It would lie before about a zone not being proto -- Scion */
         tarea = ch->in_room->area;
      }
   }

   ch_printf_color( ch, "\n\r&wName:     &W%s\n\r&wFilename: &W%-20s  &wPrototype: &W%s\n\r&wAuthor:   &W%s\n\r",
                    tarea->name, tarea->filename, proto ? "yes" : "no", tarea->author );
   ch_printf_color( ch, "&wAge: &W%-3d  &wCurrent number of players: &W%-3d  &wMax players: &W%d\n\r",
                    tarea->age, tarea->nplayer, tarea->max_players );
   if( !proto )
   {
      if( tarea->high_economy )
         ch_printf_color( ch, "&wArea economy: &W%d &wbillion and &W%d gold coins.\n\r",
                          tarea->high_economy, tarea->low_economy );
      else
         ch_printf_color( ch, "&wArea economy: &W%d &wgold coins.\n\r", tarea->low_economy );
      ch_printf_color( ch, "&wGold Looted:  &W%d   Current number of mobs: %d\n\r", tarea->gold_looted, tarea->nmob );
      ch_printf_color( ch, "&wMdeaths: &W%d   &wMkills: &W%d   &wPdeaths: &W%d   &wPkills: &W%d   &wIllegalPK: &W%d\n\r",
                       tarea->mdeaths, tarea->mkills, tarea->pdeaths, tarea->pkills, tarea->illegal_pk );
   }
   ch_printf_color( ch, "&wlow_room: &W%5d    &whi_room: &W%5d\n\r", tarea->low_r_vnum, tarea->hi_r_vnum );
   ch_printf_color( ch, "&wlow_obj : &W%5d    &whi_obj : &W%5d\n\r", tarea->low_o_vnum, tarea->hi_o_vnum );
   ch_printf_color( ch, "&wlow_mob : &W%5d    &whi_mob : &W%5d\n\r", tarea->low_m_vnum, tarea->hi_m_vnum );
   ch_printf( ch, "Index: %d      Recall: %d\n\r", tarea->low_soft_range, tarea->hi_soft_range );

/*    ch_printf_color( ch, "&wsoft range: &W%d - %d    &whard range: &W%d
- %d\n\r",
			tarea->low_soft_range, 
			tarea->hi_soft_range,
			tarea->low_hard_range, 
			tarea->hi_hard_range );
*/
   ch_printf_color( ch, "&wArea flags: &W%s\n\r", flag_string( tarea->flags, area_flags ) );
   ch_printf_color( ch, "&wWorld: &W%s\n\r", tarea->resetmsg ? tarea->resetmsg : "(default)" ); /* Rennard */
   if( tarea->city )
      ch_printf( ch, "&wCity: &W(%d) %s\n\r", tarea->reset_frequency, tarea->city->name );
   ch_printf( ch, "Generic mobs:\n\r" );
   for( genmob = tarea->first_race; genmob; genmob = genmob->next )
   {
      ch_printf( ch, "%s\n\r", genmob->nation->name );
   }
}


void do_aset( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *tarea;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   bool proto, found;
   int vnum, value;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   vnum = atoi( argument );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Usage: aset <area filename> <field> <value>\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( "  low_room hi_room low_obj hi_obj low_mob hi_mob\n\r", ch );
      send_to_char( "  name filename low_soft hi_soft low_hard hi_hard\n\r", ch );
      send_to_char( "  author world city flags\n\r", ch );
      return;
   }

   found = FALSE;
   proto = FALSE;
   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( tarea->filename, arg1 ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
      for( tarea = first_build; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, arg1 ) )
         {
            found = TRUE;
            proto = TRUE;
            break;
         }

   if( !found )
   {
      send_to_char( "Area not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      DISPOSE( tarea->name );
      tarea->name = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "filename" ) )
   {
      DISPOSE( tarea->filename );
      tarea->filename = str_dup( argument );
      write_area_list(  );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "race" ) )
   {
      NATION_DATA *nation;
      GENERIC_MOB *genmob;

      nation = find_nation( argument );
      if( !nation )
      {
         send_to_char( "No such nation.\n\r", ch );
         return;
      }
      CREATE( genmob, GENERIC_MOB, 1 );
      genmob->nation = nation;
      LINK( genmob, tarea->first_race, tarea->last_race, next, prev );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "low_economy" ) )
   {
      tarea->low_economy = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "high_economy" ) )
   {
      tarea->high_economy = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "low_room" ) )
   {
      tarea->low_r_vnum = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "hi_room" ) )
   {
      tarea->hi_r_vnum = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "low_obj" ) )
   {
      tarea->low_o_vnum = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "hi_obj" ) )
   {
      tarea->hi_o_vnum = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "low_mob" ) )
   {
      tarea->low_m_vnum = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "hi_mob" ) )
   {
      tarea->hi_m_vnum = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "low_soft" ) )
   {
      tarea->low_soft_range = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "recall" ) )
   {
      if( get_room_index( vnum ) == NULL )
      {
         send_to_char( "That is not an acceptable value.\n\r", ch );
         return;
      }

      tarea->hi_soft_range = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "low_hard" ) )
   {
      tarea->low_hard_range = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "hi_hard" ) )
   {
      tarea->hi_hard_range = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "author" ) )
   {
      STRFREE( tarea->author );
      tarea->author = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "world" ) )
   {
      if( tarea->resetmsg )
         DISPOSE( tarea->resetmsg );
      if( str_cmp( argument, "clear" ) )
         tarea->resetmsg = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }  /* Rennard */

   if( !str_cmp( arg2, "city" ) )
   {
      tarea->reset_frequency = vnum;
      send_to_char( "Done.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: aset <filename> flags <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg3 );
         value = get_areaflag( arg3 );
         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
         else
         {
            if( IS_SET( tarea->flags, 1 << value ) )
               REMOVE_BIT( tarea->flags, 1 << value );
            else
               SET_BIT( tarea->flags, 1 << value );
         }
      }
      return;
   }

   do_aset( ch, "" );
   return;
}


void do_rlist( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *room;
   int vnum;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   AREA_DATA *tarea;
   int lrange;
   int trange;

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) || !ch->pcdata || ( !ch->pcdata->area && !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) ) )
   {
      send_to_char_color( "&YYou don't have an assigned area.\n\r", ch );
      return;
   }

   tarea = ch->pcdata->area;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( tarea )
   {
      if( arg1[0] == '\0' )   /* cleaned a big scary mess */
         lrange = tarea->low_r_vnum;   /* here.     -Thoric */
      else
         lrange = atoi( arg1 );
      if( arg2[0] == '\0' )
         trange = tarea->hi_r_vnum;
      else
         trange = atoi( arg2 );

      if( ( lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum ) && !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
      {
         send_to_char_color( "&YThat is out of your vnum range.\n\r", ch );
         return;
      }
   }
   else
   {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
   }

   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) == NULL )
         continue;
      pager_printf( ch, "%5d) %s\n\r", vnum, room->name );
   }
   return;
}

void do_olist( CHAR_DATA * ch, char *argument )
{
   OBJ_INDEX_DATA *obj;
   int vnum;
   AREA_DATA *tarea;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int lrange;
   int trange;

   /*
    * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
    */

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char_color( "&YYou don't have an assigned area.\n\r", ch );
      return;
   }
   tarea = ch->pcdata->area;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( tarea )
   {
      if( arg1[0] == '\0' )   /* cleaned a big scary mess */
         lrange = tarea->low_o_vnum;   /* here.     -Thoric */
      else
         lrange = atoi( arg1 );
      if( arg2[0] == '\0' )
         trange = tarea->hi_o_vnum;
      else
         trange = atoi( arg2 );

   }
   else
   {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 3 );
   }

   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( obj = get_obj_index( vnum ) ) == NULL )
         continue;
      pager_printf( ch, "%5d) %-20s (%s)\n\r", vnum, obj->name, obj->short_descr );
   }
   return;
}

void do_mlist( CHAR_DATA * ch, char *argument )
{
   MOB_INDEX_DATA *mob;
   int vnum;
   AREA_DATA *tarea;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int lrange;
   int trange;

   set_pager_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) || !ch->pcdata )
   {
      send_to_char_color( "&YYou don't have an assigned area.\n\r", ch );
      return;
   }

   tarea = ch->pcdata->area;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( tarea )
   {
      if( arg1[0] == '\0' )   /* cleaned a big scary mess */
         lrange = tarea->low_m_vnum;   /* here.     -Thoric */
      else
         lrange = atoi( arg1 );
      if( arg2[0] == '\0' )
         trange = tarea->hi_m_vnum;
      else
         trange = atoi( arg2 );

   }
   else
   {
      lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
      trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
   }

   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( mob = get_mob_index( vnum ) ) == NULL )
         continue;
      pager_printf( ch, "%5d) %-20s '%s'\n\r", vnum, mob->player_name, mob->short_descr );
   }
}

void mpedit( CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, char *argument )
{
   if( mptype != -1 )
   {
      mprg->type = mptype;
      if( mprg->arglist )
         STRFREE( mprg->arglist );
      mprg->arglist = STRALLOC( argument );
   }
   ch->substate = SUB_MPROG_EDIT;
   ch->dest_buf = mprg;
   if( !mprg->comlist )
      mprg->comlist = STRALLOC( "" );
   start_editing( ch, mprg->comlist );
   return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
void do_mpedit( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mob's can't mpedit\n\r", ch );
      return;
   }

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_MPROG_EDIT:
         if( !ch->dest_buf )
         {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ch->dest_buf;
         if( mprog->comlist )
            STRFREE( mprog->comlist );
         mprog->comlist = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   value = atoi( arg3 );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: mpedit <victim> <command> [number] <program> <value>\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Command being one of:\n\r", ch );
      send_to_char( "  add delete insert edit list\n\r", ch );
      send_to_char( "Program being one of:\n\r", ch );
      send_to_char( "  act speech rand fight hitprcnt greet allgreet\n\r", ch );
      send_to_char( "  entry give bribe death time hour script\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "No one like that in all the realms.\n\r", ch );
      return;
   }

   if( !xIS_SET( victim->act, ACT_PROTOTYPE ) )
   {
      send_to_char( "A mobile must have a prototype flag to be mpset.\n\r", ch );
      return;
   }

   mprog = victim->pIndexData->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !str_cmp( arg2, "list" ) )
   {
      cnt = 0;
      if( !mprog )
      {
         send_to_char( "That mobile has no mob programs.\n\r", ch );
         return;
      }

      if( value < 1 )
      {
         if( strcmp( "full", arg3 ) )
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               ch_printf( ch, "%d>%s %s\n\r", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist );
            }

            return;
         }
         else
         {
            for( mprg = mprog; mprg; mprg = mprg->next )
            {
               ch_printf( ch, "%d>%s %s\n\r%s\n\r", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            }

            return;
         }
      }

      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            ch_printf( ch, "%d>%s %s\n\r%s\n\r", cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
            break;
         }
      }

      if( !mprg )
         send_to_char( "Program not found.\n\r", ch );

      return;
   }

   if( !str_cmp( arg2, "edit" ) )
   {
      if( !mprog )
      {
         send_to_char( "That mobile has no mob programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg4 );
      if( arg4[0] != '\0' )
      {
         mptype = get_mpflag( arg4 );
         if( mptype == -1 )
         {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
         }
      }
      else
         mptype = -1;
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mpedit( ch, mprg, mptype, argument );
            xCLEAR_BITS( victim->pIndexData->progtypes );
            for( mprg = mprog; mprg; mprg = mprg->next )
               xSET_BIT( victim->pIndexData->progtypes, mprg->type );
            return;
         }
      }
      send_to_char( "Program not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      int num;
      bool found;

      if( !mprog )
      {
         send_to_char( "That mobile has no mob programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg4 );
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = 0;
      found = FALSE;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = num = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
         if( mprg->type == mptype )
            num++;
      if( value == 1 )
      {
         mprg_next = victim->pIndexData->mudprogs;
         victim->pIndexData->mudprogs = mprg_next->next;
      }
      else
         for( mprg = mprog; mprg; mprg = mprg_next )
         {
            mprg_next = mprg->next;
            if( ++cnt == ( value - 1 ) )
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if( mprg_next )
      {
         STRFREE( mprg_next->arglist );
         STRFREE( mprg_next->comlist );
         DISPOSE( mprg_next );
         if( num <= 1 )
            xREMOVE_BIT( victim->pIndexData->progtypes, mptype );
         send_to_char( "Program removed.\n\r", ch );
      }
      return;
   }

   if( !str_cmp( arg2, "insert" ) )
   {
      if( !mprog )
      {
         send_to_char( "That mobile has no mob programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg4 );
      mptype = get_mpflag( arg4 );
      if( mptype == -1 )
      {
         send_to_char( "Unknown program type.\n\r", ch );
         return;
      }
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      if( value == 1 )
      {
         CREATE( mprg, MPROG_DATA, 1 );
         xSET_BIT( victim->pIndexData->progtypes, mptype );
         mpedit( ch, mprg, mptype, argument );
         mprg->next = mprog;
         victim->pIndexData->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value && mprg->next )
         {
            CREATE( mprg_next, MPROG_DATA, 1 );
            xSET_BIT( victim->pIndexData->progtypes, mptype );
            mpedit( ch, mprg_next, mptype, argument );
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      send_to_char( "Program not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "add" ) )
   {
      mptype = get_mpflag( arg3 );
      if( mptype == -1 )
      {
         send_to_char( "Unknown program type.\n\r", ch );
         return;
      }
      if( mprog != NULL )
         for( ; mprog->next; mprog = mprog->next );
      CREATE( mprg, MPROG_DATA, 1 );
      if( mprog )
         mprog->next = mprg;
      else
         victim->pIndexData->mudprogs = mprg;
      xSET_BIT( victim->pIndexData->progtypes, mptype );
      mpedit( ch, mprg, mptype, argument );
      mprg->next = NULL;
      return;
   }

   do_mpedit( ch, "" );
}

void do_opedit( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mob's can't opedit\n\r", ch );
      return;
   }

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_MPROG_EDIT:
         if( !ch->dest_buf )
         {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ch->dest_buf;
         if( mprog->comlist )
            STRFREE( mprog->comlist );
         mprog->comlist = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   value = atoi( arg3 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: opedit <object> <command> [number] <program> <value>\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Command being one of:\n\r", ch );
      send_to_char( "  add delete insert edit list\n\r", ch );
      send_to_char( "Program being one of:\n\r", ch );
      send_to_char( "  act speech rand wear remove zap get\n\r", ch );
      send_to_char( "  drop damage greet exa use\n\r", ch );
      send_to_char( "  pull push (for levers,pullchains,buttons)\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Object should be in your inventory to edit.\n\r", ch );
      return;
   }

   if( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "Nothing like that in all the realms.\n\r", ch );
      return;
   }

   if( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
   {
      send_to_char( "An object must have a prototype flag to be opset.\n\r", ch );
      return;
   }

   mprog = obj->pIndexData->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !str_cmp( arg2, "list" ) )
   {
      cnt = 0;
      if( !mprog )
      {
         send_to_char( "That object has no obj programs.\n\r", ch );
         return;
      }
      for( mprg = mprog; mprg; mprg = mprg->next )
         ch_printf( ch, "%d>%s %s\n\r%s\n\r", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
      return;
   }

   if( !str_cmp( arg2, "edit" ) )
   {
      if( !mprog )
      {
         send_to_char( "That object has no obj programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg4 );
      if( arg4[0] != '\0' )
      {
         mptype = get_mpflag( arg4 );
         if( mptype == -1 )
         {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
         }
      }
      else
         mptype = -1;
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mpedit( ch, mprg, mptype, argument );
            xCLEAR_BITS( obj->pIndexData->progtypes );
            for( mprg = mprog; mprg; mprg = mprg->next )
               xSET_BIT( obj->pIndexData->progtypes, mprg->type );
            return;
         }
      }
      send_to_char( "Program not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      int num;
      bool found;

      if( !mprog )
      {
         send_to_char( "That object has no obj programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg4 );
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = 0;
      found = FALSE;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = num = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
         if( mprg->type == mptype )
            num++;
      if( value == 1 )
      {
         mprg_next = obj->pIndexData->mudprogs;
         obj->pIndexData->mudprogs = mprg_next->next;
      }
      else
         for( mprg = mprog; mprg; mprg = mprg_next )
         {
            mprg_next = mprg->next;
            if( ++cnt == ( value - 1 ) )
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if( mprg_next )
      {
         STRFREE( mprg_next->arglist );
         STRFREE( mprg_next->comlist );
         DISPOSE( mprg_next );
         if( num <= 1 )
            xREMOVE_BIT( obj->pIndexData->progtypes, mptype );
         send_to_char( "Program removed.\n\r", ch );
      }
      return;
   }

   if( !str_cmp( arg2, "insert" ) )
   {
      if( !mprog )
      {
         send_to_char( "That object has no obj programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg4 );
      mptype = get_mpflag( arg4 );
      if( mptype == -1 )
      {
         send_to_char( "Unknown program type.\n\r", ch );
         return;
      }
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      if( value == 1 )
      {
         CREATE( mprg, MPROG_DATA, 1 );
         xSET_BIT( obj->pIndexData->progtypes, mptype );
         mpedit( ch, mprg, mptype, argument );
         mprg->next = mprog;
         obj->pIndexData->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value && mprg->next )
         {
            CREATE( mprg_next, MPROG_DATA, 1 );
            xSET_BIT( obj->pIndexData->progtypes, mptype );
            mpedit( ch, mprg_next, mptype, argument );
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      send_to_char( "Program not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "add" ) )
   {
      mptype = get_mpflag( arg3 );
      if( mptype == -1 )
      {
         send_to_char( "Unknown program type.\n\r", ch );
         return;
      }
      if( mprog != NULL )
         for( ; mprog->next; mprog = mprog->next );
      CREATE( mprg, MPROG_DATA, 1 );
      if( mprog )
         mprog->next = mprg;
      else
         obj->pIndexData->mudprogs = mprg;
      xSET_BIT( obj->pIndexData->progtypes, mptype );
      mpedit( ch, mprg, mptype, argument );
      mprg->next = NULL;
      return;
   }

   do_opedit( ch, "" );
}



/*
 * RoomProg Support
 */
void rpedit( CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, char *argument )
{
   if( mptype != -1 )
   {
      mprg->type = mptype;
      if( mprg->arglist )
         STRFREE( mprg->arglist );
      mprg->arglist = STRALLOC( argument );
   }
   ch->substate = SUB_MPROG_EDIT;
   ch->dest_buf = mprg;
   if( !mprg->comlist )
      mprg->comlist = STRALLOC( "" );
   start_editing( ch, mprg->comlist );
   return;
}

void do_rpedit( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color( AT_PLAIN, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mob's can't rpedit\n\r", ch );
      return;
   }

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_MPROG_EDIT:
         if( !ch->dest_buf )
         {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ch->dest_buf;
         if( mprog->comlist )
            STRFREE( mprog->comlist );
         mprog->comlist = copy_buffer( ch );
         stop_editing( ch );
         return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   value = atoi( arg2 );
   /*
    * argument = one_argument( argument, arg3 ); 
    */

   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: rpedit <command> [number] <program> <value>\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "Command being one of:\n\r", ch );
      send_to_char( "  add delete insert edit list\n\r", ch );
      send_to_char( "Program being one of:\n\r", ch );
      send_to_char( "  act speech rand sleep rest rfight enter\n\r", ch );
      send_to_char( "  leave death\n\r", ch );
      send_to_char( "\n\r", ch );
      send_to_char( "You should be standing in room you wish to edit.\n\r", ch );
      return;
   }

   mprog = ch->in_room->mudprogs;

   set_char_color( AT_GREEN, ch );

   if( !str_cmp( arg1, "list" ) )
   {
      cnt = 0;
      if( !mprog )
      {
         send_to_char( "This room has no room programs.\n\r", ch );
         return;
      }
      for( mprg = mprog; mprg; mprg = mprg->next )
         ch_printf( ch, "%d>%s %s\n\r%s\n\r", ++cnt, mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
      return;
   }

   if( !str_cmp( arg1, "edit" ) )
   {
      if( !mprog )
      {
         send_to_char( "This room has no room programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg3 );
      if( arg3[0] != '\0' )
      {
         mptype = get_mpflag( arg3 );
         if( mptype == -1 )
         {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
         }
      }
      else
         mptype = -1;
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mpedit( ch, mprg, mptype, argument );
            xCLEAR_BITS( ch->in_room->progtypes );
            for( mprg = mprog; mprg; mprg = mprg->next )
               xSET_BIT( ch->in_room->progtypes, mprg->type );
            return;
         }
      }
      send_to_char( "Program not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg1, "delete" ) )
   {
      int num;
      bool found;

      if( !mprog )
      {
         send_to_char( "That room has no room programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg3 );
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = 0;
      found = FALSE;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value )
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if( !found )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      cnt = num = 0;
      for( mprg = mprog; mprg; mprg = mprg->next )
         if( mprg->type == mptype )
            num++;
      if( value == 1 )
      {
         mprg_next = ch->in_room->mudprogs;
         ch->in_room->mudprogs = mprg_next->next;
      }
      else
         for( mprg = mprog; mprg; mprg = mprg_next )
         {
            mprg_next = mprg->next;
            if( ++cnt == ( value - 1 ) )
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if( mprg_next )
      {
         STRFREE( mprg_next->arglist );
         STRFREE( mprg_next->comlist );
         DISPOSE( mprg_next );
         if( num <= 1 )
            xREMOVE_BIT( ch->in_room->progtypes, mptype );
         send_to_char( "Program removed.\n\r", ch );
      }
      return;
   }

   if( !str_cmp( arg2, "insert" ) )
   {
      if( !mprog )
      {
         send_to_char( "That room has no room programs.\n\r", ch );
         return;
      }
      argument = one_argument( argument, arg3 );
      mptype = get_mpflag( arg2 );
      if( mptype == -1 )
      {
         send_to_char( "Unknown program type.\n\r", ch );
         return;
      }
      if( value < 1 )
      {
         send_to_char( "Program not found.\n\r", ch );
         return;
      }
      if( value == 1 )
      {
         CREATE( mprg, MPROG_DATA, 1 );
         xSET_BIT( ch->in_room->progtypes, mptype );
         mpedit( ch, mprg, mptype, argument );
         mprg->next = mprog;
         ch->in_room->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for( mprg = mprog; mprg; mprg = mprg->next )
      {
         if( ++cnt == value && mprg->next )
         {
            CREATE( mprg_next, MPROG_DATA, 1 );
            xSET_BIT( ch->in_room->progtypes, mptype );
            mpedit( ch, mprg_next, mptype, argument );
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      send_to_char( "Program not found.\n\r", ch );
      return;
   }

   if( !str_cmp( arg1, "add" ) )
   {
      mptype = get_mpflag( arg2 );
      if( mptype == -1 )
      {
         send_to_char( "Unknown program type.\n\r", ch );
         return;
      }
      if( mprog )
         for( ; mprog->next; mprog = mprog->next );
      CREATE( mprg, MPROG_DATA, 1 );
      if( mprog )
         mprog->next = mprg;
      else
         ch->in_room->mudprogs = mprg;
      xSET_BIT( ch->in_room->progtypes, mptype );
      mpedit( ch, mprg, mptype, argument );
      mprg->next = NULL;
      return;
   }

   do_rpedit( ch, "" );
}

void do_rdelete( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Delete which room?\n\r", ch );
      return;
   }

   /*
    * Find the room. 
    */
   if( ( location = find_location( ch, arg ) ) == NULL )
   {
      send_to_char( "No such location.\n\r", ch );
      return;
   }

   /*
    * We could go to the trouble of clearing out the room, but why? 
    */
   /*
    * Delete_room does that anyway, but this is probably safer:) 
    */
   if( location->first_person || location->first_content )
   {
      send_to_char( "The room must be empty first.\n\r", ch );
      return;
   }

   /*
    * Ok, we've determined that the room exists, it is empty and the 
    * player has the authority to delete it, so let's dump the thing. 
    * The function to do it is in db.c so it can access the top-room 
    * variable. 
    */
   delete_room( location );

   send_to_char( "Room deleted.\n\r", ch );
   return;
}

void do_odelete( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *obj;
   OBJ_DATA *temp;

   argument = one_argument( argument, arg );

   /*
    * Temporarily disable this command. 
    */
/*    return;*/

   if( arg[0] == '\0' )
   {
      send_to_char( "Delete which object?\n\r", ch );
      return;
   }

   /*
    * Find the object. 
    */
   if( !( obj = get_obj_index( atoi( arg ) ) ) )
   {
      if( !( temp = get_obj_here( ch, arg ) ) )
      {
         send_to_char( "No such object.\n\r", ch );
         return;
      }
      obj = temp->pIndexData;
   }

   /*
    * Ok, we've determined that the obj exists, it is empty and the 
    * player has the authority to delete it, so let's dump the thing. 
    * The function to do it is in db.c so it can access the top-obj 
    * variable. 
    */
   delete_obj( obj );

   send_to_char( "Object deleted.\n\r", ch );
   return;
}

void do_mdelete( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *mob;
   CHAR_DATA *temp;

   argument = one_argument( argument, arg );

   /*
    * Temporarily disable this command. 
    */
/*    return;*/

   if( arg[0] == '\0' )
   {
      send_to_char( "Delete which mob?\n\r", ch );
      return;
   }

   /*
    * Find the mob. 
    */
   if( !( mob = get_mob_index( atoi( arg ) ) ) )
   {
      if( !( temp = get_char_room( ch, arg ) ) || !IS_NPC( temp ) )
      {
         send_to_char( "No such mob.\n\r", ch );
         return;
      }
      mob = temp->pIndexData;
   }

   /*
    * Ok, we've determined that the mob exists and the player has the
    * authority to delete it, so let's dump the thing.
    * The function to do it is in db.c so it can access the top_mob_index
    * variable. 
    */
   delete_mob( mob );

   send_to_char( "Mob deleted.\n\r", ch );
   return;
}

/*
 * function to allow modification of an area's climate
 * Last modified: July 15, 1997
 * Fireblade
 */
void do_climate( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AREA_DATA *area;

   /*
    * Little error checking 
    */
   if( !ch )
   {
      bug( "do_climate: NULL character." );
      return;
   }
   else if( !ch->in_room )
   {
      bug( "do_climate: character not in a room." );
      return;
   }
   else if( !ch->in_room->area )
   {
      bug( "do_climate: character not in an area." );
      return;
   }
   else if( !ch->in_room->area->weather )
   {
      bug( "do_climate: area with NULL weather data." );
      return;
   }

   set_char_color( AT_BLUE, ch );

   area = ch->in_room->area;

   argument = strlower( argument );
   argument = one_argument( argument, arg );

   /*
    * Display current climate settings 
    */
   if( arg[0] == '\0' )
   {
      NEIGHBOR_DATA *neigh;

      ch_printf( ch, "%s:\n\r", area->name );
      ch_printf( ch, "\tTemperature:    %s\n\r", temp_settings[area->weather->climate_temp] );
      ch_printf( ch, "\tPrecipitation:  %s\n\r", precip_settings[area->weather->climate_precip] );
      ch_printf( ch, "\tWind:           %s\n\r", wind_settings[area->weather->climate_wind] );
      ch_printf( ch, "\tMana:           %s\n\r", mana_settings[area->weather->climate_mana] );

      if( area->weather->first_neighbor )
         ch_printf( ch, "\n\rNeighboring weather systems:\n\r" );

      for( neigh = area->weather->first_neighbor; neigh; neigh = neigh->next )
      {
         ch_printf( ch, "    %s\n\r", neigh->name );
      }

      return;
   }
   /*
    * set climate temperature 
    */
   else if( !str_cmp( arg, "temp" ) )
   {
      int i;
      argument = one_argument( argument, arg );

      for( i = 0; i < MAX_CLIMATE; i++ )
      {
         if( str_cmp( arg, temp_settings[i] ) )
            continue;

         area->weather->climate_temp = i;
         ch_printf( ch, "The climate temperature " "for %s is now %s.\n\r", area->name, temp_settings[i] );
         break;
      }

      if( i == MAX_CLIMATE )
      {
         ch_printf( ch, "Possible temperature " "settings:\n\r" );
         for( i = 0; i < MAX_CLIMATE; i++ )
         {
            ch_printf( ch, "\t%s\n\r", temp_settings[i] );
         }
      }

      return;
   }
   /*
    * set climate precipitation 
    */
   else if( !str_cmp( arg, "precip" ) )
   {
      int i;
      argument = one_argument( argument, arg );

      for( i = 0; i < MAX_CLIMATE; i++ )
      {
         if( str_cmp( arg, precip_settings[i] ) )
            continue;

         area->weather->climate_precip = i;
         ch_printf( ch, "The climate precipitation " "for %s is now %s.\n\r", area->name, precip_settings[i] );
         break;
      }

      if( i == MAX_CLIMATE )
      {
         ch_printf( ch, "Possible precipitation " "settings:\n\r" );
         for( i = 0; i < MAX_CLIMATE; i++ )
         {
            ch_printf( ch, "\t%s\n\r", precip_settings[i] );
         }
      }

      return;
   }
   /*
    * set climate wind 
    */
   else if( !str_cmp( arg, "wind" ) )
   {
      int i;
      argument = one_argument( argument, arg );

      for( i = 0; i < MAX_CLIMATE; i++ )
      {
         if( str_cmp( arg, wind_settings[i] ) )
            continue;

         area->weather->climate_wind = i;
         ch_printf( ch, "The climate wind for %s " "is now %s.\n\r", area->name, wind_settings[i] );
         break;
      }

      if( i == MAX_CLIMATE )
      {
         ch_printf( ch, "Possible wind settings:\n\r" );
         for( i = 0; i < MAX_CLIMATE; i++ )
         {
            ch_printf( ch, "\t%s\n\r", wind_settings[i] );
         }
      }

      return;
   }
   /*
    * add or remove neighboring weather systems 
    */
   else if( !str_cmp( arg, "neighbor" ) )
   {
      NEIGHBOR_DATA *neigh;
      AREA_DATA *tarea;

      if( argument[0] == '\0' )
      {
         ch_printf( ch, "Add or remove which area?\n\r" );
         return;
      }

      /*
       * look for a matching list item 
       */
      for( neigh = area->weather->first_neighbor; neigh; neigh = neigh->next )
      {
         if( nifty_is_name( argument, neigh->name ) )
            break;
      }

      /*
       * if the a matching list entry is found, remove it 
       */
      if( neigh )
      {
         /*
          * look for the neighbor area in question 
          */
         if( !( tarea = neigh->address ) )
            tarea = get_area( neigh->name );

         /*
          * if there is an actual neighbor area 
          */
         /*
          * remove its entry to this area 
          */
         if( tarea )
         {
            NEIGHBOR_DATA *tneigh;

            tarea = neigh->address;
            for( tneigh = tarea->weather->first_neighbor; tneigh; tneigh = tneigh->next )
            {
               if( !strcmp( area->name, tneigh->name ) )
                  break;
            }

            UNLINK( tneigh, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
            STRFREE( tneigh->name );
            DISPOSE( tneigh );
         }

         UNLINK( neigh, area->weather->first_neighbor, area->weather->last_neighbor, next, prev );
         ch_printf( ch, "The weather in %s and %s " "no longer affect each other.\n\r", neigh->name, area->name );
         STRFREE( neigh->name );
         DISPOSE( neigh );
      }
      /*
       * otherwise add an entry 
       */
      else
      {
         tarea = get_area( argument );

         if( !tarea )
         {
            ch_printf( ch, "No such area exists.\n\r" );
            return;
         }
         else if( tarea == area )
         {
            ch_printf( ch, "%s already affects its " "own weather.\n\r", area->name );
            return;
         }

         /*
          * add the entry 
          */
         CREATE( neigh, NEIGHBOR_DATA, 1 );
         neigh->name = STRALLOC( tarea->name );
         neigh->address = tarea;
         LINK( neigh, area->weather->first_neighbor, area->weather->last_neighbor, next, prev );

         /*
          * add an entry to the neighbor's list 
          */
         CREATE( neigh, NEIGHBOR_DATA, 1 );
         neigh->name = STRALLOC( area->name );
         neigh->address = area;
         LINK( neigh, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );

         ch_printf( ch, "The weather in %s and %s now " "affect one another.\n\r", tarea->name, area->name );
      }

      return;
   }
   /*
    * set climate mana 
    */
   else if( !str_cmp( arg, "mana" ) )
   {
      int i;
      argument = one_argument( argument, arg );

      for( i = 0; i < MAX_CLIMATE; i++ )
      {
         if( str_cmp( arg, mana_settings[i] ) )
            continue;

         area->weather->climate_mana = i;
         ch_printf( ch, "The climate mana " "for %s is now %s.\n\r", area->name, mana_settings[i] );
         break;
      }

      if( i == MAX_CLIMATE )
      {
         ch_printf( ch, "Possible mana " "settings:\n\r" );
         for( i = 0; i < MAX_CLIMATE; i++ )
         {
            ch_printf( ch, "\t%s\n\r", mana_settings[i] );
         }
      }

      return;
   }
   else
   {
      ch_printf( ch, "Climate may only be followed by one " "of the following fields:\n\r" );
      ch_printf( ch, "\ttemp\n\r" );
      ch_printf( ch, "\tprecip\n\r" );
      ch_printf( ch, "\twind\n\r" );
      ch_printf( ch, "\tneighbor\n\r" );
      ch_printf( ch, "\tmana\n\t" );

      return;
   }
}

/* displays all exits from the area that the imm is physically in - shogar */
void do_aexit( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *room;
   int i, vnum;
   AREA_DATA *tarea;
   EXIT_DATA *pexit;
   int lrange;
   int trange;

   tarea = ch->in_room->area;
   trange = tarea->hi_r_vnum;
   lrange = tarea->low_r_vnum;
   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) == NULL )
         continue;
      for( i = 0; i < MAX_DIR + 1; i++ )
      {
         if( ( pexit = get_exit( room, i ) ) == NULL )
            continue;
         if( pexit->to_room->area != tarea )
         {
            pager_printf( ch, "To: %-20.20s %s\n\r", pexit->to_room->area->filename, pexit->to_room->area->name );
            pager_printf( ch, "%15d %-30.30s -> %15d (%s)\n\r", vnum, room->name, pexit->vnum, dir_name[i] );
         }
      }
   }
}

/* displays all entrances to the area that the imm is physically in - shogar */
void do_aentrance( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *room;
   int i, vnum;
   AREA_DATA *tarea;
   AREA_DATA *otherarea;

   EXIT_DATA *pexit;
   int lrange;
   int trange;

   tarea = ch->in_room->area;
   for( otherarea = first_area; otherarea; otherarea = otherarea->next )
   {
      if( tarea == otherarea )
         continue;
      trange = otherarea->hi_r_vnum;
      lrange = otherarea->low_r_vnum;
      for( vnum = lrange; vnum <= trange; vnum++ )
      {

         if( ( room = get_room_index( vnum ) ) == NULL )
            continue;
         for( i = 0; i < MAX_DIR + 1; i++ )
         {
            if( ( pexit = get_exit( room, i ) ) == NULL )
               continue;
            if( pexit->to_room->area == tarea )
            {
               pager_printf( ch, "From: %-20.20s %s\n\r", otherarea->filename, otherarea->name );
               pager_printf( ch, "%15d %-30.30s -> %15d (%s)\n\r", vnum, room->name, pexit->vnum, dir_name[i] );
            }
         }
      }
   }
}
