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
 *			   Object manipulation module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
   
/*
 * External functions
 */ 
   
bool check_social args( ( CHAR_DATA * ch, char *command, char *argument ) );

void show_list_to_char args( ( OBJ_DATA * list, CHAR_DATA * ch, 
bool fShort, bool fShowNothing ) );

void write_corpses args( ( CHAR_DATA * ch, char *name, OBJ_DATA * objrem ) );

/*
 * Local functions.
 */ 
void get_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj, 
OBJ_DATA * container ) );

OBJ_DATA * recursive_note_find args( ( OBJ_DATA * obj, char *argument ) );

/*
 * how resistant an object is to damage				-Thoric
 */ 
   sh_int get_obj_resistance( OBJ_DATA * obj ) 
{
   
sh_int resist;
   
resist = number_fuzzy( MAX_ITEM_IMPACT );
   
      /*
       * magical items are more resistant 
       */ 
      if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      
resist += number_fuzzy( 15 );
   
      /*
       * metal objects are definately stronger 
       */ 
      if( IS_OBJ_STAT( obj, ITEM_METAL ) )
      
resist += number_fuzzy( 10 );
   
      /*
       * organic objects are most likely weaker 
       */ 
      if( IS_OBJ_STAT( obj, ITEM_ORGANIC ) )
      
resist -= number_fuzzy( 10 );
   
      /*
       * blessed objects should have a little bonus 
       */ 
      if( IS_OBJ_STAT( obj, ITEM_BLESS ) )
      
resist += number_fuzzy( 10 );
   
      /*
       * lets make store inventory pretty tough 
       */ 
      if( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
      
resist += 20;
   
return URANGE( 10, resist, 99 );

}

void get_obj( CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container ) 
{
   
int weight;
   
if( IS_OBJ_STAT( obj, ITEM_NO_TAKE ) 
 &&( IS_NPC( ch ) || !IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) ) )
      
   {
      
send_to_char( "You can't take that.\n\r", ch );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) 
 &&!can_take_proto( ch ) )
      
   {
      
send_to_char( "A godly force prevents you from getting close to it.\n\r", ch );
      
return;
   
}
   
if( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
      
   {
      
act( AT_PLAIN, "$d: you can't carry that many items.", 
ch, NULL, obj->name, TO_CHAR );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_COVERING ) )
      
weight = obj->weight;
   
   else
      
weight = get_obj_weight( obj );
   
if( ch->carry_weight + weight > can_carry_w( ch ) )
      
   {
      
act( AT_PLAIN, "$d: you can't carry that much weight.", 
ch, NULL, obj->name, TO_CHAR );
      
return;
   
}
   
if( obj->item_type == ITEM_VEHICLE )
   {
      
CHAR_DATA * rch;
      
for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
         
      {
         
if( rch->in_obj == obj )
         {
            
send_to_char( "Not while somebody is in it!\n\r", ch );
            
return;
         
}
      
}
   
}
   
      /*
       * Make it so players can pick up corpses and eat 'em! -- Scion 
       */ 
      if( obj->item_type == ITEM_CORPSE_NPC )
   {
      
dump_container( obj );
      
act( AT_PLAIN, "You shake all the loose items and nasty crunchy bits off the corpse.", ch, NULL, NULL, TO_CHAR );
      
act( AT_PLAIN, "$n shakes all the loose items off the corpse.", ch, NULL, NULL, TO_ROOM );
   
}
   
if( container )
      
   {
      
if( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT( container, ITEM_COVERING ) )
         
      {
         
act( AT_ACTION, "You remove $p from $P", ch, obj, container, TO_CHAR );
         
act( AT_ACTION, "$n removes $p from $P", ch, obj, container, TO_ROOM );
      
}
      
      else
         
      {
         
act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) ? 
"You get $p from beneath $P." : "You get $p from $P.",
               
ch, obj, container, TO_CHAR );
         
act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) ? 
"$n gets $p from beneath $P." : "$n gets $p from $P.",
               
ch, obj, container, TO_ROOM );
      
}
      
if( !IS_NPC( ch ) && str_cmp( container->name + 7, ch->name ) )
         
container->value[5]++;
      
obj_from_obj( obj );
   
}
   
   else
      
   {
      
act( AT_ACTION, "You get $p.", 
ch, obj, container, TO_CHAR );
      
act( AT_ACTION, "$n gets $p.", 
ch, obj, container, TO_ROOM );
      
obj_from_room( obj );
   
}
   
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
      
obj->timer = 0;
   
if( obj->item_type != ITEM_CONTAINER )
      
check_for_trap( ch, obj, TRAP_GET );
   
if( char_died( ch ) )
      
return;
   
if( obj->item_type == ITEM_MONEY )
   {
      
int gp;
      
gp = obj->value[0];
      
ch_printf( ch, "There were &C%d mithril, ", ( int )gp / 1000000 );
      
gp = gp % 1000000;
      
ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
      
gp = gp % 10000;
      
ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
      
gp = gp % 100;
      
ch_printf( ch, "and &O%d copper coins.\n\r", gp );
      
ch->gold += obj->value[0];
      
extract_obj( obj );
   
}
   
   else
      
   {
      
obj = obj_to_char( obj, ch );
   
}
   
if( char_died( ch ) || obj_extracted( obj ) )
      
return;
   
oprog_get_trigger( ch, obj );
   
return;

}

void do_get( CHAR_DATA * ch, char *argument )
{
   
char arg1[MAX_INPUT_LENGTH];
   
char arg2[MAX_INPUT_LENGTH];
   
OBJ_DATA * obj;
   
OBJ_DATA * obj_next;
   
OBJ_DATA * container;
   
sh_int number;
   
bool found;
   
argument = one_argument( argument, arg1 );
   
if( is_number( arg1 ) )
      
   {
      
number = atoi( arg1 );
      
if( number < 1 )
         
      {
         
send_to_char( "That was easy...\n\r", ch );
         
return;
      
}
      
if( ( ch->carry_number + number ) > can_carry_n( ch ) )
         
      {
         
send_to_char( "You can't carry that many.\n\r", ch );
         
return;
      
}
      
argument = one_argument( argument, arg1 );
   
}
   
   else
      
number = 0;
   
argument = one_argument( argument, arg2 );
   
      /*
       * munch optional words 
       */ 
      if( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
      
argument = one_argument( argument, arg2 );
   
      /*
       * Get type. 
       */ 
      if( arg1[0] == '\0' )
      
   {
      
send_to_char( "Get what?\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( arg2[0] == '\0' )
      
   {
      
if( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
         
      {
         
            /*
             * 'get obj' 
             */ 
            if( ch->in_room->pShop )
         {
            
send_to_char( "You can't get items without buying them when you are in a shop.\n\r", ch );
            
return;
         
}
         
if( ch->in_obj )
            
obj = get_obj_list( ch, arg1, ch->in_obj->first_content );
         
         else
            
obj = get_obj_list( ch, arg1, ch->in_room->first_content );
         
if( !obj )
            
         {
            
act( AT_PLAIN, "There is no $T you can reach from here.", 
ch, NULL, arg1, TO_CHAR );
            
return;
         
}
         
if( IS_NPC( ch ) && 
( obj->item_type == ITEM_CORPSE_NPC 
 ||obj->item_type == ITEM_TREASURE ) )
            return;
         
if( obj->item_type == ITEM_FURNITURE 
 &&count_users( obj ) > 0 )
         {
            
send_to_char( "Not when someone's on it!\n\r", ch );
            
return;
         
}
         
if( IS_AFFECTED( ch, AFF_HAS_ARTI ) && IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
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
         
            /*
             * lets not make magic items hovering in nofloor rooms ungettable -keo 
             */ 
            if( IS_AFFECTED( ch, AFF_FLYING ) 
 &&( !IS_OBJ_STAT( obj, ITEM_HOVER ) ) )
         {
            
if( ch->curr_talent[TAL_MOTION] < 10 )
            {
               
send_to_char( "You can't reach anything on the ground from up here.", ch );
               
return;
            
}
         
}
         
            /*
             * sticky floor, chance of sticking -keo 
             */ 
            if( !IS_OBJ_STAT( obj, ITEM_NO_TAKE ) 
                &&IS_SET( ch->in_room->room_flags, ROOM_STICKY ) 
 &&( !IS_OBJ_STAT( obj, ITEM_HOVER ) ) )
         {
            
STRFREE( ch->last_taken );
            
ch->last_taken = STRALLOC( "trying to pry an item off the sticky floor" );
            
WAIT_STATE( ch, PULSE_VIOLENCE );
            
if( number_range( 1, 100 ) > get_curr_str( ch ) )
            {
               
act( AT_ACTION, "$p is stuck to the floor, and you don't manage to pry it loose.", ch, obj, NULL, TO_CHAR );
               
act( AT_ACTION, "$n tries to lift $p, but it is stuck to the floor.", ch, obj, NULL, TO_ROOM );
               
return;
            
}
            else
            {
               
act( AT_ACTION, "$p is stuck to the floor, but you manage to pry it loose.", ch, obj, NULL, TO_CHAR );
               
act( AT_ACTION, "$n pries $p off the sticky floor.", ch, obj, NULL, TO_ROOM );
            
}
         
}
         
separate_obj( obj );
         
get_obj( ch, obj, obj->in_obj );
         
if( char_died( ch ) )
            
return;
  
save_house_by_vnum(ch->in_room->vnum); /* House Object Saving */
  
if( IS_SET( sysdata.save_flags, SV_GET ) )
            
save_char_obj( ch );
      
}
      
      else
         
      {
         
sh_int cnt = 0;
         
bool fAll;
         
char *chk;
         
if( IS_SET( ch->in_room->room_flags, ROOM_DONATION ) )
            
         {
            
send_to_char( "The gods frown upon such a display of greed!\n\r", ch );
            
return;
         
}
         
if( IS_AFFECTED( ch, AFF_FLYING ) )
         {
            
send_to_char( "You can't reach anything on the ground from up here.\r\n", ch );
            
return;
         
}
         
if( !str_cmp( arg1, "all" ) )
            
fAll = TRUE;
         
         else
            
fAll = FALSE;
         
if( number > 1 )
            
chk = arg1;
         
         else
            
chk = &arg1[4];
         
            /*
             * 'get all' or 'get all.obj' 
             */ 
            
if( ch->in_room->pShop )
         {
            
send_to_char( "You can't get items without buying them when you are in a shop.\n\r", ch );
            
return;
         
}
         
found = FALSE;
         
for( obj = ch->in_room->last_content; obj; obj = obj_next )
            
         {
            
obj_next = obj->prev_content;
            
if( ( fAll 
                   ||nifty_is_name_prefix( chk, obj->name ) ) 
                 &&!IS_OBJ_STAT( obj, ITEM_ARTIFACT ) 
 &&!IS_AFFECTED( ch, AFF_ETHEREAL ) 
 &&can_see_obj( ch, obj ) )
               
            {
               
found = TRUE;
               
if( number && ( cnt + obj->count ) > number )
                  
split_obj( obj, number - cnt );
               
cnt += obj->count;
               
get_obj( ch, obj, NULL );
               
if( char_died( ch ) 
                    ||ch->carry_number >= can_carry_n( ch ) 
                    ||ch->carry_weight >= can_carry_w( ch ) 
 ||( number && cnt >= number ) )
                  
               {
                  
if( IS_SET( sysdata.save_flags, SV_GET ) 
 &&!char_died( ch ) )
                     
save_char_obj( ch );
                  
return;
               
}
            
}
         
}
         
if( !found )
            
         {
            
if( fAll )
               
send_to_char( "You see nothing here.\n\r", ch );
            
            else
               
act( AT_PLAIN, "You see no $T here.", ch, NULL, chk, TO_CHAR );
         
}
         
         else
         
           {
                   save_house_by_vnum(ch->in_room->vnum); /* House Object Saving */

                   if ( IS_SET( sysdata.save_flags, SV_GET ) )
                           save_char_obj( ch );
           }
      
}
   
}
   
   else
      
   {
      
         /*
          * 'get ... container' 
          */ 
         if( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
         
      {
         
send_to_char( "You can't do that.\n\r", ch );
         
return;
      
}
      
if( ( container = get_obj_here( ch, arg2 ) ) == NULL )
         
      {
         
act( AT_PLAIN, "You see no $T here.", ch, NULL, arg2, TO_CHAR );
         
return;
      
}
      
switch ( container->item_type )
         
      {
         
default:
            
if( !IS_OBJ_STAT( container, ITEM_COVERING ) )
               
            {
               
send_to_char( "That's not a container.\n\r", ch );
               
return;
            
}
            
if( ch->carry_weight + container->weight > can_carry_w( ch ) )
               
            {
               
send_to_char( "It's too heavy for you to lift.\n\r", ch );
               
return;
            
}
            
break;
         
case ITEM_CONTAINER:
         
case ITEM_CORPSE_NPC:
         
case ITEM_KEYRING:
         
case ITEM_QUIVER:
         
case ITEM_FURNITURE:
         
case ITEM_VEHICLE:
            
break;
         
case ITEM_CORPSE_PC:
            
         {
            
char name[MAX_INPUT_LENGTH];
            
CHAR_DATA * gch;
            
char *pd;
            
if( IS_NPC( ch ) )
               
            {
               
send_to_char( "You can't do that.\n\r", ch );
               
return;
            
}
            
pd = container->short_descr;
            
pd = one_argument( pd, name );
            
pd = one_argument( pd, name );
            
pd = one_argument( pd, name );
            
pd = one_argument( pd, name );
            
if( str_cmp( name, ch->name ) )
               
            {
               
bool fGroup;
               
fGroup = FALSE;
               
for( gch = first_char; gch; gch = gch->next )
                  
               {
                  
if( !IS_NPC( gch ) 
 &&is_same_group( ch, gch ) 
 &&!str_cmp( name, gch->name ) )
                     
                  {
                     
fGroup = TRUE;
                     
break;
                  
}
               
}
            
}
         
}
      
}
      
if( !IS_OBJ_STAT( container, ITEM_COVERING ) 
            &&IS_SET( container->value[1], CONT_CLOSED ) 
            &&container->item_type != ITEM_CORPSE_NPC 
 &&container->item_type != ITEM_CORPSE_PC )
         
      {
         
act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
         
return;
      
}
      
if( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
         
      {
         
            /*
             * 'get obj container' 
             */ 
            obj = get_obj_list( ch, arg1, container->first_content );
         
if( !obj )
            
         {
            
act( AT_PLAIN,
                  IS_OBJ_STAT( container,
                               ITEM_COVERING ) ? 
"You see nothing like that beneath the $T." :
                  
"You see nothing like that in the $T.", 
ch, NULL, arg2, TO_CHAR );
            
return;
         
}
         
if( container->carried_by != ch && ( IS_AFFECTED( ch, AFF_FLYING ) ) )
            
         {
            
if( ch->curr_talent[TAL_MOTION] < 20 )
            {
               
send_to_char( "You can't reach anything on the ground from up here.\n\r", ch );
               
return;
               
                  /*
                   * check if they can apport -keo 
                   */ 
            }
            else if( number_percent(  ) > ch->curr_talent[TAL_MOTION] )
            {
               
act( AT_MAGIC, "$p shakes a little.", 
ch, obj, NULL, TO_ROOM );
               
act( AT_MAGIC, "You attempt to lift $p but fail.", 
ch, obj, NULL, TO_CHAR );
               
return;
            
}
            else
            {
               
act( AT_MAGIC, "$p suddenly flies up toward $n.", 
ch, obj, NULL, TO_ROOM );
               
act( AT_MAGIC, "You lift $p and bring it toward you.", 
ch, obj, NULL, TO_CHAR );
               
learn_talent( ch, TAL_MOTION );
               
mana_from_char( ch, 25 );
            
}
         
}
         
if( IS_AFFECTED( ch, AFF_HAS_ARTI ) && IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
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
         
separate_obj( obj );
         
get_obj( ch, obj, container );
         
check_for_trap( ch, container, TRAP_GET );
         
if( char_died( ch ) )
            
return;
         
if( IS_SET( sysdata.save_flags, SV_GET ) )
            
save_char_obj( ch );
      
}
      
      else
         
      {
         
int cnt = 0;
         
bool fAll;
         
char *chk;
         
            /*
             * 'get all container' or 'get all.obj container' 
             */ 
            if( IS_AFFECTED( ch, AFF_FLYING ) 
 &&!IS_OBJ_STAT( container, ITEM_MAGIC ) )
         {
            
send_to_char( "You can't reach items on the ground from up here.\n\r", ch );
            
return;
         
}
         
if( container->item_type == ITEM_CORPSE_PC 
 &&!nifty_is_name( ch->name, container->name ) )
         {
            
send_to_char( "Your adrenaline is pumping too hard to do that!\n\r", ch );
            
return;
         
}
         
if( !str_cmp( arg1, "all" ) )
            
fAll = TRUE;
         
         else
            
fAll = FALSE;
         
if( number > 1 )
            
chk = arg1;
         
         else
            
chk = &arg1[4];
         
found = FALSE;
         
for( obj = container->first_content; obj; obj = obj_next )
            
         {
            
obj_next = obj->next_content;
            
if( ( fAll || nifty_is_name_prefix( chk, obj->name ) ) 
                 &&!IS_OBJ_STAT( obj, ITEM_ARTIFACT ) 
 &&!IS_AFFECTED( ch, AFF_ETHEREAL ) 
 &&can_see_obj( ch, obj ) )
               
            {
               
found = TRUE;
               
if( number && ( cnt + obj->count ) > number )
                  
split_obj( obj, number - cnt );
               
cnt += obj->count;
               
get_obj( ch, obj, container );
               
if( char_died( ch ) 
                    ||ch->carry_number >= can_carry_n( ch ) 
                    ||ch->carry_weight >= can_carry_w( ch ) 
 ||( number && cnt >= number ) )
                  
return;
            
}
         
}
         
if( !found )
            
         {
            
if( fAll )
               
            {
               
if( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT( container, ITEM_COVERING ) )
                  
act( AT_PLAIN, "The $T holds no keys.", 
ch, NULL, arg2, TO_CHAR );
               
               else
                  
act( AT_PLAIN,
                        IS_OBJ_STAT( container,
                                     ITEM_COVERING ) ? 
"You see nothing beneath the $T." : 
"You see nothing in the $T.",
                        
ch, NULL, arg2, TO_CHAR );
            
}
            
            else
               
            {
               
if( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT( container, ITEM_COVERING ) )
                  
act( AT_PLAIN, "The $T does not hold that key.", 
ch, NULL, arg2, TO_CHAR );
               
               else
                  
act( AT_PLAIN,
                        IS_OBJ_STAT( container,
                                     ITEM_COVERING ) ? 
"You see nothing like that beneath the $T." :
                        
"You see nothing like that in the $T.", 
ch, NULL, arg2, TO_CHAR );
            
}
         
}
         
         else
            
check_for_trap( ch, container, TRAP_GET );
         
if( char_died( ch ) )
            
return;
         
            /*
             * Oops no wonder corpses were duping oopsie did I do that
             * * --Shaddai
             */ 
            if( container->item_type == ITEM_CORPSE_PC )
            
write_corpses( NULL, container->short_descr + 14, NULL );
         
if( found && IS_SET( sysdata.save_flags, SV_GET ) )
            
save_char_obj( ch );
      
}
   
}
   
return;

}

void do_put( CHAR_DATA * ch, char *argument ) 
{
   
char arg1[MAX_INPUT_LENGTH];
   
char arg2[MAX_INPUT_LENGTH];
   
OBJ_DATA * container;
   
OBJ_DATA * obj;
   
OBJ_DATA * obj_next;
   
sh_int count;
   
int number;
   
bool save_char = FALSE;
   
argument = one_argument( argument, arg1 );
   
if( is_number( arg1 ) )
      
   {
      
number = atoi( arg1 );
      
if( number < 1 )
         
      {
         
send_to_char( "That was easy...\n\r", ch );
         
return;
      
}
      
argument = one_argument( argument, arg1 );
   
}
   
   else
      
number = 0;
   
argument = one_argument( argument, arg2 );
   
      /*
       * munch optional words 
       */ 
      if( ( !str_cmp( arg2, "into" ) || !str_cmp( arg2, "inside" ) 
            ||!str_cmp( arg2, "in" ) || !str_cmp( arg2, "under" ) 
            ||!str_cmp( arg2, "onto" ) || !str_cmp( arg2, "on" ) ) 
 &&argument[0] != '\0' )
      
argument = one_argument( argument, arg2 );
   
if( arg1[0] == '\0' || arg2[0] == '\0' )
      
   {
      
send_to_char( "Put what in what?\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
      
   {
      
send_to_char( "You can't do that.\n\r", ch );
      
return;
   
}
   
if( ( container = get_obj_here( ch, arg2 ) ) == NULL )
      
   {
      
act( AT_PLAIN, "You see no $T here.", ch, NULL, arg2, TO_CHAR );
      
return;
   
}
   
if( !container->carried_by && IS_SET( sysdata.save_flags, SV_PUT ) )
      
save_char = TRUE;
   
if( IS_OBJ_STAT( container, ITEM_COVERING ) )
      
   {
      
if( ch->carry_weight + container->weight > can_carry_w( ch ) )
         
      {
         
send_to_char( "It's too heavy for you to lift.\n\r", ch );
         
return;
      
}
   
}
   
   else
      
   {
      
         /*
          * lets burn flammables -keo 
          */ 
         if( container->item_type == ITEM_FIRE )
      {
         
if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
            
         {
            
send_to_char( "You do not have that item.\n\r", ch );
            
return;
         
}
         
if( !IS_OBJ_STAT( obj, ITEM_FLAMMABLE ) 
 ||IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            
act( AT_FIRE, "$p won't catch fire.", ch, obj, NULL, TO_CHAR );
            
return;
         
}
         
act( AT_FIRE, "You put $p on $P, and it catches fire and burns.", 
ch, obj, container, TO_CHAR );
         
act( AT_FIRE, "$n puts $p on $P, and it catches fire and burns.", 
ch, obj, container, TO_ROOM );
         
if( container->timer > 0 )
            container->timer += obj->weight * 3;
         
obj_from_char( obj );
         
extract_obj( obj );
         
return;
      
}
      
if( container->item_type != ITEM_CONTAINER 
            &&container->item_type != ITEM_FURNITURE 
            &&container->item_type != ITEM_KEYRING 
            &&container->item_type != ITEM_VEHICLE 
 &&container->item_type != ITEM_QUIVER )
         
      {
         
send_to_char( "That's not a container.\n\r", ch );
         
return;
      
}
      
if( IS_SET( container->value[1], CONT_CLOSED ) 
            &&container->item_type != ITEM_CORPSE_NPC 
 &&container->item_type != ITEM_CORPSE_PC )
         
      {
         
act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
         
return;
      
}
   
}
   
if( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
      
   {
      
         /*
          * 'put obj container' 
          */ 
         if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
         
      {
         
send_to_char( "You do not have that item.\n\r", ch );
         
return;
      
}
      
if( obj == container )
         
      {
         
send_to_char( "You can't fold it into itself.\n\r", ch );
         
return;
      
}
      
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         
      {
         
act( AT_ACTION, "$p squirms refusing to let you put it in the container.", ch, obj, NULL, TO_CHAR );
         
act( AT_ACTION, "$p squirms as $n tries to put it in a container.", ch, obj, NULL, TO_ROOM );
         
return;
      
}
      
if( container->item_type == ITEM_KEYRING && obj->item_type != ITEM_KEY )
         
      {
         
send_to_char( "That's not a key.\n\r", ch );
         
return;
      
}
      
if( container->item_type == ITEM_QUIVER && obj->item_type != ITEM_PROJECTILE )
         
      {
         
send_to_char( "That's not a projectile.\n\r", ch );
         
return;
      
}
      
if( ( IS_OBJ_STAT( container, ITEM_COVERING ) 
              &&( get_obj_weight( obj ) * obj->count ) 
 >( ( get_real_obj_weight( container ) * container->count ) ) ) )
         
      {
         
send_to_char( "It won't fit under there.\n\r", ch );
         
return;
      
}
      
         /*
          * note use of get_real_obj_weight 
          */ 
         else if( ( get_real_obj_weight( obj ) * obj->count ) 
                  >( ( container->item_type == ITEM_CONTAINER 
                       ? container->value[0] - get_real_obj_weight( container ) 
                       : get_real_obj_weight( container ) ) * container->count ) )
         
      {
         
send_to_char( "It won't fit.\n\r", ch );
         
return;
      
}
      
separate_obj( obj );
      
separate_obj( container );
      
obj_from_char( obj );
      
         /*
          * basketball? -keo 
          */ 
         if( container->carried_by != ch && ( IS_AFFECTED( ch, AFF_FLYING ) ) )
         
      {
         
act( AT_PLAIN, "You throw the $t at the $T.", ch, myobj( obj ), myobj( container ), TO_CHAR );
         
act( AT_PLAIN, "$n throws $p at the $T.", ch, obj, myobj( container ), TO_ROOM );
         
if( ( number_range( 0, 5 ) * 100 ) < container->value[0] )
         {
            
act( AT_PLAIN, "Score! The $t lands in the $T.", ch, myobj( obj ), myobj( container ), TO_CHAR );
            
act( AT_PLAIN, "$n scores! The $t lands in the $T.", ch, myobj( obj ), myobj( container ), TO_ROOM );
            
obj = obj_to_obj( obj, container );
            
return;
         
}
         else
         {
            
act( AT_PLAIN, "You missed! The $t lands on the ground.", ch, myobj( obj ), NULL, TO_CHAR );
            
act( AT_PLAIN, "$n misses! The $t lands on the ground.", ch, myobj( obj ), NULL, TO_ROOM );
            
obj_to_room( obj, ch->in_room );
            
separate_obj( obj );
            
damage_obj( obj );
            
return;
         
}
      
}
      
obj = obj_to_obj( obj, container );
      
count = obj->count;
      
obj->count = 1;
      
if( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT( container, ITEM_COVERING ) )
         
      {
         
act( AT_ACTION, "$n slips $p onto $P.", ch, obj, container, TO_ROOM );
         
act( AT_ACTION, "You slip $p onto $P.", ch, obj, container, TO_CHAR );
      
}
      
      else
         
      {
         
act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) 
               ? "$n hides $p beneath $P." : "$n puts $p in $P.", 
ch, obj, container, TO_ROOM );
         
act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) 
               ? "You hide $p beneath $P." : "You put $p in $P.", 
ch, obj, container, TO_CHAR );
      
}
      
obj->count = count;
      
if( save_char )
         
save_char_obj( ch );
   
}
   
   else
      
   {
      
bool found = FALSE;
      
int cnt = 0;
      
bool fAll;
      
char *chk;
      
if( !str_cmp( arg1, "all" ) )
         
fAll = TRUE;
      
      else
         
fAll = FALSE;
      
if( number > 1 )
         
chk = arg1;
      
      else
         
chk = &arg1[4];
      
if( container->carried_by != ch && ( IS_AFFECTED( ch, AFF_FLYING ) ) )
         
      {
         
send_to_char( "From up here? Do it one at a time!", ch );
         
return;
      
}
      
separate_obj( container );
      
         /*
          * 'put all container' or 'put all.obj container' 
          */ 
         for( obj = ch->first_carrying; obj; obj = obj_next )
         
      {
         
obj_next = obj->next_content;
         
if( ( fAll || nifty_is_name_prefix( chk, obj->name ) ) 
               &&can_see_obj( ch, obj ) 
               &&obj->wear_loc == WEAR_NONE 
               &&obj != ch->main_hand 
               &&obj != ch->off_hand 
               &&obj != container 
               &&!IS_OBJ_STAT( obj, ITEM_ARTIFACT ) 
               &&can_drop_obj( ch, obj ) 
               &&( container->item_type != ITEM_KEYRING || obj->item_type == ITEM_KEY ) 
               &&( container->item_type != ITEM_QUIVER || obj->item_type == ITEM_PROJECTILE ) 
               &&get_obj_weight( obj ) + get_real_obj_weight( container ) 
 <=container->value[0] )
            
         {
            
if( number && ( cnt + obj->count ) > number )
               
split_obj( obj, number - cnt );
            
cnt += obj->count;
            
obj_from_char( obj );
            
if( container->item_type == ITEM_KEYRING )
               
            {
               
act( AT_ACTION, "$n slips $p onto $P.", ch, obj, container, TO_ROOM );
               
act( AT_ACTION, "You slip $p onto $P.", ch, obj, container, TO_CHAR );
            
}
            
            else
               
            {
               
act( AT_ACTION, "$n puts $p in $P.", ch, obj, container, TO_ROOM );
               
act( AT_ACTION, "You put $p in $P.", ch, obj, container, TO_CHAR );
            
}
            
obj = obj_to_obj( obj, container );
            
found = TRUE;
            
if( number && cnt >= number )
               
break;
         
}
      
}
      
         /*
          * 
          * * Don't bother to save anything if nothing was dropped   -Thoric
          */ 
         if( !found )
         
      {
         
if( fAll )
            
act( AT_PLAIN, "You are not carrying anything that would fit.", 
ch, NULL, NULL, TO_CHAR );
         
         else
            
act( AT_PLAIN, "You are not carrying any $T.", 
ch, NULL, chk, TO_CHAR );
         
return;
      
}
      
if( save_char )
         
save_char_obj( ch );
   
}
   
return;

}

void do_drop( CHAR_DATA * ch, char *argument ) 
{
   
char arg[MAX_INPUT_LENGTH];
   
OBJ_DATA * obj;
   
OBJ_DATA * obj_next;
   
bool found;
   
int number;
   
if( ch->in_room->pShop )
   {
      
send_to_char( "You can't drop items without selling them when you are in a shop.\n\r", ch );
      
return;
   
}
   
argument = one_argument( argument, arg );
   
if( is_number( arg ) )
      
   {
      
number = atoi( arg );
      
if( number < 1 )
         
      {
         
send_to_char( "That was easy...\n\r", ch );
         
return;
      
}
      
argument = one_argument( argument, arg );
   
}
   
   else
      
number = 0;
   
if( arg[0] == '\0' )
      
   {
      
send_to_char( "Drop what?\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( xIS_SET( ch->act, PLR_LITTERBUG ) )
      
   {
      
set_char_color( AT_YELLOW, ch );
      
send_to_char( "A godly force prevents you from dropping anything...\n\r", ch );
      
return;
   
}
   
if( IS_SET( ch->in_room->room_flags, ROOM_NODROP ) 
 &&ch != supermob )
      
   {
      
set_char_color( AT_MAGIC, ch );
      
send_to_char( "A magical force stops you!\n\r", ch );
      
set_char_color( AT_TELL, ch );
      
send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
      
return;
   
}
   
if( number > 0 )
      
   {
      
         /*
          * 'drop NNNN coins' 
          */ 
         
if( !str_cmp( arg, "mithril" ) || !str_cmp( arg, "gold" ) 
              ||!str_cmp( arg, "silver" ) || !str_cmp( arg, "copper" ) 
 ||!str_cmp( arg, "coins" ) )
      {
         
if( !str_cmp( arg, "mithril" ) )
            
number *= 1000000;
         
         else if( !str_cmp( arg, "gold" ) )
            
number *= 10000;
         
         else if( !str_cmp( arg, "silver" ) )
            
number *= 100;
         
if( number < 0 )
         {
            
send_to_char( "And just how do you propose to do that?\n\r", ch );
            
return;
         
}
         
if( ch->gold < number )
            
         {
            
send_to_char( "You haven't got that many coins.\n\r", ch );
            
return;
         
}
         
ch->gold -= number;
         
for( obj = ch->in_room->first_content; obj; obj = obj_next )
            
         {
            
obj_next = obj->next_content;
            
switch ( obj->pIndexData->vnum )
               
            {
               
case OBJ_VNUM_MONEY_ONE:
                  
number += 1;
                  
extract_obj( obj );
                  
break;
               
case OBJ_VNUM_MONEY_SOME:
                  
number += obj->value[0];
                  
extract_obj( obj );
                  
break;
            
}
         
}
         
act( AT_ACTION, "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
         
if( ch->in_obj )
            
obj_to_obj( create_money( number ), ch->in_obj );
         
         else
            
obj_to_room( create_money( number ), ch->in_room );

save_house_by_vnum(ch->in_room->vnum); /* House Object Saving */
         
send_to_char( "You drop some coins.\n\r", ch );
         
if( IS_SET( sysdata.save_flags, SV_DROP ) )
            
save_char_obj( ch );
         
return;
      
}
   
}
   
if( number <= 1 && str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
      
   {
      
         /*
          * 'drop obj' 
          */ 
         if( ( obj = get_obj_carry( ch, arg ) ) == NULL )
         
      {
         
send_to_char( "You do not have that item.\n\r", ch );
         
return;
      
}
      
if( !can_drop_obj( ch, obj ) )
         
      {
         
send_to_char( "You can't let go of it.\n\r", ch );
         
return;
      
}
      
separate_obj( obj );
      
act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
      
act( AT_ACTION, "You drop the $t.", ch, myobj( obj ), NULL, TO_CHAR );
      
obj_from_char( obj );
      
if( ch->in_obj )
         
obj = obj_to_obj( obj, ch->in_obj );
      
      else
         
obj = obj_to_room( obj, ch->in_room );
      
oprog_drop_trigger( ch, obj );  /* mudprogs */
      
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         
obj->timer = 30;
      
if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
      {
         
act( AT_ACTION, "The $t ripples and vanishes.", 
ch, myobj( obj ), NULL, TO_CHAR );
         
act( AT_ACTION, "$p ripples and vanishes.", 
ch, obj, NULL, TO_ROOM );
         
extract_obj( obj );
      
}
      
if( char_died( ch ) || obj_extracted( obj ) )
         
return;
   
}
   
   else
      
   {
      
int cnt = 0;
      
char *chk;
      
bool fAll;
      
if( !str_cmp( arg, "all" ) )
         
fAll = TRUE;
      
      else
         
fAll = FALSE;
      
if( number > 1 )
         
chk = arg;
      
      else
         
chk = &arg[4];
      
         /*
          * 'drop all' or 'drop all.obj' 
          */ 
         if( IS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) )
         
      {
         
send_to_char( "You can't seem to do that here...\n\r", ch );
         
return;
      
}
      
found = FALSE;
      
for( obj = ch->first_carrying; obj; obj = obj_next )
         
      {
         
obj_next = obj->next_content;
         
if( ( fAll || nifty_is_name_prefix( chk, obj->name ) ) 
               &&can_see_obj( ch, obj ) 
 &&obj->wear_loc == WEAR_NONE 
 &&can_drop_obj( ch, obj ) )
            
         {
            
found = TRUE;
            
if( HAS_PROG( obj->pIndexData, DROP_PROG ) && obj->count > 1 )
               
            {
               
++cnt;
               
separate_obj( obj );
               
obj_from_char( obj );
               
if( !obj_next )
                  
obj_next = ch->first_carrying;
            
}
            
            else
               
            {
               
if( number && ( cnt + obj->count ) > number )
                  
split_obj( obj, number - cnt );
               
cnt += obj->count;
               
obj_from_char( obj );
            
}
            
act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
            
act( AT_ACTION, "You drop the $t.", ch, myobj( obj ), NULL, TO_CHAR );
            
if( ch->in_obj )
               
obj = obj_to_obj( obj, ch->in_obj );
            
            else
               
obj = obj_to_room( obj, ch->in_room );
            
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
               
obj->timer = 30;
            
if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
            {
               
act( AT_ACTION, "The $t ripples and vanishes.", 
ch, myobj( obj ), NULL, TO_CHAR );
               
act( AT_ACTION, "$p ripples and vanishes.", 
ch, obj, NULL, TO_ROOM );
               
extract_obj( obj );
            
}
            
oprog_drop_trigger( ch, obj ); /* mudprogs */
            
if( char_died( ch ) )
               
return;
            
if( number && cnt >= number )
               
break;
         
}
      
}
      
if( !found )
         
      {
         
if( fAll )
            
act( AT_PLAIN, "You are not carrying anything.", 
ch, NULL, NULL, TO_CHAR );
         
         else
            
act( AT_PLAIN, "You are not carrying any $T.", 
ch, NULL, chk, TO_CHAR );
      
}
   
}
  
save_house_by_vnum(ch->in_room->vnum); /* House Object Saving */
  
if( IS_SET( sysdata.save_flags, SV_DROP ) )
      
save_char_obj( ch );   /* duping protector */
   
return;

}

void do_give( CHAR_DATA * ch, char *argument ) 
{
   
char arg1[MAX_INPUT_LENGTH];
   
char arg2[MAX_INPUT_LENGTH];
   
char buf[MAX_INPUT_LENGTH];
   
CHAR_DATA * victim;
   
OBJ_DATA * obj;
   
argument = one_argument( argument, arg1 );
   
argument = one_argument( argument, arg2 );
   
if( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
      
argument = one_argument( argument, arg2 );
   
if( arg1[0] == '\0' || arg2[0] == '\0' )
      
   {
      
send_to_char( "Give what to whom?\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( is_number( arg1 ) )
      
   {
      
         /*
          * 'give NNNN coins victim' 
          */ 
      int amount;
      
int gp;
      
char buf2[MAX_STRING_LENGTH];
      
amount = atoi( arg1 );
      
if( amount <= 0 )
      {
         
send_to_char( "Well, that was easy!\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "mithril" ) )
         
amount *= 1000000;
      
      else if( !str_cmp( arg2, "gold" ) )
         
amount *= 10000;
      
      else if( !str_cmp( arg2, "silver" ) )
         
amount *= 100;
      
      else if( !str_cmp( arg2, "copper" ) 
 ||!str_cmp( arg2, "coins" ) )
         
amount *= 1;
      
      else
      {
         
send_to_char( "You can't do that.\n\r", ch );
         
return;
      
}
      
argument = one_argument( argument, arg2 );
      
if( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
         
argument = one_argument( argument, arg2 );
      
if( arg2[0] == '\0' )
         
      {
         
send_to_char( "Give what to whom?\n\r", ch );
         
return;
      
}
      
if( ( victim = get_char_room( ch, arg2 ) ) == NULL )
         
      {
         
send_to_char( "They aren't here.\n\r", ch );
         
return;
      
}
      
if( amount < 0 )
      {
         
send_to_char( "And just how do you propose to do that?\n\r", ch );
         
return;
      
}
      
if( ch->gold < amount )
         
      {
         
send_to_char( "Very generous of you, but you haven't got that many coins.\n\r", ch );
         
return;
      
}
      
if( !IS_SAME_PLANE( ch, victim ) )
      {
         
send_to_char( "Your hands pass right through them!", ch );
         
return;
      
}
      
ch->gold -= amount;
      
victim->gold += amount;
      
strcpy( buf, "$n gives you " );
      
gp = amount;
      
sprintf( buf2, "&C%d mithril, ", ( int )gp / 1000000 );
      
strcat( buf, buf2 );
      
gp = gp % 1000000;
      
sprintf( buf2, "&Y%d gold, ", ( int )gp / 10000 );
      
strcat( buf, buf2 );
      
gp = gp % 10000;
      
sprintf( buf2, "&W%d silver, ", ( int )gp / 100 );
      
strcat( buf, buf2 );
      
gp = gp % 100;
      
sprintf( buf2, "and &O%d copper coins.\n\r", gp );
      
strcat( buf, buf2 );
      
act( AT_ACTION, buf, ch, NULL, victim, TO_VICT );
      
act( AT_ACTION, "$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT );
      
act( AT_ACTION, "You give $N some coins.", ch, NULL, victim, TO_CHAR );
      
send_to_char( "OK.\n\r", ch );
      
mprog_bribe_trigger( victim, ch, amount );
      
if( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died( ch ) )
         
save_char_obj( ch );
      
if( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died( victim ) )
         
save_char_obj( victim );
      
return;
   
}
   
if( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
      
   {
      
send_to_char( "You do not have that item.\n\r", ch );
      
return;
   
}
   
if( obj->wear_loc != WEAR_NONE )
      
   {
      
send_to_char( "You must remove it first.\n\r", ch );
      
return;
   
}
   
if( ( victim = get_char_room( ch, arg2 ) ) == NULL )
      
   {
      
send_to_char( "They aren't here.\n\r", ch );
      
return;
   
}
   
if( !IS_SAME_PLANE( ch, victim ) )
   {
      
send_to_char( "Your hands pass right through them!", ch );
      
return;
   
}
   
if( IS_AFFECTED( victim, AFF_HAS_ARTI ) && IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      
act( AT_ACTION, "$p evades $n's grasp.", victim, obj, NULL, TO_ROOM );
      
act( AT_ACTION, "$n tries to give you $p but it evades your grasp.", ch, obj, victim, TO_VICT );
      
return;
   
}
   
if( IS_AFFECTED( ch, AFF_ETHEREAL ) )
   {
      
act( AT_ACTION, "$n tries to give you $p but your hand passes through it.", victim, obj, ch, TO_VICT );
      
act( AT_ACTION, "$n's hand passes through $p.", victim, obj, NULL, TO_ROOM );
      
return;
   
}
   
if( victim->carry_number + ( get_obj_number( obj ) / obj->count ) > can_carry_n( victim ) )
      
   {
      
act( AT_PLAIN, "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
      
return;
   
}
   
if( victim->carry_weight + ( get_obj_weight( obj ) / obj->count ) > can_carry_w( victim ) )
      
   {
      
act( AT_PLAIN, "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
      
return;
   
}
   
if( !can_see_obj( victim, obj ) )
      
   {
      
act( AT_PLAIN, "$N can't see it.", ch, NULL, victim, TO_CHAR );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
      
   {
      
act( AT_PLAIN, "You cannot give that to $N!", ch, NULL, victim, TO_CHAR );
      
return;
   
}
   
separate_obj( obj );
   
obj_from_char( obj );
   
act( AT_ACTION, "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
   
act( AT_ACTION, "$n gives you $p.", ch, obj, victim, TO_VICT );
   
act( AT_ACTION, "You give $p to $N.", ch, obj, victim, TO_CHAR );
   
obj = obj_to_char( obj, victim );
   
mprog_give_trigger( victim, ch, obj );
   
if( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died( ch ) )
      
save_char_obj( ch );
   
if( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died( victim ) )
      
save_char_obj( victim );
   
return;

}


/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */ 
   obj_ret damage_obj( OBJ_DATA * obj ) 
{
   
CHAR_DATA * ch;
   
obj_ret objcode;
   
int chance = 50;
   
ch = obj->carried_by;
   
objcode = rNONE;
   
separate_obj( obj );
   
if( ch && !IS_NPC( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG ) )
      
act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
   
   else
   
if( obj->in_room && ( ch = obj->in_room->first_person ) != NULL )
      
   {
      
act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_ROOM );
      
act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
      
ch = NULL;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_METAL ) )
      
chance += 20;
   
if( IS_OBJ_STAT( obj, ITEM_ORGANIC ) )
      
chance -= 20;
   
if( IS_OBJ_STAT( obj, ITEM_BLESS ) )
      
chance += 20;
   
if( number_range( 0, 100 ) >= chance )
      
return rNONE;
   
obj->condition -= number_range( 1, 10 );
   
if( obj->condition <= 0 )
   {
      
make_scraps( obj );
      
return rOBJ_SCRAPPED;
   
}
   
return objcode;

}


/*
 * See if char could be capable of dual-wielding		-Thoric
 */ 
   bool could_dual( CHAR_DATA * ch ) 
{
   
return TRUE;

}


/*
 * See if char can dual wield at this time			-Thoric
 */ 
   bool can_dual( CHAR_DATA * ch ) 
{
   
OBJ_DATA * obj;
   
if( !could_dual( ch ) )
      
return FALSE;
   
if( ( obj = get_eq_char( ch, WEAR_HAND ) ) && get_eq_char( ch, WEAR_HAND2 ) )
      
   {
      
send_to_char( "You are already holding two items!\n\r", ch );
      
return FALSE;
   
}
   
if( xIS_SET( obj->extra_flags, ITEM_TWO_HANDED ) )
   {
      
send_to_char( "You cannot dual wield with a two-handed weapon!\n\r", ch );
      
return FALSE;
   
}
   
if( get_eq_char( ch, WEAR_SHIELD ) )
      
   {
      
send_to_char( "You cannot dual wield while holding a shield!\n\r", ch );
      
return FALSE;
   
}
   
return TRUE;

}


/*
 * Check to see if there is room to wear another object on this location
 * (Layered clothing support)
 */ 
   bool can_layer( CHAR_DATA * ch, OBJ_DATA * obj, sh_int wear_loc ) 
{
   
OBJ_DATA * otmp;
   
sh_int bitlayers = 0;
   
sh_int objlayers = obj->pIndexData->layers;
   
for( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
      
if( otmp->wear_loc == wear_loc )
      {
         
if( !otmp->pIndexData->layers )
            
return FALSE;
         
         else
            
bitlayers |= otmp->pIndexData->layers;
      
}
   
if( ( bitlayers && !objlayers ) || bitlayers > objlayers )
      
return FALSE;
   
if( !bitlayers || ( ( bitlayers & ~objlayers ) == bitlayers ) )
      
return TRUE;
   
return FALSE;

}

void do_bury( CHAR_DATA * ch, char *argument ) 
{
   
char arg[MAX_INPUT_LENGTH];
   
OBJ_DATA * obj;
   
bool shovel;
   
sh_int move;
   
one_argument( argument, arg );
   
if( arg[0] == '\0' )
      
   {
      
send_to_char( "What do you wish to bury?\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
shovel = FALSE;
   
for( obj = ch->first_carrying; obj; obj = obj->next_content )
      
if( obj->item_type == ITEM_SHOVEL )
         
      {
         
shovel = TRUE;
         
break;
      
}
   
obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
   
if( !obj )
      
   {
      
send_to_char( "You can't find it.\n\r", ch );
      
return;
   
}
   
separate_obj( obj );
   
switch ( ch->in_room->sector_type )
      
   {
      
case SECT_CITY:
      
case SECT_INSIDE:
         
send_to_char( "The floor is too hard to dig through.\n\r", ch );
         
return;
      
case SECT_WATER_SWIM:
      
case SECT_WATER_NOSWIM:
      
case SECT_UNDERWATER:
         
send_to_char( "You cannot bury something here.\n\r", ch );
         
return;
      
case SECT_AIR:
         
send_to_char( "What?  In the air?!\n\r", ch );
         
return;
   
}
   
if( obj->weight > ( UMAX( 5, ( can_carry_w( ch ) / 10 ) ) ) 
 &&!shovel )
      
   {
      
send_to_char( "You'd need a shovel to bury something that big.\n\r", ch );
      
return;
   
}
   
move = ( obj->weight * 50 * ( shovel ? 1 : 5 ) ) / UMAX( 1, can_carry_w( ch ) );
   
move = URANGE( 2, move, 1000 );
   
if( move > ch->move )
      
   {
      
send_to_char( "You don't have the energy to bury something of that size.\n\r", ch );
      
return;
   
}
   
ch->move -= move;
   
act( AT_ACTION, "You solemnly bury $p...", ch, obj, NULL, TO_CHAR );
   
act( AT_ACTION, "$n solemnly buries $p...", ch, obj, NULL, TO_ROOM );
   
xSET_BIT( obj->extra_flags, ITEM_BURIED );
   
WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );
   
return;

}

void do_brandish( CHAR_DATA * ch, char *argument ) 
{
   
CHAR_DATA * vch;
   
CHAR_DATA * vch_next;
   
OBJ_DATA * staff;
   
ch_ret retcode;
   
int sn;
   
if( ( staff = get_eq_char( ch, WEAR_HAND ) ) == NULL )
      
   {
      
send_to_char( "You hold nothing in your hand.\n\r", ch );
      
return;
   
}
   
if( staff->item_type != ITEM_STAFF )
      
   {
      
send_to_char( "You can brandish only with a staff.\n\r", ch );
      
return;
   
}
   
if( ( sn = staff->value[3] ) < 0 
 ||sn >= top_sn 
 ||skill_table[sn]->spell_fun == NULL )
      
   {
      
bug( "Do_brandish: bad sn %d on object vnum %d.", sn, staff->pIndexData->vnum );
      
return;
   
}
   
WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
   
if( staff->value[2] > 0 )
      
   {
      
if( !oprog_use_trigger( ch, staff, NULL, NULL, NULL ) )
         
      {
         
act( AT_MAGIC, "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
         
act( AT_MAGIC, "You brandish $p.", ch, staff, NULL, TO_CHAR );
      
}
      
for( vch = ch->in_room->first_person; vch; vch = vch_next )
         
      {
         
vch_next = vch->next_in_room;
         
if( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS ) )
            
continue;
         
         else
            
switch ( skill_table[sn]->target )
               
            {
               
default:
                  
bug( "Do_brandish: bad target for sn %d.", sn );
                  
return;
               
case TAR_IGNORE:
                  
if( vch != ch )
                     
continue;
                  
break;
               
case TAR_CHAR_OFFENSIVE:
                  
if( IS_NPC( ch ) ? IS_NPC( vch ) : !IS_NPC( vch ) )
                     
continue;
                  
break;
               
case TAR_CHAR_DEFENSIVE:
                  
if( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
                     
continue;
                  
break;
               
case TAR_CHAR_SELF:
                  
if( vch != ch )
                     
continue;
                  
break;
            
}
         
retcode = obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
         
if( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
            
         {
            
bug( "do_brandish: char died", 0 );
            
return;
         
}
      
}
   
}
   
if( --staff->value[2] <= 0 )
      
   {
      
act( AT_MAGIC, "$p blazes bright and vanishes from $n's hands!", ch, staff, NULL, TO_ROOM );
      
act( AT_MAGIC, "$p blazes bright and is gone!", ch, staff, NULL, TO_CHAR );
      
if( staff->serial == cur_obj )
         
global_objcode = rOBJ_USED;
      
extract_obj( staff );
   
}
   
return;

}


/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */ 

void obj_fall( OBJ_DATA * obj, bool through ) 
{
   
EXIT_DATA * pexit;
   
ROOM_INDEX_DATA * to_room;
   
static int fall_count;
   
char buf[MAX_STRING_LENGTH];
   
static bool is_falling;   /* Stop loops from the call to obj_to_room()  -- Altrag */
   
if( !obj->in_room || is_falling )
      
return;
   
if( fall_count > 80 )
      
   {
      
bug( "object falling in loop more than 80 times", 0 );
      
extract_obj( obj );
      
fall_count = 0;
      
return;
   
}
   
if( IS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) 
 &&CAN_GO( obj, DIR_DOWN ) 
 &&!IS_OBJ_STAT( obj, ITEM_HOVER ) )
      
   {
      
pexit = get_exit( obj->in_room, DIR_DOWN );
      
to_room = pexit->to_room;
      
if( through )
         
fall_count++;
      
      else
         
fall_count = 0;
      
if( obj->in_room == to_room )
         
      {
         
sprintf( buf, "Object falling into same room, room %d", 
to_room->vnum );
         
bug( buf, 0 );
         
extract_obj( obj );
         
return;
      
}
      
if( obj->in_room->first_person )
         
      {
         
act( AT_PLAIN, "$p falls far below.", 
obj->in_room->first_person, obj, NULL, TO_ROOM );
         
act( AT_PLAIN, "$p falls far below.", 
obj->in_room->first_person, obj, NULL, TO_CHAR );
      
}
      
obj_from_room( obj );
      
is_falling = TRUE;
      
obj = obj_to_room( obj, to_room );
      
is_falling = FALSE;
      
if( obj->in_room->first_person )
         
      {
         
act( AT_PLAIN, "$p falls from above.", 
obj->in_room->first_person, obj, NULL, TO_ROOM );
         
act( AT_PLAIN, "$p falls from above.", 
obj->in_room->first_person, obj, NULL, TO_CHAR );
      
}
      
if( !IS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) && through )
         
      {
         
int dam = obj->weight * ( fall_count / 2 );
         
            /*
             * Damage players 
             */ 
            if( obj->in_room->first_person && number_percent(  ) > 15 )
            
         {
            
CHAR_DATA * rch;
            
CHAR_DATA * vch = NULL;
            
int chcnt = 0;
            
for( rch = obj->in_room->first_person; rch; 
rch = rch->next_in_room, chcnt++ )
               
if( number_range( 0, chcnt ) == 0 )
                  
vch = rch;
            
act( AT_WHITE, "$p falls on $n!", vch, obj, NULL, TO_ROOM );
            
act( AT_WHITE, "$p falls on you!", vch, obj, NULL, TO_CHAR );
            
lose_hp( vch, dam );
         
}
         
            /*
             * Damage objects 
             */ 
            switch ( obj->item_type )
            
         {
            
case ITEM_WEAPON:
            
case ITEM_ARMOR:
               
if( ( obj->value[0] - dam ) <= 0 )
                  
               {
                  
if( obj->in_room->first_person )
                     
                  {
                     
act( AT_PLAIN, "$p is destroyed by the fall!", 
obj->in_room->first_person, obj, NULL, TO_ROOM );
                     
act( AT_PLAIN, "$p is destroyed by the fall!", 
obj->in_room->first_person, obj, NULL, TO_CHAR );
                  
}
                  
make_scraps( obj );
               
}
               
               else
                  
obj->value[0] -= dam;
               
break;
            
default:
               
if( ( dam * 15 ) > get_obj_resistance( obj ) )
                  
               {
                  
if( obj->in_room->first_person )
                     
                  {
                     
act( AT_PLAIN, "$p is destroyed by the fall!", 
obj->in_room->first_person, obj, NULL, TO_ROOM );
                     
act( AT_PLAIN, "$p is destroyed by the fall!", 
obj->in_room->first_person, obj, NULL, TO_CHAR );
                  
}
                  
make_scraps( obj );
               
}
               
break;
         
}
      
}
      
obj_fall( obj, TRUE );
   
}
   
return;

}


/* Scryn, by request of Darkur, 12/04/98 */ 
/* Reworked recursive_note_find to fix crash bug when the note was left
 * blank.  7/6/98 -- Shaddai
 */ 

void do_findnote( CHAR_DATA * ch, char *argument ) 
{
   
OBJ_DATA * obj;
   
if( IS_NPC( ch ) )
      
   {
      
send_to_char( "Huh?\n\r", ch );
      
return;
   
}
   
if( argument[0] == '\0' )
      
   {
      
send_to_char( "You must specify at least one keyword.\n\r", ch );
      
return;
   
}
   
obj = recursive_note_find( ch->first_carrying, argument );
   
if( obj )
      
   {
      
if( obj->in_obj )
         
      {
         
obj_from_obj( obj );
         
obj = obj_to_char( obj, ch );
      
}
   
}
   
   else
      
send_to_char( "Note not found.\n\r", ch );
   
return;

}


OBJ_DATA * recursive_note_find( OBJ_DATA * obj, char *argument )
{
   
OBJ_DATA * returned_obj;
   
bool match = TRUE;
   
char *argcopy;
   
char *subject;
   
char arg[MAX_INPUT_LENGTH];
   
char subj[MAX_STRING_LENGTH];
   
if( !obj )
      
return NULL;
   
switch ( obj->item_type )
      
   {
      
case ITEM_PAPER:
         
if( ( subject = get_extra_descr( "_subject_", obj->first_extradesc ) ) == NULL )
            
break;
         
sprintf( subj, "%s", strlower( subject ) );
         
subject = strlower( subj );
         
argcopy = argument;
         
while( match )
            
         {
            
argcopy = one_argument( argcopy, arg );
            
if( arg[0] == '\0' )
               
break;
            
if( !strstr( subject, arg ) )
               
match = FALSE;
         
}
         
if( match )
            
return obj;
         
break;
      
case ITEM_CONTAINER:
      
case ITEM_CORPSE_NPC:
      
case ITEM_CORPSE_PC:
         
if( obj->first_content )
            
         {
            
returned_obj = recursive_note_find( obj->first_content, argument );
            
if( returned_obj )
               
return returned_obj;
         
}
         
break;
      
default:
         
break;
   
}
   
return recursive_note_find( obj->next_content, argument );

}


