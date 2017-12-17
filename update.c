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
 *			      Regular update module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
extern int track_direction( CHAR_DATA * ch, CHAR_DATA * vict );
extern CHAR_DATA *find_keeper args( ( CHAR_DATA * ch ) );
extern void save_world( CHAR_DATA * ch );

/* from handler.c */ 
extern NATION_DATA *find_nation( char *name );
extern SPECIES_DATA *find_species( char *name );
extern int moon_visible( AREA_DATA * tarea, int flag );

/* from talent.c */ 
void player_echo( CHAR_DATA * ch, sh_int AT_COLOR, char *argument, sh_int tar );

/* from city.c */ 
void city_update( void );

/* from arena.c */ 
void save_arenas( void );
void arena_update( void );

/* from misc.c */ 
extern sh_int get_full( CHAR_DATA * ch );

/*
 * Local functions.
 */ 
int hit_gain args( ( CHAR_DATA * ch ) );
int mana_gain args( ( CHAR_DATA * ch ) );
int move_gain args( ( CHAR_DATA * ch ) );
void mobile_update args( ( void ) );
void weather_update args( ( void ) );
void time_update args( ( void ) ); /* FB */
void char_update args( ( void ) );
void char_regen args( ( void ) );  /* Scion */
void obj_update args( ( void ) );
void hunger_thirst_update args( ( void ) ); /* Scion */
void aggr_update args( ( void ) );
void room_act_update args( ( void ) );
void obj_act_update args( ( void ) );
void char_check args( ( void ) );
void drunk_randoms args( ( CHAR_DATA * ch ) );
void hallucinations args( ( CHAR_DATA * ch ) );
void subtract_times args( ( struct timeval * etime, struct timeval * stime ) );

/* weather functions - FB */ 
void adjust_vectors args( ( WEATHER_DATA * weather ) );
void get_weather_echo args( ( AREA_DATA * tarea ) );
void get_time_echo args( ( WEATHER_DATA * weather ) );

/*
 * Global Variables
 */ 
   CHAR_DATA * gch_prev;
OBJ_DATA * gobj_prev;
CHAR_DATA * timechar;
extern int top_exit;

/*
 * Regeneration stuff.
 */ 
int hit_gain( CHAR_DATA * ch )
{
   PART_DATA * part;
   int gain;
   char buf[MAX_STRING_LENGTH];
   if( char_died( ch ) )
      return 0;
   if( IS_NPC( ch ) )
      
   {
      gain = get_curr_con( ch ) * 3 / 2;
   }
   
   else
      
   {
      gain = UMIN( 5, ( get_curr_con( ch ) / 10 ) );
   }
   if( !ch->nation && !IS_NPC( ch ) )
      ch->nation = find_nation( ch->species );
   if( !ch->nation && !IS_NPC( ch ) )
   {
      bug( "hit_gain: could not find nation!", 0 );
      return 0;
   }
   switch ( ch->position )
      
   {
      case POS_STUNNED:
         gain += 10;
      case POS_SLEEPING:
         gain += get_curr_con( ch ) / 3 * 2.0;
         if( ch->on )
            gain *= 2;
         break;
      case POS_RESTING:
         gain += get_curr_con( ch ) / 3 * 1.25;
         if( ch->on )
            gain *= 1.25;
         break;
   }
   if( ch->nation )
      gain *= ch->nation->metabolism;
   gain *= ch->speed / 100;
   if( IS_AFFECTED( ch, AFF_HASTE ) )
      gain *= 2;
   if( IS_AFFECTED( ch, AFF_SLOW ) )
      gain /= 2;
   if( IS_SET( ch->in_room->runes, RUNE_LIFE ) )
      gain *= 3;
   if( !IS_AFFECTED( ch, AFF_NONLIVING ) && !IS_NPC( ch ) )
      if( IS_AFFECTED( ch, AFF_POISON ) 
           ||IS_AFFECTED( ch, AFF_PLAGUE ) 
           ||( ch->pcdata->condition[COND_FULL] == 0 
               &&ch->curr_talent[TAL_EARTH] < 100 ) 
           ||( ch->pcdata->condition[COND_THIRST] == 0  &&ch->curr_talent[TAL_WATER] < 100 ) )
         gain = 0;
   for( part = ch->first_part; part; part = part->next )
   {
      if( !IS_AFFECTED( ch, AFF_NONLIVING ) )
      {
         if( part->cond < 0 )
         {
            if( ch->in_room && IS_UNDERWATER( ch ) )
            {
               act( AT_BLOOD, "Blood clouds the water.", ch, NULL, NULL, TO_ROOM );
               ch_printf( ch, "&rYou bleed from your %s and cloud the water.\n\r", part_locs[part->loc] );
            }
            else
            {
               ch_printf( ch, "&rBlood spills from your %s.\n\r", part_locs[part->loc] );
               sprintf( buf, "$n bleeds from $s %s.", part_locs[part->loc] );
               act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );
               make_blood( ch, abs( part->cond ) / 10 );
            }
            if( direct_damage( ch, abs( part->cond ) ) )
               return 0;
         }
      }
      
         /*
          * faster due to cloting 
          */ 
         part->cond = UMIN( PART_WELL, part->cond + ( gain * 3 ) );
   }
   if( !IS_SET( ch->mood, MOOD_RELAXED ) )
      return 0;
   better_mental_state( ch, gain / 3 );
   return UMIN( gain, ch->max_hit - ch->hit );
}
int mana_gain( CHAR_DATA * ch ) 
{
   int gain;
   int i;
   if( IS_NPC( ch ) )
      
   {
      gain = get_curr_int( ch );
   }
   
   else
      
   {
      gain = UMIN( 5, ( get_curr_int( ch ) / 10 ) );
      gain /= 4; /* Compensate for the tap command */
      if( !ch->nation )
         ch->nation = find_nation( ch->species );
      if( !ch->nation )
      {
         bug( "mana_gain: could not find nation!", 0 );
         return 0;
      }
      switch ( ch->position )
         
      {
         case POS_SLEEPING:
            gain += get_curr_int( ch ) / 3 * 3.25;
            if( ch->on )
               gain += 5;
            break;
         case POS_RESTING:
            gain += get_curr_int( ch ) / 3 * 1.75;
            if( ch->on )
               gain += 2;
            break;
         default:
            
               /*
                * Fuck with players that have negative mana -- Scion 
                */ 
               if( ch->mana < 0 )
            {
               if( ch->shield )
               {
                  send_to_char( "You are unable to maintain your shield any longer, and it collapses.\n\r", ch );
                  ch->shield = MAG_NONE;
               }
               if( ch->singing )
               {
                  send_to_char( "Your voice grows hoarse, and you cannot sing any longer.\n\r", ch );
                  do_sing( ch, "none" );
               }
               if( ch->mana < -1000 )
               {
                  send_to_char( "You feel as though your soul is being ripped out!\r\n", ch );
                  lose_hp( ch, 50 );
               }
               else
               {
                  send_to_char( "You feel drained, devoid of your life essence!\r\n", ch );
               }
               check_rebirth( ch, ch->mana * -1 );
               worsen_mental_state( ch, 25 );
               ch->move -= 50;
            }
      }
      if( ch->position < POS_SLEEPING )
         return 0;
      if( ch->pcdata->condition[COND_FULL] == 0 )
         gain /= 2;
      if( ch->pcdata->condition[COND_THIRST] == 0 )
         gain /= 2;
   }
   
      /*
       * cost to maintain shields and songs -keo 
       */ 
      if( ch->shield )
   {
      if( ch->mana > 0 )
         ch->mana -= 100;
      
      else
      {
         ch->shield = 0;
         send_to_char( "You are unable to maintain your shield any longer.\n\r", ch );
      }
   }
   if( ch->singing )
   {
      if( IS_SILENT( ch ) )
      {
         do_sing( ch, "none" );
      }
      else
      {
         send_to_char( "&PYou continue singing.\n\r", ch );
         ch->mana -= 30;
      }
   }
   
      /*
       * Adjust for the amount of mana in the zone -- Scion 
       */ 
      i = number_range( 0, ch->in_room->area->weather->mana * 10 );
   ch->in_room->area->weather->mana -= ( int )i / 10;
   gain += UMAX( 0, i );
   return UMIN( gain, ch->max_mana - ch->mana );
}
int move_gain( CHAR_DATA * ch ) 
{
   int gain = 0;
   if( IS_NPC( ch ) )
      
   {
      gain = get_curr_end( ch );
   }
   
   else
      
   {
      
         /*
          * Fuck with players at negative mana -- Scion 
          */ 
         if( ch->mana < 0 || IS_SET( ch->mood, MOOD_READY ) )
         gain -= 15;
      if( ch->move < 0 )
         ch->move = 0;
      gain = UMAX( 15, ( get_curr_end( ch ) / 10 ) );
      if( !ch->nation )
         ch->nation = find_nation( ch->species );
      if( !ch->nation )
      {
         bug( "move_gain: could not find nation!", 0 );
         return 0;
      }
      switch ( ch->position )
         
      {
         case POS_STUNNED:
         case POS_SLEEPING:
            gain += get_curr_dex( ch ) / 3 * 4.5;
            if( ch->on )
               gain *= 3;
            gain *= ch->nation->metabolism;
            gain *= ch->speed / 100;
            if( IS_AFFECTED( ch, AFF_HASTE ) )
               gain *= 2;
            if( IS_AFFECTED( ch, AFF_SLOW ) )
               gain /= 2;
            break;
         case POS_RESTING:
            gain += get_curr_dex( ch ) / 3 * 2.5;
            if( ch->on )
               gain *= 2;
            gain *= ch->nation->metabolism;
            gain *= ch->speed / 100;
            if( IS_AFFECTED( ch, AFF_HASTE ) )
               gain *= 2;
            if( IS_AFFECTED( ch, AFF_SLOW ) )
               gain /= 2;
            break;
      }
      if( IS_SET( ch->in_room->runes, RUNE_LIFE ) )
         gain *= 4;
      if( ch->pcdata->condition[COND_FULL] == 0 )
         gain /= 2;
      if( ch->pcdata->condition[COND_THIRST] == 0 )
         gain /= 2;
   }
   if( IS_AFFECTED( ch, AFF_POISON ) )
      gain = 0;
   return UMIN( gain, ch->max_move - ch->move );
}
void gain_condition( CHAR_DATA * ch, int iCond, int value ) 
{
   int condition;
   ch_ret retcode = rNONE;
   if( IS_NPC( ch ) )
      return;
   
      /*
       * Make away flag go on automagically after 5 mins -- Scion 
       */ 
      if( ( ch->desc ) && ( ( ch->desc->connected == CON_PLAYING ) 
                            &&( ch->desc->idle > 1200 && !xIS_SET( ch->act, PLR_AFK ) ) ) )
      do_afk( ch, "" );
   if( ch->pcdata->condition[iCond] == -1 )
      return;
   if( IS_AFFECTED( ch, AFF_NONLIVING ) )
   {
      ch->pcdata->condition[iCond] = -1;
      return;
   }
   condition = ch->pcdata->condition[iCond];
   ch->pcdata->condition[iCond] += UMIN( value, 10000 );
   ch->pcdata->condition[iCond] = URANGE( 0, ch->pcdata->condition[iCond], 100000 );
   condition -= ch->pcdata->condition[iCond];
   if( ch->pcdata->condition[iCond] == 0 )
      
   {
      switch ( iCond )
         
      {
         case COND_FULL:
            if( ch->curr_talent[TAL_EARTH] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_HUNGRY, ch );
                  send_to_char( "You are STARVING!\n\r", ch );
                  act( AT_HUNGRY, "$n is starved half to death!", ch, NULL, NULL, TO_ROOM );
               }
               lose_ep( ch, 100 );
            }
            break;
         case COND_THIRST:
            if( ch->curr_talent[TAL_WATER] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_THIRSTY, ch );
                  send_to_char( "You are DYING of THIRST!\n\r", ch );
                  act( AT_THIRSTY, "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM );
               }
               lose_ep( ch, 100 );
            }
            break;
         case COND_DRUNK:
            if( condition != 0 )
            {
               set_char_color( AT_SOBER, ch );
               send_to_char( "You are sober.\n\r", ch );
            }
            retcode = rNONE;
            break;
         default:
            bug( "Gain_condition: invalid condition type %d", iCond );
            retcode = rNONE;
            break;
      }
      if( retcode != rNONE )
         return;
   }
   else if( ch->pcdata->condition[iCond] < get_full( ch ) / 5 )
      
   {
      switch ( iCond )
         
      {
         case COND_FULL:
            if( ch->curr_talent[TAL_EARTH] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_HUNGRY, ch );
                  send_to_char( "You are really hungry.\n\r", ch );
                  act( AT_HUNGRY, "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM );
               }
            }
            break;
         case COND_THIRST:
            if( ch->curr_talent[TAL_WATER] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_THIRSTY, ch );
                  send_to_char( "You are really thirsty.\n\r", ch );
                  act( AT_THIRSTY, "$n looks a little parched.", ch, NULL, NULL, TO_ROOM );
               }
            }
            break;
         case COND_DRUNK:
            if( condition != 0 )
            {
               set_char_color( AT_SOBER, ch );
               send_to_char( "You are feeling a little less light headed.\n\r", ch );
            }
            break;
      }
   }
   else if( ch->pcdata->condition[iCond] < get_full( ch ) / 4 )
      
   {
      switch ( iCond )
         
      {
         case COND_FULL:
            if( ch->curr_talent[TAL_EARTH] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_HUNGRY, ch );
                  send_to_char( "You are hungry.\n\r", ch );
               }
            }
            break;
         case COND_THIRST:
            if( ch->curr_talent[TAL_WATER] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_THIRSTY, ch );
                  send_to_char( "You are thirsty.\n\r", ch );
               }
            }
            break;
      }
   }
   
   else if( ch->pcdata->condition[iCond] < get_full( ch ) / 3 )
      
   {
      switch ( iCond )
         
      {
         case COND_FULL:
            if( ch->curr_talent[TAL_EARTH] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_HUNGRY, ch );
                  send_to_char( "You are a mite peckish.\n\r", ch );
               }
            }
            break;
         case COND_THIRST:
            if( ch->curr_talent[TAL_WATER] < 100 )
               
            {
               if( !xIS_SET( ch->act, PLR_NOHUNGER ) )
               {
                  set_char_color( AT_THIRSTY, ch );
                  send_to_char( "You could use a sip of something refreshing.\n\r", ch );
               }
            }
            break;
      }
   }
   return;
}


/* Affects to a char due to temperature etc -keo */ 
   bool climate_affect( CHAR_DATA * ch, int type )
{
   if( !IS_AFFECTED( ch, AFF_COLDBLOOD ) )
   {
      if( type == MAG_FIRE )
      {
         if( IS_SET( ch->immune, RIS_FIRE ) )
            return FALSE;
         if( IS_SET( ch->resistant, RIS_FIRE ) )
            return FALSE;
         if( IS_SET( ch->susceptible, RIS_FIRE ) )
         {
            if( IS_AFFECTED( ch, AFF_UNDEAD ) )
            {
               send_to_char( "Your undead flesh smoulders in the heat.\n\r", ch );
               act( AT_SOCIAL, "$n's undead flesh smoulders in the heat.\n\r", ch, NULL, NULL, TO_ROOM );
            }
            else
            {
               send_to_char( "You sweat profusely in the heat.\n\r", ch );
               act( AT_SOCIAL, "$n is sweating profusely.", ch, NULL, NULL, TO_ROOM );
            }
            direct_damage( ch, ch->speed / 5 );
            return TRUE;
         }
         return FALSE;
      }
      if( type == MAG_COLD )
      {
         if( IS_SET( ch->immune, RIS_COLD ) )
            return FALSE;
         if( IS_SET( ch->resistant, RIS_COLD ) )
            return FALSE;
         if( IS_SET( ch->susceptible, RIS_COLD ) )
         {
            send_to_char( "You shiver from the cold.\n\r", ch );
            act( AT_SOCIAL, "$n is shivering violently.", ch, NULL, NULL, TO_ROOM );
            ch->speed = UMAX( 1, ch->speed - 10 );
            direct_damage( ch, ch->speed / 3 );
            return TRUE;
         }
         return FALSE;
      }
      return FALSE;
   }
   if( type == MAG_FIRE )
   {
      if( IS_SET( ch->immune, RIS_FIRE ) )
         return FALSE;
      if( IS_SET( ch->resistant, RIS_FIRE ) )
      {
         ch->speed++;
         return TRUE;
      }
      send_to_char( "You feel dizzy from the heat.\n\r", ch );
      act( AT_SOCIAL, "$n is suffering from the heat.", ch, NULL, NULL, TO_ROOM );
      if( IS_SET( ch->susceptible, RIS_FIRE ) )
      {
         ch->speed += 10;
         direct_damage( ch, ch->speed / 3 );
         return TRUE;
      }
      ch->speed += 5;
      direct_damage( ch, ( ch->speed - 100 ) / 3 );
      return TRUE;
   }
   if( type == MAG_COLD )
   {
      if( IS_SET( ch->immune, RIS_COLD ) )
         return FALSE;
      if( IS_SET( ch->resistant, RIS_COLD ) )
      {
         ch->speed--;
         return TRUE;
      }
      send_to_char( "You shiver from the cold.\n\r", ch );
      act( AT_SOCIAL, "$n is suffering from the cold.", ch, NULL, NULL, TO_ROOM );
      if( IS_SET( ch->susceptible, RIS_COLD ) )
      {
         ch->speed = UMAX( 1, ch->speed - 20 );
         direct_damage( ch, ( 200 - ch->speed ) / 5 );
         return TRUE;
      }
      ch->speed = UMAX( 1, ch->speed - 10 );
      direct_damage( ch, ( 100 - ch->speed ) / 5 );
      return TRUE;
   }
   return FALSE;
}


/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */ 
void mobile_update( void ) 
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA * ch;
   EXIT_DATA * pexit;
   int door;
   ch_ret retcode;
   retcode = rNONE;
   
      /*
       * Examine all mobs. 
       */ 
      for( ch = last_char; ch; ch = gch_prev )
      
   {
      if( ( !ch->pcdata ) && !( xIS_SET( ch->act, ACT_IS_NPC ) ) )
      {
         bug( "Mob %d in room %d was not set with the NPC flag!  Fixing...", ch->pIndexData->vnum, ch->in_room->vnum );
         xSET_BIT( ch->act, ACT_IS_NPC );
      }
      set_cur_char( ch );
      if( ch == first_char && ch->prev )
      {
         bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
         ch->prev = NULL;
      }
      gch_prev = ch->prev;
      if( gch_prev && gch_prev->next != ch )
         
      {
         sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.", ch->name );
         bug( buf, 0 );
         bug( "Short-cutting here", 0 );
         gch_prev = NULL;
         ch->prev = NULL;
      }
      if( !IS_NPC( ch ) )
         
      {
         drunk_randoms( ch );
         hallucinations( ch );
         continue;
      }
      if( IS_AFFECTED( ch, AFF_CHARM ) )
         continue;
      
         /*
          * Remove mobs in empty zones 
          */ 
         if( IS_NPC( ch ) && ch->pIndexData->vnum == MOB_VNUM_GENERIC 
             &&( !ch->in_room || !ch->in_room->area  ||ch->in_room->area->nplayer == 0 ) )
      {
         extract_char( ch, TRUE );
         continue;
      }
      
         /*
          * Clean up 'animated corpses' that are not charmed' - Scryn 
          */ 
         if( ch->pIndexData->vnum == 5 && !IS_AFFECTED( ch, AFF_CHARM ) )
      {
         if( ch->in_room->first_person )
            act( AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM );
         if( IS_NPC( ch ) ) /* Guard against purging switched? */
            extract_char( ch, TRUE );
         continue;
      }
      
         /*
          * Check for mudprogram script on mob 
          */ 
         if( HAS_PROG( ch->pIndexData, SCRIPT_PROG ) )
         
      {
         mprog_script_trigger( ch );
         continue;
      }
      if( ch != cur_char )
         
      {
         bug( "Mobile_update: ch != cur_char after spec_fun", 0 );
         continue;
      }
      
         /*
          * That's all for sleeping / busy monster 
          */ 
         if( ch->position != POS_STANDING )
      {
         if( ch->position <= POS_KNEELING  &&ch->position > POS_STUNNED )
            interpret( ch, "wake", FALSE );
         if( !IS_AFFECTED( ch, AFF_SLEEP ) )
            ch->position = POS_STANDING;
         continue;
      }
      
         /*
          * Sentinel mobs go home -- Scion 
          */ 
         if( xIS_SET( ch->act, ACT_SENTINEL ) 
             &&ch->home_room != ch->in_room->vnum 
             &&!ch->hating 
             &&!ch->hunting 
             &&!ch->last_hit  &&!IS_AFFECTED( ch, AFF_CURSE )  &&!IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) )
      {
         act( AT_PLAIN, "$n disappears in a swirl of mist.", ch, NULL, NULL, TO_ROOM );
         char_from_room( ch );
         char_to_room( ch, get_room_index( ch->home_room ) );
         act( AT_PLAIN, "$n appears in the room, a ring of mist dissipating around $m.", ch, NULL, NULL, TO_ROOM );
      }
      
         /*
          * hating and hunting stuff -- Scion 
          */ 
         if( ch->hating && !ch->hunting && !ch->fearing )
      {
         CHAR_DATA * hunt_me;
         
            /*
             * Used ch->hating->who as the CHAR_DATA pointer here before, but when characters
             * died, it lost the data and mobs would get corrupted CHAR_DATAs that would crash the
             * mud. This will ensure that before hunting anyone, the CHAR_DATA passed to track_direction
             * is actually a valid player: they can't die between the get_char_world and the strcmp in
             * track_direction anymore! -- Scion 
             */ 
            if( ( hunt_me = get_char_world( ch, ch->hating->name ) ) != NULL )
         {
            if( hunt_me == supermob )
            {
               stop_hating( ch );
               stop_hunting( ch );
            }
            else if( track_direction( ch, hunt_me ) >= 0 );
            start_hunting( ch, hunt_me );
         }
      }
      if( !IS_FIGHTING( ch ) && ch->hunting )
      {
         WAIT_STATE( ch, PULSE_VIOLENCE );
         
            /*
             * Commented out temporarily to avoid spam - Scryn 
             * sprintf( buf, "%s hunting %s from %s.", ch->name,
             * ch->hunting->name,
             * ch->in_room->name );
             * log_string( buf ); 
             */ 
            hunt_victim( ch );
         continue;
      }
      if( xIS_SET( ch->affected_by, AFF_MOUNTED ) )
         
      {
         if( xIS_SET( ch->act, ACT_AGGRESSIVE )  ||xIS_SET( ch->act, ACT_META_AGGR ) )
            do_emote( ch, "snarls and growls." );
         continue;
      }
      if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) 
            &&( xIS_SET( ch->act, ACT_AGGRESSIVE ) || xIS_SET( ch->act, ACT_META_AGGR ) ) )
      {
         do_emote( ch, "glares around and snarls." );
      }
      
         /*
          * something else is wrong if I have to do this ... but until I can
          * track it down, this will have to do - shogar 
          */ 
         if( !ch->in_room->area )
         
      {
         bug( "Room %d for mob %s is not associated with an area?", ch->in_room->vnum, ch->name );
         if( ch->was_in_room )
            bug( "Was in room %d", ch->was_in_room->vnum );
         extract_char( ch, TRUE );
         continue;
      }
      
         /*
          * MOBprogram random trigger 
          */ 
         if( ch->in_room->area->nplayer > 0 )
         
      {
         mprog_random_trigger( ch );
         if( char_died( ch ) )
            continue;
         if( ch->position < POS_STANDING )
            continue;
      }
      
         /*
          * MOBprogram hour trigger: do something for an hour 
          */ 
         mprog_hour_trigger( ch );
      if( char_died( ch ) )
         continue;
      rprog_hour_trigger( ch );
      if( char_died( ch ) )
         continue;
      if( ch->position < POS_STANDING || IS_FIGHTING( ch ) )
         continue;
      
         /*
          * Wander 
          */ 
         if( !xIS_SET( ch->act, ACT_RUNNING ) 
             &&!xIS_SET( ch->act, ACT_SENTINEL ) 
             &&!xIS_SET( ch->act, ACT_PROTOTYPE 
                         &&!IS_FIGHTING( ch ) ) 
             &&!IS_AFFECTED( ch, AFF_MOUNTED ) 
             &&( door = number_range( 0, 15 ) ) <= 9 
             &&( pexit = get_exit( ch->in_room, door ) ) != NULL 
             &&pexit->to_room 
             &&!IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) 
             &&( !xIS_SET( ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area ) )
         
      {
         
            /*
             * perhaps make them hunt here? 
             */ 
            if( IS_SET( ch->mood, MOOD_READY ) )
         {
            REMOVE_BIT( ch->mood, MOOD_READY );
            SET_BIT( ch->mood, MOOD_RELAXED );
            act( AT_ACTION, "$n seems to relax a bit.", ch, NULL, NULL, TO_ROOM );
         }
         if( ( !IS_AFFECTED( ch, AFF_FLYING ) && !IS_AFFECTED( ch, AFF_FLOATING ) ) 
               &&( ( pexit->to_room->sector_type == SECT_WATER_NOSWIM ) 
                   ||( pexit->to_room->sector_type == SECT_WATER_SWIM ) 
                   ||IS_SET( pexit->to_room->room_flags, ROOM_NOFLOOR ) ) )
            continue;  /* Don't go falling into things! -- Scion */
         if( ( pexit->to_room->sector_type == SECT_WATER_NOSWIM 
                 ||pexit->to_room->sector_type == SECT_WATER_SWIM 
                 ||pexit->to_room->sector_type == SECT_UNDERWATER 
                 ||pexit->to_room->sector_type == SECT_OCEANFLOOR ) 
               &&!IS_AFFECTED( ch, AFF_AQUA_BREATH )  &&!IS_AFFECTED( ch, AFF_AQUATIC ) )
            continue;  /* Don't wander into water if you can't swim... -Keolah */
         if( !pexit->to_room->sector_type == SECT_WATER_NOSWIM 
               &&!pexit->to_room->sector_type == SECT_WATER_SWIM 
               &&!pexit->to_room->sector_type == SECT_UNDERWATER 
               &&!pexit->to_room->sector_type == SECT_OCEANFLOOR 
               &&!IS_AFFECTED( ch, AFF_AQUA_BREATH )  &&IS_AFFECTED( ch, AFF_AQUATIC ) )
            continue;  /* Don't wander OUT of water if you breathe air... -Keolah */
         if( pexit->to_room->sector_type == SECT_LAVA  &&!IS_SET( ch->immune, RIS_FIRE ) )
            continue;  /* Or lava :P -Keolah */
         if( ch->master )   /* Don't wander off if you're following someone */
            continue;
         if( ( IS_SET( pexit->exit_info, EX_CLOSED ) ) )
         {
            do_open( ch, dir_name[pexit->vdir] ); /* Open doors, it ain't hard -- Scion */
            retcode = move_char( ch, pexit, FALSE );
            if( char_died( ch ) )
               continue;  /* Moving can kill chars -- Scion */
            do_close( ch, rev_dir_name[pexit->vdir] );  /* Close the door behind ya */
         }
         else
            retcode = move_char( ch, pexit, FALSE );
         if( char_died( ch ) )
            continue;
      }
      
         /*
          * Flee 
          */ 
         if( ch->hit < ch->max_hit / 2 
             &&IS_FIGHTING( ch ) 
             &&( door = number_bits( 4 ) ) <= 9 
             &&( pexit = get_exit( ch->in_room, door ) ) != NULL 
             &&pexit->to_room 
             &&!IS_SET( pexit->exit_info, EX_CLOSED )  &&!IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
         
      {
         retcode = move_char( ch, pexit, 0 );
      }
      
         /*
          * Rain un-buries objects -- Scion *whistles innocently* 
          */ 
         if( ch->in_room && ch->in_room->area->weather->precip > 10  &&!IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
      {
         OBJ_DATA * pobj;
         for( pobj = ch->in_room->first_content; pobj; pobj = pobj->next_content )
            if( number_range( 0, 4 ) <= 2 && xIS_SET( pobj->extra_flags, ITEM_BURIED ) )
            {
               xREMOVE_BIT( pobj->extra_flags, ITEM_BURIED );
               act( AT_CYAN, "The rain uncovers $p, which was buried here!", ch, pobj, NULL, TO_ROOM );
            }
      }
      
         /*
          * Handle objects in inventory -- Scion 
          */ 
         if( ch->last_carrying && !xIS_SET( ch->act, ACT_PACIFIST )  &&IS_SET( ch->xflags, PART_HANDS ) )
      {
         char keyword[MAX_STRING_LENGTH];
         OBJ_DATA * obj;
         bool HaveObj = FALSE;
         bool noDrop = FALSE;
         obj = ch->last_carrying;
         while( obj && obj->wear_loc == WEAR_NONE )
            obj = obj->prev_content;
         if( obj )
            HaveObj = TRUE;
         if( HaveObj )
         {
            one_argument( obj->name, keyword );
            switch ( obj->item_type )
            {
               case ITEM_LIGHT:
               case ITEM_WAND:
               case ITEM_STAFF:
               case ITEM_RADIO:
               case ITEM_ARMOR:
               case ITEM_WORN:
               case ITEM_CONTAINER:
               case ITEM_QUIVER:
                  if( !xIS_EMPTY( obj->parts ) )
                     do_wear( ch, keyword );
                  
                  else
                     do_wield( ch, keyword );
                  break;
               case ITEM_MISSILE_WEAPON:
               case ITEM_WEAPON:
                  do_wield( ch, keyword );
               case ITEM_SCROLL:
               case ITEM_SALVE:
               case ITEM_SCANNER:
               case ITEM_TREASURE:
               case ITEM_MONEY:
               case ITEM_BOAT:
               case ITEM_PROJECTILE:
               case ITEM_SHOVEL:
               case ITEM_KEY:
               case ITEM_LOCK:
                  noDrop = TRUE;
                  break;
               case ITEM_PIPE:
                  if( number_percent(  ) < 40 )
                  {
                     if( obj->value[1] > 0 )
                     {
                        if( obj->value[3] == 0 )
                           do_tamp( ch, keyword );
                        
                        else if( obj->value[3] == 2 )
                           do_light( ch, keyword );
                        
                        else if( obj->value[3] > 2 )
                           do_smoke( ch, keyword );
                     }
                     else if( obj->value[1] == 0 )
                     {
                        OBJ_DATA * herb;
                        for( herb = ch->last_carrying; herb; herb = herb->prev_content )
                        {
                           if( herb->item_type == ITEM_HERB )
                              break;
                        }
                        if( herb != NULL )
                        {
                           if( obj->value[1] == 0 && obj->value[3] > 0 )
                              do_empty( ch, keyword );
                           
                           else
                           {
                              sprintf( buf, "%s %s", keyword, herb->name );
                              do_fill( ch, buf );
                           }
                        }
                        else if( obj->value[3] > 0 )
                           do_empty( ch, keyword );
                     }
                  }
                  noDrop = TRUE;
                  break;
               case ITEM_HERB:
                  if( number_percent(  ) < 40 )
                  {
                     OBJ_DATA * pipe;
                     for( pipe = ch->last_carrying; pipe; pipe = pipe->prev_content )
                     {
                        if( pipe->item_type == ITEM_PIPE )
                           break;
                     }
                     if( pipe != NULL )
                     {
                        one_argument( pipe->name, keyword );
                        if( pipe->value[1] == 0 && pipe->value[3] > 0 )
                           do_empty( ch, keyword );
                        
                        else
                        {
                           sprintf( buf, "%s %s", keyword, obj->name );
                           if( pipe->value[0] == pipe->value[1] )
                           {
                              do_drop( ch, keyword );
                              do_get( ch, keyword );
                           }
                           else
                              do_fill( ch, buf );
                        }
                     }
                  }
                  noDrop = TRUE;
                  break;
               case ITEM_DRINK_CON:
               case ITEM_POTION:
                  if( number_percent(  ) < 20 )
                     do_drink( ch, keyword );
                  noDrop = TRUE;
                  break;
               case ITEM_FOOD:
               case ITEM_PILL:
                  if( number_percent(  ) < 10 )
                     do_eat( ch, keyword );
                  noDrop = TRUE;
                  break;
               case ITEM_CARVINGKNIFE:
                  break;
               default:
                  if( number_percent(  ) < 75 && !xIS_SET( ch->act, ACT_SCAVENGER ) )
                     do_drop( ch, keyword );
                  break;
            }
            if( !noDrop && obj->wear_loc == WEAR_NONE && obj->carried_by == ch && number_percent(  ) < 50 )
            {
               do_wear( ch, "all" );
               do_drop( ch, keyword );
            }
         }
      }
      
         /*
          * Wearing/dropping artifacts could kill the mob -- Scion 
          */ 
         if( char_died( ch ) || ch->in_room == NULL )
         continue;
      
         /*
          * Scavenge 
          */ 
         if( ch->in_room->first_content && IS_SET( ch->xflags, PART_HANDS ) )
         
      {
         OBJ_DATA * obj;
         OBJ_DATA * obj_best;
         char keyword[MAX_STRING_LENGTH];
         if( ( xIS_SET( ch->act, ACT_SCAVENGER ) || number_percent(  ) < 35 )  &&!xIS_SET( ch->act, ACT_PACIFIST ) )
         {
            obj_best = NULL;
            for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
            {
               if( !IS_OBJ_STAT( obj, ITEM_NO_TAKE ) && obj->item_type != ITEM_CORPSE_PC
                    && obj->item_type != ITEM_CORPSE_NPC )
                  one_argument( obj->name, keyword );
               if( ch->last_taken == NULL || !nifty_is_name( keyword, ch->last_taken ) )
                  obj_best = obj;
            }
            if( obj_best != NULL )
            {
               if( IS_OBJ_STAT( obj_best, ITEM_BURIED ) )
               {
                  act( AT_ACTION, "$n digs up $p.", ch, obj_best, NULL, TO_ROOM );
                  xREMOVE_BIT( obj_best->extra_flags, ITEM_BURIED );
               }
               one_argument( obj_best->name, keyword );
               do_get( ch, keyword );
            }
         }
      }
   }
   return;
}
void char_regen( void )
{
   CHAR_DATA * ch;
   for( ch = last_char; ch; ch = gch_prev )
      
   {
      if( ch == first_char && ch->prev )
         
      {
         bug( "char_gains: first_char->prev != NULL... fixed", 0 );
         ch->prev = NULL;
      }
      gch_prev = ch->prev;
      set_cur_char( ch );
      if( gch_prev && gch_prev->next != ch )
         
      {
         bug( "char_gains: ch->prev->next != ch", 0 );
         return;
      }
      if( char_died( ch ) )
         continue;
      ch->hit += hit_gain( ch );
      if( ch->hit > ch->max_hit )
         ch->hit = ch->max_hit;
      if( !IS_NPC( ch ) )
      {
         if( ch->mana < ch->max_mana )
            ch->mana += mana_gain( ch );
         
         else if( ch->mana > ch->max_mana )
         {  /* mana burn */
            act( AT_RED, "Your mana burns you as it courses through your body!", ch, NULL, NULL, TO_CHAR );
            worsen_mental_state( ch, 25 );
            lose_hp( ch, ( ch->mana - ch->max_mana ) );
            if( char_died( ch ) )
               return;
            ch->mana = ch->max_mana;
         }
      }
      if( ch->move < ch->max_move )
      {
         ch->move += move_gain( ch );
         update_pos( ch );
      }
   }
}


/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */ 
void char_update( void )
{
   CHAR_DATA * ch;
   CHAR_DATA * ch_save;
   sh_int save_count = 0;
   ch_save = NULL;
   for( ch = last_char; ch; ch = gch_prev )
   {
      if( ch == first_char && ch->prev )
      {
         bug( "char_update: first_char->prev != NULL... fixed", 0 );
         ch->prev = NULL;
      }
      gch_prev = ch->prev;
      set_cur_char( ch );
      if( gch_prev && gch_prev->next != ch )
      {
         bug( "char_update: ch->prev->next != ch", 0 );
         return;
      }
      if( !IS_NPC( ch ) )
      {
         if( !ch->nation )
            ch->nation = find_nation( ch->species );
         if( !ch->nation )
         {
            bug( "char_update: Could not find nation!" );
            continue;
         }
      }
      
         /*
          * 
          * *  Do a room_prog rand check right off the bat
          * *   if ch disappears (rprog might wax npc's), continue
          */ 
         if( !IS_NPC( ch ) )
         rprog_random_trigger( ch );
      if( char_died( ch ) )
         continue;
      if( IS_NPC( ch ) )
         mprog_time_trigger( ch );
      if( char_died( ch ) )
         continue;
      rprog_time_trigger( ch );
      if( char_died( ch ) )
         continue;
      
         /*
          * 
          * * See if player should be auto-saved.
          */ 
         if( !IS_NPC( ch ) 
             &&( !ch->desc || ch->desc->connected == CON_PLAYING ) 
             &&current_time - ch->save_time > ( sysdata.save_frequency * 60 ) )
         ch_save = ch;
      
      else
         ch_save = NULL;
      
         /*
          * Update hps/mana/blood/moves for pcs 
          */ 
         if( !IS_NPC( ch ) )
      {
         if( !ch->nation )
         {
            ch->nation = find_nation( ch->species );
         }
         if( !ch->nation )
         {
            bug( "char_update: Could not find nation!" );
            continue;
         }
         ch->max_hit = UMAX( 20, ch->nation->hit * get_curr_con( ch ) + ch->base_hit );
         ch->max_mana = UMAX( 20, ch->nation->mana * get_curr_int( ch ) + ch->base_mana );
         ch->max_move = UMAX( 20, 25 * get_curr_end( ch ) + ch->base_move );
      }
      if( ch->position == POS_STUNNED )
         update_pos( ch );
      if( !IS_NPC( ch ) )
         
      {
         OBJ_DATA * obj;
         for( obj = ch->first_carrying; obj; obj = obj->next_content )
         {
            if( obj->item_type == ITEM_LIGHT && obj->value[2] > 0 && !IS_OBJ_STAT( obj, ITEM_GLOW ) )
            {
               if( --obj->value[2] == 0 && ch->in_room )
               {
                  act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
                  act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
                  if( obj->serial == cur_obj )
                     global_objcode = rOBJ_EXPIRED;
                  extract_obj( obj );
               }
            }
         }
         if( ++ch->timer >= 12 )
            
         {
            if( ch->timer == 12 && ch->in_room  &&ch->curr_talent[TAL_VOID] < 50 )
            {
               act( AT_ACTION, "$n disappears into the void.", ch, NULL, NULL, TO_ROOM );
               send_to_char( "You disappear into the void.\n\r", ch );
               if( IS_SET( sysdata.save_flags, SV_IDLE ) )
                  save_char_obj( ch );
               ch->was_in_room = ch->in_room;
               char_from_room( ch );
               char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
            }
         }
      }
      if( !IS_NPC( ch ) && ch->in_room->vnum == 6 && ch->pcdata->release_date <= current_time )
         
      {
         ROOM_INDEX_DATA * location;
         location = NULL;
         
            /*
             * Cleaned up 9/16/99, morts were getting stuck in hell -- Scion 
             */ 
            if( !location )
            location = get_room_index( ch->pcdata->memorize[0] );
         if( !location )
            location = get_room_index( ROOM_VNUM_NEXUS );
         if( !location )
         {
            bug( "Cannot figure out where to release %s to from hell!", ch->name );
            continue;
         }
         MOBtrigger = FALSE;
         char_from_room( ch );
         char_to_room( ch, location );
         send_to_char( "The gods have released you from hell as your sentence is up!\n\r", ch );
         do_look( ch, "auto" );
         STRFREE( ch->pcdata->helled_by );
         ch->pcdata->release_date = 0;
         save_char_obj( ch );
      }
      if( !char_died( ch ) )
         
      {
         
            /*
             * 
             * * Careful with the damages here,
             * *   MUST NOT refer to ch after damage taken, without checking
             * *   return code and/or char_died as it may be lethal damage.
             */ 
            if( IS_AFFECTED( ch, AFF_ROTTING )  &&!IS_SET( ch->immune, RIS_DRAIN ) )
         {
            act( AT_BLOOD, "$n's flesh slowly rots away.", ch, NULL, NULL, TO_ROOM );
            act( AT_BLOOD, "Your flesh slowly rots away.", ch, NULL, NULL, TO_CHAR );
            lose_hp( ch, number_range( 50, 200 ) );
         }
         if( IS_AFFECTED( ch, AFF_POISON )  &&!IS_SET( ch->immune, RIS_POISON )  &&!IS_AFFECTED( ch, AFF_NONLIVING ) )
         {
            act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
            act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = URANGE( 20, ch->mental_state + 4, 100 );
            
               /*
                * this seemed a bit wimpy. lets up the damage shall we? -keo 
                */ 
               lose_hp( ch, number_range( 10, 100 ) );
         }
         if( char_died( ch ) )
            continue;
         if( ch->mental_state >= 30 )
            switch ( ( ch->mental_state + 5 ) / 10 )
               
            {
               case 3:
                  send_to_char( "You feel feverish.\n\r", ch );
                  act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 4:
                  send_to_char( "You do not feel well at all.\n\r", ch );
                  act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 5:
                  send_to_char( "You need help!\n\r", ch );
                  act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 6:
                  send_to_char( "Seekest thou NyQuil.\n\r", ch );
                  act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 7:
                  send_to_char( "You feel reality slipping away...\n\r", ch );
                  act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
                  break;
               case 8:
                  send_to_char( "You begin to understand... everything.\n\r", ch );
                  act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
                  break;
               case 9:
                  send_to_char( "You are ONE with the universe.\n\r", ch );
                  act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL,
                        TO_ROOM );
                  break;
               case 10:
                  send_to_char( "You feel the end is near.\n\r", ch );
                  act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
                  break;
            }
         if( ch->mental_state <= -30 )
            switch ( ( abs( ch->mental_state ) + 5 ) / 10 )
               
            {
               case 10:
                  if( ch->position > POS_SLEEPING )
                     
                  {
                     if( number_percent(  ) + 10 < abs( ch->mental_state ) )
                        do_sleep( ch, "" );
                     
                     else
                        send_to_char( "You're barely conscious.\n\r", ch );
                  }
                  break;
               case 9:
                  if( ch->position > POS_SLEEPING )
                     
                  {
                     if( ( number_percent(  ) + 20 ) < abs( ch->mental_state ) )
                        do_sleep( ch, "" );
                     
                     else
                        send_to_char( "You can barely keep your eyes open.\n\r", ch );
                  }
                  break;
               case 8:
                  if( ch->position > POS_SLEEPING )
                     
                  {
                     if( ch->position < POS_SITTING  &&( number_percent(  ) + 30 ) < abs( ch->mental_state ) )
                        do_sleep( ch, "" );
                     
                     else
                        send_to_char( "You're extremely drowsy.\n\r", ch );
                  }
                  break;
               case 7:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel very unmotivated.\n\r", ch );
                  break;
               case 6:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel sedated.\n\r", ch );
                  break;
               case 5:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel sleepy.\n\r", ch );
                  break;
               case 4:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You feel tired.\n\r", ch );
                  break;
               case 3:
                  if( ch->position > POS_RESTING )
                     send_to_char( "You could use a cup of coffee.\n\r", ch );
                  break;
            }
         if( ch->timer > 24 && ch->curr_talent[TAL_VOID] < 50 )
         {
            char_from_room( ch );
            char_to_room( ch, ch->was_in_room );
            do_quit( ch, "" );
         }
         else
         if( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO )  &&++save_count < 10 ) /* save max of 10 per tick */
            save_char_obj( ch );
      }
   }
   return;
}
void scatter_arti( OBJ_DATA * obj )
{
   ROOM_INDEX_DATA * pRoomIndex;
   for( ;; )
      
   {
      pRoomIndex = get_room_index( number_range( 0, 1048576000 ) );
      if( pRoomIndex )
         if( !IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) 
              &&!IS_SET( pRoomIndex->area->flags, AFLAG_NOTELEPORT )  &&!IS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
            break;
   }
   if( obj->in_room )
      obj_from_room( obj );
   if( obj->carried_by && obj->carried_by->name )
   {
      act( AT_MAGIC, "$p vanishes in a puff of smoke.", obj->carried_by, NULL, NULL, TO_CHAR );
      obj_from_char( obj );
   }
   if( obj->in_obj )
      obj_from_obj( obj );
   obj_to_room( obj, pRoomIndex );
}


/*
 * Update all objs.
 * This function is performance sensitive.
 */ 
void obj_update( void ) 
{
   OBJ_DATA * obj;
   sh_int AT_TEMP;
   int index = 0;
   for( obj = last_object; obj; obj = gobj_prev )
      
   {
      CHAR_DATA * rch;
      char *message;
      index++;
      if( obj == first_object && obj->prev )
         
      {
         bug( "obj_update: first_object->prev != NULL... fixed", 0 );
         obj->prev = NULL;
      }
      if( obj == last_object && obj->next )
      {
         bug( "obj_update: last_object->next != NULL... fixed", 0 );
         obj->next = NULL;
      }
      gobj_prev = obj->prev;
      if( gobj_prev && gobj_prev->next != obj )
         
      {
         bug( "obj_update: obj->prev->next != obj on obj vnum %d", obj->pIndexData->vnum );
         return;
      }
      set_cur_obj( obj );
      if( obj->carried_by )
         oprog_random_trigger( obj );
      
      else
      if( obj->in_room && obj->in_room->area->nplayer > 0 )
         oprog_random_trigger( obj );
      if( obj_extracted( obj ) )
         continue;
      if( obj->item_type == ITEM_PIPE )
         
      {
         if( IS_SET( obj->value[3], PIPE_LIT ) )
            
         {
            if( --obj->value[1] <= 0 )
               
            {
               obj->value[1] = 0;
               REMOVE_BIT( obj->value[3], PIPE_LIT );
            }
            
            else
            if( IS_SET( obj->value[3], PIPE_HOT ) )
               REMOVE_BIT( obj->value[3], PIPE_HOT );
            
            else
               
            {
               if( IS_SET( obj->value[3], PIPE_GOINGOUT ) )
                  
               {
                  REMOVE_BIT( obj->value[3], PIPE_LIT );
                  REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
               }
               
               else
                  SET_BIT( obj->value[3], PIPE_GOINGOUT );
            }
            if( !IS_SET( obj->value[3], PIPE_LIT ) )
               SET_BIT( obj->value[3], PIPE_FULLOFASH );
         }
         
         else
            REMOVE_BIT( obj->value[3], PIPE_HOT );
      }
      if( obj->item_type == ITEM_DRINK_CON )
      {
         obj->value[4]++; /* alcohol aging */
      }
      if( !obj->carried_by && !obj->in_room && !obj->in_obj )
      {
         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
            scatter_arti( obj );
         
         else
            extract_obj( obj );
         continue;
      }
      if( obj->carried_by && !obj->carried_by->name )
      {
         extract_obj( obj );
         continue;
      }
      if( !obj->name || !obj->short_descr )
      {
         bug( "Object %d has no name, extracting...", obj->pIndexData->vnum );
         extract_obj( obj );
         continue;
      }
      
         /*
          * let the item absorb mana -keo 
          */ 
         if( obj->raw_mana > 0 )
      {
         if( obj->material )
            obj->raw_mana -= obj->material->magic;
         
         else
            obj->raw_mana -= 100;
         obj->mana++;
         if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
            obj->mana++;
      }
      
         /*
          * don't let inventory decay 
          */ 
         if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
         continue;
      
         /*
          * groundrot items only decay on the ground 
          */ 
         if( IS_OBJ_STAT( obj, ITEM_GROUNDROT ) && ( obj->carried_by || obj->in_obj ) )
         continue;
      if( ( obj->timer <= 0 || --obj->timer > 0 ) )
         continue;
      
         /*
          * if we get this far, object's timer has expired. 
          */ 
         
         /*
          * artifacts sometimes move at random 
          */ 
         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
      {
         obj->timer = 30;
         scatter_arti( obj );
         continue;
      }
      if( ( obj->value[4] > 0 ) && ( obj->item_type == ITEM_WEAPON ) )
      {
         separate_obj( obj );
         obj->value[4] = 0;
         continue;
      }
      AT_TEMP = AT_PLAIN;
      if( IS_OBJ_STAT( obj, ITEM_EXPLOSIVE ) )
      {
         message = "$p explodes! Shrapnel flies everywhere!";
      }
      else
      {
         switch ( obj->item_type )
            
         {
            default:
               if( IS_OBJ_STAT( obj, ITEM_METAL ) )
                  message = "$p rusts away.";
               
               else if( IS_OBJ_STAT( obj, ITEM_ORGANIC ) )
                  message = "$p rots away.";
               
               else
                  message = "$p wisps into mist and vanishes.";
               AT_TEMP = AT_PLAIN;
               break;
            case ITEM_CONTAINER:
               message = "$p falls apart, tattered from age.";
               AT_TEMP = AT_OBJECT;
               dump_container( obj );
               break;
            case ITEM_PORTAL:
               message = "$p unravels and winks from existence.";
               remove_portal( obj );
               obj->item_type = ITEM_TRASH; /* so extract_obj  */
               AT_TEMP = AT_MAGIC; /* doesn't remove_portal */
               break;
            case ITEM_FOUNTAIN:
               message = "$p dries up.";
               AT_TEMP = AT_BLUE;
               break;
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
               message = "$p decays into dust and blows away.";
               
                  /*
                   * dump all the eq on the ground instead of making is disappear -- Scion 
                   */ 
                  dump_container( obj );
               AT_TEMP = AT_MAGIC;
               break;
            case ITEM_COOK:
            case ITEM_FOOD:
               message = "$p is devoured by a swarm of maggots.";
               AT_TEMP = AT_HUNGRY;
               break;
            case ITEM_BLOOD:
               message = "$p slowly seeps into the ground.";
               AT_TEMP = AT_BLOOD;
               break;
            case ITEM_BLOODSTAIN:
               message = "$p dries up into flakes and blows away.";
               AT_TEMP = AT_BLOOD;
               break;
            case ITEM_SCRAPS:
               message = "$p crumble and decay into nothing.";
               AT_TEMP = AT_OBJECT;
               break;
            case ITEM_FIRE:
               message = "$p burns out.";
               AT_TEMP = AT_FIRE;
         }
      }
      if( obj->carried_by )
         
      {
         act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
         if( IS_OBJ_STAT( obj, ITEM_EXPLOSIVE ) )
         {
            player_echo( obj->carried_by, AT_ACTION, "An explosion echoes throughout the area.", ECHOTAR_AREA );
            lose_hp( obj->carried_by, number_range( obj->weight, obj->weight * 5 ) );
         }
      }
      
      else if( obj->in_room  &&( rch = obj->in_room->first_person ) != NULL  &&!IS_OBJ_STAT( obj, ITEM_BURIED ) )
         
      {
         act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
         act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
         if( IS_OBJ_STAT( obj, ITEM_EXPLOSIVE ) )
         {
            CHAR_DATA * victim;
            CHAR_DATA * nextinroom;
            player_echo( rch, AT_ACTION, "An explosion echoes throughout the area.", ECHOTAR_AREA );
            for( victim = rch->in_room->first_person; victim; victim = nextinroom )
               
            {
               nextinroom = victim->next_in_room;
               if( !char_died( victim ) )   /* prevent nasty double deaths */
                  
               {
                  lose_hp( victim, number_range( obj->weight, obj->weight * 5 ) );
               }
            }
         }
      }
      if( obj->serial == cur_obj )
         global_objcode = rOBJ_EXPIRED;
      extract_obj( obj );
   }
   return;
}


/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */ 
void char_check( void ) 
{
   CHAR_DATA * ch, *ch_next;
   OBJ_DATA * obj;
   EXIT_DATA * pexit;
   static int cnt = 0;
   int door, retcode;
   
      /*
       * This little counter can be used to handle periodic events 
       */ 
      cnt = ( cnt + 1 ) % SECONDS_PER_TICK;
   for( ch = first_char; ch; ch = ch_next )
      
   {
      set_cur_char( ch );
      ch_next = ch->next;
      will_fall( ch, 0 );
      if( char_died( ch ) )
         continue;
      if( IS_NPC( ch ) )
      {
         if( ( cnt & 1 ) )
            continue;
         
            /*
             * running mobs   -Thoric 
             */ 
            if( xIS_SET( ch->act, ACT_RUNNING ) )
         {
            if( ch->spec_fun )
            {
               if( ( *ch->spec_fun ) ( ch ) )
                  continue;
               if( char_died( ch ) )
                  continue;
            }
            if( !xIS_SET( ch->act, ACT_SENTINEL ) 
                  &&!xIS_SET( ch->act, ACT_PROTOTYPE ) 
                  &&( door = number_bits( 4 ) ) <= 9 
                  &&( pexit = get_exit( ch->in_room, door ) ) != NULL 
                  &&pexit->to_room 
                  &&!IS_SET( pexit->exit_info, EX_CLOSED ) 
                  &&!IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) 
                  &&( !xIS_SET( ch->act, ACT_STAY_AREA )  ||pexit->to_room->area == ch->in_room->area ) )
               
            {
               retcode = move_char( ch, pexit, 0 );
               if( char_died( ch ) )
                  continue;
               if( retcode != rNONE || xIS_SET( ch->act, ACT_SENTINEL )  ||ch->position < POS_STANDING )
                  continue;
            }
         }
         continue;
      }
      
         /*
          * Both NPC and PC (Why should only PC's drown?) 
          */ 
         if( ch->mount  &&ch->in_room != ch->mount->in_room )
         
      {
         if( IS_NPC( ch->mount ) )
            xREMOVE_BIT( ch->mount->affected_by, AFF_MOUNTED );
         
         else
            xREMOVE_BIT( ch->mount->pcdata->perm_aff, AFF_MOUNTED );
         ch->mount = NULL;
         ch->position = POS_STANDING;
         send_to_char( "No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r", ch );
      }
      if( IS_AFFECTED( ch, AFF_AQUATIC )  &&!IS_UNDERWATER( ch )  &&!IS_AFFECTED( ch, AFF_AQUA_BREATH )  &&!ch->in_obj )
      {
         int dam;
         dam = number_range( ch->max_hit / 100, ch->max_hit / 50 );
         dam = UMAX( 1, dam );
         act( AT_DANGER, "You gasp for water and flop around!", ch, NULL, NULL, TO_CHAR );
         act( AT_DANGER, "$n gasps for water and flop around!", ch, NULL, NULL, TO_ROOM );
         lose_hp( ch, dam );
      }
      if( char_died( ch ) )
         continue;
      if( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER ) 
            ||( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
         
      {
         if( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) 
              &&!IS_AFFECTED( ch, AFF_AQUATIC )  &&!( ch->in_obj && IS_SET( ch->in_obj->value[2], VEHICLE_AIRTIGHT ) ) )
         {
            int dam;
            
               /*
                * Changed amount of damage at Brittany's request. -- Narn 
                */ 
               dam = number_range( ch->max_hit / 100, ch->max_hit / 50 );
            dam = UMAX( 1, dam );
            if( IS_AFFECTED( ch, AFF_CONSTRUCT ) )
            {
               act( AT_DANGER, "Your circuitry sparks as water seeps into your body!", ch, NULL, NULL, TO_CHAR );
               act( AT_DANGER, "$n's circuitry sparks as water seeps into $s body!", ch, NULL, NULL, TO_ROOM );
            }
            else
            {
               act( AT_DANGER, "You cough and choke as you try to breathe water!", ch, NULL, NULL, TO_CHAR );
               act( AT_DANGER, "$n coughs and chokes as $e tries to breathe water!", ch, NULL, NULL, TO_ROOM );
            }
            lose_hp( ch, dam );
         }
      }
      if( char_died( ch ) )
         continue;
      if( ch->in_room  &&( ch->in_room->sector_type == SECT_WATER_NOSWIM ) )
         
      {
         if( !IS_AFFECTED( ch, AFF_FLYING ) 
              &&!IS_AFFECTED( ch, AFF_AQUA_BREATH ) 
              &&!IS_AFFECTED( ch, AFF_AQUATIC ) 
              &&!ch->mount 
              &&( ch->in_room->water > ch->height - 20 ) 
              &&!( cnt & 1 ) 
              &&!( IS_OUTSIDE( ch ) && ( ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit < 3 ) ) )
            
         {
            for( obj = ch->first_carrying; obj; obj = obj->next_content )
               if( obj->item_type == ITEM_BOAT )
                  break;
            if( !obj )
            {
               int mov;
               int dam;
               if( ch->move > 0 )
               {
                  mov = number_range( ch->max_move / 20, ch->max_move / 5 );
                  mov = UMAX( 1, mov );
                  if( ch->move - mov < 0 )
                     ch->move = 0;
                  
                  else
                  {
                     ch->move -= mov;
                     if( ch->move < ch->max_move / 2 )
                     {
                        act( AT_PLAIN, "You are starting to get tired of swimming.", ch, NULL, NULL, TO_CHAR );
                        act( AT_PLAIN, "$n starts to look a little tired from swimming.", ch, NULL, NULL, TO_ROOM );
                     }
                  }
               }
               
               else
                  
               {
                  dam = number_range( ch->max_hit / 20, ch->max_hit / 5 );
                  dam = UMAX( 1, dam );
                  act( AT_RED, "Struggling with exhaustion, you choke on a mouthful of water.", ch, NULL, NULL, TO_CHAR );
                  act( AT_RED, "$n seems to be choking on the water!", ch, NULL, NULL, TO_ROOM );
                  lose_hp( ch, dam );
               }
            }
         }
      }
   }
}


/* Mob attack update
 *
 * Altered for RW 2.0 to let mobs attack the last person
 * who hit them. Jan 3 2001 Keolah
 */ 
void aggr_update( void ) 
{
   DESCRIPTOR_DATA * d, *dnext;
   CHAR_DATA * wch;
   CHAR_DATA * ch;
   CHAR_DATA * ch_next;
   struct act_prog_data *apdtmp;
   
      /*
       * check mobprog act queue 
       */ 
      while( ( apdtmp = mob_act_list ) != NULL )
      
   {
      wch = mob_act_list->vo;
      if( !char_died( wch ) && wch->mpactnum > 0 )
         
      {
         MPROG_ACT_LIST * tmp_act;
         while( ( tmp_act = wch->mpact ) != NULL )
            
         {
            if( tmp_act->obj && obj_extracted( tmp_act->obj ) )
               tmp_act->obj = NULL;
            if( tmp_act->ch && !char_died( tmp_act->ch ) )
               mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG );
            wch->mpact = tmp_act->next;
            DISPOSE( tmp_act->buf );
            DISPOSE( tmp_act );
         }
         wch->mpactnum = 0;
         wch->mpact = NULL;
      }
      mob_act_list = apdtmp->next;
      DISPOSE( apdtmp );
   }
   
      /*
       * 
       * * Just check descriptors here for victims to aggressive mobs
       * * We can check for linkdead victims in char_check -Thoric
       */ 
      for( d = first_descriptor; d; d = dnext )
      
   {
      dnext = d->next;
      if( d->connected != CON_PLAYING || ( wch = d->character ) == NULL )
         continue;
      if( char_died( wch )  ||IS_NPC( wch )  ||!wch->in_room )
         continue;
      for( ch = wch->in_room->first_person; ch; ch = ch_next )
         
      {
         ch_next = ch->next_in_room;
         if( !IS_AWAKE( ch ) )
            continue;
         if( char_died( ch ) )
            continue;
         if( !IS_FIGHTING( ch ) )
            continue;
         
            /*
             * if not ready for an attack, be surprised 
             */ 
            if( IS_SET( ch->mood, MOOD_RELAXED ) )
         {
            REMOVE_BIT( ch->mood, MOOD_RELAXED );
            SET_BIT( ch->mood, MOOD_READY );
            start_hating( ch, ch->last_hit );
            if( IS_NPC( ch ) )
            {
               xREMOVE_BIT( ch->act, ACT_MAG_FAIL );
               xREMOVE_BIT( ch->act, ACT_PHYS_FAIL );
            }
            act( AT_ACTION, "$n jumps in surprise, and springs to readiness.", ch, NULL, NULL, TO_ROOM );
            act( AT_ACTION, "You jump in surprise, and spring to readiness.", ch, NULL, NULL, TO_CHAR );
            return;
         }
         if( ch->in_room == ch->last_hit->in_room )
         {
            mob_attack( ch, ch->last_hit );
            if( !ch->main_hand )
               mob_attack( ch, ch->last_hit );
            if( !ch->off_hand )
               mob_attack( ch, ch->last_hit );
            rprog_rfight_trigger( ch );
            if( char_died( ch ) )
               continue;
            mprog_hitprcnt_trigger( ch, ch->last_hit );
            if( char_died( ch ) )
               continue;
            mprog_fight_trigger( ch, ch->last_hit );
            if( char_died( ch ) )
               continue;
         }
      }
   }
   return;
}


/* From interp.c */ 
   bool check_social args( ( CHAR_DATA * ch, char *command, char *argument ) );

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */ 
void drunk_randoms( CHAR_DATA * ch ) 
{
   CHAR_DATA * rvch = NULL;
   CHAR_DATA * vch;
   sh_int drunk;
   sh_int position;
   if( IS_NPC( ch ) || ch->pcdata->condition[COND_DRUNK] <= 0 )
      return;
   if( number_percent(  ) < 30 )
      return;
   drunk = ch->pcdata->condition[COND_DRUNK];
   position = ch->position;
   ch->position = POS_STANDING;
   if( number_percent(  ) < ( drunk / 20 ) )
      check_social( ch, "burp", "" );
   
   else
   if( number_percent(  ) < ( drunk / 20 ) )
      check_social( ch, "hiccup", "" );
   
   else
   if( number_percent(  ) < ( drunk / 20 ) )
      check_social( ch, "drool", "" );
   
   else
   if( number_percent(  ) < ( drunk / 20 ) )
      check_social( ch, "fart", "" );
   
   else
   if( drunk > ( 10 + ( get_curr_con( ch ) / 5 ) )  &&number_percent(  ) < ( drunk / 25 ) )
      
   {
      for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
         if( number_percent(  ) < 10 )
            rvch = vch;
      check_social( ch, "puke", ( rvch ? rvch->name : "" ) );
   }
   
   else
   if( drunk > ( 20 + ( get_curr_con( ch ) / 5 ) )  &&number_percent(  ) < ( drunk / 30 ) )
   {
      uncon( ch );
   }
   ch->position = position;
   return;
}


/*
 * Random hallucinations for those suffering from an overly high mentalstate
 * (Hats off to Albert Hoffman's "problem child")	-Thoric
 */ 
void hallucinations( CHAR_DATA * ch ) 
{
   if( ch->mental_state >= 30 && number_bits( 5 - ( ch->mental_state >= 50 ) - ( ch->mental_state >= 75 ) ) == 0 )
      
   {
      char *t;
      switch ( number_range( 1, UMIN( 21, ( ch->mental_state + 5 ) / 5 ) ) )
         
      {
         default:
         case 1:
            t = "You feel very restless... you can't sit still.\n\r";
            break;
         case 2:
            t = "You're tingling all over.\n\r";
            break;
         case 3:
            t = "Your skin is crawling.\n\r";
            break;
         case 4:
            t = "You suddenly feel that something is terribly wrong.\n\r";
            break;
         case 5:
            t = "Those damn little fairies keep laughing at you!\n\r";
            break;
         case 6:
            t = "You can hear your mother crying...\n\r";
            break;
         case 7:
            t = "Have you been here before, or not?  You're not sure...\n\r";
            break;
         case 8:
            t = "Painful childhood memories flash through your mind.\n\r";
            break;
         case 9:
            t = "You hear someone call your name in the distance...\n\r";
            break;
         case 10:
            t = "Your head is pulsating... you can't think straight.\n\r";
            break;
         case 11:
            t = "The ground... seems to be squirming...\n\r";
            break;
         case 12:
            t = "You're not quite sure what is real anymore.\n\r";
            break;
         case 13:
            t = "It's all a dream... or is it?\n\r";
            break;
         case 14:
            t = "You hear your grandchildren praying for you to watch over them.\n\r";
            break;
         case 15:
            t = "They're coming to get you... coming to take you away...\n\r";
            break;
         case 16:
            t = "You begin to feel all powerful!\n\r";
            break;
         case 17:
            t = "You're light as air... the heavens are yours for the taking.\n\r";
            break;
         case 18:
            t = "Your whole life flashes by... and your future...\n\r";
            break;
         case 19:
            t = "You are everywhere and everything... you know all and are all!\n\r";
            break;
         case 20:
            t = "You feel immortal!\n\r";
            break;
         case 21:
            t = "Ahh... the power of a Supreme Entity... what to do...\n\r";
            break;
      }
      send_to_char( t, ch );
   }
   return;
}


/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */ 
void update_handler( void ) 
{
   static int pulse_area;
   static int pulse_mobile;
   static int pulse_violence;
   static int pulse_point;
   static int pulse_second;

static int pulse_houseauc;
   static int pulse_regen;
   static int pulse_time; /* slowing time down -- Scion */
   static int pulse_arena;
   struct timeval stime;
   struct timeval etime;
   if( timechar )
      
   {
      set_char_color( AT_PLAIN, timechar );
      send_to_char( "Starting update timer.\n\r", timechar );
      gettimeofday( &stime, NULL );
   }
   
if ( --pulse_houseauc  <= 0 )
	    {
		pulse_houseauc = 1800 * PULSE_PER_SECOND;
		homebuy_update();
	    }
   if( --pulse_area <= 0 )
      
   {
      pulse_area = number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
      area_update(  );
   }
   if( --pulse_mobile <= 0 )
      
   {
      pulse_mobile = PULSE_MOBILE;
      mobile_update(  );
   }
   if( --pulse_violence <= 0 )
      
   {
      pulse_violence = PULSE_VIOLENCE;
      aggr_update(  );
      violence_update(  );
   }
   if( --pulse_point <= 0 )
      
   {
      pulse_point = number_range( PULSE_TICK * 0.75, PULSE_TICK * 1.25 );
      weather_update(  );
      obj_update(  );
      char_update(  );
   }
   if( --pulse_regen <= 0 )
   {
      pulse_regen = number_range( ( PULSE_TICK * 0.75 / 4 ), ( PULSE_TICK * 1.25 / 4 ) );
      char_regen(  ); /* Sped up regen to 4 times normal, left everything else in char_update -- Scion */
   }
   if( --pulse_second <= 0 )
      
   {
      pulse_second = PULSE_PER_SECOND;
      char_check(  );
      reboot_check( 0 );
   }
   if( --pulse_time <= 0 )  /* 10 minutes */
      
   {
      pulse_time = PULSE_TIME;
      time_update(  );
      save_arenas(  );
      hunger_thirst_update(  );
   }
   
      /*
       * This will run updates for all arenas.
       */ 
      if( --pulse_arena <= 0 )
   {
      pulse_arena = PULSE_TIME / 10;
      arena_update(  );
   }
   mpsleep_update(  );  /* Check for sleeping mud progs -rkb */
   obj_act_update(  );
   room_act_update(  );
   clean_obj_queue(  );   /* dispose of extracted objects */
   clean_char_queue(  );  /* dispose of dead mobs/quitting chars */
   if( timechar )
      
   {
      gettimeofday( &etime, NULL );
      set_char_color( AT_PLAIN, timechar );
      send_to_char( "Update timing complete.\n\r", timechar );
      subtract_times( &etime, &stime );
      ch_printf( timechar, "Timing took %d.%06d seconds.\n\r", etime.tv_sec, etime.tv_usec );
      timechar = NULL;
   }
   tail_chain(  );
   return;
}
void remove_portal( OBJ_DATA * portal ) 
{
   ROOM_INDEX_DATA * fromRoom, *toRoom;
   EXIT_DATA * pexit;
   bool found;
   if( !portal )
      
   {
      bug( "remove_portal: portal is NULL", 0 );
      return;
   }
   fromRoom = portal->in_room;
   found = FALSE;
   if( !fromRoom )
      
   {
      bug( "remove_portal: portal->in_room is NULL", 0 );
      return;
   }
   for( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
      if( IS_SET( pexit->exit_info, EX_PORTAL ) )
         
      {
         found = TRUE;
         break;
      }
   if( !found )
      
   {
      bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );
      return;
   }
   if( pexit->vdir != DIR_PORTAL )
      bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );
   if( ( toRoom = pexit->to_room ) == NULL )
      bug( "remove_portal: toRoom is NULL", 0 );
   extract_exit( fromRoom, pexit );
   return;
}
void reboot_check( time_t reset ) 
{
   static char *tmsg[] = { "You feel the ground shake as the end comes near!", "Lightning crackles in the sky above!",
      "Crashes of thunder sound across the land!", "The sky has suddenly turned midnight black.",
      "You notice the life forms around you slowly dwindling away.", "The seas across the realm have turned frigid.",
      "The aura of magic that surrounds the realms seems slightly unstable.",
      "You sense a change in the magical forces surrounding you." 
   };
   static const int times[] = { 60, 120, 180, 240, 300, 600, 900, 1800 };
   static const int timesize = UMIN( sizeof( times ) / sizeof( *times ), sizeof( tmsg ) / sizeof( *tmsg ) );
   char buf[MAX_STRING_LENGTH];
   static int trun;
   static bool init = FALSE;
   if( !init || reset >= current_time )
      
   {
      for( trun = timesize - 1; trun >= 0; trun-- )
         if( reset >= current_time + times[trun] )
            break;
      init = TRUE;
      return;
   }
   if( ( current_time % 1800 ) == 0 )
      
   {
      sprintf( buf, "%.24s: %d players", ctime( &current_time ), num_descriptors );
      append_to_file( USAGE_FILE, buf );
   }
   if( new_boot_time_t - boot_time < 60 * 60 * 18 && !set_boot_time->manual )
      return;
   if( new_boot_time_t <= current_time )
      
   {
      CHAR_DATA * vch;
      extern bool mud_down;
      echo_to_all( AT_YELLOW, "You are forced from these realms by a strong " 
                     "magical presence\n\ras life here is reconstructed.", ECHOTAR_ALL );
      save_world( NULL );
      log_string( "Automatic Reboot" );
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );
      mud_down = TRUE;
      return;
   }
   if( trun != -1 && new_boot_time_t - current_time <= times[trun] )
      
   {
      echo_to_all( AT_YELLOW, tmsg[trun], ECHOTAR_ALL );
      if( trun <= 5 )
         sysdata.DENY_NEW_PLAYERS = TRUE;
      --trun;
      return;
   }
   return;
}
void subtract_times( struct timeval *etime, struct timeval *stime ) 
{
   etime->tv_sec -= stime->tv_sec;
   etime->tv_usec -= stime->tv_usec;
   while( etime->tv_usec < 0 )
      
   {
      etime->tv_usec += 1000000;
      etime->tv_sec--;
   }
   return;
}


/*
 * Function to update weather vectors according to climate
 * settings, random effects, and neighboring areas.
 * Last modified: July 18, 1997
 * - Fireblade
 */ 
void adjust_vectors( WEATHER_DATA * weather ) 
{
   NEIGHBOR_DATA * neigh;
   double dT, dP, dW, dM;
   if( !weather )
      
   {
      bug( "adjust_vectors: NULL weather data.", 0 );
      return;
   }
   dT = 0;
   dP = 0;
   dW = 0;
   dM = 0;
   
      /*
       * Add in random effects 
       */ 
      dT += number_range( -rand_factor, rand_factor );
   dP += number_range( -rand_factor, rand_factor );
   dW += number_range( -rand_factor, rand_factor );
   dM += number_range( -rand_factor, rand_factor );
   
      /*
       * Add in climate effects
       */ 
      dT += climate_factor * ( ( ( weather->climate_temp - 2 ) * weath_unit ) - ( weather->temp ) ) / weath_unit;
   dP += climate_factor * ( ( ( weather->climate_precip - 2 ) * weath_unit ) - ( weather->precip ) ) / weath_unit;
   dW += climate_factor * ( ( ( weather->climate_wind - 2 ) * weath_unit ) - ( weather->wind ) ) / weath_unit;
   dM += climate_factor * ( ( ( weather->climate_mana - 2 ) * weath_unit ) - ( weather->wind ) ) / weath_unit;
   
      /*
       * Add in effects from neighboring areas 
       */ 
      for( neigh = weather->first_neighbor; neigh; neigh = neigh->next )
      
   {
      
         /*
          * see if we have the area cache'd already 
          */ 
         if( !neigh->address )
         
      {
         
            /*
             * try and find address for area 
             */ 
            neigh->address = get_area( neigh->name );
         
            /*
             * if couldn't find area ditch the neigh 
             */ 
            if( !neigh->address )
            
         {
            NEIGHBOR_DATA * temp;
            bug( "adjust_weather: "  "invalid area name.", 0 );
            temp = neigh->prev;
            UNLINK( neigh, weather->first_neighbor, weather->last_neighbor, next, prev );
            STRFREE( neigh->name );
            DISPOSE( neigh );
            neigh = temp;
            continue;
         }
      }
      dT += ( neigh->address->weather->temp - weather->temp ) / neigh_factor;
      dP += ( neigh->address->weather->precip - weather->precip ) / neigh_factor;
      dW += ( neigh->address->weather->wind - weather->wind ) / neigh_factor;
      dM += ( neigh->address->weather->mana - weather->mana ) / neigh_factor;
   }
   
      /*
       * now apply the effects to the vectors 
       */ 
      weather->temp_vector += ( int )dT;
   weather->precip_vector += ( int )dP;
   weather->wind_vector += ( int )dW;
   weather->mana_vector += ( int )dM;
   
      /*
       * Make sure they are within the right range 
       */ 
      weather->temp_vector = URANGE( -max_vector, weather->temp_vector, max_vector );
   weather->precip_vector = URANGE( -max_vector, weather->precip_vector, max_vector );
   weather->wind_vector = URANGE( -max_vector, weather->wind_vector, max_vector );
   weather->mana_vector = URANGE( -max_vector, weather->mana_vector, max_vector );
   return;
}


/*
 * function updates weather for each area
 * Last Modified: July 31, 1997
 * Fireblade
 */ 
void weather_update(  ) 
{
   AREA_DATA * pArea;
   DESCRIPTOR_DATA * d;
   int limit;
   limit = 3 * weath_unit;
   for( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
      
   {
      
         /*
          * Apply vectors to fields 
          */ 
         pArea->weather->temp += pArea->weather->temp_vector;
      pArea->weather->precip += pArea->weather->precip_vector;
      pArea->weather->wind += pArea->weather->wind_vector;
      pArea->weather->mana += pArea->weather->mana_vector;
      if( IS_SET( sysdata.quest, QUEST_TEMPEST ) )
      {
         pArea->weather->wind += 50;
         pArea->weather->mana += 50;
         pArea->weather->precip += 50;
      }
      
         /*
          * Make sure they are within the proper range 
          */ 
         pArea->weather->temp = URANGE( -limit, pArea->weather->temp, limit );
      pArea->weather->precip = URANGE( -limit, pArea->weather->precip, limit );
      pArea->weather->wind = URANGE( -limit, pArea->weather->wind, limit );
      pArea->weather->mana = URANGE( -limit, pArea->weather->mana, limit );
      
         /*
          * get an appropriate echo for the area 
          */ 
         get_weather_echo( pArea );
   }
   for( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
      
   {
      adjust_vectors( pArea->weather );
   }
   
      /*
       * display the echo strings to the appropriate players 
       */ 
      for( d = first_descriptor; d; d = d->next )
      
   {
      WEATHER_DATA * weath;
      if( d->connected == CON_PLAYING && IS_OUTSIDE( d->character )
            && !NO_WEATHER_SECT( d->character->in_room->sector_type ) && IS_AWAKE( d->character ) )
         
      {
         int i;
         weath = d->character->in_room->area->weather;
         if( !weath->echo )
            continue;
         set_char_color( weath->echo_color, d->character );
         ch_printf( d->character, weath->echo );
         if( ( i = moon_visible( d->character->in_room->area, MOON_BLOOD ) ) 
              &&d->character->in_room->area->weather->precip > 100 )
         {
            send_to_char( "&rThe blood rain pools at your feet.\n\r", d->character );
            make_blood( d->character, i );
         }
      }
   }
   return;
}


/*
 * get weather echo messages according to area weather...
 * stores echo message in weath_data.... must be called before
 * the vectors are adjusted
 * Last Modified: August 10, 1997
 * Fireblade
 */ 
void get_weather_echo( AREA_DATA * tarea )
{
   WEATHER_DATA * weath;
   int n;
   int temp, precip, wind, mana;
   int dT, dP, dW, dM;
   int tindex, pindex, windex, mindex;
   
      /*
       * set echo to be nothing 
       */ 
      weath = tarea->weather;
   weath->echo = NULL;
   weath->echo_color = AT_GREY;
   
      /*
       * get the random number 
       */ 
      n = number_bits( 2 );
   
      /*
       * variables for convenience 
       */ 
      temp = weath->temp;
   precip = weath->precip;
   wind = weath->wind;
   mana = weath->mana;
   dT = weath->temp_vector;
   dP = weath->precip_vector;
   dW = weath->wind_vector;
   dM = weath->mana_vector;
   tindex = ( temp + 3 * weath_unit - 1 ) / weath_unit;
   pindex = ( precip + 3 * weath_unit - 1 ) / weath_unit;
   windex = ( wind + 3 * weath_unit - 1 ) / weath_unit;
   mindex = ( mana + 3 * weath_unit - 1 ) / weath_unit;
   
      /*
       * get the echo string... mainly based on precip 
       */ 
      switch ( pindex )
      
   {
      case 0:
         if( precip - dP > -2 * weath_unit )
            
         {
            char *echo_strings[4] = 
               { "The clouds disappear.\n\r", "The clouds disappear.\n\r", "The sky begins to break through " 
"the clouds.\n\r", "The clouds are slowly "  "evaporating.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         break;
      case 1:
         if( precip - dP <= -2 * weath_unit )
            
         {
            char *echo_strings[4] = 
               { "The sky is getting cloudy.\n\r", "The sky is getting cloudy.\n\r", "Light clouds cast a haze over " 
"the sky.\n\r", "Billows of clouds spread through "  "the sky.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_GREY;
         }
         break;
      case 2:
         if( precip - dP > 0 )
            
         {
            if( tindex > 1 )
               
            {
               if( moon_visible( tarea, MOON_BLOOD ) )
               {
                  char *echo_strings[4] = 
                     { "The blood rain stops.\n\r", "The blood rain tapers off.\n\r", "The blood rain stops falling.\n\r",
                     "The blood rain ceases.\n\r" 
                  };
                  weath->echo_color = AT_BLOOD;
                  weath->echo = echo_strings[n];
               }
               else
               {
                  char *echo_strings[4] = 
                     { "The rain stops.\n\r", "The rain stops.\n\r", "The rainstorm tapers " 
"off.\n\r", "The rain's intensity "  "breaks.\n\r" 
                  };
                  weath->echo_color = AT_CYAN;
                  weath->echo = echo_strings[n];
            } }
            
            else
               
            {
               char *echo_strings[4] = 
                  { "The snow stops.\n\r", "The snow stops.\n\r", "The snow showers taper " 
                  "off.\n\r", "The snow flakes disappear "  "from the sky.\n\r" 
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_WHITE;
         } }
         break;
      case 3:
         if( precip - dP <= 0 )
            
         {
            if( tindex > 1 )
            {
               if( moon_visible( tarea, MOON_BLOOD ) )
               {
                  char *echo_strings[4] = { "It starts to rain blood.\n\r", "A drop of blood falls on you.\n\r",
                     "Blood rain starts to fall.\n\r", "Blood rains from the sky.\n\r" 
                  };
                  weath->echo_color = AT_BLOOD;
                  weath->echo = echo_strings[n];
               }
               else
               {
                  char *echo_strings[4] = 
                     { "It starts to rain.\n\r", "It starts to rain.\n\r", "A droplet of rain falls " 
 "upon you.\n\r", "The rain begins to "  "patter.\n\r" 
                  };
                  weath->echo_color = AT_CYAN;
                  weath->echo = echo_strings[n];
            } }
            
            else
               
            {
               char *echo_strings[4] = 
                  { "It starts to snow.\n\r", "It starts to snow.\n\r", "Crystal flakes begin to " 
                  "fall from the "  "sky.\n\r", "Snow flakes drift down "  "from the clouds.\n\r" 
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_WHITE;
         } }
         
         else if( tindex < 2 && temp - dT > -weath_unit )
            
         {
            char *echo_strings[4] = 
               { "The temperature drops and the rain " 
"becomes a light snow.\n\r", "The temperature drops and the rain " 
"becomes a light snow.\n\r", "Flurries form as the rain freezes.\n\r", "Large snow flakes begin to fall " 
"with the rain.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         
         else if( tindex > 1 && temp - dT <= -weath_unit )
            
         {
            char *echo_strings[4] = 
               { "The snow flurries are gradually " 
"replaced by pockets of rain.\n\r", "The snow flurries are gradually " 
"replaced by pockets of rain.\n\r", "The falling snow turns to a cold drizzle.\n\r",
               "The snow turns to rain as the air warms.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
         }
         break;
      case 4:
         if( precip - dP > 2 * weath_unit )
            
         {
            if( tindex > 1 )
               
            {
               char *echo_strings[4] = 
                  { "The lightning has stopped.\n\r", "The lightning has stopped.\n\r", "The sky settles, and the " 
                  "thunder surrenders.\n\r", "The lightning bursts fade as "  "the storm weakens.\n\r" 
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_GREY;
            }
         }
         
         else if( tindex < 2 && temp - dT > -weath_unit )
            
         {
            char *echo_strings[4] = 
               { "The cold rain turns to snow.\n\r", "The cold rain turns to snow.\n\r", "Snow flakes begin to fall " 
"amidst the rain.\n\r", "The driving rain begins to freeze.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         
         else if( tindex > 1 && temp - dT <= -weath_unit )
            
         {
            char *echo_strings[4] = { "The snow becomes a freezing rain.\n\r", "The snow becomes a freezing rain.\n\r",
               "A cold rain beats down on you "  "as the snow begins to melt.\n\r",
               "The snow is slowly replaced by a heavy "  "rain.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
         }
         break;
      case 5:
         if( precip - dP <= 2 * weath_unit )
            
         {
            if( tindex > 1 )
               
            {
               char *echo_strings[4] = 
                  { "Lightning flashes in the " 
                  "sky.\n\r", "Lightning flashes in the " 
                  "sky.\n\r", "A flash of lightning splits " 
                  "the sky.\n\r", "The sky flashes, and the "  "ground trembles with "  "thunder.\n\r" 
               };
               weath->echo = echo_strings[n];
               weath->echo_color = AT_YELLOW;
            }
         }
         
         else if( tindex > 1 && temp - dT <= -weath_unit )
            
         {
            char *echo_strings[4] = 
               { "The sky rumbles with thunder as " 
"the snow changes to rain.\n\r", "The sky rumbles with thunder as " 
"the snow changes to rain.\n\r", "The falling turns to freezing rain " 
"amidst flashes of "  "lightning.\n\r", "The falling snow begins to melt as "  "thunder crashes overhead.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
         }
         
         else if( tindex < 2 && temp - dT > -weath_unit )
            
         {
            char *echo_strings[4] = 
               { "The lightning stops as the rainstorm " 
"becomes a blinding " 
"blizzard.\n\r", "The lightning stops as the rainstorm " 
"becomes a blinding " 
"blizzard.\n\r", "The thunder dies off as the " 
"pounding rain turns to "  "heavy snow.\n\r", "The cold rain turns to snow and "  "the lightning stops.\n\r" 
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
         }
         break;
      default:
         bug( "echo_weather: invalid precip index" );
         weath->precip = 0;
         break;
   }
   return;
}


/*
 * get echo messages according to time changes...
 * some echoes depend upon the weather so an echo must be
 * found for each area
 * Last Modified: August 10, 1997
 * Fireblade
 */ 
void get_time_echo( WEATHER_DATA * weath )
{
   int n;
   int pindex;
   n = number_bits( 2 );
   pindex = ( weath->precip + 3 * weath_unit - 1 ) / weath_unit;
   weath->echo = NULL;
   weath->echo_color = AT_GREY;
   if( IS_SET( sysdata.quest, QUEST_ETERNAL_NIGHT ) )
   {
      char *echo_strings[4] = { "The sky is pitch black.\n\r", "Eternal night has the realms in its grasp.\n\r",
         "Darkness covers the entire sky.\n\r", "The starless skies lie oppressive above you.\n\r" 
      };
      time_info.sunlight = SUN_DARK;
      weath->echo = echo_strings[n];
      weath->echo_color = AT_DGREY;
   }
   else
   {
      switch ( time_info.hour )
         
      {
         case 5:
            
         {
            char *echo_strings[4] = 
               { "The day has begun.\n\r", "The day has begun.\n\r", "The sky slowly begins to glow.\n\r",
               "The sun slowly embarks upon a new day.\n\r" 
            };
            time_info.sunlight = SUN_RISE;
            weath->echo = echo_strings[n];
            weath->echo_color = AT_YELLOW;
            break;
         }
         case 6:
            
         {
            char *echo_strings[4] = { "The sun rises in the east.\n\r", "The sun rises in the east.\n\r",
               "The hazy sun rises over the horizon.\n\r", "Day breaks as the sun lifts into the sky.\n\r" 
            };
            time_info.sunlight = SUN_LIGHT;
            weath->echo = echo_strings[n];
            weath->echo_color = AT_ORANGE;
            break;
         }
         case 12:
            
         {
            if( pindex > 0 )
               
            {
               weath->echo = "The sun shines directly overhead.\n\r";
            }
            
            else
               
            {
               char *echo_strings[2] = 
                  { "The intensity of the sun " 
                  "heralds the noon hour.\n\r", "The sun's bright rays beat down "  "upon your shoulders.\n\r" 
               };
               weath->echo = echo_strings[n % 2];
            } time_info.sunlight = SUN_LIGHT;
            weath->echo_color = AT_WHITE;
            break;
         }
         case 19:
            
         {
            char *echo_strings[4] = 
               { "The sun slowly disappears in the west.\n\r", "The reddish sun sets past the horizon.\n\r",
               "The sky turns a reddish orange as the sun "  "ends its journey.\n\r",
               "The sun's radiance dims as it sinks in the "  "sky.\n\r" 
            };
            time_info.sunlight = SUN_SET;
            weath->echo = echo_strings[n];
            weath->echo_color = AT_RED;
            break;
         }
         case 20:
            
         {
            if( pindex > 0 )
               
            {
               char *echo_strings[2] = { "The night begins.\n\r", "Twilight descends around you.\n\r" 
               };
               weath->echo = echo_strings[n % 2];
            }
            
            else
               
            {
               char *echo_strings[2] = 
                  { "The moon's gentle glow diffuses " 
                  "through the night sky.\n\r", "The night sky gleams with "  "glittering starlight.\n\r" 
               };
               weath->echo = echo_strings[n % 2];
            } time_info.sunlight = SUN_DARK;
            weath->echo_color = AT_DBLUE;
            break;
         }
      }
   } /* eternal night -keo */
   return;
}


/*
 * update the time
 */ 
void time_update(  ) 
{
   AREA_DATA * pArea;
   DESCRIPTOR_DATA * d;
   WEATHER_DATA * weath;
   MOON_DATA * moon;
   save_world( NULL );
   switch ( ++time_info.hour )
      
   {
      case 5:
      case 6:
      case 12:
      case 19:
      case 20:
         for( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
            
         {
            get_time_echo( pArea->weather );
         }
         for( d = first_descriptor; d; d = d->next )
            
         {
            if( d->connected == CON_PLAYING && IS_OUTSIDE( d->character ) && IS_AWAKE( d->character ) )
               
            {
               weath = d->character->in_room->area->weather;
               if( !weath->echo )
                  continue;
               set_char_color( weath->echo_color, d->character );
               ch_printf( d->character, weath->echo );
            }
         }
         break;
      case 24:
         time_info.hour = 0;
         time_info.day++;
         moon = first_moon;
         while( moon )
         {
            if( moon->phase == PHASE_FULL )
               moon->waning = TRUE;
            if( moon->phase == PHASE_NEW )
               moon->waning = FALSE;
            if( moon->waning )
               moon->phase--;
            
            else
               moon->phase++;
            moon = moon->next;
         }
         break;
   }
   moon = first_moon;
   while( moon )
   {
      if( moon->rise == time_info.hour )
      {
         for( d = first_descriptor; d; d = d->next )
            
         {
            if( d->connected == CON_PLAYING && !str_cmp( d->character->in_room->area->resetmsg, moon->world )
                 && IS_OUTSIDE( d->character ) && IS_AWAKE( d->character ) )
               
            {
               ch_printf( d->character, "^x&CThe %s %s moon slowly rises.\n\r", moon->color, moon_phase[moon->phase] );
            }
         }
         moon->up = TRUE;
      }
      else if( moon->set == time_info.hour )
      {
         for( d = first_descriptor; d; d = d->next )
            
         {
            if( d->connected == CON_PLAYING && !str_cmp( d->character->in_room->area->resetmsg, moon->world )
                 && IS_OUTSIDE( d->character ) && IS_AWAKE( d->character ) )
               
            {
               ch_printf( d->character, "^x&CThe %s %s moon slowly sets.\n\r", moon->color, moon_phase[moon->phase] );
            }
         }
         moon->up = FALSE;
      }
      moon = moon->next;
   }
   if( time_info.day >= 30 )
      
   {
      time_info.day = 0;
      time_info.month++;
   }
   if( time_info.month >= 12 )
      
   {
      time_info.month = 0;
      time_info.year++;
      
         /*
          * A little something added Dec 31 2000
          * * Happy New Year :) - Keolah
          */ 
         for( d = first_descriptor; d; d = d->next )
         
      {
         if( ( d->connected == CON_PLAYING ) && IS_OUTSIDE( d->character ) && IS_AWAKE( d->character ) )
            
         {
            ch_printf( d->character, "^x&RFireworks burst in the sky as people celebrate the coming of the Year %d.\n\r",
                        time_info.year );
         }
      }
   }
   return;
}


/* Moved this to a separate function because I'm slowing down time and players
   get hungry/thirsty far too quickly -- Scion */ 
void hunger_thirst_update(  )
{
   CHAR_DATA * ch;
   DESCRIPTOR_DATA * d;
   for( d = first_descriptor; d; d = d->next )
   {
      ch = d->character;
      if( !ch || !ch->pcdata || d->connected != CON_PLAYING )
         continue;
      if( ch->pcdata->condition[COND_DRUNK] > 8 )
         worsen_mental_state( ch, ch->pcdata->condition[COND_DRUNK] / 8 );
      switch ( ch->position )
         
      {
         case POS_SLEEPING:
            better_mental_state( ch, 20 );
            break;
         case POS_RESTING:
            better_mental_state( ch, 5 );
            break;
         case POS_SITTING:
         case POS_MOUNTED:
            better_mental_state( ch, 2 );
            break;
         case POS_STANDING:
            better_mental_state( ch, 1 );
            break;
            if( number_bits( 2 ) == 0 )
               better_mental_state( ch, 1 );
            break;
      }
      switch ( ch->position )
         
      {
         case POS_SLEEPING:
            better_mental_state( ch, 5 );
            break;
         case POS_RESTING:
            better_mental_state( ch, 3 );
            break;
         case POS_SITTING:
         case POS_MOUNTED:
            better_mental_state( ch, 2 );
            break;
         case POS_STANDING:
            better_mental_state( ch, 1 );
            break;
            if( number_bits( 2 ) == 0 )
               better_mental_state( ch, 1 );
            break;
      }
      if( !ch->nation )
         ch->nation = find_nation( ch->species );
      if( !ch->nation )
      {
         bug( "hunger_thirst_update: could not find nation!", 0 );
         continue;
      }
      ch->exp += 100000;
      ch->pcdata->age_adjust++;
      gain_condition( ch, COND_DRUNK, -( 1 * ch->nation->metabolism ) );
      gain_condition( ch, COND_THIRST, -( 1 * ch->nation->metabolism ) );
      gain_condition( ch, COND_FULL, -( 1 * ch->nation->metabolism ) );
   }
}


