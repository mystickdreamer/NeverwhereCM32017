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
 * 			Table load/save Module				    *
 ****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/* global variables */
int top_sn;
int top_herb;

SKILLTYPE *skill_table[MAX_SKILL];
SKILLTYPE *herb_table[MAX_HERB];
SKILLTYPE *disease_table[MAX_DISEASE];

char *const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Racial", "Disease" };

SPELL_FUN *spell_function( char *name )
{
   if( !str_cmp( name, "spell_smaug" ) )
      return spell_smaug;
   if( !str_cmp( name, "spell_blindness" ) )
      return spell_blindness;
   if( !str_cmp( name, "spell_cure_blindness" ) )
      return spell_cure_blindness;
   if( !str_cmp( name, "spell_cure_poison" ) )
      return spell_cure_poison;
   if( !str_cmp( name, "spell_curse" ) )
      return spell_curse;
   if( !str_cmp( name, "spell_pigeon" ) )
      return spell_pigeon;
   if( !str_cmp( name, "spell_poison" ) )
      return spell_poison;
   if( !str_cmp( name, "spell_remove_curse" ) )
      return spell_remove_curse;
   if( !str_cmp( name, "spell_sleep" ) )
      return spell_sleep;
   if( !str_cmp( name, "spell_portal" ) )
      return spell_portal;

   if( !str_cmp( name, "reserved" ) )
      return NULL;
   if( !str_cmp( name, "spell_null" ) )
      return spell_null;
   return spell_notfound;
}

DO_FUN *skill_function( char *name )
{
   switch ( name[3] )
   {
      case 'a':
         if( !str_cmp( name, "do_aassign" ) )
            return do_aassign;
		 if ( !str_cmp( name, "do_accessories"))     
			return do_accessories; 
         if( !str_cmp( name, "do_abit" ) )
            return do_abit;
         if( !str_cmp( name, "do_adopt" ) )
            return do_adopt;
         if( !str_cmp( name, "do_aentrance" ) )
            return do_aentrance;
         if( !str_cmp( name, "do_aexit" ) )
            return do_aexit;
         if( !str_cmp( name, "do_affected" ) )
            return do_affected;
         if( !str_cmp( name, "do_affix" ) )
            return do_affix;
         if( !str_cmp( name, "do_afk" ) )
            return do_afk;
         if( !str_cmp( name, "do_aid" ) )
            return do_aid;
         if( !str_cmp( name, "do_alias" ) )
            return do_alias;
         if( !str_cmp( name, "do_allow" ) )
            return do_allow;
         if( !str_cmp( name, "do_alter" ) )
            return do_alter;
         if( !str_cmp( name, "do_ansi" ) )
            return do_ansi;
         if( !str_cmp( name, "do_antimatter" ) )
            return do_antimatter;
         if( !str_cmp( name, "do_apply" ) )
            return do_apply;
         if( !str_cmp( name, "do_archmages" ) )
            return do_archmages;
         if( !str_cmp( name, "do_areas" ) )
            return do_areas;
         if( !str_cmp( name, "do_arena" ) )
            return do_arena;
         if( !str_cmp( name, "do_aset" ) )
            return do_aset;
         if( !str_cmp( name, "do_astat" ) )
            return do_astat;
         if( !str_cmp( name, "do_at" ) )
            return do_at;
         if( !str_cmp( name, "do_atmob" ) )
            return do_atmob;
         if( !str_cmp( name, "do_atobj" ) )
            return do_atobj;
         if( !str_cmp( name, "do_attributes" ) )
            return do_attributes;
         if( !str_cmp( name, "do_auto" ) )
            return do_auto;
         if( !str_cmp( name, "do_award" ) )
            return do_award;
         break;
      case 'b':
         if( !str_cmp( name, "do_balefire" ) )
            return do_balefire;
         if( !str_cmp( name, "do_bamfin" ) )
            return do_bamfin;
         if( !str_cmp( name, "do_bamfout" ) )
            return do_bamfout;
         if( !str_cmp( name, "do_ban" ) )
            return do_ban;
         if( !str_cmp( name, "do_bandage" ) )
            return do_bandage;
         if( !str_cmp( name, "do_bashdoor" ) )
            return do_bashdoor;
         if( !str_cmp( name, "do_beep" ) )
            return do_beep;
         if( !str_cmp( name, "do_bestow" ) )
            return do_bestow;
         if( !str_cmp( name, "do_bind" ) )
            return do_bind;
         if( !str_cmp( name, "do_bio" ) )
            return do_bio;
         if( !str_cmp( name, "do_bite" ) )
            return do_bite;
         if( !str_cmp( name, "do_blast" ) )
            return do_blast;
         if( !str_cmp( name, "do_blink" ) )
            return do_blink;
         if( !str_cmp( name, "do_blizzard" ) )
            return do_blizzard;
         if( !str_cmp( name, "do_body" ) )
            return do_body;
         if( !str_cmp( name, "do_bodybag" ) )
            return do_bodybag;
         if( !str_cmp( name, "do_bookwrite" ) )
            return do_bookwrite;
         if( !str_cmp( name, "do_boneset" ) )
            return do_boneset;
         if( !str_cmp( name, "do_bounty" ) )
            return do_bounty;
         if( !str_cmp( name, "do_brainshock" ) )
            return do_brainshock;
         if( !str_cmp( name, "do_brand" ) )
            return do_brand;
         if( !str_cmp( name, "do_brew" ) )
            return do_brew;
         if( !str_cmp( name, "do_bug" ) )
            return do_bug;
         if( !str_cmp( name, "do_build" ) )
            return do_build;
         if( !str_cmp( name, "do_buildwalk" ) )
            return do_buildwalk;
         if( !str_cmp( name, "do_burn" ) )
            return do_burn;
         if( !str_cmp( name, "do_bury" ) )
            return do_bury;
         if( !str_cmp( name, "do_buy" ) )
            return do_buy;
         break;
      case 'c':
         if( !str_cmp( name, "do_cast" ) )
            return do_cast;
         if( !str_cmp( name, "do_cedit" ) )
            return do_cedit;
         if( !str_cmp( name, "do_chainlightning" ) )
            return do_chainlightning;
         if( !str_cmp( name, "do_change" ) )
            return do_change;
         if( !str_cmp( name, "do_channels" ) )
            return do_channels;
         if( !str_cmp( name, "do_chat" ) )
            return do_chat;
         if( !str_cmp( name, "do_check_vnums" ) )
            return do_check_vnums;
         if( !str_cmp( name, "do_chill" ) )
            return do_chill;
         if( !str_cmp( name, "do_chop" ) )
            return do_chop;
         if( !str_cmp( name, "do_city" ) )
            return do_city;
         if( !str_cmp( name, "do_clan" ) )
            return do_clan;
         if( !str_cmp( name, "do_claw" ) )
            return do_claw;
         if( !str_cmp( name, "do_clear" ) )
            return do_clear;
         if( !str_cmp( name, "do_climate" ) )
            return do_climate;
         if( !str_cmp( name, "do_climb" ) )
            return do_climb;
         if( !str_cmp( name, "do_close" ) )
            return do_close;
         if( !str_cmp( name, "do_cmdtable" ) )
            return do_cmdtable;
         if( !str_cmp( name, "do_combine" ) )
            return do_combine;
         if( !str_cmp( name, "do_commands" ) )
            return do_commands;
         if( !str_cmp( name, "do_compare" ) )
            return do_compare;
         if( !str_cmp( name, "do_config" ) )
            return do_config;
         if( !str_cmp( name, "do_connect" ) )
            return do_connect;
         if( !str_cmp( name, "do_consent" ) )
            return do_consent;
         if( !str_cmp( name, "do_consider" ) )
            return do_consider;
         if( !str_cmp( name, "do_copyover" ) )
            return do_copyover;
         if( !str_cmp( name, "do_credits" ) )
            return do_credits;
         if( !str_cmp( name, "do_cset" ) )
            return do_cset;
         if( !str_cmp( name, "do_cure" ) )
            return do_cure;
         break;
      case 'd':
         if( !str_cmp( name, "do_decay" ) )
            return do_decay;
         if( !str_cmp( name, "do_deduct" ) )
            return do_deduct;
         if( !str_cmp( name, "do_deities" ) )
            return do_deities;
         if( !str_cmp( name, "do_delay" ) )
            return do_delay;
         if( !str_cmp( name, "do_deny" ) )
            return do_deny;
         if( !str_cmp( name, "do_deposit" ) )
            return do_deposit;
         if( !str_cmp( name, "do_description" ) )
            return do_description;
         if( !str_cmp( name, "do_destro" ) )
            return do_destro;
         if( !str_cmp( name, "do_destroy" ) )
            return do_destroy;
         if( !str_cmp( name, "do_detonate" ) )
            return do_detonate;
         if( !str_cmp( name, "do_detrap" ) )
            return do_detrap;
         if( !str_cmp( name, "do_dig" ) )
            return do_dig;
         if( !str_cmp( name, "do_disarm" ) )
            return do_disarm;
         if( !str_cmp( name, "do_disconnect" ) )
            return do_disconnect;
         if( !str_cmp( name, "do_dismiss" ) )
            return do_dismiss;
         if( !str_cmp( name, "do_dismount" ) )
            return do_dismount;
         if( !str_cmp( name, "do_disown" ) )
            return do_disown;
         if( !str_cmp( name, "do_dispel" ) )
            return do_dispel;
         if( !str_cmp( name, "do_distort" ) )
            return do_distort;
         if( !str_cmp( name, "do_divorce" ) )
            return do_divorce;
         if( !str_cmp( name, "do_dmesg" ) )
            return do_dmesg;
         if( !str_cmp( name, "do_down" ) )
            return do_down;
         if( !str_cmp( name, "do_drag" ) )
            return do_drag;
         if( !str_cmp( name, "do_drain" ) )
            return do_drain;
         if( !str_cmp( name, "do_dream" ) )
            return do_dream;
         if( !str_cmp( name, "do_drink" ) )
            return do_drink;
         if( !str_cmp( name, "do_drop" ) )
            return do_drop;
         if( !str_cmp( name, "do_diagnose" ) )
            return do_diagnose;
         if( !str_cmp( name, "do_dye" ) )
            return do_dye;
         break;
      case 'e':
         if( !str_cmp( name, "do_east" ) )
            return do_east;
         if( !str_cmp( name, "do_eat" ) )
            return do_eat;
         if( !str_cmp( name, "do_echo" ) )
            return do_echo;
         if( !str_cmp( name, "do_elbow" ) )
            return do_elbow;
         if( !str_cmp( name, "do_electrocute" ) )
            return do_electrocute;
         if( !str_cmp( name, "do_emerge" ) )
            return do_emerge;
         if( !str_cmp( name, "do_emote" ) )
            return do_emote;
         if( !str_cmp( name, "do_empty" ) )
            return do_empty;
         if( !str_cmp( name, "do_enchant" ) )
            return do_enchant;
         if( !str_cmp( name, "do_engrave" ) )
            return do_engrave;
         if( !str_cmp( name, "do_enter" ) )
            return do_enter;
         if( !str_cmp( name, "do_equipment" ) )
            return do_equipment;
         if( !str_cmp( name, "do_examine" ) )
            return do_examine;
         if( !str_cmp( name, "do_exits" ) )
            return do_exits;
         if( !str_cmp( name, "do_experience" ) )
            return do_experience;
         break;
      case 'f':

         if( !str_cmp( name, "do_family" ) )
            return do_family;
         if( !str_cmp( name, "do_fashion" ) )
            return do_fashion;
         if( !str_cmp( name, "do_feed" ) )
            return do_feed;
         if( !str_cmp( name, "do_fill" ) )
            return do_fill;
         if( !str_cmp( name, "do_fixchar" ) )
            return do_fixchar;
         if( !str_cmp( name, "do_flamingrain" ) )
            return do_flamingrain;
         if( !str_cmp( name, "do_flare" ) )
            return do_flare;
         if( !str_cmp( name, "do_fly" ) )
            return do_fly;
         if( !str_cmp( name, "do_flee" ) )
            return do_flee;
         if( !str_cmp( name, "do_flood" ) )
            return do_flood;
         if( !str_cmp( name, "do_foldarea" ) )
            return do_foldarea;
         if( !str_cmp( name, "do_follow" ) )
            return do_follow;
         if( !str_cmp( name, "do_for" ) )
            return do_for;
         if( !str_cmp( name, "do_force" ) )
            return do_force;
         if( !str_cmp( name, "do_forceclose" ) )
            return do_forceclose;
         if( !str_cmp( name, "do_form_password" ) )
            return do_form_password;
         if( !str_cmp( name, "do_fprompt" ) )
            return do_fprompt;
         if( !str_cmp( name, "do_fquit" ) )
            return do_fquit;
         if( !str_cmp( name, "do_freeze" ) )
            return do_freeze;
         if( !str_cmp( name, "do_frostbite" ) )
            return do_frostbite;
         if( !str_cmp( name, "do_funnel" ) )
            return do_funnel;
         break;
      case 'g':
         if( !str_cmp( name, "do_get" ) )
            return do_get;
         if( !str_cmp( name, "do_genarea" ) )
            return do_genarea;
         if( !str_cmp( name, "do_geyser" ) )
            return do_geyser;
         if( !str_cmp( name, "do_give" ) )
            return do_give;
         if( !str_cmp( name, "do_glance" ) )
            return do_glance;
         if( !str_cmp( name, "do_gold" ) )
            return do_gold;
		 if ( !str_cmp( name, "do_gohome" ))	        
			return do_gohome;
         if( !str_cmp( name, "do_goto" ) )
            return do_goto;
         if( !str_cmp( name, "do_graffiti" ) )
            return do_graffiti;
         if( !str_cmp( name, "do_group" ) )
            return do_group;
         if( !str_cmp( name, "do_grub" ) )
            return do_grub;
         if( !str_cmp( name, "do_gtell" ) )
            return do_gtell;
         if( !str_cmp( name, "do_guard" ) )
            return do_guard;
         if( !str_cmp( name, "do_gwhere" ) )
            return do_gwhere;
         break;
      case 'h':
         if( !str_cmp( name, "do_headbutt" ) )
            return do_headbutt;
         if( !str_cmp( name, "do_heal" ) )
            return do_heal;
         if( !str_cmp( name, "do_hell" ) )
            return do_hell;
         if( !str_cmp( name, "do_hide" ) )
            return do_hide;
         if( !str_cmp( name, "do_hire" ) )
            return do_hire;
         if( !str_cmp( name, "do_hiscore" ) )
            return do_hiscore;
         if( !str_cmp( name, "do_hiscoset" ) )
            return do_hiscoset;
         if( !str_cmp( name, "do_hit" ) )
            return do_hit;
         if( !str_cmp( name, "do_holyblast" ) )
            return do_holyblast;
         if( !str_cmp( name, "do_holylight" ) )
            return do_holylight;
		 if ( !str_cmp( name, "do_homebuy"))         
			return do_homebuy;
         if ( !str_cmp( name, "do_house" ))          
			return do_house;
         if( !str_cmp( name, "do_home" ) )
            return do_home;
         if( !str_cmp( name, "do_homepage" ) )
            return do_homepage;
         break;
      case 'i':
         if( !str_cmp( name, "do_idea" ) )
            return do_idea;
         if( !str_cmp( name, "do_identify" ) )
            return do_identify;
         if( !str_cmp( name, "do_immobilize" ) )
            return do_immobilize;
         if( !str_cmp( name, "do_induct" ) )
            return do_induct;
         if( !str_cmp( name, "do_info" ) )
            return do_info;
         if( !str_cmp( name, "do_ingredient" ) )
            return do_ingredient;
         if( !str_cmp( name, "do_installarea" ) )
            return do_installarea;
         if( !str_cmp( name, "do_instaroom" ) )
            return do_instaroom;
         if( !str_cmp( name, "do_instazone" ) )
            return do_instazone;
         if( !str_cmp( name, "do_introduce" ) )
            return do_introduce;
         if( !str_cmp( name, "do_invade" ) )
            return do_invade;
         if( !str_cmp( name, "do_inventory" ) )
            return do_inventory;
         if( !str_cmp( name, "do_invis" ) )
            return do_invis;
         if( !str_cmp( name, "do_invoke" ) )
            return do_invoke;
         break;
      case 'k':
         if( !str_cmp( name, "do_kick" ) )
            return do_kick;
         if( !str_cmp( name, "do_knee" ) )
            return do_knee;
         if( !str_cmp( name, "do_kneel" ) )
            return do_kneel;
         break;
      case 'l':
         if( !str_cmp( name, "do_last" ) )
            return do_last;
         if( !str_cmp( name, "do_laws" ) )
            return do_laws;
         if( !str_cmp( name, "do_leave" ) )
            return do_leave;
         if( !str_cmp( name, "do_light" ) )
            return do_light;
         if( !str_cmp( name, "do_lightning" ) )
            return do_lightning;
         if( !str_cmp( name, "do_link" ) )
            return do_link;
         if( !str_cmp( name, "do_list" ) )
            return do_list;
         if( !str_cmp( name, "do_litterbug" ) )
            return do_litterbug;
         if( !str_cmp( name, "do_land" ) )
            return do_land;
         if( !str_cmp( name, "do_load" ) )
            return do_load;
         if( !str_cmp( name, "do_loadarea" ) )
            return do_loadarea;
         if( !str_cmp( name, "do_loadup" ) )
            return do_loadup;
         if( !str_cmp( name, "do_lock" ) )
            return do_lock;
         if( !str_cmp( name, "do_locksmith" ) )
            return do_locksmith;
         if( !str_cmp( name, "do_log" ) )
            return do_log;
         if( !str_cmp( name, "do_look" ) )
            return do_look;
         if( !str_cmp( name, "do_low_purge" ) )
            return do_low_purge;
         if( !str_cmp( name, "do_lunge" ) )
            return do_lunge;
         break;
      case 'm':
         if( !str_cmp( name, "do_magic" ) )
            return do_magic;
         if( !str_cmp( name, "do_magma" ) )
            return do_magma;
         if( !str_cmp( name, "do_manastorm" ) )
            return do_manastorm;
         if( !str_cmp( name, "do_makedeity" ) )
            return do_makedeity;
         if( !str_cmp( name, "do_makeore" ) )
            return do_makeore;
         if( !str_cmp( name, "do_makerooms" ) )
            return do_makerooms;
         if( !str_cmp( name, "do_makeshop" ) )
            return do_makeshop;
         if( !str_cmp( name, "do_marry" ) )
            return do_marry;
         if( !str_cmp( name, "do_massign" ) )
            return do_massign;
         if( !str_cmp( name, "do_material" ) )
            return do_material;
         if( !str_cmp( name, "do_matshop" ) )
            return do_matshop;
         if( !str_cmp( name, "do_mcreate" ) )
            return do_mcreate;
         if( !str_cmp( name, "do_mdelete" ) )
            return do_mdelete;
         if( !str_cmp( name, "do_memory" ) )
            return do_memory;
         if( !str_cmp( name, "do_meteor" ) )
            return do_meteor;
         if( !str_cmp( name, "do_mfind" ) )
            return do_mfind;
         if( !str_cmp( name, "do_mindflay" ) )
            return do_mindflay;
         if( !str_cmp( name, "do_minvoke" ) )
            return do_minvoke;
         if( !str_cmp( name, "do_mix" ) )
            return do_mix;
         if( !str_cmp( name, "do_mlist" ) )
            return do_mlist;
         if( !str_cmp( name, "do_moblog" ) )
            return do_moblog;
         if( !str_cmp( name, "do_moon" ) )
            return do_moon;
         if( !str_cmp( name, "do_morph" ) )
            return do_morph;
         if( !str_cmp( name, "do_mount" ) )
            return do_mount;
         if( !str_cmp( name, "do_mp_close_passage" ) )
            return do_mp_close_passage;
         if( !str_cmp( name, "do_mp_damage" ) )
            return do_mp_damage;
         if( !str_cmp( name, "do_mp_deposit" ) )
            return do_mp_deposit;
         if( !str_cmp( name, "do_mp_open_passage" ) )
            return do_mp_open_passage;
         if( !str_cmp( name, "do_mp_practice" ) )
            return do_mp_practice;
         if( !str_cmp( name, "do_mp_restore" ) )
            return do_mp_restore;
         if( !str_cmp( name, "do_mp_slay" ) )
            return do_mp_slay;
         if( !str_cmp( name, "do_mpaset" ) )
            return do_mpaset;
         if( !str_cmp( name, "do_mpqset" ) )
            return do_mpqset;
         if( !str_cmp( name, "do_mp_withdraw" ) )
            return do_mp_withdraw;
         if( !str_cmp( name, "do_mpaddpoints" ) )
            return do_mpaddpoints;
         if( !str_cmp( name, "do_mpasound" ) )
            return do_mpasound;
         if( !str_cmp( name, "do_mpat" ) )
            return do_mpat;
         if( !str_cmp( name, "do_mpdelay" ) )
            return do_mpdelay;
         if( !str_cmp( name, "do_mpdream" ) )
            return do_mpdream;
         if( !str_cmp( name, "do_mpecho" ) )
            return do_mpecho;
         if( !str_cmp( name, "do_mpechoaround" ) )
            return do_mpechoaround;
         if( !str_cmp( name, "do_mpechoat" ) )
            return do_mpechoat;
         if( !str_cmp( name, "do_mpechozone" ) )
            return do_mpechozone;
         if( !str_cmp( name, "do_mpedit" ) )
            return do_mpedit;
         if( !str_cmp( name, "do_mpforce" ) )
            return do_mpforce;
         if( !str_cmp( name, "do_mpgoto" ) )
            return do_mpgoto;
         if( !str_cmp( name, "do_mpinvis" ) )
            return do_mpinvis;
         if( !str_cmp( name, "do_mpjunk" ) )
            return do_mpjunk;
         if( !str_cmp( name, "do_mpkill" ) )
            return do_mpkill;
         if( !str_cmp( name, "do_mpmload" ) )
            return do_mpmload;
         if( !str_cmp( name, "do_mpmset" ) )
            return do_mpmset;
         if( !str_cmp( name, "do_mpnothing" ) )
            return do_mpnothing;
         if( !str_cmp( name, "do_mpoload" ) )
            return do_mpoload;
         if( !str_cmp( name, "do_mposet" ) )
            return do_mposet;
         if( !str_cmp( name, "do_mppeace" ) )
            return do_mppeace;
         if( !str_cmp( name, "do_mpmakeitem" ) )
            return do_mpmakeitem;
         if( !str_cmp( name, "do_mpmakeore" ) )
            return do_mpmakeore;
         if( !str_cmp( name, "do_mppurge" ) )
            return do_mppurge;
         if( !str_cmp( name, "do_mpscatter" ) )
            return do_mpscatter;
         if( !str_cmp( name, "do_mpsellrand" ) )
            return do_mpsellrand;
         if( !str_cmp( name, "do_mpshowtalents" ) )
            return do_mpshowtalents;
         if( !str_cmp( name, "do_mpstat" ) )
            return do_mpstat;
         if( !str_cmp( name, "do_mptransfer" ) )
            return do_mptransfer;
         if( !str_cmp( name, "do_mrange" ) )
            return do_mrange;
         if( !str_cmp( name, "do_mset" ) )
            return do_mset;
         if( !str_cmp( name, "do_mstat" ) )
            return do_mstat;
         if( !str_cmp( name, "do_mutation" ) )
            return do_mutation;
         if( !str_cmp( name, "do_mwhere" ) )
            return do_mwhere;
         break;
      case 'n':
         if( !str_cmp( name, "do_namegen" ) )
            return do_namegen;
         if( !str_cmp( name, "do_nation" ) )
            return do_nation;
         if( !str_cmp( name, "do_neighbors" ) )
            return do_neighbors;
         if( !str_cmp( name, "do_news" ) )
            return do_news;
         if( !str_cmp( name, "do_new_help" ) )
            return do_new_help;
         if( !str_cmp( name, "do_nightmare" ) )
            return do_nightmare;
         if( !str_cmp( name, "do_nochannel" ) )
            return do_nochannel;
         if( !str_cmp( name, "do_noemote" ) )
            return do_noemote;
         if( !str_cmp( name, "do_noresolve" ) )
            return do_noresolve;
         if( !str_cmp( name, "do_north" ) )
            return do_north;
         if( !str_cmp( name, "do_northeast" ) )
            return do_northeast;
         if( !str_cmp( name, "do_northwest" ) )
            return do_northwest;
         if( !str_cmp( name, "do_notell" ) )
            return do_notell;
         if( !str_cmp( name, "do_notitle" ) )
            return do_notitle;
         break;
      case 'o':
         if( !str_cmp( name, "do_oassign" ) )
            return do_oassign;
		 if ( !str_cmp( name, "do_ocopy" ))		
			return do_ocopy;
         if( !str_cmp( name, "do_ocreate" ) )
            return do_ocreate;
         if( !str_cmp( name, "do_odelete" ) )
            return do_odelete;
         if( !str_cmp( name, "do_ofind" ) )
            return do_ofind;
         if( !str_cmp( name, "do_ogrub" ) )
            return do_ogrub;
         if( !str_cmp( name, "do_oinvoke" ) )
            return do_oinvoke;
         if( !str_cmp( name, "do_olist" ) )
            return do_olist;
		 if ( !str_cmp( name, "do_omedit" ))		
			return do_omedit;
         if( !str_cmp( name, "do_opedit" ) )
            return do_opedit;
		 if ( !str_cmp( name, "do_ooedit" ))		
			return do_ooedit;
         if( !str_cmp( name, "do_open" ) )
            return do_open;
         if( !str_cmp( name, "do_opstat" ) )
            return do_opstat;
         if( !str_cmp( name, "do_orange" ) )
            return do_orange;
         if( !str_cmp( name, "do_order" ) )
            return do_order;
		 if ( !str_cmp( name, "do_oredit" ))		
			return do_oredit;
         if( !str_cmp( name, "do_ooc_say" ) )
            return do_ooc_say;
         if( !str_cmp( name, "do_oset" ) )
            return do_oset;
         if( !str_cmp( name, "do_osearch" ) )
            return do_osearch;
         if( !str_cmp( name, "do_ostat" ) )
            return do_ostat;
         if( !str_cmp( name, "do_otransfer" ) )
            return do_otransfer;
         if( !str_cmp( name, "do_outcast" ) )
            return do_outcast;
         if( !str_cmp( name, "do_outputprefix" ) )
            return do_outputprefix;
         if( !str_cmp( name, "do_outputsuffix" ) )
            return do_outputsuffix;
         if( !str_cmp( name, "do_owhere" ) )
            return do_owhere;
         break;
      case 'p':
         if( !str_cmp( name, "do_pacify" ) )
            return do_pacify;
         if( !str_cmp( name, "do_pagelen" ) )
            return do_pagelen;
         if( !str_cmp( name, "do_pager" ) )
            return do_pager;
         if( !str_cmp( name, "do_password" ) )
            return do_password;
         if( !str_cmp( name, "do_plant" ) )
            return do_plant;
         if( !str_cmp( name, "do_player_echo" ) )
            return do_player_echo;
         if( !str_cmp( name, "do_pcrename" ) )
            return do_pcrename;
         if( !str_cmp( name, "do_peace" ) )
            return do_peace;
         if( !str_cmp( name, "do_permit" ) )
            return do_permit;
         if( !str_cmp( name, "do_pick" ) )
            return do_pick;
         if( !str_cmp( name, "do_piss" ) )
            return do_piss;
         if( !str_cmp( name, "do_pound" ) )
            return do_pound;
         if( !str_cmp( name, "do_poisoncloud" ) )
            return do_poisoncloud;
         if( !str_cmp( name, "do_practice" ) )
            return do_practice;
         if( !str_cmp( name, "do_preserve" ) )
            return do_preserve;
         if( !str_cmp( name, "do_protect" ) )
            return do_protect;
         if( !str_cmp( name, "do_prompt" ) )
            return do_prompt;
         if( !str_cmp( name, "do_pry" ) )
            return do_pry;
         if( !str_cmp( name, "do_pull" ) )
            return do_pull;
         if( !str_cmp( name, "do_punch" ) )
            return do_punch;
         if( !str_cmp( name, "do_purge" ) )
            return do_purge;
         if( !str_cmp( name, "do_purify" ) )
            return do_purify;
         if( !str_cmp( name, "do_push" ) )
            return do_push;
         if( !str_cmp( name, "do_put" ) )
            return do_put;
         if( !str_cmp( name, "do_pyrokinetics" ) )
            return do_pyrokinetics;
         break;
      case 'q':
         if( !str_cmp( name, "do_qbit" ) )
            return do_qbit;
         if( !str_cmp( name, "do_quake" ) )
            return do_quake;
         if( !str_cmp( name, "do_quaff" ) )
            return do_quaff;
         if( !str_cmp( name, "do_qui" ) )
            return do_qui;
         if( !str_cmp( name, "do_quit" ) )
            return do_quit;
         break;
      case 'r':
         if( !str_cmp( name, "do_races" ) )
            return do_races;
         if( !str_cmp( name, "do_radio" ) )
            return do_radio;
         if( !str_cmp( name, "do_rank" ) )
            return do_rank;
         if( !str_cmp( name, "do_rap" ) )
            return do_rap;
         if( !str_cmp( name, "do_rassign" ) )
            return do_rassign;
         if( !str_cmp( name, "do_rat" ) )
            return do_rat;
//		 if( !str_cmp( name, "do_rcreate" ) )
//			return do_rcreate;
         if( !str_cmp( name, "do_ready" ) )
            return do_ready;
         if( !str_cmp( name, "do_rebirth" ) )
            return do_rebirth;
         if( !str_cmp( name, "do_reboo" ) )
            return do_reboo;
         if( !str_cmp( name, "do_reboot" ) )
            return do_reboot;
         if( !str_cmp( name, "do_recall" ) )
            return do_recall;
         if( !str_cmp( name, "do_recho" ) )
            return do_recho;
         if( !str_cmp( name, "do_recipe" ) )
            return do_recipe;
         if( !str_cmp( name, "do_recite" ) )
            return do_recite;
         if( !str_cmp( name, "do_redit" ) )
            return do_redit;
         if( !str_cmp( name, "do_regoto" ) )
            return do_regoto;
         if( !str_cmp( name, "do_relax" ) )
            return do_relax;
         if( !str_cmp( name, "do_remove" ) )
            return do_remove;
         if( !str_cmp( name, "do_rent" ) )
            return do_rent;
         if( !str_cmp( name, "do_repair" ) )
            return do_repair;
         if( !str_cmp( name, "do_reply" ) )
            return do_reply;
         if( !str_cmp( name, "do_report" ) )
            return do_report;
         if( !str_cmp( name, "do_reserve" ) )
            return do_reserve;
         if( !str_cmp( name, "do_reset" ) )
            return do_reset;
		 if ( !str_cmp( name, "do_residence"))       
			return do_residence;
         if( !str_cmp( name, "do_reshape" ) )
            return do_reshape;
         if( !str_cmp( name, "do_rest" ) )
            return do_rest;
         if( !str_cmp( name, "do_restore" ) )
            return do_restore;
         if( !str_cmp( name, "do_restoretime" ) )
            return do_restoretime;
         if( !str_cmp( name, "do_resurrect" ) )
            return do_resurrect;
         if( !str_cmp( name, "do_retell" ) )
            return do_retell;
         if( !str_cmp( name, "do_retran" ) )
            return do_retran;
         if( !str_cmp( name, "do_retrieve" ) )
            return do_retrieve;
         if( !str_cmp( name, "do_return" ) )
            return do_return;
         if( !str_cmp( name, "do_rfind" ) )
            return do_rfind;
         if( !str_cmp( name, "do_rgrub" ) )
            return do_rgrub;
         if( !str_cmp( name, "do_rlist" ) )
            return do_rlist;
         if( !str_cmp( name, "do_roll" ) )
            return do_roll;
         if( !str_cmp( name, "do_roleplay" ) )
            return do_roleplay;
         if( !str_cmp( name, "do_rpedit" ) )
            return do_rpedit;
         if( !str_cmp( name, "do_rpstat" ) )
            return do_rpstat;
         if( !str_cmp( name, "do_rset" ) )
            return do_rset;
         if( !str_cmp( name, "do_rstat" ) )
            return do_rstat;
         if( !str_cmp( name, "do_rune" ) )
            return do_rune;
         break;
      case 's':
         if( !str_cmp( name, "do_sandstorm" ) )
            return do_sandstorm;
         if( !str_cmp( name, "do_save" ) )
            return do_save;
         if( !str_cmp( name, "do_saveall" ) )
            return do_saveall;
         if( !str_cmp( name, "do_savearea" ) )
            return do_savearea;
         if( !str_cmp( name, "do_say" ) )
            return do_say;
         if( !str_cmp( name, "do_scan" ) )
            return do_scan;
         if( !str_cmp( name, "do_scatter" ) )
            return do_scatter;
         if( !str_cmp( name, "do_score" ) )
            return do_score;
         if( !str_cmp( name, "do_scribe" ) )
            return do_scribe;
         if( !str_cmp( name, "do_search" ) )
            return do_search;
         if( !str_cmp( name, "do_seek" ) )
            return do_seek;
         if( !str_cmp( name, "do_sedit" ) )
            return do_sedit;
         if( !str_cmp( name, "do_sell" ) )
            return do_sell;
		 if ( !str_cmp( name, "do_sellhouse" ))	
			return do_sellhouse;
         if( !str_cmp( name, "do_set_boot_time" ) )
            return do_set_boot_time;
         if( !str_cmp( name, "do_setdeity" ) )
            return do_setdeity;
         if( !str_cmp( name, "do_set_book" ) )
            return do_set_book;
         if( !str_cmp( name, "do_set_help" ) )
            return do_set_help;
         if( !str_cmp( name, "do_setweather" ) )
            return do_setweather;
         if( !str_cmp( name, "do_sheathe" ) )
            return do_sheathe;
         if( !str_cmp( name, "do_shield" ) )
            return do_shield;
         if( !str_cmp( name, "do_shock" ) )
            return do_shock;
         if( !str_cmp( name, "do_shoot" ) )
            return do_shoot;
         if( !str_cmp( name, "do_shops" ) )
            return do_shops;
         if( !str_cmp( name, "do_shopset" ) )
            return do_shopset;
         if( !str_cmp( name, "do_shopstat" ) )
            return do_shopstat;
         if( !str_cmp( name, "do_shove" ) )
            return do_shove;
         if( !str_cmp( name, "do_showabit" ) )
            return do_showabit;
         if( !str_cmp( name, "do_showqbit" ) )
            return do_showqbit;
         if( !str_cmp( name, "do_setabit" ) )
            return do_setabit;
         if( !str_cmp( name, "do_setqbit" ) )
            return do_setqbit;
         if( !str_cmp( name, "do_showdeity" ) )
            return do_showdeity;
         if( !str_cmp( name, "do_showweather" ) )
            return do_showweather;
         if( !str_cmp( name, "do_shutdow" ) )
            return do_shutdow;
         if( !str_cmp( name, "do_shutdown" ) )
            return do_shutdown;
         if( !str_cmp( name, "do_silence" ) )
            return do_silence;
         if( !str_cmp( name, "do_sing" ) )
            return do_sing;
         if( !str_cmp( name, "do_sit" ) )
            return do_sit;
         if( !str_cmp( name, "do_skills" ) )
            return do_skills;
         if( !str_cmp( name, "do_skin" ) )
            return do_skin;
         if( !str_cmp( name, "do_slap" ) )
            return do_slap;
         if( !str_cmp( name, "do_slash" ) )
            return do_slash;
         if( !str_cmp( name, "do_slay" ) )
            return do_slay;
         if( !str_cmp( name, "do_sleep" ) )
            return do_sleep;
         if( !str_cmp( name, "do_slookup" ) )
            return do_slookup;
         if( !str_cmp( name, "do_smite" ) )
            return do_smite;
         if( !str_cmp( name, "do_smoke" ) )
            return do_smoke;
         if( !str_cmp( name, "do_snoop" ) )
            return do_snoop;
         if( !str_cmp( name, "do_sober" ) )
            return do_sober;
         if( !str_cmp( name, "do_socials" ) )
            return do_socials;
         if( !str_cmp( name, "do_sonics" ) )
            return do_sonics;
         if( !str_cmp( name, "do_soulfire" ) )
            return do_soulfire;
         if( !str_cmp( name, "do_south" ) )
            return do_south;
         if( !str_cmp( name, "do_southeast" ) )
            return do_southeast;
         if( !str_cmp( name, "do_southwest" ) )
            return do_southwest;
         if( !str_cmp( name, "do_squat" ) )
            return do_squat;
         if( !str_cmp( name, "do_species" ) )
            return do_species;
         if( !str_cmp( name, "do_spike" ) )
            return do_spike;
         if( !str_cmp( name, "do_splash" ) )
            return do_splash;
         if( !str_cmp( name, "do_split" ) )
            return do_split;
         if( !str_cmp( name, "do_sset" ) )
            return do_sset;
         if( !str_cmp( name, "do_stab" ) )
            return do_stab;
         if( !str_cmp( name, "do_stand" ) )
            return do_stand;
         if( !str_cmp( name, "do_status" ) )
            return do_status;
         if( !str_cmp( name, "do_steal" ) )
            return do_steal;
         if( !str_cmp( name, "do_steam" ) )
            return do_steam;
         if( !str_cmp( name, "do_storm" ) )
            return do_storm;
         if( !str_cmp( name, "do_story" ) )
            return do_story;
         if( !str_cmp( name, "do_strew" ) )
            return do_strew;
         if( !str_cmp( name, "do_strip" ) )
            return do_strip;
         if( !str_cmp( name, "do_summon" ) )
            return do_summon;
         if( !str_cmp( name, "do_sunstroke" ) )
            return do_sunstroke;
         if( !str_cmp( name, "do_switch" ) )
            return do_switch;
         if( !str_cmp( name, "do_sworddance" ) )
            return do_sworddance;
         if( !str_cmp( name, "do_showlayers" ) )
            return do_showlayers;
         if( !str_cmp( name, "do_suicide" ) )
            return do_suicide;
         break;
      case 't':
         if( !str_cmp( name, "do_tail" ) )
            return do_tail;
         if( !str_cmp( name, "do_taint" ) )
            return do_taint;
         if( !str_cmp( name, "do_talent_advance" ) )
            return do_talent_advance;
         if( !str_cmp( name, "do_tap" ) )
            return do_tap;
         if( !str_cmp( name, "do_tamp" ) )
            return do_tamp;
         if( !str_cmp( name, "do_telekinetics" ) )
            return do_telekinetics;
         if( !str_cmp( name, "do_tell" ) )
            return do_tell;
         if( !str_cmp( name, "do_think" ) )
            return do_think;
         if( !str_cmp( name, "do_throw" ) )
            return do_throw;
         if( !str_cmp( name, "do_time" ) )
            return do_time;
         if( !str_cmp( name, "do_timecmd" ) )
            return do_timecmd;
         if( !str_cmp( name, "do_title" ) )
            return do_title;
         if( !str_cmp( name, "do_track" ) )
            return do_track;
         if( !str_cmp( name, "do_tradein" ) )
            return do_tradein;
         if( !str_cmp( name, "do_train" ) )
            return do_train;
         if( !str_cmp( name, "do_transfer" ) )
            return do_transfer;
         if( !str_cmp( name, "do_travel" ) )
            return do_travel;
         if( !str_cmp( name, "do_turn" ) )
            return do_turn;
         if( !str_cmp( name, "do_typo" ) )
            return do_typo;
         break;
      case 'u':
         if( !str_cmp( name, "do_unfoldarea" ) )
            return do_unfoldarea;
         if( !str_cmp( name, "do_unhell" ) )
            return do_unhell;
         if( !str_cmp( name, "do_unholyblast" ) )
            return do_unholyblast;
         if( !str_cmp( name, "do_unlock" ) )
            return do_unlock;
         if( !str_cmp( name, "do_unsilence" ) )
            return do_unsilence;
         if( !str_cmp( name, "do_up" ) )
            return do_up;
         if( !str_cmp( name, "do_users" ) )
            return do_users;
         break;
      case 'v':
         if( !str_cmp( name, "do_value" ) )
            return do_value;
         if( !str_cmp( name, "do_ventriloquate" ) )
            return do_ventriloquate;
         if( !str_cmp( name, "do_vines" ) )
            return do_vines;
         if( !str_cmp( name, "do_vnums" ) )
            return do_vnums;
         if( !str_cmp( name, "do_void" ) )
            return do_void;
         if( !str_cmp( name, "do_vsearch" ) )
            return do_vsearch;
         break;
      case 'w':
         if( !str_cmp( name, "do_wail" ) )
            return do_wail;
         if( !str_cmp( name, "do_wake" ) )
            return do_wake;
         if( !str_cmp( name, "do_weapons" ) )
            return do_weapons;
         if( !str_cmp( name, "do_wear" ) )
            return do_wear;
         if( !str_cmp( name, "do_weather" ) )
            return do_weather;
         if( !str_cmp( name, "do_west" ) )
            return do_west;
         if( !str_cmp( name, "do_where" ) )
            return do_where;
         if( !str_cmp( name, "do_whisper" ) )
            return do_whisper;
         if( !str_cmp( name, "do_whip" ) )
            return do_whip;
         if( !str_cmp( name, "do_who" ) )
            return do_who;
         if( !str_cmp( name, "do_whois" ) )
            return do_whois;
         if( !str_cmp( name, "do_wield" ) )
            return do_wield;
         if( !str_cmp( name, "do_wind" ) )
            return do_wind;
         if( !str_cmp( name, "do_withdraw" ) )
            return do_withdraw;
         if( !str_cmp( name, "do_wizhelp" ) )
            return do_wizhelp;
         if( !str_cmp( name, "do_wizlock" ) )
            return do_wizlock;
         break;
      case 'y':
         break;
      case 'z':
         if( !str_cmp( name, "do_zones" ) )
            return do_zones;
   }
   return skill_notfound;
}

char *spell_name( SPELL_FUN * spell )
{
   if( spell == spell_smaug )
      return "spell_smaug";
   if( spell == spell_blindness )
      return "spell_blindness";
   if( spell == spell_cure_blindness )
      return "spell_cure_blindness";
   if( spell == spell_cure_poison )
      return "spell_cure_poison";
   if( spell == spell_curse )
      return "spell_curse";
   if( spell == spell_pigeon )
      return "spell_pigeon";
   if( spell == spell_poison )
      return "spell_poison";
   if( spell == spell_remove_curse )
      return "spell_remove_curse";
   if( spell == spell_sleep )
      return "spell_sleep";
   if( spell == spell_portal )
      return "spell_portal";

   if( spell == spell_null )
      return "spell_null";
   return "reserved";
}

char *skill_name( DO_FUN * skill )
{
   static char buf[64];

   if( skill == NULL )
      return "reserved";
   if( skill == do_aassign )
      return "do_aassign";
   if( skill == do_abit )
      return "do_abit";
   if( skill == do_adopt )
      return "do_adopt";
   if( skill == do_aentrance )
      return "do_aentrance";
   if( skill == do_aexit )
      return "do_aexit";
   if( skill == do_affected )
      return "do_affected";
   if( skill == do_affix )
      return "do_affix";
   if( skill == do_afk )
      return "do_afk";
   if( skill == do_aid )
      return "do_aid";
   if( skill == do_alias )
      return "do_alias";
   if( skill == do_allow )
      return "do_allow";
   if( skill == do_alter )
      return "do_alter";
   if( skill == do_ansi )
      return "do_ansi";
   if( skill == do_antimatter )
      return "do_antimatter";
   if( skill == do_apply )
      return "do_apply";
   if( skill == do_archmages )
      return "do_archmages";
   if( skill == do_areas )
      return "do_areas";
   if( skill == do_arena )
      return "do_arena";
   if( skill == do_aset )
      return "do_aset";
   if( skill == do_astat )
      return "do_astat";
   if( skill == do_at )
      return "do_at";
   if( skill == do_atmob )
      return "do_atmob";
   if( skill == do_atobj )
      return "do_atobj";
   if( skill == do_attributes )
      return "do_attributes";
   if( skill == do_auto )
      return "do_auto";
   if( skill == do_award )
      return "do_award";
   if( skill == do_balefire )
      return "do_balefire";
   if( skill == do_bamfin )
      return "do_bamfin";
   if( skill == do_bamfout )
      return "do_bamfout";
   if( skill == do_ban )
      return "do_ban";
   if( skill == do_bandage )
      return "do_bandage";
   if( skill == do_bashdoor )
      return "do_bashdoor";
   if( skill == do_beep )
      return "do_beep";
   if( skill == do_bestow )
      return "do_bestow";
   if( skill == do_bind )
      return "do_bind";
   if( skill == do_bio )
      return "do_bio";
   if( skill == do_bite )
      return "do_bite";
   if( skill == do_blast )
      return "do_blast";
   if( skill == do_blink )
      return "do_blink";
   if( skill == do_blizzard )
      return "do_blizzard";
   if( skill == do_body )
      return "do_body";
   if( skill == do_bodybag )
      return "do_bodybag";
   if( skill == do_bookwrite )
      return "do_bookwrite";
   if( skill == do_boneset )
      return "do_boneset";
   if( skill == do_bounty )
      return "do_bounty";
   if( skill == do_brainshock )
      return "do_brainshock";
   if( skill == do_brand )
      return "do_brand";
   if( skill == do_brew )
      return "do_brew";
   if( skill == do_bug )
      return "do_bug";
   if( skill == do_build )
      return "do_build";
   if( skill == do_buildwalk )
      return "do_buildwalk";
   if( skill == do_burn )
      return "do_burn";
   if( skill == do_bury )
      return "do_bury";
   if( skill == do_buy )
      return "do_buy";
   if( skill == do_cast )
      return "do_cast";
   if( skill == do_cedit )
      return "do_cedit";
   if( skill == do_chainlightning )
      return "do_chainlightning";
   if( skill == do_change )
      return "do_change";
   if( skill == do_channels )
      return "do_channels";
   if( skill == do_chat )
      return "do_chat";
   if( skill == do_check_vnums )
      return "do_check_vnums";
   if( skill == do_chill )
      return "do_chill";
   if( skill == do_chop )
      return "do_chop";
   if( skill == do_city )
      return "do_city";
   if( skill == do_claw )
      return "do_claw";
   if( skill == do_clan )
      return "do_clan";
   if( skill == do_clear )
      return "do_clear";
   if( skill == do_climate )
      return "do_climate";
   if( skill == do_climb )
      return "do_climb";
   if( skill == do_close )
      return "do_close";
   if( skill == do_cmdtable )
      return "do_cmdtable";
   if( skill == do_combine )
      return "do_combine";
   if( skill == do_commands )
      return "do_commands";
   if( skill == do_compare )
      return "do_compare";
   if( skill == do_config )
      return "do_config";
   if( skill == do_connect )
      return "do_connect";
   if( skill == do_consent )
      return "do_consent";
   if( skill == do_consider )
      return "do_consider";
   if( skill == do_copyover )
      return "do_copyover";
   if( skill == do_credits )
      return "do_credits";
   if( skill == do_cset )
      return "do_cset";
   if( skill == do_cure )
      return "do_cure";
   if( skill == do_decay )
      return "do_decay";
   if( skill == do_deduct )
      return "do_deduct";
   if( skill == do_deities )
      return "do_deities";
   if( skill == do_delay )
      return "do_delay";
   if( skill == do_deny )
      return "do_deny";
   if( skill == do_deposit )
      return "do_deposit";
   if( skill == do_description )
      return "do_description";
   if( skill == do_destro )
      return "do_destro";
   if( skill == do_destroy )
      return "do_destroy";
   if( skill == do_detonate )
      return "do_detonate";
   if( skill == do_detrap )
      return "do_detrap";
   if( skill == do_dig )
      return "do_dig";
   if( skill == do_disarm )
      return "do_disarm";
   if( skill == do_disconnect )
      return "do_disconnect";
   if( skill == do_dismiss )
      return "do_dismiss";
   if( skill == do_dismount )
      return "do_dismount";
   if( skill == do_disown )
      return "do_disown";
   if( skill == do_dispel )
      return "do_dispel";
   if( skill == do_distort )
      return "do_distort";
   if( skill == do_divorce )
      return "do_divorce";
   if( skill == do_dmesg )
      return "do_dmesg";
   if( skill == do_down )
      return "do_down";
   if( skill == do_drag )
      return "do_drag";
   if( skill == do_drain )
      return "do_drain";
   if( skill == do_dream )
      return "do_dream";
   if( skill == do_drink )
      return "do_drink";
   if( skill == do_drop )
      return "do_drop";
   if( skill == do_diagnose )
      return "do_diagnose";
   if( skill == do_dye )
      return "do_dye";
   if( skill == do_east )
      return "do_east";
   if( skill == do_eat )
      return "do_eat";
   if( skill == do_echo )
      return "do_echo";
   if( skill == do_elbow )
      return "do_elbow";
   if( skill == do_electrocute )
      return "do_electrocute";
   if( skill == do_emerge )
      return "do_emerge";
   if( skill == do_emote )
      return "do_emote";
   if( skill == do_empty )
      return "do_empty";
   if( skill == do_enchant )
      return "do_enchant";
   if( skill == do_engrave )
      return "do_engrave";
   if( skill == do_enter )
      return "do_enter";
   if( skill == do_equipment )
      return "do_equipment";
   if( skill == do_examine )
      return "do_examine";
   if( skill == do_exits )
      return "do_exits";
   if( skill == do_experience )
      return "do_experience";
   if( skill == do_family )
      return "do_family";
   if( skill == do_fashion )
      return "do_fashion";
   if( skill == do_feed )
      return "do_feed";
   if( skill == do_fill )
      return "do_fill";
   if( skill == do_fixchar )
      return "do_fixchar";
   if( skill == do_flamingrain )
      return "do_flamingrain";
   if( skill == do_flare )
      return "do_flare";
   if( skill == do_fly )
      return "do_fly";
   if( skill == do_flee )
      return "do_flee";
   if( skill == do_flood )
      return "do_flood";
   if( skill == do_foldarea )
      return "do_foldarea";
   if( skill == do_follow )
      return "do_follow";
   if( skill == do_for )
      return "do_for";
   if( skill == do_force )
      return "do_force";
   if( skill == do_forceclose )
      return "do_forceclose";
   if( skill == do_form_password )
      return "do_form_password";
   if( skill == do_fprompt )
      return "do_fprompt";
   if( skill == do_fquit )
      return "do_fquit";
   if( skill == do_freeze )
      return "do_freeze";
   if( skill == do_frostbite )
      return "do_frostbite";
   if( skill == do_funnel )
      return "do_funnel";
   if( skill == do_get )
      return "do_get";
   if( skill == do_genarea )
      return "do_genarea";
   if( skill == do_geyser )
      return "do_geyser";
   if( skill == do_give )
      return "do_give";
   if( skill == do_glance )
      return "do_glance";
   if( skill == do_gold )
      return "do_gold";
   if( skill == do_goto )
      return "do_goto";
   if( skill == do_graffiti )
      return "do_graffiti";
   if( skill == do_group )
      return "do_group";
   if( skill == do_grub )
      return "do_grub";
   if( skill == do_gtell )
      return "do_gtell";
   if( skill == do_guard )
      return "do_guard";
   if( skill == do_gwhere )
      return "do_gwhere";
   if( skill == do_heal )
      return "do_heal";
   if( skill == do_headbutt )
      return "do_headbutt";
   if( skill == do_hell )
      return "do_hell";
   if( skill == do_hide )
      return "do_hide";
   if( skill == do_hire )
      return "do_hire";
   if( skill == do_hiscore )
      return "do_hiscore";
   if( skill == do_hiscoset )
      return "do_hiscoset";
   if( skill == do_hit )
      return "do_hit";
   if( skill == do_holyblast )
      return "do_holyblast";
   if( skill == do_holylight )
      return "do_holylight";
   if( skill == do_home )
      return "do_home";
   if( skill == do_homepage )
      return "do_homepage";
   if ( skill == do_accessories )              
	  return "do_accessories";
   if ( skill == do_gohome )	               
	  return "do_gohome";
   if ( skill == do_homebuy )	                
	  return "do_homebuy";
   if ( skill == do_house )	                
	  return "do_house";
   if ( skill == do_residence )                
	  return "do_residence";
   if ( skill == do_sellhouse )                
	  return "do_sellhouse";
   if( skill == do_idea )
      return "do_idea";
   if( skill == do_identify )
      return "do_identify";
   if( skill == do_immobilize )
      return "do_immobilize";
   if( skill == do_induct )
      return "do_induct";
   if( skill == do_info )
      return "do_info";
   if( skill == do_ingredient )
      return "do_ingredient";
   if( skill == do_installarea )
      return "do_installarea";
   if( skill == do_instaroom )
      return "do_instaroom";
   if( skill == do_instazone )
      return "do_instazone";
   if( skill == do_introduce )
      return "do_introduce";
   if( skill == do_invade )
      return "do_invade";
   if( skill == do_inventory )
      return "do_inventory";
   if( skill == do_invis )
      return "do_invis";
   if( skill == do_invoke )
      return "do_invoke";
   if( skill == do_kick )
      return "do_kick";
   if( skill == do_knee )
      return "do_knee";
   if( skill == do_kneel )
      return "do_kneel";
   if( skill == do_last )
      return "do_last";
   if( skill == do_laws )
      return "do_laws";
   if( skill == do_leave )
      return "do_leave";
   if( skill == do_light )
      return "do_light";
   if( skill == do_lightning )
      return "do_lightning";
   if( skill == do_link )
      return "do_link";
   if( skill == do_list )
      return "do_list";
   if ( skill == do_omedit )		
	  return "do_omedit";
   if ( skill == do_ooedit )		
	   return "do_ooedit";
   if( skill == do_litterbug )
      return "do_litterbug";
   if( skill == do_land )
      return "do_land";
   if( skill == do_load )
      return "do_load";
   if( skill == do_loadarea )
      return "do_loadarea";
   if( skill == do_loadup )
      return "do_loadup";
   if( skill == do_lock )
      return "do_lock";
   if( skill == do_locksmith )
      return "do_locksmith";
   if( skill == do_log )
      return "do_log";
   if( skill == do_look )
      return "do_look";
   if( skill == do_low_purge )
      return "do_low_purge";
   if( skill == do_lunge )
      return "do_lunge";
   if( skill == do_magic )
      return "do_magic";
   if( skill == do_magma )
      return "do_magma";
   if( skill == do_manastorm )
      return "do_manastorm";
   if( skill == do_makedeity )
      return "do_makedeity";
   if( skill == do_makeore )
      return "do_makeore";
   if( skill == do_makerooms )
      return "do_makerooms";
   if( skill == do_makeshop )
      return "do_makeshop";
   if( skill == do_marry )
      return "do_marry";
   if( skill == do_massign )
      return "do_massign";
   if( skill == do_material )
      return "do_material";
   if( skill == do_matshop )
      return "do_matshop";
   if( skill == do_mcreate )
      return "do_mcreate";
   if( skill == do_mdelete )
      return "do_mdelete";
   if( skill == do_memory )
      return "do_memory";
   if( skill == do_meteor )
      return "do_meteor";
   if( skill == do_mfind )
      return "do_mfind";
   if( skill == do_mindflay )
      return "do_mindflay";
   if( skill == do_minvoke )
      return "do_minvoke";
   if( skill == do_mix )
      return "do_mix";
   if( skill == do_mlist )
      return "do_mlist";
   if( skill == do_moblog )
      return "do_moblog";
   if( skill == do_moon )
      return "do_moon";
   if( skill == do_morph )
      return "do_morph";
   if( skill == do_mount )
      return "do_mount";
   if( skill == do_mp_close_passage )
      return "do_mp_close_passage";
   if( skill == do_mp_damage )
      return "do_mp_damage";
   if( skill == do_mp_deposit )
      return "do_mp_deposit";
   if( skill == do_mp_open_passage )
      return "do_mp_open_passage";
   if( skill == do_mp_practice )
      return "do_mp_practice";
   if( skill == do_mp_restore )
      return "do_mp_restore";
   if( skill == do_mpaset )
      return "do_mpaset";
   if( skill == do_mpqset )
      return "do_mpqset";
   if( skill == do_mp_slay )
      return "do_mp_slay";
   if( skill == do_mp_withdraw )
      return "do_mp_withdraw";
   if( skill == do_mpaddpoints )
      return "do_mpaddpoints";
   if( skill == do_mpasound )
      return "do_mpasound";
   if( skill == do_mpat )
      return "do_mpat";
   if( skill == do_mpdelay )
      return "do_mpdelay";
   if( skill == do_mpdream )
      return "do_mpdream";
   if( skill == do_mpecho )
      return "do_mpecho";
   if( skill == do_mpechoaround )
      return "do_mpechoaround";
   if( skill == do_mpechoat )
      return "do_mpechoat";
   if( skill == do_mpechozone )
      return "do_mpechozone";
   if( skill == do_mpedit )
      return "do_mpedit";
   if( skill == do_mpforce )
      return "do_mpforce";
   if( skill == do_mpgoto )
      return "do_mpgoto";
   if( skill == do_mpinvis )
      return "do_mpinvis";
   if( skill == do_mpjunk )
      return "do_mpjunk";
   if( skill == do_mpkill )
      return "do_mpkill";
   if( skill == do_mpmload )
      return "do_mpmload";
   if( skill == do_mpmset )
      return "do_mpmset";
   if( skill == do_mpnothing )
      return "do_mpnothing";
   if( skill == do_mpoload )
      return "do_mpoload";
   if( skill == do_mposet )
      return "do_mposet";
   if( skill == do_mppeace )
      return "do_mppeace";
   if( skill == do_mpmakeitem )
      return "do_mpmakeitem";
   if( skill == do_mpmakeore )
      return "do_mpmakeore";
   if( skill == do_mppurge )
      return "do_mppurge";
   if( skill == do_mpscatter )
      return "do_mpscatter";
   if( skill == do_mpsellrand )
      return "do_mpsellrand";
   if( skill == do_mpshowtalents )
      return "do_mpshowtalents";
   if( skill == do_mpstat )
      return "do_mpstat";
   if( skill == do_mptransfer )
      return "do_mptransfer";
   if( skill == do_mrange )
      return "do_mrange";
   if( skill == do_mset )
      return "do_mset";
   if( skill == do_mstat )
      return "do_mstat";
   if( skill == do_mutation )
      return "do_mutation";
   if( skill == do_mwhere )
      return "do_mwhere";
   if( skill == do_namegen )
      return "do_namegen";
   if( skill == do_nation )
      return "do_nation";
   if( skill == do_neighbors )
      return "do_neighbors";
   if( skill == do_news )
      return "do_news";
   if( skill == do_new_help )
      return "do_new_help";
   if( skill == do_nightmare )
      return "do_nightmare";
   if( skill == do_nochannel )
      return "do_nochannel";
   if( skill == do_noemote )
      return "do_noemote";
   if( skill == do_noresolve )
      return "do_noresolve";
   if( skill == do_north )
      return "do_north";
   if( skill == do_northeast )
      return "do_northeast";
   if( skill == do_northwest )
      return "do_northwest";
   if( skill == do_notell )
      return "do_notell";
   if( skill == do_notitle )
      return "do_notitle";
   if( skill == do_oassign )
      return "do_oassign";
   if ( skill == do_ocopy )		
	  return "do_ocopy";
   if( skill == do_ocreate )
      return "do_ocreate";
   if( skill == do_odelete )
      return "do_odelete";
   if( skill == do_ofind )
      return "do_ofind";
   if( skill == do_ogrub )
      return "do_ogrub";
   if( skill == do_oinvoke )
      return "do_oinvoke";
   if( skill == do_olist )
      return "do_olist";
   if( skill == do_opedit )
      return "do_opedit";
   if( skill == do_open )
      return "do_open";
   if( skill == do_opstat )
      return "do_opstat";
   if( skill == do_orange )
      return "do_orange";
   if( skill == do_order )
      return "do_order";
   if ( skill == do_oredit )		
	  return "do_oredit";
   if( skill == do_oset )
      return "do_oset";
   if( skill == do_osearch )
      return "do_osearch";
   if( skill == do_ostat )
      return "do_ostat";
   if( skill == do_otransfer )
      return "do_otransfer";
   if( skill == do_outcast )
      return "do_outcast";
   if( skill == do_ooc_say )
      return "do_ooc_say";
   if( skill == do_outputprefix )
      return "do_outputprefix";

   if( skill == do_outputsuffix )
      return "do_outputsuffix";
   if( skill == do_owhere )
      return "do_owhere";
   if( skill == do_pacify )
      return "do_pacify";
   if( skill == do_pagelen )
      return "do_pagelen";
   if( skill == do_pager )
      return "do_pager";
   if( skill == do_password )
      return "do_password";
   if( skill == do_plant )
      return "do_plant";
   if( skill == do_player_echo )
      return "do_player_echo";
   if( skill == do_pcrename )
      return "do_pcrename";
   if( skill == do_peace )
      return "do_peace";
   if( skill == do_permit )
      return "do_permit";
   if( skill == do_pick )
      return "do_pick";
   if( skill == do_piss )
      return "do_piss";
   if( skill == do_pound )
      return "do_pound";
   if( skill == do_poisoncloud )
      return "do_poisoncloud";
   if( skill == do_practice )
      return "do_practice";
   if( skill == do_preserve )
      return "do_preserve";
   if( skill == do_prompt )
      return "do_prompt";
   if( skill == do_protect )
      return "do_protect";
   if( skill == do_pry )
      return "do_pry";
   if( skill == do_pull )
      return "do_pull";
   if( skill == do_punch )
      return "do_punch";
   if( skill == do_purge )
      return "do_purge";
   if( skill == do_purify )
      return "do_purify";
   if( skill == do_push )
      return "do_push";
   if( skill == do_put )
      return "do_put";
   if( skill == do_pyrokinetics )
      return "do_pyrokinetics";
   if( skill == do_qbit )
      return "do_qbit";
   if( skill == do_quaff )
      return "do_quaff";
   if( skill == do_quake )
      return "do_quake";
   if( skill == do_qui )
      return "do_qui";
   if( skill == do_quit )
      return "do_quit";
   if( skill == do_races )
      return "do_races";
   if( skill == do_radio )
      return "do_radio";
   if( skill == do_rank )
      return "do_rank";
   if( skill == do_rap )
      return "do_rap";
   if( skill == do_rassign )
      return "do_rassign";
   if( skill == do_rat )
      return "do_rat";
   if( skill == do_ready )
      return "do_ready";
   if( skill == do_rebirth )
      return "do_rebirth";
   if( skill == do_reboo )
      return "do_reboo";
   if( skill == do_reboot )
      return "do_reboot";
   if( skill == do_recall )
      return "do_recall";
   if( skill == do_recho )
      return "do_recho";
   if( skill == do_recipe )
      return "do_recipe";
   if( skill == do_recite )
      return "do_recite";
   if( skill == do_redit )
      return "do_redit";
   if( skill == do_regoto )
      return "do_regoto";
   if( skill == do_relax )
      return "do_relax";
   if( skill == do_remove )
      return "do_remove";
   if( skill == do_rent )
      return "do_rent";
   if( skill == do_repair )
      return "do_repair";
   if( skill == do_reply )
      return "do_reply";
   if( skill == do_report )
      return "do_report";
   if( skill == do_reserve )
      return "do_reserve";
   if( skill == do_reset )
      return "do_reset";
   if( skill == do_reshape )
      return "do_reshape";
   if( skill == do_rest )
      return "do_rest";
   if( skill == do_restore )
      return "do_restore";
   if( skill == do_restoretime )
      return "do_restoretime";
   if( skill == do_resurrect )
      return "do_resurrect";
   if( skill == do_retell )
      return "do_retell";
   if( skill == do_retran )
      return "do_retran";
   if( skill == do_retrieve )
      return "do_retrieve";
   if( skill == do_return )
      return "do_return";
   if( skill == do_rfind )
      return "do_rfind";
   if( skill == do_rgrub )
      return "do_rgrub";
   if( skill == do_rlist )
      return "do_rlist";
   if( skill == do_roleplay )
      return "do_roleplay";
   if( skill == do_roll )
      return "do_roll";
   if( skill == do_rpedit )
      return "do_rpedit";
   if( skill == do_rpstat )
      return "do_rpstat";
   if( skill == do_rset )
      return "do_rset";
   if( skill == do_rstat )
      return "do_rstat";
   if( skill == do_rune )
      return "do_rune";
   if( skill == do_sandstorm )
      return "do_sandstorm";
   if( skill == do_save )
      return "do_save";
   if( skill == do_saveall )
      return "do_saveall";
   if( skill == do_savearea )
      return "do_savearea";
   if( skill == do_say )
      return "do_say";
   if( skill == do_scan )
      return "do_scan";
   if( skill == do_scatter )
      return "do_scatter";
   if( skill == do_score )
      return "do_score";
   if( skill == do_scribe )
      return "do_scribe";
   if( skill == do_search )
      return "do_search";
   if( skill == do_sedit )
      return "do_sedit";
   if( skill == do_seek )
      return "do_seek";
   if( skill == do_sell )
      return "do_sell";
   if( skill == do_set_boot_time )
      return "do_set_boot_time";
   if( skill == do_setdeity )
      return "do_setdeity";
   if( skill == do_set_book )
      return "do_set_book";
   if( skill == do_set_help )
      return "do_set_help";
   if( skill == do_setweather )
      return "do_setweather";
   if( skill == do_sheathe )
      return "do_sheathe";
   if( skill == do_shield )
      return "do_shield";
   if( skill == do_shock )
      return "do_shock";
   if( skill == do_shoot )
      return "do_shoot";
   if( skill == do_shops )
      return "do_shops";
   if( skill == do_shopset )
      return "do_shopset";
   if( skill == do_shopstat )
      return "do_shopstat";
   if( skill == do_shove )
      return "do_shove";
   if( skill == do_showabit )
      return "do_showabit";
   if( skill == do_showqbit )
      return "do_showqbit";
   if( skill == do_setabit )
      return "do_setabit";
   if( skill == do_setqbit )
      return "do_setqbit";
   if( skill == do_showdeity )
      return "do_showdeity";
   if( skill == do_showweather )
      return "do_showweather";
   if( skill == do_shutdow )
      return "do_shutdow";
   if( skill == do_shutdown )
      return "do_shutdown";
   if( skill == do_silence )
      return "do_silence";
   if( skill == do_sing )
      return "do_sing";
   if( skill == do_sit )
      return "do_sit";
   if( skill == do_skills )
      return "do_skills";
   if( skill == do_skin )
      return "do_skin";
   if( skill == do_slap )
      return "do_slap";
   if( skill == do_slash )
      return "do_slash";
   if( skill == do_slay )
      return "do_slay";
   if( skill == do_sleep )
      return "do_sleep";
   if( skill == do_slookup )
      return "do_slookup";
   if( skill == do_smite )
      return "do_smite";
   if( skill == do_smoke )
      return "do_smoke";
   if( skill == do_snoop )
      return "do_snoop";
   if( skill == do_sober )
      return "do_sober";
   if( skill == do_socials )
      return "do_socials";
   if( skill == do_sonics )
      return "do_sonics";
   if( skill == do_soulfire )
      return "do_soulfire";
   if( skill == do_south )
      return "do_south";
   if( skill == do_southeast )
      return "do_southeast";
   if( skill == do_southwest )
      return "do_southwest";
   if( skill == do_squat )
      return "do_squat";
   if( skill == do_species )
      return "do_species";
   if( skill == do_spike )
      return "do_spike";
   if( skill == do_splash )
      return "do_splash";
   if( skill == do_split )
      return "do_split";
   if( skill == do_sset )
      return "do_sset";
   if( skill == do_stab )
      return "do_stab";
   if( skill == do_stand )
      return "do_stand";
   if( skill == do_status )
      return "do_status";
   if( skill == do_steal )
      return "do_steal";
   if( skill == do_steam )
      return "do_steam";
   if( skill == do_storm )
      return "do_storm";
   if( skill == do_story )
      return "do_story";
   if( skill == do_strew )
      return "do_strew";
   if( skill == do_strip )
      return "do_strip";
   if( skill == do_summon )
      return "do_summon";
   if( skill == do_sunstroke )
      return "do_sunstroke";
   if( skill == do_switch )
      return "do_switch";
   if( skill == do_sworddance )
      return "do_sworddance";
   if( skill == do_showlayers )
      return "do_showlayers";
   if( skill == do_suicide )
      return "do_suicide";
   if( skill == do_tail )
      return "do_tail";
   if( skill == do_taint )
      return "do_taint";
   if( skill == do_talent_advance )
      return "do_talent_advance";
   if( skill == do_tap )
      return "do_tap";
   if( skill == do_tamp )
      return "do_tamp";
   if( skill == do_telekinetics )
      return "do_telekinetics";
   if( skill == do_tell )
      return "do_tell";
   if( skill == do_think )
      return "do_think";
   if( skill == do_throw )
      return "do_throw";
   if( skill == do_time )
      return "do_time";
   if( skill == do_timecmd )
      return "do_timecmd";
   if( skill == do_title )
      return "do_title";
   if( skill == do_track )
      return "do_track";
   if( skill == do_tradein )
      return "do_tradein";
   if( skill == do_train )
      return "do_train";
   if( skill == do_transfer )
      return "do_transfer";
   if( skill == do_travel )
      return "do_travel";
   if( skill == do_turn )
      return "do_turn";
   if( skill == do_typo )
      return "do_typo";
   if( skill == do_unfoldarea )
      return "do_unfoldarea";
   if( skill == do_unhell )
      return "do_unhell";
   if( skill == do_unholyblast )
      return "do_unholyblast";
   if( skill == do_unlock )
      return "do_unlock";
   if( skill == do_unsilence )
      return "do_unsilence";
   if( skill == do_up )
      return "do_up";
   if( skill == do_users )
      return "do_users";
   if( skill == do_value )
      return "do_value";
   if( skill == do_ventriloquate )
      return "do_ventriloquate";
   if( skill == do_vines )
      return "do_vines";
   if( skill == do_vnums )
      return "do_vnums";
   if( skill == do_void )
      return "do_void";
   if( skill == do_vsearch )
      return "do_vsearch";
   if( skill == do_wail )
      return "do_wail";
   if( skill == do_wake )
      return "do_wake";
   if( skill == do_weapons )
      return "do_weapons";
   if( skill == do_wear )
      return "do_wear";
   if( skill == do_weather )
      return "do_weather";
   if( skill == do_west )
      return "do_west";
   if( skill == do_where )
      return "do_where";
   if( skill == do_whip )
      return "do_whip";
   if( skill == do_whisper )
      return "do_whisper";
   if( skill == do_who )
      return "do_who";
   if( skill == do_whois )
      return "do_whois";
   if( skill == do_wield )
      return "do_wield";
   if( skill == do_wind )
      return "do_wind";
   if( skill == do_withdraw )
      return "do_withdraw";
   if( skill == do_wizhelp )
      return "do_wizhelp";
   if( skill == do_wizlock )
      return "do_wizlock";
   if( skill == do_zones )
      return "do_zones";

   sprintf( buf, "(%p)", skill );
   return buf;
}

/* Load an individual clan file */
bool load_clan_file( char *fname )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   int mode = 0;
   CLAN_DATA *clan;
   CLAN_MEMBER *member;
   FILE *fp;

   sprintf( buf, "%s%s", CLAN_DIR, fname );
   if( ( fp = fopen( buf, "r" ) ) == NULL )
   {
      perror( buf );
      return FALSE;
   }

   CREATE( clan, CLAN_DATA, 1 );

   mode = 0;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;


      if( mode == 0 )
      {
         switch ( UPPER( word[0] ) )
         {
            case '*':
               fMatch = TRUE;
               fread_to_eol( fp );
               break;

            case '#':
               if( !str_cmp( word, "#MEMBER" ) )
               {
                  mode = 1;
                  CREATE( member, CLAN_MEMBER, 1 );
                  LINK( member, clan->first_member, clan->last_member, next, prev );
               }
               break;

            case 'B':
               KEY( "Bank", clan->bank, fread_number( fp ) );
               break;

            case 'E':
               if( !str_cmp( word, "End" ) )
               {
                  fclose( fp );
                  LINK( clan, first_clan, last_clan, next, prev );
                  return TRUE;
               }
               KEY( "Ext_Sec", clan->ext_sec, fread_number( fp ) );
               break;

            case 'I':
               KEY( "Info", clan->info, fread_string( fp ) );
               KEY( "Int_Sec", clan->int_sec, fread_number( fp ) );
               break;

            case 'N':
               KEY( "Name", clan->name, fread_string( fp ) );
               break;

            case 'T':
               KEY( "Title", clan->title, fread_string( fp ) );
               break;
         }
      }
      else if( mode == 1 )
      {  /* Read in member data */
         switch ( UPPER( word[0] ) )
         {
            case '*':
               fMatch = TRUE;
               fread_to_eol( fp );
               break;

            case 'B':
               KEY( "Bank", member->bank, fread_number( fp ) );
               break;

            case 'E':
               if( !strcmp( word, "End" ) )
               {
                  fMatch = TRUE;
                  mode = 0;
                  member = NULL;
               }
               break;

            case 'F':
               KEY( "Flags", member->flags, fread_bitvector( fp ) );
               break;

            case 'N':
               KEY( "Name", member->name, fread_string( fp ) );
               break;

            case 'T':
               KEY( "Title", member->title, fread_string( fp ) );
               break;
         }
      }

/*	if ( !fMatch )
	{
      sprintf( buf, "load_clan_file: no match: %s", word );
	    bug( buf, 0 );
	} */
   }
   return FALSE;
}

/*
 * Load in all the clan files.
 */
void load_clans(  )
{
   FILE *fpList;
   char *filename;
   char clanlist[256];
   char buf[MAX_STRING_LENGTH];

   sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
   if( ( fpList = fopen( clanlist, "r" ) ) == NULL )
   {
      perror( clanlist );
      exit( 1 );
   }

   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;

      if( !load_clan_file( filename ) )
      {
         sprintf( buf, "Cannot load clan file: %s", filename );
         bug( buf, 0 );
      }
   }
   fclose( fpList );
   return;
}

/* Write a clan file out */
void write_clan_file( CLAN_DATA * clan, char filename[MAX_INPUT_LENGTH] )
{
   FILE *fpout;
   char buf[MAX_STRING_LENGTH];
   CLAN_MEMBER *member;

   if( clan == NULL )
   {
      bug( "Can't write NULL clan data", 0 );
      return;
   }

   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      sprintf( buf, "Cannot open: %s for writing", filename );
      bug( buf, 0 );
      return;
   }
   fprintf( fpout, "Name	%s~\n", clan->name );
   fprintf( fpout, "Title	%s~\n", clan->title );
   fprintf( fpout, "Info	%s~\n", clan->info );
   fprintf( fpout, "Ext_Sec %d\n", clan->ext_sec );
   fprintf( fpout, "Int_Sec %d\n", clan->int_sec );
   fprintf( fpout, "Bank	%d\n", clan->bank );

   for( member = clan->first_member; member; member = member->next )
   {
      fprintf( fpout, "\n#MEMBER\n" );
      fprintf( fpout, "Name	%s~\n", member->name );
      fprintf( fpout, "Title	%s~\n", member->title );
      fprintf( fpout, "Bank	%d\n", member->bank );
      fprintf( fpout, "Flags %s~\n", print_bitvector( &member->flags ) );
      fprintf( fpout, "End\n" );
   }

   fprintf( fpout, "End\n" );
   fclose( fpout );
}

/* Write out all the clan files */
void save_clans( void )
{
   CLAN_DATA *clan;
   FILE *fpout;
   char filename[MAX_INPUT_LENGTH];
   int i = 0;

   sprintf( filename, "%sclan.lst", CLAN_DIR );
   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      bug( "Cannot open clan list for writing", 0 );
      return;
   }

   for( clan = first_clan; clan; clan = clan->next )
   {
      sprintf( filename, "%s%d.clan", CLAN_DIR, ++i );
      write_clan_file( clan, filename );
      fprintf( fpout, "%d.clan\n", i );
   }

   fprintf( fpout, "$\n" );
   fclose( fpout );
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE ** sk1, SKILLTYPE ** sk2 )
{
   SKILLTYPE *skill1 = ( *sk1 );
   SKILLTYPE *skill2 = ( *sk2 );

   if( !skill1 && skill2 )
      return 1;
   if( skill1 && !skill2 )
      return -1;
   if( !skill1 && !skill2 )
      return 0;
   if( skill1->type < skill2->type )
      return -1;
   if( skill1->type > skill2->type )
      return 1;
   return strcmp( skill1->name, skill2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table(  )
{
   log_string( "Sorting skill table..." );
   qsort( &skill_table[1], top_sn - 1, sizeof( SKILLTYPE * ), ( int ( * )( const void *, const void * ) )skill_comp );
}


/*
 * Remap slot numbers to sn values
 */
void remap_slot_numbers(  )
{
   SKILLTYPE *skill;
   SMAUG_AFF *aff;
   char tmp[32];
   int sn;

   log_string( "Remapping slots to sns" );

   for( sn = 0; sn <= top_sn; sn++ )
   {
      if( ( skill = skill_table[sn] ) != NULL )
      {
         for( aff = skill->affects; aff; aff = aff->next )
            if( aff->location == APPLY_WEAPONSPELL
                || aff->location == APPLY_WEARSPELL
                || aff->location == APPLY_REMOVESPELL
                || aff->location == APPLY_STRIPSN || aff->location == APPLY_RECURRINGSPELL )
            {
               sprintf( tmp, "%d", slot_lookup( atoi( aff->modifier ) ) );
               DISPOSE( aff->modifier );
               aff->modifier = str_dup( tmp );
            }
      }
   }
}

/*
 * Write skill data to a file
 */
void fwrite_skill( FILE * fpout, SKILLTYPE * skill )
{
   SMAUG_AFF *aff;
   int modifier;
   int i;
   DEITY_DATA *talent;

   fprintf( fpout, "Name         %s~\n", skill->name );
   fprintf( fpout, "Type         %s\n", skill_tname[skill->type] );
   fprintf( fpout, "Info         %d\n", skill->info );
   fprintf( fpout, "Flags        %d\n", skill->flags );
   if( skill->target )
      fprintf( fpout, "Target       %d\n", skill->target );
   /*
    * store as new minpos (minpos>=100 flags new style in character loading)
    */
   if( skill->minimum_position )
      fprintf( fpout, "Minpos       %d\n", skill->minimum_position + 100 );
   if( skill->saves )
      fprintf( fpout, "Saves        %d\n", skill->saves );
   if( skill->slot )
      fprintf( fpout, "Slot         %d\n", skill->slot );
   if( skill->min_mana )
      fprintf( fpout, "Mana         %d\n", skill->min_mana );
   if( skill->beats )
      fprintf( fpout, "Rounds       %d\n", skill->beats );
   if( skill->range )
      fprintf( fpout, "Range        %d\n", skill->range );
   if( skill->guild != -1 )
      fprintf( fpout, "Guild        %d\n", skill->guild );
   if( skill->skill_fun )
      fprintf( fpout, "Code         %s\n", skill_name( skill->skill_fun ) );
   else if( skill->spell_fun )
      fprintf( fpout, "Code         %s\n", spell_name( skill->spell_fun ) );
   fprintf( fpout, "Dammsg       %s~\n", skill->noun_damage );
   if( skill->msg_off && skill->msg_off[0] != '\0' )
      fprintf( fpout, "Wearoff      %s~\n", skill->msg_off );

   if( skill->hit_char && skill->hit_char[0] != '\0' )
      fprintf( fpout, "Hitchar      %s~\n", skill->hit_char );
   if( skill->hit_vict && skill->hit_vict[0] != '\0' )
      fprintf( fpout, "Hitvict      %s~\n", skill->hit_vict );
   if( skill->hit_room && skill->hit_room[0] != '\0' )
      fprintf( fpout, "Hitroom      %s~\n", skill->hit_room );
   if( skill->hit_dest && skill->hit_dest[0] != '\0' )
      fprintf( fpout, "Hitdest      %s~\n", skill->hit_dest );

   if( skill->miss_char && skill->miss_char[0] != '\0' )
      fprintf( fpout, "Misschar     %s~\n", skill->miss_char );
   if( skill->miss_vict && skill->miss_vict[0] != '\0' )
      fprintf( fpout, "Missvict     %s~\n", skill->miss_vict );
   if( skill->miss_room && skill->miss_room[0] != '\0' )
      fprintf( fpout, "Missroom     %s~\n", skill->miss_room );

   if( skill->die_char && skill->die_char[0] != '\0' )
      fprintf( fpout, "Diechar      %s~\n", skill->die_char );
   if( skill->die_vict && skill->die_vict[0] != '\0' )
      fprintf( fpout, "Dievict      %s~\n", skill->die_vict );
   if( skill->die_room && skill->die_room[0] != '\0' )
      fprintf( fpout, "Dieroom      %s~\n", skill->die_room );

   if( skill->imm_char && skill->imm_char[0] != '\0' )
      fprintf( fpout, "Immchar      %s~\n", skill->imm_char );
   if( skill->imm_vict && skill->imm_vict[0] != '\0' )
      fprintf( fpout, "Immvict      %s~\n", skill->imm_vict );
   if( skill->imm_room && skill->imm_room[0] != '\0' )
      fprintf( fpout, "Immroom      %s~\n", skill->imm_room );

   if( skill->dice && skill->dice[0] != '\0' )
      fprintf( fpout, "Dice         %s~\n", skill->dice );
   if( skill->value )
      fprintf( fpout, "Value        %d\n", skill->value );
   if( skill->difficulty )
      fprintf( fpout, "Difficulty   %d\n", skill->difficulty );
   if( skill->participants )
      fprintf( fpout, "Participants %d\n", skill->participants );
   if( skill->components && skill->components[0] != '\0' )
      fprintf( fpout, "Components   %s~\n", skill->components );
   if( skill->teachers && skill->teachers[0] != '\0' )
      fprintf( fpout, "Teachers     %s~\n", skill->teachers );
   for( aff = skill->affects; aff; aff = aff->next )
   {
      fprintf( fpout, "Affect       '%s' %d ", aff->duration, aff->location );
      modifier = atoi( aff->modifier );
      if( ( aff->location == APPLY_WEAPONSPELL
            || aff->location == APPLY_WEARSPELL
            || aff->location == APPLY_REMOVESPELL
            || aff->location == APPLY_STRIPSN || aff->location == APPLY_RECURRINGSPELL ) && IS_VALID_SN( modifier ) )
         fprintf( fpout, "'%d' ", skill_table[modifier]->slot );
      else
         fprintf( fpout, "'%s' ", aff->modifier );
      fprintf( fpout, "%d\n", aff->bitvector );
   }

   fprintf( fpout, "Stat1	%d\n", skill->stat1 );
   fprintf( fpout, "Stat2	%d\n", skill->stat2 );

   for( talent = first_deity; talent; talent = talent->next )
      fprintf( fpout, "Talent	%d %d\n", talent->index, skill->skill_level[talent->index] );

   for( i = 0; i < MAX_RACE; i++ )
      fprintf( fpout, "Race	%d %d\n", i, skill->race_level[i] );

/*    if ( skill->type != SKILL_HERB )
    {
	int y;
	int min = 1000;
	for ( y = 0; y < MAX_CLASS; y++ )
	    if ( skill->skill_level[y] < min )
		min = skill->skill_level[y];

	fprintf( fpout, "Minlevel     %d\n",	min		);

	min = 1000;
	for ( y = 0; y < MAX_RACE; y++ )
	    if ( skill->race_level[y] < min )
		min = skill->race_level[y];

    }
*/ fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table(  )
{
   int x;
   FILE *fpout;

   if( ( fpout = fopen( SKILL_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open skills.dat for writing", 0 );
      perror( SKILL_FILE );
      return;
   }

   for( x = 0; x < top_sn; x++ )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
         break;

      if( !strcmp( skill_table[x]->name, "deleted_skill" ) )
         continue;

      if( skill_table[x]->stat1 == 0 && skill_table[x]->stat2 > 0 )
      {
         skill_table[x]->stat1 = skill_table[x]->stat2;
         skill_table[x]->stat2 = 0;
         bug( "Sliding stat2 of skill '%s' to empty stat1 slot", skill_table[x]->name );
      }

      fprintf( fpout, "#SKILL\n" );
      fwrite_skill( fpout, skill_table[x] );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table(  )
{
   int x;
   FILE *fpout;

   if( ( fpout = fopen( HERB_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open herbs.dat for writting", 0 );
      perror( HERB_FILE );
      return;
   }

   for( x = 0; x < top_herb; x++ )
   {
      if( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
         break;
      fprintf( fpout, "#HERB\n" );
      fwrite_skill( fpout, herb_table[x] );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials(  )
{
   FILE *fpout;
   SOCIALTYPE *social;
   int x;

   if( ( fpout = fopen( SOCIAL_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open socials.dat for writting", 0 );
      perror( SOCIAL_FILE );
      return;
   }

   for( x = 0; x < 27; x++ )
   {
      for( social = social_index[x]; social; social = social->next )
      {
         if( !social->name || social->name[0] == '\0' )
         {
            bug( "Save_socials: blank social in hash bucket %d", x );
            continue;
         }
         fprintf( fpout, "#SOCIAL\n" );
         fprintf( fpout, "Name        %s~\n", social->name );
         fprintf( fpout, "Flags	     %d\n", social->flags );
         if( social->char_no_arg )
            fprintf( fpout, "CharNoArg   %s~\n", social->char_no_arg );
         else
            bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
         if( social->others_no_arg )
            fprintf( fpout, "OthersNoArg %s~\n", social->others_no_arg );
         if( social->char_found )
            fprintf( fpout, "CharFound   %s~\n", social->char_found );
         if( social->others_found )
            fprintf( fpout, "OthersFound %s~\n", social->others_found );
         if( social->vict_found )
            fprintf( fpout, "VictFound   %s~\n", social->vict_found );
         if( social->char_auto )
            fprintf( fpout, "CharAuto    %s~\n", social->char_auto );
         if( social->others_auto )
            fprintf( fpout, "OthersAuto  %s~\n", social->others_auto );
         if( social->adj )
            fprintf( fpout, "Adj	     %s~\n", social->adj );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

int get_skill( char *skilltype )
{
   if( !str_cmp( skilltype, "Race" ) )
      return SKILL_RACIAL;
   if( !str_cmp( skilltype, "Spell" ) )
      return SKILL_SPELL;
   if( !str_cmp( skilltype, "Skill" ) )
      return SKILL_SKILL;
   if( !str_cmp( skilltype, "Weapon" ) )
      return SKILL_WEAPON;
   if( !str_cmp( skilltype, "Tongue" ) )
      return SKILL_TONGUE;
   if( !str_cmp( skilltype, "Herb" ) )
      return SKILL_HERB;
   return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 * Added flags Aug 25, 1997 --Shaddai
 */
void save_commands(  )
{
   FILE *fpout;
   CMDTYPE *command;
   int x;

   if( ( fpout = fopen( COMMAND_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open commands.dat for writing", 0 );
      perror( COMMAND_FILE );
      return;
   }

   for( x = 0; x < 126; x++ )
   {
      for( command = command_hash[x]; command; command = command->next )
      {
         if( !command->name || command->name[0] == '\0' )
         {
            bug( "Save_commands: blank command in hash bucket %d", x );
            continue;
         }
         fprintf( fpout, "#COMMAND\n" );
         fprintf( fpout, "Name        %s~\n", command->name );
         fprintf( fpout, "Code        %s\n", skill_name( command->do_fun ) );
/* Oops I think this may be a bad thing so I changed it -- Shaddai */
         if( command->position < 100 )
            fprintf( fpout, "Position    %d\n", command->position + 100 );
         else
            fprintf( fpout, "Position    %d\n", command->position );
         fprintf( fpout, "Log         %d\n", command->log );
         if( command->permit )
            fprintf( fpout, "Permit	%d\n", command->permit );
         if( command->flags )
            fprintf( fpout, "Flags       %d\n", command->flags );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

SKILLTYPE *fread_skill( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   bool got_info = FALSE;
   SKILLTYPE *skill;
   int x;

   CREATE( skill, SKILLTYPE, 1 );
   skill->slot = 0;
   skill->min_mana = 0;

   for( x = 0; x < MAX_RACE; x++ )
   {
      skill->race_level[x] = 3;
   }

   skill->guild = -1;
   skill->target = 0;
   skill->skill_fun = NULL;
   skill->spell_fun = NULL;
   skill->stat1 = 0;
   skill->stat2 = 0;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            if( !str_cmp( word, "Affect" ) )
            {
               SMAUG_AFF *aff;

               CREATE( aff, SMAUG_AFF, 1 );
               aff->duration = str_dup( fread_word( fp ) );
               aff->location = fread_number( fp );
               aff->modifier = str_dup( fread_word( fp ) );
               aff->bitvector = fread_number( fp );
               if( !got_info )
               {
                  for( x = 0; x < 32; x++ )
                  {
                     if( IS_SET( aff->bitvector, 1 << x ) )
                     {
                        aff->bitvector = x;
                        break;
                     }
                  }
                  if( x == 32 )
                     aff->bitvector = -1;
               }
               aff->next = skill->affects;
               skill->affects = aff;
               fMatch = TRUE;
               break;
            }
            break;

         case 'C':
            if( !str_cmp( word, "Code" ) )
            {
               SPELL_FUN *spellfun;
               DO_FUN *dofun;
               char *w = fread_word( fp );

               fMatch = TRUE;
               if( ( spellfun = spell_function( w ) ) != spell_notfound )
               {
                  skill->spell_fun = spellfun;
                  skill->skill_fun = NULL;
               }
               else if( ( dofun = skill_function( w ) ) != skill_notfound )
               {
                  skill->skill_fun = dofun;
                  skill->spell_fun = NULL;
               }
               else
               {
                  bug( "fread_skill: unknown skill/spell %s", w );
                  skill->spell_fun = spell_null;
               }
               break;
            }
            KEY( "Components", skill->components, fread_string_nohash( fp ) );
            break;

         case 'D':
            KEY( "Dammsg", skill->noun_damage, fread_string_nohash( fp ) );
            KEY( "Dice", skill->dice, fread_string_nohash( fp ) );
            KEY( "Diechar", skill->die_char, fread_string_nohash( fp ) );
            KEY( "Dieroom", skill->die_room, fread_string_nohash( fp ) );
            KEY( "Dievict", skill->die_vict, fread_string_nohash( fp ) );
            KEY( "Difficulty", skill->difficulty, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( skill->saves != 0 && SPELL_SAVE( skill ) == SE_NONE )
               {
                  bug( "fread_skill(%s):  Has saving throw (%d) with no saving effect.", skill->name, skill->saves );
                  SET_SSAV( skill, SE_NEGATE );
               }
               return skill;
            }
            break;

         case 'F':
            if( !str_cmp( word, "Flags" ) )
            {
               skill->flags = fread_number( fp );
               /*
                * convert to new style       -Thoric
                */
               if( !got_info )
               {
                  skill->info = skill->flags & ( BV11 - 1 );
                  if( IS_SET( skill->flags, OLD_SF_SAVE_NEGATES ) )
                  {
                     if( IS_SET( skill->flags, OLD_SF_SAVE_HALF_DAMAGE ) )
                     {
                        SET_SSAV( skill, SE_QUARTERDAM );
                        REMOVE_BIT( skill->flags, OLD_SF_SAVE_HALF_DAMAGE );
                     }
                     else
                        SET_SSAV( skill, SE_NEGATE );
                     REMOVE_BIT( skill->flags, OLD_SF_SAVE_NEGATES );
                  }
                  else if( IS_SET( skill->flags, OLD_SF_SAVE_HALF_DAMAGE ) )
                  {
                     SET_SSAV( skill, SE_HALFDAM );
                     REMOVE_BIT( skill->flags, OLD_SF_SAVE_HALF_DAMAGE );
                  }
                  skill->flags >>= 11;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'G':
            KEY( "Guild", skill->guild, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Hitchar", skill->hit_char, fread_string_nohash( fp ) );
            KEY( "Hitdest", skill->hit_dest, fread_string_nohash( fp ) );
            KEY( "Hitroom", skill->hit_room, fread_string_nohash( fp ) );
            KEY( "Hitvict", skill->hit_vict, fread_string_nohash( fp ) );
            break;

         case 'I':
            KEY( "Immchar", skill->imm_char, fread_string_nohash( fp ) );
            KEY( "Immroom", skill->imm_room, fread_string_nohash( fp ) );
            KEY( "Immvict", skill->imm_vict, fread_string_nohash( fp ) );
            if( !str_cmp( word, "Info" ) )
            {
               skill->info = fread_number( fp );
               got_info = TRUE;
               fMatch = TRUE;
               break;
            }
            break;

         case 'M':
            KEY( "Mana", skill->min_mana, fread_number( fp ) );
            if( !str_cmp( word, "Minlevel" ) )
            {
               fread_to_eol( fp );
               fMatch = TRUE;
               break;
            }
            /*
             * KEY( "Minpos",   skill->minimum_position, fread_number( fp ) ); 
             */
            /*
             *
             */
            if( !str_cmp( word, "Minpos" ) )
            {
               fMatch = TRUE;
               skill->minimum_position = fread_number( fp );
               if( skill->minimum_position < 100 )
               {
                  switch ( skill->minimum_position )
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        skill->minimum_position = 6;
                        break;
                     case 6:
                        skill->minimum_position = 8;
                        break;
                     case 7:
                        skill->minimum_position = 9;
                        break;
                     case 8:
                        skill->minimum_position = 12;
                        break;
                     case 9:
                        skill->minimum_position = 13;
                        break;
                     case 10:
                        skill->minimum_position = 14;
                        break;
                     case 11:
                        skill->minimum_position = 15;
                        break;
                  }
               }
               else
                  skill->minimum_position -= 100;
               break;
            }

            KEY( "Misschar", skill->miss_char, fread_string_nohash( fp ) );
            KEY( "Missroom", skill->miss_room, fread_string_nohash( fp ) );
            KEY( "Missvict", skill->miss_vict, fread_string_nohash( fp ) );
            break;

         case 'N':
            KEY( "Name", skill->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            KEY( "Participants", skill->participants, fread_number( fp ) );
            break;

         case 'R':
            KEY( "Range", skill->range, fread_number( fp ) );
            KEY( "Rounds", skill->beats, fread_number( fp ) );
            if( !str_cmp( word, "Race" ) )
            {
               int race = fread_number( fp );

               skill->race_level[race] = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'S':
            KEY( "Stat1", skill->stat1, fread_number( fp ) );
            KEY( "Stat2", skill->stat2, fread_number( fp ) );
            KEY( "Slot", skill->slot, fread_number( fp ) );
            KEY( "Saves", skill->saves, fread_number( fp ) );
            break;

         case 'T':
            if( !str_cmp( word, "Talent" ) )
            {
               int index = fread_number( fp );

               skill->skill_level[index] = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "Target", skill->target, fread_number( fp ) );
            KEY( "Teachers", skill->teachers, fread_string_nohash( fp ) );
            KEY( "Type", skill->type, get_skill( fread_word( fp ) ) );
            break;

         case 'V':
            KEY( "Value", skill->value, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Wearoff", skill->msg_off, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_skill: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

void load_skill_table(  )
{
   FILE *fp;

   if( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_skill_table: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SKILL" ) )
         {
            if( top_sn >= MAX_SKILL )
            {
               bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
               fclose( fp );
               return;
            }
            skill_table[top_sn++] = fread_skill( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_skill_table: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open skills.dat", 0 );
      exit( 0 );
   }
}

void load_herb_table(  )
{
   FILE *fp;

   if( ( fp = fopen( HERB_FILE, "r" ) ) != NULL )
   {
      top_herb = 0;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_herb_table: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "HERB" ) )
         {
            if( top_herb >= MAX_HERB )
            {
               bug( "load_herb_table: more herbs than MAX_HERB %d", MAX_HERB );
               fclose( fp );
               return;
            }
            herb_table[top_herb++] = fread_skill( fp );
            if( herb_table[top_herb - 1]->slot == 0 )
               herb_table[top_herb - 1]->slot = top_herb - 1;
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_herb_table: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open herbs.dat", 0 );
      exit( 0 );
   }
}

void fread_social( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   SOCIALTYPE *social;

   CREATE( social, SOCIALTYPE, 1 );

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case 'A':
            KEY( "Adj", social->adj, fread_string_nohash( fp ) );
            break;

         case 'C':
            KEY( "CharNoArg", social->char_no_arg, fread_string_nohash( fp ) );
            KEY( "CharFound", social->char_found, fread_string_nohash( fp ) );
            KEY( "CharAuto", social->char_auto, fread_string_nohash( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !social->name )
               {
                  bug( "Fread_social: Name not found", 0 );
                  free_social( social );
                  return;
               }
               if( !social->char_no_arg )
               {
                  bug( "Fread_social: CharNoArg not found", 0 );
                  free_social( social );
                  return;
               }
               add_social( social );
               return;
            }
            break;

         case 'F':
            KEY( "Flags", social->flags, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", social->name, fread_string_nohash( fp ) );
            break;

         case 'O':
            KEY( "OthersNoArg", social->others_no_arg, fread_string_nohash( fp ) );
            KEY( "OthersFound", social->others_found, fread_string_nohash( fp ) );
            KEY( "OthersAuto", social->others_auto, fread_string_nohash( fp ) );
            break;

         case 'V':
            KEY( "VictFound", social->vict_found, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_social: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

void load_socials(  )
{
   FILE *fp;

   if( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_socials: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "SOCIAL" ) )
         {
            fread_social( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_socials: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open socials.dat", 0 );
      exit( 0 );
   }
}

/*
 *  Added the flags Aug 25, 1997 --Shaddai
 */

void fread_command( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   CMDTYPE *command;

   CREATE( command, CMDTYPE, 1 );
   command->lag_count = 0; /* can't have caused lag yet... FB */
   command->flags = 0;  /* Default to no flags set */

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'C':
            KEY( "Code", command->do_fun, skill_function( fread_word( fp ) ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !command->name )
               {
                  bug( "Fread_command: Name not found", 0 );
                  free_command( command );
                  return;
               }
               if( !command->do_fun )
               {
                  bug( "Fread_command: Function not found", 0 );
                  free_command( command );
                  return;
               }
               add_command( command );
               return;
            }
            break;

         case 'F':
            KEY( "Flags", command->flags, fread_number( fp ) );
            break;

         case 'L':
            KEY( "Log", command->log, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", command->name, fread_string_nohash( fp ) );
            break;

         case 'P':
            KEY( "Permit", command->permit, fread_number( fp ) );
            /*
             * KEY( "Position", command->position,   fread_number(fp) ); 
             */
            if( !str_cmp( word, "Position" ) )
            {
               fMatch = TRUE;
               command->position = fread_number( fp );
               if( command->position < 100 )
               {
                  switch ( command->position )
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        command->position = 6;
                        break;
                     case 6:
                        command->position = 8;
                        break;
                     case 7:
                        command->position = 9;
                        break;
                     case 8:
                        command->position = 12;
                        break;
                     case 9:
                        command->position = 13;
                        break;
                     case 10:
                        command->position = 14;
                        break;
                     case 11:
                        command->position = 15;
                        break;
                  }
               }
               else
                  command->position -= 100;
               break;
            }
            break;
      }

      if( !fMatch )
      {
         sprintf( buf, "Fread_command: no match: %s", word );
         bug( buf, 0 );
      }
   }
}

void load_commands(  )
{
   FILE *fp;

   if( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "Load_commands: # not found.", 0 );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "COMMAND" ) )
         {
            fread_command( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_commands: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open commands.dat", 0 );
      exit( 0 );
   }

}
