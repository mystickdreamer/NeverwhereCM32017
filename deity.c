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
 *                           Deity handling module                          *
 ****************************************************************************/

/*Put together by Rennard for Realms of Despair.  Brap on...*/
/* Converted to work on Rogue Winds by Scion. When in Rome... */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


DEITY_DATA *first_deity;
DEITY_DATA *last_deity;

/* local routines */

void fread_deity args( ( DEITY_DATA * deity, FILE * fp ) );
bool load_deity_file args( ( char *deityfile ) );
void write_deity_list args( ( void ) );
int get_risflag args( ( char *flag ) );
int get_npc_race args( ( char *type ) );

/* Get pointer to deity structure from deity name */

DEITY_DATA *get_deity( char *name )
{
   DEITY_DATA *deity;
   for( deity = first_deity; deity; deity = deity->next )
      if( !str_cmp( name, deity->name ) )
         return deity;
   return NULL;
}

void write_deity_list(  )
{
   DEITY_DATA *tdeity;
   FILE *fpout;
   char filename[256];

   sprintf( filename, "%s%s", DEITY_DIR, DEITY_LIST );
   fclose( fpReserve );
   fpout = fopen( filename, "w" );
   if( !fpout )
      bug( "FATAL: cannot open deity.lst for writing!\n\r", 0 );
   else
   {
      for( tdeity = first_deity; tdeity; tdeity = tdeity->next )
         fprintf( fpout, "%s\n", tdeity->filename );
      fprintf( fpout, "$\n" );
      fclose( fpout );
   }
   fpReserve = fopen( NULL_FILE, "r" );
}

/* Save a deity's data to its data file */

void save_deity( DEITY_DATA * deity )
{
   FILE *fp;
   char filename[256];
   char buf[MAX_STRING_LENGTH];

   if( !deity )
   {
      bug( "save_deity: null deity pointer!", 0 );
      return;
   }

   if( !deity->filename || deity->filename[0] == '\0' )
   {
      sprintf( buf, "save_deity: %s has no filename", deity->name );
      bug( buf, 0 );
      return;
   }

   sprintf( filename, "%s%s", DEITY_DIR, deity->filename );

   fclose( fpReserve );
   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_deity: fopen", 0 );
      perror( filename );
   }
   else
   {
      fprintf( fp, "#DEITY\n" );
      fprintf( fp, "Index			%d\n", deity->index );
      fprintf( fp, "Filename		%s~\n", deity->filename );
      fprintf( fp, "Name		%s~\n", deity->name );
      fprintf( fp, "Description	%s~\n", deity->description );
      fprintf( fp, "Message		%s~\n", deity->message );
      fprintf( fp, "Cost		%d\n", deity->cost );
      fprintf( fp, "Count		%d\n", deity->count );
      fprintf( fp, "Archmage		%s~\n", deity->archmage );
      fprintf( fp, "ArchLvl		%d\n", deity->arch_lvl );
      fprintf( fp, "Color		%s~\n", deity->color );
      fprintf( fp, "End\n\n" );
      fprintf( fp, "#END\n" );
      fclose( fp );
   }
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}

/* Read in actual deity data */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

void fread_deity( DEITY_DATA * deity, FILE * fp )
{
   char *word;
   bool fMatch;

   deity->index = -1;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Archmage", deity->archmage, fread_string( fp ) );
            KEY( "ArchLvl", deity->arch_lvl, fread_number( fp ) );
            break;

         case 'C':
            KEY( "Color", deity->color, fread_string( fp ) );
            KEY( "Cost", deity->cost, fread_number( fp ) );
            KEY( "Count", deity->count, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Description", deity->description, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !deity->name )
                  deity->name = STRALLOC( "" );
               if( !deity->description )
                  deity->description = STRALLOC( "" );
               return;
            }
            break;

         case 'F':
            KEY( "Filename", deity->filename, fread_string_nohash( fp ) );
            break;

         case 'I':
            // KEY( "Index",  deity->index,     fread_number( fp ) );
            break;

         case 'M':
            KEY( "Message", deity->message, fread_string( fp ) );

         case 'N':
            KEY( "Name", deity->name, fread_string( fp ) );
            break;
      }

      if( !fMatch )
      {
/*	    sprintf( buf, "Fread_deity: no match: %s", word );
	    bug( buf, 0 ); */
      }
   }
}

/* Load a deity file */

bool load_deity_file( char *deityfile )
{
   char filename[256];
   DEITY_DATA *deity;
   DEITY_DATA *tdeity;
   FILE *fp;
   bool found;

   found = FALSE;
   sprintf( filename, "%s%s", DEITY_DIR, deityfile );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_deity_file: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "DEITY" ) )
         {
            CREATE( deity, DEITY_DATA, 1 );
            fread_deity( deity, fp );
            LINK( deity, first_deity, last_deity, next, prev );

            /*
             * Give each deity a unique index number for spells to refer to it by 
             */
            for( tdeity = first_deity; tdeity; tdeity = tdeity->next )
            {
               deity->index++;
               if( tdeity->index != deity->index )
                  break;
            }

            found = TRUE;
            break;
         }
         else
         {
            char buf[MAX_STRING_LENGTH];
            sprintf( buf, "Load_deity_file: bad section: %s.", word );
            bug( buf, 0 );
            break;
         }
      }
      fclose( fp );
   }

   return found;
}

/* Load in all the deity files */

void load_deity(  )
{
   FILE *fpList;
   char *filename;
   char deitylist[256];
   char buf[MAX_STRING_LENGTH];

   first_deity = NULL;
   last_deity = NULL;

   log_string( "Loading talents..." );

   sprintf( deitylist, "%s%s", DEITY_DIR, DEITY_LIST );
   if( ( fpList = fopen( deitylist, "r" ) ) == NULL )
   {
      perror( deitylist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      log_string( filename );
      if( filename[0] == '$' )
         break;
      if( !load_deity_file( filename ) )
      {
         sprintf( buf, "Cannot load talent file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   log_string( " Done talents " );
   return;
}

void do_setdeity( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   DEITY_DATA *deity;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;

      case SUB_RESTRICTED:
         send_to_char( "You cannot do this while in another command.\n\r", ch );
         return;

      case SUB_DEITYDESC:
         deity = ch->dest_buf;
         STRFREE( deity->description );
         deity->description = copy_buffer( ch );
         stop_editing( ch );
         save_deity( deity );
         ch->substate = ch->tempnum;
         return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == '\0' )
   {
      send_to_char( "Usage: settalent <talent> <field> <toggle>\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( "filename name cost\n\r", ch );
      return;
   }

   deity = get_deity( arg1 );
   if( !deity )
   {
      send_to_char( "No such talent.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      STRFREE( deity->name );
      deity->name = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_deity( deity );
      return;
   }

   if( !str_cmp( arg2, "filename" ) )
   {
      DISPOSE( deity->filename );
      deity->filename = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      save_deity( deity );
      write_deity_list(  );
      return;
   }

   if( !str_cmp( arg2, "description" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_DEITYDESC;
      ch->dest_buf = deity;
      start_editing( ch, deity->description );
      return;
   }

   if( !str_cmp( arg2, "cost" ) )
   {
      deity->cost = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_deity( deity );
      return;
   }

   if( !str_cmp( arg2, "archlvl" ) )
   {
      deity->arch_lvl = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_deity( deity );
      return;
   }

   if( !str_cmp( arg2, "count" ) )
   {
      deity->count = atoi( argument );
      send_to_char( "Done.\r\n", ch );
      save_deity( deity );
      return;
   }

   if( !str_cmp( arg2, "message" ) )
   {
      STRFREE( deity->message );
      deity->message = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_deity( deity );
      return;
   }

   if( !str_cmp( arg2, "archmage" ) )
   {
      STRFREE( deity->archmage );
      deity->archmage = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_deity( deity );
      return;
   }

   if( !str_cmp( arg2, "color" ) )
   {
      STRFREE( deity->color );
      deity->color = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      save_deity( deity );
      return;
   }

   ch_printf( ch, "Done.\n\r" );
   save_deity( deity );
   return;
}


void do_showdeity( CHAR_DATA * ch, char *argument )
{
   DEITY_DATA *deity;
   int i;

   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }

   if( argument[0] == '\0' )
   {
      send_to_char( "Usage: showtalent <talent>\n\r", ch );
      return;
   }

   deity = get_deity( argument );
   if( !deity )
   {
      send_to_char( "No such talent.\n\r", ch );
      return;
   }

   ch_printf( ch, "&wTalent: %s\n\rFilename: %s\n\rDescription:\n\r%s\n\r", deity->name, deity->filename,
              deity->description );
   ch_printf( ch, "Archmage: %s  Archmage Level: %d\n\r", deity->archmage, deity->arch_lvl );
   ch_printf( ch, "Index: %d    Cost: %d\r\n", deity->index, deity->cost );
   ch_printf( ch, "Message: %s\n\r", deity->message );
   ch_printf( ch, "Color: %s\n\r", deity->color );
   ch_printf( ch, "Total levels: %d\n\r", deity->count );
   ch_printf( ch, "Ranks:\n\r" );
   for( i = 0; i < 12; i++ )
      ch_printf( ch, "%d: %s\n\r", i * 10 + 10, talent_rank[deity->index][i] );
}

void do_makedeity( CHAR_DATA * ch, char *argument )
{
   char filename[256];
   DEITY_DATA *deity;
   bool found;

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: maketalent <talent name>\n\r", ch );
      return;
   }

   found = FALSE;
   sprintf( filename, "%s%s", DEITY_DIR, strlower( argument ) );
   CREATE( deity, DEITY_DATA, 1 );
   LINK( deity, first_deity, last_deity, next, prev );
   deity->name = STRALLOC( argument );
   deity->description = STRALLOC( "" );
   deity->filename = str_dup( filename );
   write_deity_list(  );
   save_deity( deity );

}

void do_deities( CHAR_DATA * ch, char *argument )
{
   DEITY_DATA *deity;
   int count = 0;

   send_to_pager_color( "&gFor detailed information on a Talent, try 'help <talent>'\n\r", ch );
   send_to_pager_color( "Talent             Cost\n\r", ch );
   for( deity = first_deity; deity; deity = deity->next )
   {
      pager_printf_color( ch, "&G%-10s  &g%19d\n\r", deity->name, deity->cost );
      count++;
   }
   if( !count )
   {
      send_to_pager_color( "&gThere are no Talents on this world.\n\r", ch );
      return;
   }
   return;
}

void do_archmages( CHAR_DATA * ch, char *argument )
{
   DEITY_DATA *talent;

   for( talent = first_deity; talent; talent = talent->next )
   {
      ch_printf( ch, "%s is the Archmage of %s.\n\r", talent->archmage, talent->name );
   }
}
