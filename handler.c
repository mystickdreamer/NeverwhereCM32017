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
 * ------------------------------------------------------------------------ *
 *		        Main structure manipulation module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


extern int top_exit;
extern int top_ed;
extern int top_affect;
extern int cur_qobjs;
extern int cur_qchars;
extern CHAR_DATA *gch_prev;
extern OBJ_DATA *gobj_prev;

extern char *get_weather_string( CHAR_DATA * ch, char *weather );

CHAR_DATA *cur_char;
ROOM_INDEX_DATA *cur_room;
bool cur_char_died;
ch_ret global_retcode;

int cur_obj;
int cur_obj_serial;
bool cur_obj_extracted;
obj_ret global_objcode;

OBJ_DATA *group_object( OBJ_DATA * obj1, OBJ_DATA * obj2 );
bool in_magic_container( OBJ_DATA * obj );
void delete_reset( RESET_DATA * pReset );
NATION_DATA *find_nation( char *name );

/* Display a confused message -keo */
void huh( CHAR_DATA * ch )
{
   if( IS_NPC( ch ) )
   {
      act( AT_ACTION, "$n looks confused.", ch, NULL, NULL, TO_ROOM );
      return;
   }

   switch ( number_range( 1, 6 ) )
   {
      default:
         send_to_char( "???\n\r", ch );
         break;
      case 1:
         send_to_char( "Huh?\n\r", ch );
         break;
      case 2:
         ch_printf( ch, "What%s?\n\r", ( number_range( 1, 2 ) == 1 ? " was that again" : "" ) );
         break;
      case 3:
            send_to_char( "Nothing happens.\n\r", ch );
         break;
      case 4:
         ch_printf( ch, "You can't do that%s.\n\r", ( number_range( 1, 2 ) == 1 ? " here" : "" ) );
         break;
      case 5:
         ch_printf( ch, "Sorry, I didn't%s catch that.\n\r", ( number_range( 1, 2 ) == 1 ? " quite" : "" ) );
         break;
   }
}

/* Evaluate a character for comparison with other characters */
int get_phys_worth( CHAR_DATA * ch )
{
   int worth = 0;
   int i;

   worth += get_curr_str( ch );
   worth += get_curr_dex( ch );
   worth += get_curr_con( ch );
   worth += get_curr_per( ch );
   worth += get_curr_end( ch );

   worth += GET_HITROLL( ch );
   worth += GET_DAMROLL( ch );
   worth += GET_AC( ch );

   if( !IS_NPC( ch ) )
      for( i = 0; i < 20; i++ )
      {
         worth += ch->pcdata->weapon[i];
         worth += ch->pcdata->noncombat[i];
      }

   if( IS_NPC( ch ) )
      worth *= 2;

   return worth;
}

/* Return the number of lights in a room -- Scion */
int get_light_room( ROOM_INDEX_DATA * room )
{
   CHAR_DATA *ch;
   OBJ_DATA *obj;
   AFFECT_DATA *af;
   MOON_DATA *moon;
   int light = 0;

   for( ch = room->first_person; ch; ch = ch->next_in_room )
      light += get_light_char( ch );

   for( af = room->first_affect; af; af = af->next )
   {
      if( af->location == APPLY_ROOMLIGHT )
         light++;
   }

   if( !IS_SET( room->room_flags, ROOM_INDOORS ) )
   {
      moon = first_moon;
      while( moon )
      {
         if( moon->up && !str_cmp( moon->world, room->area->resetmsg ) )
            light += moon->phase;
         moon = moon->next;
      }
   }

   for( obj = room->first_content; obj; obj = obj->next_content )
   {
      if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
         light += obj->count;
      if( IS_OBJ_STAT( obj, ITEM_DARK ) )
         light -= obj->count;
   }

   return light;
}

/* Return the number of lights a character is wearing -- Scion */
int get_light_char( CHAR_DATA * ch )
{
   OBJ_DATA *obj;
   int light = 0;

   if( char_died( ch ) || !ch->in_room )
      return 0;

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
         light += obj->count;
      if( IS_OBJ_STAT( obj, ITEM_DARK ) )
         light -= obj->count;
   }

   /*
    * added at Valerezal's request -keo 
    */
   if( IS_AFFECTED( ch, AFF_GLOW ) )
      light += 5;
   if( IS_AFFECTED( ch, AFF_DARK ) )
      light -= 5;

   return light;
}

int moon_visible( AREA_DATA * tarea, int flag )
{
   MOON_DATA *moon;
   int amt = 0;

   if( !str_cmp( tarea->resetmsg, "Void" ) )
      return 0;
   moon = first_moon;
   while( moon )
   {
      if( moon->world == NULL || moon->world[0] == '\0' )
         continue;
      if( moon->type == flag && moon->up && !str_cmp( moon->world, tarea->resetmsg ) )
         amt += moon->phase;
      moon = moon->next;
   }
   return amt;
}

bool is_bane( OBJ_DATA * weapon, CHAR_DATA * victim )
{
   if( !weapon )
      return FALSE;
   if( !xIS_SET( weapon->extra_flags, ITEM_MAGIC ) )
      return FALSE;
   if( weapon->value[0] )
   {
      switch ( weapon->value[0] )
      {
         case BANE_UNDEAD:
            if( IS_UNDEAD( victim ) )
               return TRUE;
            break;
         case BANE_DEMON:
            if( IS_AFFECTED( victim, AFF_DEMON ) )
               return TRUE;
            break;
         case BANE_HUMAN:
            if( victim->race == 0 )
               return TRUE;
            break;
         case BANE_ELF:
            if( victim->race == 1 )
               return TRUE;
            break;
         case BANE_DWARF:
            if( victim->race == 2 )
               return TRUE;
            break;
         case BANE_FAERIE:
            if( victim->race == 4 )
               return TRUE;
            break;
         case BANE_GIANT:
            if( victim->race == 6 )
               return TRUE;
            break;
         case BANE_ORC:
            if( victim->race == 7 )
               return TRUE;
            break;
         case BANE_TROLL:
            if( victim->race == 8 )
               return TRUE;
            break;
         case BANE_MINOTAUR:
            if( victim->race == 15 )
               return TRUE;
            break;
         case BANE_HALFLING:
            if( victim->race == 18 )
               return TRUE;
            break;
         case BANE_CENTAUR:
            if( victim->race == 20 )
               return TRUE;
            break;
         case BANE_FELINE:
            if( IS_AFFECTED( victim, AFF_FELINE ) )
               return TRUE;
            break;
         case BANE_DRAGON:
            if( victim->race == 35 )
               return TRUE;
            break;
         case BANE_CONSTRUCT:
            if( IS_AFFECTED( victim, AFF_CONSTRUCT ) )
               return TRUE;
            break;
         case BANE_ANGEL:
            if( IS_AFFECTED( victim, AFF_ANGEL ) )
               return TRUE;
            break;
         default:
            weapon->value[0] = 0;
            weapon->value[6] = 0;
            return FALSE;
      }
   }
   else
   {
      weapon->value[6] = 0;
      if( IS_UNDEAD( victim ) )
         weapon->value[0] = BANE_UNDEAD;
      else if( IS_AFFECTED( victim, AFF_DEMON ) )
         weapon->value[0] = BANE_DEMON;
      else if( IS_AFFECTED( victim, AFF_FELINE ) )
         weapon->value[0] = BANE_FELINE;
      else if( IS_AFFECTED( victim, AFF_CONSTRUCT ) )
         weapon->value[0] = BANE_CONSTRUCT;
      else if( IS_AFFECTED( victim, AFF_ANGEL ) )
         weapon->value[0] = BANE_ANGEL;
      else
         switch ( victim->race )
         {
            case 0:
               weapon->value[0] = BANE_HUMAN;
               break;
            case 1:
               weapon->value[0] = BANE_ELF;
               break;
            case 2:
               weapon->value[0] = BANE_DWARF;
               break;
            case 4:
               weapon->value[0] = BANE_FAERIE;
               break;
            case 6:
               weapon->value[0] = BANE_GIANT;
               break;
            case 7:
               weapon->value[0] = BANE_ORC;
               break;
            case 8:
               weapon->value[0] = BANE_TROLL;
               break;
            case 15:
               weapon->value[0] = BANE_MINOTAUR;
               break;
            case 18:
               weapon->value[0] = BANE_HALFLING;
               break;
            case 20:
               weapon->value[0] = BANE_CENTAUR;
               break;
            case 35:
               weapon->value[0] = BANE_DRAGON;
               break;
         }
   }
   return FALSE;
}

int get_magic_worth( CHAR_DATA * ch )
{
   int worth = 0;
   int i;

   worth += get_curr_int( ch );
   worth += get_curr_wil( ch );

   for( i = 0; i < MAX_DEITY; i++ )
   {
      worth += TALENT( ch, i );
      worth += ch->curr_talent[i];
      worth += ch->talent[i];
   }

   return worth;
}

int get_char_worth( CHAR_DATA * ch )
{
   int worth = 0;
   worth += get_phys_worth( ch );
   worth += get_magic_worth( ch );
   return worth;
}

/*
 * Return how much exp a char has
 */
int get_exp( CHAR_DATA * ch )
{
   return ch->exp;
}

/*
 * Calculate roughly how much experience a character is worth
 */
int get_exp_worth( CHAR_DATA * ch )
{
   int exp;

   exp = get_curr_dex( ch ) * 50;
   exp += ch->max_hit;
   exp += ( ch->barenumdie * ch->baresizedie + GET_DAMROLL( ch ) );
   exp += GET_HITROLL( ch ) * get_curr_str( ch );
   if( IS_NPC( ch ) && ch->spec_fun )
   {
      exp += exp * 2;
      exp += get_curr_int( ch ) * 10;
   }
   exp = URANGE( MIN_EXP_WORTH, exp, MAX_EXP_WORTH );

   return exp;
}

sh_int get_exp_base( CHAR_DATA * ch )
{
   return sysdata.exp_base;
}

/*								-Thoric
 * Return how much experience is required for ch to get to a certain level
 */
int exp_level( CHAR_DATA * ch, sh_int level )
{
   int lvl;

   lvl = UMAX( 0, level - 1 );
   return ( lvl * lvl * lvl * get_exp_base( ch ) );
}

/* Return how much XP it will take to raise a certain talent */
int talent_exp( CHAR_DATA * ch, sh_int level, int tal )
{
   int lvl;
   DEITY_DATA *talent;
   for( talent = first_deity; talent; talent = talent->next )
   {
      if( talent->index == tal )
         break;
   }
   if( !talent )
      return 0;

   lvl = UMAX( 0, level - 1 );
   return ( lvl * lvl * lvl * get_exp_base( ch ) * talent->cost );
}

/*
 * Get what level ch is based on exp
 */
int level_exp( CHAR_DATA * ch, int exp )
{
   return 0;
}

void learn_weapon( CHAR_DATA * ch, int i )
{
   if( !IS_NPC( ch ) && ch->pcdata->weapon[i] < 100 )
   {
      ch->pcdata->weapon_exp[i] += ( int )ch->exp / 20;
      ch->exp -= ( int )ch->exp / 20;
      if( ch->pcdata->weapon_exp[i] + exp_level( ch, ch->pcdata->weapon[i] ) >= exp_level( ch, ch->pcdata->weapon[i] + 1 ) )
      {

         ch->pcdata->weapon_exp[i] -=
            ( exp_level( ch, ch->pcdata->weapon[i] + 1 ) - exp_level( ch, ch->pcdata->weapon[i] ) );
         ch->pcdata->weapon[i]++;
         ch_printf( ch, "&BYou feel your %s skill has improved.\n\r", weapon_skill[i] );
         if( ch->pcdata->weapon[i] == 100 )
         {
            ch_printf( ch, "&BYou are now a master of %s.\n\r", weapon_skill[i] );
         }
      }
   }
}

void learn_noncombat( CHAR_DATA * ch, int i )
{
   if( !IS_NPC( ch ) && ch->pcdata->noncombat[i] < 100 )
   {
      ch->pcdata->noncombat_exp[i] += ( int )ch->exp / 5;
      ch->exp -= ( int )ch->exp / 5;
      if( ch->pcdata->noncombat_exp[i] +
          exp_level( ch, ch->pcdata->noncombat[i] ) >= exp_level( ch, ch->pcdata->noncombat[i] + 1 ) )
      {

         ch->pcdata->noncombat_exp[i] -=
            ( exp_level( ch, ch->pcdata->noncombat[i] + 1 ) - exp_level( ch, ch->pcdata->noncombat[i] ) );
         ch->pcdata->noncombat[i]++;
         ch_printf( ch, "You feel your %s skill has improved.\n\r", noncombat_skill[i] );
         if( ch->pcdata->noncombat[i] == 100 )
         {
            ch_printf( ch, "You are now a master of %s.\n\r", noncombat_skill[i] );
         }
      }
      if( ch->exp < 5000 )
         ch_printf( ch, "You are getting bored with this.\n\r", ch );
   }
}


/*
 * Retrieve character's current strength.
 */
sh_int get_curr_str( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_str + ch->mod_str;
   if( ch->nation )
      stat += ch->nation->str_mod;

   if( ch->singing == SONG_STRENGTH )
      stat += TALENT( ch, TAL_SPEECH ) / 10;

   return URANGE( 1, stat, 20000 );
}



/*
 * Retrieve character's current intelligence.
 */
sh_int get_curr_int( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_int + ch->mod_int;
   if( ch->nation )
      stat += ch->nation->int_mod;

   return URANGE( 1, stat, 20000 );
}


/*
 * Retrieve character's current wisdom.
 */
sh_int get_curr_wis( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_wil + ch->mod_wis;
   if( ch->nation )
      stat += ch->nation->wis_mod;

   return URANGE( 1, stat, 20000 );
}

/*
 * Retrieve character's current willpower.
 */
sh_int get_curr_wil( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_wil + ch->mod_wis;
   if( ch->nation )
      stat += ch->nation->wis_mod;

   return URANGE( 1, stat, 20000 );
}


/*
 * Retrieve character's current dexterity.
 */
sh_int get_curr_dex( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_dex + ch->mod_dex;
   if( ch->nation )
      stat += ch->nation->dex_mod;

   return URANGE( 1, stat, 20000 );
}


/*
 * Retrieve character's current constitution.
 */
sh_int get_curr_con( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_con + ch->mod_con;
   if( ch->nation )
      stat += ch->nation->con_mod;

   if( ch->singing == SONG_VIGOR )
      stat += TALENT( ch, TAL_SPEECH ) / 10;

   return URANGE( 1, stat, 20000 );
}

/*
 * Retrieve character's current charisma.
 */
sh_int get_curr_cha( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_per + ch->mod_cha;
   if( ch->nation )
      stat += ch->nation->cha_mod;

   if( ch->singing == SONG_CHARM )
      stat += TALENT( ch, TAL_SPEECH ) / 10;

   return URANGE( 1, stat, 20000 );
}

/*
 * Retrieve character's current perception.
 */
sh_int get_curr_per( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_per + ch->mod_cha;
   if( ch->nation )
      stat += ch->nation->cha_mod;

   if( ch->singing == SONG_CHARM )
      stat += TALENT( ch, TAL_SPEECH ) / 10;

   return URANGE( 1, stat, 20000 );
}

/*
 * Retrieve character's current endurance.
 */
sh_int get_curr_end( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_lck + ch->mod_lck;
   if( ch->nation )
      stat += ch->nation->lck_mod;

   return URANGE( 1, stat, 20000 );
}

/*
 * Retrieve character's current luck.
 */
sh_int get_curr_lck( CHAR_DATA * ch )
{
   sh_int stat;

   stat = ch->perm_lck + ch->mod_lck;
   if( ch->nation )
      stat += ch->nation->lck_mod;

   return URANGE( 1, stat, 20000 );
}


/*
 * Retrieve a character's carry capacity.
 * Vastly reduced (finally) due to containers		-Thoric
 */
int can_carry_n( CHAR_DATA * ch )
{
   int penalty = 0;

   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PET ) )
      return 0;

   if( get_eq_char( ch, WEAR_HAND ) )
      ++penalty;
   if( get_eq_char( ch, WEAR_HAND2 ) )
      ++penalty;
   if( get_eq_char( ch, WEAR_SHIELD ) )
      ++penalty;
   penalty -= TALENT( ch, TAL_MOTION ) / 10;
   return URANGE( 5, ( get_curr_dex( ch ) / 5 - penalty ), 50 );
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA * ch )
{
   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PET ) )
      return 0;

   return get_curr_str( ch ) * 10 + ch->weight + TALENT( ch, TAL_MOTION ) * 10;
}


/*
 * See if a player/mob can take a piece of prototype eq		-Thoric
 */
bool can_take_proto( CHAR_DATA * ch )
{
   if( IS_SET( ch->pcdata->permissions, PERMIT_ITEM ) )
      return TRUE;
   else if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PROTOTYPE ) )
      return TRUE;
   else
      return FALSE;
}


/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *str, char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_cmp( str, name ) )
         return TRUE;
   }
}

bool is_name_prefix( const char *str, char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_prefix( str, name ) )
         return TRUE;
   }
}

/*
 * See if a string is one of the names of an object.		-Thoric
 * Treats a dash as a word delimiter as well as a space
 */
bool is_name2( const char *str, char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument2( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_cmp( str, name ) )
         return TRUE;
   }
}

bool is_name2_prefix( const char *str, char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   for( ;; )
   {
      namelist = one_argument2( namelist, name );
      if( name[0] == '\0' )
         return FALSE;
      if( !str_prefix( str, name ) )
         return TRUE;
   }
}

/*								-Thoric
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name( char *str, char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   if( !str || str[0] == '\0' )
      return FALSE;
   if( !namelist || namelist[0] == '\0' )
      return FALSE;

   for( ;; )
   {
      str = one_argument2( str, name );
      if( name[0] == '\0' )
         return TRUE;
      if( !is_name2( name, namelist ) )
         return FALSE;
   }
}

bool nifty_is_name_prefix( char *str, char *namelist )
{
   char name[MAX_INPUT_LENGTH];

   if( !str || str[0] == '\0' )
      return FALSE;

   for( ;; )
   {
      str = one_argument2( str, name );
      if( name[0] == '\0' )
         return TRUE;
      if( !is_name2_prefix( name, namelist ) )
         return FALSE;
   }
}

void room_affect( ROOM_INDEX_DATA * pRoomIndex, AFFECT_DATA * paf, bool fAdd )
{
   if( fAdd )
   {
      switch ( paf->location )
      {
         case APPLY_ROOMFLAG:
         case APPLY_SECTORTYPE:
            break;
         case APPLY_TELEVNUM:
         case APPLY_TELEDELAY:
            break;
      }
   }
   else
   {
      switch ( paf->location )
      {
         case APPLY_ROOMFLAG:
         case APPLY_SECTORTYPE:
            break;
         case APPLY_TELEVNUM:
         case APPLY_TELEDELAY:
            break;
      }
   }
}

/*
 * Modify a skill (hopefully) properly			-Thoric
 *
 * On "adding" a skill modifying affect, the value set is unimportant
 * upon removing the affect, the skill it enforced to a proper range.
 */
void modify_skill( CHAR_DATA * ch, int sn, int mod, bool fAdd )
{
   if( !IS_NPC( ch ) )
   {
      if( fAdd )
         ch->pcdata->learned[sn] += mod;
      else
         ch->pcdata->learned[sn] = URANGE( 0, ch->pcdata->learned[sn] + mod, GET_ADEPT( ch, sn ) );
   }
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd )
{
   OBJ_DATA *wield;
   int mod;
   struct skill_type *skill;
   ch_ret retcode;

   if( !paf )  /* not sure why this is being passed null, but its a prob-shogar */
   {
      bug( "affect modify on %s", ch->name );
      return;
   }
   if( !ch && !ch->name )
      return;

   mod = paf->modifier;

   if( fAdd )
   {
      xSET_BITS( ch->affected_by, paf->bitvector );
   }
   else
   {
      xREMOVE_BITS( ch->affected_by, paf->bitvector );
      /*
       * might be an idea to have a duration removespell which returns
       * the spell after the duration... but would have to store
       * the removed spell's information somewhere...    -Thoric
       * (Though we could keep the affect, but disable it for a duration)
       */
      if( ( paf->location % REVERSE_APPLY ) == APPLY_REMOVESPELL )
         return;

      switch ( paf->location % REVERSE_APPLY )
      {
         case APPLY_AFFECT:
            REMOVE_BIT( ch->affected_by.bits[0], mod );
            return;
         case APPLY_EXT_AFFECT:
            xREMOVE_BIT( ch->affected_by, mod );
            return;
         case APPLY_RESISTANT:
            REMOVE_BIT( ch->resistant, mod );
            return;
         case APPLY_IMMUNE:
            REMOVE_BIT( ch->immune, mod );
            return;
         case APPLY_SUSCEPTIBLE:
            REMOVE_BIT( ch->susceptible, mod );
            return;
         case APPLY_WEARSPELL:  /* affect only on wear */
            return;
         case APPLY_REMOVE:
            SET_BIT( ch->affected_by.bits[0], mod );
            return;
      }
      mod = 0 - mod;
   }

   switch ( paf->location % REVERSE_APPLY )
   {
      default:
         bug( "Affect_modify: unknown location %d.", paf->location );
         return;

      case APPLY_NONE:
         break;
      case APPLY_STR:
         ch->mod_str += mod;
         break;
      case APPLY_DEX:
         ch->mod_dex += mod;
         break;
      case APPLY_INT:
         ch->mod_int += mod;
         break;
      case APPLY_WIS:
         ch->mod_wis += mod;
         break;
      case APPLY_CON:
         ch->mod_con += mod;
         break;
      case APPLY_CHA:
         ch->mod_cha += mod;
         break;
      case APPLY_LCK:
         ch->mod_lck += mod;
         break;
      case APPLY_SEX:
         ch->sex = ( ch->sex + mod ) % 3;
         if( ch->sex < 0 )
            ch->sex += 2;
         ch->sex = URANGE( 0, ch->sex, 2 );
         break;

         /*
          * These are unused due to possible problems.  Enable at your own risk.
          */
      case APPLY_AGE:
         break;
      case APPLY_GOLD:
         break;
      case APPLY_EXP:
         break;

         /*
          * Regular apply types
          */
      case APPLY_HEIGHT:
         ch->height += mod;
         break;
      case APPLY_WEIGHT:
         ch->weight += mod;
         break;
      case APPLY_MANA:
         ch->base_mana += mod;
         break;
      case APPLY_HIT:
         ch->base_hit += mod;
         break;
      case APPLY_MOVE:
         ch->base_move += mod;
         break;
      case APPLY_AC:
         ch->armor += mod;
         break;
      case APPLY_HITROLL:
         ch->hitroll += mod;
         break;
      case APPLY_DAMROLL:
         ch->damroll += mod;
         break;
      case APPLY_SAVING_POISON:
         ch->saving_poison_death += mod;
         break;
      case APPLY_SAVING_ROD:
         ch->saving_wand += mod;
         break;
      case APPLY_SAVING_PARA:
         ch->saving_para_petri += mod;
         break;
      case APPLY_SAVING_BREATH:
         ch->saving_breath += mod;
         break;
      case APPLY_SAVING_SPELL:
         ch->saving_spell_staff += mod;
         break;

         /*
          * Talent apply types
          */
      case APPLY_FIRE_MAGIC:
         ch->mod_talent[TAL_FIRE] += mod;
         break;
      case APPLY_EARTH_MAGIC:
         ch->mod_talent[TAL_EARTH] += mod;
         break;
      case APPLY_WIND_MAGIC:
         ch->mod_talent[TAL_WIND] += mod;
         break;
      case APPLY_FROST_MAGIC:
         ch->mod_talent[TAL_FROST] += mod;
         break;
      case APPLY_LIGHTNING_MAGIC:
         ch->mod_talent[TAL_LIGHTNING] += mod;
         break;
      case APPLY_WATER_MAGIC:
         ch->mod_talent[TAL_WATER] += mod;
         break;
      case APPLY_DREAM_MAGIC:
         ch->mod_talent[TAL_DREAM] += mod;
         break;
      case APPLY_SPEECH_MAGIC:
         ch->mod_talent[TAL_SPEECH] += mod;
         break;
      case APPLY_HEALING_MAGIC:
         ch->mod_talent[TAL_HEALING] += mod;
         break;
      case APPLY_DEATH_MAGIC:
         ch->mod_talent[TAL_DEATH] += mod;
         break;
      case APPLY_CHANGE_MAGIC:
         ch->mod_talent[TAL_CHANGE] += mod;
         break;
      case APPLY_TIME_MAGIC:
         ch->mod_talent[TAL_TIME] += mod;
         break;
      case APPLY_MOTION_MAGIC:
         ch->mod_talent[TAL_MOTION] += mod;
         break;
      case APPLY_MIND_MAGIC:
         ch->mod_talent[TAL_MIND] += mod;
         break;
      case APPLY_ILLUSION_MAGIC:
         ch->mod_talent[TAL_ILLUSION] += mod;
         break;
      case APPLY_SEEKING_MAGIC:
         ch->mod_talent[TAL_SEEKING] += mod;
         break;
      case APPLY_SECURITY_MAGIC:
         ch->mod_talent[TAL_SECURITY] += mod;
         break;
      case APPLY_CATALYSM_MAGIC:
         ch->mod_talent[TAL_CATALYSM] += mod;
         break;
      case APPLY_VOID_MAGIC:
         ch->mod_talent[TAL_VOID] += mod;
         break;

         /*
          * Bitvector modifying apply types
          */
      case APPLY_AFFECT:
         SET_BIT( ch->affected_by.bits[0], mod );
         break;
      case APPLY_EXT_AFFECT:
         xSET_BIT( ch->affected_by, mod );
         break;
      case APPLY_RESISTANT:
         SET_BIT( ch->resistant, mod );
         break;
      case APPLY_IMMUNE:
         SET_BIT( ch->immune, mod );
         break;
      case APPLY_SUSCEPTIBLE:
         SET_BIT( ch->susceptible, mod );
         break;
      case APPLY_WEAPONSPELL:   /* see fight.c */
         break;
      case APPLY_REMOVE:
         REMOVE_BIT( ch->affected_by.bits[0], mod );
         break;

         /*
          * Player condition modifiers
          */
      case APPLY_FULL:
         if( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_FULL] = URANGE( 0, ch->pcdata->condition[COND_FULL] + mod, 48 );
         break;

      case APPLY_THIRST:
         if( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_THIRST] = URANGE( 0, ch->pcdata->condition[COND_THIRST] + mod, 48 );
         break;

      case APPLY_DRUNK:
         if( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_DRUNK] = URANGE( 0, ch->pcdata->condition[COND_DRUNK] + mod, 48 );
         break;

      case APPLY_MENTALSTATE:
         ch->mental_state = URANGE( -100, ch->mental_state + mod, 100 );
         break;
      case APPLY_EMOTION:
         ch->emotional_state = URANGE( -100, ch->emotional_state + mod, 100 );
         break;


         /*
          * Specialty modfiers
          */
      case APPLY_CONTAGIOUS:
         break;
      case APPLY_ODOR:
         break;
      case APPLY_STRIPSN:
         if( IS_VALID_SN( mod ) )
            affect_strip( ch, mod );
         else
            bug( "affect_modify: APPLY_STRIPSN invalid sn %d", mod );
         break;

/* spell cast upon wear/removal of an object	-Thoric */
      case APPLY_WEARSPELL:
      case APPLY_REMOVESPELL:
         if( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) || IS_SET( ch->immune, RIS_MAGIC ) || saving_char == ch   /* so save/quit doesn't trigger */
             || loading_char == ch )   /* so loading doesn't trigger */
            return;

         mod = abs( mod );
         if( IS_VALID_SN( mod ) && ( skill = skill_table[mod] ) != NULL && skill->type == SKILL_SPELL )
         {
            if( skill->target == TAR_IGNORE || skill->target == TAR_OBJ_INV )
            {
               bug( "APPLY_WEARSPELL trying to apply bad target spell.  SN is %d.", mod );
               return;
            }
            if( ( retcode = ( *skill->spell_fun ) ( mod, get_curr_wil( ch ), ch, ch ) ) == rCHAR_DIED || char_died( ch ) )
               return;
         }
         break;

         /*
          * Room apply types
          */
      case APPLY_ROOMFLAG:
      case APPLY_SECTORTYPE:
      case APPLY_ROOMLIGHT:
      case APPLY_TELEVNUM:
         break;

         /*
          * Object apply types
          */
   }

   /*
    * Check for weapon wielding.
    * Guard against recursion (for weapons with affects).
    */
   if( !IS_NPC( ch )
       && saving_char != ch
       && ( wield = get_eq_char( ch, WEAR_HAND ) ) != NULL
       && get_obj_weight( wield ) > ( ( ch->weight / 10 ) + ( get_curr_str( ch ) / 5 ) ) )
   {
      static int depth;

      if( depth == 0 )
      {
         depth++;
         act( AT_ACTION, "You are too weak to wield $p any longer.", ch, wield, NULL, TO_CHAR );
         act( AT_ACTION, "$n stops wielding $p.", ch, wield, NULL, TO_ROOM );
         unequip_char( ch, wield );
         depth--;
      }
   }

   return;
}



/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   AFFECT_DATA *paf_new;

   if( !ch )
   {
      bug( "Affect_to_char(NULL, %d)", paf ? paf->type : 0 );
      return;
   }

   if( !paf )
   {
      bug( "Affect_to_char(%s, NULL)", ch->name );
      return;
   }

   CREATE( paf_new, AFFECT_DATA, 1 );
   LINK( paf_new, ch->first_affect, ch->last_affect, next, prev );
   paf_new->type = paf->type;
   paf_new->duration = paf->duration;
   paf_new->location = paf->location;
   paf_new->modifier = paf->modifier;
   paf_new->bitvector = paf->bitvector;

   affect_modify( ch, paf_new, TRUE );
   return;
}


/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   if( !ch->first_affect )
   {
      bug( "Affect_remove(%s, %d): no affect.", ch->name, paf ? paf->type : 0 );
      return;
   }

   affect_modify( ch, paf, FALSE );

   UNLINK( paf, ch->first_affect, ch->last_affect, next, prev );
   DISPOSE( paf );
   return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA * ch, int sn )
{
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;

   for( paf = ch->first_affect; paf; paf = paf_next )
   {
      paf_next = paf->next;
      if( paf->type == sn )
         affect_remove( ch, paf );
   }

   return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA * ch, int sn )
{
   AFFECT_DATA *paf;

   for( paf = ch->first_affect; paf; paf = paf->next )
      if( paf->type == sn )
         return TRUE;

   return FALSE;
}


/*
 * Add or enhance an affect.
 * Limitations put in place by Thoric, they may be high... but at least
 * they're there :)
 */
void affect_join( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   AFFECT_DATA *paf_old;

   for( paf_old = ch->first_affect; paf_old; paf_old = paf_old->next )
      if( paf_old->type == paf->type )
      {
         paf->duration = UMIN( 1000000, paf->duration + paf_old->duration );
         if( paf->modifier )
            paf->modifier = UMIN( 5000, paf->modifier + paf_old->modifier );
         else
            paf->modifier = paf_old->modifier;
         affect_remove( ch, paf_old );
         break;
      }

   affect_to_char( ch, paf );
   return;
}

/* Probably could jury rig Thoric's function above, but this is more flexible -- Scion */
void obj_affect_join( OBJ_DATA * obj, AFFECT_DATA * paf )
{
   AFFECT_DATA *opaf;

   for( opaf = obj->first_affect; opaf; opaf = opaf->next )
   {
      if( opaf->type == paf->type )
      {
         paf->duration += opaf->duration;
         if( paf->modifier )
            paf->modifier += opaf->modifier;
         else
            paf->modifier = opaf->modifier;
         UNLINK( opaf, obj->first_affect, obj->last_affect, next, prev );
         DISPOSE( opaf );
         return;
      }
   }
}

/*
 * Apply only affected and RIS on a char
 */
void aris_affect( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   xSET_BITS( ch->affected_by, paf->bitvector );
   switch ( paf->location % REVERSE_APPLY )
   {
      case APPLY_AFFECT:
         SET_BIT( ch->affected_by.bits[0], paf->modifier );
         break;
      case APPLY_RESISTANT:
         SET_BIT( ch->resistant, paf->modifier );
         break;
      case APPLY_IMMUNE:
         SET_BIT( ch->immune, paf->modifier );
         break;
      case APPLY_SUSCEPTIBLE:
         SET_BIT( ch->susceptible, paf->modifier );
         break;
   }
}

/*
 * Update affecteds and RIS for a character in case things get messed.
 * This should only really be used as a quick fix until the cause
 * of the problem can be hunted down. - FB
 * Last modified: June 30, 1997
 *
 * Quick fix?  Looks like a good solution for a lot of problems.
 */

void update_aris( CHAR_DATA * ch )
{
   AFFECT_DATA *paf;
   OBJ_DATA *obj;

   if( IS_NPC( ch ) )
      return;

   if( !ch->name )
      return;

   xCLEAR_BITS( ch->affected_by );
   ch->resistant = 0;
   ch->immune = 0;
   ch->susceptible = 0;
   xCLEAR_BITS( ch->no_affected_by );
   ch->no_resistant = 0;
   ch->no_immune = 0;
   ch->no_susceptible = 0;

   /*
    * Add in effects from race 
    */
   if( ch->species )
   {
      if( !ch->nation )
         ch->nation = find_nation( ch->species );
      if( ch->nation )
      {
         xSET_BITS( ch->affected_by, ch->nation->affected );
         SET_BIT( ch->resistant, ch->nation->resist );
         SET_BIT( ch->susceptible, ch->nation->suscept );
      }
      else
      {
         bug( "update_aris: Could not find nation!" );
      }
   }

   /*
    * Add in effect from spells 
    */
   for( paf = ch->first_affect; paf; paf = paf->next )
      aris_affect( ch, paf );

   /*
    * Add in effects from equipment 
    */
   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      if( obj->wear_loc != WEAR_NONE )
      {
         for( paf = obj->first_affect; paf; paf = paf->next )
            aris_affect( ch, paf );

         for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
            aris_affect( ch, paf );
      }
   }

   /*
    * Add in effects from the room 
    */
   if( ch->in_room ) /* non-existant char booboo-fix --TRI */
      for( paf = ch->in_room->first_affect; paf; paf = paf->next )
         aris_affect( ch, paf );

   /*
    * make sure vamps are undead -keo 
    */
   if( IS_VAMPIRE( ch ) )
      xSET_BIT( ch->affected_by, AFF_UNDEAD );

   /*
    * Add in effects from songs -keo 
    */
   if( ch->singing )
   {
      switch ( ch->singing )
      {
         case SONG_FLIGHT:
            xSET_BIT( ch->affected_by, AFF_FLYING );
            break;
         case SONG_SIGHT:
            xSET_BIT( ch->affected_by, AFF_TRUESIGHT );
            break;
         case SONG_SHADOWS:
            xSET_BIT( ch->affected_by, AFF_DARK );
            break;
         case SONG_LIGHT:
            xSET_BIT( ch->affected_by, AFF_GLOW );
            break;
      }
   }

   /*
    * Add in effects from talents -keo 
    */
   if( ch->curr_talent[TAL_WIND] >= 10 )
   {
      xSET_BIT( ch->affected_by, AFF_FLOATING );
      if( ch->curr_talent[TAL_WIND] >= 70 )
      {
         xSET_BIT( ch->affected_by, AFF_PASS_DOOR );
      }
   }
   if( ch->curr_talent[TAL_WATER] >= 20 )
   {
      xSET_BIT( ch->affected_by, AFF_AQUA_BREATH );
   }
   if( ch->curr_talent[TAL_SEEKING] >= 5 )
   {
      xSET_BIT( ch->affected_by, AFF_DETECT_MAGIC );
      if( ch->curr_talent[TAL_SEEKING] >= 10 )
      {
         xSET_BIT( ch->affected_by, AFF_INFRARED );
         if( ch->curr_talent[TAL_SEEKING] >= 15 )
         {
            xSET_BIT( ch->affected_by, AFF_DETECT_EVIL );
            if( ch->curr_talent[TAL_SEEKING] >= 20 )
            {
               xSET_BIT( ch->affected_by, AFF_DETECT_INVIS );
               if( ch->curr_talent[TAL_SEEKING] >= 30 )
               {
                  xSET_BIT( ch->affected_by, AFF_DETECT_HIDDEN );
                  if( ch->curr_talent[TAL_SEEKING] >= 50 )
                  {
                     xSET_BIT( ch->affected_by, AFF_TRUESIGHT );
                  }
               }
            }
         }
      }
   }
   if( ch->curr_talent[TAL_SECURITY] >= 60 )
   {
      xSET_BIT( ch->affected_by, AFF_PASS_DOOR );
   }

   return;
}


/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA * ch )
{
   AFFECT_DATA *paf;

   if( !ch->in_room )
   {
      bug( "Char_from_room: NULL.", 0 );
      return;
   }

   if( !IS_NPC( ch ) )
      --ch->in_room->area->nplayer;
   else
      --ch->in_room->area->nmob;

   /*
    * Character's affect on the room
    */
/*    for ( paf = ch->first_affect; paf; paf = paf->next )
	room_affect(ch->in_room, paf, FALSE);
*/

   /*
    * Room's affect on the character
    */
   if( !char_died( ch ) )
   {
      for( paf = ch->in_room->first_affect; paf; paf = paf->next )
         affect_modify( ch, paf, FALSE );

      if( char_died( ch ) )   /* could die from removespell, etc */
         return;
   }

   UNLINK( ch, ch->in_room->first_person, ch->in_room->last_person, next_in_room, prev_in_room );
   ch->was_in_room = ch->in_room;
   ch->in_room = NULL;
   ch->next_in_room = NULL;
   ch->prev_in_room = NULL;

   if( !IS_NPC( ch ) && get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
      remove_timer( ch, TIMER_SHOVEDRAG );

   return;
}


/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex )
{
   AFFECT_DATA *paf;

   if( !ch )
   {
      bug( "Char_to_room: NULL ch!", 0 );
      return;
   }
   if( !pRoomIndex )
   {
      if( char_died( ch ) )
      {  /* Ack! Don't want to put dead chars back in the game! -- Scion */
         bug( "Char_to_room: trying to put dead char %s back into game", ch->name );
         return;
      }

      bug( "Char_to_room: %s -> NULL room!  Putting char in limbo (%d)", ch->name, ROOM_VNUM_LIMBO );
      /*
       * This used to just return, but there was a problem with crashing
       * and I saw no reason not to just put the char in limbo.  -Narn
       */


      send_to_char_color( "&WYou trip and fall, yet somehow miss the ground...&w\r\n", ch );
      pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
   }

   ch->in_room = pRoomIndex;
   if( ch->home_room < 1 )
      ch->home_room = ch->in_room->vnum;

   LINK( ch, pRoomIndex->first_person, pRoomIndex->last_person, next_in_room, prev_in_room );

   if( !IS_NPC( ch ) )
   {
      if( ++pRoomIndex->area->nplayer > pRoomIndex->area->max_players )
      {
         pRoomIndex->area->max_players = pRoomIndex->area->nplayer;
      }
   }
   else
      ++pRoomIndex->area->nmob;

   /*
    * Room's effect on the character
    */
   if( !char_died( ch ) )
   {
      for( paf = pRoomIndex->first_affect; paf; paf = paf->next )
         affect_modify( ch, paf, TRUE );

      if( char_died( ch ) )   /* could die from a wearspell, etc */
         return;
   }

   /*
    * Character's effect on the room
    */
   for( paf = ch->first_affect; paf; paf = paf->next )
      room_affect( pRoomIndex, paf, TRUE );


   if( !IS_NPC( ch ) && IS_SET( pRoomIndex->room_flags, ROOM_SAFE ) && get_timer( ch, TIMER_SHOVEDRAG ) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
                                                 /*-30 Seconds-*/

   if( !ch->was_in_room )
      ch->was_in_room = ch->in_room;

   /*
    * If entering a new zone, reset the area 
    */
   if( ch->was_in_room->area != ch->in_room->area )
   {
      reset_area( ch->in_room->area );
      reset_area( ch->in_room->area );
      reset_area( ch->in_room->area );
   }
   return;
}

/*
 * Give an obj to a char.
 */
OBJ_DATA *obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch )
{
   OBJ_DATA *otmp;
   OBJ_DATA *oret = obj;
   bool skipgroup, grouped;
   int oweight = get_obj_weight( obj );
   int onum = get_obj_number( obj );
   int wear_loc = obj->wear_loc;
   EXT_BV extra_flags = obj->extra_flags;

   skipgroup = FALSE;
   grouped = FALSE;

   if( loading_char == ch )
   {
      int x, y;
      for( x = 0; x < MAX_WEAR; x++ )
         for( y = 0; y < MAX_LAYERS; y++ )
            if( save_equipment[x][y] == obj )
            {
               skipgroup = TRUE;
               break;
            }
   }

   if( !skipgroup )
      for( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
         if( ( oret = group_object( otmp, obj ) ) == otmp )
         {
            grouped = TRUE;
            break;
         }
   if( !grouped )
   {
      if( !IS_NPC( ch ) )
      {
         LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
         obj->carried_by = ch;
         obj->in_room = NULL;
         obj->in_obj = NULL;
      }
      else
      {
         /*
          * If ch is a shopkeeper, add the obj using an insert sort 
          */
         for( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
         {
            INSERT( obj, otmp, ch->first_carrying, next_content, prev_content );
            break;
         }

         if( !otmp )
         {
            LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
         }

         obj->carried_by = ch;
         obj->in_room = NULL;
         obj->in_obj = NULL;
      }
   }
   if( wear_loc == WEAR_NONE )
   {
      ch->carry_number += onum;
      ch->carry_weight += oweight;
   }
   else if( !xIS_SET( extra_flags, ITEM_MAGIC ) )
      ch->carry_weight += oweight;
   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      if( IS_NPC( ch ) )
         xSET_BIT( ch->affected_by, AFF_HAS_ARTI );
      else
         xSET_BIT( ch->pcdata->perm_aff, AFF_HAS_ARTI );
   }
   return ( oret ? oret : obj );
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA * obj )
{
   CHAR_DATA *ch;

   if( ( ch = obj->carried_by ) == NULL )
   {
      bug( "Obj_from_char: null ch.", 0 );
      return;
   }

   if( obj == ch->main_hand )
   {
      ch->main_hand = NULL;
      obj_unaffect_ch( ch, obj );
   }
   if( obj == ch->off_hand )
   {
      ch->off_hand = NULL;
      obj_unaffect_ch( ch, obj );
   }

   if( IS_OBJ_STAT( obj, ITEM_IN_USE ) )
      do_sing( ch, "none" );

   if( obj->wear_loc != WEAR_NONE )
      unequip_char( ch, obj );

   /*
    * obj may drop during unequip... 
    */
   if( !obj->carried_by )
      return;

   UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

   if( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
      empty_obj( obj, NULL, NULL );

   obj->in_room = NULL;
   obj->carried_by = NULL;
   ch->carry_number -= get_obj_number( obj );
   ch->carry_weight -= get_obj_weight( obj );
   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      if( IS_NPC( ch ) )
         xREMOVE_BIT( ch->affected_by, AFF_HAS_ARTI );
      else
         xREMOVE_BIT( ch->pcdata->perm_aff, AFF_HAS_ARTI );
   }
   return;
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA * obj, int iWear )
{
   if( obj->item_type != ITEM_ARMOR )
      return 0;

   switch ( iWear )
   {
      case WEAR_BODY:
         return 3 * obj->value[0];
      case WEAR_HEAD:
         return 2 * obj->value[0];
      case WEAR_LEGS:
         return 2 * obj->value[0];
      case WEAR_FEET:
         return obj->value[0];
      case WEAR_HANDS:
         return obj->value[0];
      case WEAR_ARMS:
         return obj->value[0];
      case WEAR_SHIELD:
         return obj->value[0];
      case WEAR_FINGER_L:
         return obj->value[0];
      case WEAR_FINGER_R:
         return obj->value[0];
      case WEAR_NECK_1:
         return obj->value[0];
      case WEAR_NECK_2:
         return obj->value[0];
      case WEAR_ABOUT:
         return 2 * obj->value[0];
      case WEAR_WAIST:
         return obj->value[0];
      case WEAR_WRIST_L:
         return obj->value[0];
      case WEAR_WRIST_R:
         return obj->value[0];
      case WEAR_HAND:
         return obj->value[0];
      case WEAR_EYES:
         return obj->value[0];
      case WEAR_FACE:
         return obj->value[0];
      case WEAR_BACK:
         return obj->value[0];
      case WEAR_ANKLE_L:
         return obj->value[0];
      case WEAR_ANKLE_R:
         return obj->value[0];
   }

   return 0;
}



/*
 * Find a piece of eq on a character.
 * Will pick the top layer if clothing is layered.		-Thoric
 */
OBJ_DATA *get_eq_char( CHAR_DATA * ch, int iWear )
{
   OBJ_DATA *obj, *maxobj = NULL;

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
      if( obj->wear_loc == iWear )
      {
         if( !obj->pIndexData->layers )
         {
            return obj;
         }
         else
         {
            if( !maxobj || obj->pIndexData->layers > maxobj->pIndexData->layers )
               maxobj = obj;
         }
      }

   return maxobj;
}

/*
 * Move an obj out of a room.
 */
void write_corpses args( ( CHAR_DATA * ch, char *name, OBJ_DATA * objrem ) );

int falling;

void obj_from_room( OBJ_DATA * obj )
{
   ROOM_INDEX_DATA *in_room;
   AFFECT_DATA *paf;

   if( ( in_room = obj->in_room ) == NULL )
   {
      bug( "obj_from_room: NULL.", 0 );
      return;
   }

   for( paf = obj->first_affect; paf; paf = paf->next )
      room_affect( in_room, paf, FALSE );

   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      room_affect( in_room, paf, FALSE );

   UNLINK( obj, in_room->first_content, in_room->last_content, next_content, prev_content );

   /*
    * uncover contents 
    */
   if( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
      empty_obj( obj, NULL, obj->in_room );

   obj->carried_by = NULL;
   obj->in_obj = NULL;
   obj->in_room = NULL;
   if( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling < 1 )
      write_corpses( NULL, obj->short_descr + 14, obj );
   return;
}


/*
 * Move an obj into a room.
 */
OBJ_DATA *obj_to_room( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex )
{
   OBJ_DATA *otmp, *oret;
   AFFECT_DATA *paf;

   if( !pRoomIndex )
      pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );

   for( paf = obj->first_affect; paf; paf = paf->next )
      room_affect( pRoomIndex, paf, TRUE );

   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      room_affect( pRoomIndex, paf, TRUE );

   for( otmp = pRoomIndex->first_content; otmp; otmp = otmp->next_content )
      if( ( oret = group_object( otmp, obj ) ) == otmp )
      {
         return oret;
      }

   LINK( obj, pRoomIndex->first_content, pRoomIndex->last_content, next_content, prev_content );
   obj->in_room = pRoomIndex;
   obj->carried_by = NULL;
   obj->in_obj = NULL;
   falling++;
   obj_fall( obj, FALSE );
   falling--;
   if( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling < 1 )
      write_corpses( NULL, obj->short_descr + 14, NULL );
   return obj;
}


/*
 * Who's carrying an item -- recursive for nested objects	-Thoric
 */
CHAR_DATA *carried_by( OBJ_DATA * obj )
{
   if( obj->in_obj )
      return carried_by( obj->in_obj );

   return obj->carried_by;
}


/*
 * Move an object into an object.
 */
OBJ_DATA *obj_to_obj( OBJ_DATA * obj, OBJ_DATA * obj_to )
{
   OBJ_DATA *otmp, *oret;
   CHAR_DATA *who;

   if( obj == obj_to )
   {
      bug( "Obj_to_obj: trying to put object inside itself: vnum %d", obj->pIndexData->vnum );
      return obj;
   }

   if( !in_magic_container( obj_to ) && ( who = carried_by( obj_to ) ) != NULL )
      who->carry_weight += get_obj_weight( obj );

   for( otmp = obj_to->first_content; otmp; otmp = otmp->next_content )
      if( ( oret = group_object( otmp, obj ) ) == otmp )
         return oret;

   LINK( obj, obj_to->first_content, obj_to->last_content, next_content, prev_content );

   obj->in_obj = obj_to;
   obj->in_room = NULL;
   obj->carried_by = NULL;

   return obj;
}


/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA * obj )
{
   OBJ_DATA *obj_from;
   bool magic;

   if( ( obj_from = obj->in_obj ) == NULL )
   {
      bug( "Obj_from_obj: null obj_from.", 0 );
      return;
   }

   magic = in_magic_container( obj_from );

   UNLINK( obj, obj_from->first_content, obj_from->last_content, next_content, prev_content );

   /*
    * uncover contents 
    */
   if( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
      empty_obj( obj, obj->in_obj, NULL );

   obj->in_obj = NULL;
   obj->in_room = NULL;
   obj->carried_by = NULL;

   if( !magic )
      for( ; obj_from; obj_from = obj_from->in_obj )
         if( obj_from->carried_by )
            obj_from->carried_by->carry_weight -= get_obj_weight( obj );

   return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA * obj )
{
   OBJ_DATA *obj_content;

   if( obj_extracted( obj ) )
   {
      bug( "extract_obj: obj %d already extracted!", obj->pIndexData->vnum );
      return;
   }

   if( obj->item_type == ITEM_PORTAL )
      remove_portal( obj );

   if( obj->item_type == ITEM_FURNITURE && obj->in_room )
   {
      CHAR_DATA *ch;

      for( ch = obj->in_room->first_person; ch; ch = ch->next_in_room )
      {
         if( ch->on == obj )
         {
            ch->on = NULL;
            do_stand( ch, "" );
         }
      }
   }

   dump_passenger( obj );

   if( obj->carried_by )
      obj_from_char( obj );
   else if( obj->in_room )
      obj_from_room( obj );
   else if( obj->in_obj )
      obj_from_obj( obj );

   while( ( obj_content = obj->last_content ) != NULL )
      extract_obj( obj_content );

/* This seems to crash the mud for some reason */
/*    if (obj->gem)
	extract_obj(obj->gem);
*/

   /*
    * remove affects 
    */
   {
      AFFECT_DATA *paf;
      AFFECT_DATA *paf_next;

      for( paf = obj->first_affect; paf; paf = paf_next )
      {
         paf_next = paf->next;
         DISPOSE( paf );
      }
      obj->first_affect = obj->last_affect = NULL;
   }

   /*
    * remove extra descriptions 
    */
   {
      EXTRA_DESCR_DATA *ed;
      EXTRA_DESCR_DATA *ed_next;

      for( ed = obj->first_extradesc; ed; ed = ed_next )
      {
         ed_next = ed->next;
         STRFREE( ed->description );
         STRFREE( ed->keyword );
         DISPOSE( ed );
      }
      obj->first_extradesc = obj->last_extradesc = NULL;
   }

   if( obj == gobj_prev )
      gobj_prev = obj->prev;

   UNLINK( obj, first_object, last_object, next, prev );

   /*
    * shove onto extraction queue 
    */
   queue_extracted_obj( obj );

   obj->pIndexData->count -= obj->count;
   numobjsloaded -= obj->count;
   --physicalobjects;
   if( obj->serial == cur_obj )
   {
      cur_obj_extracted = TRUE;
      if( global_objcode == rNONE )
         global_objcode = rOBJ_EXTRACTED;
   }
   return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA * ch, bool fPull )
{
   CHAR_DATA *wch;
   OBJ_DATA *obj;

   if( !ch )
   {
      bug( "Extract_char: NULL ch.", 0 );
      return;
   }

   if( !ch->in_room )
   {
      bug( "Extract_char: %s in NULL room.", ch->name ? ch->name : "???" );
      return;
   }

   if( ch == supermob )
   {
      bug( "Extract_char: ch == supermob!", 0 );
      return;
   }

   if( char_died( ch ) )
   {
      bug( "extract_char: %s already died!", ch->name );
      return;
   }

   if( ch == cur_char )
      cur_char_died = TRUE;

   /*
    * shove onto extraction queue 
    */
   queue_extracted_char( ch, fPull );

   /*
    * Tell attached reset that it can fire again 
    */
   if( IS_NPC( ch ) )
   {
      if( ch->reset )
         ch->reset->active = FALSE;
      while( ( obj = ch->first_carrying ) )
         extract_obj( obj );
   }

   if( gch_prev == ch )
      gch_prev = ch->prev;

   if( fPull )
      die_follower( ch );

   if( ch->mount )
   {
      xREMOVE_BIT( ch->mount->affected_by, AFF_MOUNTED );
      ch->mount = NULL;
      ch->position = POS_STANDING;
   }

   /*
    * check if this char was a mount or a pet
    */
   for( wch = first_char; wch; wch = wch->next )
   {
      if( wch->mount == ch )
      {
         wch->mount = NULL;
         wch->position = POS_STANDING;
         if( wch->in_room == ch->in_room )
         {
            act( AT_SOCIAL, "Your faithful mount, $N collapses beneath you...", wch, NULL, ch, TO_CHAR );
            act( AT_SOCIAL, "Sadly you dismount $M for the last time.", wch, NULL, ch, TO_CHAR );
            act( AT_PLAIN, "$n sadly dismounts $N for the last time.", wch, NULL, ch, TO_ROOM );
         }
      }
      if( wch->pcdata && wch->pcdata->pet == ch )
      {
         wch->pcdata->pet = NULL;
         if( wch->in_room == ch->in_room )
            act( AT_SOCIAL, "You mourn for the loss of $N.", wch, NULL, ch, TO_CHAR );
      }
   }
   xREMOVE_BIT( ch->affected_by, AFF_MOUNTED );

   if( IS_NPC( ch ) && ch->first_part )
   {
      PART_DATA *part;
      PART_DATA *next;

      part = ch->first_part;
      while( 1 )
      {
         next = part->next;
         UNLINK( part, ch->first_part, ch->last_part, next, prev );
         DISPOSE( part );
         part = next;
         if( !part )
            break;
      }
   }

   if( !fPull )
      return;
   else
      char_from_room( ch );

   if( IS_NPC( ch ) )
   {
      --ch->pIndexData->count;
      --nummobsloaded;
   }

   if( ch->desc && ch->desc->original )
      do_return( ch, "" );

   for( wch = first_char; wch; wch = wch->next )
   {
      if( wch->reply == ch )
         wch->reply = NULL;
      if( wch->retell == ch )
         wch->retell = NULL;
   }

   UNLINK( ch, first_char, last_char, next, prev );

   if( ch->desc )
   {
      if( ch->desc->character != ch )
         bug( "Extract_char: char's descriptor points to another char", 0 );
      else
      {
         ch->desc->character = NULL;
         close_socket( ch->desc, FALSE );
         ch->desc = NULL;
      }
   }

   return;
}

char *pos_string( CHAR_DATA * victim )
{
   switch ( victim->position )
   {
      case POS_DEAD:
         return "is DEAD!!";
      case POS_MORTAL:
         return "is mortally wounded";
      case POS_INCAP:
         return "is incapacitated.";
      case POS_STUNNED:
         return "is lying here stunned.";
      case POS_SLEEPING:
         return "is sleeping";
      case POS_RESTING:
         return "is resting";
      case POS_STANDING:
         return "is standing";
      case POS_SITTING:
         return "is sitting";
      case POS_KNEELING:
         return "is kneeling";
   }
   return "ERROR";
}

char *fur_pos_string( int pos )
{
   switch ( pos )
   {
      case ST_ON:
         return "on";
      case SI_ON:
         return "on";
      case SL_ON:
         return "on";
      case RE_ON:
         return "on";
      case PT_ON:
         return "on";
      case ST_AT:
         return "at";
      case SI_AT:
         return "at";
      case SL_AT:
         return "at";
      case RE_AT:
         return "at";
      case PT_AT:
         return "at";
      case ST_IN:
         return "in";
      case SI_IN:
         return "in";
      case SL_IN:
         return "in";
      case RE_IN:
         return "in";
      case PT_IN:
         return "in";
      case PT_INSIDE:
         return "inside";
         break;
   }
   return "ERROR";
}

/* returns number of people on an object */
int count_users( OBJ_DATA * obj )
{
   CHAR_DATA *fch;
   int count = 0;

   if( obj->in_room == NULL )
      return 0;

   for( fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room )
      if( fch->on == obj )
         count++;

   return count;
}


/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *rch;
   int number, count, vnum;

   number = number_argument( argument, arg );
   if( !str_cmp( arg, "self" ) || !str_cmp( arg, "me" ) )
      return ch;

   if( ch->pcdata && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;

   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      if( ( can_see( ch, rch ) || number_range( 1, 3 ) == 1 )
          && ( ( nifty_is_name( arg, rch->name )
                 || ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) )
               || ( ( !IS_NPC( rch ) && rch->pcdata->name_disguise
                      && nifty_is_name( arg, rch->pcdata->name_disguise ) ) ) ) )
      {
         if( number == 0 && !IS_NPC( rch ) )
            return rch;
         else if( ++count == number )
            return rch;
      }

   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of characters
    * again looking for prefix matching, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( !can_see( ch, rch ) || !nifty_is_name_prefix( arg, rch->name ) )
         continue;
      if( number == 0 && !IS_NPC( rch ) )
         return rch;
      else if( ++count == number )
         return rch;
   }

   return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *wch;
   DESCRIPTOR_DATA *d;
   int number, count, vnum;

   number = number_argument( argument, arg );
   count = 0;
   if( !str_cmp( arg, "self" ) || !str_cmp( arg, "me" ) )
      return ch;

   if( ch->pcdata && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   /*
    * check the room for an exact match 
    */
   for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
      if( can_see( ch, wch ) && ( nifty_is_name( arg, wch->name ) || ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) )
      {
         if( number == 0 && !IS_NPC( wch ) )
            return wch;
         else if( ++count == number )
            return wch;
      }

   /*
    * Check the descriptor list first for a player -- Scion 
    */
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected != CON_PLAYING )
         continue;
      wch = d->character;
      if( can_see( ch, wch ) && ( nifty_is_name( arg, wch->name ) ) )
         return wch;
   }

   count = 0;

   /*
    * check the world for an exact match 
    */
   for( wch = first_char; wch; wch = wch->next )
      if( can_see( ch, wch ) && ( ( nifty_is_name( arg, wch->name )
                                    || ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) ) )
      {
         if( number == 0 && !IS_NPC( wch ) )
            return wch;
         else if( ++count == number )
            return wch;
      }

   /*
    * bail out if looking for a vnum match 
    */
   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, check the room for
    * for a prefix match, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
   {
      if( !can_see( ch, wch ) || !nifty_is_name_prefix( arg, wch->name ) )
         continue;
      if( number == 0 && !IS_NPC( wch ) )
         return wch;
      else if( ++count == number )
         return wch;
   }

   /*
    * If we didn't find a prefix match in the room, run through the full list
    * of characters looking for prefix matching, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( wch = first_char; wch; wch = wch->next )
   {
      if( !can_see( ch, wch ) || !nifty_is_name_prefix( arg, wch->name ) )
         continue;
      if( number == 0 && !IS_NPC( wch ) )
         return wch;
      else if( ++count == number )
         return wch;
   }

   return NULL;
}


void throw_rider( CHAR_DATA * ch )
{
   CHAR_DATA *rch;

   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( rch->mount && rch->mount == ch )
      {
         act( AT_SKILL, "$n falls off your back!", ch, NULL, rch, TO_CHAR );
         act( AT_SKILL, "$n falls off $N's back!", ch, NULL, rch, TO_NOTVICT );
         act( AT_SKILL, "You fall off $N's back!", ch, NULL, rch, TO_VICT );
         lose_hp( rch, 10 );
         if( char_died( rch ) )
            return;
         rch->position = POS_SITTING;
         rch->mount = NULL;
         return;
      }
   }
   if( IS_NPC( ch ) )
      xREMOVE_BIT( ch->affected_by, AFF_MOUNTED );
   else
      xREMOVE_BIT( ch->pcdata->perm_aff, AFF_MOUNTED );
}

void dump_passenger( OBJ_DATA * obj )
{
   CHAR_DATA *rch;
   if( !obj->in_room )
      return;

   for( rch = obj->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( rch->in_obj && rch->in_obj == obj )
      {
         act( AT_SKILL, "You fall out of $p!", rch, obj, NULL, TO_CHAR );
         act( AT_SKILL, "$n falls out of $p!", rch, obj, NULL, TO_ROOM );
         lose_hp( rch, 10 );
         if( char_died( rch ) )
            return;
         rch->position = POS_SITTING;
         rch->in_obj = NULL;
         return;
      }
   }
}

void dump_container( OBJ_DATA * obj )
{
   OBJ_DATA *obj1;

   for( obj1 = obj->first_content; obj1; obj1 = obj->first_content )
   {
      obj_from_obj( obj1 );
      if( obj->in_room )
      {
         obj_to_room( obj1, obj->in_room );
      }
      else if( obj->in_obj )
      {
         obj_to_obj( obj1, obj->in_obj );
      }
      else if( obj->carried_by )
      {
         obj_to_char( obj1, obj->carried_by );
      }
      else
      {
         bug( "Obj %d could not be removed from container, put in Void for disposal.", obj1->pIndexData->vnum );
         obj_to_room( obj1, get_room_index( 1 ) );
      }
   }
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA * ch, char *argument, OBJ_DATA * list )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number;
   int count;

   number = number_argument( argument, arg );
   count = 0;
   for( obj = list; obj; obj = obj->next_content )
      if( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = list; obj; obj = obj->next_content )
      if( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}

/*
 * Find an obj in a list...going the other way			-Thoric
 */
OBJ_DATA *get_obj_list_rev( CHAR_DATA * ch, char *argument, OBJ_DATA * list )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number;
   int count;

   number = number_argument( argument, arg );
   count = 0;
   for( obj = list; obj; obj = obj->prev_content )
      if( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = list; obj; obj = obj->prev_content )
      if( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}

/*
 * Find an obj in player's inventory or wearing via a vnum -Shaddai
 */

OBJ_DATA *get_obj_vnum( CHAR_DATA * ch, int vnum )
{
   OBJ_DATA *obj;

   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( can_see_obj( ch, obj ) && obj->pIndexData->vnum == vnum )
         return obj;
   return NULL;
}


/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number, count, vnum;

   number = number_argument( argument, arg );
   if( ch->pcdata && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc == -1 && ( nifty_is_name( arg, obj->name ) || obj->pIndexData->vnum == vnum ) )
         if( ( count += obj->count ) >= number )
            return obj;

   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc == -1 && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number, count, vnum;

   number = number_argument( argument, arg );

   if( ch->pcdata && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc != WEAR_NONE && ( nifty_is_name( arg, obj->name ) || obj->pIndexData->vnum == vnum ) )
         if( ++count == number )
            return obj;

   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      if( obj->wear_loc != WEAR_NONE
/*	&&   can_see_obj( ch, obj ) */
          && nifty_is_name_prefix( arg, obj->name ) )
         if( ++count == number )
            return obj;

   return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_obj_carry( ch, argument ) ) != NULL )
      return obj;

   if( ( obj = get_obj_list_rev( ch, argument, ch->in_room->last_content ) ) != NULL )
      return obj;

   if( ( obj = get_obj_wear( ch, argument ) ) != NULL )
      return obj;

   return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   int number, count, vnum;

   if( ( obj = get_obj_here( ch, argument ) ) != NULL )
      return obj;

   number = number_argument( argument, arg );

   if( ch->pcdata && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) && is_number( arg ) )
      vnum = atoi( arg );
   else
      vnum = -1;

   count = 0;
   for( obj = first_object; obj; obj = obj->next )
      if( can_see_obj( ch, obj ) && ( nifty_is_name( arg, obj->name ) || vnum == obj->pIndexData->vnum ) )
         if( ( count += obj->count ) >= number )
            return obj;

   /*
    * bail out if looking for a vnum 
    */
   if( vnum != -1 )
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for( obj = first_object; obj; obj = obj->next )
      if( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
         if( ( count += obj->count ) >= number )
            return obj;

   return NULL;
}


/*
 * How mental state could affect finding an object		-Thoric
 * Used by get/drop/put/quaff/recite/etc
 * Increasingly freaky based on mental state and drunkeness
 */
bool ms_find_obj( CHAR_DATA * ch )
{
   int ms = ch->mental_state;
   int drunk = IS_NPC( ch ) ? 0 : ch->pcdata->condition[COND_DRUNK];
   char *t;

   /*
    * we're going to be nice and let nothing weird happen unless
    * you're a tad messed up
    */
   drunk = UMAX( 1, drunk );
   if( abs( ms ) + ( drunk / 3 ) < 30 )
      return FALSE;
   if( ( number_percent(  ) + ( ms < 0 ? 15 : 5 ) ) > abs( ms ) / 2 + drunk / 4 )
      return FALSE;
   if( ms > 15 )  /* range 1 to 20 -- feel free to add more */
      switch ( number_range( UMAX( 1, ( ms / 5 - 15 ) ), ( ms + 4 ) / 5 ) )
      {
         default:
         case 1:
            t = "As you reach for it, you forgot what it was...\n\r";
            break;
         case 2:
            t = "As you reach for it, something inside stops you...\n\r";
            break;
         case 3:
            t = "As you reach for it, it seems to move out of the way...\n\r";
            break;
         case 4:
            t = "You grab frantically for it, but can't seem to get a hold of it...\n\r";
            break;
         case 5:
            t = "It disappears as soon as you touch it!\n\r";
            break;
         case 6:
            t = "You would if it would stay still!\n\r";
            break;
         case 7:
            t = "Whoa!  It's covered in blood!  Ack!  Ick!\n\r";
            break;
         case 8:
            t = "Wow... trails!\n\r";
            break;
         case 9:
            t = "You reach for it, then notice the back of your hand is growing something!\n\r";
            break;
         case 10:
            t = "As you grasp it, it shatters into tiny shards which bite into your flesh!\n\r";
            break;
         case 11:
            t = "What about that huge dragon flying over your head?!?!?\n\r";
            break;
         case 12:
            t = "You stratch yourself instead...\n\r";
            break;
         case 13:
            t = "You hold the universe in the palm of your hand!\n\r";
            break;
         case 14:
            t = "You're too scared.\n\r";
            break;
         case 15:
            t = "Your mother smacks your hand... 'NO!'\n\r";
            break;
         case 16:
            t = "Your hand grasps the worst pile of revoltingness that you could ever imagine!\n\r";
            break;
         case 17:
            t = "You stop reaching for it as it screams out at you in pain!\n\r";
            break;
         case 18:
            t = "What about the millions of burrow-maggots feasting on your arm?!?!\n\r";
            break;
         case 19:
            t = "That doesn't matter anymore... you've found the true answer to everything!\n\r";
            break;
         case 20:
            t = "A supreme entity has no need for that.\n\r";
            break;
      }
   else
   {
      int sub = URANGE( 1, abs( ms ) / 2 + drunk, 60 );
      switch ( number_range( 1, sub / 10 ) )
      {
         default:
         case 1:
            t = "In just a second...\n\r";
            break;
         case 2:
            t = "You can't find that...\n\r";
            break;
         case 3:
            t = "It's just beyond your grasp...\n\r";
            break;
         case 4:
            t = "...but it's under a pile of other stuff...\n\r";
            break;
         case 5:
            t = "You go to reach for it, but pick your nose instead.\n\r";
            break;
         case 6:
            t = "Which one?!?  You see two... no three...\n\r";
            break;
      }
   }
   send_to_char( t, ch );
   return TRUE;
}


/*
 * Generic get obj function that supports optional containers.	-Thoric
 * currently only used for "eat" and "quaff".
 */
OBJ_DATA *find_obj( CHAR_DATA * ch, char *argument, bool carryonly )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj = NULL;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
      argument = one_argument( argument, arg2 );

   if( arg2[0] == '\0' )
   {
      if( carryonly && ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
      {
         send_to_char( "You do not have that item.\n\r", ch );
         return NULL;
      }
      else if( !carryonly && ( obj = get_obj_here( ch, arg1 ) ) == NULL )
      {
         act( AT_PLAIN, "You see no $T here.", ch, NULL, arg1, TO_CHAR );
         return NULL;
      }
      return obj;
   }
   else
   {
      OBJ_DATA *container = NULL;

      if( carryonly
          && ( container = get_obj_carry( ch, arg2 ) ) == NULL && ( container = get_obj_wear( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not have that item.\n\r", ch );
         return NULL;
      }
      if( !carryonly && ( container = get_obj_here( ch, arg2 ) ) == NULL )
      {
         act( AT_PLAIN, "You see no $T here.", ch, NULL, arg2, TO_CHAR );
         return NULL;
      }

      if( !IS_OBJ_STAT( container, ITEM_COVERING ) && IS_SET( container->value[1], CONT_CLOSED ) )
      {
         act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
         return NULL;
      }

      obj = get_obj_list( ch, arg1, container->first_content );
      if( !obj )
         act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
              "You see nothing like that beneath $p." : "You see nothing like that in $p.", ch, container, NULL, TO_CHAR );
      return obj;
   }
   return NULL;
}

int get_obj_number( OBJ_DATA * obj )
{
   return obj->count;
}

/*
 * Return TRUE if an object is, or nested inside a magic container
 */
bool in_magic_container( OBJ_DATA * obj )
{
   if( obj->item_type == ITEM_CONTAINER && IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      return TRUE;
   if( obj->in_obj )
      return in_magic_container( obj->in_obj );
   return FALSE;
}

/*
 * Return weight of an object, including weight of contents (unless magic).
 */
int get_obj_weight( OBJ_DATA * obj )
{
   int weight;

   weight = obj->count * obj->weight;

   /*
    * magic containers 
    */
   if( obj->item_type != ITEM_CONTAINER || !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      for( obj = obj->first_content; obj; obj = obj->next_content )
         weight += get_obj_weight( obj );

   return weight;
}

/*
 * Return real weight of an object, including weight of contents.
 */
int get_real_obj_weight( OBJ_DATA * obj )
{
   int weight;

   weight = obj->count * obj->weight;

   for( obj = obj->first_content; obj; obj = obj->next_content )
      weight += get_real_obj_weight( obj );

   return weight;
}



/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA * pRoomIndex )
{
   int lights;

   if( !pRoomIndex )
   {
      bug( "room_is_dark: NULL pRoomIndex", 0 );
      return TRUE;
   }

   lights = get_light_room( pRoomIndex );

   if( lights > 0 )
      return FALSE;

   if( lights < 0 )
      return TRUE;

   if( IS_SET( pRoomIndex->room_flags, ROOM_DARK ) )
      return TRUE;

   if( pRoomIndex->sector_type == SECT_INSIDE || pRoomIndex->sector_type == SECT_CITY )
      return FALSE;

   if( time_info.sunlight == SUN_SET || time_info.sunlight == SUN_DARK )
      return TRUE;

   return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA * pRoomIndex )
{
   CHAR_DATA *rch;
   int count;

   if( !pRoomIndex )
   {
      bug( "room_is_private: NULL pRoomIndex", 0 );
      return FALSE;
   }

   count = 0;
   for( rch = pRoomIndex->first_person; rch; rch = rch->next_in_room )
      count++;

   if( IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) && count >= 2 )
      return TRUE;

   if( IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && count >= 1 )
      return TRUE;

   return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA * ch, CHAR_DATA * victim )
{
   if( !ch )
   {
      if( IS_AFFECTED( victim, AFF_INVISIBLE )
          || IS_AFFECTED( victim, AFF_HIDE )
          || IS_AFFECTED( victim, AFF_DREAMWORLD )
          || IS_AFFECTED( victim, AFF_VOID ) || xIS_SET( victim->act, PLR_WIZINVIS ) )
         return FALSE;
      else
         return TRUE;
   }

   if( ch == victim )
      return TRUE;

   if( ch == supermob )
      return TRUE;

   if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return TRUE;

   /*
    * Telepathy -keo 
    */
   if( ch->curr_talent[TAL_MIND] >= 50 &&
       TALENT( ch, TAL_MIND ) > 300 - get_curr_int( victim )
       && ( IS_NPC( victim ) || !IS_SET( victim->pcdata->flags, PCFLAG_SHIELD ) ) )
      return TRUE;

   if( ( !IS_NPC( victim )
         && xIS_SET( victim->act, PLR_WIZINVIS ) && TALENT( ch, TAL_SEEKING ) < victim->pcdata->wizinvis ) )
      return FALSE;

   /*
    * SB 
    */
    
   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;
    
    
   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_MOBINVIS ) ) //&& TALENT( ch, TAL_SEEKING ) < victim->mobinvis )
      return FALSE;


   /*
    * The miracle cure for blindness? -- Altrag 
    */
   if( !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
   {
      if( IS_AFFECTED( ch, AFF_BLIND ) )
         return FALSE;

      if( room_is_dark( ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
         return FALSE;

      if( IS_AFFECTED( victim, AFF_INVISIBLE ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
         return FALSE;

      if( IS_AFFECTED( victim, AFF_HIDE ) && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) )
         return FALSE;
   }

   if( IS_AFFECTED( victim, AFF_HIDE ) && !IS_NPC( victim )
       && victim->pcdata->noncombat[SK_STEALTH] * 5 > get_curr_per( ch ) + TALENT( ch, TAL_SEEKING ) )
      return FALSE;

   if( IS_AFFECTED( victim, AFF_DREAMWORLD )
       && !IS_AFFECTED( ch, AFF_DREAMWORLD ) && TALENT( ch, TAL_SEEKING ) + TALENT( ch, TAL_DREAM ) < 100 )
      return FALSE;

   if( IS_AFFECTED( victim, AFF_VOID ) && !IS_AFFECTED( ch, AFF_VOID ) && TALENT( ch, TAL_VOID ) < 25 )
      return FALSE;

   return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( !obj || !ch )
      return FALSE;

   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;

   if( IS_NPC( ch ) && ch->pIndexData->vnum == 3 )
      return TRUE;

   if( IS_OBJ_STAT( obj, ITEM_BURIED ) )
      return FALSE;

   if( IS_AFFECTED( ch, AFF_BLIND ) && !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return FALSE;

   if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) && !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      return FALSE;

   if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return TRUE;

   if( IS_OBJ_STAT( obj, ITEM_HIDDEN ) && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) )
      return FALSE;

   if( IS_OBJ_STAT( obj, ITEM_INVIS ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
      return FALSE;

   if( get_light_char( ch ) > 0 )
      return TRUE;

   if( room_is_dark( ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
      return FALSE;

   return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if( !IS_OBJ_STAT( obj, ITEM_NODROP ) )
      return TRUE;

   if( ch == supermob )
      return TRUE;

   return FALSE;
}


/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA * obj )
{
   if( obj->item_type < 1 || obj->item_type > MAX_ITEM_TYPE )
   {
      bug( "Item_type_name: unknown type %d.", obj->item_type );
      return "(unknown)";
   }

   return o_types[obj->item_type];
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
   switch ( location )
   {
      case APPLY_NONE:
         return "none";
      case APPLY_STR:
         return "strength";
      case APPLY_DEX:
         return "dexterity";
      case APPLY_INT:
         return "intelligence";
      case APPLY_WIS:
         return "willpower";
      case APPLY_CON:
         return "constitution";
      case APPLY_CHA:
         return "perception";
      case APPLY_FIRE_MAGIC:
         return "fire magic";
      case APPLY_EARTH_MAGIC:
         return "earth magic";
      case APPLY_WIND_MAGIC:
         return "wind magic";
      case APPLY_FROST_MAGIC:
         return "frost magic";
      case APPLY_LIGHTNING_MAGIC:
         return "lightning magic";
      case APPLY_WATER_MAGIC:
         return "water magic";
      case APPLY_DREAM_MAGIC:
         return "dream magic";
      case APPLY_SPEECH_MAGIC:
         return "speech magic";
      case APPLY_HEALING_MAGIC:
         return "healing magic";
      case APPLY_DEATH_MAGIC:
         return "death magic";
      case APPLY_CHANGE_MAGIC:
         return "change magic";
      case APPLY_TIME_MAGIC:
         return "time magic";
      case APPLY_MIND_MAGIC:
         return "mind magic";
      case APPLY_MOTION_MAGIC:
         return "motion magic";
      case APPLY_ILLUSION_MAGIC:
         return "illusion magic";
      case APPLY_SEEKING_MAGIC:
         return "seeking magic";
      case APPLY_SECURITY_MAGIC:
         return "security magic";
      case APPLY_CATALYSM_MAGIC:
         return "catalysm magic";
      case APPLY_VOID_MAGIC:
         return "void magic";
      case APPLY_LCK:
         return "endurance";
      case APPLY_SEX:
         return "sex";
      case APPLY_AGE:
         return "age";
      case APPLY_MANA:
         return "mana";
      case APPLY_HIT:
         return "hp";
      case APPLY_MOVE:
         return "moves";
      case APPLY_GOLD:
         return "gold";
      case APPLY_EXP:
         return "experience";
      case APPLY_AC:
         return "armor class";
      case APPLY_HITROLL:
         return "hit roll";
      case APPLY_DAMROLL:
         return "damage roll";
      case APPLY_SAVING_POISON:
         return "save vs poison";
      case APPLY_SAVING_ROD:
         return "save vs rod";
      case APPLY_SAVING_PARA:
         return "save vs paralysis";
      case APPLY_SAVING_BREATH:
         return "save vs breath";
      case APPLY_SAVING_SPELL:
         return "save vs spell";
      case APPLY_HEIGHT:
         return "height";
      case APPLY_WEIGHT:
         return "weight";
      case APPLY_AFFECT:
         return "affected_by";
      case APPLY_RESISTANT:
         return "resistant";
      case APPLY_IMMUNE:
         return "immune";
      case APPLY_SUSCEPTIBLE:
         return "susceptible";
      case APPLY_WEAPONSPELL:
         return "weapon spell";
      case APPLY_WEARSPELL:
         return "wear spell";
      case APPLY_REMOVESPELL:
         return "remove spell";
      case APPLY_MENTALSTATE:
         return "mental state";
      case APPLY_EMOTION:
         return "emotional state";
      case APPLY_STRIPSN:
         return "dispel";
      case APPLY_REMOVE:
         return "remove";
      case APPLY_DIG:
         return "dig";
      case APPLY_FULL:
         return "hunger";
      case APPLY_THIRST:
         return "thirst";
      case APPLY_DRUNK:
         return "drunk";
      case APPLY_BLOOD:
         return "blood";
      case APPLY_RECURRINGSPELL:
         return "recurring spell";
      case APPLY_CONTAGIOUS:
         return "contagious";
      case APPLY_ODOR:
         return "odor";
      case APPLY_ROOMFLAG:
         return "roomflag";
      case APPLY_SECTORTYPE:
         return "sectortype";
      case APPLY_ROOMLIGHT:
         return "roomlight";
   };

   bug( "Affect_location_name: unknown location %d.", location );
   return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( EXT_BV * vector )
{
   static char buf[512];

   buf[0] = '\0';
   if( xIS_SET( *vector, AFF_BLIND ) )
      strcat( buf, " blind" );
   if( xIS_SET( *vector, AFF_INVISIBLE ) )
      strcat( buf, " invisible" );
   if( xIS_SET( *vector, AFF_DETECT_EVIL ) )
      strcat( buf, " detect_align" );
   if( xIS_SET( *vector, AFF_DETECT_INVIS ) )
      strcat( buf, " detect_invis" );
   if( xIS_SET( *vector, AFF_DETECT_MAGIC ) )
      strcat( buf, " detect_magic" );
   if( xIS_SET( *vector, AFF_DETECT_HIDDEN ) )
      strcat( buf, " detect_hidden" );
   if( xIS_SET( *vector, AFF_HOLD ) )
      strcat( buf, " hold" );
   if( xIS_SET( *vector, AFF_FAERIE_FIRE ) )
      strcat( buf, " faerie_fire" );
   if( xIS_SET( *vector, AFF_INFRARED ) )
      strcat( buf, " infravision" );
   if( xIS_SET( *vector, AFF_CURSE ) )
      strcat( buf, " curse" );
   if( xIS_SET( *vector, AFF_FLAMING ) )
      strcat( buf, " flaming" );
   if( xIS_SET( *vector, AFF_POISON ) )
      strcat( buf, " poison" );
   if( xIS_SET( *vector, AFF_PROTECT ) )
      strcat( buf, " protect" );
   if( xIS_SET( *vector, AFF_PARALYSIS ) )
      strcat( buf, " paralysis" );
   if( xIS_SET( *vector, AFF_SLEEP ) )
      strcat( buf, " sleep" );
   if( xIS_SET( *vector, AFF_SNEAK ) )
      strcat( buf, " sneak" );
   if( xIS_SET( *vector, AFF_HIDE ) )
      strcat( buf, " hide" );
   if( xIS_SET( *vector, AFF_CHARM ) )
      strcat( buf, " charm" );
   if( xIS_SET( *vector, AFF_POSSESS ) )
      strcat( buf, " possess" );
   if( xIS_SET( *vector, AFF_FLYING ) )
      strcat( buf, " flying" );
   if( xIS_SET( *vector, AFF_PASS_DOOR ) )
      strcat( buf, " pass_door" );
   if( xIS_SET( *vector, AFF_FLOATING ) )
      strcat( buf, " feather_fall" );
   if( xIS_SET( *vector, AFF_TRUESIGHT ) )
      strcat( buf, " true_sight" );
   if( xIS_SET( *vector, AFF_DETECTTRAPS ) )
      strcat( buf, " detect_traps" );
   if( xIS_SET( *vector, AFF_ANTHRO ) )
      strcat( buf, " anthro" );
   if( xIS_SET( *vector, AFF_BERSERK ) )
      strcat( buf, " berserk" );
   if( xIS_SET( *vector, AFF_AQUA_BREATH ) )
      strcat( buf, " breathing" );
   if( xIS_SET( *vector, AFF_HASTE ) )
      strcat( buf, " haste" );
   if( xIS_SET( *vector, AFF_SLOW ) )
      strcat( buf, " slow" );
   if( xIS_SET( *vector, AFF_DEAF ) )
      strcat( buf, " deaf" );
   if( xIS_SET( *vector, AFF_DARK ) )
      strcat( buf, " dark" );
   if( xIS_SET( *vector, AFF_GLOW ) )
      strcat( buf, " glow" );
   if( xIS_SET( *vector, AFF_AQUATIC ) )
      strcat( buf, " aquatic" );
   if( xIS_SET( *vector, AFF_SILENT ) )
      strcat( buf, " silent" );
   if( xIS_SET( *vector, AFF_UNDEAD ) )
      strcat( buf, " undead" );
   if( xIS_SET( *vector, AFF_DEMON ) )
      strcat( buf, " demon" );
   if( xIS_SET( *vector, AFF_MOUNTABLE ) )
      strcat( buf, " mountable" );
   if( xIS_SET( *vector, AFF_MOUNTED ) )
      strcat( buf, " mounted" );
   if( xIS_SET( *vector, AFF_COLDBLOOD ) )
      strcat( buf, " coldblood" );
   if( xIS_SET( *vector, AFF_FELINE ) )
      strcat( buf, " feline" );
   if( xIS_SET( *vector, AFF_NUISANCE ) )
      strcat( buf, " hellcurse" );
   if( xIS_SET( *vector, AFF_NONLIVING ) )
      strcat( buf, " nonliving" );
   if( xIS_SET( *vector, AFF_CONSTRUCT ) )
      strcat( buf, " construct" );
   if( xIS_SET( *vector, AFF_ETHEREAL ) )
      strcat( buf, " ethereal" );
   if( xIS_SET( *vector, AFF_NO_CORPSE ) )
      strcat( buf, " no_corpse" );
   if( xIS_SET( *vector, AFF_UNHOLY ) )
      strcat( buf, " unholy" );
   if( xIS_SET( *vector, AFF_HOLY ) )
      strcat( buf, " holy" );
   if( xIS_SET( *vector, AFF_ANGEL ) )
      strcat( buf, " angel" );
   if( xIS_SET( *vector, AFF_BEAUTY ) )
      strcat( buf, " beauty" );
   if( xIS_SET( *vector, AFF_FEAR ) )
      strcat( buf, " fear" );
   if( xIS_SET( *vector, AFF_HAS_ARTI ) )
      strcat( buf, " has_artifact" );
   if( xIS_SET( *vector, AFF_MAGICAL ) )
      strcat( buf, " magical" );
   return ( buf[0] != '\0' ) ? buf + 1 : "none";
}



/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( EXT_BV * extra_flags )
{
   static char buf[512];

   buf[0] = '\0';
   if( xIS_SET( *extra_flags, ITEM_GLOW ) )
      strcat( buf, " glow" );
   if( xIS_SET( *extra_flags, ITEM_HUM ) )
      strcat( buf, " hum" );
   if( xIS_SET( *extra_flags, ITEM_DARK ) )
      strcat( buf, " dark" );
   if( xIS_SET( *extra_flags, ITEM_LOYAL ) )
      strcat( buf, " loyal" );
   if( xIS_SET( *extra_flags, ITEM_EVIL ) )
      strcat( buf, " evil" );
   if( xIS_SET( *extra_flags, ITEM_INVIS ) )
      strcat( buf, " invis" );
   if( xIS_SET( *extra_flags, ITEM_MAGIC ) )
      strcat( buf, " magic" );
   if( xIS_SET( *extra_flags, ITEM_NODROP ) )
      strcat( buf, " nodrop" );
   if( xIS_SET( *extra_flags, ITEM_BLESS ) )
      strcat( buf, " bless" );
   if( xIS_SET( *extra_flags, ITEM_HOVER ) )
      strcat( buf, " hover" );
   if( xIS_SET( *extra_flags, ITEM_NO_RESET ) )
      strcat( buf, " noreset" );
   if( xIS_SET( *extra_flags, ITEM_NOREMOVE ) )
      strcat( buf, " noremove" );
   if( xIS_SET( *extra_flags, ITEM_INVENTORY ) )
      strcat( buf, " inventory" );
   if( xIS_SET( *extra_flags, ITEM_DEATHROT ) )
      strcat( buf, " deathrot" );
   if( xIS_SET( *extra_flags, ITEM_GROUNDROT ) )
      strcat( buf, " groundrot" );
   if( xIS_SET( *extra_flags, ITEM_DREAMWORLD ) )
      strcat( buf, " dreamworld" );
   if( xIS_SET( *extra_flags, ITEM_EXPLOSIVE ) )
      strcat( buf, " explosive" );
   if( xIS_SET( *extra_flags, ITEM_FLAMMABLE ) )
      strcat( buf, " flammable" );
   if( xIS_SET( *extra_flags, ITEM_NO_TAKE ) )
      strcat( buf, " no-take" );
   if( xIS_SET( *extra_flags, ITEM_TWO_HANDED ) )
      strcat( buf, " two-handed" );
   if( xIS_SET( *extra_flags, ITEM_SHIELD ) )
      strcat( buf, " shield" );
   if( xIS_SET( *extra_flags, ITEM_ORGANIC ) )
      strcat( buf, " organic" );
   if( xIS_SET( *extra_flags, ITEM_METAL ) )
      strcat( buf, " metal" );
   if( xIS_SET( *extra_flags, ITEM_RETURNING ) )
      strcat( buf, " returning" );
   if( xIS_SET( *extra_flags, ITEM_GEM ) )
      strcat( buf, " gem" );
   if( xIS_SET( *extra_flags, ITEM_ARTIFACT ) )
      strcat( buf, " artifact" );
   if( xIS_SET( *extra_flags, ITEM_PROTOTYPE ) )
      strcat( buf, " prototype" );
   if( xIS_SET( *extra_flags, ITEM_DURABLE ) )
      strcat( buf, " durable" );
   return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/*
 * Return ascii name of pulltype exit setting.
 */
char *pull_type_name( int pulltype )
{
   if( pulltype >= PT_FIRE )
      return ex_pfire[pulltype - PT_FIRE];
   if( pulltype >= PT_AIR )
      return ex_pair[pulltype - PT_AIR];
   if( pulltype >= PT_EARTH )
      return ex_pearth[pulltype - PT_EARTH];
   if( pulltype >= PT_WATER )
      return ex_pwater[pulltype - PT_WATER];
   if( pulltype < 0 )
      return "ERROR";

   return ex_pmisc[pulltype];
}

/*
 * Set off a trap (obj) upon character (ch)			-Thoric
 */
ch_ret spring_trap( CHAR_DATA * ch, OBJ_DATA * obj )
{
   int dam;
   int typ;
   int lev;
   char *txt;
   char buf[MAX_STRING_LENGTH];
   ch_ret retcode;
   AFFECT_DATA af;

   typ = obj->value[1];
   lev = obj->value[2];

   retcode = rNONE;

   switch ( typ )
   {
      default:
         txt = "hit by a trap";
         break;
      case TRAP_TYPE_POISON_GAS:
         txt = "surrounded by a green cloud of gas";
         break;
      case TRAP_TYPE_POISON_DART:
         txt = "hit by a dart";
         break;
      case TRAP_TYPE_POISON_NEEDLE:
         txt = "pricked by a needle";
         break;
      case TRAP_TYPE_POISON_DAGGER:
         txt = "stabbed by a dagger";
         break;
      case TRAP_TYPE_POISON_ARROW:
         txt = "struck with an arrow";
         break;
      case TRAP_TYPE_BLINDNESS_GAS:
         txt = "surrounded by a red cloud of gas";
         break;
      case TRAP_TYPE_SLEEPING_GAS:
         txt = "surrounded by a yellow cloud of gas";
         break;
      case TRAP_TYPE_FLAME:
         txt = "struck by a burst of flame";
         break;
      case TRAP_TYPE_EXPLOSION:
         txt = "hit by an explosion";
         break;
      case TRAP_TYPE_ACID_SPRAY:
         txt = "covered by a spray of acid";
         break;
      case TRAP_TYPE_ELECTRIC_SHOCK:
         txt = "suddenly shocked";
         break;
      case TRAP_TYPE_BLADE:
         txt = "sliced by a razor sharp blade";
         break;
      case TRAP_TYPE_SEX_CHANGE:
         txt = "surrounded by a mysterious aura";
         break;
   }

   dam = number_range( obj->value[2], obj->value[2] * 2 );
   sprintf( buf, "You are %s!", txt );
   act( AT_HITME, buf, ch, NULL, NULL, TO_CHAR );
   sprintf( buf, "$n is %s.", txt );
   act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
   --obj->value[0];
   if( obj->value[0] <= 0 )
      extract_obj( obj );
   switch ( typ )
   {
      default:
      case TRAP_TYPE_POISON_DART:
      case TRAP_TYPE_POISON_NEEDLE:
      case TRAP_TYPE_POISON_DAGGER:
      case TRAP_TYPE_POISON_ARROW:
         /*
          * hmm... why not use spell_poison() here? 
          */
/*	 retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL ); */
         /*
          * casting a spell will probably never hit them -keo 
          */
         if( retcode == rNONE )
            lose_hp( ch, dam );
      case TRAP_TYPE_POISON_GAS:
         if( !IS_SET( ch->immune, RIS_POISON ) )
         {
            af.type = gsn_poison;
            af.duration = number_range( 1, 20 ) * 20;
            af.location = APPLY_STR;
            af.modifier = -2;
            af.bitvector = meb( AFF_POISON );
            affect_join( ch, &af );
            ch->mental_state = URANGE( 20, ch->mental_state, 100 );
            act( AT_GREEN, "You feel very sick.", ch, NULL, NULL, TO_CHAR );
         }
/*	 retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL ); */
         break;
      case TRAP_TYPE_BLINDNESS_GAS:
         retcode = obj_cast_spell( gsn_blindness, lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_SLEEPING_GAS:
         retcode = obj_cast_spell( skill_lookup( "sleep" ), lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_ACID_SPRAY:
         lose_hp( ch, dam );
         break;
      case TRAP_TYPE_SEX_CHANGE:
         retcode = obj_cast_spell( skill_lookup( "change sex" ), lev, ch, ch, NULL );
         break;
      case TRAP_TYPE_FLAME:
      case TRAP_TYPE_EXPLOSION:
         lose_hp( ch, dam );
         break;
      case TRAP_TYPE_ELECTRIC_SHOCK:
         lose_hp( ch, dam );
         break;
      case TRAP_TYPE_BLADE:
         lose_hp( ch, dam );
         break;
   }
   return retcode;
}

/*
 * Check an object for a trap					-Thoric
 */
ch_ret check_for_trap( CHAR_DATA * ch, OBJ_DATA * obj, int flag )
{
   OBJ_DATA *check;
   ch_ret retcode;

   if( !obj->first_content )
      return rNONE;

   retcode = rNONE;

   for( check = obj->first_content; check; check = check->next_content )
      if( check->item_type == ITEM_TRAP && IS_SET( check->value[3], flag ) )
      {
         retcode = spring_trap( ch, check );
         if( retcode != rNONE )
            return retcode;
      }
   return retcode;
}

/*
 * Check the room for a trap					-Thoric
 */
ch_ret check_room_for_traps( CHAR_DATA * ch, int flag )
{
   OBJ_DATA *check;
   ch_ret retcode;

   retcode = rNONE;

   if( !ch )
      return rERROR;
   if( !ch->in_room || !ch->in_room->first_content )
      return rNONE;

   for( check = ch->in_room->first_content; check; check = check->next_content )
   {
      if( check->item_type == ITEM_TRAP && IS_SET( check->value[3], flag ) )
      {
         retcode = spring_trap( ch, check );
         if( retcode != rNONE )
            return retcode;
      }
   }
   return retcode;
}

/*
 * return TRUE if an object contains a trap			-Thoric
 */
bool is_trapped( OBJ_DATA * obj )
{
   OBJ_DATA *check;

   if( !obj->first_content )
      return FALSE;

   for( check = obj->first_content; check; check = check->next_content )
      if( check->item_type == ITEM_TRAP )
         return TRUE;

   return FALSE;
}

/*
 * If an object contains a trap, return the pointer to the trap	-Thoric
 */
OBJ_DATA *get_trap( OBJ_DATA * obj )
{
   OBJ_DATA *check;

   if( !obj->first_content )
      return NULL;

   for( check = obj->first_content; check; check = check->next_content )
      if( check->item_type == ITEM_TRAP )
         return check;

   return NULL;
}

/*
 * Return a pointer to the first object of a certain type found that
 * a player is carrying/wearing
 */
OBJ_DATA *get_objtype( CHAR_DATA * ch, sh_int type )
{
   OBJ_DATA *obj;

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
      if( obj->item_type == type )
         return obj;

   return NULL;
}

/*
 * Remove an exit from a room					-Thoric
 */
void extract_exit( ROOM_INDEX_DATA * room, EXIT_DATA * pexit )
{
   UNLINK( pexit, room->first_exit, room->last_exit, next, prev );
   if( pexit->rexit )
      pexit->rexit->rexit = NULL;
   STRFREE( pexit->keyword );
   STRFREE( pexit->description );
   DISPOSE( pexit );
}

/*
 * Remove a room
 */
void extract_room( ROOM_INDEX_DATA * room )
{
   bug( "extract_room: not implemented", 0 );
   /*
    * (remove room from hash table)
    * clean_room( room )
    * DISPOSE( room );
    */
   return;
}

/*
 * clean out a room (leave list pointers intact )		-Thoric
 */
void clean_room( ROOM_INDEX_DATA * room )
{
   EXTRA_DESCR_DATA *ed, *ed_next;
   EXIT_DATA *pexit, *pexit_next;

   STRFREE( room->description );
   STRFREE( room->name );
   for( ed = room->first_extradesc; ed; ed = ed_next )
   {
      ed_next = ed->next;
      STRFREE( ed->description );
      STRFREE( ed->keyword );
      DISPOSE( ed );
      top_ed--;
   }
   room->first_extradesc = NULL;
   room->last_extradesc = NULL;
   for( pexit = room->first_exit; pexit; pexit = pexit_next )
   {
      pexit_next = pexit->next;
      STRFREE( pexit->keyword );
      STRFREE( pexit->description );
      DISPOSE( pexit );
      top_exit--;
   }
   room->first_exit = NULL;
   room->last_exit = NULL;
   room->room_flags = 0;
   room->sector_type = 0;
}

/*
 * clean out an object (index) (leave list pointers intact )	-Thoric
 */
void clean_obj( OBJ_INDEX_DATA * obj )
{
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;
   EXTRA_DESCR_DATA *ed;
   EXTRA_DESCR_DATA *ed_next;

   STRFREE( obj->name );
   STRFREE( obj->short_descr );
   STRFREE( obj->description );
   STRFREE( obj->action_desc );
   obj->item_type = 0;
   xCLEAR_BITS( obj->extra_flags );
   obj->count = 0;
   obj->weight = 0;
   obj->cost = 0;
   obj->value[0] = 0;
   obj->value[1] = 0;
   obj->value[2] = 0;
   obj->value[3] = 0;
   for( paf = obj->first_affect; paf; paf = paf_next )
   {
      paf_next = paf->next;
      DISPOSE( paf );
      top_affect--;
   }
   obj->first_affect = NULL;
   obj->last_affect = NULL;
   for( ed = obj->first_extradesc; ed; ed = ed_next )
   {
      ed_next = ed->next;
      STRFREE( ed->description );
      STRFREE( ed->keyword );
      DISPOSE( ed );
      top_ed--;
   }
   obj->first_extradesc = NULL;
   obj->last_extradesc = NULL;
}

/*
 * clean out a mobile (index) (leave list pointers intact )	-Thoric
 */
void clean_mob( MOB_INDEX_DATA * mob )
{
   MPROG_DATA *mprog, *mprog_next;

   STRFREE( mob->player_name );
   STRFREE( mob->short_descr );
   STRFREE( mob->description );
   mob->spec_fun = NULL;
   xCLEAR_BITS( mob->progtypes );

   for( mprog = mob->mudprogs; mprog; mprog = mprog_next )
   {
      mprog_next = mprog->next;
      STRFREE( mprog->arglist );
      STRFREE( mprog->comlist );
      DISPOSE( mprog );
   }
   mob->count = 0;
   mob->killed = 0;
   mob->sex = 0;
   xCLEAR_BITS( mob->act );
   xCLEAR_BITS( mob->affected_by );
   mob->mobthac0 = 0;
   mob->ac = 0;
   mob->hitnodice = 0;
   mob->hitsizedice = 0;
   mob->hitplus = 0;
   mob->damnodice = 0;
   mob->damsizedice = 0;
   mob->damplus = 0;
   mob->gold = 0;
   mob->exp = 0;
   mob->position = 0;
   mob->defposition = 0;
   mob->height = 0;
   mob->weight = 0;  /* mob->speed     = 100; */
   xCLEAR_BITS( mob->attacks );
   xCLEAR_BITS( mob->defenses );
}

extern int top_reset;

/*
 * Remove all resets from a room -Thoric
 */
void clean_resets( ROOM_INDEX_DATA * room )
{
   RESET_DATA *pReset, *pReset_next;

   for( pReset = room->first_reset; pReset; pReset = pReset_next )
   {
      pReset_next = pReset->next;
      delete_reset( pReset );
      --top_reset;
   }
   room->first_reset = NULL;
   room->last_reset = NULL;
}

/*
 * "Fix" a character's stats					-Thoric
 */
void fix_char( CHAR_DATA * ch )
{
   AFFECT_DATA *aff;
   OBJ_DATA *obj;
   PART_DATA *part;

   de_equip_char( ch );

   part = ch->first_part;
   while( part )
   {
      if( ch->nation )
         part->armor = ch->nation->ac_plus;
      else
         part->armor = 0;
      part = part->next;
   }

   for( aff = ch->first_affect; aff; aff = aff->next )
      affect_modify( ch, aff, FALSE );

   xCLEAR_BITS( ch->affected_by );
   if( ch->nation )
      xSET_BITS( ch->affected_by, ch->nation->affected );
   if( ch->curr_talent[15] >= 5 )
   {
      xSET_BIT( ch->affected_by, AFF_DETECT_MAGIC );
      if( ch->curr_talent[15] >= 10 )
      {
         xSET_BIT( ch->affected_by, AFF_DETECT_INVIS );
         if( ch->curr_talent[15] >= 12 )
         {
            xSET_BIT( ch->affected_by, AFF_INFRARED );
            if( ch->curr_talent[15] >= 17 )
            {
               xSET_BIT( ch->affected_by, AFF_DETECT_EVIL );
               if( ch->curr_talent[15] >= 20 )
               {
                  xSET_BIT( ch->affected_by, AFF_DETECT_HIDDEN );
                  if( ch->curr_talent[15] >= 50 )
                  {
                     xSET_BIT( ch->affected_by, AFF_TRUESIGHT );
                  }
               }
            }
         }
      }
   }
   if( ch->nation )
      ch->armor = ch->nation->ac_plus;
   ch->mod_str = 0;
   ch->mod_dex = 0;
   ch->mod_wis = 0;
   ch->mod_int = 0;
   ch->mod_con = 0;
   ch->mod_cha = 0;
   ch->mod_lck = 0;
   ch->damroll = 0;
   ch->hitroll = 0;
   ch->encumberance = 0;
   ch->saving_breath = 0;
   ch->saving_wand = 0;
   ch->saving_para_petri = 0;
   ch->saving_spell_staff = 0;
   ch->saving_poison_death = 0;

   ch->carry_weight = 0;
   ch->carry_number = 0;

   for( aff = ch->first_affect; aff; aff = aff->next )
      affect_modify( ch, aff, TRUE );

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      if( obj->wear_loc == WEAR_NONE )
         ch->carry_number += get_obj_number( obj );
      if( !xIS_SET( obj->extra_flags, ITEM_MAGIC ) )
         ch->carry_weight += get_obj_weight( obj );
   }

   re_equip_char( ch );
   if( ch->main_hand )
      obj_affect_ch( ch, ch->main_hand );
   if( ch->off_hand )
      obj_affect_ch( ch, ch->off_hand );
}


/*
 * Show an affect verbosely to a character			-Thoric
 */
void showaffect( CHAR_DATA * ch, AFFECT_DATA * paf )
{
   int mod;
   char buf[MAX_STRING_LENGTH];
   int x;

   if( !paf )
   {
      bug( "showaffect: NULL paf", 0 );
      return;
   }
   if( paf->location != APPLY_NONE && paf->modifier != 0 )
   {
      switch ( paf->location )
      {
         default:
            /*
             * New Affect Display - Keolah Apr 21 2002 
             */
            mod = abs( paf->modifier );
            if( paf->modifier > 0 )
               sprintf( buf, "Enhances %s ", affect_loc_name( paf->location ) );
            else
               sprintf( buf, "Reduces %s ", affect_loc_name( paf->location ) );
            if( mod < 5 )
               strcat( buf, "slightly" );
            else if( mod < 10 )
               strcat( buf, "a bit" );
            else if( mod < 15 )
               strcat( buf, "a little" );
            else if( mod < 20 )
               strcat( buf, "somewhat" );
            else if( mod < 25 )
               strcat( buf, "fairly" );
            else if( mod < 30 )
               strcat( buf, "moderately" );
            else if( mod < 35 )
               strcat( buf, "a lot" );
            else if( mod < 40 )
               strcat( buf, "strongly" );
            else if( mod < 45 )
               strcat( buf, "a great deal" );
            else if( mod < 50 )
               strcat( buf, "greatly" );
            else if( mod < 60 )
               strcat( buf, "extremely" );
            else if( mod < 70 )
               strcat( buf, "supremely" );
            else if( mod < 80 )
               strcat( buf, "superbly" );
            else if( mod < 90 )
               strcat( buf, "powerfully" );
            else if( mod < 100 )
               strcat( buf, "incredibly" );
            else if( mod < 120 )
               strcat( buf, "amazingly" );
            else if( mod < 150 )
               strcat( buf, "unbelievably" );
            else
               strcat( buf, "impossibly" );
            strcat( buf, ".\n\r" );
            break;
         case APPLY_AFFECT:
            sprintf( buf, "Affects %s by", affect_loc_name( paf->location ) );
            for( x = 0; x < 32; x++ )
               if( IS_SET( paf->modifier, 1 << x ) )
               {
                  strcat( buf, " " );
                  strcat( buf, a_flags[x] );
               }
            strcat( buf, "\n\r" );
            break;
         case APPLY_WEAPONSPELL:
         case APPLY_WEARSPELL:
         case APPLY_REMOVESPELL:
            sprintf( buf, "Casts spell '%s'\n\r",
                     IS_VALID_SN( paf->modifier ) ? skill_table[paf->modifier]->name : "unknown" );
            break;
         case APPLY_RESISTANT:
         case APPLY_IMMUNE:
         case APPLY_SUSCEPTIBLE:
            sprintf( buf, "Affects %s by", affect_loc_name( paf->location ) );
            for( x = 0; x < 32; x++ )
               if( IS_SET( paf->modifier, 1 << x ) )
               {
                  strcat( buf, " " );
                  strcat( buf, ris_flags[x] );
               }
            strcat( buf, "\n\r" );
            break;
      }
      send_to_char( buf, ch );
   }
}

/*
 * Set the current global object to obj				-Thoric
 */
void set_cur_obj( OBJ_DATA * obj )
{
   cur_obj = obj->serial;
   cur_obj_extracted = FALSE;
   global_objcode = rNONE;
}

/*
 * Check the recently extracted object queue for obj		-Thoric
 */
bool obj_extracted( OBJ_DATA * obj )
{
   OBJ_DATA *cod;

   if( obj->serial == cur_obj && cur_obj_extracted )
      return TRUE;

   for( cod = extracted_obj_queue; cod; cod = cod->next )
      if( obj == cod )
         return TRUE;
   return FALSE;
}

/*
 * Stick obj onto extraction queue
 */
void queue_extracted_obj( OBJ_DATA * obj )
{
   ++cur_qobjs;
   obj->next = extracted_obj_queue;
   extracted_obj_queue = obj;
}

/*
 * Clean out the extracted object queue
 */
void clean_obj_queue(  )
{
   OBJ_DATA *obj;

   while( extracted_obj_queue )
   {
      obj = extracted_obj_queue;
      extracted_obj_queue = extracted_obj_queue->next;
      STRFREE( obj->name );
      STRFREE( obj->description );
      STRFREE( obj->short_descr );
      DISPOSE( obj );
      --cur_qobjs;
   }
}

/*
 * Set the current global character to ch			-Thoric
 */
void set_cur_char( CHAR_DATA * ch )
{
   cur_char = ch;
   cur_char_died = FALSE;
   cur_room = ch->in_room;
   global_retcode = rNONE;
}

/*
 * Check to see if ch died recently				-Thoric
 */
bool char_died( CHAR_DATA * ch )
{
   EXTRACT_CHAR_DATA *ccd;

   if( !ch->in_room )
      return TRUE;

   if( ch == cur_char && cur_char_died )
      return TRUE;

   for( ccd = extracted_char_queue; ccd; ccd = ccd->next )
      if( ccd->ch == ch )
         return TRUE;
   return FALSE;
}

/*
 * Add ch to the queue of recently extracted characters		-Thoric
 */
void queue_extracted_char( CHAR_DATA * ch, bool extract )
{
   EXTRACT_CHAR_DATA *ccd;

   if( !ch )
   {
      bug( "queue_extracted char: ch = NULL", 0 );
      return;
   }
   CREATE( ccd, EXTRACT_CHAR_DATA, 1 );
   ccd->ch = ch;
   ccd->room = ch->in_room;
   ccd->extract = extract;
   if( ch == cur_char )
      ccd->retcode = global_retcode;
   else
      ccd->retcode = rCHAR_DIED;
   ccd->next = extracted_char_queue;
   extracted_char_queue = ccd;
   cur_qchars++;
}

/*
 * clean out the extracted character queue
 */
void clean_char_queue(  )
{
   EXTRACT_CHAR_DATA *ccd;

   for( ccd = extracted_char_queue; ccd; ccd = extracted_char_queue )
   {
      extracted_char_queue = ccd->next;
      if( ccd->extract )
         free_char( ccd->ch );
      DISPOSE( ccd );
      --cur_qchars;
   }
}

/*
 * Add a timer to ch						-Thoric
 * Support for "call back" time delayed commands
 */
void add_timer( CHAR_DATA * ch, sh_int type, sh_int count, DO_FUN * fun, int value )
{
   TIMER *timer;

   for( timer = ch->first_timer; timer; timer = timer->next )
      if( timer->type == type )
      {
         timer->count = count;
         timer->do_fun = fun;
         timer->value = value;
         break;
      }
   if( !timer )
   {
      CREATE( timer, TIMER, 1 );
      timer->count = count;
      timer->type = type;
      timer->do_fun = fun;
      timer->value = value;
      LINK( timer, ch->first_timer, ch->last_timer, next, prev );
   }
}

TIMER *get_timerptr( CHAR_DATA * ch, sh_int type )
{
   TIMER *timer;

   for( timer = ch->first_timer; timer; timer = timer->next )
      if( timer->type == type )
         return timer;
   return NULL;
}

sh_int get_timer( CHAR_DATA * ch, sh_int type )
{
   TIMER *timer;

   if( ( timer = get_timerptr( ch, type ) ) != NULL )
      return timer->count;
   else
      return 0;
}

void extract_timer( CHAR_DATA * ch, TIMER * timer )
{
   if( !timer )
   {
      bug( "extract_timer: NULL timer", 0 );
      return;
   }

   UNLINK( timer, ch->first_timer, ch->last_timer, next, prev );
   DISPOSE( timer );
   return;
}

void remove_timer( CHAR_DATA * ch, sh_int type )
{
   TIMER *timer;

   for( timer = ch->first_timer; timer; timer = timer->next )
      if( timer->type == type )
         break;

   if( timer )
   {
      extract_timer( ch, timer );
   }
}


/*
 * Scryn, standard luck check 2/2/96
 */
bool chance( CHAR_DATA * ch, sh_int percent )
{
   sh_int ms;

   if( !ch )
   {
      bug( "Chance: null ch!", 0 );
      return FALSE;
   }

   ms = 10 - abs( ch->mental_state );

   if( ( number_percent(  ) - ( get_curr_lck( ch ) / 5 ) + 13 - ms ) <= percent )
      return TRUE;
   else
      return FALSE;
}

bool chance_attrib( CHAR_DATA * ch, sh_int percent, sh_int attrib )
{
/* Scryn, standard luck check + consideration of 1 attrib 2/2/96*/

   if( !ch )
   {
      bug( "Chance: null ch!", 0 );
      return FALSE;
   }

   if( number_percent(  ) - ( get_curr_lck( ch ) / 5 ) + 13 - attrib + 13 <= percent )
      return TRUE;
   else
      return FALSE;

}


/*
 * Make a simple clone of an object (no extras...yet)		-Thoric
 */
OBJ_DATA *clone_object( OBJ_DATA * obj )
{
   OBJ_DATA *clone;

   CREATE( clone, OBJ_DATA, 1 );
   clone->pIndexData = obj->pIndexData;
   clone->name = STRALLOC( obj->name );
   clone->short_descr = STRALLOC( obj->short_descr );
   clone->description = STRALLOC( obj->description );
   clone->action_desc = STRALLOC( obj->action_desc );
   clone->item_type = obj->item_type;
   clone->extra_flags = obj->extra_flags;
   clone->wear_loc = obj->wear_loc;
   clone->weight = obj->weight;
   clone->cost = obj->cost;
   clone->condition = obj->condition;
   clone->size = obj->size;
   clone->timer = obj->timer;
   clone->mana = obj->mana;
   clone->raw_mana = obj->raw_mana;
   clone->value[0] = obj->value[0];
   clone->value[1] = obj->value[1];
   clone->value[2] = obj->value[2];
   clone->value[3] = obj->value[3];
   clone->value[4] = obj->value[4];
   clone->value[5] = obj->value[5];
   clone->material = obj->material;
   clone->count = 1;
   ++obj->pIndexData->count;
   ++numobjsloaded;
   ++physicalobjects;
   cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
   clone->serial = clone->pIndexData->serial = cur_obj_serial;
   LINK( clone, first_object, last_object, next, prev );
   return clone;
}

/*
 * If possible group obj2 into obj1				-Thoric
 * This code, along with clone_object, obj->count, and special support
 * for it implemented throughout handler.c and save.c should show improved
 * performance on MUDs with players that hoard tons of potions and scrolls
 * as this will allow them to be grouped together both in memory, and in
 * the player files.
 */
OBJ_DATA *group_object( OBJ_DATA * obj1, OBJ_DATA * obj2 )
{
   AFFECT_DATA *paf1, *paf2;
   if( !obj1 || !obj2 )
      return NULL;
   if( obj1 == obj2 )
      return obj1;

   if( obj1->pIndexData == obj2->pIndexData && QUICKMATCH( obj1->name, obj2->name ) && QUICKMATCH( obj1->short_descr, obj2->short_descr ) && QUICKMATCH( obj1->description, obj2->description ) && QUICKMATCH( obj1->action_desc, obj2->action_desc ) && obj1->item_type == obj2->item_type && xSAME_BITS( obj1->extra_flags, obj2->extra_flags ) && obj1->weight == obj2->weight && obj1->wear_loc == obj2->wear_loc && obj1->raw_mana == obj2->raw_mana && obj1->mana == obj2->mana && obj1->timer == obj2->timer && obj1->value[0] == obj2->value[0] && obj1->value[1] == obj2->value[1] && obj1->value[2] == obj2->value[2] && obj1->value[3] == obj2->value[3] && obj1->value[4] == obj2->value[4] && obj1->value[5] == obj2->value[5] && !obj1->first_extradesc && !obj2->first_extradesc && !obj1->first_content && !obj2->first_content && !obj1->gem && !obj2->gem && obj1->count + obj2->count > 0 ) /* prevent count overflow */
   {
      if( obj1->first_affect )
      {
         paf2 = obj2->first_affect;
         for( paf1 = obj1->first_affect; paf1; paf1 = paf1->next )
         {
            if( !paf2 || !paf1 )
               return obj2;
            if( paf1->location != paf2->location )
               return obj2;
            if( paf1->modifier != paf2->modifier )
               return obj2;
            paf2 = paf2->next;
         }
      }
      obj1->count += obj2->count;
      obj1->pIndexData->count += obj2->count;   /* to be decremented in */
      numobjsloaded += obj2->count; /* extract_obj */
      extract_obj( obj2 );
      return obj1;
   }
   return obj2;
}

/*
 * Split off a grouped object					-Thoric
 * decreased obj's count to num, and creates a new object containing the rest
 */
void split_obj( OBJ_DATA * obj, int num )
{
   int count = obj->count;
   OBJ_DATA *rest;

   if( count <= num || num == 0 )
      return;

   rest = clone_object( obj );
   --obj->pIndexData->count;  /* since clone_object() ups this value */
   --numobjsloaded;
   rest->count = obj->count - num;
   obj->count = num;

   if( obj->carried_by )
   {
      LINK( rest, obj->carried_by->first_carrying, obj->carried_by->last_carrying, next_content, prev_content );
      rest->carried_by = obj->carried_by;
      rest->in_room = NULL;
      rest->in_obj = NULL;
   }
   else if( obj->in_room )
   {
      LINK( rest, obj->in_room->first_content, obj->in_room->last_content, next_content, prev_content );
      rest->carried_by = NULL;
      rest->in_room = obj->in_room;
      rest->in_obj = NULL;
   }
   else if( obj->in_obj )
   {
      LINK( rest, obj->in_obj->first_content, obj->in_obj->last_content, next_content, prev_content );
      rest->in_obj = obj->in_obj;
      rest->in_room = NULL;
      rest->carried_by = NULL;
   }
}

void separate_obj( OBJ_DATA * obj )
{
   split_obj( obj, 1 );
}

/*
 * Empty an obj's contents... optionally into another obj, or a room
 */
bool empty_obj( OBJ_DATA * obj, OBJ_DATA * destobj, ROOM_INDEX_DATA * destroom )
{
   OBJ_DATA *otmp, *otmp_next;
   CHAR_DATA *ch = obj->carried_by;
   bool movedsome = FALSE;

   if( !obj )
   {
      bug( "empty_obj: NULL obj", 0 );
      return FALSE;
   }
   if( destobj || ( !destroom && !ch && ( destobj = obj->in_obj ) != NULL ) )
   {
      for( otmp = obj->first_content; otmp; otmp = otmp_next )
      {
         otmp_next = otmp->next_content;
         /*
          * only keys on a keyring 
          */
         if( destobj->item_type == ITEM_KEYRING && otmp->item_type != ITEM_KEY )
            continue;
         if( destobj->item_type == ITEM_QUIVER && otmp->item_type != ITEM_PROJECTILE )
            continue;
         if( ( destobj->item_type == ITEM_CONTAINER || destobj->item_type == ITEM_KEYRING
               || destobj->item_type == ITEM_QUIVER )
             && get_real_obj_weight( otmp ) + get_real_obj_weight( destobj ) > destobj->value[0] )
            continue;
         obj_from_obj( otmp );
         obj_to_obj( otmp, destobj );
         movedsome = TRUE;
      }
      return movedsome;
   }
   if( destroom || ( !ch && ( destroom = obj->in_room ) != NULL ) )
   {
      for( otmp = obj->first_content; otmp; otmp = otmp_next )
      {
         otmp_next = otmp->next_content;
         if( ch && HAS_PROG( otmp->pIndexData, DROP_PROG ) && otmp->count > 1 )
         {
            separate_obj( otmp );
            obj_from_obj( otmp );
            if( !otmp_next )
               otmp_next = obj->first_content;
         }
         else
            obj_from_obj( otmp );
         otmp = obj_to_room( otmp, destroom );
         if( ch )
         {
            oprog_drop_trigger( ch, otmp );  /* mudprogs */
            if( char_died( ch ) )
               ch = NULL;
         }
         movedsome = TRUE;
      }
      return movedsome;
   }
   if( ch )
   {
      for( otmp = obj->first_content; otmp; otmp = otmp_next )
      {
         otmp_next = otmp->next_content;
         obj_from_obj( otmp );
         obj_to_char( otmp, ch );
         movedsome = TRUE;
      }
      return movedsome;
   }
   bug( "empty_obj: could not determine a destination for vnum %d", obj->pIndexData->vnum );
   return FALSE;
}

/*
 * Improve mental state						-Thoric
 */
void better_mental_state( CHAR_DATA * ch, int mod )
{
   int c = URANGE( 0, abs( mod ), 20 );
   int con = get_curr_con( ch );

   c += number_percent(  ) < con ? 1 : 0;

   if( ch->mental_state < 0 )
      ch->mental_state = URANGE( -100, ch->mental_state + c, 0 );
   else if( ch->mental_state > 0 )
      ch->mental_state = URANGE( 0, ch->mental_state - c, 100 );
}

/*
 * Deteriorate mental state					-Thoric
 */
void worsen_mental_state( CHAR_DATA * ch, int mod )
{
   int c = URANGE( 0, abs( mod ), 20 );
   int con = get_curr_con( ch );

   c -= number_percent(  ) < con ? 1 : 0;
   if( c < 1 )
      return;

   if( ch->mental_state < 0 )
      ch->mental_state = URANGE( -100, ch->mental_state - c, 100 );
   else if( ch->mental_state > 0 )
      ch->mental_state = URANGE( -100, ch->mental_state + c, 100 );
   else
      ch->mental_state -= c;
}


/*
 * Add gold to an area's economy				-Thoric
 */
void boost_economy( AREA_DATA * tarea, int gold )
{
   while( gold >= 1000000000 )
   {
      ++tarea->high_economy;
      gold -= 1000000000;
   }
   tarea->low_economy += gold;
   while( tarea->low_economy >= 1000000000 )
   {
      ++tarea->high_economy;
      tarea->low_economy -= 1000000000;
   }
}

/*
 * Take gold from an area's economy				-Thoric
 */
void lower_economy( AREA_DATA * tarea, int gold )
{
   while( gold >= 1000000000 )
   {
      --tarea->high_economy;
      gold -= 1000000000;
   }
   tarea->low_economy -= gold;
   while( tarea->low_economy < 0 )
   {
      --tarea->high_economy;
      tarea->low_economy += 1000000000;
   }
}

/*
 * Check to see if economy has at least this much gold		   -Thoric
 */
bool economy_has( AREA_DATA * tarea, int gold )
{
   int hasgold = ( ( tarea->high_economy > 0 ) ? 1 : 0 ) * 1000000000 + tarea->low_economy;

   if( hasgold >= gold )
      return TRUE;
   return FALSE;
}

/*
 * Used in db.c when resetting a mob into an area		    -Thoric
 * Makes sure mob doesn't get more than 10% of that area's gold,
 * and reduces area economy by the amount of gold given to the mob
 */
void economize_mobgold( CHAR_DATA * mob )
{

   /*
    * make sure it isn't way too much 
    */
   mob->gold = UMIN( mob->gold, get_char_worth( mob ) * 400 );

/* but then leave it alone -keo */
/*    if ( !mob->in_room )
	return;
    tarea = mob->in_room->area;

   gold = ((tarea->high_economy > 0) ? 1 : 0) * 1000000000 +
tarea->low_economy;
    mob->gold = URANGE( 0, mob->gold, gold / 10 );
    if ( mob->gold )
	lower_economy( tarea, mob->gold ); */
}


/*
 * returns area with name matching input string
 * Last Modified : July 21, 1997
 * Fireblade
 */
AREA_DATA *get_area( char *name )
{
   AREA_DATA *pArea;

   if( !name )
   {
      bug( "get_area: NULL input string." );
      return NULL;
   }

   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      if( nifty_is_name( name, pArea->name ) )
         break;
   }

   if( !pArea )
   {
      for( pArea = first_build; pArea; pArea = pArea->next )
      {
         if( nifty_is_name( name, pArea->name ) )
            break;
      }
   }

   return pArea;
}

/* Return a material given the number -- Scion */
MATERIAL_DATA *material_lookup( int number )
{
   MATERIAL_DATA *material;

   material = first_material;
   if( !material )
   {
      bug( "No materials exist in material_lookup.", 0 );
      return NULL;
   }

   while( material )
   {
      if( material->number == number )
         return material;
      material = material->next;
   }
   return NULL;
}

MUT_DATA *find_mutation( int i )
{
   MUT_DATA *mut;

   mut = first_mutation;
   if( !mut )
   {
      bug( "No mutations exist in find_mutation.", 0 );
      return NULL;
   }

   while( mut )
   {
      if( mut->number == i )
         return mut;
      mut = mut->next;
   }
   return NULL;
}

RECIPE_DATA *find_recipe( int i )
{
   RECIPE_DATA *recipe;

   recipe = first_recipe;
   if( !recipe )
   {
      bug( "No recipes exist in find_recipe.", 0 );
      return NULL;
   }

   while( recipe )
   {
      if( recipe->number == i )
         return recipe;
      recipe = recipe->next;
   }
   return NULL;
}

INGRED_DATA *find_ingred( int i )
{
   INGRED_DATA *ingred;

   ingred = first_ingred;
   if( !ingred )
   {
      bug( "No ingredients exist in find_ingred.", 0 );
      return NULL;
   }

   while( ingred )
   {
      if( ingred->vnum == i )
         return ingred;
      ingred = ingred->next;
   }
   return NULL;
}


MAT_SHOP *find_matshop( int i )
{
   MAT_SHOP *matshop;

   matshop = first_matshop;
   if( !matshop )
   {
      bug( "No matshop exist in find_matshop.", 0 );
      return NULL;
   }

   while( matshop )
   {
      if( matshop->room == i )
         return matshop;
      if( matshop == last_matshop )
         break;
      matshop = matshop->next;
   }
   return NULL;
}


MOON_DATA *find_moon( char *name )
{
   MOON_DATA *moon;

   moon = first_moon;
   if( !moon )
   {
      bug( "No moons exist in find_moon.", 0 );
      return NULL;
   }

   while( moon )
   {
      if( !str_cmp( moon->name, name ) )
         return moon;
      moon = moon->next;
   }
   return NULL;
}

SPECIES_DATA *find_species( char *name )
{
   SPECIES_DATA *species;

   species = first_species;
   if( !species )
   {
      bug( "No species exist in find_species.", 0 );
      return NULL;
   }

   while( species )
   {
      if( !str_cmp( species->name, name ) )
         return species;
      if( species == last_species )
         return NULL;
      species = species->next;
   }
   return NULL;
}

NATION_DATA *find_nation( char *name )
{
   NATION_DATA *nation;

   nation = first_nation;
   if( !nation )
   {
      bug( "No nations exist in find_nations.", 0 );
      return NULL;
   }

   while( nation )
   {
      if( !str_cmp( nation->name, name ) )
         return nation;
      nation = nation->next;
   }
   return NULL;
}

/* Generate a map of the area surrounding a character */
void show_room_map( CHAR_DATA * ch )
{
   char map[11][11];
   char buf[MAX_STRING_LENGTH];
   int x, y;

   if( !ch->in_room )
   {
      send_to_char( "Something is terribly wrong...\r\n", ch );
      return;
   }

   for( y = 0; y < 11; y++ )
   {
      for( x = 0; x < 11; x++ )
      {
         if( x == y && x == 5 )
            map[x][y] = '@';
         else if( x == 10 || y == 10 )
            map[x][y] = '\0';
         else
            map[x][y] = ' ';
      }
   }


   for( y = 0; y < 10; y++ )
   {
      strcpy( buf, map[y] );
      send_to_char( buf, ch );
      send_to_char( "\r\n", ch );
   }
}

/* Generate a room description for ch based on various aspects
of the room including trails, weather, character's race, time of day,
etc. Finally, strip line breaks and add them again at their correct
places. -- Scion */
char *get_room_description( CHAR_DATA * ch, char *descr )
{
   char message[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char temp[MAX_STRING_LENGTH];
   char rdesc[MAX_STRING_LENGTH];
   int i, letters, space, newspace, line;
   int lights = get_light_room( ch->in_room );
   TRAIL_DATA *trail;
   EXIT_DATA *pexit;
   /*
    * Get a separate buffer for each type of trail info and then tack them
    * together at the end. This will use more memory, but will put all the
    * different types of trail info together, plus it will only use one for
    * loop instead of several. -- Scion 
    */
   char graffiti[MAX_STRING_LENGTH];
   char blood[MAX_STRING_LENGTH];
   char snow[MAX_STRING_LENGTH];
   char trails[MAX_STRING_LENGTH];

   /*
    * Count the number of each type of message, and abbreviate in case of
    * very long strings 
    */
   int num_graffiti, num_blood, num_snow, num_trails = 0;

   strcpy( graffiti, "" );
   strcpy( blood, "" );
   strcpy( snow, "" );
   strcpy( trails, "" );

   /*
    * Get room's desc first, tack other stuff in after that. 
    */
   strcpy( buf, ch->in_room->description );

   /*
    * Weather 
    */
   strcat( buf, get_weather_string( ch, temp ) );

   /*
    * Comment on the light level 
    */
   if( lights < -10 )
      strcpy( message, "It is extremely dark. " );
   else if( lights < -5 )
      strcpy( message, "It is very dark here. " );
   else if( lights <= 0 )
      if( room_is_dark( ch->in_room ) )
         strcpy( message, "It is dark. " );
      else
         strcpy( message, "There aren't any lights here. " );
   else if( lights == 1 )
      strcpy( message, "A single light illuminates the area. " );
   else if( lights == 2 )
      strcpy( message, "A pair of lights shed light on the surroundings. " );
   else if( lights < 7 )
      strcpy( message, "A few lights scattered around provide lighting. " );
   else
      strcpy( message, "Numerous lights shine from all around. " );

   /*
    * One sentence about the room we're in 
    */
   switch ( ch->in_room->sector_type )
   {
      case 0:
         break;
      case 1:
         strcat( message, " The city street is" );
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " lined by lush trees and hedges" );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " interspersed with small shrubs along its edges" );
         else if( ch->in_room->curr_vegetation > 5 )
            strcat( message, " dotted with small weeds" );
         else
            strcat( message, " clean and barren" );

         if( ch->in_room->curr_resources > 66 )
            strcat( message, ", and piles of trash and refuse" );
         else if( ch->in_room->curr_resources > 33 )
            strcat( message, ", and scattered with litter" );

         if( ch->in_room->curr_water > 75 )
            strcat( message, " just under the water" );
         else if( ch->in_room->curr_water > 25 )
            strcat( message, " in the mud" );

         strcat( message, ". " );

         break;
      case 2:
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " Tall grass and shrubs" );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " Low patches of grass" );
         else
            strcat( message, " Small clumps of weeds" );

         if( ch->in_room->curr_resources > 66 )
            strcat( message, " obscure broken bits of wood" );
         else
            strcat( message, " grow from the soil" );

         if( ch->in_room->curr_water > 66 )
            strcat( message, " visible through the water" );
         else if( ch->in_room->curr_water > 33 )
            strcat( message, " just under the muddy water" );

         strcat( message, ". " );

         break;
      case 3:
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " Enormous, dark evergreen trees loom tall amidst" );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " Thin evergreen trees are scattered all across" );
         else
            strcat( message, " Gnarled, dead trees twist toward the sky from" );

         if( ch->in_room->curr_water > 66 )
            strcat( message, " swirling water" );
         else if( ch->in_room->curr_water > 33 )
            strcat( message, " deep puddles scattered here and there" );
         else
            strcat( message, " the forest floor" );

         if( ch->in_room->curr_resources > 66 )
            strcat( message, ", scattered wood and underbrush obscuring the ground. " );
         else
            strcat( message, ". " );

         break;
      case 4:
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " Trees and shrubs" );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " Grasses and small trees" );
         else
            strcat( message, " Barren dirt and rocks" );

         if( ch->in_room->curr_water > 66 )
            strcat( message, " adorn the dry areas between the intertwining streams and ponds" );
         else if( ch->in_room->curr_water > 33 )
            strcat( message, " grow atop the rolling hills, small streams coursing between the hills" );
         else
            strcat( message, " rise and fall with the rolling hills throughout this area" );

         if( ch->in_room->curr_resources > 66 )
            strcat( message, ", scraps of wood littering the ground. " );
         else
            strcat( message, ". " );

         break;
      case 5:
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " Lush evergreen trees grow along" );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " Scraggly trees and bushes cling to" );
         else
            strcat( message, " Rocky, steep terrain leads up into " );

         if( ch->in_room->curr_water > 66 )
            strcat( message, " the cliffs, a cascading waterfall roaring over the edge." );
         else if( ch->in_room->curr_water > 33 )
            strcat( message, " the mountainous terrain, a trickling stream running down from the snow melt." );
         else
            strcat( message, " the mountainside." );

         break;
      case 6:
         /*
          * Is the water frozen? 
          */
         if( IS_OUTSIDE( ch ) && ( ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit < 3 ) )
            strcat( message, " The shallow water here is frozen enough to walk on. " );
         else
            strcat( message, " Shallow water flows past, bubbling and gurgling over smooth rocks. " );
         break;
      case 7:
         /*
          * Is the water frozen? 
          */
         if( IS_OUTSIDE( ch ) && ( ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit < 3 ) )
            strcat( message, " A layer of thick ice has formed over the deep water here. " );
         else
            strcat( message, " The deep water is nearly black as it rushes past. " );
         break;
      case 8:
         strcat( message, " Water swirls all around. " );
         break;
      case 9:
         strcat( message, " The thin air swirls past, barely making even its presence known. " );
         break;
      case 10:
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " An abundance of cacti scatter the sandy landscape. " );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " Several patches of grass hold out against the ruthless sand. " );
         else
            strcat( message, " The sandy soil only supports the smallest traces of life. " );
         break;
      case 11:
         strcat( message, " The details of this location are strangely difficult to determine. " );
         break;
      case 12:
         strcat( message, " The water bed is covered with" );
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " thick kelp. " );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " some scattered kelp and moss. " );
         else if( ch->in_room->curr_resources > 66 )
            strcat( message, " scattered rocks, pearls and shells. " );
         else
            strcat( message, " bare sand. " );
         break;
      case 13:
         strcat( message, " The rocky surroundings are" );
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " obscured by a grove of giant mushrooms" );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " covered with multicolored lichens" );
         else
            strcat( message, " bare" );

         if( ch->in_room->curr_resources > 66 )
            strcat( message, " with a sparkling glint every so often along the wall. " );
         else
            strcat( message, " as far as the eye can see. " );
         break;
      case 14:
         strcat( message, " Molten lava flows through the volcanic caverns. " );
         break;
      case 15:
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " A canopy of tropical trees hangs over this lush jungle. " );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " The muddy swamplands are scattered with bogs and pools. " );
         else
            strcat( message, " The misty moor is covered with tiny wildflowers and grasses. " );
         break;
      case 16:
         strcat( message, " The solid ice shows no trace of life. " );
         break;
      case 17:
         if( ch->in_room->curr_vegetation > 66 )
            strcat( message, " Lush palm trees dot this pristine beach. " );
         else if( ch->in_room->curr_vegetation > 33 )
            strcat( message, " Growths of beach grass line the sand dunes of this beach. " );
         else
            strcat( message, " This wide open beach is empty but for some broken seashells and bits of driftwood. " );
         break;
      default:
         strcat( message, "" );
         break;
   }
   strcat( buf, message );

   /*
    * List appropriate room flags 
    */
   strcpy( message, "" );
   if( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) )
   {
      if( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
         strcat( message, "There seems to be something blocking the flow of magical energy here. " );
      if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
         strcat( message, "A magical aura seems to promote a feeling of peace. " );
      if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) )
         strcat( message, "A very weak pulling sensation emenates from this place. " );
      if( IS_SET( ch->in_room->room_flags, ROOM_NO_SUMMON ) )
         strcat( message, "The very air seems in some way resilient. " );
      if( IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL ) )
         strcat( message, "The air seems a little thicker in here than it ought to be. " );
      if( IS_SET( ch->in_room->room_flags, ROOM_NOSUPPLICATE ) )
         strcat( message, "This place feels as if is has been shunned by the gods! " );
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_DARK ) )
      strcat( message, "It is quite dark in here. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_TUNNEL ) )
      strcat( message, "There is only enough room for a few people in this cramped space. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_PRIVATE ) )
      strcat( message, "A sign on the wall states, 'This room is private.' " );
   if( IS_SET( ch->in_room->room_flags, ROOM_SOLITARY ) )
      strcat( message, "There appears to be only enough space in here for one person. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) )
      strcat( message, "There is nothing but open air below here. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_AMPLIFY ) )
      strcat( message, "Even the tiniest noise echoes loudly in here. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_NOMISSILE ) )
      strcat( message, "There does not appear to be enough open space to use missile weapons here. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_STICKY ) )
      strcat( message, "The floor seems very sticky. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_SLIPPERY ) )
      strcat( message, "The floor seems very slippery. " );
   if( IS_SET( ch->in_room->room_flags, ROOM_BURNING ) )
      strcat( message, "The room is on fire! " );
   if( ch->in_room->runes )
      strcat( message, "Glowing runes line the walls and floor. " );
   strcat( buf, message );

   /*
    * List exits 
    */
   for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( pexit->to_room )
      {
         if( IS_SET( pexit->exit_info, EX_HIDDEN ) || IS_SET( pexit->exit_info, EX_SECRET ) )
            continue;
         strcpy( message, "" );
         if( IS_SET( pexit->exit_info, EX_ISDOOR ) )
         {
            if( IS_SET( pexit->exit_info, EX_BASHED ) )
               strcat( message, "bashed in " );
            else
            {
               if( IS_SET( pexit->exit_info, EX_NOPASSDOOR ) || IS_SET( pexit->exit_info, EX_BASHPROOF ) )
                  strcat( message, "thick " );
               if( IS_SET( pexit->exit_info, EX_CLOSED ) )
                  strcat( message, "closed " );
               else
                  strcat( message, "open " );
            }

            if( strcmp( pexit->keyword, "" ) )
               strcat( message, pexit->keyword );
            else
               strcat( message, "door" );
            strcat( message, " leads " );
            strcat( message, dir_name[pexit->vdir] );
            strcat( message, ". " );
            strcat( buf, capitalize( aoran( message ) ) );
         }
         else
         {
            /*
             * Don't say anything about normal exits, only interesting ones 
             */
            if( pexit->to_room->sector_type != ch->in_room->sector_type )
            {
               strcpy( temp, "" );
               switch ( pexit->to_room->sector_type )
               {
                  case 0:
                     strcat( temp, "A building lies %s from here. " );
                     break;
                  case 1:
                     strcat( temp, "A well worn road leads %s from here. " );
                     break;
                  case 2:
                     strcat( temp, "Fields lie to the %s. " );
                     break;
                  case 3:
                     strcat( temp, "Tall trees obscure the horizon to the %s. " );
                     break;
                  case 4:
                     strcat( temp, "Rough, hilly terrain lies %s from here. " );
                     break;
                  case 5:
                     strcat( temp, "Steep mountains loom to the %s. " );
                     break;
                  case 6:
                     /*
                      * Is the water frozen? 
                      */
                     if( IS_OUTSIDE( ch ) && ( ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit < 3 ) )
                        strcat( temp, "Some fairly shallow water to the %s seems frozen enough to walk on. " );
                     else
                        strcat( temp, "Fairly shallow water is visible to the %s. " );
                     break;
                  case 7:
                     /*
                      * Is the water frozen? 
                      */
                     if( IS_OUTSIDE( ch ) && ( ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit < 3 ) )
                        strcat( temp, "The water to the %s is frozen solid. " );
                     else
                        strcat( temp, "The water %s of here looks quite deep. " );
                     break;
                  case 8:
                     strcat( temp, "Murky water swirls endlessly %s of here. " );
                     break;
                  case 9:
                     strcat( temp, "There is nothing but open air %s from here. " );
                     break;
                  case 10:
                     strcat( temp, "Desert sands reach %s into the distance. " );
                     break;
                  case 12:
                     strcat( temp, "The sandy ocean floor stretches %s. " );
                     break;
                  case 13:
                     strcat( temp, "Dimly lit caverns continue %s from here. " );
                     break;
                  case 14:
                     strcat( temp, "Molten lava flows %s from here. " );
                     break;
                  case 15:
                     strcat( temp, "Muddy swamplands continue to the %s. " );
                     break;
                  case 16:
                     strcat( temp, "Solid ice is visible to the %s. " );
                     break;
                  case 17:
                     strcat( temp, "The beach can be seen to the %s. " );
               }
               sprintf( message, temp, dir_name[pexit->vdir] );
               strcat( buf, message );
            }
         }
      }
   }

   /*
    * Generate a sentence about each object in the room, grouping like objects to
    * keep the list short. 
    */
   /*
    * <article> <object> <verb> [adjective] <location>. 
    */
   /*
    * commented out until it can be made less spammy -keo 
    */
/*	for (obj = ch->in_room->first_content; obj; obj = obj->next_content) {
		extern char *munch_colors(char *word);

		char sentence[MAX_STRING_LENGTH];
		char temp[MAX_STRING_LENGTH];

		if (!can_see_obj(ch, obj))
			continue;

		strcpy(sentence, munch_colors(obj->short_descr));

		if (IS_OBJ_STAT(obj, ITEM_HOVER)) {
			switch (obj->serial % 3) {
			case 1: strcat(sentence, " floats"); break;
			case 2:	strcat(sentence, " hovers"); break;
			case 3: strcat(sentence, " flies"); break;
			default: strcat(sentence, " drifts"); break;
			}
		} else {
			switch (obj->serial % 3) {
			case 1:	strcat(sentence, " sits"); break;
			case 2:	strcat(sentence, " rests"); break;
			case 3:	strcat(sentence, " lies"); break;
			default: strcat(sentence, " has been set"); break;
			}
		}

		if (number_percent() < 25) {
			switch (obj->serial % 10) {
			case 1: strcat(sentence, " quietly"); break;
			case 2: strcat(sentence, " heavily"); break;
			case 3: strcat(sentence, " lazily"); break;
			case 4: strcat(sentence, " solemnly"); break;
			case 5: strcat(sentence, " conspicuously"); break;
			case 6: strcat(sentence, " hopefully"); break;
			case 7: strcat(sentence, " upright"); break;
			case 8: strcat(sentence, " overturned"); break;
			case 9: strcat(sentence, " auspiciously"); break;
			case 10:strcat(sentence, " comfortably"); break;
			default:strcat(sentence, " innocently"); break;
			}
		}

		i = obj->serial % 10;

		if (IS_OUTSIDE(ch)) {
			if (i <= 8)
				strcat(sentence, " just %s of here. ");
			else if (i == 9)
				strcat(sentence, " near your head. ");
			else
				strcat(sentence, " nearby. ");
		} else {
			if (i <= 4)
				strcat(sentence, " along the %s wall. ");
			else if (i <= 8)
				strcat(sentence, " in the %s corner. ");
			else if (i == 9)
				strcat(sentence, " %s the ceiling. ");
			else
				strcat(sentence, " %s the floor. ");
		}

		switch (obj->serial % 10) {
		case 0: strcpy(temp, "north"); break;
		case 1: strcpy(temp, "south"); break;
		case 2: strcpy(temp, "east"); break;
		case 3: strcpy(temp, "west"); break;
		case 4: strcpy(temp, "northwest"); break;
		case 5: strcpy(temp, "northeast"); break;
		case 6: strcpy(temp, "southwest"); break;
		case 7: strcpy(temp, "southeast"); break;
		case 8:
		case 9:
			if (IS_OBJ_STAT(obj, ITEM_HOVER))
				strcpy(temp, "near");
			else
				strcpy(temp, "on");
			break;
		}

		sprintf(message, sentence, temp);
		strcat(buf, capitalize(message));
	}
*/
   /*
    * Collect trail descriptions 
    */
   for( trail = ch->in_room->first_trail; trail; trail = trail->next )
   {
      int i = ( ( ( int )trail->age - current_time ) + 1800 );

      /*
       * Collect graffiti first 
       */
      if( trail->graffiti && strlen( trail->graffiti ) > 3 )
      {
         if( ch->curr_talent[TAL_TIME] + ch->curr_talent[TAL_SEEKING] > 50 )
         {
            strcpy( message, capitalize( trail->name ) );
            strcat( message, " has scrawled something here: " );
         }
         else
            strcpy( message, "Something has been scrawled here: " );
         strcat( message, trail->graffiti );
         strcat( message, " " );
         strcat( graffiti, message );
         num_graffiti++;
      }

      /*
       * Get blood trails next 
       */
      if( ( trail->blood == TRUE && i > 500 ) &&
          ( ch->in_room->sector_type < 6 || ch->in_room->sector_type == 10 || ch->in_room->sector_type == 13 ) )
      {

         if( i > 1700 )
            strcpy( message, "A fresh pool of blood covers the floor, leading from %s to %s. " );
         else if( i > 1600 )
            strcpy( message, "A bright red streak of blood leads from %s to %s. " );
         else if( i > 1500 )
            strcpy( message, "Wet, bloody footprints lead from %s to %s. " );
         else if( i > 1400 )
            strcpy( message, "Bloody footprints lead from %s to %s. " );
         else if( i > 1300 )
            strcpy( message, "A wet trail of dark red blood leads %s. " );
         else if( i > 1200 )
            strcpy( message, "A trail of wet, sticky blood leads %s. " );
         else if( i > 1100 )
            strcpy( message, "A drying trail of blood leads %s. " );
         else if( i > 1000 )
            strcpy( message, "Some nearly dried blood leads to the %s from here. " );
         else if( i > 900 )
            strcpy( message, "A distinct trail of dry blood leads %s. " );
         else if( i > 800 )
            strcpy( message, "A trail of dried blood leads %s. " );
         else if( i > 700 )
            strcpy( message, "A bit of dried blood seems to lead %s. " );
         else if( i > 500 )
            strcpy( message, "A few drops of dry blood are visible on the floor. " );
         else
            strcpy( message, "A flake of dried blood catches your eye. " );

         sprintf( temp, message,
                  ( trail->from > -1 ? rev_dir_name[trail->from] : "the center of the room" ),
                  ( trail->to > -1 ? dir_name[trail->to] : "right here" ) );
         strcat( temp, " " );
         strcat( blood, temp );
         num_blood++;
         continue;
      }

      /*
       * Show tracks if it's snowing *grin* -- Scion 
       */
      if( IS_OUTSIDE( ch ) && ( ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit < 3 )
          && ( ( ch->in_room->area->weather->precip + 3 * weath_unit - 1 ) / weath_unit > 3 ) )
      {
         strcpy( message, "Footprints in the snow seem to lead from %s to %s. " );
         sprintf( temp, message,
                  ( trail->from > -1 ? rev_dir_name[trail->from] : "the center of the room" ),
                  ( trail->to > -1 ? dir_name[trail->to] : "right here" ) );
         strcat( temp, " " );
         strcat( snow, temp );
         num_snow++;
         continue;
      }

      /*
       * Show trails to those with a track skill -- Scion 
       */
      if( !IS_NPC( ch ) && LEARNED( ch, gsn_track ) > 0 )
      {
         if( trail->blood == FALSE
             && ch->in_room->sector_type != 0
             && ch->in_room->sector_type != 1
             && ch->in_room->sector_type < 6
             && trail->fly == FALSE
             && strcmp( trail->name, ch->name )
             && ( number_range( 1, 100 ) < number_range( 1, ch->pcdata->noncombat[SK_NATURE] + get_curr_per( ch ) ) ) )
         {

            if( i > 1350 )
               strcpy( message, "Distinct footprints lead from %s to %s, apparently made by %s" );
            else if( i > 900 )
               strcpy( message, "Footprints lead from %s to %s" );
            else if( i > 450 )
               strcpy( message, "A faint set of footprints seems to lead %s" );
            else
               strcpy( message, "You notice a footprint on the ground" );
            learn_from_success( ch, gsn_track );
            sprintf( temp, message,
                     ( trail->from > -1 ? rev_dir_name[trail->from] : "right here" ),
                     ( trail->to > -1 ? dir_name[trail->to] : "right here" ),
                     trail->race ? ( aoran( trail->race->name ) ) : "some unknown creature" );
            strcat( temp, ". " );
            strcat( trails, temp );
            num_trails++;
            continue;
         }

      }
   }  /* For loop */

   /*
    * Check if we need to abridge any of this 
    */
   if( strlen( graffiti ) > 200 )
      strcpy( graffiti, "The area is littered with too many scrawled messages to make any out clearly. " );

   if( strlen( blood ) > 200 )
      strcpy( blood, "This place is awash in bloody trails of all kinds, leading in all directions. " );

   if( strlen( snow ) > 200 )
      strcpy( snow, "The snow is trampled with countless footprints. " );

   if( strlen( trails ) > 200 )
      strcpy( trails,
              "Dozens of footprints lead in all directions, making it impossible to distinguish one from the others. " );

   strcat( buf, " " );
   strcat( buf, graffiti );
   strcat( buf, " " );
   strcat( buf, blood );
   strcat( buf, " " );
   strcat( buf, snow );
   strcat( buf, " " );
   strcat( buf, trails );
   strcat( buf, " " );

   i = 0;
   letters = 0;

   /*
    * Strip \r and \n 
    */
   for( i = 0; i < strlen( buf ); i++ )
   {
      if( buf[i] != '\r' && buf[i] != '\n' )
      {
         rdesc[letters] = buf[i];
         letters++;
      }
      else if( buf[i] == '\r' )
      {
         rdesc[letters] = ' ';
         letters++;
      }
      rdesc[letters] = '\0';
   }

   i = 0;
   letters = 0;
   space = 0;
   newspace = 0;
   line = 0;
   strcpy( buf, rdesc );

   /*
    * Add \r\n's back in at their appropriate places 
    */
   for( i = 0; i < strlen( buf ); i++ )
   {
      if( buf[i] == ' ' )
      {
         space = i;
         newspace = letters;
      }

      if( line > 70 )
      {
         i = space;
         letters = newspace;
         rdesc[letters++] = '\r';
         rdesc[letters++] = '\n';
         line = 0;
      }
      else if( !( buf[i] == ' ' && buf[i + 1] == ' ' ) )
      {
         rdesc[letters] = buf[i];
         letters++;  /* Index for rdesc; i is the index for buf */
         line++;  /* Counts number of characters on this line */
      }
      rdesc[letters + 1] = '\0';
   }
   if( strlen( rdesc ) > 0 )
      strcat( rdesc, "\r\n" );

   descr = STRALLOC( rdesc );

   return descr;
}


/* Strip the color codes in a string and return it without them -- Scion */
char *munch_colors( char *word )
{
   char buf[MAX_STRING_LENGTH];
   int i;
   int a;

   i = 0;
   a = 0;

   while( word[i] != '\0' && i < MAX_STRING_LENGTH )
   {
      if( word[i] != '&' && ( i == 0 || word[i - 1] != '&' ) )
      {
         buf[a] = word[i];
         a++;
      }
      i++;
   }

   buf[a] = '\0';

   word = STRALLOC( buf );

   return word;
}


DEITY_DATA *get_talent_by_index( int index )
{
   DEITY_DATA *talent;

   for( talent = first_deity; talent; talent = talent->next )
   {
      if( talent->index == index )
         return talent;
   }

   return NULL;
}
