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
 *                            Ban module by Shaddai                         *
 ****************************************************************************/  
   
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"
   
/* Local functions */ 
void fread_ban args( ( FILE * fp ) );
bool check_expire args( ( BAN_DATA * ban ) );
void dispose_ban args( ( BAN_DATA * ban ) );
void free_ban args( ( BAN_DATA * pban ) );

/* Global Variables */ 
   BAN_DATA * first_ban;
BAN_DATA * last_ban;

/*
 * Load all those nasty bans up :)
 * 	Shaddai
 * Nasty is right <shudder>
 * Ban code stripped down and made cleaner -Keolah Aug 20 2001
 */ 
void load_banlist( void )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   FILE * fp;
   bool fMatch;
   if( !( fp = fopen( SYSTEM_DIR BAN_LIST, "r" ) ) )
      
   {
      bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
      perror( BAN_LIST );
      return;
   }
   for( ;; )
      
   {
      word = feof( fp ) ? "END" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
         
      {
         case 'E':
            if( !str_cmp( word, "END" ) )   /*File should always contain END */
               
            {
               fclose( fp );
               log_string( "Done." );
               return;
            }
         case 'S':
            if( !str_cmp( word, "SITE" ) )
               
            {
               fread_ban( fp );
               fMatch = TRUE;
            }
            break;
      }
      if( !fMatch )
         
      {
         sprintf( buf, "Load_banlist: no match: %s", word );
         bug( buf, 0 );
      } /* End of switch statement */
   } /* End of for loop */
}


/*
 * Load up one class or one race ban structure.
 */ 
void fread_ban( FILE * fp )
{
   BAN_DATA * pban;
   int i = 0;
   CREATE( pban, BAN_DATA, 1 );
   pban->name = fread_string_nohash( fp );
   pban->user = NULL;
   
      /*
       * was level 
       */ fread_number( fp );
   pban->duration = fread_number( fp );
   pban->unban_date = fread_number( fp );
   pban->prefix = fread_number( fp );
   pban->suffix = fread_number( fp );
   pban->ban_by = fread_string_nohash( fp );
   pban->ban_time = fread_string_nohash( fp );
   for( i = 0; i < strlen( pban->name ); i++ )
   {
      if( pban->name[i] == '@' )
         
      {
         char *temp;
         char *temp2;
         temp = str_dup( pban->name );
         temp[i] = '\0';
         temp2 = &pban->name[i + 1];
         DISPOSE( pban->name );
         pban->name = str_dup( temp2 );
         pban->user = str_dup( temp );
         DISPOSE( temp );
      }
   } LINK( pban, first_ban, last_ban, next, prev );
   return;
}
void save_banlist( void )
{
   BAN_DATA * pban;
   FILE * fp;
   fclose( fpReserve );
   if( !( fp = fopen( SYSTEM_DIR BAN_LIST, "w" ) ) )
      
   {
      bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
      perror( BAN_LIST );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }
   
      /*
       * Print out all the site bans 
       */ 
      for( pban = first_ban; pban; pban = pban->next )
   {
      fprintf( fp, "SITE\n" );
      fprintf( fp, "%s~\n", pban->name );
      fprintf( fp, "%d %d %d %d %d\n", pban->duration, pban->unban_date, pban->prefix, pban->suffix, pban->warn );
      fprintf( fp, "%s~\n%s~\n", pban->ban_by, pban->ban_time );
   }
   fprintf( fp, "END\n" );  /* File must have an END even if empty */
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}


/*
 * The main command for ban, lots of arguments so be carefull what you
 * change here.		Shaddai
 * Not anymore ;) -keo
 */ 
void do_ban( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int time;
   if( IS_NPC( ch ) ) /* Don't want mobs banning sites ;) */
      
   {
      send_to_char( "Monsters are too dumb to do that!\n\r", ch );
      return;
   }
   if( !ch->desc ) /* No desc means no go :) */
      
   {
      bug( "do_ban: no descriptor", 0 );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( !str_cmp( arg1, "list" ) )
   {
      show_bans( ch );
      return;
   }
   if( !str_cmp( arg1, "clear" ) )
   {
      return;
   }
   
      /*
       * Do we have a time duration for the ban? 
       */ 
      if( arg2[0] != '\0' && is_number( arg2 ) )
      time = atoi( arg2 );
   
   else
      time = -1;
   
      /*
       * -1 is default, but no reason the time should be greater than 1000
       * * or less than 1, after all if it is greater than 1000 you are talking
       * * around 3 years.
       */ 
      if( time != -1 && ( time < 1 || time > 1000 ) )
      
   {
      send_to_char( "Time value is -1 (forever) or from 1 to 1000.\n\r", ch );
      return;
   }
   add_ban( ch, arg1, time );
   return;
}


/*
 *  This actually puts the new ban into the proper linked list and
 *  initializes its data.  Shaddai
 */ 
int add_ban( CHAR_DATA * ch, char *arg1, int time ) 
{
   char arg[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   BAN_DATA * pban, *temp;
   struct tm *tms;
   char *name;
   bool prefix = FALSE, suffix = FALSE;
   one_argument( arg1, arg );
   smash_tilde( arg ); /* Make sure the immortals don't put a ~ in it. */
   if( arg[0] == '\0' )
      return 0;
   name = arg;
   if( name[0] == '*' )
      
   {
      prefix = TRUE;
      name++;
   }
   if( name[strlen( name ) - 1] == '*' )
      
   {
      suffix = TRUE;
      name[strlen( name ) - 1] = '\0';
   }
   for( temp = first_ban; temp; temp = temp->next )
      
   {
      if( !str_cmp( temp->name, name ) )
         
      {
         if( ( prefix && temp->prefix )  &&( suffix && temp->suffix ) )
            
         {
            send_to_char( "That entry already exists.\n\r", ch );
            return 0;
         }
         
         else
            
         {
            temp->suffix = suffix;
            temp->prefix = prefix;
            temp->warn = TRUE;
            sprintf( buf, "%24.24s", ctime( &current_time ) );
            temp->ban_time = str_dup( buf );
            if( temp->ban_by )
               DISPOSE( temp->ban_by );
            temp->ban_by = str_dup( ch->name );
            send_to_char( "Updated entry.\n\r", ch );
            return 1;
         }
      }
   }
   CREATE( pban, BAN_DATA, 1 );
   pban->ban_by = str_dup( ch->name );
   pban->suffix = suffix;
   pban->prefix = prefix;
   pban->name = str_dup( name );
   LINK( pban, first_ban, last_ban, next, prev );
   sprintf( buf, "%24.24s", ctime( &current_time ) );
   pban->ban_time = str_dup( buf );
   if( time > 0 )
   {
      pban->duration = time;
      tms = localtime( &current_time );
      tms->tm_mday += time;
      pban->unban_date = mktime( tms );
   }
   
   else
   {
      pban->duration = -1;
      pban->unban_date = -1;
   }
   save_banlist(  );
   if( pban->duration > 0 )
      
   {
      if( !pban->user )
         ch_printf( ch, "%s banned for %d days.\n\r", pban->name, pban->duration );
      
      else
         ch_printf( ch, "%s@%s banned for %d days.\n\r", pban->user, pban->name, pban->duration );
   }
   
   else
      
   {
      if( !pban->user )
         ch_printf( ch, "%s banned forever.\n\r", pban->name );
      
      else
         ch_printf( ch, "%s@%s banned forever.\n\r", pban->user, pban->name );
   }
   return 1;
}


/*
 * Print the bans out to the screen.  Shaddai
 */ 
void show_bans( CHAR_DATA * ch )
{
   BAN_DATA * pban;
   int bnum;
   set_pager_color( AT_IMMORT, ch );
   send_to_pager( "Banned sites:\n\r", ch );
   send_to_pager( "[ #] Time                     By              For   Site\n\r", ch );
   send_to_pager( "---- ------------------------ --------------- ---- ---------------\n\r", ch );
   pban = first_ban;
   set_pager_color( AT_PLAIN, ch );
   for( bnum = 1; pban; pban = pban->next, bnum++ )
      
   {
      pager_printf( ch, "[%2d] %-24s %-15s %4d  %c%s%c\n\r", bnum, pban->ban_time, pban->ban_by, pban->duration, 
                     ( pban->prefix ) ? '*' : ' ', pban->name, ( pban->suffix ) ? '*' : ' ' );
   }
   return;
}


/*
 * Check for totally banned sites.  Need this because we don't have a
 * char struct yet.  Shaddai
 */ 
   bool check_total_bans( DESCRIPTOR_DATA * d )
{
   BAN_DATA * pban;
   char new_host[MAX_STRING_LENGTH];
   int i;
   for( i = 0; i < ( int )strlen( d->host ); i++ )
      new_host[i] = LOWER( d->host[i] );
   new_host[i] = '\0';
   for( pban = first_ban; pban; pban = pban->next )
   {
      if( pban->user && str_cmp( d->user, pban->user ) )
         continue;
      if( pban->prefix && pban->suffix && strstr( pban->name, new_host ) )
         
      {
         if( check_expire( pban ) )
            
         {
            dispose_ban( pban );
            save_banlist(  );
            return FALSE;
         }
         
         else
            return TRUE;
      }
      if( pban->suffix && !str_suffix( pban->name, new_host ) )
         
      {
         if( check_expire( pban ) )
            
         {
            dispose_ban( pban );
            save_banlist(  );
            return FALSE;
         }
         
         else
            return TRUE;
      }
      if( pban->prefix && !str_prefix( pban->name, new_host ) )
         
      {
         if( check_expire( pban ) )
            
         {
            dispose_ban( pban );
            save_banlist(  );
            return FALSE;
         }
         
         else
            return TRUE;
      }
      if( !str_cmp( pban->name, new_host ) )
         
      {
         if( check_expire( pban ) )
            
         {
            dispose_ban( pban );
            save_banlist(  );
            return FALSE;
         }
         
         else
            return TRUE;
      }
   }
   return FALSE;
}


/* Check player logging in for multiplaying. Exclude imms and players with
   a PLR_MULTI flag. -- Scion */ 
   bool check_multi( CHAR_DATA * ch )
{
   DESCRIPTOR_DATA * d;
   int chars = 1;
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->character && d->character != ch 
           &&d->host == ch->desc->host  &&!d->character->pcdata->permissions  &&!xIS_SET( d->character->act, PLR_MULTI ) )
         chars++;
   }
   if( chars > sysdata.morts_allowed )
      return TRUE;
   return FALSE;
}


/*
 * The workhose, checks for bans on sites/classes and races. Shaddai
 */ 
   bool check_bans( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   BAN_DATA * pban;
   char new_host[MAX_STRING_LENGTH];
   int i;
   bool fMatch = FALSE;
   pban = first_ban;
   for( i = 0; i < ( int )( strlen( ch->desc->host ) ); i++ )
      new_host[i] = LOWER( ch->desc->host[i] );
   new_host[i] = '\0';
   for( ; pban; pban = pban->next )
   {
      if( pban->prefix && pban->suffix && strstr( pban->name, new_host ) )
         fMatch = TRUE;
      
      else if( pban->prefix && !str_suffix( pban->name, new_host ) )
         fMatch = TRUE;
      
      else if( pban->suffix && !str_prefix( pban->name, new_host ) )
         fMatch = TRUE;
      
      else if( !str_cmp( pban->name, new_host ) )
         fMatch = TRUE;
      if( fMatch && pban->user && str_cmp( pban->user, ch->desc->user ) )
         fMatch = FALSE;
      if( fMatch )
         
      {
         if( check_expire( pban ) )
            
         {
            dispose_ban( pban );
            save_banlist(  );
            return FALSE;
         }
         sprintf( buf, "%s logging in from site %s.", ch->name, ch->desc->host );
         log_string_plus( buf, LOG_WARN, 0 );
         return FALSE;
      }
   }
   return FALSE;
}

bool check_expire( BAN_DATA * pban ) 
{
   char buf[MAX_STRING_LENGTH];
   if( pban->unban_date < 0 )
      return FALSE;
   if( pban->unban_date <= current_time )
      
   {
      sprintf( buf, "%s ban has expired.", pban->name );
      log_string_plus( buf, LOG_WARN, 0 );
      return TRUE;
   }
   return FALSE;
}
void dispose_ban( BAN_DATA * pban ) 
{
   if( !pban )
      return;
   UNLINK( pban, first_ban, last_ban, next, prev );
   free_ban( pban );
   return;
}
void free_ban( BAN_DATA * pban ) 
{
   if( pban->name )
      DISPOSE( pban->name );
   if( pban->ban_time )
      DISPOSE( pban->ban_time );
   if( pban->user )
      DISPOSE( pban->user );
   if( pban->ban_by )
      DISPOSE( pban->ban_by );
   if( pban->ban_time )
      DISPOSE( pban->ban_time );
   DISPOSE( pban );
}


