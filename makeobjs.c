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
 *			Specific object creation module			    *
 ****************************************************************************/
/* Now handles random mob generation also -keo */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* from handler.c */
extern MATERIAL_DATA *material_lookup( int number );
extern SPECIES_DATA *find_species( char *name );
extern void learn_noncombat( CHAR_DATA * ch, int i );
extern NATION_DATA *find_nation( char *name );

extern int top_affect;

AREA_DATA *stockobj;

/* Turn an ordinary item into a powerful artifact! */
void make_randart( CHAR_DATA * ch, OBJ_DATA * obj )
{
   char buf[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   AFFECT_DATA *naf;
   char name[MAX_STRING_LENGTH];
   int bane;
   int power = 200 - obj->material->magic;

   bane = number_range( 1, 17 );
   strcpy( buf, obj->short_descr );
   strcat( buf, " of " );
   if( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_DRINK_CON )
   {
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            strcat( buf, "holding" );
            obj->value[0] += number_range( power, power * 10 );
            obj->cost += obj->value[0] * 100;
            xSET_BIT( obj->extra_flags, ITEM_MAGIC );
            break;
         case 2:
            strcat( buf, "light" );
            obj->cost += 10000;
            xSET_BIT( obj->extra_flags, ITEM_GLOW );
            break;
         case 3:
            strcat( buf, "darkness" );
            obj->cost += 10000;
            xSET_BIT( obj->extra_flags, ITEM_DARK );
            break;
         case 4:
            strcat( buf, "levitation" );
            obj->cost += 10000;
            xSET_BIT( obj->extra_flags, ITEM_HOVER );
            break;
         case 5:
            strcat( buf, "loyalty" );
            obj->cost += 500000;
            xSET_BIT( obj->extra_flags, ITEM_LOYAL );
            break;
      }
   }
   else if( obj->item_type == ITEM_FURNITURE || obj->item_type == ITEM_VEHICLE )
   {
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            strcat( buf, "durability" );
            obj->value[0] += number_range( power, power * 10 );
            obj->cost += obj->value[0] * 100;
            xSET_BIT( obj->extra_flags, ITEM_MAGIC );
            break;
         case 2:
            strcat( buf, "loyalty" );
            xSET_BIT( obj->extra_flags, ITEM_LOYAL );
            break;
         case 3:
            strcat( buf, "darkness" );
            xSET_BIT( obj->extra_flags, ITEM_DARK );
            break;
         case 4:
            strcat( buf, "levitation" );
            xSET_BIT( obj->extra_flags, ITEM_HOVER );
            break;
         case 5:
            strcat( buf, "light" );
            xSET_BIT( obj->extra_flags, ITEM_GLOW );
            break;
      }
   }
   else if( obj->item_type == ITEM_BOOK )
   {
      switch ( number_range( 1, 8 ) )
      {
         case 1:
            strcat( buf, "knowledge" );
            break;
         case 2:
            strcat( buf, "loyalty" );
            xSET_BIT( obj->extra_flags, ITEM_LOYAL );
            break;
         case 3:
            strcat( buf, "darkness" );
            xSET_BIT( obj->extra_flags, ITEM_DARK );
            break;
         case 4:
            strcat( buf, "levitation" );
            xSET_BIT( obj->extra_flags, ITEM_HOVER );
            break;
         case 5:
            strcat( buf, "light" );
            xSET_BIT( obj->extra_flags, ITEM_GLOW );
            break;
         case 6:
            strcat( buf, "wisdom" );
            break;
         case 7:
            strcat( buf, "intelligence" );
            break;
         case 8:
            strcat( buf, "learning" );
            break;
         case 9:
            strcat( buf, "magic" );
            xSET_BIT( obj->extra_flags, ITEM_MAGIC );
            break;
         case 10:
            strcat( buf, "spells" );
            xSET_BIT( obj->extra_flags, ITEM_MAGIC );
            break;
         case 11:
            strcat( buf, "power" );
            xSET_BIT( obj->extra_flags, ITEM_HUM );
            xSET_BIT( obj->extra_flags, ITEM_MAGIC );
            xSET_BIT( obj->extra_flags, ITEM_GLOW );
            break;
      }
   }
   else if( obj->item_type == ITEM_WEAPON && number_range( 1, 3 ) == 1 )
   {
      xSET_BIT( obj->extra_flags, ITEM_MAGIC );
      obj->value[0] = bane;
      obj->value[6] = number_range( power, power * 10 );
      switch ( bane )
      {
         case 1:
            strcat( buf, "undead slaying" );
            break;
         case 2:
            strcat( buf, "demon slaying" );
            break;
         case 3:
            strcat( buf, "elf slaying" );
            break;
         case 4:
            strcat( buf, "orc slaying" );
            break;
         case 5:
            strcat( buf, "dragon slaying" );
            break;
         case 6:
            strcat( buf, "human slaying" );
            break;
         case 7:
            strcat( buf, "dwarf slaying" );
            break;
         case 8:
            strcat( buf, "faerie slaying" );
            break;
         case 9:
            strcat( buf, "giant slaying" );
            break;
         case 10:
            strcat( buf, "minotaur slaying" );
            break;
         case 11:
            strcat( buf, "troll slaying" );
            break;
         case 12:
            strcat( buf, "halfling slaying" );
            break;
         case 13:
            strcat( buf, "animal slaying" );
            break;
         case 14:
            strcat( buf, "feline slaying" );
            break;
         case 15:
            strcat( buf, "equine slaying" );
            break;
         case 16:
            strcat( buf, "construct slaying" );
            break;
         case 17:
            strcat( buf, "angel slaying" );
            break;
      }
   }
   else
   {
      CREATE( paf, AFFECT_DATA, 1 );
      paf->type = -1;
      paf->duration = -1;
      xCLEAR_BITS( paf->bitvector );
      switch ( number_range( 0, 45 ) )
      {
         default:
            strcat( buf, "power" );
            paf->location = APPLY_STR;
            paf->modifier = number_range( 1, ( int )power / 3 );
            CREATE( naf, AFFECT_DATA, 1 );
            naf->type = -1;
            naf->duration = -1;
            naf->location = APPLY_INT;
            naf->modifier = number_range( 1, ( int )power / 3 );
            xCLEAR_BITS( naf->bitvector );
            naf->next = NULL;
            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
            break;
         case 1:
            strcat( buf, "thievery" );
            paf->location = APPLY_DEX;
            paf->modifier = number_range( 1, ( int )power / 3 );
            CREATE( naf, AFFECT_DATA, 1 );
            naf->type = -1;
            naf->duration = -1;
            naf->location = APPLY_CHA;
            naf->modifier = number_range( 1, ( int )power / 3 );
            xCLEAR_BITS( naf->bitvector );
            naf->next = NULL;
            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
            break;
         case 2:
            strcat( buf, "wizardry" );
            paf->location = APPLY_INT;
            paf->modifier = number_range( 1, ( int )power / 3 );
            CREATE( naf, AFFECT_DATA, 1 );
            naf->type = -1;
            naf->duration = -1;
            naf->location = APPLY_WIS;
            naf->modifier = number_range( 1, ( int )power / 3 );
            xCLEAR_BITS( naf->bitvector );
            naf->next = NULL;
            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
            break;
         case 3:
            strcat( buf, "might" );
            paf->location = APPLY_CON;
            paf->modifier = number_range( 1, ( int )power / 3 );
            CREATE( naf, AFFECT_DATA, 1 );
            naf->type = -1;
            naf->duration = -1;
            naf->location = APPLY_STR;
            naf->modifier = number_range( 1, ( int )power / 3 );
            xCLEAR_BITS( naf->bitvector );
            naf->next = NULL;
            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
            break;
         case 4:
            strcat( buf, "sight" );
            paf->location = APPLY_CHA;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 5:
            strcat( buf, "endurance" );
            paf->location = APPLY_LCK;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 6:
            strcat( buf, "will" );
            paf->location = APPLY_WIS;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 7:
            strcat( buf, "fire" );
            paf->location = APPLY_FIRE_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_FLAMING;
            break;
         case 8:
            strcat( buf, "protection" );
            paf->location = APPLY_AC;
            paf->modifier = number_range( 1, power );
            xSET_BIT( obj->extra_flags, ITEM_SHIELD );
            if( obj->item_type == ITEM_ARMOR )
               obj->value[0] += number_range( 1, ( int )power / 3 );
            obj->cost += obj->value[0] * 1000;
            break;
         case 9:
            strcat( buf, "ice" );
            paf->location = APPLY_FROST_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_FROZEN;
            break;
         case 10:
            strcat( buf, "lightning" );
            paf->location = APPLY_LIGHTNING_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_ELEC;
            break;
         case 11:
            strcat( buf, "levitation" );
            paf->location = APPLY_AFFECT;
            SET_BIT( paf->modifier, 1 << AFF_FLYING );
            break;
         case 12:
            strcat( buf, "breathing" );
            paf->location = APPLY_AFFECT;
            obj->cost += 100000;
            SET_BIT( paf->modifier, 1 << AFF_AQUA_BREATH );
            break;
         case 13:
            strcat( buf, "nightvision" );
            paf->location = APPLY_AFFECT;
            SET_BIT( paf->modifier, 1 << AFF_INFRARED );
            break;
         case 14:
            strcat( buf, "light" );
            xSET_BIT( obj->extra_flags, ITEM_GLOW );
            paf->location = APPLY_HITROLL;
            paf->modifier = number_range( 1, ( int )power / 4 );
            xSET_BIT( paf->bitvector, AFF_GLOW );
            break;
         case 15:
            strcat( buf, "darkness" );
            xSET_BIT( obj->extra_flags, ITEM_DARK );
            paf->location = APPLY_DAMROLL;
            paf->modifier = number_range( 1, ( int )power / 4 );
            xSET_BIT( paf->bitvector, AFF_DARK );
            break;
         case 16:
            strcat( buf, "true sight" );
            paf->location = APPLY_CHA;
            obj->cost += 100000;
            paf->modifier = number_range( 1, ( int )power / 3 );
            xSET_BIT( paf->bitvector, AFF_TRUESIGHT );
            break;
         case 17:
            strcat( buf, "feather fall" );
            paf->location = APPLY_AFFECT;
            obj->cost += 5000;
            SET_BIT( paf->modifier, 1 << AFF_FLOATING );
            break;
         case 18:
            strcat( buf, "doom" );
            paf->location = APPLY_AFFECT;
            SET_BIT( paf->modifier, 1 << AFF_CURSE );
            break;
         case 19:
            strcat( buf, "pass door" );
            paf->location = APPLY_AFFECT;
            obj->cost += 10000;
            SET_BIT( paf->modifier, 1 << AFF_PASS_DOOR );
            break;
         case 20:
            strcat( buf, "mind" );
            paf->location = APPLY_MIND_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_PSIONIC;
            break;
         case 21:
            strcat( buf, "radiance" );
            paf->location = APPLY_ILLUSION_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_RADIANT;
            break;
         case 22:
            strcat( buf, "stealth" );
            paf->location = APPLY_AFFECT;
            SET_BIT( paf->modifier, 1 << AFF_SNEAK );
            break;
         case 23:
            strcat( buf, "shadows" );
            paf->location = APPLY_AFFECT;
            SET_BIT( paf->modifier, 1 << AFF_HIDE );
            break;
         case 24:
            strcat( buf, "seeking" );
            paf->location = APPLY_SEEKING_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            break;
         case 25:
            strcat( buf, "storms" );
            paf->location = APPLY_WIND_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_STORM;
            break;
         case 26:
            strcat( buf, "sound" );
            paf->location = APPLY_SPEECH_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_SONIC;
            break;
         case 27:
            strcat( buf, "draining" );
            paf->location = APPLY_DEATH_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_DRAIN;
            break;
         case 28:
            strcat( buf, "distortion" );
            paf->location = APPLY_CHANGE_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_DISTORT;
            break;
         case 29:
            strcat( buf, "earth" );
            paf->location = APPLY_EARTH_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_CRUSH;
            break;
         case 30:
            strcat( buf, "anti-matter" );
            paf->location = APPLY_VOID_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            if( obj->item_type == ITEM_WEAPON )
               obj->value[4] = BRAND_ANTI_MATTER;
            break;
         case 31:
            strcat( buf, "strength" );
            paf->location = APPLY_STR;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 32:
            strcat( buf, "mana" );
            paf->location = APPLY_INT;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 33:
            strcat( buf, "agility" );
            paf->location = APPLY_DEX;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 34:
            strcat( buf, "vigor" );
            paf->location = APPLY_CON;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 35:
            strcat( buf, "dream" );
            paf->location = APPLY_DREAM_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            break;
         case 36:
            strcat( buf, "warding" );
            paf->location = APPLY_SECURITY_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            break;
         case 37:
            strcat( buf, "water" );
            paf->location = APPLY_WATER_MAGIC;
            paf->modifier = 1;
            obj->cost += 600000;
            xSET_BIT( paf->bitvector, AFF_AQUA_BREATH );
            break;
         case 38:
            strcat( buf, "accuracy" );
            paf->location = APPLY_HITROLL;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 39:
            strcat( buf, "damage" );
            paf->location = APPLY_DAMROLL;
            paf->modifier = number_range( 1, ( int )power / 3 );
            break;
         case 40:
            strcat( buf, "time" );
            paf->location = APPLY_TIME_MAGIC;
            paf->modifier = 1;
            obj->cost += 500000;
            break;
         case 41:
            strcat( buf, "flight" );
            paf->location = APPLY_DEX;
            paf->modifier = number_range( 1, ( int )power / 4 );
            xSET_BIT( paf->bitvector, AFF_FLYING );
            break;
         case 42:
            strcat( buf, "the cat" );
            paf->location = APPLY_DEX;
            paf->modifier = number_range( 1, ( int )power / 2 );
            CREATE( naf, AFFECT_DATA, 1 );
            naf->type = -1;
            naf->duration = -1;
            naf->location = APPLY_CHA;
            naf->modifier = number_range( 1, ( int )power / 2 );
            xCLEAR_BITS( naf->bitvector );
            xSET_BIT( naf->bitvector, AFF_FELINE );
            naf->next = NULL;
            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
            break;
         case 43:
            strcat( buf, "the snake" );
            paf->location = APPLY_DEX;
            paf->modifier = number_range( 1, ( int )power / 2 );
            break;
         case 44:
            strcat( buf, "the dragon" );
            paf->location = APPLY_STR;
            paf->modifier = number_range( 1, ( int )power / 2 );
            CREATE( naf, AFFECT_DATA, 1 );
            naf->type = -1;
            naf->duration = -1;
            naf->location = APPLY_CON;
            naf->modifier = number_range( 1, ( int )power / 2 );
            xCLEAR_BITS( naf->bitvector );
            xSET_BIT( naf->bitvector, AFF_COLDBLOOD );
            naf->next = NULL;
            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
            break;
         case 45:
            strcat( buf, "loyalty" );
            xSET_BIT( obj->extra_flags, ITEM_LOYAL );
            paf->location = APPLY_WIS;
            paf->modifier = number_range( 1, ( int )power / 5 );
            break;
      }
      if( number_range( 1, 10 ) == 1 && number_range( 1, 50 ) * 100 > numobjsloaded )
      {
         if( obj->last_affect && obj->last_affect->location != APPLY_AFFECT )
            obj->last_affect->modifier += number_range( 1, ( int )power / 3 );
         if( paf->location != APPLY_AFFECT )
            paf->modifier += number_range( 1, ( int )power / 3 );
         strcat( buf, " named " );
         if( ch->nation && !str_cmp( ch->nation->name, "drow" ) )
            strcat( buf, random_name( name, LANG_DROW ) );
         else if( ch->race == 1 || ch->race == 4 )
            strcat( buf, random_name( name, LANG_ELVEN ) );
         else if( ch->race == 2 )
            strcat( buf, random_name( name, LANG_DWARVEN ) );
         else if( ch->race == 35 )
            strcat( buf, random_name( name, LANG_DRAGON ) );
         else
            strcat( buf, random_name( name, LANG_KALORESE ) );
         if( obj->item_type == ITEM_WEAPON && number_range( 1, 2 ) == 1 )
         {
            xSET_BIT( obj->extra_flags, ITEM_MAGIC );
            obj->value[0] = bane;
            obj->value[6] = number_range( power * 2, power * 20 );
         }
         xSET_BIT( obj->extra_flags, ITEM_ARTIFACT );
      }
      if( paf->location != APPLY_AFFECT )
         obj->cost += paf->modifier * 1000;
      paf->next = NULL;
      LINK( paf, obj->first_affect, obj->last_affect, next, prev );
   }
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( obj->name );
   obj->name = STRALLOC( buf );
}

/* Generate a mobile based on the nation passed in. */
CHAR_DATA *generate_mob_nation( char *argument )
{
   NATION_DATA *nation = NULL;

   for( nation = first_nation; nation; nation = nation->next )
   {
      if( nifty_is_name( argument, nation->name ) )
         return generate_mob( nation );
   }
   return NULL;
}

/* Note: The random item generation WILL crash the mud if the item
 * does not exist that it is looking for! Please do not delete items in
 * stockobj.are unless you edit this file as well
 */
CHAR_DATA *generate_mob( NATION_DATA * nation )
{
   SPECIES_DATA *species;
   CHAR_DATA *mob = NULL;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   OBJ_DATA *obj = NULL;
   MATERIAL_DATA *mat;
   AFFECT_DATA *paf;
   char name[MAX_STRING_LENGTH];
   BOOK_DATA *book;
   EXTRA_DESCR_DATA *ed;
   int i, n;

   mob = create_mobile( get_mob_index( MOB_VNUM_GENERIC ) );
   mob->nation = nation;
   species = find_species( nation->species );
   if( species )
      mob->race = species->skill_set;

   sprintf( buf, nation->name );

   mob->perm_str = 100 + nation->str_mod;
   mob->perm_int = 100 + nation->int_mod;
   mob->perm_wis = 100 + nation->wis_mod;
   mob->perm_dex = 100 + nation->dex_mod;
   mob->perm_con = 100 + nation->con_mod;
   mob->perm_cha = 100 + nation->cha_mod;
   mob->perm_lck = 100 + nation->lck_mod;

   mob->susceptible = nation->suscept;
   mob->speed = 100;
   mob->weight = nation->weight;
   mob->height = nation->height;

   mob->xflags = nation->parts;
   check_bodyparts( mob );
   mob->affected_by = nation->affected;
   if( IS_SET( mob->xflags, 1 << PART_WINGS ) )
      xSET_BIT( mob->affected_by, AFF_FLYING );
   mob->immune = nation->resist;

   if( IS_SET( mob->xflags, 1 << PART_HANDS ) )
   {
      if( number_range( 1, 25 ) == 1 )
      {
         xSET_BIT( mob->act, ACT_NAMED );
         if( !str_cmp( mob->nation->name, "drow" ) )
            sprintf( buf2, "%s, the %s", random_name( name, LANG_DROW ), buf );
         else if( mob->race == 1 || mob->race == 4 )
            sprintf( buf2, "%s, the %s", random_name( name, LANG_ELVEN ), buf );
         else if( mob->race == 2 )
            sprintf( buf2, "%s, the %s", random_name( name, LANG_DWARVEN ), buf );
         else if( mob->race == 35 )
            sprintf( buf2, "%s, the %s", random_name( name, LANG_DRAGON ), buf );
         else
            sprintf( buf2, "%s, the %s", random_name( name, LANG_KALORESE ), buf );
         strcpy( buf, buf2 );
         mob->perm_str += 50;
         mob->perm_int += 50;
         mob->perm_wis += 50;
         mob->perm_dex += 50;
         mob->perm_con += 50;
         mob->perm_lck += 50;
         mob->perm_cha += 50;
      }
      switch ( number_range( 1, 10 ) )
      {
         default:
            mob->gold += number_range( 1, 1000 );
            switch ( number_range( 1, 10 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 28911 ), 0 );  /* shirt */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29109 ), 0 );  /* tunic */
                  break;
               case 3:
                  if( IS_SET( mob->xflags, PART_LEGS ) )
                     obj = create_object( get_obj_index( 28910 ), 0 );  /* pants */
                  else
                     obj = create_object( get_obj_index( 28917 ), 0 );  /* saddle */
                  break;
               case 4:
                  obj = create_object( get_obj_index( 29007 ), 0 );  /* belt */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 29014 ), 0 );  /* cloak */
                  break;
               case 6:
                  obj = create_object( get_obj_index( 29072 ), 0 );  /* pouch */
                  break;
               case 7:
                  obj = create_object( get_obj_index( 29062 ), 0 );  /* pack */
                  break;
               case 8:
                  obj = create_object( get_obj_index( 28937 ), 0 );  /* codpiece */
                  break;
               case 9:
                  obj = create_object( get_obj_index( 29029 ), 0 );  /* knife */
                  break;
               case 10:
                  obj = create_object( get_obj_index( 29053 ), 0 );  /* cap */
                  break;
            }
            break;
         case 1:
            switch ( number_range( 1, 5 ) )
            {
               case 1:
                  mob->perm_cha -= 50;
                  mob->perm_int -= 50;
                  mob->perm_wis -= 50;
                  mob->gold += number_range( 10, 500 );
                  strcat( buf, " squire" );
                  break;
               case 2:
                  mob->perm_str += 20;
                  mob->perm_con += 20;
                  mob->gold += number_range( 100, 1000 );
                  strcat( buf, " soldier" );
                  break;
               case 3:
                  mob->perm_str += 50;
                  mob->perm_con += 50;
                  mob->perm_dex += 50;
                  mob->gold += number_range( 2000, 20000 );
                  strcat( buf, " warrior" );
                  break;
               case 4:
                  mob->perm_str += 100;
                  mob->perm_con += 100;
                  mob->perm_dex += 100;
                  mob->perm_cha += 50;
                  mob->gold += number_range( 10000, 50000 );
                  strcat( buf, " knight" );
                  break;
               case 5:
                  mob->perm_str += 200;
                  mob->perm_con += 200;
                  mob->perm_dex += 100;
                  mob->perm_cha += 50;
                  mob->perm_lck += 50;
                  mob->gold += number_range( 20000, 100000 );
                  strcat( buf, " blademaster" );
                  break;
            }
            switch ( number_range( 1, 21 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 29033 ), 0 );  /* longsword */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29000 ), 0 );  /* chainmail */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 29039 ), 0 );  /* battle axe */
                  break;
               case 4:
                  if( IS_SET( mob->xflags, 1 << PART_FEET ) )
                     obj = create_object( get_obj_index( 29009 ), 0 );  /* shod boots */
                  else if( IS_SET( mob->xflags, 1 << PART_HOOVES ) )
                     obj = create_object( get_obj_index( 29085 ), 0 );  /* horseshoes */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 29012 ), 0 );  /* gauntlets */
                  break;
               case 6:
                  obj = create_object( get_obj_index( 29018 ), 0 );  /* greaves */
                  break;
               case 7:
                  obj = create_object( get_obj_index( 29005 ), 0 );  /* greathelm */
                  break;
               case 8:
                  obj = create_object( get_obj_index( 28905 ), 0 );  /* kite shield */
                  break;
               case 9:
                  obj = create_object( get_obj_index( 29041 ), 0 );  /* spear */
                  break;
               case 10:
                  obj = create_object( get_obj_index( 28942 ), 0 );  /* glaive */
                  break;
               case 11:
                  obj = create_object( get_obj_index( 29042 ), 0 );  /* halberd */
                  break;
               case 12:
                  obj = create_object( get_obj_index( 29035 ), 0 );  /* greatsword */
                  break;
               case 13:
                  obj = create_object( get_obj_index( 29001 ), 0 );  /* platemail */
                  break;
               case 14:
                  obj = create_object( get_obj_index( 29020 ), 0 );  /* ringmail */
                  break;
               case 15:
                  obj = create_object( get_obj_index( 29063 ), 0 );  /* broadsword */
                  break;
               case 16:
                  obj = create_object( get_obj_index( 29004 ), 0 );  /* helm */
                  break;
               case 17:
                  obj = create_object( get_obj_index( 29040 ), 0 );  /* greataxe */
                  break;
               case 18:
                  obj = create_object( get_obj_index( 29045 ), 0 );  /* crossbow */
                  break;
               case 19:
                  obj = create_object( get_obj_index( 28926 ), 0 );  /* club */
                  break;
               case 20:
                  obj = create_object( get_obj_index( 28936 ), 0 );  /* flail */
                  break;
               case 21:
                  obj = create_object( get_obj_index( 29025 ), 0 );  /* visor */
                  break;
            }
            break;
         case 2:
            if( mob->perm_int < 0 )
               break;
            switch ( number_range( 1, 5 ) )
            {
               case 1:
                  mob->perm_dex -= 50;
                  mob->perm_str -= 50;
                  mob->perm_con -= 50;
                  mob->gold += number_range( 10, 500 );
                  strcat( buf, " apprentice" );
                  break;
               case 2:
                  mob->perm_wis += 20;
                  mob->perm_int += 20;
                  mob->gold += number_range( 100, 1000 );
                  strcat( buf, " mage" );
                  break;
               case 3:
                  mob->perm_wis += 50;
                  mob->perm_int += 50;
                  mob->perm_dex += 50;
                  mob->gold += number_range( 2000, 20000 );
                  strcat( buf, " sorcerer" );
                  break;
               case 4:
                  mob->perm_wis += 100;
                  mob->perm_int += 100;
                  mob->perm_cha += 100;
                  mob->perm_con += 50;
                  mob->gold += number_range( 10000, 50000 );
                  strcat( buf, " wizard" );
                  break;
               case 5:
                  mob->perm_wis += 200;
                  mob->perm_int += 200;
                  mob->perm_cha += 100;
                  mob->perm_con += 50;
                  mob->perm_dex += 50;
                  mob->perm_lck += 50;
                  mob->gold += number_range( 20000, 100000 );
                  strcat( buf, " archmage" );
                  break;
            }
            if( IS_AFFECTED( mob, AFF_AQUA_BREATH ) )
               mob->spec_fun = spec_lookup( "spec_cast_water" );
            else if( IS_SET( mob->xflags, PART_WINGS ) )
               mob->spec_fun = spec_lookup( "spec_cast_wind" );
            else
               mob->spec_fun = spec_lookup( "spec_cast_fire" );
            switch ( number_range( 1, 12 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 28955 ), 0 );  /* staff */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29013 ), 0 );  /* robe */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 29030 ), 0 );  /* dagger */
                  break;
               case 4:
                  obj = create_object( get_obj_index( 28916 ), 0 );  /* sash */
                  break;
               case 5:
                  if( IS_SET( mob->xflags, 1 << PART_FEET ) )
                     obj = create_object( get_obj_index( 28931 ), 0 );  /* sandals */
                  else if( IS_SET( mob->xflags, 1 << PART_HOOVES ) )
                     obj = create_object( get_obj_index( 29085 ), 0 );  /* horseshoes */
                  break;
               case 6:
                  obj = create_object( get_obj_index( 29055 ), 0 );  /* quarterstaff */
                  break;
               case 7:
                  obj = create_object( get_obj_index( 29002 ), 0 );  /* ring */
                  break;
               case 8:
                  obj = create_object( get_obj_index( 29064 ), 0 );  /* amulet */
                  break;
               case 9:
                  obj = create_object( get_obj_index( 29103 ), 0 );  /* glasses */
                  break;
               case 10:
                  obj = create_object( get_obj_index( 29072 ), 0 );  /* pouch */
                  break;
               case 11:
                  obj = create_object( get_obj_index( 29066 ), 0 );  /* wand */
                  break;
               case 12:
                  obj = create_object( get_obj_index( 28928 ), 0 );  /* book */
                  break;
            }
            break;
         case 3:
            if( mob->perm_int < 0 )
               break;
            /*
             * Let's not have zombie healers 
             */
            if( IS_AFFECTED( mob, AFF_UNDEAD ) || IS_AFFECTED( mob, AFF_DEMON ) )
            {
               switch ( number_range( 1, 4 ) )
               {
                  case 1:
                     mob->perm_dex -= 50;
                     mob->perm_str -= 50;
                     mob->perm_cha -= 50;
                     mob->perm_con -= 50;
                     mob->gold += number_range( 1, 10 );
                     strcat( buf, " leper" );
                     break;
                  case 2:
                     mob->perm_wis += 50;
                     mob->perm_int += 20;
                     mob->gold += number_range( 100, 1000 );
                     strcat( buf, " poison mage" );
                     break;
                  case 3:
                     mob->perm_wis += 50;
                     mob->perm_int += 50;
                     mob->gold += number_range( 1500, 5000 );
                     strcat( buf, " venom wizard" );
                     break;
                  case 4:
                     mob->perm_wis += 100;
                     mob->perm_int += 100;
                     mob->perm_cha += 100;
                     mob->perm_con += 50;
                     mob->perm_str += 50;
                     mob->gold += number_range( 1000, 10000 );
                     strcat( buf, " plaguebringer" );
                     break;
               }
               mob->spec_fun = spec_lookup( "spec_cast_poison" );
            }
            else
            {
               switch ( number_range( 1, 4 ) )
               {
                  case 1:
                     mob->perm_dex -= 20;
                     mob->perm_str -= 50;
                     mob->perm_con -= 50;
                     mob->gold += number_range( 1, 5 );
                     strcat( buf, " treehugger" );
                     break;
                  case 2:
                     mob->perm_wis += 20;
                     mob->perm_int += 20;
                     mob->gold += number_range( 10, 100 );
                     strcat( buf, " druid" );
                     break;
                  case 3:
                     mob->perm_wis += 50;
                     mob->perm_int += 50;
                     mob->perm_str += 50;
                     mob->perm_con += 50;
                     mob->perm_dex += 50;
                     mob->gold += number_range( 1000, 10000 );
                     strcat( buf, " ranger" );
                     break;
                  case 4:
                     mob->perm_wis += 100;
                     mob->perm_int += 100;
                     mob->perm_cha += 100;
                     mob->perm_con += 50;
                     mob->gold += number_range( 1000, 5000 );
                     strcat( buf, " earth warden" );
                     break;
               }
               mob->spec_fun = spec_lookup( "spec_cast_earth" );
            }
            switch ( number_range( 1, 11 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 29013 ), 0 );  /* robe */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29036 ), 0 );  /* mace */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 28955 ), 0 );  /* staff */
                  break;
               case 4:
                  obj = create_object( get_obj_index( 29055 ), 0 );  /* quarterstaff */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 28930 ), 0 );  /* hood */
                  break;
               case 6:
                  obj = create_object( get_obj_index( 29072 ), 0 );  /* pouch */
                  break;
               case 7:
                  obj = create_object( get_obj_index( 29065 ), 0 );  /* talisman */
                  break;
               case 8:
                  obj = create_object( get_obj_index( 29002 ), 0 );  /* ring */
                  break;
               case 9:
                  obj = create_object( get_obj_index( 29064 ), 0 );  /* amulet */
                  break;
               case 10:
                  obj = create_object( get_obj_index( 29014 ), 0 );  /* cloak */
                  break;
               case 11:
                  obj = create_object( get_obj_index( 28928 ), 0 );  /* book */
                  break;
            }
            break;
         case 4:
            if( mob->perm_wis < 0 )
               break;
            /*
             * Let's not have zombie healers 
             */
            if( IS_AFFECTED( mob, AFF_UNDEAD ) || IS_AFFECTED( mob, AFF_DEMON ) )
            {
               switch ( number_range( 1, 4 ) )
               {
                  case 1:
                     mob->perm_dex -= 50;
                     mob->perm_str -= 50;
                     mob->perm_cha -= 50;
                     mob->perm_con -= 50;
                     mob->gold += number_range( 1, 10 );
                     strcat( buf, " acolyte" );
                     break;
                  case 2:
                     mob->perm_wis += 50;
                     mob->perm_int += 20;
                     mob->gold += number_range( 100, 1000 );
                     strcat( buf, " dark priest" );
                     break;
                  case 3:
                     mob->perm_wis += 50;
                     mob->perm_int += 50;
                     mob->gold += number_range( 1500, 5000 );
                     strcat( buf, " necromancer" );
                     break;
                  case 4:
                     mob->perm_wis += 100;
                     mob->perm_int += 100;
                     mob->perm_cha += 100;
                     mob->perm_con += 50;
                     mob->perm_str += 50;
                     mob->gold += number_range( 1000, 10000 );
                     strcat( buf, " soul reaver" );
                     break;
               }
               mob->spec_fun = spec_lookup( "spec_cast_death" );
            }
            else
            {
               switch ( number_range( 1, 4 ) )
               {
                  case 1:
                     mob->perm_dex -= 20;
                     mob->perm_str -= 50;
                     mob->perm_con -= 50;
                     mob->gold += number_range( 1, 5 );
                     strcat( buf, " healer" );
                     break;
                  case 2:
                     mob->perm_wis += 20;
                     mob->perm_int += 20;
                     mob->gold += number_range( 10, 100 );
                     strcat( buf, " cleric" );
                     break;
                  case 3:
                     mob->perm_wis += 50;
                     mob->perm_int += 50;
                     mob->gold += number_range( 100, 1000 );
                     strcat( buf, " priest" );
                     break;
                  case 4:
                     mob->perm_wis += 100;
                     mob->perm_int += 100;
                     mob->perm_cha += 100;
                     mob->perm_con += 50;
                     mob->gold += number_range( 1000, 5000 );
                     strcat( buf, " bishop" );
                     break;
               }
               mob->spec_fun = spec_lookup( "spec_cast_healing" );
            }
            switch ( number_range( 1, 11 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 29013 ), 0 );  /* robe */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29036 ), 0 );  /* mace */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 28955 ), 0 );  /* staff */
                  break;
               case 4:
                  if( IS_SET( mob->xflags, 1 << PART_FEET ) )
                     obj = create_object( get_obj_index( 28931 ), 0 );  /* sandals */
                  else if( IS_SET( mob->xflags, 1 << PART_HOOVES ) )
                     obj = create_object( get_obj_index( 29085 ), 0 );  /* horseshoes */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 29051 ), 0 );  /* hammer */
                  break;
               case 6:
                  obj = create_object( get_obj_index( 29065 ), 0 );  /* talisman */
                  break;
               case 7:
                  obj = create_object( get_obj_index( 28930 ), 0 );  /* hood */
                  break;
               case 8:
                  obj = create_object( get_obj_index( 29002 ), 0 );  /* ring */
                  break;
               case 9:
                  obj = create_object( get_obj_index( 28916 ), 0 );  /* sash */
                  break;
               case 10:
                  obj = create_object( get_obj_index( 29053 ), 0 );  /* cap */
                  break;
               case 11:
                  obj = create_object( get_obj_index( 28928 ), 0 );  /* book */
                  break;
            }
            break;
         case 5:
            switch ( number_range( 1, 5 ) )
            {
               case 1:
                  mob->perm_dex -= 50;
                  mob->perm_str -= 50;
                  mob->perm_cha -= 50;
                  mob->perm_con -= 50;
                  mob->gold += number_range( 1, 5 );
                  strcat( buf, " beggar" );
                  break;
               case 2:
                  mob->perm_dex += 20;
                  mob->perm_str += 20;
                  mob->gold += number_range( 1000, 3000 );
                  strcat( buf, " thief" );
                  break;
               case 3:
                  mob->perm_dex += 50;
                  mob->perm_str += 50;
                  mob->gold += number_range( 1500, 10000 );
                  strcat( buf, " rogue" );
                  break;
               case 4:
                  mob->perm_dex += 100;
                  mob->perm_str += 100;
                  mob->perm_cha += 100;
                  mob->perm_con += 50;
                  mob->gold += number_range( 10000, 30000 );
                  strcat( buf, " assassin" );
                  break;
               case 5:
                  mob->perm_dex += 200;
                  mob->perm_str += 100;
                  mob->perm_cha += 200;
                  mob->perm_con += 100;
                  mob->perm_int += 50;
                  mob->gold += number_range( 20000, 50000 );
                  strcat( buf, " ninja" );
                  break;
            }
            mob->spec_fun = spec_lookup( "spec_thief" );
            switch ( number_range( 1, 12 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 29030 ), 0 );  /* dagger */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29014 ), 0 );  /* cloak */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 29031 ), 0 );  /* dirk */
                  break;
               case 4:
                  obj = create_object( get_obj_index( 29029 ), 0 );  /* knife */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 29011 ), 0 );  /* gloves */
                  break;
               case 6:
                  obj = create_object( get_obj_index( 29109 ), 0 );  /* tunic */
                  break;
               case 7:
                  obj = create_object( get_obj_index( 28930 ), 0 );  /* hood */
                  break;
               case 8:
                  obj = create_object( get_obj_index( 29002 ), 0 );  /* ring */
                  break;
               case 9:
                  obj = create_object( get_obj_index( 29061 ), 0 );  /* necklace */
                  break;
               case 10:
                  obj = create_object( get_obj_index( 29054 ), 0 );  /* bracelet */
                  break;
               case 11:
                  obj = create_object( get_obj_index( 29043 ), 0 );  /* whip */
                  break;
               case 12:
                  obj = create_object( get_obj_index( 29032 ), 0 );  /* short sword */
                  break;
            }
            break;
         case 6:
            mob->perm_str -= 50;
            mob->perm_int -= 50;
            mob->perm_wis -= 50;
            mob->perm_con -= 50;
            mob->perm_cha += 50;
            mob->perm_dex += 50;
            strcat( buf, " child" );
            switch ( number_range( 1, 6 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 28929 ), 0 );  /* diaper */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29016 ), 0 );  /* pants */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 28911 ), 0 );  /* shirt */
                  break;
               case 4:
                  obj = create_object( get_obj_index( 29053 ), 0 );  /* cap */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 29109 ), 0 );  /* tunic */
                  break;
               case 6:
                  obj = create_object( get_obj_index( 29017 ), 0 );  /* socks */
                  break;
            }
            break;
      }
   }
   else
   {
      switch ( number_range( 1, 10 ) )
      {
         case 1:
            sprintf( buf2, "baby %s", buf );
            strcpy( buf, buf2 );
            if( number_range( 1, 10 ) == 1 )
               obj = create_object( get_obj_index( 29019 ), 0 );  /* collar */
            mob->perm_str -= 60;
            mob->perm_int -= 50;
            mob->perm_wis -= 50;
            mob->perm_con -= 70;
            mob->perm_dex -= 50;
            break;
         case 2:
            sprintf( buf2, "young %s", buf );
            strcpy( buf, buf2 );
            mob->perm_str -= 50;
            mob->perm_int -= 40;
            mob->perm_wis -= 50;
            mob->perm_con -= 50;
            mob->perm_dex -= 30;
            mob->weight /= 3;
            break;
         case 3:
            sprintf( buf2, "small %s", buf );
            strcpy( buf, buf2 );
            mob->perm_str -= 40;
            mob->perm_con -= 40;
            mob->perm_dex -= 20;
            mob->weight /= 2;
            break;
         case 8:
            sprintf( buf2, "large %s", buf );
            strcpy( buf, buf2 );
            mob->perm_str += 30;
            mob->perm_con += 30;
            mob->weight *= 1.3;
            break;
         case 9:
            sprintf( buf2, "huge %s", buf );
            strcpy( buf, buf2 );
            mob->perm_str += 50;
            mob->perm_con += 50;
            mob->weight *= 2;
            break;
         case 10:
            sprintf( buf2, "giant %s", buf );
            strcpy( buf, buf2 );
            mob->perm_str += 100;
            mob->perm_con += 100;
            mob->perm_dex += 50;
            mob->weight *= 10;
            break;
      }
      if( mob->race == 35 )
         mob->gold += 50000;
      if( IS_SET( mob->immune, RIS_FIRE ) )
         mob->spec_fun = spec_lookup( "spec_cast_fire" );
      else if( IS_SET( mob->immune, RIS_COLD ) )
         mob->spec_fun = spec_lookup( "spec_cast_frost" );
      else if( IS_SET( mob->immune, RIS_POISON ) )
         mob->spec_fun = spec_lookup( "spec_cast_poison" );
      else if( IS_SET( mob->immune, RIS_DRAIN ) )
         mob->spec_fun = spec_lookup( "spec_cast_drain" );
      else if( IS_SET( mob->immune, RIS_PSIONIC ) )
         mob->spec_fun = spec_lookup( "spec_cast_mind" );
      else if( IS_SET( mob->immune, RIS_ELECTRICITY ) )
         mob->spec_fun = spec_lookup( "spec_cast_lightning" );
   }

   STRFREE( mob->short_descr );
   mob->short_descr = STRALLOC( buf );
   STRFREE( mob->name );
   mob->name = STRALLOC( buf );

   mob->max_hit = UMAX( 100, mob->perm_con * nation->hit );
   mob->max_mana = UMAX( 100, mob->perm_int * nation->mana );
   mob->max_move = mob->perm_lck * 50;
   mob->hit = mob->max_hit;
   mob->mana = mob->max_mana;
   mob->move = mob->max_move;

   /*
    * If the mob has an object, determine its material 
    */
   if( obj )
   {
      mat = first_material;
      while( mat )
      {
         if( ( xIS_SET( obj->extra_flags, ITEM_METAL )
               && xIS_SET( mat->extra_flags, ITEM_METAL ) )
             || ( xIS_SET( obj->extra_flags, ITEM_ORGANIC ) && xIS_SET( mat->extra_flags, ITEM_ORGANIC ) ) )
         {

            if( xIS_SET( obj->extra_flags, ITEM_FLAMMABLE ) && !xIS_SET( mat->extra_flags, ITEM_FLAMMABLE ) )
            {
               mat = mat->next;
               continue;
            }
            if( number_percent(  ) < mat->rarity )
               break;
         }
         mat = mat->next;
      }
      /*
       * If we didn't find a good material, forget it 
       */
      if( !mat )
      {
         extract_obj( obj );
      }
      else
      {
         obj->material = mat;
         strcpy( buf, species->adj );
         strcat( buf, " " );
         strcat( buf, mat->name );
         sprintf( buf2, obj->short_descr, buf );
         one_argument( buf2, buf );
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( buf2 );

         strcpy( buf, mat->name );
         sprintf( buf2, obj->description, buf );
         STRFREE( obj->description );
         obj->description = STRALLOC( capitalize( buf2 ) );

         strcpy( buf, obj->name );
         strcat( buf, " " );
         strcat( buf, mat->name );
         strcat( buf, " " );
         strcat( buf, species->adj );
         STRFREE( obj->name );
         obj->name = STRALLOC( buf );

         if( obj->item_type == ITEM_WEAPON )
            obj->value[2] = mob->perm_str;

         xSET_BITS( obj->extra_flags, mat->extra_flags );

         obj->cost += number_fuzzy( mat->cost );
         obj->size = mob->height;
         obj->weight = UMAX( obj->weight + ( mob->height - 66 ) / 10, 1 );

         for( paf = mat->first_affect; paf; paf = paf->next )
         {
            AFFECT_DATA *naf;

            CREATE( naf, AFFECT_DATA, 1 );

            naf->type = paf->type;
            naf->duration = paf->duration;
            naf->location = paf->location;
            naf->modifier = paf->modifier;
            if( paf->location != APPLY_RESISTANT && xIS_SET( mob->act, ACT_NAMED ) )
               naf->modifier += number_range( 1, 20 );
            naf->bitvector = paf->bitvector;
            top_affect++;

            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
         }

         if( number_range( 1, 100 ) * 1000 < get_exp_worth( mob ) && obj->material->magic < number_range( 1, 200 ) )
            make_randart( mob, obj );
         /*
          * Random texts for books 
          */
         if( obj->item_type == ITEM_BOOK )
         {
            i = 0;
            for( book = first_book; book; book = book->next )
            {
               i++;
            }
            i = number_range( 1, i );
            n = 0;
            for( book = first_book; book; book = book->next )
            {
               n++;
               if( n == i )
               {
                  ed = SetOExtra( obj, "title" );
                  ed->description = STRALLOC( book->title );
                  ed = SetOExtra( obj, "text" );
                  ed->description = STRALLOC( book->text );
                  break;
               }
            }
         }
         xSET_BIT( obj->extra_flags, ITEM_NO_RESET );
         obj_to_char( obj, mob );
         if( xIS_EMPTY( obj->parts ) )
            mob->main_hand = obj;
         else
            equip_char( mob, obj );
      }
   }

   return mob;
}

void mob_to_area( CHAR_DATA * mob, AREA_DATA * tarea )
{
   int vnum;
   int i;
   ROOM_INDEX_DATA *pRoomIndex;

   /*
    * Try 100 times to get a valid room 
    */
   for( i = 0; i < 100; i++ )
   {
      vnum = number_range( tarea->low_r_vnum, tarea->hi_r_vnum );
      if( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
         continue;
      if( IS_SET( pRoomIndex->room_flags, ROOM_NO_MOB ) )
         continue;
      if( IS_SET( pRoomIndex->runes, RUNE_DISTRACT ) )
         continue;
      if( IS_SET( pRoomIndex->room_flags, ROOM_NOFLOOR ) && !IS_AFFECTED( mob, AFF_FLYING ) )
         continue;
      if( !IS_AFFECTED( mob, AFF_AQUA_BREATH ) && !IS_AFFECTED( mob, AFF_AQUATIC ) && pRoomIndex->curr_water > 20 )
         continue;
      if( IS_AFFECTED( mob, AFF_AQUATIC ) && !IS_AFFECTED( mob, AFF_AQUA_BREATH ) && pRoomIndex->curr_water < 20 )
         continue;
      char_to_room( mob, pRoomIndex );
      act( AT_ACTION, "You notice $n here.", mob, NULL, NULL, TO_ROOM );
      return;
   }
   bug( "Could not find room for %s in %s, extracting...", mob->name, tarea->name );
   extract_char( mob, TRUE );
}

/*
 * Make a fire.
 */
void make_fire( ROOM_INDEX_DATA * in_room, sh_int timer )
{
   OBJ_DATA *fire;

   fire = create_object( get_obj_index( OBJ_VNUM_FIRE ), 0 );
   fire->timer = number_fuzzy( timer );
   obj_to_room( fire, in_room );
   return;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap( int v0, int v1, int v2, int v3 )
{
   OBJ_DATA *trap;

   trap = create_object( get_obj_index( OBJ_VNUM_TRAP ), 0 );
   trap->timer = 0;
   trap->value[0] = v0;
   trap->value[1] = v1;
   trap->value[2] = v2;
   trap->value[3] = v3;
   return trap;
}

/* Load a piece of ore and return it -- Scion */
OBJ_DATA *make_ore( int number )
{
   MATERIAL_DATA *material;
   AREA_DATA *area;
   OBJ_DATA *obj;
   AFFECT_DATA *paf;
   AFFECT_DATA *oaf;
   char buf[MAX_STRING_LENGTH];
   int hi_vnum = 2;

   if( !stockobj )
      for( area = first_area; area; area = area->next )
      {
         if( !str_cmp( area->filename, "stockobj.are" ) )
         {
            stockobj = area;
            hi_vnum = area->hi_o_vnum;
            break;
         }
         else
            hi_vnum = 2;
      }
   else
   {
      area = stockobj;
      hi_vnum = area->hi_o_vnum;
   }

   if( hi_vnum == 2 )
      return NULL;

   material = material_lookup( number );
   if( !material )
      return NULL;

   obj = create_object( get_obj_index( hi_vnum ), 0 );

   if( !obj )
      return NULL;

   strcpy( buf, material->name );
   strcat( buf, " _material_" );
   obj->name = STRALLOC( buf );
   obj->short_descr = STRALLOC( material->short_descr );
   obj->description = STRALLOC( material->description );
   obj->weight = number_fuzzy( material->weight );
   obj->cost = number_fuzzy( material->cost );
   obj->extra_flags = material->extra_flags;
   obj->value[0] = material->number;
   obj->timer = 10;
   obj->material = material;

   xSET_BIT( obj->extra_flags, ITEM_GROUNDROT );

   for( paf = material->first_affect; paf; paf = paf->next )
   {
      CREATE( oaf, AFFECT_DATA, 1 );
      oaf->type = paf->type;
      oaf->duration = paf->duration;
      oaf->location = paf->location;
      oaf->modifier = paf->modifier;
      xCLEAR_BITS( oaf->bitvector );
      oaf->next = NULL;
      LINK( oaf, obj->first_affect, obj->last_affect, next, prev );
      ++top_affect;
   }

   return obj;
}

/* Shopkeepers can use this to stock their shops
 * Creates a random item appropriate to the shop.
 * Added by Keolah, September 17, 2002
 */
void do_mpmakeitem( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   int i, n, vnum, hi_vnum, low_vnum;
   SHOP_DATA *shop;
   AREA_DATA *area;
   MATERIAL_DATA *mat;
   AFFECT_DATA *paf;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   EXTRA_DESCR_DATA *ed;
   BOOK_DATA *book;

   shop = ch->in_room->pShop;
   if( !shop )
      return;

   if( number_range( 1, 100 ) > shop->profit_buy )
      return;

   hi_vnum = 0;
   if( !stockobj )
   {
      for( area = first_area; area; area = area->next )
      {
         if( !str_cmp( area->filename, "stockobj.are" ) )
         {
            stockobj = area;
            hi_vnum = area->hi_o_vnum;
            low_vnum = area->low_o_vnum;
            break;
         }
      }
   }
   else
   {
      area = stockobj;
      hi_vnum = area->hi_o_vnum;
      low_vnum = area->low_o_vnum;
   }
   if( hi_vnum == 0 )
      return;

   /*
    * Decrease this number if it produces too much lag 
    */
   for( i = 0; i < 100; i++ )
   {
      vnum = number_range( low_vnum, hi_vnum );
      if( get_obj_index( vnum ) )
      {
         obj = create_object( get_obj_index( vnum ), 0 );
         if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         {
            extract_obj( obj );
            obj = NULL;
            continue;
         }
         if( shop->type != -1 && obj->item_type != shop->type )
         {
            extract_obj( obj );
            obj = NULL;
            continue;
         }
         if( shop->flag != -1 && !IS_OBJ_STAT( obj, shop->flag ) )
         {
            extract_obj( obj );
            obj = NULL;
            continue;
         }
         break;
      }
   }

   if( obj )
   {
      mat = first_material;
      /*
       * Find a material to suit the item 
       */
      while( mat )
      {
         if( ( xIS_SET( obj->extra_flags, ITEM_METAL )
               && xIS_SET( mat->extra_flags, ITEM_METAL ) )
             || ( xIS_SET( obj->extra_flags, ITEM_ORGANIC ) && xIS_SET( mat->extra_flags, ITEM_ORGANIC ) ) )
         {

            if( xIS_SET( obj->extra_flags, ITEM_FLAMMABLE ) && !xIS_SET( mat->extra_flags, ITEM_FLAMMABLE ) )
            {
               mat = mat->next;
               continue;
            }

            if( number_percent(  ) < mat->rarity )
               break;
         }
         mat = mat->next;
      }
      /*
       * If we didn't find a good material, forget it 
       */
      if( !mat )
      {
         extract_obj( obj );
      }
      else
      {
         obj->material = mat;
         strcpy( buf, mat->name );
         sprintf( buf2, obj->short_descr, buf );
         one_argument( buf2, buf );
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( buf2 );

         strcpy( buf, mat->name );
         sprintf( buf2, obj->description, buf );
         STRFREE( obj->description );
         obj->description = STRALLOC( capitalize( buf2 ) );

         strcpy( buf, obj->name );
         strcat( buf, " " );
         strcat( buf, mat->name );
         STRFREE( obj->name );
         obj->name = STRALLOC( buf );

         if( obj->item_type == ITEM_WEAPON )
            obj->value[2] = number_range( 30, 300 );

         xSET_BITS( obj->extra_flags, mat->extra_flags );

         obj->cost += number_fuzzy( mat->cost );
         obj->size = number_range( 30, 80 );

         for( paf = mat->first_affect; paf; paf = paf->next )
         {
            AFFECT_DATA *naf;

            CREATE( naf, AFFECT_DATA, 1 );

            naf->type = paf->type;
            naf->duration = paf->duration;
            naf->location = paf->location;
            naf->modifier = paf->modifier;
            naf->bitvector = paf->bitvector;
            top_affect++;

            LINK( naf, obj->first_affect, obj->last_affect, next, prev );
         }
         /*
          * Random texts for books 
          */
         if( obj->item_type == ITEM_BOOK )
         {
            i = 0;
            for( book = first_book; book; book = book->next )
            {
               i++;
            }
            i = number_range( 1, i );
            n = 0;
            for( book = first_book; book; book = book->next )
            {
               n++;
               if( n == i )
               {
                  ed = SetOExtra( obj, "title" );
                  ed->description = STRALLOC( book->title );
                  ed = SetOExtra( obj, "text" );
                  ed->description = STRALLOC( book->text );
                  break;
               }
            }
         }
         if( obj->material->magic < number_range( 1, 200 ) )
            make_randart( ch, obj );
         xSET_BIT( obj->extra_flags, ITEM_NO_RESET );
         obj_to_room( obj, ch->in_room );
         act( AT_ACTION, "$n fashions $p and puts it out for sale.", ch, obj, NULL, TO_ROOM );
      }
   }
}

/* Load a raw material into the game -- Scion */
void do_makeore( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   MATERIAL_DATA *material;
   AREA_DATA *area;
   AFFECT_DATA *paf;
   AFFECT_DATA *oaf;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   int hi_vnum = 2;
   int i = -1;

   if( !is_number( argument ) )
   {
      send_to_char( "That is not a number.\r\n", ch );
      return;
   }

   i = atoi( argument );

   material = material_lookup( i );

   if( !material )
   {
      send_to_char( "There is no such material.\r\n", ch );
      return;
   }

   if( !stockobj )
      for( area = first_area; area; area = area->next )
      {
         if( !str_cmp( area->filename, "stockobj.are" ) )
         {
            stockobj = area;
            hi_vnum = area->hi_o_vnum;
            break;
         }
         else
         {
            hi_vnum = 2;
         }
      }
   else
   {
      area = stockobj;
      hi_vnum = area->hi_o_vnum;
   }
   if( hi_vnum == 2 )
   {
      send_to_char( "There does not seem to be a raw material object in the game.\r\n", ch );
      return;
   }

   obj = create_object( get_obj_index( hi_vnum ), 0 );

   sprintf( buf, "%s _material_", material->name );
   obj->name = STRALLOC( buf );
   obj->short_descr = STRALLOC( material->short_descr );
   obj->description = STRALLOC( material->description );
   obj->weight = number_fuzzy( material->weight );
   obj->cost = number_fuzzy( material->cost );
   obj->extra_flags = material->extra_flags;
   obj->value[0] = material->number;
   obj->material = material;

   for( paf = material->first_affect; paf; paf = paf->next )
   {
      CREATE( oaf, AFFECT_DATA, 1 );
      oaf->type = paf->type;
      oaf->duration = paf->duration;
      oaf->location = paf->location;
      oaf->modifier = paf->modifier;
      xCLEAR_BITS( oaf->bitvector );
      oaf->next = NULL;
      LINK( oaf, obj->first_affect, obj->last_affect, next, prev );
      ++top_affect;
   }

   xSET_BIT( obj->extra_flags, ITEM_PLRBLD );
   xSET_BIT( obj->extra_flags, ITEM_NO_RESET );
   obj_to_char( obj, ch );
   sprintf( buf, "You create $p!" );
   sprintf( buf2, "$n creates $p!" );
   act( AT_IMMORT, buf, ch, obj, NULL, TO_CHAR );
   act( AT_IMMORT, buf2, ch, obj, NULL, TO_ROOM );
   return;
}

/* Turn a raw material into a piece of equipment -- Scion */
void do_fashion( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *area;
   AFFECT_DATA *paf;
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   OBJ_DATA *ore;
   OBJ_DATA *fire;
   MATERIAL_DATA *material;
   int lo_vnum = 2;
   int hi_vnum = 2;
   int hash;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   bool found;
   extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
   extern sh_int gsn_fashion;
   SPECIES_DATA *species;

   if( !can_use_bodypart( ch, BP_RHAND ) || !can_use_bodypart( ch, BP_LHAND ) )
   {
      send_to_char( "That would be hard without functional hands.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( !stockobj )
      for( area = first_area; area; area = area->next )
      {
         if( !str_cmp( area->filename, "stockobj.are" ) )
         {
            stockobj = area;
            lo_vnum = area->low_o_vnum;
            hi_vnum = area->hi_o_vnum;
            break;
         }
         else
         {
            lo_vnum = 2;
            hi_vnum = 2;
         }
      }
   else
   {
      area = stockobj;
      lo_vnum = area->low_o_vnum;
      hi_vnum = area->hi_o_vnum;
   }
   if( lo_vnum == hi_vnum )
   {
      send_to_char( "You cannot figure out how to make anything right now.\r\n", ch );
      return;
   }

   if( !first_material )
   {
      bug( "No materials exist, cannot use do_fashion.", 0 );
      send_to_char( "You do not know how to make anything.\r\n", ch );
      return;
   }

   ore = get_obj_carry( ch, argument );
   if( !ore )
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }

   if( ore->pIndexData->vnum != hi_vnum )
   {
      send_to_char( "That isn't a raw material.\n\r", ch );
      return;
   }

   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
   {
      for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
      {
         if( nifty_is_name( arg, pObjIndex->name ) && pObjIndex->vnum >= lo_vnum && pObjIndex->vnum <= hi_vnum )
         {
            obj = create_object( pObjIndex, 0 );
            if( !obj )
            {
               send_to_char( "You cannot recall exactly how to make one of those.\r\n", ch );
               return;
            }

            obj->obj_by = STRALLOC( ch->name );

            material = ore->material;

            if( obj->pIndexData->vnum == ore->pIndexData->vnum )
            {
               send_to_char( "You cannot fashion a raw material from a raw material.\r\n", ch );
               extract_obj( obj );
               return;
            }
            if( !material )
            {
               send_to_char( "You cannot seem to identify the type of material you have.\r\n", ch );
               extract_obj( obj );
               return;
            }
            if( !ore )
            {
               send_to_char( "You do not seem to have the proper materials.\r\n", ch );
               extract_obj( obj );
               return;
            }
            if( IS_OBJ_STAT( obj, ITEM_GEM ) )
            {
               send_to_char( "Use AFFIX to affix gems to items.\n\r", ch );
               extract_obj( obj );
               return;
            }
            if( obj->pIndexData->tech > ch->pcdata->noncombat[SK_SMITH] )
            {
               send_to_char( "You are not skilled enough to make that.\n\r", ch );
               return;
            }
            if( obj->weight > ore->weight + 2 )
            {
               send_to_char( "You need more ore to make that.\r\n", ch );
               send_to_char( "Use COMBINE to add one piece of ore to another, for a bigger piece.\r\n", ch );
               extract_obj( obj );
               return;
            }
            if( IS_OBJ_STAT( obj, ITEM_METAL ) )
            {
               if( !IS_OBJ_STAT( ore, ITEM_METAL ) )
               {
                  send_to_char( "You need a metal ore to make that.\r\n", ch );
                  extract_obj( obj );
                  return;
               }
               found = FALSE;
               for( fire = ch->in_room->first_content; fire; fire = fire->next_content )
               {
                  if( fire->item_type == ITEM_FIRE )
                  {
                     found = TRUE;
                     break;
                  }
               }
               if( IS_OBJ_STAT( ore, ITEM_FLAMMABLE ) )
                  found = TRUE;
               else if( IS_OBJ_STAT( obj, ITEM_FLAMMABLE ) )
               {
                  send_to_char( "You need some wood to make that.\r\n", ch );
                  extract_obj( obj );
                  return;
               }
               if( !found )
               {
                  send_to_char( "There must be a fire in the room to forge metals.\n\r", ch );
                  extract_obj( obj );
                  return;
               }
            }
            else if( ( IS_OBJ_STAT( obj, ITEM_ORGANIC ) ) && ( !IS_OBJ_STAT( ore, ITEM_ORGANIC ) ) )
            {
               send_to_char( "You need cloth or leather to make that.\r\n", ch );
               extract_obj( obj );
               return;
            }

            if( ( IS_OBJ_STAT( obj, ITEM_MAGIC ) ) && ( !IS_OBJ_STAT( ore, ITEM_MAGIC ) ) )
            {
               send_to_char( "You need a magical raw material to build that object.\r\n", ch );
               extract_obj( obj );
               return;
            }

            separate_obj( ore );
            obj_from_char( ore );

            obj->material = material;


            species = find_species( ch->nation->species );
            strcpy( buf, species->adj );
            strcat( buf, " " );
            strcat( buf, material->name );
            sprintf( arg, obj->short_descr, buf );
            one_argument( arg, buf );
            STRFREE( obj->short_descr );
            obj->short_descr = STRALLOC( arg );

            strcpy( buf, material->name );
            sprintf( arg, obj->description, buf );
            STRFREE( obj->description );
            obj->description = STRALLOC( capitalize( arg ) );

            strcpy( buf, obj->name );
            strcat( buf, " " );
            strcat( buf, material->name );
            strcat( buf, " " );
            strcat( buf, species->adj );
            STRFREE( obj->name );
            obj->name = STRALLOC( buf );

            if( obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON )
               obj->value[2] = ch->pcdata->weapon[obj->value[5]] + ch->pcdata->noncombat[SK_SMITH];

            xSET_BITS( obj->extra_flags, ore->extra_flags );
            xSET_BIT( obj->extra_flags, ITEM_PLRBLD );
            xREMOVE_BIT( obj->extra_flags, ITEM_GROUNDROT );
            xSET_BIT( obj->extra_flags, ITEM_NO_RESET );

            obj->cost += number_fuzzy( ore->cost );
            obj->size = ch->height;

            obj->weight = UMAX( obj->weight + ( ch->height - 66 ) / 10, 1 );
            STRFREE( ch->last_taken );
            sprintf( buf, "fashioning the %s", myobj( obj ) );
            ch->last_taken = STRALLOC( buf );
            if( IS_OBJ_STAT( obj, ITEM_METAL ) )
            {
               learn_noncombat( ch, SK_SMITH );
               WAIT_STATE( ch, ( 100 - ch->pcdata->noncombat[SK_SMITH] ) / 5 );
            }
            else
            {
               learn_noncombat( ch, SK_TAILOR );
               WAIT_STATE( ch, ( 100 - ch->pcdata->noncombat[SK_TAILOR] ) / 5 );
            }

            WAIT_STATE( ch, obj->pIndexData->tech );

            /*
             * Roll some dice... 
             */
            if( number_percent(  ) > LEARNED( ch, gsn_fashion ) )
            {
               send_to_char( "You can't quite seem to get it right, and ruin your fashioned item.\r\n", ch );
               learn_from_failure( ch, gsn_fashion );
               if( ore->weight > obj->weight )
               {
                  ore->weight -= UMAX( 1, obj->weight );
                  obj_to_char( ore, ch );
               }
               else
               {
                  extract_obj( ore );
               }
               extract_obj( obj );
               return;
            }

            for( paf = ore->first_affect; paf; paf = paf->next )
            {
               AFFECT_DATA *naf;

               CREATE( naf, AFFECT_DATA, 1 );

               naf->type = paf->type;
               naf->duration = paf->duration;
               naf->location = paf->location;
               naf->modifier = paf->modifier;
               naf->bitvector = paf->bitvector;
               top_affect++;

               LINK( naf, obj->first_affect, obj->last_affect, next, prev );
            }

            if( ore->weight > obj->weight )
            {
               ore->weight -= UMAX( 1, obj->weight );
               obj_to_char( ore, ch );
            }
            else
            {
               extract_obj( ore );
            }

            obj_to_char( obj, ch );
            sprintf( buf, "You fashion $p from %s.", material->name );
            sprintf( buf2, "$n fashions $p from %s.", material->name );
            act( AT_SKILL, buf, ch, obj, NULL, TO_CHAR );
            act( AT_SKILL, buf2, ch, obj, NULL, TO_ROOM );
            learn_from_success( ch, gsn_fashion );
            return;
         }
      }
   }
   send_to_char( "You cannot figure out how to make that.\n\r", ch );
   return;

}

/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps( OBJ_DATA * obj )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *scraps, *tmpobj;
   CHAR_DATA *ch = NULL;

   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) || IS_OBJ_STAT( obj, ITEM_DURABLE ) )
      return;

   separate_obj( obj );
   scraps = create_object( get_obj_index( OBJ_VNUM_SCRAPS ), 0 );
   scraps->timer = number_range( 5, 15 );

   /*
    * don't make scraps of scraps of scraps of ... 
    */
   if( obj->pIndexData->vnum == OBJ_VNUM_SCRAPS )
   {
      STRFREE( scraps->short_descr );
      scraps->short_descr = STRALLOC( "some debris" );
      STRFREE( scraps->description );
      scraps->description = STRALLOC( "Bits of debris lie on the ground here." );
   }
   else
   {
      sprintf( buf, scraps->short_descr, obj->short_descr );
      STRFREE( scraps->short_descr );
      scraps->short_descr = STRALLOC( buf );
      sprintf( buf, scraps->description, obj->short_descr );
      STRFREE( scraps->description );
      scraps->description = STRALLOC( buf );
   }

   if( obj->carried_by )
   {

      act( AT_OBJECT, "$p falls to the ground in scraps!", obj->carried_by, obj, NULL, TO_CHAR );
      if( obj == get_eq_char( obj->carried_by, WEAR_HAND )
          && ( tmpobj = get_eq_char( obj->carried_by, WEAR_HAND2 ) ) != NULL )
         tmpobj->wear_loc = WEAR_HAND;

      obj_to_room( scraps, obj->carried_by->in_room );
   }
   else if( obj->in_room )
   {
      if( ( ch = obj->in_room->first_person ) != NULL )
      {
         act( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_ROOM );
         act( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_CHAR );
      }
      obj_to_room( scraps, obj->in_room );
   }
   if( ( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_KEYRING
         || obj->item_type == ITEM_QUIVER || obj->item_type == ITEM_CORPSE_PC ) && obj->first_content )
   {
      if( ch && ch->in_room )
      {
         act( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_ROOM );
         act( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_CHAR );
      }
      if( obj->carried_by )
         empty_obj( obj, NULL, obj->carried_by->in_room );
      else if( obj->in_room )
         empty_obj( obj, NULL, obj->in_room );
      else if( obj->in_obj )
         empty_obj( obj, obj->in_obj, NULL );
   }
   extract_obj( obj );
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *corpse;
   OBJ_DATA *cheese;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   MATERIAL_DATA *material;
   PART_DATA *part;
   char *name;

   if( IS_AFFECTED( ch, AFF_NO_CORPSE ) )
   {
      if( ch->gold > 0 )
      {
         if( ch->in_room )
         {
            ch->in_room->area->gold_looted += ch->gold;
         }
         obj_to_room( create_money( ch->gold ), ch->in_room );
         ch->gold = 0;
      }
      for( obj = ch->first_carrying; obj; obj = obj_next )
      {
         obj_next = obj->next_content;
         if( !IS_OBJ_STAT( obj, ITEM_LOYAL ) || IS_NPC( ch ) )
         {
            obj_from_char( obj );
            if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) || IS_OBJ_STAT( obj, ITEM_DEATHROT ) )
            {
               extract_obj( obj );
            }
            else
            {
               obj_to_room( obj, ch->in_room );
            }
         }
      }
      return;
   }

   if( IS_NPC( ch ) )
   {
      name = ch->short_descr;
      corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_NPC ), 0 );
      corpse->timer = 6;
      if( ch->gold > 0 )
      {
         if( ch->in_room )
         {
            ch->in_room->area->gold_looted += ch->gold;
         }
         obj_to_obj( create_money( ch->gold ), corpse );
         ch->gold = 0;
      }

      /*
       * Using corpse cost to cheat, since corpses not sellable 
       */
      corpse->cost = ( -( int )ch->pIndexData->vnum );
   }
   else
   {
      name = ch->name;
      corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );
      corpse->cost = ch->exp;
      corpse->timer = 40;

      if( ch->gold > 0 )
      {
         obj_to_obj( create_money( ch->gold ), corpse );
         ch->gold = 0;
      }
   }
   if( ch->nation )
      name = ch->nation->name;

   corpse->weight = ch->weight;
   corpse->material = NULL;

   corpse->value[0] = ch->perm_str;
   corpse->value[1] = ch->perm_int;
   corpse->value[2] = ch->perm_wis;
   corpse->value[3] = ch->perm_dex;
   corpse->value[4] = ch->perm_con;
   corpse->value[5] = ch->perm_cha;
   corpse->value[6] = ch->perm_lck;

   for( part = ch->first_part; part; part = part->next )
   {
      if( part->flags != PART_SEVERED )
         xSET_BIT( corpse->parts, part->loc );
   }

   /*
    * Added corpse name - make locate easier , other skills 
    */
   sprintf( buf, "corpse %s %s", name, ch->name );
   STRFREE( corpse->name );
   corpse->name = STRALLOC( buf );

   sprintf( buf, corpse->short_descr, name );
   STRFREE( corpse->short_descr );
   corpse->short_descr = STRALLOC( buf );

   sprintf( buf, corpse->description, IS_NPC( ch ) ? aoran( ch->short_descr ) : ch->name );
   STRFREE( corpse->description );
   corpse->description = STRALLOC( buf );

   STRFREE( corpse->obj_by );
   corpse->obj_by = STRALLOC( ch->nation ? ch->nation->name : "" );

   for( obj = ch->first_carrying; obj; obj = obj_next )
   {
      obj_next = obj->next_content;
      if( !IS_OBJ_STAT( obj, ITEM_LOYAL ) || IS_NPC( ch ) )
      {

         obj_from_char( obj );
         if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) || IS_OBJ_STAT( obj, ITEM_DEATHROT ) )
         {
            extract_obj( obj );
         }
         else
         {
            obj_to_obj( obj, corpse );
         }
      }
   }

   if( IS_NPC( ch ) )
   {
      for( material = first_material; material; material = material->next )
      {
         if( ( material->race == ch->race ) && ( number_percent(  ) < material->rarity ) )
         {
            obj = make_ore( material->number );
            if( obj )
               obj_to_obj( obj, corpse );
         }
      }
   }
   else
   {
      cheese = create_object( get_obj_index( OBJ_VNUM_CHAOS_CHEESE ), 0 );
      cheese->timer = 25;
      obj_to_room( cheese, ch->in_room );
   }

   obj_to_room( corpse, ch->in_room );
   return;
}



void make_blood( CHAR_DATA * ch, int amt )
{
   OBJ_DATA *obj;

   if( !ch->in_room )
      return;

   /*
    * lets not create tons of pools of blood -keo 
    */
   for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
   {
      if( obj->item_type == ITEM_BLOOD )
         break;
   }
   if( obj )
   {
      obj->timer += number_range( 1, 2 );
      obj->value[0] += number_fuzzy( amt );
      obj->value[1] = obj->value[0];
      return;
   }

   obj = create_object( get_obj_index( OBJ_VNUM_BLOOD ), 0 );
   obj->timer = number_range( 2, 4 );
   obj->value[0] = number_fuzzy( amt );
   obj->value[1] = obj->value[0];
   obj_to_room( obj, ch->in_room );
}


void make_bloodstain( CHAR_DATA * ch )
{
   OBJ_DATA *obj;

   obj = create_object( get_obj_index( OBJ_VNUM_BLOODSTAIN ), 0 );
   obj->timer = number_range( 1, 2 );
   obj_to_room( obj, ch->in_room );
}


/*
 * make some coinage
 */
OBJ_DATA *create_money( int amount )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;

   if( amount <= 0 )
   {
      bug( "Create_money: zero or negative money %d.", amount );
      amount = 1;
   }

   if( amount == 1 )
   {
      obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
   }
   else
   {
      obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
      sprintf( buf, obj->short_descr, amount );
      STRFREE( obj->short_descr );
      obj->short_descr = STRALLOC( buf );
      obj->value[0] = amount;
   }

   return obj;
}
