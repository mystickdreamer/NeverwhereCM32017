/*
 *      Birth
 *      =======
 *      Birth and Rebirth Module
 *      Written by Keolah for Rogue Winds 2.0
 *
 *      You may not use or distribute any of this code without the
 *      explicit permission of the code maintainer, Heather Dunn (Keolah)
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"

/* from handler.c */
extern SPECIES_DATA *find_species( char *name );

/* from misc.c */
extern sh_int get_full( CHAR_DATA * ch );

void birth( CHAR_DATA * ch );

int count_tokens( char *str )
{
   int i, count;

   count = 0;
   for( i = 0; i < MAX_STRING_LENGTH; i++ )
   {
      if( str[i] == '\0' )
         break;
      if( str[i] == ',' )
         count++;
   }
   return count;
}

char *find_token( char *token, char *str, int num )
{
   int i, count, found, n;
   count = 0;
   n = 0;
   token = STRALLOC( "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" );
   found = FALSE;
   for( i = 0; i < MAX_STRING_LENGTH; i++ )
   {
      if( str[i] == '\0' )
         break;
      if( str[i] == ',' )
      {
         count++;
         if( found )
            break;
      }
      if( count == num && !found )
      {
         found = TRUE;
      }
      if( found && str[i] != ',' )
      {
         token[n] = str[i];
         n++;
      }
   }
   token[n] = '\0';
   return token;
}

void set_race( CHAR_DATA * ch, bool died )
{
   SPECIES_DATA *species;
   OBJ_DATA *obj;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   int i;

   STRFREE( ch->species );
   ch->species = STRALLOC( ch->nation->name );
   species = find_species( ch->nation->species );
   ch->race = species->skill_set;
   STRFREE( ch->pcdata->eyes );
   if( ch->nation->eyes )
   {
      i = count_tokens( ch->nation->eyes );
      i = number_range( 0, i );
      ch->pcdata->eyes = STRALLOC( find_token( ch->pcdata->eyes, ch->nation->eyes, i ) );
   }
   STRFREE( ch->pcdata->hair );
   if( ch->nation->hair )
   {
      i = count_tokens( ch->nation->hair );
      i = number_range( 0, i );
      ch->pcdata->hair = STRALLOC( find_token( ch->pcdata->hair, ch->nation->hair, i ) );
   }
   STRFREE( ch->pcdata->skin_color );
   if( ch->nation->skin_color )
   {
      i = count_tokens( ch->nation->skin_color );
      i = number_range( 0, i );
      ch->pcdata->skin_color = STRALLOC( find_token( ch->pcdata->skin_color, ch->nation->skin_color, i ) );
   }
   STRFREE( ch->pcdata->skin_type );
   if( ch->nation->skin_type )
      ch->pcdata->skin_type = STRALLOC( ch->nation->skin_type );
   STRFREE( ch->pcdata->extra_color );
   if( ch->nation->extra_color )
   {
      i = count_tokens( ch->nation->extra_color );
      i = number_range( 0, i );
      ch->pcdata->extra_color = STRALLOC( find_token( ch->pcdata->extra_color, ch->nation->extra_color, i ) );
   }
   STRFREE( ch->pcdata->extra_type );
   if( ch->nation->extra_type )
   {
      i = count_tokens( ch->nation->extra_type );
      i = number_range( 0, i );
      ch->pcdata->extra_type = STRALLOC( find_token( ch->pcdata->extra_type, ch->nation->extra_type, i ) );
   }
   ch->max_hit = UMAX( 20, ch->nation->hit * get_curr_con( ch ) );
   ch->max_mana = UMAX( 20, ch->nation->mana * get_curr_int( ch ) );
   ch->max_move = UMAX( 20, 20 * get_curr_end( ch ) );
   ch->hit = ch->max_hit;
   ch->mana = ch->max_mana;
   ch->move = ch->max_move;
   ch->height = number_range( ch->nation->height * .9, ch->nation->height * 1.1 );
   ch->weight = get_height_weight( ch );
   if( ch->height == 0 )
      ch->height = 1;
   if( ch->weight == 0 )
      ch->weight = 1;
   ch->affected_by = ch->nation->affected;
   fix_char( ch );
   update_aris( ch );
   ch->pcdata->condition[COND_FULL] = get_full( ch ) / 2;
   ch->pcdata->condition[COND_THIRST] = get_full( ch ) / 2;
   ch->xflags = ch->nation->parts;
   check_bodyparts( ch );

   if( died )
   {
      ch->pcdata->weapon[ch->nation->weapon] = UMAX( 30, ch->pcdata->weapon[ch->nation->weapon] );
      ch->pcdata->age_adjust = ch->nation->base_age;
      STRFREE( ch->last_taken );
      ch->last_taken = STRALLOC( "being reborn" );
      WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
   }
   /*
    * If they're humanoid, give them a little equipment so they aren't naked
    * * Warning, if you change the equipment in stockobj.are
    * * remember to change it here as well.
    */
   if( IS_SET( ch->xflags, 1 << PART_HANDS ) && died )
      for( i = 0; i < 5; i++ )
      {
         obj = NULL;

         if( i == 0 )
            switch ( ch->nation->weapon )
            {
               case SK_SWORD:
                  switch ( number_range( 1, 5 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29063 ), 0 );  /* broadsword */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 29031 ), 0 );  /* short sword */
                        break;
                     case 3:
                        obj = create_object( get_obj_index( 29033 ), 0 );  /* long sword */
                        break;
                     case 4:
                        obj = create_object( get_obj_index( 29034 ), 0 );  /* bastard sword */
                        break;
                     case 5:
                        obj = create_object( get_obj_index( 29035 ), 0 );  /* great sword */
                        break;
                  }
                  break;
               case SK_DAGGER:
                  switch ( number_range( 1, 3 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29030 ), 0 );  /* dagger */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 29031 ), 0 );  /* dirk */
                        break;
                     case 3:
                        obj = create_object( get_obj_index( 29019 ), 0 );  /* knife */
                        break;
                  }
                  break;
               case SK_AXE:
                  switch ( number_range( 1, 3 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29039 ), 0 );  /* battle axe */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 29038 ), 0 );  /* hand axe */
                        break;
                     case 3:
                        obj = create_object( get_obj_index( 29040 ), 0 );  /* greataxe */
                        break;
                  }
                  break;
               case SK_WHIP:
                  switch ( number_range( 1, 2 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29043 ), 0 );  /* whip */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 28936 ), 0 );  /* flail */
                        break;
                  }
                  break;
               case SK_MACE:
                  switch ( number_range( 1, 2 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29036 ), 0 );  /* mace */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 28926 ), 0 );  /* club */
                        break;
                  }
                  break;
               case SK_STAFF:
                  switch ( number_range( 1, 2 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29055 ), 0 );  /* quarterstaff */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 28955 ), 0 );  /* staff */
                        break;
                  }
                  break;
               case SK_HALBERD:
                  switch ( number_range( 1, 2 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29042 ), 0 );  /* halberd */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 28942 ), 0 );  /* glaive */
                        break;
                  }
                  break;
               case SK_SPEAR:
                  switch ( number_range( 1, 2 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29041 ), 0 );  /* spear */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 29091 ), 0 );  /* trident */
                        break;
                  }
                  break;
               case SK_POLEARM:
                  switch ( number_range( 1, 2 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 28925 ), 0 );  /* scythe */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 29117 ), 0 );  /* sickle */
                        break;
                  }
                  break;
               case SK_BOW:
                  obj = create_object( get_obj_index( 29047 ), 0 );  /* long bow */
                  break;
               case SK_FIREARMS:
                  switch ( number_range( 1, 3 ) )
                  {
                     case 1:
                        obj = create_object( get_obj_index( 29049 ), 0 );  /* pistol */
                        break;
                     case 2:
                        obj = create_object( get_obj_index( 28939 ), 0 );  /* shotgun */
                        break;
                     case 3:
                        obj = create_object( get_obj_index( 28938 ), 0 );  /* rifle */
                        break;
                  }
                  break;
               case SK_CROSSBOW:
                  obj = create_object( get_obj_index( 29045 ), 0 );  /* crossbow */
                  break;
               case SK_WAND:
                  obj = create_object( get_obj_index( 29066 ), 0 );  /* wand */
                  break;
            }
         if( i == 1 )
         {
            if( IS_SET( ch->xflags, 1 << PART_LEGS ) )
            {
               switch ( number_range( 1, 5 ) )
               {
                  case 1:
                     obj = create_object( get_obj_index( 28910 ), 0 );  /* pants */
                     break;
                  case 2:
                     obj = create_object( get_obj_index( 29016 ), 0 );  /* leggings */
                     break;
                  case 3:
                     obj = create_object( get_obj_index( 29018 ), 0 );  /* greaves */
                     break;
                  case 4:
                     obj = create_object( get_obj_index( 28901 ), 0 );  /* kilt */
                     break;
                  case 5:
                     obj = create_object( get_obj_index( 29083 ), 0 );  /* loincloth */
                     break;
               }
            }
            else
            {
               switch ( number_range( 1, 5 ) )
               {
                  case 1:
                     obj = create_object( get_obj_index( 28917 ), 0 );  /* saddle */
                     break;
                  case 2:
                     obj = create_object( get_obj_index( 29064 ), 0 );  /* amulet */
                     break;
                  case 3:
                     obj = create_object( get_obj_index( 29061 ), 0 );  /* necklace */
                     break;
                  case 4:
                     obj = create_object( get_obj_index( 29110 ), 0 );  /* torc */
                     break;
                  case 5:
                     obj = create_object( get_obj_index( 29139 ), 0 );  /* scarf */
                     break;
               }
            }
         }
         if( i == 2 )
         {
            if( IS_SET( ch->xflags, 1 << PART_FEET ) )
            {
               switch ( number_range( 1, 4 ) )
               {
                  case 1:
                     obj = create_object( get_obj_index( 29008 ), 0 );  /* boots */
                     break;
                  case 2:
                     obj = create_object( get_obj_index( 29009 ), 0 );  /* shod boots */
                     break;
                  case 3:
                     obj = create_object( get_obj_index( 28931 ), 0 );  /* sandals */
                     break;
                  case 4:
                     obj = create_object( get_obj_index( 29017 ), 0 );  /* socks */
                     break;
               }
            }
            else if( IS_SET( ch->xflags, 1 << PART_HOOVES ) )
            {
               obj = create_object( get_obj_index( 29085 ), 0 );  /* horseshoes */
            }
         }
         if( i == 3 )
         {
            switch ( number_range( 1, 5 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 29109 ), 0 );  /* tunic */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29013 ), 0 );  /* robe */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 29000 ), 0 );  /* chainmail */
                  break;
               case 4:
                  obj = create_object( get_obj_index( 28911 ), 0 );  /* shirt */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 29014 ), 0 );  /* cloak */
                  break;
            }
         }
         if( i == 4 )
         {
            switch ( number_range( 1, 5 ) )
            {
               case 1:
                  obj = create_object( get_obj_index( 29062 ), 0 );  /* pack */
                  break;
               case 2:
                  obj = create_object( get_obj_index( 29072 ), 0 );  /* pouch */
                  break;
               case 3:
                  obj = create_object( get_obj_index( 29073 ), 0 );  /* bag */
                  break;
               case 4:
                  obj = create_object( get_obj_index( 29074 ), 0 );  /* chest */
                  break;
               case 5:
                  obj = create_object( get_obj_index( 29075 ), 0 );  /* box */
                  break;
            }
         }
         if( obj )
         {
            MATERIAL_DATA *mat;
            AFFECT_DATA *paf;
            extern int top_affect;

            mat = first_material;
            while( mat )
            {
               if( ( xIS_SET( obj->extra_flags, ITEM_METAL )
                     && xIS_SET( mat->extra_flags, ITEM_METAL ) )
                   || ( xIS_SET( obj->extra_flags, ITEM_FLAMMABLE )
                        && xIS_SET( mat->extra_flags, ITEM_FLAMMABLE ) )
                   || ( xIS_SET( obj->extra_flags, ITEM_ORGANIC ) && xIS_SET( mat->extra_flags, ITEM_ORGANIC ) ) )
               {
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

               obj->value[2] = 100;

               xSET_BITS( obj->extra_flags, mat->extra_flags );

               obj->cost += number_fuzzy( mat->cost );
               obj->size = ch->height;
               obj->weight = UMAX( obj->weight + ( ch->height - 66 ) / 10, 1 );

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
               obj_to_char( obj, ch );
               if( xIS_EMPTY( obj->parts ) )
                  do_wield( ch, obj->name );
               else
                  equip_char( ch, obj );
            }
         }
      }
}

void choose_sex( CHAR_DATA * ch, char *argument )
{
   int i;

   if( nifty_is_name( "newbie", argument ) )
   {
      do_chat( ch, argument );
      return;
   }

   if( !str_prefix( argument, "m" ) )
      ch->sex = SEX_MALE;
   else if( !str_prefix( argument, "f" ) )
      ch->sex = SEX_FEMALE;
   else if( !str_prefix( argument, "h" ) )
      ch->sex = SEX_HERMAPH;
   else if( !str_prefix( argument, "n" ) )
      ch->sex = SEX_NEUTRAL;
   else
   {
      i = number_range( 0, 100 );
      if( i == 0 )
         ch->sex = SEX_NEUTRAL;
      else if( i == 100 )
         ch->sex = SEX_HERMAPH;
      else if( i < 50 )
         ch->sex = SEX_MALE;
      else
         ch->sex = SEX_FEMALE;
   }
   send_to_char( "Your gender has been chosen.\n\r", ch );
   add_char( ch );
   ch->desc->connected = CON_PLAYING;
   return;
}

void choose_species( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   SPECIES_DATA *species;
   NATION_DATA *nation;
   int i, n;

   if( !str_cmp( argument, "races" ) )
   {
      do_races( ch, "" );
      return;
   }

   if( nifty_is_name( "races", argument ) )
   {
      do_races( ch, argument + 6 );
      return;
   }

   if( nifty_is_name( "help", argument ) )
   {
      do_new_help( ch, argument + 5 );
      return;
   }

   if( !str_cmp( argument, "random" ) )
   {
      xSET_BIT( ch->act, PLR_FREEZE );
      if( ch->pcdata->points > 100 )
         ch->pcdata->points = 30;
      birth( ch );
      xREMOVE_BIT( ch->act, PLR_FREEZE );
      if( !xIS_SET( ch->act, PLR_AUTOSEX ) )
      {
         ch->desc->connected = CON_CHOOSE_SEX;
         send_to_char( "&YChoose your gender [M/F/H/N]\n\r", ch );
      }
      else
      {
         add_char( ch );
         ch->desc->connected = CON_PLAYING;
      }
      return;
   }

   species = find_species( argument );
   if( !species )
   {
      send_to_char( "That's not a species.\n\r", ch );
      send_to_char( "What IS your species (or RANDOM if you wish)? ", ch );
      return;
   }
   i = 0;
   n = 0;
   for( nation = first_nation; nation; nation = nation->next )
   {
      if( !str_cmp( nation->species, species->name ) )
         i++;
   }
   i = number_range( 1, i );
   for( nation = first_nation; nation; nation = nation->next )
   {
      if( !str_cmp( nation->species, species->name ) )
         n++;
      if( n == i )
      {
         ch->nation = nation;
         break;
      }
   }
   set_race( ch, TRUE );
   if( ch->in_room )
      char_from_room( ch );
   char_to_room( ch, get_room_index( ROOM_VNUM_NEXUS ) );
   ch_printf( ch, "&WYou are reborn as %s.\n\r", aoran( ch->nation->name ) );
   act( AT_WHITE, "$n is reborn as $T.", ch, NULL, aoran( ch->nation->name ), TO_ROOM );
   do_look( ch, "auto" );
   sprintf( buf, "%s has been reborn as %s in %s (%d).",
            ch->name, aoran( ch->nation->name ), ch->in_room->name, ch->in_room->vnum );
   log_string_plus( buf, LOG_COMM, 0 );
   if( IS_SET( sysdata.save_flags, SV_DEATH ) )
      save_char_obj( ch );

   if( !xIS_SET( ch->act, PLR_AUTOSEX ) )
   {
      ch->desc->connected = CON_CHOOSE_SEX;
      send_to_char( "&YChoose your gender [M/F/H/N]\n\r", ch );
   }
   else
   {
      add_char( ch );
      ch->desc->connected = CON_PLAYING;
   }
}

void choose_nation( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   NATION_DATA *nation;

   if( !str_cmp( "races", argument ) )
   {
      do_races( ch, "" );
      return;
   }

   if( nifty_is_name( "races", argument ) )
   {
      do_races( ch, argument + 6 );
      return;
   }

   if( nifty_is_name( "help", argument ) )
   {
      do_new_help( ch, argument + 5 );
      return;
   }

   if( nifty_is_name( "newbie", argument ) )
   {
      do_chat( ch, argument );
      return;
   }

   if( !str_cmp( argument, "random" ) )
   {
      xSET_BIT( ch->act, PLR_FREEZE );
      if( ch->pcdata->points > 100 )
         ch->pcdata->points = 30;
      birth( ch );
      xREMOVE_BIT( ch->act, PLR_FREEZE );
      if( !xIS_SET( ch->act, PLR_AUTOSEX ) )
      {
         ch->desc->connected = CON_CHOOSE_SEX;
         send_to_char( "&YChoose your gender [M/F/H/N]\n\r", ch );
      }
      else
      {
         add_char( ch );
         ch->desc->connected = CON_PLAYING;
      }
      return;
   }

   nation = find_nation( argument );
   if( !nation )
   {
      send_to_char( "That's not a nation.\n\r", ch );
      send_to_char( "What IS your nation (or RANDOM if you wish)? ", ch );
      return;
   }
   ch->nation = nation;
   set_race( ch, TRUE );
   if( ch->in_room )
      char_from_room( ch );
   char_to_room( ch, get_room_index( ROOM_VNUM_NEXUS ) );
   if( IS_NEWBIE( ch ) )
   {
      ch_printf( ch, "&WYou are born into the Planes as %s.\n\r", aoran( ch->nation->name ) );
      act( AT_WHITE, "$n is born into the Planes as $T.", ch, NULL, aoran( ch->nation->name ), TO_ROOM );
   }
   else
   {
      ch_printf( ch, "&WYou are reborn as %s.\n\r", aoran( ch->nation->name ) );
      act( AT_WHITE, "$n is reborn as $T.", ch, NULL, aoran( ch->nation->name ), TO_ROOM );
   }
   do_look( ch, "auto" );
   sprintf( buf, "%s has been reborn as %s in %s (%d).",
            ch->name, aoran( ch->nation->name ), ch->in_room->name, ch->in_room->vnum );
   log_string_plus( buf, LOG_COMM, 0 );
   if( IS_SET( sysdata.save_flags, SV_DEATH ) )
      save_char_obj( ch );

   if( !xIS_SET( ch->act, PLR_AUTOSEX ) )
   {
      ch->desc->connected = CON_CHOOSE_SEX;
      send_to_char( "&YChoose your gender [M/F/H/N]\n\r", ch );
   }
   else
   {
      add_char( ch );
      ch->desc->connected = CON_PLAYING;
   }
}

void newbie_death( CHAR_DATA * ch )
{
   CHAR_DATA *wizard;
   char buf[MAX_STRING_LENGTH];
   PART_DATA *part;

   send_to_char( "You fall unconscious, blackness as the thickest Void encompassing you.\n\r", ch );

   /*
    * Repair any injuries 
    */
   part = ch->first_part;
   while( part )
   {
      part->cond = PART_WELL;
      part->flags = PART_WELL;
      part = part->next;
   }
   ch->hit = ch->max_hit;
   ch->mana = ch->max_mana;
   ch->move = ch->max_move;

   send_to_char( "But after a time, slowly you wake, exhausted and weary, in the Nexus.\n\r", ch );

   ch->position = POS_RESTING;
   if( ch->in_room )
      char_from_room( ch );
   char_to_room( ch, get_room_index( ROOM_VNUM_NEXUS ) );

   act( AT_MAGIC, "$n appears unconscious from a cloud of mist.", ch, NULL, NULL, TO_ROOM );
   act( AT_ACTION, "$n stirs slowly, $s eyes blinking open.", ch, NULL, NULL, TO_ROOM );

   for( wizard = ch->in_room->first_person; wizard; wizard = wizard->next_in_room )
   {
      if( nifty_is_name( "wizard", wizard->name ) )
         break;
   }
   if( wizard )
   {
      sprintf( buf, "That was a close one, %s! You should be more careful.", ch->name );
      do_say( wizard, buf );
   }
   ch->mental_state = 0;
   fix_char( ch );
   update_aris( ch );
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( "disoriented from nearly dying" );
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
}

void birth( CHAR_DATA * ch )
{
   int i, n, tal;
   int power;
   AREA_DATA *area;
   GENERIC_MOB *race;
   SPECIES_DATA *species;
   ROOM_INDEX_DATA *pRoomIndex;
   char buf[MAX_STRING_LENGTH];
   DEITY_DATA *talent;
   PART_DATA *part;
   EXPLORED_AREA *xarea;

   if( IS_NEWBIE( ch ) && IS_SET( ch->pcdata->flags, PCFLAG_HELPSTART ) )
   {
      newbie_death( ch );
      return;
   }

   act( AT_DGREY, "You are drawn into the Void!", ch, NULL, NULL, TO_CHAR );
   act( AT_DGREY, "$n is drawn into the Void!", ch, NULL, NULL, TO_ROOM );
   while( ch->first_affect )
      affect_remove( ch, ch->first_affect );

   /*
    * Strip any affects 
    */
   STRFREE( ch->pcdata->type );
   ch->pcdata->type = STRALLOC( "living" );
   REMOVE_BIT( ch->pcdata->flags, PCFLAG_MASK );
   xREMOVE_BIT( ch->act, PLR_WIZINVIS );
   REMOVE_BIT( ch->pcdata->flags, PCFLAG_SHIELD );
   REMOVE_BIT( ch->pcdata->flags, PCFLAG_VAMPIRE );
   xCLEAR_BITS( ch->pcdata->perm_aff );

   send_to_char( "Your soul shoots through the Void at a dizzying speed,\n\r", ch );

   /*
    * Repair any injuries 
    */
   part = ch->first_part;
   while( part )
   {
      part->cond = PART_WELL;
      part->flags = PART_WELL;
      part = part->next;
   }

   send_to_char( "but time and space have no meaning here.\n\r", ch );

   /*
    * Forget a random area 
    */
   if( !xIS_SET( ch->act, PLR_FREEZE ) )
   {
      n = 0;
      for( xarea = ch->pcdata->first_explored; xarea; xarea = xarea->next )
      {
         n++;
      }
      i = number_range( 1, n );
      n = 0;
      for( xarea = ch->pcdata->first_explored; xarea; xarea = xarea->next )
      {
         n++;
         if( i == n )
         {
            UNLINK( xarea, ch->pcdata->first_explored, ch->pcdata->last_explored, next, prev );
            DISPOSE( xarea );
            send_to_char( "Within the Void, your memories of your previous lives fade.\n\r", ch );
            break;
         }
      }
   }

   /*
    * Screw with their talents 
    */
   if( ( IS_SET( ch->pcdata->flags, PCFLAG_HELPSTART )
         && !xIS_SET( ch->act, PLR_FREEZE ) )
       || ( !IS_SET( ch->pcdata->flags, PCFLAG_HELPSTART ) && xIS_SET( ch->act, PLR_FREEZE ) ) )
   {
      if( ch->pcdata->inborn == -1 )
      {
         ch->pcdata->inborn = number_range( 0, 18 );
         ch->talent[ch->pcdata->inborn] = number_range( 50, 150 );
         ch->talent[number_range( 0, 18 )] += number_range( 10, 50 );
      }
      n = 0;
      tal = -1;
      power = 0;
      for( talent = first_deity; talent; talent = talent->next )
      {
         i = talent->index;
         if( ch->curr_talent[i] <= 10 )
            ch->talent[i] -= number_range( 1, 3 );
         if( ch->talent[i] < 20 && ch->curr_talent[i] >= 20 )
            ch->talent[i] += number_range( 1, 3 );
         if( ch->curr_talent[i] >= 50 && ( ch->talent[i] < 100 ) )
            ch->talent[i] += number_range( 1, 3 );
         if( ch->curr_talent[i] >= 100 && ( ch->talent[i] < 150 ) )
            ch->talent[i] += number_range( 1, 3 );
         if( ch->curr_talent[i] >= 150 && ( ch->talent[i] < 200 ) )
            ch->talent[i] += number_range( 1, 3 );
         if( ch->curr_talent[i] >= 200 )
            ch->talent[i] += number_range( 1, 3 );
         ch->talent[i] += number_range( -2, 2 );
         ch->curr_talent[i] -= number_range( 2, 6 );
         if( ch->curr_talent[i] < 1 )
            ch->curr_talent[i] = 1;
         if( ch->talent[i] < 1 )
            ch->talent[i] = 1;
         if( ch->talent[i] > n )
         {
            n = ch->talent[i];
            tal = i;
         }
         power += ch->talent[i] * talent->cost;
      }
      ch->pcdata->inborn = tal;
      send_to_char( "Magic shifts within you as strands of the Talent Pool cling and pull at you.\n\r", ch );
   }
   if( get_timer( ch, TIMER_PKILLED ) > 0 )
      remove_timer( ch, TIMER_PKILLED );
   ch->hit = ch->max_hit;
   ch->mana = ch->max_mana;
   ch->move = ch->max_move;
   ch->exp = 0;
   ch->mental_state = 0;
   ch->pcdata->condition[COND_DRUNK] = 0;
   ch->speed = 90;

   send_to_char( "Before you lies the expanse of the universe, a thousand paths in every direction...\n\r", ch );

   /*
    * Choose your destiny 
    */
   if( !xIS_SET( ch->act, PLR_FREEZE ) )
   {
      if( IS_NEWBIE( ch ) || power > number_range( 50, 200 ) )
      {
         send_to_char( "&BYou may choose your destiny...\n\r", ch );
         if( IS_NEWBIE( ch ) || power > number_range( 500, 3000 ) )
         {
            if( IS_SET( ch->pcdata->flags, PCFLAG_HELPSTART ) )
               UNLINK( ch, first_char, last_char, next, prev );
            ch->desc->connected = CON_CHOOSE_NATION;
            send_to_char( "&BAll paths are open to you. Choose a nation, or RANDOM if you wish.\n\r", ch );
            send_to_char( "&BType RACES or RACES <species> for a list, and HELP <race> for information.\n\r", ch );
            return;
         }
         else if( power > number_range( 200, 1000 ) )
         {
            UNLINK( ch, first_char, last_char, next, prev );
            ch->desc->connected = CON_CHOOSE_SPECIES;
            send_to_char( "\n\rYou may choose from the following species:\n\r", ch );
            buf[0] = '\0';
            species = first_species;
            while( species )
            {
               if( ( strlen( buf ) + strlen( species->name ) ) > 75 )
               {
                  strcat( buf, "\n\r" );
                  send_to_char( buf, ch );
                  buf[0] = '\0';
               }
               else
                  strcat( buf, " | " );
               strcat( buf, species->name );
               species = species->next;
            }
            strcat( buf, " |\n\r " );
            send_to_char( buf, ch );
            return;
         }
         else
         {
            if( !xIS_SET( ch->act, PLR_AUTOSEX ) )
            {
               UNLINK( ch, first_char, last_char, next, prev );
               ch->desc->connected = CON_CHOOSE_SEX;
               send_to_char( "&YChoose your gender [M/F/H/N]\n\r", ch );
            }
         }
      }
      else
      {
         i = number_range( 0, 100 );
         if( i == 0 )
            ch->sex = SEX_NEUTRAL;
         else if( i == 100 )
            ch->sex = SEX_HERMAPH;
         else if( i < 50 )
            ch->sex = SEX_MALE;
         else
            ch->sex = SEX_FEMALE;
      }
   }
   /*
    * Pick a random race by area. 
    * * Loop until successful.
    * * Make damn sure you have at least one area with mobs in it,
    * * or this will freeze up.
    */
   send_to_char( "One path draws you along, spinning wildly out of your control.\n\r", ch );
   while( 1 )
   {
      i = number_range( 0, last_area->low_soft_range );
      for( area = first_area; area; area = area->next )
      {
         if( area->low_soft_range != i )
            continue;
         i = 0;
         for( race = area->first_race; race; race = race->next )
            i++;
         i = number_range( 1, i );
         n = 0;
         for( race = area->first_race; race; race = race->next )
         {
            n++;
            if( n == i )
            {
               if( xIS_SET( race->nation->affected, AFF_UNDEAD ) )
                  continue;
               ch->nation = race->nation;
               set_race( ch, TRUE );

               /*
                * Now put them in the area 
                */
               for( i = 0; i < 100; i++ )
               {
                  n = number_range( area->low_r_vnum, area->hi_r_vnum );
                  if( ( pRoomIndex = get_room_index( n ) ) == NULL )
                     continue;
                  if( IS_SET( pRoomIndex->room_flags, ROOM_NOFLOOR ) )
                  {
                     if( IS_SET( ch->xflags, PART_WINGS ) )
                     {
                        xSET_BIT( ch->pcdata->perm_aff, AFF_FLYING );
                     }
                     else
                        continue;
                  }
                  if( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) && !IS_AFFECTED( ch, AFF_AQUATIC ) && pRoomIndex->curr_water > 20 )
                     continue;
                  if( IS_AFFECTED( ch, AFF_AQUATIC ) && !IS_AFFECTED( ch, AFF_AQUA_BREATH ) && pRoomIndex->curr_water < 20 )
                     continue;
                  if( ch->in_room )
                     char_from_room( ch );
                  char_to_room( ch, pRoomIndex );
               }

               ch_printf( ch, "You enter the unborn body of %s child.\n\r", aoran( ch->nation->name ) );
               act( AT_WHITE, "$n is reborn as $T.", ch, NULL, aoran( ch->nation->name ), TO_ROOM );
               do_look( ch, "auto" );
               sprintf( buf, "%s has been reborn as %s in %s (%d).",
                        ch->name, aoran( ch->nation->name ), ch->in_room->name, ch->in_room->vnum );
               log_string_plus( buf, LOG_COMM, 0 );
               if( IS_SET( sysdata.save_flags, SV_DEATH ) )
                  save_char_obj( ch );
               send_to_char( "As you are born and grow, your life begins anew.\n\r", ch );
               return;
            }
         }
      }
   }
}

void ghost( CHAR_DATA * ch )
{
   if( IS_NEWBIE( ch ) )
   {
      newbie_death( ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_ETHEREAL ) )
   {
      birth( ch );
      return;
   }
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( "disoriented from dying" );
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
   act( AT_GREY, "You drift out of your body as a ghost.", ch, NULL, NULL, TO_CHAR );
   act( AT_GREY, "$n drifts out of $s body as a ghost.", ch, NULL, NULL, TO_ROOM );
   ch->exp = 0;
   ch->hit = ch->max_hit;
   ch->mana = ch->max_mana;
   ch->move = ch->max_move;
   ch->position = POS_STANDING;
   add_timer( ch, TIMER_PKILLED, 100, NULL, 0 );
   STRFREE( ch->pcdata->type );
   ch->pcdata->type = STRALLOC( "ghost" );
   xSET_BIT( ch->pcdata->perm_aff, AFF_ETHEREAL );
   xSET_BIT( ch->pcdata->perm_aff, AFF_UNDEAD );
   xSET_BIT( ch->pcdata->perm_aff, AFF_NONLIVING );
   xSET_BIT( ch->pcdata->perm_aff, AFF_NO_CORPSE );

   if( IS_SET( sysdata.save_flags, SV_DEATH ) )
      save_char_obj( ch );

   extract_char( ch, FALSE );
}

void do_rebirth( CHAR_DATA * ch, char *argument )
{
   if( !IS_AFFECTED( ch, AFF_ETHEREAL ) )
   {
      send_to_char( "It isn't so easy entering the Void when you are in a physical form.\n\r", ch );
      return;
   }
   send_to_char( "You release your hold on the physical world, and allow the Void to take you.\n\r", ch );
   birth( ch );
}

bool check_rebirth( CHAR_DATA * ch, int i )
{
   if( !ch->desc || !ch->desc->connected == CON_PLAYING )
      return FALSE;
   if( !IS_AFFECTED( ch, AFF_ETHEREAL ) )
      i -= 1000;
   if( !IS_AFFECTED( ch, AFF_UNDEAD ) )
      i -= 1000;
   if( IS_AFFECTED( ch, AFF_MAGICAL ) )
      i -= 500;
   if( IS_AFFECTED( ch, AFF_VOID ) )
      i += 1500;
   if( IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      i += 500;
   i += number_range( -500, 200 );
   i -= TALENT( ch, TAL_DEATH );
   i -= TALENT( ch, TAL_VOID );
   if( i > 0 )
   {
      birth( ch );
      return TRUE;
   }
   else if( i > -100 )
      send_to_char( "You feel the Void pulling strongly at your soul.\n\r", ch );
   else if( i > -300 )
      send_to_char( "You are aware of the Void drawing at your soul.\n\r", ch );
   else if( i > -500 )
      send_to_char( "You sense the Void tugging faintly at your soul.\n\r", ch );
   return FALSE;
}
