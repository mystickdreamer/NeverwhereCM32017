/* Map generator designed by Dex for the Metaverse Project.
*
* This program builds a 10 by 10 zone depending on the letters
* put in the static map. It assigns sector types, exits to all
* adject rooms in the same zone and gives a brief room name.
* The room description is left empty.
* 
* No credits, trademarks or contacting required, but do you
* suggestions, bugs or need help with this program, contact
* dex@meta.mudservices.com
*
* The origin of this code is meta.mudservices.com
*
* Modified to work as a command on Rogue Winds by Scion 8/19/1999
*/  
   
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"
int makezone( CHAR_DATA * ch, char note[MAX_STRING_LENGTH] ) 
{
   FILE * fp;
   char buf[100];
   char buf2[100];
   OBJ_DATA * obj;
   static int pos;
   char *main_design;
   int range, min_range, max_range, count;
   
      /*
       * The map, Each letter sets the room type in the sections
       * below 
       */ 
   char design01[11] = "FFFFFffrff";
   char design02[11] = "FFFFfffrff";
   char design03[11] = "FFFffffrff";
   char design04[11] = "FFFffffrwf";
   char design05[11] = "Ffffffffrf";
   char design06[11] = "fffiifffrr";
   char design07[11] = "fffiiifffr";
   char design08[11] = "fffffffffr";
   char design09[11] = "ffffmmfffr";
   char design10[11] = "fffmmmmffr";
   EXTRA_DESCR_DATA * ed;
   char *map;
   int i = 0;
   int a = 0;
   bool rooms = FALSE;
   if( IS_NPC( ch ) )
   {
      send_to_char( "Hell no, mobs are NOT allowed to build zones!\r\n", ch );
      return FALSE;
   }
   if( ( obj = ch->main_hand ) != NULL )
   {
      for( ed = obj->first_extradesc; ed; ed = ed->next )
      {
         if( is_name( "_text_", ed->keyword ) )
         {
            rooms = TRUE;
            map = str_dup( strip_crn( ed->description ) );
            if( strlen( map ) < 100 )
            {
               send_to_char( "Your map has less than 100 rooms on it.\r\n", ch );
               return FALSE;
            }
            i = 0;
            while( i < 10 )
            {
               design01[i] = map[a];
               design01[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design02[i] = map[a];
               design02[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design03[i] = map[a];
               design03[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design04[i] = map[a];
               design04[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design05[i] = map[a];
               design05[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design06[i] = map[a];
               design06[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design07[i] = map[a];
               design07[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design08[i] = map[a];
               design08[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design09[i] = map[a];
               design09[i + 1] = '\0';
               i++;
               a++;
            }
            i = 0;
            while( i < 10 )
            {
               design10[i] = map[a];
               design10[i + 1] = '\0';
               i++;
               a++;
            }
         }
      }
   }
   else
   {
      send_to_char( "You are not holding a note.\r\n", ch );
      return FALSE;
   }
   if( rooms == FALSE )
   {
      send_to_char( "Could not read the rooms to build.\r\n", ch );
      return FALSE;
   }
   send_to_char( "Generating map...\r\n", ch );
   sprintf( buf, "%s\r\n", design01 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design02 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design03 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design04 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design05 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design06 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design07 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design08 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design09 );
   send_to_char( buf, ch );
   sprintf( buf, "%s\r\n", design10 );
   send_to_char( buf, ch );
   sprintf( buf, "%s's Wilderness", ch->name );  /* zone name */
   sprintf( buf2, "../building/%s.are", ch->name );  /* file name */
   max_range = ch->pcdata->r_range_lo + 100;   /* ending vnum +1 (always ends up as one less); */
   min_range = ch->pcdata->r_range_lo;   /* starting vnum */
   pos = 0;
   if( ( fp = fopen( buf2, "w" ) ) == NULL )
      
   {
      pos = 0;
      send_to_char( "Unable to generate new zone.\r\n", ch );
   }
   
   else
      
   {
      fprintf( fp, "#AREA   %s~\n", buf );
      fprintf( fp, "#VERSION 1\n" );
      fprintf( fp, "#AUTHOR %s~\n", ch->name );
      fprintf( fp, "#RANGES\n0 116 0 116\n$\n" );
      fprintf( fp, "\n#RESETMSG	Void~\n" );
      fprintf( fp, "#FLAGS\n0\n\n" );
      fprintf( fp, "#ECONOMY 1 1\n\n" );
      fprintf( fp, "#CLIMATE 1 2 3 4\n\n" );
      fprintf( fp, "#MOBILES\n#0\n\n" );
      fprintf( fp, "#OBJECTS\n#0\n\n" );
      fprintf( fp, "\n" );
      main_design = design01;
      count = 0;
      fprintf( fp, "#ROOMS\n" );
      for( range = min_range; range < max_range; range++ )
         
      {
         
            /*
             * counts through all of the static data above, switching
             * to a new line at every 10 counts.
             */ 
            switch ( count )
            
         {
            case 10:
               main_design = design10;
               pos = 0;
               break;
            case 20:
               main_design = design09;
               pos = 0;
               break;
            case 30:
               main_design = design08;
               pos = 0;
               break;
            case 40:
               main_design = design07;
               pos = 0;
               break;
            case 50:
               main_design = design06;
               pos = 0;
               break;
            case 60:
               main_design = design05;
               pos = 0;
               break;
            case 70:
               main_design = design04;
               pos = 0;
               break;
            case 80:
               main_design = design03;
               pos = 0;
               break;
            case 90:
               main_design = design02;
               pos = 0;
               break;
         }
         
            /*
             * Generates the room data, leaving description blank.
             * it assigns sector type, room name and a few related
             * flags 
             */ 
            switch ( main_design[pos] )
            
         {
            case 'i':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "a room~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 0\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'c':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "a street~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 1\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'f':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "a field~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 2\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'F':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "the forest~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 3\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'h':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "the hills~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 4\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'm':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "the mountains~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 5\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'r':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "a stream~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 6\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'w':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "deep water~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 7\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'U':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "underwater~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 8\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'a':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "the air~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 9\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'd':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "the desert~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 10\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'V':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "a void~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 11\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'o':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "the ocean floor~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 12\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            case 'u':
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "a cave~\n" );
               fprintf( fp, "~\n" );
               fprintf( fp, "0 0 13\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
            default:
               fprintf( fp, "#%d\n", range );
               fprintf( fp, "Nowhere~\n" );
               fprintf( fp, "An incorrect sector type was defined for this room.~\n" );
               fprintf( fp, "0 0 1\n" );
               fprintf( fp, "0 0 0 0 0 0\n" );
               break;
         }
         
            /*
             * Exit generation.
             */ 
            if( range < max_range - 10 )
         {
            fprintf( fp, "D0\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "0 -1 %d\n", range + 10 );
         }
         if( pos < 9 )
            
         {
            fprintf( fp, "D1\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "0 -1 %d\n", range + 1 );
         }
         if( range > min_range + 9 )
            
         {
            fprintf( fp, "D2\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "0 -1 %d\n", range - 10 );
         }
         if( pos > 0 )
            
         {
            fprintf( fp, "D3\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "~\n" );
            fprintf( fp, "0 -1 %d\n", range - 1 );
         }
         fprintf( fp, "S\n" );
         pos++;
         count++;
      }
      fprintf( fp, "#0\n" );
      fprintf( fp, "#$\n" );
      fclose( fp );
   }
   return TRUE;
}


