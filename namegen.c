/* 
 * Name generators
 *
 * Written by Keolah and Scion for Rogue Winds
 * Copyright (C) 2002 by Heather Dunn
 *
 */  
   
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
void do_namegen( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int lang;
   if( !str_cmp( argument, "elven" )  ||!str_cmp( argument, "elvish" )  ||!str_cmp( argument, "elf" ) )
      lang = LANG_ELVEN;
   
   else if( !str_cmp( argument, "dwarven" ) 
            ||!str_cmp( argument, "dwarvish" ) 
            ||!str_cmp( argument, "gnome" )  ||!str_cmp( argument, "gnomish" )  ||!str_cmp( argument, "dwarf" ) )
      lang = LANG_DWARVEN;
   
   else if( !str_cmp( argument, "orc" ) 
            ||!str_cmp( argument, "orcish" )  ||!str_cmp( argument, "kobold" )  ||!str_cmp( argument, "goblin" ) )
      lang = LANG_ORCISH;
   
   else if( !str_cmp( argument, "dragon" )  ||!str_cmp( argument, "draconic" )  ||!str_cmp( argument, "draconian" ) )
      lang = LANG_DRAGON;
   
   else if( !str_cmp( argument, "drow" ) 
            ||!str_cmp( argument, "dark elf" )  ||!str_cmp( argument, "dark elven" )  ||!str_cmp( argument, "drowish" ) )
      lang = LANG_DROW;
   
   else
      lang = LANG_KALORESE;
   strcpy( buf, random_name( buf, lang ) );
   ch_printf( ch, "%s name generated: %s\n\r", lang_names[lang], buf );
}


/* -- Random Name Generator: written by Scion, 
 * derived from Java code from Keolah -- 
 * May 10, 2002 - Multiple language support added - Keolah 
 */ 
   
/* NEXT LETTER */ 
char *next_vowel( char *word, bool dipthong )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 11 ) )
   {
      case 0:
      case 1:
      case 2:
         strcat( buf, "a" );
         break;
      case 3:
      case 4:
      case 5:
         strcat( buf, "e" );
         break;
      case 6:
      case 7:
         strcat( buf, "i" );
         break;
      case 8:
      case 9:
         strcat( buf, "o" );
         break;
      case 10:
         strcat( buf, "u" );
         break;
      case 11:
         if( dipthong )
         {
            strcpy( buf, next_vowel( word, FALSE ) );
            strcpy( word, buf );
            strcpy( buf, next_vowel( word, FALSE ) );
            break;
         }
         else
         {
            strcat( buf, "i" );
            break;
         }
   }
   strcpy( word, buf );
   return word;
}
char *next_vowel_elven( char *word, bool dipthong )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 15 ) )
   {
      case 0:
      case 1:
      case 2:
         strcat( buf, "a" );
         break;
      case 3:
      case 4:
      case 5:
         strcat( buf, "e" );
         break;
      case 6:
      case 7:
         strcat( buf, "i" );
         break;
      case 8:
      case 9:
         strcat( buf, "o" );
         break;
      case 10:
         strcat( buf, "u" );
         break;
      case 11:
      case 12:
         strcat( buf, "y" );
         break;
      case 13:
      case 14:
      case 15:
         if( dipthong )
         {
            strcpy( buf, next_vowel_elven( word, FALSE ) );
            strcpy( word, buf );
            strcpy( buf, next_vowel_elven( word, FALSE ) );
            break;
         }
         else
         {
            strcat( buf, "i" );
            break;
         }
   }
   strcpy( word, buf );
   return word;
}
char *next_vowel_dwarven( char *word, bool dipthong )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 4 ) )
   {
      case 0:
         strcat( buf, "a" );
         break;
      case 1:
         strcat( buf, "e" );
         break;
      case 2:
         strcat( buf, "i" );
         break;
      case 3:
         strcat( buf, "o" );
         break;
      case 4:
         strcat( buf, "u" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_vowel_orcish( char *word, bool dipthong )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 7 ) )
   {
      case 0:
         strcat( buf, "a" );
         break;
      case 1:
         strcat( buf, "e" );
         break;
      case 2:
         strcat( buf, "i" );
         break;
      case 3:
      case 4:
         strcat( buf, "o" );
         break;
      case 5:
      case 6:
      case 7:
         strcat( buf, "u" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_stop( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 12 ) )
   {
      case 0:
      case 1:
      case 2:
         strcat( buf, "t" );
         break;
      case 3:
      case 4:
      case 5:
         strcat( buf, "d" );
         break;
      case 6:
      case 7:
      case 8:
         strcat( buf, "k" );
         break;
      case 9:
      case 10:
         strcat( buf, "g" );
         break;
      case 11:
         strcat( buf, "p" );
         break;
      case 12:
         strcat( buf, "b" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_stop_elven( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 15 ) )
   {
      case 0:
      case 1:
      case 2:
      case 3:
         strcat( buf, "t" );
         break;
      case 4:
      case 5:
      case 6:
      case 7:
         strcat( buf, "d" );
         break;
      case 8:
      case 9:
      case 10:
         strcat( buf, "c" );
         break;
      case 11:
         strcat( buf, "g" );
         break;
      case 12:
         strcat( buf, "p" );
         break;
      case 13:
      case 14:
         strcat( buf, "b" );
         break;
      case 15:
         strcat( buf, "k" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_stop_dwarven( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 6 ) )
   {
      case 0:
         strcat( buf, "t" );
         break;
      case 1:
         strcat( buf, "d" );
         break;
      case 2:
         strcat( buf, "k" );
         break;
      case 3:
         strcat( buf, "g" );
         break;
      case 4:
         strcat( buf, "b" );
         break;
      case 5:
         strcat( buf, "kh" );
         break;
      case 6:
         strcat( buf, "th" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_labial_stop( char *word, bool labial )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   if( labial )
   {
      switch ( number_range( 0, 2 ) )
      {
         case 0:
         case 1:
            strcat( buf, "b" );
            break;
         case 2:
            strcat( buf, "p" );
            break;
      }
   }
   else
   {
      switch ( number_range( 0, 6 ) )
      {
         case 0:
         case 1:
            strcat( buf, "t" );
            break;
         case 2:
         case 3:
            strcat( buf, "d" );
            break;
         case 4:
         case 5:
            strcat( buf, "k" );
            break;
         case 6:
            strcat( buf, "g" );
            break;
      }
   }
   strcpy( word, buf );
   return word;
}
char *next_labial_stop_elven( char *word, bool labial )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   if( labial )
   {
      strcat( buf, "b" );
   }
   else
   {
      switch ( number_range( 0, 10 ) )
      {
         case 0:
         case 1:
         case 2:
         case 3:
            strcat( buf, "t" );
            break;
         case 4:
         case 5:
         case 6:
         case 7:
            strcat( buf, "d" );
            break;
         case 8:
         case 9:
            strcat( buf, "k" );
            break;
         case 10:
            strcat( buf, "g" );
            break;
      }
   }
   strcpy( word, buf );
   return word;
}
char *next_fricative( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 15 ) )
   {
      case 0:
      case 1:
      case 2:
      case 3:
         strcat( buf, "s" );
         break;
      case 4:
      case 5:
         strcat( buf, "f" );
         break;
      case 6:
      case 7:
         strcat( buf, "v" );
         break;
      case 8:
      case 9:
         strcat( buf, "z" );
         break;
      case 10:
         strcat( buf, "sh" );
         break;
      case 11:
         strcat( buf, "zh" );
         break;
      case 12:
      case 13:
      case 14:
         strcat( buf, "th" );
         break;
      case 15:
         strcat( buf, "dh" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_fricative_elven( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 12 ) )
   {
      case 0:
      case 1:
      case 2:
      case 3:
         strcat( buf, "s" );
         break;
      case 4:
         strcat( buf, "f" );
         break;
      case 5:
      case 6:
         strcat( buf, "v" );
         break;
      case 7:
      case 8:
         strcat( buf, "sh" );
         break;
      case 9:
      case 10:
      case 11:
      case 12:
         strcat( buf, "th" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_fricative_dwarven( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 12 ) )
   {
      case 0:
         strcat( buf, "s" );
         break;
      case 1:
         strcat( buf, "f" );
         break;
      case 2:
         strcat( buf, "gh" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_liquid( char *word, bool vowel )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 7 ) )
   {
      case 0:
      case 1:
      case 2:
         strcat( buf, "l" );
         break;
      case 3:
      case 4:
      case 5:
         strcat( buf, "r" );
         break;
      case 6:
         strcat( buf, "w" );
         break;
      case 7:
         strcat( buf, "y" );
         break;
   }
   strcpy( word, buf );
   if( vowel )
      next_vowel( word, TRUE );
   return word;
}
char *next_liquid_elven( char *word, bool vowel )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 1 ) )
   {
      case 0:
         strcat( buf, "l" );
         break;
      case 1:
         strcat( buf, "r" );
         break;
   }
   strcpy( word, buf );
   if( vowel )
      next_vowel_elven( word, TRUE );
   return word;
}
char *next_liquid_dwarven( char *word, bool vowel )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 2 ) )
   {
      case 0:
         strcat( buf, "l" );
         break;
      case 1:
         strcat( buf, "r" );
         break;
      case 2:
         strcat( buf, "z" );
         break;
   }
   strcpy( word, buf );
   if( vowel )
      next_vowel_dwarven( word, TRUE );
   return word;
}
char *next_nasal( char *word, bool vowel )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   if( number_range( 0, 5 ) == 0 )
      strcat( buf, "m" );
   
   else
      strcat( buf, "n" );
   strcpy( word, buf );
   if( vowel )
      next_vowel( word, TRUE );
   return word;
}
char *next_nasal_elven( char *word, bool vowel )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   if( number_range( 0, 5 ) == 0 )
      strcat( buf, "m" );
   
   else
      strcat( buf, "n" );
   strcpy( word, buf );
   if( vowel )
      next_vowel_elven( word, TRUE );
   return word;
}
char *next_weird( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 6 ) )
   {
      case 0:
      case 1:
         strcat( buf, "x" );
         break;
      case 2:
      case 3:
         strcat( buf, "ch" );
         break;
      case 4:
         strcat( buf, "j" );
         break;
      case 5:
      case 6:
         strcat( buf, "h" );
         break;
   }
   strcpy( word, buf );
   return word;
}
char *next_weird_elven( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   strcat( buf, "h" );
   strcpy( word, buf );
   return word;
}
char *next_weird_dwarven( char *word )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   switch ( number_range( 0, 2 ) )
   {
      case 0:
         strcat( buf, "x" );
         break;
      case 1:
         strcat( buf, "ch" );
         break;
      case 2:
         strcat( buf, "h" );
         break;
   }
   strcpy( word, buf );
   return word;
}


/* LAST LETTER */ 
char *last_vowel( char *word )
{
   switch ( number_range( 0, 4 ) )
   {
      case 0:
         return next_liquid( word, FALSE );
      case 1:
         return next_stop( word );
      case 2:
         return next_nasal( word, FALSE );
      case 3:
         return next_fricative( word );
      case 4:
         return next_weird( word );
   }
   return word;
}
char *last_vowel_elven( char *word )
{
   switch ( number_range( 0, 4 ) )
   {
      case 0:
      case 1:
         return next_liquid_elven( word, FALSE );
      case 2:
      case 3:
         return next_nasal_elven( word, FALSE );
      case 4:
         return next_fricative_elven( word );
   }
   return word;
}
char *last_vowel_dwarven( char *word )
{
   switch ( number_range( 0, 4 ) )
   {
      case 0:
         return next_liquid_dwarven( word, FALSE );
      case 1:
         return next_stop_dwarven( word );
      case 2:
         return next_nasal( word, FALSE );
      case 3:
         return next_fricative_dwarven( word );
      case 4:
         return next_weird_dwarven( word );
   }
   return word;
}
char *last_vowel_drow( char *word )
{
   switch ( number_range( 0, 4 ) )
   {
      case 0:
      case 1:
         return next_liquid_dwarven( word, FALSE );
      case 2:
      case 3:
         return next_nasal_elven( word, FALSE );
      case 4:
         return next_fricative_elven( word );
   }
   return word;
}
char *last_liquid( char *word )
{
   switch ( number_range( 0, 4 ) )
   {
      case 0:
         return next_vowel( word, TRUE );
      case 1:
         return next_stop( word );
      case 2:
         return next_nasal( word, FALSE );
      case 3:
         return next_fricative( word );
      case 4:
         return next_weird( word );
   }
   return word;
}
char *last_liquid_elven( char *word )
{
   switch ( number_range( 0, 4 ) )
   {
      case 0:
      case 1:
      case 2:
      case 3:
         return next_vowel_elven( word, TRUE );
      case 4:
         return next_stop_elven( word );
   }
   return word;
}
char *last_liquid_dwarven( char *word )
{
   switch ( number_range( 0, 1 ) )
   {
      case 0:
         return next_vowel_dwarven( word, TRUE );
      case 1:
         return next_stop_dwarven( word );
   }
   return word;
}
char *last_fricative( char *word, int len )
{
   if( len > 1 )
   {
      if( number_range( 0, 1 ) == 1 )
         return next_liquid( word, TRUE );
   }
   return next_vowel( word, TRUE );
}
char *last_fricative_elven( char *word, int len )
{
   if( len > 1 )
   {
      if( number_range( 0, 1 ) == 1 )
         return next_liquid_elven( word, TRUE );
   }
   return next_vowel_elven( word, TRUE );
}
char *last_stop( char *word, int len, bool dental )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   if( len > 1 )
   {
      if( number_range( 0, 1 ) == 1 )
      {
         if( dental )
         {
            strcat( buf, "r" );
            strcpy( word, buf );
            return next_vowel( word, TRUE );
         }
         else
         {
            strcpy( word, buf );
            return next_liquid( word, TRUE );
         }
      }
   }
   strcpy( word, buf );
   return next_vowel( word, TRUE );
}
char *last_stop_elven( char *word, int len, bool dental )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( buf, word );
   if( len > 1 )
   {
      if( number_range( 0, 2 ) == 0 )
      {
         if( dental )
         {
            strcat( buf, "r" );
            strcpy( word, buf );
            return next_vowel_elven( word, TRUE );
         }
         else
         {
            strcpy( word, buf );
            return next_liquid_elven( word, TRUE );
         }
      }
   }
   strcpy( word, buf );
   return next_vowel( word, TRUE );
}
char *last_stop_dwarven( char *word )
{
   return next_vowel_dwarven( word, TRUE );
}
char *last_nasal( char *word, int len, bool labial )
{
   if( number_range( 0, 2 ) == 0 )
      return next_labial_stop( word, labial );
   return next_vowel( word, TRUE );
}
char *last_nasal_elven( char *word, int len, bool labial )
{
   if( number_range( 0, 4 ) == 0 )
      return next_labial_stop_elven( word, labial );
   return next_vowel_elven( word, TRUE );
}
char *last_nasal_dwarven( char *word, int len, bool labial )
{
   if( number_range( 0, 3 ) == 0 )
      return next_stop_dwarven( word );
   return next_vowel_dwarven( word, TRUE );
}
char *last_weird( char *word )
{
   return next_vowel( word, TRUE );
}
char *last_weird_elven( char *word )
{
   return next_vowel_elven( word, TRUE );
}


/* ADD LETTER */ 
char *addletter( char *word )
{
   int len = 1;
   if( strlen( word ) == 0 )
   {
      switch ( number_range( 0, 5 ) )
      {
         case 0:
            return next_stop( word );
         case 1:
            return next_vowel( word, FALSE );
         case 2:
            return next_liquid( word, TRUE );
         case 3:
            return next_nasal( word, TRUE );
         case 4:
            return next_fricative( word );
         case 5:
            return next_weird( word );
      }
      return word;
   }
   switch ( word[strlen( word ) - 1] )
   {
      case 'a':
         return last_vowel( word );
      case 'b':
         return last_stop( word, len, FALSE );
      case 'c':
         return last_stop( word, len, FALSE );
      case 'd':
         return last_stop( word, len, TRUE );
      case 'e':
         return last_vowel( word );
      case 'f':
         return last_fricative( word, len );
      case 'g':
         return last_stop( word, len, FALSE );
      case 'h':
         return last_weird( word );
      case 'i':
         return last_vowel( word );
      case 'j':
         return last_weird( word );
      case 'k':
         return last_stop( word, len, FALSE );
      case 'l':
         return last_liquid( word );
      case 'm':
         return last_nasal( word, len, TRUE );
      case 'n':
         return last_nasal( word, len, FALSE );
      case 'o':
         return last_vowel( word );
      case 'p':
         return last_stop( word, len, FALSE );
      case 'r':
         return last_liquid( word );
      case 's':
         return last_fricative( word, len );
      case 't':
         return last_stop( word, len, TRUE );
      case 'u':
         return last_vowel( word );
      case 'v':
         return last_fricative( word, len );
      case 'w':
         return last_liquid( word );
      case 'x':
         return last_weird( word );
      case 'y':
         return last_liquid( word );
      case 'z':
         return last_fricative( word, len );
   }
   return word;
}
char *addletter_elven( char *word )
{
   int len = 1;
   if( strlen( word ) == 0 )
   {
      switch ( number_range( 0, 5 ) )
      {
         case 0:
            return next_stop_elven( word );
         case 1:
            return next_vowel_elven( word, TRUE );
         case 2:
            return next_liquid_elven( word, TRUE );
         case 3:
            return next_nasal_elven( word, TRUE );
         case 4:
            return next_fricative_elven( word );
         case 5:
            return next_weird_elven( word );
      }
      return word;
   }
   switch ( word[strlen( word ) - 1] )
   {
      case 'a':
         return last_vowel_elven( word );
      case 'b':
         return last_stop_elven( word, len, FALSE );
      case 'c':
         return last_stop_elven( word, len, FALSE );
      case 'd':
         return last_stop_elven( word, len, TRUE );
      case 'e':
         return last_vowel_elven( word );
      case 'f':
         return last_fricative_elven( word, len );
      case 'g':
         return last_stop_elven( word, len, FALSE );
      case 'h':
         return last_weird_elven( word );
      case 'i':
         return last_vowel_elven( word );
      case 'j':
         return last_weird_elven( word );
      case 'k':
         return last_stop_elven( word, len, FALSE );
      case 'l':
         return last_liquid_elven( word );
      case 'm':
         return last_nasal_elven( word, len, TRUE );
      case 'n':
         return last_nasal_elven( word, len, FALSE );
      case 'o':
         return last_vowel_elven( word );
      case 'p':
         return last_stop_elven( word, len, FALSE );
      case 'r':
         return last_liquid_elven( word );
      case 's':
         return last_fricative_elven( word, len );
      case 't':
         return last_stop_elven( word, len, TRUE );
      case 'u':
         return last_vowel_elven( word );
      case 'v':
         return last_fricative_elven( word, len );
      case 'w':
         return last_liquid_elven( word );
      case 'x':
         return last_weird_elven( word );
      case 'y':
         return last_vowel_elven( word );
      case 'z':
         return last_fricative_elven( word, len );
   }
   return word;
}
char *addletter_dwarven( char *word )
{
   int len = 1;
   if( strlen( word ) == 0 )
   {
      switch ( number_range( 0, 5 ) )
      {
         case 0:
            return next_stop_dwarven( word );
         case 1:
            return next_vowel_dwarven( word, TRUE );
         case 2:
            return next_liquid_dwarven( word, TRUE );
         case 3:
            return next_nasal( word, TRUE );
         case 4:
            return next_fricative_dwarven( word );
         case 5:
            return next_weird_dwarven( word );
      }
      return word;
   }
   switch ( word[strlen( word ) - 1] )
   {
      case 'a':
         return last_vowel_dwarven( word );
      case 'b':
         return last_stop_dwarven( word );
      case 'c':
         return last_stop_dwarven( word );
      case 'd':
         return last_stop_dwarven( word );
      case 'e':
         return last_vowel_dwarven( word );
      case 'f':
         return last_stop_dwarven( word );
      case 'g':
         return last_stop_dwarven( word );
      case 'h':
         return last_stop_dwarven( word );
      case 'i':
         return last_vowel_dwarven( word );
      case 'j':
         return last_stop_dwarven( word );
      case 'k':
         return last_stop_dwarven( word );
      case 'l':
         return last_liquid_dwarven( word );
      case 'm':
         return last_nasal_dwarven( word, len, TRUE );
      case 'n':
         return last_nasal_dwarven( word, len, FALSE );
      case 'o':
         return last_vowel_dwarven( word );
      case 'p':
         return last_stop_dwarven( word );
      case 'r':
         return last_liquid_dwarven( word );
      case 's':
         return last_stop_dwarven( word );
      case 't':
         return last_stop_dwarven( word );
      case 'u':
         return last_vowel_dwarven( word );
      case 'v':
         return last_stop_dwarven( word );
      case 'w':
         return last_liquid_dwarven( word );
      case 'x':
         return last_stop_dwarven( word );
      case 'y':
         return last_vowel_dwarven( word );
      case 'z':
         return last_liquid_dwarven( word );
   }
   return word;
}
char *addletter_drow( char *word )
{
   int len = 1;
   if( strlen( word ) == 0 )
   {
      switch ( number_range( 0, 5 ) )
      {
         case 0:
            return next_stop_elven( word );
         case 1:
            return next_vowel_elven( word, TRUE );
         case 2:
            return next_liquid_dwarven( word, TRUE );
         case 3:
            return next_nasal_elven( word, TRUE );
         case 4:
            return next_fricative_elven( word );
         case 5:
            return next_weird_elven( word );
      }
      return word;
   }
   switch ( word[strlen( word ) - 1] )
   {
      case 'a':
         return last_vowel_drow( word );
      case 'b':
         return last_stop_elven( word, len, FALSE );
      case 'c':
         return last_stop_elven( word, len, FALSE );
      case 'd':
         return last_stop_elven( word, len, TRUE );
      case 'e':
         return last_vowel_drow( word );
      case 'f':
         return last_fricative_elven( word, len );
      case 'g':
         return last_stop_elven( word, len, FALSE );
      case 'h':
         return last_weird_elven( word );
      case 'i':
         return last_vowel_drow( word );
      case 'j':
         return last_weird_elven( word );
      case 'k':
         return last_stop_elven( word, len, FALSE );
      case 'l':
         return last_liquid_elven( word );
      case 'm':
         return last_nasal_elven( word, len, TRUE );
      case 'n':
         return last_nasal_elven( word, len, FALSE );
      case 'o':
         return last_vowel_drow( word );
      case 'p':
         return last_stop_elven( word, len, FALSE );
      case 'r':
         return last_liquid_elven( word );
      case 's':
         return last_fricative_elven( word, len );
      case 't':
         return last_stop_elven( word, len, TRUE );
      case 'u':
         return last_vowel_drow( word );
      case 'v':
         return last_fricative_elven( word, len );
      case 'w':
         return last_liquid_elven( word );
      case 'x':
         return last_weird_elven( word );
      case 'y':
         return last_vowel_drow( word );
      case 'z':
         return last_fricative_elven( word, len );
   }
   return word;
}
char *generate( char *word, int len )
{
   char buf[MAX_STRING_LENGTH];
   if( len == 0 )
      return word;
   strcpy( buf, word );
   strcpy( buf, generate( addletter( word ), len - 1 ) );
   strcpy( word, buf );
   return word;
}
char *generate_elven( char *word, int len )
{
   char buf[MAX_STRING_LENGTH];
   if( len == 0 )
      return word;
   strcpy( buf, word );
   strcpy( buf, generate_elven( addletter_elven( word ), len - 1 ) );
   strcpy( word, buf );
   return word;
}
char *generate_dwarven( char *word, int len )
{
   char buf[MAX_STRING_LENGTH];
   if( len == 0 )
      return word;
   strcpy( buf, word );
   strcpy( buf, generate_dwarven( addletter_dwarven( word ), len - 1 ) );
   strcpy( word, buf );
   return word;
}
char *generate_drow( char *word, int len )
{
   char buf[MAX_STRING_LENGTH];
   if( len == 0 )
      return word;
   strcpy( buf, word );
   strcpy( buf, generate_drow( addletter_drow( word ), len - 1 ) );
   strcpy( word, buf );
   return word;
}
char *random_name( char *word, int lang )
{
   char buf[MAX_STRING_LENGTH];
   strcpy( word, "" );
   if( lang == LANG_KALORESE )
      strcpy( buf, generate( word, 3 + number_range( 0, 6 ) ) );
   if( lang == LANG_ELVEN )
      strcpy( buf, generate_elven( word, 3 + number_range( 0, 6 ) ) );
   if( lang == LANG_DWARVEN )
      strcpy( buf, generate_dwarven( word, 3 + number_range( 0, 6 ) ) );
   if( lang == LANG_DRAGON )
      strcpy( buf, generate_elven( word, 6 + number_range( 0, 6 ) ) );
   if( lang == LANG_DROW )
      strcpy( buf, generate_drow( word, 3 + number_range( 0, 6 ) ) );
   strcpy( word, capitalize( buf ) );
   return word;
}


