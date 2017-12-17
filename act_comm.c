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
 *			   Player communication module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 *  Externals
 */
void send_obj_page_to_char( CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page );
void send_room_page_to_char( CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page );
void send_page_to_char( CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page );
void send_control_page_to_char( CHAR_DATA * ch, char page );
char *act_string( const char *format, CHAR_DATA * to, CHAR_DATA * ch, const void *arg1, const void *arg2, int flags );

/* From interp.c */
bool check_social( CHAR_DATA * ch, char *command, char *argument );

/* from clan.c */
CLAN_DATA *get_clan( CHAR_DATA * ch, char *argument );
CLAN_MEMBER *get_member( char *name, CLAN_DATA * clan );

/* from act_wiz.c */
int get_permit_flag( char *flag );

/*
 * Local functions.
 */
char *scramble args( ( const char *argument, int modifier ) );
char *drunk_speech args( ( const char *argument, CHAR_DATA * ch ) );

void drop_artifacts( CHAR_DATA * ch, OBJ_DATA * obj );   /* Scion */

CHANNEL_MEMBER *get_chatter( CHAR_DATA * ch, CHANNEL_DATA * chan )
{
   CHANNEL_MEMBER *chm;

   for( chm = chan->first_member; chm; chm = chm->next )
   {
      if( chm->ch == ch )
         return chm;
   }

   return NULL;
}

bool remove_from_chan( CHAR_DATA * ch, CHANNEL_DATA * chan )
{
   CHANNEL_MEMBER *chm;

   if( ( chm = get_chatter( ch, chan ) ) == NULL )
      return FALSE;

   UNLINK( chm, chan->first_member, chan->last_member, next, prev );
   DISPOSE( chm );

   return TRUE;
}

bool room_is_silent( CHAR_DATA * ch )
{
   ROOM_INDEX_DATA *room = ch->in_room;

   if( IS_SET( room->room_flags, ROOM_SILENCE ) )
      return TRUE;

   return FALSE;
}

void update_hiscores( CHAR_DATA * ch )
{
   adjust_hiscore( "hours", ch, ( ( ch->played + ( current_time - ch->logon ) ) / 3600 ) );
   if( IS_SET( ch->pcdata->flags, PCFLAG_ANONYMOUS ) )
      return;
   if( IS_SET( ch->pcdata->flags, PCFLAG_CHEAT ) )
      return;
   adjust_hiscore( "mkill", ch, ch->pcdata->mkills );
   adjust_hiscore( "mdeath", ch, ch->pcdata->mdeaths );
   adjust_hiscore( "fire", ch, ch->curr_talent[TAL_FIRE] + ( ch->pcdata->inborn == TAL_FIRE ) );
   adjust_hiscore( "wind", ch, ch->curr_talent[TAL_WIND] + ( ch->pcdata->inborn == TAL_WIND ) );
   adjust_hiscore( "earth", ch, ch->curr_talent[TAL_EARTH] + ( ch->pcdata->inborn == TAL_EARTH ) );
   adjust_hiscore( "water", ch, ch->curr_talent[TAL_WATER] + ( ch->pcdata->inborn == TAL_WATER ) );
   adjust_hiscore( "frost", ch, ch->curr_talent[TAL_FROST] + ( ch->pcdata->inborn == TAL_FROST ) );
   adjust_hiscore( "lightning", ch, ch->curr_talent[TAL_LIGHTNING] + ( ch->pcdata->inborn == TAL_LIGHTNING ) );
   adjust_hiscore( "dream", ch, ch->curr_talent[TAL_DREAM] + ( ch->pcdata->inborn == TAL_DREAM ) );
   adjust_hiscore( "speech", ch, ch->curr_talent[TAL_SPEECH] + ( ch->pcdata->inborn == TAL_SPEECH ) );
   adjust_hiscore( "healing", ch, ch->curr_talent[TAL_HEALING] + ( ch->pcdata->inborn == TAL_HEALING ) );
   adjust_hiscore( "death", ch, ch->curr_talent[TAL_DEATH] + ( ch->pcdata->inborn == TAL_DEATH ) );
   adjust_hiscore( "change", ch, ch->curr_talent[TAL_CHANGE] + ( ch->pcdata->inborn == TAL_CHANGE ) );
   adjust_hiscore( "time", ch, ch->curr_talent[TAL_TIME] + ( ch->pcdata->inborn == TAL_TIME ) );
   adjust_hiscore( "motion", ch, ch->curr_talent[TAL_MOTION] + ( ch->pcdata->inborn == TAL_MOTION ) );
   adjust_hiscore( "mind", ch, ch->curr_talent[TAL_MIND] + ( ch->pcdata->inborn == TAL_MIND ) );
   adjust_hiscore( "illusion", ch, ch->curr_talent[TAL_ILLUSION] + ( ch->pcdata->inborn == TAL_ILLUSION ) );
   adjust_hiscore( "seeking", ch, ch->curr_talent[TAL_SEEKING] + ( ch->pcdata->inborn == TAL_SEEKING ) );
   adjust_hiscore( "security", ch, ch->curr_talent[TAL_SECURITY] + ( ch->pcdata->inborn == TAL_SECURITY ) );
   adjust_hiscore( "catalysm", ch, ch->curr_talent[TAL_CATALYSM] + ( ch->pcdata->inborn == TAL_SECURITY ) );
   adjust_hiscore( "void", ch, ch->curr_talent[TAL_VOID] + ( ch->pcdata->inborn == TAL_VOID ) );
}

/* Text scrambler -- Altrag */
char *scramble( const char *argument, int modifier )
{
   static char arg[MAX_INPUT_LENGTH];
   sh_int position;
   sh_int conversion = 0;

   modifier %= number_range( 80, 300 );   /* Bitvectors get way too large #s */
   for( position = 0; position < MAX_INPUT_LENGTH; position++ )
   {
      if( argument[position] == '\0' )
      {
         arg[position] = '\0';
         return arg;
      }
      else if( argument[position] >= 'A' && argument[position] <= 'Z' )
      {
         conversion = -conversion + position - modifier + argument[position] - 'A';
         conversion = number_range( conversion - 5, conversion + 5 );
         while( conversion > 25 )
            conversion -= 26;
         while( conversion < 0 )
            conversion += 26;
         arg[position] = conversion + 'A';
      }
      else if( argument[position] >= 'a' && argument[position] <= 'z' )
      {
         conversion = -conversion + position - modifier + argument[position] - 'a';
         conversion = number_range( conversion - 5, conversion + 5 );
         while( conversion > 25 )
            conversion -= 26;
         while( conversion < 0 )
            conversion += 26;
         arg[position] = conversion + 'a';
      }
      else if( argument[position] >= '0' && argument[position] <= '9' )
      {
         conversion = -conversion + position - modifier + argument[position] - '0';
         conversion = number_range( conversion - 2, conversion + 2 );
         while( conversion > 9 )
            conversion -= 10;
         while( conversion < 0 )
            conversion += 10;
         arg[position] = conversion + '0';
      }
      else
         arg[position] = argument[position];
   }
   arg[position] = '\0';
   return arg;
}

char *drunk_speech( const char *argument, CHAR_DATA * ch )
{
   const char *arg = argument;
   static char buf[MAX_INPUT_LENGTH * 2];
   char buf1[MAX_INPUT_LENGTH * 2];
   sh_int drunk;
   char *txt;
   char *txt1;

   if( IS_NPC( ch ) || !ch->pcdata )
      return ( char * )argument;

   drunk = ch->pcdata->condition[COND_DRUNK] / 3;

   if( drunk <= 0 && !IS_AFFECTED( ch, AFF_COLDBLOOD ) && !IS_AFFECTED( ch, AFF_FELINE ) )
      return ( char * )argument;

   buf[0] = '\0';
   buf1[0] = '\0';

   if( !argument )
   {
      bug( "Drunk_speech: NULL argument", 0 );
      return "";
   }

   /*
    * if ( *arg == '\0' )
    * return (char *) argument;
    */

   txt = buf;
   txt1 = buf1;

   while( *arg != '\0' )
   {
      /*
       * make lizzies hiss their s's -keo 
       */
      if( IS_AFFECTED( ch, AFF_COLDBLOOD ) )
      {
         if( toupper( (int)*arg ) == 'S' )
         {
            sh_int hissn = number_range( 0, 3 );
            sh_int currhiss = 0;

            while( currhiss < hissn )
               *txt++ = *arg, currhiss++;
         }
      }

      /*
       * and kitties slur their r's -keo 
       */
      if( IS_AFFECTED( ch, AFF_FELINE ) )
      {
         if( toupper( (int)*arg ) == 'R' )
         {
            sh_int slurn = number_range( 0, 3 );
            sh_int currslur = 0;

            while( currslur < slurn )
               *txt++ = *arg, currslur++;
         }
      }

      if( toupper( (int)*arg ) == 'T' )
      {
         if( number_percent(  ) < ( drunk * 2 ) )  /* add 'h' after an 'T' */
         {
            *txt++ = *arg;
            *txt++ = 'h';
         }
         else
            *txt++ = *arg;
      }
      else if( toupper( (int)*arg ) == 'X' )
      {
         if( number_percent(  ) < ( drunk * 2 / 2 ) )
         {
            *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
         }
         else
            *txt++ = *arg;
      }
      else if( number_percent(  ) < ( drunk * 2 / 5 ) )  /* slurred letters */
      {
         sh_int slurn = number_range( 1, 2 );
         sh_int currslur = 0;

         while( currslur < slurn )
            *txt++ = *arg, currslur++;
      }
      else
         *txt++ = *arg;

      arg++;
   };

   *txt = '\0';

   txt = buf;

   while( *txt != '\0' )   /* Let's mess with the string's caps */
   {
      if( number_percent(  ) < ( 2 * drunk / 2.5 ) )
      {
         if( isupper( (int)*txt ) )
            *txt1 = tolower( (int)*txt );
         else if( islower( (int)*txt ) )
            *txt1 = toupper( (int)*txt );
         else
            *txt1 = *txt;
      }
      else
         *txt1 = *txt;

      txt1++, txt++;
   };

   *txt1 = '\0';
   txt1 = buf1;
   txt = buf;

   while( *txt1 != '\0' )  /* Let's make them stutter */
   {
      if( *txt1 == ' ' )   /* If there's a space, then there's gotta be a */
      {  /* along there somewhere soon */

         while( *txt1 == ' ' )   /* Don't stutter on spaces */
            *txt++ = *txt1++;

         if( ( number_percent(  ) < ( 2 * drunk / 4 ) ) && *txt1 != '\0' )
         {
            sh_int offset = number_range( 0, 2 );
            sh_int pos = 0;

            while( *txt1 != '\0' && pos < offset )
               *txt++ = *txt1++, pos++;

            if( *txt1 == ' ' )   /* Make sure not to stutter a space after */
            {  /* the initial offset into the word */
               *txt++ = *txt1++;
               continue;
            }

            pos = 0;
            offset = number_range( 2, 4 );
            while( *txt1 != '\0' && pos < offset )
            {
               *txt++ = *txt1;
               pos++;
               if( *txt1 == ' ' || pos == offset ) /* Make sure we don't stick */
               {  /* A hyphen right before a space */
                  txt1--;
                  break;
               }
               *txt++ = '-';
            }
            if( *txt1 != '\0' )
               txt1++;
         }
      }
      else
         *txt++ = *txt1++;
   }

   *txt = '\0';

   return buf;
}

void to_channel( const char *argument, const char *verb, sh_int permit )
{
   char buf[MAX_STRING_LENGTH];
   DESCRIPTOR_DATA *d;

   if( !first_descriptor || argument[0] == '\0' )
      return;

   sprintf( buf, "%s: %s\r\n", verb, argument );

   for( d = first_descriptor; d; d = d->next )
   {
      CHAR_DATA *och;
      CHAR_DATA *vch;

      och = d->original ? d->original : d->character;
      vch = d->character;

      if( !och || !vch )
         continue;
      if( !IS_SET( vch->pcdata->permissions, permit ) )
         continue;

      if( d->connected == CON_PLAYING )
      {
         set_char_color( AT_LOG, vch );
         send_to_char_color( buf, vch );
      }
   }

   return;
}


void do_say( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   EXT_BV actflags;

   if( IS_SILENT( ch ) )
   {
      send_to_char( "No sound comes out.\n\r", ch );
      return;
   }

   MOBtrigger = TRUE;

   if( argument[0] == '\0' )
   {
      send_to_char( "Say what?\n\r", ch );
      return;
   }

   if( argument[0] == '@' )
      argument = one_argument( argument + 1, arg );
   else
      arg[0] = '\0';

   actflags = ch->act;
   if( IS_NPC( ch ) )
      xREMOVE_BIT( ch->act, ACT_SECRETIVE );

   MOBtrigger = FALSE;
   if( arg[0] != '\0' )
      act( AT_SAY, "$n says $t, '$T'", ch, arg, drunk_speech( argument, ch ), TO_ROOM );
   else
      act( AT_SAY, "$n says, '$T'", ch, NULL, drunk_speech( argument, ch ), TO_ROOM );

   ch->act = actflags;
   MOBtrigger = FALSE;
   if( arg[0] != '\0' )
      act( AT_SAY, "You say $t, '$T'", ch, arg, drunk_speech( argument, ch ), TO_CHAR );
   else
      act( AT_SAY, "You say, '$T'", ch, NULL, drunk_speech( argument, ch ), TO_CHAR );

   if( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
   {
      sprintf( buf, "%s: %s", IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
      append_to_file( LOG_FILE, buf );
   }
   /*
    * Yakkov's amplify request - shogar 
    */
   if( ch->in_room && IS_SET( ch->in_room->room_flags, ROOM_AMPLIFY ) )
   {
      ROOM_INDEX_DATA *was_in_room;
      EXIT_DATA *pexit;

      sprintf( buf, "You hear '%s'", argument );
      actflags = ch->act;
      xREMOVE_BIT( ch->act, ACT_SECRETIVE );
      was_in_room = ch->in_room;
      for( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
      {
         if( pexit->to_room && pexit->to_room != was_in_room )
         {
            ch->in_room = pexit->to_room;
            MOBtrigger = FALSE;
            act( AT_CYAN, buf, ch, NULL, NULL, TO_ROOM );
         }
      }
      ch->act = actflags;
      ch->in_room = was_in_room;
   }

   /*
    * if your hiding you give yourself away -keo 
    */
   if( IS_AFFECTED( ch, AFF_HIDE ) && !IS_NPC( ch ) )
      xREMOVE_BIT( ch->pcdata->perm_aff, AFF_HIDE );

   /*
    * can't sing while talking -keo 
    */
   if( ch->singing )
      do_sing( ch, "none" );

   if( MOBtrigger == TRUE )
   {
      mprog_speech_trigger( argument, ch );
      if( char_died( ch ) )
         return;
      oprog_speech_trigger( argument, ch );
      if( char_died( ch ) )
         return;
      rprog_speech_trigger( argument, ch );
      return;
   }
}

/* out of character says, added by popular request -keo */
void do_ooc_say( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( argument[0] == '\0' )
   {
      send_to_char( "Say what?\n\r", ch );
      return;
   }

   /*
    * Yes, it shows your real name. This is intentional 
    */
   MOBtrigger = FALSE;
   ch_printf( ch, "&z&CYou say out of character '%s'\n\r", argument );
   sprintf( buf, "&z&C%s says out of character '$t'", ch->name );
   act( AT_SAY, buf, ch, argument, NULL, TO_ROOM );
}

void do_whisper( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int position;

   MOBtrigger = TRUE;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Whisper to whom what?\n\r", ch );
      return;
   }


   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "You have a nice little chat with yourself.\n\r", ch );
      return;
   }

   if( !IS_NPC( victim ) && ( victim->switched ) && !IS_AFFECTED( victim->switched, AFF_POSSESS ) )
   {
      send_to_char( "That player is switched.\n\r", ch );
      return;
   }
   else if( !IS_NPC( victim ) && ( !victim->desc ) )
   {
      send_to_char( "That player is link-dead.\n\r", ch );
      return;
   }
   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) )
   {
      send_to_char( "That player is away.\n\r", ch );
      return;
   }
   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_SILENCE ) )
      send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );

   if( victim->desc  /* make sure desc exists first  -Thoric */
       && victim->desc->connected == CON_EDITING )
   {
      act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
      return;
   }

   act( AT_WHISPER, "You whisper to $N '$t'", ch, argument, victim, TO_CHAR );
   position = victim->position;
   victim->position = POS_STANDING;
   MOBtrigger = FALSE;
   act( AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT );
   act( AT_WHISPER, "$n whispers something to $N.", ch, argument, victim, TO_NOTVICT );

   victim->position = position;
   if( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
   {
      sprintf( buf, "%s: %s (whisper to) %s.",
               IS_NPC( ch ) ? ch->short_descr : ch->name, argument, IS_NPC( victim ) ? victim->short_descr : victim->name );
      append_to_file( LOG_FILE, buf );
   }

   if( MOBtrigger == TRUE )
      mprog_speech_trigger( argument, ch );
   return;
}

/* beep snippet installed by Scion 3/30/99 */
void do_beep( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument( argument, arg );

   if( !*arg || !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "Beep who?\n\r", ch );
      return;
   }

   /*
    * NPC check added by Samson 2-15-98 
    */
   if( IS_NPC( victim ) )
   {
      send_to_char( "Beep who?\n\r", ch );
      return;
   }

   /*
    * PCFLAG_NOBEEP check added by Samson 2-15-98 
    */
   if( IS_SET( victim->pcdata->flags, PCFLAG_NOBEEP ) )
   {
      ch_printf( ch, "%s is not accepting beeps at this time.\n\r", victim->name );
      return;
   }

   if( argument[0] != '\0' )
   {
      ch_printf( victim, "&W^z%s beeps: %s^x&w\r\n", ch->name, argument );
      if( victim != ch )
         ch_printf( ch, "&WYou beep %s: %s\r\n", victim->name, argument );
   }
   else
   {
      ch_printf( victim, "&W^z%s beeps you from '%s'^x&w\a\n\r", ch->name, ch->in_room->name );
      if( victim != ch )
         ch_printf( ch, "&WYou beep %s.&w\n\r", victim->name );
   }
   return;
}

/* an attempt to reduce redundancy -keo */
void tell( CHAR_DATA * ch, CHAR_DATA * victim, char *argument )
{
   CHAR_DATA *switched_victim = NULL;
   int position;
   char buf[MAX_STRING_LENGTH];

   if( !victim )
   {
      send_to_char( "Tell who what?\n\r", ch );
      return;
   }

   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_TELL ) )
   {
      huh( ch );
      return;
   }

   if( ch == victim )
   {
      send_to_char( "You have a nice little chat with yourself.\n\r", ch );
      return;
   }

   else if( !IS_NPC( victim ) && ( victim->switched ) && IS_AFFECTED( victim->switched, AFF_POSSESS ) )
      switched_victim = victim->switched;

   else if( !IS_NPC( victim ) && ( !victim->desc ) )
   {
      send_to_char( "That player is link-dead.\n\r", ch );
      return;
   }

   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) )
   {
      send_to_char( "That player is away and may not have received your tell.\n\r", ch );
   }

   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_SILENCE ) )
      send_to_char( "That player is silenced.  They will receive your message but cannot respond.\n\r", ch );

   if( ( !IS_AWAKE( victim ) ) || ( !IS_NPC( victim ) && IS_SET( victim->in_room->room_flags, ROOM_SILENCE ) ) )
   {
      act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
      return;
   }

   if( victim->desc  /* make sure desc exists first  -Thoric */
       && victim->desc->connected == CON_EDITING )
   {
      act( AT_PLAIN, "$E is currently in a writing buffer. Try sending a beep if urgent.\n\r", ch, 0, victim, TO_CHAR );
      return;
   }

   if( switched_victim )
      victim = switched_victim;

   act( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );
   position = victim->position;
   victim->position = POS_STANDING;
   ch_printf( victim, "&R%s tells you '%s'\n\r", ch->name, argument );
   victim->position = position;
   victim->reply = ch;
   ch->retell = victim;
   if( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
   {
      sprintf( buf, "%s: %s (tell to) %s.",
               IS_NPC( ch ) ? ch->short_descr : ch->name, argument, IS_NPC( victim ) ? victim->short_descr : victim->name );
      append_to_file( LOG_FILE, buf );
   }
}

void do_tell( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   MOBtrigger = TRUE;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Tell whom what?\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL || ( IS_NPC( victim ) && victim->in_room != ch->in_room ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   tell( ch, victim, argument );

   if( MOBtrigger == TRUE )
      mprog_speech_trigger( argument, ch );
   return;
}



void do_reply( CHAR_DATA * ch, char *argument )
{
   tell( ch, ch->reply, argument );
}

void do_retell( CHAR_DATA * ch, char *argument )
{
   tell( ch, ch->retell, argument );
}

void do_emote( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char nbuf[MAX_STRING_LENGTH];
   char *plast;
   CHAR_DATA *vch;
   EXT_BV actflags;
   int i, b;
   bool fHasName = FALSE;

   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You can't show your emotions.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Emote what?\n\r", ch );
      return;
   }

   actflags = ch->act;
   if( IS_NPC( ch ) )
      xREMOVE_BIT( ch->act, ACT_SECRETIVE );
   for( plast = argument; *plast != '\0'; plast++ )
      ;
   strcpy( buf, argument );

   if( !IS_NPC( ch ) )
   {
      b = strlen( buf );
      nbuf[0] = '\0';
      for( i = 0; i < b; i++ )
      {
         if( buf[i] == '$' && buf[i + 1] == 'n' )
         {
            fHasName = TRUE;
         }
         nbuf[i] = buf[i];
         nbuf[i + 1] = '\0';
      }
   }
   if( fHasName == FALSE )
   {
      strcpy( nbuf, "$n" );
      strcat( nbuf, " " );
      strcat( nbuf, buf );
   }

   if( isalpha( (int)plast[-1] ) )
      strcat( buf, "." );
   for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
   {
      char *sbuf = nbuf;

      MOBtrigger = FALSE;
      act( AT_SOCIAL, sbuf, ch, NULL, vch, ( vch == ch ? TO_CHAR : TO_VICT ) );
   }
   ch->act = actflags;
   if( IS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
   {
      sprintf( buf, "%s %s (emote)", IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
      append_to_file( LOG_FILE, buf );
   }
   return;
}


void do_bug( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   struct tm *t = localtime( &current_time );

   set_char_color( AT_PLAIN, ch );
   if( argument[0] == '\0' )
   {
      send_to_char( "\n\rUsage:  'bug <message>'  (your location is automatically recorded)\n\r", ch );
      return;
   }
   sprintf( buf, "(%-2.2d/%-2.2d):  %s", t->tm_mon + 1, t->tm_mday, argument );
   append_file( ch, PBUG_FILE, buf );
   send_to_char( "Thanks, your bug notice has been recorded.\n\r", ch );
   return;
}

void do_idea( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   struct tm *t = localtime( &current_time );

   set_char_color( AT_PLAIN, ch );
   if( argument[0] == '\0' )
   {
      send_to_char( "\n\rUsage:  'idea <message>'\n\r", ch );
      return;
   }
   sprintf( buf, "(%-2.2d/%-2.2d):  %s", t->tm_mon + 1, t->tm_mday, argument );
   append_file( ch, IDEA_FILE, buf );
   send_to_char( "Thanks, your idea has been recorded.\n\r", ch );
   return;
}

void do_roleplay( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   struct tm *t = localtime( &current_time );

   set_char_color( AT_PLAIN, ch );
   if( argument[0] == '\0' )
   {
      send_to_char( "\n\rUsage:  'roleplay <message>'\n\r", ch );
      if( IS_SET( ch->pcdata->permissions, PERMIT_TALENT ) || IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
         send_to_char( "Usage:  'roleplay list' or 'roleplay clear now'\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "clear now" )
       && ( IS_SET( ch->pcdata->permissions, PERMIT_TALENT ) || IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) ) )
   {
      FILE *fp = fopen( RP_FILE, "w" );
      if( fp )
         fclose( fp );
      send_to_char( "RP file cleared.\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "list" )
       && ( IS_SET( ch->pcdata->permissions, PERMIT_TALENT ) || IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) ) )
   {
      send_to_char( "\n\r VNUM \n\r.......\n\r", ch );
      show_file( ch, RP_FILE );
   }
   else
   {
      sprintf( buf, "(%-2.2d/%-2.2d):  %s", t->tm_mon + 1, t->tm_mday, argument );
      append_file( ch, RP_FILE, buf );
      send_to_char( "Thanks, your roleplaying has been recorded.\n\r", ch );
      return;
   }
}

void do_typo( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_PLAIN, ch );
   if( argument[0] == '\0' )
   {
      send_to_char( "\n\rUsage:  'typo <message>'  (your location is automatically recorded)\n\r", ch );
      if( IS_SET( ch->pcdata->permissions, PERMIT_HELP ) )
         send_to_char( "Usage: 'typo clean now'\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "clear now" ) && IS_SET( ch->pcdata->permissions, PERMIT_HELP ) )
   {
      FILE *fp = fopen( TYPO_FILE, "w" );
      if( fp )
         fclose( fp );
      send_to_char( "Typo file cleared.\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "list" ) )
   {
      send_to_char( "\n\r VNUM \n\r.......\n\r", ch );
      show_file( ch, TYPO_FILE );
   }
   else
   {
      append_file( ch, TYPO_FILE, argument );
      send_to_char( "Thanks, your typo notice has been recorded.\n\r", ch );
   }
   return;
}

void do_rent( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_WHITE, ch );
   send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
   return;
}



void do_qui( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
   return;
}

void do_quit( CHAR_DATA * ch, char *argument )
{
   int x, y;
   OBJ_DATA *obj;

   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }

   if( IS_FIGHTING( ch ) )
   {
      set_char_color( AT_RED, ch );
      send_to_char( "No way! You are fighting.\n\r", ch );
      return;
   }

   if( ch->position < POS_STUNNED )
   {
      set_char_color( AT_BLOOD, ch );
      send_to_char( "You're not DEAD yet.\n\r", ch );
      return;
   }

   if( IS_SET( ch->in_room->room_flags, ROOM_NO_QUIT ) && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
   {
      send_to_char( "You cannot quit in this room.\n\r", ch );
      return;
   }

   if( get_timer( ch, TIMER_RECENTFIGHT ) )
   {
      set_char_color( AT_RED, ch );
      send_to_char( "Your adrenaline is pumping too hard to quit now!\n\r", ch );
      return;
   }

   /*
    * make sure they aren't mounted -keo 
    */
   if( IS_AFFECTED( ch, AFF_MOUNTED ) )
      throw_rider( ch );
   if( ch->mount )
      throw_rider( ch->mount );

   if( IS_SET( ch->in_room->area->flags, AFLAG_NOQUIT ) && !ch->pcdata->permissions )
   {
      char_from_room( ch );
      char_to_room( ch, get_room_index( ROOM_VNUM_NEXUS ) );
   }

   /*
    * Get 'em dismounted until we finish mount saving -- Blodkai, 4/97 
    */
   if( ch->position == POS_MOUNTED )
      do_dismount( ch, "" );

   update_hiscores( ch );
   do_channels( ch, "quiet" );
   quitting_char = ch;
   save_char_obj( ch );

   if( sysdata.save_pets && ch->pcdata->pet )
   {
      act( AT_BYE, "$N follows $S master into the Void.", ch, NULL, ch->pcdata->pet, TO_ROOM );
      extract_char( ch->pcdata->pet, TRUE );
   }

   saving_char = NULL;

   set_char_color( AT_WHITE, ch );
   ch_printf( ch,
              "Your surroundings begin to fade as a mystical swirling vortex of colors\r\nenvelops your body...  Please come back and visit %s again.\n\r\n\r",
              sysdata.mud_name );
   act( AT_SAY, "A soothing voice says, 'May your magic never falter, $n...'", ch, NULL, NULL, TO_CHAR );
   act( AT_BYE, "$n enters the Void and is gone.", ch, NULL, NULL, TO_CANSEE );
   set_char_color( AT_GREY, ch );

   sprintf( log_buf, "%s has quit in %s (%d).", ch->name, ch->in_room->name, ch->in_room->vnum );

   while( ( obj = ch->last_carrying ) != NULL )
      extract_obj( obj );

   /*
    * After extract_char the ch is no longer valid!
    */
   extract_char( ch, TRUE );
   for( x = 0; x < MAX_WEAR; x++ )
      for( y = 0; y < MAX_LAYERS; y++ )
         save_equipment[x][y] = NULL;

   log_string_plus( log_buf, LOG_COMM, 0 );
   return;
}

/* Made this a function so it could work recursively on containers.
   Make them drop artifact items. Can't leave the game with those! -- Scion */
void drop_artifacts( CHAR_DATA * ch, OBJ_DATA * obj )
{
   /*
    * Expecting a ch->last_carrying or an obj->last_content
    * ( We go BACKWARDS through the lists!) 
    */

   if( !obj )
      return;



   while( obj )
   {
      if( obj->last_content )
         drop_artifacts( ch, obj->last_content );

      if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
      {
         OBJ_DATA *tobj;

         tobj = obj;
         obj = obj->prev_content;

         if( tobj->in_obj )
            obj_from_obj( tobj );
         if( tobj->carried_by )
            obj_from_char( tobj );

         obj_to_room( tobj, ch->in_room );
         act( AT_MAGIC, "$p falls to the ground, drawn by the magic of these lands!", ch, tobj, NULL, TO_CHAR );
         act( AT_MAGIC, "$p falls from $n's hands, drawn by the magic of these lands!", ch, tobj, NULL, TO_CANSEE );
      }
      else
         obj = obj->prev_content;
   }
}

void send_rip_screen( CHAR_DATA * ch )
{
   FILE *rpfile;
   int num = 0;
   char BUFF[MAX_STRING_LENGTH * 2];

   if( ( rpfile = fopen( RIPSCREEN_FILE, "r" ) ) != NULL )
   {
      while( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
         num++;
      fclose( rpfile );
      BUFF[num] = 0;
      write_to_buffer( ch->desc, BUFF, num );
   }
}

void do_ansi( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "ANSI ON or OFF?\n\r", ch );
      return;
   }
   if( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
   {
      xSET_BIT( ch->act, PLR_ANSI );
      set_char_color( AT_WHITE + AT_BLINK, ch );
      send_to_char( "ANSI ON!!!\n\r", ch );
      return;
   }

   if( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
   {
      xREMOVE_BIT( ch->act, PLR_ANSI );
      send_to_char( "Okay... ANSI support is now off\n\r", ch );
      return;
   }
}

void do_save( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;
   WAIT_STATE( ch, 2 ); /* For big muds with save-happy players, like RoD */
   update_aris( ch );   /* update char affects and RIS */
   save_char_obj( ch );
   saving_char = NULL;
   send_to_char( "Saved...\n\r", ch );
   update_hiscores( ch );
   return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow( CHAR_DATA * ch, CHAR_DATA * victim )
{
   CHAR_DATA *tmp;

   for( tmp = victim; tmp; tmp = tmp->master )
      if( tmp == ch )
         return TRUE;
   return FALSE;
}


void do_dismiss( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;

   check_social( ch, "dismiss", argument );

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
      return;

   if( ( IS_AFFECTED( victim, AFF_CHARM ) ) && ( victim->master == ch ) )
   {
      stop_follower( victim );
      if( IS_NPC( victim ) )
      {
         stop_hating( victim );
         stop_hunting( victim );
         stop_fearing( victim );
      }
   }

}

void do_follow( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Follow whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master )
   {
      act( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
      return;
   }

   if( victim == ch )
   {
      if( !ch->master )
      {
         send_to_char( "You already follow yourself.\n\r", ch );
         return;
      }
      stop_follower( ch );
      return;
   }
   if( circle_follow( ch, victim ) )
   {
      stop_follower( victim );
      return;
   }

   if( ch->master )
      stop_follower( ch );

   add_follower( ch, victim );
   return;
}



void add_follower( CHAR_DATA * ch, CHAR_DATA * master )
{
   if( ch->master )
   {
      bug( "Add_follower: non-null master.", 0 );
      return;
   }

   ch->master = master;
   ch->leader = NULL;

   /*
    * Support for saving pets --Shaddai 
    */
   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PET ) && !IS_NPC( master ) )
      master->pcdata->pet = ch;

   if( can_see( master, ch ) )
      act( AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT );

   act( AT_ACTION, "You now follow $N.", ch, NULL, master, TO_CHAR );

   return;
}



void stop_follower( CHAR_DATA * ch )
{
   if( !ch->master )
   {
      bug( "Stop_follower: null master.", 0 );
      return;
   }

   if( IS_NPC( ch ) && !IS_NPC( ch->master ) && ch->master->pcdata->pet == ch )
      ch->master->pcdata->pet = NULL;

   if( IS_AFFECTED( ch, AFF_CHARM ) )
   {
      xREMOVE_BIT( ch->affected_by, AFF_CHARM );
      affect_strip( ch, gsn_charm_person );
   }

   if( can_see( ch->master, ch ) )
      if( !( !IS_NPC( ch->master ) ) )
         act( AT_ACTION, "$n stops following you.", ch, NULL, ch->master, TO_VICT );
   act( AT_ACTION, "You stop following $N.", ch, NULL, ch->master, TO_CHAR );

   ch->master = NULL;
   ch->leader = NULL;
   return;
}



void die_follower( CHAR_DATA * ch )
{
   CHAR_DATA *fch;

   if( ch->master )
      stop_follower( ch );

   ch->leader = NULL;
   ch->last_hit = NULL;

   for( fch = first_char; fch; fch = fch->next )
   {
      if( fch->master == ch )
         stop_follower( fch );
      if( fch->leader == ch )
         fch->leader = fch;
      if( fch->last_hit == ch )
         fch->last_hit = NULL;
      if( !IS_NPC( fch ) )
      {
         if( fch->pcdata->mindlink == ch )
            fch->pcdata->mindlink = NULL;
         if( fch->pcdata->consenting == ch )
            fch->pcdata->consenting = NULL;
         if( fch->pcdata->magiclink == ch )
            fch->pcdata->magiclink = NULL;
      }
   }
   return;
}



void do_order( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char argbuf[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   strcpy( argbuf, argument );
   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Order whom to do what?\n\r", ch );
      return;
   }

   if( IS_AFFECTED( ch, AFF_CHARM ) )
   {
      send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Aye aye, right away!\n\r", ch );
      return;
   }

   if( !IS_AFFECTED( victim, AFF_CHARM ) || victim->master != ch )
   {
      /*
       * Attempt to command 
       */
      if( ch->curr_talent[TAL_SPEECH] + ch->curr_talent[TAL_MIND] >= 120 )
      {
         use_magic( ch, TAL_SPEECH, 130 );
         if( number_range( 1, TALENT( ch, TAL_SPEECH )
                           + TALENT( ch, TAL_MIND ) + get_curr_wil( ch ) ) < get_curr_wil( victim ) )
         {
            act( AT_MAGIC, "$N shrugs off your suggestion.", ch, NULL, victim, TO_CHAR );
            act( AT_MAGIC, "You shrug off $n's suggestion that you '$t'.", ch, argument, victim, TO_VICT );
            return;
         }
      }
      else
      {
         send_to_char( "Do it yourself!\n\r", ch );
         return;
      }
   }  /* end if charmed */

   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
   {
      act( AT_MAGIC, "$N shrugs off your suggestion.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( IS_AFFECTED( victim, AFF_BERSERK ) )
   {
      send_to_char( "Your orders fail to get through their rage.\n\r", ch );
      return;
   }

   act( AT_ACTION, "$n orders you to '$t'.", ch, argument, victim, TO_VICT );
   interpret( victim, argument, TRUE );

   sprintf( log_buf, "%s: order %s.", ch->name, argbuf );
   log_string_plus( log_buf, LOG_NORMAL, 0 );
   send_to_char( "Ok.\n\r", ch );
   WAIT_STATE( ch, PULSE_VIOLENCE );
   return;
}

/*
char *itoa(int foo)
{
  static char bar[256];

  sprintf(bar,"%d",foo);
  return(bar);

}
*/

/* Overhauled 2/97 -- Blodkai
   Keelhauled 1/00 -- Scion */
void do_group( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      CHAR_DATA *gch;
      CHAR_DATA *leader;

      leader = ch->leader ? ch->leader : ch;
      set_char_color( AT_DGREEN, ch );
      ch_printf( ch, "\n\rFollowing %-12.12s [hitpnts] [ magic ] [mst] [mvs] [race]\n\r", PERS( leader, ch ) );
      for( gch = first_char; gch; gch = gch->next )
      {
         if( is_same_group( gch, ch ) )
         {
            set_char_color( AT_DGREEN, ch );
            if( IS_AFFECTED( gch, AFF_POSSESS ) || ( !IS_NPC( gch ) && IS_SET( gch->pcdata->flags, PCFLAG_ANONYMOUS ) ) )
            {  /*reveal no information */
               ch_printf( ch,
                          "&g[&G%-3d -- %3.3s&g]  &G%-12s&g %59s &G%-12s&g\n\r",
                          0,
                          IS_NPC( gch ) ? "Mob" : "Xxx",
                          capitalize( PERS( gch, ch ) ),
                          ( ( gch->protected_by && gch->protected_by->in_room == gch->in_room ) ?
                            gch->protected_by->name : "" ) );
            }
            else
            {
               set_char_color( AT_GREEN, ch );
               ch_printf( ch, "%-12.12s ", capitalize( PERS( gch, ch ) ) );
               if( gch->hit < gch->max_hit / 4 )
                  set_char_color( AT_DANGER, ch );
               else if( gch->hit < gch->max_hit / 2.5 )
                  set_char_color( AT_YELLOW, ch );
               else
                  set_char_color( AT_GREY, ch );
               ch_printf( ch, "%5d", gch->hit );
               set_char_color( AT_GREY, ch );
               ch_printf( ch, "/%-5d ", gch->max_hit );
               set_char_color( AT_LBLUE, ch );
               ch_printf( ch, "%5d/%-5d ", gch->mana, gch->max_mana );
               if( gch->mental_state < -25 || gch->mental_state > 25 )
                  set_char_color( AT_YELLOW, ch );
               else
                  set_char_color( AT_GREEN, ch );
               ch_printf( ch, "%3.3s  ",
                          gch->mental_state > 75 ? "+++" :
                          gch->mental_state > 50 ? "=++" :
                          gch->mental_state > 25 ? "==+" :
                          gch->mental_state > -25 ? "===" :
                          gch->mental_state > -50 ? "-==" : gch->mental_state > -75 ? "--=" : "---" );
               set_char_color( AT_DGREEN, ch );
               ch_printf( ch, "%5d ", gch->move );
               ch_printf( ch, " %-10s", gch->species ? gch->species : npc_race[gch->race] );
               set_char_color( AT_GREEN, ch );

               if( gch->protected_by && gch->protected_by->in_room == gch->in_room )
                  ch_printf( ch, "%-12s ", gch->protected_by->name );
               send_to_char( "\n\r", ch );
            }
         }
      }
      return;
   }

   if( !strcmp( arg, "disband" ) )
   {
      CHAR_DATA *gch;
      int count = 0;

      if( ch->leader || ch->master )
      {
         send_to_char( "You cannot disband a group if you're following someone.\n\r", ch );
         return;
      }

      for( gch = first_char; gch; gch = gch->next )
      {
         if( is_same_group( ch, gch ) && ( ch != gch ) )
         {
            gch->leader = NULL;
            gch->master = NULL;
            count++;
            send_to_char( "Your group is disbanded.\n\r", gch );
         }
      }

      if( count == 0 )
         send_to_char( "You have no group members to disband.\n\r", ch );
      else
         send_to_char( "You disband your group.\n\r", ch );

      return;
   }

   if( !strcmp( arg, "all" ) )
   {
      CHAR_DATA *rch;
      int count = 0;

      for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      {
         if( ch != rch
             && !IS_NPC( rch )
             && can_see( ch, rch )
             && rch->master == ch
             && !ch->master && !ch->leader && !is_same_group( rch, ch ) && IS_PKILL( ch ) == IS_PKILL( rch ) )
         {
            rch->leader = ch;
            count++;
         }
      }

      if( count == 0 )
         send_to_char( "You have no eligible group members.\n\r", ch );
      else
      {
         act( AT_ACTION, "$n groups $s followers.", ch, NULL, NULL, TO_ROOM );
         send_to_char( "You group your followers.\n\r", ch );
      }
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( ch->master || ( ch->leader && ch->leader != ch ) )
   {
      send_to_char( "But you are following someone else!\n\r", ch );
      return;
   }

   if( victim->master != ch && ch != victim )
   {
      act( AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( victim == ch )
   {
      act( AT_PLAIN, "You can't group yourself.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( is_same_group( victim, ch ) && ch != victim )
   {
      victim->leader = NULL;
      act( AT_ACTION, "$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT );
      act( AT_ACTION, "$n removes you from $s group.", ch, NULL, victim, TO_VICT );
      act( AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR );
      return;
   }

   victim->leader = ch;
   act( AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
   act( AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT );
   act( AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR );
   return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *gch;
   int members;
   int amount;
   int share;
   int extra;

   one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "Split how much?\n\r", ch );
      return;
   }

   amount = atoi( arg );

   if( amount < 0 )
   {
      send_to_char( "Your group wouldn't like that.\n\r", ch );
      return;
   }

   if( amount == 0 )
   {
      send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
      return;
   }

   if( ch->gold < amount )
   {
      send_to_char( "You don't have that much gold.\n\r", ch );
      return;
   }

   members = 0;
   for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
   {
      if( is_same_group( gch, ch ) )
         members++;
   }


   if( xIS_SET( ch->act, PLR_AUTOGOLD ) && members < 2 )
      return;

   if( members < 2 )
   {
      send_to_char( "Just keep it all.\n\r", ch );
      return;
   }

   share = amount / members;
   extra = amount % members;

   if( share == 0 )
   {
      send_to_char( "Don't even bother, cheapskate.\n\r", ch );
      return;
   }

   ch->gold -= amount;
   ch->gold += share + extra;

   set_char_color( AT_GOLD, ch );
   ch_printf( ch, "You split %d gold coins.  Your share is %d gold coins.\n\r", amount, share + extra );

   sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.", amount, share );

   for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
   {
      if( gch != ch && is_same_group( gch, ch ) )
      {
         act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
         gch->gold += share;
      }
   }
   return;
}



void do_gtell( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *gch;

   if( argument[0] == '\0' )
   {
      send_to_char( "Tell your group what?\n\r", ch );
      return;
   }

   if( xIS_SET( ch->act, PLR_NO_TELL ) )
   {
      send_to_char( "Your message didn't get through!\n\r", ch );
      return;
   }

   /*
    * Note use of send_to_char, so gtell works on sleepers.
    */
   for( gch = first_char; gch; gch = gch->next )
   {
      if( is_same_group( gch, ch ) )
      {
         set_char_color( AT_GTELL, gch );
         ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, argument );
      }
   }

   return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA * ach, CHAR_DATA * bch )
{
   if( ach->leader )
      ach = ach->leader;
   if( bch->leader )
      bch = bch->leader;
   return ach == bch;
}

/* Command to leave a message on the floor using the trail code -- Scion *cackle* */
void do_graffiti( CHAR_DATA * ch, char *argument )
{
   TRAIL_DATA *trail;

   for( trail = ch->in_room->first_trail; trail; trail = trail->next )
   {
      if( !strcmp( ch->name, trail->name ) )
         break;
   }

   if( !trail )
   {
      send_to_char( "There does not appear to be any space for graffiti here.\r\n", ch );
      return;
   }

   if( strlen( argument ) < 2 )
   {
      send_to_char( "What do you want to write here?\r\n", ch );
      return;
   }

   if( strlen( argument ) > 80 )
   {
      send_to_char( "There is not enough space to write that, unfortunately.\r\n", ch );
      return;
   }

   if( trail->graffiti )
      STRFREE( trail->graffiti );
   trail->graffiti = STRALLOC( argument );

   act( AT_PLAIN, "You write a message for all to see.", ch, NULL, NULL, TO_CHAR );
   act( AT_PLAIN, "$n writes a message for all to see.", ch, NULL, NULL, TO_ROOM );
}


/* New Channel Code by Keolah - Feb 9, 2002
 *
 * In an attempt to make channels more versitile and player-controllable,
 * this code will allow the creation and use of an unlimited number of
 * user-controlled channels. The channel's owner will have full control
 * over who can use it, and allow private and public channels, as well as
 * kicking and banning people from specific channels.
 */

CHANNEL_DATA *find_chan( char *name )
{
   CHANNEL_DATA *chan;

   for( chan = first_channel; chan; chan = chan->next )
      if( !str_cmp( chan->name, name ) )
         return chan;

   return NULL;
}

void add_to_chan( CHAR_DATA * ch, CHANNEL_DATA * chan )
{
   CHANNEL_MEMBER *chm;

   CREATE( chm, CHANNEL_MEMBER, 1 );

   chm->ch = ch;

   LINK( chm, chan->first_member, chan->last_member, next, prev );
}


/* Channel editor -- Keolah */
void do_channels( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHANNEL_DATA *chan;
   CHANNEL_DATA *new_chan;
   CHANNEL_MEMBER *chm;

   if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_NO_CHANNEL ) )
   {
      ch_printf( ch, "You can't use channels!\n\r" );
      return;
   }

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "&wSyntax: chan create <chan>\r\n", ch );
      send_to_char( "        chan delete <chan>\r\n", ch );
      send_to_char( "        chan list\r\n", ch );
      send_to_char( "        chan show <chan>\r\n", ch );
      send_to_char( "        chan flags <chan> <value>\r\n", ch );
      send_to_char( "        chan join|leave <chan>\n\r", ch );
      send_to_char( "        chan invite|kick <chan> <person>\n\r", ch );
      send_to_char( "        chan save\r\n", ch );
      send_to_char( "        chan rejoin|quiet\n\r", ch );
      send_to_char( "        chan clan|permit <chan> <restriction>\n\r", ch );
      send_to_char( "        chan tag|div <chan> <string>\n\r", ch );
      send_to_char( "        chan access|ban <chan> <name>\n\r", ch );
      send_to_char( "        chan transfer <chan> <person>\n\r", ch );
      return;
   }

   if( !strcmp( arg1, "list" ) )
   {
      if( !first_channel )
      {
         send_to_char( "No channels have been defined.\r\n", ch );
         return;
      }
      send_to_char( "&gCurrently defined channels:\r\n", ch );
      for( chan = first_channel; chan; chan = chan->next )
      {
         if( ( chm = get_chatter( ch, chan ) ) != NULL )
            ch_printf( ch, "&G%s (&gowned by &G%s)\r\n", chan->name, chan->owner );
         else
            ch_printf( ch, "&Y%s &G(&gowned by &G%s)\n\r", chan->name, chan->owner );
      }
      return;
   }

   if( !strcmp( arg1, "show" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }
      ch_printf( ch, "&gChannel &G%s:\r\n", chan->name );
      ch_printf( ch, "&gInfo: &G%s\n\r", chan->info );
      ch_printf( ch, "&gOwned by &G%s\n\r", chan->owner );
      ch_printf( ch, "&gTag: %s   &gDivider: %s\n\r", chan->tag, chan->div );
      ch_printf( ch, "&gClan: %s   &gPermission: &G%d\n\r", chan->clan, chan->permit );
      ch_printf( ch, "&gAccess list: %s\n\r", chan->access );
      ch_printf( ch, "&gBan list: %s\n\r", chan->ban );
      ch_printf( ch, "&gCurrent chatters:&G\n\r" );
      for( chm = chan->first_member; chm; chm = chm->next )
         ch_printf( ch, "  %s\n\r", chm->ch->name );
      return;
   }

   if( !strcmp( arg1, "create" ) )
   {
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "What do you want to call your new channel?\n\r", ch );
         return;
      }

      CREATE( new_chan, CHANNEL_DATA, 1 );

      if( !first_channel )
      {
         first_channel = new_chan;
         last_channel = new_chan;
      }
      else
      {
         last_channel->next = new_chan;
         new_chan->prev = last_channel;
         last_channel = new_chan;
      }

      chan = new_chan;
      STRFREE( chan->name );
      chan->name = STRALLOC( arg2 );
      STRFREE( chan->owner );
      chan->owner = STRALLOC( ch->name );
      STRFREE( chan->tag );
      sprintf( buf, "&G<&g%s&G>", chan->name );
      chan->tag = STRALLOC( buf );
      STRFREE( chan->div );
      chan->div = STRALLOC( "&g:" );
      STRFREE( chan->access );
      chan->access = STRALLOC( "" );
      STRFREE( chan->ban );
      chan->ban = STRALLOC( "" );
      ch_printf( ch, "Channel %s created.\r\n", chan->name );
      add_to_chan( ch, chan );
      return;
   }

   if( !strcmp( arg1, "delete" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      send_to_char( "You delete the channel.\r\n", ch );

      UNLINK( chan, first_channel, last_channel, next, prev );
      STRFREE( chan->name );
      for( chm = chan->first_member; chm; chm = chm->next )
      {
         if( chm->prev )
            DISPOSE( chm->prev );
      }
      DISPOSE( chan->last_member );

      DISPOSE( chan );
      return;
   }

   if( !strcmp( arg1, "join" ) )
   {
      CLAN_DATA *clan;

      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( ( chm = get_chatter( ch, chan ) ) != NULL )
      {
         send_to_char( "You are already on that channel!\n\r", ch );
         return;
      }

      if( !is_name( ch->name, chan->access ) )
      {

         if( is_name( ch->name, chan->ban ) )
         {
            send_to_char( "You are banned from that channel.\n\r", ch );
            return;
         }

         if( chan->clan && ( clan = get_clan( ch, chan->clan ) ) != NULL )
         {
            if( ( get_member( ch->name, clan ) ) == NULL )
            {
               ch_printf( ch, "You must be a member of the clan %s to join that channel.\n\r", chan->clan );
               return;
            }
         }

         if( chan->permit && !IS_SET( ch->pcdata->permissions, chan->permit ) )
         {
            send_to_char( "You don't have the required permission to join that channel.\n\r", ch );
            return;
         }

      }

      add_to_chan( ch, chan );

      for( chm = chan->first_member; chm; chm = chm->next )
      {
         ch_printf( chm->ch, "%s %s has joined the channel.\n\r", chan->tag, ch->name );
      }
      return;
   }

   if( !strcmp( arg1, "leave" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( ( chm = get_chatter( ch, chan ) ) == NULL )
      {
         send_to_char( "You aren't even on that channel!\n\r", ch );
         return;
      }

      for( chm = chan->first_member; chm; chm = chm->next )
      {
         ch_printf( chm->ch, "%s %s has left the channel.\n\r", chan->tag, ch->name );
      }
      remove_from_chan( ch, chan );
      return;
   }
   if( !strcmp( arg1, "kick" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( ( chm = get_chatter( ch, chan ) ) == NULL )
      {
         send_to_char( "You aren't even on that channel!\n\r", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      victim = get_char_world( ch, argument );
      if( !victim )
      {
         send_to_char( "You can't find anyone like that.\n\r", ch );
         return;
      }

      if( ( chm = get_chatter( victim, chan ) ) == NULL )
      {
         send_to_char( "They aren't on that channel!\n\r", ch );
         return;
      }

      for( chm = chan->first_member; chm; chm = chm->next )
      {
         ch_printf( chm->ch, "%s %s has been kicked by %s!\n\r", chan->tag, victim->name, ch->name );
      }
      remove_from_chan( victim, chan );
      return;
   }

   if( !strcmp( arg1, "invite" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( ( chm = get_chatter( ch, chan ) ) == NULL )
      {
         send_to_char( "You aren't even on that channel!\n\r", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      victim = get_char_world( ch, argument );
      if( !victim )
      {
         send_to_char( "You can't find anyone like that.\n\r", ch );
         return;
      }

      if( ( chm = get_chatter( victim, chan ) ) != NULL )
      {
         send_to_char( "They are already on that channel!\n\r", ch );
         return;
      }

      add_to_chan( victim, chan );

      for( chm = chan->first_member; chm; chm = chm->next )
      {
         ch_printf( chm->ch, "%s %s has been invited to the channel by %s.\n\r", chan->tag, victim->name, ch->name );
      }
      return;
   }

   if( !strcmp( arg1, "transfer" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      victim = get_char_world( ch, argument );
      if( !victim )
      {
         send_to_char( "You can't find anyone like that.\n\r", ch );
         return;
      }

      STRFREE( chan->owner );
      chan->owner = STRALLOC( victim->name );

      for( chm = chan->first_member; chm; chm = chm->next )
      {
         ch_printf( chm->ch, "%s %s has transfered ownership of the channel to %s.\n\r", chan->tag, ch->name, victim->name );
      }
      return;
   }

   if( !strcmp( arg1, "clan" ) )
   {
      CLAN_DATA *clan;

      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      if( ( clan = get_clan( ch, argument ) ) == NULL )
      {
         send_to_char( "There is no such clan.\n\r", ch );
         return;
      }

      STRFREE( chan->clan );
      chan->clan = STRALLOC( argument );
      ch_printf( ch, "You restrict the channel %s to members of %s.\n\r", chan->name, clan->title );
      return;
   }

   if( !strcmp( arg1, "permit" ) )
   {
      int i;

      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      i = get_permit_flag( argument );
      if( i < 0 || i > MAX_BITS )
      {
         send_to_char( "Unknown flag.\r\n", ch );
         return;
      }
      chan->permit = 1 << i;
      ch_printf( ch, "You restrict the %s channel to those with %s permission.\n\r", chan->name, argument );
      return;
   }

   if( !strcmp( arg1, "tag" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      STRFREE( chan->tag );
      chan->tag = STRALLOC( argument );
      ch_printf( ch, "You set the tag of the %s channel to '%s'.\n\r", chan->name, chan->tag );
      return;
   }

   if( !strcmp( arg1, "div" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      STRFREE( chan->div );
      chan->div = STRALLOC( argument );
      ch_printf( ch, "You set the divider of the %s channel to '%s'.\n\r", chan->name, chan->div );
      return;
   }

   if( !strcmp( arg1, "info" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      STRFREE( chan->info );
      chan->info = STRALLOC( argument );
      ch_printf( ch, "You set the info of the %s channel to '%s'.\n\r", chan->name, chan->info );
      return;
   }

   if( !strcmp( arg1, "access" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      if( !strcmp( argument, "clear" ) )
      {
         STRFREE( chan->access );
         chan->access = STRALLOC( "" );
         send_to_char( "You clear the channel access list.\n\r", ch );
         return;
      }

      if( is_name( argument, chan->access ) )
      {
         ch_printf( ch, "%s already has access to %s.\n\r", capitalize( argument ), chan->name );
         return;
      }

      sprintf( buf, "%s %s", chan->access, argument );
      STRFREE( chan->access );
      chan->access = STRALLOC( buf );
      ch_printf( ch, "You allow %s access to the %s channel.\n\r", capitalize( argument ), chan->name );
      return;
   }

   if( !strcmp( arg1, "ban" ) )
   {
      if( !( chan = find_chan( arg2 ) ) )
      {
         send_to_char( "There is no channel with that name.\r\n", ch );
         return;
      }

      if( str_cmp( ch->name, chan->owner ) && !IS_SET( ch->pcdata->permissions, PERMIT_CHANNEL ) )
      {
         send_to_char( "You aren't the owner of that channel.\n\r", ch );
         return;
      }

      if( !strcmp( argument, "clear" ) )
      {
         STRFREE( chan->ban );
         chan->ban = STRALLOC( "" );
         send_to_char( "You clear the channel ban list.\n\r", ch );
         return;
      }

      if( is_name( argument, chan->ban ) )
      {
         ch_printf( ch, "%s is already banned from %s.\n\r", capitalize( argument ), chan->name );
         return;
      }

      sprintf( buf, "%s %s", chan->ban, argument );
      STRFREE( chan->ban );
      chan->ban = STRALLOC( buf );
      ch_printf( ch, "You ban %s from the %s channel.\n\r", capitalize( argument ), chan->name );
      return;
   }

   if( !strcmp( arg1, "rejoin" ) )
   {
      argument = ch->pcdata->channels;
      while( argument && argument[0] != '\0' )
      {
         argument = one_argument( argument, arg2 );
         sprintf( buf, "join %s", arg2 );
         do_channels( ch, buf );
      }
      return;
   }

   if( !strcmp( arg1, "quiet" ) )
   {
      STRFREE( ch->pcdata->channels );
      ch->pcdata->channels = STRALLOC( "" );
      for( chan = first_channel; chan; chan = chan->next )
      {
         if( get_chatter( ch, chan ) )
         {
            sprintf( buf, "%s %s", ch->pcdata->channels, chan->name );
            STRFREE( ch->pcdata->channels );
            ch->pcdata->channels = STRALLOC( buf );
            sprintf( buf, "leave %s", chan->name );
            do_channels( ch, buf );
         }
      }
      return;
   }

   if( !strcmp( arg1, "save" ) )
   {
      FILE *fp;

      if( ( fp = fopen( CHANNEL_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open channel file!", 0 );
         return;
      }

      for( chan = first_channel; chan; chan = chan->next )
      {
         fprintf( fp, "\n#CHANNEL\n" );
         fprintf( fp, "Name        %s~\n", chan->name );
         fprintf( fp, "Owner       %s~\n", chan->owner );
         fprintf( fp, "Tag	 %s~\n", chan->tag );
         fprintf( fp, "Div	 %s~\n", chan->div );
         fprintf( fp, "Flags	 %d\n", chan->flags );
         fprintf( fp, "Clan	 %s~\n", chan->clan );
         fprintf( fp, "Permit	 %d\n", chan->permit );
         fprintf( fp, "Access	 %s~\n", chan->access );
         fprintf( fp, "Ban	 %s~\n", chan->ban );
         fprintf( fp, "Info	 %s~\n", chan->info );
      }
      fprintf( fp, "#END\n" );

      fclose( fp );

      STRFREE( ch->pcdata->channels );
      ch->pcdata->channels = STRALLOC( "" );
      for( chan = first_channel; chan; chan = chan->next )
      {
         if( get_chatter( ch, chan ) )
         {
            sprintf( buf, "%s %s", ch->pcdata->channels, chan->name );
            STRFREE( ch->pcdata->channels );
            ch->pcdata->channels = STRALLOC( buf );
         }
      }

      send_to_char( "channels saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}

void do_chat( CHAR_DATA * ch, char *argument )
{
   CHANNEL_DATA *chan;
   CHANNEL_MEMBER *chm;
   SOCIALTYPE *social;
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || xIS_SET( ch->act, PLR_NO_CHANNEL ) )
   {
      ch_printf( ch, "You can't use channels!\n\r" );
      return;
   }

   argument = one_argument( argument, arg1 );

   if( ( chan = find_chan( arg1 ) ) == NULL )
   {
      send_to_char( "There is no channel with that name.\r\n", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Say what where?\n\r", ch );
      return;
   }

   if( ( chm = get_chatter( ch, chan ) ) == NULL )
   {
      send_to_char( "You aren't even on that channel!\n\r", ch );
      return;
   }

   /*
    * Channel emotes 
    */
   if( argument[0] == ',' )
   {
      for( chm = chan->first_member; chm; chm = chm->next )
      {
         ch_printf( chm->ch, "%s %s %s\n\r", chan->tag, ch->name, argument + 1 );
      }
      /*
       * Channel socials 
       */
   }
   else if( argument[0] == '@' )
   {
      argument = one_argument( argument, arg1 );
      argument = one_argument( argument, buf );
      victim = NULL;
      if( ( social = find_social( arg1 + 1 ) ) == NULL )
      {
         ch_printf( ch, "Can't find %s social.\n\r", arg1 + 1 );
         return;
      }
      if( buf != NULL )
         victim = get_char_world( ch, buf );
      if( victim && victim != ch )
         for( chm = chan->first_member; chm; chm = chm->next )
         {
            if( chm->ch == victim )
               ch_printf( chm->ch, "%s %s.\n\r", chan->tag,
                          act_string( social->vict_found, chm->ch, ch,
                                      argument[0] != '\0' ? argument : social->adj, victim, STRING_NONE ) );
            else if( chm->ch == ch )
               ch_printf( chm->ch, "%s %s.\n\r", chan->tag,
                          act_string( social->char_found, chm->ch, ch,
                                      argument[0] != '\0' ? argument : social->adj, victim, STRING_NONE ) );
            else
               ch_printf( chm->ch, "%s %s.\n\r", chan->tag,
                          act_string( social->others_found, chm->ch, ch,
                                      argument[0] != '\0' ? argument : social->adj, victim, STRING_NONE ) );
         }
      else if( victim && victim == ch )
         for( chm = chan->first_member; chm; chm = chm->next )
         {
            if( chm->ch == victim )
               ch_printf( chm->ch, "%s %s.\n\r", chan->tag,
                          act_string( social->char_auto, chm->ch, ch,
                                      argument[0] != '\0' ? argument : social->adj, victim, STRING_NONE ) );
            else
               ch_printf( chm->ch, "%s %s.\n\r", chan->tag,
                          act_string( social->others_auto, chm->ch, ch,
                                      argument[0] != '\0' ? argument : social->adj, victim, STRING_NONE ) );
         }
      else
         for( chm = chan->first_member; chm; chm = chm->next )
         {
            if( chm->ch == victim )
               ch_printf( chm->ch, "%s %s.\n\r", chan->tag,
                          act_string( social->char_no_arg, chm->ch, ch,
                                      buf[0] != '\0' ? buf : social->adj, victim, STRING_NONE ) );
            else
               ch_printf( chm->ch, "%s %s.\n\r", chan->tag,
                          act_string( social->others_no_arg, chm->ch, ch,
                                      buf[0] != '\0' ? buf : social->adj, victim, STRING_NONE ) );
         }
      /*
       * Normal chatter 
       */
   }
   else
   {
      for( chm = chan->first_member; chm; chm = chm->next )
      {
         ch_printf( chm->ch, "%s %s %s %s\n\r", chan->tag, ch->name, chan->div, argument );
      }
   }
}

bool check_channel( CHAR_DATA * ch, char *command, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHANNEL_DATA *chan;

   if( ( chan = find_chan( command ) ) != NULL && ( get_chatter( ch, chan ) != NULL ) )
   {
      sprintf( buf, "%s %s", command, argument );
      do_chat( ch, buf );
      return TRUE;
   }
   return FALSE;
}
