/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *			   Wizard/god command module			    *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"


#define RESTORE_INTERVAL 21600

char *const save_flag[] = { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
   "unused", "get", "receive", "idle", "backup", "quitbackup", "r14", "r15",
   "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
   "r27", "r28", "r29", "r30", "r31"
};

char *const quest_flag[] = { "eternal_night", "tempest", "evil", "r4", "r5", "r6", "r7", "r8", "r9",
   "r10", "r11", "r12", "r13", "r14", "r15",
   "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",
   "r26", "r27", "r28", "r29", "r30", "r31"
};

char *const permit_flags[] = {
   "admin", "help", "build", "hibuild", "security", "talent", "item",
   "misc", "channel", "mob", "r11", "prog", "r13", "r14", "r15", "r16",
   "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
   "r27", "r28", "r29", "r30", "r31"
};

/* from act_comm.c */
void drop_artifacts( CHAR_DATA * ch, OBJ_DATA * obj );

/* from comm.c */
bool write_to_descriptor args( ( int desc, char *txt, int length ) );
bool check_parse_name args( ( char *name, bool newchar ) );

/* from boards.c */
void note_attach( CHAR_DATA * ch );

/* from build.c */
int get_partflag( char *flag );
int get_risflag( char *flag );
int get_defenseflag( char *flag );
int get_attackflag( char *flag );

/* from tables.c */
void write_race_file( int ra );

void save_world( CHAR_DATA * ch );


/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location args( ( CHAR_DATA * ch, char *arg ) );
void save_banlist args( ( void ) );
void close_area args( ( AREA_DATA * pArea ) );

int get_color( char *argument ); /* function proto */

void sort_reserved args( ( RESERVE_DATA * pRes ) );

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern char *const ex_flags[];
extern char *const mprog_flags[];
extern char *const fur_pos[];

int get_permit_flag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( permit_flags ) / sizeof( permit_flags[0] ) ); x++ )
      if( !str_cmp( flag, permit_flags[x] ) )
         return x;
   return -1;
}

void do_permit( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int i;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on a mob.\n\r", ch );
      return;
   }

   if( arg2 == NULL || arg2[0] == '\0' )
   {
      ch_printf( ch, "Permissions: %s\r\n", flag_string( victim->pcdata->permissions, permit_flags ) );
      return;
   }
   i = get_permit_flag( arg2 );
   if( i < 0 || i > MAX_BITS )
   {
      send_to_char( "Unknown flag.\r\n", ch );
      return;
   }
   if( !IS_SET( victim->pcdata->permissions, 1 << i ) )
   {
      SET_BIT( victim->pcdata->permissions, 1 << i );
      ch_printf( ch, "You have given %s %s permission.\r\n", victim->name, arg2 );
      ch_printf( victim, "You have been given %s permission.\n\r", arg2 );
   }
   else
   {
      REMOVE_BIT( victim->pcdata->permissions, 1 << i );
      ch_printf( ch, "You have removed %s permission from %s.\n\r", victim->name, arg2 );
      ch_printf( victim, "You have lost %s permission!\n\r", arg2 );
   }
   return;
}

int get_saveflag( char *name )
{
   int x;

   for( x = 0; x < sizeof( save_flag ) / sizeof( save_flag[0] ); x++ )
      if( !str_cmp( name, save_flag[x] ) )
         return x;
   return -1;
}

int get_questflag( char *name )
{
   int x;

   for( x = 0; x < sizeof( quest_flag ) / sizeof( quest_flag[0] ); x++ )
      if( !str_cmp( name, quest_flag[x] ) )
         return x;
   return -1;
}

void do_saveall( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *vch;
   AREA_DATA *tarea;
   char filename[256];
   for( vch = first_char; vch; vch = vch->next )
   {
      if( !IS_NPC( vch ) )
      {
         save_char_obj( vch );
         set_pager_color( AT_PLAIN, ch );
      }
   }
   for( tarea = first_build; tarea; tarea = tarea->next )
   {
      if( !IS_SET( tarea->status, AREA_LOADED ) )
      {
         continue;
      }
      sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
      fold_area( tarea, filename, FALSE );
   }

   save_world( ch );
   do_arena( ch, "save" );

   send_to_char( "Done.\n\r", ch );

}

void do_otransfer( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *obj;

   if( ( obj = get_obj_world( ch, argument ) ) == NULL )
   {
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
      return;
   }


   if( obj->carried_by )
   {
      act( AT_YELLOW, "$p vanishes from your hands.", obj->carried_by, obj, NULL, TO_CHAR );
      obj_from_char( obj );
   }
   if( obj->in_obj )
   {
      obj_from_obj( obj );
   }
   if( obj->in_room )
   {
      obj_from_room( obj );
   }

   obj_to_char( obj, ch );
   act( AT_YELLOW, "$p appears in your hands.", ch, obj, NULL, TO_CHAR );
}


void do_wizhelp( CHAR_DATA * ch, char *argument )
{
   CMDTYPE *cmd;
   int col, hash, permit;

   col = 0;
   set_pager_color( AT_PLAIN, ch );

   for( permit = 0; permit < 31; permit++ )
   {
      if( !IS_SET( ch->pcdata->permissions, 1 << permit ) )
         continue;
      pager_printf( ch, "&g[&G%s &gpermission]&C\n\r", permit_flags[permit] );
      for( hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
            if( cmd->permit == 1 << permit )
            {
               pager_printf( ch, "%-12s", cmd->name );
               if( ++col % 6 == 0 )
                  send_to_pager( "\n\r", ch );
            }
      if( col % 6 != 0 )
         send_to_pager( "\n\r", ch );

   }
   if( col % 6 != 0 )
      send_to_pager( "\n\r", ch );
   return;

}


void do_bamfin( CHAR_DATA * ch, char *argument )
{
   char bamf[MAX_STRING_LENGTH];
   char newbamf[MAX_STRING_LENGTH];
   bool fHasName = FALSE;
   int i, a, b;

   smash_tilde( argument );

   if( strlen( argument ) > 120 )
   {
      send_to_char( "Your bamfin must be under 120 characters.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
      strcpy( argument, "$n arrives in a swirling mist." );

   strcpy( bamf, argument );

   a = 0;
   b = strlen( bamf );
   newbamf[0] = '\0';
   for( i = 0; i < b; i++ )
   {
      if( bamf[i] == '$' && bamf[i + 1] == 'n' )
      {
         fHasName = TRUE;
         if( strlen( newbamf ) == 0 )
            strcpy( newbamf, ch->name );
         else
            strcat( newbamf, ch->name );
         i++;
         a += strlen( ch->name );
      }
      else
      {
         newbamf[a] = bamf[i];
         a++;
      }
      newbamf[a + 1] = '\0';
   }

   if( fHasName == FALSE )
   {
      send_to_char( "Your bamfin must contain at least one $n for your name.\r\n", ch );
      return;
   }

   DISPOSE( ch->pcdata->bamfin );
   ch->pcdata->bamfin = str_dup( newbamf );
   send_to_char_color( "&YBamfin set.\n\r", ch );
}

void do_bamfout( CHAR_DATA * ch, char *argument )
{
   char bamf[MAX_STRING_LENGTH];
   char newbamf[MAX_STRING_LENGTH];
   bool fHasName = FALSE;
   int i, a, b;

   smash_tilde( argument );

   if( strlen( argument ) > 120 )
   {
      send_to_char( "Your bamfout must be under 120 characters.\r\n", ch );
      return;
   }

   if( argument[0] == '\0' )
      strcpy( argument, "$n leaves in a swirling mist." );

   strcpy( bamf, argument );

   a = 0;
   b = strlen( bamf );
   newbamf[0] = '\0';
   for( i = 0; i < b; i++ )
   {
      if( bamf[i] == '$' && bamf[i + 1] == 'n' )
      {
         fHasName = TRUE;
         if( strlen( newbamf ) == 0 )
            strcpy( newbamf, ch->name );
         else
            strcat( newbamf, ch->name );
         i++;
         a += strlen( ch->name );
      }
      else
      {
         newbamf[a] = bamf[i];
         a++;
      }
      newbamf[a + 1] = '\0';
   }

   if( fHasName == FALSE )
   {
      send_to_char( "Your bamfout must contain at least one $n for your name.\r\n", ch );
      return;
   }
   DISPOSE( ch->pcdata->bamfout );
   ch->pcdata->bamfout = str_dup( newbamf );
   send_to_char_color( "&YBamfout set.\n\r", ch );
}


void do_buildwalk( CHAR_DATA *ch, char *argument )
{

    if ( !IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) || IS_NPC(ch) )
    {
        send_to_char( "Huh?\r\n", ch );
        return;
    }

    if ( IS_SET( ch->pcdata->flags, PCFLAG_BUILDWALK ) )
    {
        REMOVE_BIT( ch->pcdata->flags, PCFLAG_BUILDWALK );

        send_to_char( "&GYou are no longer buildwalking.\r\n", ch );
        act( AT_GREEN, "&G$n is no longer buildwalking.", ch, NULL, NULL, TO_CANSEE );
    }
    else
    {
        SET_BIT( ch->pcdata->flags, PCFLAG_BUILDWALK );

        send_to_char( "&GYou are now buildwalking.\r\n", ch );
        act( AT_GREY, "&G$n is now buildwalking.", ch, NULL, NULL, TO_CANSEE );
        return;
    }
}


void do_rank( CHAR_DATA * ch, char *argument )
{

   if( get_char_worth( ch ) < 2000 )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage:  rank <string>.\n\r", ch );
      send_to_char( "   or:  rank none.\n\r", ch );
      return;
   }
   smash_tilde( argument );
   DISPOSE( ch->pcdata->rank );
   if( !str_cmp( argument, "none" ) )
      ch->pcdata->rank = str_dup( "" );
   else
      ch->pcdata->rank = str_dup( argument );
   send_to_char( "Ok.\n\r", ch );
   return;
}

void do_delay( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   int delay;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Syntax:  delay <victim> <# of rounds>\n\r", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "No such character online.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Mobiles are unaffected by lag.\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "For how long do you wish to delay them?\n\r", ch );
      return;
   }
   if( !str_cmp( arg, "none" ) )
   {
      send_to_char( "All character delay removed.\n\r", ch );
      victim->wait = 0;
      return;
   }
   delay = atoi( arg );
   if( delay < 1 )
   {
      send_to_char( "Pointless.  Try a positive number.\n\r", ch );
      return;
   }
   if( delay > 999 )
   {
      send_to_char( "You cruel bastard.  Just kill them.\n\r", ch );
      return;
   }
   WAIT_STATE( victim, delay * PULSE_VIOLENCE );
   ch_printf( ch, "You've delayed %s for %d rounds.\n\r", victim->name, delay );
   return;
}

void do_deny( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Deny whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   xSET_BIT( victim->act, PLR_DENY );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You are denied access!\n\r", victim );
   ch_printf( ch, "You have denied access to %s.\n\r", victim->name );
   do_quit( victim, "" );
   return;
}

void do_disconnect( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Disconnect whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( victim->desc == NULL )
   {
      act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
      return;
   }

   for( d = first_descriptor; d; d = d->next )
   {
      if( d == victim->desc )
      {
         close_socket( d, FALSE );
         send_to_char( "Ok.\n\r", ch );
         return;
      }
   }
   bug( "Do_disconnect: *** desc not found ***.", 0 );
   send_to_char( "Descriptor not found!\n\r", ch );
   return;
}

void do_fquit( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Force whom to quit?\n\r", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   send_to_char( "The MUD administrators force you to quit...\n\r", victim );
   do_quit( victim, "" );
   ch_printf( ch, "You have forced %s to quit.\n\r", victim->name );
   return;
}

void do_forceclose( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int desc;

   /*
    * this is a nasty and dangerous command , so , log it - shogar 
    */
   sprintf( arg, "Forceclose issued by %s on %s", ch->name, argument );
   log_string( arg );

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: forceclose <descriptor#>\n\r", ch );
      return;
   }

   desc = atoi( arg );
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->descriptor == desc )
      {
         /*
          * ummm might not be a good idea on switched
          * and polymorphed-shogar also beware extracted chars 
          */
         if( !d->character )
         {
            bug( "attempt to force close on already extracted char", 0 );
            return;
         }
         if( d->original || d->character->switched )
         {
            bug( "'%s' appears to be switched or polymorphed", arg );
            return;
         }
         close_socket( d, FALSE );
         send_to_char( "Ok.\n\r", ch );
         return;
      }
   }
   send_to_char( "Not found!\n\r", ch );
   return;
}

void echo_to_all( sh_int AT_COLOR, char *argument, sh_int tar )
{
   DESCRIPTOR_DATA *d;

   if( !argument || argument[0] == '\0' )
      return;

   for( d = first_descriptor; d; d = d->next )
   {
      /*
       * Added showing echoes to players who are editing, so they won't
       * miss out on important info like upcoming reboots. --Narn 
       */
      if( d->connected == CON_PLAYING || d->connected == CON_EDITING )
      {
         /*
          * This one is kinda useless except for switched.. 
          */
         if( tar == ECHOTAR_PC && IS_NPC( d->character ) )
            continue;
         set_char_color( AT_COLOR, d->character );
         send_to_char( argument, d->character );
         send_to_char( "\n\r", d->character );
      }
   }
   return;
}

void do_echo( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   sh_int color;
   int target;
   char *parg;

   set_char_color( AT_IMMORT, ch );

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
   if( !str_cmp( arg, "PC" ) || !str_cmp( arg, "player" ) )
      target = ECHOTAR_PC;
   else if( !str_cmp( arg, "imm" ) )
      target = ECHOTAR_IMM;
   else
   {
      target = ECHOTAR_ALL;
      argument = parg;
   }
   if( !color && ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );
   if( !color )
      color = AT_IMMORT;
   one_argument( argument, arg );
   echo_to_all( color, argument, target );
}

void echo_to_room( sh_int AT_COLOR, ROOM_INDEX_DATA * room, char *argument )
{
   CHAR_DATA *vic;

   for( vic = room->first_person; vic; vic = vic->next_in_room )
   {
      set_char_color( AT_COLOR, vic );
      send_to_char( argument, vic );
      send_to_char( "\n\r", vic );
   }
}

void do_recho( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   sh_int color;

   set_char_color( AT_IMMORT, ch );

   if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You can't do that right now.\n\r", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "Recho what?\n\r", ch );
      return;
   }

   one_argument( argument, arg );
   if( ( color = get_color( argument ) ) )
   {
      argument = one_argument( argument, arg );
      echo_to_room( color, ch->in_room, argument );
   }
   else
      echo_to_room( AT_IMMORT, ch->in_room, argument );
}

ROOM_INDEX_DATA *find_location( CHAR_DATA * ch, char *arg )
{
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   if( is_number( arg ) )
      return get_room_index( atoi( arg ) );

   if( ( victim = get_char_world( ch, arg ) ) != NULL )
      return victim->in_room;

   if( ( obj = get_obj_world( ch, arg ) ) != NULL )
      return obj->in_room;

   return NULL;
}

void do_transfer( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Transfer whom (and where)?\n\r", ch );
      return;
   }
   if( !str_cmp( arg1, "all" ) )
   {
      for( d = first_descriptor; d; d = d->next )
      {
         if( d->connected == CON_PLAYING
             && d->character != ch && d->character->in_room && d->newstate != 2 && can_see( ch, d->character ) )
         {
            char buf[MAX_STRING_LENGTH];
            sprintf( buf, "%s %s", d->character->name, arg2 );
            do_transfer( ch, buf );
         }
      }
      return;
   }

   /*
    * Thanks to Grodyn for the optional location parameter.
    */

   if( arg2[0] == '\0' )
   {
      location = ch->in_room;
   }
   else
   {
      if( ( location = find_location( ch, arg2 ) ) == NULL )
      {
         send_to_char( "No such location.\n\r", ch );
         return;
      }
   }
   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( !victim->in_room )
   {
      send_to_char( "They have no physical location!\n\r", ch );
      char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
      send_to_char( "You get yanked out of nowhere!\n\r", victim );
   }

   act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
   victim->retran = victim->in_room->vnum;
   char_from_room( victim );
   char_to_room( victim, location );
   act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
   if( ch != victim )
      act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
   do_look( victim, "auto" );
}

void do_retran( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Retransfer whom?\n\r", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   sprintf( buf, "'%s' %d", victim->name, victim->retran );
   do_transfer( ch, buf );
   return;
}

void do_regoto( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   sprintf( buf, "%d", ch->regoto );
   do_goto( ch, buf );
   return;
}

/*  Added do_atmob and do_atobj to reduce lag associated with at
 *  --Shaddai
 */
void do_atmob( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *wch;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "At where what?\n\r", ch );
      return;
   }
   if( ( wch = get_char_world( ch, arg ) ) == NULL || !IS_NPC( wch ) || wch->in_room == NULL )
   {
      send_to_char( "No such mobile in existence.\n\r", ch );
      return;
   }
   location = wch->in_room;

   set_char_color( AT_PLAIN, ch );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument, FALSE );

   if( !char_died( ch ) )
   {
      char_from_room( ch );
      char_to_room( ch, original );
   }
   return;
}

void do_atobj( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   OBJ_DATA *obj;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "At where what?\n\r", ch );
      return;
   }

   if( ( obj = get_obj_world( ch, arg ) ) == NULL || !obj->in_room )
   {
      send_to_char( "No such object in existence.\n\r", ch );
      return;
   }
   location = obj->in_room;

   set_char_color( AT_PLAIN, ch );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument, FALSE );

   if( !char_died( ch ) )
   {
      char_from_room( ch );
      char_to_room( ch, original );
   }
   return;
}

void do_at( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location = NULL;
   ROOM_INDEX_DATA *original;
   DESCRIPTOR_DATA *d;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "At where what?\n\r", ch );
      return;
   }
   if( is_number( arg ) )
      location = get_room_index( atoi( arg ) );
   else
   {
      for( d = first_descriptor; d; d = d->next )
      {
         if( ( d->connected != CON_PLAYING && d->connected != CON_EDITING ) ||
             !can_see( ch, d->character ) || str_cmp( d->character->name, arg ) )
            continue;
         break;
      }
      if( d )
         location = d->character->in_room;
   }

   if( !location )
   {
      send_to_char( "No such location.\n\r", ch );
      return;
   }

   set_char_color( AT_PLAIN, ch );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument, FALSE );

   if( !char_died( ch ) )
   {
      char_from_room( ch );
      char_to_room( ch, original );
   }
   return;
}

void do_rat( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   int Start, End, vnum;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
      return;
   }

   Start = atoi( arg1 );
   End = atoi( arg2 );
   if( Start < 1 || End < Start || Start > End || Start == End || End > 1048576000 )
   {
      send_to_char( "Invalid range.\n\r", ch );
      return;
   }
   if( !str_cmp( argument, "quit" ) )
   {
      send_to_char( "I don't think so!\n\r", ch );
      return;
   }

   original = ch->in_room;
   for( vnum = Start; vnum <= End; vnum++ )
   {
      if( ( location = get_room_index( vnum ) ) == NULL )
         continue;
      char_from_room( ch );
      char_to_room( ch, location );
      interpret( ch, argument, FALSE );
   }

   char_from_room( ch );
   char_to_room( ch, original );
   send_to_char( "Done.\n\r", ch );
   return;
}

void do_rstat( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char *sect;
   ROOM_INDEX_DATA *location;
   OBJ_DATA *obj;
   CHAR_DATA *rch;
   EXIT_DATA *pexit;
   AFFECT_DATA *paf;
   TRAIL_DATA *trail;
   int cnt;
   static char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };
   static char *rev_dir_text[] = { "s", "w", "n", "e", "d", "u", "sw", "se", "nw", "ne", "?" };

   one_argument( argument, arg );
   if( !str_cmp( arg, "ex" ) || !str_cmp( arg, "exits" ) )
   {
      location = ch->in_room;

      ch_printf_color( ch, "&cExits for room '&W%s&c'  Vnum &W%d\n\r", location->name, location->vnum );
      for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
         ch_printf_color( ch,
                          "&W%2d) &w%2s to %-5d  &cKey: &w%d  &cFlags: &w%d  &cKeywords: '&w%s&c'\n\r     Exdesc: &w%s\n\r     &cBack link: &w%d  &cVnum: &w%d  &cDistance: &w%d  &cPulltype: &w%s  &cPull: &w%d\n\r",
                          ++cnt,
                          dir_text[pexit->vdir],
                          pexit->to_room ? pexit->to_room->vnum : 0,
                          pexit->key,
                          pexit->exit_info,
                          pexit->keyword,
                          pexit->description[0] != '\0'
                          ? pexit->description : "(none).\n\r",
                          pexit->rexit ? pexit->rexit->vnum : 0,
                          pexit->rvnum, pexit->distance, pull_type_name( pexit->pulltype ), pexit->pull );
      return;
   }
   location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
   if( !location )
   {
      send_to_char( "No such location.\n\r", ch );
      return;
   }

   if( ch->in_room != location && room_is_private( location ) )
   {
      if( ch->curr_talent[TAL_VOID] < 105 )
      {
         send_to_char( "That room is private right now.\n\r", ch );
         return;
      }
      else
         send_to_char( "Overriding private flag!\n\r", ch );
   }

   ch_printf_color( ch, "&cName: &w%s\n\r&cArea: &w%s  &cFilename: &w%s\n\r",
                    location->name,
                    location->area ? location->area->name : "None????",
                    location->area ? location->area->filename : "None????" );

   switch ( location->sector_type )
   {
      default:
         sect = "?!?";
         break;
      case SECT_INSIDE:
         sect = "Inside";
         break;
      case SECT_CITY:
         sect = "City";
         break;
      case SECT_FIELD:
         sect = "Field";
         break;
      case SECT_FOREST:
         sect = "Forest";
         break;
      case SECT_HILLS:
         sect = "Hills";
         break;
      case SECT_MOUNTAIN:
         sect = "Mountains";
         break;
      case SECT_WATER_SWIM:
         sect = "Swim";
         break;
      case SECT_WATER_NOSWIM:
         sect = "Noswim";
         break;
      case SECT_UNDERWATER:
         sect = "Underwater";
         break;
      case SECT_AIR:
         sect = "Air";
         break;
      case SECT_DESERT:
         sect = "Desert";
         break;
      case SECT_OCEANFLOOR:
         sect = "Oceanfloor";
         break;
      case SECT_UNDERGROUND:
         sect = "Underground";
         break;
      case SECT_LAVA:
         sect = "Lava";
         break;
      case SECT_SWAMP:
         sect = "Swamp";
         break;
      case SECT_ICE:
         sect = "Ice";
         break;
      case SECT_BEACH:
         sect = "Beach";
         break;
   }

   ch_printf_color( ch, "&cVnum: &w%d   &cSector: &w%d (%s)   &cLight: &w%d",
                    location->vnum, location->sector_type, sect, get_light_room( location ) );

   send_to_char( "\n\r", ch );

   /*
    * advanced sector types -- Scion 
    */
   ch_printf_color( ch, "&cVegetation: &w(%3d%/%3d%)   &cWater level: &w(%3d%/%3d%)\r\n",
                    location->curr_vegetation, location->vegetation, location->curr_water, location->water );
   ch_printf_color( ch, "&cElevation:  &w(%3d%/%3d%)   &cResources:   &w(%3d%/%3d%)\r\n",
                    location->curr_elevation, location->elevation, location->curr_resources, location->resources );

   if( location->tele_delay > 0 || location->tele_vnum > 0 )
      ch_printf_color( ch, "&cTeleDelay: &R%d   &cTeleVnum: &R%d\n\r", location->tele_delay, location->tele_vnum );
   ch_printf_color( ch, "&cRoom flags: &w%s\n\r", flag_string( location->room_flags, r_flags ) );
   ch_printf_color( ch, "&cRunes: &w%s\n\r", flag_string( location->runes, rune_flags ) );
   ch_printf_color( ch, "&cDescription:\n\r&w%s", location->description );
   if( location->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char_color( "&cExtra description keywords: &w'", ch );
      for( ed = location->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'\n\r", ch );
   }
   for( paf = location->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffect: &w%s &cby &w%d.\n\r", affect_loc_name( paf->location ), paf->modifier );
/* lets see if we can make life easier , show the progtypes - shogar */
   ch_printf_color( ch, "&cProgs: &w%s\n\r", ext_flag_string( &location->progtypes, mprog_flags ) );
   send_to_char_color( "&cCharacters: &w", ch );
   for( rch = location->first_person; rch; rch = rch->next_in_room )
   {
      if( can_see( ch, rch ) )
      {
         send_to_char( " ", ch );
         one_argument( rch->name, buf );
         send_to_char( buf, ch );
      }
   }

   send_to_char_color( "\n\r&cObjects:    &w", ch );
   for( obj = location->first_content; obj; obj = obj->next_content )
   {
      send_to_char( " ", ch );
      one_argument( obj->name, buf );
      send_to_char( buf, ch );
   }
   send_to_char( "\n\r", ch );

   if( location->first_exit )
      send_to_char_color( "&c------------------- &wEXITS &c-------------------\n\r", ch );
   for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
   {
      ch_printf( ch,
                 "%2d) %-2s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\n\r",
                 ++cnt,
                 dir_text[pexit->vdir],
                 pexit->to_room ? pexit->to_room->vnum : 0,
                 pexit->key, pexit->exit_info, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
      /*
       * lets see the exflags here - shogar 
       */
      ch_printf( ch, "    Extra Flags: %s\n", flag_string( pexit->exit_info, ex_flags ) );
   }
   if( location->first_trail )
      send_to_char_color( "&c------------------- &wTRAILS &c------------------\n\r", ch );

   for( trail = location->first_trail; trail; trail = trail->next )
   {
      ch_printf( ch, "[%d seconds left] %s from %s to %s.\r\n",
                 ( ( trail->age - current_time ) + 1800 ),
                 trail->name,
                 ( trail->from > -1 ? rev_dir_text[trail->from] : "nowhere" ),
                 ( trail->to > -1 ? dir_text[trail->to] : "nowhere" ) );
   }
   return;
}

/* Face-lift by Demora */
void do_ostat( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   OBJ_DATA *obj;

   set_char_color( AT_CYAN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Ostat what?\n\r", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strcpy( arg, argument );

   if( ( obj = get_obj_world( ch, arg ) ) == NULL )
   {
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
      return;
   }
   ch_printf_color( ch, "&cName: &C%s\n\r", obj->name );
   ch_printf_color( ch, "&cCreated by: &C%s\n\r", obj->obj_by );
   ch_printf_color( ch, "&cVnum: &w%d  ", obj->pIndexData->vnum );
   ch_printf_color( ch, "&cType: &w%s  ", item_type_name( obj ) );
   ch_printf_color( ch, "&cCount:  &w%d  ", obj->pIndexData->count );
   ch_printf_color( ch, "&cGcount: &w%d\n\r", obj->count );
   ch_printf_color( ch, "&cSerial#: &w%d  ", obj->serial );
   ch_printf_color( ch, "&cTopIdxSerial#: &w%d  ", obj->pIndexData->serial );
   ch_printf_color( ch, "&cTopSerial#: &w%d\n\r", cur_obj_serial );
   ch_printf_color( ch, "&cShort description: &C%s\n\r", obj->short_descr );
   ch_printf_color( ch, "&cLong description : &C%s\n\r", obj->description );
   if( obj->action_desc[0] != '\0' )
      ch_printf_color( ch, "&cAction description: &w%s\n\r", obj->action_desc );
   ch_printf_color( ch, "&cPart flags : &w%s\n\r", ext_flag_string( &obj->parts, bp_flags ) );
   ch_printf_color( ch, "&cExtra flags: &w%s\n\r", ext_flag_string( &obj->extra_flags, o_flags ) );
   if( obj->material )
      ch_printf_color( ch, "&cMaterial type: &w(%d) %s&w\r\n", obj->material->number, obj->material->short_descr );
   ch_printf_color( ch, "&cNumber: &w%d/%d   ", 1, get_obj_number( obj ) );
   ch_printf_color( ch, "&cWeight: &w%d/%d   ", obj->weight, get_obj_weight( obj ) );
   ch_printf_color( ch, "&cSize: &w%d   ", obj->size );
   ch_printf_color( ch, "&cLayers: &w%d   ", obj->pIndexData->layers );
   ch_printf_color( ch, "&cWear_loc: &w%d\n\r", obj->wear_loc );
   ch_printf_color( ch, "&cCost: &Y%d  ", obj->cost );
   ch_printf_color( ch, "&cCondition: &w%d  ", obj->condition );
   ch_printf_color( ch, "&cTech: &w%d  ", obj->pIndexData->tech );
   send_to_char_color( "&cTimer: ", ch );
   if( obj->timer > 0 )
      ch_printf_color( ch, "&R%d  ", obj->timer );
   else
      ch_printf_color( ch, "&w%d  ", obj->timer );
   ch_printf_color( ch, "&cMana: &p%d&w/&r%d\n\r", obj->mana, obj->raw_mana );
   ch_printf_color( ch, "&cIn room: &w%d  ", obj->in_room == NULL ? 0 : obj->in_room->vnum );
   ch_printf_color( ch, "&cIn object: &w%s  ", obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr );
   ch_printf_color( ch, "&cCarried by: &C%s\n\r", obj->carried_by == NULL ? "(none)" : obj->carried_by->name );
   ch_printf_color( ch, "&cIndex Values : &w%d %d %d %d %d %d %d.\n\r",
                    obj->pIndexData->value[0], obj->pIndexData->value[1],
                    obj->pIndexData->value[2], obj->pIndexData->value[3],
                    obj->pIndexData->value[4], obj->pIndexData->value[5], obj->pIndexData->value[6] );
   if( obj->pIndexData->item_type == ITEM_FURNITURE )
   {
      ch_printf_color( ch, "&cFurnitureFlags: &w%s\n\r", flag_string( obj->pIndexData->value[2], fur_pos ) );
   }
   ch_printf_color( ch, "&cObject Values: &w%d %d %d %d %d %d %d.\n\r",
                    obj->value[0], obj->value[1], obj->value[2], obj->value[3],
                    obj->value[4], obj->value[5], obj->value[6] );
   if( obj->item_type == ITEM_FURNITURE )
   {
      ch_printf_color( ch, "&cFurnitureFlags: &w%s\n\r", flag_string( obj->value[2], fur_pos ) );
   }
   if( obj->pIndexData->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;
      send_to_char( "Primary description keywords:   '", ch );
      for( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'.\n\r", ch );
   }
   if( obj->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;
      send_to_char( "Secondary description keywords: '", ch );
      for( ed = obj->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );
         if( ed->next )
            send_to_char( " ", ch );
      }
      send_to_char( "'.\n\r", ch );
   }
/* lets see if we can make life easier , show the progtypes - shogar */
   ch_printf_color( ch, "&cProgs: &w%s\n\r", ext_flag_string( &obj->pIndexData->progtypes, mprog_flags ) );
   for( paf = obj->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffects &w%s &cby &w%d. (extra)\n\r", affect_loc_name( paf->location ), paf->modifier );
   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffects &w%s &cby &w%d.\n\r", affect_loc_name( paf->location ), paf->modifier );
   if( obj->gem )
   {
      ch_printf( ch, "&cGem: &w%s\n\r", obj->gem->name );
      for( paf = obj->gem->first_affect; paf; paf = paf->next )
         ch_printf_color( ch, "&cAffects &w%s &cby &w%d. (extra)\n\r", affect_loc_name( paf->location ), paf->modifier );
   }
   return;
}

void do_moblog( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_LOG, ch );
   send_to_char( "\n\r[Date_|_Time]  Current moblog:\n\r", ch );
   show_file( ch, MOBLOG_FILE );
   return;
}

void do_mstat( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char hpbuf[MAX_STRING_LENGTH];
   char mnbuf[MAX_STRING_LENGTH];
   char mvbuf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   int tp;

   set_pager_color( AT_CYAN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_pager( "Mstat whom?\n\r", ch );
      return;
   }
   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strcpy( arg, argument );

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_pager( "They aren't here.\n\r", ch );
      return;
   }
   pager_printf_color( ch, "\n\r&c%s: &C%-20s", IS_NPC( victim ) ? "Mobile name" : "Name", victim->name );
   if( !IS_NPC( victim ) && victim->desc )
      pager_printf_color( ch, "&cUser: &w%s@%s   Descriptor: %d\n\r",
                          victim->desc->user, victim->desc->host, victim->desc->descriptor );
   if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
      pager_printf_color( ch, "&cHelled until %24.24s by %s.\n\r",
                          ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
   pager_printf_color( ch, "&cVnum: &w%-5d    &cSex: &w%-6s    &cRoom: &w%-5d    &cCount: &w%d   &cKilled: &w%d\n\r",
                       IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                       victim->sex == SEX_MALE ? "male" :
                       victim->sex == SEX_FEMALE ? "female" :
                       victim->sex == SEX_HERMAPH ? "hermaphrodite" : "neutral",
                       victim->in_room == NULL ? 0 : victim->in_room->vnum,
                       IS_NPC( victim ) ? victim->pIndexData->count : 1,
                       IS_NPC( victim ) ? victim->pIndexData->killed : victim->pcdata->mdeaths + victim->pcdata->pdeaths );
   pager_printf_color( ch,
                       "&cStr: &C%2d&c )( Int: &C%2d&c )( Wil: &C%2d&c )( Dex: &C%2d&c )( Con: &C%2d&c )( Per: &C%2d&c )( End: &C%2d&c\n\r",
                       get_curr_str( victim ), get_curr_int( victim ), get_curr_wis( victim ), get_curr_dex( victim ),
                       get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ) );
   if( !IS_NPC( victim ) && ( victim->nation ) )
   {
      pager_printf_color( ch, "&cNation  : &P%-3s       ", victim->nation->name );
   }
   if( !IS_NPC( victim ) && ( victim->pcdata->parent ) )
   {
      pager_printf_color( ch, "&cParent : &C%s          ", victim->pcdata->parent );
   }
   if( victim->race < MAX_NPC_RACE && victim->race >= 0 )
      pager_printf_color( ch, "&cRace      : &w%-2.2d/%-10s  &cSize      : &w%-2.2d\n\r", victim->race, npc_race[victim->race], victim->position, victim->size );
   else
      send_to_pager( "\n\r", ch );
   sprintf( hpbuf, "%d/%d", victim->hit, victim->max_hit );
   sprintf( mnbuf, "%d/%d", victim->mana, victim->max_mana );
   sprintf( mvbuf, "%d/%d", victim->move, victim->max_move );
   pager_printf_color( ch, "&cHps     : &w%-12s    &cMana   : &w%-12s    &cEps       : &w%-12s\n\r", hpbuf, mnbuf, mvbuf );
   pager_printf_color( ch, "&cHitroll : &C%-5d          &cArmorClass: &w%d\n\r", GET_HITROLL( victim ), GET_AC( victim ) );
   pager_printf_color( ch, "&cDamroll : &C%-5d           &cPosition  : &w%s\n\r",
                       GET_DAMROLL( victim ), pos_names[victim->position] );
   pager_printf_color( ch, "&cFighting: &w%-13s   &cMaster : &w%-13s   &cLeader    : &w%s\n\r",
                       victim->last_hit ? victim->last_hit->name : "(none)",
                       victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)" );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cHating  : &w%-13s   &cHunting: &w%-13s   &cFearing   : &w%s\n\r",
                          victim->hating ? victim->hating->name : "(none)",
                          victim->hunting ? victim->hunting->name : "(none)",
                          victim->fearing ? victim->fearing->name : "(none)" );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cMob hitdie : &C%dd%d+%d    &cMob damdie : &C%dd%d+%d    &cNumAttacks : &C%d\n\r",
                          victim->pIndexData->hitnodice,
                          victim->pIndexData->hitsizedice,
                          victim->pIndexData->hitplus,
                          victim->pIndexData->damnodice,
                          victim->pIndexData->damsizedice, victim->pIndexData->damplus, victim->numattacks );
   pager_printf_color( ch, "&cMentalState: &w%-3d   &cEmotionalState: &w%-3d   ",
                       victim->mental_state, victim->emotional_state );
   if( !IS_NPC( victim ) )
      pager_printf_color( ch, "&cThirst: &w%d   &cFull: &w%d   &cDrunk: &w%d\n\r",
                          victim->pcdata->condition[COND_THIRST],
                          victim->pcdata->condition[COND_FULL], victim->pcdata->condition[COND_DRUNK] );
   else
      send_to_pager( "\n\r", ch );
   pager_printf_color( ch,
                       "&cSave versus: &w%d %d %d %d %d       &cItems: &w(%d/%d)  &cWeight carried &w(%d/%d) &cTotal weight &w(%d)\n\r",
                       victim->saving_poison_death, victim->saving_wand, victim->saving_para_petri, victim->saving_breath,
                       victim->saving_spell_staff, victim->carry_number, can_carry_n( victim ), victim->carry_weight,
                       can_carry_w( victim ), victim->weight );
   pager_printf_color( ch, "&cSecs: &w%d  &cTimer: &w%d  &cGold: &Y%ld", ( int )victim->played, victim->timer,
                       victim->gold );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "\r\n" );
   else
      pager_printf_color( ch, "  &cBank: &w%ld  &cBounty: &w%d\r\n", victim->pcdata->balance, victim->pcdata->bounty );
   pager_printf_color( ch, "&cWorth: &w%-4d\r\n", IS_NPC( victim ) ? get_exp_worth( victim ) : get_char_worth( victim ) );
   if( get_timer( victim, TIMER_PKILLED ) )
      pager_printf_color( ch, "&cTimerKilled:  &R%d\n\r", get_timer( victim, TIMER_PKILLED ) );
   if( get_timer( victim, TIMER_RECENTFIGHT ) )
      pager_printf_color( ch, "&cTimerRecentfight:  &R%d\n\r", get_timer( victim, TIMER_RECENTFIGHT ) );
   if( get_timer( victim, TIMER_ASUPRESSED ) )
      pager_printf_color( ch, "&cTimerAsupressed:  &R%d\n\r", get_timer( victim, TIMER_ASUPRESSED ) );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cReset: &w[%s]\r\n", victim->reset ? "X" : " " );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cAct Flags  : &w%s\n\r", ext_flag_string( &victim->act, act_flags ) );
   else
   {
      pager_printf_color( ch, "&cPlayerFlags: &w%s\n\r", ext_flag_string( &victim->act, plr_flags ) );
      pager_printf_color( ch, "&cPcflags    : &w%s\n\r", flag_string( victim->pcdata->flags, pc_flags ) );
   }
   pager_printf_color( ch, "&cAffected by: &C%s\n\r", affect_bit_name( &victim->affected_by ) );
   if( !IS_NPC( victim ) && victim->wait )
      pager_printf_color( ch, "   &cWaitState: &R%d\n\r", victim->wait / 12 );
   if( !IS_NPC( victim ) )
      pager_printf_color( ch, "&cPerm Affs  : &C%s\n\r", affect_bit_name( &victim->pcdata->perm_aff ) );
   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cShortdesc  : &w%s\n\r", victim->short_descr != NULL ? victim->short_descr : "(none set)" );
   else
   {
      if( victim->short_descr != NULL )
         pager_printf_color( ch, "&cShortdesc  : &w%s\n\r", victim->short_descr );
   }
   if( IS_NPC( victim ) && victim->spec_fun )
      pager_printf_color( ch, "&cMobile has spec fun: &w%s\n\r", lookup_spec( victim->spec_fun ) );

   if( victim->resistant > 0 )
      pager_printf_color( ch, "&cResistant  : &w%s\n\r", flag_string( victim->resistant, ris_flags ) );
   if( victim->immune > 0 )
      pager_printf_color( ch, "&cImmune     : &w%s\n\r", flag_string( victim->immune, ris_flags ) );
   if( victim->susceptible > 0 )
      pager_printf_color( ch, "&cSusceptible: &w%s\n\r", flag_string( victim->susceptible, ris_flags ) );

   if( !IS_NPC( victim ) )
   {
      DEITY_DATA *talent;

      tp = victim->pcdata->points;
      for( talent = first_deity; talent; talent = talent->next )
      {
         if( victim->talent[talent->index] > -1 )
         {
            tp += victim->talent[talent->index] * talent->cost;
            pager_printf_color( ch,
                                "&c%-12s: &w(%-3d of %-3d, effectivity %-3d)\n\r",
                                talent->name,
                                victim->curr_talent[talent->index],
                                victim->talent[talent->index], TALENT( victim, talent->index ) );
         }
      }
      pager_printf_color( ch, "&CTotal Power : &w%-4d\n\r", tp );
   }

/* lets see if we can make life easier , show the progtypes - shogar */
   if( IS_NPC( victim ) )
   {
      pager_printf_color( ch, "&cProgs: &w%s\n\r", ext_flag_string( &victim->pIndexData->progtypes, mprog_flags ) );
   }
/* Too spammy, put in an alternative to see this later
    for ( paf = victim->first_affect; paf; paf = paf->next )
	if ( (skill=get_skilltype(paf->type)) != NULL )
	  pager_printf_color( ch,
	    "&c%s: &w'%s' mods %s by %d for %d rnds with bits %s.\n\r",
	    skill_tname[skill->type],
	    skill->name,
	    affect_loc_name( paf->location ),
	    paf->modifier,
	    paf->duration,
	    affect_bit_name( &paf->bitvector )
	    );
*/
   return;
}

void do_mfind( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   int hash;
   int nMatch;
   bool fAll;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mfind whom?\n\r", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );
   nMatch = 0;

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_mob_index()... which loops itself, an average of 1-2 times...
    * So theoretically, the above routine may loop well over 40,000 times,
    * and my routine bellow will loop for as many index_mobiles are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
         if( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\n\r", pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
   return;
}

void do_ofind( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   int hash;
   int nMatch;
   bool fAll;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Ofind what?\n\r", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );
   nMatch = 0;

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_obj_index()... which loops itself, an average of 2-3 times...
    * So theoretically, the above routine may loop well over 50,000 times,
    * and my routine bellow will loop for as many index_objects are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
         if( fAll || nifty_is_name( arg, pObjIndex->name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\n\r", pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
   return;
}

void do_mwhere( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   bool found;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Mwhere whom?\n\r", ch );
      return;
   }

   found = FALSE;
   for( victim = first_char; victim; victim = victim->next )
   {
      if( IS_NPC( victim ) && victim->in_room && nifty_is_name( arg, victim->name ) )
      {
         found = TRUE;
         pager_printf( ch, "[%5d] %-28s [%5d] %s\n\r",
                       victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name );
      }
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   return;
}

void do_gwhere( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   bool found = FALSE;
   int count = 0;

   pager_printf_color( ch, "\n\r&cGlobal player locations:&w\n\r" );
   for( d = first_descriptor; d; d = d->next )
      if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
          && ( victim = d->character ) != NULL && !IS_NPC( victim ) && victim->in_room )
      {
         found = TRUE;
         pager_printf_color( ch, "&c(&C%3d&c) &w%-12.12s   &w[%-5d - %-19.19s]   &c%-25.25s\n\r",
                             get_char_worth( victim ), victim->name, victim->in_room->vnum, victim->in_room->area->name,
                             victim->in_room->name );
         count++;
      }
   pager_printf_color( ch, "&c%d characters found.\n\r", count );
   return;
}

/* Added 'show' argument for lowbie imms without ostat -- Blodkai */
/* Made show the default action :) Shaddai */
/* Trimmed size, added vict info, put lipstick on the pig -- Blod */
void do_bodybag( CHAR_DATA * ch, char *argument )
{
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *owner;
   OBJ_DATA *obj;
   bool found = FALSE, bag = FALSE;

   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
   {
      send_to_char_color( "&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r", ch );
      return;
   }

   sprintf( buf3, " " );
   sprintf( buf2, "the corpse of %s", arg1 );
   argument = one_argument( argument, arg2 );

   if( arg2[0] != '\0' && ( str_cmp( arg2, "yes" ) && str_cmp( arg2, "bag" ) && str_cmp( arg2, "now" ) ) )
   {
      send_to_char_color( "\n\r&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "yes" ) || !str_cmp( arg2, "bag" ) || !str_cmp( arg2, "now" ) )
      bag = TRUE;

   pager_printf_color( ch, "\n\r&P%s remains of %s ... ", bag ? "Retrieving" : "Searching for", capitalize( arg1 ) );
   for( obj = first_object; obj; obj = obj->next )
   {
      if( obj->in_room && !str_cmp( buf2, obj->short_descr ) && ( obj->pIndexData->vnum == 11 ) )
      {
         send_to_pager( "\n\r", ch );
         found = TRUE;
         pager_printf_color( ch, "&P%s:  %s%-12.12s   &PIn:  &w%-22.22s  &P[&w%5d&P]   &PTimer:  %s%2d",
                             bag ? "Bagging" : "Corpse",
                             bag ? "&R" : "&w",
                             capitalize( arg1 ),
                             obj->in_room->area->name,
                             obj->in_room->vnum,
                             obj->timer < 1 ? "&w" : obj->timer < 5 ? "&R" : obj->timer < 10 ? "&Y" : "&w", obj->timer );
         if( bag )
         {
            obj_from_room( obj );
            obj = obj_to_char( obj, ch );
            obj->timer = -1;
            save_char_obj( ch );
         }
      }
   }
   if( !found )
   {
      send_to_pager_color( "&Pno corpse was found.\n\r", ch );
      return;
   }
   send_to_pager( "\n\r", ch );
   for( owner = first_char; owner; owner = owner->next )
   {
      if( IS_NPC( owner ) )
         continue;
      if( can_see( ch, owner ) && !str_cmp( arg1, owner->name ) )
         break;
   }
   if( owner == NULL )
   {
      pager_printf_color( ch, "&P%s is not currently online.\n\r", capitalize( arg1 ) );
      return;
   }
   return;
}


/* New owhere by Altrag, 03/14/96 */
/*void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool found;
    int icnt = 0;

    set_pager_color( AT_PLAIN, ch );

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Owhere what?\n\r", ch );
	return;
    }

    argument = one_argument(argument, arg1);
    if ( arg1[0] != '\0' && !str_prefix(arg1, "nesthunt") )
    {
      if ( !(obj = get_obj_world(ch, arg)) )
      {
        send_to_char( "Nesthunt for what object?\n\r", ch );
        return;
      }
      for ( ; obj->in_obj; obj = obj->in_obj )
      {
	pager_printf(ch, "[%5d] %-28s in object [%5d] %s\n\r",
                obj->pIndexData->vnum, obj_short(obj),
                obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr);
	++icnt;
      }
      sprintf(buf, "[%5d] %-28s in ", obj->pIndexData->vnum,
		obj_short(obj));
      if ( obj->carried_by )
        sprintf(buf+strlen(buf), "invent [%5d] %s\n\r",
                (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum
                : 0), PERS(obj->carried_by, ch));
      else if ( obj->in_room )
        sprintf(buf+strlen(buf), "room   [%5d] %s\n\r",
                obj->in_room->vnum, obj->in_room->name);
      else if ( obj->in_obj )
      {
        bug("do_owhere: obj->in_obj after NULL!",0);
        strcat(buf, "object??\n\r");
      }
      else
      {
        bug("do_owhere: object doesnt have location, put in limbo!",0);
		obj_to_room(obj, get_room_index(ROOM_VNUM_LIMBO));
        strcat(buf, "nowhere??\n\r");
      }
      send_to_pager(buf, ch);
      ++icnt;
      pager_printf(ch, "Nested %d levels deep.\n\r", icnt);
      return;
    }

    found = FALSE;
    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( !nifty_is_name( arg, obj->name ) )
            continue;
        found = TRUE;

	if (IS_OBJ_STAT(obj, ITEM_GEM)) continue;
        sprintf(buf, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum,
                obj_short(obj));
        if ( obj->carried_by )
          sprintf(buf+strlen(buf), "invent [%5d] %s\n\r",
                  (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum
                  : 0), PERS(obj->carried_by, ch));
        else if ( obj->in_room )
          sprintf(buf+strlen(buf), "room   [%5d] %s\n\r",
                  obj->in_room->vnum, obj->in_room->name);
        else if ( obj->in_obj )
          sprintf(buf+strlen(buf), "object [%5d] %s\n\r",
                  obj->in_obj->pIndexData->vnum, obj_short(obj->in_obj));
        else
        {
          bug("do_owhere: object doesnt have location, put in limbo!",0);
		  obj_to_room(obj, get_room_index(ROOM_VNUM_LIMBO));
          strcat(buf, "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
    }

    if ( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    else
      pager_printf(ch, "%d matches.\n\r", icnt);
    return;
}
*/
void do_osearch( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   bool found;
   int icnt = 0;

   set_pager_color( AT_PLAIN, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || strcmp( arg, "artifact" ) )
   {
      send_to_char( "Syntax: osearch <item type>\n\r", ch );
      send_to_char( "Valid item types are currently:\n\r", ch );
      send_to_char( "artifact\r\n", ch );
      return;
   }

   found = FALSE;
   for( obj = first_object; obj; obj = obj->next )
   {
      if( !IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         continue;
      found = TRUE;

      sprintf( buf, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum, obj_short( obj ) );
      if( obj->carried_by )
         sprintf( buf + strlen( buf ), "invent [%5d] %s\n\r",
                  ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum : 0 ), PERS( obj->carried_by, ch ) );
      else if( obj->in_room )
         sprintf( buf + strlen( buf ), "room   [%5d] %s\n\r", obj->in_room->vnum, obj->in_room->name );
      else if( obj->in_obj )
         sprintf( buf + strlen( buf ), "object [%5d] %s\n\r", obj->in_obj->pIndexData->vnum, obj_short( obj->in_obj ) );
      else
      {
         bug( "do_osearch: object doesnt have location, put in limbo!", 0 );
         obj_to_room( obj, get_room_index( ROOM_VNUM_LIMBO ) );
         strcat( buf, "nowhere??\n\r" );
      }
      send_to_pager( buf, ch );
   }

   if( !found )
      act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
   else
      pager_printf( ch, "%d matches.\n\r", icnt );
   return;
}

void do_reboo( CHAR_DATA * ch, char *argument )
{
   send_to_char_color( "&YIf you want to REBOOT, spell it out.\n\r", ch );
   return;
}

void do_reboot( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   extern bool mud_down;
   CHAR_DATA *vch;

   set_char_color( AT_IMMORT, ch );

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) && str_cmp( argument, "and sort skill table" ) )
   {
      send_to_char( "Syntax:  'reboot mud now' or 'reboot nosave'\n\r", ch );
      return;
   }

   sprintf( buf, "Reboot by %s.", ch->name );
   do_echo( ch, buf );

   if( !str_cmp( argument, "and sort skill table" ) )
   {
      sort_skill_table(  );
      save_skill_table(  );
   }

   /*
    * Save all characters before booting. 
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );

   mud_down = TRUE;
   return;
}

void do_shutdow( CHAR_DATA * ch, char *argument )
{
   send_to_char_color( "&YIf you want to SHUTDOWN, spell it out.\n\r", ch );
   return;
}

void do_shutdown( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   extern bool mud_down;
   CHAR_DATA *vch;

   set_char_color( AT_IMMORT, ch );

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) )
   {
      send_to_char( "Syntax:  'shutdown mud now' or 'shutdown nosave'\n\r", ch );
      return;
   }

   sprintf( buf, "Shutdown by %s.", ch->name );
   append_file( ch, SHUTDOWN_FILE, buf );
   strcat( buf, "\n\r" );
   do_echo( ch, buf );

   /*
    * Save all characters before booting. 
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );
   mud_down = TRUE;
   return;
}

void do_snoop( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   if( ch->curr_talent[TAL_MIND] + ch->curr_talent[TAL_SEEKING] < 200 )
   {
      huh( ch );
      return;
   }

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Snoop whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( !victim->desc )
   {
      send_to_char( "No descriptor to snoop.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Cancelling all snoops.\n\r", ch );
      for( d = first_descriptor; d; d = d->next )
         if( d->snoop_by == ch->desc )
            d->snoop_by = NULL;
      return;
   }
   if( victim->desc->snoop_by )
   {
      send_to_char( "Busy already.\n\r", ch );
      return;
   }

   if( ch->desc )
   {
      for( d = ch->desc->snoop_by; d; d = d->snoop_by )
         if( d->character == victim || d->original == victim )
         {
            send_to_char( "No snoop loops.\n\r", ch );
            return;
         }
   }

   if( IS_SET( ch->pcdata->flags, PCFLAG_SHIELD )
       || ( IS_SET( victim->pcdata->flags, PCFLAG_SHIELD ) && TALENT( ch, TAL_MIND ) < TALENT( victim, TAL_MIND ) ) )
   {
      send_to_char( "A shield blocks you from connecting.\n\r", ch );
      return;
   }

   if( victim->curr_talent[TAL_SEEKING] + victim->curr_talent[TAL_MIND] >= 110 )
      write_to_descriptor( victim->desc->descriptor, "\n\rYou feel like someone is watching your every move...\n\r", 0 );
   victim->desc->snoop_by = ch->desc;
   send_to_char( "Ok.\n\r", ch );
   return;
}

void do_switch( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Switch into whom?\n\r", ch );
      return;
   }
   if( !ch->desc )
      return;
   if( ch->desc->original )
   {
      send_to_char( "You are already switched.\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Ok.\n\r", ch );
      return;
   }
   if( victim->desc )
   {
      send_to_char( "Character in use.\n\r", ch );
      return;
   }

   ch->desc->character = victim;
   ch->desc->original = ch;
   victim->desc = ch->desc;
   ch->desc = NULL;
   ch->switched = victim;
   send_to_char( "Ok.\n\r", victim );
   return;
}

void do_return( CHAR_DATA * ch, char *argument )
{

   set_char_color( AT_IMMORT, ch );

   if( !ch->desc )
      return;
   if( !ch->desc->original )
   {
      send_to_char( "You aren't switched.\n\r", ch );
      return;
   }

   send_to_char( "You return to your original body.\n\r", ch );
   ch->desc->character = ch->desc->original;
   ch->desc->original = NULL;
   ch->desc->character->desc = ch->desc;
   ch->desc->character->switched = NULL;
   ch->desc = NULL;
   return;
}

void do_minvoke( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *victim;
   int vnum;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  minvoke <vnum>\n\r", ch );
      return;
   }
   if( !is_number( arg ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg, arg2 );

      vnum = -1;
      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
            if( nifty_is_name( arg2, pMobIndex->player_name ) && ++cnt == count )
            {
               vnum = pMobIndex->vnum;
               break;
            }
      if( vnum == -1 )
      {
         send_to_char( "No such mobile exists.\n\r", ch );
         return;
      }
   }
   else
      vnum = atoi( arg );

   if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "No mobile has that vnum.\n\r", ch );
      return;
   }

   victim = create_mobile( pMobIndex );
   char_to_room( victim, ch->in_room );
   act( AT_IMMORT, "$n invokes $N!", ch, NULL, victim, TO_ROOM );
   /*
    * How about seeing what we're invoking for a change. -Blodkai
    */
   ch_printf_color( ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wworth %d&Y)\n\r",
                    pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name, get_char_worth( victim ) );
   return;
}

void do_oinvoke( CHAR_DATA * ch, char *argument )
{
   MATERIAL_DATA *material_lookup( int number );

   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int vnum, mat;
   MATERIAL_DATA *material;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: oinvoke <vnum> [material]\n\r", ch );
      return;
   }
   if( arg2[0] == '\0' )
   {
      mat = -1;
   }
   else
   {
      if( !is_number( arg2 ) )
      {
         send_to_char( "Syntax:  oinvoke <vnum> [material]\n\r", ch );
         return;
      }
      mat = atoi( arg2 );
      if( !( material = material_lookup( mat ) ) )
      {
         send_to_char( "There is no such material.\n\r", ch );
         return;
      }
   }
   if( !is_number( arg1 ) )
   {
      char arg[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg1, arg );

      vnum = -1;
      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
            if( nifty_is_name( arg, pObjIndex->name ) && ++cnt == count )
            {
               vnum = pObjIndex->vnum;
               break;
            }
      if( vnum == -1 )
      {
         send_to_char( "No such object exists.\n\r", ch );
         return;
      }
   }
   else
      vnum = atoi( arg1 );

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char( "No object has that vnum.\n\r", ch );
      return;
   }

   if( !material )
      material = first_material;
   obj = create_object( pObjIndex, 0 );

   if( mat != -1 )
   {
      char buf[MAX_STRING_LENGTH];
      char arg[MAX_STRING_LENGTH];
      AFFECT_DATA *paf;
      extern int top_affect;

      strcpy( buf, material->name );
      sprintf( arg, obj->short_descr, buf );
      one_argument( arg, buf );
      if( strcmp( buf, "some" ) )
         strcpy( arg, aoran( arg ) );
      STRFREE( obj->short_descr );
      obj->short_descr = STRALLOC( arg );

      strcpy( buf, material->name );
      sprintf( arg, obj->description, buf );
      if( strcmp( buf, "some" ) )
         strcpy( arg, aoran( arg ) );
      STRFREE( obj->description );
      obj->description = STRALLOC( capitalize( arg ) );

      strcpy( buf, obj->name );
      strcat( buf, " " );
      strcat( buf, material->name );
      STRFREE( obj->name );
      obj->name = STRALLOC( buf );

      obj->extra_flags = material->extra_flags;

      obj->cost += number_fuzzy( obj->cost );
      obj->size = ch->height;

      for( paf = material->first_affect; paf; paf = paf->next )
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

      obj->material = material;
   }

   obj->obj_by = STRALLOC( ch->name );

   if( !IS_OBJ_STAT( obj, ITEM_NO_TAKE ) )
   {
      obj = obj_to_char( obj, ch );
   }
   else
   {
      obj = obj_to_room( obj, ch->in_room );
      act( AT_IMMORT, "$n fashions $p from ether!", ch, obj, NULL, TO_ROOM );
   }
   /*
    * I invoked what? --Blodkai 
    */
   ch_printf_color( ch, "&YYou invoke %s (&W#%d &Y- &W%s&Y)\n\r", obj->short_descr, pObjIndex->vnum, pObjIndex->name );
   return;
}

void do_purge( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   set_char_color( AT_LBLUE, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      /*
       * 'purge' 
       */
      CHAR_DATA *vnext;
      OBJ_DATA *obj_next;

      for( victim = ch->in_room->first_person; victim; victim = vnext )
      {
         vnext = victim->next_in_room;
         if( IS_NPC( victim ) && victim != ch )
            extract_char( victim, TRUE );
      }

      for( obj = ch->in_room->first_content; obj; obj = obj_next )
      {
         obj_next = obj->next_content;
         extract_obj( obj );
      }

      act( AT_WHITE, "$n flicks $s hand and everything dissolves into mist!", ch, NULL, NULL, TO_ROOM );
      act( AT_WHITE, "You flick your hand and everything dissolves into mist.", ch, NULL, NULL, TO_CHAR );
	  save_house_by_vnum(ch->in_room->vnum); /* Prevent House Object Duplication */
      return;
   }
   victim = NULL;
   obj = NULL;

   /*
    * fixed to get things in room first -- i.e., purge portal (obj),
    * * no more purging mobs with that keyword in another room first
    * * -- Tri 
    */
   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      if( ( victim = get_char_world( ch, arg ) ) == NULL && ( obj = get_obj_world( ch, arg ) ) == NULL ) /* no get_obj_room */
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }
   }

   if( obj )
   {
   int vnum = (obj->in_room ? obj->in_room->vnum : ROOM_VNUM_LIMBO);
      separate_obj( obj );
      act( AT_WHITE, "$n points at $p, shattering it to dust!", ch, obj, NULL, TO_ROOM );
      act( AT_WHITE, "You point at $p and it shatters to dust.", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
	  save_house_by_vnum(vnum); /* Prevent House Object Duplication */
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "Not on PC's.\n\r", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\n\r", ch );
      return;
   }

   while( ( obj = victim->last_carrying ) != NULL )
      extract_obj( obj );

   act( AT_WHITE, "$n glares at $N and $E fades from existence..", ch, NULL, victim, TO_NOTVICT );
   act( AT_WHITE, "You glare at $N and $E fades from existence.", ch, NULL, victim, TO_CHAR );
   extract_char( victim, TRUE );
   return;
}

void do_balefire( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;

   if( ch->curr_talent[TAL_TIME] + ch->curr_talent[TAL_VOID] < 200 )
   {
      huh( ch );
      return;
   }

   act( AT_DGREY, "A bar of balefire beams from your $t!", ch, hands( ch ), NULL, TO_CHAR );
   act( AT_DGREY, "A bar of balefire beams from $n's $t!", ch, hands( ch ), NULL, TO_ROOM );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Balefire what?\n\r", ch );
      return;
   }

   obj = NULL;
   victim = NULL;
   if( ( ( obj = get_obj_here( ch, arg ) ) == NULL ) && ( ( ( victim = get_char_room( ch, arg ) ) == NULL ) ) )
   {
      send_to_char( "You can't find that here.\n\r", ch );
      return;
   }

   use_magic( ch, TAL_TIME, 300 );

   if( obj )
   {
      if( ch->in_room->pShop )
      {
         send_to_char( "Hey! Not in a shop!\n\r", ch );
         return;
      }
      separate_obj( obj );
      act( AT_DGREY, "$n burns $p out of existance!", ch, obj, NULL, TO_ROOM );
      act( AT_DGREY, "You burn $p out of existance!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
   }

   magic_damage( victim, ch, ( TALENT( ch, TAL_VOID ) + TALENT( ch, TAL_TIME ) ) * 2, MAG_ANTIMATTER, TAL_TIME, FALSE );
}

void do_low_purge( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;

   if( ch->curr_talent[TAL_FROST] < 75 )
   {
      huh( ch );
      return;
   }

   set_char_color( AT_LBLUE, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Shatter what?\n\r", ch );
      return;
   }

   obj = NULL;
   victim = NULL;
   if( ( ( obj = get_obj_here( ch, arg ) ) == NULL ) && ( ( ( victim = get_char_room( ch, arg ) ) == NULL ) ) )
   {
      send_to_char( "You can't find that here.\n\r", ch );
      return;
   }

   use_magic( ch, TAL_FROST, 120 );

   if( obj )
   {
      if( ch->in_room->pShop )
      {
         send_to_char( "Hey! Not in a shop!\n\r", ch );
         return;
      }
      separate_obj( obj );
      act( AT_FROST, "$n freezes $p, shattering it!", ch, obj, NULL, TO_ROOM );
      act( AT_FROST, "You freeze $p, shattering it with a flick of your finger!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
   }

   if( !IS_NPC( victim ) )
   {
      do_chill( ch, victim->name );
      return;
   }

   if( xIS_SET( victim->act, ACT_PACIFIST ) )
   {
      send_to_char( "Not on a pacifist.\n\r", ch );
      return;
   }

   while( ( obj = victim->first_carrying ) != NULL )
   {
      obj_from_char( obj );
      obj_to_room( obj, victim->in_room );
   }

   act( AT_FROST, "You freeze $N, then shatter $M with a flick of your finger!", ch, NULL, victim, TO_CHAR );
   act( AT_FROST, "$n freezes $N, then shatters $M with a flick of $s finger!", ch, NULL, victim, TO_ROOM );

   extract_char( victim, TRUE );
}

void do_strew( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   ROOM_INDEX_DATA *pRoomIndex;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Strew who, what?\n\r", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "It would work better if they were here.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Try taking it out on someone else first.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "coins" ) )
   {
      if( victim->gold < 1 )
      {
         send_to_char( "Drat, this one's got no gold to start with.\n\r", ch );
         return;
      }
      victim->gold = 0;
      act( AT_MAGIC, "$n gestures and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You gesture and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "As $n gestures, an unearthly gale sends your currency flying!", ch, NULL, victim, TO_VICT );
      return;
   }
   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, 1048576000 ) );
      if( pRoomIndex )
         if( !IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
             && !IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY )
             && !IS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL ) && !IS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
            break;
   }
   if( !str_cmp( arg2, "inventory" ) )
   {
      act( AT_MAGIC, "$n speaks a single word, sending $N's possessions flying!", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You speak a single word, sending $N's possessions flying!", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "$n speaks a single word, sending your possessions flying!", ch, NULL, victim, TO_VICT );
      for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
      {
         obj_next = obj_lose->next_content;
         obj_from_char( obj_lose );
         obj_to_room( obj_lose, pRoomIndex );
         pager_printf_color( ch, "\t&w%s sent to %d\n\r", capitalize( obj_lose->short_descr ), pRoomIndex->vnum );
      }
      return;
   }
   send_to_char( "Strew their coins or inventory?\n\r", ch );
   return;
}

void do_strip( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   int count = 0;

   set_char_color( AT_OBJECT, ch );
   if( !argument )
   {
      send_to_char( "Strip who?\n\r", ch );
      return;
   }
   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They're not here.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "Kinky.\n\r", ch );
      return;
   }
   act( AT_OBJECT, "Searching $N ...", ch, NULL, victim, TO_CHAR );
   for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
   {
      obj_next = obj_lose->next_content;
      obj_from_char( obj_lose );
      obj_to_char( obj_lose, ch );
      pager_printf_color( ch, "  &G... %s (&g%s) &Gtaken.\n\r", capitalize( obj_lose->short_descr ), obj_lose->name );
      count++;
   }
   if( !count )
      pager_printf_color( ch, "&GNothing found to take.\n\r", ch );
   return;
}

void do_restore( CHAR_DATA * ch, char *argument )
{
   PART_DATA *part;
   char arg[MAX_INPUT_LENGTH];

   if( ch->curr_talent[TAL_HEALING] < 100 )
   {
      huh( ch );
      return;
   }

   set_char_color( AT_WHITE, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Restore whom?\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( !ch->pcdata )
         return;

      if( !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
      {
         if( IS_NPC( ch ) )
         {
            send_to_char( "You can't do that.\n\r", ch );
            return;
         }
         else
         {
            /*
             * Check if the player did a restore all within the last 18 hours. 
             */
            if( current_time - ch->pcdata->restore_time < RESTORE_INTERVAL * 100 / TALENT( ch, TAL_HEALING ) )
            {
               send_to_char( "Sorry, you can't do a restore yet.\n\r", ch );
               do_restoretime( ch, "" );
               return;
            }
         }
      }
      last_restore_all_time = current_time;
      ch->pcdata->restore_time = current_time;
      save_char_obj( ch );
      act( AT_WHITE, "You open your arms, sending a healing wind through the worlds.", ch, NULL, NULL, TO_CHAR );
      use_magic( ch, TAL_HEALING, 1000 );
      WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) )
         {
            vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
            update_pos( vch );
            act( AT_WHITE, "A soothing wind sweeps past, healing your wounds...", ch, NULL, vch, TO_VICT );
         }
      }
   }
   else
   {

      CHAR_DATA *victim;

      if( current_time - ch->pcdata->restore_time < RESTORE_INTERVAL * 100 / TALENT( ch, TAL_HEALING ) )
      {
         send_to_char( "Sorry, you can't do a restore yet.\n\r", ch );
         do_restoretime( ch, "" );
         return;
      }

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }

      ch->pcdata->restore_time = current_time;
      use_magic( ch, TAL_HEALING, 500 );
      WAIT_STATE( ch, PULSE_VIOLENCE );
      save_char_obj( ch );
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      for( part = victim->first_part; part; part = part->next )
      {
         part->cond = PART_WELL;
         part->flags = PART_WELL;
      }
      update_pos( victim );
      if( ch != victim )
         act( AT_WHITE, "A cold mist swirls around you, healing your wounds.", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "You summon a healing mist to soothe $N's wounds.", ch, NULL, victim, TO_CHAR );
      return;
   }
}

void do_restoretime( CHAR_DATA * ch, char *argument )
{
   long int time_passed;
   int hour, minute;

   set_char_color( AT_IMMORT, ch );

   if( !last_restore_all_time )
      ch_printf( ch, "There has been no restore all since reboot.\n\r" );
   else
   {
      time_passed = current_time - last_restore_all_time;
      hour = ( int )( time_passed / 3600 );
      minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
      ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\n\r", hour, minute );
   }

   if( !ch->pcdata )
      return;

   if( !ch->pcdata->restore_time )
   {
      send_to_char( "You have never done a restore all.\n\r", ch );
      return;
   }

   time_passed = current_time - ch->pcdata->restore_time;
   hour = ( int )( time_passed / 3600 );
   minute = ( int )( ( time_passed - ( hour * 3600 ) ) / 60 );
   ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\n\r", hour, minute );
   return;
}

void do_freeze( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_LBLUE, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Freeze whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      send_to_char( "God your an idiot.\n\r", ch );
      return;
   }
   set_char_color( AT_LBLUE, victim );
   if( xIS_SET( victim->act, PLR_FREEZE ) )
   {
      xREMOVE_BIT( victim->act, PLR_FREEZE );
      act( AT_LBLUE, "You glance at $N, the ice melting from $S body.", ch, NULL, victim, TO_CHAR );
      act( AT_LBLUE, "$n glances at you, the ice melting from your body.", ch, NULL, victim, TO_VICT );
      act( AT_LBLUE, "$n glances at $N, the ice melting from $S body.", ch, NULL, victim, TO_NOTVICT );
   }
   else
   {
      xSET_BIT( victim->act, PLR_FREEZE );
      act( AT_LBLUE, "You wave a hand dismissively at $N, freezing $M solid.", ch, NULL, victim, TO_CHAR );
      act( AT_LBLUE, "$n waves a dismissive hand at you, your body covering with ice.", ch, NULL, victim, TO_VICT );
      act( AT_LBLUE, "$n waves a hand at $N, $S body becoming covered in ice.", ch, NULL, victim, TO_NOTVICT );
   }
   save_char_obj( victim );
   return;
}

void do_log( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Log whom?\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      if( fLogAll )
      {
         fLogAll = FALSE;
         send_to_char( "Log ALL off.\n\r", ch );
      }
      else
      {
         fLogAll = TRUE;
         send_to_char( "Log ALL on.\n\r", ch );
      }
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }

   if( xIS_SET( victim->act, PLR_LOG ) )
   {
      xREMOVE_BIT( victim->act, PLR_LOG );
      ch_printf( ch, "LOG removed from %s.\n\r", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_LOG );
      ch_printf( ch, "LOG applied to %s.\n\r", victim->name );
   }
   return;
}

void do_litterbug( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Set litterbug flag on whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_LITTERBUG ) )
   {
      xREMOVE_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "You can drop items again.\n\r", victim );
      ch_printf( ch, "LITTERBUG removed from %s.\n\r", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "A strange force prevents you from dropping any more items!\n\r", victim );
      ch_printf( ch, "LITTERBUG set on %s.\n\r", victim->name );
   }
   return;
}

void do_nochannel( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Nochannel whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_NO_CHANNEL ) )
   {
      xREMOVE_BIT( victim->act, PLR_NO_CHANNEL );
      do_channels( victim, "rejoin" );
      send_to_char( "You can use channels again.\n\r", victim );
      ch_printf( ch, "NOCHANNEL removed from %s.\n\r", victim->name );
   }
   else
   {
      do_channels( victim, "quiet" );
      xSET_BIT( victim->act, PLR_NO_CHANNEL );
      send_to_char( "You can't use channels!\n\r", victim );
      ch_printf( ch, "NOCHANNEL applied to %s.\n\r", victim->name );
   }
}

void do_noemote( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Noemote whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_NO_EMOTE ) )
   {
      xREMOVE_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can emote again.\n\r", victim );
      ch_printf( ch, "NOEMOTE removed from %s.\n\r", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can't emote!\n\r", victim );
      ch_printf( ch, "NOEMOTE applied to %s.\n\r", victim->name );
   }
   return;
}

void do_notell( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Notell whom?", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_NO_TELL ) )
   {
      xREMOVE_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can use tells again.\n\r", victim );
      ch_printf( ch, "NOTELL removed from %s.\n\r", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can't use tells!\n\r", victim );
      ch_printf( ch, "NOTELL applied to %s.\n\r", victim->name );
   }
   return;
}

void do_notitle( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Notitle whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      send_to_char( "You can set your own title again.\n\r", victim );
      ch_printf( ch, "NOTITLE removed from %s.\n\r", victim->name );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      set_title( victim, victim->name );
      send_to_char( "You can't set your own title!\n\r", victim );
      ch_printf( ch, "NOTITLE set on %s.\n\r", victim->name );
   }
   return;
}

void do_silence( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   if( ch->curr_talent[TAL_SPEECH] < 150 )
   {
      huh( ch );
      return;
   }

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Silence whom?", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   if( get_curr_wil( victim ) > number_range( 1, get_curr_wil( ch ) + TALENT( ch, TAL_SPEECH ) ) )
   {
      act( AT_MAGIC, "$N resists your attempt to silence $M.", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "You resist $n's attempt to silence you.", ch, NULL, victim, TO_VICT );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_SILENCE ) )
   {
      send_to_char( "Player already silenced, use unsilence to remove.\n\r", ch );
   }
   else
   {
      xSET_BIT( victim->act, PLR_SILENCE );
      act( AT_WHITE, "You look at $N and put a finger to your mouth.", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "$N's lips suddenly grow together, closing up $S mouth!", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "$n looks at you and puts a finger to $s mouth.", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "Your lips suddenly grow together, closing up your mouth!", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "$n looks at $N and puts a finger to $s mouth.", ch, NULL, victim, TO_NOTVICT );
      act( AT_WHITE, "$N's lips suddenly grow together, closing up $S mouth!", ch, NULL, victim, TO_NOTVICT );
   }
   return;
}

/* Much better than toggling this with do_silence, yech --Blodkai */
void do_unsilence( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   if( ch->curr_talent[TAL_SPEECH] < 130 )
   {
      huh( ch );
      return;
   }

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Unsilence whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
   }
   set_char_color( AT_IMMORT, victim );
   if( xIS_SET( victim->act, PLR_SILENCE ) )
   {
      xREMOVE_BIT( victim->act, PLR_SILENCE );
      act( AT_WHITE, "$N's lips separate, allowing $M to speak.", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "Your lips separate, allowing you to speak.", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "$N's lips separate, allowing $M to speak.", ch, NULL, victim, TO_NOTVICT );
   }
   else
   {
      send_to_char( "That player is not silenced.\n\r", ch );
   }
   return;
}

void do_peace( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *rch;

   act( AT_WHITE, "$n throws out $s hands, stopping the fight.", ch, NULL, NULL, TO_ROOM );
   act( AT_WHITE, "You throw out your hands, stopping the fight.", ch, NULL, NULL, TO_CHAR );
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {

      if( rch->last_hit )
      {
         rch->last_hit->last_hit = NULL;
         rch->last_hit = NULL;
      }
      /*
       * Added by Narn, Nov 28/95 
       */
      stop_hating( rch );
      stop_hunting( rch );
      stop_fearing( rch );

   }
   return;
}


void do_wizlock( CHAR_DATA * ch, char *argument )
{
   extern bool wizlock;
   wizlock = !wizlock;

   set_char_color( AT_DANGER, ch );

   if( wizlock )
      send_to_char( "Game wizlocked.\n\r", ch );
   else
      send_to_char( "Game un-wizlocked.\n\r", ch );
   return;
}

void do_noresolve( CHAR_DATA * ch, char *argument )
{
   sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

   if( sysdata.NO_NAME_RESOLVING )
      send_to_char_color( "&YName resolving disabled.\n\r", ch );
   else
      send_to_char_color( "&YName resolving enabled.\n\r", ch );
   return;
}

void do_users( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   DESCRIPTOR_DATA *d;
   int count;
   char arg[MAX_INPUT_LENGTH];

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   count = 0;
   buf[0] = '\0';

   sprintf( buf, "\n\rDesc|Con|Idle| Port | Player      @HostIP           " );
   strcat( buf, "| Username" );
   strcat( buf, "\n\r" );
   strcat( buf, "----+---+----+------+-------------------------------" );
   strcat( buf, "+---------" );
   strcat( buf, "\n\r" );
   send_to_pager( buf, ch );

   for( d = first_descriptor; d; d = d->next )
   {
      if( arg[0] == '\0' )
      {
         count++;
         sprintf( buf,
                  " %3d| %2d|%4d|%6d| %-12s@%-16s ",
                  d->descriptor,
                  d->connected,
                  d->idle / 4,
                  d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)", d->host );
         sprintf( buf + strlen( buf ), "| %s", d->user );
         strcat( buf, "\n\r" );
         send_to_pager( buf, ch );
      }
      else
      {
         if( !str_prefix( arg, d->host ) || ( d->character && !str_prefix( arg, d->character->name ) ) )
         {
            count++;
            pager_printf( ch,
                          " %3d| %2d|%4d|%6d| %-12s@%-16s ",
                          d->descriptor,
                          d->connected,
                          d->idle / 4,
                          d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)", d->host );
            buf[0] = '\0';
            sprintf( buf, "| %s", d->user );
            strcat( buf, "\n\r" );
            send_to_pager( buf, ch );
         }
      }
   }
   pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
   return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Force whom to do what?\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) )
         {
            act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
            interpret( vch, argument, TRUE );
         }
      }
   }
   else
   {
      CHAR_DATA *victim;

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\n\r", ch );
         return;
      }

      if( victim == ch )
      {
         send_to_char( "Aye aye, right away!\n\r", ch );
         return;
      }

      act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
      interpret( victim, argument, FALSE );
   }

   send_to_char( "Ok.\n\r", ch );
   return;
}

void do_invis( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   sh_int level;
   OBJ_DATA *obj;

   if( ch->curr_talent[TAL_ILLUSION] < 10 )
   {
      send_to_char( "Nothing happens.\n\r", ch );
      return;
   }

   set_char_color( AT_MAGIC, ch );

   argument = one_argument( argument, arg );
   if( arg != NULL && arg[0] != '\0' )
   {
      if( !is_number( arg ) && ch->curr_talent[TAL_ILLUSION] >= 30 )
      {
         obj = get_obj_carry( ch, arg );

         if( obj )
         {
            separate_obj( obj );
            if( IS_OBJ_STAT( obj, ITEM_INVIS ) )
            {
               xREMOVE_BIT( obj->extra_flags, ITEM_INVIS );
               act( AT_MAGIC, "$p fades into existence.", ch, obj, NULL, TO_CHAR );
            }
            else
            {
               xSET_BIT( obj->extra_flags, ITEM_INVIS );
               act( AT_MAGIC, "$p fades out of existence.", ch, obj, NULL, TO_CHAR );
            }
            use_magic( ch, TAL_ILLUSION, 30 );
            WAIT_STATE( ch, PULSE_VIOLENCE );
            return;
         }
         else
         {
            send_to_char( "You dont have that.\n\r", ch );
            return;
         }
      }
      level = atoi( arg );
      if( level > TALENT( ch, TAL_ILLUSION ) )
      {
         send_to_char( "Invalid level.\n\r", ch );
         return;
      }

      if( !IS_NPC( ch ) )
      {
         ch->pcdata->wizinvis = level;
         ch_printf( ch, "Invisibility level set to %d.\n\r", level );
      }

      return;
   }

   if( xIS_SET( ch->act, PLR_WIZINVIS ) )
   {
      xREMOVE_BIT( ch->act, PLR_WIZINVIS );
      act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\n\r", ch );
   }
   else
   {
      if( ch->pcdata->wizinvis < 1 || ch->pcdata->wizinvis > TALENT( ch, TAL_ILLUSION ) )
         ch->pcdata->wizinvis = TALENT( ch, TAL_ILLUSION );
      act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\n\r", ch );
      xSET_BIT( ch->act, PLR_WIZINVIS );
   }
   use_magic( ch, TAL_ILLUSION, TAL_ILLUSION );
   WAIT_STATE( ch, PULSE_VIOLENCE );
   return;
}


void do_holylight( CHAR_DATA * ch, char *argument )
{

   set_char_color( AT_IMMORT, ch );

   if( IS_NPC( ch ) )
      return;

   if( xIS_SET( ch->act, PLR_HOLYLIGHT ) )
   {
      xREMOVE_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode off.\n\r", ch );
   }
   else
   {
      xSET_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode on.\n\r", ch );
   }
   return;
}

void do_rassign( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   int r_lo, r_hi;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   r_lo = atoi( arg2 );
   r_hi = atoi( arg3 );

   if( arg1[0] == '\0' || r_lo < 0 || r_hi < 0 )
   {
      send_to_char( "Syntax: rassign <who> <low> <high>\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They don't seem to be around.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) || !IS_SET( victim->pcdata->permissions, PERMIT_BUILD ) )
   {
      send_to_char( "They wouldn't know what to do with a room range.\n\r", ch );
      return;
   }
   if( r_lo > r_hi )
   {
      send_to_char( "Unacceptable room range.\n\r", ch );
      return;
   }
   if( r_lo == 0 )
      r_hi = 0;
   victim->pcdata->r_range_lo = r_lo;
   victim->pcdata->r_range_hi = r_hi;
   assign_area( victim );
   send_to_char( "Done.\n\r", ch );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has assigned you the room vnum range %d - %d.\n\r", ch->name, r_lo, r_hi );
   assign_area( victim );  /* Put back by Thoric on 02/07/96 */
   if( !victim->pcdata->area )
   {
      bug( "rassign: assign_area failed", 0 );
      return;
   }

   if( r_lo == 0 )   /* Scryn 8/12/95 */
   {
      REMOVE_BIT( victim->pcdata->area->status, AREA_LOADED );
      SET_BIT( victim->pcdata->area->status, AREA_DELETED );
   }
   else
   {
      SET_BIT( victim->pcdata->area->status, AREA_LOADED );
      REMOVE_BIT( victim->pcdata->area->status, AREA_DELETED );
   }
   return;
}

void do_oassign( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   int o_lo, o_hi;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   o_lo = atoi( arg2 );
   o_hi = atoi( arg3 );

   if( arg1[0] == '\0' || o_lo < 0 || o_hi < 0 )
   {
      send_to_char( "Syntax: oassign <who> <low> <high>\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They don't seem to be around.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) || !IS_SET( victim->pcdata->permissions, PERMIT_ITEM ) )
   {
      send_to_char( "They wouldn't know what to do with an object range.\n\r", ch );
      return;
   }
   if( o_lo > o_hi )
   {
      send_to_char( "Unacceptable object range.\n\r", ch );
      return;
   }
   victim->pcdata->o_range_lo = o_lo;
   victim->pcdata->o_range_hi = o_hi;
   assign_area( victim );
   send_to_char( "Done.\n\r", ch );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has assigned you the object vnum range %d - %d.\n\r", ch->name, o_lo, o_hi );
   return;
}

void do_massign( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   int m_lo, m_hi;
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   m_lo = atoi( arg2 );
   m_hi = atoi( arg3 );

   if( arg1[0] == '\0' || m_lo < 0 || m_hi < 0 )
   {
      send_to_char( "Syntax: massign <who> <low> <high>\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They don't seem to be around.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) || !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
   {
      send_to_char( "They wouldn't know what to do with a monster range.\n\r", ch );
      return;
   }
   if( m_lo > m_hi )
   {
      send_to_char( "Unacceptable monster range.\n\r", ch );
      return;
   }
   victim->pcdata->m_range_lo = m_lo;
   victim->pcdata->m_range_hi = m_hi;
   assign_area( victim );
   send_to_char( "Done.\n\r", ch );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has assigned you the monster vnum range %d - %d.\n\r", ch->name, m_lo, m_hi );
   return;
}

void do_cmdtable( CHAR_DATA * ch, char *argument )
{
   int hash, cnt;
   CMDTYPE *cmd;
   char arg[MAX_INPUT_LENGTH];

   one_argument( argument, arg );

   if( strcmp( arg, "lag" ) ) /* display normal command table */
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Commands and Number of Uses This Run\n\r", ch );
      set_pager_color( AT_PLAIN, ch );
      for( cnt = hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
         {
            if( ( ++cnt ) % 4 )
               pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses );
            else
               pager_printf( ch, "%-6.6s %4d\n\r", cmd->name, cmd->userec.num_uses );
         }
      send_to_char( "\n\r", ch );
   }
   else  /* display commands causing lag */
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Commands that have caused lag this run\n\r", ch );
      set_pager_color( AT_PLAIN, ch );
      for( cnt = hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
         {
            if( !cmd->lag_count )
               continue;
            else if( ( ++cnt ) % 4 )
               pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->lag_count );
            else
               pager_printf( ch, "%-6.6s %4d\n\r", cmd->name, cmd->lag_count );
         }
      send_to_char( "\n\r", ch );
   }

   return;
}

/*
 * Load up a player file
 */
void do_loadup( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *temp;
   char fname[1024];
   char name[256];
   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MAX_STRING_LENGTH];

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, name );
   if( name[0] == '\0' )
   {
      send_to_char( "Usage: loadup <playername>\n\r", ch );
      return;
   }
   for( temp = first_char; temp; temp = temp->next )
   {
      if( IS_NPC( temp ) )
         continue;
      if( can_see( ch, temp ) && !str_cmp( name, temp->name ) )
         break;
   }
   if( temp != NULL )
   {
      send_to_char( "They are already playing.\n\r", ch );
      return;
   }
   name[0] = UPPER( name[0] );
   sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( (int)name[0] ), capitalize( name ) );

   if( stat( fname, &fst ) != -1 )
   {
      CREATE( d, DESCRIPTOR_DATA, 1 );
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE( d->outbuf, char, d->outsize );

      loaded = load_char_obj( d, name, FALSE );
      add_char( d->character );
      old_room_vnum = d->character->in_room->vnum;
      char_to_room( d->character, ch->in_room );
      d->character->desc = NULL;
      d->character->retran = old_room_vnum;
      d->character = NULL;
      DISPOSE( d->outbuf );
      DISPOSE( d );
      ch_printf( ch, "Player %s loaded from room %d.\n\r", capitalize( name ), old_room_vnum );
      sprintf( buf, "%s appears from nowhere, eyes glazed over.\n\r", capitalize( name ) );
      act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   /*
    * else no player file 
    */
   send_to_char( "No such player.\n\r", ch );
   return;
}

void do_fixchar( CHAR_DATA * ch, char *argument )
{
   char name[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   set_char_color( AT_IMMORT, ch );

   one_argument( argument, name );
   if( name[0] == '\0' )
      victim = ch;
   else
      victim = get_char_room( ch, name );

   if( !victim )
   {
      send_to_char( "They're not here.\n\r", ch );
      return;
   }
   fix_char( victim );
   send_to_char( "Stats fixed.\n\r", ch );
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names( char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;

   *out = '\0';
   while( inp && *inp )
   {
      inp = one_argument( inp, buf );
      if( ( len = strlen( buf ) ) >= 5 && !strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            strcat( out, " " );
         strcat( out, buf );
      }
   }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names( char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;

   *out = '\0';
   while( inp && *inp )
   {
      inp = one_argument( inp, buf );
      if( ( len = strlen( buf ) ) < 5 || strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            strcat( out, " " );
         strcat( out, buf );
      }
   }
}

struct tm *update_time( struct tm *old_time )
{
   time_t time;

   time = mktime( old_time );
   return localtime( &time );
}

void do_set_boot_time( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   bool check;

   check = FALSE;
   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch );
      send_to_char( "        setboot manual {0/1}\n\r", ch );
      send_to_char( "        setboot default\n\r", ch );
      ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\n\r", reboot_time, set_boot_time->manual );
      return;
   }

   if( !str_cmp( arg, "time" ) )
   {
      struct tm *now_time;

      argument = one_argument( argument, arg );
      argument = one_argument( argument, arg1 );
      if( !*arg || !*arg1 || !is_number( arg ) || !is_number( arg1 ) )
      {
         send_to_char( "You must input a value for hour and minute.\n\r", ch );
         return;
      }

      now_time = localtime( &current_time );
      if( ( now_time->tm_hour = atoi( arg ) ) < 0 || now_time->tm_hour > 23 )
      {
         send_to_char( "Valid range for hour is 0 to 23.\n\r", ch );
         return;
      }
      if( ( now_time->tm_min = atoi( arg1 ) ) < 0 || now_time->tm_min > 59 )
      {
         send_to_char( "Valid range for minute is 0 to 59.\n\r", ch );
         return;
      }

      argument = one_argument( argument, arg );
      if( *arg != '\0' && is_number( arg ) )
      {
         if( ( now_time->tm_mday = atoi( arg ) ) < 1 || now_time->tm_mday > 31 )
         {
            send_to_char( "Valid range for day is 1 to 31.\n\r", ch );
            return;
         }
         argument = one_argument( argument, arg );
         if( *arg != '\0' && is_number( arg ) )
         {
            if( ( now_time->tm_mon = atoi( arg ) ) < 1 || now_time->tm_mon > 12 )
            {
               send_to_char( "Valid range for month is 1 to 12.\n\r", ch );
               return;
            }
            now_time->tm_mon--;
            argument = one_argument( argument, arg );
            if( ( now_time->tm_year = atoi( arg ) - 1900 ) < 0 || now_time->tm_year > 199 )
            {
               send_to_char( "Valid range for year is 1900 to 2099.\n\r", ch );
               return;
            }
         }
      }

      now_time->tm_sec = 0;
      if( mktime( now_time ) < current_time )
      {
         send_to_char( "You can't set a time previous to today!\n\r", ch );
         return;
      }
      if( set_boot_time->manual == 0 )
         set_boot_time->manual = 1;
      new_boot_time = update_time( now_time );
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check( mktime( new_boot_time ) );
      get_reboot_string(  );

      ch_printf( ch, "Boot time set to %s\n\r", reboot_time );
      check = TRUE;
   }
   else if( !str_cmp( arg, "manual" ) )
   {
      argument = one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Please enter a value for manual boot on/off\n\r", ch );
         return;
      }
      if( !is_number( arg1 ) )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\n\r", ch );
         return;
      }
      if( atoi( arg1 ) < 0 || atoi( arg1 ) > 1 )
      {
         send_to_char( "Value for manual must be 0 (off) or 1 (on)\n\r", ch );
         return;
      }

      set_boot_time->manual = atoi( arg1 );
      ch_printf( ch, "Manual bit set to %s\n\r", arg1 );
      check = TRUE;
      get_reboot_string(  );
      return;
   }

   else if( !str_cmp( arg, "default" ) )
   {
      set_boot_time->manual = 0;
      /*
       * Reinitialize new_boot_time 
       */
      new_boot_time = localtime( &current_time );
      new_boot_time->tm_mday += 1;
      if( new_boot_time->tm_hour > 12 )
         new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time( new_boot_time );

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char( "Reboot time set back to normal.\n\r", ch );
      check = TRUE;
   }

   if( !check )
   {
      send_to_char( "Invalid argument for setboot.\n\r", ch );
      return;
   }
   else
   {
      get_reboot_string(  );
      new_boot_time_t = mktime( new_boot_time );
   }
}

/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to
 * pfiles and the correct password
 */
void do_form_password( CHAR_DATA * ch, char *argument )
{
   char *pwcheck, *p;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: formpass <password>\n\r", ch );
      return;
   }

   /*
    * This is arbitrary to discourage weak passwords 
    */
   if( strlen( argument ) < 5 )
   {
      send_to_char( "Usage: formpass <password>\n\r", ch );
      send_to_char( "New password must be at least 5 characters in length.\n\r", ch );
      return;
   }

   if( argument[0] == '!' )
   {
      send_to_char( "Usage: formpass <password>\n\r", ch );
      send_to_char( "New password cannot begin with the '!' character.\n\r", ch );
      return;
   }

   pwcheck = smaug_crypt( argument );
   for( p = pwcheck; *p != '\0'; p++ )
   {
      if( *p == '~' )
      {
         send_to_char( "New password not acceptable, cannot use the ~ character.\n\r", ch );
         return;
      }
   }
   ch_printf( ch, "%s results in the encrypted string: %s\n\r", argument, pwcheck );
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "If you want to destroy a character, spell it out!\n\r", ch );
   return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA * pArea )
{
   extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
   extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
   extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
   CHAR_DATA *ech;
   CHAR_DATA *ech_next;
   OBJ_DATA *eobj;
   OBJ_DATA *eobj_next;
   int icnt;
   ROOM_INDEX_DATA *rid;
   ROOM_INDEX_DATA *rid_next;
   OBJ_INDEX_DATA *oid;
   OBJ_INDEX_DATA *oid_next;
   MOB_INDEX_DATA *mid;
   MOB_INDEX_DATA *mid_next;
   EXTRA_DESCR_DATA *eed;
   EXTRA_DESCR_DATA *eed_next;
   EXIT_DATA *exit;
   EXIT_DATA *exit_next;
   MPROG_ACT_LIST *mpact;
   MPROG_ACT_LIST *mpact_next;
   MPROG_DATA *mprog;
   MPROG_DATA *mprog_next;
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;

   for( ech = first_char; ech; ech = ech_next )
   {
      ech_next = ech->next;

      if( IS_NPC( ech ) )
      {
         /*
          * if mob is in area, or part of area. 
          */
         if( URANGE( pArea->low_m_vnum, ech->pIndexData->vnum,
                     pArea->hi_m_vnum ) == ech->pIndexData->vnum || ( ech->in_room && ech->in_room->area == pArea ) )
            extract_char( ech, TRUE );
         continue;
      }
      if( ech->in_room && ech->in_room->area == pArea )
         do_recall( ech, "" );
   }
   for( eobj = first_object; eobj; eobj = eobj_next )
   {
      eobj_next = eobj->next;
      /*
       * if obj is in area, or part of area. 
       */
      if( URANGE( pArea->low_o_vnum, eobj->pIndexData->vnum,
                  pArea->hi_o_vnum ) == eobj->pIndexData->vnum || ( eobj->in_room && eobj->in_room->area == pArea ) )
         extract_obj( eobj );
   }
   for( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
   {
      for( rid = room_index_hash[icnt]; rid; rid = rid_next )
      {
         rid_next = rid->next;

         for( exit = rid->first_exit; exit; exit = exit_next )
         {
            exit_next = exit->next;
            if( rid->area == pArea || exit->to_room->area == pArea )
            {
               STRFREE( exit->keyword );
               STRFREE( exit->description );
               UNLINK( exit, rid->first_exit, rid->last_exit, next, prev );
               DISPOSE( exit );
               /*
                * Crash bug fix.  I know it could go from the start several times
                * * But you CAN NOT iterate over a link-list and DELETE from it or
                * * Nasty things can and will happen. --Shaddai
                */
               exit = rid->first_exit;
            }
         }
         if( rid->area != pArea )
            continue;
         STRFREE( rid->name );
         STRFREE( rid->description );
         if( rid->first_person )
         {
            bug( "close_area: room with people #%d", rid->vnum );
            for( ech = rid->first_person; ech; ech = ech_next )
            {
               ech_next = ech->next_in_room;
               if( IS_NPC( ech ) )
                  extract_char( ech, TRUE );
               else
                  do_recall( ech, "" );
            }
         }
         if( rid->first_content )
         {
            bug( "close_area: room with contents #%d", rid->vnum );
            for( eobj = rid->first_content; eobj; eobj = eobj_next )
            {
               eobj_next = eobj->next_content;
               extract_obj( eobj );
            }
         }
         for( eed = rid->first_extradesc; eed; eed = eed_next )
         {
            eed_next = eed->next;
            STRFREE( eed->keyword );
            STRFREE( eed->description );
            DISPOSE( eed );
         }
         for( mpact = rid->mpact; mpact; mpact = mpact_next )
         {
            mpact_next = mpact->next;
            STRFREE( mpact->buf );
            DISPOSE( mpact );
         }
         for( mprog = rid->mudprogs; mprog; mprog = mprog_next )
         {
            mprog_next = mprog->next;
            STRFREE( mprog->arglist );
            STRFREE( mprog->comlist );
            DISPOSE( mprog );
         }
         if( rid == room_index_hash[icnt] )
            room_index_hash[icnt] = rid->next;
         else
         {
            ROOM_INDEX_DATA *trid;

            for( trid = room_index_hash[icnt]; trid; trid = trid->next )
               if( trid->next == rid )
                  break;
            if( !trid )
               bug( "Close_area: rid not in hash list %d", rid->vnum );
            else
               trid->next = rid->next;
         }
         DISPOSE( rid );
      }

      for( mid = mob_index_hash[icnt]; mid; mid = mid_next )
      {
         mid_next = mid->next;

         if( mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum )
            continue;

         STRFREE( mid->player_name );
         STRFREE( mid->short_descr );
         STRFREE( mid->description );
         for( mprog = mid->mudprogs; mprog; mprog = mprog_next )
         {
            mprog_next = mprog->next;
            STRFREE( mprog->arglist );
            STRFREE( mprog->comlist );
            DISPOSE( mprog );
         }
         if( mid == mob_index_hash[icnt] )
            mob_index_hash[icnt] = mid->next;
         else
         {
            MOB_INDEX_DATA *tmid;

            for( tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next )
               if( tmid->next == mid )
                  break;
            if( !tmid )
               bug( "Close_area: mid not in hash list %d", mid->vnum );
            else
               tmid->next = mid->next;
         }
         DISPOSE( mid );
      }

      for( oid = obj_index_hash[icnt]; oid; oid = oid_next )
      {
         oid_next = oid->next;

         if( oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum )
            continue;

         STRFREE( oid->name );
         STRFREE( oid->short_descr );
         STRFREE( oid->description );
         STRFREE( oid->action_desc );

         for( eed = oid->first_extradesc; eed; eed = eed_next )
         {
            eed_next = eed->next;
            STRFREE( eed->keyword );
            STRFREE( eed->description );
            DISPOSE( eed );
         }
         for( paf = oid->first_affect; paf; paf = paf_next )
         {
            paf_next = paf->next;
            DISPOSE( paf );
         }
         for( mprog = oid->mudprogs; mprog; mprog = mprog_next )
         {
            mprog_next = mprog->next;
            STRFREE( mprog->arglist );
            STRFREE( mprog->comlist );
            DISPOSE( mprog );
         }
         if( oid == obj_index_hash[icnt] )
            obj_index_hash[icnt] = oid->next;
         else
         {
            OBJ_INDEX_DATA *toid;

            for( toid = obj_index_hash[icnt]; toid; toid = toid->next )
               if( toid->next == oid )
                  break;
            if( !toid )
               bug( "Close_area: oid not in hash list %d", oid->vnum );
            else
               toid->next = oid->next;
         }
         DISPOSE( oid );
      }
   }
   DISPOSE( pArea->name );
   DISPOSE( pArea->filename );
   STRFREE( pArea->author );
   UNLINK( pArea, first_build, last_build, next, prev );
   UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
   DISPOSE( pArea );
}

void do_destroy( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char *name;
   AREA_DATA *pArea;
   OBJ_DATA *obj;

   set_char_color( AT_RED, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Destroy what player file?\n\r", ch );
      return;
   }

   for( victim = first_char; victim; victim = victim->next )
      if( !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
         break;

   if( !victim )
   {
      DESCRIPTOR_DATA *d;

      /*
       * Make sure they aren't halfway logged in. 
       */
      for( d = first_descriptor; d; d = d->next )
         if( ( victim = d->character ) && !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
            break;
      if( d )
         close_socket( d, TRUE );
   }
   else
   {
      int x, y;

      do_channels( victim, "quiet" );

      quitting_char = victim;
      save_char_obj( victim );
      saving_char = NULL;
      /*
       * adjust the hiscore tables -keo 
       */
      adjust_hiscore( "hours", victim, 0 );
      adjust_hiscore( "mkill", victim, 0 );
      adjust_hiscore( "mdeath", victim, 0 );
      adjust_hiscore( "fire", victim, 0 );
      adjust_hiscore( "wind", victim, 0 );
      adjust_hiscore( "earth", victim, 0 );
      adjust_hiscore( "frost", victim, 0 );
      adjust_hiscore( "lightning", victim, 0 );
      adjust_hiscore( "water", victim, 0 );
      adjust_hiscore( "dream", victim, 0 );
      adjust_hiscore( "speech", victim, 0 );
      adjust_hiscore( "death", victim, 0 );
      adjust_hiscore( "time", victim, 0 );
      adjust_hiscore( "motion", victim, 0 );
      adjust_hiscore( "mind", victim, 0 );
      adjust_hiscore( "illusion", victim, 0 );
      adjust_hiscore( "seeking", victim, 0 );
      adjust_hiscore( "security", victim, 0 );
      adjust_hiscore( "catalysm", victim, 0 );
      adjust_hiscore( "void", victim, 0 );

      act( AT_PLAIN, "$n writhes and screams as $e is extracted from the matrix.", victim, NULL, NULL, TO_ROOM );
      act( AT_PLAIN, "You are wracked with uncontrollable pain as you are extracted from the matrix.", victim, NULL, NULL,
           TO_CHAR );
      while( ( obj = victim->last_carrying ) != NULL )
         extract_obj( obj );

      extract_char( victim, TRUE );
      for( x = 0; x < MAX_WEAR; x++ )
         for( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;
   }

   name = capitalize( arg );
   sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( (int)arg[0] ), name );
   sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower( (int)arg[0] ), name );
   if( !rename( buf, buf2 ) )
   {
      set_char_color( AT_RED, ch );
      ch_printf( ch, "Player %s destroyed.  Pfile saved in backup directory.\n\r", name );
	  
	  sprintf( buf, "%s%s", HOUSE_DIR, name );
                if ( !remove( buf ) )
                   send_to_char( "Player's housing data destroyed.\n\r", ch );
                else if ( errno != ENOENT )
                {
                   ch_printf( ch, "Unknown error #%d - %s (housing data)."
                                  "  Report to Coder.\n\r", errno, strerror( errno ) );
                   sprintf( buf2, "%s destroying %s", ch->name, buf );
                   perror( buf2 );
                }

      sprintf( buf2, "%s.are", name );
      for( pArea = first_build; pArea; pArea = pArea->next )
         if( !str_cmp( pArea->filename, buf2 ) )
         {
            sprintf( buf, "%s%s", BUILD_DIR, buf2 );
            if( IS_SET( pArea->status, AREA_LOADED ) )
               fold_area( pArea, buf, FALSE );
            close_area( pArea );
            sprintf( buf2, "%s.bak", buf );
            set_char_color( AT_RED, ch ); /* Log message changes colors */
            if( !rename( buf, buf2 ) )
               send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch );
            else if( errno != ENOENT )
            {
               ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Scion.\n\r", errno, strerror( errno ) );
               sprintf( buf2, "%s destroying %s", ch->name, buf );
               perror( buf2 );
            }
            break;
         }
   }
   else if( errno == ENOENT )
   {
      set_char_color( AT_PLAIN, ch );
      send_to_char( "Player does not exist.\n\r", ch );
   }
   else
   {
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "Unknown error #%d - %s.  Report to Keolah.\n\r", errno, strerror( errno ) );
      sprintf( buf, "%s destroying %s", ch->name, arg );
      perror( buf );
   }
   return;
}

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];   /* db.c */

/* Super-AT command:
FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>

Executes action several times, either on ALL players (not including yourself),
MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example:

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *name_expand( CHAR_DATA * ch )
{
   int count = 1;
   CHAR_DATA *rch;
   char name[MAX_INPUT_LENGTH];  /*  HOPEFULLY no mob has a name longer than THAT */

   static char outbuf[MAX_INPUT_LENGTH];

   if( !IS_NPC( ch ) )
      return ch->name;

   one_argument( ch->name, name );  /* copy the first word into name */

   if( !name[0] ) /* weird mob .. no keywords */
   {
      strcpy( outbuf, "" );   /* Do not return NULL, just an empty buffer */
      return outbuf;
   }

   /*
    * ->people changed to ->first_person -- TRI 
    */
   for( rch = ch->in_room->first_person; rch && ( rch != ch ); rch = rch->next_in_room )
      if( is_name( name, rch->name ) )
         count++;


   sprintf( outbuf, "%d.%s", count, name );
   return outbuf;
}

void do_for( CHAR_DATA * ch, char *argument )
{
   char range[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   bool fPlayers = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
   ROOM_INDEX_DATA *room, *old_room;
   CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
   int i;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, range );
   if( !range[0] || !argument[0] )  /* invalid usage? */
   {
      send_to_char( "Invalid usage.\n\r", ch );
      return;
   }

   if( !str_prefix( "quit", argument ) )
   {
      send_to_char( "Are you trying to crash the MUD or something?\n\r", ch );
      return;
   }


   if( !str_cmp( range, "all" ) )
      fPlayers = TRUE;
   else if( !str_cmp( range, "mobs" ) )
      fMobs = TRUE;
   else if( !str_cmp( range, "everywhere" ) )
      fEverywhere = TRUE;
   else
      send_to_char( "Incorrect target.\n\r", ch );

   /*
    * do not allow # to make it easier 
    */
   if( fEverywhere && strchr( argument, '#' ) )
   {
      send_to_char( "Cannot use FOR EVERYWHERE with the # thingie.\n\r", ch );
      return;
   }

   set_char_color( AT_PLAIN, ch );
   if( strchr( argument, '#' ) ) /* replace # ? */
   {
      /*
       * char_list - last_char, p_next - gch_prev -- TRI 
       */
      for( p = last_char; p; p = p_prev )
      {
         p_prev = p->prev; /* TRI */
         /*
          * p_next = p->next; 
          *//*
          * In case someone DOES try to AT MOBS SLAY # 
          */
         found = FALSE;

         if( !( p->in_room ) || room_is_private( p->in_room ) || ( p == ch ) )
            continue;

         if( IS_NPC( p ) && fMobs )
            found = TRUE;
         else if( !IS_NPC( p ) && fPlayers )
            found = TRUE;

         /*
          * It looks ugly to me.. but it works :) 
          */
         if( found ) /* p is 'appropriate' */
         {
            char *pSource = argument;  /* head of buffer to be parsed */
            char *pDest = buf;   /* parse into this */

            while( *pSource )
            {
               if( *pSource == '#' )   /* Replace # with name of target */
               {
                  const char *namebuf = name_expand( p );

                  if( namebuf )  /* in case there is no mob name ?? */
                     while( *namebuf ) /* copy name over */
                        *( pDest++ ) = *( namebuf++ );

                  pSource++;
               }
               else
                  *( pDest++ ) = *( pSource++ );
            }  /* while */
            *pDest = '\0'; /* Terminate */

            /*
             * Execute 
             */
            old_room = ch->in_room;
            char_from_room( ch );
            char_to_room( ch, p->in_room );
            interpret( ch, buf, FALSE );
            char_from_room( ch );
            char_to_room( ch, old_room );

         }  /* if found */
      }  /* for every char */
   }
   else  /* just for every room with the appropriate people in it */
   {
      for( i = 0; i < MAX_KEY_HASH; i++ ) /* run through all the buckets */
         for( room = room_index_hash[i]; room; room = room->next )
         {
            found = FALSE;

            /*
             * Anyone in here at all? 
             */
            if( fEverywhere ) /* Everywhere executes always */
               found = TRUE;
            else if( !room->first_person )   /* Skip it if room is empty */
               continue;
            /*
             * ->people changed to first_person -- TRI 
             */

            /*
             * Check if there is anyone here of the requried type 
             */
            /*
             * Stop as soon as a match is found or there are no more ppl in room 
             */
            /*
             * ->people to ->first_person -- TRI 
             */
            for( p = room->first_person; p && !found; p = p->next_in_room )
            {

               if( p == ch )  /* do not execute on oneself */
                  continue;

               if( IS_NPC( p ) && fMobs )
                  found = TRUE;
               else if( !IS_NPC( p ) )
                  found = TRUE;
            }  /* for everyone inside the room */

            if( found && !room_is_private( room ) )   /* Any of the required type here AND room not private? */
            {
               /*
                * This may be ineffective. Consider moving character out of old_room
                * once at beginning of command then moving back at the end.
                * This however, is more safe?
                */

               old_room = ch->in_room;
               char_from_room( ch );
               char_to_room( ch, room );
               interpret( ch, argument, FALSE );
               char_from_room( ch );
               char_to_room( ch, old_room );
            }  /* if found */
         }  /* for every room in a bucket */
   }  /* if strchr */
}  /* do_for */

void save_sysdata args( ( SYSTEM_DATA sys ) );

void do_cset( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];
   sh_int level;

   set_pager_color( AT_PLAIN, ch );

   if( argument[0] == '\0' )
   {
      pager_printf_color( ch, "\n\r&WMud_name: %s      Support email: %s\n\r", sysdata.mud_name, sysdata.email );
      pager_printf_color( ch, "&wAutosave frequency (minutes):  &W%d\n\r", sysdata.save_frequency );
      pager_printf_color( ch, "&wSaving Pets is:                &W%s\n\r", ( sysdata.save_pets ) ? "ON" : "off" );
      pager_printf_color( ch, "  &wSave flags: &W%s\n\r", flag_string( sysdata.save_flags, save_flag ) );
      pager_printf_color( ch, "  &wQuests: &W%s\n\r", flag_string( sysdata.quest, quest_flag ) );
      pager_printf_color( ch, "  &wIdents retries: &W%d\n\r", sysdata.ident_retries );
      pager_printf( ch, "Alias Wait: %d pulses\n\r", sysdata.alias_wait );
      pager_printf_color( ch, "  &wCrashGuard: &W%s\n\r", ( sysdata.crashguard == TRUE ) ? "ACTIVE" : "inactive" );
      pager_printf_color( ch, "  &wMultiple Mortals: &W%d\r\n", sysdata.morts_allowed );
      pager_printf_color( ch, "  &wExpbase: &W%d\r\n", sysdata.exp_base );
      return;
   }

   argument = one_argument( argument, arg );
   smash_tilde( argument );

   if( !str_cmp( arg, "save" ) )
   {
      save_sysdata( sysdata );
      send_to_char( "Cset functions saved.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "mudname" ) )
   {
      if( sysdata.mud_name )
         DISPOSE( sysdata.mud_name );
      sysdata.mud_name = str_dup( argument );
      send_to_char( "Name set.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "email" ) )
   {
      if( sysdata.email )
         DISPOSE( sysdata.email );
      sysdata.email = str_dup( argument );
      send_to_char( "Support email set.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "saveflag" ) )
   {
      int x = get_saveflag( argument );

      if( x == -1 )
         send_to_char( "Not a save flag.\n\r", ch );
      else
      {
         TOGGLE_BIT( sysdata.save_flags, 1 << x );
         send_to_char( "Ok.\n\r", ch );
      }
      return;
   }

   if( !str_cmp( arg, "quest" ) )
   {
      int x = get_questflag( argument );

      if( x == -1 )
         send_to_char( "Not a quest.\n\r", ch );
      else
      {
         TOGGLE_BIT( sysdata.quest, 1 << x );
         send_to_char( "Ok.\n\r", ch );
      }
      return;
   }

   level = ( sh_int ) atoi( argument );

   if( !str_prefix( arg, "aliaswait" ) )
   {
      sysdata.alias_wait = level;
      ch_printf( ch, "The Alias wait is now %d pulses.\n\r", sysdata.alias_wait );
      return;
   }
   if( !str_prefix( arg, "multiplay" ) )
   {
      if( level < 1 )
      {
         ch_printf( ch, "You must enter a positive number.\r\nUse wizlock to keep mortals out of the game.\r\n" );
         return;
      }
      sysdata.morts_allowed = level;
      ch_printf( ch, "Multiplaying is set to a maximum of %d characters.\r\n", sysdata.morts_allowed );
      return;
   }


   if( !str_cmp( arg, "expbase" ) )
   {
      sysdata.exp_base = level;
      ch_printf( ch, "Expbase set to %d.\r\n", sysdata.exp_base );
      return;
   }


   if( !str_prefix( arg, "savefrequency" ) )
   {
      sysdata.save_frequency = level;
      send_to_char( "Ok.\n\r", ch );
      return;
   }

   if( !str_cmp( arg, "ident_retries" ) || !str_cmp( arg, "ident" ) )
   {
      sysdata.ident_retries = level;
      if( level > 20 )
         send_to_char( "Caution:  This setting may cause the game to lag.\n\r", ch );
      else if( level <= 0 )
         send_to_char( "Ident lookups turned off.\n\r", ch );
      else
         send_to_char( "Ok.\n\r", ch );
      return;
   }

   else if( !str_cmp( arg, "petsave" ) )
   {
      if( level )
         sysdata.save_pets = TRUE;
      else
         sysdata.save_pets = FALSE;
   }

   else if( !str_cmp( arg, "crashguard" ) )
   {
      if( sysdata.crashguard == TRUE )
      {
         send_to_char( "Crashguard disabled.\r\n", ch );
         sysdata.crashguard = FALSE;
      }
      else
      {
         send_to_char( "Crashguard enabled.\r\n", ch );
         sysdata.crashguard = TRUE;
      }
   }

   else
   {
      send_to_char( "Invalid argument.\n\r", ch );
      return;
   }
   send_to_char( "Ok.\n\r", ch );
   return;
}

void get_reboot_string( void )
{
   sprintf( reboot_time, "%s", asctime( new_boot_time ) );
}

void do_orange( CHAR_DATA * ch, char *argument )
{
   send_to_char_color( "&YFunction under construction.\n\r", ch );
   return;
}

void do_mrange( CHAR_DATA * ch, char *argument )
{
   send_to_char_color( "&YFunction under construction.\n\r", ch );
   return;
}

void do_hell( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   sh_int time;
   bool h_d = FALSE;
   struct tm *tms;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Hell who, and for how long?\n\r", ch );
      return;
   }
   if( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( victim->pcdata->release_date != 0 )
   {
      ch_printf( ch, "They are already in hell until %24.24s, by %s.\n\r",
                 ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
      return;
   }

   argument = one_argument( argument, arg );
   if( !*arg || !is_number( arg ) )
   {
      send_to_char( "Hell them for how long?\n\r", ch );
      return;
   }

   time = atoi( arg );
   if( time <= 0 )
   {
      send_to_char( "You cannot hell for zero or negative time.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "hours" ) )
      h_d = TRUE;
   else if( str_cmp( arg, "days" ) )
   {
      send_to_char( "Is that value in hours or days?\n\r", ch );
      return;
   }
   else if( time > 30 )
   {
      send_to_char( "You may not hell a person for more than 30 days at a time.\n\r", ch );
      return;
   }
   tms = localtime( &current_time );

   if( h_d )
      tms->tm_hour += time;
   else
      tms->tm_mday += time;
   victim->pcdata->release_date = mktime( tms );
   victim->pcdata->helled_by = STRALLOC( ch->name );
   ch_printf( ch, "%s will be released from hell at %24.24s.\n\r", victim->name, ctime( &victim->pcdata->release_date ) );
   act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   /*
    * fix hell bug - shogar 
    */
   char_to_room( victim, get_room_index( ROOM_VNUM_HELL ) );
   act( AT_MAGIC, "$n appears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );
   do_look( victim, "auto" );
   ch_printf( victim, "The immortals are not pleased with your actions.\n\r"
              "You shall remain in hell for %d %s%s.\n\r", time, ( h_d ? "hour" : "day" ), ( time == 1 ? "" : "s" ) );
   save_char_obj( victim );   /* used to save ch, fixed by Thoric 09/17/96 */
   return;
}

void do_unhell( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;

   set_char_color( AT_IMMORT, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Unhell whom..?\n\r", ch );
      return;
   }
   location = ch->in_room;
/*ch->in_room = get_room_index(8);*/
   victim = get_char_world( ch, arg );
/*ch->in_room = location;          The case of unhell self, etc.*/
   if( !victim || IS_NPC( victim ) )
   {
      send_to_char( "No such player character present.\n\r", ch );
      return;
   }
   if( victim->in_room->vnum != ROOM_VNUM_HELL )
   {
      send_to_char( "No one like that is in hell.\n\r", ch );
      return;
   }

   location = get_room_index( ROOM_VNUM_NEXUS );
   if( !location )
      location = ch->in_room;
   MOBtrigger = FALSE;
   act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   char_from_room( victim );
   char_to_room( victim, location );
   send_to_char( "The gods have smiled on you and released you from hell early!\n\r", victim );
   do_look( victim, "auto" );
   if( victim != ch )
      send_to_char( "They have been released.\n\r", ch );
   if( victim->pcdata->helled_by )
   {
      if( str_cmp( ch->name, victim->pcdata->helled_by ) )
         ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\n\r",
                    victim->pcdata->helled_by );
      STRFREE( victim->pcdata->helled_by );
      victim->pcdata->helled_by = NULL;
   }

   MOBtrigger = FALSE;
   act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   victim->pcdata->release_date = 0;
   save_char_obj( victim );
   return;
}

/* Vnum search command by Swordbearer */
void do_vsearch( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   bool found = FALSE;
   OBJ_DATA *obj;
   OBJ_DATA *in_obj;
   int obj_counter = 1;
   int argi;

   set_pager_color( AT_PLAIN, ch );

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax:  vsearch <vnum>.\n\r", ch );
      return;
   }

   argi = atoi( arg );
   if( argi < 0 && argi > 20000 )
   {
      send_to_char( "Vnum out of range.\n\r", ch );
      return;
   }
   for( obj = first_object; obj != NULL; obj = obj->next )
   {
      if( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ) )
         continue;

      found = TRUE;
      for( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

      if( in_obj->carried_by != NULL )
         pager_printf( ch, "[%2d] %s carried by %s.\n\r", obj_counter, obj_short( obj ), PERS( in_obj->carried_by, ch ) );
      else
         pager_printf( ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
                       ( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
                       obj_short( obj ), ( in_obj->in_room == NULL ) ? "somewhere" : in_obj->in_room->name );

      obj_counter++;
   }

   if( !found )
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
   return;
}

/*
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96
 */
void do_sober( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on mobs.\n\r", ch );
      return;
   }

   if( victim->pcdata )
      victim->pcdata->condition[COND_DRUNK] = 0;
   send_to_char( "Ok.\n\r", ch );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You feel sober again.\n\r", victim );
   return;
}

/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE * social )
{
   if( social->name )
      DISPOSE( social->name );
   if( social->char_no_arg )
      DISPOSE( social->char_no_arg );
   if( social->others_no_arg )
      DISPOSE( social->others_no_arg );
   if( social->char_found )
      DISPOSE( social->char_found );
   if( social->others_found )
      DISPOSE( social->others_found );
   if( social->vict_found )
      DISPOSE( social->vict_found );
   if( social->char_auto )
      DISPOSE( social->char_auto );
   if( social->others_auto )
      DISPOSE( social->others_auto );
   if( social->adj )
      DISPOSE( social->adj );
   DISPOSE( social );
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social( SOCIALTYPE * social )
{
   SOCIALTYPE *tmp, *tmp_next;
   int hash;

   if( !social )
   {
      bug( "Unlink_social: NULL social", 0 );
      return;
   }

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( social == ( tmp = social_index[hash] ) )
   {
      social_index[hash] = tmp->next;
      return;
   }
   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;
      if( social == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE * social )
{
   int hash, x;
   SOCIALTYPE *tmp, *prev;

   if( !social )
   {
      bug( "Add_social: NULL social", 0 );
      return;
   }

   if( !social->name )
   {
      bug( "Add_social: NULL social->name", 0 );
      return;
   }

   if( !social->char_no_arg )
   {
      bug( "Add_social: NULL social->char_no_arg", 0 );
      return;
   }

   /*
    * make sure the name is all lowercase 
    */
   for( x = 0; social->name[x] != '\0'; x++ )
      social->name[x] = LOWER( social->name[x] );

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( ( prev = tmp = social_index[hash] ) == NULL )
   {
      social->next = social_index[hash];
      social_index[hash] = social;
      return;
   }

   for( ; tmp; tmp = tmp->next )
   {
      if( ( x = strcmp( social->name, tmp->name ) ) == 0 )
      {
         bug( "Add_social: trying to add duplicate name to bucket %d", hash );
         free_social( social );
         return;
      }
      else if( x < 0 )
      {
         if( tmp == social_index[hash] )
         {
            social->next = social_index[hash];
            social_index[hash] = social;
            return;
         }
         prev->next = social;
         social->next = tmp;
         return;
      }
      prev = tmp;
   }

   /*
    * add to end 
    */
   prev->next = social;
   social->next = NULL;
   return;
}

char *const soc_flags[] = {
   "sleep", "polite", "rude", "sexual", "visual", "auditory", "touch", "annoying",
   "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16",
   "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
   "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

int get_soc_flag( char *flag )
{
   int x;

   for( x = 0; x < ( sizeof( soc_flags ) / sizeof( soc_flags[0] ) ); x++ )
      if( !str_cmp( flag, soc_flags[x] ) )
         return x;
   return -1;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit( CHAR_DATA * ch, char *argument )
{
   SOCIALTYPE *social;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   set_char_color( AT_SOCIAL, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: sedit <social> [field]\n\r", ch );
      send_to_char( "Syntax: sedit <social> create\n\r", ch );
      send_to_char( "Syntax: sedit <social> delete\n\r", ch );
      send_to_char( "Syntax: sedit <social> flags [flags]\n\r", ch );
      send_to_char( "Syntax: sedit <save>\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto adj\n\r", ch );
      return;
   }

   if( !str_cmp( arg1, "save" ) )
   {
      save_socials(  );
      send_to_char( "Saved.\n\r", ch );
      return;
   }

   social = find_social( arg1 );
   if( !str_cmp( arg2, "create" ) )
   {
      if( social )
      {
         send_to_char( "That social already exists!\n\r", ch );
         return;
      }
      CREATE( social, SOCIALTYPE, 1 );
      social->name = str_dup( arg1 );
      sprintf( arg2, "You %s.", arg1 );
      social->char_no_arg = str_dup( arg2 );
      add_social( social );
      send_to_char( "Social added.\n\r", ch );
      return;
   }

   if( !social )
   {
      send_to_char( "Social not found.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Social: %s\n\r\n\rFlags: %s\n\r", social->name, flag_string( social->flags, soc_flags ) );
      ch_printf( ch, "CNoArg: %s\n\r", social->char_no_arg, ch );
      ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
                 social->others_no_arg ? social->others_no_arg : "(not set)",
                 social->char_found ? social->char_found : "(not set)",
                 social->others_found ? social->others_found : "(not set)" );
      ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
                 social->vict_found ? social->vict_found : "(not set)",
                 social->char_auto ? social->char_auto : "(not set)",
                 social->others_auto ? social->others_auto : "(not set)" );
      ch_printf( ch, "Default Adj: %s\n\r", social->adj ? social->adj : "(not set)" );
      return;
   }
   if( !str_cmp( arg2, "delete" ) )
   {
      unlink_social( social );
      free_social( social );
      send_to_char( "Deleted.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "cnoarg" ) )
   {
      if( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
      {
         send_to_char( "You cannot clear this field.  It must have a message.\n\r", ch );
         return;
      }
      if( social->char_no_arg )
         DISPOSE( social->char_no_arg );
      social->char_no_arg = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "onoarg" ) )
   {
      if( social->others_no_arg )
         DISPOSE( social->others_no_arg );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_no_arg = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "cfound" ) )
   {
      if( social->char_found )
         DISPOSE( social->char_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_found = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "ofound" ) )
   {
      if( social->others_found )
         DISPOSE( social->others_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_found = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "vfound" ) )
   {
      if( social->vict_found )
         DISPOSE( social->vict_found );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->vict_found = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "cauto" ) )
   {
      if( social->char_auto )
         DISPOSE( social->char_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->char_auto = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "oauto" ) )
   {
      if( social->others_auto )
         DISPOSE( social->others_auto );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->others_auto = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "adj" ) )
   {
      if( social->adj )
         DISPOSE( social->adj );
      if( argument[0] != '\0' && str_cmp( argument, "clear" ) )
         social->adj = str_dup( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "flags" ) )
   {
      int value;

      if( !argument || argument[0] == '\0' )
      {
         send_to_char( "Usage: sedit <social> flags <flag> [flag]...\n\r", ch );
         return;
      }
      while( argument[0] != '\0' )
      {
         argument = one_argument( argument, arg1 );
         value = get_soc_flag( arg1 );
         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg1 );
         else
            TOGGLE_BIT( social->flags, 1 << value );
      }
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "name" ) )
   {
      bool relocate;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\n\r", ch );
         return;
      }
      if( arg1[0] != social->name[0] )
      {
         unlink_social( social );
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if( social->name )
         DISPOSE( social->name );
      social->name = str_dup( arg1 );
      if( relocate )
         add_social( social );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   /*
    * display usage message 
    */
   do_sedit( ch, "" );
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE * command )
{
   if( command->name )
      DISPOSE( command->name );
   DISPOSE( command );
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE * command )
{
   CMDTYPE *tmp, *tmp_next;
   int hash;

   if( !command )
   {
      bug( "Unlink_command NULL command", 0 );
      return;
   }

   hash = command->name[0] % 126;

   if( command == ( tmp = command_hash[hash] ) )
   {
      command_hash[hash] = tmp->next;
      return;
   }
   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;
      if( command == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE * command )
{
   int hash, x;
   CMDTYPE *tmp, *prev;

   if( !command )
   {
      bug( "Add_command: NULL command", 0 );
      return;
   }

   if( !command->name )
   {
      bug( "Add_command: NULL command->name", 0 );
      return;
   }

   if( !command->do_fun )
   {
      bug( "Add_command: NULL command->do_fun", 0 );
      return;
   }

   /*
    * make sure the name is all lowercase 
    */
   for( x = 0; command->name[x] != '\0'; x++ )
      command->name[x] = LOWER( command->name[x] );

   hash = command->name[0] % 126;

   if( ( prev = tmp = command_hash[hash] ) == NULL )
   {
      command->next = command_hash[hash];
      command_hash[hash] = command;
      return;
   }

   /*
    * add to the END of the list 
    */
   for( ; tmp; tmp = tmp->next )
      if( !tmp->next )
      {
         tmp->next = command;
         command->next = NULL;
      }
   return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 * Added support for interpret flags                            -Shaddai
 */
void do_cedit( CHAR_DATA * ch, char *argument )
{
   CMDTYPE *command;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   set_char_color( AT_IMMORT, ch );

   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Syntax: cedit save cmdtable\n\r", ch );
      send_to_char( "Syntax: cedit <command> create [code]\n\r", ch );
      send_to_char( "Syntax: cedit <command> delete\n\r", ch );
      send_to_char( "Syntax: cedit <command> show\n\r", ch );
      send_to_char( "Syntax: cedit <command> raise\n\r", ch );
      send_to_char( "Syntax: cedit <command> lower\n\r", ch );
      send_to_char( "Syntax: cedit <command> list\n\r", ch );
      send_to_char( "Syntax: cedit <command> [field]\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      send_to_char( "  position log code flags permit\n\r", ch );
      send_to_char( " Permit: admin, help, build, hibuild, security,\n\r"
	      "             talent, item, misc, channel, mob, (all or none = same) ", ch);
      return;
   }

   if( !str_cmp( arg1, "save" ) && !str_cmp( arg2, "cmdtable" ) )
   {
      save_commands(  );
      send_to_char( "Saved.\n\r", ch );
      return;
   }

   command = find_command( arg1 );
   if( !str_cmp( arg2, "create" ) )
   {
      if( command )
      {
         send_to_char( "That command already exists!\n\r", ch );
         return;
      }
      CREATE( command, CMDTYPE, 1 );
      command->lag_count = 0; /* FB */
      command->name = str_dup( arg1 );
      command->permit = PERMIT_ADMIN;
      if( *argument )
         one_argument( argument, arg2 );
      else
         sprintf( arg2, "do_%s", arg1 );
      command->do_fun = skill_function( arg2 );
      add_command( command );
      send_to_char( "Command added.\n\r", ch );
      if( command->do_fun == skill_notfound )
         ch_printf( ch, "Code %s not found.  Set to no code.\n\r", arg2 );
      return;
   }

   if( !command )
   {
      send_to_char( "Command not found.\n\r", ch );
      return;
   }

   if( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Command:  %s\n\rPosition: %s\n\rLog:    %d\n\rCode: %s\n\rFlags:  %s\n\rPermit:    %d\n\r",
                 command->name, pos_names[command->position], command->log,
                 skill_name( command->do_fun ), flag_string( command->flags, cmd_flags ), command->permit );
      if( command->userec.num_uses )
         send_timer( &command->userec, ch );
      return;
   }

   if( !str_cmp( arg2, "raise" ) )
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if( ( tmp = command_hash[hash] ) == command )
      {
         send_to_char( "That command is already at the top.\n\r", ch );
         return;
      }
      if( tmp->next == command )
      {
         command_hash[hash] = command;
         tmp_next = tmp->next;
         tmp->next = command->next;
         command->next = tmp;
         ch_printf( ch, "Moved %s above %s.\n\r", command->name, command->next->name );
         return;
      }
      for( ; tmp; tmp = tmp->next )
      {
         tmp_next = tmp->next;
         if( tmp_next->next == command )
         {
            tmp->next = command;
            tmp_next->next = command->next;
            command->next = tmp_next;
            ch_printf( ch, "Moved %s above %s.\n\r", command->name, command->next->name );
            return;
         }
      }
      send_to_char( "ERROR -- Not Found!\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "lower" ) )
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if( command->next == NULL )
      {
         send_to_char( "That command is already at the bottom.\n\r", ch );
         return;
      }
      tmp = command_hash[hash];
      if( tmp == command )
      {
         tmp_next = tmp->next;
         command_hash[hash] = command->next;
         command->next = tmp_next->next;
         tmp_next->next = command;

         ch_printf( ch, "Moved %s below %s.\n\r", command->name, tmp_next->name );
         return;
      }
      for( ; tmp; tmp = tmp->next )
      {
         if( tmp->next == command )
         {
            tmp_next = command->next;
            tmp->next = tmp_next;
            command->next = tmp_next->next;
            tmp_next->next = command;

            ch_printf( ch, "Moved %s below %s.\n\r", command->name, tmp_next->name );
            return;
         }
      }
      send_to_char( "ERROR -- Not Found!\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "list" ) )
   {
      CMDTYPE *tmp;
      int hash = command->name[0] % 126;

      pager_printf( ch, "Priority placement for [%s]:\n\r", command->name );
      for( tmp = command_hash[hash]; tmp; tmp = tmp->next )
      {
         if( tmp == command )
            set_pager_color( AT_GREEN, ch );
         else
            set_pager_color( AT_PLAIN, ch );
         pager_printf( ch, "  %s\n\r", tmp->name );
      }
      return;
   }
   if( !str_cmp( arg2, "delete" ) )
   {
      unlink_command( command );
      free_command( command );
      send_to_char( "Deleted.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "code" ) )
   {
      DO_FUN *fun = skill_function( argument );

      if( fun == skill_notfound )
      {
         send_to_char( "Code not found.\n\r", ch );
         return;
      }
      command->do_fun = fun;
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "log" ) )
   {
      int log = atoi( argument );

      if( log < 0 || log > LOG_COMM )
      {
         send_to_char( "Log out of range.\n\r", ch );
         return;
      }
      command->log = log;
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "position" ) )
   {
      int position = atoi( argument );

      if( position < 0 || position > POS_DRAG )
      {
         send_to_char( "Position out of range.\n\r", ch );
         return;
      }
      command->position = position;
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "flags" ) )
   {
      int flag;
      if( is_number( argument ) )
         flag = atoi( argument );
      else
         flag = get_cmdflag( argument );
      if( flag < 0 || flag >= 32 )
      {
         if( is_number( argument ) )
            ch_printf( ch, "Invalid flag: range is from 0 to 31.\n" );
         else
            ch_printf( ch, "Unknown flag %s.\n", argument );
         return;
      }

      TOGGLE_BIT( command->flags, 1 << flag );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "permit" ) )
   {
      int flag;
      if( !str_cmp( argument, "none" ) || !str_cmp( argument, "all") )
      {
         command->permit = 0;
      }
      else
      {
         flag = get_permit_flag( argument );
         if( flag < 0 || flag > MAX_BITS )
         {
            send_to_char( "Unknown flag.\r\n", ch );
            return;
         }
         command->permit = 1 << flag;
      }
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "name" ) )
   {
      bool relocate;

      one_argument( argument, arg1 );
      if( arg1[0] == '\0' )
      {
         send_to_char( "Cannot clear name field!\n\r", ch );
         return;
      }
      if( arg1[0] != command->name[0] )
      {
         unlink_command( command );
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if( command->name )
         DISPOSE( command->name );
      command->name = str_dup( arg1 );
      if( relocate )
         add_command( command );
      send_to_char( "Done.\n\r", ch );
      return;
   }

   /*
    * display usage message 
    */
   do_cedit( ch, "" );
}


RESERVE_DATA *first_reserved;
RESERVE_DATA *last_reserved;
void save_reserved( void )
{
   RESERVE_DATA *res;
   FILE *fp;

   fclose( fpReserve );
   if( !( fp = fopen( SYSTEM_DIR RESERVED_LIST, "w" ) ) )
   {
      bug( "Save_reserved: cannot open " RESERVED_LIST, 0 );
      perror( RESERVED_LIST );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }
   for( res = first_reserved; res; res = res->next )
      fprintf( fp, "%s~\n", res->name );
   fprintf( fp, "$~\n" );
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}

void do_reserve( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   RESERVE_DATA *res;

   set_char_color( AT_PLAIN, ch );

   argument = one_argument( argument, arg );
   if( !*arg )
   {
      int wid = 0;

      send_to_char( "-- Reserved Names --\n\r", ch );
      for( res = first_reserved; res; res = res->next )
      {
         ch_printf( ch, "%c%-17s ", ( *res->name == '*' ? '*' : ' ' ), ( *res->name == '*' ? res->name + 1 : res->name ) );
         if( ++wid % 4 == 0 )
            send_to_char( "\n\r", ch );
      }
      if( wid % 4 != 0 )
         send_to_char( "\n\r", ch );
      return;
   }
   for( res = first_reserved; res; res = res->next )
      if( !str_cmp( arg, res->name ) )
      {
         UNLINK( res, first_reserved, last_reserved, next, prev );
         DISPOSE( res->name );
         DISPOSE( res );
         save_reserved(  );
         send_to_char( "Name no longer reserved.\n\r", ch );
         return;
      }
   CREATE( res, RESERVE_DATA, 1 );
   res->name = str_dup( arg );
   sort_reserved( res );
   save_reserved(  );
   send_to_char( "Name reserved.\n\r", ch );
   return;
}

/*
 * Command to display the weather status of all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void do_showweather( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *pArea;
   char arg[MAX_INPUT_LENGTH];

   if( !ch )
   {
      bug( "do_showweather: NULL char data" );
      return;
   }

   argument = one_argument( argument, arg );

   set_char_color( AT_BLUE, ch );
   ch_printf( ch, "%-40s%-8s %-8s %-8s %-8s\n\r", "Area Name:", "Temp:", "Precip:", "Wind:", "Mana:" );

   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      if( arg[0] == '\0' || nifty_is_name_prefix( arg, pArea->name ) )
      {
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "%-40s", pArea->name );
         set_char_color( AT_WHITE, ch );
         ch_printf( ch, "%3d", pArea->weather->temp );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "(" );
         set_char_color( AT_LBLUE, ch );
         ch_printf( ch, "%3d", pArea->weather->temp_vector );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, ") " );
         set_char_color( AT_WHITE, ch );
         ch_printf( ch, "%3d", pArea->weather->precip );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "(" );
         set_char_color( AT_LBLUE, ch );
         ch_printf( ch, "%3d", pArea->weather->precip_vector );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, ") " );
         set_char_color( AT_WHITE, ch );
         ch_printf( ch, "%3d", pArea->weather->wind );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "(" );
         set_char_color( AT_LBLUE, ch );
         ch_printf( ch, "%3d", pArea->weather->wind_vector );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, ")" );
         set_char_color( AT_WHITE, ch );
         ch_printf( ch, "%3d", pArea->weather->mana );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "(" );
         set_char_color( AT_LBLUE, ch );
         ch_printf( ch, "%3d", pArea->weather->mana_vector );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, ")\n\r" );
      }
   }

   return;
}

/*
 * Command to control global weather variables and to reset weather
 * Last Modified: July 23, 1997
 * Fireblade
 */
void do_setweather( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_BLUE, ch );

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
   {
      ch_printf( ch, "%-15s%-6s\n\r", "Parameters:", "Value:" );
      ch_printf( ch, "%-15s%-6d\n\r", "random", rand_factor );
      ch_printf( ch, "%-15s%-6d\n\r", "climate", climate_factor );
      ch_printf( ch, "%-15s%-6d\n\r", "neighbor", neigh_factor );
      ch_printf( ch, "%-15s%-6d\n\r", "unit", weath_unit );
      ch_printf( ch, "%-15s%-6d\n\r", "maxvector", max_vector );

      ch_printf( ch, "\n\rResulting values:\n\r" );
      ch_printf( ch, "Weather variables range from " "%d to %d.\n\r", -3 * weath_unit, 3 * weath_unit );
      ch_printf( ch, "Weather vectors range from " "%d to %d.\n\r", -1 * max_vector, max_vector );
      ch_printf( ch, "The maximum a vector can "
                 "change in one update is %d.\n\r", rand_factor + 2 * climate_factor + ( 6 * weath_unit / neigh_factor ) );
   }
   else if( !str_cmp( arg, "random" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set maximum random " "change in vectors to what?\n\r" );
      }
      else
      {
         rand_factor = atoi( argument );
         ch_printf( ch, "Maximum random " "change in vectors now " "equals %d.\n\r", rand_factor );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "climate" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set climate effect " "coefficient to what?\n\r" );
      }
      else
      {
         climate_factor = atoi( argument );
         ch_printf( ch, "Climate effect " "coefficient now equals " "%d.\n\r", climate_factor );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "neighbor" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set neighbor effect " "divisor to what?\n\r" );
      }
      else
      {
         neigh_factor = atoi( argument );

         if( neigh_factor <= 0 )
            neigh_factor = 1;

         ch_printf( ch, "Neighbor effect " "coefficient now equals " "1/%d.\n\r", neigh_factor );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "unit" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set weather unit " "size to what?\n\r" );
      }
      else
      {
         weath_unit = atoi( argument );
         ch_printf( ch, "Weather unit size " "now equals %d.\n\r", weath_unit );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "maxvector" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set maximum vector " "size to what?\n\r" );
      }
      else
      {
         max_vector = atoi( argument );
         ch_printf( ch, "Maximum vector size " "now equals %d.\n\r", max_vector );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "reset" ) )
   {
      init_area_weather(  );
      ch_printf( ch, "Weather system reinitialized.\n\r" );
   }
   else if( !str_cmp( arg, "update" ) )
   {
      int i, number;

      number = atoi( argument );

      if( number < 1 )
         number = 1;

      for( i = 0; i < number; i++ )
         weather_update(  );

      ch_printf( ch, "Weather system updated.\n\r" );
   }
   else
   {
      ch_printf( ch, "You may only use one of the " "following fields:\n\r" );
      ch_printf( ch, "\trandom\n\r\tclimate\n\r" "\tneighbor\n\r\tunit\n\r\tmaxvector\n\r" );
      ch_printf( ch, "You may also reset or update "
                 "the system using the fields 'reset' " "and 'update' respectively.\n\r" );
   }

   return;
}


void do_pcrename( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char newname[MAX_STRING_LENGTH];
   char oldname[MAX_STRING_LENGTH];
   char backname[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg1 );
   one_argument( argument, arg2 );
   smash_tilde( arg2 );


   if( IS_NPC( ch ) )
      return;

   if( arg1[0] == '\0' || arg2[0] == '\0' )
   {
      send_to_char( "Syntax: rename <victim> <new name>\n\r", ch );
      return;
   }

   if( !check_parse_name( arg2, 1 ) )
   {
      send_to_char( "Illegal name.\n\r", ch );
      return;
   }
   /*
    * Just a security precaution so you don't rename someone you don't mean
    * * too --Shaddai
    */
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That person is not in the room.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't rename NPC's.\n\r", ch );
      return;
   }

   sprintf( newname, "%s%c/%s", PLAYER_DIR, tolower( (int)arg2[0] ), capitalize( arg2 ) );
   sprintf( oldname, "%s%c/%s", PLAYER_DIR, tolower( (int)victim->pcdata->filename[0] ), capitalize( victim->pcdata->filename ) );
   sprintf( backname, "%s%c/%s", BACKUP_DIR, tolower( (int)victim->pcdata->filename[0] ),
            capitalize( victim->pcdata->filename ) );
   if( access( newname, F_OK ) == 0 )
   {
      send_to_char( "That name already exists.\n\r", ch );
      return;
   }

   /*
    * Remember to change the names of the areas 
    */
   if( ch->pcdata->area )
   {
      char filename[MAX_STRING_LENGTH];
      char newfilename[MAX_STRING_LENGTH];

      sprintf( filename, "%s%s.are", BUILD_DIR, victim->name );
      sprintf( newfilename, "%s%s.are", BUILD_DIR, capitalize( arg2 ) );
      rename( filename, newfilename );
      sprintf( filename, "%s%s.are.bak", BUILD_DIR, victim->name );
      sprintf( newfilename, "%s%s.are.bak", BUILD_DIR, capitalize( arg2 ) );
      rename( filename, newfilename );
   }

   STRFREE( victim->name );
   victim->name = STRALLOC( capitalize( arg2 ) );
   STRFREE( victim->pcdata->filename );
   victim->pcdata->filename = STRALLOC( capitalize( arg2 ) );
   remove( backname );
   if( remove( oldname ) )
   {
      sprintf( buf, "Error: Couldn't delete file %s in do_rename.", oldname );
      send_to_char( "Couldn't delete the old file!\n\r", ch );
      log_string( oldname );
   }
   /*
    * Time to save to force the affects to take place 
    */
   save_char_obj( victim );

   set_title( victim, "" );
   send_to_char( "Character was renamed.\n\r", ch );
   return;
}

void do_info( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;

   for( d = first_descriptor; d; d = d->next )
      if( d->connected == CON_PLAYING )
         ch_printf_color( d->character, "&R[&WINFO&R] &c%s\n\r", argument );
   return;
}

void do_slay( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   if( arg[0] == '\0' )
   {
      send_to_char( "Slay whom?\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( !str_cmp( arg2, "immolate" ) )
   {
      act( AT_FIRE, "Your fireball turns $N into a blazing inferno.", ch, NULL, victim, TO_CHAR );
      act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT );
      act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.", ch, NULL, victim, TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "shatter" ) )
   {
      act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.", ch, NULL, victim,
           TO_CHAR );
      act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim,
           TO_VICT );
      act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.", ch, NULL, victim,
           TO_NOTVICT );
   }


// Added by Trisk. This one I like.
   else if( !str_cmp( arg2, "kiss" ) )
   {
      act( AT_CYAN, "You kiss $N gently, taking $S last breath.", ch, NULL, victim, TO_CHAR );
      act( AT_CYAN, "$n kisses you gently, taking your last breath.", ch, NULL, victim, TO_VICT );
      act( AT_CYAN, "$n kisses $N gently.", ch, NULL, victim, TO_NOTVICT );
   }

// Also added by Triskal. This one's amusing.
   else if( !str_cmp( arg2, "heart" ) )
   {
      char buf[MAX_STRING_LENGTH];
      OBJ_DATA *obj;
      char *name;

      act( AT_BLOOD, "You reach into $N's chest and tear out $S beating heart.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n stabs $s hand inside you, tearing out your heart!", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$N screams as $n brutally tears out $S heart!", ch, NULL, victim, TO_NOTVICT );
      name = IS_NPC( victim ) ? victim->short_descr : victim->name;
      obj = create_object( get_obj_index( 13 ), 0 );

      sprintf( buf, obj->short_descr, name );
      STRFREE( obj->short_descr );
      obj->short_descr = STRALLOC( buf );

      sprintf( buf, obj->description, name );
      STRFREE( obj->description );
      obj->description = STRALLOC( buf );

      obj = obj_to_char( obj, ch );
   }

// Another by Trisk. I was bored, so I added some slays. So sue me.
   else if( !str_cmp( arg2, "rip" ) )
   {
      act( AT_DGREY, "You rip $N's soul from $M and cast it into the Void.", ch, NULL, victim, TO_CHAR );
      act( AT_DGREY, "$n rips your soul from you, casting it into the endless Void.", ch, NULL, victim, TO_VICT );
      act( AT_DGREY, "$n rips $N's soul from $M, casting it into the Void.", ch, NULL, victim, TO_NOTVICT );
   }
// This one.. is just funny =p   -Triskal
   else if( !str_cmp( arg2, "many" ) )
   {
      act( AT_WHITE, "You warp time and space, killing $N one hundred times all at once.", ch, NULL, victim, TO_CHAR );
      act( AT_WHITE, "$n warps time and space, killing you a hundred times all at once!", ch, NULL, victim, TO_VICT );
      act( AT_WHITE, "$n warps time and space, killing $N one hundred times all at once!", ch, NULL, victim, TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "demon" ) )
   {
      act( AT_BLOOD, "You gesture, and a slavering demon appears.  With a horrible grin, the", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,
           TO_CHAR );
      act( AT_BLOOD, "$n gestures, and a slavering demon appears.  The foul creature turns on", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "you with a horrible grin.   You scream in panic before being eaten alive.", ch, NULL, victim,
           TO_VICT );
      act( AT_BLOOD, "$n gestures, and a slavering demon appears.  With a horrible grin, the", ch, NULL, victim,
           TO_NOTVICT );
      act( AT_BLOOD, "foul creature turns on $N, who screams in panic before being eaten alive.", ch, NULL, victim,
           TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "pounce" ) )
   {
      act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...", ch,
           NULL, victim, TO_CHAR );
      act( AT_BLOOD,
           "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...",
           ch, NULL, victim, TO_VICT );
      act( AT_BLOOD,
           "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",
           ch, NULL, victim, TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "slit" ) )
   {
      act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
   }

   else if( !str_cmp( arg2, "dog" ) )
   {
      act( AT_BLOOD, "You order your dogs to rip $N to shreds.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n orders $s dogs to rip you apart.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n orders $s dogs to rip $N to shreds.", ch, NULL, victim, TO_NOTVICT );
   }

   else
   {
      act( AT_IMMORT, "You slay $N in cold blood!", ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT );
   }

   set_cur_char( victim );
   victim->last_hit = ch;
   ch->last_hit = victim;
   die( victim );
   return;
}
