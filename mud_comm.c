/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 ****************************************************************************
 *  The MUDprograms are heavily based on the original MOBprogram code that  *
 *  was written by N'Atas-ha.						    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
   
/* from handler.c */ 
extern MATERIAL_DATA *material_lookup( int number );

/* from shops.c */ 
extern int get_cost( CHAR_DATA * ch, SHOP_DATA * pShop, OBJ_DATA * obj, bool fBuy );
extern int top_affect;
char *mprog_type_to_name args( ( int type ) );
char *mprog_type_to_name( int type ) 
{
   switch ( type )
      
   {
      case IN_FILE_PROG:
         return "in_file_prog";
      case ACT_PROG:
         return "act_prog";
      case SPEECH_PROG:
         return "speech_prog";
      case RAND_PROG:
         return "rand_prog";
      case FIGHT_PROG:
         return "fight_prog";
      case HITPRCNT_PROG:
         return "hitprcnt_prog";
      case DEATH_PROG:
         return "death_prog";
      case ENTRY_PROG:
         return "entry_prog";
      case GREET_PROG:
         return "greet_prog";
      case ALL_GREET_PROG:
         return "all_greet_prog";
      case GIVE_PROG:
         return "give_prog";
      case BRIBE_PROG:
         return "bribe_prog";
      case HOUR_PROG:
         return "hour_prog";
      case TIME_PROG:
         return "time_prog";
      case WEAR_PROG:
         return "wear_prog";
      case REMOVE_PROG:
         return "remove_prog";
      case SAC_PROG:
         return "sac_prog";
      case LOOK_PROG:
         return "look_prog";
      case EXA_PROG:
         return "exa_prog";
      case ZAP_PROG:
         return "zap_prog";
      case GET_PROG:
         return "get_prog";
      case DROP_PROG:
         return "drop_prog";
      case REPAIR_PROG:
         return "repair_prog";
      case DAMAGE_PROG:
         return "damage_prog";
      case PULL_PROG:
         return "pull_prog";
      case PUSH_PROG:
         return "push_prog";
      case SCRIPT_PROG:
         return "script_prog";
      case SLEEP_PROG:
         return "sleep_prog";
      case REST_PROG:
         return "rest_prog";
      case LEAVE_PROG:
         return "leave_prog";
      case USE_PROG:
         return "use_prog";
      default:
         return "ERROR_PROG";
   }
}


/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MUDprograms which are set.
 */ 
void do_mpstat( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   MPROG_DATA * mprg;
   CHAR_DATA * victim;
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      send_to_char( "MProg stat whom?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
      
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }
   if( !IS_NPC( victim ) )
      
   {
      send_to_char( "Only Mobiles can have MobPrograms!\n\r", ch );
      return;
   }
   if( xIS_EMPTY( victim->pIndexData->progtypes ) )
      
   {
      send_to_char( "That Mobile has no Programs set.\n\r", ch );
      return;
   }
   pager_printf( ch, "Name: %s.  Vnum: %d.\n\r", victim->name, victim->pIndexData->vnum );
   pager_printf( ch, "Short description: %s.\n\r", victim->short_descr );
   pager_printf( ch, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d. \n\r", victim->hit, victim->max_hit, victim->mana,
                   victim->max_mana, victim->move, victim->max_move );
   pager_printf( ch, "AC: %d.  Gold: %d.  Exp: %d.\n\r", GET_AC( victim ), victim->gold, victim->exp );
   for( mprg = victim->pIndexData->mudprogs; mprg; mprg = mprg->next )
      pager_printf( ch, ">%s %s\n\r%s\n\r", mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
   return;
}


/* Opstat - Scryn 8/12*/ 
void do_opstat( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   MPROG_DATA * mprg;
   OBJ_DATA * obj;
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      send_to_char( "OProg stat what?\n\r", ch );
      return;
   }
   if( ( obj = get_obj_world( ch, arg ) ) == NULL )
      
   {
      send_to_char( "You cannot find that.\n\r", ch );
      return;
   }
   if( xIS_EMPTY( obj->pIndexData->progtypes ) )
      
   {
      send_to_char( "That object has no programs set.\n\r", ch );
      return;
   }
   pager_printf( ch, "Name: %s.  Vnum: %d.\n\r", obj->name, obj->pIndexData->vnum );
   pager_printf( ch, "Short description: %s.\n\r", obj->short_descr );
   for( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
      pager_printf( ch, ">%s %s\n\r%s\n\r", mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
   return;
}


/* Rpstat - Scryn 8/12 */ 
void do_rpstat( CHAR_DATA * ch, char *argument ) 
{
   MPROG_DATA * mprg;
   if( xIS_EMPTY( ch->in_room->progtypes ) )
      
   {
      send_to_char( "This room has no programs set.\n\r", ch );
      return;
   }
   pager_printf( ch, "Name: %s.  Vnum: %d.\n\r", ch->in_room->name, ch->in_room->vnum );
   for( mprg = ch->in_room->mudprogs; mprg; mprg = mprg->next )
      pager_printf( ch, ">%s %s\n\r%s\n\r", mprog_type_to_name( mprg->type ), mprg->arglist, mprg->comlist );
   return;
}


/* add x Talent points to ch: mpaddpoints <ch> <points> */ 
void do_mpaddpoints( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   int points;
   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      progbug( "Mpaddpoints - Bad syntax", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg ) ) == NULL )
      
   {
      progbug( "Mpaddpoints - Victim not there", ch );
      return;
   }
   if( IS_NPC( victim ) )
      
   {
      progbug( "Mpaddpoints - Victim is NPC", ch );
      return;
   }
   points = atoi( argument );
   if( points < -500 || points > 500 )
   {
      progbug( "Mpaddpoints - invalid number", ch );
      return;
   }
   victim->pcdata->points += points;
}


/* lets the mobile kill any player or mobile without murder*/ 
void do_mpkill( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   if( !ch )
      
   {
      bug( "Nonexistent ch in do_mpkill!", 0 );
      return;
   }
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      progbug( "MpKill - no argument", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg ) ) == NULL )
      
   {
      progbug( "MpKill - Victim not in room", ch );
      return;
   }
   if( victim == ch )
      
   {
      progbug( "MpKill - Bad victim to attack", ch );
      return;
   }
   ch->last_hit = victim;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */ 
void do_mpjunk( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA * obj;
   OBJ_DATA * obj_next;
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      progbug( "Mpjunk - No argument", ch );
      return;
   }
   if( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
      
   {
      if( ( obj = get_obj_wear( ch, arg ) ) != NULL )
         
      {
         unequip_char( ch, obj );
         extract_obj( obj );
         return;
      }
      if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
         return;
      extract_obj( obj );
   }
   
   else
      for( obj = ch->first_carrying; obj; obj = obj_next )
         
      {
         obj_next = obj->next_content;
         if( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            
         {
            if( obj->wear_loc != WEAR_NONE )
               unequip_char( ch, obj );
            extract_obj( obj );
         }
      }
   return;
}


/*
 * This function examines a text string to see if the first "word" is a
 * color indicator (e.g. _red, _whi_, _blu).  -  Gorog
 */ 
int get_color( char *argument ) /* get color code from command string */  
{
   char color[MAX_INPUT_LENGTH];
   char *cptr;
   static char const *color_list = "_bla_red_dgr_bro_dbl_pur_cya_cha_dch_ora_gre_yel_blu_pin_lbl_whi";
   static char const *blink_list = "*bla*red*dgr*bro*dbl*pur*cya*cha*dch*ora*gre*yel*blu*pin*lbl*whi";
   one_argument( argument, color );
   if( color[0] != '_' && color[0] != '*' )
      return 0;
   if( ( cptr = strstr( color_list, color ) ) )
      return ( cptr - color_list ) / 4;
   if( ( cptr = strstr( blink_list, color ) ) )
      return ( cptr - blink_list ) / 4 + AT_BLINK;
   return 0;
}


/* Prints the argument to all the rooms around the mobile */ 
void do_mpasound( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA * was_in_room;
   EXIT_DATA * pexit;
   sh_int color;
   EXT_BV actflags;
   if( argument[0] == '\0' )
   {
      progbug( "Mpasound - No argument", ch );
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT( ch->act, ACT_SECRETIVE );
   if( ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg1 );
   was_in_room = ch->in_room;
   for( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
      
   {
      if( pexit->to_room  &&pexit->to_room != was_in_room )
         
      {
         ch->in_room = pexit->to_room;
         MOBtrigger = FALSE;
         if( color )
            act( color, argument, ch, NULL, NULL, TO_ROOM );
         
         else
            act( AT_SAY, argument, ch, NULL, NULL, TO_ROOM );
      }
   }
   ch->act = actflags;
   ch->in_room = was_in_room;
   return;
}


/* prints the message to all in the room other than the mob and victim */ 
void do_mpechoaround( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   EXT_BV actflags;
   sh_int color;
   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      progbug( "Mpechoaround - No argument", ch );
      return;
   }
   if( !( victim = get_char_room( ch, arg ) ) )
      
   {
      progbug( "Mpechoaround - victim does not exist", ch );
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT( ch->act, ACT_SECRETIVE );
   if( ( color = get_color( argument ) ) )
      
   {
      argument = one_argument( argument, arg );
      act( color, argument, ch, NULL, victim, TO_NOTVICT );
   }
   
   else
      act( AT_ACTION, argument, ch, NULL, victim, TO_NOTVICT );
   ch->act = actflags;
}


/* prints message only to victim */ 
void do_mpechoat( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   EXT_BV actflags;
   sh_int color;
   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
      
   {
      progbug( "Mpechoat - No argument", ch );
      return;
   }
   if( !( victim = get_char_room( ch, arg ) ) )
      
   {
      progbug( "Mpechoat - victim does not exist", ch );
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT( ch->act, ACT_SECRETIVE );
   if( ( color = get_color( argument ) ) )
      
   {
      argument = one_argument( argument, arg );
      act( color, argument, ch, NULL, victim, TO_VICT );
   }
   
   else
      act( AT_ACTION, argument, ch, NULL, victim, TO_VICT );
   ch->act = actflags;
}


/* prints message to room at large. */ 
void do_mpecho( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   sh_int color;
   EXT_BV actflags;
   if( argument[0] == '\0' )
      
   {
      progbug( "Mpecho - called w/o argument", ch );
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT( ch->act, ACT_SECRETIVE );
   if( ( color = get_color( argument ) ) )
      
   {
      argument = one_argument( argument, arg1 );
      act( color, argument, ch, NULL, NULL, TO_ROOM );
   }
   
   else
      act( AT_ACTION, argument, ch, NULL, NULL, TO_ROOM );
   ch->act = actflags;
}
void do_mpsellrand( CHAR_DATA * ch, char *argument )
{
   int num, count, sold;
   OBJ_DATA * obj;
   OBJ_DATA * obj_next;
   SHOP_DATA * shop;
   char buf[MAX_STRING_LENGTH];
   shop = ch->in_room->pShop;
   if( !shop )
      return;
   act( AT_ACTION, "A customer comes in and looks at the merchandise.", ch, NULL, NULL, TO_ROOM );
   if( number_range( 100, 500 ) < shop->profit_sell )
      return;
   if( number_range( 1, 100 ) > shop->profit_buy )
      return;
   obj = ch->in_room->first_content;
   count = 0;
   sold = 0;
   num = number_range( 1, 30 );
   while( obj )
   {
      obj_next = obj->next_content;
      count++;
      if( num == count )
      {
         if( obj->cost <= 0 )
            return;
         if( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
         {
            separate_obj( obj );
            shop->gold += get_cost( ch, shop, obj, TRUE );
            extract_obj( obj );
            sold++;
         }
      }
      num = number_range( count, count + 30 );
      obj = obj_next;
   }
   if( sold )
   {
      sprintf( buf, "The customer buys %d item%s.", sold, sold > 1 ? "s" : "" );
      act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
      if( str_cmp( shop->owner, "Keolah" )  &&str_cmp( shop->owner, "(for sale)" ) )
      {
         fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      }
   }
}
void do_mpshowtalents( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( argument[0] == '\0' )
      
   {
      progbug( "Mpshowtalents - Bad syntax", ch );
      return;
   }
   if( ( victim = get_char_room( ch, argument ) ) == NULL )
      
   {
      progbug( "Mpshowtalents - Victim not there", ch );
      return;
   }
   if( IS_NPC( victim ) )
      
   {
      progbug( "Mpshowtalents - Victim is NPC", ch );
      return;
   }
   if( !IS_NPC( victim ) )
   {
      DEITY_DATA * talent;
      if( IS_SET( victim->pcdata->flags, PCFLAG_MASK ) )
      {
         send_to_char( "You see nothing but a distorted fog.\n\r", victim );
         return;
      }
      for( talent = first_deity; talent; talent = talent->next )
      {
         if( victim->talent[talent->index] > 0 )
         {
            if( xIS_SET( victim->act, PLR_SHOW_EXP )  &&victim->curr_talent[talent->index] < 200 )
            {
               pager_printf_color( victim, "&c%-12s: &w%-3d of %-3d, effectivity %-3d, %-10d of %-10d experience\n\r",
                                    talent->name, victim->curr_talent[talent->index], victim->talent[talent->index],
                                    TALENT( victim, talent->index ), victim->pcdata->talent_exp[talent->index],
                                    talent_exp( victim, victim->curr_talent[talent->index] + 1, talent->index ) );
            }
            else
            {
               pager_printf_color( victim, "&c%-12s: &wskill %-3d, power %-3d, effectivity %-3d\n\r", talent->name,
                                    victim->curr_talent[talent->index], victim->talent[talent->index], TALENT( victim,
                                                                                                                  talent->
                                                                                                                  index ) );
            }
         }
      }
   }
}


/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  */ 
void do_mpmload( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA * pMobIndex;
   CHAR_DATA * victim;
   one_argument( argument, arg );
   if( arg[0] == '\0' || !is_number( arg ) )
      
   {
      progbug( "Mpmload - Bad vnum as arg", ch );
      return;
   }
   if( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
      
   {
      progbug( "Mpmload - Bad mob vnum", ch );
      return;
   }
   victim = create_mobile( pMobIndex );
   char_to_room( victim, ch->in_room );
   return;
}
void do_mpoload( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA * pObjIndex;
   OBJ_DATA * obj;
   int timer = 0;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' || !is_number( arg1 ) )
      
   {
      progbug( "Mpoload - Bad syntax", ch );
      return;
   }
   
      /*
       * 
       * * New feature from Thoric.
       */ 
      timer = atoi( argument );
   if( timer < 0 )
      
   {
      progbug( "Mpoload - Bad timer", ch );
      return;
   }
   if( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
      
   {
      progbug( "Mpoload - Bad vnum arg", ch );
      return;
   }
   obj = create_object( pObjIndex, 0 );
   obj->timer = timer;
   if( !IS_OBJ_STAT( obj, ITEM_NO_TAKE ) )
      obj_to_char( obj, ch );
   
   else
      obj_to_room( obj, ch->in_room );
   return;
}


/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MUDprogram
   otherwise ugly stuff will happen */ 
void do_mppurge( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   OBJ_DATA * obj;
   ROOM_INDEX_DATA * pRoomIndex;
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      
         /*
          * 'purge' 
          */ 
         CHAR_DATA * vnext;
      for( victim = ch->in_room->first_person; victim; victim = vnext )
         
      {
         vnext = victim->next_in_room;
         if( IS_NPC( victim ) && victim != ch )
            extract_char( victim, TRUE );
      }
      while( ch->in_room->first_content )
      {
         if( IS_OBJ_STAT( ch->in_room->first_content, ITEM_ARTIFACT ) )
         {
            obj = ch->in_room->first_content;
            for( ;; )
               
            {
               pRoomIndex = get_room_index( number_range( 0, 1048576000 ) );
               if( pRoomIndex )
                  if( !IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE ) 
                       &&!IS_SET( pRoomIndex->area->flags, AFLAG_NOTELEPORT ) 
                       &&!IS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
                     break;
            }
            obj_from_room( obj );
            obj_to_room( obj, pRoomIndex );
         }
         else
         {
            extract_obj( ch->in_room->first_content );
         }
      }
      return;
   }
   if( ( victim = get_char_room( ch, arg ) ) == NULL )
      
   {
      if( ( obj = get_obj_here( ch, arg ) ) != NULL )
         extract_obj( obj );
      
      else
         progbug( "Mppurge - Bad argument", ch );
      return;
   }
   if( !IS_NPC( victim ) )
      
   {
      progbug( "Mppurge - Trying to purge a PC", ch );
      return;
   }
   if( victim == ch )
      
   {
      progbug( "Mppurge - Trying to purge oneself", ch );
      return;
   }
   if( IS_NPC( victim ) && victim->pIndexData->vnum == 3 )
      
   {
      progbug( "Mppurge: trying to purge supermob", ch );
      return;
   }
   while( ( obj = ch->last_carrying ) != NULL )
      extract_obj( obj );
   extract_char( victim, TRUE );
   return;
}


/* Allow mobiles to go wizinvis with programs -- SB */ 
void do_mpinvis( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   sh_int level;
   argument = one_argument( argument, arg );
   if( arg != NULL && arg[0] != '\0' )
      
   {
      if( !is_number( arg ) )
         
      {
         progbug( "Mpinvis - Non numeric argument ", ch );
         return;
      }
      level = atoi( arg );
      ch->mobinvis = level;
      ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
      return;
   }
   if( xIS_SET( ch->act, ACT_MOBINVIS ) )
      
   {
      xREMOVE_BIT( ch->act, ACT_MOBINVIS );
      act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\n\r", ch );
   }
   
   else
      
   {
      xSET_BIT( ch->act, ACT_MOBINVIS );
      act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\n\r", ch );
   }
   return;
}


/* lets the mobile goto any location it wishes that is not private */ 
/* Mounted chars follow their mobiles now - Blod, 11/97 */ 
void do_mpgoto( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA * location;
   CHAR_DATA * fch;
   CHAR_DATA * fch_next;
   ROOM_INDEX_DATA * in_room;
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      progbug( "Mpgoto - No argument", ch );
      return;
   }
   if( ( location = find_location( ch, arg ) ) == NULL )
      
   {
      progbug( "Mpgoto - No such location", ch );
      return;
   }
   in_room = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   for( fch = in_room->first_person; fch; fch = fch_next )
      
   {
      fch_next = fch->next_in_room;
      if( fch->mount && fch->mount == ch )
         
      {
         char_from_room( fch );
         char_to_room( fch, location );
      }
   }
   return;
}


/* lets the mobile do a command at another location. Very useful */ 
void do_mpat( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA * location;
   ROOM_INDEX_DATA * original;
   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
      
   {
      progbug( "Mpat - Bad argument", ch );
      return;
   }
   if( ( location = find_location( ch, arg ) ) == NULL )
      
   {
      sprintf( arg, "Mpat - No such location: \'%s\'", argument );
      bug( arg );
      
         /*
          * progbug( "Mpat - No such location", ch ); 
          */ 
         return;
   }
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


/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location 
   the area argument transfers everyone in the current area to the
   specified location */ 
void do_mptransfer( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA * location;
   CHAR_DATA * victim;
   CHAR_DATA * nextinroom;
   DESCRIPTOR_DATA * d;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
      
   {
      progbug( "Mptransfer - Bad syntax", ch );
      return;
   }
   
      /*
       * Put in the variable nextinroom to make this work right. -Narn 
       */ 
      if( !str_cmp( arg1, "all" ) )
      
   {
      for( victim = ch->in_room->first_person; victim; victim = nextinroom )
         
      {
         nextinroom = victim->next_in_room;
         if( victim != ch  &&can_see( ch, victim ) )
            
         {
            sprintf( buf, "%s %s", victim->name, arg2 );
            do_mptransfer( ch, buf );
         }
      }
      return;
   }
   
      /*
       * This will only transfer PC's in the area not Mobs --Shaddai 
       */ 
      if( !str_cmp( arg1, "area" ) )
      
   {
      for( d = first_descriptor; d; d = d->next )
         
      {
         if( !d->character || ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
              || !can_see( ch, d->character )  ||ch->in_room->area != d->character->in_room->area )
            continue;
         sprintf( buf, "%s %s", d->character->name, arg2 );
         do_mptransfer( ch, buf );
      }
      return;
   }
   
      /*
       * 
       * * Thanks to Grodyn for the optional location parameter.
       */ 
      if( arg2[0] == '\0' )
      
   {
      location = ch->in_room;
   }
   
   else
      
   {
      if( ( location = find_location( ch, arg2 ) ) == NULL )
         
      {
         progbug( "Mptransfer - No such location", ch );
         return;
      }
      if( room_is_private( location ) )
         
      {
         progbug( "Mptransfer - Private room", ch );
         return;
      }
   }
   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      
   {
      progbug( "Mptransfer - No such person", ch );
      return;
   }
   if( !victim->in_room )
      
   {
      progbug( "Mptransfer - Victim in Limbo", ch );
      return;
   }
   char_from_room( victim );
   char_to_room( victim, location );
   return;
}


/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */ 
void do_mpforce( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg );
   if( arg[0] == '\0' || argument[0] == '\0' )
      
   {
      progbug( "Mpforce - Bad syntax", ch );
      return;
   }
   if( !str_cmp( arg, "all" ) )
      
   {
      CHAR_DATA * vch;
      for( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
         if( can_see( ch, vch ) )
            interpret( vch, argument, TRUE );
   }
   
   else
      
   {
      CHAR_DATA * victim;
      if( ( victim = get_char_room( ch, arg ) ) == NULL )
         
      {
         progbug( "Mpforce - No such victim", ch );
         return;
      }
      if( victim == ch )
         
      {
         progbug( "Mpforce - Forcing oneself", ch );
         return;
      }
      interpret( victim, argument, TRUE );
   }
   return;
}
void do_mpechozone( CHAR_DATA * ch, char *argument ) /* Blod, late 97 */  
{
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA * vch;
   CHAR_DATA * vch_next;
   sh_int color;
   EXT_BV actflags;
   if( argument[0] == '\0' )
   {
      progbug( "Mpechozone - called w/o argument", ch );
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT( ch->act, ACT_SECRETIVE );
   if( ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg1 );
   for( vch = first_char; vch; vch = vch_next )
      
   {
      vch_next = vch->next;
      if( vch->in_room->area == ch->in_room->area  &&!IS_NPC( vch )  &&IS_AWAKE( vch ) )
         
      {
         if( color )
            act( color, argument, vch, NULL, NULL, TO_CHAR );
         
         else
            act( AT_ACTION, argument, vch, NULL, NULL, TO_CHAR );
      }
   }
   ch->act = actflags;
}


/*
 *  Haus' toys follow:
 */ 
   
/*
 * syntax:  mppractice victim spell_name max%
 *
 */ 
void do_mp_practice( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   int sn, max, tmp, adept;
   char *skill_name;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
      
   {
      send_to_char( "Mppractice: bad syntax", ch );
      progbug( "Mppractice - Bad syntax", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      
   {
      send_to_char( "Mppractice: Student not in room? Invis?", ch );
      progbug( "Mppractice: Invalid student not in room", ch );
      return;
   }
   if( ( sn = skill_lookup( arg2 ) ) < 0 )
      
   {
      send_to_char( "Mppractice: Invalid spell/skill name", ch );
      progbug( "Mppractice: Invalid spell/skill name", ch );
      return;
   }
   if( IS_NPC( victim ) )
      
   {
      send_to_char( "Mppractice: Can't train a mob", ch );
      progbug( "Mppractice: Can't train a mob", ch );
      return;
   }
   skill_name = skill_table[sn]->name;
   max = atoi( arg3 );
   if( ( max < 0 ) || ( max > 100 ) )
      
   {
      sprintf( log_buf, "mp_practice: Invalid maxpercent: %d", max );
      send_to_char( log_buf, ch );
      progbug( log_buf, ch );
      return;
   }
   if( get_best_talent( victim, sn ) == -1 )
      
   {
      sprintf( buf, "$n attempts to tutor you in %s, but it's beyond your comprehension.", skill_name );
      act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
      return;
   }
   
      /*
       * adept is how high the player can learn it 
       */ 
      adept = GET_ADEPT( victim, sn );
   if( ( LEARNED( victim, sn ) >= adept )  ||( LEARNED( victim, sn ) >= max ) )
      
   {
      sprintf( buf, "$n shows some knowledge of %s, but yours is clearly superior.", skill_name );
      act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
      return;
   }
   
      /*
       * past here, victim learns something 
       */ 
      tmp = UMIN( victim->pcdata->learned[sn] + get_curr_int( victim ) / 4, max );
   act( AT_ACTION, "$N demonstrates $t to you.  You feel more learned in this subject.", victim, skill_table[sn]->name, ch,
         TO_CHAR );
   victim->pcdata->learned[sn] = max;
   if( LEARNED( victim, sn ) >= adept )
      
   {
      victim->pcdata->learned[sn] = adept;
      act( AT_TELL, "$n tells you, 'You have learned all I know on this subject...'", ch, NULL, victim, TO_VICT );
   }
   return;
}
void do_mpscatter( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   CHAR_DATA * victim;
   ROOM_INDEX_DATA * pRoomIndex;
   int low_vnum, high_vnum, rvnum;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
   {
      send_to_char( "Mpscatter whom?\n\r", ch );
      progbug( "Mpscatter: invalid (nonexistent?) argument", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpscatter: victim not in room", ch );
      return;
   }
   if( arg2[0] == '\0' )
   {
      send_to_char( "You must specify a low vnum.\n\r", ch );
      progbug( "Mpscatter:  missing low vnum", ch );
      return;
   }
   if( argument[0] == '\0' )
   {
      send_to_char( "You must specify a high vnum.\n\r", ch );
      progbug( "Mpscatter:  missing high vnum", ch );
      return;
   }
   low_vnum = atoi( arg2 );
   high_vnum = atoi( argument );
   if( low_vnum < 1 || high_vnum < low_vnum || low_vnum > high_vnum || low_vnum == high_vnum || high_vnum > 1048576000 )
   {
      send_to_char( "Invalid range.\n\r", ch );
      progbug( "Mpscatter:  invalid range", ch );
      return;
   }
   while( 1 )
   {
      rvnum = number_range( low_vnum, high_vnum );
      pRoomIndex = get_room_index( rvnum );
      if( pRoomIndex )
         break;
   }
   char_from_room( victim );
   char_to_room( victim, pRoomIndex );
   victim->position = POS_RESTING;
   do_look( victim, "auto" );
   return;
}


/*
 * syntax: mpslay (character)
 */ 
void do_mp_slay( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
      
   {
      send_to_char( "mpslay whom?\n\r", ch );
      progbug( "Mpslay: invalid (nonexistent?) argument", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      
   {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpslay: victim not in room", ch );
      return;
   }
   if( victim == ch )
      
   {
      send_to_char( "You try to slay yourself.  You fail.\n\r", ch );
      progbug( "Mpslay: trying to slay self", ch );
      return;
   }
   if( IS_NPC( victim ) && victim->pIndexData->vnum == 3 )
      
   {
      send_to_char( "You cannot slay supermob!\n\r", ch );
      progbug( "Mpslay: trying to slay supermob", ch );
      return;
   }
   act( AT_IMMORT, "You slay $M in cold blood!", ch, NULL, victim, TO_CHAR );
   act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT );
   act( AT_IMMORT, "$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT );
   set_cur_char( victim );
   die( victim );
   return;
}


/*
 * syntax: mpdamage (character) (#hps)
 */ 
void do_mp_damage( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   CHAR_DATA * nextinroom;
   char buf[MAX_STRING_LENGTH];
   int dam;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
      
   {
      send_to_char( "mpdamage whom?\n\r", ch );
      progbug( "Mpdamage: invalid argument1", ch );
      return;
   }
   
/* Am I asking for trouble here or what?  But I need it. -- Blodkai */ 
      if( !str_cmp( arg1, "all" ) )
      
   {
      for( victim = ch->in_room->first_person; victim; victim = nextinroom )
         
      {
         nextinroom = victim->next_in_room;
         if( victim != ch 
              &&( !IS_NPC( victim ) 
                  ||!xIS_SET( victim->act, ACT_PACIFIST ) )  &&can_see( ch, victim ) /* Could go either way */  
              &&!char_died( victim ) ) /* prevent nasty double deaths */
            
         {
            sprintf( buf, "'%s' %s", victim->name, arg2 );
            do_mp_damage( ch, buf );
         }
      }
      return;
   }
   if( arg2[0] == '\0' )
      
   {
      send_to_char( "mpdamage inflict how many hps?\n\r", ch );
      progbug( "Mpdamage: invalid argument2", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      
   {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpdamage: victim not in room", ch );
      return;
   }
   if( victim == ch )
      
   {
      send_to_char( "You can't mpdamage yourself.\n\r", ch );
      return;
   }
   dam = atoi( arg2 );
   if( ( dam < 0 ) || ( dam > 32000 ) )
      
   {
      send_to_char( "Mpdamage how much?\n\r", ch );
      progbug( "Mpdamage: invalid (nonexistent?) argument", ch );
      return;
   }
   
      /*
       * this is kinda begging for trouble        
       */ 
      /*
       * 
       * * Note from Thoric to whoever put this in...
       * * Wouldn't it be better to call damage(ch, ch, dam, dt)?
       * * I hate redundant code
       * *
       * * changed to use damage() on Nov 21 2000 by Keolah,
       * * who hates redundant code and doesnt care to keep two
       * * functions that do the same thing in sync
       */ 
      lose_hp( victim, dam );
}


/*
 * syntax: mprestore (character) (#hps)                Gorog
 */ 
void do_mp_restore( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   int hp;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == '\0' )
      
   {
      send_to_char( "mprestore whom?\n\r", ch );
      progbug( "Mprestore: invalid argument1", ch );
      return;
   }
   if( arg2[0] == '\0' )
      
   {
      send_to_char( "mprestore how many hps?\n\r", ch );
      progbug( "Mprestore: invalid argument2", ch );
      return;
   }
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
      
   {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mprestore: victim not in room", ch );
      return;
   }
   hp = atoi( arg2 );
   if( ( hp < 0 ) || ( hp > 32000 ) )
      
   {
      send_to_char( "Mprestore how much?\n\r", ch );
      progbug( "Mprestore: invalid (nonexistent?) argument", ch );
      return;
   }
   hp += victim->hit;
   victim->hit = ( hp > 32000 || hp < 0 || hp > victim->max_hit ) ? victim->max_hit : hp;
}


/*
 * Syntax mp_open_passage x y z
 *
 * opens a 1-way passage from room x to room y in direction z
 *
 *  won't mess with existing exits
 */ 
void do_mp_open_passage( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA * targetRoom, *fromRoom;
   int targetRoomVnum, fromRoomVnum, exit_num;
   EXIT_DATA * pexit;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   if( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
      
   {
      progbug( "MpOpenPassage - Bad syntax", ch );
      return;
   }
   if( !is_number( arg1 ) )
      
   {
      progbug( "MpOpenPassage - Bad syntax", ch );
      return;
   }
   fromRoomVnum = atoi( arg1 );
   if( ( fromRoom = get_room_index( fromRoomVnum ) ) == NULL )
      
   {
      progbug( "MpOpenPassage - Bad syntax", ch );
      return;
   }
   if( !is_number( arg2 ) )
      
   {
      progbug( "MpOpenPassage - Bad syntax", ch );
      return;
   }
   targetRoomVnum = atoi( arg2 );
   if( ( targetRoom = get_room_index( targetRoomVnum ) ) == NULL )
      
   {
      progbug( "MpOpenPassage - Bad syntax", ch );
      return;
   }
   if( !is_number( arg3 ) )
      
   {
      progbug( "MpOpenPassage - Bad syntax", ch );
      return;
   }
   exit_num = atoi( arg3 );
   if( ( exit_num < 0 ) || ( exit_num > MAX_DIR ) )
      
   {
      progbug( "MpOpenPassage - Bad syntax", ch );
      return;
   }
   if( ( pexit = get_exit( fromRoom, exit_num ) ) != NULL )
      
   {
      if( !IS_SET( pexit->exit_info, EX_PASSAGE ) )
         return;
      progbug( "MpOpenPassage - Exit exists", ch );
      return;
   }
   pexit = make_exit( fromRoom, targetRoom, exit_num );
   pexit->keyword = STRALLOC( "" );
   pexit->description = STRALLOC( "" );
   pexit->key = -1;
   pexit->exit_info = EX_PASSAGE;
   
      /*
       * act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR ); 
       */ 
      /*
       * act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM ); 
       */ 
      return;
}


/*
 * Syntax mp_close_passage x y 
 *
 * closes a passage in room x leading in direction y
 *
 * the exit must have EX_PASSAGE set
 */ 
void do_mp_close_passage( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA * fromRoom;
   int fromRoomVnum, exit_num;
   EXIT_DATA * pexit;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   if( arg1[0] == '\0' || arg2[0] == '\0' || arg2[0] == '\0' )
      
   {
      progbug( "MpClosePassage - Bad syntax", ch );
      return;
   }
   if( !is_number( arg1 ) )
      
   {
      progbug( "MpClosePassage - Bad syntax", ch );
      return;
   }
   fromRoomVnum = atoi( arg1 );
   if( ( fromRoom = get_room_index( fromRoomVnum ) ) == NULL )
      
   {
      progbug( "MpClosePassage - Bad syntax", ch );
      return;
   }
   if( !is_number( arg2 ) )
      
   {
      progbug( "MpClosePassage - Bad syntax", ch );
      return;
   }
   exit_num = atoi( arg2 );
   if( ( exit_num < 0 ) || ( exit_num > MAX_DIR ) )
      
   {
      progbug( "MpClosePassage - Bad syntax", ch );
      return;
   }
   if( ( pexit = get_exit( fromRoom, exit_num ) ) == NULL )
      
   {
      return; /* already closed, ignore...  so rand_progs */
      
         /*
          * can close without spam 
          */ 
   }
   if( !IS_SET( pexit->exit_info, EX_PASSAGE ) )
      
   {
      progbug( "MpClosePassage - Exit not a passage", ch );
      return;
   }
   extract_exit( fromRoom, pexit );
   
      /*
       * act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_CHAR ); 
       */ 
      /*
       * act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_ROOM ); 
       */ 
      return;
}


/*
 * Does nothing.  Used for scripts.
 */ 
void do_mpnothing( CHAR_DATA * ch, char *argument ) 
{
   return;
}


/* Load a raw material into the game -- Scion */ 
void do_mpmakeore( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA * obj;
   MATERIAL_DATA * material;
   AREA_DATA * area;
   AFFECT_DATA * paf;
   AFFECT_DATA * oaf;
   char buf[MAX_STRING_LENGTH];
   int hi_vnum = 2;
   int i = -1;
   if( !is_number( argument ) )
   {
      progbug( "Mpmakeore: Argument not a number", ch );
      return;
   }
   i = atoi( argument );
   material = material_lookup( i );
   if( !material )
   {
      progbug( "Mpmakeore: Material not found", ch );
      return;
   }
   for( area = first_area; area; area = area->next )
   {
      if( !str_cmp( area->filename, "stockobj.are" ) )
      {
         hi_vnum = area->hi_o_vnum;
         break;
      }
      else
      {
         hi_vnum = 2;
      }
   }
   if( hi_vnum == 2 )
   {
      progbug( "Mpmakeore: No material in game", ch );
      return;
   }
   obj = create_object( get_obj_index( hi_vnum ), 0 );
   sprintf( buf, "%s _material_", material->name );
   obj->name = STRALLOC( buf );
   obj->short_descr = STRALLOC( material->short_descr );
   obj->description = STRALLOC( material->description );
   obj->weight = number_fuzzy( material->weight );
   obj->cost = number_fuzzy( material->cost );
   obj->extra_flags = material->extra_flags;
   obj->value[0] = material->number;
   obj->material = material;
   obj->timer = 10;
   xSET_BIT( obj->extra_flags, ITEM_GROUNDROT );
   for( paf = material->first_affect; paf; paf = paf->next )
   {
      CREATE( oaf, AFFECT_DATA, 1 );
      oaf->type = paf->type;
      oaf->duration = paf->duration;
      oaf->location = paf->location;
      oaf->modifier = paf->modifier;
      xCLEAR_BITS( oaf->bitvector );
      oaf->next = NULL;
      LINK( oaf, obj->first_affect, obj->last_affect, next, prev );
      ++top_affect;
   }
   obj_to_char( obj, ch );
}


/*
 *   Sends a message to sleeping character.  Should be fun
 *    with room sleep_progs
 *
 */ 
void do_mpdream( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_STRING_LENGTH];
   CHAR_DATA * vict;
   argument = one_argument( argument, arg1 );
   if( ( vict = get_char_world( ch, arg1 ) ) == NULL )
      
   {
      progbug( "Mpdream: No such character", ch );
      return;
   }
   if( vict->position <= POS_SLEEPING )
      
   {
      send_to_char( argument, vict );
      send_to_char( "\n\r", vict );
   }
   return;
}


/*
 * Deposit some gold into the current area's economy		-Thoric
 */ 
void do_mp_deposit( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_STRING_LENGTH];
   int gold;
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      progbug( "Mpdeposit - bad syntax", ch );
      return;
   }
   gold = atoi( arg );
   if( gold <= ch->gold && ch->in_room )
      
   {
      ch->gold -= gold;
      boost_economy( ch->in_room->area, gold );
   }
}


/*
 * Withdraw some gold from the current area's economy		-Thoric
 */ 
void do_mp_withdraw( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_STRING_LENGTH];
   int gold;
   one_argument( argument, arg );
   if( arg[0] == '\0' )
      
   {
      progbug( "Mpwithdraw - bad syntax", ch );
      return;
   }
   gold = atoi( arg );
   if( ch->gold < 1000000000 && gold < 1000000000 && ch->in_room  &&economy_has( ch->in_room->area, gold ) )
      
   {
      ch->gold += gold;
      lower_economy( ch->in_room->area, gold );
   }
}
void do_mpdelay( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   int delay;
   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Delay for how many rounds?n\r", ch );
      progbug( "Mpdelay: no duration specified", ch );
      return;
   }
   if( !( victim = get_char_room( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\n\r", ch );
      progbug( "Mpdelay: target not in room", ch );
      return;
   }
   argument = one_argument( argument, arg );
   if( !*arg || !is_number( arg ) )
   {
      send_to_char( "Delay them for how many rounds?\n\r", ch );
      progbug( "Mpdelay: invalid (nonexistant?) argument", ch );
      return;
   }
   delay = atoi( arg );
   if( delay < 1 || delay > 30 )
   {
      send_to_char( "Argument out of range.\n\r", ch );
      progbug( "Mpdelay:  argument out of range (1 to 30)", ch );
      return;
   }
   WAIT_STATE( victim, delay * PULSE_VIOLENCE );
   send_to_char( "Mpdelay applied.\n\r", ch );
   return;
}
void do_mppeace( CHAR_DATA * ch, char *argument ) 
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * rch;
   CHAR_DATA * victim;
   argument = one_argument( argument, arg );
   if( !*arg )
   {
      send_to_char( "Who do you want to mppeace?\n\r", ch );
      progbug( "Mppeace: invalid (nonexistent?) argument", ch );
      return;
   }
   if( !str_cmp( arg, "all" ) )
   {
      for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      {
         rch->last_hit = NULL;
         stop_hating( rch );
         stop_hunting( rch );
         stop_fearing( rch );
      }
      send_to_char( "Ok.\n\r", ch );
      return;
   }
      if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They must be in the room.n\r", ch );
      progbug( "Mppeace: target not in room", ch );
      return;
   }
   stop_hating( ch );
   stop_hunting( ch );
   stop_fearing( ch );
   stop_hating( victim );
   stop_hunting( victim );
   stop_fearing( victim );
   send_to_char( "Ok.\n\r", ch );
   return;
}


