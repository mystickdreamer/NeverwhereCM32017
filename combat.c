/*
 *      Combat
 *      =======
 *      Abilities and handling
 *      Written by Keolah for Rogue Winds 2.0
 *
 *      You may not use or distribute any of this code without the
 *      explicit permission of the code maintainer, Heather Dunn (Keolah)
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
#include <strings.h>
#endif
#include <time.h>
#include "mud.h"

/* from birth.c */
void ghost( CHAR_DATA * ch );

/* From interp.c */
bool check_social( CHAR_DATA * ch, char *command, char *argument );

/* from handler.c */
extern bool is_bane( OBJ_DATA * weapon, CHAR_DATA * victim );

/* from save.c */
extern void save_world( CHAR_DATA * ch );

/* from talent.c */
extern void player_echo( CHAR_DATA * ch, sh_int AT_COLOR, char *argument, sh_int tar );

void die( CHAR_DATA * ch )
{
   char log_buf[MAX_STRING_LENGTH];

   if( char_died( ch ) )
      return;

   if( !IS_NPC( ch ) && ch->last_hit && !IS_NPC( ch->last_hit )
       && ch->last_hit != ch && ( IS_SET( ch->last_hit->pcdata->flags, PCFLAG_SPAR ) || !ch->desc ) )
   {
      act( AT_ACTION, "$N has defeated you in a spar.", ch, NULL, ch->last_hit, TO_CHAR );
      act( AT_ACTION, "You have defeated $n in a spar.", ch, NULL, ch->last_hit, TO_VICT );
      xSET_BIT( ch->act, PLR_DEFENSE );
      xSET_BIT( ch->last_hit->act, PLR_DEFENSE );
      ch->last_hit->pcdata->pkills++;
      ch->pcdata->pdeaths++;
      ch->hit = 10;
      return;
   }

   switch ( number_range( 1, 5 ) )
   {
      default:
         act( AT_DEAD, "$n dies.", ch, NULL, NULL, TO_ROOM );
         break;
      case 1:
         act( AT_DEAD, "$n collapses motionless to the ground.", ch, NULL, NULL, TO_ROOM );
         break;
      case 2:
         act( AT_DEAD, "$n is DEAD!!", ch, NULL, NULL, TO_ROOM );
         break;
      case 3:
         act( AT_DEAD, "$n jerks limply, then is still.", ch, NULL, NULL, TO_ROOM );
         break;
      case 4:
         act( AT_DEAD, "$n slumps over lifelessly.", ch, NULL, NULL, TO_ROOM );
         break;
   }

   if( ch->last_hit )
   {
      mprog_death_trigger( ch->last_hit, ch );
      if( char_died( ch ) )
         return;

      rprog_death_trigger( ch->last_hit, ch );
      if( char_died( ch ) )
         return;
   }

   if( !IS_NEWBIE( ch ) )
      make_corpse( ch );

   if( ch->last_hit && !IS_NPC( ch->last_hit ) && ch->in_room == ch->last_hit->in_room )
   {
      act( AT_DEAD, "You have slain $N.", ch->last_hit, NULL, ch, TO_CHAR );
      if( xIS_SET( ch->last_hit->act, PLR_AUTOGOLD ) )
         do_get( ch->last_hit, "coins corpse" );
      if( xIS_SET( ch->last_hit->act, PLR_AUTOLOOT ) )
         do_get( ch->last_hit, "all corpse" );
      if( !IS_AFFECTED( ch, AFF_NONLIVING ) && IS_SET( ch->last_hit->pcdata->flags, PCFLAG_VAMPIRE ) )
      {
         act( AT_BLOOD, "You drain the last blood from $N.", ch->last_hit, NULL, ch, TO_CHAR );
         ch->last_hit->hit += get_curr_con( ch );
      }
   }

   save_world( ch );

   if( IS_NPC( ch ) )
   {
      if( ch->last_hit )
      {
         if( !IS_NPC( ch->last_hit ) )
         {
            ch->last_hit->pcdata->mkills++;
         }
         else
         {
            int i;

            for( i = get_exp_worth( ch ); i > 10000; i -= 10000 )
            {
               if( i > 100000 )
               {
                  switch ( number_range( 1, 7 ) )
                  {
                     case 1:
                        ch->last_hit->perm_str++;
                        break;
                     case 2:
                        ch->last_hit->perm_int++;
                        break;
                     case 3:
                        ch->last_hit->perm_wil++;
                        break;
                     case 4:
                        ch->last_hit->perm_con++;
                        break;
                     case 5:
                        ch->last_hit->perm_dex++;
                        break;
                     case 6:
                        ch->last_hit->perm_per++;
                        break;
                     case 7:
                        ch->last_hit->perm_lck++;
                        break;
                  }
                  i -= 90000;
                  continue;
               }
               ch->last_hit->max_hit++;
            }
         }
      }
      ch->pIndexData->killed++;
      extract_char( ch, TRUE );
      ch = NULL;
      return;
   }

   if( ch->last_hit )
   {
      sprintf( log_buf, "%s Pkill: %s killed by %s in %s (%d)",
               !IS_NPC( ch->last_hit ) ?
               ( IS_SET( ch->in_room->area->flags, AFLAG_FREEKILL ) ?
                 "Legal" : "&RILLEGAL" ) : "Mob",
               ch->name,
               ( IS_NPC( ch->last_hit )
                 ? ch->last_hit->short_descr : ch->last_hit->name ), ch->in_room->name, ch->in_room->vnum );
      log_string( log_buf );
      if( IS_NPC( ch->last_hit ) )
      {
         stop_hating( ch->last_hit );
         ch->pcdata->mdeaths++;
      }
      else
      {
         if( xIS_SET( ch->act, PLR_BOUNTY ) && ch->pcdata->bounty > 0 )
         {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "&YYou receive %d coins from the bounty!&w\r\n", ch->pcdata->bounty );
            send_to_char( buf, ch->last_hit );
            ch->last_hit->gold += ch->pcdata->bounty;
            ch->pcdata->bounty = 0;
            xREMOVE_BIT( ch->act, PLR_BOUNTY );
         }

         ch->last_hit->pcdata->rkills++;
         ch->pcdata->rdeaths++;
      }
   }
   else
   {
      sprintf( log_buf, "%s (%d) died in %s (%d)", ch->name, get_char_worth( ch ), ch->in_room->name, ch->in_room->vnum );
      log_string( log_buf );
   }

   /*
    * must be called AFTER triggers 
    */
   stop_fighting( ch, TRUE );

   ghost( ch );
}

void uncon( CHAR_DATA * ch )
{
   act( AT_HURT, "$n loses consciousness!", ch, NULL, NULL, TO_ROOM );
   act( AT_DANGER, "You lose consciousness!", ch, NULL, NULL, TO_CHAR );
   ch->position = POS_STUNNED;
}

void pain( CHAR_DATA * ch, int dam )
{
   char msg1[MAX_STRING_LENGTH];
   char msg2[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA *was_in_room;
   EXIT_DATA *pexit;
   int pain;

   if( !ch->in_room || IS_SILENT( ch ) || IS_AFFECTED( ch, AFF_NONLIVING ) )
      return;

   pain = ( ch->hit ) / dam;
   msg1[0] = '\0';
   msg2[0] = '\0';

   switch ( pain )
   {
      case 8:
         sprintf( msg1, "$n grunts in pain." );
         sprintf( msg2, "Something grunts in pain." );
         break;
      case 7:
         sprintf( msg1, "$n winces in pain." );
         break;
      case 6:
         sprintf( msg1, "$n cries out in pain!" );
         sprintf( msg2, "Something cries out in pain!" );
         break;
      case 5:
         sprintf( msg1, "$n screams loudly!" );
         sprintf( msg2, "Something screams loudly!" );
         break;
      case 4:
         sprintf( msg1, "$n groans loudly." );
         sprintf( msg2, "Something groans somewhere." );
         break;
      case 3:
         sprintf( msg1, "$n moans pitifully." );
      case 2:
         sprintf( msg1, "$n whimpers feebly." );
         break;
   }

   if( msg1[0] != '\0' )
      act( AT_BLOOD, msg1, ch, NULL, NULL, TO_ROOM );

   if( msg2[0] == '\0' )
      return;
   was_in_room = ch->in_room;
   for( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( pexit->to_room && pexit->to_room != was_in_room )
      {
         ch->in_room = pexit->to_room;
         act( AT_ACTION, msg2, ch, NULL, NULL, TO_ROOM );
      }
   }
   ch->in_room = was_in_room;
}

bool can_hit( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
   if( !obj || !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      if( ( IS_AFFECTED( victim, AFF_ETHEREAL )
            && !IS_AFFECTED( ch, AFF_ETHEREAL ) )
          || ( IS_AFFECTED( ch, AFF_ETHEREAL ) && !IS_AFFECTED( victim, AFF_ETHEREAL ) ) || !IS_SAME_PLANE( ch, victim ) )
      {
         act( AT_ACTION, "Your attack passes through $N!", ch, NULL, victim, TO_CHAR );
         act( AT_ACTION, "$n's attack passes through you!", ch, NULL, victim, TO_VICT );
         act( AT_ACTION, "$n's attack passes through $N!", ch, NULL, victim, TO_NOTVICT );
         if( IS_NPC( ch ) && number_range( 1, 40 ) < get_curr_int( ch ) )
         {
            if( IS_SET( ch->xflags, PART_HANDS ) )
               act( AT_SOCIAL, "$n gasps in surprise.", ch, NULL, NULL, TO_ROOM );
            else
               act( AT_SOCIAL, "$n roars in anger.", ch, NULL, NULL, TO_ROOM );
            xSET_BIT( ch->act, ACT_PHYS_FAIL );
         }
         return FALSE;
      }
   return TRUE;
}

bool can_gain_exp( CHAR_DATA * ch, CHAR_DATA * victim )
{

   if( !ch )
      return FALSE;
   if( !victim )
      return FALSE;
   if( ch == victim )
      return FALSE;
   if( IS_AFFECTED( victim, AFF_SLEEP ) )
      return FALSE;
   if( victim->spec_fun )
      return TRUE;
   if( IS_AFFECTED( victim, AFF_ETHEREAL ) )
   {
      if( !IS_AFFECTED( ch, AFF_ETHEREAL ) )
         return FALSE;
      return TRUE;
   }
   else
   {
      if( IS_AFFECTED( ch, AFF_ETHEREAL ) )
         return FALSE;
      return TRUE;
   }
   return TRUE;
}

void gain_exp( CHAR_DATA * ch, int gain )
{
   if( IS_NPC( ch ) )
      return;

   /*
    * xp cap just in case someone decides to cheat or something -- Scion 
    */
   gain = URANGE( 0, gain, 100000 );
   ch->exp = UMAX( 0, ch->exp + gain );
}

/* Take some hp. Return true if it killed them */
bool lose_hp( CHAR_DATA * ch, int amt )
{
   if( !IS_NPC( ch ) && ch->last_hit && !IS_NPC( ch->last_hit ) && IS_SET( ch->last_hit->pcdata->flags, PCFLAG_CHEAT ) )
      return FALSE;
   ch->hit -= amt;
   if( ch->last_hit && can_gain_exp( ch->last_hit, ch ) )
      gain_exp( ch->last_hit, get_exp_worth( ch ) / 10 );
   if( ch->hit < 1 )
   {
      die( ch );
      return TRUE;
   }
   return FALSE;
}

/* Take some ep. Return true if they went unconscious */
bool lose_ep( CHAR_DATA * ch, int amt )
{
   ch->move -= amt;
   if( ch->move < 1 )
   {
      uncon( ch );
      return TRUE;
   }
   return FALSE;
}

/* Do damage, but allow security to reduce it */
bool direct_damage( CHAR_DATA * ch, int amt )
{
   amt = ( 100 - number_range( 1, TALENT( ch, TAL_SECURITY ) ) ) * amt / 100;
   if( amt < 0 )
      amt = 0;
   learn_talent( ch, TAL_SECURITY );
   return lose_hp( ch, amt );
}

void do_auto( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      if( ch->pcdata->auto_attack )
         ch_printf( ch, "You will %s every chance in combat.\n\r", ch->pcdata->auto_attack );
      else
         send_to_char( "You won't try to fight in combat automatically.\n\r", ch );
      return;
   }
   if( ch->pcdata->auto_attack )
      STRFREE( ch->pcdata->auto_attack );
   if( str_cmp( argument, "none" ) )
   {
      ch->pcdata->auto_attack = STRALLOC( argument );
      ch_printf( ch, "You will now %s every opportunity in monster combat.\n\r", ch->pcdata->auto_attack );
      return;
   }
   send_to_char( "You will not automatically do anything in combat.\n\r", ch );
}

bool check_shield( CHAR_DATA * ch, CHAR_DATA * victim )
{
   if( !IS_AWAKE( ch ) || !IS_SET( ch->mood, MOOD_READY ) )
      return FALSE;

   if( ch->main_hand && IS_OBJ_STAT( ch->main_hand, ITEM_SHIELD ) )
   {
      if( number_percent(  ) >= 25 )
      {
         act( AT_YELLOW, "$n blocks your attack with $p.", ch, ch->main_hand, victim, TO_VICT );
         act( AT_YELLOW, "$n blocks $N's attack with $p.", ch, ch->main_hand, victim, TO_NOTVICT );
         act( AT_YELLOW, "You block $N's attack with $p.", ch, ch->main_hand, victim, TO_CHAR );
         learn_weapon( ch, SK_COMBAT );
         return TRUE;
      }
   }
   if( ch->off_hand && IS_OBJ_STAT( ch->off_hand, ITEM_SHIELD ) )
   {
      if( number_percent(  ) >= 25 )
      {
         act( AT_SKILL, "$n blocks your attack with $p.", ch, ch->off_hand, victim, TO_VICT );
         act( AT_SKILL, "$n blocks $N's attack with $p.", ch, ch->off_hand, victim, TO_NOTVICT );
         act( AT_SKILL, "You block $N's attack with $p.", ch, ch->off_hand, victim, TO_CHAR );
         learn_weapon( ch, SK_COMBAT );
         return TRUE;
      }
   }
   return FALSE;
}

/* Hit someone somewhere. All physical attacks call this */
void check_hit( CHAR_DATA * ch, CHAR_DATA * victim, int loc, int dam, int type )
{
   PART_DATA *part;
   char buf[MAX_STRING_LENGTH];
   int chance = 0;
   int i;

   if( !IS_SAME_PLANE( ch, victim ) )
      return;
   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
      return;
   if( IS_NPC( ch ) && xIS_SET( victim->act, ACT_PACIFIST ) )
      return;

   learn_weapon( ch, SK_COMBAT );
   ch->last_hit = victim;

   if( !IS_NPC( ch ) )
      chance += ch->pcdata->noncombat[0];

   /*
    * Sneak attack 
    */
   if( IS_AFFECTED( ch, AFF_HIDE ) )
   {
      dam *= 2;
      chance += 100;
      act( AT_DGREY, "You step out of your hiding spot.", ch, NULL, NULL, TO_CHAR );
      act( AT_DGREY, "$n steps out of $s hiding spot.", ch, NULL, NULL, TO_ROOM );
      if( IS_NPC( ch ) )
         xREMOVE_BIT( ch->affected_by, AFF_HIDE );
      else
         xREMOVE_BIT( ch->pcdata->perm_aff, AFF_HIDE );
   }

   if( loc == -1 )
   {  /* none specificed, pick one at random */
      chance += 100;
      i = 0;
      part = victim->first_part;
      while( part )
      {
         i++;
         part = part->next;
      }
      loc = number_range( 1, i );
   }

   /*
    * Make sure they have the part it would have hit 
    */
   if( ( part = find_bodypart( victim, loc ) ) == NULL || part->flags == PART_SEVERED )
   {
      act( AT_ACTION, "You miss $N by an inch.", ch, NULL, victim, TO_CHAR );
      act( AT_ACTION, "$n misses $N by an inch.", ch, NULL, victim, TO_NOTVICT );
      act( AT_ACTION, "$n misses you by an inch.", ch, NULL, victim, TO_VICT );
      return;
   }

   /*
    * Add in anymore bonuses 
    */
   if( IS_SET( ch->mood, MOOD_READY ) )
      chance += 250;
   if( IS_SET( victim->mood, MOOD_READY ) )
      chance -= 250;
   if( IS_SET( ch->mood, MOOD_DEFENSE ) )
      chance -= 300;
   if( IS_SET( victim->mood, MOOD_DEFENSE ) )
      chance -= 300;
   chance += IS_NPC( ch ) ? 100 : ch->pcdata->weapon[SK_COMBAT];
   chance += GET_HITROLL( ch );
   chance += get_curr_dex( ch );
   chance -= get_curr_dex( victim );
   chance -= ch->encumberance * 3;
   chance += victim->encumberance * 3;

   if( chance < 75 )
      chance = 75;

   /*
    * Roll to see if it hit 
    */
   chance = number_range( 1, chance );

   /*
    * Chance to dodge. Can't miss if they're not awake 
    */
   if( chance < 50 && IS_AWAKE( victim ) )
   {
      if( check_dodge( ch, victim ) )
         return;
      if( check_tumble( ch, victim ) )
         return;
   }
   if( check_blur( ch, victim ) )
      return;
   if( check_displacement( ch, victim ) )
      return;
   if( check_shield( victim, ch ) )
      return;

   /*
    * Everything went through, hurt them 
    */
   dam -= number_range( part->armor, part->armor * 3 );

   snprintf( buf, sizeof( buf ), "%s", part_locs[part->loc] );
   if( dam == 0 )
      strcat( buf, ", but fails to do any damage" );
   else if( dam < 20 )
      strcat( buf, " lightly." );
   else if( dam < 50 )
      strcat( buf, "." );
   else if( dam < 100 )
      strcat( buf, "!" );
   else if( dam < 200 )
      strcat( buf, " hard!" );
   else if( dam < 300 )
      strcat( buf, " very hard!" );
   else if( dam < 400 )
      strcat( buf, " extremely hard!" );
   else if( dam < 600 )
      strcat( buf, " incredibly hard!" );
   else if( dam < 800 )
      strcat( buf, " amazingly hard!" );
   else
      strcat( buf, " unbelievably hard!" );
   act( AT_ACTION, "$n hits $N in the $t", ch, buf, victim, TO_NOTVICT );
   act( AT_SKILL, "You hit $N in the $t", ch, buf, victim, TO_CHAR );
   act( AT_HURT, "$n hits you in the $t", ch, buf, victim, TO_VICT );

   victim->last_hit = ch;

   if( dam <= 0 )
      return;

   if( ch->last_hit && can_gain_exp( ch->last_hit, ch ) )
      gain_exp( ch, get_exp_worth( victim ) * dam / 100 );

   /*
    * Blunt weapons tend to cause more internal (direct) damage 
    */
   if( type == MAG_BLUNT )
   {
      if( dam > 500 )
         break_part( victim, part );
      if( direct_damage( victim, dam ) )
         return;
      hurt_part( victim, part, dam / 3 );
   }
   else
   {
      if( direct_damage( victim, dam / 10 ) )
         return;
      hurt_part( victim, part, dam );
   }
   pain( victim, dam );
}

/* This function processes all melee physical attacks */
void process_attack( CHAR_DATA * ch, char *argument, int dam, int type, OBJ_DATA * obj, int sk )
{
   int part;
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   int lag;

   learn_weapon( ch, sk );

   argument = one_argument( argument, arg1 );

   if( ( victim = find_target( ch, arg1, TRUE ) ) == NULL )
      return;

   if( victim == ch )
   {
      send_to_char( "You're a masochist, aren't you.\n\r", ch );
      return;
   }

   if( !can_hit( ch, victim, obj ) )
      return;

   if( !argument || argument[0] == '\0' )
      part = -1;
   else
   {
      part = find_part_name( argument );
   }

   /*
    * wait first so bonuses/rolling doesnt alter it 
    */
   /*
    * lighter weapons are faster 
    */
   lag = ( dam + ( obj ? obj->weight * 10 : 0 ) ) / 40;
   if( ch->speed > 100 )
      lag /= ch->speed / 100;
   else
      lag += ( 100 - ch->speed ) / 20;
   lag = UMAX( 4, lag );   /* so we don't have lagless attacks */
   if( lose_ep( ch, lag * 3 ) )
      return;

   /*
    * add in any bonuses 
    */
   dam += GET_DAMROLL( ch );
   if( !IS_NPC( ch ) )
      dam += ch->pcdata->noncombat[sk];
   if( obj && obj->item_type == ITEM_WEAPON )
   {
      dam += obj->weight * 10;
      dam += obj->value[2];
      if( obj->gem && IS_OBJ_STAT( obj->gem, ITEM_MAGIC ) )
         dam += 100;
      if( obj->value[4] )
         dam *= 2;
      if( is_bane( obj, victim ) )
      {
         if( number_range( 1, obj->value[6] ) < dam )
            obj->value[6]++;
         dam += number_range( 1, obj->value[6] / 10 );
      }
   }
   else
   {
      if( ch->nation )
         dam += ch->nation->unarmed;
   }

   /*
    * now roll for damage 
    */
   dam = number_range( dam / 5, dam );
   check_hit( ch, victim, part, dam, type );
}

/* Process a ranged attack. Obj is the projectile being sent */
void ranged_attack( CHAR_DATA * ch, OBJ_DATA * obj, char *argument )
{
   CHAR_DATA *victim = NULL;
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   int dir = -1, dist = 0, part = -1;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *was_in_room;
   CHAR_DATA *vch;
   char *dtxt = "somewhere";

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Where?  At who?\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg1 );

   if( ( pexit = find_door( ch, arg, TRUE ) ) == NULL )
   {
      if( ( ( victim = get_char_room( ch, arg ) ) == NULL ) )
      {
         send_to_char( "Aim in what direction?\n\r", ch );
         return;
      }
   }
   else
      dir = pexit->vdir;

   if( IS_SET( ch->in_room->room_flags, ROOM_PRIVATE ) )
   {
      send_to_char( "There isn't enough room here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Suicidal, are we?\n\r", ch );
      return;
   }

   if( pexit && !pexit->to_room )
   {
      send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
      return;
   }

   /*
    * Check for obstruction 
    */
   if( pexit && IS_SET( pexit->exit_info, EX_CLOSED ) )
   {
      if( IS_SET( pexit->exit_info, EX_SECRET ) || IS_SET( pexit->exit_info, EX_DIG ) )
         send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
      else
         send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
      return;
   }

   vch = NULL;
   if( pexit && arg1[0] != '\0' )
   {
      if( ( vch = scan_for_victim( ch, pexit, arg1 ) ) == NULL )
      {
         send_to_char( "You cannot see your target.\n\r", ch );
         return;
      }

      /*
       * don't allow attacks on mobs that are in a no-missile room --Shaddai 
       */
      if( IS_SET( vch->in_room->room_flags, ROOM_NOMISSILE ) )
      {
         send_to_char( "You can't get a clean shot off.\n\r", ch );
         return;
      }
   }
   victim = vch;

   was_in_room = ch->in_room;
   sprintf( buf, "firing the %s", myobj( obj ) );
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( arg );
   WAIT_STATE( ch, PULSE_VIOLENCE );

   /*
    * if pexit is NULL at this point, it must be a point blank shot 
    */
   if( !pexit )
   {
      victim = find_target( ch, arg, TRUE );
      if( !victim )
         return;

      if( !can_hit( ch, victim, obj ) )
         return;

      if( arg1 == NULL || arg1[0] == '\0' )
         part = -1;
      else
      {
         part = find_part_name( arg1 );
      }

      check_hit( ch, victim, part, 200 + obj->value[2], MAG_PIERCE );

      if( obj->in_obj )
         obj_from_obj( obj );
      if( obj->carried_by )
         obj_from_char( obj );

      /*
       * bullets and blasts aren't reusable 
       */
      if( obj->value[5] == SK_FIREARMS || obj->value[5] == SK_WAND )
         extract_obj( obj );
      else
         obj_to_room( obj, ch->in_room );

      return;
   }  /* end point blank shot */

   while( dist <= 5 )
   {

      char_from_room( ch );
      char_to_room( ch, pexit->to_room );

      if( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         /*
          * whadoyahknow, the door's closed 
          */
         sprintf( buf, "You see your %s pierce a door in the distance to the %s.", myobj( obj ), dir_name[dir] );
         act( AT_ACTION, buf, ch, NULL, NULL, TO_CHAR );
         if( dir == 4 )
            sprintf( buf, "$p flies in from %s and implants itself solidly in the ceiling.", dtxt );
         else if( dir == 5 )
            sprintf( buf, "$p flies in from %s and implants itself solidly in the floor.", dtxt );
         else
            sprintf( buf, "$p flies in from %s and implants itself solidly in the %sern door.", dtxt, dir_name[dir] );
         act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );

         if( obj->in_obj )
            obj_from_obj( obj );
         if( obj->carried_by )
            obj_from_char( obj );

         if( obj->value[5] == SK_WAND )
            extract_obj( obj );
         else if( IS_OBJ_STAT( obj, ITEM_RETURNING ) )
         {
            act( AT_SKILL, "$p returns to your hand.", ch, obj, NULL, TO_CHAR );
            act( AT_SKILL, "$p whirls around and flies back.", ch, obj, NULL, TO_ROOM );
            obj_to_char( obj, ch );
         }
         else
            obj_to_room( obj, ch->in_room );
         break;
      }

      /*
       * no victim? pick a random one 
       */
      if( !victim )
      {
         for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
         {
            if( number_bits( 1 ) == 0 && ch != vch )
            {
               victim = vch;
               break;
            }
         }
      }

      dtxt = rev_exit( pexit->vdir );

      /*
       * In the same room as our victim? 
       */
      if( victim && ch->in_room == victim->in_room )
      {

         act( AT_PLAIN, "$p flies in from $T.", ch, obj, dtxt, TO_ROOM );

         if( !can_hit( ch, victim, obj ) )
            return;

         check_hit( ch, victim, -1, 100 + obj->value[2], MAG_PIERCE );

         if( obj->in_obj )
            obj_from_obj( obj );
         if( obj->carried_by )
            obj_from_char( obj );

         /*
          * bullets and blasts aren't reusable 
          */
         if( obj->value[5] == SK_FIREARMS || obj->value[5] == SK_WAND )
            extract_obj( obj );
         else if( IS_OBJ_STAT( obj, ITEM_RETURNING ) )
         {
            act( AT_SKILL, "$p returns to your hand.", ch, obj, NULL, TO_CHAR );
            act( AT_SKILL, "$p whirls around and flies back.", ch, obj, NULL, TO_ROOM );
            obj_to_char( obj, ch );
         }
         else
            obj_to_room( obj, ch->in_room );

         char_from_room( ch );
         char_to_room( ch, was_in_room );
         return;
      }

      if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
      {
         act( AT_PLAIN, "Your $t hits a wall and bounces harmlessly to the ground to the $T.",
              ch, myobj( obj ), dir_name[dir], TO_CHAR );
         act( AT_PLAIN, "$p strikes the $Tern wall and falls harmlessly to the ground.", ch, obj, dir_name[dir], TO_ROOM );
         if( obj->in_obj )
            obj_from_obj( obj );
         if( obj->carried_by )
            obj_from_char( obj );

         if( obj->value[5] == SK_WAND )
            extract_obj( obj );
         else if( IS_OBJ_STAT( obj, ITEM_RETURNING ) )
         {
            act( AT_SKILL, "$p returns to your hand.", ch, obj, NULL, TO_CHAR );
            act( AT_SKILL, "$p whirls around and flies back.", ch, obj, NULL, TO_ROOM );
            obj_to_char( obj, ch );
         }
         else
            obj_to_room( obj, ch->in_room );
         break;
      }

      act( AT_ACTION, "$p whizzes past you from $T.", ch, obj, dtxt, TO_ROOM );
      dist++;
   }  /* end while loop */

   char_from_room( ch );
   char_to_room( ch, was_in_room );
   if( obj && IS_OBJ_STAT( obj, ITEM_RETURNING ) )
      act( AT_SKILL, "$p flies back into the room and $n deftly catches it again.", ch, obj, NULL, TO_ROOM );
}

/* Stop a fight */
void stop_fighting( CHAR_DATA * ch, bool fBoth )
{
   CHAR_DATA *fch;

   ch->last_hit = NULL;

   if( !fBoth )   /* major short cut here by Thoric */
      return;

   for( fch = first_char; fch; fch = fch->next )
   {
      if( fch->last_hit == ch )
      {
         fch->last_hit = NULL;
      }
   }
}

/* Mob attack function
 * Mob picks a random attack based on its bodyparts
 */
void mob_attack( CHAR_DATA * ch, CHAR_DATA * victim )
{
   int i = number_range( 1, 3 );

   if( !ch )
      return;
   if( !ch->in_room )
      return;
   if( !victim )
      return;
   if( victim == ch )
      return;
   if( !IS_AWAKE( ch ) )
      return;

   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_DEFENSE ) )
      return;

   if( xIS_SET( ch->act, ACT_PHYS_FAIL ) && IS_NPC( ch ) )
   {
      do_flee( ch, "" );
      return;
   }

   if( ch->main_hand )
   {
      switch ( ch->main_hand->value[5] )
      {
         case SK_SWORD:
            switch ( i )
            {
               case 1:
                  do_slash( ch, victim->name );
                  break;
               case 2:
                  do_stab( ch, victim->name );
                  break;
               case 3:
                  do_lunge( ch, victim->name );
                  break;
            }
            break;
         case SK_DAGGER:
            switch ( i )
            {
               case 1:
                  do_slash( ch, victim->name );
                  break;
               default:
                  do_stab( ch, victim->name );
                  break;
            }
            break;
         case SK_AXE:
            switch ( i )
            {
               case 1:
                  do_slash( ch, victim->name );
                  break;
               default:
                  do_chop( ch, victim->name );
                  break;
            }
         case SK_HALBERD:
         case SK_POLEARM:
            do_slash( ch, victim->name );
            break;
         case SK_MACE:
         case SK_STAFF:
            do_pound( ch, victim->name );
            break;
         case SK_SPEAR:
            do_lunge( ch, victim->name );
            break;
         case SK_CLAW:
            do_claw( ch, victim->name );
            break;
         case SK_WHIP:
            do_whip( ch, victim->name );
            break;
         case SK_FIREARMS:
         case SK_BOW:
         case SK_CROSSBOW:
         case SK_WAND:
            if( ch->main_hand->value[0] > 0 )
            {
               do_shoot( ch, victim->name );
               break;
            }
      }
   }


   if( ch->off_hand && ch->off_hand != ch->main_hand )
   {
      switch ( ch->off_hand->value[5] )
      {
         case SK_SWORD:
            switch ( i )
            {
               case 1:
                  do_slash( ch, victim->name );
                  break;
               case 2:
                  do_stab( ch, victim->name );
                  break;
               case 3:
                  do_lunge( ch, victim->name );
                  break;
            }
            break;
         case SK_DAGGER:
            switch ( i )
            {
               case 1:
                  do_slash( ch, victim->name );
                  break;
               default:
                  do_stab( ch, victim->name );
                  break;
            }
            break;
         case SK_AXE:
            switch ( i )
            {
               case 1:
                  do_slash( ch, victim->name );
                  break;
               default:
                  do_chop( ch, victim->name );
                  break;
            }
         case SK_HALBERD:
         case SK_POLEARM:
            do_slash( ch, victim->name );
            break;
         case SK_MACE:
         case SK_STAFF:
            do_pound( ch, victim->name );
            break;
         case SK_SPEAR:
            do_lunge( ch, victim->name );
            break;
         case SK_CLAW:
            do_claw( ch, victim->name );
            break;
      }
   }

   if( IS_SET( ch->xflags, 1 << PART_CLAWS ) && i == 1 )
   {
      do_claw( ch, victim->name );
      return;
   }
   if( IS_SET( ch->xflags, 1 << PART_TAIL ) && i == 2 )
   {
      do_tail( ch, victim->name );
      return;
   }
   if( IS_SET( ch->xflags, 1 << PART_HORNS ) && i == 3 )
   {
      do_headbutt( ch, victim->name );
      return;
   }
   if( IS_SET( ch->xflags, 1 << PART_FEET ) && i == 1 )
   {
      do_kick( ch, victim->name );
      return;
   }
   if( IS_SET( ch->xflags, 1 << PART_HANDS ) && i == 2 )
   {
      do_punch( ch, victim->name );
      return;
   }
   if( IS_SET( ch->xflags, 1 << PART_LEGS ) && i == 3 )
   {
      do_knee( ch, victim->name );
      return;
   }
   do_hit( ch, victim->name );
}

/* Mood functions */

void do_ready( CHAR_DATA * ch, char *argument )
{
   if( IS_SET( ch->mood, MOOD_READY ) )
   {
      send_to_char( "You are already in a ready position!\n\r", ch );
      return;
   }
   act( AT_ACTION, "You spring to readiness.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n springs to readiness.", ch, NULL, NULL, TO_ROOM );
   REMOVE_BIT( ch->mood, MOOD_RELAXED );
   SET_BIT( ch->mood, MOOD_READY );
}

void do_relax( CHAR_DATA * ch, char *argument )
{
   if( IS_SET( ch->mood, MOOD_RELAXED ) )
   {
      send_to_char( "You are already relaxed!\n\r", ch );
      return;
   }
   act( AT_ACTION, "You relax from your readiness.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n seems to relax.", ch, NULL, NULL, TO_ROOM );
   REMOVE_BIT( ch->mood, MOOD_READY );
   SET_BIT( ch->mood, MOOD_RELAXED );
}

/* Generic hit function. Swing whatever you're wielding at them */
void do_hit( CHAR_DATA * ch, char *argument )
{
   if( ch->main_hand )
   {
      act( AT_ACTION, "You swing $p.", ch, ch->main_hand, NULL, TO_CHAR );
      act( AT_ACTION, "$n swings $p.", ch, ch->main_hand, NULL, TO_ROOM );
      process_attack( ch, argument, ch->main_hand->weight * 5, MAG_BLUNT, ch->main_hand, SK_COMBAT );
   }
   else
      process_attack( ch, argument, 50, MAG_BLUNT, NULL, SK_COMBAT );
}

/* Physical attacks */

void do_punch( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj = NULL;

   if( !can_use_bodypart( ch, BP_RHAND )
       && !can_use_bodypart( ch, BP_LHAND ) && ( obj = get_weapon( ch, SK_HAND ) ) == NULL )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You swing your fist.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n swings $s fist.", ch, NULL, NULL, TO_ROOM );

   process_attack( ch, argument, 150, MAG_BLUNT, obj, SK_HAND );
}

void do_kick( CHAR_DATA * ch, char *argument )
{
   if( !can_use_bodypart( ch, BP_RFOOT )
       && !can_use_bodypart( ch, BP_LFOOT ) && !can_use_bodypart( ch, BP_RHOOF ) && !can_use_bodypart( ch, BP_LHOOF ) )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You kick with your $t.", ch, IS_SET( ch->xflags, 1 << PART_HOOVES ) ? "hooves" : "foot", NULL, TO_CHAR );
   act( AT_ACTION, "$n kicks with $s $t.", ch, IS_SET( ch->xflags, 1 << PART_HOOVES ) ? "hooves" : "foot", NULL, TO_ROOM );

   process_attack( ch, argument, 400 * ( IS_SET( ch->xflags, 1 << PART_HOOVES ) ? 2 : 1 ), MAG_BLUNT, NULL, SK_KICK );
}

void do_knee( CHAR_DATA * ch, char *argument )
{
   if( !can_use_bodypart( ch, BP_RLEG ) && !can_use_bodypart( ch, BP_LLEG ) )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You thrust your knee.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n thrusts $s knee.", ch, NULL, NULL, TO_ROOM );

   process_attack( ch, argument, 250, MAG_BLUNT, NULL, SK_KICK );
}

void do_elbow( CHAR_DATA * ch, char *argument )
{
   if( !can_use_bodypart( ch, BP_RARM ) && !can_use_bodypart( ch, BP_LARM ) )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You thrust your elbow.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n thrusts $s elbow.", ch, NULL, NULL, TO_ROOM );

   process_attack( ch, argument, 350, MAG_BLUNT, NULL, SK_HAND );
}

void do_headbutt( CHAR_DATA * ch, char *argument )
{
   act( AT_ACTION, "You butt your head.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n butts $s head.", ch, NULL, NULL, TO_ROOM );

   if( can_use_bodypart( ch, BP_RHORN ) || can_use_bodypart( ch, BP_LHORN ) || can_use_bodypart( ch, BP_HORN ) )
      process_attack( ch, argument, 500, MAG_PIERCE, NULL, SK_HEAD );
   else
      process_attack( ch, argument, 100, MAG_BLUNT, NULL, SK_HEAD );
}

void do_bite( CHAR_DATA * ch, char *argument )
{
   act( AT_ACTION, "You snap your teeth.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n snaps $s teeth.", ch, NULL, NULL, TO_ROOM );

   process_attack( ch, argument, ( can_use_bodypart( ch, BP_FANGS ) ? 100 : 600 ), MAG_PIERCE, NULL, SK_FANG );
}

void do_claw( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj = NULL;

   if( ( obj = get_weapon( ch, SK_CLAW ) ) == NULL && !can_use_bodypart( ch, BP_CLAWS ) )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You swipe your claws.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n swipes $s claws.", ch, NULL, NULL, TO_ROOM );

   process_attack( ch, argument, 300, MAG_PIERCE, obj, SK_CLAW );
}

void do_tail( CHAR_DATA * ch, char *argument )
{
   if( !can_use_bodypart( ch, BP_TAIL ) )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You swing your tail.", ch, NULL, NULL, TO_CHAR );
   act( AT_ACTION, "$n swings $s tail.", ch, NULL, NULL, TO_ROOM );

   process_attack( ch, argument, 400, MAG_BLUNT, NULL, SK_TAIL );
}

void do_slap( CHAR_DATA * ch, char *argument )
{
   if( !can_use_bodypart( ch, BP_RHAND ) && !can_use_bodypart( ch, BP_LHAND ) )
   {
      huh( ch );
      return;
   }

   check_social( ch, "slap", argument );

   process_attack( ch, argument, 20, MAG_BLUNT, NULL, SK_HAND );
}

/* If you don't want these commands, comment out this section */

void do_piss( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   PART_DATA *part;
   CHAR_DATA *victim;

   if( IS_NPC( ch ) || ch->pcdata->condition[COND_THIRST] < 10 )
   {
      send_to_char( "Nothing comes out.\n\r", ch );
      return;
   }

   if( ( part = find_bodypart( ch, BP_PENIS ) ) != NULL )
   {
      if( part->obj )
      {
         act( AT_YELLOW, "You piss your pants.", ch, NULL, NULL, TO_CHAR );
         act( AT_YELLOW, "$n pisses $s pants.", ch, NULL, NULL, TO_ROOM );
         return;
      }
   }
   else if( ( part = find_bodypart( ch, BP_VAGINA ) ) != NULL )
   {
      if( part->obj )
      {
         act( AT_YELLOW, "You piss your pants.", ch, NULL, NULL, TO_CHAR );
         act( AT_YELLOW, "$n pisses $s pants.", ch, NULL, NULL, TO_ROOM );
         return;
      }
   }

   victim = find_target( ch, argument, TRUE );

   if( victim && ( can_use_bodypart( ch, BP_PENIS ) || IS_AFFECTED( ch, AFF_FLYING ) || victim->position < POS_SQUATTING ) )
   {
      act( AT_YELLOW, "You piss on $N!", ch, NULL, victim, TO_CHAR );
      act( AT_YELLOW, "$n pisses on you!", ch, NULL, victim, TO_VICT );
      act( AT_YELLOW, "$n pisses on $N!", ch, NULL, victim, TO_NOTVICT );
   }
   else
   {
      act( AT_YELLOW, "You take a piss on the floor.", ch, NULL, NULL, TO_CHAR );
      act( AT_YELLOW, "$n takes a piss on the floor.", ch, NULL, NULL, TO_ROOM );
   }

   obj = obj_to_room( create_object( get_obj_index( OBJ_VNUM_PUDDLE ), 0 ), ch->in_room );
   obj->value[1] = 10;
   obj->value[0] = 10;
   obj->value[2] = 8;
   obj->timer = 5;
   gain_condition( ch, COND_THIRST, -10 );

   sprintf( buf, obj->name, liq_table[obj->value[2]] );
   STRFREE( obj->name );
   obj->name = STRALLOC( buf );

   sprintf( buf, obj->short_descr, liq_table[obj->value[2]] );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( "relieving yourself" );
   WAIT_STATE( ch, 10 );
   return;
}

/* end gross section */

/* START WEAPONS SECTION */

/* Weapon handling functions */

int free_hands( CHAR_DATA * ch )
{
   int i = 0;

   if( can_use_bodypart( ch, BP_RHAND ) && !ch->main_hand )
      i++;

   if( can_use_bodypart( ch, BP_LHAND ) && !ch->off_hand )
      i++;

   return i;
}

OBJ_DATA *get_weapon( CHAR_DATA * ch, int skill )
{
   bool main_good;
   bool off_good;

   main_good = FALSE;
   off_good = FALSE;

   if( ch->main_hand
       && ( ch->main_hand->item_type == ITEM_WEAPON
            || ch->main_hand->item_type == ITEM_MISSILE_WEAPON ) && ch->main_hand->value[5] == skill )
      main_good = TRUE;

   if( ch->off_hand
       && ( ch->off_hand->item_type == ITEM_WEAPON
            || ch->off_hand->item_type == ITEM_MISSILE_WEAPON ) && ch->off_hand->value[5] == skill )
      off_good = TRUE;

   if( main_good && off_good )
   {
      switch ( number_range( 1, 2 ) )
      {
         default:
            return ch->main_hand;
            break;
         case 2:
            return ch->off_hand;
            break;
      }
   }
   if( main_good )
      return ch->main_hand;
   if( off_good )
      return ch->off_hand;

   return NULL;
}


/* Weapon attack functions */

void do_whip( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_weapon( ch, SK_WHIP ) ) == NULL )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You crack your $t.", ch, myobj( obj ), NULL, TO_CHAR );
   act( AT_ACTION, "$n cracks $s $t.", ch, myobj( obj ), NULL, TO_ROOM );

   process_attack( ch, argument, 200, MAG_BLUNT, obj, obj->value[5] );
}

void do_slash( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_weapon( ch, SK_SWORD ) ) == NULL
       && ( obj = get_weapon( ch, SK_DAGGER ) ) == NULL
       && ( obj = get_weapon( ch, SK_AXE ) ) == NULL
       && ( obj = get_weapon( ch, SK_POLEARM ) ) == NULL && ( obj = get_weapon( ch, SK_HALBERD ) ) == NULL )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You slash with your $t.", ch, myobj( obj ), NULL, TO_CHAR );
   act( AT_ACTION, "$n slashes with $s $t.", ch, myobj( obj ), NULL, TO_ROOM );

   process_attack( ch, argument, 400, MAG_SLASH, obj, obj->value[5] );
}

void do_lunge( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_weapon( ch, SK_SWORD ) ) == NULL && ( obj = get_weapon( ch, SK_SPEAR ) ) == NULL )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You lunge with your $t.", ch, myobj( obj ), NULL, TO_CHAR );
   act( AT_ACTION, "$n lunges with $s $t.", ch, myobj( obj ), NULL, TO_ROOM );

   process_attack( ch, argument, 750, MAG_PIERCE, obj, obj->value[5] );
}

void do_chop( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_weapon( ch, SK_AXE ) ) == NULL && ( obj = get_weapon( ch, SK_FANG ) ) == NULL )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You chop with your $t.", ch, myobj( obj ), NULL, TO_CHAR );
   act( AT_ACTION, "$n chops with $s $t.", ch, myobj( obj ), NULL, TO_ROOM );

   process_attack( ch, argument, 700, MAG_SLASH, obj, obj->value[5] );
}

void do_stab( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_weapon( ch, SK_SWORD ) ) == NULL
       && ( obj = get_weapon( ch, SK_DAGGER ) ) == NULL && ( obj = get_weapon( ch, SK_SPEAR ) ) == NULL )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You stab with your $t.", ch, myobj( obj ), NULL, TO_CHAR );
   act( AT_ACTION, "$n stabs with $s $t.", ch, myobj( obj ), NULL, TO_ROOM );

   process_attack( ch, argument, 250, MAG_PIERCE, obj, obj->value[5] );
}

void do_pound( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_weapon( ch, SK_MACE ) ) == NULL && ( obj = get_weapon( ch, SK_STAFF ) ) == NULL )
   {
      huh( ch );
      return;
   }

   act( AT_ACTION, "You swing your $t.", ch, myobj( obj ), NULL, TO_CHAR );
   act( AT_ACTION, "$n swings $s $t.", ch, myobj( obj ), NULL, TO_ROOM );

   process_attack( ch, argument, 400, MAG_BLUNT, obj, obj->value[5] );
}

/* Ranged weapon attacks */

void do_throw( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   char arg[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );

   if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }

   if( obj->item_type == ITEM_WEAPON )
      learn_weapon( ch, obj->value[5] );

   act( AT_ACTION, "You throw $p.", ch, obj, NULL, TO_CHAR );
   act( AT_ACTION, "$n throws $p.", ch, obj, NULL, TO_ROOM );
   ranged_attack( ch, obj, argument );
}

void do_shoot( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;
   OBJ_DATA *bullet;

   if( ( obj = get_weapon( ch, SK_FIREARMS ) ) == NULL
       && ( obj = get_weapon( ch, SK_BOW ) ) == NULL
       && ( obj = get_weapon( ch, SK_CROSSBOW ) ) == NULL && ( obj = get_weapon( ch, SK_WAND ) ) == NULL )
   {
      huh( ch );
      return;
   }

   if( obj->value[0] == 0 && obj->value[5] != SK_WAND )
   {
      ch_printf( ch, "Your %s is empty.\n\r", myobj( obj ) );
      return;
   }
   else if( obj->value[5] == SK_WAND && obj->raw_mana <= 0 && !IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      ch_printf( ch, "Your %s is out of energy.\n\r", myobj( obj ) );
      return;
   }

   act( AT_ACTION, "You fire $p.", ch, obj, NULL, TO_CHAR );
   act( AT_ACTION, "$n fires $p.", ch, obj, NULL, TO_ROOM );

   if( obj->value[5] == SK_WAND )
      obj->raw_mana -= 50;
   else if( !IS_OBJ_STAT( obj, ITEM_RETURNING ) )
      obj->value[0]--;

   if( obj->value[5] == SK_FIREARMS )
   {
      WAIT_STATE( ch, 8 );
      player_echo( ch, AT_ACTION, "You hear a gunshot.", ECHOTAR_AREA );
      bullet = create_object( get_obj_index( 29050 ), 0 );  /* bullet */
      STRFREE( bullet->short_descr );
      bullet->short_descr = STRALLOC( "bullet" );
   }
   else if( obj->value[5] == SK_BOW )
   {
      WAIT_STATE( ch, 10 );
      bullet = create_object( get_obj_index( 29048 ), 0 );  /* arrow */
      STRFREE( bullet->short_descr );
      bullet->short_descr = STRALLOC( "arrow" );
   }
   else if( obj->value[5] == SK_CROSSBOW )
   {
      WAIT_STATE( ch, 12 );
      bullet = create_object( get_obj_index( 29046 ), 0 );  /* bolt */
      STRFREE( bullet->short_descr );
      bullet->short_descr = STRALLOC( "bolt" );
   }
   else if( obj->value[5] == SK_WAND )
   {
      WAIT_STATE( ch, 14 );
      bullet = create_object( get_obj_index( 50 ), 0 );  /* blast */
   }

   if( !bullet )
   {
      send_to_char( "Your weapon jams mysteriously.\n\r", ch );
      bug( "Do_shoot: Cannot find type for projectile.", 0 );
      return;
   }

   if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      xSET_BIT( bullet->extra_flags, ITEM_MAGIC );
   if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
      xSET_BIT( bullet->extra_flags, ITEM_DREAMWORLD );
   bullet->value[2] += obj->value[2];
   if( obj->value[4] )
      bullet->value[2] *= 2;
   if( obj->gem && IS_OBJ_STAT( obj->gem, ITEM_MAGIC ) )
      bullet->value[2] += 100;
   bullet->value[2] = number_range( bullet->value[2] / 5, bullet->value[2] );
   ranged_attack( ch, bullet, argument );
   learn_weapon( ch, obj->value[5] );

   /*
    * Check for a second gun 
    */
   if( ch->main_hand && ch->main_hand->item_type == ITEM_MISSILE_WEAPON
       && ( ch->main_hand->value[5] == SK_FIREARMS
            || ch->main_hand->value[5] == SK_WAND )
       && ch->off_hand && ch->off_hand->item_type == ITEM_MISSILE_WEAPON
       && ( ch->off_hand->value[5] == SK_FIREARMS || ch->off_hand->value[5] == SK_WAND ) )
   {
      obj = ch->off_hand;

      if( obj->value[0] == 0 && obj->value[5] != SK_WAND )
      {
         ch_printf( ch, "Your %s is empty.\n\r", myobj( obj ) );
         return;
      }
      else if( obj->value[5] == SK_WAND && obj->raw_mana <= 0 )
      {
         ch_printf( ch, "Your %s is out of energy.\n\r", myobj( obj ) );
         return;
      }

      act( AT_ACTION, "You fire $p.", ch, obj, NULL, TO_CHAR );
      act( AT_ACTION, "$n fires $p.", ch, obj, NULL, TO_ROOM );

      if( obj->value[5] == SK_WAND )
         obj->raw_mana -= 50;
      else if( !IS_OBJ_STAT( obj, ITEM_RETURNING ) )
         obj->value[0]--;

      if( obj->value[5] == SK_FIREARMS )
      {
         player_echo( ch, AT_ACTION, "You hear another gunshot.", ECHOTAR_AREA );
         bullet = create_object( get_obj_index( 29050 ), 0 );  /* bullet */
         STRFREE( bullet->short_descr );
         bullet->short_descr = STRALLOC( "bullet" );
      }
      else if( obj->value[5] == SK_WAND )
      {
         WAIT_STATE( ch, 14 );
         bullet = create_object( get_obj_index( 50 ), 0 );  /* blast */
      }

      if( !bullet )
      {
         send_to_char( "Your weapon jams mysteriously.\n\r", ch );
         bug( "Do_shoot: Cannot find type for projectile.", 0 );
         return;
      }

      if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
         xSET_BIT( bullet->extra_flags, ITEM_MAGIC );
      if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
         xSET_BIT( bullet->extra_flags, ITEM_DREAMWORLD );
      bullet->value[2] += obj->value[2];
      if( obj->value[4] )
         bullet->value[2] *= 2;
      if( obj->gem && IS_OBJ_STAT( obj->gem, ITEM_MAGIC ) )
         bullet->value[2] += 100;
      bullet->value[2] = number_range( bullet->value[2] / 5, bullet->value[2] );
      ranged_attack( ch, bullet, argument );
      learn_weapon( ch, obj->value[5] );

   }  /* end second gun */
}

/* Weapon manipulation commands */

void do_wield( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   obj = get_obj_carry( ch, argument );
   if( !obj )
   {
      send_to_char( "Hold what?\n\r", ch );
      return;
   }

   if( obj == ch->main_hand || obj == ch->off_hand )
   {
      send_to_char( "You are already wielding that.\n\r", ch );
      return;
   }

   if( !free_hands( ch ) )
   {
      send_to_char( "You have no free hands.\n\r", ch );
      return;
   }

   obj_from_char( obj );
   obj_to_char( obj, ch );
   separate_obj( obj );
   if( obj->weight > ch->weight / 2 )
   {
      send_to_char( "That is too heavy for you to wield.\n\r", ch );
      return;
   }
   else if( obj->weight > ch->weight / 4 || IS_OBJ_STAT( obj, ITEM_TWO_HANDED ) )
   {
      if( free_hands( ch ) < 2 )
      {
         send_to_char( "You need two hands to wield that.\n\r", ch );
         return;
      }
      ch->off_hand = obj;
      ch->main_hand = obj;
      act( AT_ACTION, "You hold $p in both hands.", ch, obj, NULL, TO_CHAR );
      act( AT_ACTION, "$n holds $p in both hands.", ch, obj, NULL, TO_ROOM );
   }
   else
   {
      if( !ch->main_hand && can_use_bodypart( ch, BP_RHAND ) )
      {
         ch->main_hand = obj;
         act( AT_ACTION, "You hold $p in your right hand.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n holds $p in $s right hand.", ch, obj, NULL, TO_ROOM );
      }
      else if( !ch->off_hand && can_use_bodypart( ch, BP_LHAND ) )
      {
         ch->off_hand = obj;
         act( AT_ACTION, "You hold $p in your left hand.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n holds $p in $s left hand.", ch, obj, NULL, TO_ROOM );
      }
      else
      {
         send_to_char( "Both of your hands are full.\n\r", ch );
         return;
      }
   }
   obj_affect_ch( ch, obj );
}

void do_sheathe( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   obj = get_obj_carry( ch, argument );
   if( !obj )
   {
      send_to_char( "You are not carrying that.\n\r", ch );
      return;
   }

   if( obj != ch->main_hand && obj != ch->off_hand )
   {
      send_to_char( "You are not holding that.\n\r", ch );
      return;
   }

   obj_from_char( obj );
   obj_to_char( obj, ch );
   act( AT_ACTION, "You put down $p.", ch, obj, NULL, TO_CHAR );
   act( AT_ACTION, "$n puts down $p.", ch, obj, NULL, TO_ROOM );
}

void do_load( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *gun;
   OBJ_DATA *bullet;
   char arg[MAX_INPUT_LENGTH];


   argument = one_argument( argument, arg );

   gun = get_obj_carry( ch, argument );

   if( !gun )
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }

   if( gun->item_type != ITEM_MISSILE_WEAPON )
   {
      send_to_char( "You can't load that!\n\r", ch );
      return;
   }


   bullet = get_obj_carry( ch, arg );

   if( !bullet )
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }

   if( bullet->item_type != ITEM_PROJECTILE )
   {
      send_to_char( "That is not a projectile!\n\r", ch );
      return;
   }

   if( gun->value[5] != bullet->value[5] )
   {
      send_to_char( "That is the wrong type of projectile for that weapon.\n\r", ch );
      return;
   }

   if( !IS_OBJ_STAT( gun, ITEM_DREAMWORLD ) && IS_OBJ_STAT( bullet, ITEM_DREAMWORLD ) )
   {
      send_to_char( "In your dreams.\n\r", ch );
      return;
   }
   if( ((gun->value[5] == SK_BOW) || (gun->value[5] == SK_CROSSBOW)) && (gun->value[0] >= 1) )
   {
	send_to_char( "Your weapon is already loaded.\n\r", ch );
	return;
   }
   
   if ( gun->value[5] == SK_FIREARMS && gun->value[0] >= 6 )
   {
   send_to_char( "Your weapon is fully loaded.\n\r", ch );
   return;
   }

   gun->value[0] += 1;//bullet->count;
   separate_obj( bullet );
   extract_obj( bullet );
   sprintf( arg, "loading the %s", myobj( gun ) );
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( arg );
   WAIT_STATE( ch, PULSE_VIOLENCE / 2 );

   act( AT_ACTION, "You load $p.", ch, gun, NULL, TO_CHAR );
   act( AT_ACTION, "$n loads $p.", ch, gun, NULL, TO_ROOM );
}
