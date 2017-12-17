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
 *		     Character saving and loading module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
#include <dirent.h>
#endif
#include "mud.h"


/*
 * Increment with every major format change.
 */
#define SAVEVERSION	5

/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;

/* handler.c */
MATERIAL_DATA *material_lookup( int number );

extern NATION_DATA *find_nation( char *name );

/*
 * Array of containers read for proper re-nesting of objects.
 */
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void fwrite_char args( ( CHAR_DATA * ch, FILE * fp ) );
void fread_char args( ( CHAR_DATA * ch, FILE * fp, bool preload ) );
void write_corpses args( ( CHAR_DATA * ch, char *name, OBJ_DATA * objrem ) );
void fwrite_alias args( ( CHAR_DATA * ch, ALIAS_DATA * first_alias, FILE * fp ) );
void fread_alias args( ( CHAR_DATA * ch, FILE * fp ) );
void save_world( CHAR_DATA * ch );
int get_obj_room_vnum_recursive( OBJ_DATA * obj );


#ifdef WIN32   /* NJG */
UINT timer_code = 0; /* needed to kill the timer */
/* Note: need to include: WINMM.LIB to link to timer functions */
void caught_alarm(  );
void CALLBACK alarm_handler( UINT IDEvent,   /* identifies timer event */
                             UINT uReserved, /* not used */
                             DWORD dwUser,   /* application-defined instance data */
                             DWORD dwReserved1, /* not used */
                             DWORD dwReserved2 )   /* not used */
{
   caught_alarm(  );
}

void kill_timer(  )
{
   if( timer_code )
      timeKillEvent( timer_code );
   timer_code = 0;
}

#endif



/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   int x, y;

   for( x = 0; x < MAX_WEAR; x++ )
      for( y = 0; y < MAX_LAYERS; y++ )
         save_equipment[x][y] = NULL;
   for( obj = ch->first_carrying; obj; obj = obj->next_content )
      if( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
      {
         for( x = 0; x < MAX_LAYERS; x++ )
            if( !save_equipment[obj->wear_loc][x] )
            {
               save_equipment[obj->wear_loc][x] = obj;
               break;
            }
         if( x == MAX_LAYERS )
         {
            sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s",
                     ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
            bug( buf, 0 );
         }
         unequip_char( ch, obj );
      }
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA * ch )
{
   int x, y;

   for( x = 0; x < MAX_WEAR; x++ )
      for( y = 0; y < MAX_LAYERS; y++ )
         if( save_equipment[x][y] != NULL )
         {
            if( quitting_char != ch )
               equip_char( ch, save_equipment[x][y] );
            save_equipment[x][y] = NULL;
         }
         else
            break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA * ch )
{
   char strsave[MAX_INPUT_LENGTH];
   char strback[MAX_INPUT_LENGTH];
   FILE *fp;

   if( !ch )
   {
      bug( "Save_char_obj: null ch!", 0 );
      return;
   }

   if( IS_NPC( ch ) )
      return;

   saving_char = ch;

   if( ch->desc && ch->desc->original )
      ch = ch->desc->original;

   de_equip_char( ch );

   ch->save_time = current_time;
   sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower( ch->pcdata->filename[0] ), capitalize( ch->pcdata->filename ) );

   /*
    * Auto-backup pfile (can cause lag with high disk access situtations).
    */
   /*
    * Backup of each pfile on save as above can cause lag in high disk
    * access situations on big muds like Realms.  Quitbackup saves most
    * of that and keeps an adequate backup -- Blodkai, 10/97 
    */

   if( IS_SET( sysdata.save_flags, SV_BACKUP ) || ( IS_SET( sysdata.save_flags, SV_QUITBACKUP ) && quitting_char == ch ) )
   {
      sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower( ch->pcdata->filename[0] ), capitalize( ch->pcdata->filename ) );
      rename( strsave, strback );
   }

   if( ( fp = fopen( strsave, "w" ) ) == NULL )
   {
      bug( "Save_char_obj: fopen", 0 );
      perror( strsave );
   }
   else
   {
      fwrite_char( ch, fp );
      if( ch->first_carrying )
         fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY );
      if( !IS_NPC( ch ) && ch->pcdata->first_alias )
         fwrite_alias( ch, ch->pcdata->first_alias, fp );

      if( sysdata.save_pets && ch->pcdata->pet )
         fwrite_mobile( fp, ch->pcdata->pet );
      fprintf( fp, "#END\n" );
      fclose( fp );
   }

   re_equip_char( ch );

   quitting_char = NULL;
   saving_char = NULL;
   return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA * ch, FILE * fp )
{
   PART_DATA *part;
   EXPLORED_AREA *xarea;
   AFFECT_DATA *paf;
   int i, sn;
   sh_int pos;
   SKILLTYPE *skill = NULL;

   fprintf( fp, "#PLAYER\n" );

   fprintf( fp, "Version      %d\n", SAVEVERSION );
   fprintf( fp, "Name         %s~\n", ch->name );
   if( ch->description )
      fprintf( fp, "Description  %s~\n", ch->description );
   fprintf( fp, "Sex          %d\n", ch->sex );
   fprintf( fp, "Race         %d\n", ch->race );
   fprintf( fp, "Permissions	%d\n", ch->pcdata->permissions );
   fprintf( fp, "Points		%d\n", ch->pcdata->points );

   for( i = 0; i < MAX_DEITY; i++ )
   {
      if( ch->talent[i] > -1 )
         fprintf( fp, "Talent		%d %d %d\n", i, ch->talent[i], ch->curr_talent[i] );
   }

   fprintf( fp, "Played       %d\n", ch->played + ( int )( current_time - ch->logon ) );
   fprintf( fp, "Room         %d\n",
            ( ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
              && ch->was_in_room ) ? ch->was_in_room->vnum : ch->in_room->vnum );

   fprintf( fp, "Memorized		%d %d %d %d %d %d %d %d\n",
            ch->pcdata->memorize[0],
            ch->pcdata->memorize[1],
            ch->pcdata->memorize[2],
            ch->pcdata->memorize[3],
            ch->pcdata->memorize[4], ch->pcdata->memorize[5], ch->pcdata->memorize[6], ch->pcdata->memorize[7] );
   fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
   fprintf( fp, "Gold         %ld\n", ch->gold );
   fprintf( fp, "Bank         %ld\n", ch->pcdata->balance );
   if( xIS_SET( ch->act, PLR_BOUNTY ) )
      fprintf( fp, "Bounty	   %d\n", ch->pcdata->bounty );
   fprintf( fp, "Exp          %d\n", ch->exp );
   fprintf( fp, "Height          %d\n", ch->height );
   fprintf( fp, "Weight          %d\n", ch->weight );
   if( !xIS_EMPTY( ch->act ) )
      fprintf( fp, "Act          %s\n", print_bitvector( &ch->act ) );
   if( !xIS_EMPTY( ch->affected_by ) )
      fprintf( fp, "AffectedBy   %s\n", print_bitvector( &ch->affected_by ) );
   if( !xIS_EMPTY( ch->no_affected_by ) )
      fprintf( fp, "NoAffectedBy %s\n", print_bitvector( &ch->no_affected_by ) );
   if( !xIS_EMPTY( ch->pcdata->perm_aff ) )
      fprintf( fp, "PermAffects	%s\n", print_bitvector( &ch->pcdata->perm_aff ) );

   /*
    * Strip off fighting positions & store as
    * new style (pos>=100 flags new style in character loading)
    */
   pos = ch->position;
   pos += 100;
   fprintf( fp, "Position     %d\n", pos );

   fprintf( fp, "SavingThrows %d %d %d %d %d\n",
            ch->saving_poison_death, ch->saving_wand, ch->saving_para_petri, ch->saving_breath, ch->saving_spell_staff );
   fprintf( fp, "Hitroll      %d\n", ch->hitroll );
   fprintf( fp, "Damroll      %d\n", ch->damroll );
   fprintf( fp, "Armor        %d\n", ch->armor );
   if( ch->wimpy )
      fprintf( fp, "Wimpy        %d\n", ch->wimpy );
   if( ch->resistant )
      fprintf( fp, "Resistant    %d\n", ch->resistant );
   if( ch->no_resistant )
      fprintf( fp, "NoResistant  %d\n", ch->no_resistant );
   if( ch->immune )
      fprintf( fp, "Immune       %d\n", ch->immune );
   if( ch->no_immune )
      fprintf( fp, "NoImmune     %d\n", ch->no_immune );
   if( ch->susceptible )
      fprintf( fp, "Susceptible  %d\n", ch->susceptible );
   if( ch->no_susceptible )
      fprintf( fp, "NoSusceptible  %d\n", ch->no_susceptible );
   if( ch->mental_state != -10 )
      fprintf( fp, "Mentalstate  %d\n", ch->mental_state );

   fprintf( fp, "Password     %s~\n", ch->pcdata->pwd );
   if( ch->pcdata->rank && ch->pcdata->rank[0] != '\0' )
      fprintf( fp, "Rank         %s~\n", ch->pcdata->rank );
   fprintf( fp, "Title        %s~\n", ch->pcdata->title );
   if( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
      fprintf( fp, "Homepage     %s~\n", ch->pcdata->homepage );
   if( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
      fprintf( fp, "Bio          %s~\n", ch->pcdata->bio );
   if( ch->pcdata && ch->pcdata->restore_time )
      fprintf( fp, "Restore_time %ld\n", ch->pcdata->restore_time );
   fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );
   if( ch->pcdata->prompt && *ch->pcdata->prompt )
      fprintf( fp, "Prompt       %s~\n", ch->pcdata->prompt );
   if( ch->pcdata->fprompt && *ch->pcdata->fprompt )
      fprintf( fp, "FPrompt	     %s~\n", ch->pcdata->fprompt );
   if( ch->pcdata->pagerlen != 24 )
      fprintf( fp, "Pagerlen     %d\n", ch->pcdata->pagerlen );
   for( pos = 0; pos < MAX_ALIAS; pos++ ) /* alias - shogar */
   {
      if( !ch->pcdata->alias[pos] || !ch->pcdata->alias_sub[pos] )
         break;
      fprintf( fp, "Alias %s %s~\n", ch->pcdata->alias[pos], ch->pcdata->alias_sub[pos] );
   }

   if( IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
   {
      if( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' )
         fprintf( fp, "Bamfin       %s~\n", ch->pcdata->bamfin );
      if( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0' )
         fprintf( fp, "Bamfout      %s~\n", ch->pcdata->bamfout );
      if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
         fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi );
      if( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
         fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi );
      if( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
         fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi );
   }
   if( ch->pcdata->deity_name && ch->pcdata->deity_name[0] != '\0' )
      fprintf( fp, "Deity	     %s~\n", ch->pcdata->deity_name );
   fprintf( fp, "Flags        %d\n", ch->pcdata->flags );
   if( ch->pcdata->release_date > current_time )
      fprintf( fp, "Helled       %d %s~\n", ( int )ch->pcdata->release_date, ch->pcdata->helled_by );
   fprintf( fp, "PKills       %d\n", ch->pcdata->pkills );
   fprintf( fp, "PDeaths      %d\n", ch->pcdata->pdeaths );
   fprintf( fp, "RKills	   %d\n", ch->pcdata->rkills );
   fprintf( fp, "RDeaths	   %d\n", ch->pcdata->rdeaths );
   if( get_timer( ch, TIMER_PKILLED ) && ( get_timer( ch, TIMER_PKILLED ) > 0 ) )
      fprintf( fp, "PTimer       %d\n", get_timer( ch, TIMER_PKILLED ) );
   fprintf( fp, "MKills       %d\n", ch->pcdata->mkills );
   fprintf( fp, "MDeaths      %d\n", ch->pcdata->mdeaths );
   fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
            ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );

   fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
            ch->mod_str, ch->mod_int, ch->mod_wis, ch->mod_dex, ch->mod_con, ch->mod_cha, ch->mod_lck );

   fprintf( fp, "Weapons	%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->pcdata->weapon[0],
            ch->pcdata->weapon[1],
            ch->pcdata->weapon[2],
            ch->pcdata->weapon[3],
            ch->pcdata->weapon[4],
            ch->pcdata->weapon[5],
            ch->pcdata->weapon[6],
            ch->pcdata->weapon[7],
            ch->pcdata->weapon[8],
            ch->pcdata->weapon[9],
            ch->pcdata->weapon[10],
            ch->pcdata->weapon[11],
            ch->pcdata->weapon[12],
            ch->pcdata->weapon[13],
            ch->pcdata->weapon[14],
            ch->pcdata->weapon[15],
            ch->pcdata->weapon[16], ch->pcdata->weapon[17], ch->pcdata->weapon[18], ch->pcdata->weapon[19] );

   fprintf( fp, "WeaponExp	%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->pcdata->weapon_exp[0],
            ch->pcdata->weapon_exp[1],
            ch->pcdata->weapon_exp[2],
            ch->pcdata->weapon_exp[3],
            ch->pcdata->weapon_exp[4],
            ch->pcdata->weapon_exp[5],
            ch->pcdata->weapon_exp[6],
            ch->pcdata->weapon_exp[7],
            ch->pcdata->weapon_exp[8],
            ch->pcdata->weapon_exp[9],
            ch->pcdata->weapon_exp[10],
            ch->pcdata->weapon_exp[11],
            ch->pcdata->weapon_exp[12],
            ch->pcdata->weapon_exp[13],
            ch->pcdata->weapon_exp[14],
            ch->pcdata->weapon_exp[15],
            ch->pcdata->weapon_exp[16], ch->pcdata->weapon_exp[17], ch->pcdata->weapon_exp[18], ch->pcdata->weapon_exp[19] );

   fprintf( fp, "TalentExp	%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->pcdata->talent_exp[0],
            ch->pcdata->talent_exp[1],
            ch->pcdata->talent_exp[2],
            ch->pcdata->talent_exp[3],
            ch->pcdata->talent_exp[4],
            ch->pcdata->talent_exp[5],
            ch->pcdata->talent_exp[6],
            ch->pcdata->talent_exp[7],
            ch->pcdata->talent_exp[8],
            ch->pcdata->talent_exp[9],
            ch->pcdata->talent_exp[10],
            ch->pcdata->talent_exp[11],
            ch->pcdata->talent_exp[12],
            ch->pcdata->talent_exp[13],
            ch->pcdata->talent_exp[14],
            ch->pcdata->talent_exp[15], ch->pcdata->talent_exp[16], ch->pcdata->talent_exp[17], ch->pcdata->talent_exp[18] );

   fprintf( fp, "Noncombat	%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->pcdata->noncombat[0],
            ch->pcdata->noncombat[1],
            ch->pcdata->noncombat[2],
            ch->pcdata->noncombat[3],
            ch->pcdata->noncombat[4],
            ch->pcdata->noncombat[5],
            ch->pcdata->noncombat[6],
            ch->pcdata->noncombat[7],
            ch->pcdata->noncombat[8],
            ch->pcdata->noncombat[9],
            ch->pcdata->noncombat[10],
            ch->pcdata->noncombat[11],
            ch->pcdata->noncombat[12],
            ch->pcdata->noncombat[13],
            ch->pcdata->noncombat[14],
            ch->pcdata->noncombat[15],
            ch->pcdata->noncombat[16], ch->pcdata->noncombat[17], ch->pcdata->noncombat[18], ch->pcdata->noncombat[19] );

   fprintf( fp, "NoncombatExp	%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->pcdata->noncombat_exp[0],
            ch->pcdata->noncombat_exp[1],
            ch->pcdata->noncombat_exp[2],
            ch->pcdata->noncombat_exp[3],
            ch->pcdata->noncombat_exp[4],
            ch->pcdata->noncombat_exp[5],
            ch->pcdata->noncombat_exp[6],
            ch->pcdata->noncombat_exp[7],
            ch->pcdata->noncombat_exp[8],
            ch->pcdata->noncombat_exp[9],
            ch->pcdata->noncombat_exp[10],
            ch->pcdata->noncombat_exp[11],
            ch->pcdata->noncombat_exp[12],
            ch->pcdata->noncombat_exp[13],
            ch->pcdata->noncombat_exp[14],
            ch->pcdata->noncombat_exp[15],
            ch->pcdata->noncombat_exp[16],
            ch->pcdata->noncombat_exp[17], ch->pcdata->noncombat_exp[18], ch->pcdata->noncombat_exp[19] );

   fprintf( fp, "Condition    %d %d %d %d\n",
            ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2], ch->pcdata->condition[3] );
   if( ch->desc && ch->desc->host )
      fprintf( fp, "Site         %s\n", ch->desc->host );
   else
      fprintf( fp, "Site         (Link-Dead)\n" );

   xarea = ch->pcdata->first_explored;
   while( xarea )
   {
      fprintf( fp, "Explored	%d %s\n", xarea->index, print_bitvector( &xarea->rooms ) );
      xarea = xarea->next;
   }

   part = ch->first_part;
   while( part )
   {
      if( part->cond != PART_WELL || part->flags != PART_WELL )
         fprintf( fp, "Part	%d %d %d\n", part->loc, part->cond, part->flags );
      part = part->next;
   }

   for( sn = 1; sn < top_sn; sn++ )
   {
      if( skill_table[sn]->name && ch->pcdata->learned[sn] > 0 )
         switch ( skill_table[sn]->type )
         {
            default:
               fprintf( fp, "Skill        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_SPELL:
               fprintf( fp, "Spell        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
         }
   }

   for( paf = ch->first_affect; paf; paf = paf->next )
   {
      if( paf->type >= 0 && ( skill = get_skilltype( paf->type ) ) == NULL )
         continue;

      if( paf->type >= 0 && paf->type < TYPE_RACIAL )
         fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n",
                  skill->name, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
      else
         fprintf( fp, "Affect       %3d %3d %3d %3d %s\n",
                  paf->type, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
   }

   if( ch->pcdata->first_qbit )
   {
      BIT_DATA *bit;

      for( bit = ch->pcdata->first_qbit; bit; bit = bit->next )
         fprintf( fp, "Qbit	%d %s~\n", bit->number, bit->desc );
   }

   if( ch->species )
      fprintf( fp, "Species	%s~\n", ch->species );
   if( ch->pcdata->type )
      fprintf( fp, "Type	%s~\n", ch->pcdata->type );
   if( ch->pcdata->parent )
      fprintf( fp, "Parent	%s~\n", ch->pcdata->parent );
   if( ch->pcdata->family )
      fprintf( fp, "Family	%s~\n", ch->pcdata->family );
   fprintf( fp, "Generation	%d\n", ch->pcdata->gen );
   fprintf( fp, "Age	%d\n", ch->pcdata->age_adjust );
   fprintf( fp, "Shield	%d\n", ch->shield );
   if( ch->pcdata->spouse )
      fprintf( fp, "Spouse	%s~\n", ch->pcdata->spouse );
   if( ch->pcdata->maiden_name )
      fprintf( fp, "Maiden	%s~\n", ch->pcdata->maiden_name );

   if( ch->pcdata->name_disguise )
      fprintf( fp, "Ndisguise	%s~\n", ch->pcdata->name_disguise );
   if( ch->pcdata->were_race )
      fprintf( fp, "Wererace	%s~\n", ch->pcdata->were_race );
   if( ch->pcdata->auto_attack )
      fprintf( fp, "AutoAttack	%s~\n", ch->pcdata->auto_attack );
   fprintf( fp, "Speed	%d\n", ch->speed );
   fprintf( fp, "Inborn	%d\n", ch->pcdata->inborn );
   fprintf( fp, "Channels	%s~\n", ch->pcdata->channels );
   if( ch->pcdata->outputprefix )
      fprintf( fp, "Outputprefix %s~\n", ch->pcdata->outputprefix );
   if( ch->pcdata->outputsuffix )
      fprintf( fp, "Outputsuffix %s~\n", ch->pcdata->outputsuffix );
   if( ch->pcdata->eyes )
      fprintf( fp, "Eyes	%s~\n", ch->pcdata->eyes );
   if( ch->pcdata->hair )
      fprintf( fp, "Hair	%s~\n", ch->pcdata->hair );
   if( ch->pcdata->skin_color )
      fprintf( fp, "Skin_color	%s~\n", ch->pcdata->skin_color );
   if( ch->pcdata->skin_type )
      fprintf( fp, "Skin_type	%s~\n", ch->pcdata->skin_type );
   if( ch->pcdata->extra_color )
      fprintf( fp, "Extra_color	%s~\n", ch->pcdata->extra_color );
   if( ch->pcdata->extra_type )
      fprintf( fp, "Extra_type	%s~\n", ch->pcdata->extra_type );

#ifdef I3
   i3save_char( ch, fp );
#endif
#ifdef IMC
   imc_savechar( ch, fp );
#endif

   fprintf( fp, "End\n\n" );
   return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest, sh_int os_type )
{
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *paf;
   sh_int wear, wear_loc, x;

   if( iNest >= MAX_NEST )
   {
      bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
      return;
   }

   /*
    * Slick recursion to write lists backwards,
    *   so loading them will load in forwards order.
    */
   if( obj->prev_content && os_type != OS_GEM )
      if( os_type == OS_CARRY )
         fwrite_obj( ch, obj->prev_content, fp, iNest, OS_CARRY );

   /*
    * Castrate storage characters.
    * Catch deleted objects                                    -Thoric
    * Do NOT save prototype items!          -Thoric
    */
   if( obj_extracted( obj ) || IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      return;

   /*
    * Gem saving -Keolah 
    */
   fprintf( fp, ( os_type == OS_GEM ? "#GEM\n" : "#OBJECT\n" ) );

   if( iNest )
      fprintf( fp, "Nest         %d\n", iNest );
   if( obj->count > 1 )
      fprintf( fp, "Count        %d\n", obj->count );
   if( QUICKMATCH( obj->name, obj->pIndexData->name ) == 0 )
      fprintf( fp, "Name         %s~\n", obj->name );
   if( QUICKMATCH( obj->short_descr, obj->pIndexData->short_descr ) == 0 )
      fprintf( fp, "ShortDescr   %s~\n", obj->short_descr );
   if( QUICKMATCH( obj->description, obj->pIndexData->description ) == 0 )
      fprintf( fp, "Description  %s~\n", obj->description );
   if( obj->obj_by != NULL )
      fprintf( fp, "ObjBy	%s~\n", obj->obj_by );
   if( QUICKMATCH( obj->action_desc, obj->pIndexData->action_desc ) == 0 )
      fprintf( fp, "ActionDesc   %s~\n", obj->action_desc );
   fprintf( fp, "Vnum         %d\n", obj->pIndexData->vnum );
   if( obj->material != NULL )
      fprintf( fp, "Material	%d\n", obj->material->number );
   if( os_type == OS_GROUND && obj->in_room )
      fprintf( fp, "Room         %d\n", obj->in_room->vnum );
/*    else if (((IS_OBJ_STAT(obj, ITEM_ARTIFACT)) || (IS_OBJ_STAT(obj, ITEM_PLRBLD))) && !obj->in_room)
		fprintf( fp, "Room		%d\n",  get_obj_room_vnum_recursive(obj));
*/ if( !xSAME_BITS( obj->extra_flags, obj->pIndexData->extra_flags ) )
      fprintf( fp, "ExtraFlags   %s\n", print_bitvector( &obj->extra_flags ) );
   wear_loc = -1;
   for( wear = 0; wear < MAX_WEAR; wear++ )
      for( x = 0; x < MAX_LAYERS; x++ )
         if( obj == save_equipment[wear][x] )
         {
            wear_loc = wear;
            break;
         }
         else if( !save_equipment[wear][x] )
            break;
   if( wear_loc != -1 )
      fprintf( fp, "WearLoc      %d\n", wear_loc );
   if( obj->item_type != obj->pIndexData->item_type )
      fprintf( fp, "ItemType     %d\n", obj->item_type );
   if( obj->weight != obj->pIndexData->weight )
      fprintf( fp, "Weight       %d\n", obj->weight );
   fprintf( fp, "Condition	%d\n", obj->condition );
   if( obj->size && !xIS_EMPTY( obj->parts ) )
      fprintf( fp, "Size	%d\n", obj->size );
   if( obj->mana )
      fprintf( fp, "Mana        %d\n", obj->mana );
   if( obj->raw_mana )
      fprintf( fp, "RawMana     %d\n", obj->raw_mana );
   if( obj->timer )
      fprintf( fp, "Timer        %d\n", obj->timer );
   if( obj->cost != obj->pIndexData->cost )
      fprintf( fp, "Cost         %d\n", obj->cost );
   if( obj->value[0] || obj->value[1] || obj->value[2] || obj->value[3] || obj->value[4] || obj->value[5] || obj->value[6] )
      fprintf( fp, "Values       %d %d %d %d %d %d %d\n",
               obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5], obj->value[6] );

   switch ( obj->item_type )
   {
      case ITEM_PILL:  /* was down there with staff and wand, wrongly - Scryn */
      case ITEM_POTION:
      case ITEM_SCROLL:
         if( IS_VALID_SN( obj->value[1] ) )
            fprintf( fp, "Spell 1      '%s'\n", skill_table[obj->value[1]]->name );

         if( IS_VALID_SN( obj->value[2] ) )
            fprintf( fp, "Spell 2      '%s'\n", skill_table[obj->value[2]]->name );

         if( IS_VALID_SN( obj->value[3] ) )
            fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );

         break;

      case ITEM_STAFF:
      case ITEM_WAND:
         if( IS_VALID_SN( obj->value[3] ) )
            fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );

         break;
      case ITEM_SALVE:
         if( IS_VALID_SN( obj->value[4] ) )
            fprintf( fp, "Spell 4      '%s'\n", skill_table[obj->value[4]]->name );

         if( IS_VALID_SN( obj->value[5] ) )
            fprintf( fp, "Spell 5      '%s'\n", skill_table[obj->value[5]]->name );
         break;
   }

   for( paf = obj->first_affect; paf; paf = paf->next )
   {
      /*
       * Save extra object affects           -Thoric
       */
      if( paf->type < 0 || paf->type >= top_sn )
      {
         fprintf( fp, "Affect       %d %d %d %d %s\n",
                  paf->type,
                  paf->duration,
                  ( ( paf->location == APPLY_WEAPONSPELL
                      || paf->location == APPLY_WEARSPELL
                      || paf->location == APPLY_REMOVESPELL
                      || paf->location == APPLY_STRIPSN
                      || paf->location == APPLY_RECURRINGSPELL )
                    && IS_VALID_SN( paf->modifier ) )
                  ? skill_table[paf->modifier]->slot : paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
      }
      else
         fprintf( fp, "AffectData   '%s' %d %d %d %s\n",
                  skill_table[paf->type]->name,
                  paf->duration,
                  ( ( paf->location == APPLY_WEAPONSPELL
                      || paf->location == APPLY_WEARSPELL
                      || paf->location == APPLY_REMOVESPELL
                      || paf->location == APPLY_STRIPSN
                      || paf->location == APPLY_RECURRINGSPELL )
                    && IS_VALID_SN( paf->modifier ) )
                  ? skill_table[paf->modifier]->slot : paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
   }

   for( ed = obj->first_extradesc; ed; ed = ed->next )
      fprintf( fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description );

   fprintf( fp, "End\n\n" );

   /*
    * If we want multiple gems on an item,
    * * will need to change them to a linked list or
    * * something. One gem is plenty for me though. -keo
    */
   if( obj->gem && obj->name )
      fwrite_obj( ch, obj->gem, fp, iNest + 1, OS_GEM );

   if( obj->first_content )
      fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY );

   return;
}

void fwrite_alias( CHAR_DATA * ch, ALIAS_DATA * first_alias, FILE * fp )
{
   ALIAS_DATA *alias;

   for( alias = first_alias; alias; alias = alias->next )
   {
      fprintf( fp, "#ALIAS\n" );
      fprintf( fp, "Name     %s~\n", alias->name );
      fprintf( fp, "Commands %s~\n", alias->alias );
      fprintf( fp, "End\n\n" );
   }
   return;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA * d, char *name, bool preload )
{
   char strsave[MAX_INPUT_LENGTH];
   CHAR_DATA *ch;
   FILE *fp;
   bool found;
   struct stat fst;
   int i, x;
   extern FILE *fpArea;
   extern char strArea[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];

   CREATE( ch, CHAR_DATA, 1 );
   for( x = 0; x < MAX_WEAR; x++ )
      for( i = 0; i < MAX_LAYERS; i++ )
         save_equipment[x][i] = NULL;
   clear_char( ch );
   loading_char = ch;

   CREATE( ch->pcdata, PC_DATA, 1 );
   d->character = ch;
   ch->desc = d;
   ch->pcdata->filename = STRALLOC( name );
   ch->name = NULL;
   ch->act = multimeb( PLR_BLANK, PLR_COMBINE, PLR_PROMPT, -1 );
   ch->perm_str = 100;
   ch->perm_int = 100;
   ch->perm_wis = 100;
   ch->perm_dex = 100;
   ch->perm_con = 100;
   ch->perm_cha = 100;
   ch->perm_lck = 100;
   ch->base_hit = 0;
   ch->base_mana = 0;
   ch->base_move = 0;
   ch->no_resistant = 0;
   ch->no_susceptible = 0;
   ch->no_immune = 0;
   ch->was_in_room = NULL;
   xCLEAR_BITS( ch->no_affected_by );
   xCLEAR_BITS( ch->pcdata->perm_aff );
   ch->pcdata->condition[COND_THIRST] = 48;
   ch->pcdata->condition[COND_FULL] = 48;
   ch->pcdata->wizinvis = 0;
   ch->pcdata->family = NULL;
   ch->pcdata->gen = 1;
   ch->pcdata->auto_attack = NULL;
   ch->in_obj = NULL;
   ch->on = NULL;
   ch->pcdata->spouse = NULL;
   ch->pcdata->eyes = NULL;
   ch->pcdata->hair = NULL;
   ch->pcdata->skin_color = NULL;
   ch->pcdata->skin_type = NULL;
   ch->pcdata->extra_color = NULL;
   ch->pcdata->extra_type = NULL;
   ch->pcdata->first_mutation = NULL;
   ch->pcdata->last_mutation = NULL;
   ch->pcdata->maiden_name = NULL;
   ch->pcdata->name_disguise = NULL;
   ch->pcdata->channels = NULL;
   ch->pcdata->outputprefix = NULL;
   ch->pcdata->outputsuffix = NULL;
   ch->pcdata->permissions = 0;
   ch->pcdata->inborn = -1;
   ch->pcdata->age_adjust = 0;
   ch->mental_state = -10;
   ch->mobinvis = 0;
   for( i = 0; i < MAX_SKILL; i++ )
      ch->pcdata->learned[i] = 0;
   ch->pcdata->release_date = 0;
   ch->pcdata->helled_by = NULL;
   ch->saving_poison_death = 0;
   ch->saving_wand = 0;
   ch->saving_para_petri = 0;
   ch->saving_breath = 0;
   ch->saving_spell_staff = 0;
   ch->pcdata->pagerlen = 24;
#ifdef I3
   i3init_char( ch );
#endif
#ifdef IMC
   imc_initchar( ch );
#endif

   found = FALSE;
   sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
   if( stat( strsave, &fst ) != -1 )
   {
      if( fst.st_size == 0 )
      {
         sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower( name[0] ), capitalize( name ) );
         send_to_char( "Restoring your backup player file...", ch );
         bug( "Restoring backup pfile...", 0 );
      }
      else
      {
         sprintf( buf, "%s player data for: %s (%dK)",
                  preload ? "Preloading" : "Loading", ch->pcdata->filename, ( int )fst.st_size / 1024 );
         log_string_plus( buf, LOG_COMM, 0 );
      }
   }
   /*
    * else no player file 
    */

   if( ( fp = fopen( strsave, "r" ) ) == NULL )
   {
      return FALSE;
   }  /* if fopen */
   else
   {
      int iNest;

      for( iNest = 0; iNest < MAX_NEST; iNest++ )
         rgObjNest[iNest] = NULL;

      found = TRUE;
      /*
       * Cheat so that bug will show line #'s -- Altrag 
       */
      fpArea = fp;
      strcpy( strArea, strsave );
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
            bug( "Load_char_obj: # not found.", 0 );
            bug( name, 0 );
            return FALSE;
         }

         word = fread_word( fp );
         if( !strcmp( word, "PLAYER" ) )
         {
            fread_char( ch, fp, preload );
            if( preload )
               break;
         }
         else if( !strcmp( word, "ALIAS" ) ) /* Aliases     */
            fread_alias( ch, fp );
         else if( !strcmp( word, "OBJECT" ) )   /* Objects  */
            fread_obj( ch, fp, OS_CARRY );
         else if( !strcmp( word, "GEM" ) )
            fread_obj( ch, fp, OS_GEM );
         else if( !strcmp( word, "MOBILE" ) )
         {
            CHAR_DATA *mob;

            mob = fread_mobile( fp );
            if( !mob )
               bug( "Bad pet vnum.", 0 );
            else
            {
               ch->pcdata->pet = mob;
               mob->master = ch;
               xSET_BIT( mob->affected_by, AFF_CHARM );
            }
         }
         else if( !strcmp( word, "END" ) )   /* Done     */
            break;
         else
         {
            bug( "Load_char_obj: bad section.", 0 );
            bug( name, 0 );
            break;
         }

      }
      fclose( fp );
      fpArea = NULL;
      strcpy( strArea, "$" );
   }

   if( !found )
   {
      ch->name = STRALLOC( name );
      ch->short_descr = STRALLOC( "" );
      ch->description = STRALLOC( "" );
      ch->species = STRALLOC( "human" );
      ch->nation = NULL;
      ch->speed = 100;
      ch->pcdata->parent = STRALLOC( "" );
      ch->encumberance = 0;
      ch->last_hit = NULL;
      ch->pcdata->were_race = STRALLOC( "" );
      ch->shield = 0;
      ch->pcdata->consenting = NULL;
      ch->pcdata->magiclink = NULL;
      ch->pcdata->mindlink = NULL;
      ch->editor = NULL;
      ch->pcdata->deity_name = STRALLOC( "" );
      ch->pcdata->deity = NULL;
      ch->pcdata->pet = NULL;
      ch->pcdata->pwd = str_dup( "" );
      ch->pcdata->bamfin = NULL;
      ch->pcdata->bamfout = NULL;
      ch->pcdata->rank = str_dup( "" );
      ch->pcdata->title = STRALLOC( "" );
      ch->pcdata->homepage = str_dup( "" );
      ch->pcdata->bio = STRALLOC( "" );
      ch->pcdata->prompt = STRALLOC( "" );
      ch->pcdata->fprompt = STRALLOC( "" );
      ch->pcdata->r_range_lo = 0;
      ch->pcdata->r_range_hi = 0;
      ch->pcdata->m_range_lo = 0;
      ch->pcdata->m_range_hi = 0;
      ch->pcdata->o_range_lo = 0;
      ch->pcdata->o_range_hi = 0;
      ch->pcdata->rkills = 0;
      ch->pcdata->rdeaths = 0;
      ch->pcdata->wizinvis = 0;

   }
   else
   {
      if( !ch->name )
         ch->name = STRALLOC( name );
      if( !ch->pcdata->deity_name )
      {
         ch->pcdata->deity_name = STRALLOC( "" );
         ch->pcdata->deity = NULL;
      }
      if( !ch->pcdata->bio )
         ch->pcdata->bio = STRALLOC( "" );

      {  /* Give 'em a brand new body! -- Scion */
         check_bodyparts( ch );
      }

      if( IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
      {
         assign_area( ch );
      }
      if( file_ver > 1 )
      {
         for( i = 0; i < MAX_WEAR; i++ )
         {
            for( x = 0; x < MAX_LAYERS; x++ )
            {
               if( save_equipment[i][x] )
               {
                  equip_char( ch, save_equipment[i][x] );
                  save_equipment[i][x] = NULL;
               }
               else
                  break;
            }
         }
      }
   }

   /*
    * Rebuild affected_by and RIS to catch errors - FB 
    */
   update_aris( ch );

   loading_char = NULL;
   return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA * ch, FILE * fp, bool preload )
{
   PART_DATA *part;
   EXPLORED_AREA *xarea;
   char buf[MAX_STRING_LENGTH];
   char *line;
   char *word;
   int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20;
   sh_int killcnt;
   bool fMatch;
   int count = 0;

   file_ver = 0;
   killcnt = 0;
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
            KEY( "Act", ch->act, fread_bitvector( fp ) );
            KEY( "AffectedBy", ch->affected_by, fread_bitvector( fp ) );
            KEY( "AutoAttack", ch->pcdata->auto_attack, fread_string( fp ) );
            KEY( "Age", ch->pcdata->age_adjust, fread_number( fp ) );
            KEY( "Armor", ch->armor, fread_number( fp ) );

            /*
             * convert from old system -keo 
             */
            if( ch->armor < 0 )
               ch->armor = abs( ch->armor / 10 );

            if( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;

               if( preload )
               {
                  fMatch = TRUE;
                  fread_to_eol( fp );
                  break;
               }
               CREATE( paf, AFFECT_DATA, 1 );
               if( !strcmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
               else
               {
                  int sn;
                  char *sname = fread_word( fp );

                  if( ( sn = skill_lookup( sname ) ) < 0 )
                  {
                     if( ( sn = herb_lookup( sname ) ) < 0 )
                        bug( "Fread_char: unknown skill.", 0 );
                     else
                        sn += TYPE_HERB;
                  }
                  paf->type = sn;
               }

               paf->duration = fread_number( fp );
               paf->modifier = fread_number( fp );
               paf->location = fread_number( fp );
               if( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_REMOVESPELL
                   || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL )
                  paf->modifier = slot_lookup( paf->modifier );
               paf->bitvector = fread_bitvector( fp );
               LINK( paf, ch->first_affect, ch->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "AttrMod" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 13;
               sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               ch->mod_str = x1;
               ch->mod_int = x2;
               ch->mod_wis = x3;
               ch->mod_dex = x4;
               ch->mod_con = x5;
               ch->mod_cha = x6;
               ch->mod_lck = x7;
               fMatch = TRUE;
               break;
            }
            if( !str_cmp( word, "Alias" ) )
            {
               if( count >= MAX_ALIAS )
               {
                  fread_to_eol( fp );
                  fMatch = TRUE;
                  break;
               }

               ch->pcdata->alias[count] = str_dup( fread_word( fp ) );
               ch->pcdata->alias_sub[count] = fread_string_nohash( fp );
               count++;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "AttrPerm" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
               sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               ch->perm_str = x1;
               ch->perm_int = x2;
               ch->perm_wis = x3;
               ch->perm_dex = x4;
               ch->perm_con = x5;
               ch->perm_cha = x6;
               ch->perm_lck = x7;
               fMatch = TRUE;
               break;
            }
            break;

         case 'B':
            KEY( "Bamfin", ch->pcdata->bamfin, fread_string_nohash( fp ) );
            KEY( "Bamfout", ch->pcdata->bamfout, fread_string_nohash( fp ) );
            KEY( "Bio", ch->pcdata->bio, fread_string( fp ) );
            KEY( "Bank", ch->pcdata->balance, fread_number( fp ) );
            KEY( "Bounty", ch->pcdata->bounty, fread_number( fp ) );
            break;

         case 'C':
            KEY( "Channels", ch->pcdata->channels, fread_string( fp ) );
            if( !strcmp( word, "Condition" ) )
            {
               line = fread_line( fp );
               sscanf( line, "%d %d %d %d", &x1, &x2, &x3, &x4 );
               ch->pcdata->condition[0] = x1;
               ch->pcdata->condition[1] = x2;
               ch->pcdata->condition[2] = x3;
               ch->pcdata->condition[3] = x4;
               fMatch = TRUE;
               break;
            }

            break;

         case 'D':
            KEY( "Damroll", ch->damroll, fread_number( fp ) );
            KEY( "Description", ch->description, fread_string( fp ) );
            break;

            /*
             * 'E' was moved to after 'S' 
             */
         case 'F':
            KEY( "Family", ch->pcdata->family, fread_string( fp ) );
            if( !strcmp( word, "Filename" ) )
            {
               /*
                * File Name already set externally.
                */
               fread_to_eol( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "Flags", ch->pcdata->flags, fread_number( fp ) );
            KEY( "FPrompt", ch->pcdata->fprompt, fread_string( fp ) );
            break;

         case 'G':
            KEY( "Generation", ch->pcdata->gen, fread_number( fp ) );
            KEY( "Gold", ch->gold, fread_number( fp ) );
            break;

         case 'H':
            KEY( "Hair", ch->pcdata->hair, fread_string( fp ) );
            KEY( "Height", ch->height, fread_number( fp ) );

            if( !strcmp( word, "Helled" ) )
            {
               ch->pcdata->release_date = fread_number( fp );
               ch->pcdata->helled_by = fread_string( fp );
               if( ch->pcdata->release_date < current_time )
               {
                  STRFREE( ch->pcdata->helled_by );
                  ch->pcdata->helled_by = NULL;
                  ch->pcdata->release_date = 0;
               }
               fMatch = TRUE;
               break;
            }

            KEY( "Hitroll", ch->hitroll, fread_number( fp ) );
            KEY( "Homepage", ch->pcdata->homepage, fread_string_nohash( fp ) );

            if( !strcmp( word, "HpManaMove" ) )
            {
               ch->hit = fread_number( fp );
               ch->max_hit = fread_number( fp );
               ch->mana = fread_number( fp );
               ch->max_mana = fread_number( fp );
               ch->move = fread_number( fp );
               ch->max_move = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'I':
            KEY( "Inborn", ch->pcdata->inborn, fread_number( fp ) );
            KEY( "Immune", ch->immune, fread_number( fp ) );
#ifdef I3
            if( ( fMatch = i3load_char( ch, fp, word ) ) )
               break;
#endif
#ifdef IMC
            if( ( fMatch = imc_loadchar( ch, fp, word ) ) )
               break;
#endif
            break;

         case 'K':
            break;

         case 'L':
            KEY( "Long", ch->description, fread_string( fp ) );
            break;

         case 'M':
            KEY( "Maiden", ch->pcdata->maiden_name, fread_string( fp ) );
            KEY( "MDeaths", ch->pcdata->mdeaths, fread_number( fp ) );
            if( !strcmp( word, "Memorized" ) )
            {
               ch->pcdata->memorize[0] = fread_number( fp );
               ch->pcdata->memorize[1] = fread_number( fp );
               ch->pcdata->memorize[2] = fread_number( fp );
               ch->pcdata->memorize[3] = fread_number( fp );
               ch->pcdata->memorize[4] = fread_number( fp );
               ch->pcdata->memorize[5] = fread_number( fp );
               ch->pcdata->memorize[6] = fread_number( fp );
               ch->pcdata->memorize[7] = fread_number( fp );
               fMatch = TRUE;
            }
            KEY( "Mentalstate", ch->mental_state, fread_number( fp ) );
            KEY( "MKills", ch->pcdata->mkills, fread_number( fp ) );
            KEY( "Mobinvis", ch->mobinvis, fread_number( fp ) );
            if( !strcmp( word, "MobRange" ) )
            {
               ch->pcdata->m_range_lo = fread_number( fp );
               ch->pcdata->m_range_hi = fread_number( fp );
               fMatch = TRUE;
            }
            break;

         case 'N':
            if( !strcmp( word, "Noncombat" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = x15 = x16 = x17 = x18 = x19 = x20 =
                  1;
               sscanf( line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6,
                       &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15, &x16, &x17, &x18, &x19, &x20 );
               ch->pcdata->noncombat[0] = x1;
               ch->pcdata->noncombat[1] = x2;
               ch->pcdata->noncombat[2] = x3;
               ch->pcdata->noncombat[3] = x4;
               ch->pcdata->noncombat[4] = x5;
               ch->pcdata->noncombat[5] = x6;
               ch->pcdata->noncombat[6] = x7;
               ch->pcdata->noncombat[7] = x8;
               ch->pcdata->noncombat[8] = x9;
               ch->pcdata->noncombat[9] = x10;
               ch->pcdata->noncombat[10] = x11;
               ch->pcdata->noncombat[11] = x12;
               ch->pcdata->noncombat[12] = x13;
               ch->pcdata->noncombat[13] = x14;
               ch->pcdata->noncombat[14] = x15;
               ch->pcdata->noncombat[15] = x16;
               ch->pcdata->noncombat[16] = x17;
               ch->pcdata->noncombat[17] = x18;
               ch->pcdata->noncombat[18] = x19;
               ch->pcdata->noncombat[19] = x20;
               fMatch = TRUE;
               break;
            }
            if( !strcmp( word, "NoncombatExp" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = x15 = x16 = x17 = x18 = x19 = x20 =
                  1;
               sscanf( line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6,
                       &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15, &x16, &x17, &x18, &x19, &x20 );
               ch->pcdata->noncombat_exp[0] = x1;
               ch->pcdata->noncombat_exp[1] = x2;
               ch->pcdata->noncombat_exp[2] = x3;
               ch->pcdata->noncombat_exp[3] = x4;
               ch->pcdata->noncombat_exp[4] = x5;
               ch->pcdata->noncombat_exp[5] = x6;
               ch->pcdata->noncombat_exp[6] = x7;
               ch->pcdata->noncombat_exp[7] = x8;
               ch->pcdata->noncombat_exp[8] = x9;
               ch->pcdata->noncombat_exp[9] = x10;
               ch->pcdata->noncombat_exp[10] = x11;
               ch->pcdata->noncombat_exp[11] = x12;
               ch->pcdata->noncombat_exp[12] = x13;
               ch->pcdata->noncombat_exp[13] = x14;
               ch->pcdata->noncombat_exp[14] = x15;
               ch->pcdata->noncombat_exp[15] = x16;
               ch->pcdata->noncombat_exp[16] = x17;
               ch->pcdata->noncombat_exp[17] = x18;
               ch->pcdata->noncombat_exp[18] = x19;
               ch->pcdata->noncombat_exp[19] = x20;
               fMatch = TRUE;
               break;
            }
            KEY( "Name", ch->name, fread_string( fp ) );
            KEY( "Ndisguise", ch->pcdata->name_disguise, fread_string( fp ) );
            KEY( "NoAffectedBy", ch->no_affected_by, fread_bitvector( fp ) );
            KEY( "NoImmune", ch->no_immune, fread_number( fp ) );
            KEY( "NoResistant", ch->no_resistant, fread_number( fp ) );
            KEY( "NoSusceptible", ch->no_susceptible, fread_number( fp ) );
            break;
         case 'O':
            KEY( "Outputprefix", ch->pcdata->outputprefix, fread_string( fp ) );
            KEY( "Outputsuffix", ch->pcdata->outputsuffix, fread_string( fp ) );
            if( !strcmp( word, "ObjRange" ) )
            {
               ch->pcdata->o_range_lo = fread_number( fp );
               ch->pcdata->o_range_hi = fread_number( fp );
               fMatch = TRUE;
            }
            break;

         case 'P':
            KEY( "Permissions", ch->pcdata->permissions, fread_number( fp ) );
            KEY( "Pagerlen", ch->pcdata->pagerlen, fread_number( fp ) );
            KEY( "Parent", ch->pcdata->parent, fread_string( fp ) );
            if( !strcmp( word, "Part" ) )
            {
               if( !ch->first_part )
                  check_bodyparts( ch );
               if( ( part = find_bodypart( ch, fread_number( fp ) ) ) != NULL )
               {
                  part->cond = fread_number( fp );
                  part->flags = fread_number( fp );
               }
               fMatch = TRUE;
               break;
            }
            KEY( "Password", ch->pcdata->pwd, fread_string_nohash( fp ) );
            KEY( "PDeaths", ch->pcdata->pdeaths, fread_number( fp ) );
            KEY( "PKills", ch->pcdata->pkills, fread_number( fp ) );
            KEY( "PermAffects", ch->pcdata->perm_aff, fread_bitvector( fp ) );
            KEY( "Played", ch->played, fread_number( fp ) );
            KEY( "Points", ch->pcdata->points, fread_number( fp ) );
            /*
             *  new positions are stored in the file from 100 up
             *  old positions are from 0 up
             *  if reading an old position, some translation is necessary
             */
            if( !strcmp( word, "Position" ) )
            {
               ch->position = fread_number( fp );
               if( ch->position < 100 )
               {
                  switch ( ch->position )
                  {
                     default:;
                     case 0:;
                     case 1:;
                     case 2:;
                     case 3:;
                     case 4:
                        break;
                     case 5:
                        ch->position = 6;
                        break;
                     case 6:
                        ch->position = 7;
                        break;
                     case 7:
                        ch->position = 9;
                        break;
                     case 8:
                        ch->position = 12;
                        break;
                     case 9:
                        ch->position = 13;
                        break;
                     case 10:
                        ch->position = 14;
                        break;
                     case 11:
                        ch->position = 15;
                        break;
                  }
                  fMatch = TRUE;
               }
               else
               {
                  ch->position -= 100;
                  fMatch = TRUE;
               }
            }
            KEY( "Prompt", ch->pcdata->prompt, fread_string( fp ) );
            if( !strcmp( word, "PTimer" ) )
            {
               add_timer( ch, TIMER_PKILLED, fread_number( fp ), NULL, 0 );
               fMatch = TRUE;
               break;
            }
            break;

         case 'Q':
            if( !strcmp( word, "Qbit" ) )
            {
               BIT_DATA *bit;
               BIT_DATA *desc;

               CREATE( bit, BIT_DATA, 1 );

               bit->number = fread_number( fp );
               if( ( desc = find_qbit( bit->number ) ) == NULL )
                  strcpy( bit->desc, fread_string( fp ) );
               else
               {
                  strcpy( bit->desc, desc->desc );
                  fread_string( fp );
               }

               LINK( bit, ch->pcdata->first_qbit, ch->pcdata->last_qbit, next, prev );
               fMatch = TRUE;
            }
            break;
         case 'R':
            KEY( "Race", ch->race, fread_number( fp ) );
            KEY( "Rank", ch->pcdata->rank, fread_string_nohash( fp ) );
            KEY( "Resistant", ch->resistant, fread_number( fp ) );
            KEY( "Restore_time", ch->pcdata->restore_time, fread_number( fp ) );
            KEY( "RKills", ch->pcdata->rkills, fread_number( fp ) );
            KEY( "RDeaths", ch->pcdata->rdeaths, fread_number( fp ) );

            if( !strcmp( word, "Room" ) )
            {
               ch->in_room = get_room_index( fread_number( fp ) );
               if( !ch->in_room )
                  ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
               fMatch = TRUE;
               break;
            }
            if( !strcmp( word, "RoomRange" ) )
            {
               ch->pcdata->r_range_lo = fread_number( fp );
               ch->pcdata->r_range_hi = fread_number( fp );
               fMatch = TRUE;
            }
            break;

         case 'S':
            KEY( "Sex", ch->sex, fread_number( fp ) );
            KEY( "Shield", ch->shield, fread_number( fp ) );
            KEY( "Speed", ch->speed, fread_number( fp ) );
            KEY( "ShortDescr", ch->short_descr, fread_string( fp ) );
            KEY( "Skin_color", ch->pcdata->skin_color, fread_string( fp ) );
            KEY( "Skin_type", ch->pcdata->skin_type, fread_string( fp ) );
            KEY( "Species", ch->species, fread_string( fp ) );
            KEY( "Spouse", ch->pcdata->spouse, fread_string( fp ) );
            KEY( "Susceptible", ch->susceptible, fread_number( fp ) );
            if( ch->species )
            {
               ch->nation = find_nation( ch->species );
               ch->xflags = ch->nation->parts;
            }
            if( !strcmp( word, "SavingThrow" ) )
            {
               ch->saving_wand = fread_number( fp );
               ch->saving_poison_death = ch->saving_wand;
               ch->saving_para_petri = ch->saving_wand;
               ch->saving_breath = ch->saving_wand;
               ch->saving_spell_staff = ch->saving_wand;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "SavingThrows" ) )
            {
               ch->saving_poison_death = fread_number( fp );
               ch->saving_wand = fread_number( fp );
               ch->saving_para_petri = fread_number( fp );
               ch->saving_breath = fread_number( fp );
               ch->saving_spell_staff = fread_number( fp );
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "Site" ) )
            {
               if( !preload )
               {
                  sprintf( buf, "Last connected from: %s\n\r", fread_word( fp ) );
                  send_to_char( buf, ch );
               }
               else
                  fread_to_eol( fp );
               fMatch = TRUE;
               if( preload )
                  word = "End";
               else
                  break;
            }

            if( !strcmp( word, "Skill" ) )
            {
               int sn;
               int value;

               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );
                  if( file_ver < 3 )
                     sn = skill_lookup( fread_word( fp ) );
                  else
                     sn = bsearch_skill_exact( fread_word( fp ), gsn_first_skill, gsn_top_sn - 1 );
                  if( sn < 0 )
                     bug( "Fread_char: unknown skill.", 0 );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                  }
                  fMatch = TRUE;
                  break;
               }
            }

            if( !strcmp( word, "Spell" ) )
            {
               int sn;
               int value;

               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );

                  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_spell, gsn_first_skill - 1 );
                  if( sn < 0 )
                     bug( "Fread_char: unknown spell.", 0 );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( skill_table[sn]->skill_level[get_best_talent( ch, sn )] >= 1000 )
                     {
                        ch->pcdata->learned[sn] = 0;
                     }
                  }
                  fMatch = TRUE;
                  break;
               }
            }
            if( strcmp( word, "End" ) )
               break;

         case 'E':
            if( !strcmp( word, "End" ) )
            {
               if( !ch->short_descr )
                  ch->short_descr = STRALLOC( "" );
               if( !ch->description )
                  ch->description = STRALLOC( "" );
               if( !ch->pcdata->pwd )
                  ch->pcdata->pwd = str_dup( "" );
               if( !ch->pcdata->bio )
                  ch->pcdata->bio = STRALLOC( "" );
               if( !ch->pcdata->rank )
                  ch->pcdata->rank = str_dup( "" );
               if( !ch->pcdata->title )
                  ch->pcdata->title = STRALLOC( "" );
               if( !ch->pcdata->homepage )
                  ch->pcdata->homepage = str_dup( "" );
               if( !ch->pcdata->prompt )
                  ch->pcdata->prompt = STRALLOC( "" );
               if( !ch->pcdata->fprompt )
                  ch->pcdata->fprompt = STRALLOC( "" );
               ch->editor = NULL;

               if( !ch->pcdata->prompt )
                  ch->pcdata->prompt = STRALLOC( "" );

               return;
            }
            KEY( "Exp", ch->exp, fread_number( fp ) );
            if( !strcmp( word, "Explored" ) )
            {
               CREATE( xarea, EXPLORED_AREA, 1 );
               xarea->index = fread_number( fp );
               xarea->rooms = fread_bitvector( fp );
               LINK( xarea, ch->pcdata->first_explored, ch->pcdata->last_explored, next, prev );
               fMatch = TRUE;
               break;
            }
            KEY( "Extra_color", ch->pcdata->extra_color, fread_string( fp ) );
            KEY( "Extra_type", ch->pcdata->extra_type, fread_string( fp ) );
            KEY( "Eyes", ch->pcdata->eyes, fread_string( fp ) );
            break;

         case 'T':
            if( !strcmp( word, "Talent" ) )
            {
               int i;

               i = fread_number( fp );

               if( i == -1 || i >= MAX_DEITY )
                  bug( "Fread_char: illegal Talent number %d", i );
               else
               {
                  ch->talent[i] = fread_number( fp );
                  ch->curr_talent[i] = fread_number( fp );
               }
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "TalentExp" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = x15 = x16 = x17 = x18 = x19 = 1;
               sscanf( line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                       &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10,
                       &x11, &x12, &x13, &x14, &x15, &x16, &x17, &x18, &x19 );
               ch->pcdata->talent_exp[0] = x1;
               ch->pcdata->talent_exp[1] = x2;
               ch->pcdata->talent_exp[2] = x3;
               ch->pcdata->talent_exp[3] = x4;
               ch->pcdata->talent_exp[4] = x5;
               ch->pcdata->talent_exp[5] = x6;
               ch->pcdata->talent_exp[6] = x7;
               ch->pcdata->talent_exp[7] = x8;
               ch->pcdata->talent_exp[8] = x9;
               ch->pcdata->talent_exp[9] = x10;
               ch->pcdata->talent_exp[10] = x11;
               ch->pcdata->talent_exp[11] = x12;
               ch->pcdata->talent_exp[12] = x13;
               ch->pcdata->talent_exp[13] = x14;
               ch->pcdata->talent_exp[14] = x15;
               ch->pcdata->talent_exp[15] = x16;
               ch->pcdata->talent_exp[16] = x17;
               ch->pcdata->talent_exp[17] = x18;
               ch->pcdata->talent_exp[18] = x19;
               fMatch = TRUE;
               break;
            }

            KEY( "Type", ch->pcdata->type, fread_string( fp ) );

            if( !strcmp( word, "Title" ) )
            {
               ch->pcdata->title = fread_string( fp );
               if( isalpha( ch->pcdata->title[0] ) || isdigit( ch->pcdata->title[0] ) )
               {
                  sprintf( buf, " %s", ch->pcdata->title );
                  if( ch->pcdata->title )
                     STRFREE( ch->pcdata->title );
                  ch->pcdata->title = STRALLOC( buf );
               }
               fMatch = TRUE;
               break;
            }

            break;

         case 'V':
            if( !strcmp( word, "Vnum" ) )
            {
               ch->pIndexData = get_mob_index( fread_number( fp ) );
               fMatch = TRUE;
               break;
            }
            if( !str_cmp( word, "Version" ) )
            {
               file_ver = fread_number( fp );
               ch->pcdata->version = file_ver;
               fMatch = TRUE;
               break;
            }
            break;

         case 'W':
            if( !strcmp( word, "Weapons" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = x15 = x16 = x17 = x18 = x19 = x20 =
                  1;
               sscanf( line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6,
                       &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15, &x16, &x17, &x18, &x19, &x20 );
               ch->pcdata->weapon[0] = x1;
               ch->pcdata->weapon[1] = x2;
               ch->pcdata->weapon[2] = x3;
               ch->pcdata->weapon[3] = x4;
               ch->pcdata->weapon[4] = x5;
               ch->pcdata->weapon[5] = x6;
               ch->pcdata->weapon[6] = x7;
               ch->pcdata->weapon[7] = x8;
               ch->pcdata->weapon[8] = x9;
               ch->pcdata->weapon[9] = x10;
               ch->pcdata->weapon[10] = x11;
               ch->pcdata->weapon[11] = x12;
               ch->pcdata->weapon[12] = x13;
               ch->pcdata->weapon[13] = x14;
               ch->pcdata->weapon[14] = x15;
               ch->pcdata->weapon[15] = x16;
               ch->pcdata->weapon[16] = x17;
               ch->pcdata->weapon[17] = x18;
               ch->pcdata->weapon[18] = x19;
               ch->pcdata->weapon[19] = x20;
               fMatch = TRUE;
               break;
            }
            if( !strcmp( word, "WeaponExp" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = x15 = x16 = x17 = x18 = x19 = x20 =
                  1;
               sscanf( line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6,
                       &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15, &x16, &x17, &x18, &x19, &x20 );
               ch->pcdata->weapon_exp[0] = x1;
               ch->pcdata->weapon_exp[1] = x2;
               ch->pcdata->weapon_exp[2] = x3;
               ch->pcdata->weapon_exp[3] = x4;
               ch->pcdata->weapon_exp[4] = x5;
               ch->pcdata->weapon_exp[5] = x6;
               ch->pcdata->weapon_exp[6] = x7;
               ch->pcdata->weapon_exp[7] = x8;
               ch->pcdata->weapon_exp[8] = x9;
               ch->pcdata->weapon_exp[9] = x10;
               ch->pcdata->weapon_exp[10] = x11;
               ch->pcdata->weapon_exp[11] = x12;
               ch->pcdata->weapon_exp[12] = x13;
               ch->pcdata->weapon_exp[13] = x14;
               ch->pcdata->weapon_exp[14] = x15;
               ch->pcdata->weapon_exp[15] = x16;
               ch->pcdata->weapon_exp[16] = x17;
               ch->pcdata->weapon_exp[17] = x18;
               ch->pcdata->weapon_exp[18] = x19;
               ch->pcdata->weapon_exp[19] = x20;
               fMatch = TRUE;
               break;
            }
            KEY( "Weight", ch->weight, fread_number( fp ) );
            KEY( "Wererace", ch->pcdata->were_race, fread_string( fp ) );
            KEY( "Wimpy", ch->wimpy, fread_number( fp ) );
            KEY( "WizInvis", ch->pcdata->wizinvis, fread_number( fp ) );
            break;
      }  /* switch */

/*	if ( !fMatch )
	{
	    sprintf( buf, "Fread_char: no match: %s", word );
	    bug( buf, 0 );
	} */
   }  /* for (;;) */

   if( !ch->speed )
      ch->speed = 100;
}


void fread_obj( CHAR_DATA * ch, FILE * fp, sh_int os_type )
{
   OBJ_DATA *obj;
   char *word;
   int iNest;
   bool fMatch;
   bool fNest;
   bool fVnum;
   ROOM_INDEX_DATA *room = NULL;

   if( ch )
      room = ch->in_room;
   CREATE( obj, OBJ_DATA, 1 );
   obj->count = 1;
   obj->wear_loc = -1;
   obj->weight = 1;
   obj->condition = 100;
   obj->size = 0;
   if( ch )
      obj->size = ch->height;

   fNest = TRUE;  /* Requiring a Nest 0 is a waste */
   fVnum = TRUE;
   iNest = 0;

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
            KEY( "ActionDesc", obj->action_desc, fread_string( fp ) );
            if( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;
               int pafmod;

               CREATE( paf, AFFECT_DATA, 1 );
               if( !strcmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
               else
               {
                  int sn;

                  sn = skill_lookup( fread_word( fp ) );
                  if( sn < 0 )
                     bug( "Fread_obj: unknown skill.", 0 );
                  else
                     paf->type = sn;
               }
               paf->duration = fread_number( fp );
               pafmod = fread_number( fp );
               paf->location = fread_number( fp );
               paf->bitvector = fread_bitvector( fp );
               if( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_STRIPSN
                   || paf->location == APPLY_REMOVESPELL || paf->location == APPLY_RECURRINGSPELL )
                  paf->modifier = slot_lookup( pafmod );
               else
                  paf->modifier = pafmod;

               /*
                * convert over old eq -keo 
                */
               if( paf->location == APPLY_AC )
               {
                  if( paf->modifier == -5 )
                  {
                     paf->location = APPLY_HITROLL;
                     paf->modifier = 1;
                  }
                  else if( paf->modifier < 0 )
                  {
                     paf->modifier = abs( paf->modifier / 10 );
                  }
                  else if( paf->modifier == 0 )
                  {
                     paf->modifier = 1;
                  }
               }
               LINK( paf, obj->first_affect, obj->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            break;

         case 'C':
            KEY( "Cost", obj->cost, fread_number( fp ) );
            KEY( "Count", obj->count, fread_number( fp ) );
            KEY( "Condition", obj->condition, fread_number( fp ) );
            break;

         case 'D':
            KEY( "Description", obj->description, fread_string( fp ) );
            break;

         case 'E':
            KEY( "ExtraFlags", obj->extra_flags, fread_bitvector( fp ) );

            if( !strcmp( word, "ExtraDescr" ) )
            {
               EXTRA_DESCR_DATA *ed;

               CREATE( ed, EXTRA_DESCR_DATA, 1 );
               ed->keyword = fread_string( fp );
               ed->description = fread_string( fp );
               LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
               fMatch = TRUE;
            }

            if( !strcmp( word, "End" ) )
            {
               if( !fNest || !fVnum || !obj->pIndexData )
               {
/*		    if ( obj->name )
			sprintf ( buf, "Fread_obj: %s incomplete object.",
				obj->name );
		    else
			sprintf ( buf, "Fread_obj: incomplete object." );
		    bug( buf, 0 );
*/ if( obj->name )
                     STRFREE( obj->name );
                  if( obj->description )
                     STRFREE( obj->description );
                  if( obj->short_descr )
                     STRFREE( obj->short_descr );
                  DISPOSE( obj );
                  return;
               }
               else
               {
                  sh_int wear_loc = obj->wear_loc;

                  if( !obj->name )
                     obj->name = QUICKLINK( obj->pIndexData->name );
                  if( !obj->description )
                     obj->description = QUICKLINK( obj->pIndexData->description );
                  if( !obj->short_descr )
                     obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
                  if( !obj->action_desc )
                     obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
                  LINK( obj, first_object, last_object, next, prev );
                  obj->pIndexData->count += obj->count;
                  if( !obj->serial )
                  {
                     cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
                     obj->serial = obj->pIndexData->serial = cur_obj_serial;
                  }
                  if( fNest )
                     rgObjNest[iNest] = obj;
                  numobjsloaded += obj->count;
                  ++physicalobjects;
                  if( file_ver > 1 || obj->wear_loc < -1 || obj->wear_loc >= MAX_WEAR )
                     obj->wear_loc = -1;
                  if( os_type == OS_GROUND && room )
                  {
                     obj_to_room( obj, room );
                  }
                  else if( iNest == 0 || rgObjNest[iNest] == NULL )
                  {
                     int slot = -1;
                     bool reslot = FALSE;

                     if( file_ver > 1 && wear_loc > -1 && wear_loc < MAX_WEAR )
                     {
                        int x;

                        for( x = 0; x < MAX_LAYERS; x++ )
                           if( !save_equipment[wear_loc][x] )
                           {
                              save_equipment[wear_loc][x] = obj;
                              slot = x;
                              reslot = TRUE;
                              break;
                           }
                        if( x == MAX_LAYERS )
                           bug( "Fread_obj: too many layers %d", wear_loc );
                     }
                     obj = obj_to_char( obj, ch );
                     if( reslot && slot != -1 )
                        save_equipment[wear_loc][slot] = obj;
                  }
                  else
                  {
                     if( rgObjNest[iNest - 1] )
                     {
                        separate_obj( rgObjNest[iNest - 1] );
                        if( os_type != OS_GEM )
                        {
                           obj = obj_to_obj( obj, rgObjNest[iNest - 1] );
                        }
                        else
                        {
                           rgObjNest[iNest - 1]->gem = obj;
                           obj->in_obj = rgObjNest[iNest - 1];
                        }
                     }
                     else
                        bug( "Fread_obj: nest layer missing %d", iNest - 1 );
                  }
                  if( fNest )
                     rgObjNest[iNest] = obj;
                  return;
               }
            }
            break;

         case 'I':
            KEY( "ItemType", obj->item_type, fread_number( fp ) );
            break;

         case 'M':
            KEY( "Mana", obj->mana, fread_number( fp ) );
            KEY( "Material", obj->material, material_lookup( fread_number( fp ) ) );
            break;
         case 'N':
            KEY( "Name", obj->name, fread_string( fp ) );

            if( !strcmp( word, "Nest" ) )
            {
               iNest = fread_number( fp );
               if( iNest < 0 || iNest >= MAX_NEST )
               {
                  bug( "Fread_obj: bad nest %d.", iNest );
                  iNest = 0;
                  fNest = FALSE;
               }
               fMatch = TRUE;
            }
            break;

         case 'O':
            KEY( "ObjBy", obj->obj_by, fread_string( fp ) );
            break;

         case 'P':
            KEY( "PartFlags", obj->parts, fread_bitvector( fp ) );
            break;

         case 'R':
            KEY( "RawMana", obj->raw_mana, fread_number( fp ) );
            KEY( "Room", room, get_room_index( fread_number( fp ) ) );
            break;

         case 'S':
            KEY( "ShortDescr", obj->short_descr, fread_string( fp ) );

            if( !strcmp( word, "Spell" ) )
            {
               int iValue;
               int sn;

               iValue = fread_number( fp );
               sn = skill_lookup( fread_word( fp ) );
               if( iValue < 0 || iValue > 5 )
                  bug( "Fread_obj: bad iValue %d.", iValue );
               else if( sn < 0 )
                  bug( "Fread_obj: unknown skill.", 0 );
               else
                  obj->value[iValue] = sn;
               fMatch = TRUE;
               break;
            }

            KEY( "Size", obj->size, fread_number( fp ) );
            break;

         case 'T':
            KEY( "Timer", obj->timer, fread_number( fp ) );
            break;

         case 'V':
            if( !strcmp( word, "Values" ) )
            {
               int x1, x2, x3, x4, x5, x6, x7;
               char *ln = fread_line( fp );

               x1 = x2 = x3 = x4 = x5 = x6 = 0;
               sscanf( ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               /*
                * clean up some garbage 
                */
/*		if ( file_ver < 3 )
		   x5=x6=x7=0;
*/
               obj->value[0] = x1;
               obj->value[1] = x2;
               obj->value[2] = x3;
               obj->value[3] = x4;
               obj->value[4] = x5;
               obj->value[5] = x6;
               obj->value[6] = x7;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "Vnum" ) )
            {
               int vnum;

               vnum = fread_number( fp );
               /*
                * bug( "Fread_obj: bad vnum %d.", vnum );  
                */
               if( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                  fVnum = FALSE;
               else
               {
                  fVnum = TRUE;
                  obj->cost = obj->pIndexData->cost;
                  obj->weight = obj->pIndexData->weight;
                  obj->item_type = obj->pIndexData->item_type;
                  obj->extra_flags = obj->pIndexData->extra_flags;
                  obj->parts = obj->pIndexData->parts;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'W':
            KEY( "WearLoc", obj->wear_loc, fread_number( fp ) );
            KEY( "Weight", obj->weight, fread_number( fp ) );
            break;

      }

      if( !fMatch )
      {
         EXTRA_DESCR_DATA *ed;
         AFFECT_DATA *paf;

         bug( "Fread_obj: no match.", 0 );
         bug( word, 0 );
         fread_to_eol( fp );
         if( obj->name )
            STRFREE( obj->name );
         if( obj->description )
            STRFREE( obj->description );
         if( obj->short_descr )
            STRFREE( obj->short_descr );
         while( ( ed = obj->first_extradesc ) != NULL )
         {
            STRFREE( ed->keyword );
            STRFREE( ed->description );
            UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
            DISPOSE( ed );
         }
         while( ( paf = obj->first_affect ) != NULL )
         {
            UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
            DISPOSE( paf );
         }
         DISPOSE( obj );
         return;
      }
   }
}

void fread_alias( CHAR_DATA * ch, FILE * fp )
{
   ALIAS_DATA *alias;
   char *word;
   bool fMatch;

   CREATE( alias, ALIAS_DATA, 1 );

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
            KEY( "Commands", alias->alias, fread_string( fp ) );
            break;

         case 'E':
            if( !strcmp( word, "End" ) )
            {
               if( !alias->name || !alias->alias )
               {
                  bug( "fread_alias: %s%s%s", alias->name ? "" : "NULL alias->name",
                       ( !alias->name && !alias->alias ) ? " and " : "NULL alias->alias" );
                  DISPOSE( alias );
                  return;
               }
               else
               {
                  /*
                   * bug( "Debug: fread_alias: %s    %s", alias->name, alias->alias ); 
                   */
                  LINK( alias, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
                  return;
               }
            }
            break;

         case 'N':
            KEY( "Name", alias->name, fread_string( fp ) );
            break;

      }

      if( !fMatch )
      {
         bug( "Fread_obj: no match." );
         bug( word );
         fread_to_eol( fp );
         DISPOSE( alias );
         return;
      }
   }
}

void set_alarm( long seconds )
{
#ifdef WIN32
   kill_timer(  );   /* kill old timer */
   timer_code = timeSetEvent( seconds * 1000L, 1000, alarm_handler, 0, TIME_PERIODIC );
#else
   alarm( seconds );
#endif
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char name[MAX_INPUT_LENGTH];
   struct stat fst;

   one_argument( argument, arg );
   if( arg[0] == '\0' )
   {
      send_to_char( "Usage: last <playername>\n\r", ch );
      return;
   }
   strcpy( name, capitalize( arg ) );
   sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
   if( stat( buf, &fst ) != -1 )
      sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
   else
      sprintf( buf, "%s was not found.\n\r", name );
   send_to_char( buf, ch );
}

/*
 * Added support for removeing so we could take out the write_corpses
 * so we could take it out of the save_char_obj function. --Shaddai
 */

void write_corpses( CHAR_DATA * ch, char *name, OBJ_DATA * objrem )
{
   FILE *fp = NULL;

   /*
    * Name and ch support so that we dont have to have a char to save their
    * corpses.. (ie: decayed corpses while offline) 
    */
   if( ch && IS_NPC( ch ) )
   {
      bug( "Write_corpses: writing NPC corpse.", 0 );
      return;
   }
   if( ch )
      name = ch->name;
   /*
    * Go by vnum, less chance of screwups. -- Altrag 
    */
/*  for ( corpse = first_object; corpse; corpse = corpse->next )
    if ( corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
         corpse->in_room != NULL &&
        !str_cmp(corpse->short_descr+14, name) &&
	objrem != corpse )
    {
      if ( !fp )
      {
        char buf[127];
        
        sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
        if ( !(fp = fopen(buf, "w")) )
        {
          bug( "Write_corpses: Cannot open file.", 0 );
          perror(buf);
          return;
        }
      }
    }
*/
   if( fp )
   {
      fprintf( fp, "#END\n\n" );
      fclose( fp );
   }
   else
   {
      char buf[127];

      sprintf( buf, "%s%s", CORPSE_DIR, capitalize( name ) );
      remove( buf );
   }
   return;
}

void load_corpses( void )
{
}

/*
 * This will write one mobile structure pointed to be fp --Shaddai
 */

void fwrite_mobile( FILE * fp, CHAR_DATA * mob )
{
   if( !IS_NPC( mob ) || !fp )
      return;
   fprintf( fp, "#MOBILE\n" );
   fprintf( fp, "Vnum	%d\n", mob->pIndexData->vnum );
   if( mob->in_room )
   {
      if( xIS_SET( mob->act, ACT_SENTINEL ) )
      {
         /*
          * Sentinel mobs get stamped with a "home room" when they are created
          * by create_mobile(), so we need to save them in their home room regardless
          * of where they are right now, so they will go to their home room when they
          * enter the game from a reboot or copyover -- Scion 
          */
         fprintf( fp, "Room	%d\n", mob->home_room );
      }
      else
      {
         fprintf( fp, "Room	%d\n",
                  ( mob->in_room == get_room_index( ROOM_VNUM_LIMBO )
                    && mob->was_in_room ) ? mob->was_in_room->vnum : mob->in_room->vnum );
      }
   }
   if( QUICKMATCH( mob->name, mob->pIndexData->player_name ) == 0 )
      fprintf( fp, "Name     %s~\n", mob->name );
   if( QUICKMATCH( mob->short_descr, mob->pIndexData->short_descr ) == 0 )
      fprintf( fp, "Short	%s~\n", mob->short_descr );
   if( QUICKMATCH( mob->description, mob->pIndexData->description ) == 0 )
      fprintf( fp, "Description %s~\n", mob->description );
   fprintf( fp, "Gold %ld\n", mob->gold );
   fprintf( fp, "Flags %s\n", print_bitvector( &mob->act ) );
   /*
    * This is needed for generic mobs 
    */
   fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
            mob->perm_str, mob->perm_int, mob->perm_wis, mob->perm_dex, mob->perm_con, mob->perm_cha, mob->perm_lck );
   fprintf( fp, "Part	%d\n", mob->xflags );
   if( mob->first_carrying )
      fwrite_obj( mob, mob->last_carrying, fp, 0, OS_CARRY );
   fprintf( fp, "EndMobile\n\n" );
   return;
}

/*
 * This will read one mobile structure pointer to by fp --Shaddai
 */
CHAR_DATA *fread_mobile( FILE * fp )
{
   CHAR_DATA *mob = NULL;
   char *word;
   bool fMatch;
   int inroom = 0;
   char *line;
   ROOM_INDEX_DATA *pRoomIndex = NULL;
   int x1, x2, x3, x4, x5, x6, x7;

   word = feof( fp ) ? "EndMobile" : fread_word( fp );
   if( !strcmp( word, "Vnum" ) )
   {
      int vnum;

      vnum = fread_number( fp );
      mob = create_mobile( get_mob_index( vnum ) );
      if( !mob )
      {
         for( ;; )
         {
            word = feof( fp ) ? "EndMobile" : fread_word( fp );
            /*
             * So we don't get so many bug messages when something messes up
             * * --Shaddai 
             */
            if( !strcmp( word, "EndMobile" ) )
               break;
         }
         bug( "Fread_mobile: No index data for vnum %d", vnum );
         return NULL;
      }
   }
   else
   {
      for( ;; )
      {
         word = feof( fp ) ? "EndMobile" : fread_word( fp );
         /*
          * So we don't get so many bug messages when something messes up
          * * --Shaddai 
          */
         if( !strcmp( word, "EndMobile" ) )
            break;
      }
      extract_char( mob, TRUE );
      bug( "Fread_mobile: Vnum not found", 0 );
      return NULL;
   }
   for( ;; )
   {
      word = feof( fp ) ? "EndMobile" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case '#':
            if( !strcmp( word, "#OBJECT" ) )
               fread_obj( mob, fp, OS_CARRY );
            else if( !str_cmp( word, "#GEM" ) )
               fread_obj( mob, fp, OS_GEM );
         case 'A':
            if( !strcmp( word, "AttrPerm" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
               sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               mob->perm_str = x1;
               mob->perm_int = x2;
               mob->perm_wis = x3;
               mob->perm_dex = x4;
               mob->perm_con = x5;
               mob->perm_cha = x6;
               mob->perm_lck = x7;
               fMatch = TRUE;
               break;
            }
            break;
         case 'D':
            KEY( "Description", mob->description, fread_string( fp ) );
            break;
         case 'E':
            if( !strcmp( word, "EndMobile" ) )
            {
               if( inroom == 0 )
                  inroom = ROOM_VNUM_NEXUS;
               pRoomIndex = get_room_index( inroom );
               if( !pRoomIndex )
                  pRoomIndex = get_room_index( ROOM_VNUM_NEXUS );
               char_to_room( mob, pRoomIndex );
               return mob;
            }
            if( !strcmp( word, "End" ) )  /* End of object, need to ignore this.
                                           * sometimes they creep in there somehow -- Scion */
               fMatch = TRUE; /* Trick the system into thinking it matched something */
            break;
         case 'F':
            KEY( "Flags", mob->act, fread_bitvector( fp ) );
            break;
         case 'G':
            KEY( "Gold", mob->gold, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", mob->name, fread_string( fp ) );
            break;
         case 'P':
            KEY( "Position", mob->position, fread_number( fp ) );
            KEY( "Part", mob->xflags, fread_number( fp ) );
            break;
         case 'R':
            KEY( "Room", inroom, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Short", mob->short_descr, fread_string( fp ) );
            break;
      }
      if( !fMatch && strcmp( word, "End" ) )
      {
         bug( "Fread_mobile: no match.", 0 );
         bug( word, 0 );
      }
   }
   return NULL;
}

/*
 * This will write in the saved mobile for a char --Shaddai
 */
void write_char_mobile( CHAR_DATA * ch, char *argument )
{
   FILE *fp;
   CHAR_DATA *mob;
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->pcdata->pet )
      return;

   fclose( fpReserve );
   if( ( fp = fopen( argument, "w" ) ) == NULL )
   {
      sprintf( buf, "Write_char_mobile: couldn't open %s for writing!\n\r", argument );
      bug( buf, 0 );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }
   mob = ch->pcdata->pet;
   xSET_BIT( mob->affected_by, AFF_CHARM );
   fwrite_mobile( fp, mob );
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}

/*
 * This will read in the saved mobile for a char --Shaddai
 */

void read_char_mobile( char *argument )
{
   FILE *fp;
   CHAR_DATA *mob;
   char buf[MAX_STRING_LENGTH];

   fclose( fpReserve );
   if( ( fp = fopen( argument, "r" ) ) == NULL )
   {
      sprintf( buf, "Read_char_mobile: couldn't open %s for reading!\n\r", argument );
      bug( buf, 0 );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }
   mob = fread_mobile( fp );
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}

/* Return the vnum the obj is in, regardless of being carried or inside containers. -- Scion */
int get_obj_room_vnum_recursive( OBJ_DATA * obj )
{
   if( !obj )
      return ROOM_VNUM_LIMBO;

   if( obj->in_obj )
   {
      return get_obj_room_vnum_recursive( obj->in_obj );
   }
   else if( obj->carried_by )
   {
      if( obj->carried_by->in_room )
      {
         return obj->carried_by->in_room->vnum;
      }
      else
      {
         bug( "get_obj_room_vnum_recursive: Char \"%s\" not in room?!", obj->carried_by->name );
         return ROOM_VNUM_LIMBO;
      }
   }
   else if( obj->in_room )
   {
      return obj->in_room->vnum;
   }
   else
   {
      bug( "get_obj_room_vnum_recursive: obj isn't carried, inside containers, or inside a room!", 0 );
      bug( "putting in limbo...", 0 );
      obj_to_room( obj, get_room_index( ROOM_VNUM_LIMBO ) );
      return ROOM_VNUM_LIMBO;
   }
}


/*
 * Save the world's objects and mobs in their current positions -- Scion
 */
void save_world( CHAR_DATA * ch )
{
   FILE *mobfp;
   FILE *objfp;
   int mobfile = 0;
   int objfile = 0;
   char filename[256];
   CHAR_DATA *rch;
   OBJ_DATA *obj;

   sprintf( filename, "%s%s", SYSTEM_DIR, MOB_FILE );
   if( ( mobfp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_world: fopen mob file", 0 );
      perror( filename );
   }
   else
      mobfile++;

   sprintf( filename, "%s%s", SYSTEM_DIR, OBJ_FILE );
   if( ( objfp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "save_world: fopen obj file", 0 );
      perror( filename );
   }
   else
      objfile++;

   if( objfile )
   {
      for( obj = first_object; obj; obj = obj->next )
      {
         /*
          * Objects that are in inventories get written in fwrite_mob, or to a player's pfile 
          */
         if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) || ( !obj->in_room ) /* Don't keep objs that aren't in rooms */
             || ( obj->carried_by ) /* Carried objs save on the char that's carrying them */
             || ( obj->in_obj )  /* Don't keep objs in other objs.. they save when their container saves */
             || IS_SET( obj->in_room->room_flags, ROOM_DONATION ) )
            continue;
         fwrite_obj( NULL, obj, objfp, 0, OS_GROUND );
      }
      fprintf( objfp, "#END" );
      fclose( objfp );
   }

   if( mobfile )
   {
      for( rch = first_char; rch; rch = rch->next )
      {
         if( !IS_NPC( rch ) || rch == supermob || xIS_SET( rch->act, ACT_PROTOTYPE ) || xIS_SET( rch->act, ACT_PET ) )
            continue;
         else
            fwrite_mobile( mobfp, rch );
      }
      fprintf( mobfp, "#END" );
      fclose( mobfp );
   }
}

void load_world( CHAR_DATA * ch )
{
   FILE *mobfp;
   FILE *objfp;
   char filename[256];
   char *word;
   int done = 0;
   int mobfile = 0;
   int objfile = 0;

   sprintf( filename, "%s%s", SYSTEM_DIR, MOB_FILE );
   if( ( mobfp = fopen( filename, "r" ) ) == NULL )
   {
      bug( "load_world: fopen mob file", 0 );
      perror( filename );
      return;
   }
   else
      mobfile++;

   sprintf( filename, "%s%s", SYSTEM_DIR, OBJ_FILE );
   if( ( objfp = fopen( filename, "r" ) ) == NULL )
   {
      bug( "load_world: fopen obj file", 0 );
      perror( filename );
      return;
   }
   else
      objfile++;

   if( mobfile )
   {
      while( done == 0 )
      {
         if( feof( mobfp ) )
            done++;
         else
         {
            word = fread_word( mobfp );
            if( str_cmp( word, "#END" ) )
               fread_mobile( mobfp );
            else
               done++;
         }

      }
      fclose( mobfp );
   }

   done = 0;

   if( objfile )
   {
      while( done == 0 )
      {
         if( feof( objfp ) )
            done++;
         else
         {
            word = fread_word( objfp );
            if( str_cmp( word, "#END" ) )
               fread_obj( NULL, objfp, OS_GROUND );
            else
               done++;
         }

      }
      fclose( objfp );
   }
}
