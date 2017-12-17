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
 *			     Informational module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"

BOOK_DATA *first_book;
BOOK_DATA *last_book;

/* handler.c */
extern char *get_room_description( CHAR_DATA * ch, char *descr );

/* clan.c */
extern CLAN_MEMBER *get_member( char *name, CLAN_DATA * clan );

extern int top_help;
char *help_greeting;

/*
 * Local functions.
 */
void show_char_to_char_0 args( ( CHAR_DATA * victim, CHAR_DATA * ch ) );
void show_char_to_char_1 args( ( CHAR_DATA * victim, CHAR_DATA * ch ) );
void show_char_to_char args( ( CHAR_DATA * list, CHAR_DATA * ch ) );
bool check_blind args( ( CHAR_DATA * ch ) );
void show_condition args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
char *get_talent_rank args( ( CHAR_DATA * ch ) );

/* act_wiz.c */
int get_permit_flag args( ( char *argument ) );

char *format_obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort )
{
   static char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   buf[0] = '\0';

   if( !obj->short_descr )
   {
      sprintf( buf, "a bugged item, report to an imm" );
      return buf;
   }

   if( obj->condition < obj->weight * 30 && obj->item_type != ITEM_TREASURE )
      strcat( buf, "&Rdamaged&W, " );

   if( !IS_OBJ_STAT( obj, ITEM_NO_TAKE ) && !xIS_EMPTY( obj->parts ) )
   {
      if( obj->size > ch->height + ch->height * .50 )
         strcat( buf, "huge, " );
      else if( obj->size < ch->height - ch->height * .50 )
         strcat( buf, "tiny, " );
      else if( obj->size > ch->height + ch->height * .30 )
         strcat( buf, "large, " );
      else if( obj->size < ch->height - ch->height * .30 )
         strcat( buf, "small, " );
   }

   if( obj->raw_mana > 100 && IS_AFFECTED( ch, AFF_DETECT_MAGIC ) )
      strcat( buf, "&pmagically charged&w, " );

   if( IS_OBJ_STAT( obj, ITEM_INVIS ) )
      strcat( buf, "invisible, " );
   if( ( IS_AFFECTED( ch, AFF_DETECT_EVIL ) ) && IS_OBJ_STAT( obj, ITEM_EVIL ) )
      strcat( buf, "&Revil&W, " );
   if( ( IS_AFFECTED( ch, AFF_DETECT_EVIL ) ) && IS_OBJ_STAT( obj, ITEM_BLESS ) )
      strcat( buf, "&Bblessed&W, " );

   if( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) && IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      strcat( buf, "&Bmagical&w, " );
   if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
      strcat( buf, "&Yglowing&w, " );
   if( IS_OBJ_STAT( obj, ITEM_DARK ) )
      strcat( buf, "&zshadowed&w, " );
   if( IS_OBJ_STAT( obj, ITEM_HUM ) )
      strcat( buf, "&Chumming&w, " );
   if( IS_OBJ_STAT( obj, ITEM_HIDDEN ) )
      strcat( buf, "&ghidden&w, " );
   if( IS_OBJ_STAT( obj, ITEM_BURIED ) )
      strcat( buf, "&Oburied&w, " );
   if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
      strcat( buf, "&pimaginary&w, " );
   if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      strcat( buf, "&WPROTOTYPE&w, " );
   if( IS_AFFECTED( ch, AFF_DETECTTRAPS ) && is_trapped( obj ) )
      strcat( buf, "&RTRAPPED&w " );

   if( obj->gem )
   {
      if( obj->gem->name )
      {
         one_argument( obj->gem->name, buf2 );
      }
      else
      {
         sprintf( buf2, "gem-studded" );
      }
      sprintf( buf2, "%s-studded ", buf2 );
      strcat( buf, buf2 );
   }

   if( fShort )
   {
      if( obj->short_descr )
         strcat( buf, myobj( obj ) );
      sprintf( buf, aoran( buf ) );
   }
   else
   {
      if( obj->short_descr )
         strcat( buf, myobj( obj ) );
      if( ch->in_obj && ch->in_obj != obj->in_obj )
      {
         if( obj != ch->in_obj )
         {
            strcat( buf, " is outside the " );
            strcat( buf, myobj( ch->in_obj ) );
            strcat( buf, "." );
         }
         else
         {
            strcat( buf, " is carrying you." );
         }
      }
      else
         strcat( buf, " is here." );
      sprintf( buf, capitalize( aoran( buf ) ) );
   }

   return buf;
}


/*
 * Some increasingly freaky hallucinated objects		-Thoric
 * (Hats off to Albert Hoffman's "problem child")
 */
char *hallucinated_object( int ms, bool fShort )
{
   int sms = URANGE( 1, ( ms + 10 ) / 5, 20 );

   if( fShort )
      switch ( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
      {
         case 1:
            return "a sword";
         case 2:
            return "a stick";
         case 3:
            return "something shiny";
         case 4:
            return "something";
         case 5:
            return "something interesting";
         case 6:
            return "something colorful";
         case 7:
            return "something that looks cool";
         case 8:
            return "a nifty thing";
         case 9:
            return "a cloak of flowing colors";
         case 10:
            return "a mystical flaming sword";
         case 11:
            return "a swarm of insects";
         case 12:
            return "&zSto&prmbri&znger&w";
         case 13:
            return "a figment of your imagination";
         case 14:
            return "your gravestone";
         case 15:
            return "the &YApple of D&Oi&Ys&Wc&Yo&Or&Yd&w";
         case 16:
            return "a glowing tome of arcane knowledge";
         case 17:
            return "a long sought secret";
         case 18:
            return "the meaning of it all";
         case 19:
            return "the answer";
         case 20:
            return "the key to life, the universe and everything";
      }
   switch ( number_range( 6 - URANGE( 1, sms / 2, 5 ), sms ) )
   {
      case 1:
         return "A nice looking sword catches your eye.";
      case 2:
         return "The ground is covered in small sticks.";
      case 3:
         return "Something shiny catches your eye.";
      case 4:
         return "Something catches your attention.";
      case 5:
         return "Something interesting catches your eye.";
      case 6:
         return "Something colorful flows by.";
      case 7:
         return "Something that looks cool calls out to you.";
      case 8:
         return "A nifty thing of great importance stands here.";
      case 9:
         return "A cloak of flowing colors asks you to wear it.";
      case 10:
         return "A mystical flaming sword awaits your grasp.";
      case 11:
         return "A swarm of insects buzzes in your face!";
      case 12:
         return "A black runesword moans with power and chaotic energy.";
      case 13:
         return "A figment of your imagination is at your command.";
      case 14:
         return "You notice a gravestone here... upon closer examination, it reads your name.";
      case 15:
         return "The most beautiful sparkling apple hangs in the air.";
      case 16:
         return "A glowing tome of arcane knowledge hovers in the air before you.";
      case 17:
         return "A long sought secret of all mankind is now clear to you.";
      case 18:
         return "The meaning of it all, so simple, so clear... of course!";
      case 19:
         return "The answer.  One.  It's always been One.";
      case 20:
         return "The key to life, the universe and everything awaits your hand.";
   }
   return "Whoa!!!";
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing )
{
   char **prgpstrShow;
   int *prgnShow;
   int *pitShow;
   char *pstrShow;
   OBJ_DATA *obj;
   int nShow;
   int iShow;
   int count, offcount, tmp, ms, cnt;
   bool fCombine;

   if( !ch->desc )
      return;

   /*
    * if there's no list... then don't do all this crap!  -Thoric
    */
   if( !list )
   {
      if( fShowNothing )
      {
         if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
            send_to_char( "     ", ch );
         set_char_color( AT_OBJECT, ch );
         send_to_char( "Nothing.\n\r", ch );
      }
      return;
   }
   /*
    * Alloc space for output lines.
    */
   count = 0;
   for( obj = list; obj; obj = obj->next_content )
      count++;

   ms = ( ch->mental_state ? ch->mental_state : 1 )
      * ( IS_NPC( ch ) ? 1 : ( ch->pcdata->condition[COND_DRUNK] ? ( ch->pcdata->condition[COND_DRUNK] / 12 ) : 1 ) );

   /*
    * If not mentally stable...
    */
   if( abs( ms ) > 40 )
   {
      offcount = URANGE( -( count ), ( count * ms ) / 100, count * 2 );
      if( offcount < 0 )
         offcount += number_range( 0, abs( offcount ) );
      else if( offcount > 0 )
         offcount -= number_range( 0, offcount );
   }
   else
      offcount = 0;

   if( count + offcount <= 0 )
   {
      if( fShowNothing )
      {
         if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
            send_to_char( "     ", ch );
         set_char_color( AT_OBJECT, ch );
         send_to_char( "Nothing.\n\r", ch );
      }
      return;
   }

   CREATE( prgpstrShow, char *, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   CREATE( prgnShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   CREATE( pitShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
   nShow = 0;
   tmp = ( offcount > 0 ) ? offcount : 0;
   cnt = 0;

   /*
    * Format the list of objects.
    */
   for( obj = list; obj; obj = obj->next_content )
   {
      if( offcount < 0 && ++cnt > ( count + offcount ) )
         break;
      if( tmp > 0 && number_bits( 1 ) == 0 )
      {
         prgpstrShow[nShow] = str_dup( hallucinated_object( ms, fShort ) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
         nShow++;
         --tmp;
      }
      if( obj->wear_loc == WEAR_NONE
          && can_see_obj( ch, obj )
          && ( !obj->carried_by
               || ( obj != obj->carried_by->main_hand
                    && obj != obj->carried_by->off_hand ) )
          && ( obj->item_type != ITEM_TRAP || IS_AFFECTED( ch, AFF_DETECTTRAPS ) ) )
      {
         pstrShow = format_obj_to_char( obj, ch, fShort );
         fCombine = FALSE;

         if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
         {
            /*
             * Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwords.
             */
            for( iShow = nShow - 1; iShow >= 0; iShow-- )
            {
               if( !strcmp( prgpstrShow[iShow], pstrShow ) )
               {
                  prgnShow[iShow] += obj->count;
                  fCombine = TRUE;
                  break;
               }
            }
         }

         pitShow[nShow] = obj->item_type;
         /*
          * Couldn't combine, or didn't want to.
          */
         if( !fCombine )
         {
            prgpstrShow[nShow] = str_dup( pstrShow );
            prgnShow[nShow] = obj->count;
            nShow++;
         }
      }
   }
   if( tmp > 0 )
   {
      int x;
      for( x = 0; x < tmp; x++ )
      {
         prgpstrShow[nShow] = str_dup( hallucinated_object( ms, fShort ) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
         nShow++;
      }
   }

   /*
    * Output the formatted list.      -Color support by Thoric
    */
   for( iShow = 0; iShow < nShow; iShow++ )
   {
      switch ( pitShow[iShow] )
      {
         default:
            set_char_color( AT_PLAIN, ch );
            break;
         case ITEM_BLOOD:
            set_char_color( AT_BLOOD, ch );
            break;
         case ITEM_MONEY:
         case ITEM_TREASURE:
            set_char_color( AT_YELLOW, ch );
            break;
         case ITEM_COOK:
         case ITEM_FOOD:
            set_char_color( AT_HUNGRY, ch );
            break;
         case ITEM_DRINK_CON:
         case ITEM_FOUNTAIN:
            set_char_color( AT_THIRSTY, ch );
            break;
         case ITEM_FIRE:
            set_char_color( AT_FIRE, ch );
            break;
         case ITEM_SCROLL:
         case ITEM_WAND:
         case ITEM_STAFF:
            set_char_color( AT_MAGIC, ch );
            break;
      }
      if( fShowNothing )
         send_to_char( "     ", ch );
      send_to_char( prgpstrShow[iShow], ch );
/*	if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) ) */
      {
         if( prgnShow[iShow] != 1 )
            ch_printf( ch, " (x %d)", prgnShow[iShow] );
      }

      send_to_char( "\n\r", ch );
      DISPOSE( prgpstrShow[iShow] );
   }

   if( fShowNothing && nShow == 0 )
   {
      if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
         send_to_char( "     ", ch );
      set_char_color( AT_OBJECT, ch );
      send_to_char( "Nothing.\n\r", ch );
   }

   /*
    * Clean up.
    */
   DISPOSE( prgpstrShow );
   DISPOSE( prgnShow );
   DISPOSE( pitShow );
   return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char( CHAR_DATA * victim, CHAR_DATA * ch )
{
   char name[MAX_STRING_LENGTH];

   if( IS_NPC( victim ) )
      strcpy( name, victim->short_descr );
   else
      strcpy( name, victim->name );
   name[0] = toupper( name[0] );

   if( !IS_NPC( victim ) && ch->curr_talent[TAL_SEEKING] >= 80 && victim->pcdata->name_disguise )
      ch_printf( ch, "&W...shimmering with the image of %s.\n\r", victim->pcdata->name_disguise );

   if( victim->shield )
      ch_printf( ch, "&W...surrounded by a shield of %s.\n\r", magic_table[victim->shield] );

   if( IS_AFFECTED( victim, AFF_BEAUTY ) )
      send_to_char( "...surrounded by an eerie aura of beauty and attraction.\n\r", ch );

   if( IS_AFFECTED( victim, AFF_FEAR ) )
      send_to_char( "...surrounded by an aura of fear and loathing.\n\r", ch );

   if( IS_AFFECTED( victim, AFF_UNHOLY ) )
   {
      set_char_color( AT_DGREY, ch );
      switch ( number_range( 1, 5 ) )
      {
         default:
            send_to_char( "...emanating an unholy aura", ch );
            break;
         case 1:
            send_to_char( "...surrounded by the stench of death", ch );
            break;
         case 2:
            send_to_char( "...feeling of dread and horror", ch );
            break;
         case 3:
            send_to_char( "...shrouded in terrible shadow", ch );
            break;
      }
      send_to_char( ".\n\r", ch );
   }
   else if( IS_AFFECTED( victim, AFF_HOLY ) )
   {
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "...glowing with an aura of divine radiance.\n\r" );
   }
   if( IS_AFFECTED( victim, AFF_HOLD ) )
   {
      set_char_color( AT_CYAN, ch );
      ch_printf( ch, "...held firmly in place.\n\r" );
   }
}

void show_char_to_char_0( CHAR_DATA * victim, CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   buf[0] = '\0';

   if( IS_NPC( victim ) )
   {
      if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
         if( IS_NPC( victim ) )  /* ha tagith - i didnt forget this time */
            ch_printf( ch, "[%d] ", victim->pIndexData->vnum );
      if( !xIS_SET( victim->act, ACT_NAMED ) )
         send_to_char( "A ", ch );
   }
   else
      send_to_char( "The ", ch );

   set_char_color( AT_PERSON, ch );
   if( !IS_NPC( victim ) && !victim->desc )
   {
      if( !victim->switched )
         send_to_char_color( "&PLINK-DEAD&W, ", ch );
      else if( !IS_AFFECTED( victim, AFF_POSSESS ) )
         strcat( buf, "switched, " );
   }

   if( ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_WIZINVIS ) )
       || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_MOBINVIS ) ) )
   {
      sprintf( buf, "&W&Yinvisible&W, " );
   }

   set_char_color( AT_PERSON, ch );
   if( IS_AFFECTED( victim, AFF_GLOW ) )
      strcat( buf, "&W&Yglowing&W, " );
   if( IS_AFFECTED( victim, AFF_DARK ) )
      strcat( buf, "&W&zshadowed&W, " );
   if( IS_AFFECTED( victim, AFF_FLAMING ) )
      strcat( buf, "&W&Rflaming&W, " );
   if( IS_AFFECTED( victim, AFF_INVISIBLE ) )
      strcat( buf, "&W&Yinvisible&W, " );
   if( IS_AFFECTED( victim, AFF_HIDE ) )
      strcat( buf, "&W&ghidden,&W " );
   if( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
      strcat( buf, "&W&btranslucent&W, " );
   if( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
      strcat( buf, "&W&Pfaerie fired&W, " );

   if( IS_AFFECTED( victim, AFF_BERSERK ) )
      strcat( buf, "&W&Rwild-eyed&W, " );

   if( IS_AFFECTED( ch, AFF_DREAMWORLD ) && !IS_AFFECTED( victim, AFF_DREAMWORLD ) )
   {
      if( !IS_AWAKE( victim ) )
      {
         strcat( buf, "&W&Bsleeping&W, " );
      }
      else
         strcat( buf, "&W&Cwaking&W, " );
   }

   if( IS_AFFECTED( victim, AFF_DREAMWORLD ) )
      strcat( buf, "&W&pdreaming&W, " );

   if( victim->desc && victim->desc->connected == CON_EDITING )
      strcat( buf, "&W&Cwriting&W " );

   if( !IS_NPC( victim ) )
   {
      if( victim->species )
      {
         strcat( buf, victim->species );
      }
   }
   else
   {
      strcat( buf, victim->short_descr );
   }

   if( IS_AFFECTED( ch, AFF_VOID ) && !IS_AFFECTED( victim, AFF_VOID ) && !IS_AFFECTED( victim, AFF_DREAMWORLD ) )
      strcat( buf, "&W&C, in the real world&W" );

   if( IS_AFFECTED( victim, AFF_VOID ) )
      strcat( buf, "&b&z, in the Void&W" );

   if( xIS_SET( victim->act, ACT_PROTOTYPE ) )
      strcat( buf, "&W&Y PROTOTYPE&W" );
   if( ch->mount && ch->mount == victim && ch->in_room == ch->mount->in_room )
      strcat( buf, "&W, which you are riding&W" );

   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) )
      strcat( buf, "&G, who is idle&W" );

   if( !IS_NPC( victim ) )
   {
      strcat( buf, ", " );
      strcat( buf, PERS( victim, ch ) );
   }

   switch ( victim->position )
   {
      case POS_DEAD:
         strcat( buf, " is &RDEAD&W!!" );
         break;
      case POS_MORTAL:
         strcat( buf, " is &rmortally wounded&W." );
         break;
      case POS_INCAP:
         strcat( buf, " is &rincapacitated&W." );
         break;
      case POS_STUNNED:
         strcat( buf, " is lying here &Runconscious&W." );
         break;
      case POS_SLEEPING:
         if( ch->position == POS_SITTING || ch->position == POS_RESTING )
            strcat( buf, " is sleeping nearby." );
         else
            strcat( buf, " is deep in slumber here." );
         break;
      case POS_RESTING:
         if( ch->position == POS_RESTING )
            strcat( buf, " is sprawled out alongside you." );
         else if( ch->position == POS_MOUNTED )
            strcat( buf, " is sprawled out at the foot of your mount." );
         else
            strcat( buf, " is sprawled out here." );
         break;
      case POS_SITTING:
         if( ( ch->position == POS_SITTING ) || ( ch->position == POS_KNEELING ) )
            strcat( buf, " sits here with you." );
         else if( ch->position == POS_RESTING )
            strcat( buf, " sits nearby as you lie around." );
         else
            strcat( buf, " sits upright here." );
         break;
      case POS_KNEELING:
         if( ( ch->position == POS_KNEELING ) || ( ch->position == POS_SITTING ) )
            strcat( buf, " is kneeling beside you." );
         else
            strcat( buf, " is kneeling here." );
         break;
      case POS_SQUATTING:
         strcat( buf, " is squatted down here." );
         break;
      case POS_SWIMMING:
         strcat( buf, " is swimming here." );
         break;
      case POS_FLYING:
         strcat( buf, " is flying here." );
         break;
      case POS_STANDING:
         if( victim->in_obj )
         {
            strcat( buf, " is in " );
            strcat( buf, aoran( myobj( victim->in_obj ) ) );
         }
         else if( ( victim->in_room->sector_type == SECT_UNDERWATER )
                  && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
            strcat( buf, " is &Rdrowning here&W" );
         else if( IS_UNDERWATER( victim ) )
            strcat( buf, " is here in the water" );
         else if( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
                  && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
            strcat( buf, " is&R drowning here&W" );
         else if( victim->wait && victim->last_taken )
         {
            strcat( buf, " is " );
            strcat( buf, victim->last_taken );
            strcat( buf, " here" );
         }
         else if( IS_AFFECTED( victim, AFF_FLYING ) )
            strcat( buf, " is hovering here" );
         else
            strcat( buf, " is standing here" );

         if( IS_SET( victim->mood, MOOD_READY ) )
            strcat( buf, " ready to fight!" );
         else
            strcat( buf, "." );
         break;
      case POS_SHOVE:
         strcat( buf, " is being shoved around." );
         break;
      case POS_DRAG:
         strcat( buf, " is being dragged around." );
         break;
      case POS_MOUNTED:
         strcat( buf, " is here, upon " );
         if( !victim->mount )
            strcat( buf, "thin air???" );
         else if( victim->mount == ch )
            strcat( buf, "your back." );
         else if( victim->in_room == victim->mount->in_room )
         {
            strcat( buf, PERS( victim->mount, ch ) );
            strcat( buf, "." );
         }
         else
            strcat( buf, "someone who left??" );
         break;
   }
   if( victim->fur_pos > FURNITURE_NONE )
   {
      if( victim->on )
         sprintf( buf2, " %s %s %s %s.",
                  PERS( victim, ch ), pos_string( victim ), fur_pos_string( victim->fur_pos ), victim->on->short_descr );
      strcat( buf, buf2 );
   }

   strcat( buf, "\n\r" );
   send_to_char( buf, ch );
   if( !xIS_SET( ch->act, PLR_BRIEF ) )
      show_visible_affects_to_char( victim, ch );
   return;
}

void show_char_to_char_2( CHAR_DATA * ch, CHAR_DATA * victim )
{
   show_race_line( ch, victim );
   if( victim->description != NULL )
   {
      send_to_char( victim->description, ch );
   }
   else
   {
      if( IS_NPC( victim ) )
         act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
      else if( ch != victim )
         act( AT_PLAIN, "$E isn't much to look at...", ch, NULL, victim, TO_CHAR );
      else
         act( AT_PLAIN, "You're not much to look at...", ch, NULL, NULL, TO_CHAR );
   }

   /*
    * Format a coloring description of the character. 
    */
   if( !IS_NPC( victim ) )
   {
      ch_printf( ch, "%s with", capitalize( aoran( victim->nation->name ) ) );
      if( victim->pcdata->eyes )
         ch_printf( ch, " %s eyes", victim->pcdata->eyes );
      if( victim->pcdata->hair )
         ch_printf( ch, ", %s hair", victim->pcdata->hair );
      if( victim->pcdata->skin_color )
      {
         ch_printf( ch, ", %s", victim->pcdata->skin_color );
         if( victim->pcdata->skin_type )
            ch_printf( ch, " %s", victim->pcdata->skin_type );
         else
            send_to_char( " skin", ch );
      }
      if( victim->pcdata->extra_color )
      {
         ch_printf( ch, ", and %s", victim->pcdata->extra_color );
         if( victim->pcdata->extra_type )
            ch_printf( ch, " %s", victim->pcdata->extra_type );
         else
            send_to_char( " spots", ch );
      }
      send_to_char( ".\n\r", ch );
   }
}



void show_char_to_char_1( CHAR_DATA * victim, CHAR_DATA * ch )
{
   char buf2[MAX_STRING_LENGTH];

   if( can_see( victim, ch )
       && !IS_NPC( ch )
       && !xIS_SET( ch->act, PLR_WIZINVIS )
       && ( ch->curr_talent[TAL_SEEKING] < 70 || victim->curr_talent[TAL_SEEKING] > 70 ) )
   {
      act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT );
      if( victim != ch )
         act( AT_ACTION, "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
      else
         act( AT_ACTION, "$n looks at $mself.", ch, NULL, victim, TO_NOTVICT );
   }

   show_char_to_char_2( ch, victim );

   if( victim->fur_pos > FURNITURE_NONE )
   {
      if( victim->on )
         sprintf( buf2, "%s %s %s %s.\n",
                  PERS( victim, ch ), pos_string( victim ), fur_pos_string( victim->fur_pos ), victim->on->short_descr );
      send_to_char( buf2, ch );
   }
   show_condition( ch, victim );

   if( victim != ch )
      act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
   else
      act( AT_PLAIN, "You are using:", ch, NULL, NULL, TO_CHAR );
   show_equip( victim, ch );

   /*
    * Crash fix here by Thoric
    */
   if( IS_NPC( ch ) || victim == ch )
      return;

   if( number_percent(  ) < ch->curr_talent[TAL_SEEKING] )
   {
      ch_printf( ch, "\n\rYou peek at %s inventory:\n\r", victim->sex == 1 ? "his" : victim->sex == 2 ? "her" : "its" );
      show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
      use_magic( ch, TAL_SEEKING, 1 );
   }
   return;
}


void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch )
{
   CHAR_DATA *rch;

   for( rch = list; rch; rch = rch->next_in_room )
   {
      if( rch == ch )
         continue;

      if( can_see( ch, rch ) != FALSE )
      {
         show_char_to_char_0( rch, ch );
      }
   }

   return;
}



bool check_blind( CHAR_DATA * ch )
{
   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;

   if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return TRUE;

   if( IS_AFFECTED( ch, AFF_BLIND ) || ( !can_use_bodypart( ch, BP_REYE ) && !can_use_bodypart( ch, BP_LEYE ) ) )
   {
      send_to_char( "You can't see a thing!\n\r", ch );
      return FALSE;
   }

   return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( char *arg )
{
   int door;

   if( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) )
      door = 0;
   else if( !str_cmp( arg, "e" ) || !str_cmp( arg, "east" ) )
      door = 1;
   else if( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) )
      door = 2;
   else if( !str_cmp( arg, "w" ) || !str_cmp( arg, "west" ) )
      door = 3;
   else if( !str_cmp( arg, "u" ) || !str_cmp( arg, "up" ) )
      door = 4;
   else if( !str_cmp( arg, "d" ) || !str_cmp( arg, "down" ) )
      door = 5;
   else if( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) )
      door = 6;
   else if( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) )
      door = 7;
   else if( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) )
      door = 8;
   else if( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) )
      door = 9;
   else
      door = -1;
   return door;
}

void do_look( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char roomdesc[MAX_STRING_LENGTH];   /* Generated Descs -- Scion */
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *original;
   char *pdesc;
   bool doexaprog;
   sh_int door;
   int number, cnt;

   if( !ch->desc )
      return;

   if( ch->position < POS_SLEEPING )
   {
      send_to_char( "You can't see anything but stars!\n\r", ch );
      return;
   }

   if( ch->position == POS_SLEEPING )
   {
      send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
      return;
   }

   if( !check_blind( ch ) )
      return;

   if( !IS_NPC( ch )
       && !xIS_SET( ch->act, PLR_HOLYLIGHT )
       && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && !IS_AFFECTED( ch, AFF_INFRARED ) && room_is_dark( ch->in_room ) )
   {
      set_char_color( AT_DGREY, ch );
      send_to_char( "It is pitch black ... \n\r", ch );
      show_char_to_char( ch->in_room->first_person, ch );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   doexaprog = str_cmp( "noprog", arg2 ) && str_cmp( "noprog", arg3 );

   if( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
   {
      /*
       * 'look' or 'look auto' 
       */
      set_char_color( AT_WHITE, ch );
/* make life easier - shogar */

      if( !IS_NPC( ch ) && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
         ch_printf( ch, "[%d] ", ch->in_room->vnum );
      /*
       * let's show your position here -keo 
       */
      if( ch->in_obj )
      {
         send_to_char( "Riding", ch );
      }
      else if( IS_UNDERWATER( ch ) )
      {
         send_to_char( "Swimming", ch );
      }
      else if( IS_AFFECTED( ch, AFF_FLYING ) )
      {
         send_to_char( "Flying", ch );
      }
      else if( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) )
      {
         send_to_char( "Falling", ch );
      }
      else
      {
         switch ( ch->position )
         {
            case POS_RESTING:
               send_to_char( "Resting", ch );
               break;
            case POS_SITTING:
               send_to_char( "Sitting", ch );
               break;
            case POS_KNEELING:
               send_to_char( "Kneeling", ch );
               break;
            case POS_SQUATTING:
               send_to_char( "Squatting", ch );
               break;
            case POS_STANDING:
               send_to_char( "Standing", ch );
               break;
            case POS_MOUNTED:
               send_to_char( "Riding", ch );
               break;
            default:
               send_to_char( "Walking", ch );
         }
      }
      send_to_char( " in ", ch );
      if( IS_AFFECTED( ch, AFF_VOID ) )
         send_to_char( "the Void", ch );
      else
         send_to_char( ch->in_room->name, ch );
      if( IS_AFFECTED( ch, AFF_DREAMWORLD ) )
         send_to_char( " &P[&pDreamworld&P]&W", ch );
      send_to_char( "\n\r", ch );

      /*
       * Room Desc Colors, to make things more interesting -keo 
       */
      if( !IS_OUTSIDE( ch ) )
      {
         if( ch->in_room->sector_type == SECT_LAVA )
            set_char_color( AT_RED, ch );
         else if( ch->in_room->sector_type == SECT_DUNNO )
            set_char_color( AT_DGREY, ch );
         else if( !IS_UNDERWATER( ch ) )
            set_char_color( AT_GREY, ch );
         else
            set_char_color( AT_BLUE, ch );
      }
      else if( time_info.hour > 6 && time_info.hour <= 18 && !IS_SET( sysdata.quest, QUEST_ETERNAL_NIGHT ) )
      {
         /*
          * Day Colors 
          */
         switch ( ch->in_room->sector_type )
         {
            default:
               set_char_color( AT_YELLOW, ch );
               break;
            case SECT_FOREST:
            case SECT_SWAMP:
               set_char_color( AT_GREEN, ch );
               break;
            case SECT_MOUNTAIN:
            case SECT_ICE:
               set_char_color( AT_WHITE, ch );
               break;
            case SECT_WATER_SWIM:
            case SECT_WATER_NOSWIM:
               set_char_color( AT_BLUE, ch );
               break;
            case SECT_AIR:
               set_char_color( AT_LBLUE, ch );
               break;
            case SECT_DUNNO:
               set_char_color( AT_PURPLE, ch );
               break;
         }
      }
      else
      {
         /*
          * Night colors 
          */
         switch ( ch->in_room->sector_type )
         {
            default:
               set_char_color( AT_BLUE, ch );
               break;
            case SECT_FOREST:
            case SECT_SWAMP:
               set_char_color( AT_DGREEN, ch );
               break;
            case SECT_MOUNTAIN:
               set_char_color( AT_DGREY, ch );
               break;
            case SECT_WATER_SWIM:
            case SECT_WATER_NOSWIM:
               set_char_color( AT_DBLUE, ch );
               break;
            case SECT_AIR:
               set_char_color( AT_CYAN, ch );
               break;
            case SECT_DUNNO:
               set_char_color( AT_DGREY, ch );
               break;
         }
      }

      /*
       * Generated Descriptions -- Scion 
       */
      if( ( !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_BRIEF ) ) || IS_NPC( ch ) )
      {
         strcpy( roomdesc, get_room_description( ch, roomdesc ) );
         send_to_char( roomdesc, ch );
      }

      if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_LONG_EXIT ) )
         do_exits( ch, "" );
      else if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOEXIT ) )
         do_exits( ch, "auto" );

      if( !ch->in_room->pShop )
         show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
      if( ch->in_obj )
         show_list_to_char( ch->in_obj->first_content, ch, FALSE, FALSE );
      show_char_to_char( ch->in_room->first_person, ch );
      return;
   }

   if( !str_cmp( arg1, "under" ) )
   {
      int count;

      /*
       * 'look under' 
       */
      if( arg2[0] == '\0' )
      {
         send_to_char( "Look beneath what?\n\r", ch );
         return;
      }

      if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not see that here.\n\r", ch );
         return;
      }
      if( ch->carry_weight + obj->weight > can_carry_w( ch ) )
      {
         send_to_char( "It's too heavy for you to look under.\n\r", ch );
         return;
      }
      count = obj->count;
      obj->count = 1;
      act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
      act( AT_PLAIN, "$n lifts $p and looks beneath it.", ch, obj, NULL, TO_ROOM );
      obj->count = count;
      if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
         show_list_to_char( obj->first_content, ch, TRUE, TRUE );
      else
         send_to_char( "Nothing.\n\r", ch );
      if( doexaprog )
         oprog_examine_trigger( ch, obj );
      return;
   }

   if( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
   {
      int count;

      /*
       * 'look in' 
       */
      if( arg2[0] == '\0' )
      {
         send_to_char( "Look in what?\n\r", ch );
         return;
      }

      if( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not see that here.\n\r", ch );
         return;
      }

      switch ( obj->item_type )
      {
         default:
            send_to_char( "That is not a container.\n\r", ch );
            break;

         case ITEM_DRINK_CON:
            if( obj->value[1] <= 0 )
            {
               send_to_char( "It is empty.\n\r", ch );
               if( doexaprog )
                  oprog_examine_trigger( ch, obj );
               break;
            }

            ch_printf( ch, "It's %s full of a %s liquid.\n\r",
                       obj->value[1] < obj->value[0] / 4
                       ? "less than" :
                       obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq_table[obj->value[2]].liq_color );

            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            break;

         case ITEM_PORTAL:
            for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
            {
               if( pexit->vdir == DIR_PORTAL && IS_SET( pexit->exit_info, EX_PORTAL ) )
               {
                  if( room_is_private( pexit->to_room ) )
                  {
                     set_char_color( AT_WHITE, ch );
                     send_to_char( "That room is private buster!\n\r", ch );
                     return;
                  }
                  original = ch->in_room;
                  char_from_room( ch );
                  char_to_room( ch, pexit->to_room );
                  do_look( ch, "auto" );
                  char_from_room( ch );
                  char_to_room( ch, original );
                  return;
               }
            }
            send_to_char( "You see swirling chaos...\n\r", ch );
            break;

         case ITEM_CONTAINER:
         case ITEM_QUIVER:
            if( IS_SET( obj->value[1], CONT_CLOSED ) )
            {
               send_to_char( "It is closed.\n\r", ch );
               break;
            }

         case ITEM_CORPSE_NPC:
         case ITEM_CORPSE_PC:
         case ITEM_FURNITURE:
         case ITEM_VEHICLE:
         case ITEM_KEYRING:
            count = obj->count;
            obj->count = 1;
            if( obj->item_type == ITEM_CONTAINER )
               act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
            else
               act( AT_PLAIN, "$p holds:", ch, obj, NULL, TO_CHAR );
            obj->count = count;
            show_list_to_char( obj->first_content, ch, TRUE, TRUE );
            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            break;
      }
      return;
   }

   if( ( pdesc = get_extra_descr( arg1, ch->in_room->first_extradesc ) ) != NULL )
   {
      send_to_char_color( pdesc, ch );
      return;
   }

   door = get_door( arg1 );
   if( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
   {
      if( IS_SET( pexit->exit_info, EX_CLOSED ) && !IS_SET( pexit->exit_info, EX_WINDOW ) )
      {
         if( ( IS_SET( pexit->exit_info, EX_SECRET ) || IS_SET( pexit->exit_info, EX_DIG ) ) && door != -1 )
            send_to_char( "Nothing special there.\n\r", ch );
         else
            act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
         return;
      }
      if( IS_SET( pexit->exit_info, EX_BASHED ) )
         act( AT_RED, "The $d has been bashed from its hinges!", ch, NULL, pexit->keyword, TO_CHAR );

      if( pexit->description && pexit->description[0] != '\0' )
         send_to_char( pexit->description, ch );
      else
         send_to_char( "Nothing special there.\n\r", ch );

      /*
       * Ability to look into the next room        -Thoric
       */
      if( pexit->to_room && ( TALENT( ch, TAL_SEEKING ) >= 10 || IS_SET( pexit->exit_info, EX_xLOOK ) ) )
      {
         original = ch->in_room;
         char_from_room( ch );
         char_to_room( ch, pexit->to_room );
         do_look( ch, "auto" );
         char_from_room( ch );
         char_to_room( ch, original );
      }
      return;
   }
   else if( door != -1 )
   {
      send_to_char( "Nothing special there.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) != NULL )
   {
      if( doexaprog )
         show_char_to_char_1( victim, ch );
      else
         show_char_to_char_2( ch, victim );
      return;
   }


   /*
    * finally fixed the annoying look 2.obj desc bug -Thoric 
    */
   number = number_argument( arg1, arg );
   for( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
   {
      if( can_see_obj( ch, obj ) )
      {
         if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            return;
         }

         if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            return;
         }
         if( nifty_is_name_prefix( arg, obj->name ) )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char( obj->description, ch );
            send_to_char( "\n\r", ch );
            pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
            if( !pdesc )
               pdesc = get_extra_descr( obj->name, obj->first_extradesc );
            if( !pdesc )
            {
               int diff = obj->weight - ( obj->condition / 50 );

               if( diff < -32 )
                  act( AT_PLAIN, "$p is in the best condition it could possibly be.", ch, obj, NULL, TO_CHAR );
               else if( diff < -16 )
                  act( AT_PLAIN, "$p has been enhanced to withstand much damage.", ch, obj, NULL, TO_CHAR );
               else if( diff < -8 )
                  act( AT_PLAIN, "$p looks very strong.", ch, obj, NULL, TO_CHAR );
               else if( diff < -4 )
                  act( AT_PLAIN, "$p seems like it would be able to withstand significant damage.", ch, obj, NULL, TO_CHAR );
               else if( diff < -2 )
                  act( AT_PLAIN, "$p looks somewhat stronger than normal.", ch, obj, NULL, TO_CHAR );
               else if( diff < 1 )
                  act( AT_PLAIN, "$p is in perfect condition.", ch, obj, NULL, TO_CHAR );
               else if( diff < 2 )
                  act( AT_PLAIN, "$p is slightly damaged.", ch, obj, NULL, TO_CHAR );
               else if( diff < 4 )
                  act( AT_PLAIN, "$p is somewhat damaged.", ch, obj, NULL, TO_CHAR );
               else if( diff < 8 )
                  act( AT_PLAIN, "$p is pretty damaged.", ch, obj, NULL, TO_CHAR );
               else if( diff < 16 )
                  act( AT_PLAIN, "$p is damaged.", ch, obj, NULL, TO_CHAR );
               else if( diff < 32 )
                  act( AT_PLAIN, "$p is severly damaged.", ch, obj, NULL, TO_CHAR );
               else
                  act( AT_PLAIN, "$p is falling apart!", ch, obj, NULL, TO_CHAR );
            }
            else
               send_to_char_color( pdesc, ch );

            if( obj->item_type == ITEM_BOOK )
            {
               EXTRA_DESCR_DATA *ed;
               send_to_char( "Contents:\n\r", ch );
               for( ed = obj->first_extradesc; ed; ed = ed->next )
               {
                  send_to_char( ed->keyword, ch );
                  if( ed->next )
                     send_to_char( "\n\r", ch );
               }
               send_to_char( "\n\r", ch );
            }

            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            return;
         }
      }
   }

   for( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
   {
      if( can_see_obj( ch, obj ) )
      {
         if( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            return;
         }

         if( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            send_to_char_color( pdesc, ch );
            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            return;
         }
         if( nifty_is_name_prefix( arg, obj->name ) )
         {
            if( ( cnt += obj->count ) < number )
               continue;
            pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
            if( !pdesc )
               pdesc = get_extra_descr( obj->name, obj->first_extradesc );
            if( !pdesc )
               send_to_char( "You see nothing special.\r\n", ch );
            else
               send_to_char_color( pdesc, ch );
            if( doexaprog )
               oprog_examine_trigger( ch, obj );
            return;
         }
      }
   }

   send_to_char( "You do not see that here.\n\r", ch );
   return;
}

void show_race_line( CHAR_DATA * ch, CHAR_DATA * victim )
{
   char buf[MAX_STRING_LENGTH];
   int feet, inches;

   if( !IS_NPC( victim ) )
      ch_printf( ch, "%s is %s %s.\n\r", PERS( victim, ch ), aoran( victim->pcdata->type ), victim->species );


   if( !IS_NPC( victim ) && ( victim != ch ) )
   {
      feet = victim->height / 12;
      inches = victim->height % 12;
      sprintf( buf, "%s is %d'%d\" and weighs %d pounds.\n\r", PERS( victim, ch ), feet, inches, victim->weight );
      send_to_char( buf, ch );
      return;
   }
   if( !IS_NPC( victim ) && ( victim == ch ) )
   {
      feet = victim->height / 12;
      inches = victim->height % 12;
      sprintf( buf, "You are %d'%d\" and weigh %d pounds.\n\r", feet, inches, victim->weight );
      send_to_char( buf, ch );
      return;
   }

}


void show_condition( CHAR_DATA * ch, CHAR_DATA * victim )
{
   char buf[MAX_STRING_LENGTH];
   int percent;

   if( victim->max_hit > 0 )
      percent = ( 100 * victim->hit ) / victim->max_hit;
   else
      percent = -1;


   if( victim != ch )
   {
      strcpy( buf, PERS( victim, ch ) );
      if( percent >= 100 )
         strcat( buf, " is in perfect health.\n\r" );
      else if( percent >= 90 )
         strcat( buf, " is slightly scratched.\n\r" );
      else if( percent >= 80 )
         strcat( buf, " has a few bruises.\n\r" );
      else if( percent >= 70 )
         strcat( buf, " has some cuts.\n\r" );
      else if( percent >= 60 )
         strcat( buf, " has several wounds.\n\r" );
      else if( percent >= 50 )
         strcat( buf, " has many nasty wounds.\n\r" );
      else if( percent >= 40 )
         strcat( buf, " is bleeding freely.\n\r" );
      else if( percent >= 30 )
         strcat( buf, " is covered in blood.\n\r" );
      else if( percent >= 20 )
         strcat( buf, " is leaking guts.\n\r" );
      else if( percent >= 10 )
         strcat( buf, " is almost dead.\n\r" );
      else
         strcat( buf, " is DYING.\n\r" );
   }
   else
   {
      strcpy( buf, "You" );
      if( percent >= 100 )
         strcat( buf, " are in perfect health.\n\r" );
      else if( percent >= 90 )
         strcat( buf, " are slightly scratched.\n\r" );
      else if( percent >= 80 )
         strcat( buf, " have a few bruises.\n\r" );
      else if( percent >= 70 )
         strcat( buf, " have some cuts.\n\r" );
      else if( percent >= 60 )
         strcat( buf, " have several wounds.\n\r" );
      else if( percent >= 50 )
         strcat( buf, " have many nasty wounds.\n\r" );
      else if( percent >= 40 )
         strcat( buf, " are bleeding freely.\n\r" );
      else if( percent >= 30 )
         strcat( buf, " are covered in blood.\n\r" );
      else if( percent >= 20 )
         strcat( buf, " are leaking guts.\n\r" );
      else if( percent >= 10 )
         strcat( buf, " are almost dead.\n\r" );
      else
         strcat( buf, " are DYING.\n\r" );
   }

   buf[0] = UPPER( buf[0] );
   send_to_char( buf, ch );
   return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
void do_glance( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   bool brief;

   if( !ch->desc )
      return;

   if( ch->position < POS_SLEEPING )
   {
      send_to_char( "You can't see anything but stars!\n\r", ch );
      return;
   }

   if( ch->position == POS_SLEEPING )
   {
      send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
      return;
   }

   if( !check_blind( ch ) )
      return;

   set_char_color( AT_ACTION, ch );
   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' )
   {
      if( xIS_SET( ch->act, PLR_BRIEF ) )
         brief = TRUE;
      else
         brief = FALSE;
      xSET_BIT( ch->act, PLR_BRIEF );
      do_look( ch, "auto" );
      if( !brief )
         xREMOVE_BIT( ch->act, PLR_BRIEF );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They're not here.\n\r", ch );
      return;
   }
   else
   {
      if( can_see( victim, ch ) )
      {
         act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT );
         act( AT_ACTION, "$n glances at $N.", ch, NULL, victim, TO_NOTVICT );
      }
      show_char_to_char_2( ch, victim );
   }
   show_condition( ch, victim );

   return;
}

void do_examine( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   sh_int dam;

   if( !argument )
   {
      bug( "do_examine: null argument.", 0 );
      return;
   }

   if( !ch )
   {
      bug( "do_examine: null ch.", 0 );
      return;
   }

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Examine what?\n\r", ch );
      return;
   }

   sprintf( buf, "%s noprog", arg );
   do_look( ch, buf );

   /*
    * Support for looking at boards, checking equipment conditions,
    * and support for trigger positions by Thoric
    */
   if( ( obj = get_obj_here( ch, arg ) ) != NULL )
   {
      ch_printf( ch, "%s\n\r", obj->description );
      switch ( obj->item_type )
      {
         case ITEM_COOK:
            strcpy( buf, "As you examine it carefully you notice that it " );
            dam = obj->value[2];
            if( dam >= 3 )
               strcat( buf, "is burned to a crisp." );
            else if( dam == 1 )
               strcat( buf, "is a little over cooked." );
            else if( dam == 1 )
               strcat( buf, "is perfectly roasted." );
            else
               strcat( buf, "is raw." );
            strcat( buf, "\n\r" );
            send_to_char( buf, ch );
         case ITEM_FOOD:
            if( obj->timer > 0 && obj->value[1] > 0 )
               dam = ( obj->timer * 10 ) / obj->value[1];
            else
               dam = 10;
            if( obj->item_type == ITEM_FOOD )
               strcpy( buf, "As you examine it carefully you notice that it " );
            else
               strcpy( buf, "Also it " );
            if( dam >= 10 )
               strcat( buf, "is fresh." );
            else if( dam == 9 )
               strcat( buf, "is nearly fresh." );
            else if( dam == 8 )
               strcat( buf, "is perfectly fine." );
            else if( dam == 7 )
               strcat( buf, "looks good." );
            else if( dam == 6 )
               strcat( buf, "looks ok." );
            else if( dam == 5 )
               strcat( buf, "is a little stale." );
            else if( dam == 4 )
               strcat( buf, "is a bit stale." );
            else if( dam == 3 )
               strcat( buf, "smells slightly off." );
            else if( dam == 2 )
               strcat( buf, "smells quite rank." );
            else if( dam == 1 )
               strcat( buf, "smells revolting!" );
            else if( dam <= 0 )
               strcat( buf, "is crawling with maggots!" );
            strcat( buf, "\n\r" );
            send_to_char( buf, ch );
            break;


         case ITEM_SWITCH:
         case ITEM_LEVER:
         case ITEM_PULLCHAIN:
            if( IS_SET( obj->value[0], TRIG_UP ) )
               send_to_char( "You notice that it is in the up position.\n\r", ch );
            else
               send_to_char( "You notice that it is in the down position.\n\r", ch );
            break;
         case ITEM_BUTTON:
            if( IS_SET( obj->value[0], TRIG_UP ) )
               send_to_char( "You notice that it is depressed.\n\r", ch );
            else
               send_to_char( "You notice that it is not depressed.\n\r", ch );
            break;

         case ITEM_CORPSE_PC:
         case ITEM_CORPSE_NPC:
         {
            sh_int timerfrac = obj->timer;
            if( obj->item_type == ITEM_CORPSE_PC )
               timerfrac = ( int )obj->timer / 8 + 1;

            switch ( timerfrac )
            {
               default:
                  send_to_char( "This corpse has recently been slain.\n\r", ch );
                  break;
               case 4:
                  send_to_char( "This corpse was slain a little while ago.\n\r", ch );
                  break;
               case 3:
                  send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\n\r", ch );
                  break;
               case 2:
                  send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\n\r", ch );
                  break;
               case 1:
               case 0:
                  send_to_char( "Little more than bones, there isn't much left of this corpse.\n\r", ch );
                  break;
            }
         }
         case ITEM_CONTAINER:
            if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
               break;
         case ITEM_DRINK_CON:
         case ITEM_FURNITURE:
         case ITEM_QUIVER:
            send_to_char( "When you look inside, you see:\n\r", ch );
         case ITEM_KEYRING:
            sprintf( buf, "in %s noprog", arg );
            do_look( ch, buf );
            break;
      }
      if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
      {
         sprintf( buf, "under %s noprog", arg );
         do_look( ch, buf );
      }
      oprog_examine_trigger( ch, obj );
      if( char_died( ch ) || obj_extracted( obj ) )
         return;

      check_for_trap( ch, obj, TRAP_EXAMINE );
   }
   return;
}


void do_exits( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   EXIT_DATA *pexit;
   bool found;
   bool fAuto;

   set_char_color( AT_EXITS, ch );
   buf[0] = '\0';
   fAuto = !str_cmp( argument, "auto" );

   if( !check_blind( ch ) )
      return;

   strcpy( buf, fAuto ? "Exits:" : "Obvious exits:\n\r" );

   found = FALSE;
   for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( pexit->to_room && ( !IS_SET( pexit->exit_info, EX_HIDDEN ) || ch->curr_talent[TAL_SEEKING] >= 65 ) )
      {
         found = TRUE;
         if( fAuto )
         {
            strcat( buf, " " );
/* make life easier - shogar */
            if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
               sprintf( buf + strlen( buf ), "[%d]-", pexit->to_room->vnum );
            if( !IS_SET( pexit->exit_info, EX_SECRET ) || ch->curr_talent[TAL_SEEKING] >= 80 )
            {
               if( IS_SET( pexit->exit_info, EX_CLOSED ) )
                  strcat( buf, "&O#&w" );
               strcat( buf, dir_name[pexit->vdir] );
            }
         }
         else
         {
            if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
               sprintf( buf + strlen( buf ), "[%-5d] %-1s%-5s - %s\n\r",
                        pexit->to_room->vnum,
                        IS_SET( pexit->exit_info, EX_CLOSED )
                        ? "&O#&w" : "&w", capitalize( dir_name[pexit->vdir] ), pexit->to_room->name );
            else
               sprintf( buf + strlen( buf ), "%-1s%-5s - %s\n\r",
                        IS_SET( pexit->exit_info, EX_CLOSED )
                        ? "&O#&w" : "&w", capitalize( dir_name[pexit->vdir] ), pexit->to_room->name );
         }
      }
   }

   if( !found )
      strcat( buf, fAuto ? " none.\n\r" : "None.\n\r" );
   else if( fAuto )
      strcat( buf, ".\n\r" );
   send_to_char( buf, ch );
   return;
}

/*
To:       Scion
From:     Keolah
Date:     Thu Jul 29 16:40:35 1999
Subject:  months and days

Days:
Vannes (Sun), Kelivan (mountain), Vesevan (star), Tativan (tree),
Retivan (water), Daltavan (song), Vestivan (wind)

Months:
Brykena (change), Relithoves (sleet), Vestejen (gale), Tatarmyr (bloom),
Sesiddet (traveling), Heygavan (summer), Veseveret (blue star),
Vankampa (glitter), Dalteyves (thunder), Nastarmath (autumn),
Retsoves (rain), Kinrelth (snow)
*/

char *const day_name[] = {
   "Kelivan", "Vesevan", "Tativan", "Retivan", "Daltevan", "Vestivan", "Vannes"
};

char *const month_name[] = {
   "Brykena", "Relithoves", "Vestejan", "Tatarmyr",
   "Sesiddet", "Heygavan", "Veseveret", "Vankampa", "Dalteyves",
   "Nastarmath", "Retsoves", "Kinrelth"
};

void do_time( CHAR_DATA * ch, char *argument )
{
   extern char str_boot_time[];
   extern char reboot_time[];
   char *suf;
   int day;

   day = time_info.day + 1;

   if( day > 4 && day < 20 )
      suf = "th";
   else if( day % 10 == 1 )
      suf = "st";
   else if( day % 10 == 2 )
      suf = "nd";
   else if( day % 10 == 3 )
      suf = "rd";
   else
      suf = "th";

   set_char_color( AT_YELLOW, ch );
   ch_printf( ch,
              "It is %d o'clock %s on %s, the %d%s of the Month of %s, the year %d.\n\r"
              "The mud started up at:    %s\r"
              "The system time (E.S.T.): %s\r"
              "Next Reboot is set for:   %s\r",
              ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
              time_info.hour >= 12 ? "pm" : "am",
              day_name[day % 7],
              day, suf,
              month_name[time_info.month], time_info.year, str_boot_time, ( char * )ctime( &current_time ), reboot_time );

   return;
}

/*
 * Produce a description of the weather based on area weather using
 * the following sentence format:
 *		<combo-phrase> and <single-phrase>.
 * Where the combo-phrase describes either the precipitation and
 * temperature or the wind and temperature. The single-phrase
 * describes either the wind or precipitation depending upon the
 * combo-phrase.
 * Last Modified: July 31, 1997
 * Fireblade - Under Construction
 */
void do_weather( CHAR_DATA * ch, char *argument )
{
   int climate;
   AREA_DATA *tarea = ch->in_room->area;
   char *combo, *single;
   char buf[MAX_INPUT_LENGTH];
   int temp, precip, wind, mana;
   MOON_DATA *moon;

   if( !argument || argument[0] == '\0' )
   {

      temp = ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit;
      precip = ( ch->in_room->area->weather->precip + 3 * weath_unit - 1 ) / weath_unit;
      wind = ( ch->in_room->area->weather->wind + 3 * weath_unit - 1 ) / weath_unit;
      mana = ( ch->in_room->area->weather->mana + 3 * weath_unit - 1 ) / weath_unit;

      if( IS_OUTSIDE( ch ) )
      {

         if( precip >= 3 )
         {
            combo = preciptemp_msg[precip][temp];
            single = wind_msg[wind];
         }
         else
         {
            combo = windtemp_msg[wind][temp];
            single = precip_msg[precip];
         }

         sprintf( buf, "%s and %s.\n\r", combo, single );

         set_char_color( AT_BLUE, ch );

         ch_printf( ch, buf );

         if( IS_SET( sysdata.quest, QUEST_ETERNAL_NIGHT ) )
         {
            sprintf( buf, "The sky lies black under eternal night.\n\r" );
         }
         else
         {
            if( precip >= 3 )
               sprintf( buf, "%s\n\r", cloudyday_msg[( int )time_info.hour / 4] );
            else
               sprintf( buf, "%s\n\r", clearday_msg[( int )time_info.hour / 4] );
         }
         ch_printf( ch, buf );

         moon = first_moon;
         while( moon )
         {
            if( ( moon->up ) && ( !str_cmp( moon->world, ch->in_room->area->resetmsg ) ) )
            {
               sprintf( buf, "%s %s moon hangs in the sky.\n\r",
                        capitalize( aoran( moon->color ) ), moon_phase[moon->phase] );
               ch_printf( ch, buf );
            }
            moon = moon->next;
         }

      }
      else
         ch_printf( ch, "You can't see the sky from here.\n\r" );

      mana = URANGE( 0, mana, 5 );
      temp = URANGE( 0, temp, 5 );

      if( ch->mana > ( int )ch->max_mana / 20 )
      {
         sprintf( buf, "%s.\r\n", manatemp_msg[mana][temp] );
         ch_printf( ch, buf );
      }
      return;
   }

   /*
    * Let you alter the weather in one area based on
    * * your effectivity in the appropriate talent
    */
   if( !str_cmp( argument, "warmer" ) && ch->curr_talent[TAL_FIRE] >= 30 )
   {
      climate = tarea->weather->climate_temp * 100 - 200;
      if( tarea->weather->temp >= climate + TALENT( ch, TAL_FIRE ) )
      {
         send_to_char( "You can't make this area any hotter.\n\r", ch );
         return;
      }
      tarea->weather->temp += 10;
      send_to_char( "&RYou heat up the area a bit.\n\r", ch );
      use_magic( ch, TAL_FIRE, 250 );
      return;

   }
   else if( !str_cmp( argument, "colder" ) && ch->curr_talent[TAL_FROST] >= 30 )
   {
      climate = tarea->weather->climate_temp * 100 - 200;
      if( tarea->weather->temp <= climate - TALENT( ch, TAL_FROST ) )
      {
         send_to_char( "You can't make this area any colder.\n\r", ch );
         return;
      }
      tarea->weather->temp -= 10;
      send_to_char( "&CYou bring a winter chill to the region.\n\r", ch );
      use_magic( ch, TAL_FROST, 250 );
      return;

   }
   else if( !str_cmp( argument, "windier" ) && ch->curr_talent[TAL_WIND] >= 30 )
   {
      climate = tarea->weather->climate_wind * 100 - 200;
      if( tarea->weather->wind >= climate + TALENT( ch, TAL_WIND ) )
      {
         send_to_char( "You can't make this area any windier.\n\r", ch );
         return;
      }
      tarea->weather->wind += 10;
      send_to_char( "&cYou whip up the winds in the area.\n\r", ch );
      use_magic( ch, TAL_WIND, 250 );
      return;

   }
   else if( !str_cmp( argument, "stiller" ) && ch->curr_talent[TAL_WIND] >= 50 )
   {
      climate = tarea->weather->climate_precip * 100 - 200;
      if( tarea->weather->precip <= climate - TALENT( ch, TAL_WIND ) )
      {
         send_to_char( "You can't make the wind in this area any stiller.\n\r", ch );
         return;
      }
      tarea->weather->precip += 10;
      send_to_char( "&cYou calm the winds in the region.\n\r", ch );
      use_magic( ch, TAL_WIND, 250 );
      return;

   }
   else if( !str_cmp( argument, "wetter" ) && ch->curr_talent[TAL_WATER] >= 30 )
   {
      climate = tarea->weather->climate_precip * 100 - 200;
      if( tarea->weather->precip >= climate + TALENT( ch, TAL_WATER ) )
      {
         send_to_char( "You can't make this area any wetter.\n\r", ch );
         return;
      }
      tarea->weather->precip += 10;
      send_to_char( "&BYou build up moisture in the air.\n\r", ch );
      use_magic( ch, TAL_WATER, 250 );
      return;

   }
   else if( !str_cmp( argument, "drier" ) && ch->curr_talent[TAL_FIRE] >= 60 )
   {
      climate = tarea->weather->climate_precip * 100 - 200;
      if( tarea->weather->precip <= climate - TALENT( ch, TAL_FIRE ) )
      {
         send_to_char( "You can't make this area any drier.\n\r", ch );
         return;
      }
      tarea->weather->precip -= 10;
      send_to_char( "&YYou evaporate moisture in the air.\n\r", ch );
      use_magic( ch, TAL_FIRE, 250 );
      return;

   }
   else
   {
      send_to_char( "You can do no such thing to the weather.\n\r", ch );
      return;
   }
}

/* A more internal version of do_weather that returns the weather
 * string instead of sending it to the char. Used for generated
 * room descriptions -- Scion
 */
char *get_weather_string( CHAR_DATA * ch, char *weather )
{
   char *combo, *single;
   char buf[MAX_INPUT_LENGTH];
   char magic[MAX_INPUT_LENGTH];
   int temp, precip, wind, mana;
   MOON_DATA *moon;

   temp = ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit;
   precip = ( ch->in_room->area->weather->precip + 3 * weath_unit - 1 ) / weath_unit;
   wind = ( ch->in_room->area->weather->wind + 3 * weath_unit - 1 ) / weath_unit;
   mana = ( ch->in_room->area->weather->mana + 3 * weath_unit - 1 ) / weath_unit;

   if( IS_OUTSIDE( ch ) )
   {

      if( precip >= 3 )
      {
         combo = preciptemp_msg[precip][temp];
         single = wind_msg[wind];
      }
      else
      {
         combo = windtemp_msg[wind][temp];
         single = precip_msg[precip];
      }

      sprintf( buf, "%s and %s.\n\r", combo, single );

      if( IS_SET( sysdata.quest, QUEST_ETERNAL_NIGHT ) )
      {
         sprintf( magic, "The sky lies black under eternal night.\n\r" );
      }
      else
      {
         if( precip >= 3 )
            sprintf( magic, "%s\n\r", cloudyday_msg[( int )time_info.hour / 4] );
         else
            sprintf( magic, "%s\n\r", clearday_msg[( int )time_info.hour / 4] );
      }
      strcat( buf, magic );

      moon = first_moon;
      while( moon )
      {
         if( ( moon->up ) && ( !str_cmp( moon->world, ch->in_room->area->resetmsg ) ) )
         {
            sprintf( magic, "%s %s moon hangs in the sky.\n\r",
                     capitalize( aoran( moon->color ) ), moon_phase[moon->phase] );
            strcat( buf, magic );
         }
         moon = moon->next;
      }

   }
   else
      strcpy( buf, "" );

   /*
    * Were getting crashes when this hit 6 for some reason -- Scion 
    */
   mana = URANGE( 0, mana, 5 );

   if( ch->mana > ( int )ch->max_mana / 20 )
   {
      sprintf( magic, " %s.\r\n", manatemp_msg[mana][temp] );
   }

   strcat( buf, magic );

   weather = STRALLOC( buf );

   return weather;
}

/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help( CHAR_DATA * ch, char *argument )
{
   char argall[MAX_INPUT_LENGTH];
   char argone[MAX_INPUT_LENGTH];
   char argnew[MAX_INPUT_LENGTH];
   HELP_DATA *pHelp;
   int lev;

   if( argument[0] == '\0' )
      argument = "summary";

   if( isdigit( argument[0] ) )
   {
      lev = number_argument( argument, argnew );
      argument = argnew;
   }
   else
      lev = -2;
   /*
    * Tricky argument handling so 'help a b' doesn't match a.
    */
   argall[0] = '\0';
   while( argument[0] != '\0' )
   {
      argument = one_argument( argument, argone );
      if( argall[0] != '\0' )
         strcat( argall, " " );
      strcat( argall, argone );
   }

   for( pHelp = first_help; pHelp; pHelp = pHelp->next )
   {
      if( pHelp->permit && !IS_SET( ch->pcdata->permissions, pHelp->permit ) )
         continue;

      if( is_name( argall, pHelp->keyword ) )
         return pHelp;
   }

   return NULL;
}

/*
 * LAWS command
 */
void do_laws( CHAR_DATA * ch, char *argument )
{
   char buf[1024];

   if( argument == NULL )
      do_new_help( ch, "rules" );
   else
   {
      sprintf( buf, "rules_%s", argument );
      do_new_help( ch, buf );
   }
}

void do_who( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char invis_str[MAX_INPUT_LENGTH];
   char rank_text[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int nMatch = 0;

   CHAR_DATA *wch;
   char *rank;

   send_to_pager( "\n\r&Y                        Elkandu in the Planes\n\r\n\r", ch );

   for( d = first_descriptor; d; d = d->next )
   {

      if( d->connected != CON_PLAYING && d->connected != CON_EDITING )
         continue;

      if( !d->character )
         continue;
      wch = d->character;

      sprintf( rank_text, "%s", ( get_talent_rank( wch ) ) );
      rank = rank_text;

      if( get_char_worth( wch ) >= 2000 && ( wch->pcdata->rank && wch->pcdata->rank[0] != '\0' ) )
         rank = wch->pcdata->rank;


      else if( IS_SET( wch->pcdata->flags, PCFLAG_ANONYMOUS ) )
         rank = "&zAnonymous";



      if( xIS_SET( wch->act, PLR_WIZINVIS ) )
         sprintf( invis_str, " &R(&WInvis %d&R)&w", wch->pcdata->wizinvis );
      else
         invis_str[0] = '\0';
      sprintf( buf, "^x&C[&W%s&C] &w%s %s%s%s%s%s%s%s\n\r",
               const_color_align( rank, 12, ALIGN_CENTER ),
               wch->pcdata->title,
               invis_str,
               ( wch->desc && wch->desc->connected ) ? " &Y[&WWRITING&Y]&w " : "",
               IS_AFFECTED( wch, AFF_DREAMWORLD ) ? "&P[&pDreamworld&P]&w " : "",
               IS_AFFECTED( wch, AFF_VOID ) ? " &b[&zVoid&b]&w " : "",
               xIS_SET( wch->act, PLR_OOC ) ? " &Y[&WOOC&Y]&w " : "",
               xIS_SET( wch->act, PLR_AFK ) ? " &Y[&WIDLE&Y]&w " : "",
               xIS_SET( wch->act, PLR_BOUNTY ) ? " &R(&WBOUNTY&R)&w " : "" );

      nMatch++;
      send_to_char( buf, ch );

   }

   send_to_pager( "&x\n\r", ch );
   set_char_color( AT_YELLOW, ch );
   ch_printf( ch, "&Y[&WThere %s &Y%d player%s&W on %s.&Y]&w\n\r",
              nMatch == 1 ? "is" : "are", nMatch, nMatch == 1 ? "" : "s", sysdata.mud_name );
   return;
}


/* This command SUCKS! It's misleading enough to be dangerous to morts. -- Scion
 * Re-written Jan 25, 2001 by Keolah
 */

void do_compare( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj1;
   OBJ_DATA *obj2;
   int diff;
   int value1;
   int value2;
   char *msg;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Compare what to what?\n\r", ch );
      return;
   }

   if( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
   {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' )
   {
      for( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
      {
         if( obj2->wear_loc != WEAR_NONE && can_see_obj( ch, obj2 ) && obj1->item_type == obj2->item_type )
            break;
      }

      if( !obj2 )
      {
         send_to_char( "You aren't wearing anything comparable.\n\r", ch );
         return;
      }
   }
   else
   {
      if( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
      {
         send_to_char( "You do not have that item.\n\r", ch );
         return;
      }
   }

   msg = NULL;
   value1 = 0;
   value2 = 0;

   if( obj1 == obj2 )
   {
      msg = "You compare $p to itself.  It looks about the same.";
   }
   else if( obj1->item_type != obj2->item_type )
   {
      msg = "You can't compare $p and $P.";
   }
   else
   {
      switch ( obj1->item_type )
      {
         default:
            msg = "You can't compare $p and $P.";
            break;

         case ITEM_ARMOR:
            value1 = obj1->cost;
            value2 = obj2->cost;
            break;

         case ITEM_WEAPON:
            value1 = obj1->cost;
            value2 = obj2->cost;
            break;
      }
   }

   diff = abs( value1 - value2 );
   if( !msg )
   {
      if( diff < 100000 )
         msg = "$p and $P look about the same value.";
      else if( value1 > value2 )
      {
         if( diff > 500000 )
         {
            msg = "$p looks far more valuable than $P.";
         }
         else
         {
            msg = "$p looks more valuable than $P.";
         }
      }
      else
      {
         if( diff > 500000 )
         {
            msg = "$p looks far less valuable than $P.";
         }
         else
         {
            msg = "$p looks less valuable than $P.";
         }
      }
   }

   act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
   return;
}


void do_where( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   bool found;

   if( ch->curr_talent[TAL_SEEKING] < 40 )
   {
      ch_printf( ch, "You are in %s.", ch->in_room->name );
      return;
   }

   set_pager_color( AT_PERSON, ch );
   pager_printf( ch, "Elkandu who you can Seek right now:\n\r" );
   found = FALSE;
   for( d = first_descriptor; d; d = d->next )
      if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
          && ( victim = d->character ) != NULL
          && !IS_NPC( victim )
          && victim->in_room
          && ( victim->in_room->area == ch->in_room->area
               || ch->curr_talent[TAL_SEEKING] >= 60 )
          && ( !str_cmp( victim->in_room->area->resetmsg,
                         ch->in_room->area->resetmsg )
               || ch->curr_talent[TAL_SEEKING] >= 80 )
          && ( !IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL )
               || ch->curr_talent[TAL_SEEKING] >= 90 ) && can_see( ch, victim ) )
      {
         found = TRUE;
         pager_printf_color( ch, "%-15s&P  ", PERS( victim, ch ) );
         pager_printf_color( ch, "%-20s  ", victim->in_room->name );
         pager_printf_color( ch, "%-15s  ", victim->in_room->area->name );
         pager_printf_color( ch, "%-10s\n\r", victim->in_room->area->resetmsg );
      }
   if( !found )
      send_to_char( "None\n\r", ch );

   return;
}



/* Recoded by Keolah Oct 12 2000 */
void do_consider( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char *msg;
   int diff;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Consider killing whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They're not here.\n\r", ch );
      return;
   }

   diff = get_char_worth( victim );

   if( diff < 200 )
      msg = "$N is probably not worth killing.";
   else if( diff < 500 )
      msg = "$N appears inexperienced.";
   else if( diff < 1000 )
      msg = "$N seems to move with confidence.";
   else if( diff < 1500 )
      msg = "$N appears experienced and confident.";
   else if( diff < 2000 )
      msg = "$N moves with a deadly grace and power.";
   else
      msg = "$N radiates an aura of power and command.";

/*    diff = get_char_worth(victim) - get_char_worth(ch);

	 if ( diff < -150) msg = "$N is as weak as a feather!";
    else if ( diff < -100) msg = "You could kill $N with your hands tied!";
    else if ( diff <  -50) msg = "Piece of cake.";
    else if ( diff <  -20) msg = "$N is a wimp.";
    else if ( diff <    0) msg = "$N looks weaker than you.";
    else if ( diff <   20) msg = "$N looks about as strong as you.";
    else if ( diff <   50) msg = "It would take a bit of luck...";
    else if ( diff <  100) msg = "It would take a lot of luck, and great equipment!";
    else if ( diff <  150) msg = "Why don't you dig a grave for yourself first?";
    else                    msg = "It'd be easier to kill Keolah with your head chopped off.";
*/ act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

   act( AT_CONSIDER, "$n looks at you very closely...", ch, NULL, victim, TO_VICT );
   act( AT_CONSIDER, "$n looks at $N very closely.", ch, NULL, victim, TO_NOTVICT );

   return;
}


void do_practice( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int sn;

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      int col;
      sh_int lasttype, cnt;

      col = cnt = 0;
      lasttype = SKILL_SPELL;
      set_pager_color( AT_MAGIC, ch );
      for( sn = 0; sn < top_sn; sn++ )
      {
         if( !skill_table[sn]->name )
            break;

         if( strcmp( skill_table[sn]->name, "reserved" ) == 0 && ( CAN_CAST( ch ) ) )
         {
            if( col % 3 != 0 )
               send_to_pager( "\n\r", ch );
            set_pager_color( AT_MAGIC, ch );
            send_to_pager_color( " ----------------------------------[&CSpells&B]----------------------------------\n\r",
                                 ch );
            col = 0;
         }
         if( skill_table[sn]->type != lasttype )
         {
            if( !cnt )
               send_to_pager( "                                   (none)\n\r", ch );
            else if( col % 3 != 0 )
               send_to_pager( "\n\r", ch );
            set_pager_color( AT_MAGIC, ch );
            pager_printf_color( ch,
                                " ----------------------------------&C%ss&B----------------------------------\n\r",
                                skill_tname[skill_table[sn]->type] );
            col = cnt = 0;
         }
         lasttype = skill_table[sn]->type;

         if( skill_available( ch, sn ) == FALSE )
            continue;

         if( ( LEARNED( ch, sn ) < 1 ) && SPELL_FLAG( skill_table[sn], SF_SECRETSKILL ) )
            continue;

         if( lasttype != SKILL_SPELL && LEARNED( ch, sn ) < 1 )
            continue;

         ++cnt;
         set_pager_color( AT_MAGIC, ch );
         pager_printf( ch, "%20.20s", skill_table[sn]->name );

         if( LEARNED( ch, sn ) > 0 )
            set_pager_color( AT_SCORE, ch );
         pager_printf( ch, " %3d%% ", LEARNED( ch, sn ) );

         if( ++col % 3 == 0 )
            send_to_pager( "\n\r", ch );
      }

      if( col % 3 != 0 )
         send_to_pager( "\n\r", ch );
      set_pager_color( AT_MAGIC, ch );
   }
   else
   {
      CHAR_DATA *mob;
      bool can_prac = TRUE;

      if( !IS_AWAKE( ch ) )
      {
         send_to_char( "In your dreams, or what?\n\r", ch );
         return;
      }

      for( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
         if( IS_NPC( mob ) && xIS_SET( mob->act, ACT_PRACTICE ) )
            break;

      if( !mob )
      {
         send_to_char( "You can't do that here.\n\r", ch );
         return;
      }

      sn = skill_lookup( argument );

      can_prac = skill_available( ch, sn );

      if( can_prac == FALSE )
      {
         act( AT_TELL, "$n tells you 'You cannot learn any such skill...'", mob, NULL, ch, TO_VICT );
         return;
      }


      /*
       * Skill requires a special teacher
       */
      if( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
      {
         sprintf( buf, "%d", mob->pIndexData->vnum );
         if( !is_name( buf, skill_table[sn]->teachers ) )
         {
            act( AT_TELL, "$n tells you, 'I know not know how to teach that.'", mob, NULL, ch, TO_VICT );
            return;
         }
      }

      if( LEARNED( ch, sn ) >= 1 )
      {
         sprintf( buf, "$n tells you, 'I've taught you everything I can about %s.'", skill_table[sn]->name );
         act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
      }
      else
      {
         ch->pcdata->learned[sn] += number_range( 1, 15 );
         act( AT_ACTION, "You practice $T.", ch, NULL, skill_table[sn]->name, TO_CHAR );
         act( AT_ACTION, "$n practices $T.", ch, NULL, skill_table[sn]->name, TO_ROOM );
      }
   }
   return;
}



void do_password( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char *pwdnew;
   char *p;

   if( IS_NPC( ch ) )
      return;

   argument = one_argument_retain_case( argument, arg1 );
   argument = one_argument_retain_case( argument, arg2 );

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: password <new> <again>.\n\r", ch );
      return;
   }

/* This should stop all the mistyped password problems --Shaddai */
   if( strcmp( arg1, arg2 ) )
   {
      send_to_char( "Passwords don't match try again.\n\r", ch );
      return;
   }
   if( strlen( arg2 ) < 5 )
   {
      send_to_char( "New password must be at least five characters long.\n\r", ch );
      return;
   }

   /*
    * No tilde allowed because of player file format.
    */
   pwdnew = smaug_crypt( arg2 );
   for( p = pwdnew; *p != '\0'; p++ )
   {
      if( *p == '~' )
      {
         send_to_char( "New password not acceptable, try again.\n\r", ch );
         return;
      }
   }

   DISPOSE( ch->pcdata->pwd );
   ch->pcdata->pwd = str_dup( pwdnew );
   if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
      save_char_obj( ch );
   send_to_char( "Ok.\n\r", ch );
   return;
}



void do_socials( CHAR_DATA * ch, char *argument )
{
   int iHash;
   int col = 0;
   SOCIALTYPE *social;

   set_pager_color( AT_PLAIN, ch );
   for( iHash = 0; iHash < 27; iHash++ )
      for( social = social_index[iHash]; social; social = social->next )
      {
         pager_printf( ch, "%-12s", social->name );
         if( ++col % 6 == 0 )
            send_to_pager( "\n\r", ch );
      }

   if( col % 6 != 0 )
      send_to_pager( "\n\r", ch );
   return;
}


void do_commands( CHAR_DATA * ch, char *argument )
{
   int col;
   bool found;
   int hash;
   CMDTYPE *command;

   col = 0;
   set_pager_color( AT_PLAIN, ch );
   if( argument[0] == '\0' )
   {
      for( hash = 0; hash < 126; hash++ )
         for( command = command_hash[hash]; command; command = command->next )
            if( command->permit == 0 )
            {
               pager_printf( ch, "%-12s", command->name );
               if( ++col % 6 == 0 )
                  send_to_pager( "\n\r", ch );
            }
      if( col % 6 != 0 )
         send_to_pager( "\n\r", ch );
   }
   else
   {
      found = FALSE;
      for( hash = 0; hash < 126; hash++ )
         for( command = command_hash[hash]; command; command = command->next )
            if( command->permit == 0 && !str_prefix( argument, command->name ) )
            {
               pager_printf( ch, "%-12s", command->name );
               found = TRUE;
               if( ++col % 6 == 0 )
                  send_to_pager( "\n\r", ch );
            }

      if( col % 6 != 0 )
         send_to_pager( "\n\r", ch );
      if( !found )
         ch_printf( ch, "No command found under %s.\n\r", argument );
   }
   return;
}

/*
 * Contributed by Grodyn.
 * Display completely overhauled, 2/97 -- Blodkai
 */
void do_config( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;

   one_argument( argument, arg );

   set_char_color( AT_GREEN, ch );

   if( arg[0] == '\0' )
   {
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\rConfigurations ", ch );
      set_char_color( AT_GREEN, ch );
      send_to_char( "(use 'config +/- <keyword>' to toggle, see 'help config')\n\r\n\r", ch );
      set_char_color( AT_DGREEN, ch );
      send_to_char( "Display:   ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch,
                 "%-12s   %-12s   %-12s   %-12s\n\r           %-12s    %-12s   %-12s   %-12s\n\r           %-12s   %-12s   %-12s   %-12s",
                 IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) ? "[+] PAGER" : "[-] pager", IS_SET( ch->pcdata->flags,
                                                                                                  PCFLAG_GAG ) ? "[+] GAG" :
                 "[-] gag", xIS_SET( ch->act, PLR_BRIEF ) ? "[+] BRIEF" : "[-] brief", xIS_SET( ch->act,
                                                                                                PLR_COMBINE ) ? "[+] COMBINE"
                 : "[-] combine", xIS_SET( ch->act, PLR_BLANK ) ? "[+] BLANK" : "[-] blank", xIS_SET( ch->act,
                                                                                                      PLR_PROMPT ) ?
                 "[+] PROMPT" : "[-] prompt", xIS_SET( ch->act, PLR_ANSI ) ? "[+] ANSI" : "[-] ansi",
                 IS_SET( ch->pcdata->flags, PCFLAG_NOBEEP ) ? "[+] NOBEEP" : "[-] nobeep", IS_SET( ch->pcdata->flags,
                                                                                                   PCFLAG_ANONYMOUS ) ?
                 "[+] ANONYMOUS" : "[-] anonymous", xIS_SET( ch->act, PLR_NOHUNGER ) ? "[+] NOHUNGER" : "[-] nohunger",
                 xIS_SET( ch->act, PLR_OOC ) ? "[+] OOC" : "[-] ooc", xIS_SET( ch->act,
                                                                               PLR_LONG_EXIT ) ? "[+] LONGEXIT" :
                 "[-] longexit" );

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rAuto:      ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s   %-12s   %-12s",
                 xIS_SET( ch->act, PLR_AUTOSEX ) ? "[+] AUTOSEX"
                 : "[-] autosex",
                 xIS_SET( ch->act, PLR_AUTOGOLD ) ? "[+] AUTOGOLD"
                 : "[-] autogold",
                 xIS_SET( ch->act, PLR_AUTOLOOT ) ? "[+] AUTOLOOT"
                 : "[-] autoloot", xIS_SET( ch->act, PLR_AUTOEXIT ) ? "[+] AUTOEXIT" : "[-] autoexit" );

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rSafeties:  ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s     ",
                 IS_SET( ch->pcdata->flags, PCFLAG_SPAR ) ? "[+] SPAR"
                 : "[-] spar", xIS_SET( ch->act, PLR_DEFENSE ) ? "[+] DEFENSE" : "[-] defense" );

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rMisc:      ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s",
                 xIS_SET( ch->act, PLR_TELNET_GA ) ? "[+] TELNETGA"
                 : "[-] telnetga", IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) ? "[+] NOINTRO" : "[-] nointro" );
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rSettings:  ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf_color( ch, "Pager Length (%d)", ch->pcdata->pagerlen );

      if( IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
      {
         set_char_color( AT_DGREEN, ch );
         send_to_char( "\n\r\n\rImmortal toggles:  ", ch );
         set_char_color( AT_GREY, ch );
         ch_printf( ch, "%-12s\n\r        ", xIS_SET( ch->act, PLR_ROOMVNUM ) ? "[+] VNUM" : "[-] vnum" );
      }

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rTalent toggles:  ", ch );

      if( ch->curr_talent[TAL_ILLUSION] >= 70 )
      {
         set_char_color( AT_GREY, ch );
         ch_printf( ch, "%-12s    ", IS_SET( ch->pcdata->flags, PCFLAG_MASK ) ? "[+] AURAMASK" : "[-] auramask" );
      }
      if( ch->curr_talent[TAL_MIND] >= 90 )
      {
         set_char_color( AT_GREY, ch );
         ch_printf( ch, "%-12s    ", IS_SET( ch->pcdata->flags, PCFLAG_SHIELD ) ? "[+] MINDSHIELD" : "[-] mindshield" );
      }
      ch_printf( ch, "\n\r" );

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rSentences imposed on you (if any):", ch );
      set_char_color( AT_YELLOW, ch );
      ch_printf( ch, "\n\r%s%s%s%s%s%s",
                 xIS_SET( ch->act, PLR_SILENCE ) ?
                 " For your abuse of channels, you are currently silenced.\n\r" : "",
                 xIS_SET( ch->act, PLR_NO_EMOTE ) ?
                 " The gods have removed your emotes.\n\r" : "",
                 xIS_SET( ch->act, PLR_NO_TELL ) ?
                 " You are not permitted to send 'tells' to others.\n\r" : "",
                 xIS_SET( ch->act, PLR_LITTERBUG ) ?
                 " A convicted litterbug.  You cannot drop anything.\n\r" : "",
                 xIS_SET( ch->act, PLR_THIEF ) ?
                 " A proven thief, you will be hunted by the authorities.\n\r" : "",
                 xIS_SET( ch->act, PLR_KILLER ) ? " For the crime of murder you are sentenced to death...\n\r" : "" );
   }
   else
   {
      bool fSet;
      int bit = 0;

      if( arg[0] == '+' )
         fSet = TRUE;
      else if( arg[0] == '-' )
         fSet = FALSE;
      else
      {
         send_to_char( "Config -option or +option?\n\r", ch );
         return;
      }

      if( !str_prefix( arg + 1, "autoexit" ) )
         bit = PLR_AUTOEXIT;
      else if( !str_prefix( arg + 1, "autoloot" ) )
         bit = PLR_AUTOLOOT;
      else if( !str_prefix( arg + 1, "autosex" ) )
         bit = PLR_AUTOSEX;
      else if( !str_prefix( arg + 1, "longexit" ) )
         bit = PLR_LONG_EXIT;
      else if( !str_prefix( arg + 1, "nohunger" ) )
         bit = PLR_NOHUNGER;
      else if( !str_prefix( arg + 1, "defense" ) )
         bit = PLR_DEFENSE;
      else if( !str_prefix( arg + 1, "autogold" ) )
         bit = PLR_AUTOGOLD;
      else if( !str_prefix( arg + 1, "blank" ) )
         bit = PLR_BLANK;
      else if( !str_prefix( arg + 1, "brief" ) )
         bit = PLR_BRIEF;
      else if( !str_prefix( arg + 1, "combine" ) )
         bit = PLR_COMBINE;
      else if( !str_prefix( arg + 1, "prompt" ) )
         bit = PLR_PROMPT;
      else if( !str_prefix( arg + 1, "ooc" ) )
         bit = PLR_OOC;
      else if( !str_prefix( arg + 1, "telnetga" ) )
         bit = PLR_TELNET_GA;
      else if( !str_prefix( arg + 1, "ansi" ) )
         bit = PLR_ANSI;
      else if( !str_prefix( arg + 1, "vnum" ) )
         bit = PLR_ROOMVNUM;

      if( bit )
      {
         if( fSet )
            xSET_BIT( ch->act, bit );
         else
            xREMOVE_BIT( ch->act, bit );
         send_to_char( "Ok.\n\r", ch );
         return;
      }
      else
      {
         if( !str_prefix( arg + 1, "norecall" ) )
            bit = PCFLAG_NORECALL;
         else if( !str_prefix( arg + 1, "nointro" ) )
            bit = PCFLAG_NOINTRO;
         else if( !str_prefix( arg + 1, "nosummon" ) )
            bit = PCFLAG_NOSUMMON;
         else if( !str_prefix( arg + 1, "gag" ) )
            bit = PCFLAG_GAG;
         else if( !str_prefix( arg + 1, "pager" ) )
            bit = PCFLAG_PAGERON;
         else if( !str_prefix( arg + 1, "spar" ) )
            bit = PCFLAG_SPAR;
         else if( !str_prefix( arg + 1, "nobeep" ) )
            bit = PCFLAG_NOBEEP;
         else if( !str_prefix( arg + 1, "anonymous" ) )
            bit = PCFLAG_ANONYMOUS;
         else if( ch->curr_talent[TAL_ILLUSION] >= 70 && !str_prefix( arg + 1, "auramask" ) )
            bit = PCFLAG_MASK;
         else if( ch->curr_talent[TAL_MIND] >= 90 && !str_prefix( arg + 1, "mindshield" ) )
            bit = PCFLAG_SHIELD;
         else
         {
            send_to_char( "Config which option?\n\r", ch );
            return;
         }

         if( fSet )
            SET_BIT( ch->pcdata->flags, bit );
         else
            REMOVE_BIT( ch->pcdata->flags, bit );

         send_to_char( "Ok.\n\r", ch );
         return;
      }
   }

   return;
}


void do_credits( CHAR_DATA * ch, char *argument )
{
   do_new_help( ch, "credits" );
}


extern int top_area;

/*
 * New do_areas, written by Fireblade, last modified - 4/27/97
 *
 *   Syntax: area            ->      lists areas in alphanumeric order
 *           area <world>    ->	     lists areas on a given world
 *           area old        ->      list areas in order loaded
 *
 */
void do_areas( CHAR_DATA * ch, char *argument )
{
   char *header_string1 = "\n\r   Author    |             Area" "                     | " "World\n\r";
   char *header_string2 = "-------------+-----------------" "---------------------+----" "---------+-----------\n\r";
   char *print_string = "%-12s | %-36s | %11s | \n\r";

   AREA_DATA *pArea;
   bool fWorld;

   if( !argument || argument[0] != '\0' )
   {
      if( !strcmp( argument, "all" ) )
      {
         set_pager_color( AT_PLAIN, ch );
         send_to_pager( header_string1, ch );
         send_to_pager( header_string2, ch );
         for( pArea = first_area; pArea; pArea = pArea->next )
         {
            pager_printf( ch, print_string, pArea->author, pArea->name, pArea->resetmsg );
         }
         return;
      }
      else
      {
         fWorld = TRUE;
      }
   }

   set_pager_color( AT_PLAIN, ch );
   send_to_pager( header_string1, ch );
   send_to_pager( header_string2, ch );

   for( pArea = first_area_name; pArea; pArea = pArea->next_sort_name )
   {
      if( !fWorld || !str_cmp( argument, pArea->resetmsg ) )
      {
         pager_printf( ch, print_string, pArea->author, pArea->name, pArea->resetmsg );
      }
   }
   return;
}

void do_afk( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( xIS_SET( ch->act, PLR_AFK ) )
   {
      xREMOVE_BIT( ch->act, PLR_AFK );
      send_to_char( "You are no longer idle.\n\r", ch );
      act( AT_GREY, "$n is no longer idle.", ch, NULL, NULL, TO_CANSEE );
   }
   else
   {
      xSET_BIT( ch->act, PLR_AFK );
      send_to_char( "You are idling.\n\r", ch );
      act( AT_GREY, "$n is idling.", ch, NULL, NULL, TO_CANSEE );
      return;
   }

}

void do_whois( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   CLAN_DATA *clan;
   CLAN_MEMBER *chm;
   DESCRIPTOR_DATA *d;

   buf[0] = '\0';

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      send_to_pager( "You must input the name of an online character.\n\r", ch );
      return;
   }

   for( d = first_descriptor; d; d = d->next )
   {
      if( !d->character )
         continue;
      if( !str_prefix( argument, d->character->name ) )
         break;
   }
   if( !d )
   {
      send_to_char( "No such person is online.\n\r", ch );
      return;
   }
   victim = d->character;

   set_pager_color( AT_GREY, ch );
   pager_printf( ch, "\n\r%s %s\n\r %s is a %s %s %s, %d years of age, with %d hours of playing time.\n\r",
                 victim->name,
                 victim->pcdata->family ? victim->pcdata->family : "",
                 victim->sex == SEX_MALE ? "He" :
                 victim->sex == SEX_FEMALE ? "She" :
                 victim->sex == SEX_HERMAPH ? "Shi" : "It",
                 victim->sex == SEX_MALE ? "male" :
                 victim->sex == SEX_FEMALE ? "female" :
                 victim->sex == SEX_HERMAPH ? "hermaphrodite" : "neuter",
                 victim->pcdata->type ? victim->pcdata->type : "normal",
                 victim->species,
                 victim->pcdata->age_adjust, ( ( victim->played + ( current_time - victim->logon ) ) / 3600 ) );

   if( victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0' )
      pager_printf( ch, " %s homepage can be found at %s\n\r",
                    victim->sex == SEX_MALE ? "His" :
                    victim->sex == SEX_FEMALE ? "Her" :
                    victim->sex == SEX_HERMAPH ? "Hir" : "Its", show_tilde( victim->pcdata->homepage ) );

   if( xIS_SET( victim->act, PLR_BOUNTY ) )
   {
      pager_printf( ch, " %s has a total bounty of %d coins.\r\n",
                    victim->sex == SEX_MALE ? "He" :
                    victim->sex == SEX_FEMALE ? "She" : victim->sex == SEX_HERMAPH ? "Shi" : "It", victim->pcdata->bounty );
   }

   pager_printf( ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\n\r",
                 victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );
   if( victim->pcdata->pkills || victim->pcdata->pdeaths )
      pager_printf( ch, "%s has won %d spars, and lost %d.\n\r",
                    victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );
   if( victim->pcdata->rkills || victim->pcdata->rdeaths )
      pager_printf( ch, "%s has killed %d players, and been killed by a player %d times.\n\r",
                    victim->name, victim->pcdata->rkills, victim->pcdata->rdeaths );

   for( clan = first_clan; clan; clan = clan->next )
   {
      if( ( chm = get_member( victim->name, clan ) ) == NULL )
         continue;
      if( ( !xIS_SET( chm->flags, CL_SECRET ) && clan->ext_sec <= 3 ) || IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
         pager_printf( ch, "%s&w of %s&w.\n\r", chm->title, clan->title );
   }

   if( victim->pcdata->bio && victim->pcdata->bio[0] != '\0' )
      pager_printf( ch, " %s's personal bio:\n\r%s", victim->name, victim->pcdata->bio );
   else
      pager_printf( ch, " %s has yet to create a bio.\n\r", victim->name );

   pager_printf( ch, "---------\n\r", ch );

   if( victim->first_abit )
   {
      BIT_DATA *bit;

      for( bit = victim->first_abit; bit; bit = bit->next )
      {
         if( strlen( bit->desc ) > 3 )
            pager_printf( ch, "%s\r\n", bit->desc );
      }
   }

   if( victim->pcdata->first_qbit )
   {
      BIT_DATA *bit;

      for( bit = victim->pcdata->first_qbit; bit; bit = bit->next )
      {
         if( strlen( bit->desc ) > 3 )
            pager_printf( ch, "%s\r\n", bit->desc );
      }
   }

   if( IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
   {
      send_to_pager( "-------------------\n\r", ch );

      pager_printf( ch, "%s is %shelled at the moment.\n\r",
                    victim->name, ( victim->pcdata->release_date == 0 ) ? "not " : "" );

      if( victim->pcdata->release_date != 0 )
         pager_printf( ch, "%s was helled by %s, and will be released on %24.24s.\n\r",
                       victim->sex == SEX_MALE ? "He" :
                       victim->sex == SEX_FEMALE ? "She" : "It",
                       victim->pcdata->helled_by, ctime( &victim->pcdata->release_date ) );

      if( xIS_SET( victim->act, PLR_NO_CHANNEL ) || xIS_SET( victim->act, PLR_NO_EMOTE )
          || xIS_SET( victim->act, PLR_NO_TELL ) || xIS_SET( victim->act, PLR_THIEF ) || xIS_SET( victim->act, PLR_KILLER ) )
      {
         sprintf( buf2, "This player has the following flags set:" );
         if( xIS_SET( victim->act, PLR_NO_CHANNEL ) )
            strcat( buf2, " nochannel" );
         if( xIS_SET( victim->act, PLR_NO_EMOTE ) )
            strcat( buf2, " noemote" );
         if( xIS_SET( victim->act, PLR_NO_TELL ) )
            strcat( buf2, " notell" );
         if( xIS_SET( victim->act, PLR_THIEF ) )
            strcat( buf2, " thief" );
         if( xIS_SET( victim->act, PLR_KILLER ) )
            strcat( buf2, " killer" );
         strcat( buf2, ".\n\r" );
         send_to_pager( buf2, ch );
      }
      if( victim->desc && victim->desc->host[0] != '\0' )   /* added by Gorog */
      {
         sprintf( buf2, "%s's IP info: %s ", victim->name, victim->desc->host );
         strcat( buf2, victim->desc->user );
         strcat( buf2, "\n\r" );
         send_to_pager( buf2, ch );
      }
   }
}

void do_pager( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
      return;
   set_char_color( AT_NOTE, ch );
   argument = one_argument( argument, arg );
   if( !*arg )
   {
      if( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
      {
         send_to_char( "Pager disabled.\n\r", ch );
         do_config( ch, "-pager" );
      }
      else
      {
         ch_printf( ch, "Pager is now enabled at %d lines.\n\r", ch->pcdata->pagerlen );
         do_config( ch, "+pager" );
      }
      return;
   }
   if( !is_number( arg ) )
   {
      send_to_char( "Set page pausing to how many lines?\n\r", ch );
      return;
   }
   ch->pcdata->pagerlen = atoi( arg );
   if( ch->pcdata->pagerlen < 5 )
      ch->pcdata->pagerlen = 5;
   ch_printf( ch, "Page pausing set to %d lines.\n\r", ch->pcdata->pagerlen );
   return;
}

char *get_talent_rank( CHAR_DATA * ch )
{
   int max = 0;
   int i;
   if( ch->pcdata->inborn > -1 )
   {
      if( ch->curr_talent[ch->pcdata->inborn] < 10 )
         return capitalize( ch->species );
      return talent_rank[ch->pcdata->inborn][( int )( UMIN( ch->curr_talent[ch->pcdata->inborn], 120 ) - 10 ) / 10];
   }
   else
   {
      for( i = 0; i < MAX_DEITY; i++ )
      {
         if( ch->curr_talent[i] >= ch->curr_talent[max] )
            max = i;
      }
      if( ch->curr_talent[max] < 10 )
         return capitalize( ch->species );
      return talent_rank[max][( int )( UMIN( ch->curr_talent[max], 120 ) - 10 ) / 10];
   }
}

/* RANDOM BOOKS */


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )				\
				if ( !str_cmp( word, literal ) )	\
				{						\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;					\
				}						\

void add_book( BOOK_DATA * book )
{
   BOOK_DATA *temp_book;

   if( !book && !book->name )
      return;

   book->next = NULL;
   book->prev = NULL;
   temp_book = NULL;
   for( temp_book = first_book; temp_book; temp_book = temp_book->next )
   {
      if( strcasecmp( book->name, temp_book->name ) < 0 )
      {
         INSERT( book, temp_book, first_book, next, prev );
         break;
      }
   }
   if( !temp_book )
   {
      LINK( book, first_book, last_book, next, prev );
   }
}

void delete_book( BOOK_DATA * book )
{
   UNLINK( book, first_book, last_book, next, prev );
   STRFREE( book->name );
   STRFREE( book->title );
   STRFREE( book->text );
   DISPOSE( book );
}

void save_books(  )
{
   FILE *fpout;
   BOOK_DATA *book;

   if( ( fpout = fopen( BOOK_FILE, "w" ) ) == NULL )
   {
      perror( BOOK_FILE );
      return;
   }

   for( book = first_book; book; book = book->next )
   {
      fprintf( fpout, "\n#BOOK\n" );
      fprintf( fpout, "Name     %s~\n", book->name );
      fprintf( fpout, "Title     %s~\n", book->title );
      fprintf( fpout, "Text     %s~\n", book->text );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

void load_books( void )
{
   FILE *fp;
   BOOK_DATA *book;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( BOOK_FILE, "r" ) ) == NULL )
   {
      bug( "Load_books: no books found!", 0 );
      return;
   }

   first_book = NULL;
   book = NULL;

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
            if( !str_cmp( "#BOOK", word ) )
            {
               if( book )
                  add_book( book );
               CREATE( book, BOOK_DATA, 1 );
               book->title = NULL;
               book->text = NULL;
               book->name = NULL;
               continue;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               add_book( book );
               done = TRUE;
               continue;
            }
         case 'N':
            KEY( "Name", book->name, fread_string( fp ) );
            break;
         case 'T':
            KEY( "Text", book->text, fread_string( fp ) );
            KEY( "Title", book->title, fread_string( fp ) );
            break;
         default:
            bug( "Unknown book keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void do_set_book( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   BOOK_DATA *book;
   BOOK_DATA *temp_book;
   int i, n;
   bool found;

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_HELP_EDIT:
         if( ( book = ch->dest_buf ) == NULL )
         {
            bug( "set_help: sub_help_edit: NULL ch->dest_buf", 0 );
            stop_editing( ch );
            return;
         }

         STRFREE( book->text );
         book->text = copy_buffer( ch );

         stop_editing( ch );
         return;
      case SUB_OBJ_EXTRA:
         if( ( book = ch->dest_buf ) == NULL )
         {
            bug( "set_help: sub_help_edit: NULL ch->dest_buf", 0 );
            stop_editing( ch );
            return;
         }

         STRFREE( book->title );
         book->title = copy_buffer( ch );

         stop_editing( ch );
         return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg1, "add" ) )
   {
      if( str_cmp( arg2, "book" ) )
      {
         send_to_char( "Syntax: setbook add book <title>\n\r", ch );
         return;
      }
      if( !argument )
      {
         send_to_char( "Name the book what?\n\r", ch );
         return;
      }
      CREATE( book, BOOK_DATA, 1 );
      book->name = STRALLOC( argument );
      add_book( book );
      i = 0;
      for( temp_book = first_book; temp_book; temp_book = temp_book->next )
      {
         i++;
         if( temp_book == book )
            break;
      }
      sprintf( buf, "Book %s created - number %d.\n\r", book->name, i );
      send_to_char( buf, ch );
      log_string( buf );
      return;
   }
   else if( !str_cmp( arg1, "list" ) )
   {
      if( !first_book )
      {
         send_to_char( "There are no books.\n\r", ch );
         return;
      }
      i = 0;
      for( book = first_book; book; book = book->next )
      {
         i++;
         ch_printf( ch, "&C%d&c: %s\n\r", i, book->name );
      }
      return;
   }
   else if( !str_cmp( arg1, "count" ) )
   {
      if( !first_book )
      {
         send_to_char( "There are no books.\n\r", ch );
         return;
      }
      i = 0;
      for( book = first_book; book; book = book->next )
         i++;
      ch_printf( ch, "&cThere are &C%d &cbooks.\n\r", i );
      return;
   }
   else if( !str_cmp( arg1, "find" ) )
   {
      found = FALSE;
      i = 0;
      for( book = first_book; book; book = book->next )
      {
         i++;
         if( is_name( arg2, book->name ) || is_name( argument, book->name ) )
         {
            ch_printf( ch, "&C%d&c: %s\n\r", i, book->name );
            found = TRUE;
         }
      }
      if( !found )
         send_to_char( "No matching books found.\n\r", ch );
      return;
   }
   else if( !str_cmp( arg1, "save" ) )
   {
      save_books(  );
      send_to_char( "Books saved.\n\r", ch );
      return;
   }
   else if( !str_cmp( arg1, "load" ) )
   {
      load_books(  );
      send_to_char( "Books loaded.\n\r", ch );
      return;
   }
   if( arg2 && arg2[0] != '\0' )
   {
      i = atoi( arg2 );
      n = 0;
      for( book = first_book; book; book = book->next )
      {
         n++;
         if( n == i )
            break;
      }

      if( !book )
      {
         ch_printf( ch, "&cBook &C%d &cnot found.\n\r", i );
         return;
      }
      if( !str_cmp( arg1, "edit" ) )
      {
         ch->substate = SUB_HELP_EDIT;
         ch->dest_buf = book;
         start_editing( ch, book->text );
         return;
      }
      if( !str_cmp( arg1, "show" ) )
      {
         ch_printf( ch, "&C%d: %s\n\r", i, book->name );
         ch_printf( ch, "&C&c%s", book->title );
         ch_printf( ch, "&z&W%s\n\r", book->text );
         return;
      }
      if( !str_cmp( arg1, "name" ) )
      {
         UNLINK( book, first_book, last_book, next, prev );
         STRFREE( book->name );
         book->name = STRALLOC( argument );
         add_book( book );
         i = 0;
         for( temp_book = first_book; temp_book; temp_book = temp_book->next )
         {
            i++;
            if( temp_book == book )
               break;
         }
         ch_printf( ch, "&cBook name changed to &C%s&c, number is now %d.\n\r", book->name, i );
         return;
      }
      if( !str_cmp( arg1, "title" ) )
      {
         ch->substate = SUB_OBJ_EXTRA;
         ch->dest_buf = book;
         start_editing( ch, book->title );
         return;
      }
      if( !str_cmp( arg1, "delete" ) )
      {
         delete_book( book );
         ch_printf( ch, "&cBook &C%d &cdeleted.\n\r", i );
         return;
      }
   }
   send_to_char( "\n\rSyntax: setbook <add|edit|delete|text|title> <number>", ch );
   send_to_char( "\n\r        setbook <save|list>\n\r", ch );
}

/* NEW SERRIYAN DREAMS HELP FILE SYSTEM */

/* Compares two strings and returns how many letters match */
int best_match( const char *astr, const char *bstr )
{
   int match, a, b;

   if( !astr )
      return 0;
   if( !bstr )
      return 0;

   match = 0;

   for( a = 0; astr[a] != '\0'; a++ )
   {
      for( b = a; bstr[b] != '\0'; b++ )
      {
         if( astr[a] == bstr[b] )
         {
            match++;
            if( a == b )
               match++;
            break;
         }
      }
   }
   return match;
}

/* Add a help file to the help hash table */
void add_new_help( NEW_HELP_DATA * help )
{
   int hash, x;
   NEW_HELP_DATA *tmp;

   if( !help )
      return;
   if( !help->name )
      return;

   for( x = 0; help->name[x] != '\0'; x++ )
      help->name[x] = LOWER( help->name[x] );

   hash = help->name[0] % 126;

   if( ( tmp = help_hash[hash] ) == NULL )
   {
      help->next = help_hash[hash];
      help_hash[hash] = help;
      return;
   }

   for( ; tmp; tmp = tmp->next )
      if( !tmp->next )
      {
         tmp->next = help;
         help->next = NULL;
      }
   top_help++;
   return;
}

void free_new_help( NEW_HELP_DATA * help )
{
   STRFREE( help->name );
   STRFREE( help->text );
   DISPOSE( help );
}

void unlink_new_help( NEW_HELP_DATA * help )
{
   NEW_HELP_DATA *tmp, *tmp_next;
   int hash;

   if( !help )
      return;

   hash = help->name[0] % 126;

   if( help == ( tmp = help_hash[hash] ) )
   {
      help_hash[hash] = tmp->next;
      return;
   }
   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;
      if( help == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
   top_help--;
}

void save_new_helps(  )
{
   FILE *fpout;
   NEW_HELP_DATA *help;
   int x;

   if( ( fpout = fopen( HELP_FILE, "w" ) ) == NULL )
   {
      perror( HELP_FILE );
      return;
   }

   for( x = 0; x < 126; x++ )
   {
      for( help = help_hash[x]; help; help = help->next )
      {
         if( !help->name || help->name[0] == '\0' )
            continue;
         fprintf( fpout, "\n#HELP\n" );
         fprintf( fpout, "Name     %s~\n", help->name );
         fprintf( fpout, "Text     %s~\n", help->text );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

void load_new_helps( void )
{
   FILE *fp;
   NEW_HELP_DATA *help;
   char *word;
   bool done = FALSE;
   bool fMatch = FALSE;

   if( ( fp = fopen( HELP_FILE, "r" ) ) == NULL )
   {
      bug( "Load_helps: no helps found!", 0 );
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
            if( !str_cmp( "#HELP", word ) )
            {
               CREATE( help, NEW_HELP_DATA, 1 );
               help->name = NULL;
               help->text = NULL;
               continue;
            }
            else if( !str_cmp( word, "#END" ) )
            {
               done = TRUE;
               continue;
            }
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !help->name )
               {
                  free_new_help( help );
                  return;
               }
               if( !str_cmp( help->name, "greeting" ) )
                  help_greeting = help->text;
               add_new_help( help );
            }
            break;
         case 'N':
            KEY( "Name", help->name, fread_string( fp ) );
            break;
         case 'T':
            KEY( "Text", help->text, fread_string( fp ) );
            break;
         default:
            bug( "Unknown help keyword: %s", word );
      }
   }
   fclose( fp );
   return;
}

void do_set_help( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   NEW_HELP_DATA *help;
   int hash;

   switch ( ch->substate )
   {
      default:
         break;
      case SUB_HELP_EDIT:
         if( ( help = ch->dest_buf ) == NULL )
         {
            bug( "set_help: sub_help_edit: NULL ch->dest_buf", 0 );
            stop_editing( ch );
            return;
         }

         if( help_greeting == help->text )
            help_greeting = NULL;

         STRFREE( help->text );
         help->text = copy_buffer( ch );

         if( !help_greeting )
            help_greeting = help->text;

         stop_editing( ch );
         return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg1, "add" ) )
   {
      if( arg2 && arg2[0] != '\0' )
      {
         CREATE( help, NEW_HELP_DATA, 1 );
         help->name = STRALLOC( arg2 );
         add_new_help( help );
         sprintf( buf, "Help for %s created.\n\r", arg2 );
         send_to_char( buf, ch );
         log_string( buf );
         return;
      }
   }
   else if( !str_cmp( arg1, "save" ) )
   {
      save_new_helps(  );
      send_to_char( "Help files saved.\n\r", ch );
      return;
   }
   else if( !str_cmp( arg1, "load" ) )
   {
      load_new_helps(  );
      send_to_char( "Help files loaded.\n\r", ch );
      return;
   }
   else if( !str_cmp( arg1, "convert" ) )
   {
      HELP_DATA *old_help;

      for( old_help = first_help; old_help; old_help = old_help->next )
      {
         CREATE( help, NEW_HELP_DATA, 1 );
         help->name = STRALLOC( old_help->keyword );
         help->text = STRALLOC( old_help->text );
         add_new_help( help );
      }
      send_to_char( "All help files converted successfully.\n\rPlease double-check to make sure everything went properly.",
                    ch );
      return;
   }
   if( arg2 && arg2[0] != '\0' )
   {
      hash = LOWER( arg2[0] ) % 126;

      for( help = help_hash[hash]; help; help = help->next )
         if( !str_cmp( arg2, help->name ) )
            break;

      if( !help )
      {
         ch_printf( ch, "&cNo help found for &C%s.\n\r", arg2 );
         return;
      }
      if( !str_cmp( arg1, "edit" ) )
      {
         ch->substate = SUB_HELP_EDIT;
         ch->dest_buf = help;
         start_editing( ch, help->text );
         return;
      }
      if( !str_cmp( arg1, "text" ) )
      {
         sprintf( arg2, "%s\n\r", argument );
         STRFREE( help->text );
         help->text = STRALLOC( arg2 );
         ch_printf( ch, "&cHelp text for &C%s &cchanged.\n\r", arg2 );
         return;
      }
      if( !str_cmp( arg1, "append" ) )
      {
         sprintf( arg2, "%s%s\n\r", help->text, argument );
         STRFREE( help->text );
         help->text = STRALLOC( arg2 );
         ch_printf( ch, "&CHelp text for &C%s &cappended.\n\r", help->name );
         return;
      }
      if( !str_cmp( arg1, "delete" ) )
      {
         unlink_new_help( help );
         free_new_help( help );
         ch_printf( ch, "&cHelp for &C%s &cdeleted.\n\r", arg2 );
         return;
      }
      if( !str_cmp( arg1, "rename" ) )
      {
         if( !argument || argument[0] == '\0' )
         {
            send_to_char( "Rename it to what?\n\r", ch );
            return;
         }
         unlink_new_help( help );
         STRFREE( help->name );
         help->name = STRALLOC( argument );
         add_new_help( help );
         ch_printf( ch, "&cHelp for &C%s &crenamed to &C%s.\n\r", arg2, argument );
         return;
      }
   }
   send_to_char( "\n\rSyntax: sethelp <add|edit|rename|delete|text|append> <name>", ch );
   send_to_char( "\n\r        sethelp save\n\r", ch );
}

/* Searches for a help file, and displays the best match */
void do_new_help( CHAR_DATA * ch, char *argument )
{
   int hash, match, i;
   NEW_HELP_DATA *help, *best;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "&cAll help files:&C\n\r", ch );
      for( hash = 0; hash < 126; hash++ )
         for( help = help_hash[hash]; help; help = help->next )
         {
            ch_printf( ch, "%-15s", help->name );
            if( ++i % 5 == 0 )
               send_to_char( "\n\r", ch );
         }
      send_to_char( "\n\r", ch );
      return;
   }

   if( argument[1] == '\0' )
   {
      ch_printf( ch, "&cHelp files starting with &C%c:\n\r", argument[0] );
      for( help = help_hash[LOWER( argument[0] ) % 126]; help; help = help->next )
      {
         ch_printf( ch, "%-15s", help->name );
         if( ++i % 5 == 0 )
            send_to_char( "\n\r", ch );
      }
      send_to_char( "\n\r", ch );
      return;
   }

   if( !IS_NPC( ch ) && !str_cmp( argument, "newbie" ) )
      SET_BIT( ch->pcdata->flags, PCFLAG_HELPSTART );

   hash = LOWER( argument[0] ) % 126;

   match = i = 0;
   help = best = NULL;
   for( help = help_hash[hash]; help; help = help->next )
   {
      i = best_match( argument, help->name );
      if( i > match )
      {
         match = i;
         best = help;
         if( i >= strlen( argument ) )
            ch_printf( ch, "&cPossible match: &C%s&c, with match quality of &C%d.\n\r", help->name, i );
      }
      else if( i >= strlen( argument ) * 1.5 )
      {
         ch_printf( ch, "&cPossible match: &C%s&c, with match quality of &C%d.\n\r", help->name, i );
      }
   }

   if( !best )
   {
      ch_printf( ch, "&cUnable to find help for &C%s.\n\r", argument );
      return;
   }

   ch_printf( ch, "&cBest match: &C%s&c, with match quality of &C%d.\n\r", best->name, match );
   ch_printf( ch, "&z&W%s\n\r", best->text );
}

void do_story( CHAR_DATA * ch, char *argument )
{
   do_new_help( ch, "story" );
}

void do_news( CHAR_DATA * ch, char *argument )
{
   do_new_help( ch, "news_latest" );
}
