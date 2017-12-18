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
 *			     Player skills module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
   
/* from makeobjs.c  */ 
extern OBJ_DATA *make_ore( int number );

/* from handler.c */ 
extern char *munch_colors( char *word );

/* from talent.c */ 
void player_echo( CHAR_DATA * ch, sh_int AT_COLOR, char *argument, sh_int tar );

char *const spell_flag[] = 
   { "water", "earth", "air", "astral", "area", "distant", "reverse", 
"noself", "ground", "accumulative", "recastable",
   "noscribe", 
"nobrew", "group", "object", "character", "secretskill", "pksensitive", 
"stoponfail", "nofight", "nodispel",
   "instant", "r2", "r3",
   "r4", "r5", 
"r6", 
"r7", "r8", "r9", "r10", "r11" 
};

char *const spell_damage[] = 
{ "none", "fire", "cold", "electricity", "energy", "acid", "poison", "drain" };

char *const spell_action[] = 
{ "none", "create", "destroy", "resist", "suscept", "divinate", "obscure", 
"change"
};

char *const spell_power[] = 
{ "none", "minor", "greater", "major" };

char *const spell_class[] = 
{ "none", "lunar", "solar", "travel", "summon", "life", "death", "illusion" };

char *const target_type[] = 
{ "ignore", "offensive", "defensive", "self", "objinv" };

void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch );

void show_list_to_char( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, 
bool fShowN );

int ris_save( CHAR_DATA * ch, int chance, int ris );

bool check_illegal_psteal( CHAR_DATA * ch, CHAR_DATA * victim );

char *scramble( const char *argument, int modifier );

/* from magic.c */ 
void failed_casting( struct skill_type *skill, CHAR_DATA * ch, 
CHAR_DATA * victim, OBJ_DATA * obj );

/*
 * Dummy function
 */ 
void skill_notfound( CHAR_DATA * ch, char *argument ) 
{
   
huh( ch );
   
return;

}

int get_starget( char *name ) 
{
   
int x;
   
for( x = 0; x < sizeof( target_type ) / sizeof( target_type[0] ); x++ )
      
if( !str_cmp( name, target_type[x] ) )
         
return x;
   
return -1;

}

int get_sflag( char *name ) 
{
   
int x;
   
for( x = 0; x < sizeof( spell_flag ) / sizeof( spell_flag[0] ); x++ )
      
if( !str_cmp( name, spell_flag[x] ) )
         
return x;
   
return -1;

}

int get_sdamage( char *name ) 
{
   
int x;
   
for( x = 0; x < sizeof( spell_damage ) / sizeof( spell_damage[0] ); x++ )
      
if( !str_cmp( name, spell_damage[x] ) )
         
return x;
   
return -1;

}

int get_saction( char *name ) 
{
   
int x;
   
for( x = 0; x < sizeof( spell_action ) / sizeof( spell_action[0] ); x++ )
      
if( !str_cmp( name, spell_action[x] ) )
         
return x;
   
return -1;

}

int get_spower( char *name ) 
{
   
int x;
   
for( x = 0; x < sizeof( spell_power ) / sizeof( spell_power[0] ); x++ )
      
if( !str_cmp( name, spell_power[x] ) )
         
return x;
   
return -1;

}

int get_sclass( char *name ) 
{
   
int x;
   
for( x = 0; x < sizeof( spell_class ) / sizeof( spell_class[0] ); x++ )
      
if( !str_cmp( name, spell_class[x] ) )
         
return x;
   
return -1;

}


bool is_legal_kill( CHAR_DATA * ch, CHAR_DATA * vch ) 
{
   
if( IS_NPC( ch ) || IS_NPC( vch ) )
      
return TRUE;
   
if( !IS_PKILL( ch ) || !IS_PKILL( vch ) )
      
return FALSE;
   
return TRUE;

}

extern char *target_name;  /* from magic.c */

/* Figure out the current maximum the char can get a skill to -- Scion */ 
int get_adept( CHAR_DATA * ch, int sn )
{
   
int stat1;
   
int stat2;
   
switch ( skill_table[sn]->stat1 )
   {
      
case 1:
         
stat1 = get_curr_str( ch );
         
break;
      
case 2:
         
stat1 = get_curr_int( ch );
         
break;
      
case 3:
         
stat1 = get_curr_wil( ch );
         
break;
      
case 4:
         
stat1 = get_curr_dex( ch );
         
break;
      
case 5:
         
stat1 = get_curr_con( ch );
         
break;
      
case 6:
         
stat1 = get_curr_per( ch );
         
break;
      
case 7:
         
stat1 = get_curr_lck( ch );
         
break;
      
default:
         
stat1 = 0;
         
break;
   
}
   
switch ( skill_table[sn]->stat2 )
   {
      
case 1:
         
stat2 = get_curr_str( ch );
         
break;
      
case 2:
         
stat2 = get_curr_int( ch );
         
break;
      
case 3:
         
stat2 = get_curr_wil( ch );
         
break;
      
case 4:
         
stat2 = get_curr_dex( ch );
         
break;
      
case 5:
         
stat2 = get_curr_con( ch );
         
break;
      
case 6:
         
stat2 = get_curr_per( ch );
         
break;
      
case 7:
         
stat2 = get_curr_lck( ch );
         
break;
      
default:
         
stat2 = stat1;
         
break;
   
}
   
      /*
       * Average the two stats for the skill cap 
       */ 
      return UMIN( ( ( stat1 + stat2 ) / 2 ) * 3, 95 );

}


/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */ 
   bool check_skill( CHAR_DATA * ch, char *command, char *argument )
{
   
int sn;
   
int first = gsn_first_skill;
   
int top = gsn_top_sn - 1;
   
int mana;
   
struct timeval time_used;
   
      /*
       * bsearch for the skill 
       */ 
      for( ;; )
      
   {
      
sn = ( first + top ) >> 1;
      
if( sn < 0 )
         
return FALSE;
      
if( LOWER( command[0] ) == LOWER( skill_table[sn]->name[0] ) 
            &&!str_prefix( command, skill_table[sn]->name ) 
            &&( skill_table[sn]->skill_fun || skill_table[sn]->spell_fun != spell_null ) 
 &&( can_use_skill( ch, 0, sn ) ) )
         
break;
      
if( first >= top )
         
return FALSE;
      
if( strcmp( command, skill_table[sn]->name ) < 1 )
         
top = sn - 1;
      
      else
         
first = sn + 1;
   
}
   
if( !check_pos( ch, skill_table[sn]->minimum_position ) )
      
return TRUE;
   
if( IS_NPC( ch ) 
 &&( IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS ) ) )
      
   {
      
send_to_char( "For some reason, you seem unable to perform that...\n\r", ch );
      
act( AT_GREY, "$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM );
      
return TRUE;
   
}
   
      /*
       * check if mana is required 
       */ 
      if( skill_table[sn]->min_mana )
      
   {
      
mana =
         IS_NPC( ch ) ? 0 : UMAX( skill_table[sn]->min_mana,
                                  
100 / ( 100 - skill_table[sn]->skill_level[get_best_talent( ch, sn )] ) );
      
if( !IS_NPC( ch ) && ch->mana < mana )
         
      {
         
send_to_char( "You don't have enough mana.\n\r", ch );
         
return TRUE;
      
}
   
}
   
   else
      
   {
      
mana = 0;
   
}
   
      /*
       * 
       * * Is this a real do-fun, or a really a spell?
       */ 
      if( !skill_table[sn]->skill_fun )
      
   {
      
ch_ret retcode = rNONE;
      
void *vo = NULL;
      
CHAR_DATA * victim = NULL;
      
OBJ_DATA * obj = NULL;
      
target_name = "";
      
switch ( skill_table[sn]->target )
         
      {
         
default:
            
bug( "Check_skill: bad target for sn %d.", sn );
            
send_to_char( "Something went wrong...\n\r", ch );
            
return TRUE;
         
case TAR_IGNORE:
            
vo = NULL;
            
if( argument[0] == '\0' )
               
            {
               
if( ( victim = ch->last_hit ) != NULL )
                  
target_name = victim->name;
            
}
            
            else
               
target_name = argument;
            
break;
         
case TAR_CHAR_OFFENSIVE:
            
if( argument[0] == '\0' 
 &&( victim = ch->last_hit ) == NULL )
               
            {
               
ch_printf( ch, "Confusion overcomes you as your '%s' has no target.\n\r", skill_table[sn]->name );
               
return TRUE;
            
}
            
            else
            
if( argument[0] != '\0' 
 &&( victim = get_char_room( ch, argument ) ) == NULL )
               
            {
               
send_to_char( "They aren't here.\n\r", ch );
               
return TRUE;
            
}
            
if( !IS_SAME_PLANE( ch, victim ) )
               
            {
               
ch_printf( ch, "Your %s passes right through them!\n\r", skill_table[sn]->name );
               
return TRUE;
            
}
            
if( ch == victim && SPELL_FLAG( skill_table[sn], SF_NOSELF ) )
               
            {
               
send_to_char( "You can't target yourself!\n\r", ch );
               
return TRUE;
            
}
            
if( !IS_NPC( ch ) )
               
            {
               
if( !IS_NPC( victim ) )
               {
                  
if( get_timer( ch, TIMER_PKILLED ) > 0 )
                     
                  {
                     
send_to_char( "You have been killed in the last 5 minutes.\n\r", ch );
                     
return TRUE;
                  
}
                  
if( get_timer( victim, TIMER_PKILLED ) > 0 )
                     
                  {
                     
send_to_char( "This player has been killed in the last 5 minutes.\n\r", ch );
                     
return TRUE;
                  
}
               
}
               
if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
                  
               {
                  
send_to_char( "You can't do that on your own follower.\n\r", ch );
                  
return TRUE;
               
}
            
}
            
vo = ( void * )victim;
            
break;
         
case TAR_CHAR_DEFENSIVE:
            
if( argument[0] != '\0' 
 &&( victim = get_char_room( ch, argument ) ) == NULL )
               
            {
               
send_to_char( "They aren't here.\n\r", ch );
               
return TRUE;
            
}
            
if( !victim )
               
victim = ch;
            
if( !IS_SAME_PLANE( ch, victim ) )
               
            {
               
ch_printf( ch, "Your %s passes right through them!\n\r", skill_table[sn]->name );
               
return TRUE;
            
}
            
if( ch == victim && SPELL_FLAG( skill_table[sn], SF_NOSELF ) )
               
            {
               
send_to_char( "You can't target yourself!\n\r", ch );
               
return TRUE;
            
}
            
vo = ( void * )victim;
            
break;
         
case TAR_CHAR_SELF:
            
vo = ( void * )ch;
            
break;
         
case TAR_OBJ_INV:
            
if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
               
            {
               
send_to_char( "You can't find that.\n\r", ch );
               
return TRUE;
            
}
            
vo = ( void * )obj;
            
break;
      
}
      
         /*
          * waitstate 
          */ 
         WAIT_STATE( ch, skill_table[sn]->beats );
      
         /*
          * check for failure 
          */ 
         if( ( number_percent(  ) + skill_table[sn]->difficulty * 5 ) 
 >( IS_NPC( ch ) ? 75 : LEARNED( ch, sn ) ) )
         
      {
         
failed_casting( skill_table[sn], ch, vo, obj );
         
learn_from_failure( ch, sn );
         
if( mana )
            
         {
            
ch->mana -= mana / 2;
            
ch->in_room->area->weather->mana += ( int )mana / 10;
         
}
         
return TRUE;
      
}
      
if( mana )
         
      {
         
ch->mana -= mana;
         
ch->in_room->area->weather->mana += ( int )mana / 10;
      
}
      
start_timer( &time_used );
      
retcode = ( *skill_table[sn]->spell_fun ) ( sn, get_curr_wil( ch ), ch, vo );
      
end_timer( &time_used );
      
update_userec( &time_used, &skill_table[sn]->userec );
      
if( retcode == rCHAR_DIED || retcode == rERROR )
         
return TRUE;
      
if( char_died( ch ) )
         
return TRUE;
      
if( retcode == rSPELL_FAILED )
         
      {
         
learn_from_failure( ch, sn );
         
retcode = rNONE;
      
}
      
      else
      {
         
learn_from_success( ch, sn );
      
}
      
return TRUE;
   
}
   
if( mana )
      
   {
      
ch->mana -= mana;
      
ch->in_room->area->weather->mana += ( int )mana / 10;
   
}
   
      /*
       * waitstate 
       */ 
      WAIT_STATE( ch, skill_table[sn]->beats );
   
ch->prev_cmd = ch->last_cmd;   /* haus, for automapping */
   
ch->last_cmd = skill_table[sn]->skill_fun;
   
start_timer( &time_used );
   
( *skill_table[sn]->skill_fun ) ( ch, argument );
   
end_timer( &time_used );
   
update_userec( &time_used, &skill_table[sn]->userec );
   
tail_chain(  );
   
return TRUE;

}

void do_skin( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA *corpse, *skin;   
bool found;
char *name;
int percent;
char buf[MAX_STRING_LENGTH];
   
found = FALSE;
   
if( !can_use_skill( ch, 0, gsn_skin ) )
            
         {
            
send_to_char( "You do not yet know how to skin.\n\r", ch );
            
return;
         
}   
   
   
if( argument[0] == '\0' )
      
   {
      
send_to_char( "What corpse do you wish to skin?\n\r", ch );
      
return;
   
}
   
if( ( corpse = get_obj_here( ch, argument ) ) == NULL )
      
   {
      
send_to_char( "You cannot find that here.\n\r", ch );
      
return;
   
}
   //this is the old way CM3 had the check for a weapon or claws, but it didn't work so changed it
//if( ( obj = get_eq_char( ch, WEAR_HAND ) ) == NULL 
// &&!can_use_bodypart( ch, BP_CLAWS ) )

	 if(( !ch->main_hand || ch->main_hand->value[5] != SK_DAGGER ) && !can_use_bodypart( ch, BP_CLAWS ) )
   {
      
send_to_char( "You have no weapon with which to skin this creature.\n\r", ch );
      
return;
   
}
   
if( corpse->item_type != ITEM_CORPSE_NPC )
      
   {
      
send_to_char( "You can only skin the corpses of creatures.\n\r", ch );
      
return;
   
}
   

      

   
if( corpse->value[5] == 1 )
   {
      
send_to_char( "That corpse seems to have been skinned already.\r\n", ch );
      
return;
   
}
   
if( get_obj_index( OBJ_VNUM_SKIN ) == NULL )
      
   {
      
bug( "Vnum 23 (OBJ_VNUM_SKIN) not found for do_skin!", 0 );
      
return;
   
}
   
   
   
percent = number_percent(  ) - ( get_curr_lck( ch ) / 5 );
   
if( !can_use_skill( ch, percent, gsn_skin ) )
      
   {
      
send_to_char( "You mangle the skin beyond recognition.\n\r", ch );
      
	  corpse->value[5] = 1;
	  
learn_from_failure( ch, gsn_skin );
      
return;
   
}
   
skin = create_object( get_obj_index( OBJ_VNUM_SKIN ), 0 );
   
name = corpse->short_descr;
   
sprintf( buf, skin->short_descr, name );
   
STRFREE( skin->short_descr );
   
skin->short_descr = STRALLOC( buf );
   
sprintf( buf, skin->description, name );
   
STRFREE( skin->description );
   
skin->description = STRALLOC( buf );
   
act( AT_BLOOD, "$n skins $p.", ch, corpse, NULL, TO_ROOM );
   
act( AT_BLOOD, "You skin $p.", ch, corpse, NULL, TO_CHAR );
   
corpse->value[5] = 1;
   
obj_to_char( skin, ch );
   
learn_from_success( ch, gsn_skin );

learn_noncombat( ch, SK_NATURE );
   
return;

}

void do_invoke( CHAR_DATA * ch, char *argument )
{
	OBJ_DATA *fire, *shard;
	bool found;
	//int mana, dam;
	int mana;
	 
	 
if( !can_use_skill( ch, 0, gsn_invoke ) )
   {
      send_to_char( "A skill such as this requires more magical ability than you have.\n\r", ch );
      return;
   }
if( argument[0] == '\0' )
	{
		send_to_char( "Invoke which element?\n\r", ch);
		return;
	}
   
		mana = (ch->max_mana / 5);
	if( !str_cmp( argument, "fireshard" ) || !str_cmp( argument, "fire" ) )
	{
			found = FALSE;
			for( fire = ch->in_room->first_content; fire; fire = fire->next_content )
				{
					if( fire->item_type == ITEM_FIRE )
					{
					found = TRUE;
					break;
					}
				}
			if( !found )
			{
				send_to_char(" There must be a fire here to invoke a FireShard.\n\r", ch);
				return;
			}
		   if( !can_use_skill( ch, number_percent(  ), gsn_invoke ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You failed to make a FireShard.\n\r", ch );
				learn_from_failure( ch, gsn_invoke );
				ch->mana -= ( mana );
				return;
			}

					//SUCCESS
					
					shard = create_object( get_obj_index( OBJ_VNUM_SM_FIRESHARD ), 0 );
					learn_from_success(ch, gsn_invoke);
					ch->mana -= (ch->mana / 10);
					act( AT_MAGIC, "$n invokes the fire and creates a FireShard.", ch, NULL, NULL, TO_ROOM );
					act( AT_MAGIC, "You invoke the fire and create a FireShard.", ch, NULL, NULL, TO_CHAR );
					obj_to_char( shard, ch);
					learn_noncombat( ch, SK_CHANNEL);
					learn_talent( ch, TAL_CATALYSM );
			
	}
	
	if( !str_cmp( argument, "water" ) || !str_cmp( argument, "watershard") )
	{

			if( ch->in_room->curr_water < 15 )
			{
				send_to_char(" There must be some water here to invoke a WaterShard.\n\r", ch);
				return;
			}
		   if( !can_use_skill( ch, number_percent(  ), gsn_invoke ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You failed to make a WaterShard.\n\r", ch );
				learn_from_failure( ch, gsn_invoke );
				ch->mana -= ( mana );
				return;
			}

					//SUCCESS
					
					shard = create_object( get_obj_index( OBJ_VNUM_SM_WATERSHARD ), 0 );
					learn_from_success(ch, gsn_invoke);
					ch->mana -= (mana / 2);
					act( AT_MAGIC, "$n invokes the water here and creates a WaterShard.", ch, NULL, NULL, TO_ROOM );
					act( AT_MAGIC, "You invoke the water here and creates a WaterShard.", ch, NULL, NULL, TO_CHAR );
					obj_to_char( shard, ch);
					learn_noncombat( ch, SK_CHANNEL);
					learn_talent( ch, TAL_CATALYSM );
			
	}
	
	if( !str_cmp( argument, "airshard" ) || !str_cmp( argument, "air" ))
	{

			if( IS_UNDERWATER( ch )  )
			{
				send_to_char(" There is no air here for an AirShard.\n\r", ch);
				return;
			}
		   if( !can_use_skill( ch, number_percent(  ), gsn_invoke ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You failed to make an AirShard.\n\r", ch );
				learn_from_failure( ch, gsn_invoke );
				ch->mana -= ( mana );
				return;
			}

					//SUCCESS
					
					shard = create_object( get_obj_index( OBJ_VNUM_SM_AIRSHARD ), 0 );
					learn_from_success(ch, gsn_invoke);
					ch->mana -= (mana / 2);
					act( AT_MAGIC, "$n invokes the surrounding air and creates an AirShard.", ch, NULL, NULL, TO_ROOM );
					act( AT_MAGIC, "You invoke the surrounding air and creates an AirShard.", ch, NULL, NULL, TO_CHAR );
					obj_to_char( shard, ch);
					learn_noncombat( ch, SK_CHANNEL);
					learn_talent( ch, TAL_CATALYSM );
			
	}
	
	if( !str_cmp( argument, "earthshard" ) || !str_cmp( argument, "earth" ))
	{

			if( IS_AFFECTED( ch, AFF_FLYING ) || IS_SET( ch->in_room->room_flags, ROOM_INDOORS ))// || IS_AFFECTED( ch, AFF_FLOATING ))
			{
				send_to_char(" You cannot reach the ground to create an EarthShard.\n\r", ch);
				return;
			}
		   if( !can_use_skill( ch, number_percent(  ), gsn_invoke ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You failed to make an EarthShard.\n\r", ch );
				learn_from_failure( ch, gsn_invoke );
				ch->mana -= ( mana );
				return;
			}

					//SUCCESS
					
					shard = create_object( get_obj_index( OBJ_VNUM_SM_EARTHSHARD ), 0 );
					learn_from_success(ch, gsn_invoke);
					ch->mana -= (mana / 2);
					act( AT_MAGIC, "$n invokes the surrounding earth and creates an EarthShard.", ch, NULL, NULL, TO_ROOM );
					act( AT_MAGIC, "You invoke the surrounding earth and creates an EarthShard.", ch, NULL, NULL, TO_CHAR );
					obj_to_char( shard, ch);
					learn_noncombat( ch, SK_CHANNEL);
					learn_talent( ch, TAL_CATALYSM );
			
	}
	// Decided against being able to get soulshards from yourself
	/*
	if( !str_cmp( argument, "soulshard" ) || !str_cmp( argument, "soul" ) )
	{
		dam = ch->max_hit / 3;
		   if( !can_use_skill( ch, number_percent(  ), gsn_invoke ) )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You failed to make an SoulShard.\n\r", ch );
				learn_from_failure( ch, gsn_invoke );
				ch->mana -= ( mana );
				return;
			}

					//SUCCESS
					
					shard = create_object( get_obj_index( OBJ_VNUM_SM_SOULSHARD ), 0 );
					learn_from_success(ch, gsn_invoke);
					ch->mana -= (mana * 4);
					ch->hit -= (dam * 2);
					act( AT_MAGIC, "$n invokes their soul and creates a SoulShard.", ch, NULL, NULL, TO_ROOM );
					act( AT_MAGIC, "You invoke your soul and creates a SoulShard.", ch, NULL, NULL, TO_CHAR );
					obj_to_char( shard, ch);
					learn_noncombat( ch, SK_CHANNEL);
					learn_talent( ch, TAL_CATALYSM );
			
	}*/
	
}

/*
 * Lookup a skills information
 * High god command
 */ 
void do_slookup( CHAR_DATA * ch, char *argument ) 
{
   
char buf[MAX_STRING_LENGTH];
   
char arg[MAX_INPUT_LENGTH];
   
int sn;
   
int iRace;
   
SKILLTYPE * skill = NULL;
   
one_argument( argument, arg );
   
if( arg[0] == '\0' )
      
   {
      
send_to_char( "Slookup what?\n\r", ch );
      
return;
   
}
   
if( !str_cmp( arg, "all" ) )
      
   {
      
for( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
         
pager_printf( ch, "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r", 
sn, skill_table[sn]->slot,
                        skill_table[sn]->name, 
spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
   
}
   
   else
   
if( !str_cmp( arg, "nostat" ) )
   {
      
for( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
         
if( skill_table[sn]->stat1 == 0 )
            
pager_printf( ch, "Sn: %4d Slot: %4d Skill/spell: '%-20s'\r\n", 
sn, skill_table[sn]->slot,
                           skill_table[sn]->name );
   
}
   
   else
   
if( !str_cmp( arg, "herbs" ) )
      
   {
      
for( sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name; sn++ )
         
pager_printf( ch, "%d) %s\n\r", sn, herb_table[sn]->name );
   
}
   
   else
      
   {
      
SMAUG_AFF * aff;
      
int cnt = 0;
      
if( arg[0] == 'h' && is_number( arg + 1 ) )
         
      {
         
sn = atoi( arg + 1 );
         
if( !IS_VALID_HERB( sn ) )
            
         {
            
send_to_char( "Invalid herb.\n\r", ch );
            
return;
         
}
         
skill = herb_table[sn];
      
}
      
      else
      
if( is_number( arg ) )
         
      {
         
sn = atoi( arg );
         
if( ( skill = get_skilltype( sn ) ) == NULL )
            
         {
            
send_to_char( "Invalid sn.\n\r", ch );
            
return;
         
}
         
sn %= 1000;
      
}
      
      else
      
if( ( sn = skill_lookup( arg ) ) >= 0 )
         
skill = skill_table[sn];
      
      else
      
if( ( sn = herb_lookup( arg ) ) >= 0 )
         
skill = herb_table[sn];
      
      else
         
      {
         
send_to_char( "No such skill, spell, proficiency or tongue.\n\r", ch );
         
return;
      
}
      
if( !skill )
         
      {
         
send_to_char( "Not created yet.\n\r", ch );
         
return;
      
}
      
ch_printf( ch, "Sn: %4d Slot: %4d %s: '%-20s'\n\r", 
sn, skill->slot, skill_tname[skill->type], skill->name );
      
if( skill->info )
         
ch_printf( ch, "DamType: %s  ActType: %s   ClassType: %s   PowerType: %s\n\r",
                     
spell_damage[SPELL_DAMAGE( skill )], 
spell_action[SPELL_ACTION( skill )],
                     
spell_class[SPELL_CLASS( skill )], 
spell_power[SPELL_POWER( skill )] );
      
if( skill->flags )
         
      {
         
int x;
         
strcpy( buf, "Flags:" );
         
for( x = 0; x < 32; x++ )
            
if( SPELL_FLAG( skill, 1 << x ) )
               
            {
               
strcat( buf, " " );
               
strcat( buf, spell_flag[x] );
            
}
         
strcat( buf, "\n\r" );
         
send_to_char( buf, ch );
      
}
      
if( skill->difficulty != '\0' )
         
ch_printf( ch, "Difficulty: %d\n\r", ( int )skill->difficulty );
      
ch_printf( ch, "Type: %s  Target: %s  Minpos: %d  Mana: %d  Beats: %d  Range: %d\n\r", 
skill_tname[skill->type],
                   
target_type[URANGE( TAR_IGNORE, skill->target, TAR_OBJ_INV )], 
skill->minimum_position,
                   
skill->min_mana, skill->beats, skill->range );
      
ch_printf( ch, "Flags: %d  Club: %d  Value: %d  Info: %d  Code: %s\n\r", 
skill->flags, 
skill->guild, 
skill->value,
                  
skill->info, 
skill->skill_fun ? skill_name( skill->skill_fun ) 
 : spell_name( skill->spell_fun ) );
      
ch_printf( ch, "Dammsg: %s\n\rWearoff: %s\n", 
skill->noun_damage, 
skill->msg_off ? skill->msg_off : "(none set)" );
      
if( skill->dice && skill->dice[0] != '\0' )
         
ch_printf( ch, "Dice: %s\n\r", skill->dice );
      
if( skill->teachers && skill->teachers[0] != '\0' )
         
ch_printf( ch, "Teachers: %s\n\r", skill->teachers );
      
if( skill->components && skill->components[0] != '\0' )
         
ch_printf( ch, "Components: %s\n\r", skill->components );
      
if( skill->participants )
         
ch_printf( ch, "Participants: %d\n\r", ( int )skill->participants );
      
if( skill->userec.num_uses )
         
send_timer( &skill->userec, ch );
      
for( aff = skill->affects; aff; aff = aff->next )
         
      {
         
if( aff == skill->affects )
            
send_to_char( "\n\r", ch );
         
sprintf( buf, "Affect %d", ++cnt );
         
if( aff->location )
            
         {
            
strcat( buf, " modifies " );
            
strcat( buf, a_types[aff->location % REVERSE_APPLY] );
            
strcat( buf, " by '" );
            
strcat( buf, aff->modifier );
            
if( aff->bitvector != -1 )
               
strcat( buf, "' and" );
            
            else
               
strcat( buf, "'" );
         
}
         
if( aff->bitvector != -1 )
            
         {
            
strcat( buf, " applies " );
            
strcat( buf, a_flags[aff->bitvector] );
         
}
         
if( aff->duration[0] != '\0' && aff->duration[0] != '0' )
            
         {
            
strcat( buf, " for '" );
            
strcat( buf, aff->duration );
            
strcat( buf, "' rounds" );
         
}
         
if( aff->location >= REVERSE_APPLY )
            
strcat( buf, " (affects caster only)" );
         
strcat( buf, "\n\r" );
         
send_to_char( buf, ch );
         
if( !aff->next )
            
send_to_char( "\n\r", ch );
      
}
      
if( skill->hit_char && skill->hit_char[0] != '\0' )
         
ch_printf( ch, "Hitchar   : %s\n\r", skill->hit_char );
      
if( skill->hit_vict && skill->hit_vict[0] != '\0' )
         
ch_printf( ch, "Hitvict   : %s\n\r", skill->hit_vict );
      
if( skill->hit_room && skill->hit_room[0] != '\0' )
         
ch_printf( ch, "Hitroom   : %s\n\r", skill->hit_room );
      
if( skill->hit_dest && skill->hit_dest[0] != '\0' )
         
ch_printf( ch, "Hitdest   : %s\n\r", skill->hit_dest );
      
if( skill->miss_char && skill->miss_char[0] != '\0' )
         
ch_printf( ch, "Misschar  : %s\n\r", skill->miss_char );
      
if( skill->miss_vict && skill->miss_vict[0] != '\0' )
         
ch_printf( ch, "Missvict  : %s\n\r", skill->miss_vict );
      
if( skill->miss_room && skill->miss_room[0] != '\0' )
         
ch_printf( ch, "Missroom  : %s\n\r", skill->miss_room );
      
if( skill->die_char && skill->die_char[0] != '\0' )
         
ch_printf( ch, "Diechar   : %s\n\r", skill->die_char );
      
if( skill->die_vict && skill->die_vict[0] != '\0' )
         
ch_printf( ch, "Dievict   : %s\n\r", skill->die_vict );
      
if( skill->die_room && skill->die_room[0] != '\0' )
         
ch_printf( ch, "Dieroom   : %s\n\r", skill->die_room );
      
if( skill->imm_char && skill->imm_char[0] != '\0' )
         
ch_printf( ch, "Immchar   : %s\n\r", skill->imm_char );
      
if( skill->imm_vict && skill->imm_vict[0] != '\0' )
         
ch_printf( ch, "Immvict   : %s\n\r", skill->imm_vict );
      
if( skill->imm_room && skill->imm_room[0] != '\0' )
         
ch_printf( ch, "Immroom   : %s\n\r", skill->imm_room );
      
if( skill->type != SKILL_HERB )
         
      {
         
const char *stat_table[] =
            { 
"None", 
"Strength", 
"Intelligence", 
"Willpower", 
"Dexterity", 
"Constitution", 
"Perception", 
"Luck" 
         };
         
send_to_char( "--------------------------[STAT USE]--------------------------\n\r", ch );
         
if( skill->stat1 < 8 )
            
ch_printf( ch, "Stat 1: %s\r\n", stat_table[skill->stat1] );
         
if( skill->stat2 < 8 && skill->stat2 != skill->stat1 )
            
ch_printf( ch, "Stat 2: %s\r\n", stat_table[skill->stat2] );
         
if( skill->type == SKILL_SPELL )
         {
            
DEITY_DATA * talent;
            
int i = 0;
            
send_to_char( "--------------------------[ TALENT ]--------------------------\n\r", ch );
            
for( talent = first_deity; talent; talent = talent->next )
            {
               
sprintf( buf, "(%-3d:%-5s", talent->index, talent->name );
               
sprintf( buf + 9, ") lvl: %3d", 
skill->skill_level[talent->index] );
               
if( i % 3 == 2 )
                  
strcat( buf, "\n\r" );
               
               else
                  
strcat( buf, "  " );
               
send_to_char( buf, ch );
               
i++;
            
}
         
}
         else
         {
            
send_to_char( "\n\r--------------------------[RACE USE]--------------------------\n\r", ch );
            
for( iRace = 0; iRace < MAX_RACE; iRace++ )
            {
               
sprintf( buf, "%-3d): %8.8s", 
iRace, 
                         ( skill->race_level[iRace] ==
                           1 ? "innate" : ( skill->race_level[iRace] == 2 ? "learn" : "no learn" ) ) );
               
if( ( iRace > 0 ) && ( iRace % 3 == 1 ) )
                  
strcat( buf, "\n\r" );
               
               else
                  
strcat( buf, "  " );
               
send_to_char( buf, ch );
            
}
         
}
      
}
      
send_to_char( "\n\r", ch );
   
}
   
return;

}


/*
 * Set a skill's attributes or what skills a player has.
 * High god command, with support for creating skills/spells/herbs/etc
 */ 
void do_sset( CHAR_DATA * ch, char *argument ) 
{
   
char arg1[MAX_INPUT_LENGTH];
   
char arg2[MAX_INPUT_LENGTH];
   
CHAR_DATA * victim;
   
int value;
   
int sn, i;
   
bool fAll;
   
argument = one_argument( argument, arg1 );
   
argument = one_argument( argument, arg2 );
   
if( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
   {
      
send_to_char( "Syntax: sset <victim> <skill> <value>\n\r", ch );
      
send_to_char( "or:     sset <victim> all     <value>\n\r", ch );
      
send_to_char( "or:     sset save skill table\n\r", ch );
      
send_to_char( "or:     sset save herb table\n\r", ch );
      
send_to_char( "or:     sset create skill 'new skill'\n\r", ch );
      
send_to_char( "or:     sset create herb 'new herb'\n\r", ch );
      
send_to_char( "or:     sset create ability 'new ability'\n\r", ch );
      
send_to_char( "or:     sset delete <sn> really\r\n", ch );
      
if( IS_SET( ch->pcdata->permissions, PERMIT_ADMIN ) )
         
send_to_char( "or:     sset copy <from talent#> <to talent#>\n\r", ch );
      
send_to_char( "or:     sset <sn>     <field> <value>\n\r", ch );
      
send_to_char( "\n\rField being one of:\n\r", ch );
      
send_to_char( "  name code target minpos slot mana beats dammsg wearoff club minlevel\n\r", ch );
      
send_to_char( "  type damtype acttype classtype powertype seffect flag dice value difficulty\n\r", ch );
      
send_to_char( "  affect rmaffect level adept hit miss die imm (char/vict/room)\n\r", ch );
      
send_to_char( "  components teachers racelevel raceadept talent\n\r", ch );
      
send_to_char( "Affect having the fields: <location> <modfifier> [duration] [bitvector]\n\r", ch );
      
send_to_char( "(See AFFECTTYPES for location, and AFFECTED_BY for bitvector)\n\r", ch );
      
send_to_char( "Skill being any skill or spell.\n\r", ch );
      
return;
   
}
   
      /*
       * Scion's code! 
       */ 
      if( IS_SET( ch->pcdata->permissions, PERMIT_ADMIN ) 
 &&!str_cmp( arg1, "copy" ) )
   {
      
char arg3[MAX_INPUT_LENGTH];
      
int class1, class2, index;
      
argument = one_argument( argument, arg3 );
      
if( arg3[0] == '\0' || arg2[0] == '\0' )
      {
         
send_to_char( "Syntax: sset copy <from talent#> <to talent#>\r\n", ch );
         
return;
      
}
      
class1 = atoi( arg2 );
      
class2 = atoi( arg3 );
      
if( class1 >= MAX_DEITY || class1 < 0 )
      {
         
send_to_char( "Source class invalid.\r\n", ch );
         
return;
      
}
      
if( class2 >= MAX_DEITY || class2 < 0 )
      {
         
send_to_char( "Destination class invalid.\r\n", ch );
         
return;
      
}
      
for( index = 0; index < top_sn; index++ )
         
skill_table[index]->skill_level[class2] = 
            ( skill_table[index]->skill_level[class1] <= 100 ? 
skill_table[index]->skill_level[class1] : 101 );
      
send_to_char( "Skill table copied.\r\n", ch );
      
save_skill_table(  );
      
return;
   
}
   
if( !str_cmp( arg1, "delete" ) )
   {
      
int i = 0;
      
if( str_cmp( argument, "really" ) )
      {
         
send_to_char( "You must specify 'really' as the last argument to confirm that you want to delete this skill.\r\n",
                        ch );
         
return;
      
}
      
if( is_number( arg2 ) )
         
i = atoi( arg2 );
      
      else
      {
         
send_to_char( "Please specify the sn to delete.\r\n", ch );
         
return;
      
}
      
if( i > top_sn || i < 0 )
      {
         
send_to_char( "That is an invalid sn.\r\n", ch );
         
return;
      
}
      
STRFREE( skill_table[i]->name );
      
skill_table[i]->name = STRALLOC( "deleted_skill" );
      
save_skill_table(  );
      
send_to_char( "Please reboot the MUD as soon as possible to enact the changes to the skill table.\r\n", ch );
      
return;
   
}
   
if( !str_cmp( arg1, "save" ) 
 &&!str_cmp( argument, "table" ) )
      
   {
      
if( !str_cmp( arg2, "skill" ) )
         
      {
         
send_to_char( "Saving skill table...\n\r", ch );
         
save_skill_table(  );
         
return;
      
}
      
if( !str_cmp( arg2, "herb" ) )
         
      {
         
send_to_char( "Saving herb table...\n\r", ch );
         
save_herb_table(  );
         
return;
      
}
   
}
   
if( !str_cmp( arg1, "create" ) 
        &&( !str_cmp( arg2, "skill" ) || !str_cmp( arg2, "herb" ) || !str_cmp( arg2, "ability" ) ) )
      
   {
      
struct skill_type *skill;
      
sh_int type = SKILL_UNKNOWN;
      
if( !str_cmp( arg2, "herb" ) )
         
      {
         
type = SKILL_HERB;
         
if( top_herb >= MAX_HERB )
            
         {
            
ch_printf( ch, "The current top herb is %d, which is the maximum.  " 
                        "To add more herbs,\n\rMAX_HERB will have to be " 
                        "raised in mud.h, and the mud recompiled.\n\r", 
top_sn );
            
return;
         
}
      
}
      
      else
      
if( top_sn >= MAX_SKILL )
         
      {
         
ch_printf( ch, "The current top sn is %d, which is the maximum.  " 
                     "To add more skills,\n\rMAX_SKILL will have to be " 
                     "raised in mud.h, and the mud recompiled.\n\r", 
top_sn );
         
return;
      
}
      
CREATE( skill, struct skill_type, 1 );
      
skill->slot = 0;
      
if( type == SKILL_HERB )
         
      {
         
int max, x;
         
herb_table[top_herb++] = skill;
         
for( max = x = 0; x < top_herb - 1; x++ )
            
if( herb_table[x] && herb_table[x]->slot > max )
               
max = herb_table[x]->slot;
         
skill->slot = max + 1;
      
}
      
      else
         
skill_table[top_sn++] = skill;
      
skill->min_mana = 0;
      
skill->name = str_dup( argument );
      
skill->noun_damage = str_dup( "" );
      
skill->msg_off = str_dup( "" );
      
skill->spell_fun = spell_smaug;
      
skill->type = type;
      
if( !str_cmp( arg2, "ability" ) )
         
skill->type = SKILL_RACIAL;
      
for( i = 0; i < MAX_RACE; i++ )
         
      {
         
skill->race_level[i] = 3;
      
}
      
send_to_char( "Done.\n\r", ch );
      
return;
   
}
   
if( arg1[0] == 'h' )
      
sn = atoi( arg1 + 1 );
   
   else
      
sn = atoi( arg1 );
   
if( ( ( arg1[0] == 'h' && is_number( arg1 + 1 ) && ( sn = atoi( arg1 + 1 ) ) >= 0 ) 
          ||( is_number( arg1 ) && ( sn = atoi( arg1 ) ) >= 0 ) ) )
      
   {
      
struct skill_type *skill;
      
if( arg1[0] == 'h' )
         
      {
         
if( sn >= top_herb )
            
         {
            
send_to_char( "Herb number out of range.\n\r", ch );
            
return;
         
}
         
skill = herb_table[sn];
      
}
      
      else
         
      {
         
if( ( skill = get_skilltype( sn ) ) == NULL )
            
         {
            
send_to_char( "Skill number out of range.\n\r", ch );
            
return;
         
}
         
sn %= 1000;
      
}
      
if( !str_cmp( arg2, "difficulty" ) )
         
      {
         
skill->difficulty = atoi( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "participants" ) )
         
      {
         
skill->participants = atoi( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "damtype" ) )
         
      {
         
int x = get_sdamage( argument );
         
if( x == -1 )
            
send_to_char( "Not a spell damage type.\n\r", ch );
         
         else
            
         {
            
SET_SDAM( skill, x );
            
send_to_char( "Ok.\n\r", ch );
         
}
         
return;
      
}
      
if( !str_cmp( arg2, "acttype" ) )
         
      {
         
int x = get_saction( argument );
         
if( x == -1 )
            
send_to_char( "Not a spell action type.\n\r", ch );
         
         else
            
         {
            
SET_SACT( skill, x );
            
send_to_char( "Ok.\n\r", ch );
         
}
         
return;
      
}
      
if( !str_cmp( arg2, "classtype" ) )
         
      {
         
int x = get_sclass( argument );
         
if( x == -1 )
            
send_to_char( "Not a spell class type.\n\r", ch );
         
         else
            
         {
            
SET_SCLA( skill, x );
            
send_to_char( "Ok.\n\r", ch );
         
}
         
return;
      
}
      
if( !str_cmp( arg2, "powertype" ) )
         
      {
         
int x = get_spower( argument );
         
if( x == -1 )
            
send_to_char( "Not a spell power type.\n\r", ch );
         
         else
            
         {
            
SET_SPOW( skill, x );
            
send_to_char( "Ok.\n\r", ch );
         
}
         
return;
      
}
      
if( !str_cmp( arg2, "flag" ) )
         
      {
         
int x = get_sflag( argument );
         
if( x == -1 )
            
send_to_char( "Not a spell flag.\n\r", ch );
         
         else
            
         {
            
TOGGLE_BIT( skill->flags, 1 << x );
            
send_to_char( "Ok.\n\r", ch );
         
}
         
return;
      
}
      
if( !str_cmp( arg2, "code" ) )
         
      {
         
SPELL_FUN * spellfun;
         
DO_FUN * dofun;
         
if( ( spellfun = spell_function( argument ) ) != spell_notfound )
            
         {
            
skill->spell_fun = spellfun;
            
skill->skill_fun = NULL;
         
}
         
         else
         
if( ( dofun = skill_function( argument ) ) != skill_notfound )
            
         {
            
skill->skill_fun = dofun;
            
skill->spell_fun = NULL;
         
}
         
         else
            
         {
            
send_to_char( "Not a spell or skill.\n\r", ch );
            
return;
         
}
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "target" ) )
         
      {
         
int x = get_starget( argument );
         
if( x == -1 )
            
send_to_char( "Not a valid target type.\n\r", ch );
         
         else
            
         {
            
skill->target = x;
            
send_to_char( "Ok.\n\r", ch );
         
}
         
return;
      
}
      
if( !str_cmp( arg2, "minpos" ) )
         
      {
         
skill->minimum_position = URANGE( POS_DEAD, atoi( argument ), POS_DRAG );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "minlevel" ) )
         
      {
         
skill->min_level = atoi( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "slot" ) )
         
      {
         
skill->slot = URANGE( 0, atoi( argument ), 30000 );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "mana" ) )
         
      {
         
skill->min_mana = URANGE( 0, atoi( argument ), 2000 );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "beats" ) )
         
      {
         
skill->beats = URANGE( 0, atoi( argument ), 120 );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "range" ) )
         
      {
         
skill->range = URANGE( 0, atoi( argument ), 20 );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "club" ) )
         
      {
         
skill->guild = atoi( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "value" ) )
         
      {
         
skill->value = atoi( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "type" ) )
         
      {
         
skill->type = get_skill( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "rmaffect" ) )
         
      {
         
SMAUG_AFF * aff = skill->affects;
         
SMAUG_AFF * aff_next;
         
int num = atoi( argument );
         
int cnt = 1;
         
if( !aff )
            
         {
            
send_to_char( "This spell has no special affects to remove.\n\r", ch );
            
return;
         
}
         
if( num == 1 )
            
         {
            
skill->affects = aff->next;
            
DISPOSE( aff->duration );
            
DISPOSE( aff->modifier );
            
DISPOSE( aff );
            
send_to_char( "Removed.\n\r", ch );
            
return;
         
}
         
for( ; aff; aff = aff->next )
            
         {
            
if( ++cnt == num && ( aff_next = aff->next ) != NULL )
               
            {
               
aff->next = aff_next->next;
               
DISPOSE( aff_next->duration );
               
DISPOSE( aff_next->modifier );
               
DISPOSE( aff_next );
               
send_to_char( "Removed.\n\r", ch );
               
return;
            
}
         
}
         
send_to_char( "Not found.\n\r", ch );
         
return;
      
}
      
         /*
          * 
          * * affect <location> <modifier> <duration> <bitvector>
          */ 
         if( !str_cmp( arg2, "affect" ) )
         
      {
         
char location[MAX_INPUT_LENGTH];
         
char modifier[MAX_INPUT_LENGTH];
         
char duration[MAX_INPUT_LENGTH];
         
/*	    char bitvector[MAX_INPUT_LENGTH];	*/ 
         int loc, bit, tmpbit;
         
SMAUG_AFF * aff;
         
argument = one_argument( argument, location );
         
argument = one_argument( argument, modifier );
         
argument = one_argument( argument, duration );
         
if( location[0] == '!' )
            
loc = get_atype( location + 1 ) + REVERSE_APPLY;
         
         else
            
loc = get_atype( location );
         
if( ( loc % REVERSE_APPLY ) < 0 
 ||( loc % REVERSE_APPLY ) >= MAX_APPLY_TYPE )
            
         {
            
send_to_char( "Unknown affect location.  See AFFECTTYPES.\n\r", ch );
            
return;
         
}
         
bit = -1;
         
if( argument[0] != 0 )
            
         {
            
if( ( tmpbit = get_aflag( argument ) ) == -1 )
               
ch_printf( ch, "Unknown bitvector: %s.  See AFFECTED_BY\n\r", argument );
            
            else
               
bit = tmpbit;
         
}
         
CREATE( aff, SMAUG_AFF, 1 );
         
if( !str_cmp( duration, "0" ) )
            
duration[0] = '\0';
         
if( !str_cmp( modifier, "0" ) )
            
modifier[0] = '\0';
         
aff->duration = str_dup( duration );
         
aff->location = loc;
         
aff->modifier = str_dup( modifier );
         
aff->bitvector = bit;
         
aff->next = skill->affects;
         
skill->affects = aff;
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "level" ) )
         
      {
         
char arg3[MAX_INPUT_LENGTH];
         
int num;
         
argument = one_argument( argument, arg3 );
         
num = atoi( arg3 );
         
if( skill->type == SKILL_SPELL )
         {
            
if( !get_talent_by_index( num ) )
               
send_to_char( "Not a valid talent.\n\r", ch );
            
            else
               
skill->skill_level[num] = atoi( argument );
         
}
         else
         {
            
if( num >= MAX_RACE || num < -1 )
               
send_to_char( "Not a valid race.\n\r", ch );
            
            else if( num == -1 )
            {
               
for( num = 0; num < MAX_RACE; num++ )
                  
skill->race_level[num] = atoi( argument );
            
}
            else
               
skill->race_level[num] = atoi( argument );
         
}
         
return;
      
}
      
if( !str_cmp( arg2, "stat1" ) )
      {
         
char arg3[MAX_INPUT_LENGTH];
         
int stat;
         
argument = one_argument( argument, arg3 );
         
if( !str_prefix( arg3, "strength" ) )
            
stat = 1;
         
         else if( !str_prefix( arg3, "intelligence" ) )
            
stat = 2;
         
         else if( !str_prefix( arg3, "willpower" ) )
            
stat = 3;
         
         else if( !str_prefix( arg3, "dexterity" ) )
            
stat = 4;
         
         else if( !str_prefix( arg3, "constitution" ) )
            
stat = 5;
         
         else if( !str_prefix( arg3, "perception" ) )
            
stat = 6;
         
         else if( !str_prefix( arg3, "luck" ) )
            
stat = 7;
         
         else
            
stat = 0;
         
skill->stat1 = stat;
         
return;
      
}
      
if( !str_cmp( arg2, "stat2" ) )
      {
         
char arg3[MAX_INPUT_LENGTH];
         
int stat;
         
argument = one_argument( argument, arg3 );
         
if( !str_prefix( arg3, "strength" ) )
            
stat = 1;
         
         else if( !str_prefix( arg3, "intelligence" ) )
            
stat = 2;
         
         else if( !str_prefix( arg3, "willpower" ) )
            
stat = 3;
         
         else if( !str_prefix( arg3, "dexterity" ) )
            
stat = 4;
         
         else if( !str_prefix( arg3, "constitution" ) )
            
stat = 5;
         
         else if( !str_prefix( arg3, "perception" ) )
            
stat = 6;
         
         else if( !str_prefix( arg3, "luck" ) )
            
stat = 7;
         
         else
            
stat = 0;
         
skill->stat2 = stat;
         
return;
      
}
      
if( !str_cmp( arg2, "name" ) )
         
      {
         
DISPOSE( skill->name );
         
skill->name = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "dammsg" ) )
         
      {
         
DISPOSE( skill->noun_damage );
         
if( !str_cmp( argument, "clear" ) )
            
skill->noun_damage = str_dup( "" );
         
         else
            
skill->noun_damage = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "wearoff" ) )
         
      {
         
DISPOSE( skill->msg_off );
         
if( str_cmp( argument, "clear" ) )
            
skill->msg_off = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "hitchar" ) )
         
      {
         
if( skill->hit_char )
            
DISPOSE( skill->hit_char );
         
if( str_cmp( argument, "clear" ) )
            
skill->hit_char = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "hitvict" ) )
         
      {
         
if( skill->hit_vict )
            
DISPOSE( skill->hit_vict );
         
if( str_cmp( argument, "clear" ) )
            
skill->hit_vict = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "hitroom" ) )
         
      {
         
if( skill->hit_room )
            
DISPOSE( skill->hit_room );
         
if( str_cmp( argument, "clear" ) )
            
skill->hit_room = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "hitdest" ) )
         
      {
         
if( skill->hit_dest )
            
DISPOSE( skill->hit_dest );
         
if( str_cmp( argument, "clear" ) )
            
skill->hit_dest = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "misschar" ) )
         
      {
         
if( skill->miss_char )
            
DISPOSE( skill->miss_char );
         
if( str_cmp( argument, "clear" ) )
            
skill->miss_char = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "missvict" ) )
         
      {
         
if( skill->miss_vict )
            
DISPOSE( skill->miss_vict );
         
if( str_cmp( argument, "clear" ) )
            
skill->miss_vict = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "missroom" ) )
         
      {
         
if( skill->miss_room )
            
DISPOSE( skill->miss_room );
         
if( str_cmp( argument, "clear" ) )
            
skill->miss_room = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "diechar" ) )
         
      {
         
if( skill->die_char )
            
DISPOSE( skill->die_char );
         
if( str_cmp( argument, "clear" ) )
            
skill->die_char = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "dievict" ) )
         
      {
         
if( skill->die_vict )
            
DISPOSE( skill->die_vict );
         
if( str_cmp( argument, "clear" ) )
            
skill->die_vict = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "dieroom" ) )
         
      {
         
if( skill->die_room )
            
DISPOSE( skill->die_room );
         
if( str_cmp( argument, "clear" ) )
            
skill->die_room = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "immchar" ) )
         
      {
         
if( skill->imm_char )
            
DISPOSE( skill->imm_char );
         
if( str_cmp( argument, "clear" ) )
            
skill->imm_char = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "immvict" ) )
         
      {
         
if( skill->imm_vict )
            
DISPOSE( skill->imm_vict );
         
if( str_cmp( argument, "clear" ) )
            
skill->imm_vict = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "immroom" ) )
         
      {
         
if( skill->imm_room )
            
DISPOSE( skill->imm_room );
         
if( str_cmp( argument, "clear" ) )
            
skill->imm_room = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "dice" ) )
         
      {
         
if( skill->dice )
            
DISPOSE( skill->dice );
         
if( str_cmp( argument, "clear" ) )
            
skill->dice = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "components" ) )
         
      {
         
if( skill->components )
            
DISPOSE( skill->components );
         
if( str_cmp( argument, "clear" ) )
            
skill->components = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
if( !str_cmp( arg2, "teachers" ) )
         
      {
         
if( skill->teachers )
            
DISPOSE( skill->teachers );
         
if( str_cmp( argument, "clear" ) )
            
skill->teachers = str_dup( argument );
         
send_to_char( "Ok.\n\r", ch );
         
return;
      
}
      
do_sset( ch, "" );
      
return;
   
}
   
if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
      
   {
      
if( ( sn = skill_lookup( arg1 ) ) >= 0 )
         
      {
         
sprintf( arg1, "%d %s %s", sn, arg2, argument );
         
do_sset( ch, arg1 );
      
}
      
      else
         
send_to_char( "They aren't here.\n\r", ch );
      
return;
   
}
   
if( IS_NPC( victim ) )
      
   {
      
send_to_char( "Not on NPC's.\n\r", ch );
      
return;
   
}
   
fAll = !str_cmp( arg2, "all" );
   
sn = 0;
   
if( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
      
   {
      
send_to_char( "No such skill or spell.\n\r", ch );
      
return;
   
}
   
      /*
       * 
       * * Snarf the value.
       */ 
      if( !is_number( argument ) )
      
   {
      
send_to_char( "Value must be numeric.\n\r", ch );
      
return;
   
}
   
value = atoi( argument );
   
if( value < 0 || value > 100 )
      
   {
      
send_to_char( "Value range is 0 to 100.\n\r", ch );
      
return;
   
}
   
if( fAll )
      
   {
      
for( sn = 0; sn < top_sn; sn++ )
         
      {
         
            /*
             * Fix by Narn to prevent ssetting skills the player shouldn't have. 
             */ 
            if( skill_table[sn]->name 
 &&( skill_available( victim, sn ) == TRUE 
 ||value == 0 ) )
            
         {
            
victim->pcdata->learned[sn] = value;
         
}
      
}
   
}
   
   else
      
victim->pcdata->learned[sn] = value;
   
return;

}

/* Use cedit to add in as imm command.
 * Syntax is: Invade <area filename> <# of invaders> <vnum of mobs> 
 * example: Invade newacad.are 300 10399 would send 300 mistress tsythia's rampaging 
 * through the academy. This function doesnt make the mobiles aggressive but can be
 * modified to do so easily if you wish this, or you can just edit the mob before
 * hand.
 */
void do_invade( CHAR_DATA *ch , char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AREA_DATA *tarea;
    int count, created;
    bool found=FALSE;
    MOB_INDEX_DATA *pMobIndex;
    ROOM_INDEX_DATA *location;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    count = atoi( arg2 );
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Invade <area> <# of invaders> <mob vnum>\n\r", ch );
	return;
    }
    for ( tarea = first_area; tarea; tarea = tarea->next )
	if ( !str_cmp( tarea->filename, arg1 ) )
	{
	  found = TRUE;
	  break;
	}
    if ( !found )
    {
	send_to_char( "Area not found.\n\r", ch );
	return;
    }
    if ( count > 300)
    {
	send_to_char( "Whoa...Less than 300 please.\n\r", ch );
	return;
    }
    if ( ( pMobIndex = get_mob_index( atoi(arg3) ) ) == NULL )
    {
	send_to_char( "No mobile has that vnum.\n\r", ch );
	return;
    }

    for ( created=0; created < count; created++ )
    {
	if ( (location = get_room_index(number_range(tarea->low_r_vnum, tarea->hi_r_vnum ))) == NULL )
        {
          --created;
	  continue;
        }
	if (IS_SET( location->room_flags, ROOM_SAFE))
        {
          --created;
	  continue;
        }
        victim = create_mobile( pMobIndex );
        char_to_room( victim, location );
        act( AT_IMMORT, "$N appears as part of an invasion force!", ch, NULL, victim, TO_ROOM );
    }
	send_to_char( "The invasion was successful!\n\r", ch );

 return;
}

void learn_from_success( CHAR_DATA * ch, int sn ) 
{
   
int gain, learn, percent, chance, i;
   
if( IS_NPC( ch ) || LEARNED( ch, sn ) <= 0 )
      
return;
   
i = get_best_talent( ch, sn );
   
if( i > -1 && skill_table[sn]->type == SKILL_SPELL )
      
learn_talent( ch, i );
   
if( LEARNED( ch, sn ) < GET_ADEPT( ch, sn ) )
   {
      
chance = LEARNED( ch, sn ) + ( 5 * skill_table[sn]->difficulty );
      
percent = number_percent(  );
      
if( percent >= chance )
         
learn = 2;
      
      else
      
if( chance - percent > 25 )
         
return;
      
      else
         
learn = 1;
      
ch->pcdata->learned[sn] = UMIN( GET_ADEPT( ch, sn ), LEARNED( ch, sn ) + learn );
      
if( LEARNED( ch, sn ) == GET_ADEPT( ch, sn ) )   /* fully learned already, no xp! */
         
return;
      
      else
      {
         
gain = 50 * number_range( 1, 2000 );
      
}
      
gain_exp( ch, gain );
   
}

}

void learn_from_failure( CHAR_DATA * ch, int sn ) 
{
   
int adept, chance;
   
if( IS_NPC( ch ) || LEARNED( ch, sn ) <= 0 )
      
return;
   
chance = LEARNED( ch, sn ) + ( 5 * skill_table[sn]->difficulty );
   
if( chance - number_percent(  ) > 25 )
      
return;
   
adept = GET_ADEPT( ch, sn );
   
if( LEARNED( ch, sn ) < ( adept - 1 ) )
      
ch->pcdata->learned[sn] = UMIN( adept, ch->pcdata->learned[sn] + 1 );

}

void do_detrap( CHAR_DATA * ch, char *argument )
{
   
char arg[MAX_INPUT_LENGTH];
   
OBJ_DATA * obj;
   
OBJ_DATA * trap;
   
int percent;
   
bool found = FALSE;
   
switch ( ch->substate )
      
   {
      
default:
         
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
            
         {
            
send_to_char( "You can't concentrate enough for that.\n\r", ch );
            
return;
         
}
         
argument = one_argument( argument, arg );
         
if( !can_use_skill( ch, 0, gsn_detrap ) )
            
         {
            
send_to_char( "You do not yet know of this skill.\n\r", ch );
            
return;
         
}
         
if( arg[0] == '\0' )
            
         {
            
send_to_char( "Detrap what?\n\r", ch );
            
return;
         
}
         
if( ms_find_obj( ch ) )
            
return;
         
found = FALSE;
         
if( ch->mount )
            
         {
            
send_to_char( "You can't do that while mounted.\n\r", ch );
            
return;
         
}
         
if( !ch->in_room->first_content )
            
         {
            
send_to_char( "You can't find that here.\n\r", ch );
            
return;
         
}
         
for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
            
         {
            
if( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
               
            {
               
found = TRUE;
               
break;
            
}
         
}
         
if( !found )
            
         {
            
send_to_char( "You can't find that here.\n\r", ch );
            
return;
         
}
         
act( AT_ACTION, "You carefully begin your attempt to remove a trap from $p...", ch, obj, NULL, TO_CHAR );
         
act( AT_ACTION, "$n carefully attempts to remove a trap from $p...", ch, obj, NULL, TO_ROOM );
         
ch->alloc_ptr = str_dup( obj->name );
         
add_timer( ch, TIMER_DO_FUN, 3, do_detrap, 1 );
         
return;
      
case 1:
         
if( !ch->alloc_ptr )
            
         {
            
send_to_char( "Your detrapping was interrupted!\n\r", ch );
            
bug( "do_detrap: ch->alloc_ptr NULL!", 0 );
            
return;
         
}
         
strcpy( arg, ch->alloc_ptr );
         
DISPOSE( ch->alloc_ptr );
         
ch->alloc_ptr = NULL;
         
ch->substate = SUB_NONE;
         
break;
      
case SUB_TIMER_DO_ABORT:
         
DISPOSE( ch->alloc_ptr );
         
ch->substate = SUB_NONE;
         
send_to_char( "You carefully stop what you were doing.\n\r", ch );
         
return;
   
}
   
if( !ch->in_room->first_content )
      
   {
      
send_to_char( "You can't find that here.\n\r", ch );
      
return;
   
}
   
for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
      
   {
      
if( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
         
      {
         
found = TRUE;
         
break;
      
}
   
}
   
if( !found )
      
   {
      
send_to_char( "You can't find that here.\n\r", ch );
      
return;
   
}
   
if( ( trap = get_trap( obj ) ) == NULL )
      
   {
      
send_to_char( "You find no trap on that.\n\r", ch );
      
return;
   
}
   
percent = number_percent(  ) - ( get_curr_lck( ch ) + get_curr_dex( ch ) / 4 );
   
separate_obj( obj );
   
if( can_use_skill( ch, percent, gsn_detrap ) )
      
   {
      
send_to_char( "Oops!\n\r", ch );
      
spring_trap( ch, trap );
      
learn_from_failure( ch, gsn_detrap );
      
return;
   
}
   
extract_obj( trap );
   
send_to_char( "You successfully remove a trap.\n\r", ch );
   
learn_from_success( ch, gsn_detrap );
   
return;

}

void do_dig( CHAR_DATA * ch, char *argument ) 
{
   
char arg[MAX_INPUT_LENGTH];
   
OBJ_DATA * obj;
   
OBJ_DATA * startobj;
   
bool found, shovel;
   
EXIT_DATA * pexit;
   
switch ( ch->substate )
      
   {
      
default:
         
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
            
         {
            
send_to_char( "You can't concentrate enough for that.\n\r", ch );
            
return;
         
}
         
if( ch->mount )
            
         {
            
send_to_char( "You can't do that while mounted.\n\r", ch );
            
return;
         
}
         
if( IS_AFFECTED( ch, AFF_FLYING ) )
         {
            
send_to_char( "You are flying high above the ground.\n\r", ch );
            
return;
         
}
         
one_argument( argument, arg );
         
if( arg[0] != '\0' )
            
         {
            
if( ( pexit = find_door( ch, arg, TRUE ) ) == NULL 
 &&get_dir( arg ) == -1 )
               
            {
               
send_to_char( "What direction is that?\n\r", ch );
               
return;
            
}
            
if( pexit )
               
            {
               
if( !IS_SET( pexit->exit_info, EX_DIG ) 
 &&!IS_SET( pexit->exit_info, EX_CLOSED ) )
                  
               {
                  
send_to_char( "There is no need to dig out that exit.\n\r", ch );
                  
return;
               
}
            
}
         
}
         
         else
            
         {
            
switch ( ch->in_room->sector_type )
               
            {
               
case SECT_CITY:
               
case SECT_INSIDE:
                  
send_to_char( "The floor is too hard to dig through.\n\r", ch );
                  
return;
               
case SECT_WATER_SWIM:
               
case SECT_WATER_NOSWIM:
               
case SECT_UNDERWATER:
                  
send_to_char( "You cannot dig here.\n\r", ch );
                  
return;
               
case SECT_AIR:
                  
send_to_char( "What?  In the air?!\n\r", ch );
                  
return;
            
}
         
}
         
add_timer( ch, TIMER_DO_FUN, UMIN( skill_table[gsn_dig]->beats / 10, 3 ), do_dig, 1 );
         
ch->alloc_ptr = str_dup( arg );
         
send_to_char( "You begin digging...\n\r", ch );
         
act( AT_PLAIN, "$n begins digging...", ch, NULL, NULL, TO_ROOM );
         
return;
      
case 1:
         
if( !ch->alloc_ptr )
            
         {
            
send_to_char( "Your digging was interrupted!\n\r", ch );
            
act( AT_PLAIN, "$n's digging was interrupted!", ch, NULL, NULL, TO_ROOM );
            
bug( "do_dig: alloc_ptr NULL", 0 );
            
return;
         
}
         
strcpy( arg, ch->alloc_ptr );
         
DISPOSE( ch->alloc_ptr );
         
break;
      
case SUB_TIMER_DO_ABORT:
         
DISPOSE( ch->alloc_ptr );
         
ch->substate = SUB_NONE;
         
send_to_char( "You stop digging...\n\r", ch );
         
act( AT_PLAIN, "$n stops digging...", ch, NULL, NULL, TO_ROOM );
         
return;
   
}
   
ch->substate = SUB_NONE;
   
      /*
       * not having a shovel makes it harder to succeed 
       */ 
      shovel = FALSE;
   
for( obj = ch->first_carrying; obj; obj = obj->next_content )
      
if( obj->item_type == ITEM_SHOVEL )
         
      {
         
shovel = TRUE;
         
break;
      
}
   
      /*
       * dig out an EX_DIG exit... 
       */ 
      if( arg[0] != '\0' )
      
   {
      
if( ( pexit = find_door( ch, arg, TRUE ) ) != NULL 
           &&IS_SET( pexit->exit_info, EX_DIG ) 
 &&IS_SET( pexit->exit_info, EX_CLOSED ) )
         
      {
         
            /*
             * 4 times harder to dig open a passage without a shovel 
             */ 
            if( can_use_skill( ch, ( number_percent(  ) * ( shovel ? 1 : 4 ) ), 
gsn_dig ) )
            
         {
            
REMOVE_BIT( pexit->exit_info, EX_CLOSED );
            
send_to_char( "You dig open a passageway!\n\r", ch );
            
act( AT_PLAIN, "$n digs open a passageway!", ch, NULL, NULL, TO_ROOM );
            
learn_from_success( ch, gsn_dig );
            
return;
         
}
      
}
      
learn_from_failure( ch, gsn_dig );
      
send_to_char( "Your dig did not discover any exit...\n\r", ch );
      
act( AT_PLAIN, "$n's dig did not discover any exit...", ch, NULL, NULL, TO_ROOM );
      
return;
   
}
   
startobj = ch->in_room->first_content;
   
found = FALSE;
   
for( obj = startobj; obj; obj = obj->next_content )
      
   {
      
         /*
          * twice as hard to find something without a shovel 
          */ 
         if( IS_OBJ_STAT( obj, ITEM_BURIED ) 
             &&( can_use_skill( ch, ( number_percent(  ) * ( shovel ? 1 : 2 ) ), 
gsn_dig ) ) )
         
      {
         
found = TRUE;
         
break;
      
}
   
}
   
learn_noncombat( ch, SK_MINING );
   
if( !found && number_range( 1, 300 ) < ch->in_room->curr_resources 
        +get_curr_per( ch ) + ch->pcdata->noncombat[SK_MINING] 
 &&ch->in_room->curr_resources > 0 )
   {
      
MATERIAL_DATA * material;
      
for( material = first_material; material; material = material->next )
      {
         
if( ( ( !shovel && number_percent(  ) < 50 ) || ( shovel ) ) && ( material->sector == ch->in_room->sector_type )
              && ( number_percent(  ) < material->rarity ) )
         {
            
               /*
                * Can't find magical ore in mana dead areas 
                */ 
               if( xIS_SET( material->extra_flags, ITEM_MAGIC ) 
 &&ch->in_room->area->weather->mana < -100 )
               
continue;
            
obj = make_ore( material->number );
            
if( obj )
            {
               
obj_to_room( obj, ch->in_room );
               
ch->in_room->curr_resources--;
               
xSET_BIT( obj->extra_flags, ITEM_BURIED );
               
found = TRUE;
               
break;
            
}
         
}
      
}
   
}
   
if( !found )
      
   {
      
send_to_char( "Your dig uncovered nothing.\n\r", ch );
      
act( AT_PLAIN, "$n's dig uncovered nothing.", ch, NULL, NULL, TO_ROOM );
      
if( ch->in_room->curr_resources < 10 )
         
send_to_char( "There doesn't seem to be much left here.\r\n", ch );
      
learn_from_failure( ch, gsn_dig );
      
return;
   
}
   
separate_obj( obj );
   
xREMOVE_BIT( obj->extra_flags, ITEM_BURIED );
   
act( AT_SKILL, "Your dig uncovered $p!", ch, obj, NULL, TO_CHAR );
   
act( AT_SKILL, "$n's dig uncovered $p!", ch, obj, NULL, TO_ROOM );
   
learn_from_success( ch, gsn_dig );
   
return;

}

void do_search( CHAR_DATA * ch, char *argument ) 
{
   
char arg[MAX_INPUT_LENGTH];
   
OBJ_DATA * obj;
   
OBJ_DATA * container;
   
OBJ_DATA * startobj;
   
int percent, door;
   
door = -1;
   
switch ( ch->substate )
      
   {
      
default:
         
if( ch->mount )
            
         {
            
send_to_char( "You can't do that while mounted.\n\r", ch );
            
return;
         
}
         
argument = one_argument( argument, arg );
         
if( arg[0] != '\0' && ( door = get_door( arg ) ) == -1 )
            
         {
            
container = get_obj_here( ch, arg );
            
if( !container )
               
            {
               
send_to_char( "You can't find that here.\n\r", ch );
               
return;
            
}
            
if( container->item_type != ITEM_CONTAINER )
               
            {
               
send_to_char( "You can't search in that!\n\r", ch );
               
return;
            
}
            
if( IS_SET( container->value[1], CONT_CLOSED ) )
               
            {
               
send_to_char( "It is closed.\n\r", ch );
               
return;
            
}
         
}
         
add_timer( ch, TIMER_DO_FUN, UMIN( skill_table[gsn_search]->beats / 10, 3 ), 
do_search, 1 );
         
send_to_char( "You begin your search...\n\r", ch );
         
ch->alloc_ptr = str_dup( arg );
         
return;
      
case 1:
         
if( !ch->alloc_ptr )
            
         {
            
send_to_char( "Your search was interrupted!\n\r", ch );
            
bug( "do_search: alloc_ptr NULL", 0 );
            
return;
         
}
         
strcpy( arg, ch->alloc_ptr );
         
DISPOSE( ch->alloc_ptr );
         
break;
      
case SUB_TIMER_DO_ABORT:
         
DISPOSE( ch->alloc_ptr );
         
ch->substate = SUB_NONE;
         
send_to_char( "You stop your search...\n\r", ch );
         
return;
   
}
   
ch->substate = SUB_NONE;
   
if( arg[0] == '\0' )
      
startobj = ch->in_room->first_content;
   
   else
      
   {
      
if( ( door = get_door( arg ) ) != -1 )
         
startobj = NULL;
      
      else
         
      {
         
container = get_obj_here( ch, arg );
         
if( !container )
            
         {
            
send_to_char( "You can't find that here.\n\r", ch );
            
return;
         
}
         
startobj = container->first_content;
      
}
   
}
   
percent = number_percent(  ) + number_percent(  ) - ( get_curr_per( ch ) / 10 );
   
if( door != -1 )
      
   {
      
EXIT_DATA * pexit;
      
if( ( pexit = get_exit( ch->in_room, door ) ) != NULL 
            &&IS_SET( pexit->exit_info, EX_SECRET ) 
            &&IS_SET( pexit->exit_info, EX_xSEARCHABLE ) 
 &&can_use_skill( ch, percent, gsn_search ) )
         
      {
         
act( AT_SKILL, "Your search reveals the $d!", ch, NULL, pexit->keyword, TO_CHAR );
         
act( AT_SKILL, "$n finds the $d!", ch, NULL, pexit->keyword, TO_ROOM );
         
REMOVE_BIT( pexit->exit_info, EX_SECRET );
         
learn_from_success( ch, gsn_search );
         
return;
      
}
   
}
   
   else
      
for( obj = startobj; obj; obj = obj->next_content )
         
      {
         
if( IS_OBJ_STAT( obj, ITEM_HIDDEN ) 
 &&can_use_skill( ch, percent, gsn_search ) )
            
         {
            
separate_obj( obj );
            
xREMOVE_BIT( obj->extra_flags, ITEM_HIDDEN );
            
act( AT_SKILL, "Your search reveals $p!", ch, obj, NULL, TO_CHAR );
            
act( AT_SKILL, "$n finds $p!", ch, obj, NULL, TO_ROOM );
            
learn_from_success( ch, gsn_search );
            
return;
         
}
      
}
   
learn_noncombat( ch, SK_NATURE );
   
if( number_range( 1, 300 ) < ch->in_room->curr_vegetation 
        +get_curr_per( ch ) + ch->pcdata->noncombat[SK_NATURE] 
 &&ch->in_room->curr_vegetation > 0 )
   {
      
INGRED_DATA * ingred;
      
for( ingred = first_ingred; ingred; ingred = ingred->next )
      {
         
if( ingred->sector != ch->in_room->sector_type )
            continue;
         
if( ingred->moisture > ch->in_room->curr_water )
            continue;
         
if( ingred->elevation > ch->in_room->curr_elevation )
            continue;
         
if( ingred->precip > 0 
 &&ingred->precip < ch->in_room->area->weather->precip )
            continue;
         
if( ingred->precip < 0 
 &&ingred->precip > ch->in_room->area->weather->precip )
            continue;
         
if( ingred->temp > 0 
 &&ingred->temp < ch->in_room->area->weather->temp )
            continue;
         
if( ingred->temp < 0 
 &&ingred->temp > ch->in_room->area->weather->temp )
            continue;
         
if( ingred->mana > 0 
 &&ingred->mana < ch->in_room->area->weather->mana )
            continue;
         
if( number_percent(  ) > ingred->rarity )
            continue;
         
if( !get_obj_index( ingred->vnum ) )
         {
            
bug( "Do_search: can't find vnum for ingredient %d!", 
ingred->vnum );
            
send_to_char( "You thought you saw something, but it disappeared...\n\r", ch );
            
return;
         
}
         
obj = create_object( get_obj_index( ingred->vnum ), 0 );
         
if( obj )
         {
            
obj_to_room( obj, ch->in_room );
            
ch->in_room->curr_vegetation--;
            
act( AT_SKILL, "You have found $p!", ch, obj, NULL, TO_CHAR );
            
act( AT_SKILL, "$n has found $p!", ch, obj, NULL, TO_ROOM );
            
return;
         
}
      
}
   
}
   
send_to_char( "You find nothing.\n\r", ch );
   
learn_from_failure( ch, gsn_search );
   
return;

}

void do_steal( CHAR_DATA * ch, char *argument )
{
   
char arg1[MAX_INPUT_LENGTH];
   
char arg2[MAX_INPUT_LENGTH];
   
CHAR_DATA * victim, *mst;
   
OBJ_DATA * obj;
   
int gp;
   
argument = one_argument( argument, arg1 );
   
argument = one_argument( argument, arg2 );
   
if( ch->mount )
      
   {
      
send_to_char( "You can't do that while mounted.\n\r", ch );
      
return;
   
}
   
if( arg1[0] == '\0' || arg2[0] == '\0' )
      
   {
      
send_to_char( "Steal what from whom?\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( ( victim = get_char_room( ch, arg2 ) ) == NULL )
      
   {
      
send_to_char( "They aren't here.\n\r", ch );
      
return;
   
}
   
if( victim == ch )
      
   {
      
send_to_char( "That's pointless.\n\r", ch );
      
return;
   
}
   
if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
      
   {
      
set_char_color( AT_MAGIC, ch );
      
send_to_char( "A magical force interrupts you.\n\r", ch );
      
return;
   
}
   
if( check_illegal_psteal( ch, victim ) )
      
   {
      
send_to_char( "You can't steal from that player.\n\r", ch );
      
return;
   
}
   
if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )  /* Gorog */
      
   {
      
send_to_char( "They are a pacifist - Shame on you!\n\r", ch );
      
return;
   
}
   
if( !IS_SAME_PLANE( ch, victim ) )
   {
      
send_to_char( "Your hands pass right through them!\n\r", ch );
      
return;
   
}
   
learn_noncombat( ch, SK_STEALTH );
   
learn_noncombat( victim, SK_STREET );
   
if( !IS_NPC( ch ) && 
number_range( 1, ch->pcdata->noncombat[SK_STEALTH] * 2 
                                         +get_curr_dex( ch ) ) < ( IS_NPC( victim ) ? 50 : 
number_range( 1,
                                                                                                          victim->pcdata->
                                                                                                          noncombat
                                                                                                          [SK_STREET] *
                                                                                                          2 
                                                                                                          +get_curr_per
                                                                                                          ( victim ) ) ) )
   {
      
         /*
          * 
          * * Failure.
          */ 
         send_to_char( "Oops...\n\r", ch );
      
act( AT_ACTION, "$n tries to steal from you!", ch, NULL, victim, TO_VICT );
      
act( AT_ACTION, "$n tries to steal from $N.", ch, NULL, victim, TO_NOTVICT );
      
learn_from_failure( ch, gsn_steal );
      
if( IS_NPC( ch ) )
         
      {
         
if( ( mst = ch->master ) == NULL )
            
return;
      
}
      
return;
   
}
   
if( !str_cmp( arg1, "coin" ) 
 ||!str_cmp( arg1, "coins" ) 
 ||!str_cmp( arg1, "gold" ) 
 ||!str_cmp( arg1, "money" ) )
      
   {
      
int amount;
      
amount = ( int )( victim->gold * number_range( 1, 10 ) / 100 );
      
if( amount <= 0 )
         
      {
         
send_to_char( "You couldn't get any coins.\n\r", ch );
         
learn_from_failure( ch, gsn_steal );
         
return;
      
}
      
ch->gold += amount;
      
victim->gold -= amount;
      
gp = amount;
      
ch_printf( ch, "You steal &w%dmithril, ", ( int )gp / 1000000 );
      
gp = gp % 1000000;
      
ch_printf( ch, "&Y%dgold, ", ( int )gp / 10000 );
      
gp = gp % 10000;
      
ch_printf( ch, "&W%dsilver, ", ( int )gp / 100 );
      
gp = gp % 100;
      
ch_printf( ch, "and &O%dcopper&w coins.\n\r", gp );
      
learn_from_success( ch, gsn_steal );
      
return;
   
}
   
if( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
      
   {
      
send_to_char( "You can't seem to find it.\n\r", ch );
      
learn_from_failure( ch, gsn_steal );
      
return;
   
}
   
if( !can_drop_obj( ch, obj ) 
         ||IS_OBJ_STAT( obj, ITEM_INVENTORY ) 
         ||IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) 
 ||obj == victim->main_hand 
 ||obj == victim->off_hand )
      
   {
      
send_to_char( "You can't manage to pry it away.\n\r", ch );
      
learn_from_failure( ch, gsn_steal );
      
return;
   
}
   
if( ch->carry_number + ( get_obj_number( obj ) / obj->count ) > 
can_carry_n( ch ) )
      
   {
      
send_to_char( "You have your hands full.\n\r", ch );
      
learn_from_failure( ch, gsn_steal );
      
return;
   
}
   
if( ch->carry_weight + ( get_obj_weight( obj ) / obj->count ) > 
can_carry_w( ch ) )
      
   {
      
send_to_char( "You can't carry that much weight.\n\r", ch );
      
learn_from_failure( ch, gsn_steal );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) 
 &&IS_AFFECTED( ch, AFF_HAS_ARTI ) )
   {
      
act( AT_MAGIC, "$p evades your grasp.", ch, obj, NULL, TO_CHAR );
      
return;
   
}
   
separate_obj( obj );
   
obj_from_char( obj );
   
obj_to_char( obj, ch );
   
send_to_char( "Got it!\n\r", ch );
   
learn_from_success( ch, gsn_steal );
   
return;

}

void do_feed( CHAR_DATA * ch, char *argument ) 
{
   
CHAR_DATA * victim;
   
if( !IS_VAMPIRE( ch ) )
   {
      
huh( ch );
      
return;
   
}
   
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      
send_to_char( "You can't concentrate enough for that.\n\r", ch );
      
return;
   
}
   
if( ch->mount )
      
   {
      
send_to_char( "You can't do that while mounted.\n\r", ch );
      
return;
   
}
   
if( ( victim = get_char_room( ch, argument ) ) == NULL )
      
   {
      
send_to_char( "They aren't here.\n\r", ch );
      
global_retcode = rNONE;
      
return;
   
}
   
if( !IS_SAME_PLANE( ch, victim ) )
   {
      
send_to_char( "You pass right through them!\n\r", ch );
      
return;
   
}
   
if( IS_NPC( victim ) )
   {
      
send_to_char( "They are not worthy of being embraced.\n\r", ch );
      
return;
   
}
   
if( ( victim->hit ) > ( victim->max_hit / 3 ) )
      
   {
      
send_to_char( "They are not low enough on blood to feed.\n\r", ch );
      
return;
   
}
   
if( IS_AFFECTED( ch, AFF_COLDBLOOD ) )
   {
      
send_to_char( "Their blood is incompatible.\n\r", ch );
      
return;
   
}
   
if( !IS_CONSENTING( victim, ch ) )
   {
      
send_to_char( "You do not have their consent to do that.\n\r", ch );
      
return;
   
}
   
victim->hit += ( int )ch->hit / 2;
   
ch->hit -= ( int )ch->hit / 2;
   
act( AT_BLOOD, "You slash open your wrist and feed blood to $N.", 
ch, NULL, victim, TO_CHAR );
   
act( AT_BLOOD, "$n slashes open $s wrist and feeds you blood.", 
ch, NULL, victim, TO_VICT );
   
act( AT_BLOOD, "$n slashes open $s wrist and feeds blood to $N.", 
ch, NULL, victim, TO_NOTVICT );
   
if( !IS_SET( victim->pcdata->flags, PCFLAG_VAMPIRE ) )
   {
      
SET_BIT( victim->pcdata->flags, PCFLAG_VAMPIRE );
      
victim->pcdata->condition[COND_FULL] = -1;
      
victim->pcdata->condition[COND_THIRST] = -1;
      
STRFREE( victim->pcdata->parent );
      
victim->pcdata->parent = STRALLOC( ch->name );
      
if( ch->pcdata->family )
      {
         
STRFREE( victim->pcdata->family );
         
victim->pcdata->family = STRALLOC( ch->pcdata->family );
      
}
      
victim->pcdata->gen = ch->pcdata->gen + 1;
      
STRFREE( victim->pcdata->type );
      
victim->pcdata->type = STRALLOC( "vampire" );
   
}
   
return;

}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 * Check for loyalty flag (weapon disarms to inventory) for pkillers -Blodkai
 */ 
void disarm( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
OBJ_DATA * obj, *tmpobj;
   
if( ( obj = victim->main_hand ) == NULL )
      
return;
   
if( ( tmpobj = victim->off_hand ) != NULL 
 &&number_bits( 1 ) == 0 )
      
obj = tmpobj;
   
if( IS_NPC( ch ) && !can_see_obj( ch, obj ) && number_bits( 1 ) == 0 )
      
   {
      
learn_from_failure( ch, gsn_disarm );
      
return;
   
}
   
if( check_grip( ch, victim ) )
   {
      
learn_from_failure( ch, gsn_disarm );
      
return;
   
}
   
act( AT_SKILL, "$n DISARMS you!", ch, NULL, victim, TO_VICT );
   
act( AT_SKILL, "You disarm $N!", ch, NULL, victim, TO_CHAR );
   
act( AT_SKILL, "$n disarms $N!", ch, NULL, victim, TO_NOTVICT );
   
learn_from_success( ch, gsn_disarm );
   
obj_from_char( obj );
   
if( IS_OBJ_STAT( obj, ITEM_LOYAL ) )
   {
      
act( AT_MAGIC, "Your weapon returns to your hand!", ch, NULL, victim, TO_VICT );
      
act( AT_MAGIC, "$N\'s weapon returns to $s hand!", ch, NULL, victim, TO_CHAR );
      
act( AT_MAGIC, "$N\'s weapon returns to $s hand!", ch, NULL, victim, TO_NOTVICT );
      
obj_to_char( obj, victim );
   
}
   else
      
obj_to_room( obj, victim->in_room );
   
return;

}

void do_disarm( CHAR_DATA * ch, char *argument ) 
{
   
CHAR_DATA * victim;
   
OBJ_DATA * obj;
   
int percent;
   
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      
send_to_char( "You can't concentrate enough for that.\n\r", ch );
      
return;
   
}
   
if( !skill_available( ch, gsn_disarm ) )
      
   {
      
send_to_char( "You don't know how to disarm opponents.\n\r", ch );
      
return;
   
}
   
if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      
send_to_char( "Disarm who?\n\r", ch );
      
return;
   
}
   
if( ( obj = victim->main_hand ) == NULL 
 &&( obj = victim->off_hand ) == NULL )
   {
      
send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
      
return;
   
}
   
percent = number_percent(  ) - ( get_curr_dex( ch ) / 10 ) + ( get_curr_dex( victim ) / 10 ) 
      +victim->weight + get_curr_str( victim ) - ch->weight - get_curr_dex( ch );
   
if( !can_see_obj( ch, obj ) )
      
percent += 10;
   
if( can_use_skill( ch, ( percent * 3 / 2 ), gsn_disarm ) )
   {
      
if( IS_OBJ_STAT( obj, ITEM_LOYAL ) )
      {
         
act( AT_SKILL, "$n DISARMS you, but $p returns to your hand!", ch, obj, victim, TO_VICT );
         
act( AT_SKILL, "You disarm $N, but $p returns to $s hand!", ch, obj, victim, TO_CHAR );
         
act( AT_SKILL, "$n disarms $N, but $p returns to $s hand!", ch, obj, victim, TO_NOTVICT );
         
learn_from_success( ch, gsn_disarm );
         
return;
      
}
      
disarm( ch, victim );
   
}
   else
      
   {
      
send_to_char( "You failed.\n\r", ch );
      
learn_from_failure( ch, gsn_disarm );
   
}
   
return;

}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */ 
void trip( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
if( IS_AFFECTED( victim, AFF_FLYING ) 
 ||IS_AFFECTED( victim, AFF_FLOATING ) )
      
return;
   
if( victim->mount )
      
   {
      
if( IS_AFFECTED( victim->mount, AFF_FLYING ) 
 ||IS_AFFECTED( victim->mount, AFF_FLOATING ) )
         
return;
      
act( AT_SKILL, "$n trips your mount and you fall off!", ch, NULL, victim, TO_VICT );
      
act( AT_SKILL, "You trip $N's mount and $N falls off!", ch, NULL, victim, TO_CHAR );
      
act( AT_SKILL, "$n trips $N's mount and $N falls off!", ch, NULL, victim, TO_NOTVICT );
      
if( IS_NPC( victim->mount ) )
         
xREMOVE_BIT( victim->mount->affected_by, AFF_MOUNTED );
      
      else
         
xREMOVE_BIT( victim->mount->pcdata->perm_aff, AFF_MOUNTED );
      
victim->mount = NULL;
      
WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      
WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
      
victim->position = POS_RESTING;
      
return;
   
}
   
if( victim->wait == 0 )
      
   {
      
act( AT_SKILL, "$n trips you and you go down!", ch, NULL, victim, TO_VICT );
      
act( AT_SKILL, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR );
      
act( AT_SKILL, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );
      
WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
      
WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
      
victim->position = POS_RESTING;
   
}
   
return;

}


/* locksmith <door> <lock> <key> */ 
void do_locksmith( CHAR_DATA * ch, char *argument )
{
   
char arg[MAX_INPUT_LENGTH];
   
char buf[MAX_INPUT_LENGTH];
   
char door[MAX_INPUT_LENGTH];
   
OBJ_DATA * key;
   
OBJ_DATA * lock;
   
EXIT_DATA * pexit;
   
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
   {
      
send_to_char( "You can't concentrate enough for that.\r\n", ch );
      
return;
   
}
   
argument = one_argument( argument, door );
   
argument = one_argument( argument, arg );
   
argument = one_argument( argument, buf );
   
if( door[0] == '\0' )
   {
      
send_to_char( "Syntax: locksmith <door> <lock> <key>\r\n", ch );
      
return;
   
}
   
if( arg[0] == '\0' )
   {
      
send_to_char( "What lock would you like to use?\r\n", ch );
      
return;
   
}
   
if( buf[0] == '\0' )
   {
      
send_to_char( "What would you like to use as a key to your lock?\r\n", ch );
      
return;
   
}
   
if( ( key = get_obj_carry( ch, buf ) ) == NULL )
   {
      
send_to_char( "You don't seem to have that key.\r\n", ch );
      
return;
   
}
   
if( ( lock = get_obj_carry( ch, arg ) ) == NULL || key == lock )
   {
      
send_to_char( "You don't have a lock by that name.\r\n", ch );
      
return;
   
}
   
if( key->item_type != ITEM_KEY )
   {
      
send_to_char( "That is not a key.\r\n", ch );
      
return;
   
}
   
if( lock->item_type != ITEM_LOCK )
   {
      
send_to_char( "That is not a lock.\r\n", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( ch->mount )
   {
      
send_to_char( "You can't do that while mounted.\n\r", ch );
      
return;
   
}
   
if( ( pexit = find_door( ch, door, TRUE ) ) != NULL )
   {
      
EXIT_DATA * pexit_rev;
      
if( !IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
         
send_to_char( "It's not closed.\n\r", ch );
         
return;
      
}
      
if( IS_SET( pexit->exit_info, EX_BASHED ) )
      {
         
send_to_char( "That door has been bashed off its hinges!\r\n", ch );
         
return;
      
}
      
if( pexit->key >= 0 )
      {
         
send_to_char( "It already has a lock.\r\n", ch );
         
return;
      
}
      
if( IS_SET( pexit->exit_info, EX_LOCKED ) )
      {
         
send_to_char( "It's already locked.\n\r", ch );
         
return;
      
}
      
if( !can_use_skill( ch, number_percent(  ), gsn_locksmith ) )
      {
         
send_to_char( "You fail to install the lock.\n\r", ch );
         
learn_from_failure( ch, gsn_locksmith );
         
separate_obj( lock );
         
obj_from_char( lock );
         
extract_obj( lock );
         
return;
      
}
      
SET_BIT( pexit->exit_info, EX_LOCKED );
      
pexit->key = number_range( 1, 9999 );
      
key->value[0] = pexit->key;
      
send_to_char( "You install a sturdy lock.\n\r", ch );
      
act( AT_SKILL, "$n installs a lock in the $d.", ch, NULL, pexit->keyword, TO_ROOM );
      
separate_obj( lock );
      
obj_from_char( lock );
      
extract_obj( lock );
      
learn_from_success( ch, gsn_locksmith );
      
         /*
          * lock the other side 
          */ 
         if( ( pexit_rev = pexit->rexit ) != NULL 
 &&pexit_rev->to_room == ch->in_room )
      {
         
SET_BIT( pexit_rev->exit_info, EX_LOCKED );
         
pexit_rev->key = pexit->key;
      
}
      
fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      
if( pexit->to_room->area != ch->in_room->area )
         
fold_area( pexit->to_room->area, pexit->to_room->area->filename, FALSE );
      
WAIT_STATE( ch, skill_table[gsn_locksmith]->beats );
      
return;
   
}
   
ch_printf( ch, "You see no %s here.\n\r", arg );

}

void do_pick( CHAR_DATA * ch, char *argument ) 
{
   
char arg[MAX_INPUT_LENGTH];
   
CHAR_DATA * gch;
   
OBJ_DATA * obj;
   
EXIT_DATA * pexit;
   
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      
send_to_char( "You can't concentrate enough for that.\n\r", ch );
      
return;
   
}
   
one_argument( argument, arg );
   
if( arg[0] == '\0' )
      
   {
      
send_to_char( "Pick what?\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( ch->mount )
      
   {
      
send_to_char( "You can't do that while mounted.\n\r", ch );
      
return;
   
}
   
      /*
       * look for guards 
       */ 
      for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
      
   {
      
if( IS_AWAKE( gch ) && IS_AFFECTED( gch, AFF_GUARD ) )
         
      {
         
act( AT_PLAIN, "$N is standing too close to the lock.", 
ch, NULL, gch, TO_CHAR );
         
return;
      
}
   
}
   
if( !can_use_skill( ch, number_percent(  ), gsn_pick_lock ) )
      
   {
      
send_to_char( "You fail to pick the lock.\n\r", ch );
      
learn_from_failure( ch, gsn_pick_lock );
      
return;
   
}
   
if( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
      
   {
      
         /*
          * 'pick door' 
          */ 
         EXIT_DATA * pexit_rev;
      
if( !IS_SET( pexit->exit_info, EX_CLOSED ) )
         
      {
         send_to_char( "It's not closed.\n\r", ch );
         return;
      }
      
if( !IS_SET( pexit->exit_info, EX_LOCKED ) )
         
      {
         send_to_char( "It's already unlocked.\n\r", ch );
         return;
      }
      
if( IS_SET( pexit->exit_info, EX_PICKPROOF ) )
         
      {
         
send_to_char( "You fail to pick the lock.\n\r", ch );
         
learn_from_failure( ch, gsn_pick_lock );
         
check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
         
return;
      
}
      
REMOVE_BIT( pexit->exit_info, EX_LOCKED );
      
send_to_char( "You spring the lock.\n\r", ch );
      
act( AT_ACTION, "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
      
learn_from_success( ch, gsn_pick_lock );
      
         /*
          * pick the other side 
          */ 
         if( ( pexit_rev = pexit->rexit ) != NULL 
 &&pexit_rev->to_room == ch->in_room )
         
      {
         
REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
      
}
      
check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
      
return;
   
}
   
if( ( obj = get_obj_here( ch, arg ) ) != NULL )
      
   {
      
         /*
          * 'pick object' 
          */ 
         if( obj->item_type != ITEM_CONTAINER )
         
      {
         send_to_char( "That's not a container.\n\r", ch );
         return;
      }
      
if( !IS_SET( obj->value[1], CONT_CLOSED ) )
         
      {
         send_to_char( "It's not closed.\n\r", ch );
         return;
      }
      
if( !IS_SET( obj->value[1], CONT_LOCKED ) )
         
      {
         send_to_char( "It's already unlocked.\n\r", ch );
         return;
      }
      
if( IS_SET( obj->value[1], CONT_PICKPROOF ) )
         
      {
         
send_to_char( "You failed.\n\r", ch );
         
learn_from_failure( ch, gsn_pick_lock );
         
check_for_trap( ch, obj, TRAP_PICK );
         
return;
      
}
      
separate_obj( obj );
      
REMOVE_BIT( obj->value[1], CONT_LOCKED );
      
send_to_char( "*Click*\n\r", ch );
      
act( AT_ACTION, "$n picks $p.", ch, obj, NULL, TO_ROOM );
      
learn_from_success( ch, gsn_pick_lock );
      
return;
   
}
   
ch_printf( ch, "You see no %s here.\n\r", arg );
   
return;

}

void do_sneak( CHAR_DATA * ch, char *argument )
{
   
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      
send_to_char( "You can't concentrate enough for that.\n\r", ch );
      
return;
   
}
   
if( ch->mount )
      
   {
      
send_to_char( "You can't do that while mounted.\n\r", ch );
      
return;
   
}
   
if( !IS_NPC( ch ) )
   {
      
send_to_char( "You attempt to move silently.\n\r", ch );
      
xSET_BIT( ch->pcdata->perm_aff, AFF_SNEAK );
   
}
   
return;

}

void do_hide( CHAR_DATA * ch, char *argument ) 
{
   
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      
send_to_char( "You can't concentrate enough for that.\n\r", ch );
      
return;
   
}
   
if( ch->mount )
      
   {
      
send_to_char( "You can't do that while mounted.\n\r", ch );
      
return;
   
}
   
send_to_char( "You attempt to hide.\n\r", ch );
   
if( can_use_skill( ch, number_percent(  ), gsn_hide ) )
      
   {
      
if( IS_NPC( ch ) )
         
xSET_BIT( ch->affected_by, AFF_HIDE );
      
      else
         
xSET_BIT( ch->pcdata->perm_aff, AFF_HIDE );
      
learn_from_success( ch, gsn_hide );
   
}
   
   else
      
learn_from_failure( ch, gsn_hide );
   
return;

}

void do_emerge( CHAR_DATA * ch, char *argument )
{
   
if( !IS_NPC( ch ) )
   {
      
xREMOVE_BIT( ch->pcdata->perm_aff, AFF_SNEAK );
      
xREMOVE_BIT( ch->pcdata->perm_aff, AFF_HIDE );
   
}
   
xREMOVE_BIT( ch->affected_by, AFF_SNEAK );
   
xREMOVE_BIT( ch->affected_by, AFF_HIDE );
   
act( AT_DGREY, "You emerge from hiding.\n\r", ch, NULL, NULL, TO_CHAR );
   
act( AT_DGREY, "$n emerges from the shadows.\n\r", ch, NULL, NULL, TO_ROOM );

}

void do_aid( CHAR_DATA * ch, char *argument ) 
{
   
char arg[MAX_INPUT_LENGTH];
   
CHAR_DATA * victim;
   
int percent;
   
if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
      
   {
      
send_to_char( "You can't concentrate enough for that.\n\r", ch );
      
return;
   
}
   
one_argument( argument, arg );
   
if( arg[0] == '\0' )
      
   {
      
send_to_char( "Aid whom?\n\r", ch );
      
return;
   
}
   
if( ( victim = get_char_room( ch, arg ) ) == NULL )
      
   {
      
send_to_char( "They aren't here.\n\r", ch );
      
return;
   
}
   
if( IS_NPC( victim ) )   /* Gorog */
      
   {
      
send_to_char( "Not on mobs.\n\r", ch );
      
return;
   
}
   
if( !can_use_bodypart( ch, BP_RHAND ) 
 &&!can_use_bodypart( ch, BP_LHAND ) )
   {
      
send_to_char( "That would be hard without functional hands.\n\r", ch );
      
return;
   
}
   
if( ch->mount )
      
   {
      
send_to_char( "You can't do that while mounted.\n\r", ch );
      
return;
   
}
   
if( victim == ch )
      
   {
      
send_to_char( "Aid yourself?\n\r", ch );
      
return;
   
}
   
if( !IS_SAME_PLANE( victim, ch ) )
   {
      
send_to_char( "Your hands pass right through them!\n\r", ch );
      
return;
   
}
   
if( victim->position > POS_STUNNED )
      
   {
      
act( AT_PLAIN, "$N doesn't need your help.", ch, NULL, victim, 
TO_CHAR );
      
return;
   
}
   
percent = number_percent(  ) - ( get_curr_lck( ch ) / 5 );
   
if( !can_use_skill( ch, percent, gsn_aid ) )
      
   {
      
send_to_char( "You fail.\n\r", ch );
      
learn_from_failure( ch, gsn_aid );
      
return;
   
}
   
act( AT_SKILL, "You aid $N!", ch, NULL, victim, TO_CHAR );
   
act( AT_SKILL, "$n aids $N!", ch, NULL, victim, TO_NOTVICT );
   
learn_from_success( ch, gsn_aid );
   
if( victim->hit < 10 )
      
victim->hit = 10;
   
if( victim->move < 10 )
      
victim->move = 10;
   
update_pos( victim );
   
act( AT_SKILL, "$n aids you!", ch, NULL, victim, TO_VICT );
   
return;

}

void do_mount( CHAR_DATA * ch, char *argument ) 
{
   
CHAR_DATA * victim;
   
if( !skill_available( ch, gsn_mount ) )
      
   {
      
send_to_char( 
"I don't think that would be a good idea...\n\r", ch );
      
return;
   
}
   
if( ch->mount )
      
   {
      
send_to_char( "You're already mounted!\n\r", ch );
      
return;
   
}
   
if( ( victim = get_char_room( ch, argument ) ) == NULL )
      
   {
      
send_to_char( "You can't find that here.\n\r", ch );
      
return;
   
}
   
if( !IS_AFFECTED( victim, AFF_MOUNTABLE ) )
      
   {
      
send_to_char( "You can't mount that!\n\r", ch );
      
return;
   
}
   
if( IS_AFFECTED( victim, AFF_MOUNTED ) )
      
   {
      
send_to_char( "That mount already has a rider.\n\r", ch );
      
return;
   
}
   
if( !IS_NPC( victim ) && !IS_CONSENTING( victim, ch ) )
   {
      
send_to_char( "Not without their consent.\n\r", ch );
      
return;
   
}
   
if( IS_FIGHTING( victim ) )
      
   {
      
send_to_char( "Your mount is moving around too much.\n\r", ch );
      
return;
   
}
   
if( !IS_SAME_PLANE( ch, victim ) )
   {
      
send_to_char( "You pass right through them!\n\r", ch );
      
return;
   
}
   
if( can_use_skill( ch, number_percent(  ), gsn_mount ) )
      
   {
      
if( IS_NPC( victim ) )
         
xSET_BIT( victim->affected_by, AFF_MOUNTED );
      
      else
         
xSET_BIT( victim->pcdata->perm_aff, AFF_MOUNTED );
      
ch->mount = victim;
      
act( AT_SKILL, "You mount $N.", ch, NULL, victim, TO_CHAR );
      
act( AT_SKILL, "$n skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT );
      
act( AT_SKILL, "$n mounts you.", ch, NULL, victim, TO_VICT );
      
learn_from_success( ch, gsn_mount );
      
ch->position = POS_MOUNTED;
   
}
   
   else
      
   {
      
act( AT_SKILL, "You unsuccessfully try to mount $N.", ch, NULL, victim, TO_CHAR );
      
act( AT_SKILL, "$n unsuccessfully attempts to mount $N.", ch, NULL, victim, TO_NOTVICT );
      
act( AT_SKILL, "$n tries to mount you.", ch, NULL, victim, TO_VICT );
      
learn_from_failure( ch, gsn_mount );
   
}
   
return;

}

void do_dismount( CHAR_DATA * ch, char *argument ) 
{
   
CHAR_DATA * victim;
   
if( ( victim = ch->mount ) == NULL )
      
   {
      
if( ch->position == POS_MOUNTED )
         
ch->position = POS_STANDING;
      
send_to_char( "You're not mounted.\n\r", ch );
      
return;
   
}
   
if( can_use_skill( ch, number_percent(  ), gsn_mount ) )
      
   {
      
act( AT_SKILL, "You dismount $N.", ch, NULL, victim, TO_CHAR );
      
act( AT_SKILL, "$n skillfully dismounts $N.", ch, NULL, victim, TO_NOTVICT );
      
act( AT_SKILL, "$n dismounts you.  Whew!", ch, NULL, victim, TO_VICT );
      
if( IS_NPC( victim ) )
         
xREMOVE_BIT( victim->affected_by, AFF_MOUNTED );
      
      else
         
xREMOVE_BIT( victim->pcdata->perm_aff, AFF_MOUNTED );
      
ch->mount = NULL;
      
ch->position = POS_STANDING;
      
learn_from_success( ch, gsn_mount );
   
}
   
   else
      
   {
      
act( AT_SKILL, "You fall off while dismounting $N.  Ouch!", ch, NULL, victim, TO_CHAR );
      
act( AT_SKILL, "$n falls off of $N while dismounting.", ch, NULL, victim, TO_NOTVICT );
      
act( AT_SKILL, "$n falls off your back.", ch, NULL, victim, TO_VICT );
      
learn_from_failure( ch, gsn_mount );
      
if( IS_NPC( victim ) )
         
xREMOVE_BIT( victim->affected_by, AFF_MOUNTED );
      
      else
         
xREMOVE_BIT( victim->pcdata->perm_aff, AFF_MOUNTED );
      
ch->mount = NULL;
      
ch->position = POS_SITTING;
   
}
   
return;

}


/**************************************************************************/ 
   
/*
 * Check for parry.
 */ 
   bool check_parry( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
int chances;
   
if( !IS_AWAKE( victim ) )
      
return FALSE;
   
chances = UMIN( 60, 100 );
   
if( get_eq_char( victim, WEAR_HAND ) == NULL )
      
return FALSE;
   
if( !chance
         ( victim,
           chances - ( get_curr_str( ch ) + get_curr_dex( ch ) ) + ( get_curr_str( victim ) + get_curr_dex( victim ) ) ) )
      
   {
      
learn_from_failure( victim, gsn_parry );
      
return FALSE;
   
}
   
if( !IS_NPC( victim ) 
 &&!IS_SET( victim->pcdata->flags, PCFLAG_GAG ) )
       /*SB*/ 
act( AT_SKILL, "You parry $n's attack.", ch, NULL, victim, TO_VICT );
   
if( !IS_NPC( ch ) 
 &&!IS_SET( ch->pcdata->flags, PCFLAG_GAG ) )   /* SB */
      
act( AT_SKILL, "$N parries your attack.", ch, NULL, victim, TO_CHAR );
   
learn_from_success( victim, gsn_parry );
   
return TRUE;

}


/*
 * Check for dodge.
 */ 
   bool check_dodge( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
int chances;
   
if( !IS_AWAKE( victim ) )
      
return FALSE;
   
if( IS_NPC( victim ) && !xIS_SET( victim->defenses, DFND_DODGE ) )
      
return FALSE;
   
chances = UMIN( 60, get_curr_dex( victim ) );
   
chances += get_curr_dex( victim ) + 20;
   
chances -= GET_HITROLL( ch );
   
if( chances < 0 )
      chances = 20;
   
if( IS_SET( victim->mood, MOOD_DEFENSE ) )
      
chances *= 2;
   
if( IS_SET( victim->mood, MOOD_RELAXED ) )
      
chances = 0;
   
if( number_range( 1, chances ) < number_range( 1, get_curr_dex( ch ) ) )
   {
      
learn_from_failure( victim, gsn_dodge );
      
return FALSE;
   
}
   
if( !IS_NPC( victim ) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG ) )
      
act( AT_SKILL, "You dodge $n's attack.", ch, NULL, victim, TO_VICT );
   
if( !IS_NPC( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG ) )
      
act( AT_SKILL, "$N dodges your attack.", ch, NULL, victim, TO_CHAR );
   
gain_exp( ch, get_exp_worth( victim ) );
   
gain_exp( victim, get_exp_worth( ch ) );
   
learn_from_success( victim, gsn_dodge );
   
return TRUE;

}


bool check_tumble( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
int chances;
   
if( !IS_AWAKE( victim ) )
      
return FALSE;
   
if( !IS_NPC( victim ) 
 &&!LEARNED( victim, gsn_tumble ) > 0 )
      
return FALSE;
   
chances = UMIN( 60, get_curr_dex( victim ) );
   
chances += get_curr_dex( victim ) + 25;
   
chances -= GET_HITROLL( ch );
   
if( chances < 0 )
      chances = 20;
   
if( IS_SET( victim->mood, MOOD_DEFENSE ) )
      
chances *= 2;
   
if( IS_SET( victim->mood, MOOD_RELAXED ) )
      
chances = 0;
   
if( number_range( 1, chances ) < number_range( 1, get_curr_dex( ch ) ) )
   {
      
learn_from_failure( victim, gsn_tumble );
      
return FALSE;
   
}
   
if( !IS_NPC( victim ) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG ) )
      
act( AT_SKILL, "You tumble away from $n's attack.", ch, NULL, victim, TO_VICT );
   
if( !IS_NPC( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG ) )
      
act( AT_SKILL, "$N tumbles away from your attack.", ch, NULL, victim, TO_CHAR );
   
gain_exp( ch, get_exp_worth( victim ) );
   
gain_exp( victim, get_exp_worth( ch ) );
   
learn_from_success( victim, gsn_tumble );
   
return TRUE;

}


/*
 * Check for counterspell.
 */ 
   bool check_counterspell( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
int chances;
   
if( victim == NULL )
      
return FALSE;
   
if( !IS_AWAKE( victim ) )
      
return FALSE;
   
if( ch->curr_talent[TAL_CATALYSM] < 50 )
      
return FALSE;
   
if( victim->mana < 50 )
      
return FALSE;
   
chances = ( int )( LEARNED( victim, gsn_counterspell ) / 4 );
   
if( !chance( victim, chances - ( get_curr_int( ch ) + get_curr_wil( ch ) ) + 
                  ( get_curr_int( victim ) + get_curr_wil( victim ) ) ) )
      
   {
      
learn_from_failure( victim, gsn_counterspell );
      
return FALSE;
   
}
   
act( AT_MAGIC, "Thinking fast, you utter a quick counterspell.", ch, NULL, victim, TO_VICT );
   
act( AT_MAGIC, "$N utters a quick counterspell.", ch, NULL, victim, TO_CHAR );
   
act( AT_MAGIC, "$N utters a quick counterspell.", ch, NULL, victim, TO_NOTVICT );
   
victim->mana -= 5;
   
victim->in_room->area->weather->mana += 1;
   
learn_from_success( victim, gsn_counterspell );
   
return TRUE;

}

void do_poison_weapon( CHAR_DATA * ch, char *argument ) 
{
   
OBJ_DATA * obj, *pobj, *wobj;
   
char arg[MAX_INPUT_LENGTH];
   
int percent;
   
if( !skill_available( ch, gsn_poison_weapon ) )
      
   {
      
send_to_char( "What do you think you are, a thief?\n\r", ch );
      
return;
   
}
   
one_argument( argument, arg );
   
if( arg[0] == '\0' )
      
   {
      
send_to_char( "What are you trying to poison?\n\r", ch );
      
return;
   
}
   
if( IS_FIGHTING( ch ) )
      
   {
      
send_to_char( "While you're fighting?  Nice try.\n\r", ch );
      
return;
   
}
   
if( ms_find_obj( ch ) )
      
return;
   
if( !( obj = get_obj_carry( ch, arg ) ) )
      
   {
      
send_to_char( "You do not have that weapon.\n\r", ch );
      
return;
   
}
   
if( obj->item_type != ITEM_WEAPON )
      
   {
      
send_to_char( "That item is not a weapon.\n\r", ch );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_POISONED ) )
      
   {
      
send_to_char( "That weapon is already poisoned.\n\r", ch );
      
return;
   
}
   
      /*
       * Now we have a valid weapon...check to see if we have the powder. 
       */ 
      for( pobj = ch->first_carrying; pobj; pobj = pobj->next_content )
      
   {
      
if( pobj->pIndexData->vnum == OBJ_VNUM_BLACK_POWDER )
         
break;
   
}
   
if( !pobj )
      
   {
      
send_to_char( "You do not have the black poison powder.\n\r", ch );
      
return;
   
}
   
      /*
       * Okay, we have the powder...do we have water? 
       */ 
      for( wobj = ch->first_carrying; wobj; wobj = wobj->next_content )
      
   {
      
if( wobj->item_type == ITEM_DRINK_CON 
 &&wobj->value[1] > 0 
 &&wobj->value[2] == 0 )
         
break;
   
}
   
if( !wobj )
      
   {
      
send_to_char( "You have no water to mix with the powder.\n\r", ch );
      
return;
   
}
   
      /*
       * Great, we have the ingredients...but is the thief smart enough? 
       */ 
      if( !IS_NPC( ch ) && number_percent(  ) > get_curr_wil( ch ) )
      
   {
      
send_to_char( "You can't quite remember what to do...\n\r", ch );
      
return;
   
}
   
      /*
       * And does the thief have steady enough hands? 
       */ 
      if( !IS_NPC( ch ) 
 &&( ( number_percent(  ) > get_curr_dex( ch ) ) || ch->pcdata->condition[COND_DRUNK] > 0 ) )
      
   {
      
send_to_char( "Your hands aren't steady enough to properly mix the poison.\n\r", ch );
      
return;
   
}
   
percent = ( number_percent(  ) - get_curr_lck( ch ) / 100 );
   
      /*
       * Check the skill percentage 
       */ 
      separate_obj( pobj );
   
separate_obj( wobj );
   
if( !can_use_skill( ch, percent, gsn_poison_weapon ) )
      
   {
      
set_char_color( AT_RED, ch );
      
send_to_char( "You failed and spill some on yourself.  Ouch!\n\r", ch );
      
set_char_color( AT_GREY, ch );
      
act( AT_RED, "$n spills the poison all over!", ch, NULL, NULL, TO_ROOM );
      
extract_obj( pobj );
      
extract_obj( wobj );
      
learn_from_failure( ch, gsn_poison_weapon );
      
return;
   
}
   
separate_obj( obj );
   
      /*
       * Well, I'm tired of waiting.  Are you? 
       */ 
      act( AT_RED, "You mix $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_CHAR );
   
act( AT_RED, "$n mixes $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_ROOM );
   
act( AT_GREEN, "You pour the poison over $p, which glistens wickedly!", ch, obj, NULL, TO_CHAR );
   
act( AT_GREEN, "$n pours the poison over $p, which glistens wickedly!", ch, obj, NULL, TO_ROOM );
   
xSET_BIT( obj->extra_flags, ITEM_POISONED );
   
      /*
       * Set an object timer.  Don't want proliferation of poisoned weapons 
       */ 
      obj->timer = number_range( 1, 20 );
   
if( IS_OBJ_STAT( obj, ITEM_BLESS ) )
      
obj->timer *= 2;
   
if( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      
obj->timer *= 2;
   
      /*
       * WHAT?  All of that, just for that one bit?  How lame. ;) 
       */ 
      act( AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_CHAR );
   
act( AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_ROOM );
   
extract_obj( pobj );
   
extract_obj( wobj );
   
learn_from_success( ch, gsn_poison_weapon );
   
return;

}

void do_scribe( CHAR_DATA * ch, char *argument ) 
{
   OBJ_DATA *scroll, *pen;
   int sn, mana;
   char buf[MAX_STRING_LENGTH];
   bool found;

   if( IS_NPC( ch ) )
      return;

   if( !skill_available( ch, gsn_scribe ) )
   {
      send_to_char( "A skill such as this requires more magical ability than you have.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' || !str_cmp( argument, "" ) )
   {
      send_to_char( "Scribe what?\n\r", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( sn = find_spell( ch, argument, TRUE ) ) < 0 )
   {
      send_to_char( "You have not learned that spell.\n\r", ch );
      return;
   }

   if( skill_table[sn]->spell_fun == spell_null )
   {
      send_to_char( "That's not a spell!\n\r", ch );
      return;
   }

   if( SPELL_FLAG( skill_table[sn], SF_NOSCRIBE ) )
   {
      send_to_char( "You cannot scribe that spell.\n\r", ch );
      return;
   }

   mana = IS_NPC( ch ) ? 0 : UMAX( skill_table[sn]->min_mana, 100 / ( 100 - skill_table[sn]->skill_level[get_best_talent( ch, sn )] ) );
   mana *= 5;

   if( !IS_NPC( ch ) && ch->mana < mana )
   {
      send_to_char( "You don't have enough mana.\n\r", ch );
      return;
   }

   scroll = get_eq_char( ch, -1 );
   if( !scroll || scroll->pIndexData->vnum != OBJ_VNUM_SCROLL_SCRIBING )
      scroll = get_eq_char( ch, -2 );
   if( !scroll || scroll->pIndexData->vnum != OBJ_VNUM_SCROLL_SCRIBING )
   {
      send_to_char( "You must be holding a blank scroll to scribe it.\n\r", ch );
      return;
   }

   if( scroll->value[1] != -1 )
   {
      send_to_char( "That scroll has already been inscribed.\n\r", ch );
      return;
   }

   found = FALSE;
   for( pen = ch->first_carrying; pen; pen = pen->next_content )
   {
      if( ( pen->item_type == ITEM_PEN ) && ( IS_OBJ_STAT( pen, ITEM_MAGIC ) ) )
      {
         found = TRUE;
         break;
      }
   }

   if( !found )
   {
      send_to_char( "You must have a magical pen to scribe.\n\r", ch );
      return;
   }

   if( !process_spell_components( ch, sn ) )
   {
      learn_from_failure( ch, gsn_scribe );
      ch->mana -= ( mana / 2 );
      ch->in_room->area->weather->mana += ( int )mana / 10;
      return;
   }

   if( !can_use_skill( ch, number_percent(  ), gsn_scribe ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You failed.\n\r", ch );
      learn_from_failure( ch, gsn_scribe );
      ch->mana -= ( mana / 2 );
      ch->in_room->area->weather->mana += ( int )mana / 10;
      return;
   }

   scroll->value[1] = sn;

   snprintf( buf, sizeof( buf ), "%s scroll", skill_table[sn]->name );
   STRFREE( scroll->short_descr );
   scroll->short_descr = STRALLOC( aoran( buf ) );

   snprintf( buf, sizeof( buf ), "A glowing scroll inscribed '%s' lies in the dust.", skill_table[sn]->name );
   STRFREE( scroll->description );
   scroll->description = STRALLOC( buf );

   snprintf( buf, sizeof( buf ), "scroll scribing %s", skill_table[sn]->name );
   STRFREE( scroll->name );
   scroll->name = STRALLOC( buf );

   act( AT_MAGIC, "$n magically scribes $p.", ch, scroll, NULL, TO_ROOM );
   act( AT_MAGIC, "You magically scribe $p.", ch, scroll, NULL, TO_CHAR );

   learn_from_success( ch, gsn_scribe );
   ch->mana -= mana;
   ch->in_room->area->weather->mana += ( int )mana / 10;
} 

void do_brew( CHAR_DATA * ch, char *argument ) 
{
   OBJ_DATA *potion, *fire;
   int sn, mana;
   char buf[MAX_STRING_LENGTH];
   bool found;

   if( IS_NPC( ch ) )
      return;

   if( !skill_available( ch, gsn_brew ) )
   {
      send_to_char( "A skill such as this requires more magical ability than you have.\n\r", ch );
      return;
   }

   if( argument[0] == '\0' || !str_cmp( argument, "" ) )
   {
      send_to_char( "Brew what?\n\r", ch );
      return;
   }

   if( ms_find_obj( ch ) )
      return;

   if( ( sn = find_spell( ch, argument, TRUE ) ) < 0 )
   {
      send_to_char( "You have not learned that spell.\n\r", ch );
      return;
   }

   if( skill_table[sn]->spell_fun == spell_null )
   {
      send_to_char( "That's not a spell!\n\r", ch );
      return;
   }

   if( SPELL_FLAG( skill_table[sn], SF_NOBREW ) )
   {
      send_to_char( "You cannot brew that spell.\n\r", ch );
      return;
   }

   mana = IS_NPC( ch ) ? 0 : UMAX( skill_table[sn]->min_mana, 100 / ( 100 - skill_table[sn]->skill_level[get_best_talent( ch, sn )] ) );
   mana *= 4;

   if( !IS_NPC( ch ) && ch->mana < mana )
   {
      send_to_char( "You don't have enough mana.\n\r", ch );
      return;
   }

   found = FALSE;

   for( fire = ch->in_room->first_content; fire; fire = fire->next_content )
   {
      if( fire->item_type == ITEM_FIRE )
      {
         found = TRUE;
         break;
      }
   }

   if( !found )
   {
      send_to_char( "There must be a fire in the room to brew a potion.\n\r", ch );
      return;
   }

   potion = get_eq_char( ch, -1 );
   if( !potion || potion->pIndexData->vnum != OBJ_VNUM_EMPTY_FLASK )
      potion = get_eq_char( ch, -2 );
   if( !potion || potion->pIndexData->vnum != OBJ_VNUM_EMPTY_FLASK )
   {
      send_to_char( "You must be holding an empty flask to brew a potion.\n\r", ch );
      return;
   }

   if( potion->value[1] != -1 )
   {
      send_to_char( "That's not an empty flask.\n\r", ch );
      return;
   }

   if( !process_spell_components( ch, sn ) )
   {
      learn_from_failure( ch, gsn_brew );
      ch->mana -= ( mana / 2 );
      ch->in_room->area->weather->mana += ( int )mana / 10;
      return;
   }

   if( !can_use_skill( ch, number_percent(  ), gsn_brew ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You failed.\n\r", ch );
      learn_from_failure( ch, gsn_brew );
      ch->mana -= ( mana / 2 );
      ch->in_room->area->weather->mana += ( int )mana / 10;
      return;
   }

   potion->value[1] = sn;

   snprintf( buf, sizeof( buf ), "%s potion", skill_table[sn]->name );
   STRFREE( potion->short_descr );
   potion->short_descr = STRALLOC( aoran( buf ) );

   snprintf( buf, sizeof( buf ), "A strange potion labelled '%s' sizzles in a glass flask.", skill_table[sn]->name );
   STRFREE( potion->description );
   potion->description = STRALLOC( buf );

   snprintf( buf, sizeof( buf ), "flask potion %s", skill_table[sn]->name );
   STRFREE( potion->name );
   potion->name = STRALLOC( buf );

   act( AT_MAGIC, "$n brews up $p.", ch, potion, NULL, TO_ROOM );
   act( AT_MAGIC, "You brew up $p.", ch, potion, NULL, TO_CHAR );

   learn_from_success( ch, gsn_brew );
   ch->mana -= mana;
   ch->in_room->area->weather->mana += ( int )mana / 10;
} 

bool check_grip( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
int chance;
   
if( !IS_AWAKE( victim ) )
      
return FALSE;
   
if( IS_NPC( victim ) && !xIS_SET( victim->defenses, DFND_GRIP ) )
      
return FALSE;
   
if( IS_NPC( victim ) )
      
chance = UMIN( 60, 100 );
   
   else
      
chance = ( int )( LEARNED( victim, gsn_grip ) / 2 );
   
      /*
       * Consider luck as a factor 
       */ 
      chance += ( 2 * ( get_curr_lck( victim ) / 10 ) );
   
if( number_percent(  ) >= chance + number_percent(  ) )
      
   {
      
learn_from_failure( victim, gsn_grip );
      
return FALSE;
   
}
   
act( AT_SKILL, "You evade $n's attempt to disarm you.", ch, NULL, victim, TO_VICT );
   
act( AT_SKILL, "$N holds $S weapon strongly, and is not disarmed.", 
ch, NULL, victim, TO_CHAR );
   
learn_from_success( victim, gsn_grip );
   
return TRUE;

}


bool check_illegal_psteal( CHAR_DATA * ch, CHAR_DATA * victim ) 
{
   
if( !IS_NPC( victim ) && !IS_NPC( ch ) )
      
   {
      
if( xIS_SET( ch->act, PLR_AFK ) )
         
      {
         
            /*
             * sprintf( log_buf, "%s illegally stealing from %s at %d",
             * (IS_NPC(ch) ? ch->short_descr : ch->name),
             * victim->name,
             * victim->in_room->vnum );
             * log_string( log_buf );
             * to_channel( log_buf, CHANNEL_MONITOR, "Monitor", PERMIT_SECURITY );
             * xSET_BIT(ch->act, PLR_THIEF); 
             */ 
            return TRUE;
      
}
   
}
   
return FALSE;

}

void do_guard( CHAR_DATA * ch, char *argument ) 
{
   
if( !IS_SET( ch->mood, MOOD_GUARD ) )
   {
      
SET_BIT( ch->mood, MOOD_GUARD );
      
act( AT_SKILL, "You stand in a ready posture, prepared to defend against intruders.", ch, NULL, NULL, TO_CHAR );
      
act( AT_SKILL, "$n stands in a ready posture, prepared to defend against intruders.", ch, NULL, NULL, TO_ROOM );
   
}
   else
   {
      
REMOVE_BIT( ch->mood, MOOD_GUARD );
      
act( AT_SKILL, "You stand down from your defensive posture.", ch, NULL, NULL, TO_CHAR );
      
act( AT_SKILL, "$n stands down from $s defensive posture.", ch, NULL, NULL, TO_ROOM );
   
}

}

void do_scan( CHAR_DATA * ch, char *argument ) 
{
   
ROOM_INDEX_DATA * was_in_room;
   
EXIT_DATA * pexit;
   
sh_int dir = -1;
   
sh_int dist;
   
sh_int max_dist = 8;
   
set_char_color( AT_ACTION, ch );
   
if( IS_AFFECTED( ch, AFF_BLIND ) && !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      
   {
      
send_to_char( "Not very effective when you can't see...\n\r", ch );
      
return;
   
}
   
if( argument[0] == '\0' )
      
   {
      
send_to_char( "Scan in a direction...\n\r", ch );
      
return;
   
}
   
if( ( dir = get_door( argument ) ) == -1 )
      
   {
      
send_to_char( "Scan in WHAT direction?\n\r", ch );
      
return;
   
}
   
was_in_room = ch->in_room;
   
act( AT_GREY, "Scanning $t...", ch, dir_name[dir], NULL, TO_CHAR );
   
act( AT_GREY, "$n scans $t.", ch, dir_name[dir], NULL, TO_ROOM );
   
if( !can_use_skill( ch, number_percent(  ), gsn_scan ) )
      
   {
      
act( AT_GREY, "You stop scanning $t as your vision blurs.", ch, 
dir_name[dir], NULL, TO_CHAR );
      
learn_from_failure( ch, gsn_scan );
      
return;
   
}
   
if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
      
   {
      
act( AT_GREY, "You can't see $t.", ch, dir_name[dir], NULL, TO_CHAR );
      
return;
   
}
   
max_dist -= ( 100 - get_curr_per( ch ) ) / 10;
   
for( dist = 1; dist <= max_dist; )
      
   {
      
if( IS_SET( pexit->exit_info, EX_CLOSED ) )
         
      {
         
if( IS_SET( pexit->exit_info, EX_SECRET ) 
 ||IS_SET( pexit->exit_info, EX_DIG ) )
            
act( AT_GREY, "Your view $t is blocked by a wall.", ch, 
dir_name[dir], NULL, TO_CHAR );
         
         else
            
act( AT_GREY, "Your view $t is blocked by a door.", ch, 
dir_name[dir], NULL, TO_CHAR );
         
break;
      
}
      
if( room_is_private( pexit->to_room ) )
         
      {
         
act( AT_GREY, "Your view $t is blocked by a private room.", ch, 
dir_name[dir], NULL, TO_CHAR );
         
break;
      
}
      
char_from_room( ch );
      
char_to_room( ch, pexit->to_room );
      
set_char_color( AT_WHITE, ch );
      
send_to_char( ch->in_room->name, ch );
      
send_to_char( "\n\r", ch );
      
show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
      
show_char_to_char( ch->in_room->first_person, ch );
      
switch ( ch->in_room->sector_type )
         
      {
         
default:
            dist++;
            break;
         
case SECT_AIR:
            
if( number_percent(  ) < 80 )
               dist++;
            break;
         
case SECT_INSIDE:
         
case SECT_FIELD:
         
case SECT_UNDERGROUND:
            
dist++;
            break;
         
case SECT_FOREST:
         
case SECT_CITY:
         
case SECT_DESERT:
         
case SECT_HILLS:
            
dist += 2;
            break;
         
case SECT_WATER_SWIM:
         
case SECT_WATER_NOSWIM:
            
dist += 3;
            break;
         
case SECT_MOUNTAIN:
         
case SECT_UNDERWATER:
         
case SECT_OCEANFLOOR:
            
dist += 4;
            break;
      
}
      
if( dist >= max_dist )
         
      {
         
act( AT_GREY, "Your vision blurs with distance and you see no " 
               "farther $t.", ch, dir_name[dir], NULL, TO_CHAR );
         
break;
      
}
      
if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
         
      {
         
act( AT_GREY, "Your view $t is blocked by a wall.", ch, 
dir_name[dir], NULL, TO_CHAR );
         
break;
      
}
   
}
   
char_from_room( ch );
   
char_to_room( ch, was_in_room );
   
learn_from_success( ch, gsn_scan );
   
return;

}


/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.	-Thoric
 */ 
   CHAR_DATA * scan_for_victim( CHAR_DATA * ch, EXIT_DATA * pexit, char *name )
{
   
CHAR_DATA * victim;
   
ROOM_INDEX_DATA * was_in_room;
   
sh_int dist, dir;
   
sh_int max_dist = 8;
   
if( IS_AFFECTED( ch, AFF_BLIND ) || !pexit )
      
return NULL;
   
was_in_room = ch->in_room;
   
      /*
       * Keo wanted a perception stat.. I now see why :) -- Scion 
       */ 
      
max_dist -= ( 100 - get_curr_per( ch ) ) / 10;
   
for( dist = 1; dist <= max_dist; )
      
   {
      
if( IS_SET( pexit->exit_info, EX_CLOSED ) )
         
break;
      
if( room_is_private( pexit->to_room ) )
         
break;
      
char_from_room( ch );
      
char_to_room( ch, pexit->to_room );
      
if( ( victim = get_char_room( ch, name ) ) != NULL )
         
      {
         
char_from_room( ch );
         
char_to_room( ch, was_in_room );
         
return victim;
      
}
      
switch ( ch->in_room->sector_type )
         
      {
         
default:
            dist++;
            break;
         
case SECT_AIR:
            
if( number_percent(  ) < 80 )
               dist++;
            break;
         
case SECT_INSIDE:
         
case SECT_FIELD:
         
case SECT_UNDERGROUND:
            
dist++;
            break;
         
case SECT_FOREST:
         
case SECT_CITY:
         
case SECT_DESERT:
         
case SECT_HILLS:
            
dist += 2;
            break;
         
case SECT_WATER_SWIM:
         
case SECT_WATER_NOSWIM:
            
dist += 3;
            break;
         
case SECT_MOUNTAIN:
         
case SECT_UNDERWATER:
         
case SECT_OCEANFLOOR:
            
dist += 4;
            break;
      
}
      
if( dist >= max_dist )
         
break;
      
dir = pexit->vdir;
      
if( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
         
break;
   
}
   
char_from_room( ch );
   
char_to_room( ch, was_in_room );
   
return NULL;

}


/*
 * Search inventory for an appropriate projectile to fire.
 * Also search open quivers.					-Thoric
 */ 
   OBJ_DATA * find_projectile( CHAR_DATA * ch, int type )
{
   
OBJ_DATA * obj, *obj2;
   
for( obj = ch->last_carrying; obj; obj = obj->prev_content )
      
   {
      
if( can_see_obj( ch, obj ) )
         
      {
         
if( obj->item_type == ITEM_QUIVER && !IS_SET( obj->value[1], CONT_CLOSED ) )
            
         {
            
for( obj2 = obj->last_content; obj2; obj2 = obj2->prev_content )
               
            {
               
if( obj2->item_type == ITEM_PROJECTILE 
 &&obj2->value[3] == type )
                  
return obj2;
            
}
         
}
         
if( obj->item_type == ITEM_PROJECTILE && obj->value[3] == type )
            
return obj;
      
}
   
}
   
return NULL;

}


ch_ret spell_attack( int, int, CHAR_DATA *, void * );

/*
 * Perform the actual attack on a victim			-Thoric
 */ 
   ch_ret ranged_got_target( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon, 
OBJ_DATA * projectile, sh_int dist,
                             sh_int dt, char *stxt, sh_int color ) 
{
   
if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
      
   {
      
         /*
          * safe room, bubye projectile 
          */ 
         if( projectile )
         
      {
         
ch_printf( ch, "Your %s is blasted from existence by a mystical force.", 
myobj( projectile ) );
         
act( color, "A mystical force smites $p!", ch, projectile, NULL, TO_ROOM );
         
extract_obj( projectile );
      
}
      
      else
         
      {
         
ch_printf( ch, "Your %s is blasted from existence by a mystical force.", 
stxt );
         
act( color, "A mystical force smites $t!", ch, aoran( stxt ), NULL, TO_ROOM );
      
}
      
return rNONE;
   
}
   
if( victim == ch )
   {
      
act( AT_MAGIC, "Your magic fizzles.", ch, NULL, NULL, TO_CHAR );
      
return rNONE;
   
}
   
if( number_percent(  ) > 50 || ( projectile && weapon 
 &&can_use_skill( ch, number_percent(  ), 100 ) ) )
      
   {
      
global_retcode = spell_attack( dt, get_curr_wil( ch ), ch, victim );
   
}
   
   else
      
   {
      
if( projectile )
         
      {
         
            /*
             * 50% chance of getting lost 
             */ 
            if( number_percent(  ) < 50 && projectile->item_type == ITEM_PROJECTILE )
            
extract_obj( projectile );
         
         else
            
         {
            
if( projectile->in_obj )
               
obj_from_obj( projectile );
            
if( projectile->carried_by )
               
obj_from_char( projectile );
            
obj_to_room( projectile, victim->in_room );
         
}
      
}
   
}
   
return global_retcode;

}


/*  New check to see if you can use skills to support morphs --Shaddai */ 
   bool can_use_skill( CHAR_DATA * ch, int percent, int gsn )
{
   
bool check = FALSE;
   
if( IS_NPC( ch ) && percent < 85 )
      check = TRUE;
   
   else if( !IS_NPC( ch ) && percent < LEARNED( ch, gsn ) )
      check = TRUE;
   
return check;

}


/* Alchemy: Mix two items together to produce a third item -keo */ 
void do_mix( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA * first;
   
OBJ_DATA * second;
   
OBJ_DATA * obj;
   
RECIPE_DATA * recipe;
   
char arg1[MAX_INPUT_LENGTH];
   
bool found;
   
OBJ_DATA * drinkcon = NULL;
   
argument = one_argument( argument, arg1 );
   
      /*
       * Do we have the ingredients? 
       */ 
      first = get_obj_carry( ch, arg1 );
   
if( !first )
   {
      
send_to_char( "You don't have that.\n\r", ch );
      
return;
   
}
   
second = get_obj_carry( ch, argument );
   
if( !second )
   {
      
send_to_char( "You don't have that.\n\r", ch );
      
return;
   
}
   
      /*
       * Now that we have the ingredients, see if there's a recipe to match 
       */ 
      recipe = first_recipe;
   
while( recipe )
   {
      
if( recipe->ingred1 == first->pIndexData->vnum 
 &&recipe->ingred2 == second->pIndexData->vnum )
         
break;
      
if( recipe == last_recipe )
      {
         
send_to_char( "Nothing happens.\n\r", ch );
         
return;
      
}
      
recipe = recipe->next;
   
}
   
if( !recipe )
      return;
   
      /*
       * We found the recipe, now make sure we have anything we need 
       */ 
      
      /*
       * Check non obvious ones first, so they think there isn't a recipe 
       */ 
      if( IS_SET( recipe->flags, RECIPE_LIGHT ) )
   {
      
if( get_light_room( ch->in_room ) < 10 )
      {
         
send_to_char( "Nothing happens.\n\r", ch );
         
return;
      
}
   
}
   
if( IS_SET( recipe->flags, RECIPE_DARK ) )
   {
      
if( get_light_room( ch->in_room ) > -10 )
      {
         
send_to_char( "Nothing happens.\n\r", ch );
         
return;
      
}
   
}
   
      /*
       * Now check obvious ones 
       */ 
      if( IS_SET( recipe->flags, RECIPE_FIRE ) )
   {
      
found = FALSE;
      
for( obj = ch->in_room->first_content; obj; 
obj = obj->next_content )
      {
         
if( obj->item_type == ITEM_FIRE )
         {
            
found = TRUE;
            
break;
         
}
      
}
      
if( !found )
      {
         
send_to_char( 
"There must be a fire in the room to use this recipe.\n\r", ch );
         
return;
      
}
   
}
   
if( IS_SET( recipe->flags, RECIPE_WATER ) )
   {
      
found = FALSE;
      
if( ch->in_room->curr_water > 50 )
         
found = TRUE;
      
      else
         
for( obj = ch->in_room->first_content; obj; 
obj = obj->next_content )
         {
            
if( obj->item_type == ITEM_FOUNTAIN )
            {
               
found = TRUE;
               
break;
            
}
         
}
      
if( !found )
      {
         
send_to_char( 
"There must be a source of water in the room to use this recipe.\n\r", ch );
         
return;
      
}
   
}
   
if( IS_SET( recipe->flags, RECIPE_BLOOD ) )
   {
      
found = FALSE;
      
for( obj = ch->in_room->first_content; obj; 
obj = obj->next_content )
      {
         
if( obj->item_type == ITEM_BLOOD )
         {
            
found = TRUE;
            
break;
         
}
      
}
      
if( !found )
      {
         
send_to_char( 
"There must fresh blood spilled in the room to use this recipe.\n\r", ch );
         
return;
      
}
   
}
   
if( IS_SET( recipe->flags, RECIPE_DRINKCON ) )
   {
      
found = FALSE;
      
for( obj = ch->first_carrying; obj; 
obj = obj->next_content )
      {
         
if( obj->item_type == ITEM_DRINK_CON )
         {
            
found = TRUE;
            
drinkcon = obj;
            
break;
         
}
      
}
      
if( !found )
      {
         
send_to_char( 
"You must have a drink container to mix this recipe in.\n\r", ch );
         
return;
      
}
   
}
   
if( IS_SET( recipe->flags, RECIPE_ICE ) )
   {
      
if( ch->in_room->area->weather->temp > -200 
 &&ch->curr_talent[TAL_FROST] < 20 )
      {
         
send_to_char( 
"You must have ice available to make this recipe.\n\r", ch );
         
return;
      
}
   
}
   
if( IS_SET( recipe->flags, RECIPE_KNIFE ) )
   {
      
found = FALSE;
      
if( can_use_bodypart( ch, BP_CLAWS ) )
         
found = TRUE;
      
      else
         
for( obj = ch->first_carrying; obj; 
obj = obj->next_content )
         {
            
if( obj->item_type == ITEM_WEAPON 
 &&( obj->value[5] == 1 
 ||obj->value[5] == 2 ) )
            {
               
found = TRUE;
               
break;
            
}
         
}
      
if( !found )
      {
         
send_to_char( 
"You must have a cutting implement to use this recipe.\n\r", ch );
         
return;
      
}
   
}
   
      /*
       * Everything in check, now put them together and give you the result 
       */ 
      if( !get_obj_index( recipe->result ) )
   {
      
bug( "Do_mix: can't find vnum for result on recipe %d", recipe->number );
      
send_to_char( "Something went wrong somewhere...\n\r", ch );
      
return;
   
}
   
separate_obj( first );
   
separate_obj( second );
   
if( drinkcon )
      separate_obj( drinkcon );
   
learn_noncombat( ch, SK_ALCHEMY );
   
      /*
       * Did it work or not? 
       */ 
      if( number_range( 1, 30 ) < number_range( 1, ch->pcdata->noncombat[SK_ALCHEMY] ) )
   {
      
obj = create_object( get_obj_index( recipe->result ), 0 );
      
act( AT_SKILL, "$n mixes some ingredients to create $p.", ch, obj, NULL, TO_ROOM );
      
ch_printf( ch, "&GYou mix %s and %s, to create %s!\n\r", 
first->short_descr, second->short_descr,
                  
obj->short_descr );
      
obj_to_char( obj, ch );
      
if( drinkcon )
         extract_obj( drinkcon );
      
learn_from_success( ch, gsn_mix );
   
}
   else
   {
      
act( AT_SKILL, "$n messes up a recipe.", ch, NULL, NULL, TO_ROOM );
      
act( AT_SKILL, "You screw up the recipe somewhere and end up with an awful mess.", ch, NULL, NULL, TO_CHAR );
      
learn_from_failure( ch, gsn_mix );
   
}
   
obj_from_char( first );
   
obj_from_char( second );
   
extract_obj( first );
   
extract_obj( second );

}

void do_engrave( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA * obj;
   
char arg[MAX_STRING_LENGTH];
   
char buf[MAX_STRING_LENGTH];
   
extern sh_int gsn_engrave;
   
      /*
       * engrave <obj> <string> 
       */ 
      argument = one_argument( argument, arg );
   
obj = get_obj_carry( ch, arg );
   
if( !obj )
   {
      
send_to_char( "What object do you want to engrave?\r\n", ch );
      
return;
   
}
   
separate_obj( obj );
   
if( strlen( argument ) > obj->weight * 3 )
   {
      
send_to_char( "That won't fit on that item, please try something shorter.\r\n", ch );
      
return;
   
}
   
if( strlen( argument ) < 2 )
   {
      
send_to_char( "What do you want to engrave on your item?\r\n", ch );
      
return;
   
}
   
if( !IS_OBJ_STAT( obj, ITEM_METAL ) )
   {
      
send_to_char( "You can only engrave metal items.\r\n", ch );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      
send_to_char( "There is no way you can even leave a scratch on that item!\r\n", ch );
      
return;
   
}
   
if( nifty_is_name( "named", obj->short_descr ) )
   {
      
send_to_char( "That item has already been engraved.\r\n", ch );
      
return;
   
}
   
      /*
       * Scramble their text if they fail their skill roll. Mwahaha! -- Scion 
       */ 
      if( !IS_NPC( ch ) && number_percent(  ) + skill_table[gsn_engrave]->difficulty * 5 > LEARNED( ch, gsn_engrave ) )
   {
      
argument = scramble( argument, LEARNED( ch, gsn_engrave ) );
      
learn_from_failure( ch, gsn_engrave );
   
}
   
act( AT_SKILL, "You engrave $p.", ch, obj, NULL, TO_CHAR );
   
act( AT_SKILL, "$n engraves $p.", ch, obj, NULL, TO_ROOM );
   
      /*
       * change the namelist 
       */ 
      strcpy( buf, obj->name );
   
strcat( buf, " " );
   
strcat( buf, argument );
   
STRFREE( obj->name );
   
obj->name = STRALLOC( buf );
   
      /*
       * change the short desc 
       */ 
      strcpy( buf, obj->short_descr );
   
strcat( buf, " named '" );
   
strcat( buf, argument );
   
strcat( buf, "&w'" );
   
STRFREE( obj->short_descr );
   
obj->short_descr = STRALLOC( buf );
   
learn_from_success( ch, gsn_engrave );

}

void do_combine( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA * obj;
   
OBJ_DATA * obj2;
   
char arg[MAX_STRING_LENGTH];
   
char arg2[MAX_STRING_LENGTH];
   
char buf[MAX_STRING_LENGTH];
   
AFFECT_DATA * paf;
   
extern sh_int gsn_combine;
   
int div;
   
STRFREE( ch->last_taken );
   
ch->last_taken = STRALLOC( "combining ores" );
   
if( !can_use_bodypart( ch, BP_RHAND ) )
   {
      
send_to_char( "That would be hard without functional hands.\n\r", ch );
      
return;
   
}
   
      /*
       * combine <ore> <other ore> 
       */ 
      argument = one_argument( argument, arg );
   
argument = one_argument( argument, arg2 );
   
obj = get_obj_carry( ch, arg );
   
if( !obj )
   {
      
send_to_char( "Which raw materials do you want to combine?\r\n", ch );
      
return;
   
}
   
separate_obj( obj );
   
obj_from_char( obj );  /* in case arg and arg2 are the same */
   
obj2 = get_obj_carry( ch, arg2 );
   
if( !obj2 )
   {
      
send_to_char( "What raw material do you want to add to the first one?\r\n", ch );
      
obj_to_char( obj, ch );
      
return;
   
}
   
if( obj->item_type != ITEM_TREASURE 
 ||obj2->item_type != ITEM_TREASURE )
   {
      
send_to_char( "You may only combine raw materials.\r\n", ch );
      
obj_to_char( obj, ch );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_GEM ) || IS_OBJ_STAT( obj2, ITEM_GEM ) )
   {
      
send_to_char( "Use AFFIX to affix gems to items.\n\r", ch );
      
obj_to_char( obj, ch );
      
return;
   
}
   
if( ( IS_OBJ_STAT( obj, ITEM_METAL ) && ( IS_OBJ_STAT( obj2, ITEM_ORGANIC ) ) ) 
         ||( IS_OBJ_STAT( obj, ITEM_ORGANIC ) && ( IS_OBJ_STAT( obj2, ITEM_METAL ) ) ) )
   {
      
send_to_char( "You can only combine metals with metals, or nonmetals with nonmetals.\r\n", ch );
      
obj_to_char( obj, ch );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) 
 ||IS_OBJ_STAT( obj2, ITEM_DREAMWORLD ) )
   {
      
send_to_char( "Your ore is too insubstantial!\n\r", ch );
      
return;
   
}
   
if( obj == obj2 )
   {
      
send_to_char( "You cannot combine a material with itself.\r\n", ch );
      
obj_to_char( obj, ch );
      
return;
   
}
   
separate_obj( obj2 );
   
obj_from_char( obj2 );
   
if( IS_OBJ_STAT( obj, ITEM_METAL ) )
   {
      
learn_noncombat( ch, SK_SMITH );
      
WAIT_STATE( ch, ( 100 - ch->pcdata->noncombat[SK_SMITH] ) / 5 );
   
}
   else
   {
      
learn_noncombat( ch, SK_TAILOR );
      
WAIT_STATE( ch, ( 100 - ch->pcdata->noncombat[SK_TAILOR] ) / 5 );
   
}
   
      /*
       * Take away their ore if they fail their skill roll. Mwahaha! -- Scion 
       */ 
      if( !IS_NPC( ch ) && number_percent(  ) + 
skill_table[gsn_combine]->difficulty * 5 > LEARNED( ch, gsn_combine ) )
   {
      
act( AT_SKILL, "You fail to combine your raw materials properly, rendering the alloy unusable.", ch, NULL, NULL,
             TO_CHAR );
      
learn_from_failure( ch, gsn_combine );
      
extract_obj( obj );
      
extract_obj( obj2 );
      
return;
   
}
   else
   {
      
act( AT_SKILL, "You combine your two materials.", ch, obj, NULL, TO_CHAR );
   
}
   
act( AT_SKILL, "$n combines some materials.", ch, obj, NULL, TO_ROOM );
   
      /*
       * combine the stats of the two ores if they're different kinds 
       */ 
      if( obj->weight < obj2->weight )
   {
      
strcpy( buf, obj2->name );
      
STRFREE( obj->name );
      
obj->name = STRALLOC( buf );
      
strcpy( buf, obj2->short_descr );
      
STRFREE( obj->short_descr );
      
obj->short_descr = STRALLOC( buf );
      
strcpy( buf, obj2->description );
      
STRFREE( obj->description );
      
obj->description = STRALLOC( buf );
      
obj->extra_flags = obj2->extra_flags;
      
obj->material = obj2->material;
   
}
   
div = ( obj->weight + obj2->weight ) / obj->weight;
   
obj->weight += obj2->weight;
   
obj->cost += obj2->cost;
   
obj->condition = number_fuzzy( obj->weight * 50 );
   
if( div <= 1 )
      div = 2;
   
for( paf = obj->first_affect; paf; )
   {
      
AFFECT_DATA * npaf;
      
npaf = paf->next;
      
if( paf->location != APPLY_RESISTANT 
 &&paf->location != APPLY_AFFECT )
      {
         
int oldmod;
         
oldmod = paf->modifier;
         
paf->modifier /= div;
         
obj->cost -= 50000 * oldmod - paf->modifier;
         
if( paf->modifier == 0 )
         {
            
UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
            
DISPOSE( paf );
         
}
      
}
      
paf = npaf;
   
}
   
div = obj->weight / obj2->weight;
   
if( div <= 1 )
      div = 2;
   
for( paf = obj2->first_affect; paf; )
   {
      
AFFECT_DATA * npaf;
      
npaf = paf->next;
      
if( paf->location != APPLY_RESISTANT 
 &&paf->location != APPLY_AFFECT )
      {
         
paf->modifier /= div;
         
if( paf->modifier == 0 )
         {
            
UNLINK( paf, obj2->first_affect, obj2->last_affect, next, prev );
            
DISPOSE( paf );
         
}
      
}
      
paf = npaf;
   
}
   
      /*
       * transfer obj2's aff list to obj and clear the pointers off obj2 
       */ 
      if( !obj->last_affect && obj2->last_affect )
   {
      
obj->first_affect = obj2->first_affect;
      
obj->last_affect = obj2->last_affect;
      
obj2->first_affect = NULL;
      
obj2->last_affect = NULL;
   
}
   else if( obj->last_affect && obj2->first_affect )
   {
      
obj->last_affect->next = obj2->first_affect;
      
obj2->first_affect->prev = obj->last_affect;
      
obj->last_affect = obj2->last_affect;
      
obj2->first_affect = NULL;
      
obj2->last_affect = NULL;
   
}
   
obj->mana += obj2->mana;
   
obj->raw_mana += obj2->raw_mana;
   
      /*
       * Balancing: Weaken the effects on the ore based off weight
       * * instead of a random roll.
       */ 
      obj->mana /= div;
   
obj->raw_mana /= div;
   
obj_to_char( obj, ch );
   
extract_obj( obj2 );
   
if( obj->cost < 1 )
      obj->cost = 1;
   
obj->cost += obj->material->cost / obj->material->weight * obj->weight;
   
learn_from_success( ch, gsn_combine );

}

void do_spike( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA * obj = NULL;
   
OBJ_DATA * obj2 = NULL;
   
AFFECT_DATA * paf;
   
char arg[MAX_STRING_LENGTH];
   
char arg2[MAX_STRING_LENGTH];
   
extern int top_affect;
   
extern sh_int gsn_spike;
   
      /*
       * spike <obj> <ore> 
       */ 
      argument = one_argument( argument, arg );
   
argument = one_argument( argument, arg2 );
   
obj = get_obj_carry( ch, arg );
   
if( !obj )
   {
      
send_to_char( "Which piece of equipment do you want to add spikes to?\r\n", ch );
      
return;
   
}
   
separate_obj( obj );
   
obj2 = get_obj_carry( ch, arg2 );
   
if( !obj2 )
   {
      
obj_to_char( obj, ch );
      
send_to_char( "What raw material do you want to use to make the spikes?\r\n", ch );
      
return;
   
}
   
separate_obj( obj2 );
   
if( nifty_is_name( "spiked", obj->short_descr ) )
   {
      
send_to_char( "That item already has spikes.\r\n", ch );
      
return;
   
}
   
if( !nifty_is_name( "_material_", obj2->name ) )
   {
      
send_to_char( "You must use a raw material to make spikes with.\r\n", ch );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      
send_to_char( "You cannot seem to make the spikes stick to that.\r\n", ch );
      
return;
   
}
   
if( !IS_OBJ_STAT( obj2, ITEM_METAL ) )
   {
      
send_to_char( "You can only make spikes out of metal.\r\n", ch );
      
return;
   
}
   
if( obj->item_type != ITEM_ARMOR 
         &&( obj->item_type == ITEM_WEAPON 
             &&obj->value[3] != DAM_HIT 
             &&obj->value[3] != DAM_POUND 
 &&obj->value[3] != DAM_CRUSH 
 &&obj->value[3] != DAM_GREP 
 ) )
   {
      
send_to_char( "You can't add spikes to that.\r\n", ch );
      
return;
   
}
   
if( obj == obj2 )
   {
      
send_to_char( "You need two objects to add spikes to armor.\r\n", ch );
      
return;
   
}
   
obj_from_char( obj );
   
obj_from_char( obj2 );
   
      /*
       * Take away their ore if they fail their skill roll. Mwahaha! -- Scion 
       */ 
      if( !IS_NPC( ch ) && number_percent(  ) + skill_table[gsn_spike]->difficulty * 5 > LEARNED( ch, gsn_spike ) )
   {
      
act( AT_SKILL, "You cannot seem to form spikes from $p.", ch, obj, NULL, TO_CHAR );
      
act( AT_SKILL, "$n fails to add spikes to $p.", ch, obj, NULL, TO_ROOM );
      
learn_from_failure( ch, gsn_spike );
      
obj->weight++;   /* residue left over from botched spikes, no condition bonus */
      
obj_to_char( obj, ch );
      
extract_obj( obj2 );
      
return;
   
}
   
act( AT_SKILL, "You add spikes to $p.", ch, obj, NULL, TO_CHAR );
   
act( AT_SKILL, "$n adds spikes to $p.", ch, obj, NULL, TO_ROOM );
   
      /*
       * add +1 dr to obj and the word 'spiked' to the names 
       */ 
      
if( !nifty_is_name( "spiked", obj->name ) )
   {
      
char name_string[MAX_STRING_LENGTH];
      
strcpy( name_string, obj->name );
      
strcat( name_string, " spiked" );
      
STRFREE( obj->name );
      
obj->name = STRALLOC( name_string );
      
obj->short_descr = one_argument( obj->short_descr, name_string );
      
if( !strcmp( name_string, "some" ) )
         
strcpy( name_string, "some spiked " );
      
      else
         
strcpy( name_string, "spiked " );
      
strcat( name_string, obj->short_descr );
      
STRFREE( obj->short_descr );
      
obj->short_descr = STRALLOC( aoran( name_string ) );
      
obj->description = one_argument( obj->description, name_string );
      
if( !strcmp( name_string, "some" ) )
         
strcpy( name_string, "some spiked " );
      
      else
         
strcpy( name_string, "spiked " );
      
strcat( name_string, obj->description );
      
STRFREE( obj->description );
      
obj->description = STRALLOC( capitalize( aoran( name_string ) ) );
   
}
   
      /*
       * Transfer affects if skill rolls succeed and diff material types 
       */ 
      if( obj->material != NULL && obj2->material != NULL 
 &&obj->material != obj2->material )
   {
      
for( paf = obj2->first_affect; paf; paf = paf->next )
      {
         
if( number_percent(  ) < LEARNED( ch, gsn_combine ) )
         {
            
UNLINK( paf, obj2->first_affect, obj2->last_affect, next, prev );
            
LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         
}
      
}
   
}
   
      /*
       * Success: Add +1 dr affect and -1 dex 
       */ 
      if( number_percent(  ) + skill_table[gsn_spike]->difficulty * 5 <= LEARNED( ch, gsn_spike ) )
   {
      
bool found = FALSE;
      
         /*
          * Success: Add +1 dr affect 
          */ 
         for( paf = obj->first_affect; paf; paf = paf->next )
      {
         
if( paf->location == APPLY_DAMROLL && !found )
         {
            
paf->modifier++;
            
found = TRUE;
         
}
      
}
      
if( !found )
      {
         
CREATE( paf, AFFECT_DATA, 1 );
         
paf->type = -1;
         
paf->duration = -1;
         
paf->location = APPLY_DAMROLL;
         
paf->modifier = 1;
         
xCLEAR_BITS( paf->bitvector );
         
paf->next = NULL;
         
LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         
++top_affect;
      
}
      
found = FALSE;
      
for( paf = obj->first_affect; paf; paf = paf->next )
      {
         
if( paf->location == APPLY_DEX && !found )
         {
            
paf->modifier--;
            
if( paf->modifier == 0 )
            {
               
UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
               
DISPOSE( paf );
            
}
            
found = TRUE;
         
}
      
}
      
if( !found )
      {
         
CREATE( paf, AFFECT_DATA, 1 );
         
paf->type = -1;
         
paf->duration = -1;
         
paf->location = APPLY_DEX;
         
paf->modifier = -1;
         
xCLEAR_BITS( paf->bitvector );
         
paf->next = NULL;
         
LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         
++top_affect;
      
}
   
}
   
obj->weight++;  /* Always add to weight.. spikes are heavy :) */
   
obj_to_char( obj, ch );
   
extract_obj( obj2 );
   
learn_from_success( ch, gsn_spike );

}

void do_dye( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA * obj;
   
char arg[MAX_STRING_LENGTH];
   
char arg2[MAX_STRING_LENGTH];
   
char buf[MAX_STRING_LENGTH];
   
int color;
   
extern sh_int gsn_dye;
   
      /*
       * spike <obj> <ore> 
       */ 
      argument = one_argument( argument, arg );
   
argument = one_argument( argument, arg2 );
   
obj = get_obj_carry( ch, arg );
   
if( !obj )
   {
      
send_to_char( "Which piece of clothing do you want to change the color of?\r\n", ch );
      
return;
   
}
   
color = 0;
   
if( !strcmp( arg2, "red" ) )
      
color = 1;
   
   else if( !strcmp( arg2, "yellow" ) )
      
color = 2;
   
   else if( !strcmp( arg2, "blue" ) )
      
color = 3;
   
   else if( !strcmp( arg2, "black" ) )
      
color = 4;
   
   else if( !strcmp( arg2, "white" ) )
      
color = 5;
   
if( color == 0 )
   {
      
send_to_char( "Valid colors are: red, yellow, blue, black, and white.\r\n", ch );
      
return;
   
}
   
separate_obj( obj );
   
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      
send_to_char( "That's colorful enough as it is.\r\n", ch );
      
return;
   
}
   
if( !IS_OBJ_STAT( obj, ITEM_ORGANIC ) )
   {
      
send_to_char( "You can only dye cloth.\r\n", ch );
      
return;
   
}
   
      /*
       * Produce unpredictable results for a failed roll! -- Scion 
       */ 
      if( !IS_NPC( ch ) && number_percent(  ) + skill_table[gsn_dye]->difficulty * 5 > LEARNED( ch, gsn_dye ) )
   {
      
act( AT_SKILL, "You seem to have mixed the dyes incorrectly.", ch, obj, NULL, TO_CHAR );
      
act( AT_SKILL, "$n seems to have mixed $s dyes incorrectly.", ch, obj, NULL, TO_ROOM );
      
         /*
          * Get rid of the color codes in the object's description, if any 
          */ 
         strcpy( buf, munch_colors( obj->short_descr ) );
      
STRFREE( obj->short_descr );
      
obj->short_descr = STRALLOC( buf );
      
color = number_range( 1, 16 );
      
switch ( color )
      {
         
case 1:
            
strcpy( buf, "&z" );
            
break;
         
case 2:
            
strcpy( buf, "&g" );
            
break;
         
case 3:
            
strcpy( buf, "&b" );
            
break;
         
case 4:
            
strcpy( buf, "&c" );
            
break;
         
case 5:
            
strcpy( buf, "&z" );
            
break;
         
case 6:
            
strcpy( buf, "&G" );
            
break;
         
case 7:
            
strcpy( buf, "&B" );
            
break;
         
case 8:
            
strcpy( buf, "&C" );
            
break;
         
case 9:
            
strcpy( buf, "&r" );
            
break;
         
case 10:
            
strcpy( buf, "&O" );
            
break;
         
case 11:
            
strcpy( buf, "&p" );
            
break;
         
case 12:
            
strcpy( buf, "&w" );
            
break;
         
case 13:
            
strcpy( buf, "&R" );
            
break;
         
case 14:
            
strcpy( buf, "&Y" );
            
break;
         
case 15:
            
strcpy( buf, "&P" );
            
break;
         
case 16:
            
strcpy( buf, "&W" );
            
break;
      
}
      
strcat( buf, obj->short_descr );
      
strcat( buf, "&w" );
      
STRFREE( obj->short_descr );
      
obj->short_descr = STRALLOC( buf );
      
learn_from_failure( ch, gsn_dye );
      
return;
   
}
   
act( AT_SKILL, "You dye $p.", ch, obj, NULL, TO_CHAR );
   
act( AT_SKILL, "$n dyes $p.", ch, obj, NULL, TO_ROOM );
   
      /*
       * If there's no color code already, just add one. Otherwise mix the colors!! 
       */ 
      if( obj->short_descr[0] != '&' )
   {
      
         /*
          * Get rid of the color codes in the object's description, if any 
          */ 
         strcpy( buf, munch_colors( obj->short_descr ) );
      
STRFREE( obj->short_descr );
      
obj->short_descr = STRALLOC( buf );
      
switch ( color )
      {
         
case 1:
            
strcpy( buf, "&R" );
            
break;
         
case 2:
            
strcpy( buf, "&Y" );
            
break;
         
case 3:
            
strcpy( buf, "&B" );
            
break;
         
case 4:
            
strcpy( buf, "&z" );
            
break;
         
case 5:
            
strcpy( buf, "&W" );
            
break;
      
}
      
strcat( buf, obj->short_descr );
      
strcat( buf, "&w" );
      
STRFREE( obj->short_descr );
      
obj->short_descr = STRALLOC( buf );
   
}
   else
   {
      
/*
  &x  - Black                     &r  - Red (blood)
  &g  - Green                     &O  - Orange (brown)
  &b  - Dark Blue                 &p  - Purple
  &c  - Cyan                      &w  - Gray (default color)
  &z  - Dark Grey                 &R  - Light Red
  &G  - Light Green               &Y  - Yellow
  &B  - Blue                      &P  - Pink
  &C  - Light Blue                &W  - White
*/ 
         if( obj->short_descr[1] == 'g' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'O';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'G';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'O';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'G';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'b' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'p';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'B';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'b';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'B';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'c' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'P';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'G';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'C';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'b';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'C';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'z' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'r';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'O';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'b';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'w';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'G' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'O';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'Y';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'C';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'g';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'G';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'B' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'p';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'G';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'b';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'b';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'C';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'C' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'P';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'G';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'B';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'b';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'C';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'r' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'r';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'R';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'p';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'r';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'R';
               break;
         
}
      
}
      
if( obj->short_descr[1] == 'O' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'O';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'O';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'O';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'Y';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'p' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'r';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'P';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'p';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'P';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'w' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'r';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'O';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'b';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'w';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'R' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'r';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'O';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'p';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'r';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'P';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'Y' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'O';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'Y';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'G';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'O';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'Y';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'P' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'O';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'G';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'O';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'G';
               break;
         
}
      
}
      
      else if( obj->short_descr[1] == 'W' )
      {
         
switch ( color )
         {
            
case 1:   /* red */
               
obj->short_descr[1] = 'R';
               break;
            
case 2:   /* yellow */
               
obj->short_descr[1] = 'Y';
               break;
            
case 3:   /* blue */
               
obj->short_descr[1] = 'B';
               break;
            
case 4:   /* black */
               
obj->short_descr[1] = 'z';
               break;
            
case 5:   /* white */
               
obj->short_descr[1] = 'W';
               break;
         
}
      
}
   
}
   
learn_from_success( ch, gsn_dye );

}

void do_affix( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA * obj;
   
OBJ_DATA * gem;
   
char arg[MAX_STRING_LENGTH];
   
extern sh_int gsn_affix;
   
if( !can_use_bodypart( ch, BP_RHAND ) )
   {
      
send_to_char( "That would be hard without functional hands.\n\r", ch );
      
return;
   
}
   
      /*
       * affix <gem> <obj> 
       */ 
      argument = one_argument( argument, arg );
   
gem = get_obj_carry( ch, arg );
   
if( !gem )
   {
      
send_to_char( "What gem do you want to affix?\r\n", ch );
      
return;
   
}
   
separate_obj( gem );
   
argument = one_argument( argument, arg );
   
obj = get_obj_carry( ch, arg );
   
if( !obj || obj == gem )
   {
      
send_to_char( "What object do you want to affix gems to?\r\n", ch );
      
return;
   
}
   
separate_obj( obj );
   
if( !IS_OBJ_STAT( obj, ITEM_METAL ) )
   {
      
send_to_char( "You can only attach gems to metal items.\r\n", ch );
      
return;
   
}
   
if( obj->gem )
   {
      
send_to_char( "That object already has a gem.\n\r", ch );
      
return;
   
}
   
if( !IS_OBJ_STAT( gem, ITEM_GEM ) )
   {
      
send_to_char( "You can only affix gems.\r\n", ch );
      
return;
   
}
   
if( IS_OBJ_STAT( gem, ITEM_DREAMWORLD ) 
 &&!IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
   {
      
send_to_char( "Your gem is insubstantial!\n\r", ch );
      
return;
   
}
   
if( obj->item_type == ITEM_TREASURE )
   {
      
send_to_char( "You can only affix gems to the finished product.\n\r", ch );
      
return;
   
}
   
if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      
send_to_char( "The gem won't stick to that artifact.\r\n", ch );
      
return;
   
}
   
obj_from_char( gem );
   
obj_from_char( obj );
   
learn_noncombat( ch, SK_JEWEL );
   
      /*
       * Damage the object if they fail their skill roll. Mwahaha! -- Scion 
       */ 
      if( number_range( 1, 20 ) > LEARNED( ch, gsn_affix ) )
   {
      
act( AT_SKILL, "The gem breaks!", ch, NULL, NULL, TO_CHAR );
      
act( AT_SKILL, "$n tries to affix a gem to $p, but it breaks.", ch, obj, NULL, TO_ROOM );
      
damage_obj( obj );
      
if( !obj_extracted( obj ) )
         
obj_to_char( obj, ch );
      
extract_obj( gem );
      
learn_from_failure( ch, gsn_affix );
      
return;
   
}
   
act( AT_SKILL, "You affix a gem to $p.", ch, obj, NULL, TO_CHAR );
   
act( AT_SKILL, "$n affixes a gem to $p.", ch, obj, NULL, TO_ROOM );
   
obj->gem = gem;
   
gem->in_obj = obj;
   
obj->cost += gem->cost;
   
obj_to_char( obj, ch );
   
learn_from_success( ch, gsn_affix );

}

void do_pry( CHAR_DATA * ch, char *argument )
{
   
OBJ_DATA * obj;
   
OBJ_DATA * gem;
   
obj = get_obj_carry( ch, argument );
   
if( !obj )
   {
      
send_to_char( "You have nothing like that.\n\r", ch );
      
return;
   
}
   
if( !obj->gem )
   {
      
send_to_char( "That object has no gem.\n\r", ch );
      
return;
   
}
   
gem = obj->gem;
   
obj->gem = NULL;
   
gem->in_obj = NULL;
   
obj->cost -= gem->cost;
   
obj_to_char( gem, ch );
   
learn_noncombat( ch, SK_JEWEL );
   
act( AT_SKILL, "You pry $p loose from $P.", ch, gem, obj, TO_CHAR );
   
act( AT_SKILL, "$n pries $p loose from $P.", ch, gem, obj, TO_ROOM );

}


/* Return the best talent for a char to use for a given sn */ 
int get_best_talent( CHAR_DATA * ch, int sn )
{
   
int i, best;
   
best = -1;
   
for( i = 0; i < MAX_DEITY; i++ )
   {
      
if( ch->curr_talent[i] >= skill_table[sn]->skill_level[i] )
      {
         
if( ch->curr_talent[i] - skill_table[sn]->skill_level[i] > best )
            
best = i;
      
}
   
}
   
return best;

}


bool skill_available( CHAR_DATA * ch, int sn )
{
   
int talent;
   
if( sn < 0 )
      
return FALSE;
   
talent = get_best_talent( ch, sn );
   
if( ( skill_table[sn]->type != SKILL_SPELL 
 &&( skill_table[sn]->race_level[ch->race] == 1 /* innate */  
                                                      ||skill_table[sn]->race_level[ch->race] == 2 ) /* can learn */  
          ) 
         ||( skill_table[sn]->type == SKILL_SPELL && ( talent > -1 )
             && ( skill_table[sn]->skill_level[talent] <= ch->curr_talent[talent] ) ) 
 )
      
return TRUE;
   
   else
      
return FALSE;

}


/* Trains another player up to 1/4 of your percentage in a skill -- Scion 
	train <victim> <skill>
*/ 
void do_train( CHAR_DATA * ch, char *argument )
{
   
char arg1[MAX_INPUT_LENGTH];
   
char arg2[MAX_INPUT_LENGTH];
   
char buf[MAX_STRING_LENGTH];
   
CHAR_DATA * victim;
   
int sn;
   
argument = one_argument( argument, arg1 );
   
argument = one_argument( argument, arg2 );
   
      /*
       * Find victim 
       */ 
      if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      
send_to_char( "They're not here.\r\n", ch );
      
return;
   
}
   
      /*
       * Look up skill 
       */ 
      if( ( sn = bsearch_skill_prefix( arg2, gsn_first_spell, gsn_first_skill - 1 ) ) < 0 )
   {
      
if( ( sn = bsearch_skill_prefix( arg2, gsn_first_skill, gsn_top_sn - 1 ) ) < 0 )
      {
         
send_to_char( "That is not a skill or a spell.\r\n", ch );
         
return;
      
}
   
}
   
      /*
       * Does ch have skill, and does victim -not- have skill? 
       */ 
      if( LEARNED( ch, sn ) < 1 )
   {
      
send_to_char( "You do not know that skill.\r\n", ch );
      
return;
   
}
   
if( LEARNED( victim, sn ) > 0 )
   {
      
send_to_char( "They already know that skill.\r\n", ch );
      
return;
   
}
   
      /*
       * Can victim actually have this skill? 
       */ 
      if( skill_table[sn]->type == SKILL_SPELL )
   {
      
if( get_best_talent( ch, sn ) == -1 || skill_table[sn]->race_level[victim->race] == 3 )
      {
         
send_to_char( "They are physically unable to do that.\r\n", ch );
         
return;
      
}
   
}
   
      /*
       * Give 1 to 1/4 LEARNED(ch, sn) to victim 
       */ 
      victim->pcdata->learned[sn] = number_range( 1, ( int )LEARNED( ch, sn ) / 4 );
   
      /*
       * Lower ch's mentalstate 
       */ 
      worsen_mental_state( ch, number_range( 1, 25 ) );
   
      /*
       * Print messages 
       */ 
      sprintf( buf, "You teach $N the art of %s.", skill_table[sn]->name );
   
act( AT_SKILL, buf, ch, NULL, victim, TO_CHAR );
   
sprintf( buf, "$n teaches you the art of %s.", skill_table[sn]->name );
   
act( AT_SKILL, buf, ch, NULL, victim, TO_VICT );
   
sprintf( buf, "$n teaches $N the art of %s.", skill_table[sn]->name );
   
act( AT_SKILL, buf, ch, NULL, victim, TO_NOTVICT );
   
send_to_char( "You are a bit tired after the long lesson.\r\n", ch );

} 
int learned( CHAR_DATA * ch, int sn )
{
   
if( IS_NPC( ch ) )
      
return 80;
   
if( sn < 0 )
      
return 0;
   
if( skill_available( ch, sn ) == FALSE )
      
return 0;
   
if( skill_table[sn]->type == SKILL_SPELL )
      
return URANGE( 0, ch->pcdata->learned[sn], 100 );
   
   else
      
switch ( skill_table[sn]->race_level[ch->race] )
      {
         
case 1:
            
return 100;
            
break;
         
case 2:
            
return URANGE( 0, ch->pcdata->learned[sn], 100 );
            
break;
         
default:
            
return 0;
      
}

}

void do_consent( CHAR_DATA * ch, char *argument )
{
   
CHAR_DATA * victim;
   
if( IS_NPC( ch ) )
   {
      
send_to_char( "Mobs can't consent.\n\r", ch );
      
return;
   
}
   
if( ( victim = get_char_world( ch, argument ) ) == NULL )
      
   {
      
send_to_char( "They aren't here.\n\r", ch );
      
return;
   
}
   
if( IS_NPC( victim ) )
   {
      
send_to_char( "They are not worthy of your consent.\n\r", ch );
      
return;
   
}
   
if( ch->pcdata->consenting && ch->pcdata->consenting != NULL )
   {
      
act( AT_PLAIN, "$n no longer consents to you.", 
ch, NULL, ch->pcdata->consenting, TO_VICT );
      
act( AT_PLAIN, "You no longer consent to $N.", 
ch, NULL, ch->pcdata->consenting, TO_CHAR );
      
ch->pcdata->consenting = NULL;
   
}
   
ch->pcdata->consenting = victim;
   
act( AT_PLAIN, "$n has given you $s consent.", 
ch, NULL, victim, TO_VICT );
   
act( AT_PLAIN, "You give your consent to $N.", 
ch, NULL, victim, TO_CHAR );

}


