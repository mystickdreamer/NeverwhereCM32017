/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 *			 Tracking/hunting module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
   
#define BFS_ERROR	   -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH	   -3
#define BFS_MARK    536870912
   
#define TRACK_THROUGH_DOORS
extern sh_int top_room;

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/ 
   
/* Track code modified to use trails 9/25/1999 -- Scion */ 
int track_direction( CHAR_DATA * ch, CHAR_DATA * vict )
{
   TRAIL_DATA * trail;
   if( !ch || !vict )
   {
      bug( "Illegal value passed to track_direction (track.c)", 0 );
      return BFS_ERROR;
   }
   
      /*
       * make sure the char still exists 
       */ 
      if( vict->name == NULL )
      return BFS_ERROR;
   if( char_died( vict ) )
      return BFS_ERROR;
   if( ch->in_room == vict->in_room )
      return BFS_ALREADY_THERE;
   for( trail = ch->in_room->first_trail; trail; trail = trail->next )
   {
      if( !strcmp( trail->name, vict->name ) )
      {
         if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
            return trail->to;
         if( trail->blood == TRUE )
            return trail->to;
         if( trail->fly == FALSE )
            return trail->to;
      }
   }
   return BFS_NO_PATH;
}
typedef struct bfs_queue_struct BFS_DATA;
struct bfs_queue_struct 
{
   ROOM_INDEX_DATA * room;
   char dir;
    BFS_DATA * next;
 };
static BFS_DATA *queue_head = NULL, *queue_tail = NULL, *room_queue = NULL;

/* Utility macros */ 
#define MARK(room)	(SET_BIT(	(room)->room_flags, BFS_MARK) )
#define UNMARK(room)	(REMOVE_BIT(	(room)->room_flags, BFS_MARK) )
#define IS_MARKED(room)	(IS_SET(	(room)->room_flags, BFS_MARK) )
   bool valid_edge( EXIT_DATA * pexit ) 
{
   if( pexit->to_room 
#ifndef TRACK_THROUGH_DOORS
        &&!IS_SET( pexit->exit_info, EX_CLOSED ) 
#endif /*  */
        &&!IS_MARKED( pexit->to_room ) )
      return TRUE;
   
   else
      return FALSE;
}
void bfs_enqueue( ROOM_INDEX_DATA * room, char dir ) 
{
   BFS_DATA * curr;
   curr = malloc( sizeof( BFS_DATA ) );
   curr->room = room;
   curr->dir = dir;
   curr->next = NULL;
   if( queue_tail )
      
   {
      queue_tail->next = curr;
      queue_tail = curr;
   }
   
   else
      queue_head = queue_tail = curr;
}
void bfs_dequeue( void ) 
{
   BFS_DATA * curr;
   curr = queue_head;
   if( !( queue_head = queue_head->next ) )
      queue_tail = NULL;
   free( curr );
}
void bfs_clear_queue( void ) 
{
   while( queue_head )
      bfs_dequeue(  );
}
void room_enqueue( ROOM_INDEX_DATA * room ) 
{
   BFS_DATA * curr;
   curr = malloc( sizeof( BFS_DATA ) );
   curr->room = room;
   curr->next = room_queue;
   room_queue = curr;
}
void clean_room_queue( void ) 
{
   BFS_DATA * curr, *curr_next;
   for( curr = room_queue; curr; curr = curr_next )
      
   {
      UNMARK( curr->room );
      curr_next = curr->next;
      free( curr );
   }
   room_queue = NULL;
}
int find_first_step( ROOM_INDEX_DATA * src, ROOM_INDEX_DATA * target, int maxdist ) 
{
   int curr_dir, count;
   EXIT_DATA * pexit;
   if( !src || !target )
      
   {
      bug( "Illegal value passed to find_first_step (track.c)", 0 );
      return BFS_ERROR;
   }
   if( src == target )
      return BFS_ALREADY_THERE;
   
#ifndef TRACK_THROUGH_DOORS
      if( src->area != target->area )
      return BFS_NO_PATH;
   
#endif /*  */
      room_enqueue( src );
   MARK( src );
   
      /*
       * first, enqueue the first steps, saving which direction we're going. 
       */ 
      for( pexit = src->first_exit; pexit; pexit = pexit->next )
      if( valid_edge( pexit ) )
         
      {
         curr_dir = pexit->vdir;
         MARK( pexit->to_room );
         room_enqueue( pexit->to_room );
         bfs_enqueue( pexit->to_room, curr_dir );
      }
   count = 0;
   while( queue_head )
      
   {
      if( ++count > maxdist )
         
      {
         bfs_clear_queue(  );
         clean_room_queue(  );
         return BFS_NO_PATH;
      }
      if( queue_head->room == target )
         
      {
         curr_dir = queue_head->dir;
         bfs_clear_queue(  );
         clean_room_queue(  );
         return curr_dir;
      }
      
      else
         
      {
         for( pexit = queue_head->room->first_exit; pexit; pexit = pexit->next )
            if( valid_edge( pexit ) )
               
            {
               curr_dir = pexit->vdir;
               MARK( pexit->to_room );
               room_enqueue( pexit->to_room );
               bfs_enqueue( pexit->to_room, queue_head->dir );
            }
         bfs_dequeue(  );
      }
   }
   clean_room_queue(  );
   return BFS_NO_PATH;
}
void do_track( CHAR_DATA * ch, char *argument ) 
{
   CHAR_DATA * vict;
   char arg[MAX_INPUT_LENGTH];
   int dir;
   
      //int maxdist;
      if( !IS_NPC( ch ) && ch->pcdata->learned[gsn_track] <= 0 )
      
   {
      send_to_char( "You do not know of this skill yet.\n\r", ch );
      return;
   }
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      send_to_char( "Whom are you trying to track?\n\r", ch );
      return;
   }
   WAIT_STATE( ch, skill_table[gsn_track]->beats );
   if( !( vict = get_char_world( ch, arg ) ) )
      
   {
      send_to_char( "You can't find a trail of anyone like that.\n\r", ch );
      return;
   }
   dir = track_direction( ch, vict );
   if( dir >= 0 && number_range( 1, 30 ) > number_range( 1, ch->pcdata->noncombat[SK_NATURE] ) )
      dir = URANGE( 0, number_range( 0, MAX_DIR ), MAX_DIR + 1 );
   switch ( dir )
      
   {
      case BFS_ERROR:
         send_to_char( "Hmm... something seems to be wrong.\n\r", ch );
         break;
      case BFS_ALREADY_THERE:
         send_to_char( "You're already in the same room!\n\r", ch );
         break;
      case BFS_NO_PATH:
         send_to_char( "You can't sense a trail from here.\n\r", ch );
         learn_from_failure( ch, gsn_track );
         if( IS_NPC( ch ) )
            do_travel( ch, vict->name );
         break;
      default:
         ch_printf( ch, "You sense a trail %s from here.\n\r", dir_name[dir] );
         learn_from_success( ch, gsn_track );
         break;
   }
}
void found_prey( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   if( victim == NULL )
      
   {
      bug( "Found_prey: null victim", 0 );
      return;
   }
   if( victim->in_room == NULL )
      
   {
      bug( "Found_prey: null victim->in_room", 0 );
      return;
   }
   if( ch->in_room == NULL )
   {
      bug( "Found_prey: null ch->in_room", 0 );
      return;
   }
   if( !IS_FIGHTING( ch ) )
   {
      if( ch->spec_fun )
         do_travel( ch, victim->name );
      return;
   }
   mob_attack( ch, victim );
   
      /*
       * player default auto attacking 
       */ 
      if( !IS_NPC( victim ) && !victim->wait && victim->pcdata->auto_attack )
   {
      interpret( victim, victim->pcdata->auto_attack, FALSE );
   }
   return;
}
void hunt_victim( CHAR_DATA * ch ) 
{
   bool found;
   CHAR_DATA * tmp;
   EXIT_DATA * pexit;
   sh_int ret;
   if( !ch || !ch->hunting || ch->fearing )
      return;
   
      /*
       * make sure the char still exists 
       */ 
      for( found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next )
      if( ch->hunting->who == tmp )
         found = TRUE;
   if( !found )
      
   {
      stop_hunting( ch );
      return;
   }
   if( ch->in_room == ch->hunting->who->in_room )
      
   {
      if( IS_FIGHTING( ch ) )
         return;
      found_prey( ch, ch->hunting->who );
      return;
   }
   ret = track_direction( ch, ch->hunting->who );
   if( ret < 0 )
      
   {
      stop_hunting( ch );
      return;
   }
   
   else
      
   {
      if( ( pexit = get_exit( ch->in_room, ret ) ) == NULL )
         
      {
         bug( "Hunt_victim: lost exit for %s in room %d", ch->name, ch->in_room );
         return;
      }
      if( ch->position > POS_SITTING )  /* Sleeping mobs were chasing people.. hmm.. -- Scion */
         return;
      if( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         do_open( ch, dir_name[pexit->vdir] ); /* Open doors, it ain't hard -- Scion */
      }
      move_char( ch, pexit, FALSE );
      if( !ch->hunting )
         
      {
         if( !ch->in_room )
            
         {
            bug( "Hunt_victim: no ch->in_room!  Mob #%d, name: %s.  Placing mob in limbo.", ch->pIndexData->vnum,
                  ch->name );
            char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
            return;
         }
         return;
      }
      if( ch->in_room == ch->hunting->who->in_room )
         found_prey( ch, ch->hunting->who );
      return;
   }
}


