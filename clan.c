/* clan.c -- Dynamic Clan System for the Rogue Winds by Scion
   Copyright 2000 by Peter Keeler, All Rights Reserved. The content
   of this file may NOT be used by anyone other than the author for
   any purpose without explicit written permission on a non-electronic
   medium. -- Scion
*/  
   
#include <stdio.h>
#include <string.h>
#include "mud.h"
extern char *munch_colors( char *word );
CLAN_DATA * first_clan;
CLAN_DATA * last_clan;
char *const c_flags[] =
   { "leader", "induct", "outcast", "bank", "finance", "bestow", "secret", "knowledge", "allow", "r10", "r11", "r12",
   "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
   "r30", "r31", "r32" 
};
int get_clan_flag( char *type ) 
{
   int x;
   for( x = 0; x < ( sizeof( c_flags ) / sizeof( c_flags[0] ) ); x++ )
      if( !str_cmp( type, c_flags[x] ) )
         return x;
   return -1;
}

CLAN_MEMBER * get_member( char *name, CLAN_DATA * clan )
{
   CLAN_MEMBER * member;
   for( member = clan->first_member; member; member = member->next )
      if( !str_cmp( member->name, capitalize( name ) ) )
         return member;
   return NULL;
}

CLAN_DATA * get_clan( CHAR_DATA * ch, char *name )
{
   CLAN_DATA * clan;
   for( clan = first_clan; clan; clan = clan->next )
      if( !str_cmp( clan->name, name ) )
         return clan;
   return NULL;
}


/* get a clan by its name as above, but pay no attention to what char is asking */ 
   CLAN_DATA * clan_name( char *name )
{
   CLAN_DATA * clan;
   for( clan = first_clan; clan; clan = clan->next )
   {
      if( !strcmp( strlower( clan->name ), name ) )
         return clan;
   }
   return NULL;
}
void clan_add( CHAR_DATA * ch, CLAN_DATA * clan )
{
   char buf[MAX_STRING_LENGTH];
   CLAN_MEMBER * member;
   CREATE( member, CLAN_MEMBER, 1 );
   strcpy( buf, ch->name );
   member->name = STRALLOC( buf );
   member->title = STRALLOC( "Inductee" );
   LINK( member, clan->first_member, clan->last_member, next, prev );
   save_clans(  );
} bool clan_remove( char *name, CLAN_DATA * clan )
{
   CLAN_MEMBER * member;
   if( ( member = get_member( name, clan ) ) == NULL )
      return FALSE;
   UNLINK( member, clan->first_member, clan->last_member, next, prev );
   STRFREE( member->name );
   STRFREE( member->title );
   DISPOSE( member );
   save_clans(  );
   return TRUE;
}
void do_clan( CHAR_DATA * ch, char *argument )
{
   CLAN_DATA * clan;
   CLAN_DATA * cl;
   CLAN_MEMBER * member;
   CLAN_MEMBER * chm;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int i = 0;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1 == NULL || arg1[0] == '\0' )
   {
      do_new_help( ch, "clan_syntax" );
      return;
   }
   if( ( clan = get_clan( ch, arg2 ) ) == NULL )
   {
      if( strcmp( arg1, "create" )  &&strcmp( arg1, "list" ) )
      {
         send_to_char( "No clan goes by that abbreviation.\n\r", ch );
         return;
      }
   }
   else
   {
      if( !strcmp( arg1, "create" ) )
      {
         send_to_char( "There is already a clan by that abbreviation.\n\r", ch );
         return;
      }
      chm = get_member( ch->name, clan );
   }
   if( !strcmp( arg1, "list" ) )
   {
      send_to_char( "&cClans that you know about:&w\r\n", ch );
      for( clan = first_clan; clan; clan = clan->next )
      {
         i++;
         if( get_member( ch->name, clan ) != NULL )
            ch_printf( ch, "&Y%d&O: &w%s&w &G(&g%s&G)\r\n", i, clan->title, clan->name );
         
         else if( clan->ext_sec < 6  ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
            ch_printf( ch, "&Y%d&O: &w%s&w &Y(&O%s&Y)\r\n", i, clan->title, clan->name );
      }
      return;
   }
   else if( !strcmp( arg1, "create" ) )
   {
      if( arg2 == NULL || arg2[0] == '\0' )
      {
         send_to_char( "What name do you wish to give your new clan?\r\n", ch );
         return;
      }
      CREATE( clan, CLAN_DATA, 1 );
      LINK( clan, first_clan, last_clan, next, prev );
      if( clan->name )
         STRFREE( clan->name );
      clan->name = STRALLOC( munch_colors( arg2 ) );
      if( clan->title )
         STRFREE( clan->title );
      clan->title = STRALLOC( arg2 );
      clan->int_sec = 5;
      clan->ext_sec = 5;
      clan_add( ch, clan );
      chm = clan->first_member;
      STRFREE( chm->title );
      chm->title = STRALLOC( "Leader" );
      xSET_BIT( chm->flags, CL_LEADER );
      xSET_BIT( chm->flags, CL_ALLOW );
      xSET_BIT( chm->flags, CL_INDUCT );
      xSET_BIT( chm->flags, CL_OUTCAST );
      xSET_BIT( chm->flags, CL_BESTOW );
      xSET_BIT( chm->flags, CL_BANK );
      xSET_BIT( chm->flags, CL_KNOWLEDGE );
      xSET_BIT( chm->flags, CL_FINANCE );
      save_clans(  );
      ch_printf( ch, "Clan '%s' created.\r\n", clan->name );
      return;
   }
   else if( !strcmp( arg1, "info" ) )
   {
      if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&( !chm || !xIS_SET( chm->flags, CL_LEADER ) ) )
      {
         send_to_char( "You are not the leader of that clan.\r\n", ch );
         return;
      }
      STRFREE( clan->info );
      clan->info = STRALLOC( argument );
      save_clans(  );
      send_to_char( "You have changed the clan's info.\n\r", ch );
      return;
   }
   else if( !strcmp( arg1, "name" ) )
   {
      if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&( !chm || !xIS_SET( chm->flags, CL_LEADER ) ) )
      {
         send_to_char( "You are not the leader of that clan.\r\n", ch );
         return;
      }
      STRFREE( clan->title );
      clan->title = STRALLOC( argument );
      save_clans(  );
      send_to_char( "You have changed the clan's name.\n\r", ch );
      return;
   }
   else if( !strcmp( arg1, "abbr" ) )
   {
      argument = munch_colors( argument );
      for( cl = first_clan; cl; cl = cl->next )
      {
         if( !str_cmp( cl->name, argument ) )
         {
            send_to_char( "There is already a clan with that abbreviation. Please choose another.\n\r", ch );
            return;
         }
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&( !chm || !xIS_SET( chm->flags, CL_LEADER ) ) )
      {
         send_to_char( "You are not the leader of that clan.\r\n", ch );
         return;
      }
      STRFREE( clan->name );
      clan->name = STRALLOC( argument );
      save_clans(  );
      send_to_char( "You have changed the clan's abbreviation.\n\r", ch );
      return;
   }
   else if( !strcmp( arg1, "disband" ) )
   {
      if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&( !ch || !xIS_SET( chm->flags, CL_LEADER ) ) )
      {
         send_to_char( "You are not the leader of that clan.\r\n", ch );
         return;
      }
      send_to_char( "You disband the clan.\r\n", ch );
      UNLINK( clan, first_clan, last_clan, next, prev );
      STRFREE( clan->name );
      for( member = clan->first_member; member; member = member->next )
      {
         STRFREE( member->name );
         STRFREE( member->title );
         if( member->prev )
            DISPOSE( member->prev );
      }
      DISPOSE( clan->last_member );
      DISPOSE( clan );
      save_clans(  );
      return;
   }
   else if( !strcmp( arg1, "intsec" ) )
   {
      i = atoi( argument );
      if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&( !ch || !xIS_SET( chm->flags, CL_LEADER ) ) )
      {
         send_to_char( "You are not the leader of that clan.\r\n", ch );
         return;
      }
      clan->int_sec = URANGE( 1, i, 6 );
      save_clans(  );
      ch_printf( ch, "You change the internal secrecy level of %s&w to %d.\n\r", clan->title, clan->int_sec );
      return;
   }
   else if( !strcmp( arg1, "extsec" ) )
   {
      i = atoi( argument );
      if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&( !ch || !xIS_SET( chm->flags, CL_LEADER ) ) )
      {
         send_to_char( "You are not the leader of that clan.\r\n", ch );
         return;
      }
      clan->ext_sec = URANGE( 1, i, 6 );
      save_clans(  );
      ch_printf( ch, "You change the external secrecy level of %s&w to %d.\n\r", clan->title, clan->ext_sec );
      return;
   }
   else if( !strcmp( arg1, "deposit" ) )
   {
      OBJ_DATA * atm;
      bool bank = FALSE;
      int amt = atoi( argument );
      if( !chm )
      {
         send_to_char( "You aren't even a member of that clan!\n\r", ch );
         return;
      }
      for( atm = ch->in_room->first_content; atm; atm = atm->next_content )
      {
         if( atm->item_type == ITEM_BANK )
         {
            bank = TRUE;
            break;
         }
      }
      if( !bank )
      {
         send_to_char( "There is no bank here.\r\n", ch );
         return;
      }
      if( amt > 2000 )
      {
         send_to_char( "You can only deposit &C2000 mithril&w at a time.\n\r", ch );
         return;
      }
      if( ch->gold < amt * 1000000 )
      {
         send_to_char( "You don't have that many &Cmithril coins.\n\r", ch );
         return;
      }
      if( amt <= 0 )
      {
         send_to_char( "Err, how do you plan to do that?\n\r", ch );
         return;
      }
      ch->gold -= amt * 1000000;
      chm->bank += amt;
      clan->bank += amt;
      save_clans(  );
      ch_printf( ch, "You deposit &C%d mithril coin%s &winto the %s&w clan bank.\n\r", amt, amt > 1 ? "s" : "",
                   clan->title );
      return;
   }
   else if( !strcmp( arg1, "withdraw" ) )
   {
      OBJ_DATA * atm;
      bool bank = FALSE;
      int amt = atoi( argument );
      if( !chm )
      {
         send_to_char( "You aren't even a member of that clan!\n\r", ch );
         return;
      }
      if( !xIS_SET( chm->flags, CL_BANK ) )
      {
         send_to_char( "You don't have permission to withdraw from the clan bank.\n\r", ch );
         return;
      }
      if( !ch->in_room->first_content )
      {
         send_to_char( "There is no bank here.\n\r", ch );
         return;
      }
      for( atm = ch->in_room->first_content; atm; atm = atm->next_content )
      {
         if( atm && atm->item_type == ITEM_BANK )
         {
            bank = TRUE;
            break;
         }
      }
      if( !bank )
      {
         send_to_char( "There is no bank here.\r\n", ch );
         return;
      }
      if( chm->bank < amt && !xIS_SET( chm->flags, CL_FINANCE ) )
      {
         send_to_char( "You don't have that many &Cmithril coins in your bank account.\n\r", ch );
         return;
      }
      if( clan->bank < amt )
      {
         send_to_char( "There aren't that many &Cmithril coins&w in that clan's bank.\n\r", ch );
         return;
      }
      if( amt <= 0 )
      {
         send_to_char( "Err, how do you plan to do that?\n\r", ch );
         return;
      }
      if( amt > 1000 )
      {
         send_to_char( "You cannot withdraw more than &C1000 mithril&w at once.\n\r", ch );
         return;
      }
      if( ch->gold > 1000000000 )
      {
         send_to_char( "You are carrying too much &Cmithril&w already.\n\r", ch );
         return;
      }
      ch->gold += amt * 1000000;
      chm->bank -= amt;
      clan->bank -= amt;
      save_clans(  );
      ch_printf( ch, "You withdraw &C%d mithril coin%s&w from the %s&w clan bank.\n\r", amt, amt > 1 ? "s" : "",
                   clan->title );
      return;
   }
   else if( !strcmp( arg1, "show" ) )
   {
      if( !chm && clan->ext_sec >= 5  &&!IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
      {
         send_to_char( "You cannot view info on that clan.\n\r", ch );
         return;
      }
      ch_printf( ch, "&cName: &C%s&c   Abbreviation: &C%s\n\r", clan->title, clan->name );
      ch_printf( ch, "&cInfo: &C%s\n\r", clan->info );
      if( ( chm && xIS_SET( chm->flags, CL_KNOWLEDGE ) )  ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
         ch_printf( ch, "&cSecrecy - Internal: &C%d&c   External: &C%d&w\n\r", clan->int_sec, clan->ext_sec );
      if( ( chm && xIS_SET( chm->flags, CL_FINANCE ) )  ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
         ch_printf( ch, "&cBank: &C%d mithril\n\r", clan->bank );
      if( ( !chm && clan->ext_sec <= 3 ) 
           ||( chm && clan->int_sec <= 3 ) 
           ||( chm && xIS_SET( chm->flags, CL_KNOWLEDGE ) )  ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
      {
         ch_printf( ch, "&cMembers:&w\r\n" );
         for( member = clan->first_member; member; member = member->next )
         {
            if( !xIS_SET( member->flags, CL_SECRET ) 
                 ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  ||( chm && xIS_SET( chm->flags, CL_KNOWLEDGE ) ) )
            {
               ch_printf( ch, "%s", member->name );
               if( ( !chm && clan->ext_sec <= 2 ) 
                    ||( chm && clan->int_sec <= 2 ) 
                    ||( chm && xIS_SET( chm->flags, CL_KNOWLEDGE ) )  ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
                  ch_printf( ch, " - %s", member->title );
               if( ( !chm && clan->ext_sec <= 1 ) 
                    ||( chm && clan->int_sec <= 1 ) 
                    ||( chm && xIS_SET( chm->flags, CL_KNOWLEDGE ) )  ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
                  ch_printf( ch, " &G(&g%s&G)", ext_flag_string( &member->flags, c_flags ) );
               ch_printf( ch, "\n\r" );
               if( chm == member 
                    ||( chm && xIS_SET( chm->flags, CL_FINANCE ) )  ||IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
                  ch_printf( ch, "         &c- Donated &C%d mithril&w\n\r", member->bank );
            }
         }
      }
      return;
   }
   do_new_help( ch, "clan_syntax" );
   return;
}


/* induct <person> <clan> */ 
void do_induct( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CLAN_DATA * clan;
   CLAN_MEMBER * chm;
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg1 );
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They're not here.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "They don't look interested in your offer.\r\n", ch );
      return;
   }
   if( ( clan = get_clan( ch, argument ) ) == NULL )
   {
      send_to_char( "There is no such clan.\r\n", ch );
      return;
   }
   if( ( chm = get_member( ch->name, clan ) ) == NULL  &&!IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
   {
      send_to_char( "You are not in that clan.\n\r", ch );
      return;
   }
   if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&!xIS_SET( chm->flags, CL_INDUCT ) )
   {
      send_to_char( "You are permitted to induct new members into that clan.\r\n", ch );
      return;
   }
   if( get_member( victim->name, clan ) != NULL )
   {
      send_to_char( "They are already a member of that clan.\r\n", ch );
      return;
   }
   if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&!IS_CONSENTING( victim, ch ) )
   {
      send_to_char( "You may not induct them without their consent.\n\r", ch );
      return;
   }
   clan_add( victim, clan );
   sprintf( buf, "You add $N to %s&w.", clan->title );
   act( AT_PLAIN, buf, ch, NULL, victim, TO_CHAR );
   sprintf( buf, "$n adds you to %s&w.", clan->title );
   act( AT_PLAIN, buf, ch, NULL, victim, TO_VICT );
}


/* outcast <person> <clan> */ 
void do_outcast( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CLAN_DATA * clan;
   CLAN_MEMBER * chm;
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   victim = get_char_world( ch, arg1 );
   if( ( clan = get_clan( ch, ( char * )arg2 ) ) == NULL )
   {
      send_to_char( "There is no such clan.\r\n", ch );
      return;
   }
   if( get_member( ( char * )arg1, clan ) == NULL )
   {
      send_to_char( "That would be much easier if they were in that clan to begin with.\r\n", ch );
      return;
   }
   if( ( chm = get_member( ch->name, clan ) ) == NULL  &&!IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
   {
      send_to_char( "You are not in that clan.\n\r", ch );
      return;
   }
   if( ch != victim  &&!IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&!xIS_SET( chm->flags, CL_OUTCAST ) )
   {
      send_to_char( "You do not have permission to outcast from clan.\r\n", ch );
      return;
   }
   if( !xIS_SET( chm->flags, CL_LEADER )  &&!IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
   {
      chm = get_member( victim->name, clan );
      if( xIS_SET( chm->flags, CL_LEADER ) )
      {
         act( AT_RED, "Hey, $N is a leader and you aren't! You can't outcast $M!", ch, NULL, victim, TO_CHAR );
         act( AT_RED, "Hey, $n is trying to outcast you but isn't a leader!", ch, NULL, victim, TO_VICT );
         return;
      }
   }
   clan_remove( arg1, clan );
   if( ch == victim )
   {
      sprintf( buf, "You renounce the ways of %s&w.", clan->title );
      act( AT_PLAIN, buf, ch, NULL, NULL, TO_CHAR );
      sprintf( buf, "$n renounces the ways of %s&w.", clan->title );
      act( AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      sprintf( buf, "You remove $T from %s&w.", clan->title );
      act( AT_PLAIN, buf, ch, NULL, capitalize( arg1 ), TO_CHAR );
      if( victim )
      {
         sprintf( buf, "$n removes you from %s&w.", clan->title );
         act( AT_PLAIN, buf, ch, NULL, victim, TO_VICT );
      }
   }
}
void do_bestow( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   CLAN_MEMBER * chm;
   CLAN_DATA * clan;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( ( clan = get_clan( ch, arg2 ) ) == NULL )
   {
      send_to_char( "No clan goes by that abbreviation.\n\r", ch );
      return;
   }
   else
   {
      chm = get_member( ch->name, clan );
   }
   if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&!xIS_SET( chm->flags, CL_BESTOW ) )
   {
      send_to_char( "You don't have permission to bestow titles for that clan.\n\r", ch );
      return;
   }
   victim = get_char_room( ch, arg1 );
   if( !victim || !argument || argument[0] == '\0' )
   {
      send_to_char( "Bestow what title upon who?\n\r", ch );
      return;
   }
   chm = get_member( victim->name, clan );
   if( !chm )
   {
      send_to_char( "They aren't even a member of that clan.\n\r", ch );
      return;
   }
   smash_tilde( argument );
   STRFREE( chm->title );
   chm->title = STRALLOC( argument );
   save_clans(  );
   if( ch == victim )
   {
      sprintf( buf, "You declare yourself to be %s&w of %s&w.", chm->title, clan->title );
      act( AT_PLAIN, buf, ch, NULL, NULL, TO_CHAR );
      sprintf( buf, "$n declares $mself of %s&w of %s&w.", chm->title, clan->title );
      act( AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      sprintf( buf, "You bestow upon $N the title of %s&w of %s&w.", chm->title, clan->title );
      act( AT_PLAIN, buf, ch, NULL, victim, TO_CHAR );
      if( victim )
      {
         sprintf( buf, "$n bestows upon you the title of %s&w of %s&w.", chm->title, clan->title );
         act( AT_PLAIN, buf, ch, NULL, victim, TO_VICT );
      }
   }
}
void do_allow( CHAR_DATA * ch, char *argument )
{
   int i;
   CHAR_DATA * victim;
   CLAN_DATA * clan;
   CLAN_MEMBER * chm;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( ( clan = get_clan( ch, arg2 ) ) == NULL )
   {
      send_to_char( "No clan goes by that abbreviation.\n\r", ch );
      return;
   }
   else
   {
      chm = get_member( ch->name, clan );
   }
   if( !IS_SET( ch->pcdata->permissions, PERMIT_SECURITY )  &&!xIS_SET( chm->flags, CL_ALLOW ) )
   {
      send_to_char( "You don't have permission to alter priviledges for that clan", ch );
      return;
   }
   victim = get_char_room( ch, arg1 );
   if( !victim || !argument || argument[0] == '\0' )
   {
      send_to_char( "Bestow what title upon who?\n\r", ch );
      return;
   }
   i = get_clan_flag( argument );
   if( i == -1 )
   {
      send_to_char( "There is no such priveledge.\n\r", ch );
      return;
   }
   if( ( i == CL_LEADER || i == CL_ALLOW )  &&!IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
   {
      if( !chm )
      {
         send_to_char( "You aren't even a member of that clan!\n\r", ch );
         return;
      }
      if( !xIS_SET( chm->flags, CL_LEADER ) )
      {
         send_to_char( "Only a leader can grant that priviledge.\n\r", ch );
         return;
      }
   }
   if( ch == victim  &&!IS_SET( ch->pcdata->permissions, PERMIT_SECURITY ) )
   {
      send_to_char( "Grant yourself priviledges? That would be a trick!\n\r", ch );
      return;
   }
   chm = get_member( victim->name, clan );
   if( !chm )
   {
      send_to_char( "They aren't even a member of that clan.\n\r", ch );
      return;
   }
   xTOGGLE_BIT( chm->flags, i );
   save_clans(  );
   if( xIS_SET( chm->flags, i ) )
   {
      sprintf( buf, "You grant $N %s priviledges in %s&w.", argument, clan->title );
      act( AT_PLAIN, buf, ch, NULL, victim, TO_CHAR );
      sprintf( buf, "$n grants you %s priviledges in %s&w.", argument, clan->title );
      act( AT_PLAIN, buf, ch, NULL, victim, TO_VICT );
   }
   else
   {
      sprintf( buf, "You restrict $N from %s priviledges in %s&w.", argument, clan->title );
      act( AT_PLAIN, buf, ch, NULL, victim, TO_CHAR );
      sprintf( buf, "$n restricts you from %s priviledges in %s&w.", argument, clan->title );
      act( AT_PLAIN, buf, ch, NULL, victim, TO_VICT );
   }
}


