/*
 *      Cities
 *      =======
 *	Copyright 2001 Heather Dunn
 *	City construction, management, and update code
 *
 *      You may not use or distribute any of this code without the
 *      explicit permission of the code maintainer, Heather Dunn (Keolah)
 */  
   
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
#include <strings.h>
#endif /*  */
#include <time.h>
#include "mud.h"
   CITY_DATA * first_city;
CITY_DATA * last_city;

/****************************************************************************
This is all gonna be very complicated when it's finished. The player or
clan will be able to manage the city, build buildings for it, manage
trade, war, hire people to do various jobs in the city, manage food and
population. They will build buildings perhaps with a build command, which
will begin the construction of the building to be done after several
mud-hours perhaps, and need to pay the workers. They will need farms to
grow crops to produce food to feed the population, places for the people
to live and work. Each shop will produce different resources and trade.
They will be able to hire guards to protect the city, perhaps build city
walls and roads, clear forests for wood, mine mountains for ores and such.
Perhaps also allow researching of different technologies and magicks to
benefit the city in different ways and allow construction of different
things and hiring of different types of units. A demolish command to
destroy an existing building restoring it to an empty field. Perhaps
buildings should be able to take damage from spells and such as well.
Needless to say, it is all very complicated and ambitious, and it may be
quite some time before it is fully operational.
***************************************************************************/ 
int busy_builders( CITY_DATA * city )
{
   int builders = 0;
   CONSTRUCT_DATA * bldg;
   bldg = city->first_building;
   while( bldg )
   {
      builders += bldg->builders;
      bldg = bldg->next;
   }
   return builders;
}


/* Returns the total population of the city */ 
int population( CITY_DATA * city )
{
   int pop = 0;
   pop = city->peasants;
   pop += city->farmers;
   pop += city->builders;
   pop += city->merchants;
   pop += city->miners;
   pop += busy_builders( city );
   return pop;
}


/* Determine how much gold the city makes */ 
int gold_income( CITY_DATA * city )
{
   int gold = 0;
   if( city->shops > city->merchants )
   {
      gold += city->merchants * 10;
   }
   else
   {
      gold += city->shops * 10;
      if( city->merchants * 10 > city->peasants )
      {
         gold += city->peasants;
      }
      else
      {
         gold += city->merchants * 10;
      }
   }
   gold -= city->builders * 2;
   gold -= city->miners * 4;
   gold -= busy_builders( city ) * 5;
   gold -= city->farmers * 2;
   return gold;
}
int food_income( CITY_DATA * city )
{
   int food = 0;
   food -= ( int )city->food / 100;
   food -= ( int )population( city ) / 10;
   food -= ( int )city->merchants;
   if( city->farms > city->farmers )
      food += city->farmers * 10;
   
   else
      food += city->farms * 10;
   return food;
}
int wood_income( CITY_DATA * city )
{
   int wood = 0;
   if( city->forests > city->builders )
      wood = city->builders * 10;
   
   else
      wood = city->forests * 10;
   return wood;
}
int iron_income( CITY_DATA * city )
{
   int iron = 0;
   if( city->mines > city->miners )
      iron = city->miners;
   
   else
      iron = city->mines;
   return iron;
}


/* This will update all cities, produce income, finish buildings, etc */ 
void city_update( void )
{
   CITY_DATA * city;
   CONSTRUCT_DATA * bldg;
   city = first_city;
   while( city )
   {
      city->gold += gold_income( city );
      city->food += food_income( city );
      city->wood += wood_income( city );
      city->iron += iron_income( city );
      if( city->gold < 0 )
      {
         if( city->miners )
         {
            city->peasants += city->miners;
            city->miners = 0;
         }
         if( city->builders )
         {
            city->peasants += city->builders;
            city->builders = 0;
         }
         city->gold = 0;
         while( city->first_building )
         {
            city->peasants += city->first_building->builders;
            UNLINK( city->first_building, city->first_building, city->last_building, next, prev );
            DISPOSE( city->first_building );
         }
      }
      if( city->food < 0 )
      {
         city->food = 0;
         city->peasants -= URANGE( 1, ( int )city->peasants / 5, city->peasants );
         if( city->merchants )
            city->merchants -= URANGE( 1, ( int )city->merchants / 20, city->merchants );
         if( city->builders )
            city->builders -= URANGE( 1, ( int )city->builders / 10, city->builders );
         if( city->miners )
            city->miners -= URANGE( 1, ( int )city->miners / 10, city->miners );
         if( city->farms < city->farmers )
            city->farmers -= URANGE( 1, ( int )city->farmers / 5, city->farmers );
         while( city->first_building )
         {
            city->peasants += city->first_building->builders;
            UNLINK( city->first_building, city->first_building, city->last_building, next, prev );
            DISPOSE( city->first_building );
         }
      }
      else if( city->houses * 100 > population( city ) )
         city->peasants += ( int )population( city ) / 100 + 1;
      bldg = city->first_building;
      while( bldg )
      {
         if( city->wood < bldg->builders )
         {
            if( bldg->done + city->wood > bldg->needed )
            {
               city->wood -= bldg->needed - bldg->done;
               bldg->done = bldg->needed;
            }
            else
            {
               bldg->done += city->wood;
               city->wood = 0;
            }
         }
         else
         {
            if( bldg->done + bldg->builders > bldg->needed )
            {
               city->wood -= bldg->needed - bldg->done;
               bldg->done = bldg->needed;
            }
            else
            {
               bldg->done += bldg->builders;
               city->wood -= bldg->builders;
            }
         }
         
            /*
             * Now check for finished buildings 
             */ 
            if( bldg->done == bldg->needed )
         {
         }
         bldg = bldg->next;
      }
      city = city->next;
   }
}
CITY_DATA * find_city( char *name )
{
   CITY_DATA * city;
   city = first_city;
   if( !city )
   {
      bug( "No cities exist in find_city.", 0 );
      return NULL;
   }
   while( city )
   {
      if( !str_cmp( city->name, name ) )
         return city;
      city = city->next;
   }
   return NULL;
}


/* View the list of cities, stats on a specific city, etc */ 
void do_city( CHAR_DATA * ch, char *argument )
{
   CITY_DATA * city;
   CITY_DATA * new_city;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   argument = one_argument( argument, arg4 );
   if( !str_cmp( arg1, "list" ) )
   {
      int i = 0;
      city = first_city;
      if( !city )
      {
         send_to_char( "There are no cities in existance.\n\r", ch );
         return;
      }
      send_to_char( "Cities in the universe:\n\r", ch );
      while( city )
      {
         i++;
         ch_printf( ch, "%d: %s\n\r", i, capitalize( city->name ) );
         city = city->next;
      }
      return;
   }
   if( !strcmp( arg1, "show" ) )
   {
      if( !( city = find_city( arg2 ) ) )
      {
         send_to_char( "There is no city with that name.\r\n", ch );
         return;
      }
      ch_printf( ch, "City %s:\r\n", capitalize( city->name ) );
      ch_printf( ch, "Population: %d\n\r", population( city ) );
      ch_printf( ch, "Food: %d (%d/day) Gold: %d (%d/day) Wood: %d (%d/day) Iron: %d (%d/day)\n\r", city->food,
                  food_income( city ), city->gold, gold_income( city ), city->wood, wood_income( city ), city->iron,
                  iron_income( city ) );
      ch_printf( ch, "Peasants: %d  Builders: %d  Farmers: %d  Merchants: %d  Miners: %d\n\r", city->peasants,
                  city->builders, city->farmers, city->merchants, city->miners );
      ch_printf( ch, "Houses: %d  Farms: %d  Shops: %d  Forests: %d  Mines: %d\n\r", city->houses, city->farms,
                  city->shops, city->forests, city->mines );
      return;
   }
   if( !strcmp( arg1, "set" ) )
   {
      city = find_city( arg2 );
      if( !city )
      {
         send_to_char( "Unknown city name.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "name" ) )
      {
         city->name = str_dup( arg4 );
         send_to_char( "city name set.\r\n", ch );
         return;
      }
      if( !str_cmp( arg3, "peasants" ) )
      {
         city->peasants = atoi( arg4 );
         send_to_char( "city peasants set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "gold" ) )
      {
         city->gold = atoi( arg4 );
         send_to_char( "city gold set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "food" ) )
      {
         city->food = atoi( arg4 );
         send_to_char( "city food set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "wood" ) )
      {
         city->wood = atoi( arg4 );
         send_to_char( "city wood set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "iron" ) )
      {
         city->iron = atoi( arg4 );
         send_to_char( "city iron set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "houses" ) )
      {
         city->houses = atoi( arg4 );
         send_to_char( "city houses set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "farms" ) )
      {
         city->farms = atoi( arg4 );
         send_to_char( "city farms set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "forests" ) )
      {
         city->forests = atoi( arg4 );
         send_to_char( "city forests set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "shops" ) )
      {
         city->shops = atoi( arg4 );
         send_to_char( "city shops set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "mines" ) )
      {
         city->mines = atoi( arg4 );
         send_to_char( "city mines set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "builders" ) )
      {
         city->builders = atoi( arg4 );
         send_to_char( "city builders set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "farmers" ) )
      {
         city->farmers = atoi( arg4 );
         send_to_char( "city farmers set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "merchants" ) )
      {
         city->merchants = atoi( arg4 );
         send_to_char( "city merchants set.\n\r", ch );
         return;
      }
      if( !str_cmp( arg3, "miners" ) )
      {
         city->miners = atoi( arg4 );
         send_to_char( "city miners set.\n\r", ch );
         return;
      }
      send_to_char( "Invalid field.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "create" ) )
   {
      CREATE( new_city, CITY_DATA, 1 );
      if( !first_city )
      {
         first_city = new_city;
         last_city = new_city;
         new_city->next = NULL;
         new_city->prev = NULL;
      }
      else
      {
         last_city->next = new_city;
         new_city->prev = last_city;
         last_city = new_city;
         new_city->next = NULL;
      }
      city = new_city;
      if( arg2 )
         city->name = STRALLOC( arg2 );
      
      else
         city->name = STRALLOC( "newcity" );
      city->peasants = 0;
      city->food = 0;
      city->gold = 0;
      city->wood = 0;
      city->iron = 0;
      city->builders = 0;
      city->farmers = 0;
      city->merchants = 0;
      city->miners = 0;
      city->houses = 0;
      city->shops = 0;
      city->farms = 0;
      city->forests = 0;
      city->mines = 0;
      sprintf( buf, "city %s created.\r\n", city->name );
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "save" ) )
   {
      FILE * fp;
      if( ( fp = fopen( CITY_FILE, "w" ) ) == NULL )
      {
         bug( "Cannot open city file!", 0 );
         return;
      }
      for( city = first_city; city; city = city->next )
      {
         fprintf( fp, "\n#CITY\n" );
         fprintf( fp, "Name   %s~\n", city->name );
         fprintf( fp, "Builders	%d\n", city->builders );
         fprintf( fp, "Farmers	%d\n", city->farmers );
         fprintf( fp, "Farms	%d\n", city->farms );
         fprintf( fp, "Food	%d\n", city->food );
         fprintf( fp, "Forests	%d\n", city->forests );
         fprintf( fp, "Gold	%d\n", city->gold );
         fprintf( fp, "Houses	%d\n", city->houses );
         fprintf( fp, "Iron	%d\n", city->iron );
         fprintf( fp, "Merchants	%d\n", city->merchants );
         fprintf( fp, "Miners	%d\n", city->miners );
         fprintf( fp, "Mines	%d\n", city->mines );
         fprintf( fp, "Peasants	%d\n", city->peasants );
         fprintf( fp, "Shops	%d\n", city->shops );
         fprintf( fp, "Wood	%d\n", city->wood );
         if( city == last_city )
            break;
      }
      fprintf( fp, "#END\n" );
      fclose( fp );
      send_to_char( "cities saved.\r\n", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}


/* This command will be used to build walls, buildings, roads, etc */ 
void do_build( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CITY_DATA * city;
   if( !ch->in_room->area->city )
   {
      send_to_char( "There is no city here.\n\r", ch );
      return;
   }
   city = ch->in_room->area->city;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
   {
      send_to_char( "There is already a building here.\n\r", ch );
      send_to_char( "If you wish to build another type, DEMOLISH it first.\n\r", ch );
      return;
   }
   if( ch->in_room->curr_vegetation > 50 )
   {
      send_to_char( "There is too much vegetation here.\n\r", ch );
      return;
   }
   if( !str_cmp( arg1, "house" ) )
   {
      city->houses++;
      ch->in_room->sector_type = SECT_INSIDE;
      SET_BIT( ch->in_room->room_flags, ROOM_INDOORS );
      STRFREE( ch->in_room->name );
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            sprintf( buf, "quaint " );
            break;
         case 2:
            sprintf( buf, "small " );
            break;
         case 3:
            sprintf( buf, "cozy " );
            break;
         case 4:
            sprintf( buf, "filthy " );
            break;
         case 5:
            sprintf( buf, "comfortable " );
            break;
      }
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            strcat( buf, "house" );
            break;
         case 2:
            strcat( buf, "home" );
            break;
         case 3:
            strcat( buf, "cottage" );
            break;
         case 4:
            strcat( buf, "hovel" );
            break;
         case 5:
            strcat( buf, "shack" );
            break;
      }
      ch->in_room->name = STRALLOC( aoran( buf ) );
   }
   else if( !str_cmp( arg1, "shop" ) )
   {
      city->shops++;
      ch->in_room->sector_type = SECT_INSIDE;
      SET_BIT( ch->in_room->room_flags, ROOM_INDOORS );
      STRFREE( ch->in_room->name );
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            sprintf( buf, "well-tended " );
            break;
         case 2:
            sprintf( buf, "well-stocked " );
            break;
         case 3:
            sprintf( buf, "clean " );
            break;
         case 4:
            sprintf( buf, "general goods " );
            break;
         case 5:
            sprintf( buf, "clothing " );
            break;
      }
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            strcat( buf, "shop" );
            break;
         case 2:
            strcat( buf, "store" );
            break;
         case 3:
            strcat( buf, "merchant" );
            break;
         case 4:
            strcat( buf, "stall" );
            break;
         case 5:
            strcat( buf, "market" );
            break;
      }
      ch->in_room->name = STRALLOC( aoran( buf ) );
   }
   else if( !str_cmp( arg1, "mine" ) )
   {
      city->mines++;
      ch->in_room->sector_type = SECT_UNDERGROUND;
      ch->in_room->resources = 100;
      SET_BIT( ch->in_room->room_flags, ROOM_INDOORS );
      STRFREE( ch->in_room->name );
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            sprintf( buf, "dusty " );
            break;
         case 2:
            sprintf( buf, "dark " );
            break;
         case 3:
            sprintf( buf, "dank " );
            break;
         case 4:
            sprintf( buf, "musty " );
            break;
         case 5:
            sprintf( buf, "dirty " );
            break;
      }
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            strcat( buf, "mine" );
            break;
         case 2:
            strcat( buf, "mining tunnel" );
            break;
         case 3:
            strcat( buf, "mining cave" );
            break;
         case 4:
            strcat( buf, "mining cavern" );
            break;
         case 5:
            strcat( buf, "mineshaft" );
            break;
      }
      ch->in_room->name = STRALLOC( aoran( buf ) );
   }
   else if( !str_cmp( arg1, "road" ) )
   {
      ch->in_room->sector_type = SECT_CITY;
      STRFREE( ch->in_room->name );
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            sprintf( buf, "cobblestone " );
            break;
         case 2:
            sprintf( buf, "gravel " );
            break;
         case 3:
            sprintf( buf, "dirt " );
            break;
         case 4:
            sprintf( buf, "paved " );
            break;
         case 5:
            sprintf( buf, "well-trod " );
            break;
      }
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            strcat( buf, "road" );
            break;
         case 2:
            strcat( buf, "lane" );
            break;
         case 3:
            strcat( buf, "boulevard" );
            break;
         case 4:
            strcat( buf, "avenue" );
            break;
         case 5:
            strcat( buf, "street" );
            break;
      }
      ch->in_room->name = STRALLOC( aoran( buf ) );
   }
   else
   {
      send_to_char( "You can't build any such thing.\n\r", ch );
      return;
   }
   ch_printf( ch, "You build %s here.\n\r", aoran( arg1 ) );
}


/* To tear down a wall, building, road, etc */ 
void do_demolish( CHAR_DATA * ch, char *argument )
{
} 

/* Plant crops or whatnot to harvest food from */ 
void do_plant( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   CITY_DATA * city;
   char buf[MAX_STRING_LENGTH];
   if( !ch->in_room->area->city )
   {
      send_to_char( "There is no city here.\n\r", ch );
      return;
   }
   city = ch->in_room->area->city;
   argument = one_argument( argument, arg1 );
   if( !str_cmp( arg1, "forest" ) )
   {
      ch->in_room->sector_type = SECT_FOREST;
      city->forests++;
      STRFREE( ch->in_room->name );
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            sprintf( buf, "pine " );
            break;
         case 2:
            sprintf( buf, "drenak " );
            break;
         case 3:
            sprintf( buf, "lern " );
            break;
         case 4:
            sprintf( buf, "cherry " );
            break;
         case 5:
            sprintf( buf, "maple " );
            break;
      }
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            strcat( buf, "forest" );
            break;
         case 2:
            strcat( buf, "grove" );
            break;
         case 3:
            strcat( buf, "glade" );
            break;
         case 4:
            strcat( buf, "orchard" );
            break;
         case 5:
            strcat( buf, "stand" );
            break;
      }
      ch->in_room->name = STRALLOC( aoran( buf ) );
   }
   else if( !str_cmp( arg1, "farm" ) )
   {
      ch->in_room->sector_type = SECT_FIELD;
      city->farms++;
      STRFREE( ch->in_room->name );
      switch ( number_range( 1, 5 ) )
      {
         case 1:
            sprintf( buf, "zim'adar " );
            break;
         case 2:
            sprintf( buf, "calluv " );
            break;
         case 3:
            sprintf( buf, "corn " );
            break;
         case 4:
            sprintf( buf, "jazma " );
            break;
         case 5:
            sprintf( buf, "worfberry " );
            break;
      }
      switch ( number_range( 1, 2 ) )
      {
         case 1:
            strcat( buf, "farm" );
            break;
         case 2:
            strcat( buf, "field" );
            break;
      }
      ch->in_room->name = STRALLOC( aoran( buf ) );
   }
   else
   {
      send_to_char( "You can't plant any such thing.\n\r", ch );
      return;
   }
   ch_printf( ch, "You plant %s here.\n\r", aoran( arg1 ) );
   ch->in_room->vegetation = 100;
   ch->in_room->curr_vegetation = 100;
}


/* Allows you to hire peasants to work as farmers, builders, etc */ 
void do_hire( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CITY_DATA * city;
   if( !ch->in_room->area->city )
   {
      send_to_char( "There is no city here.\n\r", ch );
      return;
   }
   city = ch->in_room->area->city;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( city->peasants < atoi( arg1 ) )
   {
      send_to_char( "There are not enough peasants here to fill your demands.\n\r", ch );
      return;
   }
   if( !str_cmp( arg2, "farmers" ) )
   {
      if( ( city->farmers + atoi( arg1 ) ) < 0 )
      {
         send_to_char( "You do not employ that many farmers to fire.\n\r", ch );
         return;
      }
      city->farmers += atoi( arg1 );
   }
   else if( !str_cmp( arg2, "builders" ) )
   {
      if( ( city->builders + atoi( arg1 ) ) < 0 )
      {
         send_to_char( "You do not employ that many builders to fire.\n\r", ch );
         return;
      }
      city->builders += atoi( arg1 );
   }
   else if( !str_cmp( arg2, "miners" ) )
   {
      if( ( city->miners + atoi( arg1 ) ) < 0 )
      {
         send_to_char( "You do not employ that many miners to fire.\n\r", ch );
         return;
      }
      city->miners += atoi( arg1 );
   }
   else if( !str_cmp( arg2, "merchants" ) )
   {
      if( ( city->merchants + atoi( arg1 ) ) < 0 )
      {
         send_to_char( "You do not employ that many merchants to fire.\n\r", ch );
         return;
      }
      city->merchants += atoi( arg1 );
   }
   else
   {
      send_to_char( "You can't hire any such thing.\n\r", ch );
      return;
   }
   city->peasants -= atoi( arg1 );
   ch_printf( ch, "You %s %d %s.\n\r", atoi( arg1 ) > 0 ? "hire" : "fire", abs( atoi( arg1 ) ), arg2 );
}


