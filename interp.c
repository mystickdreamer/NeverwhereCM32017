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
 *			 Command interpretation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 * Externals
 */
void refresh_page( CHAR_DATA * ch );
void subtract_times( struct timeval *etime, struct timeval *stime );

bool check_channel args( ( CHAR_DATA * ch, char *command, char *argument ) );

bool check_social args( ( CHAR_DATA * ch, char *command, char *argument ) );
bool check_alias args( ( CHAR_DATA * ch, char *command, char *argument ) );



/*
 * Log-all switch.
 */
bool fLogAll = FALSE;


CMDTYPE *command_hash[126];   /* hash table for cmd_table */
SOCIALTYPE *social_index[27]; /* hash table for socials   */

/*
 * Character not in position for command?
 */
bool check_pos( CHAR_DATA * ch, sh_int position )
{
   if( !IS_NPC( ch ) && IS_SET( ch->pcdata->permissions, PERMIT_ADMIN ) )
      return TRUE;

   if( IS_NPC( ch ) && ch->position > 3 ) /*Band-aid alert?  -- Blod */
      return TRUE;

   if( ch->wait && position > POS_CHANNEL )
   {
      ch_printf( ch, "You are still %s.\n\r", ch->last_taken );
      return FALSE;
   }

   if( ch->position < position )
   {
      switch ( ch->position )
      {
         case POS_DEAD:
            send_to_char( "A little difficult to do when you are DEAD...\n\r", ch );
            break;

         case POS_CHANNEL:
            send_to_char( "You are busy channeling.\n\r", ch );
            break;

         case POS_INCAP:
            send_to_char( "You are hurt far too bad for that.\n\r", ch );
            break;

         case POS_STUNNED:
            send_to_char( "You are unconscious.\n\r", ch );
            break;

         case POS_SLEEPING:
            send_to_char( "In your dreams, or what?\n\r", ch );
            break;

         case POS_RESTING:
            send_to_char( "Nah... You feel too relaxed...\n\r", ch );
            break;

         case POS_SITTING:
            send_to_char( "You can't do that sitting down.\n\r", ch );
            break;

         case POS_KNEELING:
            send_to_char( "You should get up off your knees first.\n\r", ch );
            break;

         case POS_SQUATTING:
            send_to_char( "Maybe you should stand up first?\n\r", ch );
            break;

         case POS_SWIMMING:
            send_to_char( "You're too busy concentrating on swimming.\n\r", ch );
            break;

         case POS_FLYING:
            send_to_char( "You're too busy concentrating on flying.\n\r", ch );
            break;

      }
      return FALSE;
   }
   return TRUE;
}

extern char lastplayercmd[MAX_INPUT_LENGTH * 2];

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA * ch, char *argument, bool forced )
{
   char command[MAX_INPUT_LENGTH];
   char logline[MAX_INPUT_LENGTH];
   char logname[MAX_INPUT_LENGTH];
   TIMER *timer = NULL;
   CMDTYPE *cmd = NULL;
   int loglvl;
   bool found;
   struct timeval time_used;
   long tmptime;


   if( !ch )
   {
      bug( "interpret: null ch!", 0 );
      return;
   }

   if( !ch->in_room )
   {
      bug( "interpret: null in_room!", 0 );
      if( !IS_NPC( ch ) )
         char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
      send_to_char( "You are in the middle of nowhere.\n\r", ch );
      return;
   }

   found = FALSE;
   if( ch->substate == SUB_REPEATCMD )
   {
      DO_FUN *fun;

      if( ( fun = ch->last_cmd ) == NULL )
      {
         ch->substate = SUB_NONE;
         bug( "interpret: SUB_REPEATCMD with NULL last_cmd", 0 );
         return;
      }
      else
      {
         int x;

         /*
          * yes... we lose out on the hashing speediness here...
          * but the only REPEATCMDS are wizcommands (currently)
          */
         for( x = 0; x < 126; x++ )
         {
            for( cmd = command_hash[x]; cmd; cmd = cmd->next )
               if( cmd->do_fun == fun )
               {
                  found = TRUE;
                  break;
               }
            if( found )
               break;
         }
         if( !found )
         {
            cmd = NULL;
            bug( "interpret: SUB_REPEATCMD: last_cmd invalid", 0 );
            return;
         }
         sprintf( logline, "(%s) %s", cmd->name, argument );
      }
   }

   if( !cmd )
   {
      /*
       * Changed the order of these ifchecks to prevent crashing. 
       */
      if( !argument || !strcmp( argument, "" ) )
      {
         bug( "interpret: null argument!", 0 );
         return;
      }

      /*
       * Strip leading spaces.
       */
      while( isspace( *argument ) )
         argument++;
      if( argument[0] == '\0' )
         return;

      /*
       * Implement freeze command.
       */
      if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_FREEZE ) )
      {
         send_to_char( "You're totally frozen!\n\r", ch );
         return;
      }

      /*
       * Grab the command word.
       * Special parsing so ' can be a command,
       *   also no spaces needed after punctuation.
       */
      strcpy( logline, argument );
      if( !isalpha( argument[0] ) && !isdigit( argument[0] ) )
      {
         command[0] = argument[0];
         command[1] = '\0';
         argument++;
         while( isspace( *argument ) )
            argument++;
      }
      else
         argument = one_argument( argument, command );

      for( cmd = command_hash[LOWER( command[0] ) % 126]; cmd; cmd = cmd->next )
      {
         if( !str_prefix( command, cmd->name ) )
         {
            if( !cmd->permit )
            {
               found = TRUE;
               break;
            }
            if( IS_NPC( ch ) && cmd->permit == PERMIT_MOB )
            {
               found = TRUE;
               break;
            }
            if( !IS_NPC( ch ) && IS_SET( ch->pcdata->permissions, cmd->permit ) )
            {
               found = TRUE;
               break;
            }
         }
      }

      /*
       * Catch being forced to change password etc 
       */
      if( forced && found && cmd->log == LOG_NEVER )
         return;

      if( forced && found && cmd->permit )
         return;

      /*
       * Turn off afk bit when any command performed.
       * don't turn it off if they were forced -keo
       */
      if( xIS_SET( ch->act, PLR_AFK ) && ( str_cmp( command, "AFK" ) ) && !forced )
      {
         xREMOVE_BIT( ch->act, PLR_AFK );
         act( AT_GREY, "$n is no longer idle.", ch, NULL, NULL, TO_CANSEE );
         send_to_char( "You are no longer idle.\r\n", ch );
      }
   }

   /*
    * Log and snoop.
    */
   sprintf( lastplayercmd, "%s used %s", ch->name, logline );

   if( found && cmd->log == LOG_NEVER )
      strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );

   loglvl = found ? cmd->log : LOG_NORMAL;

   if( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_LOG ) )
       || fLogAll || loglvl == LOG_BUILD || loglvl == LOG_HIGH || loglvl == LOG_ALWAYS )
   {
      /*
       * Added by Narn to show who is switched into a mob that executes
       * a logged command.  Check for descriptor in case force is used. 
       */
      if( ch->desc && ch->desc->original )
         sprintf( log_buf, "Log %s (%s): %s", ch->name, ch->desc->original->name, logline );
      else
         sprintf( log_buf, "Log %s: %s", ch->name, logline );

      /*
       * Make it so a 'log all' will send most output to the log
       * file only, and not spam the log channel to death   -Thoric
       */
      if( fLogAll && loglvl == LOG_NORMAL && ( IS_NPC( ch ) || !xIS_SET( ch->act, PLR_LOG ) ) )
         loglvl = LOG_ALL;

      log_string_plus( log_buf, loglvl, 0 );
   }

   if( ch->desc && ch->desc->snoop_by )
   {
      sprintf( logname, "%s", ch->name );
      write_to_buffer( ch->desc->snoop_by, logname, 0 );
      write_to_buffer( ch->desc->snoop_by, "% ", 2 );
      write_to_buffer( ch->desc->snoop_by, logline, 0 );
      write_to_buffer( ch->desc->snoop_by, "\n\r", 2 );
   }

   /*
    * check for a timer delayed command (search, dig, detrap, etc) 
    */
   if( ( timer = get_timerptr( ch, TIMER_DO_FUN ) ) != NULL )
   {
      int tempsub;

      tempsub = ch->substate;
      ch->substate = SUB_TIMER_DO_ABORT;
      ( timer->do_fun ) ( ch, "" );
      if( char_died( ch ) )
         return;
      if( ch->substate != SUB_TIMER_CANT_ABORT )
      {
         ch->substate = tempsub;
         extract_timer( ch, timer );
      }
      else
      {
         ch->substate = tempsub;
         return;
      }
   }


   /*
    * Look for command in skill and socials table.
    */
   if( !found )
   {
      if( !check_alias( ch, command, argument )
          && !check_skill( ch, command, argument )
          && !check_social( ch, command, argument ) && !check_channel( ch, command, argument )
#ifdef I3
          && !I3_command_hook( ch, command, argument )
#endif
#ifdef IMC
          && !imc_command_hook( ch, command, argument )
#endif
          )
      {
         EXIT_DATA *pexit;

         /*
          * check for an auto-matic exit command 
          */
         if( ( pexit = find_door( ch, command, TRUE ) ) != NULL && IS_SET( pexit->exit_info, EX_xAUTO ) )
         {
            if( IS_SET( pexit->exit_info, EX_CLOSED )
                && ( !IS_AFFECTED( ch, AFF_PASS_DOOR ) || IS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
            {
               /*
                * If they've set outputprefix, spam it here! -- Scion 
                */

               if( !IS_NPC( ch ) && ch->pcdata->outputprefix )
               {
                  send_to_char( ch->pcdata->outputprefix, ch );
                  send_to_char( "\r\n", ch );
               }

               if( !IS_SET( pexit->exit_info, EX_SECRET ) )
                  act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
               else
                  send_to_char( "You cannot do that here.\n\r", ch );

               /*
                * If they've set outputsuffix, spam it here! -- Scion 
                */
               if( !IS_NPC( ch ) && ch->pcdata->outputsuffix )
               {
                  send_to_char( ch->pcdata->outputsuffix, ch );
                  send_to_char( "\r\n", ch );
               }

               return;
            }
            move_char( ch, pexit, 0 );
            /*
             * If they've set outputsuffix, spam it here! -- Scion 
             */
            if( !IS_NPC( ch ) && ch->pcdata->outputsuffix )
            {
               send_to_char( ch->pcdata->outputsuffix, ch );
               send_to_char( "\r\n", ch );
            }

            return;
         }
         huh( ch );
         /*
          * If they've set outputsuffix, spam it here! -- Scion 
          */
         if( !IS_NPC( ch ) && ch->pcdata->outputsuffix )
         {
            send_to_char( ch->pcdata->outputsuffix, ch );
            send_to_char( "\r\n", ch );
         }
      }
      return;
   }

   /*
    * Character not in position for command?
    */
   if( !check_pos( ch, cmd->position ) )
      return;

   /*
    * If they've set outputprefix, spam it here! -- Scion 
    */

   if( !IS_NPC( ch ) && ch->pcdata->outputprefix )
   {
      send_to_char( ch->pcdata->outputprefix, ch );
      send_to_char( "\r\n", ch );
   }

   /*
    * Dispatch the command.
    */

   ch->prev_cmd = ch->last_cmd;  /* haus, for automapping */
   ch->last_cmd = cmd->do_fun;
   start_timer( &time_used );
   ( *cmd->do_fun ) ( ch, argument );
   end_timer( &time_used );

   /*
    * Command's done, send the outputsuffix if they want it -- Scion 
    */

   if( !IS_NPC( ch ) && ch->pcdata->outputsuffix )
   {
      send_to_char( ch->pcdata->outputsuffix, ch );
      send_to_char( "\r\n", ch );
   }

   /*
    * Update the record of how many times this command has been used (haus)
    */
   update_userec( &time_used, &cmd->userec );
   tmptime = UMIN( time_used.tv_sec, 19 ) * 1000000 + time_used.tv_usec;

   /*
    * laggy command notice: command took longer than 1.5 seconds 
    */
   if( tmptime > 1500000 )
   {
#ifdef sun
      sprintf( log_buf, "[*****] LAG: %s: %s %s (R:%d S:%d.%06d)", ch->name,
               cmd->name, ( cmd->log == LOG_NEVER ? "XXX" : argument ),
               ch->in_room ? ch->in_room->vnum : 0, time_used.tv_sec, time_used.tv_usec );
#else
      sprintf( log_buf, "[*****] LAG: %s: %s %s (R:%d S:%ld.%06ld)", ch->name,
               cmd->name, ( cmd->log == LOG_NEVER ? "XXX" : argument ),
               ch->in_room ? ch->in_room->vnum : 0, time_used.tv_sec, time_used.tv_usec );
#endif
      log_string_plus( log_buf, LOG_NORMAL, 0 );
      cmd->lag_count++; /* count the lag flags */
   }

   tail_chain(  );
}

CMDTYPE *find_command( char *command )
{
   CMDTYPE *cmd;
   int hash;

   hash = LOWER( command[0] ) % 126;

   for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
      if( !str_prefix( command, cmd->name ) )
         return cmd;

   return NULL;
}

SOCIALTYPE *find_social( char *command )
{
   SOCIALTYPE *social;
   int hash;

   if( command[0] < 'a' || command[0] > 'z' )
      hash = 0;
   else
      hash = ( command[0] - 'a' ) + 1;

   for( social = social_index[hash]; social; social = social->next )
      if( !str_prefix( command, social->name ) )
         return social;

   return NULL;
}

bool check_social( CHAR_DATA * ch, char *command, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   SOCIALTYPE *social;
   char adj[MAX_STRING_LENGTH];

   if( ( social = find_social( command ) ) == NULL )
      return FALSE;

   /*
    * If they've set outputprefix, spam it here! -- Scion 
    */

   if( !IS_NPC( ch ) && ch->pcdata->outputprefix )
   {
      send_to_char( ch->pcdata->outputprefix, ch );
      send_to_char( "\r\n", ch );
   }

   if( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You are anti-social!\n\r", ch );
      return TRUE;
   }

   if( IS_AFFECTED( ch, AFF_SILENT ) && IS_SET( social->flags, SOC_AUDITORY ) )
   {
      send_to_char( "You make no sound.\n\r", ch );
      return TRUE;
   }

   switch ( ch->position )
   {
      case POS_STUNNED:
         send_to_char( "You are too stunned to do that.\n\r", ch );
         return TRUE;

      case POS_SLEEPING:
         if( IS_SET( social->flags, SOC_SLEEP ) )
            break;
         send_to_char( "In your dreams, or what?\n\r", ch );
         return TRUE;

   }

   argument = one_argument( argument, arg );
   argument = one_argument( argument, adj );
   victim = NULL;

   if( arg[0] == '\0' )
   {
      act( AT_SOCIAL, social->others_no_arg, ch, social->adj, victim, TO_ROOM );
      act( AT_SOCIAL, social->char_no_arg, ch, social->adj, victim, TO_CHAR );
   }
   else if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      if( !victim )
      {
         act( AT_SOCIAL, social->others_no_arg, ch, arg, victim, TO_ROOM );
         act( AT_SOCIAL, social->char_no_arg, ch, arg, victim, TO_CHAR );
      }
   }
   else if( victim == ch )
   {
      if( adj && adj[0] != '\0' )
      {
         if( IS_SET( social->flags, SOC_VISUAL ) )
            act( AT_SOCIAL, social->others_auto, ch, adj, victim, TO_CANSEE );
         else
            act( AT_SOCIAL, social->others_auto, ch, adj, victim, TO_ROOM );
         act( AT_SOCIAL, social->char_auto, ch, adj, victim, TO_CHAR );
      }
      else
      {
         if( IS_SET( social->flags, SOC_VISUAL ) )
            act( AT_SOCIAL, social->others_auto, ch, social->adj, victim, TO_CANSEE );
         else
            act( AT_SOCIAL, social->others_auto, ch, social->adj, victim, TO_ROOM );
         act( AT_SOCIAL, social->char_auto, ch, social->adj, victim, TO_CHAR );
      }
   }
   else
   {
      if( adj && adj[0] != '\0' )
      {
         act( AT_SOCIAL, social->others_found, ch, adj, victim, TO_NOTVICT );
         act( AT_SOCIAL, social->char_found, ch, adj, victim, TO_CHAR );
         act( AT_SOCIAL, social->vict_found, ch, adj, victim, TO_VICT );
      }
      else
      {
         act( AT_SOCIAL, social->others_found, ch, social->adj, victim, TO_NOTVICT );
         act( AT_SOCIAL, social->char_found, ch, social->adj, victim, TO_CHAR );
         act( AT_SOCIAL, social->vict_found, ch, social->adj, victim, TO_VICT );
      }
      /*
       * Make mobs react accordingly... 
       * * Don't respond to other mobs, could get into infinte loop.
       */
      if( IS_NPC( victim ) && !IS_NPC( ch ) && IS_SET( victim->xflags, PART_HANDS ) && can_see( victim, ch ) )
      {
         if( IS_SET( social->flags, SOC_POLITE ) )
         {
            switch ( number_range( 1, 3 ) )
            {
               case 1:
                  check_social( victim, "smile", ch->name );
                  break;
               case 2:
                  check_social( victim, "bow", ch->name );
                  break;
               case 3:
                  check_social( victim, "pat", ch->name );
                  break;
            }
         }
         else if( IS_SET( social->flags, SOC_SEXUAL ) )
         {
            if( victim->sex == ch->sex )
               switch ( number_range( 1, 6 ) )
               {
                  case 1:
                     check_social( victim, "slap", ch->name );
                     break;
                  case 2:
                     check_social( victim, "gasp", ch->name );
                     break;
                  case 3:
                     check_social( victim, "puke", ch->name );
                     break;
                  case 4:
                     check_social( victim, "edge", ch->name );
                     break;
                  case 5:
                     check_social( victim, "smack", ch->name );
                     break;
                  case 6:
                     check_social( victim, "thwap", ch->name );
                     break;
               }
            else
               switch ( number_range( 1, 5 ) )
               {
                  case 1:
                     check_social( victim, "kiss", ch->name );
                     break;
                  case 2:
                     check_social( victim, "hug", ch->name );
                     break;
                  case 3:
                     check_social( victim, "smile", ch->name );
                     break;
                  case 4:
                     check_social( victim, "cuddle", ch->name );
                     break;
                  case 5:
                     check_social( victim, "snuggle", ch->name );
                     break;
               }
         }
         else if( IS_SET( social->flags, SOC_RUDE ) )
         {
            switch ( number_range( 1, 3 ) )
            {
               case 1:
                  check_social( victim, "slap", ch->name );
                  break;
               case 2:
                  check_social( victim, "smack", ch->name );
                  break;
               case 3:
                  check_social( victim, "growl", ch->name );
                  break;
            }
         }
         else if( IS_SET( social->flags, SOC_ANNOYING ) )
         {
            switch ( number_range( 1, 3 ) )
            {
               case 1:
                  check_social( victim, "eyebrow", ch->name );
                  break;
               case 2:
                  check_social( victim, "poke", ch->name );
                  break;
               case 3:
                  check_social( victim, "smirk", ch->name );
                  break;
            }
         }
      }
   }

   /*
    * If they've set outputsuffix, spam it here! -- Scion 
    */

   if( !IS_NPC( ch ) && ch->pcdata->outputsuffix )
   {
      send_to_char( ch->pcdata->outputsuffix, ch );
      send_to_char( "\r\n", ch );
   }

   return TRUE;
}

/*
 * Alias Command Parser
 * June 24, 1999 - Fixed rather annoying wait state bug in alias parser - Justice@AaernMUD
 */
bool check_alias( CHAR_DATA * ch, char *command, char *argument )
{
   ALIAS_DATA *alias;
   ALIAS_QUEUE *queue;
   ALIAS_QUEUE *new_queue;
   ALIAS_QUEUE *first;
   char buf[MAX_STRING_LENGTH];
   int str = 0;
   int i = 0;
   int aliaslimit = 0;  /* To prevent recursive alias stacking -- Scion */

   if( IS_NPC( ch ) )
      return FALSE;

   for( alias = ch->pcdata->first_alias; alias; alias = alias->next )
   {
      if( !str_cmp( alias->name, command ) )
         break;
   }

   if( !alias )
      return FALSE;

   queue = ch->pcdata->alias_queue;
   first = ch->pcdata->alias_queue;

   while( queue && queue->next )
      queue = queue->next;

   buf[0] = '\0';

   for( str = 0; alias->alias[str] != '\0'; str++ )
   {
      aliaslimit++;
      if( aliaslimit >= 1000 )
      {
         sprintf( buf, "Alias limit reached on character %s!", ch->name );
         ch->pcdata->alias_queue = NULL;
         return TRUE;
      }

      if( alias->alias[str] == ';' && strlen( buf ) > 0 )
      {

         if( sysdata.alias_wait != -1 )
            WAIT_STATE( ch, sysdata.alias_wait + ch->wait );
         else
            WAIT_STATE( ch, 4 + ch->wait );

         /*
          * Add command to the alias queue -- Scion 
          */
         CREATE( new_queue, ALIAS_QUEUE, 1 );
         new_queue->next = NULL;
         new_queue->cmd = str_dup( buf );
         i = 0;

         if( queue )
         {
            queue->next = new_queue;
            queue = queue->next;
         }
         else
         {
            queue = new_queue;
            first = queue;
         }
      }
      /*
       * else if (alias->alias[str]=='%') {} 
       */
      else
      {
         buf[i] = alias->alias[str];
         buf[i + 1] = '\0';
         i++;
      }
   }
   /*
    * Add the last command to the alias queue -- Scion 
    */
   CREATE( new_queue, ALIAS_QUEUE, 1 );
   new_queue->next = NULL;
   new_queue->cmd = str_dup( buf );
   i = 0;

   if( queue )
   {
      queue->next = new_queue;
      queue = queue->next;
   }
   else
   {
      queue = new_queue;
      first = queue;
   }
   ch->pcdata->alias_queue = first;
   return TRUE;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
   if( *arg == '\0' )
      return FALSE;

   for( ; *arg != '\0'; arg++ )
   {
      if( !isdigit( *arg ) )
         return FALSE;
   }

   return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
   char *pdot;
   int number;

   for( pdot = argument; *pdot != '\0'; pdot++ )
   {
      if( *pdot == '.' )
      {
         *pdot = '\0';
         number = atoi( argument );
         *pdot = '.';
         strcpy( arg, pdot + 1 );
         return number;
      }
   }

   strcpy( arg, argument );
   return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
   char cEnd;
   sh_int count;

   count = 0;

   while( isspace( *argument ) )
      argument++;

   cEnd = ' ';
   if( *argument == '\'' || *argument == '"' )
      cEnd = *argument++;

   while( *argument != '\0' || ++count >= 255 )
   {
      if( *argument == cEnd )
      {
         argument++;
         break;
      }
      *arg_first = LOWER( *argument );
      arg_first++;
      argument++;
   }
   *arg_first = '\0';

   while( isspace( *argument ) )
      argument++;

   return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.  Delimiters = { ' ', '-' }
 */
char *one_argument2( char *argument, char *arg_first )
{
   char cEnd;
   sh_int count;

   count = 0;

   while( isspace( *argument ) )
      argument++;

   cEnd = ' ';
   if( *argument == '\'' || *argument == '"' )
      cEnd = *argument++;

   while( *argument != '\0' || ++count >= 255 )
   {
      if( *argument == cEnd || *argument == '-' )
      {
         argument++;
         break;
      }
      *arg_first = LOWER( *argument );
      arg_first++;
      argument++;
   }
   *arg_first = '\0';

   while( isspace( *argument ) )
      argument++;

   return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes. Retains case.
 */
char *one_argument_retain_case( char *argument, char *arg_first )
{
   char cEnd;
   sh_int count;

   count = 0;

   while( isspace( *argument ) )
      argument++;

   cEnd = ' ';
   if( *argument == '\'' || *argument == '"' )
      cEnd = *argument++;

   while( *argument != '\0' || ++count >= 255 )
   {
      if( *argument == cEnd )
      {
         argument++;
         break;
      }
      *arg_first = *argument;
      arg_first++;
      argument++;
   }
   *arg_first = '\0';

   while( isspace( *argument ) )
      argument++;

   return argument;
}


void do_timecmd( CHAR_DATA * ch, char *argument )
{
   struct timeval stime;
   struct timeval etime;
   static bool timing;
   extern CHAR_DATA *timechar;
   char arg[MAX_INPUT_LENGTH];

   send_to_char( "Timing\n\r", ch );
   if( timing )
      return;
   one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "No command to time.\n\r", ch );
      return;
   }
   if( !str_cmp( arg, "update" ) )
   {
      if( timechar )
         send_to_char( "Another person is already timing updates.\n\r", ch );
      else
      {
         timechar = ch;
         send_to_char( "Setting up to record next update loop.\n\r", ch );
      }
      return;
   }
   set_char_color( AT_PLAIN, ch );
   send_to_char( "Starting timer.\n\r", ch );
   timing = TRUE;
   gettimeofday( &stime, NULL );
   interpret( ch, argument, FALSE );
   gettimeofday( &etime, NULL );
   timing = FALSE;
   set_char_color( AT_PLAIN, ch );
   send_to_char( "Timing complete.\n\r", ch );
   subtract_times( &etime, &stime );
   ch_printf( ch, "Timing took %d.%06d seconds.\n\r", etime.tv_sec, etime.tv_usec );
   return;
}

void start_timer( struct timeval *stime )
{
   if( !stime )
   {
      bug( "Start_timer: NULL stime.", 0 );
      return;
   }
   gettimeofday( stime, NULL );
   return;
}

time_t end_timer( struct timeval * stime )
{
   struct timeval etime;

   /*
    * Mark etime before checking stime, so that we get a better reading.. 
    */
   gettimeofday( &etime, NULL );
   if( !stime || ( !stime->tv_sec && !stime->tv_usec ) )
   {
      bug( "End_timer: bad stime.", 0 );
      return 0;
   }
   subtract_times( &etime, stime );
   /*
    * stime becomes time used 
    */
   *stime = etime;
   return ( etime.tv_sec * 1000000 ) + etime.tv_usec;
}

void send_timer( struct timerset *vtime, CHAR_DATA * ch )
{
   struct timeval ntime;
   int carry;

   if( vtime->num_uses == 0 )
      return;
   ntime.tv_sec = vtime->total_time.tv_sec / vtime->num_uses;
   carry = ( vtime->total_time.tv_sec % vtime->num_uses ) * 1000000;
   ntime.tv_usec = ( vtime->total_time.tv_usec + carry ) / vtime->num_uses;
   ch_printf( ch, "Has been used %d times this boot.\n\r", vtime->num_uses );
   ch_printf( ch, "Time (in secs): min %d.%0.6d; avg: %d.%0.6d; max %d.%0.6d"
              "\n\r", vtime->min_time.tv_sec, vtime->min_time.tv_usec, ntime.tv_sec,
              ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec );
   return;
}

void update_userec( struct timeval *time_used, struct timerset *userec )
{
   userec->num_uses++;
   if( !timerisset( &userec->min_time ) || timercmp( time_used, &userec->min_time, < ) )
   {
      userec->min_time.tv_sec = time_used->tv_sec;
      userec->min_time.tv_usec = time_used->tv_usec;
   }
   if( !timerisset( &userec->max_time ) || timercmp( time_used, &userec->max_time, > ) )
   {
      userec->max_time.tv_sec = time_used->tv_sec;
      userec->max_time.tv_usec = time_used->tv_usec;
   }
   userec->total_time.tv_sec += time_used->tv_sec;
   userec->total_time.tv_usec += time_used->tv_usec;
   while( userec->total_time.tv_usec >= 1000000 )
   {
      userec->total_time.tv_sec++;
      userec->total_time.tv_usec -= 1000000;
   }
   return;
}

/*
 *  This function checks the command against the command flags to make
 *  sure they can use the command online.  This allows the commands to be
 *  edited online to allow or disallow certain situations.  May be an idea
 *  to rework this so we can edit the message sent back online, as well as
 *  maybe a crude parsing language so we can add in new checks online without
 *  haveing to hard-code them in.     -- Shaddai   August 25, 1997
 */

void do_alias( CHAR_DATA * ch, char *argument )
{
   ALIAS_DATA *alias = NULL;
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs can't use aliases.\n\r", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      if( !ch->pcdata->first_alias )
      {
         send_to_char( "You don't have any aliases.\n\r", ch );
         return;
      }
      else
      {
         int count = 0;

         send_to_char( "You have the following aliases:\n\r", ch );

         for( alias = ch->pcdata->first_alias; alias; alias = alias->next, count++ )
            ch_printf( ch, "%s     %s\n\r", alias->name, alias->alias );

         ch_printf( ch, "You have %d alias%s.\n\r", count, count == 1 ? "" : "es" );
         return;
      }
   }

   argument = one_argument( argument, arg );

   for( alias = ch->pcdata->first_alias; alias; alias = alias->next )
      if( !str_cmp( arg, alias->name ) )
         break;
   if( !alias )
   {
      if( argument[0] == '\0' )
      {
         ch_printf( ch, "You don't have any alias called %s.\n\r", arg );
         return;
      }
      CREATE( alias, ALIAS_DATA, 1 );
      alias->name = strdup( arg );
      alias->alias = strdup( argument );
      LINK( alias, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );

      send_to_char( "Alias Added:\n\r", ch );
      ch_printf( ch, "%s     %s\n\r", alias->name, alias->alias );
      return;
   }
   if( argument[0] == '\0' )
   {
      UNLINK( alias, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
      DISPOSE( alias );
      send_to_char( "Alias deleted.\n\r", ch );
      return;
   }

   send_to_char( "Alias Changed.\n\r", ch );
   alias->alias = strdup( argument );
   ch_printf( ch, "%s     %s\n\r", alias->name, alias->alias );
   return;
}
