/*
 *	Talents
 *	=======
 *	Abilities and handling
 *	Written by Keolah and Triskal for Rogue Winds
 *
 *	You may not use or distribute any of this code without the
 *	explicit permission of the code maintainer, Heather Dunn (Keolah)
 */  
   
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
#include <strings.h>
#endif /*  */
#include <time.h>
#include "mud.h"
   
/* from birth.c */ 
extern void set_race( CHAR_DATA * ch, bool died );

/* from handler.c */ 
extern SPECIES_DATA *find_species( char *name );
extern char *munch_colors( char *word );

/* from update.c */ 
extern void check_bodyparts( CHAR_DATA * ch );

/* from interp.c */ 
extern bool check_social( CHAR_DATA * ch, char *command, char *argument );

/* from mud_comm.c */ 
extern int get_color( char *argument );

/* from combat.c */ 
extern void ranged_attack( CHAR_DATA * ch, OBJ_DATA * obj, char *argument );
extern bool can_gain_exp( CHAR_DATA * ch, CHAR_DATA * victim );

/* from bodyparts.c */ 
extern void add_part( CHAR_DATA * ch, int loc, int to_loc );

/* from misc.c */ 
extern sh_int get_full( CHAR_DATA * ch );

/* Trade in Talent points for Talents 
	tradein <amount> <Talent name> */ 
void do_tradein( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int amount;
   char arg[MAX_INPUT_LENGTH];
   DEITY_DATA * talent;
   bool inborn;
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      for( talent = first_deity; talent; talent = talent->next )
      {
         sprintf( buf, "&Y%-20s: %-3d  &CCost per point: %d\r\n", talent->name, 
                   ( ch->talent[talent->index] > 0 ? ch->talent[talent->index] : 0 ), talent->cost );
         send_to_char( buf, ch );
      }
      ch_printf( ch, "&YYou have %d points remaining to spend.\r\n", ch->pcdata->points );
      ch_printf( ch, "&YSyntax: tradein <# of points to raise> <talent to raise>&w\r\n" );
      ch_printf( ch, "&YSyntax: tradein <points> <attribute>\n\r" );
      ch_printf( ch, "&YSyntax: tradein inborn <talent>&w\n\r" );
      return;
   }
   argument = one_argument( argument, arg );
   inborn = FALSE;
   if( !str_cmp( arg, "inborn" ) )
      inborn = TRUE;
   
   else if( !is_number( arg ) )
   {
      send_to_char( "That isn't a number!\n\r", ch );
      return;
   }
   amount = atoi( arg );
   argument = one_argument( argument, arg );
   if( arg == NULL || arg[0] == '\0' )
   {
      send_to_char( "Trade into what?\n\r", ch );
      return;
   }
   for( talent = first_deity; talent; talent = talent->next )
   {
      if( toupper( (int)arg[0] ) == toupper( (int)talent->name[0] )  &&!str_prefix( arg, talent->name ) )
         break;
   }
   if( !talent && arg != NULL && arg[0] != '\0' )
   {
      if( amount < 1 )
      {
         send_to_char( "You wish you could do that.\n\r", ch );
         return;
      }
      if( ch->pcdata->points < amount )
      {
         send_to_char( "You don't have enough points for that.\n\r", ch );
         return;
      }
      if( !str_prefix( arg, "strength" ) )
         ch->perm_str += amount;
      
      else if( !str_prefix( arg, "dexterity" ) )
         ch->perm_dex += amount;
      
      else if( !str_prefix( arg, "constitution" ) )
         ch->perm_con += amount;
      
      else if( !str_prefix( arg, "willpower" ) )
         ch->perm_wil += amount;
      
      else if( !str_prefix( arg, "intelligence" ) )
         ch->perm_int += amount;
      
      else if( !str_prefix( arg, "perception" ) )
         ch->perm_per += amount;
      
      else if( !str_prefix( arg, "endurance" ) )
         ch->perm_lck += amount;
      
      else
      {
         send_to_char( "You know of no such Talent or attribute.\r\n", ch );
         return;
      }
      ch->pcdata->points -= amount;
      send_to_char( "&YDone.\n\r", ch );
      return;
   }
   if( inborn )
   {
      if( ch->pcdata->inborn > -1 )
      {
         send_to_char( "You already have been inborn with a Talent.\n\r", ch );
         return;
      }
      else
      {
         ch_printf( ch, "You have been inborn with the %s Talent.\n\r", talent->name );
         ch->pcdata->inborn = talent->index;
         return;
      }
   }
   if( amount * talent->cost > ch->pcdata->points )
   {
      send_to_char( "You don't have that many points to spend.\r\n", ch );
      return;
   }
   if( amount < 1 )
   {
      send_to_char( "You'd like to do that, wouldn't you?\r\n", ch );
      return;
   }
   if( ch->talent[talent->index] + amount > 200 )
   {
      send_to_char( "You may only raise a Talent to 200 points.\r\n", ch );
      return;
   }
   if( ch->talent[talent->index] < 0 )
      ch->talent[talent->index] = 0;
   if( ch->curr_talent[talent->index] < 0 )
      ch->curr_talent[talent->index] = 1;
   ch->pcdata->points -= amount * talent->cost;
   ch->talent[talent->index] += amount;
   talent->count += amount;
   save_deity( talent );
   ch_printf( ch, "Your %s power raises by %d to %d.\r\n", talent->name, amount, ch->talent[talent->index] );
}


/* see if they can travel to the victim -keo */ 
   bool can_travel( int i, CHAR_DATA * ch, CHAR_DATA * victim )
{
   if( TALENT( ch, TAL_MOTION ) >= 200 )
      return TRUE;
   if( !ch->in_room 
         ||( IS_SET( ch->in_room->room_flags, ROOM_NO_SUMMON ) 
             &&( TALENT( ch, TAL_MOTION ) < i 
                 ||IS_SET( ch->in_room->area->flags, AFLAG_NOBYPASS ) 
                 ||IS_SET( ch->in_room->runes, RUNE_WARD ) ) ) 
         ||( IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL ) 
             &&( TALENT( ch, TAL_MOTION ) < i 
                 ||IS_SET( victim->in_room->area->flags, AFLAG_NOBYPASS ) 
                 ||IS_SET( victim->in_room->runes, RUNE_WARD ) ) ) 
         ||( IS_AFFECTED( ch, AFF_DREAMWORLD ) 
             &&IS_SET( victim->in_room->room_flags, ROOM_NO_DREAM ) ) 
         ||( ch != victim && !IS_NPC( ch ) 
             &&TALENT( ch, TAL_SEEKING ) < TALENT( victim, TAL_SECURITY ) 
             &&!IS_NPC( victim ) && IS_SET( victim->pcdata->flags, PCFLAG_NOSUMMON ) ) )
      return FALSE;
   if( ch->curr_talent[TAL_MOTION] > 0 )
      return TRUE;
   return FALSE;
}


/* I was very reluctant to make this command, but I'm sick of
 * doing it manually -keo
 */ 
void do_talent_advance( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   char arg1[MAX_STRING_LENGTH];
   int tal;
   int amt;
   argument = one_argument( argument, arg1 );
   victim = get_char_world( ch, arg1 );
   if( !victim )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   if( !is_number( arg1 ) || !is_number( argument ) )
   {
      ch_printf( ch, "&w%s, %s\n\r", arg1, argument );
      send_to_char( "That's not a number.\n\r", ch );
      return;
   }
   else
   {
      tal = atoi( arg1 );
      amt = atoi( argument );
   }
   if( tal > MAX_DEITY )
   {
      send_to_char( "There is no such talent.\n\r", ch );
      return;
   }
   victim->curr_talent[tal] = amt;
   send_to_char( "You change their magic.\n\r", ch );
   send_to_char( "You feel your magic change.\n\r", victim );
   SET_BIT( victim->pcdata->flags, PCFLAG_CHEAT );
}
void do_deduct( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   int amt;
   char arg2[MAX_STRING_LENGTH];
   argument = one_argument( argument, arg2 );
   victim = get_char_world( ch, arg2 );
   if( !victim )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) || IS_NPC( ch ) )
   {
      send_to_char( "Mobs have no use for talents.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Awarding yourself? How conceited!\n\r", ch );
      return;
   }
   if( !is_number( argument ) )
   {
      send_to_char( "Invalid number of points.\n\r", ch );
      return;
   }
   amt = atoi( argument );
   ch_printf( ch, "You deduct %d talent points from %s.\n\r", amt, victim->name );
   ch_printf( victim, "You lose %d talent points!\n\r", amt );
   victim->pcdata->points -= amt;
}
void do_award( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   int amt;
   char arg2[MAX_STRING_LENGTH];
   argument = one_argument( argument, arg2 );
   victim = get_char_world( ch, arg2 );
   if( !victim )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) || IS_NPC( ch ) )
   {
      send_to_char( "Mobs have no use for talents.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Awarding yourself? How conceited!\n\r", ch );
      return;
   }
   if( !is_number( argument ) )
   {
      send_to_char( "Invalid number of points.\n\r", ch );
      return;
   }
   amt = atoi( argument );
   ch_printf( ch, "You award %s with %d talent points.\n\r", victim->name, amt );
   ch_printf( victim, "You gain %d talent points!\n\r", amt );
   victim->pcdata->points += amt;
}


/* sense use of magic in the room. the more magic being used,
 * the easier it is to detect -keo
 */ 
void sense_magic( CHAR_DATA * ch, int tal, int mana )
{
   CHAR_DATA * rch;
   if( mana < 1 )
      mana = 1;
   if( !ch->in_room )
      return;
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( IS_NPC( rch ) )
         continue;
      if( rch == ch )
         continue;
      if( mana > 1200 - TALENT( ch, TAL_SEEKING ) * 5 )
         ch_printf( rch, "&BYou sense %s magic.\n\r", talent_name[tal] );
   }
}
void learn_talent( CHAR_DATA * ch, int tal )
{
   if( IS_NPC( ch ) )
      return;
   if( ch->curr_talent[tal] < 200 && ch->talent[tal] > 0 )
   {
      ch->pcdata->talent_exp[tal] += ( int )ch->exp / 5;
      ch->exp -= ( int )ch->exp / 5;
      if( ch->pcdata->talent_exp[tal] + talent_exp( ch, ch->curr_talent[tal], tal ) 
           >=talent_exp( ch, ch->curr_talent[tal] + 1, tal ) )
      {
         ch->pcdata->talent_exp[tal] -= ( talent_exp( ch, ch->curr_talent[tal] + 1, tal ) 
                                            -talent_exp( ch, ch->curr_talent[tal], tal ) );
         ch->curr_talent[tal]++;
         update_aris( ch );
         ch_printf( ch, "&BYou feel your skill at %s magic has improved.\n\r", talent_name[tal] );
         if( ch->curr_talent[tal] == 100 )
         {
            ch_printf( ch, "&BYou have mastered the magic of %s.\n\r", talent_name[tal] );
         }
         if( ch->curr_talent[tal] == 200 )
         {
            ch_printf( ch, "&BYou have truly mastered the magic of %s.\n\r", talent_name[tal] );
         }
      }
   }
}
void mana_from_char( CHAR_DATA * ch, int i )
{
   ch->mana -= UMAX( 0, i );
   ch->in_room->area->weather->mana += ( int )i / 100;
   if( ch->mana < -1000 )
   {
      send_to_char( "Your life force is drained into the spell!\n\r", ch );
      lose_hp( ch, i / 10 );
   }
   check_rebirth( ch, i );
}
void use_magic( CHAR_DATA * ch, int tal, int mana )
{
   if( IS_NPC( ch ) )
      return;
   mana -= ch->pcdata->noncombat[SK_CHANNEL];
   if( number_range( 1, 5 ) == 1 )
      learn_noncombat( ch, SK_CHANNEL );
   if( mana < 0 )
      mana = 0;
   learn_talent( ch, tal );
   sense_magic( ch, tal, mana );
   if( ch->pcdata->magiclink )
   {
      mana_from_char( ch->pcdata->magiclink, mana / 2 );
      mana_from_char( ch, mana / 2 );
   }
   else
      mana_from_char( ch, mana );
}
CHAR_DATA * find_target( CHAR_DATA * ch, char *argument, bool offense )
{
   CHAR_DATA * victim = NULL;
   if( !argument || argument[0] == '\0' )
   {
      if( !offense )
         victim = ch;
      
      else if( ch->last_hit && ch->last_hit->in_room ? ch->last_hit->in_room == ch->in_room : FALSE )
         victim = ch->last_hit;
   }
   else
   {
      victim = get_char_room( ch, argument );
   }
   if( !victim || victim == NULL )
   {
      if( !offense && ch->curr_talent[TAL_MOTION] >= 100  &&ch->curr_talent[TAL_SEEKING] >= 100 )
      {
         victim = get_char_world( ch, argument );
         if( !victim )
         {
            send_to_char( "You are unable to find anyone like that.\n\r", ch );
            return NULL;
         }
      }
      else
      {
         send_to_char( "They aren't here.\n\r", ch );
         return NULL;
      }
   }
   if( !IS_SAME_PLANE( ch, victim ) )
   {
      send_to_char( "You can't seem to affect them.\n\r", ch );
      victim = NULL;
   }
   return victim;
}
void magic_damage( CHAR_DATA * victim, CHAR_DATA * ch, int dam, int type, int talent, bool dont_wait )
{
   if( check_manastorm( ch ) )
      return;
   if( !victim || victim == NULL )
   {
      send_to_char( "Attack whom?\n\r", ch );
      return;
   }
   if( char_died( victim ) )
      return;
   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
      return;
   if( !IS_SAME_PLANE( ch, victim ) )
   {
      if( CAN_SEE_PLANE( ch, victim ) )
         send_to_char( "That appears to have no affect.\n\r", ch );
      return;
   }
   if( IS_NPC( ch ) )
      dam = get_curr_int( ch );
   use_magic( ch, talent, dam );
   if( !dont_wait )
   {
      STRFREE( ch->last_taken );
      ch->last_taken = STRALLOC( "channeling" );
      WAIT_STATE( ch, PULSE_VIOLENCE );
   }
   if( victim->shield == type )
   {
      act( AT_MAGIC, "Your $t bounces off $N's shield!", ch, magic_table[type], victim, TO_CHAR );
      act( AT_MAGIC, "$n's $t bounces off $N's shield!", ch, magic_table[type], victim, TO_NOTVICT );
      act( AT_MAGIC, "$n's $t bounces off your shield!", ch, magic_table[type], victim, TO_VICT );
      
         /*
          * don't keep them bouncing back and forth 
          */ 
         if( ch->shield == type )
      {
         act( AT_MAGIC, "Your $t fizzles out harmlessly.", ch, magic_table[type], NULL, TO_CHAR );
         act( AT_MAGIC, "$n's $t fizzles out harmlessly", ch, magic_table[type], NULL, TO_ROOM );
         return;
      }
      
         /*
          * send it back at them 
          */ 
         magic_damage( ch, victim, dam, type, talent, TRUE );
      return;
   }
   if( check_displacement( ch, victim ) )
      return;
   if( check_antimagic( ch, victim ) )
      return;
   dam = number_range( dam, dam * 5 );
   dam = URANGE( 1, dam, 5000 );
   dam = ( 100 - TALENT( victim, talent ) ) * dam / 100;
   if( dam <= 0 )
   {
      act( AT_MAGIC, "The magic has no effect.", victim, NULL, NULL, TO_ROOM );
      act( AT_MAGIC, "The magic has no effect.", victim, NULL, NULL, TO_CHAR );
      if( IS_NPC( ch ) && number_range( 20, 50 ) < get_curr_int( ch ) )
         xSET_BIT( ch->act, ACT_MAG_FAIL );
      return;
   }
   act( AT_MAGIC, "Your $t hits $N!", ch, magic_table[type], victim, TO_CHAR );
   act( AT_MAGIC, "$n's $t hits $N!", ch, magic_table[type], victim, TO_NOTVICT );
   act( AT_MAGIC, "$n's $t hits you!", ch, magic_table[type], victim, TO_VICT );
   victim->last_hit = ch;
   ch->last_hit = victim;
   if( can_gain_exp( ch, victim ) )
      gain_exp( ch, get_exp_worth( victim ) * dam / 100 );
   direct_damage( victim, dam );
   if( char_died( victim ) )
      return;
   pain( victim, dam );
   climate_affect( victim, type );
}
void do_status( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Don't worry about it, you're fine.\n\r", ch );
      return;
   }
   send_to_char( "&BYour current status:\n\r", ch );
   if( ch->pcdata->condition[COND_FULL] == -1 )
      ch_printf( ch, "You don't need to eat.\n\r" );
   
   else if( ch->pcdata->condition[COND_FULL] == 0 )
      ch_printf( ch, "You are starving to death.\n\r" );
   
   else if( ch->pcdata->condition[COND_FULL] < get_full( ch ) / 5 )
      ch_printf( ch, "You are really hungry.\n\r" );
   
   else if( ch->pcdata->condition[COND_FULL] < get_full( ch ) / 4 )
      ch_printf( ch, "You are hungry.\n\r" );
   
   else if( ch->pcdata->condition[COND_FULL] < get_full( ch ) / 3 )
      ch_printf( ch, "You are a mite peckish.\n\r" );
   if( ch->pcdata->condition[COND_THIRST] == -1 )
      ch_printf( ch, "You don't need to drink.\n\r" );
   
   else if( ch->pcdata->condition[COND_THIRST] == 0 )
      ch_printf( ch, "You are dying of thirst.\n\r" );
   
   else if( ch->pcdata->condition[COND_THIRST] < get_full( ch ) / 5 )
      ch_printf( ch, "You are really thirsty.\n\r" );
   
   else if( ch->pcdata->condition[COND_THIRST] < get_full( ch ) / 4 )
      ch_printf( ch, "You are thirsty.\n\r" );
   
   else if( ch->pcdata->condition[COND_THIRST] < get_full( ch ) / 3 )
      ch_printf( ch, "You could use something to drink.\n\r" );
   if( ch->mental_state == 100 )
      ch_printf( ch, "Reality is slipping away.\n\r" );
   
   else if( ch->mental_state > 50 )
      ch_printf( ch, "You are hallucinating.\n\r" );
   
   else if( ch->mental_state > 0 )
      ch_printf( ch, "You are light-headed.\n\r" );
   
   else if( ch->mental_state == -100 )
      ch_printf( ch, "You can't keep your eyes open.\n\r" );
   
   else if( ch->mental_state < -50 )
      ch_printf( ch, "You are extremely drowsy.\n\r" );
   
   else if( ch->mental_state < 0 )
      ch_printf( ch, "You are tired.\n\r" );
   if( ch->pcdata->condition[COND_DRUNK] > 50 )
      ch_printf( ch, "You are drunk.\n\r" );
   
   else if( ch->pcdata->condition[COND_DRUNK] > 10 )
      ch_printf( ch, "You are a bit tipsy.\n\r" );
   if( ch->pcdata->inborn == -1 )
      ch_printf( ch, "You do not have an inborn Talent.\n\r" );
   
   else
      ch_printf( ch, "You have been inborn with the %s Talent.\n\r", talent_name[ch->pcdata->inborn] );
   if( ch->pcdata->consenting )
      ch_printf( ch, "You give consent to %s.\n\r", ch->pcdata->consenting->name );
   if( ch->pcdata->mindlink )
      ch_printf( ch, "You are mind-linked to %s.\n\r", ch->pcdata->mindlink->name );
   if( ch->pcdata->magiclink )
      ch_printf( ch, "Your magic is linked to %s.\n\r", ch->pcdata->magiclink->name );
   if( ch->shield )
      ch_printf( ch, "You are surrounded by a shield of %s.\n\r", magic_table[ch->shield] );
   if( ch->pcdata->were_race )
   {
      if( has_bodypart( ch, BP_RHAND ) )
      {
         ch_printf( ch, "You are a were-%s.\n\r", ch->pcdata->were_race );
      }
      else
      {
         ch_printf( ch, "You are a were-%s.\n\r", ch->species );
      }
   }
   if( ch->pcdata->name_disguise )
      ch_printf( ch, "You are disguised as %s.\n\r", ch->pcdata->name_disguise );
   if( ch->singing )
      ch_printf( ch, "You are singing a song of %s.\n\r", song_name[ch->singing] );
   if( ch->pcdata->wizinvis )
      ch_printf( ch, "Invisibility &C[&W%s&C][&W%d&C]&B\n\r", xIS_SET( ch->act, PLR_WIZINVIS ) ? "*" : " ",
                  ch->pcdata->wizinvis );
}
void do_heal( CHAR_DATA * ch, char *argument )
{
   PART_DATA * part;
   CHAR_DATA * victim;
   int amt;
   if( ch->curr_talent[TAL_HEALING] <= 0 )
   {
      huh( ch );
      return;
   }
   victim = find_target( ch, argument, FALSE );
   if( !victim )
      return;
   if( IS_AFFECTED( victim, AFF_UNDEAD ) )
   {
      magic_damage( victim, ch, TALENT( ch, TAL_HEALING ) * 3  +TALENT( ch, TAL_DEATH ), MAG_DRAIN, TAL_HEALING, FALSE );
      return;
   }
   if( IS_AFFECTED( victim, AFF_CONSTRUCT ) )
   {
      send_to_char( "For some reason, that doesn't work.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      act( AT_MAGIC, "You close your eyes and relax, focusing on healing.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n closes $s eyes and breathes deeply, $s wounds closing.", ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      act( AT_MAGIC, "$n concentrates on you, and pain eases out of your body", ch, NULL, victim, TO_VICT );
      act( AT_MAGIC, "$n concentrates on $N, whose wounds begin to close.", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You concentrate on $N, easing $S pain and healing $S wounds.", ch, NULL, victim, TO_CHAR );
   }
   amt = number_range( TALENT( ch, TAL_HEALING ), TALENT( ch, TAL_HEALING ) * 5 );
   
      /*
       * take mana first so the bonus doesn't cost more 
       */ 
      use_magic( ch, TAL_HEALING, amt );
   if( IS_AFFECTED( ch, AFF_UNHOLY ) && amt != 0 )
      amt /= 2;
   if( IS_AFFECTED( ch, AFF_HOLY ) )
      amt *= 2;
   if( victim == ch )
      amt *= 2;
   if( !IS_FIGHTING( ch ) )
      amt *= 2;
   victim->hit += amt;
   for( part = victim->first_part; part; part = part->next )
   {
      if( part->flags == PART_SEVERED 
           &&part->cond == PART_WELL 
           &&( part->connect_to  ? part->connect_to->flags != PART_SEVERED  : TRUE )  &&amt > 300 )
      {
         act( AT_MAGIC, "Your $t grows back!", victim, part_locs[part->loc], NULL, TO_CHAR );
         act( AT_MAGIC, "$n's $t grows back!", victim, part_locs[part->loc], NULL, TO_ROOM );
         part->flags = PART_WELL;
         return; /* No more healing after a part is regened */
      }
      else
      {
         part->cond = UMIN( PART_WELL, part->cond + amt / 10 );
      }
   }
   if( victim != ch && ch->curr_talent[TAL_DEATH] > 0 )
   {
      amt = number_range( TALENT( ch, TAL_DEATH ), TALENT( ch, TAL_DEATH ) * 5 );
      mana_from_char( victim, amt );
      if( !IS_FIGHTING( ch ) )
         amt *= 2;
      victim->hit += amt;
   }
   if( victim->hit > victim->max_hit )
      victim->hit = victim->max_hit;
   WAIT_STATE( ch, PULSE_VIOLENCE );
}


/* moved from skills.c and modified -keo */ 
void do_recall( CHAR_DATA * ch, char *argument )
{
   int slot;
   int mem;
   ROOM_INDEX_DATA * location;
   if( ch->curr_talent[TAL_MOTION] <= 0 )
   {
      huh( ch );
      return;
   }
   location = NULL;
   if( !argument || argument[0] == '\0' )
      slot = 0;
   
   else
      slot = atoi( argument );
   if( !str_cmp( argument, "nexus" ) )
   {
      if( IS_SET( ch->in_room->area->flags, AFLAG_FREEKILL ) )
         location = get_room_index( ROOM_VNUM_PKILL );
      
      else
         location = get_room_index( ROOM_VNUM_NEXUS );
   }
   else if( !str_cmp( argument, "area" ) )
   {
      location = get_room_index( ch->in_room->area->hi_soft_range );
   }
   else
   {
      mem = ( ch->curr_talent[TAL_MOTION] - 20 ) / 10;
      if( mem > 7 )
         mem = 7;
      if( mem < 0 && ch->curr_talent[TAL_MOTION] > 0 )
         mem = 0;
      if( slot > mem )
      {
         send_to_char( "You cannot recall the details of that location.\n\r", ch );
         return;
      }
      if( IS_NPC( ch ) )
      {
         if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master && !IS_NPC( ch->master ) )
         {
            location = get_room_index( ch->master->pcdata->memorize[slot] );
         }
         else
         {
            return;
         }
      }
      else
      {
         location = get_room_index( ch->pcdata->memorize[slot] );
      }
   }
   if( !location )
      location = get_room_index( ROOM_VNUM_NEXUS );
   if( !location )
      
   {
      send_to_char( "You are completely lost.\n\r", ch );
      return;
   }
   if( ch->in_room == location )
   {
      send_to_char( "You are already here!\n\r", ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_DREAMWORLD )  &&IS_SET( location->room_flags, ROOM_NO_DREAM ) )
   {
      send_to_char( "A mysteriously shield prevents you from recalling.\n\r", ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )  &&ch->curr_talent[TAL_MOTION] < 80 )
   {
      send_to_char( "For some strange reason... nothing happens.\n\r", ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_CURSE )  &&ch->curr_talent[TAL_MOTION] < 70 )
   {
      send_to_char( "You are cursed and cannot recall!\n\r", ch );
      return;
   }
   if( IS_FIGHTING( ch ) && ch->curr_talent[TAL_MOTION] < 60 )
   {
      send_to_char( "You cannot recall from combat!\r\n", ch );
      return;
   }
   use_magic( ch, TAL_MOTION, ch->mount ? 50 : 25 );
   act( AT_ACTION, "$n disappears in a swirl of mist.", ch, NULL, NULL, TO_ROOM );
   char_from_room( ch );
   ch->in_obj = NULL;
   char_to_room( ch, location );
   if( ch->mount )
   {
      char_from_room( ch->mount );
      char_to_room( ch->mount, location );
      do_look( ch->mount, "auto" );
   }
   act( AT_ACTION, "$n appears in the room, a ring of mist dissipating around $m.", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   WAIT_STATE( ch, PULSE_VIOLENCE );
}


/* Each attack command must find its own target so that some of them
 * can optionally do other things to the victim, such as mindflay's
 * affect on mental-state -keo
 */ 
void do_burn( CHAR_DATA * ch, char *argument )
{
   AFFECT_DATA af;
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_FIRE] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_FIRE, "Flames fly from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_FIRE, "Flames fly from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   if( !str_cmp( argument, "room" )  &&ch->curr_talent[TAL_FIRE] >= 60 )
   {
      if( ch->in_room->curr_vegetation < 10 )
      {
         send_to_char( "There is no vegetation here to burn.\n\r", ch );
         return;
      }
      if( ch->in_room->curr_water > 30 )
      {
         send_to_char( "The vegetation is too damp to burn.\n\r", ch );
         return;
      }
      use_magic( ch, TAL_FIRE, 300 );
      act( AT_FIRE, "You set fire to the room!", ch, NULL, NULL, TO_CHAR );
      act( AT_FIRE, "$n sets fire to the room!", ch, NULL, NULL, TO_ROOM );
      SET_BIT( ch->in_room->room_flags, ROOM_BURNING );
learn_talent( ch, TAL_FIRE );      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, IS_UNDERWATER( ch ) ? TALENT( ch, TAL_FIRE ) / 2 : TALENT( ch, TAL_FIRE ), MAG_FIRE,
                   TAL_FIRE, FALSE );
   if( ch->curr_talent[TAL_FIRE] >= 50 && !IS_SET( victim->immune, RIS_FIRE )
         && !( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PACIFIST ) ) )
   {
learn_talent( ch, TAL_FIRE );      af.type = gsn_sticky_flame;
      af.duration = 20;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb( AFF_FLAMING );
      affect_join( victim, &af );
   }
}
void do_poisoncloud( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   AFFECT_DATA af;
   if( ch->curr_talent[TAL_DEATH]  +ch->curr_talent[TAL_WIND] < 200 )
   {
      huh( ch );
      return;
   }
   act( AT_DGREEN, "A cloud of poisonous gas erupts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_DGREEN, "A cloud if poisonous gas erupts from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   for( victim = ch->in_room->first_person; victim; victim = next_vict )
   {
      next_vict = victim->next_in_room;
      magic_damage( victim, ch, ( TALENT( ch, TAL_DEATH )  +TALENT( ch, TAL_WIND ) ), MAG_POISON, TAL_DEATH, FALSE );
      if( char_died( ch ) )
         return;
      if( ch->curr_talent[TAL_DEATH] >= 10 && !IS_SET( victim->immune, RIS_POISON ) )
      {
learn_talent( ch, TAL_DEATH );
learn_talent( ch, TAL_WIND );         af.type = gsn_poison;
         af.duration = 20;
         af.location = APPLY_STR;
         af.modifier = -2;
         af.bitvector = meb( AFF_POISON );
         affect_join( victim, &af );
         victim->mental_state = URANGE( 20, victim->mental_state, 100 );
      }
   }
}
void do_quake( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   if( ( ch->curr_talent[TAL_EARTH] < 20 && !IS_NPC( ch ) )  ||IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) )
   {
      huh( ch );
      return;
   }
   act( AT_HUNGRY, "The earth trembles at your command!", ch, NULL, NULL, TO_CHAR );
   act( AT_HUNGRY, "The ground begins to tremble and shake!", ch, NULL, NULL, TO_ROOM );
   for( victim = ch->in_room->first_person; victim; victim = next_vict )
   {
      next_vict = victim->next_in_room;
      if( IS_AFFECTED( victim, AFF_FLYING )  &&!IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
         continue;
      magic_damage( victim, ch, TALENT( ch, TAL_EARTH ), MAG_BLUNT, TAL_EARTH, FALSE );
learn_talent( ch, TAL_EARTH );      if( char_died( ch ) )
         return;
   }
}
void do_telekinetics( CHAR_DATA * ch, char *argument )
{
   int n;
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   if( !IS_NPC( ch ) && ( ch->curr_talent[TAL_MOTION] < 20  ||ch->curr_talent[TAL_MIND] < 10 ) )
   {
      huh( ch );
      return;
   }
   if( !str_cmp( argument, "all" ) )
   {
      for( victim = ch->in_room->first_person; victim; victim = next_vict )
         
      {
         next_vict = victim->next_in_room;
         n++;
         if( victim == ch )
            continue;
         learn_talent( ch, TAL_MIND );
         magic_damage( victim, ch, 
                        ( ch->curr_talent[TAL_MIND] + ch->curr_talent[TAL_MOTION] ) / 2, MAG_PSIONIC, TAL_MOTION, FALSE );
      }
      use_magic( ch, TAL_MOTION, n * TALENT( ch, TAL_MOTION ) );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_talent( ch, TAL_MIND );
   magic_damage( victim, ch, TALENT( ch, TAL_MIND ) + TALENT( ch, TAL_MOTION ), MAG_PSIONIC, TAL_MOTION, FALSE );
}
void do_sandstorm( CHAR_DATA * ch, char *argument )
{
   AFFECT_DATA af;
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   int n = 0;
   if( ch->curr_talent[TAL_WIND] < 1 && ch->curr_talent[TAL_EARTH] < 10 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_ORANGE, "Blasting sand swirls from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_ORANGE, "Blasting sand swirls from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   if( !str_cmp( argument, "all" ) )
   {
      for( victim = ch->in_room->first_person; victim; victim = next_vict )
         
      {
         next_vict = victim->next_in_room;
         n++;
         if( victim == ch )
            continue;
         learn_talent( ch, TAL_EARTH );
         magic_damage( victim, ch, ( TALENT( ch, TAL_WIND ) + TALENT( ch, TAL_EARTH ) ) / 2, MAG_WIND, TAL_WIND, FALSE );
      }
      use_magic( ch, TAL_WIND, n * TALENT( ch, TAL_WIND ) );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_talent( ch, TAL_EARTH );
   magic_damage( victim, ch, TALENT( ch, TAL_EARTH ) + TALENT( ch, TAL_WIND ), MAG_WIND, TAL_WIND, FALSE );
   if( number_range( 50, 300 ) < TALENT( ch, TAL_EARTH ) )
   {
      af.type = gsn_blindness;
      af.duration = 20;
      af.location = APPLY_HITROLL;
      af.modifier = -2;
      af.bitvector = meb( AFF_BLIND );
      affect_join( victim, &af );
   }
}
void do_magma( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   int n = 0;
   if( ( ch->curr_talent[TAL_EARTH] < 10  ||ch->curr_talent[TAL_FIRE] < 10 ) && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_FIRE, "Molten lava pours from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_FIRE, "Molten lava pours from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   if( !str_cmp( argument, "all" ) )
   {
      for( victim = ch->in_room->first_person; victim; victim = next_vict )
         
      {
         next_vict = victim->next_in_room;
         n++;
         if( victim == ch )
            continue;
         learn_talent( ch, TAL_FIRE );
         magic_damage( victim, ch, ( TALENT( ch, TAL_FIRE ) + TALENT( ch, TAL_EARTH ) ) / 2, MAG_FIRE, TAL_EARTH, FALSE );
      }
      use_magic( ch, TAL_FIRE, n * TALENT( ch, TAL_FIRE ) );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_talent( ch, TAL_EARTH );
   magic_damage( victim, ch, TALENT( ch, TAL_EARTH ) + TALENT( ch, TAL_FIRE ), MAG_FIRE, TAL_FIRE, FALSE );
}
void do_flare( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   int n = 0;
   if( ch->curr_talent[TAL_ILLUSION] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_YELLOW, "Searing light radiates from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_YELLOW, "Searing light radiates from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   if( !str_cmp( argument, "all" ) )
   {
      for( victim = ch->in_room->first_person; victim; victim = next_vict )
         
      {
         next_vict = victim->next_in_room;
         n++;
         if( victim == ch )
            continue;
         learn_talent( ch, TAL_CATALYSM );
         magic_damage( victim, ch, 
                        ( TALENT( ch, TAL_CATALYSM ) + TALENT( ch, TAL_ILLUSION ) ) / 2, MAG_LIGHT, TAL_ILLUSION, FALSE );
      }
      use_magic( ch, TAL_ILLUSION, n * TALENT( ch, TAL_ILLUSION ) );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_talent( ch, TAL_CATALYSM );
   magic_damage( victim, ch, TALENT( ch, TAL_CATALYSM ) + TALENT( ch, TAL_ILLUSION ), MAG_LIGHT, TAL_ILLUSION, FALSE );
}
void do_antimatter( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_VOID] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_DGREY, "Crackling black antimatter streams from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_DGREY, "Crackling black antimatter streams from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, TALENT( ch, TAL_VOID ), MAG_ANTIMATTER, TAL_VOID, FALSE );
}
void do_chainlightning( CHAR_DATA * ch, char *argument )
{
   int zaps;
   CHAR_DATA * victim = NULL;
   CHAR_DATA * next_vict = NULL;
   if( ch->curr_talent[TAL_LIGHTNING] < 50 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_YELLOW, "Chain lightning streaks from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_YELLOW, "Chain lightning streaks from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   zaps = number_range( 1, TALENT( ch, TAL_LIGHTNING ) / 25 );
   if( ch->in_room->curr_water > 60 )
      zaps *= 2;
   use_magic( ch, TAL_LIGHTNING, zaps * 100 );
   while( zaps-- )
   {
      victim = next_vict;
      if( !next_vict )
         victim = ch->in_room->first_person;
      next_vict = victim->next_in_room;
      magic_damage( victim, ch, 100, MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
   }
}
void do_electrocute( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_LIGHTNING]  +ch->curr_talent[TAL_WIND] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) 
         ||ch->in_room->area->weather->precip < 100  ||ch->in_room->area->weather->temp < 100 )
   {
      send_to_char( "The weather isn't stormy enough here to call lightning.\n\r", ch );
      return;
   }
   act( AT_YELLOW, "A dozen bolts of electricity merge in the sky and shoot down a powerful bolt!", ch, NULL, NULL,
         TO_CHAR );
   act( AT_YELLOW, "A dozen bolts of electricity merge in the sky and shoot down a powerful bolt!", ch, NULL, NULL,
         TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, TALENT( ch, TAL_WIND ) + TALENT( ch, TAL_LIGHTNING ) * 5, MAG_ELECTRICITY, TAL_LIGHTNING,
                   FALSE );
}
void do_flamingrain( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   if( ch->curr_talent[TAL_FIRE]  +ch->curr_talent[TAL_WIND] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
   {
      send_to_char( "You can't call flaming rain indoors.\n\r", ch );
      return;
   }
   act( AT_FIRE, "The sky turns red as fire rains from the heavens.", ch, NULL, NULL, TO_CHAR );
   act( AT_FIRE, "The sky turns red as fire rains from the heavens.", ch, NULL, NULL, TO_ROOM );
   for( victim = ch->in_room->first_person; victim; victim = next_vict )
   {
      next_vict = victim->next_in_room;
      magic_damage( victim, ch, TALENT( ch, TAL_WIND ) + TALENT( ch, TAL_FIRE ) + ch->in_room->area->weather->temp,
                      MAG_FIRE, TAL_FIRE, FALSE );
   }
}
void do_manastorm( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   if( ch->curr_talent[TAL_CATALYSM]  +ch->curr_talent[TAL_WIND] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
   {
      send_to_char( "You can't call a mana-storm indoors.\n\r", ch );
      return;
   }
   act( AT_PURPLE, "You make the energy rage in a mana-storm!", ch, NULL, NULL, TO_CHAR );
   act( AT_PURPLE, "The energy in the air rages in a mana-storm!", ch, NULL, NULL, TO_ROOM );
   for( victim = ch->in_room->first_person; victim; victim = next_vict )
   {
      next_vict = victim->next_in_room;
      magic_damage( victim, ch, TALENT( ch, TAL_WIND ) + TALENT( ch, TAL_CATALYSM ) + ch->in_room->area->weather->mana,
                      MAG_ENERGY, TAL_CATALYSM, FALSE );
   }
}
void do_meteor( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_EARTH]  +ch->curr_talent[TAL_VOID] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
   {
      send_to_char( "You can't call a meteor indoors.\n\r", ch );
      return;
   }
   act( AT_DGREY, "A meteor streaks from the sky!", ch, NULL, NULL, TO_CHAR );
   act( AT_DGREY, "A meteor streaks from the sky!", ch, NULL, NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, ( TALENT( ch, TAL_EARTH ) + TALENT( ch, TAL_VOID ) ) * 2, MAG_BLUNT, TAL_EARTH, FALSE );
}
void do_sonics( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   if( ch->curr_talent[TAL_SPEECH] < 100 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( ch->in_room->curr_water >= 100 )
   {
      send_to_char( "There is not enough air here.\n\r", ch );
      return;
   }
   act( AT_YELLOW, "You make the air vibrate loudly!", ch, NULL, NULL, TO_CHAR );
   act( AT_YELLOW, "The very air seems to hum loudly!", ch, NULL, NULL, TO_ROOM );
   for( victim = ch->in_room->first_person; victim; victim = next_vict )
   {
      next_vict = victim->next_in_room;
      learn_talent( ch, TAL_SPEECH );
      magic_damage( victim, ch, TALENT( ch, TAL_SPEECH ) + TALENT( ch, TAL_MOTION ) + TALENT( ch, TAL_WIND ), MAG_SOUND,
                     TAL_SPEECH, FALSE );
   }
}
void do_steam( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_WATER]  +ch->curr_talent[TAL_FIRE] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_BLUE, "A burst of steam blasts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_BLUE, "A burst of steam blasts from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_talent( ch, TAL_FIRE );
   magic_damage( victim, ch, 
                  ( TALENT( ch, TAL_FIRE ) + TALENT( ch, TAL_WATER ) ) * 
                  ( IS_UNDERWATER( ch ) ? 2 : 1 ), MAG_FIRE, TAL_WATER, FALSE );
}
void do_blizzard( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   if( ch->curr_talent[TAL_FROST]  +ch->curr_talent[TAL_WIND] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) 
         ||ch->in_room->area->weather->precip < 50  ||ch->in_room->area->weather->temp > -100 )
   {
      send_to_char( "The weather isn't right here to call a blizzard.\n\r", ch );
      return;
   }
   act( AT_FROST, "A swirling blizzard blasts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_FROST, "A swirling blizzard blasts from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   for( victim = ch->in_room->first_person; victim; victim = next_vict )
   {
      next_vict = victim->next_in_room;
      learn_talent( ch, TAL_WIND );
      magic_damage( victim, ch, ( TALENT( ch, TAL_WIND ) + TALENT( ch, TAL_FROST ) ) * 2, MAG_COLD, TAL_FROST, FALSE );
   }
}
void do_sunstroke( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_ILLUSION]  +ch->curr_talent[TAL_FIRE] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) 
         ||ch->in_room->area->weather->precip > 0 
         ||time_info.hour < 6 || time_info.hour >= 18  ||get_light_room( ch->in_room ) < 0 )
   {
      send_to_char( "There isn't enough sunlight here.\n\r", ch );
      return;
   }
   act( AT_YELLOW, "Your $t beam a ray of searing sunlight!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_YELLOW, "$n's $t beam a ray of searing sunlight!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_talent( ch, TAL_FIRE );
   magic_damage( victim, ch, 
                  ( TALENT( ch, TAL_ILLUSION ) + TALENT( ch, TAL_FIRE ) ) * 2, MAG_LIGHT, TAL_ILLUSION, FALSE );
}
void do_soulfire( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_DEATH]  +ch->curr_talent[TAL_FIRE] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_DGREY, "Black fire streaks from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_DGREY, "Black fire streaks from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( !IS_AFFECTED( victim, AFF_UNHOLY ) 
         &&!IS_AFFECTED( victim, AFF_UNDEAD )  &&!IS_AFFECTED( victim, AFF_NONLIVING ) )
   {
      learn_talent( ch, TAL_FIRE );
      magic_damage( victim, ch, 
                     ( TALENT( ch, TAL_DEATH ) + TALENT( ch, TAL_FIRE ) ) * 1.5, MAG_UNHOLY, TAL_DEATH, FALSE );
   }
   else
   {
      act( AT_DGREY, "$N is unaffected.", ch, NULL, victim, TO_CHAR );
   }
}
void do_unholyblast( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_DEATH]  +ch->curr_talent[TAL_SECURITY] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_DGREY, "Unholy energy blasts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_DGREY, "Unholy energy blasts from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( IS_AFFECTED( victim, AFF_HOLY )  ||IS_AFFECTED( victim, AFF_ANGEL ) )
   {
      learn_talent( ch, TAL_SECURITY );
      magic_damage( victim, ch, 
                     ( TALENT( ch, TAL_DEATH ) + TALENT( ch, TAL_SECURITY ) ) * 2, MAG_UNHOLY, TAL_DEATH, FALSE );
   }
   else
   {
      act( AT_DGREY, "$N is unaffected.", ch, NULL, victim, TO_CHAR );
   }
}
void do_sworddance( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( IS_NPC( ch ) || ch->curr_talent[TAL_TIME] + ch->curr_talent[TAL_VOID] 
         +ch->curr_talent[TAL_CATALYSM] + ch->pcdata->weapon[SK_SWORD] < 200 )
   {
      huh( ch );
      return;
   }
   if( !ch->main_hand || ch->main_hand->value[5] != SK_SWORD )
   {
      send_to_char( "You must be holding a sword in your main hand to use the Sword Dance.\n\r", ch );
      return;
   }
   act( AT_WHITE, "You dance with the sword, and a dozen other swords dance with you.", ch, NULL, NULL, TO_CHAR );
   act( AT_WHITE, "$n dances, a dozen whirling swords slashing around $n!", ch, NULL, NULL, TO_ROOM );
   lose_ep( ch, 100 );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, ch->main_hand->value[2] + ch->pcdata->weapon[SK_SWORD] 
                   +ch->curr_talent[TAL_CATALYSM], MAG_SLASH, TAL_CATALYSM, FALSE );
   learn_talent( ch, TAL_VOID );
   learn_talent( ch, TAL_TIME );
   learn_weapon( ch, SK_SWORD );
}
void do_pyrokinetics( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_MIND]  +ch->curr_talent[TAL_FIRE] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_BLUE, "Blazing blue psionic flames streak from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_BLUE, "Blazing blue psionic flames streak from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( IS_AFFECTED( victim, AFF_CONSTRUCT )  ||IS_AFFECTED( victim, AFF_UNDEAD ) )
   {
      magic_damage( victim, ch, TALENT( ch, TAL_FIRE ), MAG_FIRE, TAL_FIRE, FALSE );
   }
   else
   {
      learn_talent( ch, TAL_FIRE );
      magic_damage( victim, ch, ( TALENT( ch, TAL_MIND ) + TALENT( ch, TAL_FIRE ) ) * 2, MAG_PSIONIC, TAL_MIND, FALSE );
      learn_talent( ch, TAL_FIRE );
   }
}
void do_geyser( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_FIRE] + ch->curr_talent[TAL_WATER]  +ch->curr_talent[TAL_EARTH] < 100 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) )
   {
      send_to_char( "There is no ground to make a geyser erupt from here.\n\r", ch );
      return;
   }
   act( AT_BLUE, "A geyser erupts from the ground!", ch, NULL, NULL, TO_CHAR );
   act( AT_BLUE, "A geyser erupts from the ground!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_talent( ch, TAL_FIRE );
   learn_talent( ch, TAL_EARTH );
   magic_damage( victim, ch, TALENT( ch, TAL_EARTH ) + TALENT( ch, TAL_WATER ) + TALENT( ch, TAL_FIRE ), MAG_WATER,
                  TAL_WATER, FALSE );
}
void do_brainshock( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_MIND]  +ch->curr_talent[TAL_LIGHTNING] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_YELLOW, "A surge of psionic lightning bursts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_YELLOW, "A surge of psionic lightning bursts from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( IS_AFFECTED( victim, AFF_CONSTRUCT )  ||IS_AFFECTED( victim, AFF_UNDEAD ) )
   {
      magic_damage( victim, ch, TALENT( ch, TAL_LIGHTNING ), MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
   }
   else
   {
      learn_talent( ch, TAL_LIGHTNING );
      magic_damage( victim, ch, 
                     ( TALENT( ch, TAL_MIND ) + TALENT( ch, TAL_LIGHTNING ) ) * 2, MAG_ELECTRICITY, TAL_MIND, FALSE );
   }
}
void do_smite( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_HEALING] + ch->curr_talent[TAL_LIGHTNING] 
         +ch->curr_talent[TAL_SECURITY] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_WHITE, "Holy lightning erupts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_WHITE, "Holy lightning erupts from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( IS_AFFECTED( victim, AFF_UNHOLY )  ||IS_AFFECTED( victim, AFF_UNDEAD )  ||IS_AFFECTED( victim, AFF_DEMON ) )
   {
      magic_damage( victim, ch, TALENT( ch, TAL_HEALING ) * 3 + ( TALENT( ch, TAL_LIGHTNING ) 
                                                                    +TALENT( ch, TAL_SECURITY ) ) * 2, MAG_HOLY,
                     TAL_HEALING, FALSE );
      learn_talent( ch, TAL_LIGHTNING );
      learn_talent( ch, TAL_SECURITY );
   }
   else
   {
      magic_damage( victim, ch, TALENT( ch, TAL_LIGHTNING ) * 2, MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
   }
}
void do_holyblast( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_HEALING]  +ch->curr_talent[TAL_SECURITY] < 200 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_WHITE, "Holy energy blasts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_WHITE, "Holy energy blasts from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( IS_AFFECTED( victim, AFF_UNHOLY )  ||IS_AFFECTED( victim, AFF_UNDEAD )  ||IS_AFFECTED( victim, AFF_DEMON ) )
   {
      magic_damage( victim, ch, 
                     ( TALENT( ch, TAL_HEALING ) + TALENT( ch, TAL_SECURITY ) ) * 2, MAG_HOLY, TAL_HEALING, FALSE );
      learn_talent( ch, TAL_SECURITY );
   }
   else
   {
      act( AT_WHITE, "$N is unaffected.", ch, NULL, victim, TO_CHAR );
   }
}
void do_lightning( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA * bullet;
   int amt;
   if( ch->curr_talent[TAL_LIGHTNING] < 30 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_YELLOW, "A bolt of lightning flies from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_YELLOW, "A bolt of lightning flies from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   bullet = create_object( get_obj_index( 50 ), 0 );   /* blast */
   if( !bullet )
   {
      send_to_char( "Your weapon jams mysteriously.\n\r", ch );
      bug( "Do_shoot: Cannot find type for projectile.", 0 );
      return;
   }
   amt = number_range( TALENT( ch, TAL_LIGHTNING ), TALENT( ch, TAL_LIGHTNING ) * 5 );
   use_magic( ch, TAL_LIGHTNING, amt );
   bullet->value[2] = amt;
   ranged_attack( ch, bullet, argument );
}
void do_chill( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_FROST] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_FROST, "Frigid snow flies from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_FROST, "Frigid snow flies from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   learn_talent( ch, TAL_WATER );
   magic_damage( victim, ch, TALENT( ch, TAL_FROST ) + TALENT( ch, TAL_WATER ), MAG_COLD, TAL_FROST, FALSE );
}
void do_frostbite( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( IS_NPC( ch ) || ch->curr_talent[TAL_FROST]  +ch->pcdata->weapon[SK_FANG] < 150 )
   {
      huh( ch );
      return;
   }
   act( AT_FROST, "You snap your teeth with the chill of frost!", ch, NULL, NULL, TO_CHAR );
   act( AT_FROST, "$n snaps $s glistening icy teeth!", ch, NULL, NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   learn_weapon( ch, SK_FANG );
   magic_damage( victim, ch, ( TALENT( ch, TAL_FROST ) + ch->pcdata->weapon[SK_FANG] ) * 2, MAG_COLD, TAL_FROST, FALSE );
}
void do_taint( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   AFFECT_DATA af;
   if( ch->curr_talent[TAL_DEATH] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_DGREEN, "Noxious fumes burst from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_DGREEN, "Noxious fumes burst from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( ch->curr_talent[TAL_DEATH] >= 10 && !IS_SET( victim->immune, RIS_POISON ) )
   {
      af.type = gsn_poison;
      af.duration = 20;
      af.location = APPLY_STR;
      af.modifier = -2;
      af.bitvector = meb( AFF_POISON );
      affect_join( victim, &af );
      victim->mental_state = URANGE( 20, victim->mental_state, 100 );
   }
   magic_damage( victim, ch, TALENT( ch, TAL_DEATH ), MAG_POISON, TAL_DEATH, FALSE );
}
void do_splash( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_WATER] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( ch->curr_talent[TAL_EARTH] < 1 )
   {
      act( AT_BLUE, "Water shoots from your $t!", ch, hands( ch ), NULL, TO_CHAR );
      act( AT_BLUE, "Water shoots fly from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   }
   else
   {
      act( AT_GREEN, "Acid sprays from your $t!", ch, hands( ch ), NULL, TO_CHAR );
      act( AT_GREEN, "Acid sprays from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( ch->curr_talent[TAL_EARTH] < 1 )
      magic_damage( victim, ch, TALENT( ch, TAL_WATER ), MAG_WATER, TAL_WATER, FALSE );
   
   else
      magic_damage( victim, ch, TALENT( ch, TAL_EARTH ) + TALENT( ch, TAL_WATER ), MAG_ACID, TAL_WATER, FALSE );
}
void do_distort( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_CHANGE] > 0  ||ch->curr_talent[TAL_VOID] > 0  ||ch->curr_talent[TAL_TIME] > 0 )
   {
      act( AT_FIRE, "Ripples of distortion radiate from your $t!", ch, hands( ch ), NULL, TO_CHAR );
      act( AT_FIRE, "Ripples of distortion radiate from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( ch->curr_talent[TAL_CHANGE] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_CHANGE ), MAG_CHANGE, TAL_CHANGE, FALSE );
   if( ch->curr_talent[TAL_TIME] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_TIME ), MAG_CHANGE, TAL_TIME, FALSE );
   if( ch->curr_talent[TAL_VOID] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_VOID ), MAG_CHANGE, TAL_VOID, FALSE );
}
void do_nightmare( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
   {
      send_to_char( "Only in the dreamworld.\n\r", ch );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, 2 * ( TALENT( ch, TAL_DREAM ) + TALENT( ch, TAL_MIND ) + TALENT( ch, TAL_ILLUSION ) ),
                   MAG_DREAM, TAL_DREAM, FALSE );
}
void do_wail( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * next_vict;
   CHAR_DATA * victim;
   int n = 0;
   if( IS_SET( ch->in_room->room_flags, ROOM_SILENCE )  ||( ch->curr_talent[TAL_SPEECH] < 1 && !IS_NPC( ch ) ) )
   {
      huh( ch );
      return;
   }
   act( AT_YELLOW, "You emit a high pitched wail!", ch, NULL, NULL, TO_CHAR );
   act( AT_YELLOW, "$n emits a high pitched wail!", ch, NULL, NULL, TO_ROOM );
   if( !str_cmp( argument, "all" ) )
   {
      for( victim = ch->in_room->first_person; victim; victim = next_vict )
         
      {
         next_vict = victim->next_in_room;
         n++;
         if( victim == ch )
            continue;
         magic_damage( victim, ch, TALENT( ch, TAL_SPEECH ), MAG_SOUND, TAL_SPEECH, FALSE );
      }
      use_magic( ch, TAL_SPEECH, n * TALENT( ch, TAL_SPEECH ) );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, TALENT( ch, TAL_SPEECH ), MAG_SOUND, TAL_SPEECH, FALSE );
}
void do_shock( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_LIGHTNING] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_YELLOW, "Lightning crackles from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_YELLOW, "Lightning crackles from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( IS_UNDERWATER( ch ) || ch->in_room->curr_water > 30 )
   {
      act( AT_YELLOW, "The electricity surges alarmingly through the water!", ch, NULL, NULL, TO_CHAR );
      act( AT_YELLOW, "$n's electricity surges alarmingly through the water!", ch, NULL, NULL, TO_ROOM );
      if( !IS_NPC( ch ) )
         magic_damage( ch, ch, TALENT( ch, TAL_LIGHTNING ), MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
      magic_damage( victim, ch, TALENT( ch, TAL_LIGHTNING ) * 2, MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
   }
   else
   {
      magic_damage( victim, ch, TALENT( ch, TAL_LIGHTNING ), MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
   }
}


/* creates a storm from a hybrid of wind and whichever talents of
 * fire, water, frost, lightning, or catalysm that they have -keo
 */ 
void do_storm( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   int last_attack;
   int n = 0;
   if( ch->curr_talent[TAL_WIND] < 10 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( ch->in_room->curr_water >= 100 )
   {
      act( AT_CYAN, "There isn't enough air here.", ch, NULL, NULL, TO_CHAR );
      act( AT_CYAN, "Some air bubbles float up toward the surface.", ch, NULL, NULL, TO_ROOM );
      return;
   }
   act( AT_CYAN, "An elemental storm rages around you!", ch, NULL, NULL, TO_CHAR );
   act( AT_CYAN, "An elemental storm rages around $n!", ch, NULL, NULL, TO_ROOM );
   if( !str_cmp( argument, "all" ) )
   {
      last_attack = 0;
      for( victim = ch->in_room->first_person; victim; victim = next_vict )
         
      {
         next_vict = victim->next_in_room;
         n++;
         if( victim == ch )
            continue;
         if( last_attack == TAL_FIRE )
         {
            last_attack = TAL_WIND;
            magic_damage( victim, ch, TALENT( ch, TAL_WIND ), MAG_WIND, TAL_WIND, FALSE );
            continue;
         }
         if( last_attack == TAL_WIND )
         {
            last_attack = TAL_WATER;
            if( ch->curr_talent[TAL_WATER] > 0 )
            {
               magic_damage( victim, ch, TALENT( ch, TAL_WATER ), MAG_WATER, TAL_WATER, FALSE );
               continue;
            }
         }
         if( last_attack == TAL_WATER )
         {
            last_attack = TAL_FROST;
            if( ch->curr_talent[TAL_FROST] > 0 )
            {
               magic_damage( victim, ch, TALENT( ch, TAL_FROST ), MAG_COLD, TAL_FROST, FALSE );
               continue;
            }
         }
         if( last_attack == TAL_FROST )
         {
            last_attack = TAL_LIGHTNING;
            if( ch->curr_talent[TAL_LIGHTNING] > 0 )
            {
               magic_damage( victim, ch, TALENT( ch, TAL_LIGHTNING ), MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
               continue;
            }
         }
         if( last_attack == TAL_LIGHTNING )
         {
            last_attack = TAL_CATALYSM;
            if( ch->curr_talent[TAL_CATALYSM] > 0 )
            {
               magic_damage( victim, ch, TALENT( ch, TAL_CATALYSM ), MAG_ENERGY, TAL_CATALYSM, FALSE );
               continue;
            }
         }
         if( last_attack == TAL_CATALYSM )
         {
            last_attack = TAL_FIRE;
            if( ch->curr_talent[TAL_FIRE] > 0 )
            {
               magic_damage( victim, ch, TALENT( ch, TAL_FIRE ), MAG_FIRE, TAL_FIRE, FALSE );
               continue;
            }
         }
         if( last_attack == TAL_FIRE )
         {
            last_attack = TAL_WIND;
            magic_damage( victim, ch, TALENT( ch, TAL_WIND ), MAG_WIND, TAL_WIND, FALSE );
            continue;
         }
      }
      use_magic( ch, TAL_WIND, n * TALENT( ch, TAL_WIND ) );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, TALENT( ch, TAL_WIND ), MAG_WIND, TAL_WIND, FALSE );
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_WATER] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_WATER ), MAG_WATER, TAL_WATER, FALSE );
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_FROST] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_FROST ), MAG_COLD, TAL_FROST, FALSE );
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_LIGHTNING] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_LIGHTNING ), MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_FIRE] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_FIRE ), MAG_FIRE, TAL_FIRE, FALSE );
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_CATALYSM] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_WIND ), MAG_ENERGY, TAL_CATALYSM, FALSE );
}
void do_wind( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_WIND] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( ch->in_room->curr_water >= 100 )
   {
      act( AT_CYAN, "There isn't enough air here.", ch, NULL, NULL, TO_CHAR );
      act( AT_CYAN, "Some air bubbles float up toward the surface.", ch, NULL, NULL, TO_ROOM );
      return;
   }
   act( AT_CYAN, "Strong winds blow from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_CYAN, "Strong winds blow from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, 
                   ( TALENT( ch, TAL_WIND ) * 2 + ch->in_room->area->weather->wind ) / 3, MAG_WIND, TAL_WIND, FALSE );
}
void do_vines( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_EARTH] < 1 && !IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   act( AT_GREEN, "Thorny vines coil like snakes!", ch, NULL, NULL, TO_CHAR );
   act( AT_GREEN, "Thorny vines coil like snakes!", ch, NULL, NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, 
                   ( TALENT( ch, TAL_EARTH ) * 2 + ch->in_room->curr_vegetation ) / 3, MAG_EARTH, TAL_EARTH, FALSE );
}


/* a destructive blast of fire, lightning, and/or catalysm - keo */ 
void do_blast( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_CATALYSM] > 0  ||ch->curr_talent[TAL_FIRE] > 0  ||ch->curr_talent[TAL_LIGHTNING] > 0 )
   {
      act( AT_FIRE, "A blast of energy bursts from your $t!", ch, hands( ch ), NULL, TO_CHAR );
      act( AT_FIRE, "A blast of energy bursts from $s $t!", ch, hands( ch ), NULL, TO_ROOM );
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( ch->curr_talent[TAL_CATALYSM] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_CATALYSM ), MAG_ENERGY, TAL_CATALYSM, FALSE );
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_FIRE] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_FIRE ), MAG_FIRE, TAL_FIRE, FALSE );
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_LIGHTNING] > 0 )
      magic_damage( victim, ch, TALENT( ch, TAL_LIGHTNING ), MAG_ELECTRICITY, TAL_LIGHTNING, FALSE );
}
void do_mindflay( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   act( AT_PURPLE, "You concentrate.", ch, NULL, NULL, TO_CHAR );
   act( AT_PURPLE, "$n concentrates.", ch, NULL, NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( ( ch->curr_talent[TAL_MIND] > 40 || IS_NPC( ch ) )  &&!IS_SET( victim->resistant, RIS_PSIONIC ) )
      victim->mental_state += number_range( 0, ch->curr_talent[13] / 10 );
   if( ch->curr_talent[TAL_MIND] >= 1 )
      magic_damage( victim, ch, TALENT( ch, TAL_MIND ) + victim->perm_int - victim->perm_wil, MAG_PSIONIC, TAL_MIND,
                     FALSE );
   if( ch->curr_talent[TAL_DREAM] >= 1 )
      magic_damage( victim, ch, TALENT( ch, TAL_DREAM ), MAG_DREAM, TAL_DREAM, FALSE );
   if( ch->curr_talent[TAL_ILLUSION] >= 1 )
      magic_damage( victim, ch, TALENT( ch, TAL_ILLUSION ), MAG_LIGHT, TAL_ILLUSION, FALSE );
}
void do_flood( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CHAR_DATA * next_vict;
   int n = 0;
   if( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR )  ||( ch->curr_talent[TAL_WATER] < 1 && !IS_NPC( ch ) ) )
   {
      huh( ch );
      return;
   }
   if( ch->curr_talent[TAL_WATER] > 10  &&ch->in_room->curr_water < 100 )
   {
      act( AT_BLUE, "Water floods the room!", ch, NULL, NULL, TO_CHAR );
      act( AT_BLUE, "Water floods the room!", ch, NULL, NULL, TO_ROOM );
      //if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
         ch->in_room->curr_water += TALENT( ch, TAL_WATER ) / 10;
   }
   if( !str_cmp( argument, "all" ) )
   {
      for( victim = ch->in_room->first_person; victim; victim = next_vict )
         
      {
         next_vict = victim->next_in_room;
         n++;
         if( victim == ch )
            continue;
         magic_damage( victim, ch, TALENT( ch, TAL_WATER ), MAG_WATER, TAL_WATER, FALSE );
      }
      use_magic( ch, TAL_WATER, n * TALENT( ch, TAL_WATER ) );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   magic_damage( victim, ch, 
                   ( TALENT( ch, TAL_WATER ) * 2 + ch->in_room->curr_water ) / 3, MAG_WATER, TAL_WATER, FALSE );
}


/* if someone has both death and catalysm, they will end up with
 * a double energy drain. -keo
 */ 
void do_drain( CHAR_DATA * ch, char *argument )
{
   PART_DATA * part;
   CHAR_DATA * victim;
   int amt;
   if( ch->curr_talent[TAL_DEATH] < 1 && !IS_NPC( ch ) 
         &&ch->curr_talent[TAL_CATALYSM] < 1  &&ch->curr_talent[TAL_HEALING] < 1 )
   {
      huh( ch );
      return;
   }
   act( AT_DGREY, "Tendrils of darkness surround you.", ch, NULL, NULL, TO_CHAR );
   act( AT_DGREY, "Tendrils of darkness surround $n.", ch, NULL, NULL, TO_ROOM );
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( IS_AFFECTED( victim, AFF_UNDEAD ) )
   {
      if( ch != victim && IS_FIGHTING( victim ) )
      {
         magic_damage( ch, victim, TALENT( ch, TAL_DEATH ) + TALENT( ch, TAL_HEALING ), MAG_DRAIN, TAL_DEATH, TRUE );
      }
      amt = number_range( TALENT( ch, TAL_DEATH ) + TALENT( ch, TAL_HEALING ), 
                           ( TALENT( ch, TAL_DEATH ) + TALENT( ch, TAL_HEALING ) ) * 10 );
      use_magic( ch, TAL_DEATH, amt );
      learn_talent( ch, TAL_HEALING );
      if( !IS_SET( ch->immune, RIS_DRAIN )  ||ch == victim )
      {
         act( AT_MAGIC, "Your undead flesh repairs itself.", victim, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n's undead flesh repairs itself.", victim, NULL, NULL, TO_ROOM );
         if( IS_AFFECTED( ch, AFF_UNHOLY ) )
            amt *= 2;
         if( IS_AFFECTED( ch, AFF_HOLY ) && amt != 0 )
            amt /= 2;
         victim->hit += amt;
         if( victim->hit > victim->max_hit )
            victim->hit = victim->max_hit;
         for( part = victim->first_part; part; part = part->next )
         {
            if( part->flags == PART_SEVERED 
                 &&part->cond == PART_WELL 
                 &&( part->connect_to  ? part->connect_to->flags != PART_SEVERED  : TRUE )  &&amt > 300 )
            {
               act( AT_MAGIC, "Your $t grows back!", victim, part_locs[part->loc], NULL, TO_CHAR );
               act( AT_MAGIC, "$n's $t grows back!", victim, part_locs[part->loc], NULL, TO_ROOM );
               part->flags = PART_WELL;
               return; /* No more healing after a part is regened */
            }
            else
            {
               part->cond = UMIN( PART_WELL, part->cond + amt / 10 );
            }
         }
         WAIT_STATE( ch, PULSE_VIOLENCE );
         if( IS_NPC( ch ) && number_range( 1, 40 ) < get_curr_int( ch ) )
            xSET_BIT( ch->act, ACT_MAG_FAIL );
         return;
      }
   }
   if( ch->curr_talent[TAL_DEATH] > 0 )
   {
      magic_damage( victim, ch, TALENT( ch, TAL_DEATH ) + TALENT( ch, TAL_HEALING ), MAG_DRAIN, TAL_DEATH, FALSE );
      amt = number_range( 1, TALENT( ch, TAL_DEATH ) + TALENT( ch, TAL_HEALING ) );
      use_magic( ch, TAL_HEALING, amt );
      if( !IS_SET( victim->immune, RIS_DRAIN )  &&TALENT( victim, TAL_DEATH ) < 100 )
      {
         if( IS_AFFECTED( ch, AFF_UNHOLY ) )
            amt *= 2;
         if( IS_AFFECTED( victim, AFF_UNHOLY ) )
            amt *= 2;
         if( IS_AFFECTED( ch, AFF_HOLY ) && amt != 0 )
            amt /= 2;
         if( IS_AFFECTED( victim, AFF_HOLY ) && amt != 0 )
            amt /= 2;
         ch->hit += amt;
         if( ch->hit > ch->max_hit )
            ch->hit = ch->max_hit;
      }
   }
   if( char_died( victim ) )
      return;
   if( ch->curr_talent[TAL_CATALYSM] > 0 )
   {
      magic_damage( victim, ch, 1, MAG_DRAIN, TAL_CATALYSM, FALSE );
      amt = number_range( TALENT( ch, TAL_CATALYSM ), TALENT( ch, TAL_CATALYSM ) * 5 );
      if( !IS_SET( victim->immune, RIS_DRAIN ) )
      {
         victim->mana -= amt;
         ch->mana += amt;
      }
      learn_talent( ch, TAL_CATALYSM );
      
         /*
          * no extra mana cost from mana drain (that would be pointless) 
          */ 
   }
}
void seek_object( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];
   OBJ_DATA * obj;
   OBJ_DATA * in_obj;
   int cnt, found = 0;
   for( obj = first_object; obj; obj = obj->next )
      
   {
      if( !can_see_obj( ch, obj ) || !nifty_is_name( argument, obj->name ) )
         continue;
      if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) || IS_OBJ_STAT( obj, ITEM_NOLOCATE ) || IS_OBJ_STAT( obj, ITEM_ARTIFACT )
           || IS_OBJ_STAT( obj, ITEM_GEM ) || number_percent(  ) > TALENT( ch, TAL_SEEKING ) )
         continue;
      found++;
      for( cnt = 0, in_obj = obj; in_obj->in_obj && cnt < 100; in_obj = in_obj->in_obj, ++cnt )
         ;
      if( cnt >= MAX_NEST )
         
      {
         sprintf( buf, "seek_object: object [%d] %s is nested more than %d times!", obj->pIndexData->vnum,
                   obj->short_descr, MAX_NEST );
         bug( buf, 0 );
         continue;
      }
      if( in_obj->carried_by )
         
      {
         if( TALENT( ch, TAL_SEEKING ) < 30 )
            sprintf( buf, "%s is carried by %s.\n\r", obj_short( obj ), PERS( in_obj->carried_by, ch ) );
         
         else if( TALENT( ch, TAL_SEEKING ) < 60 )
            sprintf( buf, "%s is carried by %s, in %s.\n\r", obj_short( obj ), PERS( in_obj->carried_by, ch ),
                      in_obj->carried_by->in_room->name );
         
         else
            sprintf( buf, "%s is carried by %s, in %s in %s.\n\r", obj_short( obj ), PERS( in_obj->carried_by, ch ),
                      in_obj->carried_by->in_room->name, in_obj->carried_by->in_room->area->name );
      }
      
      else
         
      {
         if( in_obj->in_room == NULL )
            sprintf( buf, "%s is somewhere\n\r", obj_short( obj ) );
         
         else if( TALENT( ch, TAL_SEEKING ) < 50 )
            sprintf( buf, "%s is in %s.\n\r", obj_short( obj ), in_obj->in_room->name );
         
         else
            sprintf( buf, "%s is in %s, in %s.\n\r", obj_short( obj ), in_obj->in_room->name,
                      in_obj->in_room->area->name );
      }
      buf[0] = UPPER( buf[0] );
      set_char_color( AT_MAGIC, ch );
      send_to_char( buf, ch );
   }
   if( !found )
      
   {
      send_to_char( "You were unable to locate anything like that.\n\r", ch );
   }
   mana_from_char( ch, 10 );
   learn_talent( ch, TAL_SEEKING );
   return;
}
void do_seek( CHAR_DATA * ch, char *argument ) 
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_SEEKING] < 20 )
   {
      send_to_char( "You are unable to find anything.\n\r", ch );
      return;
   }
   if( TALENT( ch, TAL_SEEKING ) < 50 )
      for( victim = first_char; victim; victim = victim->next )
         
      {
         if( victim->in_room  &&can_see( ch, victim )  &&nifty_is_name( argument, victim->name ) )
            
         {
            pager_printf( ch, "%s is in %s\n\r", PERS( victim, ch ), victim->in_room->name );
         }
      }
   
   else
      for( victim = first_char; victim; victim = victim->next )
         
      {
         if( victim->in_room  &&can_see( ch, victim )  &&nifty_is_name( argument, victim->name ) )
            
         {
            pager_printf( ch, "%s is in %s, in %s\n\r", PERS( victim, ch ), victim->in_room->name,
                           victim->in_room->area->name );
         }
      }
   seek_object( ch, argument );
   use_magic( ch, TAL_SEEKING, 120 - ch->curr_talent[TAL_SEEKING] );
}
void travel_teleport( CHAR_DATA * ch )
{
   ROOM_INDEX_DATA * pRoomIndex;
   if( char_died( ch ) )
      return;
   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_SENTINEL ) )
      return;
   if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )  &&ch->curr_talent[TAL_MOTION] < 80 )
   {
      send_to_char( "For some reason, nothing happens.\n\r", ch );
      return;
   }
   for( ;; )
      
   {
      pRoomIndex = get_room_index( number_range( 0, 1048576000 ) );
      if( pRoomIndex )
         if( !IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) 
              &&!IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) 
              &&!IS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL ) 
              &&!IS_SET( pRoomIndex->area->flags, AFLAG_NOTELEPORT ) 
              &&!IS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE )  &&!IS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL ) )
            break;
   }
   use_magic( ch, TAL_MOTION, ch->mount ? 50 : 25 );
   act( AT_MAGIC, "Reality seems to twist for a moment, and your surroundings warp and change.", ch, NULL, NULL,
          TO_CHAR );
   act( AT_MAGIC, "Reality seems to twist for a moment, and $n vanishes.", ch, NULL, NULL, TO_ROOM );
   char_from_room( ch );
   ch->in_obj = NULL;
   char_to_room( ch, pRoomIndex );
   if( ch->mount )
   {
      char_from_room( ch->mount );
      char_to_room( ch->mount, pRoomIndex );
   }
   act( AT_MAGIC, "Reality ripples for a moment, and $n suddenly appears.", ch, NULL, NULL, TO_ROOM );
   do_look( ch, "auto" );
   WAIT_STATE( ch, PULSE_VIOLENCE );
}
void do_travel( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   DEITY_DATA * talent;
   AREA_DATA * pArea;
   ROOM_INDEX_DATA * pRoomIndex;
   EXPLORED_AREA * xarea;
   int i, vnum;
   if( IS_NPC( ch ) && ( xIS_SET( ch->act, ACT_PACIFIST )  ||xIS_SET( ch->act, ACT_SENTINEL ) ) )
      return;
   if( ch->curr_talent[TAL_MOTION] < 20 && !IS_NPC( ch ) )
   {
      travel_teleport( ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      travel_teleport( ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_VOID ) )
   {
      travel_teleport( ch );
      return;
   }
   if( IS_FIGHTING( ch ) && ch->curr_talent[TAL_MOTION] < 60 )
   {
      send_to_char( "You can't travel in combat!\n\r", ch );
      return;
   }
   if( !can_travel( 90, ch, ch ) )
   {
      send_to_char( "You feel trapped in this place.\n\r", ch );
      return;
   }
   
      /*
       * Start travel to location 
       */ 
      for( pArea = first_area; pArea; pArea = pArea->next )
      
   {
      if( nifty_is_name_prefix( argument, pArea->name ) )
      {
         
            /*
             * Only allow you to target areas you've been to already 
             */ 
            xarea = ch->pcdata->first_explored;
         while( xarea )
         {
            if( xarea->index == pArea->low_soft_range )
            {
               ch_printf( ch, "&BYou bring to mind your memory of %s...\n\r", pArea->name );
               break;
            }
            xarea = xarea->next;
         }
         if( !xarea )
         {
            send_to_char( "&BYou don't know what that area looks like yet.\n\r", ch );
            return;
         }
         if( IS_SET( pArea->flags, AFLAG_NOTRAVEL ) )
         {
            send_to_char( "&BStrong wards surround the area, blocking your magic.\n\r", ch );
            return;
         }
         for( i = 0; i <= 20; i++ )
         {
            if( i == 20 )
            {
               send_to_char( "&BYou fail to penetrate the wards surrounding that area.\n\r", ch );
               return;
            }
            vnum = number_range( pArea->low_r_vnum, pArea->hi_r_vnum );
            if( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
               continue;
            if( IS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL ) )
            {
               if( IS_SET( pRoomIndex->runes, RUNE_WARD ) )
                  continue;
               if( IS_SET( pArea->flags, AFLAG_NOBYPASS ) )
                  continue;
               if( TALENT( ch, TAL_MOTION ) < 100 )
                  continue;
            }
            break;
         }
         if( !pRoomIndex )
         {
            send_to_char( "You can't find a place to land.\n\r", ch );
            return;
         }
         use_magic( ch, TAL_MOTION, ( 250 - ch->curr_talent[TAL_MOTION] * 2 )  *( ch->mount ? 2 : 1 ) );
         if( ch->pcdata->inborn == -1 )
            talent = get_talent_by_index( TAL_MOTION );
         
         else
            talent = get_talent_by_index( ch->pcdata->inborn );
         act( AT_MAGIC, "$T the room, and $n is gone.", ch, NULL, talent->message, TO_ROOM );
         act( AT_MAGIC, "$T the room, and you travel.", ch, NULL, talent->message, TO_CHAR );
         char_from_room( ch );
         ch->in_obj = NULL;
         char_to_room( ch, pRoomIndex );
         if( ch->mount )
         {
            char_from_room( ch->mount );
            char_to_room( ch->mount, pRoomIndex );
            do_look( ch->mount, "auto" );
         }
         act( AT_MAGIC, "$T the room, and $n appears.", ch, NULL, talent->message, TO_ROOM );
         do_look( ch, "auto" );
         learn_talent( ch, TAL_MOTION );
         WAIT_STATE( ch, PULSE_VIOLENCE * 2 - ch->curr_talent[TAL_MOTION] / 5 );
         return;
      } /* end travel to location */
   }
   if( ch->curr_talent[TAL_MOTION] < 50 )
   {
      act( AT_MAGIC, "You are unable to locate any place like that.", ch, NULL, NULL, TO_CHAR );
      return;
   }
   
      /*
       * start travel to person 
       */ 
      if( ( victim = get_char_world( ch, argument ) ) == NULL 
          ||!can_travel( 90, ch, victim )  ||( TALENT( victim, TAL_SECURITY ) > TALENT( ch, TAL_SEEKING ) ) )
      
   {
      send_to_char( "&BYou are unable to locate any place or anyone like that.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "That was a short trip!\n\r", ch );
      return;
   }
   if( victim == ch->mount )
   {
      send_to_char( "You are riding them already!\n\r", ch );
      return;
   }
   use_magic( ch, TAL_MOTION, ( 250 - ch->curr_talent[TAL_MOTION] * 2 )  *( ch->mount ? 2 : 1 ) );
   if( ch->pcdata->inborn == -1 )
      talent = get_talent_by_index( TAL_MOTION );
   
   else
      talent = get_talent_by_index( ch->pcdata->inborn );
   act( AT_MAGIC, "$T the room, and $n is gone.", ch, NULL, talent->message, TO_ROOM );
   act( AT_MAGIC, "$T the room, and you travel.", ch, NULL, talent->message, TO_CHAR );
   char_from_room( ch );
   ch->in_obj = NULL;
   char_to_room( ch, victim->in_room );
   if( ch->mount )
   {
      char_from_room( ch->mount );
      char_to_room( ch->mount, victim->in_room );
      do_look( ch->mount, "auto" );
   }
   act( AT_MAGIC, "$T the room, and $n appears.", ch, NULL, talent->message, TO_ROOM );
   do_look( ch, "auto" );
   learn_talent( ch, TAL_MOTION );
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 - ch->curr_talent[TAL_MOTION] / 5 );
}
void do_summon( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim = NULL;
   char buf[MAX_STRING_LENGTH];
   if( ch->curr_talent[TAL_MOTION] < 40 && !IS_NPC( ch ) )
   {
      travel_teleport( ch );
      return;
   }
   if( IS_SET( ch->in_room->area->flags, AFLAG_NOBYPASS ) )
   {
      huh( ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_VOID ) )
   {
      travel_teleport( ch );
      return;
   }
   victim = get_char_world( ch, argument );
   if( !victim  ||( IS_NPC( victim ) && !xIS_SET( victim->act, ACT_NAMED ) ) )
      victim = generate_mob_nation( argument );
   if( !victim )
   {
      send_to_char( "You cannot find anything like that.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "That was a short trip!\n\r", ch );
      return;
   }
   if( victim->in_room && !can_travel( 100, ch, victim ) )
   {
      send_to_char( "Something prevents you from opening a rift.\n\r", ch );
      return;
   }
   use_magic( ch, TAL_MOTION, 500 - ( ch->curr_talent[TAL_MOTION] * 4 ) );
   if( !IS_NPC( victim ) )
   {
      sprintf( buf, "%s summoned %s to %s (%d).", ch->name, victim->name, ch->in_room->name, ch->in_room->vnum );
      to_channel( buf, "Spell", PERMIT_SECURITY );
   }
   if( victim->in_room )
   {
      act( AT_MAGIC, "A swirling rift opens for a moment, sucking $n through.", victim, NULL, ch, TO_ROOM );
      act( AT_MAGIC, "A swirling rift opens suddenly, and $n pulls you through!", ch, NULL, victim, TO_VICT );
   }
   act( AT_MAGIC, "You open a swirling rift and pull $N through.", ch, NULL, victim, TO_CHAR );
   act( AT_MAGIC, "$n opens a swirling rift and pulls $N through.", ch, NULL, victim, TO_ROOM );
   if( victim->in_room )
      char_from_room( victim );
   victim->in_obj = NULL;
   char_to_room( victim, ch->in_room );
   do_look( victim, "auto" );
   WAIT_STATE( ch, PULSE_VIOLENCE );
   WAIT_STATE( victim, PULSE_VIOLENCE );
}
void do_morph( CHAR_DATA * ch, char *argument )
{
   NATION_DATA * newrace;
   SPECIES_DATA * species;
   OBJ_DATA * obj;
   if( ch->curr_talent[TAL_CHANGE] < 10 )
   {
      huh( ch );
      return;
   }
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( argument && !ch->pcdata->were_race )
   {
      
         /*
          * if they don't have a morph, set it to the argument 
          */ 
         newrace = find_nation( argument );
      if( !newrace )
      {
         send_to_char( "There is no such race.\n\r", ch );
         return;
      }
      if( IS_SET( newrace->flags, NAT_NPC ) )
      {
         send_to_char( "You cannot change into that!", ch );
         return;
      }
      if( has_bodypart( ch, BP_RHAND )  &&IS_SET( newrace->parts, PART_HANDS ) )
      {
         send_to_char( "That race is too similar, pick one without hands.\n\r", ch );
         return;
      }
      if( !has_bodypart( ch, BP_RHAND )  &&!IS_SET( newrace->parts, PART_HANDS ) )
      {
         send_to_char( "That race is too similar, pick one with hands.\n\r", ch );
         return;
      }
      if( has_bodypart( ch, BP_RHAND ) )
         ch_printf( ch, "You are now a were-%s.\n\r", newrace->name );
      
      else
         ch_printf( ch, "You are now a were-%s.\n\r", ch->species );
      ch->pcdata->were_race = STRALLOC( newrace->name );
      return;
   }
   if( ch->mount )
   {
      send_to_char( "And scare your mount?\n\r", ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_MOUNTED ) )
      throw_rider( ch );
   if( ch->mana <= 0 )
      
   {
      send_to_char( "You do not have enough mana to change.\n\r", ch );
      return;
   }
   newrace = find_nation( ch->pcdata->were_race );
   STRFREE( ch->pcdata->were_race );
   if( IS_AFFECTED( ch, AFF_ANTHRO ) )
      ch->pcdata->were_race = STRALLOC( "anthro" );
   
   else
      ch->pcdata->were_race = STRALLOC( ch->species );
   if( !newrace )
   {
      send_to_char( "Select your were-race first with morph <race>.", ch );
      return;
   }
   use_magic( ch, TAL_CHANGE, 100 );
   de_equip_char( ch );
   if( IS_AFFECTED( ch, AFF_FLYING ) )
      do_land( ch, "" );
   species = find_species( newrace->species );
   if( !species )
   {
      bug( "do_morph: cannot find species!" );
      send_to_char( "You suddenly forget what one of those is.", ch );
      return;
   }
   act( AT_MAGIC, "Your body changes and warps, and you become $t.", ch, aoran( newrace->name ), NULL, TO_CHAR );
   act( AT_MAGIC, "$n's body changes and warps, and $e becomes $t.", ch, aoran( newrace->name ), NULL, TO_ROOM );
   if( IS_AFFECTED( ch, AFF_ANTHRO ) )
   {
      xREMOVE_BIT( ch->pcdata->perm_aff, AFF_ANTHRO );
      STRFREE( ch->pcdata->type );
      if( IS_AFFECTED( ch, AFF_UNDEAD ) )
         ch->pcdata->type = STRALLOC( "undead" );
      
      else
         ch->pcdata->type = STRALLOC( "living" );
   }
   ch->nation = newrace;
   set_race( ch, FALSE );
   if( !can_use_bodypart( ch, BP_RHAND ) )
   {
      if( ch->main_hand )
      {
         obj_unaffect_ch( ch, ch->main_hand );
         ch->main_hand = NULL;
      }
      if( ch->off_hand )
      {
         obj_unaffect_ch( ch, ch->off_hand );
         ch->off_hand = NULL;
      }
   }
   if( ch->curr_talent[TAL_CHANGE] >= 50 )
   {
      for( obj = ch->first_carrying; obj; obj = obj->next_content )
      {
         obj->size = ch->height;
      }
   }
   re_equip_char( ch );
   update_aris( ch );
   ch->max_hit = UMAX( 20, ch->nation->hit * get_curr_con( ch ) + ch->base_hit );
   ch->max_mana = UMAX( 20, ch->nation->mana * get_curr_int( ch ) + ch->base_mana );
   ch->max_move = UMAX( 20, 25 * get_curr_end( ch ) + ch->base_move );
   ch->mana = ch->max_mana;
}
void do_change( CHAR_DATA * ch, char *argument )
{
   NATION_DATA * newrace;
   SPECIES_DATA * species;
   char arg[MAX_STRING_LENGTH];
   CHAR_DATA * victim;
   OBJ_DATA * obj;
   if( ch->curr_talent[TAL_CHANGE] < 1 )
   {
      huh( ch );
      return;
   }
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Change into what?\n\r", ch );
      return;
   }
   if( ch->mount )
   {
      send_to_char( "And scare your mount?\n\r", ch );
      return;
   }
   if( IS_AFFECTED( ch, AFF_MOUNTED ) )
      throw_rider( ch );
   if( ch->mana <= 0 )
      
   {
      send_to_char( "You do not have enough mana to change.\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg );
   victim = find_target( ch, arg, FALSE );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't change that!\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "anthro" ) )
   {
      if( has_bodypart( victim, BP_RHAND ) )
      {
         send_to_char( "They already have hands.\n\r", ch );
         return;
      }
      if( victim->height > 100 )
      {
         send_to_char( "They are too large to become an anthropomorphic form.\n\r", ch );
         return;
      }
      xSET_BIT( victim->pcdata->perm_aff, AFF_ANTHRO );
      check_bodyparts( victim );
      if( victim->height < 60 )
         victim->height = number_range( 36, 60 );
      
      else
         victim->height = number_range( 60, 76 );
      victim->weight = get_height_weight( victim );
      STRFREE( victim->pcdata->type );
      victim->pcdata->type = STRALLOC( "anthropomorphic" );
      act( AT_MAGIC, "Your body warps into an anthropomorphic form.", victim, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n's body warps into an anthropomorphic form.", ch, NULL, NULL, TO_NOTVICT );
      if( victim != ch )
         act( AT_MAGIC, "You warp $N's body into an anthropomorphic form.", ch, NULL, victim, TO_CHAR );
      return;
   }
   if( ch->curr_talent[TAL_CHANGE] < 30 || !argument )
   {
      send_to_char( "Your Changing magic goes awry...\n\r", ch );
      if( !add_mutation( victim, 0 ) )
         send_to_char( "However, there is no effect.\n\r", ch );
      return;
   }
   if( victim != ch )
   {
      if( TALENT( ch, TAL_CHANGE ) < 80 )
      {
         send_to_char( "You are not strong enough to change others yet.\n\r", ch );
         return;
      }
      if( !IS_CONSENTING( victim, ch )  &&TALENT( ch, TAL_CHANGE ) < 400 )
      {
         send_to_char( "You cannot do that without their consent.\n\r", ch );
         return;
      }
   }
   newrace = find_nation( argument );
   if( !newrace )
   {
      send_to_char( "You can't change into any such thing.", ch );
      return;
   }
   if( str_cmp( newrace->species, victim->nation->species )  &&( ch->curr_talent[TAL_CHANGE] < 60 ) )
   {
      send_to_char( "You are not skilled enough to make such a drastic change.\n\r", ch );
      return;
   }
   use_magic( ch, TAL_CHANGE, 300 );
   de_equip_char( victim );
   if( IS_AFFECTED( victim, AFF_FLYING ) )
      do_land( victim, "" );
   species = find_species( newrace->species );
   if( !species )
   {
      bug( "do_change: cannot find species!" );
      send_to_char( "You suddenly forget what one of those is.", ch );
      return;
   }
   ch_printf( victim, "Your body changes and warps, and you become %s.\n\r", aoran( newrace->name ) );
   act( AT_MAGIC, "$N's body changes and warps, and $E becomes $t.", ch, aoran( newrace->name ), victim, TO_NOTVICT );
   if( ch != victim )
   {
      act( AT_MAGIC, "You change and warp $N's body into $t.", ch, aoran( newrace->name ), victim, TO_CHAR );
   }
   if( IS_AFFECTED( victim, AFF_ANTHRO ) )
   {
      xREMOVE_BIT( victim->pcdata->perm_aff, AFF_ANTHRO );
      STRFREE( victim->pcdata->type );
      if( IS_AFFECTED( victim, AFF_UNDEAD ) )
         victim->pcdata->type = STRALLOC( "undead" );
      
      else
         victim->pcdata->type = STRALLOC( "living" );
   }
   victim->nation = newrace;
   set_race( victim, FALSE );
   if( !IS_SET( newrace->parts, PART_HANDS ) )
   {
      if( ch->main_hand )
      {
         obj_unaffect_ch( ch, ch->main_hand );
         ch->main_hand = NULL;
      }
      if( ch->off_hand )
      {
         obj_unaffect_ch( ch, ch->off_hand );
         ch->off_hand = NULL;
      }
   }
   if( ch->curr_talent[TAL_CHANGE] >= 50 )
   {
      for( obj = victim->first_carrying; obj; obj = obj->next_content )
      {
         obj->size = victim->height;
      }
   }
   re_equip_char( victim );
   update_aris( victim );
   victim->max_hit = UMAX( 20, victim->nation->hit * get_curr_con( victim ) + victim->base_hit );
   victim->max_mana = UMAX( 20, victim->nation->mana * get_curr_int( victim ) + victim->base_mana );
   victim->max_move = UMAX( 20, 25 * get_curr_end( victim ) + victim->base_move );
   victim->mana = UMIN( victim->max_mana, victim->mana );
}
void do_alter( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int i;
   if( ch->curr_talent[TAL_CHANGE] < 25 )
   {
      huh( ch );
      return;
   }
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Alter what?\n\r", ch );
      return;
   }
   if( ch->mana <= 0 )
      
   {
      send_to_char( "You do not have enough mana to change.\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   victim = find_target( ch, arg1, FALSE );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't change that!\n\r", ch );
      return;
   }
   if( victim != ch )
   {
      if( TALENT( ch, TAL_CHANGE ) < 50 )
      {
         send_to_char( "You are not strong enough to change others yet.\n\r", ch );
         return;
      }
      if( !IS_CONSENTING( victim, ch )  &&TALENT( ch, TAL_CHANGE ) < 100 )
      {
         send_to_char( "You cannot do that without their consent.\n\r", ch );
         return;
      }
   }
   if( !str_cmp( arg2, "height" ) )
   {
      i = atoi( argument );
      if( i >= victim->nation->height * ( 1 + TALENT( ch, TAL_CHANGE ) / 100 ) )
      {
         send_to_char( "You can't make them that tall.\n\r", ch );
         return;
      }
      if( i <= victim->nation->height - victim->nation->height * TALENT( ch, TAL_CHANGE ) / 100  ||victim->height <= 10 )
      {
         send_to_char( "You can't make them that short.\n\r", ch );
         return;
      }
      victim->height = i;
      if( victim != ch )
      {
         act( AT_MAGIC, "Your height changes.", ch, NULL, victim, TO_VICT );
         act( AT_MAGIC, "You make $N's height change.", ch, NULL, victim, TO_CHAR );
      }
      else
         act( AT_MAGIC, "You change your height.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$N's height changes.", ch, NULL, victim, TO_NOTVICT );
   }
   else if( !str_cmp( arg2, "weight" ) )
   {
      i = atoi( argument );
      if( i >= victim->nation->weight * ( 1 + TALENT( ch, TAL_CHANGE ) / 100 ) )
      {
         send_to_char( "You can't make them that heavy.\n\r", ch );
         return;
      }
      if( i <= victim->nation->weight - victim->nation->weight * TALENT( ch, TAL_CHANGE ) / 100  ||victim->weight <= 10 )
      {
         send_to_char( "You can't make them that light.\n\r", ch );
         return;
      }
      victim->weight = i;
      if( victim != ch )
      {
         if( victim != ch )
         {
            act( AT_MAGIC, "Your weight changes.", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "You make $N's weight change.", ch, NULL, victim, TO_CHAR );
         }
         else
            act( AT_MAGIC, "You change your weight.", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$N's weight changes.", ch, NULL, victim, TO_NOTVICT );
      }
   }
   else if( !str_cmp( arg2, "sex" ) )
   {
      if( !str_prefix( "f", argument ) )
         victim->sex = SEX_FEMALE;
      
      else if( !str_prefix( "m", argument ) )
         victim->sex = SEX_MALE;
      
      else if( !str_prefix( "n", argument ) )
         victim->sex = SEX_NEUTRAL;
      
      else if( !str_prefix( "h", argument ) )
         victim->sex = SEX_HERMAPH;
      if( victim != ch )
      {
         act( AT_MAGIC, "You feel something strange change in your crotch.", ch, NULL, victim, TO_VICT );
         act( AT_MAGIC, "You change $N's gender.", ch, NULL, victim, TO_CHAR );
      }
      else
         act( AT_MAGIC, "You change your own sex.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$N's crotch seems to shift a little.", ch, NULL, victim, TO_NOTVICT );
      check_bodyparts( victim );
   }
   else if( victim->pcdata->eyes  &&!str_cmp( "eyes", arg2 ) )
   {
      STRFREE( victim->pcdata->eyes );
      victim->pcdata->eyes = STRALLOC( argument );
      act( AT_MAGIC, "Your eyes itch.", victim, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n's eyes change to $T.", victim, NULL, argument, TO_ROOM );
   }
   else if( victim->pcdata->hair  &&!str_cmp( "hair", arg2 ) )
   {
      STRFREE( victim->pcdata->hair );
      victim->pcdata->hair = STRALLOC( argument );
      act( AT_MAGIC, "Your hair shimmers for a moment.", victim, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n's hair changes to $T.", victim, NULL, argument, TO_ROOM );
   }
   else if( victim->pcdata->skin_type  &&!str_cmp( victim->pcdata->skin_type, arg2 ) )
   {
      STRFREE( victim->pcdata->skin_color );
      victim->pcdata->skin_color = STRALLOC( argument );
      act( AT_MAGIC, "Your $t shimmers for a moment.", victim, victim->pcdata->skin_type, NULL, TO_CHAR );
      act( AT_MAGIC, "$n's $t changes to $T.", victim, victim->pcdata->skin_type, argument, TO_ROOM );
   }
   else if( victim->pcdata->extra_type  &&!str_cmp( victim->pcdata->extra_type, arg2 ) )
   {
      STRFREE( victim->pcdata->extra_color );
      victim->pcdata->extra_color = STRALLOC( argument );
      act( AT_MAGIC, "Your $t shimmers for a moment.", victim, victim->pcdata->extra_type, NULL, TO_CHAR );
      act( AT_MAGIC, "$n's $t changes to $T.", victim, victim->pcdata->extra_type, argument, TO_ROOM );
   }
   else
   {
      send_to_char( "They don't have that.\n\r", ch );
      return;
   }
   use_magic( ch, TAL_CHANGE, 50 );
}


/*
 * Check for antimagic.
 */ 
   bool check_antimagic( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !victim )
      return FALSE;
   if( ch == victim )
      return FALSE;
   if( !IS_AWAKE( victim ) )
      return FALSE;
   if( !IS_SET( victim->mood, MOOD_READY ) )
      return FALSE;
   if( victim->curr_talent[TAL_VOID] < 10 )
      return FALSE;
   if( victim->mana < 100 )
      return FALSE;
   if( number_range( 1, 105 ) > ( number_range( 1, TALENT( victim, TAL_VOID ) ) ) )
      
   {
      return FALSE;
   }
   use_magic( victim, TAL_VOID, 100 );
   act( AT_MAGIC, "You cast $n's magic into the Void.", ch, NULL, victim, TO_VICT );
   act( AT_MAGIC, "$N channels antimagic, stopping your attack.", ch, NULL, victim, TO_CHAR );
   act( AT_MAGIC, "$N channels antimagic, stopping $n's attack.", ch, NULL, victim, TO_NOTVICT );
   return TRUE;
}


/*
 * Check for blur.
 */ 
   bool check_blur( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !victim )
      return FALSE;
   if( ch == victim )
      return FALSE;
   if( !IS_AWAKE( victim ) )
      return FALSE;
   if( !IS_SET( victim->mood, MOOD_READY ) )
      return FALSE;
   if( victim->curr_talent[TAL_TIME] < 10 )
      return FALSE;
   if( victim->mana < 100 )
      return FALSE;
   if( number_range( 1, 200 ) > ( number_range( 1, TALENT( victim, TAL_TIME ) - victim->encumberance ) ) )
      
   {
      return FALSE;
   }
   use_magic( victim, TAL_TIME, 50 );
   act( AT_MAGIC, "$n swings at you, but you blur out of the way.", ch, NULL, victim, TO_VICT );
   act( AT_MAGIC, "You swing at $N, but $E blurs out of the way.", ch, NULL, victim, TO_CHAR );
   act( AT_MAGIC, "$n swings at $N, but $E blurs out of the way.", ch, NULL, victim, TO_NOTVICT );
   return TRUE;
}


/*
 * Check for displacement.
 */ 
   bool check_displacement( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( !victim )
      return FALSE;
   if( ch == victim )
      return FALSE;
   if( !IS_AWAKE( victim ) )
      return FALSE;
   if( !IS_SET( victim->mood, MOOD_READY ) )
      return FALSE;
   if( victim->curr_talent[TAL_ILLUSION] < 10 )
      return FALSE;
   if( victim->mana < 50 )
      return FALSE;
   if( number_range( 1, 100 + get_curr_per( ch ) ) > ( number_range( 1, TALENT( victim, TAL_ILLUSION ) ) ) )
      
   {
      return FALSE;
   }
   use_magic( victim, TAL_ILLUSION, 50 );
   act( AT_MAGIC, "$n strikes at your displaced image, missing you.", ch, NULL, victim, TO_VICT );
   act( AT_MAGIC, "You strike at $N's displaced image, missing $M.", ch, NULL, victim, TO_CHAR );
   act( AT_MAGIC, "$n strikes at $N's displaced image, missing $M.", ch, NULL, victim, TO_NOTVICT );
   return TRUE;
}
void do_blink( CHAR_DATA * ch, char *argument )
{
   if( ch->curr_talent[TAL_MOTION] < 50 )
   {
      check_social( ch, "blink", argument );
      return;
   }
   use_magic( ch, TAL_MOTION, 20 );
   act( AT_MAGIC, "You blink a short distance away.", ch, NULL, NULL, TO_CHAR );
   act( AT_MAGIC, "$n blinks out of existence.", ch, NULL, NULL, TO_ROOM );
   stop_fighting( ch, TRUE );
}
void do_pacify( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( IS_SILENT( ch ) )
   {
      send_to_char( "No sound comes out.\n\r", ch );
      return;
   }
   if( ch->curr_talent[TAL_SPEECH] < 60 )
   {
      send_to_char( "They don't seem overly impressed by your singing skills.", ch );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   use_magic( ch, TAL_SPEECH, 1000 - ch->curr_talent[TAL_SPEECH] * 9 );
   stop_fighting( ch, FALSE );
   act( AT_SAY, "You hum a tranquil melody to $N.", ch, NULL, victim, TO_CHAR );
   act( AT_SAY, "$n hums a tranquil melody to $N.", ch, NULL, victim, TO_NOTVICT );
   act( AT_SAY, "$n hums a tranquil melody, and you feel at peace.", ch, NULL, victim, TO_VICT );
}
void do_purify( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_HEALING] < 100 )
   {
      huh( ch );
      return;
   }
   victim = find_target( ch, argument, FALSE );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot purify them!\n\r", ch );
      return;
   }
   if( victim != ch )
   {
      act( AT_WHITE, "You completely purify $N!", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "A white aura surrounds $N as $n purifies $M.", ch, NULL, victim, TO_NOTVICT );
      act( AT_WHITE, "Your blood is cleansed and you feel purified.", ch, NULL, victim, TO_VICT );
   }
   else
   {
      act( AT_WHITE, "You completely purify yourself.", ch, NULL, NULL, TO_CHAR );
      act( AT_WHITE, "A white aura surrounds $n as $e purifies $mself.", ch, NULL, NULL, TO_ROOM );
   }
   victim->pcdata->condition[COND_FULL] = 10;
   victim->pcdata->condition[COND_THIRST] = 10;
   victim->pcdata->condition[COND_DRUNK] = 0;
   if( is_affected( victim, gsn_poison ) )
      affect_strip( victim, gsn_poison );
   if( is_affected( victim, gsn_corruption ) )
      affect_strip( victim, gsn_corruption );
   if( is_affected( victim, gsn_curse ) )
      affect_strip( victim, gsn_curse );
   if( is_affected( victim, gsn_blindness ) )
      affect_strip( victim, gsn_blindness );
   if( is_affected( victim, gsn_weaken ) )
      affect_strip( victim, gsn_weaken );
   if( is_affected( victim, gsn_disease ) )
      affect_strip( victim, gsn_disease );
   if( is_affected( victim, gsn_mummy_rot ) )
      affect_strip( victim, gsn_mummy_rot );
  
//Removing the purify ability to correct vampirism, I want there to be a whole quest for people to get rid of vampirism  //REMOVE_BIT( victim->pcdata->flags, PCFLAG_VAMPIRE );
}
void do_cure( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   char arg[MAX_STRING_LENGTH];
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Cure whom of what?\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg );
   victim = find_target( ch, argument, FALSE );
   if( !victim )
      return;
   if( !str_cmp( arg, "poison" )  &&ch->curr_talent[TAL_HEALING] >= 15 )
   {
      if( is_affected( victim, gsn_poison ) )
         
      {
         affect_strip( victim, gsn_poison );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "A warm feeling runs through your body.\n\r", victim );
         victim->mental_state = URANGE( -100, victim->mental_state, -10 );
         if( ch != victim )
            
         {
            act( AT_MAGIC, "A flush of health washes over $N.", ch, NULL, victim, TO_NOTVICT );
            act( AT_MAGIC, "You lift the poison from $N's body.", ch, NULL, victim, TO_CHAR );
         }
      }
      
      else
         
      {
         set_char_color( AT_MAGIC, ch );
         if( ch != victim )
            send_to_char( "You work your cure, but it has no apparent effect.\n\r", ch );
         
         else
            send_to_char( "You don't seem to be poisoned.\n\r", ch );
      }
   }
   else if( !str_cmp( arg, "rot" )  &&( ch->curr_talent[TAL_HEALING] >= 60 ) )
   {
      if( !is_affected( victim, gsn_mummy_rot ) )
         
      {
         if( ch != victim )
            send_to_char( "They don't seem to be rotting.\n\r", ch );
         
         else
            send_to_char( "You don't seem to be rotting.\n\r", ch );
         return;
      }
      affect_strip( victim, gsn_mummy_rot );
      set_char_color( AT_MAGIC, victim );
      send_to_char( "Your flesh stops rotting.\n\r", victim );
      if( ch != victim )
         send_to_char( "You restore the rotten flesh to normal.\n\r", ch );
   }
   else if( !str_cmp( arg, "plague" )  &&( ch->curr_talent[TAL_HEALING] >= 75 ) )
   {
      if( !is_affected( victim, gsn_disease ) )
         
      {
         if( ch != victim )
            send_to_char( "They don't seem to be infected.\n\r", ch );
         
         else
            send_to_char( "You don't seem to be infected.\n\r", ch );
         return;
      }
      affect_strip( victim, gsn_disease );
      set_char_color( AT_MAGIC, victim );
      send_to_char( "The plague is purged from you.\n\r", victim );
      if( ch != victim )
         send_to_char( "You work your cure, purging the plague.\n\r", ch );
   }
   else if( !str_cmp( arg, "blind" )  &&( ch->curr_talent[TAL_HEALING] >= 25  ||ch->curr_talent[TAL_SEEKING] >= 25 ) )
   {
      if( !is_affected( victim, gsn_blindness ) )
         
      {
         if( ch != victim )
            send_to_char( "They don't seem to be blind.\n\r", ch );
         
         else
            send_to_char( "You don't seem to be blind.\n\r", ch );
         return;
      }
      affect_strip( victim, gsn_blindness );
      set_char_color( AT_MAGIC, victim );
      send_to_char( "Your vision returns!\n\r", victim );
      if( ch != victim )
         send_to_char( "You work your cure, restoring vision.\n\r", ch );
   }
   else if( !str_cmp( arg, "curse" )  &&ch->curr_talent[TAL_SECURITY] >= 35 )
   {
      if( is_affected( victim, gsn_curse ) )
         
      {
         affect_strip( victim, gsn_curse );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "The weight of your curse is lifted.\n\r", victim );
         if( ch != victim )
            
         {
            act( AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR );
            act( AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT );
         }
      }
      else
      {
         if( ch != victim )
            send_to_char( "They don't seem to be cursed.\n\r", ch );
         
         else
            send_to_char( "You don't seem to be cursed.\n\r", ch );
      }
   }
   else if( !str_cmp( arg, "corrupt" )  &&( ch->curr_talent[TAL_HEALING] >= 35  ||ch->curr_talent[TAL_CHANGE] >= 50 ) )
   {
      if( is_affected( victim, gsn_corruption ) )
         
      {
         affect_strip( victim, gsn_corruption );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "Waves of magic cleanse your body.\n\r", victim );
         if( ch != victim )
            
         {
            act( AT_MAGIC, "You cleanse $N's body of corruption.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N's body returns to normal again.", ch, NULL, victim, TO_NOTVICT );
      }
      else
      {
         if( ch != victim )
            send_to_char( "They don't seem to be corrupted.\n\r", ch );
         
         else
            send_to_char( "You don't seem to be corrupted.\n\r", ch );
      }
   }
   else if( !str_cmp( arg, "weak" )  &&ch->curr_talent[TAL_HEALING] >= 10 )
   {
      if( is_affected( victim, gsn_weaken ) )
         
      {
         affect_strip( victim, gsn_weaken );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "You feel your strength returning.\n\r", victim );
         if( ch != victim )
            
         {
            act( AT_MAGIC, "You dispel the magical weakness from $N.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N appears to grow stronger.", ch, NULL, victim, TO_NOTVICT );
      }
      else
      {
         if( ch != victim )
            send_to_char( "They don't seem to be magically weakened.\n\r", ch );
         
         else
            send_to_char( "You don't seem to be magically weakened.\n\r", ch );
      }
   }
   else if( !str_cmp( arg, "drunk" )  &&ch->curr_talent[TAL_MIND] + ch->curr_talent[TAL_HEALING] >= 20 )
   {
      if( victim->pcdata )
         victim->pcdata->condition[COND_DRUNK] = 0;
      act( AT_MAGIC, "You purge the alcohol from $N's system.", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "$n purges the alcohol from your system.", ch, NULL, victim, TO_VICT );
      learn_talent( ch, TAL_MIND );
   }
   else
   {
      send_to_char( "You do not know how to cure any such thing.\n\r", ch );
      return;
   }
   use_magic( ch, TAL_HEALING, 110 - ch->curr_talent[TAL_HEALING] );
   WAIT_STATE( ch, PULSE_VIOLENCE );
}
void do_dispel( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   OBJ_DATA * obj;
   if( ch->curr_talent[TAL_CATALYSM] < 10  &&ch->curr_talent[TAL_VOID] < 10 )
   {
      huh( ch );
      return;
   }
   if( !str_cmp( argument, "room" )  &&ch->curr_talent[TAL_CATALYSM] >= 50 )
   {
      act( AT_MAGIC, "You wave your hands, dispelling any enchantments on the room.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n has dispelled the room!", ch, NULL, NULL, TO_ROOM );
      ch->in_room->runes = 0;
      return;
   }
   obj = get_obj_carry( ch, argument );
   if( !obj )
   {
      victim = find_target( ch, argument, TRUE );
      if( !victim )
         return;
      if( ch->curr_talent[TAL_CATALYSM] < 30  &&victim != ch  &&ch->curr_talent[TAL_VOID] < 30 )
      {
         send_to_char( "You are not yet strong enough to strip another's enchantments.\n\r", ch );
         return;
      }
      if( IS_AFFECTED( victim, AFF_MAGICAL ) )
      {
         magic_damage( victim, ch, TALENT( ch, TAL_CATALYSM ) * 3 
                        +TALENT( ch, TAL_VOID ) * 5, MAG_ANTIMAGIC, TAL_CATALYSM, FALSE );
      } /* fall through to get their affs too */
      use_magic( ch, TAL_CATALYSM, 200 - ch->curr_talent[TAL_CATALYSM] );
      if( TALENT( victim, TAL_SECURITY ) > TALENT( ch, TAL_CATALYSM )  +TALENT( ch, TAL_VOID )  &&ch != victim )
      {
         act( AT_MAGIC, "$N's magic wavers but holds.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "Your magic wavers but holds.", ch, NULL, victim, TO_VICT );
         learn_talent( victim, TAL_SECURITY );
         return;
      }
      if( victim->first_affect )
      {
         set_char_color( AT_MAGIC, victim );
         send_to_char( "Your enchantments are stripped away by powerful magic.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You strip away $N's enchantments.", ch, NULL, victim, TO_CHAR );
         }
         while( victim->first_affect )
            affect_remove( victim, victim->first_affect );
         WAIT_STATE( ch, PULSE_VIOLENCE );
      }
      else
      {
         if( ch == victim )
            send_to_char( "You are not affected by any enchantments.\n\r", ch );
         
         else
            send_to_char( "They don't seem to be affected by any enchantments.\n\r", ch );
      }
   } /* end player dispel */
   
   else
   {
      if( ch->curr_talent[TAL_CATALYSM] < 60 )
      {
         send_to_char( "You cannot yet disenchant items.\n\r", ch );
         return;
      }
      if( !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
         
      {
         send_to_char( "This item appears to have no enchantments on it.\r\n", ch );
         return;
      }
      if( xIS_SET( obj->pIndexData->extra_flags, ITEM_MAGIC ) )
         
      {
         send_to_char( "This item is inherently magical, you cannot dispel its affects.\r\n", ch );
         return;
      }
      act( AT_DGREY, "Wisps of black smoke rise from $p.", ch, obj, NULL, TO_CHAR );
      act( AT_DGREY, "Wisps of black smoke rise from $p.", ch, obj, NULL, TO_ROOM );
      xREMOVE_BIT( obj->extra_flags, ITEM_MAGIC );
      xREMOVE_BIT( obj->extra_flags, ITEM_GLOW );
      xREMOVE_BIT( obj->extra_flags, ITEM_DARK );
      xREMOVE_BIT( obj->extra_flags, ITEM_HUM );
      if( obj->item_type == ITEM_WEAPON )
      {
         
            /*
             * dispel banes and brands too 
             */ 
            obj->value[0] = 0;
         obj->value[4] = 0;
         obj->value[7] = 0;
      }
      use_magic( ch, TAL_CATALYSM, 1000 - ch->curr_talent[TAL_CATALYSM] * 4 );
      WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
   }
}
void do_sing( CHAR_DATA * ch, char *argument )
{
   int i;
   char buf[MAX_STRING_LENGTH];
   bool found;
   OBJ_DATA * obj;
   if( IS_SILENT( ch ) )
   {
      send_to_char( "No sound comes out.\n\r", ch );
      return;
   }
   if( ch->curr_talent[TAL_SPEECH] < 10 )
   {
      check_social( ch, "sing", argument );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      if( ch->singing )
         ch_printf( ch, "&PYou are singing a song of %s.\n\r", song_name[ch->singing] );
      
      else
         send_to_char( "&PSing what?\n\r", ch );
      return;
   }
   if( ch->singing == SONG_FLIGHT )
      xREMOVE_BIT( ch->affected_by, AFF_FLYING );
   found = FALSE;
   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      if( obj->item_type == ITEM_INSTRUMENT )
      {
         found = TRUE;
         break;
      }
   }
   if( !str_cmp( argument, "none" ) )
   {
      act( AT_PINK, "You stop singing.", ch, NULL, NULL, TO_CHAR );
      act( AT_PINK, "$n stops singing.", ch, NULL, NULL, TO_ROOM );
      ch->singing = SONG_NONE;
      if( obj )
         xREMOVE_BIT( obj->extra_flags, ITEM_IN_USE );
      update_aris( ch );
      return;
   }
   if( ch->curr_talent[TAL_SPEECH] < 50 )
   {
      if( found )
      {
         act( AT_PINK, "You play $p.", ch, obj, NULL, TO_CHAR );
         act( AT_PINK, "$n plays $p.", ch, obj, NULL, TO_ROOM );
         xSET_BIT( obj->extra_flags, ITEM_IN_USE );
      }
      else
      {
         send_to_char( "You need an instrument to play.\n\r", ch );
         ch->singing = SONG_NONE;
         return;
      }
   }
   for( i = 1; i < MAX_SONG; i++ )
   {
      if( !str_cmp( argument, song_name[i] ) )
      {
         ch->singing = i;
         ch_printf( ch, "&PYou sing a song of %s.\n\r", song_name[i] );
         sprintf( buf, "$n sings a song of %s.", song_name[i] );
         act( AT_PINK, buf, ch, NULL, NULL, TO_ROOM );
         update_aris( ch );
         return;
      }
   }
   send_to_char( "You know of no such song.\n\r", ch );
}
void do_rune( CHAR_DATA * ch, char *argument )
{
   int tal;
   if( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
   {
      send_to_char( "The runes fade and vanish.\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "life" )  &&ch->curr_talent[TAL_HEALING] >= 100 )
   {
      act( AT_WHITE, "You trace runes of life on the walls.", ch, NULL, NULL, TO_CHAR );
      act( AT_WHITE, "$n traces glowing white runes on the walls.", ch, NULL, NULL, TO_ROOM );
      SET_BIT( ch->in_room->runes, RUNE_LIFE );
      tal = TAL_HEALING;
   }
   else if( !str_cmp( argument, "distract" )  &&ch->curr_talent[TAL_DREAM] >= 100 )
   {
      act( AT_PINK, "You trace runes of distraction on the walls.", ch, NULL, NULL, TO_CHAR );
      act( AT_PINK, "$n traces glowing pink runes on the walls.", ch, NULL, NULL, TO_ROOM );
      SET_BIT( ch->in_room->runes, RUNE_DISTRACT );
      tal = TAL_DREAM;
   }
   else if( !str_cmp( argument, "ward" )  &&ch->curr_talent[TAL_SECURITY] >= 100 )
   {
      act( AT_CYAN, "You trace runes of warding on the walls.", ch, NULL, NULL, TO_CHAR );
      act( AT_CYAN, "$n traces glowing cyan runes on the walls.", ch, NULL, NULL, TO_ROOM );
      SET_BIT( ch->in_room->runes, RUNE_WARD );
      tal = TAL_SECURITY;
   }
   else
   {
      send_to_char( "You know no such rune.\n\r", ch );
      return;
   }
   use_magic( ch, tal, 1000 );
}
void do_shield( CHAR_DATA * ch, char *argument )
{
   int tal;
   if( !argument || argument[0] == '\0' )
   {
      if( ch->shield )
         ch_printf( ch, "You are surrounded by a shield of %s.\n\r", magic_table[ch->shield] );
      
      else
         send_to_char( "You are not shielded.\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "none" )  &&ch->shield )
   {
      act( AT_MAGIC, "You lower your magical shield.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n lowers $s magical shield.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_NONE;
      return;
   }
   else if( !str_cmp( argument, "acid" )  &&ch->curr_talent[TAL_WATER] >= 30  &&ch->curr_talent[TAL_EARTH] >= 10 )
   {
      act( AT_GREEN, "A glistening bubble of acid encompasses you.", ch, NULL, NULL, TO_CHAR );
      act( AT_GREEN, "A glistening bubble of acid encompasses $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_ACID;
      tal = TAL_WATER;
   }
   else if( !str_cmp( argument, "dagger" )  &&ch->curr_talent[TAL_CATALYSM] >= 30  &&ch->curr_talent[TAL_EARTH] >= 10 )
   {
      act( AT_WHITE, "Dozens of dancing daggers twirl around you.", ch, NULL, NULL, TO_CHAR );
      act( AT_WHITE, "Dozens of dancing daggers twirl around $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_PIERCE;
      tal = TAL_CATALYSM;
   }
   else if( !str_cmp( argument, "dream" )  &&ch->curr_talent[TAL_DREAM] >= 40 )
   {
      act( AT_PINK, "Wispy dreams and whispered thoughts surround you.", ch, NULL, NULL, TO_CHAR );
      act( AT_PINK, "Wispy dreams and whispered thoughts surround $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_DREAM;
      tal = TAL_DREAM;
   }
   else if( !str_cmp( argument, "energy" )  &&ch->curr_talent[TAL_CATALYSM] >= 40 )
   {
      act( AT_PURPLE, "A rippling shield of energy surrounds you.", ch, NULL, NULL, TO_CHAR );
      act( AT_PURPLE, "A rippling shield of energy surrounds $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_ENERGY;
      tal = TAL_CATALYSM;
   }
   else if( !str_cmp( argument, "fire" )  &&ch->curr_talent[TAL_FIRE] >= 40 )
   {
      act( AT_FIRE, "Mystical flames rise to enshroud you.", ch, NULL, NULL, TO_CHAR );
      act( AT_FIRE, "Mystical flames rise to enshroud $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_FIRE;
      tal = TAL_FIRE;
   }
   else if( !str_cmp( argument, "ice" )  &&ch->curr_talent[TAL_FROST] >= 40 )
   {
      act( AT_FROST, "A glistening hail of ice encompasses you.", ch, NULL, NULL, TO_CHAR );
      act( AT_FROST, "A glistening hail of ice encompasses $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_COLD;
      tal = TAL_FROST;
   }
   else if( !str_cmp( argument, "light" )  &&ch->curr_talent[TAL_ILLUSION] >= 40 )
   {
      act( AT_WHITE, "Searing light radiates from you.", ch, NULL, NULL, TO_CHAR );
      act( AT_WHITE, "Searing light radiates from $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_LIGHT;
      tal = TAL_ILLUSION;
   }
   else if( !str_cmp( argument, "mana" )  &&ch->curr_talent[TAL_CATALYSM] >= 60 )
   {
      act( AT_PURPLE, "A field of prismatic mana surrounds you.", ch, NULL, NULL, TO_CHAR );
      act( AT_PURPLE, "A field of prismatic mana surrounds $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_ALL;
      tal = TAL_CATALYSM;
   }
   else if( !str_cmp( argument, "poison" )  &&ch->curr_talent[TAL_DEATH] >= 40 )
   {
      act( AT_DGREEN, "Deadly venom oozes from your skin.", ch, NULL, NULL, TO_CHAR );
      act( AT_DGREEN, "Deadly venom oozes from $n's skin.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_POISON;
      tal = TAL_DEATH;
   }
   else if( !str_cmp( argument, "shock" )  &&ch->curr_talent[TAL_LIGHTNING] >= 40 )
   {
      act( AT_YELLOW, "Torrents of cascading energy form around you.", ch, NULL, NULL, TO_CHAR );
      act( AT_YELLOW, "Torrents of cascading energy form around $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_ELECTRICITY;
      tal = TAL_LIGHTNING;
   }
   else if( !str_cmp( argument, "sound" )  &&ch->curr_talent[TAL_SPEECH] >= 40 )
   {
      act( AT_PINK, "A discordant wail of deafening sound surrounds you.", ch, NULL, NULL, TO_CHAR );
      act( AT_PINK, "A discordant wail of deafening sound surrounds $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_SOUND;
      tal = TAL_SPEECH;
   }
   else if( !str_cmp( argument, "stone" )  &&ch->curr_talent[TAL_EARTH] >= 40 )
   {
      act( AT_DGREY, "A hail of stones rotates around you.", ch, NULL, NULL, TO_CHAR );
      act( AT_DGREY, "A hail of stones rotates around $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_BLUNT;
      tal = TAL_EARTH;
   }
   else if( !str_cmp( argument, "sword" )  &&ch->curr_talent[TAL_CATALYSM] >= 10  &&ch->curr_talent[TAL_EARTH] >= 30 )
   {
      act( AT_WHITE, "A dozen dancing swords spin around you.", ch, NULL, NULL, TO_CHAR );
      act( AT_WHITE, "A dozen dancing swords spin around $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_SLASH;
      tal = TAL_EARTH;
   }
   else if( !str_cmp( argument, "thought" )  &&ch->curr_talent[TAL_MIND] >= 40 )
   {
      act( AT_PURPLE, "You concentrate, forming a mental barrier.", ch, NULL, NULL, TO_CHAR );
      act( AT_PURPLE, "$n concentrates, forming a mental barrier.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_PSIONIC;
      tal = TAL_MIND;
   }
   else if( !str_cmp( argument, "vines" )  &&ch->curr_talent[TAL_EARTH] >= 40 )
   {
      act( AT_GREEN, "An array of thorny vines surrounds you.", ch, NULL, NULL, TO_CHAR );
      act( AT_GREEN, "An array of thorny vines surrounds $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_EARTH;
      tal = TAL_EARTH;
   }
   else if( !str_cmp( argument, "warp" ) 
            &&( ch->curr_talent[TAL_CHANGE] >= 40  ||ch->curr_talent[TAL_TIME] >= 40  ||ch->curr_talent[TAL_VOID] >= 40 ) )
   {
      act( AT_DBLUE, "The air warps and distorts around you.", ch, NULL, NULL, TO_CHAR );
      act( AT_DBLUE, "The air warps and distorts around $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_CHANGE;
      tal = TAL_CHANGE;
   }
   else if( !str_cmp( argument, "water" )  &&ch->curr_talent[TAL_WATER] >= 40 )
   {
      act( AT_BLUE, "Foaming waves rise to encompass you.", ch, NULL, NULL, TO_CHAR );
      act( AT_BLUE, "Foaming waves rise to encompass $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_WATER;
      tal = TAL_WATER;
   }
   else if( !str_cmp( argument, "wind" )  &&ch->curr_talent[TAL_WIND] >= 40 )
   {
      act( AT_CYAN, "Swirling gusts of wind blow around you.", ch, NULL, NULL, TO_CHAR );
      act( AT_CYAN, "Swirling gusts of wind blow around $n.", ch, NULL, NULL, TO_ROOM );
      ch->shield = MAG_WIND;
      tal = TAL_WIND;
   }
   else
   {
      send_to_char( "You can't form any such shield.\n\r", ch );
      return;
   }
   use_magic( ch, tal, 500 - ( ch->curr_talent[tal] * 4 ) );
   WAIT_STATE( ch, PULSE_VIOLENCE );
}
void do_bind( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   char arg[MAX_STRING_LENGTH];
   OBJ_DATA * obj;
   DEITY_DATA * talent;
   if( ch->curr_talent[TAL_CATALYSM] < 60 )
   {
      huh( ch );
      return;
   }
   argument = one_argument( argument, arg );
   victim = find_target( ch, argument, FALSE );
   if( !victim )
      return;
   obj = get_obj_carry( ch, arg );
   if( !obj )
   {
      send_to_char( "You don't have one of those to bind.\r\n", ch );
      return;
   }
   if( IS_OBJ_STAT( obj, ITEM_LOYAL )  ||IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      send_to_char( "Your magic twists around it but fails to take hold.\n\r", ch );
      return;
   }
   if( obj->mana < 500 )
   {
      send_to_char( "The mana in this item is insufficient to hold the binding.\n\r", ch );
      return;
   }
   if( ch->pcdata->inborn == -1 )
      talent = get_talent_by_index( TAL_CATALYSM );
   
   else
      talent = get_talent_by_index( ch->pcdata->inborn );
   use_magic( ch, TAL_CATALYSM, 1000 - ch->curr_talent[TAL_CATALYSM] * 9 );
   obj->mana -= 500;
   obj->cost += 500000;
   xSET_BIT( obj->extra_flags, ITEM_LOYAL );
   act( AT_MAGIC, "You bind $p to $N.", ch, obj, victim, TO_CHAR );
   act( AT_BLUE, "$T $p, binding it with power.", ch, obj, talent->message, TO_ROOM );
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
}
void do_brand( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA * obj;
   int tal;
   argument = one_argument( argument, arg );
   obj = get_obj_carry( ch, arg );
   if( !obj )
   {
      send_to_char( "You don't have one of those to brand.\r\n", ch );
      return;
   }
   if( obj->item_type != ITEM_WEAPON  &&obj->item_type != ITEM_MISSILE_WEAPON )
   {
      ch_printf( ch, "You can only brand weapons.\n\r" );
      return;
   }
   if( obj->value[4] )
   {
      ch_printf( ch, "That item is already branded.\n\r" );
      return;
   }
   if( obj->mana < 500 )
   {
      ch_printf( ch, "That item lacks the mana to sustain the enchantment.\n\r" );
      return;
   }
   if( !str_cmp( argument, "fire" )  &&ch->curr_talent[TAL_FIRE] >= 50 )
   {
      act( AT_FIRE, "$p bursts into flame!", ch, obj, NULL, TO_CHAR );
      act( AT_FIRE, "$n's $p bursts into flame!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_FLAMING;
      tal = TAL_FIRE;
   }
   else if( !str_cmp( argument, "frost" )  &&ch->curr_talent[TAL_FROST] >= 50 )
   {
      act( AT_FROST, "$p shimmers with cold!", ch, obj, NULL, TO_CHAR );
      act( AT_FROST, "$n's $p shimmers with cold!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_FROZEN;
      tal = TAL_FROST;
   }
   else if( !str_cmp( argument, "lightning" )  &&ch->curr_talent[TAL_LIGHTNING] >= 50 )
   {
      act( AT_YELLOW, "$p crackles with electricity!", ch, obj, NULL, TO_CHAR );
      act( AT_YELLOW, "$n's $p crackles with electricity!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_ELEC;
      tal = TAL_LIGHTNING;
   }
   else if( !str_cmp( argument, "acid" )  &&ch->curr_talent[TAL_WATER] >= 50 )
   {
      act( AT_YELLOW, "$p hisses with acid!", ch, obj, NULL, TO_CHAR );
      act( AT_YELLOW, "$n's $p hisses with acid!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_ACID;
      tal = TAL_WATER;
   }
   else if( !str_cmp( argument, "antimatter" )  &&ch->curr_talent[TAL_VOID] >= 60 )
   {
      act( AT_DGREY, "Black lightning crackles around $p!", ch, obj, NULL, TO_CHAR );
      act( AT_DGREY, "Black lightning crackles around $n's $p!", ch, obj, NULL, TO_ROOM );
      xREMOVE_BIT( obj->extra_flags, ITEM_GLOW );
      xSET_BIT( obj->extra_flags, ITEM_DARK );
      obj->value[4] = BRAND_ANTI_MATTER;
      tal = TAL_VOID;
   }
   else if( !str_cmp( argument, "sonic" )  &&ch->curr_talent[TAL_SPEECH] >= 50 )
   {
      act( AT_GREEN, "$p emits a sonic hum!", ch, obj, NULL, TO_CHAR );
      act( AT_GREEN, "$n's $p emits a sonic hum!", ch, obj, NULL, TO_ROOM );
      xSET_BIT( obj->extra_flags, ITEM_HUM );
      obj->value[4] = BRAND_SONIC;
      tal = TAL_SPEECH;
   }
   else if( !str_cmp( argument, "distort" ) 
            &&ch->curr_talent[TAL_CHANGE]  +ch->curr_talent[TAL_TIME]  +ch->curr_talent[TAL_VOID] >= 60 )
   {
      act( AT_DBLUE, "The air warps and twists around $p!", ch, obj, NULL, TO_CHAR );
      act( AT_DBLUE, "The air warps and twists around $n's $p!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_DISTORT;
      tal = TAL_CHANGE;
   }
   else if( !str_cmp( argument, "venom" )  &&ch->curr_talent[TAL_DEATH] >= 50 )
   {
      act( AT_GREEN, "$p drips with venom!", ch, obj, NULL, TO_CHAR );
      act( AT_GREEN, "$n's $p drips with venom!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_POISON;
      tal = TAL_DEATH;
   }
   else if( !str_cmp( argument, "vorpal" )  &&ch->curr_talent[TAL_CATALYSM] >= 60 )
   {
      if( obj->value[5] != SK_SWORD 
           &&obj->value[5] != SK_DAGGER 
           &&obj->value[5] != SK_AXE 
           &&obj->value[5] != SK_HALBERD 
           &&obj->value[5] != SK_SPEAR  &&obj->value[5] != SK_POLEARM  &&obj->value[5] != SK_CLAW  )
      {
         send_to_char( "You can only vorpalize edged weapons.\r\n", ch );
         return;
      }
      act( AT_WHITE, "$p glints razor-sharp!", ch, obj, NULL, TO_CHAR );
      act( AT_WHITE, "$n's $p glints razor-sharp!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_VORPAL;
      tal = TAL_CATALYSM;
   }
   else if( !str_cmp( argument, "storm" )  &&ch->curr_talent[TAL_WIND] >= 60 )
   {
      act( AT_CYAN, "Stormy winds swirl about $p!", ch, obj, NULL, TO_CHAR );
      act( AT_CYAN, "Stormy winds swirl about $n's $p!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_STORM;
      tal = TAL_WIND;
   }
   else if( !str_cmp( argument, "radiant" )  &&ch->curr_talent[TAL_ILLUSION] >= 50 )
   {
      act( AT_YELLOW, "$p glows with a blinding radiance!", ch, obj, NULL, TO_CHAR );
      act( AT_YELLOW, "$n's $p glows with a blinding radiance!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_RADIANT;
      xREMOVE_BIT( obj->extra_flags, ITEM_DARK );
      xSET_BIT( obj->extra_flags, ITEM_GLOW );
      tal = TAL_ILLUSION;
   }
   else if( !str_cmp( argument, "crush" )  &&ch->curr_talent[TAL_EARTH] >= 60 )
   {
      if( obj->value[5] != SK_MACE  &&obj->value[5] != SK_STAFF  &&obj->value[5] != SK_WHIP  )
      {
         send_to_char( "Only blunt weapons can crush.\n\r", ch );
         return;
      }
      act( AT_DGREY, "$p becomes harder than rock!", ch, obj, NULL, TO_CHAR );
      act( AT_DGREY, "$n's $p becomes harder than rock!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_CRUSH;
      tal = TAL_EARTH;
   }
   else if( !str_cmp( argument, "drain" )  &&ch->curr_talent[TAL_DEATH] >= 60 )
   {
      act( AT_DGREY, "$p thirsts for blood!", ch, obj, NULL, TO_CHAR );
      act( AT_DGREY, "$n's $p thirsts for blood!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_DRAIN;
      tal = TAL_DEATH;
   }
   else if( !str_cmp( argument, "psionic" )  &&ch->curr_talent[TAL_MIND] >= 50 )
   {
      act( AT_PURPLE, "A strange humming sound emanates from $p!", ch, obj, NULL, TO_CHAR );
      act( AT_PURPLE, "A strange humming sound emanates from $n's $p!", ch, obj, NULL, TO_ROOM );
      obj->value[4] = BRAND_PSIONIC;
      xSET_BIT( obj->extra_flags, ITEM_HUM );
      tal = TAL_MIND;
   }
   else
   {
      send_to_char( "You can't make any such brand.\n\r", ch );
      return;
   }
   obj->mana -= 500;
   obj->cost += 500000;
   use_magic( ch, tal, 200 - ch->curr_talent[tal] );
   WAIT_STATE( ch, PULSE_VIOLENCE );
   sprintf( buf, "%s%s", weapon_brand[obj->value[4]], myobj( obj ) );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
}
void do_funnel( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   int amt;
   int magic;
   OBJ_DATA * obj;
   obj = get_obj_carry( ch, argument );
   if( obj )
   {
      act( AT_MAGIC, "You funnel raw energy into $p.", ch, obj, NULL, TO_CHAR );
      if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
      {
         send_to_char( "The magic is absorbed with no effect.\n\r", ch );
         return;
      }
      amt = get_curr_wil( ch ) * 10;
      use_magic( ch, TAL_CATALYSM, amt );
      amt /= obj->weight == 0 ? 1 : obj->weight;
      obj->raw_mana += amt;
      if( obj->gem )
         obj->gem->raw_mana += amt;
      if( obj->raw_mana > 2000  +TALENT( ch, TAL_CATALYSM ) * 5  +ch->pcdata->noncombat[SK_CHANNEL] * 20 )
      {
         act( AT_DANGER, "The overcharged item explodes in your hands!", ch, NULL, NULL, TO_CHAR );
         act( AT_ACTION, "$p explodes in $n's hands.", ch, obj, NULL, TO_ROOM );
         lose_hp( ch, obj->mana * 3 + ( obj->raw_mana / 30 ) );
         extract_obj( obj );
      }
      if( obj->raw_mana > 2000 )
         act( AT_PURPLE, "$p vibrates violently!", ch, obj, NULL, TO_CHAR );
      return;
   }
   if( ch->curr_talent[TAL_CATALYSM] < 30 )
   {
      send_to_char( "You have nothing like that.\n\r", ch );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   if( ch == victim )
      return;
   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
   {
      send_to_char( "You shouldn't do that to a pacifist.\n\r", ch );
      return;
   }
   magic = UMIN( ch->curr_talent[TAL_CATALYSM], 100 );
   magic = 100 / magic;
   amt = ch->mana / magic;
   WAIT_STATE( ch, PULSE_VIOLENCE );
   if( victim->shield == MAG_ALL  ||IS_SET( ch->pcdata->flags, PCFLAG_CHEAT ) )
   {
      act( AT_PURPLE, "Your mana bounces off $N's shield!", ch, NULL, victim, TO_CHAR );
      act( AT_PURPLE, "$n's mana bounces off your shield!", ch, NULL, victim, TO_VICT );
      act( AT_PURPLE, "$n's mana bounces off $N's shield!", ch, NULL, victim, TO_NOTVICT );
      ch->mana = ch->max_mana + amt;
      return;
   }
   act( AT_PURPLE, "You funnel your magic into $N's body!", ch, NULL, victim, TO_CHAR );
   act( AT_PURPLE, "$n glows brightly, funnelling $s magic into you!", ch, NULL, victim, TO_VICT );
   act( AT_PURPLE, "$n glows brightly, funnelling $s magic into $N!", ch, NULL, victim, TO_NOTVICT );
   ch->mana -= amt;
   victim->mana += amt;
   return;
}
void do_dream( CHAR_DATA * ch, char *argument )
{
   int i, vnum;
   ROOM_INDEX_DATA * pRoomIndex;
   AREA_DATA * pArea;
   CHAR_DATA * victim;
   char arg[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA * location;
   ROOM_INDEX_DATA * original;
   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "enter" )  &&ch->curr_talent[TAL_DREAM] >= 10 )
   {
      if( IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      {
         act( AT_MAGIC, "You are already in the dreamworld!", ch, NULL, NULL, TO_CHAR );
         return;
      }
      if( IS_AFFECTED( ch, AFF_VOID ) )
      {
         send_to_char( "The dream twists your mind!\n\r", ch );
         ch->mental_state = 100;
         return;
      }
      if( IS_SET( ch->in_room->room_flags, ROOM_NO_DREAM ) 
            &&( ch->curr_talent[TAL_DREAM] < 100 
                ||( IS_SET( ch->in_room->runes, RUNE_WARD )  &&ch->curr_talent[TAL_DREAM] < 200 ) ) )
      {
         act( AT_MAGIC, "For some reason, you cannot enter the dream here.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      act( AT_MAGIC, "You step into the world of dreams.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n slowly fades out of the waking world.", ch, NULL, NULL, TO_ROOM );
      xSET_BIT( ch->pcdata->perm_aff, AFF_DREAMWORLD );
      ch->position = POS_STANDING;
      act( AT_MAGIC, "$n slowly fades into the dream.", ch, NULL, NULL, TO_ROOM );
   }
      else if( !str_cmp( arg, "leave" ) && ch->curr_talent[TAL_DREAM] >= 10 )
   {
      if( !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      {
         send_to_char( "You aren't in the Dream!\n\r", ch );
         return;
      }
      act( AT_MAGIC, "You step out of the Dream.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n steps out of the Dream.", ch, NULL, NULL, TO_ROOM );
      xREMOVE_BIT( ch->pcdata->perm_aff, AFF_DREAMWORLD );
      act( AT_MAGIC, "$n steps into reality.", ch, NULL, NULL, TO_ROOM );
   }
   else if( !str_cmp( arg, "pull" )  &&ch->curr_talent[TAL_DREAM] >= 40 )
   {
      victim = get_char_room( ch, argument );
      if( !victim )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
      if( !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      {
         act( AT_MAGIC, "You need to be in the dreamworld to cast that.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      if( victim->position != POS_SLEEPING )
      {
         act( AT_MAGIC, "$N needs to be asleep to pull $M into your dream.", ch, NULL, victim, TO_CHAR );
         return;
      }
      act( AT_MAGIC, "You pull $N into the dreamworld!", ch, NULL, victim, TO_CHAR );
      victim->position = POS_STANDING;
      if( IS_NPC( victim ) )
         xSET_BIT( victim->affected_by, AFF_DREAMWORLD );
      
      else
         xSET_BIT( victim->pcdata->perm_aff, AFF_DREAMWORLD );
      act( AT_MAGIC, "Your dreams seem to change, and become more vivid.", ch, NULL, victim, TO_VICT );
      do_look( victim, "auto" );
      return;
   }
   else if( !str_cmp( arg, "rip" )  &&ch->curr_talent[TAL_DREAM] >= 60 )
   {
      victim = get_char_world( ch, argument );
      if( !victim )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
      if( victim == ch 
            ||!victim->in_room  ||!IS_AFFECTED( ch, AFF_DREAMWORLD )  ||!IS_AFFECTED( victim, AFF_DREAMWORLD ) )
         
      {
         send_to_char( "You both need to be in dreamworld for this spell.", ch );
         return;
      }
      act( AT_MAGIC, "You are ripped out of your dream and $n pulls you into $s dream!", ch, NULL, victim, TO_VICT );
      act( AT_MAGIC, "$n is ripped out of the dream!", victim, NULL, NULL, TO_ROOM );
      char_from_room( victim );
      victim->in_obj = NULL;
      char_to_room( victim, ch->in_room );
      act( AT_MAGIC, "$n is ripped into the dream!", victim, NULL, NULL, TO_ROOM );
      do_look( victim, "auto" );
   }
   else if( !str_cmp( arg, "shift" )  &&ch->curr_talent[TAL_DREAM] >= 30 )
   {
      if( !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      {
         act( AT_MAGIC, "You need to be in the dreamworld to cast that.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      victim = get_char_world( ch, argument );
      if( !victim )
      {
         
            /*
             * Start travel to location 
             */ 
            for( pArea = first_area; pArea; pArea = pArea->next )
            
         {
            if( ( arg[0] == '\0' || nifty_is_name_prefix( argument, pArea->name ) ) 
                 &&!IS_SET( pArea->flags, AFLAG_NOTRAVEL ) )
               
            {
               for( i = 0; i < 20; i++ )
               {
                  vnum = number_range( pArea->low_r_vnum, pArea->hi_r_vnum );
                  if( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
                     continue;
                  if( IS_SET( pRoomIndex->room_flags, ROOM_NO_DREAM ) )
                     continue;
                  break;
               }
               if( !pRoomIndex )
               {
                  send_to_char( "You can't find a place to land.\n\r", ch );
                  return;
               }
               act( AT_MAGIC, "You shift into another dream.", ch, NULL, NULL, TO_CHAR );
               act( AT_MAGIC, "$n fades out of existence.", ch, NULL, NULL, TO_ROOM );
               char_from_room( ch );
               ch->in_obj = NULL;
               char_to_room( ch, pRoomIndex );
               act( AT_MAGIC, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
               do_look( ch, "auto" );
            }
         }
         send_to_char( "You can't find any place like that.\n\r", ch );
         return;
      }
      if( victim == ch )
      {
         send_to_char( "That was a short trip!\n\r", ch );
         return;
      }
      if( IS_SET( victim->in_room->room_flags, ROOM_NO_DREAM ) 
            &&( ch->curr_talent[TAL_DREAM] < 100 
                ||( IS_SET( victim->in_room->runes, RUNE_WARD )  &&ch->curr_talent[TAL_DREAM] < 200 ) ) )
      {
         act( AT_MAGIC, "You are unable to locate anyone like that.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      act( AT_MAGIC, "You shift into another dream.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n fades out of existence.", ch, NULL, NULL, TO_ROOM );
      char_from_room( ch );
      ch->in_obj = NULL;
      char_to_room( ch, victim->in_room );
      act( AT_MAGIC, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
      do_look( ch, "auto" );
   }
   else if( !str_cmp( arg, "watch" )  &&ch->curr_talent[TAL_DREAM] >= 25 )
   {
      victim = get_char_world( ch, argument );
      if( !victim )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
      if( !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      {
         act( AT_MAGIC, "You need to be in the dreamworld to cast that.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      location = victim->in_room;
      if( !location )
         
      {
         send_to_char( "You can't find them.\n\r", ch );
         return;
      }
      original = ch->in_room;
      char_from_room( ch );
      char_to_room( ch, location );
      do_look( ch, "auto" );
      char_from_room( ch );
      char_to_room( ch, original );
   }
   else
   {
      send_to_char( "You don't know how to do that.\n\r", ch );
      return;
   }
   use_magic( ch, TAL_DREAM, 200 - ch->curr_talent[TAL_DREAM] );
   WAIT_STATE( ch, PULSE_VIOLENCE );
}
void do_connect( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_MIND] < 10 )
   {
      huh( ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Telepathically connect to whom?\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg );
   victim = get_char_world( ch, arg );
   if( ( !victim ) || IS_NPC( victim ) || IS_NPC( ch ) )
      
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_SET( ch->pcdata->flags, PCFLAG_SHIELD ) 
         ||( IS_SET( victim->pcdata->flags, PCFLAG_SHIELD )  &&TALENT( ch, TAL_MIND ) < TALENT( victim, TAL_MIND ) ) )
   {
      send_to_char( "A shield blocks you from connecting.\n\r", ch );
      return;
   }
   if( argument[0] != '\0' && ch != victim )
   {
      sprintf( buf, "&pYou tell %s telepathically '%s'\n\r", victim->name, argument );
      send_to_char( buf, ch );
      sprintf( buf, "&p%s telepathically tells you '%s'\n\r", ch->name, argument );
      send_to_char( buf, victim );
      use_magic( ch, TAL_MIND, 10 );
      return;
   }
   if( ch == victim )
   {
      if( ch->pcdata->mindlink )
      {
         victim = ch->pcdata->mindlink;
         send_to_char( "Your telepathic connection is broken.\n\r", victim );
         victim->pcdata->mindlink = NULL;
         send_to_char( "You break your telepathic connection.\n\r", ch );
         ch->pcdata->mindlink = NULL;
      }
      else
      {
         send_to_char( "You aren't telepathically connected.\n\r", ch );
      }
      return;
   }
   if( ch->pcdata->mindlink )
   {
      send_to_char( "You are already linked to someone.\n\r", ch );
      return;
   }
   if( victim->pcdata->mindlink )
   {
      send_to_char( "They are already linked to someone.\n\r", ch );
      return;
   }
   use_magic( ch, TAL_MIND, 25 );
   sprintf( buf, "You open your mind in a telepathic connection to %s.\n\r", victim->name );
   send_to_char( buf, ch );
   sprintf( buf, "%s opens a telepathic connection to you.\n\r", ch->name );
   send_to_char( buf, victim );
   ch->pcdata->mindlink = victim;
   victim->pcdata->mindlink = ch;
}
void do_think( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->pcdata->mindlink )
   {
      victim = ch->pcdata->mindlink;
   }
   else
   {
      ch_printf( ch, "&pYou think '%s'\n\r", argument );
      for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
      {
         if( victim == ch )
            continue;
         if( TALENT( victim, TAL_MIND ) >= number_percent(  ) )
            ch_printf( victim, "&p%s thinks '%s'\n\r", ch->name, argument );
      }
      return;
   }
   ch_printf( ch, "&pYou think '%s'\n\r", argument );
   ch_printf( victim, "&p%s thinks '%s'\n\r", ch->name, argument );
}


/*
 * Improved version of reanimate -keo
 * Now restores exp lost from death, July 9, 2001
 */ 
void do_resurrect( CHAR_DATA * ch, char *argument ) 
{
   CHAR_DATA * mob;
   OBJ_DATA * corpse;
   OBJ_DATA * obj;
   OBJ_DATA * obj_next;
   MOB_INDEX_DATA * pMobIndex;
   AFFECT_DATA af;
   CHAR_DATA * victim;
   DESCRIPTOR_DATA * d;
   PART_DATA * part;
   int amt;
   if( ch->curr_talent[TAL_DEATH] < 100 || ch->curr_talent[TAL_HEALING] < 100 )
   {
      huh( ch );
      return;
   }
   corpse = get_obj_here( ch, argument );
   if( !corpse )
   {
      send_to_char( "You see nothing like that here.\n\r", ch );
      return;
   }
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING )
         if( nifty_is_name( d->character->name, corpse->name ) )
         {
            victim = d->character;
            if( ch->mana < victim->perm_con * 10 )
            {
               send_to_char( "You don't have enough energy to resurrect this corpse.\n\r", ch );
               return;
            }
            if( !IS_CONSENTING( victim, ch ) )
            {
               send_to_char( "You cannot resurrect them without their consent.\n\r", ch );
               return;
            }
            if( !IS_AFFECTED( victim, AFF_ETHEREAL ) )
            {
               send_to_char( "You cannot resurrect them when they are not ethereal.\n\r", ch );
               return;
            }
            while( ( obj = victim->first_carrying ) )
            {
               obj_from_char( obj );
               obj_to_room( obj, victim->in_room );
            }
            while( victim->first_affect )
               affect_remove( victim, victim->first_affect );
            act( AT_DGREY, "$n screams in agony as $e is drawn back into $s body.", victim, NULL, NULL, TO_ROOM );
            act( AT_DGREY, "You scream in agony as you are drawn back into your body.", victim, NULL, NULL, TO_CHAR );
            if( victim->in_room != ch->in_room )
            {
               char_from_room( victim );
               victim->in_obj = NULL;
               char_to_room( victim, ch->in_room );
            }
            part = victim->first_part;
            while( part )
            {
               part->cond = PART_WELL;
               part->flags = PART_WELL;
               part = part->next;
            }
            victim->exp += corpse->cost;
            if( get_timer( victim, TIMER_PKILLED ) > 0 )
               remove_timer( victim, TIMER_PKILLED );
            use_magic( ch, TAL_HEALING, 1500 );
            act( AT_WHITE, "A radiant white aura surrounds the corpse of $n.", victim, NULL, NULL, TO_ROOM );
            while( ( obj = corpse->first_content ) )
            {
               obj_from_obj( obj );
               if( obj->item_type == ITEM_MONEY )
               {
                  amt = obj->value[0];
                  ch->gold += amt;
                  extract_obj( obj );
               }
               else
               {
                  obj_to_char( obj, victim );
               }
            }
            separate_obj( corpse );
            extract_obj( corpse );
            fix_char( victim );
            victim->position = POS_RESTING;
            act( AT_WHITE, "$n's eyes flutter open, and $e takes in a deep breath.", victim, NULL, NULL, TO_ROOM );
            act( AT_WHITE, "Your eyes flutter open, and you take in a deep breath.", victim, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "You feel drained and exhausted from the resurrection.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = -100;
            WAIT_STATE( ch, PULSE_VIOLENCE * 4 );
            WAIT_STATE( victim, PULSE_VIOLENCE * 2 );
            
               /*
                * If they were undead, resurrection will revive them 
                */ 
               if( xIS_SET( victim->pcdata->perm_aff, AFF_UNDEAD ) )
            {
               STRFREE( victim->pcdata->type );
               victim->pcdata->type = STRALLOC( "living" );
               xCLEAR_BITS( victim->pcdata->perm_aff );
               REMOVE_BIT( victim->pcdata->flags, PCFLAG_VAMPIRE );
            }
            return;
         }
   }   /* end player resurrection */
   
      /*
       * mob resurrection disabled for the moment 
       */ 
      send_to_char( "You can't resurrect that.\n\r", ch );
   return;
   if( ( pMobIndex = get_mob_index( ( sh_int ) abs( corpse->cost ) ) ) == NULL )
      
   {
      bug( "Can not find mob for cost of corpse, do_resurrect", 0 );
      return;
   }
   if( ch->mana - ( pMobIndex->perm_con * 50 ) < 0 )
      
   {
      send_to_char( "You do not have enough mana to resurrect this corpse.\n\r", ch );
      return;
   }
   if( ( chance( ch, 75 )  &&pMobIndex->perm_con - ch->curr_talent[TAL_DEATH] < 10 ) )
   {
      mob = create_mobile( pMobIndex );
      char_to_room( mob, ch->in_room );
      use_magic( ch, TAL_HEALING, ch->mana );
      act( AT_MAGIC, "$n resurrect $T!", ch, NULL, pMobIndex->short_descr, TO_ROOM );
      act( AT_MAGIC, "You resurrect $T!", ch, NULL, pMobIndex->short_descr, TO_CHAR );
      add_follower( mob, ch );
      if( corpse->timer > 0 )
      {
         af.type = gsn_charm_person;
         af.duration = ( number_fuzzy( corpse->timer ) * DUR_CONV );
         af.location = 0;
         af.modifier = 0;
         af.bitvector = meb( AFF_CHARM );
         affect_to_char( mob, &af );
      }
      else
      {
         xSET_BIT( mob->affected_by, AFF_CHARM );
      }
      if( corpse->first_content )
         for( obj = corpse->first_content; obj; obj = obj_next )
            
         {
            obj_next = obj->next_content;
            obj_from_obj( obj );
            obj_to_char( obj, mob );
         }
      separate_obj( corpse );
      extract_obj( corpse );
      ch->mental_state = -100;
      WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
      return;
   }
   else
   {
      send_to_char( "You fail to reanimate the corpse.\n\r", ch );
      use_magic( ch, TAL_HEALING, 300 - ch->curr_talent[TAL_DEATH] * 2 );
      WAIT_STATE( ch, PULSE_VIOLENCE );
   }
}
void do_identify( CHAR_DATA * ch, char *argument ) 
{
   OBJ_DATA * obj;
   AFFECT_DATA * paf;
   SKILLTYPE * sktmp;
   CHAR_DATA * victim;
   char *name;
   if( ch->curr_talent[TAL_SEEKING] <= 0 )
      
   {
      send_to_char( "You are unable to do that.\n\r", ch );
      return;
   }
   if( argument[0] == '\0' )
      
   {
      send_to_char( "Identify what?\n\r", ch );
      return;
   }
   if( ( obj = get_obj_here( ch, argument ) ) == NULL )
      if( ( victim = get_char_room( ch, argument ) ) == NULL )
         
      {
         ch_printf( ch, "You can't find %s!\n\r", argument );
         return;
      }
   WAIT_STATE( ch, PULSE_VIOLENCE );
   use_magic( ch, TAL_SEEKING, 20 );
   if( ( obj = get_obj_here( ch, argument ) ) != NULL )
      
   {
      int gp;
      set_char_color( AT_LBLUE, ch );
      ch_printf( ch, "\n\rThe %s is %s", myobj( obj ), aoran( item_type_name( obj ) ) );
      if( !xIS_EMPTY( obj->parts ) )
         ch_printf( ch, ", worn on:  %s\n\r", ext_flag_string( &obj->parts, bp_flags ) );
      
      else
         send_to_char( ".\n\r", ch );
      if( ch->curr_talent[TAL_SEEKING] >= 10 )
         ch_printf( ch, "Special properties:  %s\n\rIts weight is %d.\r\n", extra_bit_name( &obj->extra_flags ),
                    obj->weight );
      gp = obj->cost;
      ch_printf( ch, "&wIt is worth &C%d mithril, ", ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "&Oand %d copper coins&C.\n\r", gp );
      if( ch->curr_talent[TAL_SEEKING] >= 5 )
         ch_printf( ch, "It appears to have been made for someone %d inches tall.\n\r", obj->size );
      if( ch->curr_talent[TAL_SEEKING] >= 10 && obj->material )
         ch_printf( ch, "It appears to be made of %s.\r\n", obj->material->name );
      if( ch->curr_talent[TAL_SEEKING] >= 25 )
         
      {
         if( obj->timer == 0 )
            ch_printf( ch, "It will not decay.\n\r" );
         
         else
            ch_printf( ch, "It will last another %d hours.\n\r", obj->timer );
      }
      if( obj->obj_by && ch->curr_talent[TAL_SEEKING] >= 50 )
         ch_printf( ch, "It was created by %s.\n\r", obj->obj_by );
      if( obj->raw_mana && ch->curr_talent[TAL_SEEKING] >= 5 )
         ch_printf( ch, "This item %s with raw mana.\n\r",
                    obj->raw_mana > 1500 ? "radiates brightly" : obj->raw_mana > 1000 ? "shines" : obj->raw_mana >
                    500 ? "vibrates" : "tingles" );
      if( obj->mana && ch->curr_talent[TAL_SEEKING] >= 40 )
         ch_printf( ch, "%d points of energy have been absorbed into this item.\n\r", obj->mana );
      set_char_color( AT_MAGIC, ch );
      switch ( obj->item_type )
         
      {
         case ITEM_CONTAINER:
            ch_printf( ch, "%s appears to be %s.\n\r", capitalize( obj->short_descr ),
                        obj->value[0] < 76 ? "of a small capacity" : obj->value[0] <
                        150 ? "of a small to medium capacity" : obj->value[0] <
                        300 ? "of a medium capacity" : obj->value[0] <
                        550 ? "of a medium to large capacity" : obj->value[0] <
                        751 ? "of a large capacity" : "of a giant capacity" );
            break;
         case ITEM_PILL:
         case ITEM_SCROLL:
         case ITEM_POTION:
            if( ch->curr_talent[TAL_SEEKING] >= 20 )
            {
               ch_printf( ch, "Casts:" );
               if( obj->value[1] >= 0 && ( sktmp = get_skilltype( obj->value[1] ) ) != NULL )
                  
               {
                  send_to_char( " '", ch );
                  send_to_char( sktmp->name, ch );
                  send_to_char( "'", ch );
               }
               if( obj->value[2] >= 0 && ( sktmp = get_skilltype( obj->value[2] ) ) != NULL )
                  
               {
                  send_to_char( " '", ch );
                  send_to_char( sktmp->name, ch );
                  send_to_char( "'", ch );
               }
               if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
                  
               {
                  send_to_char( " '", ch );
                  send_to_char( sktmp->name, ch );
                  send_to_char( "'", ch );
               }
               send_to_char( ".\n\r", ch );
            }
            break;
         case ITEM_SALVE:
            ch_printf( ch, "Has %d out of %d applications left", obj->value[1], obj->value[2] );
            if( ch->curr_talent[TAL_SEEKING] >= 20 )
            {
               ch_printf( ch, " of " );
               if( obj->value[4] >= 0 && ( sktmp = get_skilltype( obj->value[4] ) ) != NULL )
                  
               {
                  send_to_char( " '", ch );
                  send_to_char( sktmp->name, ch );
                  send_to_char( "'", ch );
               }
               if( obj->value[5] >= 0 && ( sktmp = get_skilltype( obj->value[5] ) ) != NULL )
                  
               {
                  send_to_char( " '", ch );
                  send_to_char( sktmp->name, ch );
                  send_to_char( "'", ch );
               }
            }
            send_to_char( ".\n\r", ch );
            break;
         case ITEM_WAND:
         case ITEM_STAFF:
            if( ch->curr_talent[TAL_SEEKING] >= 20 )
            {
               ch_printf( ch, "Has %d out of %d charges left of", obj->value[1], obj->value[2] );
               if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
                  
               {
                  send_to_char( " '", ch );
                  send_to_char( sktmp->name, ch );
                  send_to_char( "'", ch );
               }
               send_to_char( ".\n\r", ch );
            }
            break;
         case ITEM_WEAPON:
            if( obj->value[4] && obj->value[4] < 14 )
               ch_printf( ch, "This is a magically %s&z&Bweapon.\n\r", weapon_brand[obj->value[4]] );
            ch_printf( ch, "This weapon falls into the '%s' category.\n\r", weapon_skill[obj->value[5]] );
            if( ch->curr_talent[TAL_SEEKING] >= 40 && obj->value[0] )
            {
               if( ch->curr_talent[TAL_SEEKING] >= 70 )
               {
                  if( obj->value[0] <= 17 )
                     ch_printf( ch, "This weapon will inflict %d extra damage against all %s.\n\r", obj->value[6] / 10,
                                 weapon_bane[obj->value[0]] );
               }
               else
               {
                  ch_printf( ch, "This weapon is the bane of all %s.\n\r", weapon_bane[obj->value[0]] );
               }
            }
            ch_printf( ch, "Damage bonus is %d.\n\r", ( obj->value[2] ) / 2 );
            break;
         case ITEM_ARMOR:
            ch_printf( ch, "Armor class is %d.\n\r", obj->value[0] );
            break;
         case ITEM_MISSILE_WEAPON:
            if( obj->value[4] && obj->value[4] < 13 )
               ch_printf( ch, "This is a magically %s&z&Bweapon.\n\r", weapon_brand[obj->value[4]] );
            ch_printf( ch, "This weapon falls into the '%s' category.\n\r", weapon_skill[obj->value[5]] );
            ch_printf( ch, "Damage bonus is %d.\n\r", obj->value[2] );
            if( obj->value[5] != SK_WAND )
            {
               if( obj->value[0] == 0 )
               {
                  ch_printf( ch, "It is empty.\n\r" );
               }
               else
               {
                  ch_printf( ch, "%d shots have been loaded into this weapon.\n\r", obj->value[0] );
               }
            }
            break;
         case ITEM_SCANNER:
         case ITEM_RADIO:
            ch_printf( ch, "The volume is set at %d.\n\r", obj->value[0] );
            ch_printf( ch, "The frequency is set at %d.\n\r", obj->value[1] );
            break;
      }
      if( ch->curr_talent[TAL_SEEKING] >= 15 )
      {
         for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
            showaffect( ch, paf );
         for( paf = obj->first_affect; paf; paf = paf->next )
            showaffect( ch, paf );
      }
      if( obj->gem )
      {
         ch_printf( ch, "%s has been affixed to this item.\n\r", aoran( myobj( obj->gem ) ) );
         for( paf = obj->gem->first_affect; paf; paf = paf->next )
            showaffect( ch, paf );
      }
   }
   
      /*
       * START CHARACTER IDENTIFY 
       */ 
      else if( ( victim = get_char_room( ch, argument ) ) != NULL )
      
   {
      if( IS_NPC( victim ) )
         name = capitalize( victim->short_descr );
      
      else
         name = victim->name;
      ch_printf( ch, "%s looks like %s.\n\r", name, victim->nation ? aoran( victim->nation->name ) 
                   : get_race( victim ) );
      if( ch->curr_talent[TAL_SEEKING] >= 30 )
      {
         if( TALENT( ch, TAL_SEEKING ) > TALENT( victim, TAL_SECURITY ) )
         {
            ch_printf( ch, "%s's current stats: %-2d str %-2d wil %-2d int %-2d dex %2d con %-2d per %-2d end.\r\n",
                        victim->name, get_curr_str( victim ), get_curr_wil( victim ), get_curr_int( victim ),
                        get_curr_dex( victim ), get_curr_con( victim ), get_curr_per( victim ), get_curr_lck( victim ) );
         }
      }
      if( ch->curr_talent[TAL_SEEKING] >= 40 )
      {
         if( ( chance( ch, 50 ) && get_curr_wil( ch ) >= get_curr_wil( victim ) + 10 ) )
            
         {
            ch_printf( ch, "%s appears to be affected by: ", name );
            if( !victim->first_affect )
               
            {
               send_to_char( "nothing.\n\r", ch );
            }
            else
            {
               for( paf = victim->first_affect; paf; paf = paf->next )
                  
               {
                  if( victim->first_affect != victim->last_affect )
                     
                  {
                     if( paf != victim->last_affect && ( sktmp = get_skilltype( paf->type ) ) != NULL )
                        ch_printf( ch, "%s, ", sktmp->name );
                     if( paf == victim->last_affect && ( sktmp = get_skilltype( paf->type ) ) != NULL )
                        
                     {
                        ch_printf( ch, "and %s.\n\r", sktmp->name );
                     }
                  }
                  
                  else
                     
                  {
                     if( ( sktmp = get_skilltype( paf->type ) ) != NULL )
                        ch_printf( ch, "%s.\n\r", sktmp->name );
                     
                     else
                        send_to_char( "\n\r", ch );
                  }
               }
            }
         }
         if( !IS_NPC( victim ) )
         {
            DEITY_DATA * talent;
            ch_printf( ch, "%s's aura:\n\r", PERS( victim, ch ) );
            if( IS_SET( victim->pcdata->flags, PCFLAG_MASK ) 
                  &&TALENT( ch, TAL_SEEKING ) > TALENT( victim, TAL_SEEKING ) )
            {
               send_to_char( "You sense only a distorted fog.\n\r", ch );
               return;
            }
            for( talent = first_deity; talent; talent = talent->next )
            {
               if( victim->pcdata->inborn == talent->index )
               {
                  if( victim->talent[talent->index] >= 150 )
                     ch_printf( ch, "predominately radiant %s\n\r", talent->color );
                  
                  else if( victim->talent[talent->index] >= 100 )
                     ch_printf( ch, "primarily brilliant %s\n\r", talent->color );
                  
                  else if( victim->talent[talent->index] >= 50 )
                     ch_printf( ch, "mainly bright %s\n\r", talent->color );
                  
                  else
                     ch_printf( ch, "mostly pale %s\n\r", talent->color );
               }
               
               else if( victim->talent[talent->index] >= 200 )
                  ch_printf( ch, "blindingly brilliant %s\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 160  &&ch->curr_talent[TAL_SEEKING] >= 10 )
                  ch_printf( ch, "awash with a cascade of %s\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 130  &&ch->curr_talent[TAL_SEEKING] >= 20 )
                  ch_printf( ch, "raging torrents of brilliant %s\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 100  &&ch->curr_talent[TAL_SEEKING] >= 30 )
                  ch_printf( ch, "radiating %s brightly\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 75  &&ch->curr_talent[TAL_SEEKING] >= 50 )
                  ch_printf( ch, "bright streaks of %s\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 50  &&ch->curr_talent[TAL_SEEKING] >= 80 )
                  ch_printf( ch, "shining %s sparkles\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 30  &&ch->curr_talent[TAL_SEEKING] >= 120 )
                  ch_printf( ch, "traces of glittering %s\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 10  &&ch->curr_talent[TAL_SEEKING] >= 150 )
                  ch_printf( ch, "pale flecks of %s\n\r", talent->color );
               
               else if( victim->talent[talent->index] >= 1  &&ch->curr_talent[TAL_SEEKING] >= 200 )
                  ch_printf( ch, "faint tinge of %s barely visible\n\r", talent->color );
            }
         }
      }
   }
}
void do_enchant( CHAR_DATA * ch, char *argument ) 
{
   OBJ_DATA * obj;
   AFFECT_DATA * paf;
   int tal;
   int amt;
   char arg2[MAX_INPUT_LENGTH];
   bool found;
   argument = one_argument( argument, arg2 );
   obj = get_obj_carry( ch, arg2 );
   if( !obj )
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }
   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
      
   {
      act( AT_MAGIC, "Your magic twists and winds around $p but cannot take hold.", ch, obj, NULL, TO_CHAR );
      return;
   }
   separate_obj( obj );
   amt = 50;
   tal = TAL_CATALYSM;
   found = FALSE;
   if( obj->mana < amt )
   {
      send_to_char( "That item doesn't have enough energy to hold the enchantment.\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "magic" ) )
   {
      if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      {
         send_to_char( "That item is already magic.\n\r", ch );
         return;
      }
      if( ch->curr_talent[tal] < 40 )
      {
         send_to_char( "You can't imbue something with magic.\n\r", ch );
         return;
      }
      act( AT_MAGIC, "You imbue $p with a field of magic.", ch, obj, NULL, TO_CHAR );
      xSET_BIT( obj->extra_flags, ITEM_MAGIC );
   }
   else if( !str_cmp( argument, "shield" ) )
   {
      tal = TAL_SECURITY;
      amt = 500;
      if( IS_OBJ_STAT( obj, ITEM_SHIELD ) )
      {
         send_to_char( "That item already emits a forcefield.\n\r", ch );
         return;
      }
      if( ch->curr_talent[TAL_SECURITY]  +ch->curr_talent[TAL_CATALYSM] < 195 )
      {
         send_to_char( "You can't imbue something with a forcefield.\n\r", ch );
         return;
      }
      if( obj->mana < amt )
      {
         send_to_char( "That item does not have enough energy to sustain the enchantment.\n\r", ch );
         return;
      }
      act( AT_MAGIC, "You imbue $p with a protective forcefield.", ch, obj, NULL, TO_CHAR );
      xSET_BIT( obj->extra_flags, ITEM_SHIELD );
   }
   else if( !str_cmp( argument, "returning" ) )
   {
      tal = TAL_MOTION;
      amt = 100;
      if( IS_OBJ_STAT( obj, ITEM_RETURNING ) )
      {
         send_to_char( "That item is already returning.\n\r", ch );
         return;
      }
      if( ch->curr_talent[TAL_MOTION] < 50  &&TALENT( ch, TAL_MOTION ) < 50 )
      {
         send_to_char( "You can't enchant something to return when thrown.\n\r", ch );
         return;
      }
      if( obj->mana < amt )
      {
         send_to_char( "That item does not have enough energy to hold the enchantment.\n\r", ch );
         return;
      }
      act( AT_MAGIC, "You enchant $p to return when thrown.", ch, obj, NULL, TO_CHAR );
      xSET_BIT( obj->extra_flags, ITEM_RETURNING );
   }
   else if( !str_cmp( argument, "durable" ) )
   {
      tal = TAL_SECURITY;
      amt = 200;
      if( IS_OBJ_STAT( obj, ITEM_DURABLE ) )
      {
         send_to_char( "That item is already durable.\n\r", ch );
         return;
      }
      if( ch->curr_talent[TAL_SECURITY]  +ch->curr_talent[TAL_VOID] < 100 )
      {
         send_to_char( "You can't imbue something with durability.\n\r", ch );
         return;
      }
      if( obj->mana < amt )
      {
         send_to_char( "That item needs 200 energy to sustain this enchantment.\n\r", ch );
         return;
      }
      act( AT_MAGIC, "You imbue $p with durability.", ch, obj, NULL, TO_CHAR );
      xSET_BIT( obj->extra_flags, ITEM_DURABLE );
   }
   else if( !str_cmp( argument, "breath" )  &&ch->curr_talent[TAL_WATER] >= 40 )
   {
      tal = TAL_WATER;
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_AFFECT;
         SET_BIT( paf->modifier, 1 << AFF_AQUA_BREATH );
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You imbue $p with magical breathing.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_cmp( argument, "flight" )  &&ch->curr_talent[TAL_WIND] >= 60 )
   {
      tal = TAL_WIND;
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_AFFECT;
         SET_BIT( paf->modifier, 1 << AFF_FLYING );
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You imbue $p with the power of flight.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_cmp( argument, "nightvision" )  &&ch->curr_talent[TAL_SEEKING] >= 35 )
   {
      tal = TAL_SEEKING;
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_AFFECT;
         SET_BIT( paf->modifier, 1 << AFF_INFRARED );
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You imbue $p with nightvision.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_cmp( argument, "armor" )  &&obj->item_type == ITEM_ARMOR )
   {
      tal = TAL_SECURITY;
      if( TALENT( ch, tal ) / 10  >( obj->value[0] - obj->pIndexData->value[0] ) )
      {
         act( AT_MAGIC, "You enhance the protective ability of $p.", ch, obj, NULL, TO_CHAR );
         obj->value[0]++;
      }
      else
      {
         send_to_char( "You are not strong enough to further enchant that item.\n\r", ch );
         return;
      }
   }
   else if( !str_cmp( argument, "weapon" ) 
            &&( obj->item_type == ITEM_WEAPON 
                ||obj->item_type == ITEM_MISSILE_WEAPON )  &&ch->curr_talent[TAL_DEATH] >= 10 )
   {
      tal = TAL_DEATH;
      amt = URANGE( 10, 250 - TALENT( ch, TAL_DEATH ), 200 );
      amt += obj->value[2];
      if( obj->mana < amt )
      {
         send_to_char( "This item can't hold the enchantment.\n\r", ch );
         return;
      }
      obj->value[2] += 5;
      act( AT_MAGIC, "You imbue $p with the power of Death.", ch, obj, NULL, TO_CHAR );
   }
   else if( !str_cmp( argument, "weight" ) )
   {
      if( obj->weight > 1 && TALENT( ch, tal ) / 10  >( obj->pIndexData->weight - obj->weight ) )
      {
         act( AT_MAGIC, "You magically lighten $p.", ch, obj, NULL, TO_CHAR );
         obj->weight--;
      }
      else
      {
         send_to_char( "You can't lighten that item any further.\n\r", ch );
         return;
      }
   }
   else if( !str_prefix( argument, "constitution" ) )
   {
      tal = TAL_EARTH;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_CON && !found )
         {
            if( TALENT( ch, tal ) / 10 > paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the strength of the Earth.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_CON;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the strength of the Earth.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "endurance" ) )
   {
      tal = TAL_FROST;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_LCK && !found )
         {
            if( TALENT( ch, tal ) / 10  >paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the strength of the Ice.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            paf->modifier++;
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_LCK;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the strength of the Ice.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "strength" ) )
   {
      tal = TAL_FIRE;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_STR && !found )
         {
            if( TALENT( ch, tal ) / 10  >paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the strength of the Flames.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_STR;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the strength of the Flames.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "perception" ) )
   {
      tal = TAL_SEEKING;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_CHA && !found )
         {
            if( TALENT( ch, tal ) / 10 > paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the power of Seeking.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_CHA;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the power of Seeking.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "dexterity" ) )
   {
      tal = TAL_WIND;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_DEX && !found )
         {
            if( TALENT( ch, tal ) / 10  >paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the speed of the Wind.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_DEX;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the speed of the Wind.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "willpower" ) )
   {
      tal = TAL_WATER;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_WIS && !found )
         {
            if( TALENT( ch, tal ) / 10  >paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the power of Water.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_WIS;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the power of Water.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "intelligence" ) )
   {
      tal = TAL_MIND;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_INT && !found )
         {
            if( TALENT( ch, tal ) / 10  >paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the power of the Mind.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_INT;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the power of the Mind.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "damroll" ) )
   {
      tal = TAL_LIGHTNING;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_DAMROLL && !found )
         {
            if( TALENT( ch, tal ) / 10  >paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the strength of the Storm.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_DAMROLL;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the strength of the Storm.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_prefix( argument, "hitroll" ) )
   {
      tal = TAL_MOTION;
      for( paf = obj->first_affect; paf; paf = paf->next )
      {
         if( paf->location == APPLY_HITROLL && !found )
         {
            if( TALENT( ch, tal ) / 10  >paf->modifier )
            {
               act( AT_MAGIC, "You enchant $p with the speed of Motion.", ch, obj, NULL, TO_CHAR );
               paf->modifier++;
            }
            else
            {
               send_to_char( "You are not strong enough to further", ch );
               send_to_char( " enchant this item.\n\r", ch );
               return;
            }
            found = TRUE;
         }
      }
      if( found == FALSE )
      {
         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_HITROLL;
         paf->modifier = 1;
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_MAGIC, "You enchant $p with the speed of Motion.", ch, obj, NULL, TO_CHAR );
      }
   }
   else if( !str_cmp( argument, "container" )  &&( obj->item_type == ITEM_CONTAINER  &&ch->curr_talent[TAL_VOID] >= 10 ) )
   {
      tal = TAL_VOID;
      amt = URANGE( 10, 250 - TALENT( ch, TAL_VOID ), 200 );
      amt += obj->value[0] / 10;
      if( obj->mana < amt )
      {
         send_to_char( "This item can't hold the enchantment.\n\r", ch );
         return;
      }
      obj->value[0] += TALENT( ch, TAL_VOID ) / 10;
      act( AT_MAGIC, "You imbue $p with the power of the Void.", ch, obj, NULL, TO_CHAR );
   }
   else
   {
      send_to_char( "You can't add any such enchantment.\n\r", ch );
      return;
   }
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
   obj->mana -= amt;
   use_magic( ch, tal, amt * 5 );
   obj->cost += 50000;
}
void do_preserve( CHAR_DATA * ch, char *argument ) 
{
   OBJ_DATA * obj;
   int lvl;
   obj = get_obj_here( ch, argument );
   if( !obj )
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }
   lvl = TALENT( ch, TAL_EARTH ) + TALENT( ch, TAL_DEATH ) 
      +TALENT( ch, TAL_FROST ) + TALENT( ch, TAL_TIME )  +TALENT( ch, TAL_VOID ) + TALENT( ch, TAL_CATALYSM );
   if( lvl <= 0 )
   {
      huh( ch );
      return;
   }
   if( obj->timer <= 0  ||IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      act( AT_MAGIC, "$p is not deteriorating.", ch, obj, NULL, TO_CHAR );
      return;
   }
   if( lvl >= 100 )
      obj->timer = 0;
   
   else
      obj->timer = lvl;
   act( AT_MAGIC, "$p flickers for a moment.", ch, obj, NULL, TO_CHAR );
   WAIT_STATE( ch, PULSE_VIOLENCE );
   use_magic( ch, TAL_CATALYSM, 600 - lvl );
}
void do_retrieve( CHAR_DATA * ch, char *argument )
{
   char buf3[MAX_STRING_LENGTH];
   OBJ_DATA * obj;
   bool found;
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_DEATH] + ch->curr_talent[TAL_MOTION] < 95 )
      
   {
      huh( ch );
      return;
   }
   victim = get_char_world( ch, argument );
   if( !victim )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( victim != ch && !IS_CONSENTING( victim, ch ) )
   {
      send_to_char( "You can't retrieve their corpse without their consent.\n\r", ch );
      return;
   }
   found = FALSE;
   sprintf( buf3, " " );
   for( obj = first_object; obj; obj = obj->next )
      
   {
      if( obj->in_room  &&nifty_is_name( victim->name, obj->name )  &&( obj->pIndexData->vnum == 11 ) )
         
      {
         found = TRUE;
         if( obj->timer > 38 )
            
         {
            act( AT_MAGIC, "The image of your corpse appears, but suddenly wavers away.", victim, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "The image of $n's corpse appears, but suddenly wavers away.", victim, NULL, NULL, TO_ROOM );
            return;
         }
         use_magic( ch, TAL_DEATH, 1000 - ch->curr_talent[TAL_DEATH] * 9 );
         if( IS_SET( obj->in_room->room_flags, ROOM_NOSUPPLICATE ) 
               &&ch->curr_talent[TAL_DEATH] + ch->curr_talent[TAL_MOTION] < 200 )
         {
            act( AT_MAGIC, "Only silence answers from that forsaken place.", ch, NULL, NULL, TO_CHAR );
            return;
         }
         act( AT_MAGIC, "Your corpse appears suddenly, surrounded by a misty aura...", victim, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n's corpse appears suddenly, surrounded by a misty aura...", victim, NULL, NULL, TO_ROOM );
         obj_from_room( obj );
         obj = obj_to_room( obj, ch->in_room );
      }
   }
   if( !found )
      
   {
      send_to_char( "No corpse of that sort litters the world...\n\r", ch );
      return;
   }
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
}


/* Summer 1997 --Blod */ 
/* Modified for players Oct 12 2000 -keo */ 
/* Support for objects Nov 29 2000 -keo */ 
void do_scatter( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA * pRoomIndex;
   OBJ_DATA * obj;
   if( ch->curr_talent[TAL_MOTION] < 70 )
   {
      huh( ch );
      return;
   }
   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Scatter what?\n\r", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg ) ) == NULL  &&( obj = get_obj_carry( ch, arg ) ) == NULL )
   {
      send_to_char( "You see nothing like that here.\n\r", ch );
      return;
   }
   if( victim )
   {
      if( victim == ch )
      {
         send_to_char( "It's called travel.  Try it.\n\r", ch );
         return;
      }
      if( ch->curr_talent[TAL_MOTION] < 100  &&( IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL )  ||IS_NPC( victim ) ) )
      {
         send_to_char( "Your magic fails to take hold.\n\r", ch );
         return;
      }
   }
   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, 1048576000 ) );
      if( pRoomIndex )
         if( !IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) 
              &&!IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) 
              &&!IS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL )  &&!IS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
            break;
   }
   use_magic( ch, TAL_MOTION, 750 - ch->curr_talent[TAL_MOTION] * 4 );
   if( victim )
   {
      act( AT_MAGIC, "With the sweep of an arm, $n flings $N to the winds.", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "With the sweep of an arm, $n flings you to the winds.", ch, NULL, victim, TO_VICT );
      act( AT_MAGIC, "With the sweep of an arm, you fling $N to the winds.", ch, NULL, victim, TO_CHAR );
      char_from_room( victim );
      victim->in_obj = NULL;
      char_to_room( victim, pRoomIndex );
      victim->position = POS_RESTING;
      act( AT_MAGIC, "$n staggers forth from a sudden gust of wind, and collapses.", victim, NULL, NULL, TO_ROOM );
      do_look( victim, "auto" );
   }
   else if( obj )
   {
      act( AT_MAGIC, "You throw $p high into the air where it vanishes.", ch, obj, NULL, TO_CHAR );
      act( AT_MAGIC, "$n throws $p high into the air where it vanishes.", ch, obj, NULL, TO_ROOM );
      obj_from_char( obj );
      obj_to_room( obj, pRoomIndex );
   }
   WAIT_STATE( ch, PULSE_VIOLENCE );
}
void do_reshape( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA * obj;
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_STRING_LENGTH];
   int lvl;
   argument = one_argument( argument, arg );
   obj = get_obj_carry( ch, arg );
   if( !obj )
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }
   lvl = ch->curr_talent[TAL_CHANGE]  +ch->curr_talent[TAL_CATALYSM]  +ch->curr_talent[TAL_ILLUSION];
   if( lvl < 95 )
   {
      if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
      {
         if( ch->curr_talent[TAL_DREAM] < 40 )
         {
            huh( ch );
            return;
         }
      }
      else
      {
         if( !obj->material  ||obj->material->number == 116 )
         {  /* ether */
            if( ch->curr_talent[TAL_VOID] < 30 )
            {
               huh( ch );
               return;
            }
         }
         else
         {
            huh( ch );
            return;
         }
      }
   }
   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      send_to_char( "Your magic won't take hold.\n\r", ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Reshape it to what?\n\r", ch );
      return;
   }
   separate_obj( obj );  /* Thanks Zemus */
   act( AT_MAGIC, "Your magic twists around $p, making it look like $T!", ch, obj, argument, TO_CHAR );
   act( AT_MAGIC, "$n's magic twists around $p, and suddenly $e is holding $T!", ch, obj, argument, TO_ROOM );
   STRFREE( obj->name );
   STRFREE( obj->short_descr );
   STRFREE( obj->description );
   obj->name = STRALLOC( munch_colors( argument ) );
   obj->short_descr = STRALLOC( argument );
   strcpy( buf, argument );
   strcat( buf, " is here." );
   obj->description = STRALLOC( buf );
   use_magic( ch, TAL_CATALYSM, 1000 - lvl * 3 );
   WAIT_STATE( ch, PULSE_VIOLENCE );
}
void do_void( CHAR_DATA * ch, char *argument )
{
   int i, vnum;
   CHAR_DATA * victim;
   char arg[MAX_STRING_LENGTH];
   AREA_DATA * pArea;
   ROOM_INDEX_DATA * pRoomIndex;
   if( number_range( 1, 100 ) > TALENT( ch, TAL_VOID ) )
  {
      travel_teleport( ch );
      lose_hp( ch, number_range( 1, 100 ) );
      return;
   }
   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "enter" ) && ch->curr_talent[TAL_VOID] >= 1 )
   {
      if( IS_AFFECTED( ch, AFF_VOID ) )
      {
         send_to_char( "You are already in the Void!\n\r", ch );
         return;
      }
      if( IS_AFFECTED( ch, AFF_DREAMWORLD ) )
      {
         huh( ch );
         return;
      }
      act( AT_DGREY, "You enter the Void.", ch, NULL, NULL, TO_CHAR );
      act( AT_DGREY, "$n steps out of reality.", ch, NULL, NULL, TO_ROOM );
      xSET_BIT( ch->pcdata->perm_aff, AFF_VOID );
      act( AT_DGREY, "$n steps into the Void.", ch, NULL, NULL, TO_ROOM );
   }
   else if( !str_cmp( arg, "leave" ) && ch->curr_talent[TAL_VOID] >= 20 )
   {
      if( !IS_AFFECTED( ch, AFF_VOID ) )
      {
         send_to_char( "You aren't in the Void!\n\r", ch );
         return;
      }
      act( AT_DGREY, "You step out of the Void.", ch, NULL, NULL, TO_CHAR );
      act( AT_DGREY, "$n steps out of the Void.", ch, NULL, NULL, TO_ROOM );
      xREMOVE_BIT( ch->pcdata->perm_aff, AFF_VOID );
      act( AT_DGREY, "$n steps into reality.", ch, NULL, NULL, TO_ROOM );
   }
   else if( !str_cmp( arg, "pull" )  &&ch->curr_talent[TAL_VOID] >= 35 )
   {
      victim = get_char_room( ch, argument );
      if( !victim )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
      if( IS_AFFECTED( victim, AFF_DREAMWORLD ) )
      {
         send_to_char( "You are unable to touch them.\n\r", ch );
         return;
      }
      if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
      {
         send_to_char( "Stop that you twit, don't void pacifists.\n\r", ch );
         return;
      }
      act( AT_MAGIC, "You pull $N into the Void!", ch, NULL, victim, TO_CHAR );
      victim->position = POS_STANDING;
      if( IS_NPC( victim ) )
         xSET_BIT( victim->affected_by, AFF_VOID );
      
      else
         xSET_BIT( victim->pcdata->perm_aff, AFF_VOID );
      act( AT_MAGIC, "Reality seems to fade away into nothingness.", ch, NULL, victim, TO_VICT );
      do_look( victim, "auto" );
      return;
   }
   else if( !str_cmp( arg, "extract" )  &&ch->curr_talent[TAL_VOID] >= 45 )
   {
      victim = get_char_room( ch, argument );
      if( !victim )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
      act( AT_DGREY, "You extract $N from the Void!", ch, NULL, victim, TO_CHAR );
      victim->position = POS_STANDING;
      if( IS_NPC( victim ) )
         xREMOVE_BIT( victim->affected_by, AFF_VOID );
      
      else
         xREMOVE_BIT( victim->pcdata->perm_aff, AFF_VOID );
      act( AT_DGREY, "The Void vanishes and reality fades into view.", ch, NULL, victim, TO_VICT );
      do_look( victim, "auto" );
      return;
   }
   else if( !str_cmp( arg, "shift" )  &&ch->curr_talent[TAL_VOID] >= 50 )
   {
      if( !IS_AFFECTED( ch, AFF_VOID ) )
      {
         act( AT_MAGIC, "You need to be in the Void to do that that.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      victim = get_char_world( ch, argument );
      if( !victim )
      {
         
            /*
             * Start travel to location 
             */ 
            for( pArea = first_area; pArea; pArea = pArea->next )
            
         {
            if( ( arg[0] == '\0' || nifty_is_name_prefix( argument, pArea->name ) ) 
                 &&!IS_SET( pArea->flags, AFLAG_NOTRAVEL ) )
               
            {
               for( i = 0; i < 20; i++ )
               {
                  vnum = number_range( pArea->low_r_vnum, pArea->hi_r_vnum );
                  if( ( pRoomIndex = get_room_index( vnum ) ) == NULL )
                     continue;
                  break;
               }
               if( !pRoomIndex )
               {
                  send_to_char( "You can't find a place to land.\n\r", ch );
                  return;
               }
               act( AT_DGREY, "You shift into another part of the Void.", ch, NULL, NULL, TO_CHAR );
               act( AT_DGREY, "$n fades out of existence.", ch, NULL, NULL, TO_ROOM );
               char_from_room( ch );
               ch->in_obj = NULL;
               char_to_room( ch, pRoomIndex );
               act( AT_DGREY, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
               do_look( ch, "auto" );
            }
         }
         send_to_char( "You can't find any place like that.\n\r", ch );
         return;
      }
      if( victim == ch )
      {
         send_to_char( "That was a soft trip!\n\r", ch );
         return;
      }
      act( AT_DGREY, "You shift into another part of the Void.", ch, NULL, NULL, TO_CHAR );
      act( AT_DGREY, "$n fades out of existence.", ch, NULL, NULL, TO_ROOM );
      char_from_room( ch );
      ch->in_obj = NULL;
      char_to_room( ch, victim->in_room );
      act( AT_DGREY, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
      do_look( ch, "auto" );
   }
   else if( !str_cmp( arg, "rift" )  &&ch->curr_talent[TAL_VOID] >= 80 )
   {
      if( !IS_AFFECTED( ch, AFF_VOID ) )
      {
         send_to_char( "You need to be in the Void to do that.\n\r", ch );
         return;
      }
      victim = get_char_world( ch, argument );
      if( !victim  ||( IS_NPC( victim ) && !xIS_SET( victim->act, ACT_NAMED ) ) )
         victim = generate_mob_nation( argument );
      if( !victim )
      {
         send_to_char( "You cannot find anything like that.", ch );
         return;
      }
      if( victim == ch )
      {
         send_to_char( "That was a soft trip!\n\r", ch );
         return;
      }
      if( victim->in_room )
      {
         act( AT_DGREY, "$n tears a rift in reality and pulls you through!", ch, NULL, victim, TO_VICT );
         act( AT_DGREY, "$n is ripped through a rift in reality!", victim, NULL, NULL, TO_ROOM );
         char_from_room( victim );
      }
      victim->in_obj = NULL;
      char_to_room( victim, ch->in_room );
      if( IS_NPC( victim ) )
         xSET_BIT( victim->affected_by, AFF_VOID );
      
      else
         xSET_BIT( victim->pcdata->perm_aff, AFF_VOID );
      act( AT_DGREY, "$n is pulled into the Void!", victim, NULL, NULL, TO_ROOM );
      do_look( victim, "auto" );
   }
   else
   {
      send_to_char( "You can't do any such thing.\n\r", ch );
      return;
   }
   WAIT_STATE( ch, PULSE_VIOLENCE );
   use_magic( ch, TAL_VOID, 1000 - ch->curr_talent[TAL_VOID] * 9 );
}
void do_decay( CHAR_DATA * ch, char *argument ) 
{
   OBJ_DATA * obj;
   int lvl;
   obj = get_obj_carry( ch, argument );
   if( !obj )
      
   {
      send_to_char( "You don't have that.\n\r", ch );
      return;
   }
   lvl = ( ch->curr_talent[TAL_TIME] + ch->curr_talent[TAL_EARTH] 
             +ch->curr_talent[TAL_CATALYSM] + ch->curr_talent[TAL_DEATH]  +ch->curr_talent[TAL_VOID] ) / 2;
   if( lvl <= 0 )
      
   {
      huh( ch );
      return;
   }
   if( obj->timer <= 0 )
      
   {
      act( AT_BLOOD, "$p starts to decay slowly.", ch, obj, NULL, TO_CHAR );
      obj->timer = 100;
      return;
   }
   act( AT_BLOOD, "$p decays further.", ch, obj, NULL, TO_CHAR );
   if( lvl < 40 )
      obj->timer -= lvl;
   
   else
      obj->timer -= 40;
   if( obj->timer < 1 )
      obj->timer = 1;
   WAIT_STATE( ch, PULSE_VIOLENCE );
   use_magic( ch, TAL_DEATH, 600 - lvl );
   return;
}
void player_echo( CHAR_DATA * ch, sh_int AT_COLOR, char *argument, sh_int tar ) 
{
   DESCRIPTOR_DATA * d;
   if( !argument || argument[0] == '\0' )
      return;
   for( d = first_descriptor; d; d = d->next )
      
   {
      if( d->connected == CON_PLAYING )
         
      {
         if( tar == ECHOTAR_AREA  &&ch->in_room->area != d->character->in_room->area )
            continue;
         if( tar == ECHOTAR_ROOM  &&ch->in_room != d->character->in_room )
            continue;
         if( tar == ECHOTAR_WORLD  &&str_cmp( ch->in_room->area->resetmsg, d->character->in_room->area->resetmsg ) )
            continue;
         set_char_color( AT_COLOR, d->character );
         send_to_char( argument, d->character );
         send_to_char( "\n\r", d->character );
      }
   }
   return;
}
void do_player_echo( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   sh_int color;
   int target;
   char *parg;
   int lvl = ch->curr_talent[TAL_SPEECH];
   if( lvl < 30 && ch->curr_talent[TAL_ILLUSION] < 60 )
   {
      huh( ch );
      return;
   }
   set_char_color( AT_PLAIN, ch );
   if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You can't do that right now.\n\r", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Echo what?\n\r", ch );
      return;
   }
   if( ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );
   parg = argument;
   argument = one_argument( argument, arg );
   if( !str_cmp( arg, "room" ) )
      target = ECHOTAR_ROOM;
   
   else if( !str_cmp( arg, "area" ) && lvl >= 50 )
      target = ECHOTAR_AREA;
   
   else if( !str_cmp( arg, "world" ) && lvl >= 80 )
      target = ECHOTAR_WORLD;
   
   else
      
   {
      if( lvl >= 100 )
         target = ECHOTAR_ALL;
      
      else if( lvl >= 80 )
         target = ECHOTAR_WORLD;
      
      else if( lvl >= 50 )
         target = ECHOTAR_AREA;
      
      else
         target = ECHOTAR_ROOM;
      argument = parg;
   }
   if( !color && ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );
   if( !color )
      color = AT_PLAIN;
   one_argument( argument, arg );
   player_echo( ch, color, argument, target );
   mana_from_char( ch, 500 - ch->curr_talent[TAL_SPEECH] * 4 );
   WAIT_STATE( ch, PULSE_VIOLENCE );
   learn_talent( ch, TAL_SPEECH );
}
void do_detonate( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];
   OBJ_DATA * obj;
   argument = one_argument( argument, arg );
   obj = get_obj_carry( ch, arg );
   if( ch->curr_talent[TAL_FIRE] < 75 )
   {
      huh( ch );
      return;
   }
   if( !obj )
   {
      send_to_char( "You don't have one of those to add an explosive charge to.\r\n", ch );
      return;
   }
   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      send_to_char( "Your magic fails to take hold.\n\r", ch );
      return;
   }
   if( !IS_OBJ_STAT( obj, ITEM_EXPLOSIVE ) )
   {
      if( obj->weight < TALENT( ch, TAL_FIRE ) )
      {
         obj->timer = 1;
         xSET_BIT( obj->extra_flags, ITEM_EXPLOSIVE );
         act( AT_FIRE, "You add an explosive charge to $p.", ch, obj, NULL, TO_CHAR );
      }
      else
      {
         send_to_char( "You are not strong enough to detonate that.\n\r", ch );
         return;
      }
   }
   else
   {
      obj->timer = 0;
      xREMOVE_BIT( obj->extra_flags, ITEM_EXPLOSIVE );
      act( AT_FIRE, "You remove the explosive charge from $p.", ch, obj, NULL, TO_CHAR );
   }
   use_magic( ch, TAL_FIRE, 500 );
}
void do_immobilize( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   int lvl;
   if( IS_NPC( ch ) )
      return;
   lvl = ch->curr_talent[TAL_EARTH] + ch->curr_talent[TAL_TIME]  +ch->curr_talent[TAL_FROST];
   if( lvl < 20 )
   {
      huh( ch );
      return;
   }
   victim = find_target( ch, argument, TRUE );
   if( !victim )
      return;
   
      /*
       * if they're frost, freeze 
       */ 
      if( ch->pcdata->inborn == TAL_FROST )
   {
      if( ch->in_room->area->weather->temp > ch->curr_talent[TAL_FROST] * 10 - 400 )
      {
         act( AT_FROST, "It isn't cold enough here.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      if( ch->curr_talent[TAL_FROST] > number_range( 10, 50 ) )
      {
         act( AT_FROST, "Ice forms around you, holding you in place!", ch, NULL, victim, TO_VICT );
         act( AT_FROST, "Ice forms around $N!", ch, NULL, victim, TO_NOTVICT );
         act( AT_FROST, "You hold $N in ice!", ch, NULL, victim, TO_CHAR );
         if( IS_NPC( victim ) )
            xSET_BIT( victim->affected_by, AFF_HOLD );
         
         else
            xSET_BIT( victim->pcdata->perm_aff, AFF_HOLD );
      }
      else
      {
         act( AT_FROST, "Ice forms around you but fails to hold you!", ch, NULL, victim, TO_VICT );
         act( AT_FROST, "Your ice fails to hold $N!", ch, NULL, victim, TO_CHAR );
      }
      use_magic( ch, TAL_FROST, 100 );
      return;
   } /* end freeze */
   
      /*
       * if they're earth, entangle 
       */ 
      if( ch->pcdata->inborn == TAL_EARTH )
   {
      if( ch->in_room->curr_vegetation < 100 - ch->curr_talent[TAL_EARTH] )
      {
         act( AT_GREEN, "There isn't enough vegetation here.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      if( ch->curr_talent[TAL_EARTH] > number_range( 10, 50 ) )
      {
         act( AT_GREEN, "You are entangled in vines!", ch, NULL, victim, TO_VICT );
         act( AT_GREEN, "$N is entangled in vines!", ch, NULL, victim, TO_NOTVICT );
         act( AT_GREEN, "You entangle $N in vines!", ch, NULL, victim, TO_CHAR );
         if( IS_NPC( victim ) )
            xSET_BIT( victim->affected_by, AFF_HOLD );
         
         else
            xSET_BIT( victim->pcdata->perm_aff, AFF_HOLD );
      }
      else
      {
         act( AT_GREEN, "Vines grab for you but fail to hold you!", ch, NULL, victim, TO_VICT );
         act( AT_GREEN, "Your vines fail to hold $N!", ch, NULL, victim, TO_CHAR );
      }
      use_magic( ch, TAL_EARTH, 100 );
      return;
   } /* end entangle */
   
      /*
       * otherwise, a generic hold 
       */ 
      if( lvl > number_range( 20, 60 ) )
   {
      act( AT_MAGIC, "You are held in place!", ch, NULL, victim, TO_VICT );
      act( AT_MAGIC, "$N's feet are held to the floor!", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You hold $N in place!", ch, NULL, victim, TO_CHAR );
      if( IS_NPC( victim ) )
         xSET_BIT( victim->affected_by, AFF_HOLD );
      
      else
         xSET_BIT( victim->pcdata->perm_aff, AFF_HOLD );
   }
   else
   {
      act( AT_MAGIC, "Something grabs at you, but fails to hold you!", ch, NULL, victim, TO_VICT );
      act( AT_MAGIC, "Your magic fails to hold $N!", ch, NULL, victim, TO_CHAR );
   }
   use_magic( ch, TAL_TIME, 200 );
   return;
}
void do_ventriloquate( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char speaker[MAX_STRING_LENGTH];
   CHAR_DATA * vch;
   if( ch->curr_talent[TAL_SPEECH] < 10 )
   {
      huh( ch );
      return;
   }
   argument = one_argument( argument, speaker );
   sprintf( buf, "%s says '%s'\n\r", capitalize( speaker ), argument );
   act( AT_SAY, "You make the words '$t' echo in the room.", ch, argument, NULL, TO_CHAR );
   use_magic( ch, TAL_SPEECH, 2000 - TALENT( ch, TAL_SPEECH ) * 10 );
   for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
      
   {
      set_char_color( AT_SAY, vch );
      send_to_char( buf, vch );
   }
}


