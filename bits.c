/* bits.c -- Abits and Qbits for the Rogue Winds by Scion
   Copyright 2000 by Peter Keeler, All Rights Reserved. The content
   of this file may be used by anyone for any purpose so long as this
   original header remains entirely intact and credit is given to the
   original author(s).

   The concept for this was inspired by Mallory's mob scripting system
   from AntaresMUD.

   It is not required, but I'd appreciate hearing back from people
   who use this code. What are you using it for, what have you done
   to it, ideas, comments, etc. So while it's not necessary, I'd love
   to get a note from you at keeler@teleport.com. Thanks! -- Scion
*/  
   
#include <stdio.h>
#include <string.h>
#include "mud.h"
   
/* These are the ends of the linked lists that store the mud's library
   of valid bits. */ 
   BIT_DATA * first_abit;
BIT_DATA * last_abit;
BIT_DATA * first_qbit;
BIT_DATA * last_qbit;

/* QBITS save, ABITS do not save. There are enough of each to give a range
   of them to builders the same as their vnums. They are identifiable by mobs
   running mob progs, and can be used as little identifiers for players..
   player X has done this and this and this.. think of them like a huge array
   of boolean variables you can put on a player or mob with a mob prog. -- Scion
*/ 
   
/* Find an abit in the mud's listing */ 
   BIT_DATA * find_abit( int number )
{
   BIT_DATA * bit;
   for( bit = first_abit; bit; bit = bit->next )
   {
      if( bit->number == number )
         return bit;
   }
   return NULL;
}


/* Find a qbit in the mud's listing */ 
   BIT_DATA * find_qbit( int number )
{
   BIT_DATA * bit;
   for( bit = first_qbit; bit; bit = bit->next )
   {
      if( bit->number == number )
         return bit;
   }
   return NULL;
}


/* Find an abit on a character and return it */ 
   BIT_DATA * get_abit( CHAR_DATA * ch, int number )
{
   BIT_DATA * bit;
   for( bit = ch->first_abit; bit; bit = bit->next )
   {
      if( bit->number == number )
         return bit;
   }
   return NULL;
}


/* Find a qbit on a character and return it */ 
   BIT_DATA * get_qbit( CHAR_DATA * ch, int number )
{
   BIT_DATA * bit;
   if( IS_NPC( ch ) )
      return NULL;
   for( bit = ch->pcdata->first_qbit; bit; bit = bit->next )
   {
      if( bit->number == number )
         return bit;
   }
   return NULL;
}


/* Write out the abit and qbit files */ 
void save_bits( void )
{
   BIT_DATA * start_bit;
   BIT_DATA * bit;
   FILE * fpout;
   char filename[MAX_INPUT_LENGTH];
   int mode = 0;
   
      /*
       * Print 2 files 
       */ 
      for( mode = 0; mode < 2; mode++ )
   {
      if( mode == 0 )
      {
         sprintf( filename, "%sabit.lst", SYSTEM_DIR );
         start_bit = first_abit;
      }
      else
      {
         sprintf( filename, "%sqbit.lst", SYSTEM_DIR );
         start_bit = first_qbit;
      }
      if( ( fpout = fopen( filename, "w" ) ) == NULL )
      {
         bug( "Cannot open bit list %d for writing", mode );
         return;
      }
      for( bit = start_bit; bit; bit = bit->next )
      {
         fprintf( fpout, "Number	%d\n", bit->number );
         fprintf( fpout, "Desc	%s~\n", bit->desc );
         fprintf( fpout, "End\n\n" );
      }
      fprintf( fpout, "#END\n" );
      fclose( fpout );
   }
}


/* Load the abits and qbits */ 
void load_bits( void )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   int mode = 0;
   BIT_DATA * bit;
   FILE * fp;
   sprintf( buf, "%sabit.lst", SYSTEM_DIR );
   if( ( fp = fopen( buf, "r" ) ) == NULL )
   {
      perror( buf );
      return;
   }
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
         case '#':
            if( !str_cmp( word, "#END" ) )
            {
               fclose( fp );
               if( mode == 0 )
               {
                  mode = 1;  /* We have two files to read, I reused the same code to read both */
                  sprintf( buf, "%sqbit.lst", SYSTEM_DIR );
                  if( ( fp = fopen( buf, "r" ) ) == NULL )
                  {
                     perror( buf );
                     return;
                  }
               }
               else
                  return;
               fMatch = TRUE;
            }
            break;
         case 'D':
            if( !str_cmp( word, "Desc" ) )
            {
               fMatch = TRUE;
               strcpy( bit->desc, fread_string( fp ) );
            }
            break;
         case 'E':
            if( !strcmp( word, "End" ) )
            {
               if( mode == 0 )
                  LINK( bit, first_abit, last_abit, next, prev );
               
               else
                  LINK( bit, first_qbit, last_qbit, next, prev );
               bit = NULL;
               fMatch = TRUE;
            }
            break;
         case 'N':
            if( !strcmp( word, "Number" ) )
            {
               CREATE( bit, BIT_DATA, 1 );
               bit->number = fread_number( fp );
               fMatch = TRUE;
            }
            break;
      }
      if( !fMatch )
      {
         sprintf( buf, "load_bits: no match: %s", word );
         bug( buf, 0 );
      }
   }
}


/* Add an abit to a character */ 
void set_abit( CHAR_DATA * ch, int number )
{
   BIT_DATA * bit;
   BIT_DATA * proto_bit;
   if( number < 0 || number > MAX_xBITS )
      return;
   for( proto_bit = first_abit; proto_bit; proto_bit = proto_bit->next )
   {
      if( proto_bit->number == number )
         break;
   }
   if( proto_bit == NULL )
      return;
   if( ( bit = get_abit( ch, number ) ) == NULL )
   {
      CREATE( bit, BIT_DATA, 1 );
      bit->number = proto_bit->number;
      strcpy( bit->desc, proto_bit->desc );
      LINK( bit, ch->first_abit, ch->last_abit, next, prev );
   }
}


/* Add a qbit to a character */ 
void set_qbit( CHAR_DATA * ch, int number )
{
   BIT_DATA * bit;
   BIT_DATA * proto_bit;
   if( IS_NPC( ch ) )
      return;
   if( number < 0 || number > MAX_xBITS )
      return;
   for( proto_bit = first_qbit; proto_bit; proto_bit = proto_bit->next )
   {
      if( proto_bit->number == number )
         break;
   }
   if( proto_bit == NULL )
      return;
   if( ( bit = get_qbit( ch, number ) ) == NULL )
   {
      CREATE( bit, BIT_DATA, 1 );
      bit->number = proto_bit->number;
      strcpy( bit->desc, proto_bit->desc );
      LINK( bit, ch->pcdata->first_qbit, ch->pcdata->last_qbit, next, prev );
   }
}


/* Take an abit off a character */ 
void remove_abit( CHAR_DATA * ch, int number )
{
   BIT_DATA * bit;
   if( number < 0 || number > MAX_xBITS )
      return;
   if( !ch->first_abit )
      return;
   bit = ch->first_abit;
   while( bit )
   {
      BIT_DATA * tmp;
      if( bit->number == number )
      {
         tmp = bit;
         bit = bit->next;
         UNLINK( tmp, ch->first_abit, ch->last_abit, next, prev );
         DISPOSE( tmp );
      }
      else
         bit = bit->next;
   }
}


/* Take a qbit off a character */ 
void remove_qbit( CHAR_DATA * ch, int number )
{
   BIT_DATA * bit;
   if( IS_NPC( ch ) )
      return;
   if( number < 0 || number > MAX_xBITS )
      return;
   if( !ch->pcdata->first_qbit )
      return;
   bit = ch->pcdata->first_qbit;
   while( bit )
   {
      BIT_DATA * tmp;
      if( bit->number == number )
      {
         tmp = bit;
         bit = bit->next;
         UNLINK( tmp, ch->pcdata->first_qbit, ch->pcdata->last_qbit, next, prev );
         DISPOSE( tmp );
      }
      else
         bit = bit->next;
   }
}


/* Show an abit from the mud's linked list, or all of them if 'all' is the argument */ 
void do_showabit( CHAR_DATA * ch, char *argument )
{
   int number;
   char buf[MAX_STRING_LENGTH];
   BIT_DATA * bit;
   if( !strcmp( argument, "all" ) )
   {
      for( bit = first_abit; bit; bit = bit->next )
      {
         sprintf( buf, "&wABIT: %d <%25.25s>\r\n", bit->number, bit->desc );
         send_to_char( buf, ch );
      }
      return;
   }
   number = atoi( argument );
   if( number < 0 || number > MAX_xBITS )
      return;
   for( bit = first_abit; bit; bit = bit->next )
   {
      if( bit->number == number )
         break;
   }
   if( bit == NULL )
   {
      send_to_char( "That abit does not exist.\r\n", ch );
      return;
   }
   sprintf( buf, "&wABIT: %d\n\r", bit->number );
   send_to_char( buf, ch );
   sprintf( buf, "'%s&w'\n\r", bit->desc );
   send_to_char( buf, ch );
}


/* Show a qbit from the mud's linked list or all of them if 'all' is the argument */ 
void do_showqbit( CHAR_DATA * ch, char *argument )
{
   int number;
   char buf[MAX_STRING_LENGTH];
   BIT_DATA * bit;
   if( !strcmp( argument, "all" ) )
   {
      for( bit = first_qbit; bit; bit = bit->next )
      {
         sprintf( buf, "&wQBIT: %d <%25.25s>\r\n", bit->number, bit->desc );
         send_to_char( buf, ch );
      }
      return;
   }
   number = atoi( argument );
   if( number < 0 || number > MAX_xBITS )
      return;
   for( bit = first_qbit; bit; bit = bit->next )
   {
      if( bit->number == number )
         break;
   }
   if( bit == NULL )
   {
      send_to_char( "That qbit does not exist.\r\n", ch );
      return;
   }
   sprintf( buf, "&wQBIT: %d\r\n", bit->number );
   send_to_char( buf, ch );
   sprintf( buf, "'%s&w'\n\r", bit->desc );
   send_to_char( buf, ch );
}


/* setabit <number> <desc> */ 
/* Set the description for a particular abit */ 
void do_setabit( CHAR_DATA * ch, char *argument )
{
   BIT_DATA * bit;
   int number;
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg );
   number = atoi( arg );
   strcpy( arg, argument );
   if( number < 0 || number > MAX_xBITS )
   {
      send_to_char( "That is not a valid number for an abit.\r\n", ch );
      return;
   }
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: setabit <number> <description>\n", ch );
      return;
   }
   for( bit = first_abit; bit; bit = bit->next )
   {
      if( bit->number == number )
         break;
   }
   if( bit == NULL )
   {
      CREATE( bit, BIT_DATA, 1 );
      bit->number = number;
      strcpy( bit->desc, "" );
      LINK( bit, first_abit, last_abit, next, prev );
      sprintf( buf, "Abit %d created.\r\n", bit->number );
   }
   strcpy( bit->desc, arg );
   sprintf( buf, "Description for abit %d set to '%s&w'.\r\n", bit->number, bit->desc );
   send_to_char( buf, ch );
   save_bits(  );
}


/* setqbit <number> <desc> */ 
/* Set the description for a particular qbit */ 
void do_setqbit( CHAR_DATA * ch, char *argument )
{
   BIT_DATA * bit;
   int number;
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg );
   number = atoi( arg );
   strcpy( arg, argument );
   if( number < 0 || number > MAX_xBITS )
   {
      send_to_char( "That is not a valid number for a qbit.\r\n", ch );
      return;
   }
   if( arg[0] == '\0' )
   {
      send_to_char( "Syntax: setqbit <number> <description>\n", ch );
      return;
   }
   for( bit = first_qbit; bit; bit = bit->next )
   {
      if( bit->number == number )
         break;
   }
   if( bit == NULL )
   {
      CREATE( bit, BIT_DATA, 1 );
      bit->number = number;
      strcpy( bit->desc, "" );
      LINK( bit, first_qbit, last_qbit, next, prev );
      sprintf( buf, "Qbit %d created.\r\n", bit->number );
   }
   strcpy( bit->desc, arg );
   sprintf( buf, "Description for qbit %d set to '%s&w'.\r\n", bit->number, bit->desc );
   send_to_char( buf, ch );
   save_bits(  );
}


/* Imm command to toggle an abit on a character or to list the abits already on a character */ 
void do_abit( CHAR_DATA * ch, char *argument )
{
   BIT_DATA * bit;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA * victim;
   argument = one_argument( argument, buf );
   if( ( victim = get_char_world( ch, buf ) ) == NULL )
   {
      send_to_char( "They are not in the game.\r\n", ch );
      return;
   }
   if( !argument || argument[0] == '\0' )
   {
      if( !victim->first_abit )
      {
         send_to_char( "They have no abits set on them.\r\n", ch );
         return;
      }
      send_to_char( "ABITS:\r\n", ch );
      for( bit = victim->first_abit; bit; bit = bit->next )
      {
         sprintf( buf, "%4.4d: <%30.30s&w>\r\n", bit->number, bit->desc );
         send_to_char( buf, ch );
      }
   }
   else
   {
      int abit;
      abit = atoi( argument );
      if( abit < 0 || abit > MAX_xBITS )
      {
         send_to_char( "That is an invalid abit number.\r\n", ch );
         return;
      }
      if( get_abit( victim, abit ) != NULL )
      {
         remove_abit( victim, abit );
         sprintf( buf, "Removed abit %d from %s.\r\n", abit, victim->name );
         send_to_char( buf, ch );
      }
      else
      {
         set_abit( victim, abit );
         sprintf( buf, "Added abit %d to %s.\r\n", abit, victim->name );
         send_to_char( buf, ch );
      }
   }
}


/* Immortal command to toggle a qbit on a character or to list the qbits already on a character */ 
void do_qbit( CHAR_DATA * ch, char *argument )
{
   BIT_DATA * bit;
   CHAR_DATA * victim;
   char buf[MAX_STRING_LENGTH];
   argument = one_argument( argument, buf );
   if( ( victim = get_char_world( ch, buf ) ) == NULL )
   {
      send_to_char( "They are not in the game.\r\n", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "NPCs cannot have qbits.\r\n", ch );
      return;
   }
   argument = one_argument( argument, buf );
   if( !buf || buf[0] == '\0' )
   {
      if( !victim->pcdata->first_qbit )
      {
         send_to_char( "They do not have any qbits.\r\n", ch );
         return;
      }
      send_to_char( "QBITS:\r\n", ch );
      for( bit = victim->pcdata->first_qbit; bit; bit = bit->next )
      {
         sprintf( buf, "%4.4d: <%30.30s&w>\r\n", bit->number, bit->desc );
         send_to_char( buf, ch );
      }
   }
   else
   {
      int qbit;
      qbit = atoi( buf );
      if( qbit < 0 || qbit > MAX_xBITS )
      {
         send_to_char( "That is an invalid qbit number.\r\n", ch );
         return;
      }
      if( get_qbit( victim, qbit ) != NULL )
      {
         remove_qbit( victim, qbit );
         sprintf( buf, "Removed qbit %d from %s.\r\n", qbit, victim->name );
         send_to_char( buf, ch );
      }
      else
      {
         set_qbit( victim, qbit );
         sprintf( buf, "Added qbit %d to %s.\r\n", qbit, victim->name );
         send_to_char( buf, ch );
      }
   }
}


/* mpaset <char> */ 
/* Mob prog version of do_abit */ 
void do_mpaset( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   int number;
   if( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
      
   {
      send_to_char( "mpaset whom?\n\r", ch );
      progbug( "Mpaset: invalid argument1", ch );
      return;
   }
   if( arg2[0] == '\0' )
      
   {
      send_to_char( "mpaset which bit?\n\r", ch );
      progbug( "Mpaset: invalid argument2", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      
   {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpaset: victim not in room", ch );
      return;
   }
   number = atoi( arg2 );
   if( get_abit( victim, number ) != NULL )
      remove_abit( victim, number );
   
   else
      set_abit( victim, number );
}


/* mpqset <char> */ 
/* Mob prog version of do_qbit */ 
void do_mpqset( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   int number;
   if( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
      
   {
      send_to_char( "mpqset whom?\n\r", ch );
      progbug( "Mpqset: invalid argument1", ch );
      return;
   }
   if( arg2[0] == '\0' )
      
   {
      send_to_char( "mpqset which bit?\n\r", ch );
      progbug( "Mpqset: invalid argument2", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      
   {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpqset: victim not in room", ch );
      return;
   }
   if( IS_NPC( victim ) )
      return;
   number = atoi( arg2 );
   if( get_qbit( victim, number ) != NULL )
      remove_qbit( victim, number );
   
   else
      set_qbit( victim, number );
}


