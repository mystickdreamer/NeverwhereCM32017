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
 *			    Battle & death module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
extern char lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *gch_prev;
extern OBJ_DATA *find_projectile( CHAR_DATA * ch, int type );

/* from talent.c */ 
extern void magic_damage( CHAR_DATA * victim, CHAR_DATA * ch, int dam, int type, int talent, bool dont_wait );
extern void travel_teleport( CHAR_DATA * ch );

/* from handler.c */ 
extern void learn_weapon( CHAR_DATA * ch, int i );
extern bool is_bane( OBJ_DATA * weapon, CHAR_DATA * victim );

/*
 * Local functions.
 */ 
int obj_hitroll args( ( OBJ_DATA * obj ) );
void show_condition args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );

/*
 * Check to see if player's attacks are (still?) suppressed
 * #ifdef TRI
 */ 
   bool is_attack_supressed( CHAR_DATA * ch ) 
{
   TIMER * timer;
   if( IS_NPC( ch ) )
      return FALSE;
   timer = get_timerptr( ch, TIMER_ASUPRESSED );
   if( !timer )
      return FALSE;
   
      /*
       * perma-supression -- bard? (can be reset at end of fight, or spell, etc) 
       */ 
      if( timer->value == -1 )
      return TRUE;
   
      /*
       * this is for timed supressions 
       */ 
      if( timer->count >= 1 )
      return TRUE;
   return FALSE;
}


/*
 * Check to see if weapon is poisoned.
 */ 
   bool is_wielding_poisoned( CHAR_DATA * ch ) 
{
   
/*    OBJ_DATA *obj;

    if ( (obj=get_eq_char(ch, WEAR_HAND)) != NULL
    &&    IS_OBJ_STAT(obj, ITEM_POISONED) )
	return TRUE; */ 
      return FALSE;
}


/*
 * hunting, hating and fearing code				-Thoric
 */ 
   bool is_hunting( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !ch->hunting || ch->hunting->who != victim )
      return FALSE;
   return TRUE;
}

bool is_hating( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !ch->hating || ch->hating->who != victim )
      return FALSE;
   return TRUE;
}

bool is_fearing( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !ch->fearing || ch->fearing->who != victim )
      return FALSE;
   return TRUE;
}
void stop_hunting( CHAR_DATA * ch ) 
{
   if( ch->hunting )
      
   {
      STRFREE( ch->hunting->name );
      DISPOSE( ch->hunting );
      ch->hunting = NULL;
   }
   return;
}
void stop_hating( CHAR_DATA * ch ) 
{
   if( ch->hating )
      
   {
      STRFREE( ch->hating->name );
      DISPOSE( ch->hating );
      ch->hating = NULL;
   }
   return;
}
void stop_fearing( CHAR_DATA * ch ) 
{
   if( ch->fearing )
      
   {
      STRFREE( ch->fearing->name );
      DISPOSE( ch->fearing );
      ch->fearing = NULL;
   }
   return;
}
void start_hunting( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !victim->name )
   {
      stop_hunting( ch );
      return;
   }
   if( ch->hunting )
      stop_hunting( ch );
   CREATE( ch->hunting, HHF_DATA, 1 );
   ch->hunting->name = QUICKLINK( victim->name );
   ch->hunting->who = victim;
   return;
}
void start_hating( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !victim->name )
   {
      stop_hunting( ch );
      return;
   }
   if( ch->hating )
      stop_hating( ch );
   CREATE( ch->hating, HHF_DATA, 1 );
   ch->hating->name = QUICKLINK( victim->name );
   ch->hating->who = victim;
   return;
}
void start_fearing( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( ch->fearing )
      stop_fearing( ch );
   CREATE( ch->fearing, HHF_DATA, 1 );
   ch->fearing->name = QUICKLINK( victim->name );
   ch->fearing->who = victim;
   return;
}
int max_fight( CHAR_DATA * ch ) 
{
   return 2 + ( ch->weight / 500 );
}


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 *
 * Note:  This function also handles some non-violence updates.
 */ 
void violence_update( void ) 
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA * ch;
   CHAR_DATA * lst_ch;
   AFFECT_DATA * paf, *paf_next;
   TIMER * timer, *timer_next;
   ch_ret retcode;
   int i;
   SKILLTYPE * skill;
   static int pulse = 0;
   lst_ch = NULL;
   pulse = ( pulse + 1 ) % 100;
   for( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
      
   {
      set_cur_char( ch );
      if( ch == first_char && ch->prev )
         
      {
         bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
         ch->prev = NULL;
      }
      gch_prev = ch->prev;
      if( gch_prev && gch_prev->next != ch )
         
      {
         sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.", ch->name );
         bug( buf, 0 );
         bug( "Short-cutting here", 0 );
         ch->prev = NULL;
         gch_prev = NULL;
      }
      
         /*
          * 
          * * See if we got a pointer to someone who recently died...
          * * if so, either the pointer is bad... or it's a player who
          * * "died", and is back at the healer...
          * * Since he/she's in the char_list, it's likely to be the later...
          * * and should not already be in another fight already
          */ 
         if( char_died( ch ) )
         continue;
      
         /*
          * 
          * * See if we got a pointer to some bad looking data...
          */ 
         if( !ch->in_room || !ch->name )
         
      {
         log_string( "violence_update: bad ch record!  (Shortcutting.)" );
         sprintf( buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d", 
                   ( int )ch, ( int )ch->in_room, ( int )ch->prev, ( int )ch->next );
         log_string( buf );
         log_string( lastplayercmd );
         if( lst_ch )
            sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d", 
                      ( int )lst_ch, ( int )lst_ch->prev, ( int )lst_ch->next );
         
         else
            strcpy( buf, "lst_ch: NULL" );
         log_string( buf );
         gch_prev = NULL;
         continue;
      }
      for( timer = ch->first_timer; timer; timer = timer_next )
         
      {
         timer_next = timer->next;
         if( --timer->count <= 0 )
         {
            if( timer->type == TIMER_DO_FUN )
            {
               int tempsub;
               tempsub = ch->substate;
               ch->substate = timer->value;
               ( timer->do_fun ) ( ch, "" );
               if( char_died( ch ) )
                  break;
               ch->substate = tempsub;
            }
            else if( timer->type == TIMER_PKILLED )
            {
               if( xIS_SET( ch->pcdata->perm_aff, AFF_ETHEREAL ) )
                  if( check_rebirth( ch, 3000 ) )
                     continue;
            }
            extract_timer( ch, timer );
         }
      }
      if( char_died( ch ) )
         continue;
      
         /*
          * 
          * * We need spells that have shorter durations than an hour.
          * * So a melee round sounds good to me... -Thoric
          */ 
         for( paf = ch->first_affect; paf; paf = paf_next )
         
      {
         paf_next = paf->next;
         if( paf->duration > 0 )
            paf->duration--;
         
         else
         if( paf->duration < 0 )
            ;
         
         else
            
         {
            if( !paf_next  ||paf_next->type != paf->type  ||paf_next->duration > 0 )
               
            {
               skill = get_skilltype( paf->type );
               if( paf->type > 0 && skill && skill->msg_off )
                  
               {
                  set_char_color( AT_WEAROFF, ch );
                  send_to_char( skill->msg_off, ch );
                  send_to_char( "\n\r", ch );
                  update_pos( ch );
               }
            }
            affect_remove( ch, paf );
         }
      }
      if( char_died( ch ) )
         continue;
      if( ( IS_AFFECTED( ch, AFF_FLAMING ) ) 
            ||IS_SET( ch->in_room->room_flags, ROOM_BURNING ) 
            ||ch->in_room->sector_type == SECT_LAVA 
            ||( ( IS_VAMPIRE( ch ) ) 
                &&( time_info.hour > 6 && time_info.hour <= 18 ) 
                &&!IS_SET( sysdata.quest, QUEST_ETERNAL_NIGHT ) 
                &&( IS_OUTSIDE( ch ) )  &&( get_light_room( ch->in_room ) > -1 ) ) )
         
      {
         if( ch->in_room->curr_water > 30  &&ch->in_room->sector_type != SECT_LAVA )
         {
            REMOVE_BIT( ch->in_room->room_flags, ROOM_BURNING );
            act( AT_BLUE, "The water puts out the flames covering you.", ch, NULL, NULL, TO_CHAR );
            act( AT_BLUE, "The water puts out the flames covering $n.", ch, NULL, NULL, TO_ROOM );
            xREMOVE_BIT( ch->affected_by, AFF_FLAMING );
         }
         else if( IS_PACIFIST( ch ) )
         {
            REMOVE_BIT( ch->in_room->room_flags, ROOM_BURNING );
            act( AT_BLUE, "$n irritably puts out the fire.", ch, NULL, NULL, TO_ROOM );
            xREMOVE_BIT( ch->affected_by, AFF_FLAMING );
         }
         else if( !IS_SET( ch->immune, RIS_FIRE ) 
                  &&( TALENT( ch, TAL_FIRE ) < 100 
                      ||( IS_SET( ch->resistant, RIS_FIRE ) && IS_AFFECTED( ch, AFF_CONSTRUCT ) ) ) )
         {
            act( AT_FIRE, "The searing flames covering you sear your flesh!", ch, NULL, NULL, TO_CHAR );
            act( AT_FIRE, "The flames covering $n sear $s flesh!", ch, NULL, NULL, TO_ROOM );
            lose_hp( ch, 25 );
            climate_affect( ch, MAG_FIRE );
         }
      }
      if( char_died( ch ) )
         continue;
      if( IS_AFFECTED( ch, AFF_PLAGUE )  &&!IS_SET( ch->immune, RIS_POISON )  &&!IS_UNDEAD( ch ) )
      {
         act( AT_DGREEN, "You convulse in agony as the plague ravages your body.", ch, NULL, NULL, TO_CHAR );
         lose_hp( ch, 100 );
      }
      if( char_died( ch ) )
         continue;
      if( ch->speed < 0 )
         ch->speed = 1;
      
      else if( ch->speed > 100 )
         ch->speed--;
      
      else if( ch->speed < 100 )
         ch->speed++;
      if( ch->in_room->area->weather->temp > 100 )
      {
         if( climate_affect( ch, MAG_FIRE ) )
            if( IS_NPC( ch ) && ch->spec_fun )
               travel_teleport( ch );
      }
      
      else if( ch->in_room->area->weather->temp < -100 )
      {
         if( climate_affect( ch, MAG_COLD ) )
            if( IS_NPC( ch ) && ch->spec_fun )
               travel_teleport( ch );
      }
      if( char_died( ch ) )
         continue;
      if( IS_FIGHTING( ch ) && ch->singing )
      {
         if( ch->mana < 0 )
         {
            send_to_char( "Your voice grows hoarse and you can't sing anymore.\n\r", ch );
            do_sing( ch, "none" );
         }
         else
         {
            switch ( ch->singing )
            {
               case SONG_SPEED:
                  ch->speed += 5;
                  break;
               case SONG_FIRE:
                  learn_talent( ch, TAL_FIRE );
                  magic_damage( ch->last_hit, ch, ch->curr_talent[TAL_SPEECH] + ch->curr_talent[TAL_FIRE], MAG_FIRE,
                                 TAL_SPEECH, TRUE );
                  break;
               case SONG_THUNDER:
                  learn_talent( ch, TAL_LIGHTNING );
                  magic_damage( ch->last_hit, ch, ch->curr_talent[TAL_SPEECH] + ch->curr_talent[TAL_LIGHTNING],
                                 MAG_ELECTRICITY, TAL_SPEECH, TRUE );
                  break;
               case SONG_WIND:
                  learn_talent( ch, TAL_WIND );
                  magic_damage( ch->last_hit, ch, ch->curr_talent[TAL_SPEECH] + ch->curr_talent[TAL_WIND], MAG_WIND,
                                 TAL_SPEECH, TRUE );
                  break;
               case SONG_RIVERS:
                  learn_talent( ch, TAL_WATER );
                  magic_damage( ch->last_hit, ch, ch->curr_talent[TAL_SPEECH] + ch->curr_talent[TAL_WATER], MAG_WATER,
                                 TAL_SPEECH, TRUE );
                  break;
               case SONG_WINTER:
                  learn_talent( ch, TAL_FROST );
                  magic_damage( ch->last_hit, ch, ch->curr_talent[TAL_SPEECH] + ch->curr_talent[TAL_FROST], MAG_COLD,
                                 TAL_SPEECH, TRUE );
                  break;
               case SONG_LIFE:
                  learn_talent( ch, TAL_HEALING );
                  i = number_range( ch->curr_talent[TAL_SPEECH], ch->curr_talent[TAL_SPEECH] * 5 );
                  ch->hit += i;
                  use_magic( ch, TAL_SPEECH, i );
                  act( AT_PINK, "$n's song revitalizes $m.", ch, NULL, NULL, TO_ROOM );
                  act( AT_PINK, "Your song revitalizes you.", ch, NULL, NULL, TO_CHAR );
                  break;
            }
         }
      }
      
         /*
          * check for exits moving players around 
          */ 
         if( ( retcode = pullcheck( ch, pulse ) ) == rCHAR_DIED || char_died( ch ) )
         continue;
      
         /*
          * Examine call for special procedure 
          */ 
         if( IS_NPC( ch ) && !xIS_SET( ch->act, ACT_RUNNING )  &&ch->spec_fun  &&IS_AWAKE( ch ) )
         
      {
         if( ch->last_hit )
         {
            if( !xIS_SET( ch->act, ACT_PHYS_FAIL )  &&( xIS_SET( ch->act, ACT_MAG_FAIL )  ||number_range( 1, 2 ) == 1 ) )
            {
               mob_attack( ch, ch->last_hit );
               continue;
            }
            if( xIS_SET( ch->act, ACT_PHYS_FAIL )  &&xIS_SET( ch->act, ACT_MAG_FAIL ) )
            {
               do_flee( ch, "" );
               continue;
            }
         }
         if( ( *ch->spec_fun ) ( ch ) )
            continue;
         if( char_died( ch ) )
            continue;
      }
   }
   return;
}


/*
 * Do one group of attacks.
 */ 
   ch_ret multi_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt )
{
   int chance;
   int dual_bonus;
   ch_ret retcode;
   OBJ_DATA * obj;
   
      /*
       * add timer to pkillers 
       */ 
      if( !IS_NPC( ch ) && !IS_NPC( victim ) )
      
   {
      add_timer( ch, TIMER_RECENTFIGHT, 11, NULL, 0 );
      add_timer( victim, TIMER_RECENTFIGHT, 11, NULL, 0 );
   }
   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_NOATTACK ) )
      return rNONE;
   
      /*
       * Group protecting -- Scion 
       */ 
      if( victim->protected_by )
   {
      if( victim->protected_by->in_room == victim->in_room )
      {
         if( number_percent(  ) < ( get_curr_dex( victim->protected_by ) + get_curr_int( victim->protected_by ) ) / 2 )
         {
            act( AT_HITME, "You dive in front of $N to protect $M!", victim->protected_by, NULL, victim, TO_CHAR );
            act( AT_HIT, "$n dives in front of $N to protect $M!", victim->protected_by, NULL, victim, TO_NOTVICT );
            act( AT_HIT, "$n dives in front of you to protect you!", victim->protected_by, NULL, victim, TO_VICT );
            victim = victim->protected_by;
         }
      }
      else if( char_died( victim->protected_by ) )
         victim->protected_by = NULL;
   }
   obj = get_eq_char( ch, WEAR_HAND );
   if( get_eq_char( ch, WEAR_HAND2 ) || ( obj && xIS_SET( obj->extra_flags, ITEM_TWO_HANDED ) ) )
      
   {
      dual_bonus = ( ch->pcdata ? ch->pcdata->weapon[0] / 10 : 10 );
      chance = ch->pcdata ? ch->pcdata->weapon[0] : 100;
   }
   
      /*
       * 
       * * Wimp out?
       */ 
      if( IS_NPC( victim ) )
   {
      if( ( xIS_SET( victim->act, ACT_WIMPY ) && number_bits( 1 ) == 0 && victim->hit < victim->max_hit / 2 ) 
           ||( IS_AFFECTED( victim, AFF_CHARM ) && victim->master && victim->master->in_room != victim->in_room ) 
           ||( get_curr_wil( victim ) < 33 )  ||( get_curr_wil( victim ) > 66 && victim->hit < victim->max_hit / 4 ) )
      {
         start_fearing( victim, ch );
         stop_hunting( victim );
         do_flee( victim, "" );
      }
   }
   return retcode;
}


/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */ 
int obj_hitroll( OBJ_DATA * obj ) 
{
   int tohit = 0;
   AFFECT_DATA * paf;
   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      if( paf->location == APPLY_HITROLL )
         tohit += paf->modifier;
   for( paf = obj->first_affect; paf; paf = paf->next )
      if( paf->location == APPLY_HITROLL )
         tohit += paf->modifier;
   return tohit;
}


/*
 * Set position of a victim.
 */ 
void update_pos( CHAR_DATA * victim ) 
{
   if( !victim )
      
   {
      bug( "update_pos: null victim", 0 );
      return;
   }
   if( victim->move < 0 )
   {
      victim->position = POS_STUNNED;
   }
   else
   {
      if( victim->position == POS_STUNNED )
      {
         victim->position = POS_STANDING;
         act( AT_ACTION, "You come to and climb to your feet.", victim, NULL, NULL, TO_CHAR );
         act( AT_ACTION, "$n climbs to $s feet.", victim, NULL, NULL, TO_ROOM );
      }
   }
   if( victim->hit <= -11 )
      
   {
      if( victim->mount )
         
      {
         act( AT_ACTION, "$n falls from $N.", victim, NULL, victim->mount, TO_ROOM );
         if( IS_NPC( victim->mount ) )
            xREMOVE_BIT( victim->mount->affected_by, AFF_MOUNTED );
         
         else
            xREMOVE_BIT( victim->mount->pcdata->perm_aff, AFF_MOUNTED );
         victim->mount = NULL;
      }
      victim->position = POS_DEAD;
      return;
   }
   if( victim->position > POS_STUNNED  &&IS_AFFECTED( victim, AFF_PARALYSIS ) )
      victim->position = POS_STUNNED;
   if( victim->mount )
      
   {
      act( AT_ACTION, "$n falls unconscious from $N.", victim, NULL, victim->mount, TO_ROOM );
      if( IS_NPC( victim->mount ) )
         xREMOVE_BIT( victim->mount->affected_by, AFF_MOUNTED );
      
      else
         xREMOVE_BIT( victim->mount->pcdata->perm_aff, AFF_MOUNTED );
      victim->mount = NULL;
   }
   return;
}


/* This is now mob only, since players can just walk away from combat.
It has been modified accordingly. -- Scion */ 
void do_flee( CHAR_DATA * ch, char *argument ) 
{
   ROOM_INDEX_DATA * was_in;
   ROOM_INDEX_DATA * now_in;
   int attempt;
   sh_int door;
   EXIT_DATA * pexit;
   if( !IS_NPC( ch ) )
   {
      send_to_char( "Just run away with the direction commands! Quickly now!\r\n", ch );
      return;
   }
   if( ch->position <= POS_SLEEPING )
      return;
   if( number_range( 1, 5 ) != 1 )
      return;
   if( ch->spec_fun )
   {
      travel_teleport( ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_BERSERK ) )
   {
      act( AT_FLEE, "$n looks wide eyed and panicked!", ch, NULL, NULL, TO_ROOM );
      return;
   }
   if( ch->move <= 0 )
   {
      act( AT_FLEE, "$n starts to look very tired.", ch, NULL, NULL, TO_ROOM );
      return;
   }
   
      /*
       * Make flee fail for mobs (give players a fighting chance, no pun intended) -- Scion 
       */ 
      if( IS_FIGHTING( ch ) )
   {
      if( number_range( 1, ( get_curr_dex( ch ) + get_curr_lck( ch ) ) / 2 ) 
           <number_range( 1, ( get_curr_str( ch->last_hit )  +get_curr_dex( ch->last_hit ) ) ) )
      {
         act( AT_FLEE, "$n tries to escape, but you manage to stop $m!", ch, NULL, ch->last_hit, TO_VICT );
         act( AT_FLEE, "$n tries to escape, but $N manages to stop $m!", ch, NULL, ch->last_hit, TO_NOTVICT );
         return;
      }
   }
   was_in = ch->in_room;
   for( attempt = 0; attempt < 8; attempt++ )
   {
      door = number_door(  );
      if( ( pexit = get_exit( was_in, door ) ) == NULL 
            ||!pexit->to_room 
            ||IS_SET( pexit->exit_info, EX_NOFLEE ) 
            ||( IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_AFFECTED( ch, AFF_PASS_DOOR ) ) 
            ||IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) 
            ||( IS_SET( pexit->to_room->room_flags, ROOM_NOFLOOR ) && !IS_FLOATING( ch ) ) )
         continue;
      affect_strip( ch, gsn_sneak );
      xREMOVE_BIT( ch->affected_by, AFF_SNEAK );
      move_char( ch, pexit, 0 );
      if( ( now_in = ch->in_room ) == was_in )
         continue;
      ch->in_room = was_in;
      act( AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );
      ch->in_room = now_in;
      act( AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );
      stop_hunting( ch );
      if( ch->last_hit )
         start_fearing( ch, ch->last_hit );
      return;
   }
   return;
}


