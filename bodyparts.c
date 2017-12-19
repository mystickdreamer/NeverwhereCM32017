/*
 *      Bodyparts
 *      =======
 *      Abilities and handling
 *      Written by Keolah for Rogue Winds 2.0
 *
 *      You may not use or distribute any of this code without the
 *      explicit permission of the code maintainer, Heather Dunn (Keolah)
 */  
   
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
#include <strings.h>
#endif /*  */
#include <time.h>
#include "mud.h"
   
/* from handler.c */ 
extern NATION_DATA *find_nation( char *name );
PART_DATA * find_bodypart( CHAR_DATA * ch, int loc )
{
   PART_DATA * part;
   part = ch->first_part;
   while( part )
   {
      if( part->loc == loc )
         return part;
      part = part->next;
   }
   return NULL;
}
int find_part_name( char *argument )
{
   int loc;
   for( loc = 0; loc <= BP_CLAWS; loc++ )
   {
      if( !str_cmp( argument, part_locs[loc] )  ||is_name( argument, part_locs[loc] ) )
         return loc;
   }
   return -1;
}

bool has_bodypart( CHAR_DATA * ch, int loc )
{
   if( find_bodypart( ch, loc ) )
      return TRUE;
   return FALSE;
}

bool can_use_bodypart( CHAR_DATA * ch, int loc )
{
   PART_DATA * part;
   part = find_bodypart( ch, loc );
   if( !part )
      return FALSE;
   if( part->flags != PART_WELL )
      return FALSE;
   return TRUE;
}
void add_part( CHAR_DATA * ch, int loc, int to_loc )
{
   PART_DATA * new_part;
   PART_DATA * part;
   if( ( part = find_bodypart( ch, loc ) ) != NULL )
   {
      if( ch->nation )
      {
         part->armor = ch->nation->ac_plus;
      }
      else
      {
         part->armor = 0;
      }
      return;
   }
   CREATE( new_part, PART_DATA, 1 );
   if( !ch->first_part )
   {
      ch->first_part = new_part;
      ch->last_part = new_part;
   }
   else
   {
      ch->last_part->next = new_part;
      new_part->prev = ch->last_part;
      ch->last_part = new_part;
   }
   new_part->loc = loc;
   new_part->cond = PART_WELL;
   new_part->flags = PART_WELL;
   new_part->obj = NULL;
   new_part->obj2 = NULL;
   if( ch->nation )
      new_part->armor = ch->nation->ac_plus;
   
   else if( IS_NPC( ch ) )
      new_part->armor = ch->pIndexData->ac;
   
   else
      new_part->armor = 0;
   if( ( part = find_bodypart( ch, to_loc ) ) == NULL )
      return;
   new_part->connect_to = part;
}
void remove_part( CHAR_DATA * ch, int loc )
{
   PART_DATA * part;
   part = find_bodypart( ch, loc );
   if( !part )
      return;
   if( part->obj )
      unequip_char( ch, part->obj );
   if( part->obj2 )
      unequip_char( ch, part->obj2 );
   UNLINK( part, ch->first_part, ch->last_part, next, prev );
   DISPOSE( part );
}
void check_bodyparts( CHAR_DATA * ch )
{
   int i = 0;
   if( !IS_NPC( ch ) )
   {
      if( !ch->species )
         return;
      if( !ch->nation )
         ch->nation = find_nation( ch->species );
      if( !ch->nation )
      {
         bug( "check_bodyparts: could not find nation!" );
         bug( "setting char to first_nation..." );
         ch->nation = first_nation;
         set_char_color( AT_DANGER, ch );
         send_to_char( "Your species has been reset to avoid a possible crash.\n\r", ch );
         send_to_char( "Please ask an immortal to fix your species as soon as possible.\n\r", ch );
      }
      ch->xflags = ch->nation->parts;
      if( IS_VAMPIRE( ch ) )
         SET_BIT( ch->xflags, 1 << PART_FANGS );
      if( IS_AFFECTED( ch, AFF_ANTHRO ) )
      {
         SET_BIT( ch->xflags, 1 << PART_HANDS );
         SET_BIT( ch->xflags, 1 << PART_ARMS );
         SET_BIT( ch->xflags, 1 << PART_LEGS );
         REMOVE_BIT( ch->xflags, 1 << PART_FORELEGS );
         REMOVE_BIT( ch->xflags, 1 << PART_HAUNCH );
      }
   }
   for( i = 0; i < MAX_PARTS; i++ )
   {
      if( ch == supermob )
         return;
      
         /*
          * give them parts they should have 
          */ 
         if( IS_SET( ch->xflags, 1 << i ) )
      {
         switch ( i )
         {
            case PART_HEAD:
               add_part( ch, BP_HEAD, BP_NECK );
               add_part( ch, BP_FACE, BP_HEAD );
               add_part( ch, BP_NOSE, BP_HEAD );
               add_part( ch, BP_TONGUE, BP_HEAD );
               break;
            case PART_BRAINS:
               add_part( ch, BP_NECK, BP_CHEST );
               break;
            case PART_HEART:
               add_part( ch, BP_CHEST, -1 );
               add_part( ch, BP_BACK, -1 );
               break;
            case PART_GUTS:
               add_part( ch, BP_STOMACH, -1 );
               add_part( ch, BP_WAIST, -1 );
               break;
            case PART_ARMS:
               add_part( ch, BP_RARM, BP_CHEST );
               add_part( ch, BP_LARM, BP_CHEST );
               add_part( ch, BP_RWRIST, BP_RARM );
               add_part( ch, BP_LWRIST, BP_LARM );
               break;
            case PART_HANDS:
               add_part( ch, BP_RHAND, BP_RWRIST );
               add_part( ch, BP_LHAND, BP_LWRIST );
               break;
            case PART_LEGS:
               add_part( ch, BP_RLEG, BP_WAIST );
               add_part( ch, BP_LLEG, BP_WAIST );
               add_part( ch, BP_RANKLE, BP_RLEG );
               add_part( ch, BP_LANKLE, BP_LLEG );
               break;
            case PART_WINGS:
               add_part( ch, BP_RWING, BP_BACK );
               add_part( ch, BP_LWING, BP_BACK );
               break;
            case PART_FEET:
               add_part( ch, BP_RFOOT, BP_RANKLE );
               add_part( ch, BP_LFOOT, BP_LANKLE );
               break;
            case PART_PAWS:
               add_part( ch, BP_RPAW, BP_RANKLE );
               add_part( ch, BP_LPAW, BP_LANKLE );
               break;
            case PART_HOOVES:
               add_part( ch, BP_RHOOF, BP_RANKLE );
               add_part( ch, BP_LHOOF, BP_LANKLE );
               break;
            case PART_FANGS:
               add_part( ch, BP_FANGS, BP_HEAD );
               break;
            case PART_TAIL:
               add_part( ch, BP_TAIL, BP_WAIST );
               break;
            case PART_HORNS:
               add_part( ch, BP_RHORN, BP_HEAD );
               add_part( ch, BP_LHORN, BP_HEAD );
               break;
            case PART_FORELEGS:
               add_part( ch, BP_RFLEG, BP_CHEST );
               add_part( ch, BP_LFLEG, BP_CHEST );
               break;
            case PART_HAUNCH:
               add_part( ch, BP_RRLEG, BP_WAIST );
               add_part( ch, BP_LRLEG, BP_WAIST );
               break;
            case PART_EYE:
               add_part( ch, BP_REYE, BP_HEAD );
               add_part( ch, BP_LEYE, BP_HEAD );
               break;
            case PART_FINS:
               add_part( ch, BP_RFIN, BP_CHEST );
               add_part( ch, BP_LFIN, BP_CHEST );
               add_part( ch, BP_DFIN, BP_BACK );
               break;
            case PART_CLAWS:
               add_part( ch, BP_CLAWS, BP_RPAW );
               break;
            case PART_EAR:
               add_part( ch, BP_REAR, BP_HEAD );
               add_part( ch, BP_LEAR, BP_HEAD );
               break;
         } /* switch */
      }
      else
      {  /* otherwise remove parts they shouldnt have */
         switch ( i )
         {
            case PART_HEAD:
               remove_part( ch, BP_HEAD );
               remove_part( ch, BP_FACE );
               remove_part( ch, BP_NOSE );
               remove_part( ch, BP_TONGUE );
               break;
            case PART_BRAINS:
               remove_part( ch, BP_NECK );
               break;
            case PART_HEART:
               remove_part( ch, BP_CHEST );
               remove_part( ch, BP_BACK );
               break;
            case PART_GUTS:
               remove_part( ch, BP_STOMACH );
               remove_part( ch, BP_WAIST );
               break;
            case PART_ARMS:
               remove_part( ch, BP_RARM );
               remove_part( ch, BP_LARM );
               remove_part( ch, BP_RWRIST );
               remove_part( ch, BP_LWRIST );
               break;
            case PART_HANDS:
               remove_part( ch, BP_RHAND );
               remove_part( ch, BP_LHAND );
               break;
            case PART_LEGS:
               remove_part( ch, BP_RLEG );
               remove_part( ch, BP_LLEG );
               remove_part( ch, BP_RANKLE );
               remove_part( ch, BP_LANKLE );
               break;
            case PART_WINGS:
               remove_part( ch, BP_RWING );
               remove_part( ch, BP_LWING );
               break;
            case PART_FEET:
               remove_part( ch, BP_RFOOT );
               remove_part( ch, BP_LFOOT );
               break;
            case PART_PAWS:
               remove_part( ch, BP_RPAW );
               remove_part( ch, BP_LPAW );
               break;
            case PART_HOOVES:
               remove_part( ch, BP_RHOOF );
               remove_part( ch, BP_LHOOF );
               break;
            case PART_FANGS:
               remove_part( ch, BP_FANGS );
               break;
            case PART_TAIL:
               remove_part( ch, BP_TAIL );
               break;
            case PART_HORNS:
               remove_part( ch, BP_RHORN );
               remove_part( ch, BP_LHORN );
               break;
            case PART_FORELEGS:
               remove_part( ch, BP_RFLEG );
               remove_part( ch, BP_LFLEG );
               break;
            case PART_HAUNCH:
               remove_part( ch, BP_RRLEG );
               remove_part( ch, BP_LRLEG );
               break;
            case PART_EYE:
               remove_part( ch, BP_REYE );
               remove_part( ch, BP_LEYE );
               break;
            case PART_FINS:
               remove_part( ch, BP_RFIN );
               remove_part( ch, BP_LFIN );
               remove_part( ch, BP_DFIN );
               break;
            case PART_CLAWS:
               remove_part( ch, BP_CLAWS );
               break;
            case PART_EAR:
               remove_part( ch, BP_REAR );
               remove_part( ch, BP_LEAR );
               break;
         } /* switch */
      } /* if */
   } /* for */
   
      /*
       * comment out if you dont want anatomically correct chars 
       */ 
/*      if( ch->sex == SEX_MALE || ch->sex == SEX_HERMAPH )
      add_part( ch, BP_PENIS, BP_WAIST );
   
   else
      remove_part( ch, BP_PENIS );
   if( ch->sex == SEX_FEMALE || ch->sex == SEX_HERMAPH )
      add_part( ch, BP_VAGINA, BP_WAIST );
   
   else
      remove_part( ch, BP_VAGINA ); */
}


/* Returns the object on the top layer of the part */ 
OBJ_DATA *outer_layer( PART_DATA *part )
{
   if( part->obj2 )
      return part->obj2;

   return part->obj;
}


/* For fancy messages for magic */ 
char *hands( CHAR_DATA * ch )
{
   if( IS_SET( ch->xflags, 1 << PART_HANDS ) )
      return "hands";
   if( IS_AFFECTED( ch, AFF_COLDBLOOD ) )
      return "mouth";
   return "eyes";
}
void do_body( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' )
      victim = ch;
   else if( IS_SET( ch->pcdata->permissions, PERMIT_MISC ) )
   {
      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }
   }
   else
   {
      if( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }
      else
      {
         act( AT_PLAIN, "$n looks you over.", ch, NULL, victim, TO_VICT );
         act( AT_PLAIN, "$n looks $N over.", ch, NULL, victim, TO_NOTVICT );
      }
   }

   if( victim == ch )
      send_to_char( "Your body:\r\n", ch );
   else
      ch_printf( ch, "%s's body:\r\n", PERS( victim, ch ) );

   if( victim->first_part )
   {
      PART_DATA * part;

      part = victim->first_part;
      while( part )
      {
         pager_printf_color( ch, "&W%16s: &Y%dac &C%dhp", part_locs[part->loc], part->armor, part->cond );

         switch ( part->flags )
         {
            case PART_BROKEN:
               pager_printf_color( ch, " &Obroken&w " );
               break;

            case PART_SEVERED:
               pager_printf_color( ch, " &rsevered&w " );
               break;

            case PART_BOUND:
               pager_printf_color( ch, " &Cbound&w " );
               break;

            case PART_WELL:
               switch ( part->cond )
               {
                  case PART_WELL:
                     pager_printf_color( ch, " &Gfine " );
                     break;

                  default:
                     pager_printf_color( ch, " &Yinjured " );
               }
               break;
         }

         /* See if any object is attached that shouldn't be */ 
/*
	 if (part->obj && part->obj->carried_by != ch)
         {
            part->obj = part->obj2;
            part->obj2 = NULL;
         }
         if (part->obj2 && part->obj2->carried_by != ch)
            part->obj2 = NULL;
*/ 

         if( ( obj = outer_layer( part ) ) != NULL )
            pager_printf( ch, obj->short_descr ? obj->short_descr : "???" );
         pager_printf( ch, "\n\r" );

         part = part->next;
      }
   }
}
void show_equip( CHAR_DATA * ch, CHAR_DATA * to )
{
   OBJ_DATA * obj;
   PART_DATA * part;
   bool found = FALSE;

   /* first tell them what they're wielding, if anything */ 
   if( ( obj = ch->main_hand ) )
   {
      ch_printf( to, "     %s, held in right hand\n\r", format_obj_to_char( obj, ch, TRUE ) );
      found = TRUE;
   }

   if( ( obj = ch->off_hand ) )
   {
      ch_printf( to, "     %s, held in left hand\n\r", format_obj_to_char( obj, ch, TRUE ) );
      found = TRUE;
   }

   /* now run through their parts and find any armor */ 
   if( ch == to )
   {
      for( part = ch->first_part; part; part = part->next )
      {
         if( part->obj && can_see_obj( to, part->obj ) )
         {
            ch_printf( to, "     %s, worn on %s\n\r", format_obj_to_char( part->obj, ch, TRUE ), part_locs[part->loc] );
            found = TRUE;
         }

         if( part->obj2 && can_see_obj( to, part->obj2 ) )
         {
            ch_printf( to, "     %s, worn on %s\n\r", format_obj_to_char( part->obj2, ch, TRUE ), part_locs[part->loc] );
            found = TRUE;
         }
      }
   }
   else
   {
      for( part = ch->first_part; part; part = part->next )
      {
         if( ( obj = outer_layer( part ) ) != NULL )
         {
            ch_printf( to, "     %s, worn on %s\n\r", format_obj_to_char( obj, ch, TRUE ), part_locs[part->loc] );
            found = TRUE;
         }
      }
   }

   if( !found )
      send_to_char( "     &GNothing.&w\n\r", to );
}

/* Display wielded items and worn armor */ 
void do_equipment( CHAR_DATA * ch, char *argument )
{
   send_to_char( "You are using:\n\r", ch );
   show_equip( ch, ch );
}

void obj_affect_ch( CHAR_DATA * ch, OBJ_DATA * obj )
{
   AFFECT_DATA * paf;
   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      affect_modify( ch, paf, TRUE );
   for( paf = obj->first_affect; paf; paf = paf->next )
      affect_modify( ch, paf, TRUE );
   if( obj->gem )
   {
      for( paf = obj->gem->first_affect; paf; paf = paf->next )
         affect_modify( ch, paf, TRUE );
   }
}


/* Equip a char with an object */ 
   bool equip_char( CHAR_DATA * ch, OBJ_DATA * obj )
{
   PART_DATA * part;
   bool found = FALSE;
   if( obj->size > ch->height * 1.5 )
      return FALSE;
   if( obj->size < ch->height / 2 )
      return FALSE;
   for( part = ch->first_part; part; part = part->next )
   {
      if( xIS_SET( obj->parts, part->loc )  &&part->flags == PART_WELL )
      {
         if( part->obj )
         {
            if( part->obj2 )
               continue;
            if( part->obj->pIndexData->layers < obj->pIndexData->layers )
            {
               part->obj2 = obj;
            }
            else
            {
               continue;
            }
         }
         else
         {
            part->obj = obj;
         }
         if( obj->item_type == ITEM_ARMOR )
            part->armor += obj->value[0];
         obj->wear_loc = part->loc;
         found = TRUE;
      }
   }
   if( found )
   {
      ch->carry_number -= get_obj_number( obj );
      ch->encumberance += obj->weight;
      obj_affect_ch( ch, obj );
      return TRUE;
   }
   return FALSE;
}
void obj_unaffect_ch( CHAR_DATA * ch, OBJ_DATA * obj )
{
   AFFECT_DATA * paf;
   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      affect_modify( ch, paf, FALSE );
   for( paf = obj->first_affect; paf; paf = paf->next )
      affect_modify( ch, paf, FALSE );
   if( obj->gem )
   {
      for( paf = obj->gem->first_affect; paf; paf = paf->next )
         affect_modify( ch, paf, FALSE );
   }
}


/* Unequip a char with an object */ 
void unequip_char( CHAR_DATA * ch, OBJ_DATA * obj )
{
   PART_DATA * part;
   if( !ch || !ch->name )
      return;
   for( part = ch->first_part; part; part = part->next )
   {
      if( part->obj == obj )
      {
         if( obj->item_type == ITEM_ARMOR )
            part->armor -= obj->value[0];
         part->obj = NULL;
         if( part->obj2 )
         {
            part->obj = part->obj2;
            part->obj2 = NULL;
         }
      }
      if( part->obj2 == obj )
      {
         if( obj->item_type == ITEM_ARMOR )
            part->armor -= obj->value[0];
         part->obj2 = NULL;
      }
   }
   obj->wear_loc = -1;
   ch->carry_number += get_obj_number( obj );
   ch->encumberance -= obj->weight;
   obj_unaffect_ch( ch, obj );
   update_aris( ch );
}
void do_wear( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA * obj;
   bool found;
   found = FALSE;
   if( !str_cmp( "all", argument ) )
   {
      for( obj = ch->first_carrying; obj; obj = obj->next_content )
      {
         if( obj->wear_loc == -1 )
            if( equip_char( ch, obj ) )
               found = TRUE;
      }
      if( found )
      {
         act( AT_ACTION, "You get dressed.", ch, obj, NULL, TO_CHAR );
         act( AT_ACTION, "$n gets dressed.", ch, obj, NULL, TO_ROOM );
      }
      return;
   }
   if( ( obj = get_obj_carry( ch, argument ) ) == NULL )
   {
      send_to_char( "You have nothing like that.\n\r", ch );
      return;
   }
   if( obj->wear_loc != -1 )
   {
      send_to_char( "You are already wearing that!\n\r", ch );
      return;
   }
   separate_obj( obj );
   obj_from_char( obj );  /* To make sure layering is */
   obj_to_char( obj, ch );   /* in the correct order */
   if( equip_char( ch, obj ) == FALSE )
   {
	   act( AT_ACTION, "The mob can't wear that. $T", TO_ROOM );
      send_to_char( "You can't wear that.\n\r", ch );
      return;
   }
   if( !obj->action_desc || obj->action_desc[0] == '\0' )
   {
      act( AT_ACTION, "You wear $p on your $T.", ch, obj, part_locs[obj->wear_loc], TO_CHAR );
      act( AT_ACTION, "$n wears $p on $s $T.", ch, obj, part_locs[obj->wear_loc], TO_ROOM );
   }
   else
   {
      actiondesc( ch, obj, NULL );
   }
}
void do_remove( CHAR_DATA * ch, char *argument )
{
   OBJ_DATA * obj;
   if( !str_cmp( argument, "all" ) )
   {
      for( obj = ch->first_carrying; obj; obj = obj->next_content )
      {
         if( obj->wear_loc != -1 )
            unequip_char( ch, obj );
      }
      act( AT_ACTION, "You strip naked.", ch, obj, NULL, TO_CHAR );
      act( AT_ACTION, "$n strips naked.", ch, obj, NULL, TO_ROOM );
      return;
   }
   if( ( obj = get_obj_wear( ch, argument ) ) == NULL )
   {
      send_to_char( "You have nothing like that.\n\r", ch );
      return;
   }
   if( obj->wear_loc == -1 )
   {
      send_to_char( "You aren't wearing that.\n\r", ch );
      return;
   }
   unequip_char( ch, obj );
   act( AT_ACTION, "You take off $p.", ch, obj, NULL, TO_CHAR );
   act( AT_ACTION, "$n takes off $p.", ch, obj, NULL, TO_ROOM );
}

bool sever_part( CHAR_DATA * ch, PART_DATA * part )
{
   OBJ_DATA * obj;
   char buf[MAX_STRING_LENGTH];
   PART_DATA * connected_part;
   bool was_neck;
   if( part->flags == PART_SEVERED )
      return FALSE;
   
      /*
       * parts that can't be severed 
       */ 
      if( part->loc == BP_CHEST 
          ||part->loc == BP_WAIST 
          ||part->loc == BP_VAGINA  ||part->loc == BP_STOMACH  ||part->loc == BP_BACK  ||part->loc == BP_FACE )
   {
      return FALSE;
   }
   
      /*
       * cutting off the neck will remove the head
       * * if you make a hit hard enough to sever the neck
       * * survivable, this will need to be changed
       * * better, get in that 'attached' code.
       */ 
      if( part->loc == BP_NECK )
   {
      was_neck = TRUE;
      part->loc = BP_HEAD;
   }
   act( AT_ACTION, "$n's $t is severed!", ch, part_locs[part->loc], NULL, TO_ROOM );
   act( AT_DANGER, "Your severed $t flies off!", ch, part_locs[part->loc], NULL, TO_CHAR );
   part->flags = PART_SEVERED;
   part->cond /= 2; /* there's less of it to bleed */
   if( part->obj )
      unequip_char( ch, part->obj );
   if( part->obj2 )
      unequip_char( ch, part->obj2 );
   
      /*
       * make a severed limb 
       */ 
      if( !IS_AFFECTED( ch, AFF_NO_CORPSE ) )
   {
      obj = create_object( get_obj_index( OBJ_VNUM_SEVERED_PART ), 0 );
      STRFREE( obj->short_descr );
      STRFREE( obj->name );
      sprintf( buf, "%s of %s", part_locs[part->loc], IS_NPC( ch ) ? ch->short_descr : ch->name );
      obj->short_descr = STRALLOC( buf );
      obj->name = STRALLOC( part_locs[part->loc] );
      obj->timer = 5;
      obj->value[1] = part->loc;
      if( IS_AFFECTED( ch, AFF_CONSTRUCT ) )
      {
         obj->item_type = ITEM_WEAPON;
         obj->value[5] = SK_MACE;
         obj->value[2] = 20;
         obj->value[3] = 7;
         obj->weight = 15;
         obj->value[0] = 0;
      } /* end construct check */
      obj_to_room( obj, ch->in_room );
   } /* end ethereal check */
   
      /*
       * find anything that was attached to it and remove it too 
       */ 
      connected_part = ch->first_part;
   while( connected_part )
   {
      if( connected_part->connect_to  &&connected_part->connect_to->flags == PART_SEVERED )
      {
         connected_part->flags = PART_SEVERED;
         if( connected_part->obj )
            unequip_char( ch, connected_part->obj );
         if( connected_part->obj2 )
            unequip_char( ch, connected_part->obj2 );
      }
      if( connected_part == ch->last_part )
         break;
      connected_part = connected_part->next;
   }
   if( part->loc == BP_HEAD )
   {
      if( was_neck )
         part->loc = BP_NECK;
      switch ( number_range( 1, 10 ) )
      {
         default:
            act( AT_DEAD, "$n's headless corpse falls to the ground.", ch, NULL, NULL, TO_ROOM );
            break;
         case 1:
            act( AT_DEAD, "$n runs around like a chicken with its head cut off for a moment before laying still.", ch, NULL,
                  NULL, TO_ROOM );
            break;
         case 2:
            act( AT_DEAD, "$n's face becomes forever frozen in a look of shock.", ch, NULL, NULL, TO_ROOM );
            break;
      }
      die( ch );
      return TRUE;
   }
   return direct_damage( ch, 100 );
}
void break_part( CHAR_DATA * ch, PART_DATA * part )
{
   if( part->flags != PART_WELL )
      return;
   
      /*
       * parts that can't be broken 
       */ 
      if( part->loc == BP_STOMACH 
          ||part->loc == BP_REAR 
          ||part->loc == BP_LEAR  ||part->loc == BP_PENIS  ||part->loc == BP_REYE  ||part->loc == BP_LEYE )
   {
      return;
   }
   act( AT_ACTION, "You hear a sickening crunch.", ch, NULL, NULL, TO_ROOM );
   act( AT_DANGER, "You hear a sickening crunch from your $t.", ch, part_locs[part->loc], NULL, TO_CHAR );
   part->flags = PART_BROKEN;
}
void unbreak_part( CHAR_DATA * ch, PART_DATA * part )
{
   if( part->flags != PART_BROKEN )
      return;
   act( AT_ACTION, "$n's $t snaps back into place.", ch, part_locs[part->loc], NULL, TO_ROOM );
   act( AT_ACTION, "Your $t snaps back into place.", ch, part_locs[part->loc], NULL, TO_CHAR );
   part->flags = PART_WELL;
}
void hurt_part( CHAR_DATA * ch, PART_DATA * part, int dam )
{
   if( part->cond == PART_SEVERED )
      return;
   if( !IS_NPC( ch ) && ch->last_hit && !IS_NPC( ch->last_hit )  &&IS_SET( ch->last_hit->pcdata->flags, PCFLAG_CHEAT ) )
      return;
   dam = ( 100 - number_range( 1, TALENT( ch, TAL_SECURITY ) ) ) * dam / 100;
   if( dam < 0 )
      dam = 0;
   learn_talent( ch, TAL_SECURITY );
   if( dam > 50 )
   {
      if( outer_layer( part ) )
         damage_obj( outer_layer( part ) );
   }
   
      /*
       * see if the hit severed or broke it 
       */ 
      if( ( dam > 500 && part->cond < -2000 )  &&part->armor < number_range( 1, 100 ) )
   {
      if( sever_part( ch, part ) )
         return;
   }
   else if( dam > 300 && number_range( 1, 2 ) == 1 )
      break_part( ch, part );
   part->cond -= dam;
   
      /*
       * now make sure they can still use their weapons 
       */ 
      if( ch->main_hand  &&!can_use_bodypart( ch, BP_RHAND ) )
   {
      act( AT_HURT, "You are unable to wield your $t anymore and drop it.", ch, myobj( ch->main_hand ), NULL, TO_CHAR );
      act( AT_HURT, "$n winces as $s $t drops to the ground.", ch, myobj( ch->main_hand ), NULL, TO_ROOM );
      obj_unaffect_ch( ch, ch->main_hand );
      ch->main_hand = NULL;
   }
   if( ch->off_hand  &&!can_use_bodypart( ch, BP_LHAND ) )
   {
      act( AT_HURT, "You are unable to wield your $t anymore and drop it.", ch, myobj( ch->off_hand ), NULL, TO_CHAR );
      act( AT_HURT, "$n winces as $s $t drops to the ground.", ch, myobj( ch->off_hand ), NULL, TO_ROOM );
      obj_unaffect_ch( ch, ch->off_hand );
      ch->off_hand = NULL;
   }
   
      /*
       * check to see if they could survive the blow 
       */ 
      if( dam - part->armor * 5 > 500 && part->cond < -3000 
          &&( part->loc == BP_HEAD  ||part->loc == BP_CHEST  ||part->loc == BP_BACK  ||part->loc == BP_NECK ) )
   {
      act( AT_DANGER, "You have taken a fatal injury and die instantly.", ch, NULL, NULL, TO_CHAR );
      act( AT_HURT, "$n has taken a fatal injury and dies instantly.", ch, NULL, NULL, TO_ROOM );
      die( ch );
   }
}


/* A skill to set a broken bone */ 
void do_boneset( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   PART_DATA * part;
   char arg2[MAX_INPUT_LENGTH];
   int loc;
   argument = one_argument( argument, arg2 );
   victim = find_target( ch, argument, FALSE );
   if( !victim )
      return;
   loc = find_part_name( arg2 );
   if( loc == -1 )
   {
      send_to_char( "Set what bone?\n\r", ch );
      return;
   }
   part = find_bodypart( victim, loc );
   if( !part )
   {
      send_to_char( "You don\'t see any such part.\n\r", ch );
      return;
   }
   if( part->flags != PART_BROKEN )
   {
      send_to_char( "That bone is not broken.\n\r", ch );
      return;
   }
   unbreak_part( victim, part );
   act( AT_SKILL, "You carefully set the bone.", ch, NULL, NULL, TO_CHAR );
   act( AT_SKILL, "$n carefully sets a bone.", ch, NULL, NULL, TO_ROOM );
   learn_noncombat( ch, SK_MEDICINE );
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( "setting the bone" );
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
}


/* Bandage up a bleeding part */ 
void do_bandage( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA * victim;
   PART_DATA * part;
   char arg2[MAX_INPUT_LENGTH];
   int loc;
   argument = one_argument( argument, arg2 );
   victim = find_target( ch, argument, FALSE );
   if( !victim )
      return;
   loc = find_part_name( arg2 );
   if( loc == -1 )
   {
      send_to_char( "Bandage what part?\n\r", ch );
      return;
   }
   part = find_bodypart( victim, loc );
   if( !part )
   {
      send_to_char( "You don\'t see any such part.\n\r", ch );
      return;
   }
   if( part->cond > 0 )
   {
      send_to_char( "That part is not bleeding.\n\r", ch );
      return;
   }
   part->cond += number_range( SK_MEDICINE, SK_MEDICINE * 10 );
   act( AT_SKILL, "You carefully bandage the wound.", ch, NULL, NULL, TO_CHAR );
   act( AT_SKILL, "$n carefully bandages a wound.", ch, NULL, NULL, TO_ROOM );
   learn_noncombat( ch, SK_MEDICINE );
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( "bandaging the wound" );
   WAIT_STATE( ch, PULSE_VIOLENCE );
}


