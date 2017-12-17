/*
 * Gladiator Arenas
 *
 * Written by Keolah for Rogue Winds
 *
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
   ARENA_DATA * first_arena;
ARENA_DATA * last_arena;
extern char *munch_colors( char *word );

/**************************************************************************
Arenas, with a twist. Rather than the actual players duking it out, they
hire gladiators to fight for them, purely with physical combat, not magic.
Each gladiator will have its own name, race, stats, and such, when this is
complete. I plan to also allow teams, so that a team might be sponsered by
a clan or such, and gladiators on the same team will help each other.
**************************************************************************/ 
void save_arenas( void )
{
   ARENA_DATA * arena;
   ARENA_MEMBER * member;
   BET_DATA * bet;
   FILE * fp;
   if( ( fp = fopen( ARENA_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open arena file!", 0 );
      return;
   }
   for( arena = first_arena; arena; arena = arena->next )
   {
      fprintf( fp, "\n#ARENA\n" );
      fprintf( fp, "Name   %s~\n", arena->name );
      fprintf( fp, "Phase	%d\n", arena->phase );
      fprintf( fp, "Vnum	%d\n", arena->vnum );
      fprintf( fp, "Winner	%s~\n", arena->winner );
      fprintf( fp, "Flags	%d\n", arena->flags );
      for( member = arena->first_member; member; member = member->next )
      {
         fprintf( fp, "\n#MEMBER\n" );
         fprintf( fp, "Member_name   %s~\n", member->name );
         fprintf( fp, "Owner   %s~\n", member->owner );
         fprintf( fp, "HP		%d\n", member->hp );
         fprintf( fp, "Kills	%d\n", member->kills );
         fprintf( fp, "Wins	%d\n", member->wins );
         fprintf( fp, "Played	%d\n", member->played );
         fprintf( fp, "Str	%d\n", member->str );
         fprintf( fp, "Con	%d\n", member->con );
         fprintf( fp, "Dex	%d\n", member->dex );
         fprintf( fp, "Spd	%d\n", member->spd );
         fprintf( fp, "Skill	%d\n", member->skill );
         fprintf( fp, "Auto	%d\n", member->flags );
         fprintf( fp, "Team	%d\n", member->team );
         fprintf( fp, "Age	%d\n", member->age );
         fprintf( fp, "Score	%d\n", member->score );
         fprintf( fp, "Total	%d\n", member->total );
         for( bet = member->first_bet; bet; bet = bet->next )
         {
            fprintf( fp, "\n#BET\n" );
            fprintf( fp, "Better   %s~\n", bet->better );
            fprintf( fp, "Wager		%d\n", bet->wager );
            fprintf( fp, "Odds	%d\n", bet->odds );
            fprintf( fp, "Winnings	%ld\n", bet->winnings );
         }
      }
      if( arena == last_arena )
         break;
   }
   fprintf( fp, "#END\n" );
   fclose( fp );
}


/* Echo a message to all players in an arena room */ 
void arena_echo( ROOM_INDEX_DATA * room, char *argument )
{
   CHAR_DATA * ch;
   if( !room )
      return;
   if( !room->first_person )
      return;
   for( ch = room->first_person; ch; ch = ch->next_in_room )
   {
      send_to_char( argument, ch );
   }
}
void random_message( ROOM_INDEX_DATA * room )
{
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 20 ) )
   {
      default:
         break;
      case 1:
         arena_echo( room, "The gladiators are circling each other.\n\r" );
         break;
      case 2:
         arena_echo( room, "The crowd lets out a bloodthirsty cheer.\n\r" );
         break;
      case 3:
         arena_echo( room, "A boy approaches you and tries to sell you a hot dog.\n\r" );
         break;
      case 4:
         arena_echo( room, "Somebody near you munches on popcorn.\n\r" );
         break;
      case 5:
         arena_echo( room, "Somebody spills a beverage next to your seat.\n\r" );
         break;
      case 6:
         arena_echo( room, "The crowd starts chanting for their favorite.\n\r" );
         break;
      case 7:
         arena_echo( room, "The crowd jeers and taunts.\n\r" );
         break;
      case 8:
         arena_echo( room, "Someone slips and falls in the mud.\n\r" );
         break;
      case 9:
         arena_echo( room, "The crowd demands blood!\n\r" );
         break;
      case 10:
         arena_echo( room, "Somebody bets a hot dog on the winner.\n\r" );
         break;
      case 11:
         arena_echo( room, "The crowd boos and hisses.\n\r" );
         break;
      case 12:
         arena_echo( room, "The crowd applauds wildly.\n\r" );
         break;
      case 13:
         arena_echo( room, "The crowd throws fruit angrily.\n\r" );
         break;
      case 14:
         arena_echo( room, "The crowd throws popcorn excitedly.\n\r" );
         break;
      case 15:
         arena_echo( room, "Somebody stumbles past you on the way to the restroom.\n\r" );
         break;
   }
}
void bet_message( ROOM_INDEX_DATA * room )
{
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 10 ) )
   {
      default:
         break;
      case 1:
         arena_echo( room, "The crowd eagerly waits for the fight to commence.\n\r" );
         break;
      case 2:
         arena_echo( room, "'Place your bets! The arena will be starting soon.'\n\r" );
         break;
      case 3:
         arena_echo( room, "The gladiators restlessly wait for the arena to begin.\n\r" );
         break;
      case 4:
         arena_echo( room, "A boy approaches you and tries to sell you a hot dog.\n\r" );
         break;
      case 5:
         arena_echo( room, "A man with a mop stumbles past, wiping up a spilled beverage.\n\r" );
         break;
      case 6:
         arena_echo( room, "A janitor walks past you, picking up some trash.\n\r" );
         break;
   }
}
void start_message( ROOM_INDEX_DATA * room )
{
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 10 ) )
   {
      default:
         arena_echo( room, "The fight has begun.\n\r" );
         break;
      case 1:
         arena_echo( room, "A whistle blows signalling the start of the arena.\n\r" );
         break;
      case 2:
         arena_echo( room, "'Take your seats, ladies and gentlemen. The arena has begun!'\n\r" );
         break;
      case 3:
         arena_echo( room, "A gunshot echoes, and the fight begins.\n\r" );
         break;
      case 4:
         arena_echo( room, "The gladiators rush into the arena, commencing the fight.\n\r" );
         break;
   }
}
void win_message( ROOM_INDEX_DATA * room, ARENA_MEMBER * winner )
{
   char buf[MAX_STRING_LENGTH];
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 5 ) )
   {
      default:
         sprintf( buf, "%s is the winner!\n\r", winner->name );
         break;
      case 1:
         sprintf( buf, "%s does a victory dance!\n\r", winner->name );
         break;
      case 2:
         sprintf( buf, "'Congratulations %s, you are the winner!'\n\r", winner->name );
         break;
      case 3:
         sprintf( buf, "%s has won this arena!\n\r", winner->name );
         break;
      case 4:
         sprintf( buf, "%s wins the fight!\n\r", winner->name );
         break;
   }
   arena_echo( room, buf );
}
void draw_message( ROOM_INDEX_DATA * room )
{
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 10 ) )
   {
      default:
         arena_echo( room, "The arena is a draw.\n\r" );
         break;
      case 1:
         arena_echo( room, "No winner for this arena.\n\r" );
         break;
      case 2:
         arena_echo( room, "'Ladies and gentlemen, it is a draw.'\n\r" );
         break;
   }
}
void miss_message( ROOM_INDEX_DATA * room, ARENA_MEMBER * dodger, ARENA_MEMBER * attacker )
{
   char buf[MAX_STRING_LENGTH];
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 10 ) )
   {
      default:
         sprintf( buf, "%s misses %s.\n\r", attacker->name, dodger->name );
         break;
      case 1:
         sprintf( buf, "%s dodges %s's attack.\n\r", dodger->name, attacker->name );
         break;
      case 2:
         sprintf( buf, "%s blocks %s's attack.\n\r", dodger->name, attacker->name );
         break;
      case 3:
         sprintf( buf, "%s tumbles out of the way of %s's attack.\n\r", dodger->name, attacker->name );
         break;
      case 4:
         sprintf( buf, "%s ducks %s's high punch.\n\r", dodger->name, attacker->name );
         break;
      case 5:
         sprintf( buf, "%s jumps to avoid %s's trip.\n\r", dodger->name, attacker->name );
         break;
      case 6:
         sprintf( buf, "%s rolls out of the way of %s's strike.\n\r", dodger->name, attacker->name );
         break;
      case 7:
         sprintf( buf, "%s lightly sidesteps %s's attack.\n\r", dodger->name, attacker->name );
         break;
   }
   arena_echo( room, buf );
}
void hit_message( ROOM_INDEX_DATA * room, ARENA_MEMBER * victim, ARENA_MEMBER * attacker )
{
   char buf[MAX_STRING_LENGTH];
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 20 ) )
   {
      default:
         sprintf( buf, "%s strikes %s.\n\r", attacker->name, victim->name );
         break;
      case 1:
         sprintf( buf, "%s punches %s.\n\r", attacker->name, victim->name );
         break;
      case 2:
         sprintf( buf, "%s knees %s in the groin.\n\r", attacker->name, victim->name );
         break;
      case 3:
         sprintf( buf, "%s hits %s.\n\r", attacker->name, victim->name );
         break;
      case 4:
         sprintf( buf, "%s shoves %s into the mud.\n\r", attacker->name, victim->name );
         break;
      case 5:
         sprintf( buf, "%s kicks %s.\n\r", attacker->name, victim->name );
         break;
      case 6:
         sprintf( buf, "%s trips %s.\n\r", attacker->name, victim->name );
         break;
      case 7:
         sprintf( buf, "%s punches %s in the stomach.\n\r", attacker->name, victim->name );
         break;
      case 8:
         sprintf( buf, "%s punches %s in the nose.\n\r", attacker->name, victim->name );
         break;
      case 9:
         sprintf( buf, "%s attacks %s.\n\r", attacker->name, victim->name );
         break;
      case 10:
         sprintf( buf, "%s kicks %s in the butt.\n\r", attacker->name, victim->name );
         break;
      case 11:
         sprintf( buf, "%s elbows %s.\n\r", attacker->name, victim->name );
         break;
      case 12:
         sprintf( buf, "%s bitch slaps %s.\n\r", attacker->name, victim->name );
         break;
      case 13:
         sprintf( buf, "%s kicks %s in the crotch!\n\r", attacker->name, victim->name );
         break;
      case 14:
         sprintf( buf, "%s slams a fist into %s's gut.\n\r", attacker->name, victim->name );
         break;
      case 15:
         sprintf( buf, "%s hurls %s into a wall.\n\r", attacker->name, victim->name );
         break;
      case 16:
         sprintf( buf, "%s flings a glob of mud at %s.\n\r", attacker->name, victim->name );
         break;
      case 17:
         sprintf( buf, "%s uppercuts %s.\n\r", attacker->name, victim->name );
         break;
   }
   arena_echo( room, buf );
}
void ko_message( ROOM_INDEX_DATA * room, ARENA_MEMBER * victim, ARENA_MEMBER * attacker )
{
   char buf[MAX_STRING_LENGTH];
   if( !room )
      return;
   if( !room->first_person )
      return;
   switch ( number_range( 0, 10 ) )
   {
      default:
         sprintf( buf, "%s knocks %s out!\n\r", attacker->name, victim->name );
         break;
      case 1:
         sprintf( buf, "%s has knocked %s out!\n\r", attacker->name, victim->name );
         break;
      case 2:
         sprintf( buf, "%s pins %s to the ground!\n\r", attacker->name, victim->name );
         break;
      case 3:
         sprintf( buf, "%s knocks %s unconscious!\n\r", attacker->name, victim->name );
         break;
      case 4:
         sprintf( buf, "%s knocks %s out of the arena!\n\r", attacker->name, victim->name );
         break;
      case 5:
         sprintf( buf, "%s collapses. The crowd cheers %s on!\n\r", victim->name, attacker->name );
         break;
      case 6:
         sprintf( buf, "%s is knocked out! The crowd begins chanting, '%s! %s! %s!'.\n\r", victim->name, attacker->name,
                   attacker->name, attacker->name );
         break;
      case 7:
         sprintf( buf, "%s is knocked out! Somebody mumbles, 'Damn that %s, made me lose another bet.'\n\r", victim->name,
                   attacker->name );
         break;
      case 8:
         sprintf( buf, "%s passes out. The crowd applauds %s!\n\r", victim->name, attacker->name );
         break;
   }
   arena_echo( room, buf );
}


/* Figure out the odds of a given gladiator winning */ 
int get_odds( ARENA_MEMBER * calc )
{
   ARENA_MEMBER * member;
   int count, diff, odds, team, opp;
   if( calc == NULL )
      return 0;
   count = diff = team = opp = 0;
   for( member = calc->arena->first_member; member; member = member->next )
   {
      if( IS_SET( member->flags, GLAD_WAIT ) )
         continue;
      diff += member->played;
      if( member->team != 0 && member->team == calc->team )
         team += member->played;
      
      else
         opp++;
      count++;
   }
   
      /*
       * Odds for a team arena 
       */ 
      if( IS_SET( calc->arena->flags, ARENA_TEAM )  &&calc->team != 0 )
   {
      if( opp == 0 )
         return 0;
      if( diff == 0 )
         return 2;
      diff = ( int )diff / opp;
      odds = opp;
      
         /*
          * Calculate the chance of member winning based off wins 
          */ 
         if( team < diff - diff / 2 )
         odds += count * 2;
      
      else if( team < diff - diff / 5 )
         odds += count;
      
      else if( team < diff - diff / 10 )
         odds += count / 3;
      
      else if( team > diff + diff / 2 )
         odds = 2;
      
      else if( team > diff + diff / 5 )
         odds -= count / 3;
      
      else if( team > diff + diff / 10 )
         odds -= count / 5;
      return UMAX( 1, odds );
   }
   
      /*
       * Odds for an FFA 
       */ 
      if( count == 0 )
      return 0;
   if( diff == 0 )
      return 2;
   diff = ( int )diff / count;
   odds = count;
   
      /*
       * Calculate the chance of member winning based off wins 
       */ 
      if( calc->played < diff - diff / 2 )
      odds += count * 2;
   
   else if( calc->played < diff - diff / 5 )
      odds += count;
   
   else if( calc->played < diff - diff / 10 )
      odds += count / 3;
   
   else if( calc->played > diff + diff / 2 )
      odds = 2;
   
   else if( calc->played > diff + diff / 5 )
      odds -= count / 3;
   
   else if( calc->played > diff + diff / 10 )
      odds -= count / 5;
   return UMAX( 1, odds );
}


/* This will update all arenas, complete fights, and announce results */ 
void arena_update( void )
{
   ARENA_DATA * arena;
   ARENA_MEMBER * member;
   ARENA_MEMBER * winner;
   BET_DATA * bet;
   bool found;
   int count, i, num, team;
   char buf[MAX_STRING_LENGTH];
   arena = first_arena;
   while( arena )
   {
      if( IS_SET( arena->flags, ARENA_HALT )  ||!arena->first_member  ||arena->first_member == arena->last_member )
      {
         arena = arena->next;
         continue;
      }
      
         /*
          * Team check - prevent free wins if only one team present 
          */ 
         if( IS_SET( arena->flags, ARENA_TEAM ) )
      {
         team = 0;
         for( member = arena->first_member; member; member = member->next )
         {
            if( IS_SET( member->flags, GLAD_WAIT ) )
               continue;
            if( member->team == 0 )
            {
               team = 0;
               break;
            }
            if( team != 0 && team != member->team )
            {
               team = 0;
               break;
            }
            team = member->team;
         }
         if( team != 0 )
         {
            arena = arena->next;
            continue;
         }
      }
      
         /*
          * The gladiators fight! 
          */ 
         if( arena->phase == -1 )
      {
         start_message( arena->room );
         
            /*
             * Recalculate all odds to ensure accuracy 
             */ 
            for( member = arena->first_member; member; member = member->next )
         {
            member->score = 0;
            if( !member->first_bet )
               continue;
            if( IS_SET( member->flags, GLAD_WAIT ) )
               continue;
            i = get_odds( member );
            for( bet = member->first_bet; bet; bet = bet->next )
               bet->odds = i;
         }
         arena->phase = 1;
      }
      else
         arena->phase++;
      if( arena->phase < 0 )
      {
         bet_message( arena->room );
         arena = arena->next;
         continue;
      }
      found = FALSE;
      random_message( arena->room );
      count = 0;
      for( member = arena->first_member; member; member = member->next )
      {
         if( IS_SET( member->flags, GLAD_WAIT ) )
            continue;
         member->age++;
         if( member->hp <= 0 )
            continue;
         member->score++;
         count++;
      }
      
         /*
          * Team check 
          */ 
         if( IS_SET( arena->flags, ARENA_TEAM ) )
      {
         team = 0;
         for( member = arena->first_member; member; member = member->next )
         {
            if( member->hp <= 0 )
               continue;
            if( IS_SET( member->flags, GLAD_WAIT ) )
               continue;
            if( member->team == 0 )
            {
               team = 0;
               break;
            }
            if( team != 0 && team != member->team )
            {
               team = 0;
               break;
            }
            team = member->team;
         }
         if( team != 0 )
            arena->phase = 0;
      }
      if( count <= 1 )
         arena->phase = 0;
      
      else
      {
         i = 0;
         num = number_range( 1, count );
         if( IS_SET( arena->flags, ARENA_DEBUG ) )
            bug( "DEBUG: Valid Targets: %d  Attacker: %d", count, num );
         for( member = arena->first_member; member; member = member->next )
         {
            if( member->hp <= 0 )
               continue;
            if( IS_SET( member->flags, GLAD_WAIT ) )
               continue;
            i++;
            if( i != num )
               continue;
            i = count = 0;
            for( winner = arena->first_member; winner; winner = winner->next )
            {
               if( winner->hp <= 0 )
                  continue;
               if( IS_SET( winner->flags, GLAD_WAIT ) )
                  continue;
               if( winner == member )
                  continue;
               if( IS_SET( arena->flags, ARENA_TEAM )  &&winner->team != 0  &&member->team == winner->team )
                  continue;
               count++;
            }
            num = number_range( 1, count );
            if( IS_SET( arena->flags, ARENA_DEBUG ) )
               bug( "DEBUG: Valid Targets: %d  Defender: %d", count, num );
            for( winner = arena->first_member; winner; winner = winner->next )
            {
               if( winner->hp <= 0 )
                  continue;
               if( winner == member )
                  continue;
               if( IS_SET( winner->flags, GLAD_WAIT ) )
                  continue;
               if( IS_SET( arena->flags, ARENA_TEAM )  &&winner->team != 0  &&member->team == winner->team )
                  continue;
               i++;
               if( i != num )
                  continue;
               if( number_range( 0, member->spd )  >number_range( 0, winner->spd ) )
               {
                  if( number_range( 0, winner->dex )  >number_range( 0, member->dex ) )
                  {
                     num = number_range( 1, winner->str );
                     member->hp -= num;
                     winner->score += num;
                     hit_message( arena->room, member, winner );
                  }
                  else
                  {
                     miss_message( arena->room, member, winner );
                  }
                  if( member->hp <= 0 )
                  {
                     winner->kills++;
                     winner->score += member->played;
                     ko_message( arena->room, member, winner );
                  }
                  else
                  {
                     if( number_range( 0, member->dex )  >number_range( 0, winner->dex ) )
                     {
                        num = number_range( 1, member->str );
                        winner->hp -= num;
                        member->score += num;
                        hit_message( arena->room, winner, member );
                     }
                     else
                     {
                        miss_message( arena->room, winner, member );
                     }
                     if( winner->hp <= 0 )
                     {
                        member->kills++;
                        member->score += winner->played;
                        ko_message( arena->room, winner, member );
                     }
                  }
               }
               else
               {
                  if( number_range( 0, member->dex )  >number_range( 0, winner->dex ) )
                  {
                     num = number_range( 1, member->str );
                     winner->hp -= num;
                     member->score += num;
                     hit_message( arena->room, winner, member );
                  }
                  else
                  {
                     miss_message( arena->room, winner, member );
                  }
                  if( winner->hp <= 0 )
                  {
                     member->kills++;
                     member->score += winner->played;
                     ko_message( arena->room, winner, member );
                  }
                  else
                  {
                     if( number_range( 0, winner->dex )  >number_range( 0, member->dex ) )
                     {
                        num = number_range( 1, winner->str );
                        member->hp -= num;
                        winner->score += num;
                        hit_message( arena->room, member, winner );
                     }
                     else
                     {
                        miss_message( arena->room, member, winner );
                     }
                     if( member->hp <= 0 )
                     {
                        winner->kills++;
                        winner->score += member->played;
                        ko_message( arena->room, member, winner );
                     }
                  }
               }
               break;
            }
         }
      }
      
         /*
          * Arena is over 
          */ 
         if( arena->phase == 0 )
      {
         winner = NULL;
         arena->phase = -10;
         if( arena->first_member )
         {
            
               /*
                * Find out who won 
                */ 
               
               /*
                * Team check 
                */ 
               if( IS_SET( arena->flags, ARENA_TEAM ) )
            {
               team = 0;
               for( member = arena->first_member; member; member = member->next )
               {
                  if( member->hp <= 0 )
                     continue;
                  if( IS_SET( member->flags, GLAD_WAIT ) )
                     continue;
                  if( member->team == 0 )
                  {
                     winner = member;
                     break;
                  }
                  team = member->team;
                  break;
               }
            }
            else
            {
               
                  /*
                   * FFA check 
                   */ 
                  for( member = arena->first_member; member; member = member->next )
               {
                  if( IS_SET( member->flags, GLAD_WAIT ) )
                     continue;
                  if( member->hp <= 0 )
                     continue;
                  winner = member;
               }
            }
            
               /*
                * Announce the winner 
                */ 
               if( winner )
            {
               win_message( arena->room, winner );
               winner->wins++;
               winner->score += ( get_odds( winner ) - 1 ) * 100;
               STRFREE( arena->winner );
               arena->winner = STRALLOC( winner->name );
               
                  /*
                   * Add winnings for the winner 
                   */ 
                  for( bet = winner->first_bet; bet; bet = bet->next )
               {
                  if( bet->odds == 1 )
                     bet->winnings = 1;
                  
                  else if( !bet->winnings )
                     bet->winnings = bet->wager * bet->odds;
                  
                  else
                     bet->winnings += bet->winnings * bet->odds;
               }
            }
            else if( team )
            {
               STRFREE( arena->winner );
               sprintf( buf, "Team %d", team );
               arena->winner = STRALLOC( buf );
               sprintf( buf, "Team %d has won this arena!\n\r", team );
               arena_echo( arena->room, buf );
               
                  /*
                   * Add winnings for the winning team 
                   */ 
                  for( member = arena->first_member; member; member = member->next )
               {
                  if( IS_SET( member->flags, GLAD_WAIT ) )
                     continue;
                  if( member->team != team )
                     continue;
                  member->wins++;
                  member->score += ( get_odds( member ) - 1 ) * 100;
                  for( bet = member->first_bet; bet; bet = bet->next )
                     if( !bet->winnings )
                        bet->winnings = bet->wager * bet->odds;
                  
                     else
                        bet->winnings += bet->winnings * bet->odds;
               }
            }
            else
            {
               draw_message( arena->room );
            }
            
               /*
                * Reset their hp after the arena is over 
                */ 
               for( member = arena->first_member; member; member = member = winner )
            {
               winner = member->next;
               member->hp = member->con;
               if( !IS_SET( member->flags, GLAD_WAIT ) )
               {
                  member->played++;
                  
                     /*
                      * Gladiators degenerate as they get old 
                      */ 
                     if( member->age > 15000 )
                  {
                     if( member->age > 20000 || number_range( 0, 1 ) == 1 )
                        switch ( number_range( 1, 5 ) )
                        {
                           case 1:
                              member->str--;
                              break;
                           case 2:
                              member->con--;
                              break;
                           case 3:
                              member->dex--;
                              break;
                           case 4:
                              member->spd--;
                              break;
                           case 5:
                              member->end--;
                              break;
                        }
                     if( member->str <= 0 
                          ||member->con <= 0  ||member->dex <= 0  ||member->spd <= 0  ||member->end <= 0 )
                     {
                        sprintf( buf, "%s is too weak to continue fighting, and retires.\n\r", member->name );
                        arena_echo( arena->room, buf );
                        STRFREE( member->name );
                        STRFREE( member->owner );
                        UNLINK( member, arena->first_member, arena->last_member, next, prev );
                        DISPOSE( member );
                        continue;
                     }
                  }
                  else if( member->age < 5000 || number_range( 0, 1 ) == 0 )
                  {
                     if( IS_SET( member->flags, GLAD_RANDOM ) )
                        switch ( number_range( 1, 4 ) )
                        {
                           case 1:
                              member->str++;
                              break;
                           case 2:
                              member->con++;
                              break;
                           case 3:
                              member->dex++;
                              break;
                           case 4:
                              member->spd++;
                              break;
                        }
                     
                     else
                        member->skill++;
                  }
               }
               member->total += member->score;
               if( IS_SET( member->flags, GLAD_TOGGLE ) )
               {
                  TOGGLE_BIT( member->flags, GLAD_WAIT );
                  REMOVE_BIT( member->flags, GLAD_TOGGLE );
               }
               
                  /*
                   * Dispose of loser's bets 
                   */ 
                  if( member == winner )
                  continue;
               if( IS_SET( arena->flags, ARENA_TEAM )  &&member->team == team )
                  continue;
               bet = member->last_bet;
               while( bet )
               {
                  bet = member->last_bet;
                  member->last_bet = bet->prev;
                  STRFREE( bet->better );
                  DISPOSE( bet );
               }
               member->first_bet = NULL;
            }
         }
      }
      arena = arena->next;
   }
   tail_chain(  );
}
ARENA_DATA * find_arena( char *name )
{
   ARENA_DATA * arena;
   arena = first_arena;
   if( !arena )
   {
      bug( "No arenas exist in find_arena.", 0 );
      return NULL;
   }
   while( arena )
   {
      if( !str_prefix( name, arena->name ) )
         return arena;
      arena = arena->next;
   }
   return NULL;
}


/* View the list of arenas, stats on a specific arena, etc */ 
void do_arena( CHAR_DATA * ch, char *argument )
{
   ARENA_DATA * arena;
   ARENA_DATA * new_arena;
   ARENA_MEMBER * member;
   ARENA_MEMBER * new_member;
   BET_DATA * bet;
   BET_DATA * new_bet;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   unsigned long gp;
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   argument = one_argument( argument, arg4 );
   if( !str_cmp( arg1, "help" ) )
   {
      do_new_help( ch, "arena_syntax" );
      return;
   }
   if( !str_cmp( arg1, "list" ) )
   {
      int i = 0;
      arena = first_arena;
      if( !arena )
      {
         send_to_char( "There are no arenas in existance.\n\r", ch );
         return;
      }
      send_to_char( "Arenas in the universe:\n\r", ch );
      while( arena )
      {
         i++;
         ch_printf( ch, "&R%d: &Y%s &R(", i, capitalize( arena->name ) );
         if( !IS_SET( arena->flags, ARENA_HALT ) )
            ch_printf( ch, "&Y%d &Rminutes)\n\r", arena->phase );
         
         else
            ch_printf( ch, "&rhalted&R)\n\r" );
         arena = arena->next;
      }
      return;
   }
   if( !strcmp( arg1, "show" ) )
   {
      if( !( arena = find_arena( arg2 ) ) )
      {
         send_to_char( "There is no arena with that name.\r\n", ch );
         return;
      }
      ch_printf( ch, "&RArena &Y%s&R:\r\n", capitalize( arena->name ) );
      if( arena->room )
      {
         if( !IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
            ch_printf( ch, "Located in %s.\n\r", arena->room->name );
         
         else
            ch_printf( ch, "Located in %s &Y[&R%d&Y]&R.\n\r", arena->room->name, arena->room->vnum );
      }
      if( IS_SET( arena->flags, ARENA_HALT ) )
         ch_printf( ch, "This arena is halted at the moment.\n\r" );
      
      else if( arena->phase > 0 )
         ch_printf( ch, "The arena has been in progress for &Y%d &Rminutes.\n\r", arena->phase );
      
      else
         ch_printf( ch, "The arena will begin in &Y%d &Rminutes.\n\r", abs( arena->phase ) );
      if( IS_SET( arena->flags, ARENA_TEAM ) )
         ch_printf( ch, "&RThis arena is a &YTeam Arena&R.\n\r" );
      
      else
         ch_printf( ch, "&RThis arena is a &YFree For All&R.\n\r" );
      if( arena->winner )
         ch_printf( ch, "Last round's arena was won by &Y%s&R.\n\r", arena->winner );
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to see who is fighting.\n\r", ch );
         return;
      }
      ch_printf( ch, "Gladiators:\n\r" );
      for( member = arena->first_member; member; member = member->next )
      {
         if( IS_SET( arena->flags, ARENA_TEAM ) )
         {
            if( member->team )
            {
               ch_printf( ch, "&Y(&RTeam %d&Y) ", member->team );
            }
            else
            {
               ch_printf( ch, "&Y(&RNo Team&Y) " );
            }
         }
         if( member->hp <= 0 )
            ch_printf( ch, "&r" );
         
         else if( IS_SET( member->flags, GLAD_WAIT ) )
            ch_printf( ch, "&b" );
         
         else
            ch_printf( ch, "&Y" );
         ch_printf( ch, "%-10s%s &B(&b%s&B)  &B%d &bhp", member->name, IS_SET( member->flags, GLAD_TOGGLE ) ? "&b*" : "",
                     member->owner, member->hp );
         ch_printf( ch, "  &R%d &rkills, &C%d &cwins, &C%d &cplayed\n\r", member->kills, member->wins, member->played );
         ch_printf( ch, "  &gSTR: &G%d  &gCON: &G%d  &gDEX: &G%d  &gSPD: &G%d  ", member->str, member->con, member->dex,
                     member->spd );
         if( IS_SET( member->flags, GLAD_RANDOM ) )
            ch_printf( ch, "&r" );
         
         else
            ch_printf( ch, "&g" );
         ch_printf( ch, "Skill Points: &G%d ", member->skill );
         if( member->age < 5000 )
            ch_printf( ch, "&c(&Cyoung&c)\n\r" );
         
         else if( member->age < 10000 )
            ch_printf( ch, "&g(&Gprime&G)\n\r" );
         
         else if( member->age < 15000 )
            ch_printf( ch, "&b(&Bmiddle-age&B)\n\r" );
         
         else if( member->age < 20000 )
            ch_printf( ch, "&P(&paging&P)\n\r" );
         
         else
            ch_printf( ch, "&r(&Rold&r)\n\r" );
         if( member->first_bet )
            for( bet = member->first_bet; bet; bet = bet->next )
            {
               gp = bet->wager;
               ch_printf( ch, "  &RBet: &Y%s &Rbet &C%dm", bet->better, ( int )gp / 1000000 );
               gp = gp % 1000000;
               ch_printf( ch, " &Y%dg", ( int )gp / 10000 );
               gp = gp % 10000;
               ch_printf( ch, " &W%ds", ( int )gp / 100 );
               gp = gp % 100;
               ch_printf( ch, " &O%dc &Rat &Y%d:1 &Rodds", gp, bet->odds );
               if( bet->winnings )
               {
                  gp = bet->winnings;
                  ch_printf( ch, " &Rand won &C%dm", ( int )gp / 1000000 );
                  gp = gp % 1000000;
                  ch_printf( ch, " &Y%dg", ( int )gp / 10000 );
                  gp = gp % 10000;
                  ch_printf( ch, " &W%ds", ( int )gp / 100 );
                  gp = gp % 100;
                  ch_printf( ch, " &O%dc!\n\r", gp );
               }
               else
                  ch_printf( ch, ".\n\r" );
            }
      }
      return;
   }
   if( !strcmp( arg1, "find" ) )
   {
      bool found = FALSE;
      ch_printf( ch, "You send out an apprentice to search for %s...\n\r", capitalize( arg2 ) );
      for( arena = first_arena; arena; arena = arena->next )
      {
         for( member = arena->first_member; member; member = member->next )
         {
            if( !str_cmp( member->name, arg2 ) )
            {
               found = TRUE;
               ch_printf( ch, "%s is in %s.\n\r", member->name, arena->name );
            }
         }
      }
      if( !found )
         ch_printf( ch, "%s could not be found.\n\r", capitalize( arg2 ) );
      return;
   }
   if( !strcmp( arg1, "move" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( arena->phase > 0  &&!IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
               return;
            }
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            new_arena = find_arena( arg4 );
            if( !new_arena )
            {
               send_to_char( "Unknown arena name.\n\r", ch );
               return;
            }
            if( new_arena->phase > 0  &&!IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
               return;
            }
            for( new_member = new_arena->first_member; new_member; new_member = new_member->next )
            {
               if( IS_SET( new_arena->flags, ARENA_TEAM )  &&!str_cmp( new_member->owner, member->owner ) )
               {
                  send_to_char( "You already own a gladiator theree.\n\r", ch );
                  return;
               }
            }
            UNLINK( member, arena->first_member, arena->last_member, next, prev );
            LINK( member, new_arena->first_member, new_arena->last_member, next, prev );
            member->arena = new_arena;
            ch_printf( ch, "Gladiator %s has been moved from %s to %s.\n\r", member->name, arena->name, new_arena->name );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "odds" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to place your bets.\n\r", ch );
         return;
      }
      ch_printf( ch, "&rArena &R%s &rodds:\n\r", arena->name );
      for( member = arena->first_member; member; member = member->next )
      {
         if( IS_SET( member->flags, GLAD_WAIT ) )
            continue;
         ch_printf( ch, "&ROdds for &Y%s&R are &Y%d:1&R.\n\r", member->name, get_odds( member ) );
      }
      return;
   }
   if( !strcmp( arg1, "score" ) )
   {
      int i, count, last, score;
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to look at the scores.\n\r", ch );
         return;
      }
      score = count = 0;
      last = 1000000000;
      i = 1;
      for( member = arena->first_member; member; member = member->next )
      {
         count++;
      }
      ch_printf( ch, "&rArena &R%s &rscores:\n\r", arena->name );
      while( count > 0 )
      {
         score = 0;
         for( member = arena->first_member; member; member = member->next )
         {
            if( IS_SET( member->flags, GLAD_WAIT ) )
               continue;
            if( member->score > score  &&member->score < last )
            {
               score = member->score;
               new_member = member;
            }
         }
         ch_printf( ch, "&R#%d) &Y%-10s &R%d &rthis match, &R%d &rtotal.\n\r", i, new_member->name, new_member->score,
                     new_member->total );
         last = score;
         i++;
         count--;
      }
      return;
   }
   if( !strcmp( arg1, "bet" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( arena->phase > 0 )
      {
         send_to_char( "Bets are closed on this arena, sorry.\n\r", ch );
         return;
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to place your bets.\n\r", ch );
         return;
      }
      if( !is_number( arg4 ) )
      {
         send_to_char( "Bet how much?\n\r", ch );
         return;
      }
      gp = atoi( arg4 );
      if( !str_cmp( argument, "mithril" ) )
         gp *= 1000000;
      
      else if( !str_cmp( argument, "gold" ) )
         gp *= 10000;
      
      else if( !str_cmp( argument, "silver" ) )
         gp *= 100;
      if( ch->gold < gp )
      {
         send_to_char( "You don't have that kind of money.\n\r", ch );
         return;
      }
      if( gp > 100000000 )
      {
         send_to_char( "You cannot bet more than &C100 mithril&w at once.\n\r", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "That gladiator is not fighting in this arena.\n\r", ch );
               return;
            }
            CREATE( new_bet, BET_DATA, 1 );
            LINK( new_bet, member->first_bet, member->last_bet, next, prev );
            new_bet->odds = get_odds( member );
            new_bet->wager = gp;
            new_bet->winnings = 0;
            new_bet->better = STRALLOC( ch->name );
            ch->gold -= gp;
            ch_printf( ch, "You place a bet of &C%dm", ( int )gp / 1000000 );
            gp = gp % 1000000;
            ch_printf( ch, " &Y%dg", ( int )gp / 10000 );
            gp = gp % 10000;
            ch_printf( ch, " &W%ds", ( int )gp / 100 );
            gp = gp % 100;
            ch_printf( ch, " &O%dc on %s.\n\r", gp, member->name );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "collect" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to collect your winnings.\n\r", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         for( bet = member->first_bet; bet; bet = bet->next )
         {
            if( str_cmp( bet->better, ch->name ) )
               continue;
            if( bet->winnings == 0 )
               continue;
            ch->gold += bet->winnings;
            gp = bet->winnings;
            ch_printf( ch, "You collect your winnings of &C%dm", ( int )gp / 1000000 );
            gp = gp % 1000000;
            ch_printf( ch, " &Y%dg", ( int )gp / 10000 );
            gp = gp % 10000;
            ch_printf( ch, " &W%ds", ( int )gp / 100 );
            gp = gp % 100;
            ch_printf( ch, " &O%dc.\n\r", gp );
            STRFREE( bet->better );
            UNLINK( bet, member->first_bet, member->last_bet, next, prev );
            DISPOSE( bet );
            return;
         }
      }
      send_to_char( "You have no winning bets.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "delete" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to fire a gladiator.\n\r", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( arena->phase > 0  &&!IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
               return;
            }
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            ch_printf( ch, "You fire %s.\n\r", member->name );
            STRFREE( member->name );
            STRFREE( member->owner );
            UNLINK( member, arena->first_member, arena->last_member, next, prev );
            DISPOSE( member );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "team" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to set a team.\n\r", ch );
         return;
      }
      if( !IS_SET( arena->flags, ARENA_TEAM ) )
      {
         send_to_char( "This is not a team arena.\n\r", ch );
         return;
      }
      if( arena->phase > 0 )
      {
         send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            member->team = atoi( arg4 );
            ch_printf( ch, "Gladiator %s is now on team %d.\n\r", member->name, member->team );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "auto" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            if( IS_SET( member->flags, GLAD_RANDOM ) )
            {
               REMOVE_BIT( member->flags, GLAD_RANDOM );
               ch_printf( ch, "Gladiator %s will now no longer gain skills automatically.\n\r", member->name );
            }
            else
            {
               SET_BIT( member->flags, GLAD_RANDOM );
               ch_printf( ch, "Gladiator %s will now gain skills automatically.\n\r", member->name );
            }
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "wait" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( arena->phase > 0 )
      {
         send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            if( IS_SET( member->flags, GLAD_WAIT ) )
            {
               REMOVE_BIT( member->flags, GLAD_WAIT );
               ch_printf( ch, "Gladiator %s has been removed from the waiting room.\n\r", member->name );
            }
            else
            {
               SET_BIT( member->flags, GLAD_WAIT );
               ch_printf( ch, "Gladiator %s has been sent to the waiting room.\n\r", member->name );
            }
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "toggle" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            if( IS_SET( member->flags, GLAD_TOGGLE ) )
            {
               REMOVE_BIT( member->flags, GLAD_TOGGLE );
               ch_printf( ch, "Gladiator %s toggle has been disabled.\n\r", member->name );
            }
            else
            {
               SET_BIT( member->flags, GLAD_TOGGLE );
               ch_printf( ch, "Gladiator %s will now %s the waiting room when the arena is over.\n\r", member->name,
                           IS_SET( member->flags, GLAD_WAIT ) ? "leave" : "enter" );
            }
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "rename" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      for( new_arena = first_arena; new_arena; new_arena = new_arena->next )
      {
         for( member = new_arena->first_member; member; member = member->next )
         {
            if( !str_cmp( member->name, arg4 ) )
            {
               send_to_char( "There is already a gladiator by that name.\n\r", ch );
               return;
            }
         }
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( arena->phase > 0  &&!IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
               return;
            }
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            add_hiscore( "glad", member->name, 0 );
            STRFREE( member->name );
            member->name = STRALLOC( capitalize( arg4 ) );
            add_hiscore( "glad", member->name, member->total );
            if( !str_cmp( member->name, "" ) )
            {
               STRFREE( member->name );
               strcpy( buf, random_name( buf, LANG_DROW ) );
               member->name = STRALLOC( capitalize( munch_colors( buf ) ) );
            }
            ch_printf( ch, "Gladiator name has been changed to %s.\n\r", member->name );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "transfer" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         CHAR_DATA * victim;
         if( !str_cmp( member->name, arg3 ) )
         {
            if( arena->phase > 0  &&!IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
               return;
            }
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            victim = get_char_world( ch, arg4 );
            if( !victim || IS_NPC( victim ) )
            {
               send_to_char( "No such player around right now.\n\r", ch );
               return;
            }
            for( new_member = arena->first_member; new_member; new_member = new_member->next )
            {
               if( IS_SET( arena->flags, ARENA_TEAM )  &&!str_cmp( new_member->owner, victim->name ) )
               {
                  send_to_char( "They already own a gladiator here.\n\r", ch );
                  return;
               }
            }
            STRFREE( member->owner );
            member->owner = STRALLOC( victim->name );
            ch_printf( ch, "Ownership of gladiator %s has been transfered to %s.\n\r", member->name, member->owner );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "skill" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( arena->phase > 0  &&!IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
               return;
            }
            if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&str_cmp( member->owner, ch->name ) )
            {
               send_to_char( "That is not your gladiator.\n\r", ch );
               return;
            }
            if( member->skill <= 0 )
            {
               send_to_char( "You don't have enough skill points to raise anything.\n\r", ch );
               return;
            }
            if( !str_cmp( arg4, "str" ) )
               member->str++;
            
            else if( !str_cmp( arg4, "con" ) )
               member->con++;
            
            else if( !str_cmp( arg4, "dex" ) )
               member->dex++;
            
            else if( !str_cmp( arg4, "spd" ) )
               member->spd++;
            
            else
            {
               send_to_char( "No such stat.\n\r", ch );
               return;
            }
            member->skill--;
            ch_printf( ch, "%s's %s has been raised.\n\r", member->name, arg4 );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "add" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC )  &&ch->in_room != arena->room )
      {
         send_to_char( "Head to the arena to hire a gladiator.\n\r", ch );
         return;
      }
      for( new_arena = first_arena; new_arena; new_arena = new_arena->next )
      {
         for( member = new_arena->first_member; member; member = member->next )
         {
            if( !str_cmp( member->name, arg4 ) )
            {
               send_to_char( "There is already a gladiator by that name.\n\r", ch );
               return;
            }
         }
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( IS_SET( arena->flags, ARENA_TEAM )  &&!str_cmp( member->owner, ch->name ) )
         {
            send_to_char( "You already own a gladiator here.\n\r", ch );
            return;
         }
      }
      if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC ) )
      {
         if( ch->gold >= 1000000 )
         {
            send_to_char( "You pay &C1 mithril&w.\n\r", ch );
            ch->gold -= 1000000;
         }
         else
         {
            send_to_char( "You do not have the &C1 mithril&w required to hire a gladiator.\n\r", ch );
            return;
         }
      }
      CREATE( new_member, ARENA_MEMBER, 1 );
      if( !arena->first_member )
      {
         arena->first_member = new_member;
         arena->last_member = new_member;
         new_member->next = NULL;
         new_member->prev = NULL;
      }
      else
      {
         arena->last_member->next = new_member;
         new_member->prev = arena->last_member;
         arena->last_member = new_member;
         new_member->next = NULL;
      }
      member = new_member;
      member->arena = arena;
      if( arg3 && arg3 != NULL )
      {
         member->name = STRALLOC( capitalize( munch_colors( arg3 ) ) );
      }
      if( !str_cmp( member->name, "" ) )
      {
         STRFREE( member->name );
         strcpy( buf, random_name( buf, LANG_DROW ) );
         member->name = STRALLOC( capitalize( buf ) );
      }
      sprintf( buf, "gladiator %s added to arena %s.\r\n", member->name, arena->name );
      member->owner = STRALLOC( ch->name );
      member->hp = 5;
      member->wins = 0;
      member->kills = 0;
      member->played = 0;
      member->con = 5;
      member->str = 5;
      member->dex = 5;
      member->spd = 5;
      SET_BIT( member->flags, GLAD_RANDOM );
      if( arena->phase > 0 )
      {
         SET_BIT( member->flags, GLAD_WAIT + GLAD_TOGGLE );
      }
      send_to_char( buf, ch );
      do_arena( ch, "save" );
      return;
   }
   if( !strcmp( arg1, "save" ) )
   {
      save_arenas(  );
      send_to_char( "arenas saved.\n\r", ch );
      return;
   }
   if( !IS_SET( ch->pcdata->permissions, PERMIT_MISC ) )
   {
      send_to_char( "Unknown option.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "topten" ) )
   {
      for( arena = first_arena; arena; arena = arena->next )
      {
         for( member = arena->first_member; member; member = member->next )
         {
            add_hiscore( "glad", member->name, member->total );
         }
      }
      send_to_char( "Gladiator top ten updated.\n\r", ch );
      do_hiscore( ch, "glad" );
      return;
   }
   if( !strcmp( arg1, "scatter" ) )
   {
      int i, count;
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      for( member = arena->first_member; member; member = member->next )
      {
         if( !str_cmp( member->name, arg3 ) )
         {
            if( arena->phase > 0  &&!IS_SET( member->flags, GLAD_WAIT ) )
            {
               send_to_char( "Not while the arena is in progress, sorry.\n\r", ch );
               return;
            }
            count = i = 0;
            for( new_arena = first_arena; new_arena; new_arena = new_arena->next )
            {
               if( IS_SET( new_arena->flags, ARENA_HALT ) )
                  continue;
               if( IS_SET( new_arena->flags, ARENA_TEAM ) )
                  continue;
               count++;
            }
            count = number_range( 1, count );
            for( new_arena = first_arena; new_arena; new_arena = new_arena->next )
            {
               if( IS_SET( new_arena->flags, ARENA_HALT ) )
                  continue;
               if( IS_SET( new_arena->flags, ARENA_TEAM ) )
                  continue;
               if( ++i == count )
               {
                  UNLINK( member, arena->first_member, arena->last_member, next, prev );
                  LINK( member, new_arena->first_member, new_arena->last_member, next, prev );
                  if( new_arena->phase > 0 )
                  {
                     SET_BIT( member->flags, GLAD_WAIT + GLAD_TOGGLE );
                  }
                  ch_printf( ch, "Gladiator %s has been moved to %s.\n\r", member->name, new_arena->name );
                  return;
               }
            }
            send_to_char( "Scatter failed.\n\r", ch );
            return;
         }
      }
      send_to_char( "No such gladiator.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "create" ) )
   {
      CREATE( new_arena, ARENA_DATA, 1 );
      if( !first_arena )
      {
         first_arena = new_arena;
         last_arena = new_arena;
         new_arena->next = NULL;
         new_arena->prev = NULL;
      }
      else
      {
         last_arena->next = new_arena;
         new_arena->prev = last_arena;
         last_arena = new_arena;
         new_arena->next = NULL;
      }
      arena = new_arena;
      if( arg2 )
         arena->name = STRALLOC( capitalize( arg2 ) );
      
      else
         arena->name = STRALLOC( "newarena" );
      sprintf( buf, "arena %s created.\r\n", arena->name );
      arena->phase = -20;
      arena->flags = 0;
      send_to_char( buf, ch );
      return;
   }
   if( !strcmp( arg1, "set" ) )
   {
      arena = find_arena( arg2 );
      if( !arena )
      {
         send_to_char( "Unknown arena name.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "name" ) )
      {
         arena->name = str_dup( arg4 );
         send_to_char( "arena name set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "room" ) )
      {
         arena->vnum = ch->in_room->vnum;
         arena->room = ch->in_room;
         send_to_char( "arena room set.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "team" ) )
      {
         TOGGLE_BIT( arena->flags, ARENA_TEAM );
         send_to_char( "arena team status toggled.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "halt" ) )
      {
         TOGGLE_BIT( arena->flags, ARENA_HALT );
         send_to_char( "arena halt toggled.\r\n", ch );
         return;
      }
      if( !strcmp( arg3, "debug" ) )
      {
         TOGGLE_BIT( arena->flags, ARENA_DEBUG );
         send_to_char( "arena debug mode toggled.\r\n", ch );
         return;
      }
      send_to_char( "Invalid field.\n\r", ch );
      return;
   }
   if( !strcmp( arg1, "tick" ) )
   {
      arena_update(  );
      return;
   }
   if( !strcmp( arg1, "clear" ) )
   {
      while( last_arena )
      {
         arena = last_arena;
         last_arena = arena->prev;
         while( arena->last_member )
         {
            member = arena->last_member;
            arena->last_member = member->prev;
            STRFREE( member->name );
            STRFREE( member->owner );
            DISPOSE( member );
         }
         STRFREE( arena->name );
         DISPOSE( arena );
      }
      first_arena = NULL;
      last_arena = NULL;
      send_to_char( "arena list cleared.\n\r", ch );
      return;
   }
   send_to_char( "Unknown option.\r\n", ch );
}


