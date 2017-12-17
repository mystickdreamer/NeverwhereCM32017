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
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* handler.c */
extern NATION_DATA *find_nation( char *name );
extern MUT_DATA *find_mutation( int i );

/*
 *  Locals
 */
char *tiny_affect_loc_name( int location );

/* For TinyMUD Bots! -- Scion */
void do_outputprefix( CHAR_DATA * ch, char *argument )
{
   if( strlen( argument ) > 40 )
   {
      send_to_char( "Sorry, that string is too long!\r\n", ch );
      return;
   }

   if( ch->pcdata->outputprefix )
      STRFREE( ch->pcdata->outputprefix );
   ch->pcdata->outputprefix = STRALLOC( argument );
}

void do_outputsuffix( CHAR_DATA * ch, char *argument )
{
   if( strlen( argument ) > 40 )
   {
      send_to_char( "Sorry, that string is too long!\r\n", ch );
      return;
   }

   if( ch->pcdata->outputsuffix )
      STRFREE( ch->pcdata->outputsuffix );
   ch->pcdata->outputsuffix = STRALLOC( argument );
}


void do_gold( CHAR_DATA * ch, char *argument )
{
   unsigned long gp;

   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
   {
      ch_printf( ch, "&YYou have %ld total currency.\n\r", ch->gold );
      ch_printf( ch, "&YYour bank account contains %ld total currency.\n\r\n\r", ch->pcdata->balance );
   }

   gp = ch->gold;
   ch_printf( ch, "&CYou have %d mithril pieces.\n\r", gp / 1000000 );
   gp = gp % 1000000;
   ch_printf( ch, "&YYou have %d gold pieces.\n\r", gp / 10000 );
   gp = gp % 10000;
   ch_printf( ch, "&WYou have %d silver pieces.\n\r", gp / 100 );
   gp = gp % 100;
   ch_printf( ch, "&OYou have %d copper pieces.\n\r", gp );

   ch_printf( ch, "\n\r&YYour bank account contains:\n\r" );
   gp = ch->pcdata->balance;
   ch_printf( ch, "&C%d mithril coins\n\r", gp / 1000000 );
   gp = gp % 1000000;
   ch_printf( ch, "&Y%d gold coins\r\n", gp / 10000 );
   gp = gp % 10000;
   ch_printf( ch, "&W%d silver coins\n\r", gp / 100 );
   gp = gp % 100;
   ch_printf( ch, "&O%d copper coins\n\r", gp );
}

/*
 * New score command by Haus, heavily modified by Scion
 */
void do_score( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int worth = get_char_worth( ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "Don't worry about it, you're fine.\r\n", ch );
      return;
   }
   set_pager_color( AT_SCORE, ch );

   pager_printf( ch, "\n\r&CScore for: &W%s&C.\n\r", ch->pcdata->title );

   send_to_pager( "----------------------------------------------------------------------------\n\r", ch );

   pager_printf( ch, "&CRace : &W%-15.15s &CRank: &W%s &CPlayed: &W%d &Chours\n\r",
                 capitalize( ch->species ),
                 worth < 1500 ? "Apprentice" :
                 worth < 2000 ? "Novice" :
                 worth < 2500 ? "Mage" :
                 worth < 2750 ? "Wizard" :
                 worth < 3000 ? "Sorcerer" :
                 worth < 3250 ? "Archmage" :
                 worth < 3500 ? "Elkandu" :
                 worth < 3750 ? "Master" :
                 worth < 4000 ? "Lord" :
                 worth < 4500 ? "High Lord" :
                 worth < 5000 ? "Demigod" : "God", ( ( ch->played + ( current_time - ch->logon ) ) / 3600 ) );

   pager_printf( ch, "&CAGE  : &W%d years                            &CLog In: &W%s\r",
                 ch->pcdata->age_adjust, ctime( &( ch->logon ) ) );

   pager_printf( ch, "&CSTR  : &W%3d&C(&w%3d&C)    HitRoll: &R%-4d             &CSaved: &W%s\r",
                 get_curr_str( ch ), ch->perm_str, GET_HITROLL( ch ),
                 ch->save_time ? ctime( &( ch->save_time ) ) : "no save this session\n" );

   pager_printf( ch, "&CINT  : &W%3d&C(&w%3d&C)    DamRoll: &R%-4d              &CTime: &W%s\r",
                 get_curr_int( ch ), ch->perm_int, GET_DAMROLL( ch ), ctime( &current_time ) );

   if( GET_AC( ch ) < 0 )
      sprintf( buf, "vulnerable" );
   else if( GET_AC( ch ) < 20 )
      sprintf( buf, "unprotected" );
   else if( GET_AC( ch ) < 40 )
      sprintf( buf, "lightly shielded" );
   else if( GET_AC( ch ) < 80 )
      sprintf( buf, "somewhat shielded" );
   else if( GET_AC( ch ) < 200 )
      sprintf( buf, "moderately shielded" );
   else if( GET_AC( ch ) < 400 )
      sprintf( buf, "strongly shielded" );
   else if( GET_AC( ch ) < 600 )
      sprintf( buf, "extremely shielded" );
   else if( GET_AC( ch ) < 800 )
      sprintf( buf, "powerfully shielded" );
   else if( GET_AC( ch ) < 1000 )
      sprintf( buf, "incredibly shielded" );
   else
      sprintf( buf, "almost invulnerable" );

   pager_printf( ch, "&CWIL  : &W%3d&C(&w%3d&C)      Armor: &W%s &C(&w%4.4d&C)\n\r",
                 get_curr_wis( ch ), ch->perm_wil, buf, GET_AC( ch ) );

   pager_printf( ch, "&CDEX  : &W%3d&C(&w%3d&C)    &CItems: &W%5.5d   &C(&wmax %5.5d&C)\n\r",
                 get_curr_dex( ch ), ch->perm_dex, ch->carry_number, can_carry_n( ch ) );

   pager_printf( ch, "&CCON  : &W%3d&C(&w%3d&C)      Pos'n: &W%-21.21s  &CWeight: &W%5.5d &C(&wmax %7.7d&C)\n\r",
                 get_curr_con( ch ), ch->perm_con, pos_names[ch->position], ch->carry_weight, can_carry_w( ch ) );

   pager_printf( ch, "&CPER  : &W%3d&C(&w%3d&C)      Speed: &W%-3.3d                  &CEncumberance: &W%d\r\n",
                 get_curr_cha( ch ), ch->perm_per, ch->speed, ch->encumberance );

   pager_printf( ch, "&CEND  : &W%3d&C(&w%3d&C) \n\r", get_curr_end( ch ), ch->perm_lck );

   pager_printf( ch, "&CTP   : &W%4.4d                &CPager: [&W%c&C][&W%3d &wlines&C] \n\r",
                 ch->pcdata->points, ( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) ? '*' : ' ' ), ch->pcdata->pagerlen );

   pager_printf( ch, "&CHit: &R%-5d &Cof &r%-5d &CAutoExit[&W%c&C]\n\r",
                 ch->hit, ch->max_hit, xIS_SET( ch->act, PLR_AUTOEXIT ) ? '*' : ' ' );

   pager_printf( ch, "&CMana: &B%-5d &Cof &b%-5d &CAutoLoot[&W%c&C]\n\r",
                 ch->mana, ch->max_mana, xIS_SET( ch->act, PLR_AUTOLOOT ) ? '*' : ' ' );

   pager_printf( ch, "&CEnergy: &W%-5d &Cof &w%-5d\n\r", ch->move, ch->max_move );

   if( ch->pcdata->permissions )
      pager_printf( ch, "&CYou have the permissions: &Y%s&C.\n\r", flag_string( ch->pcdata->permissions, permit_flags ) );

   send_to_pager( "&C---------------------------------------------------------------------------\n\r", ch );
   if( ch->pcdata->family )
      pager_printf( ch, "&CFamily: &W%s &CGeneration: &W%d ", ch->pcdata->family, ch->pcdata->gen );
   if( ch->pcdata->parent )
      pager_printf( ch, "&CParent: &W%s ", ch->pcdata->parent );
   if( ch->pcdata->spouse )
      pager_printf( ch, "&CSpouse: &W%s ", ch->pcdata->spouse );
   pager_printf( ch, "\n\r&CNonPC DATA:  Mkills (&W%5.5d&C)   Mdeaths (&W%5.5d&C)", ch->pcdata->mkills,
                 ch->pcdata->mdeaths );
   pager_printf( ch, "\n\r&CSPAR DATA :  Wins (&W%3.3d&C)     Losses (&W%3.3d&C)", ch->pcdata->pkills, ch->pcdata->pdeaths );
   pager_printf( ch, "\n\r&CPKILL DATA:  Wins (&W%3.3d&C)     Losses (&W%3.3d&C)", ch->pcdata->rkills, ch->pcdata->rdeaths );

   if( IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
   {
      send_to_pager( "&C\n\r----------------------------------------------------------------------------\n\r", ch );

      pager_printf( ch, "&CBUILDER DATA:\n\r" );

      pager_printf( ch, "&CBamfin:  &Y%s%s\n\r", ( ( ch->pcdata->bamfin ) ? "" : ch->name ),
                    ( ch->pcdata->bamfin ) ? ch->pcdata->bamfin : " appears in a swirling mist." );
      pager_printf( ch, "&CBamfout: &Y%s%s\n\r", ( ( ch->pcdata->bamfout ) ? "" : ch->name ),
                    ( ch->pcdata->bamfout ) ? ch->pcdata->bamfout : " leaves in a swirling mist." );


      /*
       * Area Loaded info - Scryn 8/11
       */
      if( ch->pcdata->area )
      {
         pager_printf( ch,
                       "&CVnums:   Room (&W%-5.5d &C- &W%-5.5d&C)   Object (&W%-5.5d &C- &W%-5.5d&C)   Mob (&W%-5.5d &C- &W%-5.5d&C)\n\r",
                       ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum, ch->pcdata->area->low_o_vnum,
                       ch->pcdata->area->hi_o_vnum, ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum );
         pager_printf( ch, "&CArea Loaded [&W%s&C]\n\r",
                       ( IS_SET( ch->pcdata->area->status, AREA_LOADED ) ) ? "yes" : "no" );
      }
   }
   send_to_pager( "\n\r", ch );
   return;
}

/*
 * Return ascii name of an affect location.
 */
char *tiny_affect_loc_name( int location )
{
   switch ( location )
   {
      case APPLY_NONE:
         return "NIL";
      case APPLY_STR:
         return " STR  ";
      case APPLY_DEX:
         return " DEX  ";
      case APPLY_INT:
         return " INT  ";
      case APPLY_WIS:
         return " WIL  ";
      case APPLY_CON:
         return " CON  ";
      case APPLY_CHA:
         return " PER  ";
      case APPLY_LCK:
         return " END  ";
      case APPLY_SEX:
         return " SEX  ";
      case APPLY_AGE:
         return " AGE  ";
      case APPLY_MANA:
         return " MANA ";
      case APPLY_HIT:
         return " HV   ";
      case APPLY_MOVE:
         return " MOVE ";
      case APPLY_GOLD:
         return " GOLD ";
      case APPLY_EXP:
         return " EXP  ";
      case APPLY_AC:
         return " AC   ";
      case APPLY_HITROLL:
         return " HITRL";
      case APPLY_DAMROLL:
         return " DAMRL";
      case APPLY_SAVING_POISON:
         return "SV POI";
      case APPLY_SAVING_ROD:
         return "SV ROD";
      case APPLY_SAVING_PARA:
         return "SV PARA";
      case APPLY_SAVING_BREATH:
         return "SV BRTH";
      case APPLY_SAVING_SPELL:
         return "SV SPLL";
      case APPLY_HEIGHT:
         return "HEIGHT";
      case APPLY_WEIGHT:
         return "WEIGHT";
      case APPLY_AFFECT:
         return "AFF BY";
      case APPLY_RESISTANT:
         return "RESIST";
      case APPLY_IMMUNE:
         return "IMMUNE";
      case APPLY_SUSCEPTIBLE:
         return "SUSCEPT";
      case APPLY_WEAPONSPELL:
         return " WEAPON";
      case APPLY_FIRE_MAGIC:
         return " FIRE  ";
      case APPLY_EARTH_MAGIC:
         return " EARTH ";
      case APPLY_WIND_MAGIC:
         return " WIND  ";
      case APPLY_FROST_MAGIC:
         return " FROST ";
      case APPLY_WATER_MAGIC:
         return " WATER ";
      case APPLY_LIGHTNING_MAGIC:
         return "LIGHTNG";
      case APPLY_DREAM_MAGIC:
         return " DREAM ";
      case APPLY_HEALING_MAGIC:
         return "HEALING";
      case APPLY_DEATH_MAGIC:
         return " DEATH ";
      case APPLY_TIME_MAGIC:
         return " TIME  ";
      case APPLY_CHANGE_MAGIC:
         return " CHANGE";
      case APPLY_MOTION_MAGIC:
         return " MOTION";
      case APPLY_MIND_MAGIC:
         return " MIND  ";
      case APPLY_ILLUSION_MAGIC:
         return " ILLUSN";
      case APPLY_SEEKING_MAGIC:
         return "SEEKING";
      case APPLY_CATALYSM_MAGIC:
         return "CATALSM";
      case APPLY_VOID_MAGIC:
         return " VOID  ";
      case APPLY_WEARSPELL:
         return " WEAR  ";
      case APPLY_REMOVESPELL:
         return " REMOVE";
      case APPLY_EMOTION:
         return "EMOTION";
      case APPLY_MENTALSTATE:
         return " MENTAL";
      case APPLY_STRIPSN:
         return " DISPEL";
      case APPLY_REMOVE:
         return " REMOVE";
      case APPLY_DIG:
         return " DIG   ";
      case APPLY_FULL:
         return " HUNGER";
      case APPLY_THIRST:
         return " THIRST";
      case APPLY_DRUNK:
         return " DRUNK ";
      case APPLY_BLOOD:
         return " BLOOD ";
      case APPLY_COOK:
         return " COOK  ";
      case APPLY_RECURRINGSPELL:
         return " RECURR";
      case APPLY_CONTAGIOUS:
         return "CONTGUS";
      case APPLY_ODOR:
         return " ODOR  ";
      case APPLY_ROOMFLAG:
         return " RMFLG ";
      case APPLY_SECTORTYPE:
         return " SECTOR";
      case APPLY_ROOMLIGHT:
         return " LIGHT ";
      case APPLY_TELEVNUM:
         return " TELEVN";
      case APPLY_TELEDELAY:
         return " TELEDY";
   };

   bug( "Affect_location_name: unknown location %d.", location );
   return "(?)";
}


char *get_race( CHAR_DATA * ch )
{
   if( ch->species )
   {
      if( !ch->nation )
         ch->nation = find_nation( ch->species );
      if( !ch->nation )
         return ( npc_race[ch->race] );
      else
         return ( ch->nation->name );
   }
   else
      return ( npc_race[ch->race] );
   return ( "Unknown" );
}

/* Affects-at-a-glance, Blodkai */
void do_affected( CHAR_DATA * ch, char *argument )
{
   PMUT_DATA *pmut;
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   if( IS_NPC( ch ) )
      return;

   set_char_color( AT_SCORE, ch );

   if( !ch->first_affect && !ch->pcdata->first_mutation )
   {
      send_to_char_color( "\n\r&CNo cantrip, skill, or mutation affects you.\n\r", ch );
   }
   else
   {
      send_to_char( "\n\r", ch );
      for( paf = ch->first_affect; paf; paf = paf->next )
      {
         if( ( skill = get_skilltype( paf->type ) ) != NULL )
         {
            set_char_color( AT_SCORE, ch );
            ch_printf( ch,
                       "&B%s: &C'%s' mods %s by %d for %d rnds, imbues %s.\n\r",
                       skill_tname[skill->type],
                       skill->name,
                       affect_loc_name( paf->location ), paf->modifier, paf->duration, affect_bit_name( &paf->bitvector ) );
         }
      }
      send_to_char( "\n\r", ch );
      for( pmut = ch->pcdata->first_mutation; pmut; pmut = pmut->next )
      {
         ch_printf( ch, "&B%s\n\r", pmut->mut->desc );
      }
   }

   send_to_char_color( "\n\r&BImbued with:\n\r", ch );
   ch_printf_color( ch, "&C%s\n\r", !xIS_EMPTY( ch->affected_by ) ? affect_bit_name( &ch->affected_by ) : "nothing" );
   send_to_char_color( "\n\r&BPermanent affects:\n\r", ch );
   ch_printf_color( ch, "&C%s\n\r",
                    !xIS_EMPTY( ch->pcdata->perm_aff ) ? affect_bit_name( &ch->pcdata->perm_aff ) : "nothing" );
   send_to_char( "\n\r", ch );
   if( ch->resistant > 0 )
   {
      send_to_char_color( "&BResistances:  ", ch );
      ch_printf_color( ch, "&C%s\n\r", flag_string( ch->resistant, ris_flags ) );
   }
   if( ch->immune > 0 )
   {
      send_to_char_color( "&BImmunities:   ", ch );
      ch_printf_color( ch, "&C%s\n\r", flag_string( ch->immune, ris_flags ) );
   }
   if( ch->susceptible > 0 )
   {
      send_to_char_color( "&BSuscepts:     ", ch );
      ch_printf_color( ch, "&C%s\n\r", flag_string( ch->susceptible, ris_flags ) );
   }

   return;
}

/* This function will add a specific mutation to a player */

bool add_mutation( CHAR_DATA * ch, int i )
{
/*	PMUT_DATA *pmut;
	MUT_DATA *mut;

	if (IS_NPC(ch)) return FALSE;

	* If none specified, do a random one. *
	if (!i)
		i = number_range(1, last_mutation->number);

	* Check to see if they already have it
	 * Perhaps in the future make it more severe?
	 *
	pmut = ch->pcdata->first_mutation;
	while (pmut && pmut->mut) {
		if (pmut->mut->number == i)
			return FALSE;
		pmut = pmut->next;
	}

	if ((mut = find_mutation(i)) == NULL) {
		bug("Add_mutation: Invalid mutation", 0);
		return FALSE;
	}

	CREATE(pmut, PMUT_DATA, 1);
	pmut->mut = mut;
	pmut->duration = -1;	* if we want a duration, set it elsewhere *
	LINK(pmut, ch->pcdata->first_mutation, ch->pcdata->last_mutation, next, prev);

	if (mut->gain_self)
		act(AT_CHAOS, mut->gain_self, ch, NULL, NULL, TO_CHAR);
	if (mut->gain_other)
		act(AT_CHAOS, mut->gain_other, ch, NULL, NULL, TO_ROOM);
	return TRUE;
*/
   return FALSE;
}

void do_inventory( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "You are carrying:\n\r", ch );
   show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
   return;
}

void do_weapons( CHAR_DATA * ch, char *argument )
{
   int i;

   if( IS_NPC( ch ) )
      return;

   for( i = 0; i < 20; i++ )
   {
      if( xIS_SET( ch->act, PLR_SHOW_EXP ) && ch->pcdata->weapon[i] < 100 )
      {
         pager_printf_color( ch, "&c%-20s: &w%-3d, %d experience needed\n\r",
                             weapon_skill[i],
                             ch->pcdata->weapon[i],
                             exp_level( ch, ch->pcdata->weapon[i] + 1 ) - ch->pcdata->weapon_exp[i]
                             - exp_level( ch, ch->pcdata->weapon[i] ) );
      }
      else
      {
         pager_printf_color( ch, "&c%-20s: &w%-3d\n\r", weapon_skill[i], ch->pcdata->weapon[i] );
      }
   }
}

void do_skills( CHAR_DATA * ch, char *argument )
{
   int sn;
   

   if( IS_NPC( ch ) )
      return;

	  
	        int col;
      sh_int lasttype, cnt;

      col = cnt = 0;
      lasttype = SKILL_SPELL;
      set_pager_color( AT_MAGIC, ch );
      for( sn = 0; sn < top_sn; sn++ )
      {
         if( !skill_table[sn]->name )
            break;

         /*if( strcmp( skill_table[sn]->name, "reserved" ) == 0 && ( CAN_CAST( ch ) ) )
         {
            if( col % 3 != 0 )
               send_to_pager( "\n\r", ch );
            set_pager_color( AT_MAGIC, ch );
            send_to_pager_color( " ----------------------------------[&CSpells&B]----------------------------------\n\r",
                                 ch );
            col = 0;
         }*/
         if( skill_table[sn]->type != lasttype )
         {
            if( !cnt )
               //send_to_pager( "                                   (none)\n\r", ch );
			   send_to_pager( "\n\r", ch );
            else if( col % 3 != 0 )
               send_to_pager( "\n\r", ch );
            set_pager_color( AT_MAGIC, ch );
            pager_printf_color( ch,
                                " ----------------------------------&C%ss&B-------------------------------------\n\r",
                                skill_tname[skill_table[sn]->type] );
            col = cnt = 0;
         }
         lasttype = skill_table[sn]->type;

         if( skill_available( ch, sn ) == FALSE )
            continue;

         if( ( LEARNED( ch, sn ) < 1 ) && SPELL_FLAG( skill_table[sn], SF_SECRETSKILL ) )
            continue;

         if( lasttype != SKILL_SPELL && LEARNED( ch, sn ) < 1 )
            continue;

         ++cnt;
         set_pager_color( AT_MAGIC, ch );
         pager_printf( ch, "%20.20s", skill_table[sn]->name );

         if( LEARNED( ch, sn ) > 0 )
            set_pager_color( AT_SCORE, ch );
         pager_printf( ch, " %3d%% ", LEARNED( ch, sn ) );

         if( ++col % 3 == 0 )
            send_to_pager( "\n\r", ch );
      }
	  
	  set_pager_color( AT_MAGIC, ch );
	  pager_printf_color( ch,
                                " ----------------------------------&CNonCombat Skills&B----------------------------\n\r");
	  pager_printf_color(ch, " 	    Smithing: &C%5d&B	Alchemy: &C%5d&B		Tailoring: &C%5d&B\n\r", ch->pcdata->noncombat[SK_SMITH], ch->pcdata->noncombat[SK_ALCHEMY], ch->pcdata->noncombat[SK_TAILOR]);
	  pager_printf_color(ch, " 	      Mining: &C%5d&B	 Nature: &C%5d&B		  Jewelry: &C%5d&B\n\r", ch->pcdata->noncombat[SK_MINING], ch->pcdata->noncombat[SK_NATURE], ch->pcdata->noncombat[SK_JEWEL]);
	  pager_printf_color(ch, " 	     Stealth: &C%5d&B       Medicine: &C%5d&B	       Streetwise: &C%5d&B\n\r", ch->pcdata->noncombat[SK_STEALTH], ch->pcdata->noncombat[SK_MEDICINE], ch->pcdata->noncombat[SK_STREET]);
	  pager_printf_color(ch, " 	  Channeling: &C%5d&B\n\r", ch->pcdata->noncombat[SK_CHANNEL]);


/* This is the old way to list non combat skills
	  for( i = 0; i < 10; i++ )
   {
      if( xIS_SET( ch->act, PLR_SHOW_EXP ) && ch->pcdata->noncombat[i] < 100 )
      {
         pager_printf_color( ch, "&c%s: &w%d, %d experience needed\n\r",
                             noncombat_skill[i],
                             ch->pcdata->noncombat[i],
                             exp_level( ch, ch->pcdata->noncombat[i] + 1 ) - ch->pcdata->noncombat_exp[i]
                             - exp_level( ch, ch->pcdata->noncombat[i] ) );
      }
      else
      {
         pager_printf_color( ch, "&c%-20s: &w%-3d\n\r", noncombat_skill[i], ch->pcdata->noncombat[i] );
      }
   }*/
}

void do_experience( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_BLUE, ch );

   send_to_char( "You ", ch );
   if( xIS_SET( ch->act, PLR_SHOW_EXP ) )
   {
      ch_printf( ch, "have %d experience points.\n\r", ch->exp );
      return;
   }
   if( ch->exp > 2000000000 )
      send_to_char( "feel like your head is about to explode!\n\r", ch );
   else if( ch->exp > 1000000000 )
      send_to_char( "feel dizzy from so much knowledge!\n\r", ch );
   else if( ch->exp > 50000000 )
      send_to_char( "are overflowing with inspiration!\n\r", ch );
   else if( ch->exp > 10000000 )
      send_to_char( "are brimming with inspiration!\n\r", ch );
   else if( ch->exp > 5000000 )
      send_to_char( "feel extremely inspired!\n\r", ch );
   else if( ch->exp > 2000000 )
      send_to_char( "feel greatly inspired!\n\r", ch );
   else if( ch->exp > 1000000 )
      send_to_char( "feel very inspired!\n\r", ch );
   else if( ch->exp > 500000 )
      send_to_char( "feel inspired!\n\r", ch );
   else if( ch->exp > 300000 )
      send_to_char( "feel rather creative.\n\r", ch );
   else if( ch->exp > 100000 )
      send_to_char( "feel somewhat creative.\n\r", ch );
   else if( ch->exp > 50000 )
      send_to_char( "are getting bored.\n\r", ch );
   else if( ch->exp > 10000 )
      send_to_char( "are quite bored.\n\r", ch );
   else if( ch->exp > 0 )
      send_to_char( "are extremely bored.\n\r", ch );
   else
      send_to_char( "feel dizzy and your head aches.\n\r", ch );
}

void do_attributes( CHAR_DATA * ch, char *argument )
{
   int stat;
   set_char_color( AT_BLUE, ch );

   do_experience( ch, "" );
   send_to_char( "Your current attributes:\n\r", ch );

   stat = get_curr_str( ch );
   send_to_char( "You ", ch );
   if( stat < 20 )
      send_to_char( "are weak as a kitten.\n\r", ch );
   else if( stat < 30 )
      send_to_char( "are flabby and weak.\n\r", ch );
   else if( stat < 50 )
      send_to_char( "are somewhat flabby.\n\r", ch );
   else if( stat < 100 )
      send_to_char( "are of average strength.\n\r", ch );
   else if( stat < 120 )
      send_to_char( "have moderately good strength.\n\r", ch );
   else if( stat < 150 )
      send_to_char( "are well-built and muscular.\n\r", ch );
   else if( stat < 200 )
      send_to_char( "are strong as a body builder.\n\r", ch );
   else if( stat < 250 )
      send_to_char( "have the strength of a titan.\n\r", ch );
   else if( stat < 300 )
      send_to_char( "can leap tall buildings in a single bound.\n\r", ch );
   else if( stat < 400 )
      send_to_char( "can bend steel with your bare hands.\n\r", ch );
   else
      send_to_char( "could probably lift a mountain.\n\r", ch );

   stat = get_curr_int( ch );
   send_to_char( "You ", ch );
   if( stat < 20 )
      send_to_char( "are brain dead.\n\r", ch );
   else if( stat < 30 )
      send_to_char( "are severely mentally retarded.\n\r", ch );
   else if( stat < 40 )
      send_to_char( "are a complete idiot.\n\r", ch );
   else if( stat < 50 )
      send_to_char( "are somewhat slow in thinking.\n\r", ch );
   else if( stat < 100 )
      send_to_char( "are of average intelligence.\n\r", ch );
   else if( stat < 130 )
      send_to_char( "are clever and witty.\n\r", ch );
   else if( stat < 160 )
      send_to_char( "are quick thinking.\n\r", ch );
   else if( stat < 200 )
      send_to_char( "are brilliant and intellectual.\n\r", ch );
   else if( stat < 250 )
      send_to_char( "are a genius.\n\r", ch );
   else if( stat < 400 )
      send_to_char( "are a super-genius.\n\r", ch );
   else
      send_to_char( "have a mind like a computer.\n\r", ch );

   stat = get_curr_wil( ch );
   send_to_char( "You ", ch );
   if( stat < 20 )
      send_to_char( "are a pushover.\n\r", ch );
   else if( stat < 50 )
      send_to_char( "get pushed around sometimes.\n\r", ch );
   else if( stat < 80 )
      send_to_char( "go with the flow.\n\r", ch );
   else if( stat < 100 )
      send_to_char( "have average willpower.\n\r", ch );
   else if( stat < 130 )
      send_to_char( "don't let people push you around.\n\r", ch );
   else if( stat < 170 )
      send_to_char( "are stubborn as a mule.\n\r", ch );
   else if( stat < 200 )
      send_to_char( "are willful and unshakable.\n\r", ch );
   else if( stat < 250 )
      send_to_char( "are dominant and willful.\n\r", ch );
   else if( stat < 300 )
      send_to_char( "are forceful and charismatic.\n\r", ch );
   else if( stat < 400 )
      send_to_char( "have a will of iron.\n\r", ch );
   else
      send_to_char( "radiate an aura of command.\n\r", ch );

   stat = get_curr_dex( ch );
   send_to_char( "You ", ch );
   if( stat < 10 )
      send_to_char( "make glaciers look fast.\n\r", ch );
   else if( stat < 20 )
      send_to_char( "are a klutz.\n\r", ch );
   else if( stat < 30 )
      send_to_char( "are extremely clumsy.\n\r", ch );
   else if( stat < 40 )
      send_to_char( "are very clumsy.\n\r", ch );
   else if( stat < 50 )
      send_to_char( "are rather clumsy.\n\r", ch );
   else if( stat < 60 )
      send_to_char( "are somewhat clumsy.\n\r", ch );
   else if( stat < 80 )
      send_to_char( "aren't particularly graceful.\n\r", ch );
   else if( stat < 100 )
      send_to_char( "have average dexterity.\n\r", ch );
   else if( stat < 120 )
      send_to_char( "have quicker than normal reflexes.\n\r", ch );
   else if( stat < 150 )
      send_to_char( "are agile and graceful.\n\r", ch );
   else if( stat < 200 )
      send_to_char( "are flexible as a dancer.\n\r", ch );
   else if( stat < 300 )
      send_to_char( "bend like willow boughs and are quick as a deer.\n\r", ch );
   else if( stat < 400 )
      send_to_char( "make a sonic boom when you run.\n\r", ch );
   else if( stat < 500 )
      send_to_char( "are faster than a speeding bullet.\n\r", ch );
   else if( stat < 600 )
      send_to_char( "are quick as a bolt of lightning.\n\r", ch );
   else
      send_to_char( "warp space when you run.\n\r", ch );

   stat = get_curr_con( ch );
   send_to_char( "You ", ch );
   if( stat < 20 )
      send_to_char( "are sickly and frail.\n\r", ch );
   else if( stat < 50 )
      send_to_char( "get sick often.\n\r", ch );
   else if( stat < 80 )
      send_to_char( "get sick sometimes.\n\r", ch );
   else if( stat < 100 )
      send_to_char( "have an average constitution.\n\r", ch );
   else if( stat < 120 )
      send_to_char( "rarely get sick.\n\r", ch );
   else if( stat < 150 )
      send_to_char( "are healthy as a horse.\n\r", ch );
   else if( stat < 200 )
      send_to_char( "are hale and hardy.\n\r", ch );
   else if( stat < 250 )
      send_to_char( "are built like a rock.\n\r", ch );
   else if( stat < 400 )
      send_to_char( "have a powerful body that can take great punishment.\n\r", ch );
   else
      send_to_char( "have the body of a mountain.\n\r", ch );

   stat = get_curr_per( ch );
   send_to_char( "You ", ch );
   if( stat < 20 )
      send_to_char( "are blind as a bat.\n\r", ch );
   else if( stat < 30 )
      send_to_char( "can't see your hand in front of your face.\n\r", ch );
   else if( stat < 40 )
      send_to_char( "squint at your surroundings.\n\r", ch );
   else if( stat < 70 )
      send_to_char( "have a little trouble seeing things.\n\r", ch );
   else if( stat < 100 )
      send_to_char( "have average perception.\n\r", ch );
   else if( stat < 120 )
      send_to_char( "have better than normal vision.\n\r", ch );
   else if( stat < 140 )
      send_to_char( "have sharp eyes.\n\r", ch );
   else if( stat < 170 )
      send_to_char( "have extremely sharp eyes.\n\r", ch );
   else if( stat < 200 )
      send_to_char( "pick up every detail of your surroundings.\n\r", ch );
   else if( stat < 250 )
      send_to_char( "have the eyes of an eagle.\n\r", ch );
   else
      send_to_char( "sense what's around you with almost mystical clarity.\n\r", ch );

   stat = get_curr_end( ch );
   send_to_char( "You ", ch );
   if( stat < 20 )
      send_to_char( "collapse if you stand up.\n\r", ch );
   else if( stat < 80 )
      send_to_char( "are short of breath.\n\r", ch );
   else if( stat < 100 )
      send_to_char( "can walk normally.\n\r", ch );
   else if( stat < 120 )
      send_to_char( "can walk for hours.\n\r", ch );
   else if( stat < 150 )
      send_to_char( "can exercise strenuously for periods of time.\n\r", ch );
   else if( stat < 200 )
      send_to_char( "can jog without breaking a sweat.\n\r", ch );
   else if( stat < 300 )
      send_to_char( "could run a marathon.\n\r", ch );
   else if( stat < 400 )
      send_to_char( "can run for miles without stopping.\n\r", ch );
   else
      send_to_char( "just keep going and going and going...\n\r", ch );
}


void set_title( CHAR_DATA * ch, char *title )
{
   char buf[MAX_STRING_LENGTH];
   char newbuf[MAX_STRING_LENGTH];
   int i, a, b;
   bool fHasName = FALSE;

   if( IS_NPC( ch ) )
   {
      bug( "Set_title: NPC.", 0 );
      return;
   }
   if( title[0] == '\0' )
      strcpy( buf, "$n" );
   else
      strcpy( buf, title );

   a = 0;
   b = strlen( buf );
   newbuf[0] = '\0';
   for( i = 0; i < b; i++ )
   {
      if( buf[i] == '$' && buf[i + 1] == 'n' )
      {
         fHasName = TRUE;
         if( strlen( newbuf ) == 0 )
            strcpy( newbuf, ch->name );
         else
            strcat( newbuf, ch->name );
         i++;
         a += strlen( ch->name );
      }
      else
      {
         newbuf[a] = buf[i];
         a++;
      }
      newbuf[a + 1] = '\0';
   }

   if( fHasName == FALSE )
   {
      strcpy( newbuf, ch->name );
      strcat( newbuf, " " );
      strcat( newbuf, title );
   }

   STRFREE( ch->pcdata->title );
   ch->pcdata->title = STRALLOC( newbuf );
   return;
}

void do_title( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
      return;

   if( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "The Gods prohibit you from changing your title.\n\r", ch );
      return;
   }

   if( strlen( argument ) > 80 )
      argument[80] = '\0';

   smash_tilde( argument );
   set_title( ch, argument );
   send_to_char( "Ok.\n\r", ch );
}

void do_homepage( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( argument[0] == '\0' )
   {
      if( !ch->pcdata->homepage )
         ch->pcdata->homepage = str_dup( "" );
      ch_printf( ch, "Your homepage is: %s\n\r", show_tilde( ch->pcdata->homepage ) );
      return;
   }

   if( !str_cmp( argument, "clear" ) )
   {
      if( ch->pcdata->homepage )
         DISPOSE( ch->pcdata->homepage );
      ch->pcdata->homepage = str_dup( "" );
      send_to_char( "Homepage cleared.\n\r", ch );
      return;
   }

   if( strstr( argument, "://" ) )
      strcpy( buf, argument );
   else
      sprintf( buf, "http://%s", argument );
   if( strlen( buf ) > 70 )
      buf[70] = '\0';

   hide_tilde( buf );
   if( ch->pcdata->homepage )
      DISPOSE( ch->pcdata->homepage );
   ch->pcdata->homepage = str_dup( buf );
   send_to_char( "Homepage set.\n\r", ch );
}



/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Monsters are too dumb to do that!\n\r", ch );
      return;
   }

   if( !ch->desc )
   {
      bug( "do_description: no descriptor", 0 );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "do_description: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\n\r", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_DESC;
         ch->dest_buf = ch;
         start_editing( ch, ch->description );
         return;

      case SUB_PERSONAL_DESC:
         STRFREE( ch->description );
         ch->description = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA * ch, char *argument )
{
   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs cannot set a bio.\n\r", ch );
      return;
   }
   if( !ch->desc )
   {
      bug( "do_bio: no descriptor", 0 );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "do_bio: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\n\r", ch );
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_BIO;
         ch->dest_buf = ch;
         start_editing( ch, ch->pcdata->bio );
         return;

      case SUB_PERSONAL_BIO:
         STRFREE( ch->pcdata->bio );
         ch->pcdata->bio = copy_buffer( ch );
         stop_editing( ch );
         return;
   }
}

/*
 * New report command coded by Morphina
 * Bug fixes by Shaddai
 */

void do_report( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];

   ch_printf( ch, "You report: %d/%d hp %d/%d mana %d/%d ep.\n\r",
              ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
   sprintf( buf, "$n reports: %d/%d hp %d/%d mana %d/%d ep.",
            ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   ch_printf( ch, "Your base stats:    %-2d str %-2d wil %-2d int %-2d dex %-2d con %-2d per %-2d end.\n\r",
              ch->perm_str, ch->perm_wil, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_per, ch->perm_lck );
   sprintf( buf, "$n's base stats:    %-2d str %-2d wil %-2d int %-2d dex %-2d con %-2d per %-2d end.",
            ch->perm_str, ch->perm_wil, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_per, ch->perm_lck );
   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

   ch_printf( ch, "Your current stats: %-2d str %-2d wil %-2d int %-2d dex %-2d con %-2d per %-2d end.\n\r",
              get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
              get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_end( ch ) );
   sprintf( buf, "$n's current stats: %-2d str %-2d wil %-2d int %-2d dex %-2d con %-2d per %-2d end.",
            get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
            get_curr_dex( ch ), get_curr_con( ch ), get_curr_cha( ch ), get_curr_end( ch ) );
   act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
   return;
}

void do_fprompt( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\n\r", ch );
      return;
   }
   smash_tilde( argument );
   one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current fighting prompt string:\n\r", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)" : ch->pcdata->fprompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\n\r", ch );
      return;
   }
   send_to_char( "Replacing old prompt of:\n\r", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)" : ch->pcdata->fprompt );
   if( ch->pcdata->fprompt )
      STRFREE( ch->pcdata->fprompt );
   if( strlen( argument ) > 128 )
      argument[128] = '\0';

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->fprompt = STRALLOC( "" );
   else
      ch->pcdata->fprompt = STRALLOC( argument );
   return;
}

void do_prompt( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

   set_char_color( AT_GREY, ch );

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPC's can't change their prompt..\n\r", ch );
      return;
   }
   smash_tilde( argument );
   one_argument( argument, arg );
   if( !*arg || !str_cmp( arg, "display" ) )
   {
      send_to_char( "Your current prompt string:\n\r", ch );
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
      set_char_color( AT_GREY, ch );
      send_to_char( "Type 'help prompt' for information on changing your prompt.\n\r", ch );
      return;
   }
   send_to_char( "Replacing old prompt of:\n\r", ch );
   set_char_color( AT_WHITE, ch );
   ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
   if( ch->pcdata->prompt )
      STRFREE( ch->pcdata->prompt );
   if( strlen( argument ) > 128 )
      argument[128] = '\0';

   /*
    * Can add a list of pre-set prompts here if wanted.. perhaps
    * 'prompt 1' brings up a different, pre-set prompt 
    */
   if( !str_cmp( arg, "default" ) )
      ch->pcdata->prompt = STRALLOC( "" );
   else
      ch->pcdata->prompt = STRALLOC( argument );
   return;
}

void do_withdraw( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *atm;
   char arg2[MAX_INPUT_LENGTH];
   bool bank = FALSE;
   int amount = 0, gp = 0;

   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Syntax: withdraw <amount>\r\n", ch );
      return;
   }

   /*
    * Player-owned shops 
    */
   if( ch->in_room->pShop && !str_cmp( ch->in_room->pShop->owner, ch->name ) )
   {
      int salary;

      if( !str_cmp( argument, "all" ) )
         amount = ch->in_room->pShop->gold;
      else
         amount = atoi( argument );

      if( amount < 1 )
      {
         send_to_char( "You can't withdraw less than 1 gold.\n\r", ch );
         return;
      }

      if( amount > ch->in_room->pShop->gold )
      {
         send_to_char( "You can't withdraw more than is in the shop.\n\r", ch );
         return;
      }

      gp = amount;

      /*
       * Low salary needs the precision,
       * * but high salary needs to not overflow MAXINT
       */
      if( ch->in_room->pShop->gold < 1000000 )
         salary = ( gp * ch->in_room->pShop->profit_buy ) / 100;
      else
         salary = ( gp / 100 ) * ch->in_room->pShop->profit_buy;

      ch_printf( ch, "You withdraw &C%d mithril, ", ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "and &O%d copper coins.\n\r", gp );
      act( AT_PLAIN, "$n withdraws some coins.", ch, NULL, NULL, TO_ROOM );

      gp = salary;
      ch_printf( ch, "You pay &C%d mithril, ", ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "and &O%d copper coins in salary to your shopkeeper.\n\r", gp );

      /* Take the amount from the shop */
      ch->in_room->pShop->gold -= amount;
      /* Take away the salary */
      amount -= salary;
      /* Give the rest of amount to the character */
      ch->gold += amount;
      /* Save the area */
      fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      return;
   }

   for( atm = ch->in_room->first_content; atm; atm = atm->next_content )
   {
      if( atm->item_type == ITEM_BANK )
      {
         bank = TRUE;
         break;
      }
   }

   if( !bank )
   {
      send_to_char( "There is no bank here.\r\n", ch );
      return;
   }

   if( !str_cmp( argument, "all" ) )
   {
      gp = ch->pcdata->balance;
      ch_printf( ch, "You withdraw &C%d mithril, ", ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "and &O%d copper coins.\n\r", gp );
      act( AT_PLAIN, "$n withdraws some coins.", ch, NULL, NULL, TO_ROOM );
      ch->gold += ch->pcdata->balance;
      ch->pcdata->balance = 0;
      return;
   }

   argument = one_argument( argument, arg2 );
   amount = atoi( arg2 );

   if( !str_cmp( argument, "mithril" ) )
      amount *= 1000000;
   else if( !str_cmp( argument, "gold" ) )
      amount *= 10000;
   else if( !str_cmp( argument, "silver" ) )
      amount *= 100;
   else if( !str_cmp( argument, "copper" ) || !str_cmp( argument, "coins" ) || !argument || argument[0] == '\0' )
      amount *= 1;
   else
   {
      send_to_char( "You can't do that.\n\r", ch );
      return;
   }

   if( amount > ch->pcdata->balance )
   {
      send_to_char( "You don't have that much in your bank account.\r\n", ch );
      return;
   }

   if( amount < 1 )
   {
      send_to_char( "That'd be fun, wouldn't it?\r\n", ch );
      return;
   }

   ch->pcdata->balance -= amount;
   ch->gold += amount;

   gp = amount;
   ch_printf( ch, "You withdraw &C%d mithril, ", ( int )gp / 1000000 );
   gp = gp % 1000000;
   ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
   gp = gp % 10000;
   ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
   gp = gp % 100;
   ch_printf( ch, "and &O%d copper coins.\n\r", gp );
   act( AT_PLAIN, "$n withdraws some coins.", ch, NULL, NULL, TO_ROOM );
}

void do_deposit( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA *atm;
   char arg2[MAX_INPUT_LENGTH];

   bool bank = FALSE;
   int amount = 0, gp = 0;

   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Syntax: deposit <amount>\r\n", ch );
      return;
   }

   /*
    * Player-owned shops 
    */
   if( ch->in_room->pShop && !str_cmp( ch->in_room->pShop->owner, ch->name ) )
   {
      if( !str_cmp( argument, "all" ) )
         amount = ch->gold;
      else
         amount = atoi( argument );

      if( amount < 1 )
      {
         send_to_char( "You can't deposit less than 1 gold.\n\r", ch );
         return;
      }

      if( amount > ch->gold )
      {
         send_to_char( "You can't deposit more than you have on hand.\n\r", ch );
         return;
      }

      gp = amount;

      /* When withdrawing they pay a salary to the shopkeeper, but we aren't going to do anything when depositing...for now */
      ch_printf( ch, "You deposit &C%d mithril, ", ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "and &O%d copper coins.\n\r", gp );
      act( AT_PLAIN, "$n deposits some coins.", ch, NULL, NULL, TO_ROOM );

      /* Put the amount in the shop */
      ch->in_room->pShop->gold += amount;
      /* Take the amount from the character */
      ch->gold -= amount;
      /* Save the area */
      fold_area( ch->in_room->area, ch->in_room->area->filename, FALSE );
      return;
   }

   for( atm = ch->in_room->first_content; atm; atm = atm->next_content )
   {
      if( atm->item_type == ITEM_BANK )
      {
         bank = TRUE;
         break;
      }
   }

   if( !bank )
   {
      send_to_char( "There is no bank here.\r\n", ch );
      return;
   }

   if( !str_cmp( argument, "all" ) )
   {
      gp = ch->gold;
      ch_printf( ch, "You deposit &C%d mithril, ", ( int )gp / 1000000 );
      gp = gp % 1000000;
      ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
      gp = gp % 10000;
      ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
      gp = gp % 100;
      ch_printf( ch, "and &O%d copper coins.\n\r", gp );
      act( AT_PLAIN, "$n deposits some coins.", ch, NULL, NULL, TO_ROOM );
      ch->pcdata->balance += ch->gold;
      ch->gold = 0;
      return;
   }

   argument = one_argument( argument, arg2 );
   amount = atoi( arg2 );

   if( !str_cmp( argument, "mithril" ) )
      amount *= 1000000;
   else if( !str_cmp( argument, "gold" ) )
      amount *= 10000;
   else if( !str_cmp( argument, "silver" ) )
      amount *= 100;
   else if( !str_cmp( argument, "copper" ) || !str_cmp( argument, "coins" ) || !argument || argument[0] == '\0' )
      amount *= 1;
   else
   {
      send_to_char( "You can't do that.\n\r", ch );
      return;
   }

   if( amount > ch->gold )
   {
      send_to_char( "You aren't carrying that many coins.\r\n", ch );
      return;
   }

   if( amount < 1 )
   {
      send_to_char( "That'd be fun, wouldn't it?\r\n", ch );
      return;
   }

   ch->pcdata->balance += amount;
   ch->gold -= amount;

   gp = amount;
   ch_printf( ch, "You deposit &C%d mithril, ", ( int )gp / 1000000 );
   gp = gp % 1000000;
   ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
   gp = gp % 10000;
   ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
   gp = gp % 100;
   ch_printf( ch, "and &O%d copper coins.\n\r", gp );
   act( AT_PLAIN, "$n deposits some coins.", ch, NULL, NULL, TO_ROOM );
}

/* Places a bounty on a victim, that can be collected when they are killed.
	Syntax:
	bounty <victim> <amount>
 */
void do_bounty( CHAR_DATA * ch, char *argument )
{
   int reward;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   if( !argument )
   {
      send_to_char( "Syntax: bounty <victim> <amount>\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );

   if( !argument )
   {
      send_to_char( "How much money do you want to post on their head?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg2 );

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }
   else if( IS_NPC( victim ) )
   {
      send_to_char( "You can't post bounties on mobs.\r\n", ch );
      return;
   }
   else if( ch == victim )
   {
      if( ch->gold >= ch->pcdata->bounty * 3 )
      {
         ch->gold -= ch->pcdata->bounty * 3;
         ch->pcdata->bounty = 0;
         send_to_char( "You pay off the bounty on your head.\n\r", ch );
      }
      else
      {
         send_to_char( "You cannot afford to pay off this bounty.\n\r", ch );
      }
      return;
   }

   reward = atoi( arg2 );

   if( reward < 500000 )
   {
      send_to_char( "A bounty of less than 500k coins would just be pathetic.\r\n", ch );
      return;
   }

   if( reward > ch->gold )
   {
      send_to_char( "Alas! You haven't got that much to post for a bounty!\r\n", ch );
      return;
   }

   victim->pcdata->bounty += reward;
   ch->gold -= reward;

   sprintf( arg1, "You post a bounty of %d coins on %s!", reward, victim->name );
   sprintf( arg2, "$n posts a bounty of %d coins on %s!", reward, victim->name );

   act( AT_YELLOW, arg1, ch, NULL, NULL, TO_CHAR );
   act( AT_YELLOW, arg2, ch, NULL, victim, TO_NOTVICT );

   sprintf( arg1, "$n posts a bounty of %d coins on you!", reward );
   if( ch->in_room == victim->in_room )
      act( AT_YELLOW, arg1, ch, NULL, victim, TO_VICT );

   if( !xIS_SET( victim->act, PLR_BOUNTY ) )
   {
      xSET_BIT( victim->act, PLR_BOUNTY );
      act( AT_YELLOW, "A bounty has been placed on your head!", ch, NULL, victim, TO_VICT );
   }
   else
      act( AT_YELLOW, "The bounty on your head has been increased!", ch, NULL, victim, TO_VICT );
}


/* Determine a char's height based off their weight */
int get_height_weight( CHAR_DATA * ch )
{
   int weight;

   weight = ch->height * ch->height;
   weight = number_range( ( int )weight / 50, ( int )weight / 20 );

   return weight;
}


/* Families -keo */

void do_introduce( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
   {
      huh( ch );
      return;
   }

   if( ch->pcdata->parent )
   {
      ch_printf( ch, "&CYou say 'I am %s, %s of %s.'\n\r",
                 ch->name,
                 IS_VAMPIRE( ch ) ? "childe" :
                 ch->sex == SEX_MALE ? "son" : ch->sex == SEX_FEMALE ? "daughter" : "child", ch->pcdata->parent );
      sprintf( buf, "$n says 'I am %s, %s of %s.'",
               ch->name,
               IS_VAMPIRE( ch ) ? "childe" :
               ch->sex == SEX_MALE ? "son" : ch->sex == SEX_FEMALE ? "daughter" : "child", ch->pcdata->parent );
      act( AT_SAY, buf, ch, NULL, NULL, TO_ROOM );
   }
   else
   {
      ch_printf( ch, "&CYou say 'I am %s the %s.'\n\r", ch->name, ch->species );
      sprintf( buf, "$n says 'I am %s the %s.'", ch->name, ch->species );
      act( AT_SAY, buf, ch, NULL, NULL, TO_ROOM );
   }

   if( ch->pcdata->family )
   {
      ch_printf( ch, "&CYou say 'I am of Generation %d of the House %s.'\n\r", ch->pcdata->gen, ch->pcdata->family );
      sprintf( buf, "$n says 'I am of Generation %d of the House %s.'", ch->pcdata->gen, ch->pcdata->family );
      act( AT_SAY, buf, ch, NULL, NULL, TO_ROOM );
   }

   if( ch->pcdata->spouse )
   {
      ch_printf( ch, "&CYou say 'I am the %s of %s.'\n\r",
                 ch->sex == SEX_MALE ? "husband" : ch->sex == SEX_FEMALE ? "wife" : "spouse", ch->pcdata->spouse );
      sprintf( buf, "$n says 'I am the %s of %s.'",
               ch->sex == SEX_MALE ? "husband" : ch->sex == SEX_FEMALE ? "wife" : "spouse", ch->pcdata->spouse );
      act( AT_SAY, buf, ch, NULL, NULL, TO_ROOM );
   }
}

void do_family( CHAR_DATA * ch, char *argument )
{

   if( IS_NPC( ch ) )
   {
      send_to_char( "What use have you for a family?\n\r", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "You belong to ", ch );
      if( ch->pcdata->family )
      {
         ch_printf( ch, "the %s %s.\n\r", IS_VAMPIRE( ch ) ? "Clan" : "House", ch->pcdata->family );
      }
      else
      {
         send_to_char( "no family.\n\r", ch );
      }
      return;
   }

   if( get_char_worth( ch ) < 500 )
   {
      send_to_char( "No family founded by one so young as you would be recognized.\n\r", ch );
      return;
   }

   if( get_char_worth( ch ) < 1000 && IS_VAMPIRE( ch ) )
   {
      send_to_char( "You are yet too weak to break away from your master.\n\r", ch );
      return;
   }

   if( ch->pcdata->family )
      send_to_char( "You disown your former family!\n\r", ch );

   STRFREE( ch->pcdata->parent );
   STRFREE( ch->pcdata->family );
   ch->pcdata->family = STRALLOC( argument );
   if( !ch->pcdata->maiden_name )
      ch->pcdata->maiden_name = STRALLOC( argument );
   ch_printf( ch, "You found the House %s.\n\r", argument );
   save_char_obj( ch );
}

void do_adopt( CHAR_DATA * ch, char *argument )
{

   CHAR_DATA *victim;

   if( IS_NPC( ch ) )
   {
      send_to_char( "What use have you for a family?\n\r", ch );
      return;
   }

   if( !ch->pcdata->family )
   {
      send_to_char( "You belong to no family.\n\r", ch );
      return;
   }

   if( get_char_worth( ch ) < 300 )
   {
      send_to_char( "Your parents would not appreciate one so young bringing in new members of the family.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      global_retcode = rNONE;
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "They are not worthy of adoption.\n\r", ch );
      return;
   }

   if( victim->pcdata->family && !str_cmp( victim->pcdata->family, ch->pcdata->family ) )
   {
      send_to_char( "They are already a member of your family.\n\r", ch );
      return;
   }

   if( !IS_CONSENTING( victim, ch ) )
   {
      send_to_char( "You may not adopt them without their consent.\n\r", ch );
      return;
   }

   STRFREE( victim->pcdata->parent );
   victim->pcdata->parent = STRALLOC( ch->name );
   STRFREE( victim->pcdata->family );
   victim->pcdata->family = STRALLOC( ch->pcdata->family );
   if( !victim->pcdata->maiden_name )
      victim->pcdata->maiden_name = STRALLOC( ch->pcdata->family );
   victim->pcdata->gen = ch->pcdata->gen + 1;
   save_char_obj( victim );
   act( AT_PLAIN, "You adopt $N into your family.", ch, NULL, victim, TO_CHAR );
   act( AT_PLAIN, "$n adopts you into $s family.", ch, NULL, victim, TO_VICT );
   act( AT_PLAIN, "$n adopts $N into $s family.", ch, NULL, victim, TO_NOTVICT );
}

void do_marry( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "What use have you for a family?\n\r", ch );
      return;
   }

   if( !ch->pcdata->family )
   {
      send_to_char( "You belong to no family.\n\r", ch );
      return;
   }

   if( ch->pcdata->spouse )
   {
      ch_printf( ch, "You are already married to %s!\n\r", ch->pcdata->spouse );
      return;
   }

   if( get_char_worth( ch ) < 200 )
   {
      send_to_char( "Your parents would not appreciate one so young marrying.\n\r", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "They are not worthy of marriage.\n\r", ch );
      return;
   }

   if( victim->pcdata->family && !str_cmp( victim->pcdata->family, ch->pcdata->family ) )
   {
      send_to_char( "They are a member of your family, that would be incest!\n\r", ch );
      return;
   }

   if( victim->pcdata->spouse )
   {
      send_to_char( "They are already married!\n\r", ch );
      return;
   }

   if( ch->sex == SEX_NEUTRAL || victim->sex == SEX_NEUTRAL )
   {
      send_to_char( "Those without gender may not marry.\n\r", ch );
      return;
   }

   if( ch->sex == victim->sex )
   {
      send_to_char( "Same-sex marriages are not recognized in this universe.\n\r", ch );
      return;
   }

   if( !IS_CONSENTING( victim, ch ) )
   {
      send_to_char( "You may not marry them without their consent.\n\r", ch );
      return;
   }

   if( victim->pcdata->family )
      sprintf( buf, "%s-%s", ch->pcdata->family, victim->pcdata->family );
   else
      sprintf( buf, "%s", ch->pcdata->family );

   STRFREE( ch->pcdata->family );
   STRFREE( victim->pcdata->family );
   ch->pcdata->family = STRALLOC( buf );
   victim->pcdata->family = STRALLOC( buf );
   ch->pcdata->spouse = STRALLOC( victim->name );
   victim->pcdata->spouse = STRALLOC( ch->name );
   save_char_obj( ch );
   save_char_obj( victim );
   act( AT_PLAIN, "You take $N's hand in marriage.", ch, NULL, victim, TO_CHAR );
   act( AT_PLAIN, "$n takes your hand in marriage.", ch, NULL, victim, TO_VICT );
   act( AT_PLAIN, "$n takes $N's hand in marriage.", ch, NULL, victim, TO_NOTVICT );
   sprintf( buf, "%s and %s are now married!", ch->name, victim->name );
   do_info( ch, buf );
}

void do_divorce( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Divorce who?\n\r", ch );
      return;
   }

   if( !ch->pcdata->spouse )
   {
      send_to_char( "But you aren't married!\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( !victim->pcdata->spouse || str_cmp( victim->pcdata->spouse, ch->name ) )
   {
      send_to_char( "But they aren't married to you!\n\r", ch );
      return;
   }

   ch_printf( ch, "You divorce %s!\n\r", victim->name );
   ch_printf( victim, "%s divorces you!\n\r", ch->name );
   STRFREE( victim->pcdata->spouse );
   STRFREE( ch->pcdata->spouse );
   if( ch->pcdata->maiden_name )
      ch->pcdata->family = STRALLOC( ch->pcdata->maiden_name );
   else if( ch->pcdata->family )
      ch->pcdata->maiden_name = STRALLOC( ch->pcdata->family );
   if( victim->pcdata->maiden_name )
      victim->pcdata->family = STRALLOC( victim->pcdata->maiden_name );
   else if( victim->pcdata->family )
      victim->pcdata->maiden_name = STRALLOC( victim->pcdata->family );
   sprintf( buf, "%s and %s are now divorced!", ch->name, victim->name );
   save_char_obj( ch );
   save_char_obj( victim );
   do_info( ch, buf );
}

void do_disown( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;

   if( IS_NPC( ch ) || !ch->pcdata->family )
   {
      send_to_char( "But you aren't even in a family!\n\r", ch );
      return;
   }

   if( ( victim = get_char_world( ch, argument ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if( IS_NPC( victim ) || !victim->pcdata->family
       || ( str_cmp( ch->pcdata->family, victim->pcdata->family )
            && str_cmp( ch->pcdata->maiden_name, victim->pcdata->family ) ) )
   {
      send_to_char( "But they aren't in your family!\n\r", ch );
      return;
   }

   send_to_char( "You disown them from your family!\n\r", ch );
   send_to_char( "Your family has disowned you!\n\r", victim );
   STRFREE( victim->pcdata->family );
   STRFREE( victim->pcdata->parent );
   STRFREE( victim->pcdata->maiden_name );

}


void do_suicide( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
   argument = one_argument_retain_case( argument, arg );

   if( arg[0] == '\0' )
   {
      send_to_char( "If you want to commit suicide, the syntax is: suicide <password>\n\r", ch );
      return;
   }

   if( strcmp( smaug_crypt( arg ), ch->pcdata->pwd ) )
   {
      send_to_char( "Wrong password.\n\r", ch );
      return;
   }
   else
   {
      do_destroy( ch, ch->name );
      sprintf( buf, "%s has committed suicide.", ch->name );
      log_string( buf );
   }
}
