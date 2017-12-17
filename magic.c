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
 *			     Spell handling module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
#include <strings.h>
#endif /*  */
#include <time.h>
#include "mud.h"
   
/*
 * Local functions.
 */ 
void say_spell args( ( CHAR_DATA * ch, int sn ) );
ch_ret spell_affect args( ( int sn, int wil, CHAR_DATA * ch, void *vo ) );
ch_ret spell_affectchar args( ( int sn, int wil, CHAR_DATA * ch, void *vo ) );
int dispel_casting( AFFECT_DATA * paf, CHAR_DATA * ch, CHAR_DATA * victim, int affect, bool dispel );
void check_bodyparts( CHAR_DATA * ch );
bool check_counterspell( CHAR_DATA * ch, CHAR_DATA * victim );
extern SPECIES_DATA *find_species( char *name );
extern NATION_DATA *find_nation( char *name );
extern void learn_talent( CHAR_DATA * ch, int i );

/*
 * Is immune to a damage type
 */ 
   bool is_immune( CHAR_DATA * ch, sh_int damtype ) 
{
   switch ( damtype )
      
   {
      case SD_FIRE:
         if( IS_SET( ch->immune, RIS_FIRE ) )
            return TRUE;
      case SD_COLD:
         if( IS_SET( ch->immune, RIS_COLD ) )
            return TRUE;
      case SD_ELECTRICITY:
         if( IS_SET( ch->immune, RIS_ELECTRICITY ) )
            return TRUE;
      case SD_ENERGY:
         if( IS_SET( ch->immune, RIS_ENERGY ) )
            return TRUE;
      case SD_ACID:
         if( IS_SET( ch->immune, RIS_ACID ) )
            return TRUE;
      case SD_POISON:
         if( IS_SET( ch->immune, RIS_POISON ) )
            return TRUE;
      case SD_DRAIN:
         if( IS_SET( ch->immune, RIS_DRAIN ) )
            return TRUE;
   }
   return FALSE;
}


/*
 * Lookup a skill by name, only stopping at skills the player has.
 */ 
int ch_slookup( CHAR_DATA * ch, const char *name ) 
{
   int sn;
   if( IS_NPC( ch ) )
      return skill_lookup( name );
   for( sn = 0; sn < top_sn; sn++ )
      
   {
      if( !skill_table[sn]->name )
         break;
      if( LEARNED( ch, sn ) > 0 
           &&get_best_talent( ch, sn ) != -1 
           &&LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )  &&!str_prefix( name, skill_table[sn]->name ) )
         return sn;
   }
   return -1;
}


/*
 * Lookup an herb by name.
 */ 
int herb_lookup( const char *name ) 
{
   int sn;
   for( sn = 0; sn < top_herb; sn++ )
      
   {
      if( !herb_table[sn] || !herb_table[sn]->name )
         return -1;
      if( LOWER( name[0] ) == LOWER( herb_table[sn]->name[0] )  &&!str_prefix( name, herb_table[sn]->name ) )
         return sn;
   }
   return -1;
}


/*
 * Lookup a skill by name.
 */ 
int skill_lookup( const char *name ) 
{
   int sn;
   if( ( sn = bsearch_skill_exact( name, gsn_first_spell, gsn_first_skill - 1 ) ) == -1 )
      if( ( sn = bsearch_skill_exact( name, gsn_first_skill, gsn_top_sn - 1 ) ) == -1 )
         if( ( sn = bsearch_skill_prefix( name, gsn_first_spell, gsn_first_skill - 1 ) ) == -1 )
            if( ( sn = bsearch_skill_prefix( name, gsn_first_skill, gsn_top_sn - 1 ) ) == -1 )
               
            {
               for( sn = gsn_top_sn; sn < top_sn; sn++ )
                  
               {
                  if( !skill_table[sn] || !skill_table[sn]->name )
                     return -1;
                  if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )  &&!str_prefix( name, skill_table[sn]->name ) )
                     return sn;
               }
               return -1;
            }
   return sn;
}


/*
 * Return a skilltype pointer based on sn			-Thoric
 * Returns NULL if bad or unused
 */ 
   SKILLTYPE * get_skilltype( int sn )
{
   if( sn >= TYPE_HERB )
      return IS_VALID_HERB( sn - TYPE_HERB ) ? herb_table[sn - TYPE_HERB] : NULL;
   if( sn >= TYPE_HIT )
      return NULL;
   return IS_VALID_SN( sn ) ? skill_table[sn] : NULL;
}


/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for prefix matches
 */ 
int bsearch_skill_prefix( const char *name, int first, int top ) 
{
   int sn;
   for( ;; )
      
   {
      sn = ( first + top ) >> 1;
      if( sn < 0 )
         return -1;
      if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )  &&!str_prefix( name, skill_table[sn]->name ) )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      
      else
         first = sn + 1;
   }
   return -1;
}


/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for exact matches only
 */ 
int bsearch_skill_exact( const char *name, int first, int top ) 
{
   int sn;
   for( ;; )
      
   {
      sn = ( first + top ) >> 1;
      if( sn < 0 )
         return -1;
      if( !str_cmp( name, skill_table[sn]->name ) )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      
      else
         first = sn + 1;
   }
   return -1;
}


/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check exact match first, then a prefix match
 */ 
int bsearch_skill( const char *name, int first, int top ) 
{
   int sn = bsearch_skill_exact( name, first, top );
   return ( sn == -1 ) ? bsearch_skill_prefix( name, first, top ) : sn;
}


/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */ 
int ch_bsearch_skill_prefix( CHAR_DATA * ch, const char *name, int first, int top ) 
{
   int sn;
   for( ;; )
      
   {
      sn = ( first + top ) >> 1;
      if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) 
            &&!str_prefix( name, skill_table[sn]->name )  &&LEARNED( ch, sn ) > 0  &&get_best_talent( ch, sn ) != -1 )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      
      else
         first = sn + 1;
   }
   return -1;
}
int ch_bsearch_skill_exact( CHAR_DATA * ch, const char *name, int first, int top ) 
{
   int sn;
   for( ;; )
      
   {
      sn = ( first + top ) >> 1;
      if( !str_cmp( name, skill_table[sn]->name )  &&LEARNED( ch, sn ) > 0  &&get_best_talent( ch, sn ) != -1 )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      
      else
         first = sn + 1;
   }
   return -1;
}
int ch_bsearch_skill( CHAR_DATA * ch, const char *name, int first, int top ) 
{
   int sn = ch_bsearch_skill_exact( ch, name, first, top );
   return ( sn == -1 ) ? ch_bsearch_skill_prefix( ch, name, first, top ) : sn;
}
int find_spell( CHAR_DATA * ch, const char *name, bool know ) 
{
   if( IS_NPC( ch ) || !know )
      return bsearch_skill( name, gsn_first_spell, gsn_first_skill - 1 );
   
   else
      return ch_bsearch_skill( ch, name, gsn_first_spell, gsn_first_skill - 1 );
}
int find_skill( CHAR_DATA * ch, const char *name, bool know ) 
{
   if( IS_NPC( ch ) || !know )
      return bsearch_skill( name, gsn_first_skill, gsn_top_sn - 1 );
   
   else
      return ch_bsearch_skill( ch, name, gsn_first_skill, gsn_top_sn - 1 );
}


/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */ 
int slot_lookup( int slot ) 
{
   extern bool fBootDb;
   int sn;
   if( slot <= 0 )
      return -1;
   for( sn = 0; sn < top_sn; sn++ )
      if( slot == skill_table[sn]->slot )
         return sn;
   if( fBootDb )
      
   {
      bug( "Slot_lookup: bad slot %d.", slot );
      
/*	abort( ); */ 
   }
   return -1;
}


/*
 * Fancy message handling for a successful casting		-Thoric
 */ 
void successful_casting( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj ) 
{
   sh_int chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
   sh_int chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
   sh_int chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );
   if( skill->target != TAR_CHAR_OFFENSIVE )
      
   {
      chit = chitroom;
      chitme = chitroom;
   }
   if( ch && ch != victim )
      
   {
      if( skill->hit_char && skill->hit_char[0] != '\0' )
         act( chit, skill->hit_char, ch, obj, victim, TO_CHAR );
      
      else
      if( skill->type == SKILL_SPELL )
         act( chit, "Ok.", ch, NULL, NULL, TO_CHAR );
   }
   if( ch && skill->hit_room && skill->hit_room[0] != '\0' )
      act( chitroom, skill->hit_room, ch, obj, victim, TO_NOTVICT );
   if( ch && victim && skill->hit_vict && skill->hit_vict[0] != '\0' )
      
   {
      if( ch != victim )
         act( chitme, skill->hit_vict, ch, obj, victim, TO_VICT );
      
      else
         act( chitme, skill->hit_vict, ch, obj, victim, TO_CHAR );
   }
   
   else
   if( ch && ch == victim && skill->type == SKILL_SPELL )
      act( chitme, "Ok.", ch, NULL, NULL, TO_CHAR );
}


/*
 * Fancy message handling for a failed casting			-Thoric
 */ 
void failed_casting( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj ) 
{
   sh_int chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
   sh_int chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
   sh_int chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );
   if( skill->target != TAR_CHAR_OFFENSIVE )
      
   {
      chit = chitroom;
      chitme = chitroom;
   }
   if( ch && ch != victim )
      
   {
      if( skill->miss_char && skill->miss_char[0] != '\0' )
         act( chit, skill->miss_char, ch, obj, victim, TO_CHAR );
      
      else
      if( skill->type == SKILL_SPELL )
         act( chit, "You failed.", ch, NULL, NULL, TO_CHAR );
   }
   if( ch && skill->miss_room && skill->miss_room[0] != '\0'  &&str_cmp( skill->miss_room, "supress" ) )
      act( chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT );
   if( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
      
   {
      if( ch != victim )
         act( chitme, skill->miss_vict, ch, obj, victim, TO_VICT );
      
      else
         act( chitme, skill->miss_vict, ch, obj, victim, TO_CHAR );
   }
   
   else
   if( ch && ch == victim )
      
   {
      if( skill->miss_char && skill->miss_char[0] != '\0' )
         act( chitme, skill->miss_char, ch, obj, victim, TO_CHAR );
      
      else
      if( skill->type == SKILL_SPELL )
         act( chitme, "You failed.", ch, NULL, NULL, TO_CHAR );
   }
}


/*
 * Fancy message handling for being immune to something		-Thoric
 */ 
void immune_casting( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj ) 
{
   sh_int chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
   sh_int chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
   sh_int chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );
   if( skill->target != TAR_CHAR_OFFENSIVE )
      
   {
      chit = chitroom;
      chitme = chitroom;
   }
   if( ch && ch != victim )
      
   {
      if( skill->imm_char && skill->imm_char[0] != '\0' )
         act( chit, skill->imm_char, ch, obj, victim, TO_CHAR );
      
      else
      if( skill->miss_char && skill->miss_char[0] != '\0' )
         act( chit, skill->hit_char, ch, obj, victim, TO_CHAR );
      
      else
      if( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
         act( chit, "That appears to have no effect.", ch, NULL, NULL, TO_CHAR );
   }
   if( ch && skill->imm_room && skill->imm_room[0] != '\0' )
      act( chitroom, skill->imm_room, ch, obj, victim, TO_NOTVICT );
   
   else
   if( ch && skill->miss_room && skill->miss_room[0] != '\0' )
      act( chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT );
   if( ch && victim && skill->imm_vict && skill->imm_vict[0] != '\0' )
      
   {
      if( ch != victim )
         act( chitme, skill->imm_vict, ch, obj, victim, TO_VICT );
      
      else
         act( chitme, skill->imm_vict, ch, obj, victim, TO_CHAR );
   }
   
   else
   if( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
      
   {
      if( ch != victim )
         act( chitme, skill->miss_vict, ch, obj, victim, TO_VICT );
      
      else
         act( chitme, skill->miss_vict, ch, obj, victim, TO_CHAR );
   }
   
   else
   if( ch && ch == victim )
      
   {
      if( skill->imm_char && skill->imm_char[0] != '\0' )
         act( chit, skill->imm_char, ch, obj, victim, TO_CHAR );
      
      else
      if( skill->miss_char && skill->miss_char[0] != '\0' )
         act( chit, skill->hit_char, ch, obj, victim, TO_CHAR );
      
      else
      if( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
         act( chit, "That appears to have no affect.", ch, NULL, NULL, TO_CHAR );
   }
}


/*
 * Utter mystical words for an sn.
 */ 
void say_spell( CHAR_DATA * ch, int sn ) 
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   CHAR_DATA * rch;
   char *pName;
   int iSyl;
   int length;
   SKILLTYPE * skill = get_skilltype( sn );
   struct syl_type 
   {
      char *old;
       char *new;
    };
   static const struct syl_type syl_table[] = 
      { 
{" ", " "}, 
{"ar", "kata"}, 
{"fi", "kh"}, 
{"ice", "nyst"}, 
{"ing", "nay"}, 
{"el", "r"}, 
{"ld", "ck"}, 
{"light", "khar"}, 
{"ne", "lyx"}, 
{"ow", "er"}, 
{"re", "an"}, 
{"ru", "tanta"}, 
{"tra", "ill"}, 
{"sha", "se"}, 
{"shi", "er"}, 
{"a", "o"}, {"b", "b"}, {"c", "q"}, {"d", "dh"}, 
{"e", "y"}, {"f", "z"}, {"g", "o"}, {"h", "p"}, 
{"i", "u"}, {"j", "n"}, {"k", "t"}, {"l", "r"}, 
{"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"}, 
{"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "j"}, {"v", "y"}, {"w", "r"}, {"x", "n"}, {"y", "i"}, {"z", "k"}, {"", ""} 
   };
   buf[0] = '\0';
   for( pName = skill->name; *pName != '\0'; pName += length )
      
   {
      for( iSyl = 0; ( length = strlen( syl_table[iSyl].old ) ) != 0; iSyl++ )
         
      {
         if( !str_prefix( syl_table[iSyl].old, pName ) )
            
         {
            strcat( buf, syl_table[iSyl].new );
            break;
         }
      }
      if( length == 0 )
         length = 1;
   }
   sprintf( buf2, "$n closes $s eyes and utters the words, '%s!'", buf );
   sprintf( buf, "$n closes $s eyes and utters the words, '%s!'", skill->name );
   
      /*
       * by saying the spell out loud, you give yourself away -keo 
       */ 
      if( IS_AFFECTED( ch, AFF_HIDE ) )
      xREMOVE_BIT( ch->affected_by, AFF_HIDE );
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
      
   {
      if( rch != ch )
         act( AT_MAGIC, buf2, ch, NULL, rch, TO_VICT );
   }
   return;
}


/*
 * Make adjustments to saving throw based in RIS		-Thoric
 */ 
int ris_save( CHAR_DATA * ch, int chance, int ris ) 
{
   sh_int modifier;
   modifier = 10;
   if( IS_SET( ch->immune, ris ) )
      modifier -= 10;
   if( IS_SET( ch->resistant, ris ) )
      modifier -= 2;
   if( IS_SET( ch->susceptible, ris ) )
      modifier += 2;
   if( modifier <= 0 )
      return 1000;
   if( modifier == 10 )
      return chance;
   return ( chance * modifier ) / 10;
}


/*								    -Thoric
 * Fancy dice expression parsing complete with order of operations,
 * simple exponent support, dice support as well as a few extra
 * variables: H = hp, M = mana, V = move, S = str, X = dex
 *            I = int, W = wil, C = con, P = per, E = end, A = age
 *
 * Used for spell dice parsing, ie: 3d8+W-6
 *
 */ 
int rd_parse( CHAR_DATA * ch, char *exp ) 
{
   int x, lop = 0, gop = 0, eop = 0;
   char operation;
   char *sexp[2];
   int total = 0, len = 0;
   
      /*
       * take care of nulls coming in 
       */ 
      if( !exp || !strlen( exp ) )
      return 0;
   
      /*
       * get rid of brackets if they surround the entire expresion 
       */ 
      if( ( *exp == '(' ) && !index( exp + 1, '(' ) && exp[strlen( exp ) - 1] == ')' )
      
   {
      exp[strlen( exp ) - 1] = '\0';
      exp++;
   }
   
      /*
       * check if the expresion is just a number 
       */ 
      len = strlen( exp );
   if( len == 1 && isalpha( exp[0] ) )
      
   {
      switch ( exp[0] )
         
      {
         case 'H':
         case 'h':
            return ch->hit;
         case 'M':
         case 'm':
            return ch->mana;
         case 'V':
         case 'v':
            return ch->move;
         case 'S':
         case 's':
            return get_curr_str( ch );
         case 'I':
         case 'i':
            return get_curr_int( ch );
         case 'W':
         case 'w':
            return get_curr_wil( ch );
         case 'X':
         case 'x':
            return get_curr_dex( ch );
         case 'C':
         case 'c':
            return get_curr_con( ch );
         case 'P':
         case 'p':
            return get_curr_per( ch );
         case 'E':
         case 'e':
            return get_curr_end( ch );
      }
   }
   for( x = 0; x < len; ++x )
      if( !isdigit( exp[x] ) && !isspace( exp[x] ) )
         break;
   if( x == len )
      return atoi( exp );
   
      /*
       * break it into 2 parts 
       */ 
      for( x = 0; x < strlen( exp ); ++x )
      switch ( exp[x] )
         
      {
         case '^':
            if( !total )
               eop = x;
            break;
         case '-':
         case '+':
            if( !total )
               lop = x;
            break;
         case '*':
         case '/':
         case '%':
         case 'd':
         case 'D':
         case '<':
         case '>':
         case '{':
         case '}':
         case '=':
            if( !total )
               gop = x;
            break;
         case '(':
            ++total;
            break;
         case ')':
            --total;
            break;
      }
   if( lop )
      x = lop;
   
   else
   if( gop )
      x = gop;
   
   else
      x = eop;
   operation = exp[x];
   exp[x] = '\0';
   sexp[0] = exp;
   sexp[1] = ( char * )( exp + x + 1 );
   
      /*
       * work it out 
       */ 
      total = rd_parse( ch, sexp[0] );
   switch ( operation )
      
   {
      case '-':
         total -= rd_parse( ch, sexp[1] );
         break;
      case '+':
         total += rd_parse( ch, sexp[1] );
         break;
      case '*':
         total *= rd_parse( ch, sexp[1] );
         break;
      case '/':
         total /= rd_parse( ch, sexp[1] );
         break;
      case '%':
         total %= rd_parse( ch, sexp[1] );
         break;
      case 'd':
      case 'D':
         total = dice( total, rd_parse( ch, sexp[1] ) );
         break;
      case '<':
         total = ( total < rd_parse( ch, sexp[1] ) );
         break;
      case '>':
         total = ( total > rd_parse( ch, sexp[1] ) );
         break;
      case '=':
         total = ( total == rd_parse( ch, sexp[1] ) );
         break;
      case '{':
         total = UMIN( total, rd_parse( ch, sexp[1] ) );
         break;
      case '}':
         total = UMAX( total, rd_parse( ch, sexp[1] ) );
         break;
      case '^':
         
      {
         int y = rd_parse( ch, sexp[1] ), z = total;
         for( x = 1; x < y; ++x, z *= total );
         total = z;
         break;
      }
   }
   return total;
}


/* wrapper function so as not to destroy exp */ 
int dice_parse( CHAR_DATA * ch, char *exp ) 
{
   char buf[MAX_INPUT_LENGTH];
   strcpy( buf, exp );
   return rd_parse( ch, buf );
}


/*
 * Process the spell's required components, if any		-Thoric
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @@ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */ 
   bool process_spell_components( CHAR_DATA * ch, int sn )
{
   SKILLTYPE * skill = get_skilltype( sn );
   char *comp = skill->components;
   char *check;
   char arg[MAX_INPUT_LENGTH];
   bool consume, fail, found;
   int val, value;
   OBJ_DATA * obj;
   
      /*
       * if no components necessary, then everything is cool 
       */ 
      if( !comp || comp[0] == '\0' )
      return TRUE;
   while( comp[0] != '\0' )
      
   {
      comp = one_argument( comp, arg );
      consume = TRUE;
      fail = found = FALSE;
      val = -1;
      switch ( arg[1] )
         
      {
         default:
            check = arg + 1;
            break;
         case '!':
            check = arg + 2;
            fail = TRUE;
            break;
         case '+':
            check = arg + 2;
            consume = FALSE;
            break;
         case '@':
            check = arg + 2;
            val = 0;
            break;
         case '#':
            check = arg + 2;
            val = 1;
            break;
         case '$':
            check = arg + 2;
            val = 2;
            break;
         case '%':
            check = arg + 2;
            val = 3;
            break;
         case '^':
            check = arg + 2;
            val = 4;
            break;
         case '&':
            check = arg + 2;
            val = 5;
            break;
            
               /*
                * reserve '*', '(' and ')' for v6, v7 and v8   
                */ 
      }
      value = atoi( check );
      obj = NULL;
      switch ( UPPER( arg[0] ) )
         
      {
         case 'T':
            for( obj = ch->first_carrying; obj; obj = obj->next_content )
               if( obj->item_type == value )
                  
               {
                  if( fail )
                     
                  {
                     send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'V':
            for( obj = ch->first_carrying; obj; obj = obj->next_content )
               if( obj->pIndexData->vnum == value )
                  
               {
                  if( fail )
                     
                  {
                     send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'K':
            for( obj = ch->first_carrying; obj; obj = obj->next_content )
               if( nifty_is_name( check, obj->name ) )
                  
               {
                  if( fail )
                     
                  {
                     send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'G':
            if( ch->gold >= value )
            {
               if( fail )
                  
               {
                  send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                  return FALSE;
               }
               
               else
                  
               {
                  if( consume )
                     
                  {
                     set_char_color( AT_GOLD, ch );
                     send_to_char( "You feel a little lighter...\n\r", ch );
                     ch->gold -= value;
                  }
                  continue;
               }
               break;
            }
         case 'H':
            if( ch->hit >= value )
            {
               if( fail )
                  
               {
                  send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                  return FALSE;
               }
               
               else
                  
               {
                  if( consume )
                     
                  {
                     set_char_color( AT_BLOOD, ch );
                     send_to_char( "You feel a little weaker...\n\r", ch );
                     ch->hit -= value;
                     update_pos( ch );
                  }
                  continue;
               }
               break;
            }
      }
      
         /*
          * having this component would make the spell fail... if we get
          * here, then the caster didn't have that component 
          */ 
         if( fail )
         continue;
      if( !found )
         
      {
         send_to_char( "Something is missing...\n\r", ch );
         return FALSE;
      }
      if( obj )
         
      {
         if( val >= 0 && val < 6 )
            
         {
            separate_obj( obj );
            if( obj->value[val] <= 0 )
               
            {
               act( AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
               act( AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
               extract_obj( obj );
               return FALSE;
            }
            
            else
            if( --obj->value[val] == 0 )
               
            {
               act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
               act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
               extract_obj( obj );
            }
            
            else
               act( AT_MAGIC, "$p glows briefly and a whisp of smoke rises from it.", ch, obj, NULL, TO_CHAR );
         }
         
         else
         if( consume )
            
         {
            separate_obj( obj );
            act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
            act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
            extract_obj( obj );
         }
         
         else
            
         {
            int count = obj->count;
            obj->count = 1;
            act( AT_MAGIC, "$p glows briefly.", ch, obj, NULL, TO_CHAR );
            obj->count = count;
      } }
   } return TRUE;
}
int pAbort;

/*
 * Locate targets.
 */ 
void *locate_targets( CHAR_DATA * ch, char *arg, int sn, CHAR_DATA ** victim, OBJ_DATA ** obj ) 
{
   SKILLTYPE * skill = get_skilltype( sn );
   void *vo = NULL;
   *victim = NULL;
   *obj = NULL;
   switch ( skill->target )
      
   {
      default:
         bug( "Do_cast: bad target for sn %d.", sn );
         return &pAbort;
      case TAR_IGNORE:
         break;
      case TAR_CHAR_OFFENSIVE:
         if( arg[0] == '\0' )
            
         {
            if( ( *victim = ch->last_hit ) == NULL )
               
            {
               send_to_char( "Cast the spell on whom?\n\r", ch );
               return &pAbort;
            }
         }
         else
         {
            if( ( *victim = get_char_room( ch, arg ) ) == NULL )
               
            {
               if( skill->range > 0 )
                  break;
               send_to_char( "They aren't here.\n\r", ch );
               return &pAbort;
            }
         }
         if( ch == *victim )
            
         {
            if( SPELL_FLAG( get_skilltype( sn ), SF_NOSELF ) )
               
            {
               send_to_char( "You can't cast this on yourself!\n\r", ch );
               return &pAbort;
            }
            else if( skill->range < 1 )
               send_to_char( "Cast this on yourself?  Okay...\n\r", ch );
         }
         if( !IS_NPC( ch ) )
            
         {
            if( !IS_NPC( *victim ) )
               
            {
               if( get_timer( ch, TIMER_PKILLED ) > 0 )
                  
               {
                  send_to_char( "You have been killed in the last 5 minutes.\n\r", ch );
                  return &pAbort;
               }
               if( get_timer( *victim, TIMER_PKILLED ) > 0 )
                  
               {
                  send_to_char( "This player has been killed in the last 5 minutes.\n\r", ch );
                  return &pAbort;
               }
            }
            if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == *victim )
               
            {
               send_to_char( "You can't do that on your own follower.\n\r", ch );
               return &pAbort;
            }
         }
         vo = ( void * )*victim;
         break;
      case TAR_CHAR_DEFENSIVE:
         if( arg[0] == '\0' )
            *victim = ch;
         
         else
            
         {
            if( ch->curr_talent[TAL_MOTION] >= 100  &&ch->curr_talent[TAL_SEEKING] >= 100 )
            {
               if( ( *victim = get_char_world( ch, arg ) ) == NULL )
                  
               {
                  send_to_char( "They aren't here.\n\r", ch );
                  return &pAbort;
               }
            }
            else
            {
               if( ( *victim = get_char_room( ch, arg ) ) == NULL )
                  
               {
                  send_to_char( "They aren't here.\n\r", ch );
                  return &pAbort;
               }
            }
         }
         if( ch == *victim && SPELL_FLAG( get_skilltype( sn ), SF_NOSELF ) )
            
         {
            send_to_char( "You can't cast this on yourself!\n\r", ch );
            return &pAbort;
         }
         vo = ( void * )*victim;
         break;
      case TAR_CHAR_SELF:
         if( arg[0] != '\0' && !nifty_is_name( arg, ch->name ) )
            
         {
            send_to_char( "You cannot cast this spell on another.\n\r", ch );
            return &pAbort;
         }
         vo = ( void * )ch;
         break;
      case TAR_OBJ_INV:
         if( arg[0] == '\0' )
            
         {
            send_to_char( "What should the spell be cast upon?\n\r", ch );
            return &pAbort;
         }
         if( ( *obj = get_obj_carry( ch, arg ) ) == NULL )
            
         {
            send_to_char( "You are not carrying that.\n\r", ch );
            return &pAbort;
         }
         vo = ( void * )*obj;
         break;
   }
   return vo;
}


/*
 * The kludgy global is for spells who want more stuff from command line.
 */ 
char *target_name;
char *ranged_target_name = NULL;

/*
 * Cast a spell.  Multi-caster and component support by Thoric
 * Ranged spell support by Scion
 */ 
void do_cast( CHAR_DATA * ch, char *argument ) 
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   static char staticbuf[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;
   OBJ_DATA * obj;
   void *vo = NULL;
   int mana;
   int sn;
   ch_ret retcode;
   bool dont_wait = FALSE;
   SKILLTYPE * skill = NULL;
   struct timeval time_used;
   retcode = rNONE;
   switch ( ch->substate )
      
   {
      default:
         
            /*
             * no ordering charmed mobs to cast spells 
             */ 
            if( IS_NPC( ch )  &&( IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS ) ) )
            
         {
            send_to_char( "You can't seem to do that right now...\n\r", ch );
            return;
         }
         if( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
            
         {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "Your magic flickers and dies.\n\r", ch );
            return;
         }
         
            /*
             * arg1 = spell name
             * arg2 = dir or target
             * target_name = dir+target || dir || target
             */ 
            target_name = one_argument( argument, arg1 );
         one_argument( target_name, arg2 );
         if( ranged_target_name )
            DISPOSE( ranged_target_name );
         ranged_target_name = str_dup( target_name );
         if( arg1[0] == '\0' )
            
         {
            send_to_char( "Cast which what where?\n\r", ch );
            return;
         }
         
            /*
             * Regular mortal spell casting 
             */ 
            if( !ch->pcdata || !IS_SET( ch->pcdata->permissions, PERMIT_MISC ) )
            
         {
            if( ( sn = find_spell( ch, arg1, TRUE ) ) < 0  ||( !IS_NPC( ch ) && get_best_talent( ch, sn ) == -1 ) )
               
            {
               send_to_char( "You don't know how to do that.\n\r", ch );
               return;
            }
            if( ( skill = get_skilltype( sn ) ) == NULL )
               
            {
               send_to_char( "You can't do that.\n\r", ch );
               return;
            }
         }
         
         else
            
               /*
                * 
                * * Godly "spell builder" spell casting with debugging messages
                */ 
         {
            if( ( sn = skill_lookup( arg1 ) ) < 0 )
               
            {
               send_to_char( "That's not a spell.\n\r", ch );
               return;
            }
            if( sn >= MAX_SKILL )
               
            {
               send_to_char( "Someone used too many skill numbers, that spell is unusable.\n\r", ch );
               return;
            }
            if( ( skill = get_skilltype( sn ) ) == NULL )
               
            {
               send_to_char( "Something is severely wrong with that spell.\n\r", ch );
               return;
            }
            if( skill->type != SKILL_SPELL )
               
            {
               send_to_char( "That isn't a spell.\n\r", ch );
               return;
            }
            if( !skill->spell_fun )
               
            {
               send_to_char( "We didn't finish that one yet.\n\r", ch );
               return;
            }
         }
         
            /*
             * 
             * * Something else removed by Merc       -Thoric
             */ 
            /*
             * Band-aid alert!  !IS_NPC check -- Blod 
             */ 
            if( ch->position < skill->minimum_position && !IS_NPC( ch ) )
            
         {
            switch ( ch->position )
               
            {
               default:
                  send_to_char( "You can't concentrate enough.\n\r", ch );
                  break;
               case POS_SITTING:
                  send_to_char( "You can't summon enough energy sitting down.\n\r", ch );
                  break;
               case POS_RESTING:
                  send_to_char( "You're too relaxed to cast that spell.\n\r", ch );
                  break;
               case POS_KNEELING:
                  send_to_char( "You should get to your feet first.\n\r", ch );
                  break;
               case POS_SQUATTING:
                  send_to_char( "You should stand up first.\n\r", ch );
                  break;
               case POS_SLEEPING:
                  send_to_char( "You dream about great feats of magic.\n\r", ch );
                  break;
            }
            return;
         }
         if( skill->spell_fun == spell_null )
            
         {
            send_to_char( "That's not a spell!\n\r", ch );
            return;
         }
         if( !skill->spell_fun )
            
         {
            send_to_char( "You cannot cast that... yet.\n\r", ch );
            return;
         }
         mana = IS_NPC( ch ) ? 0 : skill->min_mana;
         if( !IS_NPC( ch ) && GET_ADEPT( ch, skill_lookup( skill->name ) ) )
            mana = mana - ( mana / 4 );
         
            /*
             * 
             * * Locate targets.
             */ 
            vo = locate_targets( ch, arg2, sn, &victim, &obj );
         if( vo == &pAbort )
            return;
         if( victim  &&!IS_SAME_PLANE( ch, victim ) )
            
         {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "You don't seem to be able to cast spells on them.\n\r", ch );
            return;
         }
         if( !IS_NPC( ch ) && ch->mana < mana )
            
         {
            send_to_char( "You don't have enough mana.\n\r", ch );
            return;
         }
         if( ( SPELL_FLAG( skill, SF_INSTANT ) )  ||( IS_NPC( ch ) ) )
            break;
         if( number_percent(  ) < ch->curr_talent[TAL_TIME] )
         {
            learn_talent( ch, TAL_TIME );
            act( AT_MAGIC, "You sidestep time.", ch, NULL, NULL, TO_CHAR );
            dont_wait = TRUE;
            break;
         }
         
            /*
             * multi-participant spells         -Thoric 
             */ 
            /*
             * we don't use multi-participent spells, lets make this 
             */ 
            /*
             * for all spells instead -keo 
             */ 
            add_timer( ch, TIMER_DO_FUN, UMIN( skill->beats / 10, 3 ), do_cast, 1 );
         act( AT_MAGIC, "You begin to channel...", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n begins to channel...", ch, NULL, NULL, TO_ROOM );
         sprintf( staticbuf, "%s %s", arg2, target_name );
         ch->alloc_ptr = str_dup( staticbuf );
         ch->tempnum = sn;
         mana_from_char( ch, mana );
         return;
      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->alloc_ptr );
         if( IS_VALID_SN( ( sn = ch->tempnum ) ) )
            
         {
            if( ( skill = get_skilltype( sn ) ) == NULL )
               
            {
               send_to_char( "Something went wrong...\n\r", ch );
               bug( "do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn );
               return;
            }
         }
         set_char_color( AT_MAGIC, ch );
         send_to_char( "You stop channeling...\n\r", ch );
         return;
      case 1:
         sn = ch->tempnum;
         if( ( skill = get_skilltype( sn ) ) == NULL )
            
         {
            send_to_char( "Something went wrong...\n\r", ch );
            bug( "do_cast: substate 1: bad sn %d", sn );
            return;
         }
         if( !ch->alloc_ptr || !IS_VALID_SN( sn ) || skill->type != SKILL_SPELL )
            
         {
            send_to_char( "Something cancels out the spell!\n\r", ch );
            bug( "do_cast: ch->alloc_ptr NULL or bad sn (%d)", sn );
            return;
         }
         strcpy( staticbuf, ch->alloc_ptr );
         target_name = one_argument( staticbuf, arg2 );
         DISPOSE( ch->alloc_ptr );
         ch->substate = SUB_NONE;
         dont_wait = TRUE;
         send_to_char( "You concentrate all the energy into a burst of mystical words!\n\r", ch );
         vo = locate_targets( ch, arg2, sn, &victim, &obj );
         if( vo == &pAbort )
            return;
   }
   say_spell( ch, sn );
   if( !dont_wait )
      WAIT_STATE( ch, skill->beats );
   
      /*
       * 
       * * Getting ready to cast... check for spell components   -Thoric
       */ 
      if( !process_spell_components( ch, sn ) )
      
   {
      learn_from_failure( ch, sn );
      return;
   }
   if( !IS_NPC( ch )  &&( number_percent(  ) + skill->difficulty * 5 ) > LEARNED( ch, sn ) )
      
   {
      
         /*
          * Some more interesting loss of concentration messages  -Thoric 
          */ 
         switch ( number_bits( 2 ) )
         
      {
         case 0:   /* too busy */
            if( IS_FIGHTING( ch ) )
               send_to_char( "This round of battle is too hectic to concentrate properly.\n\r", ch );
            
            else
               send_to_char( "You lost your concentration.\n\r", ch );
            break;
         case 1:   /* irritation */
            if( number_bits( 2 ) == 0 )
               
            {
               switch ( number_bits( 2 ) )
                  
               {
                  case 0:
                     send_to_char( "A tickle in your nose prevents you from keeping your concentration.\n\r", ch );
                     break;
                  case 1:
                     send_to_char( "An itch on your leg keeps you from properly casting your spell.\n\r", ch );
                     break;
                  case 2:
                     send_to_char( "Something in your throat prevents you from uttering the proper phrase.\n\r", ch );
                     break;
                  case 3:
                     send_to_char( "A twitch in your eye disrupts your concentration for a moment.\n\r", ch );
                     break;
               }
            }
            
            else
               send_to_char( "Something distracts you, and you lose your concentration.\n\r", ch );
            break;
         case 2:   /* not enough time */
            if( IS_FIGHTING( ch ) )
               send_to_char( "There wasn't enough time this round to complete the casting.\n\r", ch );
            
            else
               send_to_char( "You lost your concentration.\n\r", ch );
            break;
         case 3:
            send_to_char( "You get a mental block mid-way through the casting.\n\r", ch );
            break;
      }
      learn_from_failure( ch, sn );
      return;
   }
   
   else
      
   {
      if( ch != victim && ( skill->target == TAR_CHAR_OFFENSIVE ) )
      {
         if( check_counterspell( ch, victim ) || check_displacement( ch, victim ) || check_antimagic( ch, victim ) )
         {
            learn_from_success( ch, sn );
            return;
         }
      }
      
         /*
          * 
          * * check for immunity to magic if victim is known...
          * * and it is a TAR_CHAR_DEFENSIVE/SELF spell
          * * otherwise spells will have to check themselves
          */ 
         if( ( ( skill->target == TAR_CHAR_DEFENSIVE 
                 ||skill->target == TAR_CHAR_SELF )  &&victim && IS_SET( victim->immune, RIS_MAGIC ) ) )
         
      {
         immune_casting( skill, ch, victim, NULL );
         retcode = rSPELL_FAILED;
      }
      
      else
         
      {
         start_timer( &time_used );
         if( !char_died( ch ) )
            retcode = ( *skill->spell_fun ) ( sn, get_curr_wil( ch ), ch, vo );
         end_timer( &time_used );
         update_userec( &time_used, &skill->userec );
      }
   }
   if( retcode == rCHAR_DIED || retcode == rERROR || char_died( ch ) )
      return;
   
      /*
       * learning 
       */ 
      if( retcode != rSPELL_FAILED )
      learn_from_success( ch, sn );
   
   else
      learn_from_failure( ch, sn );
   
      /*
       * 
       * * Fixed up a weird mess here, and added double safeguards  -Thoric
       */ 
      if( skill->target == TAR_CHAR_OFFENSIVE && victim && !char_died( victim ) && victim != ch )
      
   {
      CHAR_DATA * vch, *vch_next;
      for( vch = ch->in_room->first_person; vch; vch = vch_next )
         
      {
         vch_next = vch->next_in_room;
      }
   }
   return;
}


/*
 * Cast spells at targets using a magical object.
 */ 
   ch_ret obj_cast_spell( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
   void *vo;
   ch_ret retcode = rNONE;
   SKILLTYPE * skill = get_skilltype( sn );
   struct timeval time_used;
   if( sn == -1 )
      return retcode;
   if( !skill || !skill->spell_fun )
      
   {
      bug( "Obj_cast_spell: bad sn %d.", sn );
      return rERROR;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
      
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "Nothing seems to happen...\n\r", ch );
      return rNONE;
   }
   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) && skill->target == TAR_CHAR_OFFENSIVE )
      
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "Nothing seems to happen...\n\r", ch );
      return rNONE;
   }
   
      /*
       * 
       * * Basically this was added to cut down on level 5 players using level
       * * 40 scrolls in battle too often ;)      -Thoric
       */ 
      if( ( skill->target == TAR_CHAR_OFFENSIVE  ||number_bits( 7 ) == 1 ) /* 1/128 chance if non-offensive */  
          &&skill->type != SKILL_HERB  &&!chance( ch, 100 ) )
      
   {
      switch ( number_bits( 2 ) )
         
      {
         case 0:
            failed_casting( skill, ch, victim, NULL );
            break;
         case 1:
            act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
            if( victim )
               act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
            act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
         case 2:
            failed_casting( skill, ch, victim, NULL );
            break;
         case 3:
            act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
            if( victim )
               act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
            act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
      }
      return rNONE;
   }
   target_name = "";
   switch ( skill->target )
      
   {
      default:
         bug( "Obj_cast_spell: bad target for sn %d.", sn );
         return rERROR;
      case TAR_IGNORE:
         vo = NULL;
         if( victim )
            target_name = victim->name;
         
         else
         if( obj )
            target_name = obj->name;
         break;
      case TAR_CHAR_OFFENSIVE:
         if( victim != ch )
            
         {
            if( !victim )
               victim = ch->last_hit;
            if( !victim )
               
            {
               send_to_char( "You can't do that.\n\r", ch );
               return rNONE;
            }
         }
         vo = ( void * )victim;
         break;
      case TAR_CHAR_DEFENSIVE:
         if( victim == NULL )
            victim = ch;
         vo = ( void * )victim;
         if( skill->type != SKILL_HERB  &&IS_SET( victim->immune, RIS_MAGIC ) )
            
         {
            immune_casting( skill, ch, victim, NULL );
            return rNONE;
         }
         break;
      case TAR_CHAR_SELF:
         vo = ( void * )ch;
         if( skill->type != SKILL_HERB  &&IS_SET( ch->immune, RIS_MAGIC ) )
            
         {
            immune_casting( skill, ch, victim, NULL );
            return rNONE;
         }
         break;
      case TAR_OBJ_INV:
         if( obj == NULL )
            
         {
            send_to_char( "You can't do that.\n\r", ch );
            return rNONE;
         }
         vo = ( void * )obj;
         break;
   }
   start_timer( &time_used );
   retcode = ( *skill->spell_fun ) ( sn, get_curr_wil( ch ), ch, vo );
   end_timer( &time_used );
   update_userec( &time_used, &skill->userec );
   if( retcode == rSPELL_FAILED )
      retcode = rNONE;
   if( retcode == rCHAR_DIED || retcode == rERROR )
      return retcode;
   if( char_died( ch ) )
      return rCHAR_DIED;
   return retcode;
}


/*
 * Spell functions.
 */ 
   ch_ret spell_blindness( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   AFFECT_DATA af;
   int tmp;
   SKILLTYPE * skill = get_skilltype( sn );
   if( SPELL_FLAG( skill, SF_PKSENSITIVE )  &&!IS_NPC( ch ) && !IS_NPC( victim ) )
      tmp = level / 2;
   
   else
      tmp = level;
   if( IS_SET( victim->immune, RIS_MAGIC ) )
      
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( IS_AFFECTED( victim, AFF_BLIND ) )
      
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.location = APPLY_HITROLL;
   af.modifier = -4;
   af.duration = ( 1 + ( level / 3 ) ) * DUR_CONV;
   af.bitvector = meb( AFF_BLIND );
   affect_to_char( victim, &af );
   set_char_color( AT_MAGIC, victim );
   send_to_char( "You are blinded!\n\r", victim );
   if( ch != victim )
      
   {
      act( AT_MAGIC, "You weave a spell of blindness around $N.", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "$n weaves a spell of blindness about $N.", ch, NULL, victim, TO_NOTVICT );
   }
   return rNONE;
}

ch_ret spell_balance_mind( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   victim->mental_state = 0;
   act( AT_MAGIC, "You balance $N's mind.", ch, NULL, victim, TO_CHAR );
   act( AT_MAGIC, "Your mind is balanced.", ch, NULL, victim, TO_VICT );
   return rNONE;
}

ch_ret spell_bloat( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   if( IS_NPC( victim ) || victim->pcdata->condition[COND_THIRST] < 0 )
      
   {
      act( AT_MAGIC, "$N seems unaffected.", ch, NULL, victim, TO_CHAR );
      return rSPELL_FAILED;
   }
   victim->pcdata->condition[COND_THIRST] += 10;
   act( AT_MAGIC, "You feel bloated.", ch, NULL, victim, TO_VICT );
   act( AT_MAGIC, "You fill $N up with water.", ch, NULL, victim, TO_CHAR );
   return rNONE;
}

ch_ret spell_change_sex( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   AFFECT_DATA af;
   SKILLTYPE * skill = get_skilltype( sn );
   if( IS_SET( victim->immune, RIS_MAGIC ) )
      
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( is_affected( victim, sn ) )
      
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.duration = 10 * level * DUR_CONV;
   af.location = APPLY_SEX;
   
   do
      
   {
      af.modifier = number_range( 0, 2 ) - victim->sex;
   }
   while( af.modifier == 0 );
   xCLEAR_BITS( af.bitvector );
   affect_to_char( victim, &af );
   
/*    set_char_color( AT_MAGIC, victim );
    send_to_char( "You feel different.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );*/ 
      successful_casting( skill, ch, victim, NULL );
   return rNONE;
}

ch_ret spell_cure_blindness( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   SKILLTYPE * skill = get_skilltype( sn );
   set_char_color( AT_MAGIC, ch );
   if( IS_SET( victim->immune, RIS_MAGIC ) )
      
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( !is_affected( victim, gsn_blindness ) )
      
   {
      if( ch != victim )
         send_to_char( "You work your cure, but it has no apparent effect.\n\r", ch );
      
      else
         send_to_char( "You don't seem to be blind.\n\r", ch );
      return rSPELL_FAILED;
   }
   affect_strip( victim, gsn_blindness );
   set_char_color( AT_MAGIC, victim );
   send_to_char( "Your vision returns!\n\r", victim );
   if( ch != victim )
      send_to_char( "You work your cure, restoring vision.\n\r", ch );
   return rNONE;
}

ch_ret spell_cure_poison( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   SKILLTYPE * skill = get_skilltype( sn );
   if( IS_SET( victim->immune, RIS_MAGIC ) )
      
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( is_affected( victim, gsn_poison ) )
      
   {
      affect_strip( victim, gsn_poison );
      set_char_color( AT_MAGIC, victim );
      send_to_char( "A warm feeling runs through your body.\n\r", victim );
      victim->mental_state = URANGE( -100, victim->mental_state, -10 );
      if( ch != victim )
         
      {
         act( AT_MAGIC, "A flush of health washes over $N.", ch, NULL, victim, TO_NOTVICT );
         act( AT_MAGIC, "You lift the poison from $N's body.", ch, NULL, victim, TO_CHAR );
      }
      return rNONE;
   }
   
   else
      
   {
      set_char_color( AT_MAGIC, ch );
      if( ch != victim )
         send_to_char( "You work your cure, but it has no apparent effect.\n\r", ch );
      
      else
         send_to_char( "You don't seem to be poisoned.\n\r", ch );
      return rSPELL_FAILED;
   }
}

ch_ret spell_curse( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   AFFECT_DATA af;
   SKILLTYPE * skill = get_skilltype( sn );
   if( IS_SET( victim->immune, RIS_MAGIC ) )
      
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( IS_AFFECTED( victim, AFF_CURSE ) )
      
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.duration = ( 4 * level ) * DUR_CONV;
   af.location = APPLY_HITROLL;
   af.modifier = -1;
   af.bitvector = meb( AFF_CURSE );
   affect_to_char( victim, &af );
   af.location = APPLY_SAVING_SPELL;
   af.modifier = 1;
   affect_to_char( victim, &af );
   set_char_color( AT_MAGIC, victim );
   send_to_char( "You feel unclean.\n\r", victim );
   if( ch != victim )
      
   {
      act( AT_MAGIC, "You utter a curse upon $N.", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "$n utters a curse upon $N.", ch, NULL, victim, TO_NOTVICT );
   }
   return rNONE;
}


/* modified version of spell_transport -keo */ 
   ch_ret spell_pigeon( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim;
   char arg3[MAX_STRING_LENGTH];
   OBJ_DATA * obj;
   target_name = one_argument( target_name, arg3 );
   if( ( victim = get_char_world( ch, target_name ) ) == NULL 
         ||victim == ch 
         ||IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) 
         ||IS_SET( victim->in_room->room_flags, ROOM_INDOORS )  ||IS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE ) )
      
   {
      send_to_char( "The pigeon fails to find the recipient.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( ( obj = get_obj_carry( ch, arg3 ) ) == NULL 
        ||( victim->carry_weight + get_obj_weight( obj ) ) > can_carry_w( victim ) )
      
   {
      send_to_char( "The pigeon flies off then returns with the item undelivered.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( obj->weight > 10 )
   {
      send_to_char( "The pigeon cannot carry that much weight.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      send_to_char( "The item squirms out of the pigeon's grasp!\n\r", ch );
      return rSPELL_FAILED;
   }
   separate_obj( obj );
   if( IS_OBJ_STAT( obj, ITEM_NODROP ) )
      
   {
      send_to_char( "You can't seem to let go of it.\n\r", ch );
      return rSPELL_FAILED;
   }
   act( AT_MAGIC, "You attach $p to the pigeon's legs and it flies away.", ch, obj, NULL, TO_CHAR );
   act( AT_MAGIC, "$n attaches $p to a pigeons's legs and it flies away.", ch, obj, NULL, TO_ROOM );
   obj_from_char( obj );
   obj_to_char( obj, victim );
   act( AT_MAGIC, "A pigeon flies in and drops $p into your hands.", victim, obj, NULL, TO_CHAR );
   act( AT_MAGIC, "A pigeon flies in and drops $p into $n's hands.", victim, obj, NULL, TO_ROOM );
   save_char_obj( ch );
   save_char_obj( victim );
   return rNONE;
}

ch_ret spell_poison( int sn, int wil, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   AFFECT_DATA af;
   bool first = TRUE;
   if( IS_AFFECTED( victim, AFF_POISON ) )
      first = FALSE;
   af.type = sn;
   af.duration = wil * DUR_CONV;
   af.location = APPLY_STR;
   af.modifier = -2;
   af.bitvector = meb( AFF_POISON );
   affect_join( victim, &af );
   set_char_color( AT_GREEN, victim );
   send_to_char( "You feel very sick.\n\r", victim );
   victim->mental_state = URANGE( 10, victim->mental_state  +( first ? 5 : 0 ), 100 );
   if( ch != victim )
      
   {
      act( AT_GREEN, "$N shivers as your poison spreads through $S body.", ch, NULL, victim, TO_CHAR );
      act( AT_GREEN, "$N shivers as $n's poison spreads through $S body.", ch, NULL, victim, TO_NOTVICT );
   }
   return rNONE;
}

ch_ret spell_remove_curse( int sn, int level, CHAR_DATA * ch, void *vo )
{
   OBJ_DATA * obj;
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   SKILLTYPE * skill = get_skilltype( sn );
   if( IS_SET( victim->immune, RIS_MAGIC ) )
      
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( is_affected( victim, gsn_curse ) )
      
   {
      affect_strip( victim, gsn_curse );
      set_char_color( AT_MAGIC, victim );
      send_to_char( "The weight of your curse is lifted.\n\r", victim );
      if( ch != victim )
         
      {
         act( AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT );
      }
   }
   
   else
   if( victim->first_carrying )
      
   {
      for( obj = victim->first_carrying; obj; obj = obj->next_content )
         if( !obj->in_obj  &&( IS_OBJ_STAT( obj, ITEM_NOREMOVE )  ||IS_OBJ_STAT( obj, ITEM_NODROP ) ) )
            
         {
            if( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
               xREMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE );
            if( IS_OBJ_STAT( obj, ITEM_NODROP ) )
               xREMOVE_BIT( obj->extra_flags, ITEM_NODROP );
            set_char_color( AT_MAGIC, victim );
            send_to_char( "You feel a burden released.\n\r", victim );
            if( ch != victim )
               
            {
               act( AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR );
               act( AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT );
            }
            return rNONE;
         }
   }
   return rNONE;
}

ch_ret spell_remove_trap( int sn, int level, CHAR_DATA * ch, void *vo )
{
   OBJ_DATA * obj;
   OBJ_DATA * trap;
   bool found;
   int retcode;
   SKILLTYPE * skill = get_skilltype( sn );
   if( !target_name || target_name[0] == '\0' )
      
   {
      send_to_char( "Remove trap on what?\n\r", ch );
      return rSPELL_FAILED;
   }
   found = FALSE;
   if( !ch->in_room->first_content )
      
   {
      send_to_char( "You can't find that here.\n\r", ch );
      return rNONE;
   }
   for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
      if( can_see_obj( ch, obj ) && nifty_is_name( target_name, obj->name ) )
         
      {
         found = TRUE;
         break;
      }
   if( !found )
      
   {
      send_to_char( "You can't find that here.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( ( trap = get_trap( obj ) ) == NULL )
      
   {
      failed_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
   }
   if( chance( ch, get_curr_wis( ch ) ) )
      
   {
      send_to_char( "Oops!\n\r", ch );
      retcode = spring_trap( ch, trap );
      if( retcode == rNONE )
         retcode = rSPELL_FAILED;
      return retcode;
   }
   extract_obj( trap );
   successful_casting( skill, ch, NULL, NULL );
   return rNONE;
}

ch_ret spell_satiate( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   if( IS_NPC( victim ) || victim->pcdata->condition[COND_FULL] < 0 )
   {
      act( AT_MAGIC, "$N seems unaffected.", ch, NULL, victim, TO_CHAR );
      return rSPELL_FAILED;
   }
   victim->pcdata->condition[COND_FULL] += 20;
   act( AT_MAGIC, "You are satiated.", ch, NULL, victim, TO_VICT );
   act( AT_MAGIC, "You fill $N's stomach.", ch, NULL, victim, TO_CHAR );
   return rNONE;
}


/* room identify - keo */ 
   ch_ret spell_sense_location( int sn, int level, CHAR_DATA * ch, void *vo )
{
   char *sect;
   int temp;
   ch_printf( ch, "You are in %s in %s, on the world of %s.\n\r", ch->in_room->name, ch->in_room->area->name,
               ch->in_room->area->resetmsg );
   switch ( ch->in_room->sector_type )
   {
      default:
         sect = "an indeterminate location";
         break;
      case SECT_INSIDE:
         sect = "inside";
         break;
      case SECT_CITY:
         sect = "a city street";
         break;
      case SECT_FIELD:
         sect = "a field";
         break;
      case SECT_FOREST:
         sect = "a forest";
         break;
      case SECT_HILLS:
         sect = "in the hills";
         break;
      case SECT_MOUNTAIN:
         sect = "in the mountains";
         break;
      case SECT_WATER_SWIM:
         sect = "swallow water";
         break;
      case SECT_WATER_NOSWIM:
         sect = "deep water";
         break;
      case SECT_UNDERWATER:
         sect = "underwater";
         break;
      case SECT_AIR:
         sect = "in the air";
         break;
      case SECT_DESERT:
         sect = "in the desert";
         break;
      case SECT_OCEANFLOOR:
         sect = "on the oceanfloor";
         break;
      case SECT_UNDERGROUND:
         sect = "underground";
         break;
      case SECT_LAVA:
         sect = "molten lava";
         break;
      case SECT_SWAMP:
         sect = "in the swamps";
         break;
      case SECT_BEACH:
         sect = "on a beach";
         break;
      case SECT_ICE:
         sect = "on ice";
         break;
   }
   ch_printf( ch, "It appears to be %s.\n\r", sect );
   if( ch->in_room->room_flags )
      ch_printf( ch, "It seems to be %s.\n\r", flag_string( ch->in_room->room_flags, r_flags ) );
   temp = ch->in_room->curr_water;
   ch_printf( ch, "%s, and ",
               temp >= 100 ? "The room is completely underwater" : temp > 50 ? "The water is deep" : temp >
               0 ? "The water is muddy and shallow"  : "The room is dry" );
   temp = ch->in_room->curr_vegetation;
   ch_printf( ch, "%s vegetation grows.\n\r", temp >= 66 ? "thick" : temp > 33 ? "some" : "no" );
   temp = ch->in_room->curr_resources;
   ch_printf( ch, "There is a %s concentration of resources here.\n\r",
               temp >= 66 ? "high" : temp > 33 ? "moderate" : "low" );
   if( ch->in_room->runes )
      ch_printf( ch, "Runes of %s line the walls.\n\r", flag_string( ch->in_room->runes, rune_flags ) );
   return rNONE;
}

ch_ret spell_shadowshroud( int sn, int level, CHAR_DATA * ch, void *vo )
{
   OBJ_DATA * obj = ( OBJ_DATA * ) vo;
   if( IS_OBJ_STAT( obj, ITEM_DARK ) )
   {
      ch_printf( ch, "That item is already shrouded in shadows.\n\r" );
      return rSPELL_FAILED;
   }
   if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
   {
      if( obj->value[4] == BRAND_RADIANT )
      {
         send_to_char( "You are unable to quench this radiant light.\n\r", ch );
         return rSPELL_FAILED;
      }
      act( AT_MAGIC, "The $t grows dim and ceases to shine.", ch, myobj( obj ), NULL, TO_CHAR );
      act( AT_MAGIC, "$n's $t grows dim and ceases to shine.", ch, myobj( obj ), NULL, TO_ROOM );
      xREMOVE_BIT( obj->extra_flags, ITEM_GLOW );
      return rNONE;
   }
   act( AT_MAGIC, "Swirling shadows shroud the $t.", ch, myobj( obj ), NULL, TO_CHAR );
   act( AT_MAGIC, "Swirling shadows shroud $n's $t.", ch, myobj( obj ), NULL, TO_ROOM );
   xSET_BIT( obj->extra_flags, ITEM_DARK );
   return rNONE;
}

ch_ret spell_sleep( int sn, int level, CHAR_DATA * ch, void *vo )
{
   AFFECT_DATA af;
   int tmp;
   CHAR_DATA * victim;
   SKILLTYPE * skill = get_skilltype( sn );
   if( ( victim = get_char_room( ch, target_name ) ) == NULL )
      
   {
      send_to_char( "They aren't here.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( !IS_NPC( victim ) && IS_FIGHTING( victim ) )
      
   {
      send_to_char( "You cannot sleep a fighting player.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( IS_SET( victim->immune, RIS_SLEEP ) )
      
   {
      immune_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( SPELL_FLAG( skill, SF_PKSENSITIVE )  &&!IS_NPC( ch ) && !IS_NPC( victim ) )
      tmp = get_curr_wil( ch ) / 2;
   
   else
      tmp = get_curr_wil( ch );
   if( number_range( 1, get_curr_wil( ch ) ) 
         <number_range( 1, get_curr_wil( victim ) + TALENT( victim, TAL_SECURITY ) ) )
      
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( ch == victim )
      return rSPELL_FAILED;
   if( !IS_FIGHTING( victim ) )
      
   {
      victim->last_hit = ch;
      ch->last_hit = victim;
   }
   else
      return rSPELL_FAILED;
   af.type = sn;
   af.duration = get_curr_wil( ch ) / 5;
   af.location = APPLY_NONE;
   af.modifier = 0;
   af.bitvector = meb( AFF_SLEEP );
   affect_join( victim, &af );
   
      /*
       * Added by Narn at the request of Dominus. 
       */ 
      if( !IS_NPC( victim ) )
      
   {
      sprintf( log_buf, "%s has cast sleep on %s.", ch->name, victim->name );
      to_channel( log_buf, "Spell", PERMIT_SECURITY );
   }
   if( IS_AWAKE( victim ) )
      
   {
      act( AT_MAGIC, "You feel very sleepy ..... zzzzzz.", victim, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
      victim->position = POS_SLEEPING;
   }
   if( IS_NPC( victim ) )
      start_hating( victim, ch );
   return rNONE;
}

ch_ret spell_null( int sn, int level, CHAR_DATA * ch, void *vo )
{
   send_to_char( "That's not a spell!\n\r", ch );
   return rNONE;
}


/* don't remove, may look redundant, but is important */ 
   ch_ret spell_notfound( int sn, int level, CHAR_DATA * ch, void *vo )
{
   send_to_char( "That's not a spell!\n\r", ch );
   return rNONE;
}


/*
 *   Haus' Spell Additions
 *
 */ 
   
/*
 * Syntax portal (mob/char) 
 * opens a 2-way EX_PORTAL from caster's room to room inhabited by  
 *  mob or character won't mess with existing exits
 *
 * do_mp_open_passage, combined with spell_astral
 */ 
   ch_ret spell_portal( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim;
   ROOM_INDEX_DATA * targetRoom, *fromRoom;
   int targetRoomVnum;
   OBJ_DATA * portalObj;
   EXIT_DATA * pexit;
   char buf[MAX_STRING_LENGTH];
   SKILLTYPE * skill = get_skilltype( sn );
   
      /*
       * No go if all kinds of things aren't just right, including the caster
       * and victim are not both pkill or both peaceful. -- Narn
       */ 
      if( ( victim = get_char_world( ch, target_name ) ) == NULL 
          ||victim == ch 
          ||!victim->in_room 
          ||IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) 
          ||IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) 
          ||IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL ) 
          ||IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) 
          ||IS_SET( victim->in_room->room_flags, ROOM_PROTOTYPE ) 
          ||IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) 
          ||IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL ) 
          ||( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) ) )
      
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( victim->in_room == ch->in_room )
      
   {
      send_to_char( "They are right beside you!", ch );
      return rSPELL_FAILED;
   }
   targetRoomVnum = victim->in_room->vnum;
   fromRoom = ch->in_room;
   targetRoom = victim->in_room;
   
      /*
       * Check if there already is a portal in either room. 
       */ 
      for( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
      
   {
      if( IS_SET( pexit->exit_info, EX_PORTAL ) )
         
      {
         send_to_char( "There is already a portal in this room.\n\r", ch );
         return rSPELL_FAILED;
      }
      if( pexit->vdir == DIR_PORTAL )
         
      {
         send_to_char( "You may not create a portal in this room.\n\r", ch );
         return rSPELL_FAILED;
      }
   }
   for( pexit = targetRoom->first_exit; pexit; pexit = pexit->next )
      if( pexit->vdir == DIR_PORTAL )
         
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }
   pexit = make_exit( fromRoom, targetRoom, DIR_PORTAL );
   pexit->keyword = STRALLOC( "portal" );
   pexit->description = STRALLOC( "You gaze into the shimmering portal...\n\r" );
   pexit->key = -1;
   pexit->exit_info = EX_PORTAL | EX_xENTER | EX_HIDDEN | EX_xLOOK;
   pexit->vnum = targetRoomVnum;
   portalObj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
   portalObj->timer = 5;
   sprintf( buf, "a portal created by %s", ch->name );
   STRFREE( portalObj->short_descr );
   portalObj->short_descr = STRALLOC( buf );
   
      /*
       * support for new casting messages 
       */ 
      if( !skill->hit_char || skill->hit_char[0] == '\0' )
      
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You utter an incantation, and a portal forms in front of you!\n\r", ch );
   }
   
   else
      act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
   if( !skill->hit_room || skill->hit_room[0] == '\0' )
      act( AT_MAGIC, "$n utters an incantation, and a portal forms in front of you!", ch, NULL, NULL, TO_ROOM );
   
   else
      act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );
   if( !skill->hit_vict || skill->hit_vict[0] == '\0' )
   {
      act( AT_MAGIC, "A shimmering portal forms in front of you!", victim, NULL, NULL, TO_ROOM );
      act( AT_MAGIC, "A shimmering portal forms in front of you!", victim, NULL, NULL, TO_CHAR );
   }
   else
      act( AT_MAGIC, skill->hit_vict, victim, NULL, victim, TO_ROOM );
   portalObj = obj_to_room( portalObj, ch->in_room );
   pexit = make_exit( targetRoom, fromRoom, DIR_PORTAL );
   pexit->keyword = STRALLOC( "portal" );
   pexit->description = STRALLOC( "You gaze into the shimmering portal...\n\r" );
   pexit->key = -1;
   pexit->exit_info = EX_PORTAL | EX_xENTER | EX_HIDDEN;
   pexit->vnum = targetRoomVnum;
   portalObj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
   portalObj->timer = 5;
   STRFREE( portalObj->short_descr );
   portalObj->short_descr = STRALLOC( buf );
   portalObj = obj_to_room( portalObj, targetRoom );
   return rNONE;
}

ch_ret spell_recharge( int sn, int level, CHAR_DATA * ch, void *vo )
{
   OBJ_DATA * obj = ( OBJ_DATA * ) vo;
   if( obj->item_type == ITEM_STAFF  ||obj->item_type == ITEM_WAND )
      
   {
      separate_obj( obj );
      if( obj->value[2] == obj->value[1]  ||obj->value[1] > ( obj->pIndexData->value[1] * 4 ) )
         
      {
         act( AT_FIRE, "$p bursts into flames, injuring you!", ch, obj, NULL, TO_CHAR );
         act( AT_FIRE, "$p bursts into flames, charring $n!", ch, obj, NULL, TO_ROOM );
         extract_obj( obj );
         return rSPELL_FAILED;
      }
      if( chance( ch, 2 ) )
         
      {
         act( AT_YELLOW, "$p glows with a blinding magical luminescence.", ch, obj, NULL, TO_CHAR );
         obj->value[1] *= 2;
         obj->value[2] = obj->value[1];
         return rNONE;
      }
      
      else
      if( chance( ch, 5 ) )
         
      {
         act( AT_YELLOW, "$p glows brightly for a few seconds...", ch, obj, NULL, TO_CHAR );
         obj->value[2] = obj->value[1];
         return rNONE;
      }
      
      else
      if( chance( ch, 10 ) )
         
      {
         act( AT_WHITE, "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR );
         act( AT_WHITE, "$n's attempt at recharging fails, and $p disintegrates.", ch, obj, NULL, TO_ROOM );
         extract_obj( obj );
         return rSPELL_FAILED;
      }
      
      else
      if( chance( ch, 20 ) )
         
      {
         send_to_char( "Nothing happens.\n\r", ch );
         return rSPELL_FAILED;
      }
      
      else
         
      {
         act( AT_MAGIC, "$p feels warm to the touch.", ch, obj, NULL, TO_CHAR );
         --obj->value[1];
         obj->value[2] = obj->value[1];
         return rNONE;
      }
   }
   
   else
      
   {
      send_to_char( "You can't recharge that!\n\r", ch );
      return rSPELL_FAILED;
   }
}


/* Ignores pickproofs, but can't unlock containers. -- Altrag 17/2/96 */ 
   ch_ret spell_knock( int sn, int level, CHAR_DATA * ch, void *vo )
{
   EXIT_DATA * pexit;
   SKILLTYPE * skill = get_skilltype( sn );
   set_char_color( AT_MAGIC, ch );
   
      /*
       * 
       * * shouldn't know why it didn't work, and shouldn't work on pickproof
       * * exits.  -Thoric
       */ 
      if( !( pexit = find_door( ch, target_name, FALSE ) ) 
          ||!IS_SET( pexit->exit_info, EX_CLOSED ) 
          ||!IS_SET( pexit->exit_info, EX_LOCKED )  ||IS_SET( pexit->exit_info, EX_PICKPROOF ) )
      
   {
      failed_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
   }
   REMOVE_BIT( pexit->exit_info, EX_LOCKED );
   send_to_char( "*Click*\n\r", ch );
   if( pexit->rexit && pexit->rexit->to_room == ch->in_room )
      REMOVE_BIT( pexit->rexit->exit_info, EX_LOCKED );
   check_room_for_traps( ch, TRAP_UNLOCK | trap_door[pexit->vdir] );
   return rNONE;
}


/* Tells to sleepers in are. -- Altrag 17/2/96 */ 
   ch_ret spell_dream( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim;
   char arg[MAX_INPUT_LENGTH];
   target_name = one_argument( target_name, arg );
   set_char_color( AT_MAGIC, ch );
   if( !( victim = get_char_world( ch, arg ) ) )
      
   {
      send_to_char( "They aren't here.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( victim->position != POS_SLEEPING )
      
   {
      send_to_char( "They aren't asleep.\n\r", ch );
      return rSPELL_FAILED;
   }
   if( !target_name )
      
   {
      send_to_char( "What do you want them to dream about?\n\r", ch );
      return rSPELL_FAILED;
   }
   set_char_color( AT_TELL, victim );
   ch_printf( victim, "You have dreams about %s telling you '%s'.\n\r", PERS( ch, victim ), target_name );
   successful_casting( get_skilltype( sn ), ch, victim, NULL );
   
/*  send_to_char("Ok.\n\r", ch);*/ 
      return rNONE;
}


   /*******************************************************
	 * Everything after this point is part of SMAUG SPELLS *
	 *******************************************************/ 
   
/*
 * Generic offensive spell damage attack			-Thoric
 */ 
   ch_ret spell_attack( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   SKILLTYPE * skill = get_skilltype( sn );
   int dam;
   ch_ret retcode = rNONE;
   if( victim == NULL )
   {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   if( skill->dice )
      dam = UMAX( 0, dice_parse( ch, skill->dice ) );
   
   else
      dam = dice( 1, level / 2 );
   if( retcode == rNONE && skill->affects 
        &&!char_died( ch ) && !char_died( victim ) 
        &&( !is_affected( victim, sn )  ||SPELL_FLAG( skill, SF_ACCUMULATIVE )  ||SPELL_FLAG( skill, SF_RECASTABLE ) ) )
      retcode = spell_affectchar( sn, level, ch, victim );
   return retcode;
}


/*
 * Generic area attack						-Thoric
 */ 
   ch_ret spell_area_attack( int sn, int level, CHAR_DATA * ch, void *vo )
{
   CHAR_DATA * vch, *vch_next;
   SKILLTYPE * skill = get_skilltype( sn );
   bool affects;
   int dam;
   bool ch_died = FALSE;
   ch_ret retcode = rNONE;
   if( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
      
   {
      failed_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
   }
   affects = ( skill->affects ? TRUE : FALSE );
   if( skill->hit_char && skill->hit_char[0] != '\0' )
      act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
   if( skill->hit_room && skill->hit_room[0] != '\0' )
      act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );
   for( vch = ch->in_room->first_person; vch; vch = vch_next )
      
   {
      vch_next = vch->next_in_room;
      if( vch == ch )
         continue;
      if( !IS_SAME_PLANE( ch, vch ) )
         continue;
      if( skill->dice )
         dam = dice_parse( ch, skill->dice );
      
      else
         dam = dice( 1, level / 2 );
      if( retcode == rNONE && affects && !char_died( ch ) && !char_died( vch ) 
           &&( !is_affected( vch, sn )  ||SPELL_FLAG( skill, SF_ACCUMULATIVE )  ||SPELL_FLAG( skill, SF_RECASTABLE ) ) )
         retcode = spell_affectchar( sn, level, ch, vch );
      if( retcode == rCHAR_DIED || char_died( ch ) )
         
      {
         ch_died = TRUE;
         break;
      }
   }
   return retcode;
}

ch_ret spell_affectchar( int sn, int level, CHAR_DATA * ch, void *vo )
{
   AFFECT_DATA af;
   SMAUG_AFF * saf;
   SKILLTYPE * skill = get_skilltype( sn );
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   int chance;
   ch_ret retcode = rNONE;
   if( SPELL_FLAG( skill, SF_RECASTABLE ) )
      affect_strip( victim, sn );
   for( saf = skill->affects; saf; saf = saf->next )
   {
      if( saf->location >= REVERSE_APPLY )
         victim = ch;
      
      else
         victim = ( CHAR_DATA * ) vo;
      
         /*
          * Check if char has this bitvector already 
          */ 
         af.bitvector = meb( saf->bitvector );
      if( saf->bitvector >= 0 
            &&xIS_SET( victim->affected_by, saf->bitvector )  &&!SPELL_FLAG( skill, SF_ACCUMULATIVE ) )
         continue;
      
         /*
          * 
          * * necessary for affect_strip to work properly...
          */ 
         switch ( saf->bitvector )
         
      {
         default:
            af.type = sn;
            break;
         case AFF_POISON:
            af.type = gsn_poison;
            chance = ris_save( victim, level, RIS_POISON );
            if( chance == 1000 )
            {
               retcode = rVICT_IMMUNE;
               if( SPELL_FLAG( skill, SF_STOPONFAIL ) )
                  return retcode;
               continue;
            }
            victim->mental_state = URANGE( 30, victim->mental_state + 2, 100 );
            break;
         case AFF_BLIND:
            af.type = gsn_blindness;
            break;
         case AFF_CURSE:
            af.type = gsn_curse;
            break;
         case AFF_SLEEP:
            af.type = gsn_sleep;
            chance = ris_save( victim, level, RIS_SLEEP );
            if( chance == 1000 )
               
            {
               retcode = rVICT_IMMUNE;
               if( SPELL_FLAG( skill, SF_STOPONFAIL ) )
                  return retcode;
               continue;
            }
            break;
         case AFF_CHARM:
            af.type = gsn_charm_person;
            chance = ris_save( victim, level, RIS_CHARM );
            if( chance == 1000 )
               
            {
               retcode = rVICT_IMMUNE;
               if( SPELL_FLAG( skill, SF_STOPONFAIL ) )
                  return retcode;
               continue;
            }
            break;
      }
      af.duration = dice_parse( ch, saf->duration );
      af.modifier = dice_parse( ch, saf->modifier );
      af.location = saf->location % REVERSE_APPLY;
      if( af.duration == 0 )
         
      {
         switch ( af.location )
            
         {
            case APPLY_HIT:
               victim->hit = URANGE( 0, victim->hit + af.modifier, victim->max_hit );
               update_pos( victim );
               break;
            case APPLY_MANA:
               victim->mana = URANGE( 0, victim->mana + af.modifier, victim->max_mana );
               update_pos( victim );
               break;
            case APPLY_MOVE:
               victim->move = URANGE( 0, victim->move + af.modifier, victim->max_move );
               update_pos( victim );
               break;
            default:
               affect_modify( victim, &af, TRUE );
               break;
         }
      }
      
      else
      if( SPELL_FLAG( skill, SF_ACCUMULATIVE ) )
         affect_join( victim, &af );
      
      else
         affect_to_char( victim, &af );
   }
   update_pos( victim );
   return retcode;
}


/*
 * Generic spell affect						-Thoric
 */ 
   ch_ret spell_affect( int sn, int level, CHAR_DATA * ch, void *vo )
{
   SMAUG_AFF * saf;
   SKILLTYPE * skill = get_skilltype( sn );
   CHAR_DATA * victim = ( CHAR_DATA * ) vo;
   bool groupsp;
   bool areasp;
   bool hitchar = FALSE, hitroom = FALSE, hitvict = FALSE;
   ch_ret retcode;
   if( !skill->affects )
      
   {
      bug( "spell_affect has no affects sn %d", sn );
      return rNONE;
   }
   if( SPELL_FLAG( skill, SF_GROUPSPELL ) )
      groupsp = TRUE;
   
   else
      groupsp = FALSE;
   if( SPELL_FLAG( skill, SF_AREA ) )
      areasp = TRUE;
   
   else
      areasp = FALSE;
   if( !groupsp && !areasp )
      
   {
      
         /*
          * Can't find a victim 
          */ 
         if( !victim )
         
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }
      if( ( skill->type != SKILL_HERB 
              &&IS_SET( victim->immune, RIS_MAGIC ) )  ||is_immune( victim, SPELL_DAMAGE( skill ) ) )
         
      {
         immune_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }
      
         /*
          * Spell is already on this guy 
          */ 
         if( is_affected( victim, sn )  &&!SPELL_FLAG( skill, SF_ACCUMULATIVE )  &&!SPELL_FLAG( skill, SF_RECASTABLE ) )
         
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }
      if( ( saf = skill->affects ) && !saf->next 
            &&saf->location == APPLY_STRIPSN  &&!is_affected( victim, dice_parse( ch, saf->modifier ) ) )
         
      {
         failed_casting( skill, ch, victim, NULL );
         return rSPELL_FAILED;
      }
   }
   
   else
      
   {
      if( skill->hit_char && skill->hit_char[0] != '\0' )
         
      {
         if( strstr( skill->hit_char, "$N" ) )
            hitchar = TRUE;
         
         else
            act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
      }
      if( skill->hit_room && skill->hit_room[0] != '\0' )
         
      {
         if( strstr( skill->hit_room, "$N" ) )
            hitroom = TRUE;
         
         else
            act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );
      }
      if( skill->hit_vict && skill->hit_vict[0] != '\0' )
         hitvict = TRUE;
      if( victim )
         victim = victim->in_room->first_person;
      
      else
         victim = ch->in_room->first_person;
   }
   if( !victim )
      
   {
      bug( "spell_affect: could not find victim: sn %d", sn );
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
   }
   for( ; victim; victim = victim->next_in_room )
      
   {
      if( groupsp || areasp )
         
      {
         if( ( groupsp && !is_same_group( victim, ch ) ) 
              ||IS_SET( victim->immune, RIS_MAGIC ) 
              ||is_immune( victim, SPELL_DAMAGE( skill ) ) 
              ||( !SPELL_FLAG( skill, SF_RECASTABLE ) && is_affected( victim, sn ) ) )
            continue;
         if( hitvict && ch != victim )
            
         {
            act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );
            if( hitroom )
               
            {
               act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
               act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_CHAR );
            }
         }
         
         else
         if( hitroom )
            act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );
         if( ch == victim )
            
         {
            if( hitvict )
               act( AT_MAGIC, skill->hit_vict, ch, NULL, ch, TO_CHAR );
            
            else
            if( hitchar )
               act( AT_MAGIC, skill->hit_char, ch, NULL, ch, TO_CHAR );
         }
         
         else
         if( hitchar )
            act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
      }
      retcode = spell_affectchar( sn, level, ch, victim );
      if( !groupsp && !areasp )
         
      {
         if( retcode == rVICT_IMMUNE )
            immune_casting( skill, ch, victim, NULL );
         
         else
            successful_casting( skill, ch, victim, NULL );
         break;
      }
   }
   return rNONE;
}


/*
 * Generic inventory object spell				-Thoric
 */ 
   ch_ret spell_obj_inv( int sn, int level, CHAR_DATA * ch, void *vo )
{
   OBJ_DATA * obj = ( OBJ_DATA * ) vo;
   SKILLTYPE * skill = get_skilltype( sn );
   if( !obj )
      
   {
      failed_casting( skill, ch, NULL, NULL );
      return rNONE;
   }
   switch ( SPELL_ACTION( skill ) )
      
   {
      default:
      case SA_NONE:
         return rNONE;
      case SA_CREATE:
         if( SPELL_FLAG( skill, SF_WATER ) )   /* create water */
            
         {
            int water;
            WEATHER_DATA * weath = ch->in_room->area->weather;
            if( obj->item_type != ITEM_DRINK_CON )
               
            {
               send_to_char( "It is unable to hold water.\n\r", ch );
               return rSPELL_FAILED;
            }
            if( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
               
            {
               send_to_char( "It contains some other liquid.\n\r", ch );
               return rSPELL_FAILED;
            }
            water = UMIN( ( skill->dice ? dice_parse( ch, skill->dice ) : level ) 
                            *( weath->precip >= 0 ? 2 : 1 ), obj->value[0] - obj->value[1] );
            if( water > 0 )
               
            {
               separate_obj( obj );
               obj->value[2] = LIQ_WATER;
               obj->value[1] += water;
               if( !is_name( "water", obj->name ) )
                  
               {
                  char buf[MAX_STRING_LENGTH];
                  sprintf( buf, "%s water", obj->name );
                  STRFREE( obj->name );
                  obj->name = STRALLOC( buf );
               }
            }
            successful_casting( skill, ch, NULL, obj );
            return rNONE;
         }
         if( SPELL_DAMAGE( skill ) == SD_FIRE )   /* burn object */
            
         {
            
               /*
                * return rNONE; 
                */ 
         }
         if( SPELL_DAMAGE( skill ) == SD_POISON /* poison object */  
              ||SPELL_CLASS( skill ) == SC_DEATH )
            
         {
            switch ( obj->item_type )
               
            {
               default:
                  failed_casting( skill, ch, NULL, obj );
                  break;
               case ITEM_COOK:
               case ITEM_FOOD:
               case ITEM_DRINK_CON:
                  separate_obj( obj );
                  obj->value[3] = 1;
                  successful_casting( skill, ch, NULL, obj );
                  break;
            }
            return rNONE;
         }
         if( SPELL_CLASS( skill ) == SC_LIFE /* purify food/water */  
              &&( obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON  ||obj->item_type == ITEM_COOK ) )
            
         {
            switch ( obj->item_type )
               
            {
               default:
                  failed_casting( skill, ch, NULL, obj );
                  break;
               case ITEM_COOK:
               case ITEM_FOOD:
               case ITEM_DRINK_CON:
                  separate_obj( obj );
                  obj->value[3] = 0;
                  successful_casting( skill, ch, NULL, obj );
                  break;
            }
            return rNONE;
         }
         if( SPELL_CLASS( skill ) != SC_NONE )
            
         {
            failed_casting( skill, ch, NULL, obj );
            return rNONE;
         }
         switch ( SPELL_POWER( skill ) ) /* clone object */
            
         {
               OBJ_DATA * clone;
            default:
            case SP_NONE:
               if( obj->cost > ( get_curr_int( ch ) * get_curr_wis( ch ) ) / 5 )
                  
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               break;
            case SP_MINOR:
               if( obj->cost > ( get_curr_int( ch ) * get_curr_wis( ch ) ) / 3 )
                  
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               break;
            case SP_GREATER:
               if( obj->cost > ( get_curr_int( ch ) * get_curr_wis( ch ) ) / 2 )
                  
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               break;
            case SP_MAJOR:
               if( obj->cost > ( get_curr_int( ch ) * get_curr_wis( ch ) ) )
                  
               {
                  failed_casting( skill, ch, NULL, obj );
                  return rNONE;
               }
               break;
               clone = clone_object( obj );
               clone->timer = skill->dice ? dice_parse( ch, skill->dice ) : 0;
               obj_to_char( clone, ch );
               successful_casting( skill, ch, NULL, obj );
         }
         return rNONE;
      case SA_DESTROY:
      case SA_RESIST:
      case SA_SUSCEPT:
      case SA_DIVINATE:
         if( SPELL_DAMAGE( skill ) == SD_POISON ) /* detect poison */
            
         {
            if( obj->item_type == ITEM_DRINK_CON  ||obj->item_type == ITEM_FOOD  ||obj->item_type == ITEM_COOK )
               
            {
               if( obj->item_type == ITEM_COOK && obj->value[2] == 0 )
                  send_to_char( "It looks undercooked.\n\r", ch );
               
               else if( obj->value[3] != 0 )
                  send_to_char( "You smell poisonous fumes.\n\r", ch );
               
               else
                  send_to_char( "It looks very delicious.\n\r", ch );
            }
            
            else
               send_to_char( "It doesn't look poisoned.\n\r", ch );
            return rNONE;
         }
         return rNONE;
      case SA_OBSCURE:   /* make obj invis */
         if( IS_OBJ_STAT( obj, ITEM_INVIS )  ||chance( ch, skill->dice ? dice_parse( ch, skill->dice ) : 20 ) )
            
         {
            failed_casting( skill, ch, NULL, NULL );
            return rSPELL_FAILED;
         }
         successful_casting( skill, ch, NULL, obj );
         xSET_BIT( obj->extra_flags, ITEM_INVIS );
         return rNONE;
      case SA_CHANGE:
         return rNONE;
   }
   return rNONE;
}


/*
 * Generic object creating spell				-Thoric
 */ 
   ch_ret spell_create_obj( int sn, int level, CHAR_DATA * ch, void *vo )
{
   SKILLTYPE * skill = get_skilltype( sn );
   int vnum = skill->value;
   OBJ_DATA * obj;
   OBJ_INDEX_DATA * oi;
   
      /*
       * 
       * * Add predetermined objects here
       */ 
      if( ( oi = get_obj_index( vnum ) ) == NULL  ||( obj = create_object( oi, 0 ) ) == NULL )
      
   {
      failed_casting( skill, ch, NULL, NULL );
      return rNONE;
   }
   obj->timer = skill->dice ? dice_parse( ch, skill->dice ) : 0;
   successful_casting( skill, ch, NULL, obj );
   if( !IS_OBJ_STAT( obj, ITEM_NO_TAKE ) )
      obj_to_char( obj, ch );
   
   else
      obj_to_room( obj, ch->in_room );
   return rNONE;
}


/*
 * Generic mob creating spell					-Thoric
 */ 
   ch_ret spell_create_mob( int sn, int level, CHAR_DATA * ch, void *vo )
{
   SKILLTYPE * skill = get_skilltype( sn );
   int vnum = skill->value;
   CHAR_DATA * mob;
   MOB_INDEX_DATA * mi;
   AFFECT_DATA af;
   
      /*
       * 
       * * Add predetermined mobiles here
       */ 
      if( vnum == 0 )
      
   {
      if( !str_cmp( target_name, "cityguard" ) )
         vnum = MOB_VNUM_CITYGUARD;
      if( !str_cmp( target_name, "vampire" ) )
         vnum = MOB_VNUM_VAMPIRE;
   }
   if( ( mi = get_mob_index( vnum ) ) == NULL  ||( mob = create_mobile( mi ) ) == NULL )
      
   {
      failed_casting( skill, ch, NULL, NULL );
      return rNONE;
   }
   mob->max_hit = get_curr_con( mob ) * 25;
   mob->hit = mob->max_hit;
   mob->gold = 0;
   successful_casting( skill, ch, mob, NULL );
   char_to_room( mob, ch->in_room );
   add_follower( mob, ch );
   af.type = sn;
   af.duration = ( number_fuzzy( ( level + 1 ) / 3 ) + 1 ) * DUR_CONV;
   af.location = 0;
   af.modifier = 0;
   af.bitvector = meb( AFF_CHARM );
   affect_to_char( mob, &af );
   return rNONE;
}


/*
 * Generic handler for new "SMAUG" spells			-Thoric
 */ 
   ch_ret spell_smaug( int sn, int level, CHAR_DATA * ch, void *vo )
{
   char arg2[MAX_STRING_LENGTH];
   struct skill_type *skill = get_skilltype( sn );
   
/* added bad skill check - shogar */ 
      if( !skill || sn == -1 )
      
   {
      if( ch && ch->name && ch->in_room && ch->in_room->vnum )
         bug( "bad skill %d passed to spell_smaug: ch->name %s, room %d", sn, ch->name, ch->in_room->vnum );
      return rNONE;
   }
   if( ranged_target_name )
      one_argument( ranged_target_name, arg2 );
   if( SPELL_FLAG( skill, SF_EARTH )  &&( ch->in_room->curr_vegetation < 33 ) )
   {
      act( AT_GREEN, "There isn't enough vegetation here to cast this spell.", ch, NULL, NULL, TO_CHAR );
      return rNONE;
   }
   if( SPELL_FLAG( skill, SF_AIR )  &&( ch->in_room->curr_water >= 100 ) )
   {
      act( AT_CYAN, "There isn't enough air here to cast this spell.", ch, NULL, NULL, TO_CHAR );
      act( AT_CYAN, "Some water bubbles float up toward the surface.", ch, NULL, NULL, TO_ROOM );
      return rNONE;
   }
   if( SPELL_FLAG( skill, SF_WATER )  &&( ch->in_room->curr_water < 33 ) )
   {
      act( AT_BLUE, "There isn't enough water here to cast this spell.", ch, NULL, NULL, TO_CHAR );
      return rNONE;
   }
   if( SPELL_FLAG( skill, SF_ASTRAL )  &&( !IS_AFFECTED( ch, AFF_DREAMWORLD ) ) )
   {
      send_to_char( "You must be in the dreamworld to cast this spell.\n\r", ch );
      return rNONE;
   }
   if( SPELL_FLAG( skill, SF_GROUND )  &&( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) ) )
   {
      send_to_char( "There needs to be ground nearby to cast this spell.\n\r", ch );
      return rNONE;
   }
   switch ( skill->target )
      
   {
      case TAR_IGNORE:
         
            /*
             * offensive area spell 
             */ 
            if( SPELL_FLAG( skill, SF_AREA ) 
                &&( ( SPELL_ACTION( skill ) == SA_DESTROY 
                      &&SPELL_CLASS( skill ) == SC_LIFE ) 
                    ||( SPELL_ACTION( skill ) == SA_CREATE  &&SPELL_CLASS( skill ) == SC_DEATH ) ) )
            return spell_area_attack( sn, level, ch, vo );
         if( SPELL_ACTION( skill ) == SA_CREATE )
            
         {
            if( SPELL_FLAG( skill, SF_OBJECT ) )  /* create object */
               return spell_create_obj( sn, level, ch, vo );
            if( SPELL_CLASS( skill ) == SC_LIFE ) /* create mob */
               return spell_create_mob( sn, level, ch, vo );
         }
         
            /*
             * affect a distant player 
             */ 
            if( SPELL_FLAG( skill, SF_DISTANT )  &&SPELL_FLAG( skill, SF_CHARACTER ) )
            return spell_affect( sn, level, ch, get_char_world( ch, target_name ) );
         
            /*
             * affect a player in this room (should have been TAR_CHAR_XXX) 
             */ 
            if( SPELL_FLAG( skill, SF_CHARACTER ) )
            return spell_affect( sn, level, ch, get_char_room( ch, target_name ) );
         if( skill->range > 0 && find_door( ch, arg2, TRUE ) != NULL && ( 
                                                                            ( SPELL_ACTION( skill ) == SA_DESTROY 
                                                                              &&SPELL_CLASS( skill ) == SC_LIFE ) 
                                                                            ||( SPELL_ACTION( skill ) == SA_CREATE 
                                                                                &&SPELL_CLASS( skill ) == SC_DEATH ) ) )
            
               /*
                * will fail, or be an area/group affect 
                */ 
               return spell_affect( sn, level, ch, vo );
      case TAR_CHAR_OFFENSIVE:
         if( ( skill->range > 0 ) 
               &&ranged_target_name 
               &&find_door( ch, arg2, TRUE ) != NULL 
               &&( ( SPELL_ACTION( skill ) == SA_DESTROY 
                     &&SPELL_CLASS( skill ) == SC_LIFE ) 
                   ||( SPELL_ACTION( skill ) == SA_CREATE 
                       &&SPELL_CLASS( skill ) == SC_DEATH ) )  &&get_char_room( ch, ranged_target_name ) == NULL )
            
               /*
                * a regular damage inflicting spell attack 
                */ 
               if( ( SPELL_ACTION( skill ) == SA_DESTROY && SPELL_CLASS( skill ) == SC_LIFE ) 
                   ||( SPELL_ACTION( skill ) == SA_CREATE && SPELL_CLASS( skill ) == SC_DEATH ) )
               return spell_attack( sn, level, ch, vo );
         
            /*
             * a nasty spell affect 
             */ 
            return spell_affect( sn, level, ch, vo );
      case TAR_CHAR_DEFENSIVE:
      case TAR_CHAR_SELF:
         return spell_affect( sn, level, ch, vo );
      case TAR_OBJ_INV:
         return spell_obj_inv( sn, level, ch, vo );
   }
   return rNONE;
}


/* tap <amount> - draws <amount> mana from a zone into a char, plus or minus a random element */ 
void do_tap( CHAR_DATA * ch, char *argument )
{
   int amount;
   char arg[MAX_INPUT_LENGTH];
   argument = one_argument( argument, arg );
   amount = atoi( arg );
   if( amount < 1 )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }
   if( amount > ch->max_mana )
   {
      send_to_char( "That might be harmful to your health.\r\n", ch );
      return;
   }
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( "tapping mana" );
   ch->wait = UMAX( ch->wait, ( amount / ( 100 + TALENT( ch, TAL_CATALYSM ) ) ) );
   amount += number_range( -10, ( ch->in_room->area->weather->mana * 10 ) - amount );
   if( amount < 0 )
   {
      act( AT_MAGIC, "There seems to be a powerful mana vaccuum here!", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n concentrates, then looks startled as $s body glows white for an instant.", ch, NULL, NULL,
            TO_ROOM );
   }
   else
   {
      act( AT_MAGIC, "You open yourself to the surroundings, letting extra mana flow into you.", ch, NULL, NULL, TO_CHAR );
      act( AT_MAGIC, "$n concentrates, then gets a content look on $s face.", ch, NULL, NULL, TO_ROOM );
   }
   ch->mana += amount;
   ch->in_room->area->weather->mana -= ( int )amount / 100;
} void do_link( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   if( ch->curr_talent[TAL_CATALYSM] < 100 )
   {
      huh( ch );
      return;
   }
   victim = find_target( ch, argument, FALSE );
   if( !victim )
      return;
   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't link with a mensch!\n\r", ch );
      return;
   }
   if( victim == ch )
   {
      if( ch->pcdata->magiclink )
      {
         act( AT_MAGIC, "$n breaks off the magic link.", ch, NULL, victim, TO_VICT );
         act( AT_MAGIC, "You break off the magic link with $N.", ch, NULL, ch->pcdata->magiclink, TO_CHAR );
         ch->pcdata->magiclink->pcdata->magiclink = NULL;
         ch->pcdata->magiclink = NULL;
      }
      else
      {
         send_to_char( "You are not linked to anyone.\n\r", ch );
      }
      return;
   }
   if( ch->pcdata->magiclink )
   {
      send_to_char( "You are already linked with somebody.\n\r", ch );
      return;
   }
   if( victim->pcdata->magiclink )
   {
      send_to_char( "They are already linked with somebody.\n\r", ch );
      return;
   }
   if( IS_NPC( victim ) )
   {
      send_to_char( "They have no magic to link with.\n\r", ch );
      return;
   }
   if( !IS_CONSENTING( victim, ch ) )
   {
      send_to_char( "You cannot link your magic with them without their consent.\n\r", ch );
      return;
   }
   act( AT_MAGIC, "You link your magic together with $N.", ch, NULL, victim, TO_CHAR );
   act( AT_MAGIC, "$n links $s magic together with you.", ch, NULL, victim, TO_VICT );
   ch->pcdata->magiclink = victim;
   victim->pcdata->magiclink = ch;
}


/* Return TRUE if it stops you from casting the spell */ 
   bool check_manastorm( CHAR_DATA * ch )
{
   WEATHER_DATA * weath;
   int storm;
   weath = ch->in_room->area->weather;
   if( weath->mana < 200 )
      return FALSE;
   if( weath->wind < 200 )
      return FALSE;
   storm = weath->wind + weath->precip;
   storm *= weath->mana;
   if( storm < 4000 )
      return FALSE;
   switch ( number_range( 1, 5 ) )
   {
      case 1:
         act( AT_CHAOS, "The errant mana surges through your being!", ch, NULL, NULL, TO_CHAR );
         ch->mana += 100 + ( int )storm / 4000;
         return TRUE;
      case 2:
         act( AT_CHAOS, "White-violet lightning sears your being!", ch, NULL, NULL, TO_CHAR );
         act( AT_CHAOS, "White-violet lightning sears $n's being!", ch, NULL, NULL, TO_ROOM );
         lose_hp( ch, 100 + ( int )storm / 4000 );
         return TRUE;
      default:
         if( number_range( 1, get_curr_wil( ch ) ) < 100 )
         {
            act( AT_CHAOS, "You are unable to control the mana flows.", ch, NULL, NULL, TO_CHAR );
            return TRUE;
         }
         else
         {
            act( AT_CHAOS, "With great concentration you control the mana through the storm.", ch, NULL, NULL, TO_CHAR );
            return FALSE;
         }
   }
   return TRUE;
}


