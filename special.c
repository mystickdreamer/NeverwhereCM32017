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
 *			   "Special procedure" module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
   
/* from makeobjs.c */ 
extern OBJ_DATA *make_ore( int number );

/* from elkandu.c */ 
extern void ch_harm( CHAR_DATA * victim, CHAR_DATA * ch, int dam, int type, bool dont_wait );

/*
 * The following special functions are available for mobiles.
 */ 
   DECLARE_SPEC_FUN( spec_cast_fire );
DECLARE_SPEC_FUN( spec_cast_wind );
DECLARE_SPEC_FUN( spec_cast_water );
DECLARE_SPEC_FUN( spec_cast_earth );
DECLARE_SPEC_FUN( spec_cast_drain );
DECLARE_SPEC_FUN( spec_cast_mind );
DECLARE_SPEC_FUN( spec_cast_lightning );
DECLARE_SPEC_FUN( spec_cast_frost );
DECLARE_SPEC_FUN( spec_cast_poison );
DECLARE_SPEC_FUN( spec_cast_light );
DECLARE_SPEC_FUN( spec_cast_healing );
DECLARE_SPEC_FUN( spec_cast_all );
DECLARE_SPEC_FUN( spec_fido );
DECLARE_SPEC_FUN( spec_guard );
DECLARE_SPEC_FUN( spec_janitor );
DECLARE_SPEC_FUN( spec_poison );
DECLARE_SPEC_FUN( spec_thief );
DECLARE_SPEC_FUN( spec_catalyst );
DECLARE_SPEC_FUN( spec_miner );
DECLARE_SPEC_FUN( spec_farmer );

/*
 * Given a name, return the appropriate spec fun.
 */ 
   SPEC_FUN * spec_lookup( const char *name ) 
{
   if( !str_cmp( name, "spec_cast_healing" ) )
      return spec_cast_healing;
   if( !str_cmp( name, "spec_cast_fire" ) )
      return spec_cast_fire;
   if( !str_cmp( name, "spec_cast_wind" ) )
      return spec_cast_wind;
   if( !str_cmp( name, "spec_cast_frost" ) )
      return spec_cast_frost;
   if( !str_cmp( name, "spec_cast_lightning" ) )
      return spec_cast_lightning;
   if( !str_cmp( name, "spec_cast_water" ) )
      return spec_cast_water;
   if( !str_cmp( name, "spec_cast_earth" ) )
      return spec_cast_earth;
   if( !str_cmp( name, "spec_cast_drain" ) )
      return spec_cast_drain;
   if( !str_cmp( name, "spec_cast_mind" ) )
      return spec_cast_mind;
   if( !str_cmp( name, "spec_cast_poison" ) )
      return spec_cast_poison;
   if( !str_cmp( name, "spec_cast_light" ) )
      return spec_cast_light;
   if( !str_cmp( name, "spec_cast_all" ) )
      return spec_cast_all;
   if( !str_cmp( name, "spec_fido" ) )
      return spec_fido;
   if( !str_cmp( name, "spec_guard" ) )
      return spec_guard;
   if( !str_cmp( name, "spec_janitor" ) )
      return spec_janitor;
   if( !str_cmp( name, "spec_poison" ) )
      return spec_poison;
   if( !str_cmp( name, "spec_thief" ) )
      return spec_thief;
   if( !str_cmp( name, "spec_catalyst" ) )
      return spec_catalyst;
   if( !str_cmp( name, "spec_miner" ) )
      return spec_miner;
   if( !str_cmp( name, "spec_farmer" ) )
      return spec_farmer;
   return 0;
}


/*
 * Given a pointer, return the appropriate spec fun text.
 */ 
char *lookup_spec( SPEC_FUN * special ) 
{
   if( special == spec_cast_healing )
      return "spec_cast_healing";
   if( special == spec_cast_fire )
      return "spec_cast_fire";
   if( special == spec_cast_wind )
      return "spec_cast_wind";
   if( special == spec_cast_water )
      return "spec_cast_water";
   if( special == spec_cast_frost )
      return "spec_cast_frost";
   if( special == spec_cast_lightning )
      return "spec_cast_lightning";
   if( special == spec_cast_mind )
      return "spec_cast_mind";
   if( special == spec_cast_poison )
      return "spec_cast_poison";
   if( special == spec_cast_drain )
      return "spec_cast_drain";
   if( special == spec_cast_earth )
      return "spec_cast_earth";
   if( special == spec_cast_light )
      return "spec_cast_light";
   if( special == spec_cast_all )
      return "spec_cast_all";
   if( special == spec_fido )
      return "spec_fido";
   if( special == spec_guard )
      return "spec_guard";
   if( special == spec_janitor )
      return "spec_janitor";
   if( special == spec_poison )
      return "spec_poison";
   if( special == spec_thief )
      return "spec_thief";
   if( special == spec_catalyst )
      return "spec_catalyst";
   if( special == spec_miner )
      return "spec_miner";
   if( special == spec_farmer )
      return "spec_farmer";
   return "";
}

bool spec_cast_healing( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   CHAR_DATA * v_next;
   if( !IS_AWAKE( ch ) || IS_FIGHTING( ch ) )
      return FALSE;
   for( victim = ch->in_room->first_person; victim; victim = v_next )
      
   {
      v_next = victim->next_in_room;
      if( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
         break;
   }
   if( !victim )
      return FALSE;
   switch ( number_range( 0, 7 ) )
   {
      case 0:
         act( AT_MAGIC, "$n utters the word 'sikana'.", ch, NULL, NULL, TO_ROOM );
         spell_smaug( skill_lookup( "security magic" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
      case 1:
         act( AT_MAGIC, "$n utters the word 'nambreh'.", ch, NULL, NULL, TO_ROOM );
         spell_smaug( skill_lookup( "motion magic" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
      case 2:
         act( AT_MAGIC, "$n utters the word 'anomis'.", ch, NULL, NULL, TO_ROOM );
         spell_cure_blindness( skill_lookup( "cure blindness" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
      case 3:
         act( AT_MAGIC, "$n utters the word 'neosporin'.", ch, NULL, NULL, TO_ROOM );
         spell_smaug( skill_lookup( "cure light" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
      case 4:
         act( AT_MAGIC, "$n utters the word 'ahpla'.", ch, NULL, NULL, TO_ROOM );
         spell_cure_poison( skill_lookup( "cure poison" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
      case 5:
         act( AT_MAGIC, "$n utters the word 'gatorade'.", ch, NULL, NULL, TO_ROOM );
         spell_smaug( skill_lookup( "refresh" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
      case 6:
         act( AT_MAGIC, "$n utters the word 'haloek'.", ch, NULL, NULL, TO_ROOM );
         spell_smaug( skill_lookup( "cure serious" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
      case 7:
         act( AT_MAGIC, "$n utters the word 'noics'.", ch, NULL, NULL, TO_ROOM );
         spell_remove_curse( skill_lookup( "remove curse" ), get_curr_wil( ch ), ch, victim );
         return TRUE;
   }
   return FALSE;
}

bool spec_cast_fire( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_FIRE, FALSE );
   return TRUE;
}

bool spec_cast_wind( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_WIND, FALSE );
   return TRUE;
}

bool spec_cast_water( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_WATER, FALSE );
   return TRUE;
}

bool spec_cast_earth( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_EARTH, FALSE );
   return TRUE;
}

bool spec_cast_frost( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_COLD, FALSE );
   return TRUE;
}

bool spec_cast_lightning( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_ELECTRICITY, FALSE );
   return TRUE;
}

bool spec_cast_mind( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_PSIONIC, FALSE );
   return TRUE;
}

bool spec_cast_poison( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_POISON, FALSE );
   return TRUE;
}

bool spec_cast_drain( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_DRAIN, FALSE );
   return TRUE;
}

bool spec_cast_light( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), MAG_LIGHT, FALSE );
   return TRUE;
}

bool spec_cast_all( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) || !ch->in_room || char_died( ch ) )
      return FALSE;
   victim = ch->last_hit;
   if( !victim || victim == ch )
      return FALSE;
   ch_harm( victim, ch, get_curr_int( ch ), number_range( MAG_NONE, MAG_ALL ), FALSE );
   return TRUE;
}

bool spec_fido( CHAR_DATA * ch ) 
{
   OBJ_DATA * corpse;
   OBJ_DATA * c_next;
   OBJ_DATA * obj;
   OBJ_DATA * obj_next;
   if( !IS_AWAKE( ch ) )
      return FALSE;
   for( corpse = ch->in_room->first_content; corpse; corpse = c_next )
      
   {
      c_next = corpse->next_content;
      if( corpse->item_type != ITEM_CORPSE_NPC )
         continue;
      act( AT_ACTION, "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
      for( obj = corpse->first_content; obj; obj = obj_next )
         
      {
         obj_next = obj->next_content;
         obj_from_obj( obj );
         obj_to_room( obj, ch->in_room );
      }
      extract_obj( corpse );
      return TRUE;
   }
   return FALSE;
}

bool spec_guard( CHAR_DATA * ch )
{
   CHAR_DATA * victim;
   CHAR_DATA * v_next;
   CHAR_DATA * ech;
   char *crime;
   if( !IS_AWAKE( ch ) || ch->last_hit )
      return FALSE;
   ech = NULL;
   crime = "";
   for( victim = ch->in_room->first_person; victim; victim = v_next )
      
   {
      v_next = victim->next_in_room;
      if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_KILLER ) )
         
      {
         crime = "KILLER";
         break;
      }
      if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_THIEF ) )
         
      {
         crime = "THIEF";
         break;
      }
   }
   return FALSE;
}

bool spec_janitor( CHAR_DATA * ch ) 
{
   OBJ_DATA * trash;
   OBJ_DATA * trash_next;
   if( !IS_AWAKE( ch ) )
      return FALSE;
   for( trash = ch->in_room->first_content; trash; trash = trash_next )
      
   {
      trash_next = trash->next_content;
      if( IS_OBJ_STAT( trash, ITEM_NO_TAKE ) 
           ||IS_OBJ_STAT( trash, ITEM_BURIED )  ||trash->item_type == ITEM_MONEY  ||trash->item_type == ITEM_CORPSE_NPC )
         continue;
      if( trash->item_type == ITEM_DRINK_CON 
           ||trash->item_type == ITEM_TRASH 
           ||trash->cost < 1000  ||( trash->pIndexData->vnum == OBJ_VNUM_SHOPPING_BAG  &&!trash->first_content ) )
         
      {
         act( AT_ACTION, "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
         obj_from_room( trash );
         extract_obj( trash );
         return TRUE;
      }
   }
   return FALSE;
}

bool spec_poison( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   if( !IS_FIGHTING( ch ) )
      return FALSE;
   if( ( victim = ch->last_hit ) == NULL  ||number_percent(  ) > get_curr_dex( ch ) )
      return FALSE;
   act( AT_HIT, "You bite $N!", ch, NULL, victim, TO_CHAR );
   act( AT_ACTION, "$n bites $N!", ch, NULL, victim, TO_NOTVICT );
   act( AT_POISON, "$n bites you!", ch, NULL, victim, TO_VICT );
   spell_poison( gsn_poison, get_curr_end( ch ), ch, victim );
   return TRUE;
}

bool spec_thief( CHAR_DATA * ch ) 
{
   CHAR_DATA * victim;
   CHAR_DATA * v_next;
   char buf[MAX_STRING_LENGTH];
   char name[MAX_STRING_LENGTH];
   if( ch->position != POS_STANDING )
      return FALSE;
   for( victim = ch->in_room->first_person; victim; victim = v_next )
      
   {
      v_next = victim->next_in_room;
      if( IS_NPC( victim )  ||number_bits( 2 ) != 0  ||!can_see( ch, victim ) ) /* Thx Glop */
         continue;
      if( number_percent(  ) < 80 || victim->last_carrying == NULL )
      {
         sprintf( buf, "coins %s", victim->name );
         do_steal( ch, buf );
         return TRUE;
      }
      else
      {
         one_argument( victim->last_carrying->name, name );
         sprintf( buf, "%s %s", name, victim->name );
         do_steal( ch, buf );
         return TRUE;
      }
   }
   return FALSE;
}

bool spec_catalyst( CHAR_DATA * ch )
{
   OBJ_DATA * obj;
   CHAR_DATA * victim;
   unsigned long oldgold;
   oldgold = ch->gold;
   if( !IS_AWAKE( ch ) )
      return FALSE;
   if( ch->gold == 0 )
   {
      for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
      {
         if( victim == ch )
            continue;
         for( obj = ch->first_carrying; obj; obj = obj->next_content )
         {
            if( obj->obj_by && !str_cmp( obj->obj_by, victim->name ) )
            {
               do_say( ch, "Your funneling is done, Master" );
               do_drop( ch, "all" );
               return TRUE;
            }
         }
      }
      return FALSE;
   }
   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      obj->mana += 1;
      if( xIS_SET( obj->extra_flags, ITEM_MAGIC ) )
         obj->mana += 1;
      if( obj->material )
      {
         ch->gold -= obj->material->magic * obj->weight * 20;
      }
      else
      {
         ch->gold -= 100 * obj->weight * 20;
      }
   }
   if( ch->gold < 0 )
      ch->gold = 0;
   if( ch->gold > oldgold )
      ch->gold = 0;
   return TRUE;
}

bool spec_miner( CHAR_DATA * ch )
{
   CHAR_DATA * victim;
   OBJ_DATA * obj;
   MATERIAL_DATA * material;
   if( !IS_AWAKE( ch ) )
      return FALSE;
   if( ch->gold <= 40000 )
   {
      for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
      {
         if( victim == ch )
            continue;
         for( obj = ch->first_carrying; obj; obj = obj->next_content )
         {
            if( obj->obj_by && !str_cmp( obj->obj_by, victim->name ) )
            {
               do_say( ch, "Your mining is done, Master" );
               do_mpjunk( ch, "receipt" );
               do_drop( ch, "all" );
               return TRUE;
            }
         }
      }
      return FALSE;
   }
   if( !ch->first_carrying )
   {
      for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
      {
         if( victim == ch )
            continue;
         obj = create_object( get_obj_index( OBJ_VNUM_SCROLL_SCRIBING ), 0 );
         STRFREE( obj->obj_by );
         obj->obj_by = STRALLOC( victim->name );
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( "receipt" );
         STRFREE( obj->name );
         obj->name = STRALLOC( "receipt" );
         obj_to_char( obj, ch );
         return TRUE;
      }
      return FALSE;
   }
   for( material = first_material; material; material = material->next )
   {
      if( material->sector != ch->in_room->sector_type )
         continue;
      if( material->cost < 100 )
         continue;   /* only get "good" ores */
      if( xIS_SET( material->extra_flags, ITEM_MAGIC )  &&ch->in_room->area->weather->mana < -100 )
         continue;
      if( number_percent(  ) > material->rarity )
         continue;
      obj = make_ore( material->number );
      obj_to_char( obj, ch );
      ch->gold = UMAX( ch->gold - 50000, 0 );
      return TRUE;
   }
   return FALSE;
}

bool spec_farmer( CHAR_DATA * ch )
{
   CHAR_DATA * victim;
   OBJ_DATA * obj;
   INGRED_DATA * ingred;
   if( !IS_AWAKE( ch ) )
      return FALSE;
   if( ch->gold <= 500 )
   {
      for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
      {
         if( victim == ch )
            continue;
         for( obj = ch->first_carrying; obj; obj = obj->next_content )
         {
            if( obj->obj_by && !str_cmp( obj->obj_by, victim->name ) )
            {
               do_say( ch, "Your harvesting is done, Master" );
               do_mpjunk( ch, "receipt" );
               do_drop( ch, "all" );
               return TRUE;
            }
         }
      }
      return FALSE;
   }
   if( !ch->first_carrying )
   {
      for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
      {
         if( victim == ch )
            continue;
         obj = create_object( get_obj_index( OBJ_VNUM_SCROLL_SCRIBING ), 0 );
         STRFREE( obj->obj_by );
         obj->obj_by = STRALLOC( victim->name );
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( "receipt" );
         STRFREE( obj->name );
         obj->name = STRALLOC( "receipt" );
         obj_to_char( obj, ch );
         return TRUE;
      }
      return FALSE;
   }
   for( ingred = first_ingred; ingred; ingred = ingred->next )
   {
      if( ingred->sector != ch->in_room->sector_type )
         continue;
      if( ingred->moisture > ch->in_room->curr_water )
         continue;
      if( ingred->elevation > ch->in_room->curr_elevation )
         continue;
      if( ingred->precip > 0  &&ingred->precip < ch->in_room->area->weather->precip )
         continue;
      if( ingred->precip < 0  &&ingred->precip > ch->in_room->area->weather->precip )
         continue;
      if( ingred->temp > 0  &&ingred->temp < ch->in_room->area->weather->temp )
         continue;
      if( ingred->temp < 0  &&ingred->temp > ch->in_room->area->weather->temp )
         continue;
      if( ingred->mana > 0  &&ingred->mana < ch->in_room->area->weather->mana )
         continue;
      if( number_percent(  ) > ingred->rarity )
         continue;
      if( !get_obj_index( ingred->vnum ) )
      {
         bug( "Spec_farmer: can't find vnum for ingredient %d!", ingred->vnum );
         continue;
      }
      obj = create_object( get_obj_index( ingred->vnum ), 0 );
      obj_to_char( obj, ch );
      ch->gold = UMAX( ch->gold - 1000, 0 );
      return TRUE;
   }
   return FALSE;
}


