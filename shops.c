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
 *			 Shop and repair shop module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
#include <dirent.h>
#endif /*  */
#include "mud.h"
   
/*
 * Local functions
 */ 
   
#define	CD	CHAR_DATA
int get_cost args( ( CHAR_DATA * ch, SHOP_DATA * pShop, OBJ_DATA * obj, bool fBuy ) );

#undef CD
   
/* handler.c */ 
extern MAT_SHOP *find_matshop( int i );
extern MATERIAL_DATA *material_lookup( int number );
extern void learn_noncombat( CHAR_DATA * ch, int i );

/* makeobjs.c */ 
extern OBJ_DATA *make_ore( int i );
int get_cost( CHAR_DATA * ch, SHOP_DATA * pShop, OBJ_DATA * obj, bool fBuy ) 
{
   if( obj )
   {
      if( fBuy )
      {
         
            /*
             * Low cost items need the precision,
             * * but high cost items need to not overflow MAXINT -
             * * items over 21 mithril could not be sold before
             */ 
            if( obj->cost < 1000000 )
            return ( obj->cost * pShop->profit_sell ) / 100;
         
         else
            return ( obj->cost / 100 ) * pShop->profit_sell;
      }
      else
      {
         return obj->cost;
      }
   }
   
   else
      return 0;
}
void do_buy( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int maxgold;
   OBJ_DATA * obj;
   int cost;
   int noi = 1;  /* Number of items */
   sh_int mnoi = 100;  /* Max number of items to be bought at once */
   MAT_SHOP * matshop;
   MATERIAL_DATA * mat;
   int i;
   argument = one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Buy what?\n\r", ch );
      return;
   }
   if( ( matshop = find_matshop( ch->in_room->vnum ) ) != NULL )
   {
      for( i = 0; i < 5; i++ )
      {
         mat = material_lookup( matshop->mat[i] );
         if( !mat )
            continue;
         if( !str_cmp( arg, mat->name )  ||!str_prefix( arg, mat->name ) )
         {
            if( ch->gold < mat->cost )
            {
               send_to_char( "You don't have enough gold.\n\r", ch );
               return;
            }
            if( ch->carry_number + 1 > can_carry_n( ch ) )
            {
               send_to_char( "You have your hands full.\n\r", ch );
               return;
            }
            if( ch->carry_weight + mat->weight > can_carry_w( ch ) )
            {
               send_to_char( "You can't carry that much weight.\n\r", ch );
               return;
            }
            ch->gold -= mat->cost;
            boost_economy( ch->in_room->area, mat->cost );
            obj = make_ore( matshop->mat[i] );
            obj_to_char( obj, ch );
            ch_printf( ch, "You buy %s.\n\r", mat->short_descr );
            act( AT_ACTION, "$n buys $p.", ch, obj, NULL, TO_ROOM );
            return;
         }
      }
      return;
   }
   if( !ch->in_room->pShop )
   {
      send_to_char( "This isn't a shop.\n\r", ch );
      return;
   }
   if( !str_cmp( ch->in_room->pShop->owner, "(for sale)" ) )
   {
      if( !str_cmp( arg, "shop" ) )
      {
         if( ch->gold < 1000000 )
         {
            send_to_char( "You can't afford it.\n\r", ch );
            return;
         }
         act( AT_ACTION, "$n purchases this shop.", ch, NULL, NULL, TO_ROOM );
         send_to_char( "You purchase this shop for &C1 mithril.\n\r", ch );
         ch->gold -= 1000000;
         STRFREE( ch->in_room->pShop->owner );
         ch->in_room->pShop->owner = STRALLOC( ch->name );
         fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
         return;
      }
   }
   maxgold = ch->in_room->pShop->gold;
   if( is_number( arg ) )
   {
      noi = atoi( arg );
      argument = one_argument( argument, arg );
      if( noi > mnoi )
      {
         send_to_char( "You can't buy that many items at once.\n\r", ch );
         return;
      }
   }
   obj = get_obj_list( ch, arg, ch->in_room->first_content );
   if( !obj )
   {
      send_to_char( "There is no such item here.\n\r", ch );
      return;
   }
   cost = ( get_cost( ch, ch->in_room->pShop, obj, TRUE ) * noi );
   if( obj && IS_AFFECTED( ch, AFF_HAS_ARTI ) && IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      act( AT_ACTION, "$p evades your grasp.", ch, obj, NULL, TO_CHAR );
      act( AT_ACTION, "$p evades $n's grasp.", ch, obj, NULL, TO_ROOM );
      return;
   }
   if( IS_AFFECTED( ch, AFF_ETHEREAL ) )
   {
      act( AT_ACTION, "Your hand passes through $p.", ch, obj, NULL, TO_CHAR );
      act( AT_ACTION, "$n's hand passes through $p.", ch, obj, NULL, TO_ROOM );
      return;
   }
   if( cost <= 0 || !can_see_obj( ch, obj ) )
   {
      send_to_char( "That item isn't sold here. Try 'list'.\n\r", ch );
      return;
   }
   if( noi > obj->count && !IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
   {
      send_to_char( "There aren't enough of that item here to buy that many.\n\r", ch );
      return;
   }
   if( ch->gold < cost )
   {
      send_to_char( "You can't afford to buy that.\n\r", ch );
      return;
   }
   if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
   {
      send_to_char( "That is only a prototype, you can't buy that.\n\r", ch );
      return;
   }
   separate_obj( obj );
   if( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
      
   {
      send_to_char( "You can't carry that many items.\n\r", ch );
      return;
   }
   if( ch->carry_weight + ( get_obj_weight( obj ) * noi )  +( noi > 1 ? 2 : 0 ) > can_carry_w( ch ) )
      
   {
      send_to_char( "You can't carry that much weight.\n\r", ch );
      return;
   }
   ch->gold -= cost;
   if( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
      ch->in_room->pShop->gold += cost;
   if( noi == 1 )
   {
      act( AT_ACTION, "$n buys $p.", ch, obj, NULL, TO_ROOM );
      act( AT_ACTION, "You buy $p.", ch, obj, NULL, TO_CHAR );
      if( str_cmp( ch->in_room->pShop->owner, "Keolah" ) )
      {
         
            /*
             * If the shop is owned by a player, make sure their
             * * profits are saved
             */ 
            fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
         WAIT_STATE( ch, PULSE_VIOLENCE );
      }
   }
   else
   {
      sprintf( arg, "$n buys %d $p%s.", noi, ( obj->short_descr[strlen( obj->short_descr ) - 1] == 's'  ? "" : "s" ) );
      act( AT_ACTION, arg, ch, obj, NULL, TO_ROOM );
      sprintf( arg, "You buy %d $p%s.", noi, ( obj->short_descr[strlen( obj->short_descr ) - 1] == 's'  ? "" : "s" ) );
      act( AT_ACTION, arg, ch, obj, NULL, TO_CHAR );
      act( AT_ACTION, "A clerk puts them into a bag and hands it to you.", ch, NULL, NULL, TO_CHAR );
   }
   if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
   {
      OBJ_DATA * buy_obj, *bag;
      buy_obj = create_object( obj->pIndexData, 0 );
      
         /*
          * 
          * * Due to grouped objects and carry limitations in SMAUG
          * * The shopkeeper gives you a bag with multiple-buy,
          * * and also, only one object needs be created with a count
          * * set to the number bought.     -Thoric
          */ 
         if( noi > 1 )
      {
         bag = create_object( get_obj_index( OBJ_VNUM_SHOPPING_BAG ), 1 );
         xSET_BIT( bag->extra_flags, ITEM_GROUNDROT );
         bag->timer = 10; /* Blodkai, 4/97 */
         
            /*
             * perfect size bag ;) 
             */ 
            bag->value[0] = bag->weight + ( buy_obj->weight * noi );
         buy_obj->count = noi;
         obj->pIndexData->count += ( noi - 1 );
         numobjsloaded += ( noi - 1 );
         obj_to_obj( buy_obj, bag );
         obj_to_char( bag, ch );
      }
      else
         obj_to_char( buy_obj, ch );
   }
   else
   {
      obj_from_room( obj );
      obj_to_char( obj, ch );
   }
   return;
}
void do_list( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *rest;
   OBJ_DATA * obj;
   int cost, gp, count;
   MAT_SHOP * matshop;
   int i;
   MATERIAL_DATA * mat;
   rest = one_argument( argument, arg );
   if( ( matshop = find_matshop( ch->in_room->vnum ) ) != NULL )
   {
      for( i = 0; i < 5; i++ )
      {
         mat = material_lookup( matshop->mat[i] );
         if( !mat )
            continue;
         gp = mat->cost;
         pager_printf( ch, "[&C%dm ", ( int )gp / 1000000 );
         gp = gp % 1000000;
         pager_printf( ch, "&Y%dg ", ( int )gp / 10000 );
         gp = gp % 10000;
         pager_printf( ch, "&W%ds ", ( int )gp / 100 );
         gp = gp % 100;
         pager_printf( ch, "&O%dc&w", gp );
         pager_printf( ch, "] - %s\n\r", mat->short_descr );
      } return;
   }
   if( !ch->in_room->pShop )
   {
      send_to_char( "This isn't a shop.\n\r", ch );
      return;
   }
   count = 0;
   for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
      
   {
      if( can_see_obj( ch, obj ) 
           &&( cost = get_cost( ch, ch->in_room->pShop, obj, TRUE ) ) > 0 
           &&( arg[0] == '\0' || nifty_is_name( arg, obj->name ) ) )
         
      {
         if( !count )
            send_to_pager( "[Price] Item\n\r", ch );
         count++;
         gp = get_cost( ch, ch->in_room->pShop, obj, TRUE );
         pager_printf( ch, "[&C%dm ", ( int )gp / 1000000 );
         gp = gp % 1000000;
         pager_printf( ch, "&Y%dg ", ( int )gp / 10000 );
         gp = gp % 10000;
         pager_printf( ch, "&W%ds ", ( int )gp / 100 );
         gp = gp % 100;
         pager_printf( ch, "&O%dc&w", gp );
         pager_printf( ch, "] %s.\n\r", capitalize( aoran( myobj( obj ) ) ) );
         if( count > 50 )
         {
            send_to_char( "There are many other items here besides these.\n\r", ch );
            break;
         }
      }
   }
   if( !count )
      
   {
      if( arg[0] == '\0' )
         send_to_char( "There is nothing for sale here.\n\r", ch );
      
      else
         send_to_char( "That isnt for sale here.\n\r", ch );
   }
   return;
}
void do_sell( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA * obj;
   int cost, gp;
   one_argument( argument, arg );
   if( !ch->in_room->pShop )
   {
      send_to_char( "This isn't a shop.\n\r", ch );
      return;
   }
   if( arg[0] == '\0' )
      
   {
      send_to_char( "Sell what?\n\r", ch );
      return;
   }
   if( !str_cmp( arg, "shop" )  &&!str_cmp( ch->name, ch->in_room->pShop->owner ) )
   {
      do_withdraw( ch, "" );
      act( AT_ACTION, "$n sells the deed to the shop.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You sell the shop to the city for &C1 mithril.\n\r", ch );
      ch->gold += 1000000;
      STRFREE( ch->in_room->pShop->owner );
      ch->in_room->pShop->owner = STRALLOC( "(for sale)" );
      fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      return;
   }
   if( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
      
   {
      
         /*
          * 'sell obj' 
          */ 
         if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
         
      {
         send_to_char( "You don't have that.\n\r", ch );
         return;
      }
      if( !can_drop_obj( ch, obj ) )
         
      {
         send_to_char( "You can't let go of it!\n\r", ch );
         return;
      }
      if( obj->timer > 0 )
         
      {
         send_to_char( "You should preserve that first.\n\r", ch );
         return;
      }
      if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
      {
         send_to_char( "The shop won't buy your imaginary items.\n\r", ch );
         return;
      }
      if( ( cost = get_cost( ch, ch->in_room->pShop, obj, FALSE ) ) <= 0 )
         
      {
         send_to_char( "The shop isn't interested.\r\n", ch );
         return;
      }
      if( ch->in_room->pShop->type != -1  &&ch->in_room->pShop->type != obj->item_type )
      {
         send_to_char( "This shop does not deal in this type of items.\n\r", ch );
         return;
      }
      if( ch->in_room->pShop->flag != -1  &&!xIS_SET( obj->extra_flags, ch->in_room->pShop->flag ) )
      {
         send_to_char( "This shop does not deal in items of this nature.\n\r", ch );
         return;
      }
      separate_obj( obj );
      if( !str_cmp( ch->in_room->pShop->owner, ch->name ) )
      {
         act( AT_ACTION, "$n puts $p up for sale.", ch, obj, NULL, TO_ROOM );
         ch_printf( ch, "You put %s up for sale.\n\r", aoran( myobj( obj ) ) );
         obj_from_char( obj );
         obj_to_room( obj, ch->in_room );
         return;
      }
      else if( ch->in_room->pShop->gold < obj->cost  &&str_cmp( ch->in_room->pShop->owner, "Keolah" ) )
      {
         send_to_char( "The shop cannot afford to buy this.\n\r", ch );
         return;
      }
      act( AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM );
      gp = get_cost( ch, ch->in_room->pShop, obj, FALSE );
      ch_printf( ch, "You sell %s for &C%dmithril, ", aoran( obj->short_descr ), ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%dgold, ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%dsilver, ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "and &O%dcopper&w coins.\n\r", gp );
      ch->gold += cost;
      ch->in_room->pShop->gold -= cost;
      obj_from_char( obj );
      obj_to_room( obj, ch->in_room );
   }
   else
   {
      char *chk;
      bool fAll, found;
      OBJ_DATA * obj_next;
      
         /*
          * 'sell all' or 'sell all.obj' 
          */ 
         if( !str_cmp( arg, "all" ) )
      {
         chk = arg;
         fAll = TRUE;
      }
      else
      {
         chk = &arg[4];
         fAll = FALSE;
      }
      found = FALSE;
      gp = 0;
      for( obj = ch->first_carrying; obj; obj = obj_next )
         
      {
         obj_next = obj->next_content;
         if( ( fAll || nifty_is_name_prefix( chk, obj->name ) ) 
               &&can_see_obj( ch, obj )  &&obj->wear_loc == WEAR_NONE  &&can_drop_obj( ch, obj ) )
            
         {
            if( obj->timer > 0 )
               continue;
            if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
               continue;
            if( ( cost = get_cost( ch, ch->in_room->pShop, obj, FALSE ) ) <= 0 )
               continue;
            if( ch->in_room->pShop->type != -1  &&ch->in_room->pShop->type != obj->item_type )
               continue;
            if( ch->in_room->pShop->flag != -1  &&!xIS_SET( obj->extra_flags, ch->in_room->pShop->flag ) )
               continue;
            if( ch->in_room->pShop->gold < obj->cost 
                  &&str_cmp( ch->in_room->pShop->owner, "Keolah" )  &&str_cmp( ch->in_room->pShop->owner, ch->name ) )
               continue;
            separate_obj( obj );
            found = TRUE;
            cost = get_cost( ch, ch->in_room->pShop, obj, FALSE );
            if( str_cmp( ch->in_room->pShop->owner, ch->name ) )
            {
               gp += cost;
               ch->in_room->pShop->gold -= cost;
            }
            obj_from_char( obj );
            obj_to_room( obj, ch->in_room );
         }
      }
      if( found )
      {
         if( !str_cmp( ch->in_room->pShop->owner, ch->name ) )
         {
            act( AT_ACTION, "$n puts a bunch of stuff up for sale.", ch, obj, NULL, TO_ROOM );
            send_to_char( "You put a bunch of stuff up for sale.\n\r", ch );
            return;
         }
         else
         {
            ch->gold += gp;
            act( AT_ACTION, "$n sells a bunch of stuff.", ch, NULL, NULL, TO_ROOM );
            ch_printf( ch, "You sell a bunch of stuff for &C%dmithril, ", ( int )gp / 1000000 );
            gp = gp % 1000000;
            ch_printf( ch, "&Y%dgold, ", ( int )gp / 10000 );
            gp = gp % 10000;
            ch_printf( ch, "&W%dsilver, ", ( int )gp / 100 );
            gp = gp % 100;
            ch_printf( ch, "and &O%dcopper&w coins.\n\r", gp );
            STRFREE( ch->last_taken );
            ch->last_taken = STRALLOC( "selling things" );
            WAIT_STATE( ch, PULSE_VIOLENCE );
            return;
         }
      }
      else
      {
         send_to_char( "You have nothing this shop can buy.\n\r", ch );
         return;
      }
   }
}
void do_value( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA * obj;
   int gp;
   if( argument[0] == '\0' )
      
   {
      send_to_char( "Value what?\n\r", ch );
      return;
   }
   if( !ch->in_room->pShop )
   {
      send_to_char( "This isn't a shop.\n\r", ch );
      return;
   }
   if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
      
   {
      send_to_char( "You don't have any such item.\n\r", ch );
      return;
   }
   if( ( gp = get_cost( ch, ch->in_room->pShop, obj, FALSE ) ) <= 0 )
      
   {
      send_to_char( "The shop isn't interested in that.\n\r", ch );
      return;
   }
   ch_printf( ch, "%s is worth &C%dmithril, ", aoran( obj->short_descr ), ( int )gp / 1000000 );
   gp = gp % 1000000;
   ch_printf( ch, "&Y%dgold, ", ( int )gp / 10000 );
   gp = gp % 10000;
   ch_printf( ch, "&W%dsilver, ", ( int )gp / 100 );
   gp = gp % 100;
   ch_printf( ch, "and &O%dcopper&w coins.\n\r", gp );
   return;
}
void do_repair( CHAR_DATA * ch, char *argument )
{
   PART_DATA * part;
   OBJ_DATA * obj;
   CHAR_DATA * victim = NULL;
   char buf[MAX_STRING_LENGTH];
   if( argument[0] == '\0' )
   {
      victim = ch;
   }
   if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
   {
      if( ( victim = get_char_room( ch, argument ) ) == NULL )
      {
         send_to_char( "You don't see that here.\n\r", ch );
         return;
      }
   }
   learn_noncombat( ch, SK_SMITH );
   if( number_percent(  ) * ( IS_AFFECTED( ch, AFF_NUISANCE ) ? 2 : 1 ) 
         >( IS_NPC( ch ) ? 100 : LEARNED( ch, gsn_repair ) ) )
   {
      if( obj )
      {
         sprintf( buf, "repairing the %s", myobj( obj ) );
         ch->last_taken = STRALLOC( buf );
         act( AT_SKILL, "You attempt to repair your $t, but damage it further.", ch, myobj( obj ), NULL, TO_CHAR );
         act( AT_SKILL, "$n attempts to repair $s $t, but damages it.", ch, myobj( obj ), NULL, TO_ROOM );
         damage_obj( obj );
      }
      if( victim )
      {
         sprintf( buf, "repairing %s", victim->name );
         ch->last_taken = STRALLOC( buf );
         if( !IS_AFFECTED( victim, AFF_CONSTRUCT ) )
         {
            send_to_char( "Just how do you propose to do that?\n\r", ch );
            return;
         }
         if( victim != ch )
         {
            act( AT_SKILL, "You attempt to repair $N, but damage $M further.", ch, NULL, victim, TO_CHAR );
            act( AT_SKILL, "$n attempts to repair you, but damages you further.", ch, NULL, victim, TO_VICT );
            act( AT_SKILL, "$n attempts to repair $N, but damages $M further.", ch, NULL, victim, TO_NOTVICT );
         }
         else
         {
            act( AT_SKILL, "You attempt to repair yourself, but damage yourself further.", ch, NULL, NULL, TO_CHAR );
            act( AT_SKILL, "$n attempts to repair $mself, but damages $mself further.", ch, NULL, NULL, TO_ROOM );
         }
         lose_hp( victim, 20 );
      }
      learn_from_failure( ch, gsn_repair );
   }
   else
   {
      if( obj )
      {
         sprintf( buf, "repairing the %s", myobj( obj ) );
         ch->last_taken = STRALLOC( buf );
         act( AT_SKILL, "You carefully repair your $t.", ch, myobj( obj ), NULL, TO_CHAR );
         act( AT_SKILL, "$n carefully repairs $s $t.", ch, myobj( obj ), NULL, TO_ROOM );
         obj->condition = number_fuzzy( obj->weight * 50 );
      }
      if( victim )
      {
         int amt;
         if( !IS_AFFECTED( victim, AFF_CONSTRUCT ) )
         {
            send_to_char( "Just how do you propose to do that?\n\r", ch );
            return;
         }
         sprintf( buf, "repairing %s", victim->name );
         ch->last_taken = STRALLOC( buf );
         if( victim != ch )
         {
            act( AT_SKILL, "You carefully repair $N.", ch, NULL, victim, TO_CHAR );
            act( AT_SKILL, "$n carefully repairs you.", ch, NULL, victim, TO_VICT );
            act( AT_SKILL, "$n carefully repairs $N.", ch, NULL, victim, TO_NOTVICT );
         }
         else
         {
            act( AT_SKILL, "You carefully repair yourself.", ch, NULL, NULL, TO_CHAR );
            act( AT_SKILL, "$n carefully repairs $mself.", ch, NULL, NULL, TO_ROOM );
         }
         amt = number_range( ch->pcdata->noncombat[SK_SMITH], ch->pcdata->noncombat[SK_SMITH] * 5 );
         if( victim != ch )
            amt *= 2;
         if( !IS_FIGHTING( ch ) )
            amt *= 2;
         victim->hit += amt;
         for( part = victim->first_part; part; part = part->next )
         {
            if( part->flags == PART_SEVERED 
                 &&part->cond == PART_WELL 
                 &&( part->connect_to  ? part->connect_to->flags != PART_SEVERED  : TRUE )  &&amt > 300 )
            {
               act( AT_MAGIC, "You replace your $t.", victim, part_locs[part->loc], NULL, TO_CHAR );
               act( AT_MAGIC, "$n replaces $s $t.", victim, part_locs[part->loc], NULL, TO_ROOM );
               part->flags = PART_WELL;
               return; /* No more healing after a part is regened */
            }
            else
            {
               part->cond = UMIN( PART_WELL, part->cond + amt / 10 );
            }
         }
      }
      learn_from_success( ch, gsn_repair );
   }
}


/* ------------------ Shop Building and Editing Section ----------------- */ 
void do_makeshop( CHAR_DATA * ch, char *argument ) 
{
   SHOP_DATA * shop;
   int vnum;
   ROOM_INDEX_DATA * room;
   if( !argument || argument[0] == '\0' )
      
   {
      vnum = ch->in_room->vnum;
      room = ch->in_room;
   }
   else
   {
      vnum = atoi( argument );
      if( ( room = get_room_index( vnum ) ) == NULL )
         
      {
         send_to_char( "Room not found.\n\r", ch );
         return;
      }
   }
   if( room->pShop )
      
   {
      send_to_char( "This room already has a shop.\n\r", ch );
      return;
   }
   CREATE( shop, SHOP_DATA, 1 );
   LINK( shop, first_shop, last_shop, next, prev );
   shop->room = vnum;
   shop->profit_buy = 90;
   shop->profit_sell = 120;
   shop->type = -1;
   shop->flag = -1;
   shop->owner = STRALLOC( "(for sale)" );
   shop->gold = 0;
   room->pShop = shop;
   send_to_char( "Done.\n\r", ch );
   return;
}
void do_shopset( CHAR_DATA * ch, char *argument ) 
{
   SHOP_DATA * shop;
   ROOM_INDEX_DATA * room, *room2;
   char arg1[MAX_INPUT_LENGTH];
   int vnum, value;
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   argument = one_argument( argument, arg1 );
   if( arg1[0] == '\0' )
      
   {
      send_to_char( "Usage: shopset <field> value\n\r", ch );
      send_to_char( "\n\rField being one of:\n\r", ch );
      if( IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
         send_to_char( "  room salary sell type flag owner gold\n\r", ch );
      
      else
         send_to_char( "  sell salary\n\r", ch );
      return;
   }
   vnum = atoi( arg1 );
   room = ch->in_room;
   if( !room->pShop )
      
   {
      send_to_char( "This room doesn't keep a shop.\n\r", ch );
      return;
   }
   shop = room->pShop;
   value = atoi( argument );
   if( !str_cmp( arg1, "sell" ) 
         &&( IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD )  ||!str_cmp( ch->name, shop->owner ) ) )
      
   {
      if( value < 0 )
         
      {
         send_to_char( "Out of range.\n\r", ch );
         return;
      }
      shop->profit_sell = value;
      send_to_char( "The selling profit for this shop has been changed.\n\r", ch );
      fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      return;
   }
   if( !str_cmp( arg1, "salary" ) 
         &&( IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD )  ||!str_cmp( ch->name, shop->owner ) ) )
      
   {
      if( shop->gold && !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
      {
         send_to_char
            ( "You can't change your shopkeeper's salary with money in the vault.\n\rWithdraw first, then change it.\n\r",
              ch );
         return;
      }
      if( value < 0 || value > 100 )
         
      {
         send_to_char( "Out of range.\n\r", ch );
         return;
      }
      shop->profit_buy = value;
      send_to_char( "The salary for this shopkeeper has been changed.\n\r", ch );
      fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      return;
   }
   if( !IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
   {
      send_to_char( "Invalid option.\n\r", ch );
      return;
   }
   if( !str_cmp( arg1, "type" ) )
      
   {
      if( !str_cmp( argument, "none" ) )
      {
         shop->flag = -1;
      }
      else
      {
         if( !is_number( argument ) )
            value = get_otype( argument );
         if( value < 0 || value > MAX_ITEM_TYPE )
            
         {
            send_to_char( "Invalid item type.\n\r", ch );
            return;
         }
         shop->type = value;
      }
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg1, "flag" ) )
      
   {
      if( !str_cmp( argument, "none" ) )
      {
         shop->flag = -1;
      }
      else
      {
         value = get_oflag( argument );
         if( value < 0 || value > MAX_BITS )
         {
            send_to_char( "Invalid flag.\n\r", ch );
            return;
         }
         shop->flag = value;
      }
      send_to_char( "Done.\n\r", ch );
      return;
   }
   if( !str_cmp( arg1, "owner" ) )
      
   {
      STRFREE( shop->owner );
      shop->owner = STRALLOC( argument );
      send_to_char( "Done.\n\r", ch );
      return;
   }
   
      /*
       * This doesn't work right now for some reason 
       */ 
      if( !str_cmp( arg1, "room" ) )
      
   {
      if( ( room2 = get_room_index( vnum ) ) == NULL )
         
      {
         send_to_char( "Room not found.\n\r", ch );
         return;
      }
      if( room2->pShop )
         
      {
         send_to_char( "That room already has a shop.\n\r", ch );
         return;
      }
      room->pShop = NULL;
      room2->pShop = shop;
      shop->room = value;
      send_to_char( "Done.\n\r", ch );
      return;
   }
   do_shopset( ch, "" );
   return;
}
void do_shopstat( CHAR_DATA * ch, char *argument ) 
{
   SHOP_DATA * shop;
   ROOM_INDEX_DATA * room;
   int vnum, gp;
   struct stat fst;
   char buf[MAX_STRING_LENGTH];
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( argument[0] == '\0' )
      
   {
      vnum = ch->in_room->vnum;
      room = ch->in_room;
   }
   else
   {
      vnum = atoi( argument );
      if( ( room = get_room_index( vnum ) ) == NULL )
         
      {
         send_to_char( "Room not found.\n\r", ch );
         return;
      }
   }
   if( !room->pShop )
      
   {
      send_to_char( "This room doesn't keep a shop.\n\r", ch );
      return;
   }
   shop = room->pShop;
   if( IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
      ch_printf( ch, "Room: [%d] %s\n\r", shop->room, room->name );
   
   else
      ch_printf( ch, "Stats for %s...\n\r", room->name );
   if( !str_cmp( shop->owner, ch->name )  ||IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
   {
      ch_printf( ch, "Profit:  Salary %3d%%  Sell %3d%%  Vault: ", shop->profit_buy, shop->profit_sell );
      gp = shop->gold;
      ch_printf( ch, "&C%dm ", ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%dg ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%ds ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "&O%dc&w\n\r", gp );
   }
   
      /*
       * Clear out shops of players that no longer exist 
       */ 
      if( str_cmp( shop->owner, "(for sale)" ) )
   {
      sprintf( buf, "%s%c/%s", PLAYER_DIR, LOWER( shop->owner[0] ), shop->owner );
      if( stat( buf, &fst ) == -1 )
      {
         STRFREE( shop->owner );
         shop->owner = STRALLOC( "(for sale)" );
         fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      }
   }
   ch_printf( ch, "The owner of this shop is %s.\n\r", capitalize( shop->owner ) );
   ch_printf( ch, "This shop will sell %s %ss.\n\r", shop->flag == -1 ? "any" : o_flags[shop->flag],
               shop->type == -1 ? "item" : o_types[shop->type] );
   return;
}
void do_shops( CHAR_DATA * ch, char *argument )
{
   int gp;
   SHOP_DATA * shop;
   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }
   if( !first_shop )
      
   {
      send_to_char( "There are no shops.\n\r", ch );
      return;
   }
   set_char_color( AT_NOTE, ch );
   if( IS_SET( ch->pcdata->permissions, PERMIT_HIBUILD ) )
      for( shop = first_shop; shop; shop = shop->next )
         pager_printf( ch, "Room: %5d Salary: %3d Sell: %3d Type: %d Flag: %d Owner: %s\n\r", shop->room, shop->profit_buy,
                        shop->profit_sell, shop->type, shop->flag, shop->owner );
   
   else
      for( shop = first_shop; shop; shop = shop->next )
      {
         if( !str_cmp( shop->owner, ch->name ) )
         {
            pager_printf( ch, "Room: %20s in %10s   Salary: %3d   Sell: %3d  Vault: ", get_room_index( shop->room )->name,
                           get_room_index( shop->room )->area->name, shop->profit_buy, shop->profit_sell );
            gp = shop->gold;
            pager_printf( ch, "&C%dm ", ( int )gp / 1000000 );
            gp = gp % 1000000;
            pager_printf( ch, "&Y%dg ", ( int )gp / 10000 );
            gp = gp % 10000;
            pager_printf( ch, "&W%ds ", ( int )gp / 100 );
            gp = gp % 100;
            pager_printf( ch, "&O%dc&w\n\r", gp );
         }
      } return;
}


