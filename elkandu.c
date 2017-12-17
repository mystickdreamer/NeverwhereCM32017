/*
 * Elkandu Magic System
 *
 * Written by Keolah for Rogue Winds
 *
 * This is an experiment at creating a non spell based magic system, involving
 * different combinations of keywords to create a desired magical effect.
 * Assumption: All players are mages, and can make at least minimal use of their magic.
 *
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef sun
#include <strings.h>
#endif
#include <time.h>
#include "mud.h"

/* from act_move.c */
extern void drop_dream_items( CHAR_DATA * ch, OBJ_DATA * obj );

/* from birth.c */
extern void set_race( CHAR_DATA * ch, bool died );

/* from bodyparts.c */
extern void add_part( CHAR_DATA * ch, int part, int to_part );

/* from combat.c */
extern void ranged_attack( CHAR_DATA * ch, OBJ_DATA * obj, char *argument );
extern bool can_gain_exp( CHAR_DATA * ch, CHAR_DATA * victim );

/* from handler.c */
extern MATERIAL_DATA *material_lookup( int number );
extern SPECIES_DATA *find_species( char *name );
extern char *munch_colors( char *word );

/* from talent.c */
extern bool can_travel( int i, CHAR_DATA * ch, CHAR_DATA * victim );

typedef enum
{
   ACT_TRICKS, ACT_HARM, ACT_HELP, ACT_CHANGE, ACT_MOVE, ACT_FIND, ACT_CREATE,
   ACT_ENHANCE, ACT_HINDER, ACT_FOCUS, ACT_RUNE, ACT_SING, ACT_CONTROL
} action_types;

void obj_trick( CHAR_DATA * ch, OBJ_DATA * obj, int type )
{
   char buf[MAX_STRING_LENGTH];

   switch ( type )
   {
      default:
         sprintf( buf, "&PRaw mana %s $p and %s.",
                  number_range( 1, 2 ) == 1 ? "surrounds" : "circles",
                  number_range( 1, 2 ) == 1 ? "vanishes" : "disappears" );
         break;
      case MAG_FIRE:
         sprintf( buf, "&R%s flames %s %s $p and %s.",
                  number_range( 1, 2 ) == 1 ? "Flickering" : "Lambent",
                  number_range( 1, 2 ) == 1 ? "streak" : "dance",
                  number_range( 1, 2 ) == 1 ? "over" : "along", number_range( 1, 2 ) == 1 ? "vanish" : "disappear" );
         break;
      case MAG_EARTH:
         sprintf( buf, "&GLeaves %s %s $p and %s.",
                  number_range( 1, 2 ) == 1 ? "sprout" : "grow",
                  number_range( 1, 2 ) == 1 ? "from" : "all over", number_range( 1, 2 ) == 1 ? "wither" : "fall off" );
         break;
      case MAG_BLUNT:
         sprintf( buf, "&O%s %s %s $p %s.",
                  number_range( 1, 2 ) == 1 ? "Tiny" : "Little",
                  number_range( 1, 2 ) == 1 ? "pebbles" : "bits of rock",
                  number_range( 1, 2 ) == 1 ? "pelt" : "strike", number_range( 1, 2 ) == 1 ? "lightly" : "softly" );
         break;
      case MAG_ELECTRICITY:
         sprintf( buf, "&Y%s %s %s %s $p and %s.",
                  number_range( 1, 2 ) == 1 ? "Crackling" : "White",
                  number_range( 1, 2 ) == 1 ? "lightning" : "electricity",
                  number_range( 1, 2 ) == 1 ? "shoots" : "races",
                  number_range( 1, 2 ) == 1 ? "over" : "along", number_range( 1, 2 ) == 1 ? "vanishes" : "disappears" );
         break;
      case MAG_WIND:
         sprintf( buf, "&c%s %s %s $p and %s %s.",
                  number_range( 1, 2 ) == 1 ? "Howling" : "Whipping",
                  number_range( 1, 2 ) == 1 ? "winds" : "storms",
                  number_range( 1, 2 ) == 1 ? "twist around" : "surround",
                  number_range( 1, 2 ) == 1 ? "abruptly" : "suddenly", number_range( 1, 2 ) == 1 ? "cease" : "stop" );
         break;
      case MAG_COLD:
         sprintf( buf, "&CA %s layer of %s %s $p and %s.",
                  number_range( 1, 2 ) == 1 ? "light" : "thin",
                  number_range( 1, 2 ) == 1 ? "frost" : "ice",
                  number_range( 1, 2 ) == 1 ? "covers" : "spreads over", number_range( 1, 2 ) == 1 ? "melts" : "vanishes" );
         break;
      case MAG_WATER:
         sprintf( buf, "&B%s %s of water %s $p and %s.",
                  number_range( 1, 2 ) == 1 ? "Small" : "Tiny",
                  number_range( 1, 2 ) == 1 ? "drops" : "droplets",
                  number_range( 1, 2 ) == 1 ? "cover" : "flow over", number_range( 1, 2 ) == 1 ? "drip away" : "evaporate" );
         break;
      case MAG_ACID:
         sprintf( buf, "&g%s of acid %s from $p.",
                  number_range( 1, 2 ) == 1 ? "Flecks" : "Tiny droplets", number_range( 1, 2 ) == 1 ? "drip" : "drizzle" );
         break;
      case MAG_DREAM:
         sprintf( buf, "&P$p %s %s as if %s from a %s.",
                  number_range( 1, 2 ) == 1 ? "flashes" : "flickers",
                  number_range( 1, 2 ) == 1 ? "for an instant" : "for a moment",
                  number_range( 1, 2 ) == 1 ? "an image" : "something", number_range( 1, 2 ) == 1 ? "dream" : "nightmare" );
         break;
      case MAG_SOUND:
         sprintf( buf, "&Y$p %s %s for a %s.",
                  number_range( 1, 2 ) == 1 ? "hums" : "sings",
                  number_range( 1, 2 ) == 1 ? "eerily" : "faintly", number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_HOLY:
         sprintf( buf, "&z&W%s %s light %s $p %s.",
                  number_range( 1, 2 ) == 1 ? "Shimmering" : "Holy",
                  number_range( 1, 2 ) == 1 ? "white" : "silver",
                  number_range( 1, 2 ) == 1 ? "surrounds" : "encompasses",
                  number_range( 1, 2 ) == 1 ? "briefly" : "for a moment" );
         break;
      case MAG_UNHOLY:
         sprintf( buf, "&z$p %s a %s %s for a %s.",
                  number_range( 1, 2 ) == 1 ? "emits" : "casts",
                  number_range( 1, 2 ) == 1 ? "deep" : "dark",
                  number_range( 1, 2 ) == 1 ? "shadow" : "darkness", number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_POISON:
         sprintf( buf, "&g%s of %s %s %s $p.",
                  number_range( 1, 2 ) == 1 ? "Wisps" : "Puffs",
                  number_range( 1, 2 ) == 1 ? "smoke" : "poisonous gas",
                  number_range( 1, 2 ) == 1 ? "drift" : "waft", number_range( 1, 2 ) == 1 ? "over" : "around" );
         break;
      case MAG_CHANGE:
      case MAG_TIME:
         sprintf( buf, "&B$p %s %s %s.",
                  number_range( 1, 2 ) == 1 ? "twists" : "warps",
                  number_range( 1, 2 ) == 1 ? "strangely" : "eerily",
                  number_range( 1, 2 ) == 1 ? "briefly" : "for a moment" );
         break;
      case MAG_TELEKINETIC:
         sprintf( buf, "&Y$p %s and %s %s.",
                  number_range( 1, 2 ) == 1 ? "twitches" : "dances",
                  number_range( 1, 2 ) == 1 ? "tingles" : "hums", number_range( 1, 2 ) == 1 ? "briefly" : "for a moment" );
         break;
      case MAG_LIGHT:
      case MAG_SEEK:
         sprintf( buf, "&Y$p %s %s for a %s.",
                  number_range( 1, 2 ) == 1 ? "glows" : "shines",
                  number_range( 1, 2 ) == 1 ? "brightly" : "faintly", number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_PSIONIC:
         sprintf( buf, "&p$p %s %s with %s %s.",
                  number_range( 1, 2 ) == 1 ? "shimmers" : "shines",
                  number_range( 1, 2 ) == 1 ? "violet" : "faintly",
                  number_range( 1, 2 ) == 1 ? "mental" : "psionic", number_range( 1, 2 ) == 1 ? "energy" : "power" );
         break;
      case MAG_ANTIMATTER:
      case MAG_ANTIMAGIC:
         sprintf( buf, "&z%s black %s %s %s $p and %s.",
                  number_range( 1, 2 ) == 1 ? "Crackling" : "Humming",
                  number_range( 1, 2 ) == 1 ? "lightning" : "antimatter",
                  number_range( 1, 2 ) == 1 ? "shoots" : "streaks",
                  number_range( 1, 2 ) == 1 ? "around" : "along", number_range( 1, 2 ) == 1 ? "vanishes" : "disappears" );
         break;
      case MAG_ETHER:
         sprintf( buf, "&w%s %s of %s %s %s $p.",
                  number_range( 1, 2 ) == 1 ? "Insubstantial" : "Pale",
                  number_range( 1, 2 ) == 1 ? "wisps" : "tendrils",
                  number_range( 1, 2 ) == 1 ? "ether" : "mist",
                  number_range( 1, 2 ) == 1 ? "float" : "drift", number_range( 1, 2 ) == 1 ? "over" : "along" );
         break;
   }
   act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
   act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
}

void ch_trick( CHAR_DATA * ch, int type )
{
   char buf[MAX_STRING_LENGTH];

   switch ( type )
   {
      default:
         sprintf( buf, "&PRaw mana %s $n and %s.",
                  number_range( 1, 2 ) == 1 ? "surrounds" : "circles",
                  number_range( 1, 2 ) == 1 ? "vanishes" : "disappears" );
         break;
      case MAG_FIRE:
         sprintf( buf, "&R%s flames %s %s $n and %s.",
                  number_range( 1, 2 ) == 1 ? "Flickering" : "Lambent",
                  number_range( 1, 2 ) == 1 ? "streak" : "dance",
                  number_range( 1, 2 ) == 1 ? "around" : "in front of", number_range( 1, 2 ) == 1 ? "vanish" : "disappear" );
         break;
      case MAG_EARTH:
         sprintf( buf, "%s leaves %s %s $n and %s.",
                  number_range( 1, 2 ) == 1 ? "&GGreen" : "&OAutumn",
                  number_range( 1, 2 ) == 1 ? "swirl" : "blow",
                  number_range( 1, 2 ) == 1 ? "around" : "in front of",
                  number_range( 1, 2 ) == 1 ? "blow away" : "disappear" );
         break;
      case MAG_BLUNT:
         sprintf( buf, "&O%s %s %s $n %s.",
                  number_range( 1, 2 ) == 1 ? "Tiny" : "Little",
                  number_range( 1, 2 ) == 1 ? "pebbles" : "bits of rock",
                  number_range( 1, 2 ) == 1 ? "spin around" : "encircle",
                  number_range( 1, 2 ) == 1 ? "briefly" : "momentarily" );
         break;
      case MAG_ELECTRICITY:
         sprintf( buf, "&Y%s %s %s $n and %s.",
                  number_range( 1, 2 ) == 1 ? "Crackling" : "White",
                  number_range( 1, 2 ) == 1 ? "lightning" : "electricity",
                  number_range( 1, 2 ) == 1 ? "surrounds" : "splits around",
                  number_range( 1, 2 ) == 1 ? "vanishes" : "disappears" );
         break;
      case MAG_WIND:
         sprintf( buf, "&C%s %s %s $n and %s %s.",
                  number_range( 1, 2 ) == 1 ? "Howling" : "Whipping",
                  number_range( 1, 2 ) == 1 ? "winds" : "storms",
                  number_range( 1, 2 ) == 1 ? "swirl around" : "surround",
                  number_range( 1, 2 ) == 1 ? "abruptly" : "suddenly", number_range( 1, 2 ) == 1 ? "cease" : "stop" );
         break;
      case MAG_COLD:
         sprintf( buf, "&C%s %s %s $n and %s %s.",
                  number_range( 1, 2 ) == 1 ? "Swirling cold" : "Frigid",
                  number_range( 1, 2 ) == 1 ? "winds" : "snowflakes",
                  number_range( 1, 2 ) == 1 ? "swirl around" : "surround",
                  number_range( 1, 2 ) == 1 ? "abruptly" : "suddenly", number_range( 1, 2 ) == 1 ? "cease" : "stop" );
         break;
      case MAG_WATER:
         sprintf( buf, "&C%s bubbles %s around $n and %s with a %s.",
                  number_range( 1, 2 ) == 1 ? "Small" : "Shimmering",
                  number_range( 1, 2 ) == 1 ? "spin" : "swirl",
                  number_range( 1, 2 ) == 1 ? "vanish" : "disappear", number_range( 1, 2 ) == 1 ? "pop" : "splash" );
         break;
      case MAG_ACID:
         sprintf( buf, "&gA %s of %s acid %s around $n.",
                  number_range( 1, 2 ) == 1 ? "shower" : "drizzle",
                  number_range( 1, 2 ) == 1 ? "glistening" : "tiny flecks of",
                  number_range( 1, 2 ) == 1 ? "swirls" : "spins" );
         break;
      case MAG_DREAM:
         sprintf( buf, "&P$n %s %s as if %s from a %s.",
                  number_range( 1, 2 ) == 1 ? "flashes" : "flickers",
                  number_range( 1, 2 ) == 1 ? "for an instant" : "for a moment",
                  number_range( 1, 2 ) == 1 ? "an image" : "something", number_range( 1, 2 ) == 1 ? "dream" : "nightmare" );
         break;
      case MAG_SOUND:
         sprintf( buf, "&YA %s sound %s $n for a %s.",
                  number_range( 1, 2 ) == 1 ? "humming" : "singing",
                  number_range( 1, 2 ) == 1 ? "surrounds" : "eminates", number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_HOLY:
         sprintf( buf, "&z&W%s %s light %s $n for a %s.",
                  number_range( 1, 2 ) == 1 ? "Shimmering" : "Holy",
                  number_range( 1, 2 ) == 1 ? "white" : "silver",
                  number_range( 1, 2 ) == 1 ? "eminates from" : "surrounds",
                  number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_UNHOLY:
      case MAG_DRAIN:
         sprintf( buf, "&z%s %s %s $n for a %s.",
                  number_range( 1, 2 ) == 1 ? "Tendrils of" : "Eerie",
                  number_range( 1, 2 ) == 1 ? "shadows" : "darkness",
                  number_range( 1, 2 ) == 1 ? "surround" : "twist around", number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_POISON:
         sprintf( buf, "&g%s of %s %s %s $n.",
                  number_range( 1, 2 ) == 1 ? "Wisps" : "Puffs",
                  number_range( 1, 2 ) == 1 ? "smoke" : "poisonous gas",
                  number_range( 1, 2 ) == 1 ? "swirl" : "circle", number_range( 1, 2 ) == 1 ? "in front of" : "around" );
         break;
      case MAG_CHANGE:
      case MAG_TIME:
         sprintf( buf, "&B$n %s %s %s.",
                  number_range( 1, 2 ) == 1 ? "twists" : "warps",
                  number_range( 1, 2 ) == 1 ? "strangely" : "eerily",
                  number_range( 1, 2 ) == 1 ? "briefly" : "for a moment" );
         break;
      case MAG_TELEKINETIC:
         sprintf( buf, "&Y$n %s and %s for a %s.",
                  number_range( 1, 2 ) == 1 ? "shivers" : "itches",
                  number_range( 1, 2 ) == 1 ? "tickles" : "tingles", number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_LIGHT:
      case MAG_SEEK:
         sprintf( buf, "&Y$n %s %s for a %s.",
                  number_range( 1, 2 ) == 1 ? "glows" : "shines",
                  number_range( 1, 2 ) == 1 ? "brightly" : "faintly", number_range( 1, 2 ) == 1 ? "moment" : "while" );
         break;
      case MAG_PSIONIC:
         sprintf( buf, "&p$n %s %s with %s %s.",
                  number_range( 1, 2 ) == 1 ? "shimmers" : "shines",
                  number_range( 1, 2 ) == 1 ? "violet" : "faintly",
                  number_range( 1, 2 ) == 1 ? "mental" : "psionic", number_range( 1, 2 ) == 1 ? "energy" : "power" );
         break;
      case MAG_ANTIMATTER:
      case MAG_ANTIMAGIC:
         sprintf( buf, "&z%s black %s %s $n and %s.",
                  number_range( 1, 2 ) == 1 ? "Crackling" : "Humming",
                  number_range( 1, 2 ) == 1 ? "lightning" : "antimatter",
                  number_range( 1, 2 ) == 1 ? "surrounds" : "encircles",
                  number_range( 1, 2 ) == 1 ? "vanishes" : "disappears" );
         break;
      case MAG_ETHER:
         sprintf( buf, "&w%s %s of %s %s %s $n.",
                  number_range( 1, 2 ) == 1 ? "Insubstantial" : "Pale",
                  number_range( 1, 2 ) == 1 ? "wisps" : "tendrils",
                  number_range( 1, 2 ) == 1 ? "ether" : "mist",
                  number_range( 1, 2 ) == 1 ? "float" : "swirl", number_range( 1, 2 ) == 1 ? "around" : "in front of" );
         break;
   }
   act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
   act( AT_ACTION, buf, ch, NULL, NULL, TO_CHAR );
}

void obj_harm( CHAR_DATA * ch, OBJ_DATA * obj, int type, int power )
{
   AREA_DATA *area;
   int vnum;
   char buf[MAX_STRING_LENGTH];

   if( ch->in_room->pShop )
   {
      obj_trick( ch, obj, type );
      return;
   }

   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         break;

      case MAG_FIRE:
         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) || obj->item_type == ITEM_FIRE )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->weight * 30 > power )
         {
            if( IS_OBJ_STAT( obj, ITEM_FLAMMABLE ) )
            {
               act( AT_FIRE, "You set $p on fire.", ch, obj, NULL, TO_CHAR );
               act( AT_FIRE, "$n set $p on fire.", ch, obj, NULL, TO_ROOM );
               obj->item_type = ITEM_FIRE;
               obj->timer = obj->weight - ( power / 30 );
               sprintf( buf, "burning %s", myobj( obj ) );
               STRFREE( obj->short_descr );
               obj->short_descr = STRALLOC( buf );
               break;
            }
            else
            {
               obj_trick( ch, obj, type );
               break;
            }
         }

         if( IS_OBJ_STAT( obj, ITEM_FLAMMABLE ) )
         {
            act( AT_FIRE, "You burn $p into ashes.", ch, obj, NULL, TO_CHAR );
            act( AT_FIRE, "$n burns $p into ashes.", ch, obj, NULL, TO_ROOM );
            dump_container( obj );
            extract_obj( obj );
            return;
         }

         if( !obj->material || obj->material->number == 116 /* ether */  )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !IS_OBJ_STAT( obj, ITEM_METAL ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         vnum = 0;
         if( !stockobj )
            for( area = first_area; area; area = area->next )
            {
               if( !str_cmp( area->filename, "stockobj.are" ) )
               {
                  stockobj = area;
                  vnum = area->hi_o_vnum;
                  break;
               }
            }
         else
         {
            area = stockobj;
            vnum = area->hi_o_vnum;
         }
         if( vnum == 0 )
         {
            bug( "Cannot find stockobj.are!", 0 );
            return;
         }
         act( AT_FIRE, "You melt $p into a misshapen lump of metal.", ch, obj, NULL, TO_CHAR );
         act( AT_FIRE, "$n melts $p into a misshapen lump of metal.", ch, obj, NULL, TO_ROOM );
         obj->cost -= obj->pIndexData->cost;
         obj->pIndexData = get_obj_index( vnum );
         STRFREE( obj->name );
         obj->name = STRALLOC( obj->material->name );
         STRFREE( obj->short_descr );
         obj->short_descr = STRALLOC( obj->material->short_descr );
         obj->item_type = ITEM_TREASURE;
         xCLEAR_BITS( obj->parts );
         break;

      case MAG_ANTIMATTER:
         if( ch->curr_talent[TAL_VOID] < 100 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->weight * 10 > power )
         {
            obj_trick( ch, obj, type );
            break;
         }

         act( AT_DGREY, "You burn $p out of existance!", ch, obj, NULL, TO_CHAR );
         act( AT_DGREY, "$n burns $p out of existance!", ch, obj, NULL, TO_ROOM );
         dump_container( obj );
         extract_obj( obj );
         break;

      case MAG_COLD:
         if( ch->curr_talent[TAL_FROST] < 100 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->weight * 30 > power )
         {
            obj_trick( ch, obj, type );
            break;
         }

         act( AT_FROST, "You freeze $p solid and shatter it!", ch, obj, NULL, TO_CHAR );
         act( AT_FROST, "$n freezes $p solid and shatters it!", ch, obj, NULL, TO_ROOM );
         dump_container( obj );
         extract_obj( obj );
         break;

      case MAG_DRAIN:
         if( ch->curr_talent[TAL_DEATH] < 20 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->timer <= 0 )
         {
            act( AT_BLOOD, "$p starts to decay slowly.", ch, obj, NULL, TO_CHAR );
            obj->timer = 100;
            return;
         }

         act( AT_BLOOD, "$p decays further.", ch, obj, NULL, TO_CHAR );
         obj->timer -= TALENT( ch, TAL_DEATH ) / 10;
         if( obj->timer < 1 )
            obj->timer = 1;
         break;

      case MAG_ELECTRICITY:
         if( ch->curr_talent[TAL_LIGHTNING] < 75 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !IS_OBJ_STAT( obj, ITEM_EXPLOSIVE ) )
         {
            if( obj->weight * 20 < power )
            {
               obj->timer = 1;
               xSET_BIT( obj->extra_flags, ITEM_EXPLOSIVE );
               act( AT_FIRE, "You add an explosive charge to $p.", ch, obj, NULL, TO_CHAR );
            }
            else
            {
               obj_trick( ch, obj, type );
               break;
            }
         }
         else
         {
            obj->timer = 0;
            xREMOVE_BIT( obj->extra_flags, ITEM_EXPLOSIVE );
            act( AT_FIRE, "You remove the explosive charge from $p.", ch, obj, NULL, TO_CHAR );
         }
         break;

   }
}

void ch_harm( CHAR_DATA * victim, CHAR_DATA * ch, int dam, int type, bool dont_wait )
{
   if( !victim || victim == NULL )
   {
      ch_trick( ch, type );
      return;
   }

   if( char_died( victim ) )
      return;

   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
   {
      ch_trick( victim, type );
      return;
   }

   if( !IS_SAME_PLANE( ch, victim ) )
   {
      if( CAN_SEE_PLANE( ch, victim ) )
         send_to_char( "That appears to have no affect.\n\r", ch );
      return;
   }

   if( !dont_wait )
   {
      STRFREE( ch->last_taken );
      ch->last_taken = STRALLOC( "channeling" );
      WAIT_STATE( ch, PULSE_VIOLENCE );
   }

   if( victim->shield == type )
   {
      act( AT_MAGIC, "Your $t bounces off $N's shield!", ch, magic_table[type], victim, TO_CHAR );
      act( AT_MAGIC, "$n's $t bounces off $N's shield!", ch, magic_table[type], victim, TO_NOTVICT );
      act( AT_MAGIC, "$n's $t bounces off your shield!", ch, magic_table[type], victim, TO_VICT );
      /*
       * don't keep them bouncing back and forth 
       */
      if( ch->shield == type )
      {
         act( AT_MAGIC, "Your $t fizzles out harmlessly.", ch, magic_table[type], NULL, TO_CHAR );
         act( AT_MAGIC, "$n's $t fizzles out harmlessly", ch, magic_table[type], NULL, TO_ROOM );
         return;
      }
      /*
       * send it back at them 
       */
      ch_harm( ch, victim, dam, type, TRUE );
      return;
   }

   if( check_displacement( ch, victim ) )
      return;
   if( check_antimagic( ch, victim ) )
      return;

   if( type == MAG_UNHOLY )
   {
      if( IS_AFFECTED( victim, AFF_ANGEL ) )
      {
         dam *= 2;
      }
      else
      {
         dam /= 2;
      }
   }

   else if( type == MAG_HOLY )
   {
      if( IS_AFFECTED( victim, AFF_DEMON ) )
      {
         dam *= 2;
      }
      else
      {
         dam /= 2;
      }
   }

   else if( type == MAG_DREAM )
   {
      if( IS_AFFECTED( victim, AFF_DREAMWORLD ) )
      {
         dam *= 3;
      }
      else
      {
         dam = 0;
      }
   }

   else if( IS_UNDERWATER( victim ) && type == MAG_FIRE )
      dam /= 2;

   else if( type == MAG_EARTH )
      dam = ( dam * 200 ) / ( victim->in_room->curr_vegetation + 10 );

   else if( type == MAG_WATER )
      dam = ( dam * 200 ) / ( victim->in_room->curr_water + 10 );

   dam = number_range( dam, dam * 5 );
   dam = URANGE( 1, dam, 5000 );
/*	dam = (100 - TALENT(victim, talent)) * dam / 100; */

   if( type == MAG_DRAIN )
   {
      dam /= 2;
      ch->hit += dam;
      if( ch->hit > ch->max_hit )
         ch->hit = ch->max_hit;
   }

   if( dam <= 0 )
   {
      act( AT_MAGIC, "The magic has no effect.", victim, NULL, NULL, TO_ROOM );
      act( AT_MAGIC, "The magic has no effect.", victim, NULL, NULL, TO_CHAR );

      if( IS_NPC( ch ) && number_range( 20, 50 ) < get_curr_int( ch ) )
         xSET_BIT( ch->act, ACT_MAG_FAIL );
      return;
   }

   act( AT_MAGIC, "Your $t hits $N!", ch, magic_table[type], victim, TO_CHAR );
   act( AT_MAGIC, "$n's $t hits $N!", ch, magic_table[type], victim, TO_NOTVICT );
   act( AT_MAGIC, "$n's $t hits you!", ch, magic_table[type], victim, TO_VICT );
   victim->last_hit = ch;
   ch->last_hit = victim;
   if( can_gain_exp( ch, victim ) )
      gain_exp( ch, get_exp_worth( victim ) * dam / 100 );
   direct_damage( victim, dam );
   if( char_died( victim ) )
      return;
   pain( victim, dam );
   climate_affect( victim, type );
}

void obj_help( CHAR_DATA * ch, OBJ_DATA * obj, int type, int power )
{
   int amt;
   bool found;
   AFFECT_DATA *paf;

   if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
   {
      obj_trick( ch, obj, type );
      return;
   }

   separate_obj( obj );
   amt = 50;
   found = FALSE;

   if( obj->mana < amt && type != MAG_TIME && type != MAG_ALL && type != MAG_LIGHT && type != MAG_UNHOLY )
   {
      obj_trick( ch, obj, type );
      return;
   }
   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         break;

      case MAG_ALL:
         if( obj->mana < amt || IS_OBJ_STAT( obj, ITEM_MAGIC ) || ch->curr_talent[TAL_CATALYSM] < 40 )
         {
            act( AT_MAGIC, "You funnel raw energy into $p.", ch, obj, NULL, TO_CHAR );
            if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
            {
               send_to_char( "The magic is absorbed with no effect.\n\r", ch );
               return;
            }
            power /= obj->weight == 0 ? 1 : obj->weight;
            obj->raw_mana += power;
            if( obj->gem )
               obj->gem->raw_mana += power;
            if( obj->raw_mana > 2000 + TALENT( ch, TAL_CATALYSM ) * 5 + ch->pcdata->noncombat[SK_CHANNEL] * 20 )
            {
               act( AT_DANGER, "The overcharged item explodes in your hands!", ch, NULL, NULL, TO_CHAR );
               act( AT_ACTION, "$p explodes in $n's hands.", ch, obj, NULL, TO_ROOM );
               lose_hp( ch, obj->mana * 3 + ( obj->raw_mana / 20 ) );
               extract_obj( obj );
            }
            else if( obj->raw_mana > 2000 )
               act( AT_PURPLE, "$p vibrates violently!", ch, obj, NULL, TO_CHAR );
            return;
         }
         act( AT_PINK, "The raw mana settles into $n's $p, emitting a mystic glow.", ch, obj, NULL, TO_ROOM );
         act( AT_PINK, "The raw mana settles into $p, emitting a mystic glow.", ch, obj, NULL, TO_CHAR );
         xSET_BIT( obj->extra_flags, ITEM_MAGIC );
         break;

      case MAG_HOLY:
         amt = 500;
         if( IS_OBJ_STAT( obj, ITEM_SHIELD ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( ch->curr_talent[TAL_SECURITY] + ch->curr_talent[TAL_CATALYSM] < 195 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( obj->mana < amt )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_WHITE, "A holy light eminates from $n's $p, coalescing into a protective barrier.",
              ch, obj, NULL, TO_ROOM );
         act( AT_WHITE, "A holy light eminates from $p, coalescing into a protective barrier.", ch, obj, NULL, TO_CHAR );
         xSET_BIT( obj->extra_flags, ITEM_SHIELD );
         obj->cost += 450000;
         break;

      case MAG_LIGHT:
         if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( ch->curr_talent[TAL_ILLUSION] < 30 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( IS_OBJ_STAT( obj, ITEM_DARK ) )
         {
            if( obj->value[4] == BRAND_ANTI_MATTER )
            {
               send_to_char( "The weapon absorbs all the light and remains dark.\n\r", ch );
               return;
            }
            act( AT_YELLOW, "The $t takes in some of the light around it.", ch, myobj( obj ), NULL, TO_CHAR );
            act( AT_YELLOW, "$n's $t takes in some of the light around it.", ch, myobj( obj ), NULL, TO_ROOM );
            xREMOVE_BIT( obj->extra_flags, ITEM_DARK );
            return;
         }
         act( AT_YELLOW, "The $t shines brightly!", ch, myobj( obj ), NULL, TO_CHAR );
         act( AT_YELLOW, "$n's $t shines brightly!", ch, myobj( obj ), NULL, TO_ROOM );
         xSET_BIT( obj->extra_flags, ITEM_GLOW );
         return;

      case MAG_UNHOLY:
         if( IS_OBJ_STAT( obj, ITEM_DARK ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( ch->curr_talent[TAL_DEATH] < 30 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( IS_OBJ_STAT( obj, ITEM_GLOW ) )
         {
            if( obj->value[4] == BRAND_RADIANT )
            {
               send_to_char( "You are unable to quench this radiant light.\n\r", ch );
               return;
            }
            act( AT_MAGIC, "The $t grows dim and ceases to shine.", ch, myobj( obj ), NULL, TO_CHAR );
            act( AT_MAGIC, "$n's $t grows dim and ceases to shine.", ch, myobj( obj ), NULL, TO_ROOM );
            xREMOVE_BIT( obj->extra_flags, ITEM_GLOW );
            return;
         }
         act( AT_MAGIC, "Swirling shadows shroud the $t.", ch, myobj( obj ), NULL, TO_CHAR );
         act( AT_MAGIC, "Swirling shadows shroud $n's $t.", ch, myobj( obj ), NULL, TO_ROOM );
         xSET_BIT( obj->extra_flags, ITEM_DARK );
         return;

      case MAG_TELEKINETIC:
         amt = 100;
         if( IS_OBJ_STAT( obj, ITEM_RETURNING ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( ch->curr_talent[TAL_MOTION] < 50 && TALENT( ch, TAL_MOTION ) < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( obj->mana < amt )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_YELLOW, "$n's $p twitches for a moment, as a telekinetic enchantment settles in.", ch, obj, NULL, TO_ROOM );
         act( AT_YELLOW, "$p twitches for a moment, as a telekinetic enchantment settles in.", ch, obj, NULL, TO_CHAR );
         xSET_BIT( obj->extra_flags, ITEM_RETURNING );
         break;

      case MAG_WATER:
         if( ch->curr_talent[TAL_WATER] < 40 )
         {
            obj_trick( ch, obj, type );
            return;
         }

         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_AFFECT;
         SET_BIT( paf->modifier, 1 << AFF_AQUA_BREATH );
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_BLUE, "Bubbles erupt from $n's $p as it glows with a watery film.", ch, obj, NULL, TO_ROOM );
         act( AT_BLUE, "Bubbles erupt from $p as it glows with a watery film.", ch, obj, NULL, TO_CHAR );
         break;

      case MAG_WIND:
         if( ch->curr_talent[TAL_WIND] < 60 )
         {
            obj_trick( ch, obj, type );
            return;
         }

         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_AFFECT;
         SET_BIT( paf->modifier, 1 << AFF_FLYING );
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_LBLUE, "$n's $p sprouts a pair of tiny wings.", ch, obj, NULL, TO_ROOM );
         act( AT_LBLUE, "$p sprouts a pair of tiny wings.", ch, obj, NULL, TO_CHAR );
         break;

      case MAG_SEEK:
         if( ch->curr_talent[TAL_SEEKING] < 80 )
         {
            obj_trick( ch, obj, type );
            return;
         }

         CREATE( paf, AFFECT_DATA, 1 );
         paf->type = -1;
         paf->duration = -1;
         paf->location = APPLY_AFFECT;
         SET_BIT( paf->modifier, 1 << AFF_TRUESIGHT );
         xCLEAR_BITS( paf->bitvector );
         paf->next = NULL;
         LINK( paf, obj->first_affect, obj->last_affect, next, prev );
         act( AT_YELLOW, "$n's $p glows for a long while with a pure and true light.", ch, obj, NULL, TO_ROOM );
         act( AT_YELLOW, "$p glows for a long while with a pure and true light.", ch, obj, NULL, TO_CHAR );
         break;

      case MAG_DRAIN:
         if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_MISSILE_WEAPON )
         {
            obj_trick( ch, obj, type );
            return;
         }

         if( ch->curr_talent[TAL_DEATH] < 10 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         amt = URANGE( 10, 250 - TALENT( ch, TAL_DEATH ), 200 );
         amt += obj->value[2];
         if( obj->mana < amt )
         {
            obj_trick( ch, obj, type );
            return;
         }
         obj->value[2] += 5;
         act( AT_DGREY, "$n's $p glints with a deadly power.", ch, obj, NULL, TO_ROOM );
         act( AT_DGREY, "$p glints with a deadly power.", ch, obj, NULL, TO_CHAR );
         break;

      case MAG_ANTIMATTER:
         if( obj->item_type != ITEM_CONTAINER )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( ch->curr_talent[TAL_VOID] >= 10 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         amt = URANGE( 10, 250 - TALENT( ch, TAL_VOID ), 200 );
         amt += obj->value[0] / 10;
         if( obj->mana < amt )
         {
            obj_trick( ch, obj, type );
            return;
         }
         obj->value[0] += TALENT( ch, TAL_VOID ) / 10;
         act( AT_DGREY, "$n's $p ripples strangely, the space within stretching to infinity.", ch, obj, NULL, TO_ROOM );
         act( AT_DGREY, "$p ripples strangely, the space within stretching to infinity.", ch, obj, NULL, TO_CHAR );
         break;

      case MAG_BLUNT:
         amt = 200;
         if( IS_OBJ_STAT( obj, ITEM_DURABLE ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( ch->curr_talent[TAL_SECURITY] + ch->curr_talent[TAL_EARTH] < 100 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( obj->mana < amt )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_ORANGE, "$n's $p radiates strentch and durability.", ch, obj, NULL, TO_ROOM );
         act( AT_ORANGE, "$p radiates strentch and durability.", ch, obj, NULL, TO_CHAR );
         xSET_BIT( obj->extra_flags, ITEM_DURABLE );
         break;

      case MAG_TIME:
         if( ch->curr_talent[TAL_TIME] < 10 )
         {
            obj_trick( ch, obj, type );
            return;
         }

         if( obj->timer <= 0 || IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         obj->timer += TALENT( ch, TAL_TIME );
         if( obj->timer > 100 )
            obj->timer = 0;
         act( AT_MAGIC, "$p flickers for a moment.", ch, obj, NULL, TO_CHAR );
         return;

   }
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
   obj->mana -= amt;
   obj->cost += 50000;
}

void ch_help( CHAR_DATA * victim, CHAR_DATA * ch, int type, int power )
{
   PART_DATA *part;
   AFFECT_DATA af;
   int sn;

   switch ( type )
   {
      default:
         ch_trick( victim, type );
         break;

      case MAG_HOLY:
         if( victim == ch )
         {
            act( AT_MAGIC, "You close your eyes and relax, focusing on healing.", ch, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n closes $s eyes and breathes deeply, $s wounds closing.", ch, NULL, NULL, TO_ROOM );
         }
         else
         {
            act( AT_MAGIC, "$n concentrates on you, and pain eases out of your body", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "$n concentrates on $N, whose wounds begin to close.", ch, NULL, victim, TO_NOTVICT );
            act( AT_MAGIC, "You concentrate on $N, easing $S pain and healing $S wounds.", ch, NULL, victim, TO_CHAR );
         }

         if( IS_AFFECTED( ch, AFF_UNHOLY ) && power != 0 )
            power /= 2;
         if( IS_AFFECTED( ch, AFF_HOLY ) )
            power *= 2;
         if( victim == ch )
            power *= 2;
         if( !IS_FIGHTING( ch ) )
            power *= 2;
         victim->hit += power;
         for( part = victim->first_part; part; part = part->next )
         {
            if( part->flags == PART_SEVERED
                && part->cond == PART_WELL
                && ( part->connect_to ? part->connect_to->flags != PART_SEVERED : TRUE ) && power > 300 )
            {
               act( AT_MAGIC, "Your $t grows back!", victim, part_locs[part->loc], NULL, TO_CHAR );
               act( AT_MAGIC, "$n's $t grows back!", victim, part_locs[part->loc], NULL, TO_ROOM );
               part->flags = PART_WELL;
               return;  /* No more healing after a part is regened */
            }
            else
            {
               part->cond = UMIN( PART_WELL, part->cond + power / 10 );
            }
         }
         if( victim->hit > victim->max_hit )
            victim->hit = victim->max_hit;
         break;

      case MAG_LIGHT:
         if( power > TALENT( ch, TAL_ILLUSION ) )
         {
            ch->pcdata->wizinvis = TALENT( ch, TAL_ILLUSION );
         }
         else
         {
            ch->pcdata->wizinvis = power;
         }
         if( xIS_SET( ch->act, PLR_WIZINVIS ) )
         {
            xREMOVE_BIT( ch->act, PLR_WIZINVIS );
            act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You slowly fade back into existence.\n\r", ch );
         }
         else
         {
            if( ch->pcdata->wizinvis < 1 || ch->pcdata->wizinvis > TALENT( ch, TAL_ILLUSION ) )
               ch->pcdata->wizinvis = TALENT( ch, TAL_ILLUSION );
            act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You slowly vanish into thin air.\n\r", ch );
            xSET_BIT( ch->act, PLR_WIZINVIS );
         }
         break;

      case MAG_POISON:
         if( ch->curr_talent[TAL_HEALING] < 15 )
         {
            ch_trick( victim, type );
            return;
         }
         if( !is_affected( victim, gsn_poison ) )
         {
            ch_trick( victim, type );
            return;
         }
         affect_strip( victim, gsn_poison );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "A warm feeling runs through your body.\n\r", victim );
         victim->mental_state = URANGE( -100, victim->mental_state, -10 );
         act( AT_MAGIC, "A flush of health washes over $N.", ch, NULL, victim, TO_NOTVICT );
         if( ch != victim )
         {
            act( AT_MAGIC, "You lift the poison from $N's body.", ch, NULL, victim, TO_CHAR );
         }
         break;

      case MAG_DRAIN:
         if( ch->curr_talent[TAL_HEALING] < 60 )
         {
            ch_trick( victim, type );
            return;
         }
         if( !is_affected( victim, gsn_mummy_rot ) )
         {
            ch_trick( victim, type );
            return;
         }
         affect_strip( victim, gsn_mummy_rot );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "Your flesh feels more resilient.\n\r", victim );
         victim->mental_state = URANGE( -100, victim->mental_state, -10 );
         act( AT_MAGIC, "$N's flesh stops rotting.", ch, NULL, victim, TO_NOTVICT );
         if( ch != victim )
         {
            act( AT_MAGIC, "You cure the rotting disease from $N's body.", ch, NULL, victim, TO_CHAR );
         }
         break;

      case MAG_SEEK:
         if( ch->curr_talent[TAL_HEALING] < 25 && ch->curr_talent[TAL_SEEKING] < 25 )
         {
            ch_trick( victim, type );
            return;
         }
         if( !is_affected( victim, gsn_blindness ) )
         {
            ch_trick( victim, type );
            return;
         }
         affect_strip( victim, gsn_blindness );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "Your vision returns!\n\r", victim );
         if( ch != victim )
            send_to_char( "You work your cure, restoring vision.\n\r", ch );
         break;

      case MAG_CHANGE:
         if( ch->curr_talent[TAL_HEALING] < 25 && ch->curr_talent[TAL_CHANGE] < 25 )
         {
            ch_trick( victim, type );
            return;
         }
         if( !is_affected( victim, gsn_corruption ) )
         {
            ch_trick( victim, type );
            return;
         }
         affect_strip( victim, gsn_corruption );
         set_char_color( AT_MAGIC, victim );
         send_to_char( "Waves of magic cleanse your body.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You cleanse $N's body of corruption.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N's body returns to normal again.", ch, NULL, victim, TO_NOTVICT );
         break;

      case MAG_FIRE:
         if( ch->curr_talent[TAL_FIRE] < 50 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_fire;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_RESISTANT;
         af.modifier = RIS_FIRE;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         send_to_char( "You are protected from heat.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You protect $N from heat.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N is protected from heat.", ch, NULL, victim, TO_NOTVICT );
         break;

      case MAG_COLD:
         if( ch->curr_talent[TAL_FROST] < 50 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_frost;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_RESISTANT;
         af.modifier = RIS_COLD;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         update_pos( victim );
         send_to_char( "You are protected from cold.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You protect $N from cold.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N is protected from cold.", ch, NULL, victim, TO_NOTVICT );
         break;

      case MAG_PSIONIC:
         if( ch->curr_talent[TAL_MIND] < 50 )
         {
            ch_trick( victim, type );
            return;
         }
         victim->mental_state = 0;
         act( AT_MAGIC, "You balance $N's mind.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "Your mind is balanced.", ch, NULL, victim, TO_VICT );
         break;

      case MAG_ACID:
         if( ch->curr_talent[TAL_HEALING] < 20 && ch->curr_talent[TAL_WATER] < 20 )
         {
            ch_trick( victim, type );
            return;
         }
         if( victim->pcdata && victim->pcdata->condition[COND_DRUNK] <= 0 )
         {
            ch_trick( victim, type );
            return;
         }
         if( victim->pcdata )
            victim->pcdata->condition[COND_DRUNK] = 0;
         act( AT_MAGIC, "You purge the alcohol from $N's system.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n purges the alcohol from your system.", ch, NULL, victim, TO_VICT );
         break;

      case MAG_TELEKINETIC:
         if( ch->curr_talent[TAL_HEALING] < 10 )
         {
            ch_trick( victim, type );
            return;
         }
         if( victim->move >= victim->max_move )
         {
            ch_trick( victim, type );
            return;
         }
         victim->move = UMAX( victim->move + power, victim->max_move );
         update_pos( victim );
         send_to_char( "Vitality and stamina return to you.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You let vitality flow into $N's body.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N's body seems to grow less exhausted.", ch, NULL, victim, TO_NOTVICT );
         break;

      case MAG_BLUNT:
         if( ch->curr_talent[TAL_EARTH] < 30 )
         {
            ch_trick( victim, type );
            return;
         }
         if( IS_NPC( victim ) || victim->pcdata->condition[COND_FULL] < 0 )
         {
            ch_trick( victim, type );
            return;
         }

         victim->pcdata->condition[COND_FULL] += 20;
         send_to_char( "You are satiated.", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You fill $N's stomach.", ch, NULL, victim, TO_CHAR );
         }
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_HEALING] < 30 )
         {
            ch_trick( victim, type );
            return;
         }
         if( !is_affected( victim, gsn_curse ) )
         {
            ch_trick( victim, type );
            return;
         }
         affect_strip( victim, gsn_curse );
         send_to_char( "The weight of your curse is lifted.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You dispel the curse afflicting $N.", ch, NULL, victim, TO_CHAR );
            act( AT_MAGIC, "$n's dispels the curse afflicting $N.", ch, NULL, victim, TO_NOTVICT );
         }
         break;

      case MAG_WATER:
         if( ch->curr_talent[TAL_WATER] < 50 )
         {
            ch_trick( victim, type );
            return;
         }
         if( IS_NPC( victim ) || victim->pcdata->condition[COND_THIRST] < 0 )
         {
            ch_trick( victim, type );
            return;
         }

         victim->pcdata->condition[COND_THIRST] += 10;
         act( AT_MAGIC, "You feel bloated.", ch, NULL, victim, TO_VICT );
         act( AT_MAGIC, "You fill $N up with water.", ch, NULL, victim, TO_CHAR );
         break;

   }
}

void obj_hinder( CHAR_DATA * ch, OBJ_DATA * obj, int type )
{
   bool found;
   AFFECT_DATA *paf;

   found = FALSE;
   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         return;

      case MAG_HOLY:
         if( obj->item_type != ITEM_ARMOR )
         {
            obj_trick( ch, obj, type );
            break;
         }
         if( TALENT( ch, TAL_SECURITY ) / 10 > ( obj->pIndexData->value[0] - obj->value[0] ) )
         {
            act( AT_WHITE, "You reduce the protective ability of $p.", ch, obj, NULL, TO_CHAR );
            obj->value[0]--;
         }
         else
         {
            obj_trick( ch, obj, type );
            return;
         }
         break;

      case MAG_EARTH:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_CON && !found )
            {
               if( TALENT( ch, TAL_EARTH ) / 10 > -1 * paf->modifier )
               {
                  act( AT_GREEN, "You hinder $p with the strength of the Earth.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_CON;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_GREEN, "You hinder $p with the strength of the Earth.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_COLD:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_LCK && !found )
            {
               if( TALENT( ch, TAL_FROST ) / 10 > -1 * paf->modifier )
               {
                  act( AT_FROST, "You hinder $p with the strength of the Ice.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_LCK;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_FROST, "You hinder $p with the strength of the Ice.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_FIRE:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_STR && !found )
            {
               if( TALENT( ch, TAL_FIRE ) / 10 > -1 * paf->modifier )
               {
                  act( AT_FIRE, "You hinder $p with the strength of the Flames.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_STR;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_FIRE, "You hinder $p with the strength of the Flames.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_SEEK:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_CHA && !found )
            {
               if( TALENT( ch, TAL_SEEKING ) / 10 > -1 * paf->modifier )
               {
                  act( AT_GREEN, "You hinder $p with the power of Seeking.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_CHA;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_GREEN, "You hinder $p with the power of Seeking.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_WIND:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_DEX && !found )
            {
               if( TALENT( ch, TAL_WIND ) / 10 > -1 * paf->modifier )
               {
                  act( AT_CYAN, "You hinder $p with the speed of the Wind.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_DEX;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_CYAN, "You hinder $p with the speed of the Wind.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_WATER:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_WIS && !found )
            {
               if( TALENT( ch, TAL_WATER ) / 10 > -1 * paf->modifier )
               {
                  act( AT_BLUE, "You hinder $p with the power of Water.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_WIS;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_BLUE, "You hinder $p with the power of Water.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_PSIONIC:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_INT && !found )
            {
               if( TALENT( ch, TAL_MIND ) / 10 > -1 * paf->modifier )
               {
                  act( AT_PURPLE, "You hinder $p with the power of the Mind.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_INT;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_PURPLE, "You hinder $p with the power of the Mind.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_ELECTRICITY:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_DAMROLL && !found )
            {
               if( TALENT( ch, TAL_LIGHTNING ) / 10 > -1 * paf->modifier )
               {
                  act( AT_YELLOW, "You hinder $p with the strength of the Storm.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_DAMROLL;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_YELLOW, "You hinder $p with the strength of the Storm.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_TELEKINETIC:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_HITROLL && !found )
            {
               if( TALENT( ch, TAL_MOTION ) / 10 > -1 * paf->modifier )
               {
                  act( AT_YELLOW, "You hinder $p with the speed of Motion.", ch, obj, NULL, TO_CHAR );
                  paf->modifier--;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_HITROLL;
            paf->modifier = -1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_YELLOW, "You hinder $p with the speed of Motion.", ch, obj, NULL, TO_CHAR );
         }
         break;
   }
   WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
   obj->mana += 50;
   obj->cost -= 50000;
}

void ch_hinder( CHAR_DATA * victim, CHAR_DATA * ch, int type, int power )
{
   AFFECT_DATA af;
   int sn;

   switch ( type )
   {
      default:
         ch_trick( victim, type );
         break;

      case MAG_EARTH:
         if( ch->in_room->curr_vegetation < 100 - ch->curr_talent[TAL_EARTH] )
         {
            ch_trick( victim, type );
            return;
         }

         if( ch->curr_talent[TAL_EARTH] > number_range( 10, 50 ) )
         {
            act( AT_GREEN, "You are entangled in vines!", ch, NULL, victim, TO_VICT );
            act( AT_GREEN, "$N is entangled in vines!", ch, NULL, victim, TO_NOTVICT );
            act( AT_GREEN, "You entangle $N in vines!", ch, NULL, victim, TO_CHAR );
            if( IS_NPC( victim ) )
               xSET_BIT( victim->affected_by, AFF_HOLD );
            else
               xSET_BIT( victim->pcdata->perm_aff, AFF_HOLD );
         }
         else
         {
            act( AT_GREEN, "Vines grab for you but fail to hold you!", ch, NULL, victim, TO_VICT );
            act( AT_GREEN, "Your vines fail to hold $N!", ch, NULL, victim, TO_CHAR );
         }
         break;

      case MAG_COLD:
         if( ch->in_room->area->weather->temp > ch->curr_talent[TAL_FROST] * 10 - 400 )
         {
            ch_trick( victim, type );
            return;
         }

         if( ch->curr_talent[TAL_FROST] > number_range( 10, 50 ) )
         {
            act( AT_FROST, "Ice forms around you, holding you in place!", ch, NULL, victim, TO_VICT );
            act( AT_FROST, "Ice forms around $N!", ch, NULL, victim, TO_NOTVICT );
            act( AT_FROST, "You hold $N in ice!", ch, NULL, victim, TO_CHAR );
            if( IS_NPC( victim ) )
               xSET_BIT( victim->affected_by, AFF_HOLD );
            else
               xSET_BIT( victim->pcdata->perm_aff, AFF_HOLD );
         }
         else
         {
            act( AT_FROST, "Ice forms around you but fails to hold you!", ch, NULL, victim, TO_VICT );
            act( AT_FROST, "Your ice fails to hold $N!", ch, NULL, victim, TO_CHAR );
         }
         break;

      case MAG_WATER:
         if( !IS_UNDERWATER( victim ) )
         {
            if( ch->curr_talent[TAL_WATER] < 60 )
            {
               ch_trick( victim, type );
               return;
            }
            if( IS_NPC( victim ) || victim->pcdata->condition[COND_THIRST] < 0 )
            {
               ch_trick( victim, type );
               return;
            }

            victim->pcdata->condition[COND_THIRST] = 0;
            act( AT_MAGIC, "You are DYING of THIRST!", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "You drain all the water from $N's body.", ch, NULL, victim, TO_CHAR );
            break;
         }
         if( ch->curr_talent[TAL_WATER] > number_range( 10, 50 ) )
         {
            act( AT_MAGIC, "A whirlpool spins around you, holding you in place!", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "$N is held in a whirlpool!", ch, NULL, victim, TO_NOTVICT );
            act( AT_MAGIC, "You hold $N in a whirlpool!", ch, NULL, victim, TO_CHAR );
            if( IS_NPC( victim ) )
               xSET_BIT( victim->affected_by, AFF_HOLD );
            else
               xSET_BIT( victim->pcdata->perm_aff, AFF_HOLD );
         }
         else
         {
            act( AT_MAGIC, "A whirlpool spins near you, but you swim away!", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "Your whirlpool fails to hold $N!", ch, NULL, victim, TO_CHAR );
         }
         break;

      case MAG_TIME:
         if( ch->curr_talent[TAL_TIME] > number_range( 10, 50 ) )
         {
            act( AT_MAGIC, "You are held in place!", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "$N's feet are held to the floor!", ch, NULL, victim, TO_NOTVICT );
            act( AT_MAGIC, "You hold $N in place!", ch, NULL, victim, TO_CHAR );
            if( IS_NPC( victim ) )
               xSET_BIT( victim->affected_by, AFF_HOLD );
            else
               xSET_BIT( victim->pcdata->perm_aff, AFF_HOLD );
         }
         else
         {
            act( AT_MAGIC, "Something grabs at you, but fails to hold you!", ch, NULL, victim, TO_VICT );
            act( AT_MAGIC, "Your magic fails to hold $N!", ch, NULL, victim, TO_CHAR );
         }
         break;

      case MAG_ANTIMAGIC:
         if( victim != ch && ch->curr_talent[TAL_VOID] < 30 )
         {
            ch_trick( victim, type );
            return;
         }

         if( TALENT( victim, TAL_SECURITY ) > TALENT( ch, TAL_VOID ) && ch != victim && !IS_CONSENTING( victim, ch ) )
         {
            act( AT_MAGIC, "$N's magic wavers but holds.", ch, NULL, victim, TO_CHAR );
            act( AT_MAGIC, "Your magic wavers but holds.", ch, NULL, victim, TO_VICT );
            learn_talent( victim, TAL_SECURITY );
            return;
         }

         if( victim->first_affect )
         {
            send_to_char( "Your enchantments are stripped away by powerful magic.\n\r", victim );
            if( ch != victim )
            {
               act( AT_MAGIC, "You strip away $N's enchantments.", ch, NULL, victim, TO_CHAR );
            }
            while( victim->first_affect )
               affect_remove( victim, victim->first_affect );
            WAIT_STATE( ch, PULSE_VIOLENCE );
         }
         else
         {
            ch_trick( victim, type );
            return;
         }
         break;

      case MAG_FIRE:
         if( ch->curr_talent[TAL_FIRE] < 30 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_sticky_flame;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_SET( victim->immune, RIS_FIRE ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power / 10;
         af.location = APPLY_DAMROLL;
         af.modifier = TALENT( ch, TAL_FIRE ) / 40;
         xCLEAR_BITS( af.bitvector );
         xSET_BIT( af.bitvector, AFF_FLAMING );
         affect_join( victim, &af );
         send_to_char( "Sticky flames cover your body!\n\r", victim );
         if( ch != victim )
            act( AT_FIRE, "You cover $N in sticky fire!", ch, NULL, victim, TO_CHAR );
         break;

      case MAG_SEEK:
         if( ch->curr_talent[TAL_ILLUSION] < 30 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_blindness;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_AFFECTED( victim, AFF_TRUESIGHT ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power / 10;
         af.location = APPLY_HITROLL;
         af.modifier = -1 * TALENT( ch, TAL_ILLUSION ) / 20;
         xCLEAR_BITS( af.bitvector );
         xSET_BIT( af.bitvector, AFF_BLIND );
         affect_join( victim, &af );
         send_to_char( "You are blinded by a flash of light!\n\r", victim );
         if( ch != victim )
            act( AT_YELLOW, "You blind $N with a flash of light!", ch, NULL, victim, TO_CHAR );
         break;

      case MAG_POISON:
         if( ch->curr_talent[TAL_DEATH] < 30 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_poison;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_SET( victim->immune, RIS_POISON ) || IS_AFFECTED( victim, AFF_NONLIVING ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power / 10;
         af.location = APPLY_STR;
         af.modifier = -1 * TALENT( ch, TAL_DEATH ) / 40;
         xCLEAR_BITS( af.bitvector );
         xSET_BIT( af.bitvector, AFF_POISON );
         affect_join( victim, &af );
         victim->mental_state = URANGE( 10, victim->mental_state + 10, 100 );
         send_to_char( "You feel very sick.\n\r", victim );
         if( ch != victim )
            act( AT_DGREEN, "You spread poison into $N's body.", ch, NULL, victim, TO_CHAR );
         act( AT_DGREEN, "$N begins to look very sick.", ch, NULL, victim, TO_ROOM );
         break;

      case MAG_DRAIN:
         if( ch->curr_talent[TAL_DEATH] < 60 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_mummy_rot;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_SET( victim->immune, RIS_DRAIN ) || IS_AFFECTED( victim, AFF_NONLIVING ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_CON;
         af.modifier = -1 * TALENT( ch, TAL_DEATH ) / 50;
         xCLEAR_BITS( af.bitvector );
         xSET_BIT( af.bitvector, AFF_ROTTING );
         affect_join( victim, &af );
         send_to_char( "Your flesh begins to rot away.\n\r", victim );
         if( ch != victim )
            act( AT_DGREY, "You make $N's flesh begin to rot away.", ch, NULL, victim, TO_CHAR );
         act( AT_DGREY, "$N's flesh begins to rot away.", ch, NULL, victim, TO_ROOM );
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_DEATH] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_curse;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_HITROLL;
         af.modifier = -1 * TALENT( ch, TAL_DEATH ) / 50;
         xCLEAR_BITS( af.bitvector );
         xSET_BIT( af.bitvector, AFF_CURSE );
         affect_join( victim, &af );
         send_to_char( "You feel unlucky.\n\r", victim );
         if( ch != victim )
            act( AT_DGREY, "You inflict a curse upon $N.", ch, NULL, victim, TO_CHAR );
         act( AT_DGREY, "A malignant aura surrounds $N for a moment.", ch, NULL, victim, TO_ROOM );
         break;

      case MAG_DREAM:
         sn = gsn_sleep;
         if( ch->curr_talent[TAL_DREAM] < 30 )
         {
            ch_trick( victim, type );
            return;
         }
         if( !IS_NPC( victim ) && IS_FIGHTING( victim ) )
         {
            ch_trick( victim, type );
            return;
         }
         if( IS_SET( victim->immune, RIS_SLEEP ) )
         {
            ch_trick( victim, type );
            return;
         }
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         if( number_range( 1, get_curr_wil( ch ) )
             < number_range( 1, get_curr_wil( victim ) + TALENT( victim, TAL_SECURITY ) ) )
         {
            ch_trick( victim, type );
            return;
         }
         if( !IS_FIGHTING( victim ) )
         {
            victim->last_hit = ch;
            ch->last_hit = victim;
         }
         else
         {
            ch_trick( victim, type );
            return;
         }

         af.type = sn;
         af.duration = power / 20;
         af.location = APPLY_NONE;
         af.modifier = 0;
         af.bitvector = meb( AFF_SLEEP );
         affect_join( victim, &af );

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
         break;

      case MAG_BLUNT:
         if( ch->curr_talent[TAL_EARTH] < 60 )
         {
            ch_trick( victim, type );
            return;
         }
         if( IS_NPC( victim ) || victim->pcdata->condition[COND_FULL] < 0 )
         {
            ch_trick( victim, type );
            return;
         }

         victim->pcdata->condition[COND_FULL] = 0;
         act( AT_HUNGRY, "You are STARVING!", ch, NULL, victim, TO_VICT );
         act( AT_MAGIC, "You reach out your hand, and $N's body shrivels.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n reaches out $s hand, and $N's body shrivels.", ch, NULL, victim, TO_NOTVICT );
         break;

      case MAG_SOUND:
         if( ch->curr_talent[TAL_SPEECH] < 60 )
         {
            ch_trick( victim, type );
            return;
         }

         stop_fighting( ch, TRUE );
         act( AT_SAY, "You hum a tranquil melody to $N.", ch, NULL, victim, TO_CHAR );
         act( AT_SAY, "$n hums a tranquil melody to $N.", ch, NULL, victim, TO_NOTVICT );
         act( AT_SAY, "$n hums a tranquil melody, and you feel at peace.", ch, NULL, victim, TO_VICT );
         break;

      case MAG_PSIONIC:
         if( ch->curr_talent[TAL_MIND] < 50 )
         {
            ch_trick( victim, type );
            return;
         }

         sn = gsn_mind;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_MENTALSTATE;
         af.modifier = ch->curr_talent[TAL_MIND];
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         send_to_char( "Suddenly the world seems strange to you as you begin to hallucinate...\n\r", victim );
         if( ch != victim )
            act( AT_DGREY, "You fill $N's mind with rampant hallucinations.", ch, NULL, victim, TO_CHAR );
         act( AT_DGREY, "$N's eyes grow wide and $E starts hallucinating.", ch, NULL, victim, TO_ROOM );
         break;

      case MAG_HOLY:
         if( ch->curr_talent[TAL_SECURITY] < 50 )
         {
            ch_trick( victim, type );
            return;
         }

         sn = gsn_security;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_AC;
         af.modifier = -1 * TALENT( ch, TAL_SECURITY ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         send_to_char( "You feel vulnerable.\n\r", victim );
         if( ch != victim )
            act( AT_DGREY, "You make $N vulnerable.", ch, NULL, victim, TO_CHAR );
         act( AT_DGREY, "$N seems naked for a moment as $E becomes vulnerable.", ch, NULL, victim, TO_ROOM );
         break;

   }
}

void room_hinder( CHAR_DATA * ch, ROOM_INDEX_DATA * room, int type, int power, char *argument )
{
   EXIT_DATA *pexit;

   switch ( type )
   {
      default:
         ch_trick( ch, type );
         break;

      case MAG_EARTH:
         if( ch->curr_talent[TAL_DEATH] < 20 )
         {
            ch_trick( ch, type );
            return;
         }
         if( ch->in_room->curr_vegetation <= 0 )
         {
            ch_trick( ch, type );
            return;
         }
         ch->in_room->curr_vegetation -= 10;
         act( AT_HUNGRY, "You touch the plants in the room, and they begin to wither.", ch, NULL, NULL, TO_CHAR );
         act( AT_HUNGRY, "The vegetation in the room withers.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_WATER:
         if( ch->curr_talent[TAL_FIRE] < 20 )
         {
            ch_trick( ch, type );
            return;
         }
         if( ch->in_room->curr_water <= 0 )
         {
            ch_trick( ch, type );
            return;
         }
         ch->in_room->curr_water -= 10;
         act( AT_THIRSTY, "Water drains out of the room.", ch, NULL, NULL, TO_CHAR );
         act( AT_THIRSTY, "Water begins to drain out of the room.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_FIRE:
         if( ch->curr_talent[TAL_FIRE] < 60 )
         {
            ch_trick( ch, type );
            return;
         }
         if( ch->in_room->curr_vegetation < 10 )
         {
            ch_trick( ch, type );
            return;
         }
         if( ch->in_room->curr_water > 30 )
         {
            ch_trick( ch, type );
            return;
         }
         act( AT_FIRE, "You set fire to the room!", ch, NULL, NULL, TO_CHAR );
         act( AT_FIRE, "$n sets fire to the room!", ch, NULL, NULL, TO_ROOM );
         SET_BIT( ch->in_room->room_flags, ROOM_BURNING );
         break;

      case MAG_HOLY:
         if( !( pexit = find_door( ch, argument, FALSE ) )
             || !IS_SET( pexit->exit_info, EX_CLOSED )
             || !IS_SET( pexit->exit_info, EX_LOCKED ) || IS_SET( pexit->exit_info, EX_PICKPROOF ) )
         {
            ch_trick( ch, type );
            return;
         }
         REMOVE_BIT( pexit->exit_info, EX_LOCKED );
         send_to_char( "*Click*\n\r", ch );
         if( pexit->rexit && pexit->rexit->to_room == ch->in_room )
            REMOVE_BIT( pexit->rexit->exit_info, EX_LOCKED );
         check_room_for_traps( ch, TRAP_UNLOCK | trap_door[pexit->vdir] );
         break;
   }
}

void void_create( CHAR_DATA * ch, int power, char *argument )
{
   MATERIAL_DATA *material_lookup( int number );
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_DATA *obj = NULL;
   extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
   OBJ_INDEX_DATA *pObjIndex;
   AREA_DATA *area;
   int lo_vnum = 2;
   int hi_vnum = 2;
   MATERIAL_DATA *material;
   int hash;
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   extern int top_affect;

   if( ch->curr_talent[TAL_VOID] < 100 )
   {
      huh( ch );
      return;
   }

   if( argument == NULL )
   {
      ch_trick( ch, MAG_ETHER );
      return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   material = material_lookup( 116 );  /* Ether material */

   if( !( material ) )
   {
      send_to_char( "You can't seem to reach the Ether for some reason.\n\r", ch );
      bug( "Cannot find Ether material for Void Create!", 0 );
      return;
   }

   if( !stockobj )
      for( area = first_area; area; area = area->next )
      {
         if( !str_cmp( area->filename, "stockobj.are" ) )
         {
            stockobj = area;
            lo_vnum = area->low_o_vnum;
            hi_vnum = area->hi_o_vnum;
            break;
         }
         else
         {
            lo_vnum = 2;
            hi_vnum = 2;
         }
      }
   else
   {
      area = stockobj;
      lo_vnum = area->low_o_vnum;
      hi_vnum = area->hi_o_vnum;
   }
   if( lo_vnum == hi_vnum )
   {
      send_to_char( "You cannot figure out how to create anything right now.\n\r", ch );
      return;
   }

   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
   {
      for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
      {
         if( pObjIndex == NULL )
         {
            bug( "NULL pObjIndex on void_create!" );
            send_to_char( "Your mind is a blank.\n\r", ch );
            return;
         }
         if( nifty_is_name( arg1, pObjIndex->name ) && pObjIndex->vnum >= lo_vnum && pObjIndex->vnum <= hi_vnum )
         {

            if( pObjIndex->tech > ch->pcdata->noncombat[SK_SMITH] + ch->curr_talent[TAL_VOID] - 100 )
            {
               ch_trick( ch, MAG_ETHER );
               return;
            }

            if( pObjIndex->weight * 10 > power && !IS_AFFECTED( ch, AFF_VOID ) )
            {
               ch_trick( ch, MAG_ETHER );
               return;
            }

            obj = create_object( pObjIndex, 0 );
            if( !obj )
            {
               ch_trick( ch, MAG_ETHER );
               return;
            }
            break;
         }
      }
      if( obj )
         break;
   }

   if( obj == NULL )
   {
      ch_trick( ch, MAG_ETHER );
      return;
   }

   strcpy( buf, material->name );
   sprintf( arg, obj->short_descr, buf );
   one_argument( arg, buf );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( arg );

   strcpy( buf, material->name );
   sprintf( arg, obj->description, buf );
   STRFREE( obj->description );
   obj->description = STRALLOC( capitalize( arg ) );

   strcpy( buf, obj->name );
   strcat( buf, " " );
   strcat( buf, material->name );
   STRFREE( obj->name );
   obj->name = STRALLOC( buf );

   if( obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON )
      obj->value[2] = TALENT( ch, TAL_VOID );

   /*
    * This is important so that Void mages cannot create
    * * infinite free items and sell them.
    * * Enchanting them will still add value, however,
    * * but that requires actual work, so lets let them... -keo
    */
   obj->cost = 0;

   for( paf = material->first_affect; paf; paf = paf->next )
   {
      AFFECT_DATA *naf;

      CREATE( naf, AFFECT_DATA, 1 );

      naf->type = paf->type;
      naf->duration = paf->duration;
      naf->location = paf->location;
      naf->modifier = paf->modifier;
      naf->bitvector = paf->bitvector;
      top_affect++;

      LINK( naf, obj->first_affect, obj->last_affect, next, prev );
   }

   obj->size = ch->height;
   obj->material = material;
   obj->obj_by = STRALLOC( ch->name );
   obj = obj_to_char( obj, ch );
   act( AT_MAGIC, "You weave $p from ether.", ch, obj, NULL, TO_CHAR );
   act( AT_MAGIC, "$n weaves $p from ether.", ch, obj, NULL, TO_ROOM );
}

void dream_create( CHAR_DATA * ch, int power, char *arg1, char *arg2 )
{
   MATERIAL_DATA *material_lookup( int number );
   OBJ_DATA *obj = NULL;
   extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
   OBJ_INDEX_DATA *pObjIndex;
   AREA_DATA *area;
   int lo_vnum = 2;
   int hi_vnum = 2;
   MATERIAL_DATA *material;
   int hash;
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   extern int top_affect;

   if( !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
   {
      ch_trick( ch, MAG_DREAM );
      return;
   }

   if( arg1 == NULL || arg2 == NULL )
   {
      ch_trick( ch, MAG_DREAM );
      return;
   }

   material = first_material;
   while( material )
   {
      if( !str_prefix( material->name, arg2 ) )
         break;
      material = material->next;
   }

   if( !( material ) )
   {
      ch_trick( ch, MAG_DREAM );
      return;
   }

   if( !stockobj )
      for( area = first_area; area; area = area->next )
      {
         if( !str_cmp( area->filename, "stockobj.are" ) )
         {
            stockobj = area;
            lo_vnum = area->low_o_vnum;
            hi_vnum = area->hi_o_vnum;
            break;
         }
         else
         {
            lo_vnum = 2;
            hi_vnum = 2;
         }
      }
   else
   {
      area = stockobj;
      lo_vnum = area->low_o_vnum;
      hi_vnum = area->hi_o_vnum;
   }

   if( lo_vnum == hi_vnum )
   {
      ch_trick( ch, MAG_DREAM );
      return;
   }

   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
   {
      for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
      {
         if( pObjIndex == NULL )
         {
            bug( "NULL pObjIndex on spell_imagine!" );
            send_to_char( "Your mind is a blank.\n\r", ch );
            return;
         }
         if( nifty_is_name( arg1, pObjIndex->name ) && pObjIndex->vnum >= lo_vnum && pObjIndex->vnum <= hi_vnum )
         {
            obj = create_object( pObjIndex, 0 );
            if( !obj )
            {
               ch_trick( ch, MAG_DREAM );
               return;
            }
            break;
         }
      }
      if( obj )
         break;
   }

   if( obj == NULL )
   {
      ch_trick( ch, MAG_DREAM );
      return;
   }

   strcpy( buf, material->name );
   sprintf( arg, obj->short_descr, buf );
   one_argument( arg, buf );
   if( strcmp( buf, "some" ) )
      strcpy( arg, aoran( arg ) );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( arg );

   strcpy( buf, material->name );
   sprintf( arg, obj->description, buf );
   if( strcmp( buf, "some" ) )
      strcpy( arg, aoran( arg ) );
   STRFREE( obj->description );
   obj->description = STRALLOC( capitalize( arg ) );

   strcpy( buf, obj->name );
   strcat( buf, " " );
   strcat( buf, material->name );
   STRFREE( obj->name );
   obj->name = STRALLOC( buf );

   obj->cost += number_fuzzy( obj->cost );

   for( paf = material->first_affect; paf; paf = paf->next )
   {
      AFFECT_DATA *naf;

      CREATE( naf, AFFECT_DATA, 1 );

      naf->type = paf->type;
      naf->duration = paf->duration;
      naf->location = paf->location;
      naf->modifier = paf->modifier;
      naf->bitvector = paf->bitvector;
      top_affect++;

      LINK( naf, obj->first_affect, obj->last_affect, next, prev );
   }
   obj->material = material;
   obj->size = ch->height;
   obj->obj_by = STRALLOC( ch->name );
   obj = obj_to_char( obj, ch );
   act( AT_MAGIC, "You imagine $p.", ch, obj, NULL, TO_CHAR );
   act( AT_MAGIC, "$n concentrates, and $p appears in $s hands.", ch, obj, NULL, TO_ROOM );
   xSET_BIT( obj->extra_flags, ITEM_DREAMWORLD );
}

void obj_create( CHAR_DATA * ch, int type, int power, char *argument )
{
   int create, x, y;
   OBJ_DATA *obj;
   MATERIAL_DATA *material;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   material = material_lookup( 116 );  /* Ether material */

   if( !( material ) )
   {
      send_to_char( "You can't seem to reach the Ether for some reason.\n\r", ch );
      bug( "Cannot find Ether material for Void Create!", 0 );
      return;
   }

   switch ( type )
   {
      default:
         ch_trick( ch, type );
         return;

      case MAG_FIRE:
         if( IS_UNDERWATER( ch ) || ch->curr_talent[TAL_FIRE] < 10 )
         {
            ch_trick( ch, MAG_WATER );
            return;
         }

         act( AT_FIRE, "You make a burst of mystic flame burn before you.", ch, NULL, NULL, TO_CHAR );
         act( AT_FIRE, "$n makes a burst of mystic flame burn before $m.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_FIRE;
         break;

      case MAG_WATER:
         if( IS_UNDERWATER( ch ) || ch->curr_talent[TAL_WATER] < 10 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_BLUE, "A spring of clear water gushes forth in midair.", ch, NULL, NULL, TO_CHAR );
         act( AT_BLUE, "A spring of clear water gushes forth in midair.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_SPRING;
         break;

      case MAG_ACID:
         if( ch->curr_talent[TAL_WATER] < 20 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_BLUE, "You conjure a hot cup of tea.", ch, NULL, NULL, TO_CHAR );
         act( AT_BLUE, "$n conjures a hot cup of tea.", ch, NULL, NULL, TO_ROOM );
         create = 54;
         break;

      case MAG_BLUNT:
         if( ch->curr_talent[TAL_EARTH] < 10 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_HUNGRY, "You conjure a loaf of bread.", ch, NULL, NULL, TO_CHAR );
         act( AT_HUNGRY, "$n conjures a loaf of bread.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_MUSHROOM;
         break;

      case MAG_EARTH:
         if( ch->curr_talent[TAL_EARTH] < 10 )
         {
            ch_trick( ch, type );
            return;
         }
         act( AT_RED, "You open your hand and a beautiful red rose grows and opens.", ch, NULL, NULL, TO_CHAR );
         act( AT_RED, "$n opens $s hand and a beautiful red rose grows and opens.", ch, NULL, NULL, TO_ROOM );
         create = 53;
         break;

      case MAG_DRAIN:
         if( ch->curr_talent[TAL_MOTION] < 10 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_PURPLE, "You conjure a mystic shovel.", ch, NULL, NULL, TO_CHAR );
         act( AT_PURPLE, "$n conjures a mystic shovel.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_MYSTIC_SHOVEL;
         break;

      case MAG_ANTIMATTER:
         if( ch->curr_talent[TAL_VOID] < 30 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_MAGIC, "You weave a bag of holding from the Void.", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n weaves a bag of holding from the Void.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_BAG_OF_HOLDING;
         break;

      case MAG_LIGHT:
         if( ch->curr_talent[TAL_ILLUSION] < 10 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_YELLOW, "You conjure a globe of light.", ch, NULL, NULL, TO_CHAR );
         act( AT_YELLOW, "$n conjures a globe of light.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_LIGHT_BALL;
         break;

      case MAG_POISON:
      case MAG_CHANGE:
         if( ch->curr_talent[TAL_DEATH] < 10 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_CHAOS, "You conjure a piece of chaos cheese.", ch, NULL, NULL, TO_CHAR );
         act( AT_CHAOS, "$n conjures a piece of chaos cheese.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_CHAOS_CHEESE;
         break;

      case MAG_SOUND:
         if( ch->curr_talent[TAL_SPEECH] < 10 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_MAGIC, "You conjure a magic marker.", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n conjures a magic marker.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_MAGIC_MARKER;
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_DEATH] < 10 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_DGREY, "You conjure a globe of darkness.", ch, NULL, NULL, TO_CHAR );
         act( AT_DGREY, "$n conjures a globe of darkness.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_BALL_DARK;
         break;

      case MAG_PSIONIC:
         if( ch->curr_talent[TAL_MIND] < 20 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_BLUE, "You conjure a bottle of wine.", ch, NULL, NULL, TO_CHAR );
         act( AT_BLUE, "$n conjures a bottle of wine.", ch, NULL, NULL, TO_ROOM );
         create = OBJ_VNUM_WINE;
         break;

      case MAG_SEEK:
         if( ch->curr_talent[TAL_SEEKING] < 100 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_BLUE, "You conjure a reflecting pool.", ch, NULL, NULL, TO_CHAR );
         act( AT_BLUE, "$n conjures a reflecting pool.", ch, NULL, NULL, TO_ROOM );
         create = 9006;
         break;

      case MAG_SLASH:
         if( ch->curr_talent[TAL_CATALYSM] < 30 )
         {
            ch_trick( ch, type );
            return;
         }

         act( AT_YELLOW, "You draw pure energy together to form a gleaming mageblade.", ch, NULL, NULL, TO_CHAR );
         act( AT_YELLOW, "$n draws $s hands together and forges a mageblade of raw energy.", ch, NULL, NULL, TO_ROOM );
         create = 98;
         break;

      case MAG_TELEKINETIC:
         if( ch->curr_talent[TAL_MOTION] < 40 )
         {
            ch_trick( ch, type );
            return;
         }
         if( IS_SET( ch->in_room->area->flags, AFLAG_NOBYPASS ) )
         {
            ch_trick( ch, type );
            return;
         }
         victim = generate_mob_nation( argument );
         if( !victim )
         {
            send_to_char( "You cannot find anything like that.\n\r", ch );
            return;
         }
         act( AT_MAGIC, "You open a swirling rift and pull $N through.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n opens a swirling rift and pulls $N through.", ch, NULL, victim, TO_ROOM );
         char_to_room( victim, ch->in_room );
         do_look( victim, "auto" );
         WAIT_STATE( ch, PULSE_VIOLENCE );
         return;

   }
   if( !get_obj_index( create ) )
   {
      bug( "obj_create: vnum %d not found!", create );
      send_to_char( "At the last minute, you lose your train of thought!\n\r", ch );
      return;
   }
   obj = create_object( get_obj_index( create ), 0 );
   if( !IS_OBJ_STAT( obj, ITEM_NO_TAKE ) )
   {
      obj = obj_to_char( obj, ch );
   }
   else
   {
      obj = obj_to_room( obj, ch->in_room );
   }
   obj->cost = 0;
   obj->obj_by = STRALLOC( ch->name );
   obj->timer = power / 10;
   obj->material = material;
   /*
    * Random mageblade appearances 
    */
   if( type == MAG_ALL )
   {
      x = number_range( 1, 11 );
      y = number_range( 1, 12 );
      sprintf( buf, "%s %s %s",
               x == 1 ? "gleaming" :
               x == 2 ? "shining" :
               x == 3 ? "blazing" :
               x == 4 ? "crackling" :
               x == 5 ? "sparkling" :
               x == 6 ? "fiery" :
               x == 7 ? "glimmering" :
               x == 8 ? "glistening" :
               x == 9 ? "glinting" :
               x == 10 ? "mystic" :
               "shimmering",
               y == 1 ? "&Rred" :
               y == 2 ? "&Wwhite" :
               y == 3 ? "&wsilver" :
               y == 4 ? "&Bblue" :
               y == 5 ? "&Ggreen" :
               y == 6 ? "&Pviolet" :
               y == 7 ? "&ppurple" :
               y == 8 ? "&Ppink" : y == 9 ? "&Ccyan" : y == 10 ? "&Rmu&Ylti-&Gco&Clo&Pred" : "&Ygolden", obj->short_descr );
      STRFREE( obj->short_descr );
      obj->short_descr = STRALLOC( buf );
      STRFREE( obj->name );
      obj->name = STRALLOC( buf );
   }
   WAIT_STATE( ch, PULSE_VIOLENCE );
}

void obj_find( CHAR_DATA * ch, OBJ_DATA * obj, int type )
{
   AFFECT_DATA *paf;
   SKILLTYPE *sktmp;
   MATERIAL_DATA *mat;
   int gp;

   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         break;

      case MAG_ALL:
      case MAG_ENERGY:
         if( ch->curr_talent[TAL_SEEKING] < 15 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( obj->first_affect || obj->pIndexData->first_affect )
            ch_printf( ch, "The %s has the following affects:\n\r", myobj( obj ) );
         else
            ch_printf( ch, "The %s has no inherent affects.\n\r", myobj( obj ) );

         for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
            showaffect( ch, paf );

         for( paf = obj->first_affect; paf; paf = paf->next )
            showaffect( ch, paf );

         if( obj->gem )
         {
            ch_printf( ch, "%s has been affixed to this item.\n\r", aoran( myobj( obj->gem ) ) );
            for( paf = obj->gem->first_affect; paf; paf = paf->next )
               showaffect( ch, paf );
         }
         break;

      case MAG_SEEK:
         ch_printf( ch, "\n\rThe %s is %s", myobj( obj ), aoran( item_type_name( obj ) ) );
         if( !xIS_EMPTY( obj->parts ) )
            ch_printf( ch, ", worn on:  %s\n\r", ext_flag_string( &obj->parts, bp_flags ) );
         else
            send_to_char( ".\n\r", ch );
         if( ch->curr_talent[TAL_SEEKING] >= 10 )
            ch_printf( ch,
                       "Special properties:  %s\n\rIts weight is %d.\r\n",
                       extra_bit_name( &obj->extra_flags ), obj->weight );
         if( obj->cost > 0 )
         {
            gp = obj->cost;
            ch_printf( ch, "&wIt is worth &C%d mithril, ", ( int )gp / 1000000 );
            gp = gp % 1000000;
            ch_printf( ch, "&Y%d gold, ", ( int )gp / 10000 );
            gp = gp % 10000;
            ch_printf( ch, "&W%d silver, ", ( int )gp / 100 );
            gp = gp % 100;
            ch_printf( ch, "&Oand %d copper coins&C.\n\r", gp );
         }

         if( ch->curr_talent[TAL_SEEKING] >= 5 )
            ch_printf( ch, "It appears to have been made for someone %d inches tall.\n\r", obj->size );
         if( ch->curr_talent[TAL_SEEKING] >= 10 && obj->material )
            ch_printf( ch, "It appears to be made of %s.\r\n", obj->material->name );
         if( ch->curr_talent[TAL_SEEKING] >= 25 )
         {
            if( obj->timer == 0 )
               ch_printf( ch, "It will not decay.\n\r" );
            else
               ch_printf( ch, "It will last another %d hours.\n\r", obj->timer );
         }
         if( obj->obj_by && ch->curr_talent[TAL_SEEKING] >= 50 )
            ch_printf( ch, "It was created by %s.\n\r", obj->obj_by );
         if( obj->raw_mana && ch->curr_talent[TAL_SEEKING] >= 5 )
            ch_printf( ch,
                       "This item %s with raw mana.\n\r",
                       obj->raw_mana > 2000 ? "vibrates violently" :
                       obj->raw_mana > 1500 ? "radiates brightly" :
                       obj->raw_mana > 1000 ? "shines" : obj->raw_mana > 500 ? "vibrates" : "tingles" );

         if( obj->mana && ch->curr_talent[TAL_SEEKING] >= 40 )
            ch_printf( ch, "%d points of energy have been absorbed into this item.\n\r", obj->mana );

         switch ( obj->item_type )
         {
            case ITEM_CONTAINER:
               ch_printf( ch, "%s appears to be %s.\n\r", capitalize( obj->short_descr ),
                          obj->value[0] < 76 ? "of a small capacity" :
                          obj->value[0] < 150 ? "of a small to medium capacity" :
                          obj->value[0] < 300 ? "of a medium capacity" :
                          obj->value[0] < 550 ? "of a medium to large capacity" :
                          obj->value[0] < 751 ? "of a large capacity" : "of a giant capacity" );
               break;

            case ITEM_PILL:
            case ITEM_SCROLL:
            case ITEM_POTION:
               if( ch->curr_talent[TAL_SEEKING] >= 20 )
               {
                  ch_printf( ch, "Casts:" );

                  if( obj->value[1] >= 0 && ( sktmp = get_skilltype( obj->value[1] ) ) != NULL )
                  {
                     send_to_char( " '", ch );
                     send_to_char( sktmp->name, ch );
                     send_to_char( "'", ch );
                  }

                  if( obj->value[2] >= 0 && ( sktmp = get_skilltype( obj->value[2] ) ) != NULL )
                  {
                     send_to_char( " '", ch );
                     send_to_char( sktmp->name, ch );
                     send_to_char( "'", ch );
                  }

                  if( obj->value[3] >= 0 && ( sktmp = get_skilltype( obj->value[3] ) ) != NULL )
                  {
                     send_to_char( " '", ch );
                     send_to_char( sktmp->name, ch );
                     send_to_char( "'", ch );
                  }

                  send_to_char( ".\n\r", ch );
               }
               break;

            case ITEM_SALVE:
               ch_printf( ch, "Has %d out of %d applications left", obj->value[1], obj->value[2] );
               if( ch->curr_talent[TAL_SEEKING] >= 20 )
               {
                  ch_printf( ch, " of " );
                  if( obj->value[4] >= 0 && ( sktmp = get_skilltype( obj->value[4] ) ) != NULL )
                  {
                     send_to_char( " '", ch );
                     send_to_char( sktmp->name, ch );
                     send_to_char( "'", ch );
                  }
                  if( obj->value[5] >= 0 && ( sktmp = get_skilltype( obj->value[5] ) ) != NULL )
                  {
                     send_to_char( " '", ch );
                     send_to_char( sktmp->name, ch );
                     send_to_char( "'", ch );
                  }
               }
               send_to_char( ".\n\r", ch );
               break;

            case ITEM_WEAPON:
               if( obj->value[4] && obj->value[4] < 14 )
                  ch_printf( ch, "This is a magically %s&z&Bweapon.\n\r", weapon_brand[obj->value[4]] );
               ch_printf( ch, "This weapon falls into the '%s' category.\n\r", weapon_skill[obj->value[5]] );
               if( ch->curr_talent[TAL_SEEKING] >= 40 && obj->value[0] )
               {
                  if( ch->curr_talent[TAL_SEEKING] >= 70 )
                  {
                     if( obj->value[0] <= 17 )
                        ch_printf( ch,
                                   "This weapon will inflict %d extra damage against all %s.\n\r",
                                   obj->value[6] / 10, weapon_bane[obj->value[0]] );
                  }
                  else
                  {
                     ch_printf( ch, "This weapon is the bane of all %s.\n\r", weapon_bane[obj->value[0]] );
                  }
               }
               ch_printf( ch, "Damage bonus is %d.\n\r", ( obj->value[2] ) / 2 );
               break;

            case ITEM_ARMOR:
               ch_printf( ch, "Armor class is %d.\n\r", obj->value[0] );
               break;

            case ITEM_MISSILE_WEAPON:
               if( obj->value[4] && obj->value[4] < 13 )
                  ch_printf( ch, "This is a magically %s&z&Bweapon.\n\r", weapon_brand[obj->value[4]] );
               ch_printf( ch, "This weapon falls into the '%s' category.\n\r", weapon_skill[obj->value[5]] );
               ch_printf( ch, "Damage bonus is %d.\n\r", obj->value[2] );
               if( obj->value[5] != SK_WAND )
               {
                  if( obj->value[0] == 0 )
                  {
                     ch_printf( ch, "It is empty.\n\r" );
                  }
                  else
                  {
                     ch_printf( ch, "%d shots have been loaded into this weapon.\n\r", obj->value[0] );
                  }
                  break;

            case ITEM_SCANNER:
            case ITEM_RADIO:
                  ch_printf( ch, "The volume is set at %d.\n\r", obj->value[0] );
                  ch_printf( ch, "The frequency is set at %d.\n\r", obj->value[1] );
                  break;
               }
         }
         break;

      case MAG_EARTH:
      case MAG_BLUNT:
         if( !obj->material )
         {
            obj_trick( ch, obj, type );
            return;
         }
         mat = obj->material;
         ch_printf( ch, "The %s is made of %s.\n\r", myobj( obj ), mat->name );
         ch_printf( ch, "%s has a cost of %d.\n\r", capitalize( mat->name ), mat->cost );
         if( !xIS_EMPTY( obj->extra_flags ) && ch->curr_talent[TAL_EARTH] >= 20 )
            ch_printf( ch, "It has the properties of %s.\n\r", extra_bit_name( &mat->extra_flags ) );
         if( ch->curr_talent[TAL_CATALYSM] >= 40 )
            ch_printf( ch, "%s is a%s magical ore.\n\r", capitalize( mat->name ),
                       mat->magic <= 10 ? "n incredibly" :
                       mat->magic <= 25 ? "n extremely" :
                       mat->magic <= 50 ? " highly" :
                       mat->magic <= 70 ? " very" :
                       mat->magic <= 80 ? " rather" :
                       mat->magic <= 90 ? " fairly" :
                       mat->magic <= 100 ? " somewhat" : mat->magic <= 120 ? " poorly" : " non" );
         if( ch->curr_talent[TAL_SEEKING] >= 15 )
            for( paf = mat->first_affect; paf; paf = paf->next )
               showaffect( ch, paf );
         break;

      case MAG_WATER:
         if( obj->item_type != ITEM_DRINK_CON && obj->item_type != ITEM_FOUNTAIN )
         {
            obj_trick( ch, obj, type );
            return;
         }

         if( obj->value[2] >= LIQ_MAX )
         {
            bug( "Do_drink: bad liquid number %d.", obj->value[2] );
            obj->value[2] = 0;
         }

         if( obj->value[1] <= 0 )
         {
            ch_printf( ch, "The %s is empty.\n\r", myobj( obj ) );
            break;
         }

         if( ch->curr_talent[TAL_WATER] >= 10 )
            ch_printf( ch, "The %s is %s full of %s.\n\r", myobj( obj ),
                       obj->value[1] < obj->value[0] / 4
                       ? "less than" :
                       obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq_table[obj->value[2]].liq_name );
         else
            ch_printf( ch, "The %s is %s full of a %s liquid.\n\r", myobj( obj ),
                       obj->value[1] < obj->value[0] / 4
                       ? "less than" :
                       obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq_table[obj->value[2]].liq_color );

         if( ch->curr_talent[TAL_WATER] >= 40 )
            ch_printf( ch, "It will make you %s, %s, %s.\n\r",
                       liq_table[obj->value[2]].liq_affect[COND_THIRST] > 0 ? "less thirsty" :
                       liq_table[obj->value[2]].liq_affect[COND_THIRST] < 0 ? "more thirsty" :
                       "no less thirsty",
                       liq_table[obj->value[2]].liq_affect[COND_FULL] > 0 ? "less hungry" :
                       liq_table[obj->value[2]].liq_affect[COND_FULL] < 0 ? "more hungry" :
                       "no less hungry",
                       liq_table[obj->value[2]].liq_affect[COND_DRUNK] > 0 ? "and drunk" : "but not drunk" );

         if( obj->value[3] && ch->curr_talent[TAL_WATER] > 20 )
            ch_printf( ch, "The %s is poisoned.\n\r", myobj( obj ) );
         break;

      case MAG_POISON:
         if( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD || obj->item_type == ITEM_COOK )
         {
            if( obj->item_type == ITEM_COOK && obj->value[2] == 0 )
               send_to_char( "It looks undercooked.\n\r", ch );
            else if( obj->value[3] != 0 )
               send_to_char( "You smell poisonous fumes.\n\r", ch );
            else
               send_to_char( "It looks very delicious.\n\r", ch );
         }
         else
         {
            send_to_char( "It doesn't look poisoned.\n\r", ch );
         }
         break;

   }
}

void ch_find( CHAR_DATA * victim, CHAR_DATA * ch, int type )
{
   char *name;
   DEITY_DATA *talent;
   AFFECT_DATA *paf;
   SKILLTYPE *sktmp;

   switch ( type )
   {
      default:
         ch_trick( victim, type );
         break;

      case MAG_ALL:
         if( IS_NPC( victim ) )
            name = capitalize( victim->short_descr );
         else
            name = victim->name;

         ch_printf( ch, "%s looks like %s.\n\r", name, victim->nation ? aoran( victim->nation->name ) : get_race( victim ) );

         if( ch->curr_talent[TAL_SEEKING] >= 30 )
         {
            if( TALENT( ch, TAL_SEEKING ) > TALENT( victim, TAL_SECURITY ) )
            {
               ch_printf( ch, "%s's current stats: %-2d str, %-2d wil, %-2d int, %-2d dex, %2d con, %-2d per, %-2d end.\r\n",
                          victim->name,
                          get_curr_str( victim ), get_curr_wil( victim ), get_curr_int( victim ),
                          get_curr_dex( victim ), get_curr_con( victim ), get_curr_per( victim ), get_curr_lck( victim ) );
            }
         }

         if( ch->curr_talent[TAL_SEEKING] >= 40 )
         {
            if( ( chance( ch, 50 ) && get_curr_wil( ch ) >= get_curr_wil( victim ) + 10 ) )
            {
               ch_printf( ch, "%s appears to be affected by: ", name );

               if( !victim->first_affect )
               {
                  send_to_char( "nothing.\n\r", ch );
               }
               else
               {

                  for( paf = victim->first_affect; paf; paf = paf->next )
                  {
                     if( victim->first_affect != victim->last_affect )
                     {
                        if( paf != victim->last_affect && ( sktmp = get_skilltype( paf->type ) ) != NULL )
                           ch_printf( ch, "%s, ", sktmp->name );

                        if( paf == victim->last_affect && ( sktmp = get_skilltype( paf->type ) ) != NULL )
                        {
                           ch_printf( ch, "and %s.\n\r", sktmp->name );
                        }
                     }
                     else
                     {
                        if( ( sktmp = get_skilltype( paf->type ) ) != NULL )
                           ch_printf( ch, "%s.\n\r", sktmp->name );
                        else
                           send_to_char( "\n\r", ch );
                     }
                  }
               }
            }
         }

         if( IS_NPC( victim ) )
            return;

         ch_printf( ch, "%s's aura:\n\r", PERS( victim, ch ) );

         if( IS_SET( victim->pcdata->flags, PCFLAG_MASK ) && TALENT( ch, TAL_SEEKING ) > TALENT( victim, TAL_SEEKING ) )
         {
            send_to_char( "You sense only a distorted fog.\n\r", ch );
            return;
         }

         for( talent = first_deity; talent; talent = talent->next )
         {
            if( victim->pcdata->inborn == talent->index )
            {
               if( victim->talent[talent->index] >= 150 )
                  ch_printf( ch, "predominately radiant %s\n\r", talent->color );
               else if( victim->talent[talent->index] >= 100 )
                  ch_printf( ch, "primarily brilliant %s\n\r", talent->color );
               else if( victim->talent[talent->index] >= 50 )
                  ch_printf( ch, "mainly bright %s\n\r", talent->color );
               else
                  ch_printf( ch, "mostly pale %s\n\r", talent->color );
            }
            else if( victim->talent[talent->index] >= 200 )
               ch_printf( ch, "blindingly brilliant %s\n\r", talent->color );
            else if( victim->talent[talent->index] >= 160 && ch->curr_talent[TAL_SEEKING] >= 10 )
               ch_printf( ch, "awash with a cascade of %s\n\r", talent->color );
            else if( victim->talent[talent->index] >= 130 && ch->curr_talent[TAL_SEEKING] >= 20 )
               ch_printf( ch, "raging torrents of brilliant %s\n\r", talent->color );
            else if( victim->talent[talent->index] >= 100 && ch->curr_talent[TAL_SEEKING] >= 30 )
               ch_printf( ch, "radiating %s brightly\n\r", talent->color );
            else if( victim->talent[talent->index] >= 85 && ch->curr_talent[TAL_SEEKING] >= 40 )
               ch_printf( ch, "shimmering rays of %s\n\r", talent->color );
            else if( victim->talent[talent->index] >= 70 && ch->curr_talent[TAL_SEEKING] >= 50 )
               ch_printf( ch, "bright streaks of %s\n\r", talent->color );
            else if( victim->talent[talent->index] >= 50 && ch->curr_talent[TAL_SEEKING] >= 80 )
               ch_printf( ch, "shining %s sparkles\n\r", talent->color );
            else if( victim->talent[talent->index] >= 30 && ch->curr_talent[TAL_SEEKING] >= 120 )
               ch_printf( ch, "traces of glittering %s\n\r", talent->color );
            else if( victim->talent[talent->index] >= 10 && ch->curr_talent[TAL_SEEKING] >= 150 )
               ch_printf( ch, "pale flecks of %s\n\r", talent->color );
            else if( victim->talent[talent->index] >= 1 && ch->curr_talent[TAL_SEEKING] >= 200 )
               ch_printf( ch, "faint tinge of %s barely visible\n\r", talent->color );
         }

         break;
   }
}  /* ch_find */

void seek_find( CHAR_DATA * ch, int type, char *argument )
{
   OBJ_DATA *obj;
   OBJ_DATA *in_obj;
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char *str;
   char buf[MAX_STRING_LENGTH];
   char name[MAX_INPUT_LENGTH];
   int found, temp;
   struct stat fst;

   switch ( type )
   {
      default:
         ch_trick( ch, type );
         break;

      case MAG_TELEKINETIC:
         if( ch->curr_talent[TAL_SEEKING] < 20 )
         {
            ch_trick( ch, type );
            break;
         }

         found = FALSE;
         if( TALENT( ch, TAL_SEEKING ) < 50 )
            for( victim = first_char; victim; victim = victim->next )
            {
               if( victim->in_room && can_see( ch, victim ) && nifty_is_name( argument, victim->name ) )
               {
                  found++;
                  pager_printf( ch, "%s is in %s\n\r", PERS( victim, ch ), victim->in_room->name );
               }
            }
         else
            for( victim = first_char; victim; victim = victim->next )
            {
               if( victim->in_room && can_see( ch, victim ) && nifty_is_name( argument, victim->name ) )
               {
                  found++;
                  pager_printf( ch, "%s is in %s, in %s\n\r",
                                PERS( victim, ch ), victim->in_room->name, victim->in_room->area->name );
               }
            }
         for( obj = first_object; obj; obj = obj->next )
         {
            if( !can_see_obj( ch, obj ) || !nifty_is_name( argument, obj->name ) )
               continue;
            if( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) ||
                IS_OBJ_STAT( obj, ITEM_NOLOCATE ) ||
                IS_OBJ_STAT( obj, ITEM_ARTIFACT ) ||
                IS_OBJ_STAT( obj, ITEM_GEM ) || number_percent(  ) > TALENT( ch, TAL_SEEKING ) )
               continue;

            found++;

            for( temp = 0, in_obj = obj; in_obj->in_obj && temp < 100; in_obj = in_obj->in_obj, ++temp );
            if( temp >= MAX_NEST )
            {
               sprintf( buf, "seek_object: object [%d] %s is nested more than %d times!",
                        obj->pIndexData->vnum, obj->short_descr, MAX_NEST );
               bug( buf, 0 );
               continue;
            }

            if( in_obj->carried_by )
            {
               if( TALENT( ch, TAL_SEEKING ) < 30 )
                  sprintf( buf, "%s is carried by %s.\n\r", obj_short( obj ), PERS( in_obj->carried_by, ch ) );
               else if( TALENT( ch, TAL_SEEKING ) < 60 )
                  sprintf( buf, "%s is carried by %s, in %s.\n\r",
                           obj_short( obj ), PERS( in_obj->carried_by, ch ), in_obj->carried_by->in_room->name );
               else
                  sprintf( buf, "%s is carried by %s, in %s in %s.\n\r",
                           obj_short( obj ), PERS( in_obj->carried_by, ch ),
                           in_obj->carried_by->in_room->name, in_obj->carried_by->in_room->area->name );
            }
            else
            {
               if( in_obj->in_room == NULL )
                  sprintf( buf, "%s is somewhere\n\r", obj_short( obj ) );
               else if( TALENT( ch, TAL_SEEKING ) < 50 )
                  sprintf( buf, "%s is in %s.\n\r", obj_short( obj ), in_obj->in_room->name );
               else
                  sprintf( buf, "%s is in %s, in %s.\n\r",
                           obj_short( obj ), in_obj->in_room->name, in_obj->in_room->area->name );
            }

            buf[0] = UPPER( buf[0] );
            send_to_char( buf, ch );
         }

         if( !found )
         {
            send_to_char( "You were unable to locate anything like that.\n\r", ch );
         }
         break;

      case MAG_EARTH:
      case MAG_BLUNT:
         if( ch->curr_talent[TAL_SEEKING] + ch->curr_talent[TAL_EARTH] < 20 )
         {
            ch_trick( ch, type );
            return;
         }

         ch_printf( ch, "You are in %s in %s, on the world of %s.\n\r",
                    ch->in_room->name, ch->in_room->area->name, ch->in_room->area->resetmsg );
         switch ( ch->in_room->sector_type )
         {
            default:
               str = "somewhere strange";
               break;
            case SECT_INSIDE:
               str = "inside";
               break;
            case SECT_CITY:
               str = "a city street";
               break;
            case SECT_FIELD:
               str = "a field";
               break;
            case SECT_FOREST:
               str = "a forest";
               break;
            case SECT_HILLS:
               str = "in the hills";
               break;
            case SECT_MOUNTAIN:
               str = "in the mountains";
               break;
            case SECT_WATER_SWIM:
               str = "swallow water";
               break;
            case SECT_WATER_NOSWIM:
               str = "deep water";
               break;
            case SECT_UNDERWATER:
               str = "underwater";
               break;
            case SECT_AIR:
               str = "in the air";
               break;
            case SECT_DESERT:
               str = "in the desert";
               break;
            case SECT_OCEANFLOOR:
               str = "on the oceanfloor";
               break;
            case SECT_UNDERGROUND:
               str = "underground";
               break;
            case SECT_LAVA:
               str = "molten lava";
               break;
            case SECT_SWAMP:
               str = "in the swamps";
               break;
            case SECT_BEACH:
               str = "on a beach";
               break;
            case SECT_ICE:
               str = "on ice";
               break;
         }
         ch_printf( ch, "It appears to be %s.\n\r", str );
         if( ch->in_room->room_flags )
            ch_printf( ch, "It seems to be %s.\n\r", flag_string( ch->in_room->room_flags, r_flags ) );
         temp = ch->in_room->curr_water;
         ch_printf( ch, "%s, and ", temp >= 100 ?
                    "The room is completely underwater" : temp > 50 ?
                    "The water is deep" : temp > 0 ? "The water is muddy and shallow" : "The room is dry" );
         temp = ch->in_room->curr_vegetation;
         ch_printf( ch, "%s vegetation grows.\n\r", temp >= 66 ? "thick" : temp > 33 ? "some" : "no" );
         temp = ch->in_room->curr_resources;
         ch_printf( ch, "There is a %s concentration of resources here.\n\r", temp >= 66 ?
                    "high" : temp > 33 ? "moderate" : "low" );
         if( ch->in_room->runes )
            ch_printf( ch, "Runes of %s line the walls.\n\r",
                       ch->curr_talent[TAL_CATALYSM] < 20 ? "some sort" : flag_string( ch->in_room->runes, rune_flags ) );
         break;

      case MAG_PSIONIC:
         if( ch->curr_talent[TAL_MIND] + ch->curr_talent[TAL_SEEKING] < 200 )
         {
            ch_trick( ch, type );
            break;
         }
         if( ( victim = get_char_world( ch, argument ) ) == NULL )
         {
            ch_trick( ch, type );
            break;
         }
         if( !victim->desc )
         {
            ch_trick( ch, type );
            break;
         }
         if( victim == ch )
         {
            for( d = first_descriptor; d; d = d->next )
               if( d->snoop_by == ch->desc )
                  d->snoop_by = NULL;
            send_to_char( "You break your mental connections watching over people.\n\r", ch );
            return;
         }
         if( victim->desc->snoop_by )
         {
            for( d = first_descriptor; d; d = d->next )
               if( d->snoop_by == ch->desc )
                  d->snoop_by = NULL;
            send_to_char( "Your mental links become tangled and sever themselves.\n\r", ch );
            return;
         }

         for( d = ch->desc->snoop_by; d; d = d->snoop_by )
            if( d->character == victim || d->original == victim )
            {
               send_to_char( "Your target's mind gets in the way.\n\r", ch );
               return;
            }

         if( IS_SET( ch->pcdata->flags, PCFLAG_SHIELD )
             || ( IS_SET( victim->pcdata->flags, PCFLAG_SHIELD ) && TALENT( ch, TAL_MIND ) < TALENT( victim, TAL_MIND ) ) )
         {
            send_to_char( "A shield blocks you from connecting.\n\r", ch );
            return;
         }

         if( victim->curr_talent[TAL_SEEKING] + victim->curr_talent[TAL_MIND] >= 110 )
            send_to_char( "You feel like someone is watching your every move...\n\r", victim );
         victim->desc->snoop_by = ch->desc;
         send_to_char( "You establish a connection to watch over them.\n\r", ch );
         break;

      case MAG_TIME:
         send_to_char( "You direct your thoughts backward along the paths of Time...\n\r", ch );
         strcpy( name, capitalize( argument ) );
         sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( argument[0] ), name );
         if( stat( buf, &fst ) != -1 )
            sprintf( buf, "%s last entered the Planes on %s\r", name, ctime( &fst.st_mtime ) );
         else
            sprintf( buf, "You could not find %s's thread within the Planes.\n\r", name );
         send_to_char( buf, ch );
         break;

   }
}  /* seek_find */

void ch_focus( CHAR_DATA * victim, CHAR_DATA * ch, int type, int power )
{
   switch ( type )
   {
      default:
         ch_trick( victim, type );
         break;

      case MAG_ANTIMAGIC:
         if( !victim->shield )
         {
            ch_trick( victim, type );
            return;
         }
         if( victim == ch )
         {
            act( AT_MAGIC, "You lower your magical shield.", ch, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n lowers $s magical shield.", ch, NULL, NULL, TO_ROOM );
         }
         else
         {
            if( ch->curr_talent[TAL_VOID] < 50 )
            {
               ch_trick( victim, type );
               return;
            }
            act( AT_MAGIC, "You disrupt $N's magical shield!", ch, NULL, victim, TO_CHAR );
            act( AT_MAGIC, "$n disrupts $N's magical shield!", ch, NULL, victim, TO_ROOM );
            act( AT_MAGIC, "$n disrupts your magical shield!", ch, NULL, victim, TO_VICT );
         }
         victim->shield = MAG_NONE;
         return;

      case MAG_ACID:
         if( ch->curr_talent[TAL_WATER] < 30 || ch->curr_talent[TAL_EARTH] < 10 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_GREEN, "A glistening bubble of acid encompasses you.", ch, NULL, NULL, TO_CHAR );
         act( AT_GREEN, "A glistening bubble of acid encompasses $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_DREAM:
         if( ch->curr_talent[TAL_DREAM] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_PINK, "Wispy dreams and whispered thoughts surround you.", ch, NULL, NULL, TO_CHAR );
         act( AT_PINK, "Wispy dreams and whispered thoughts surround $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_ENERGY:
         if( ch->curr_talent[TAL_CATALYSM] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_PURPLE, "A rippling shield of energy surrounds you.", ch, NULL, NULL, TO_CHAR );
         act( AT_PURPLE, "A rippling shield of energy surrounds $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_FIRE:
         if( ch->curr_talent[TAL_FIRE] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_FIRE, "Mystical flames rise to enshroud you.", ch, NULL, NULL, TO_CHAR );
         act( AT_FIRE, "Mystical flames rise to enshroud $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_COLD:
         if( ch->curr_talent[TAL_FROST] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_FROST, "A glistening hail of ice encompasses you.", ch, NULL, NULL, TO_CHAR );
         act( AT_FROST, "A glistening hail of ice encompasses $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_LIGHT:
         if( ch->curr_talent[TAL_ILLUSION] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_WHITE, "Searing light radiates from you.", ch, NULL, NULL, TO_CHAR );
         act( AT_WHITE, "Searing light radiates from $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_ALL:
         if( victim == ch )
         {
            if( ch->curr_talent[TAL_CATALYSM] < 60 )
            {
               ch_trick( victim, type );
               return;
            }
            act( AT_PURPLE, "A field of prismatic mana surrounds you.", ch, NULL, NULL, TO_CHAR );
            act( AT_PURPLE, "A field of prismatic mana surrounds $n.", ch, NULL, NULL, TO_ROOM );
         }
         else
         {
            if( ch->curr_talent[TAL_CATALYSM] < 30 )
            {
               ch_trick( victim, type );
               return;
            }
            WAIT_STATE( ch, PULSE_VIOLENCE );

            if( victim->shield == MAG_ALL || IS_SET( ch->pcdata->flags, PCFLAG_CHEAT ) )
            {
               act( AT_PURPLE, "Your mana bounces off $N's shield!", ch, NULL, victim, TO_CHAR );
               act( AT_PURPLE, "$n's mana bounces off your shield!", ch, NULL, victim, TO_VICT );
               act( AT_PURPLE, "$n's mana bounces off $N's shield!", ch, NULL, victim, TO_NOTVICT );
               ch->mana = ch->max_mana + power;
               return;
            }

            act( AT_PURPLE, "You funnel your magic into $N's body!", ch, NULL, victim, TO_CHAR );
            act( AT_PURPLE, "$n glows brightly, funneling $s magic into you!", ch, NULL, victim, TO_VICT );
            act( AT_PURPLE, "$n glows brightly, funneling $s magic into $N!", ch, NULL, victim, TO_NOTVICT );
            ch->mana -= power;
            victim->mana += power;
         }
         break;

      case MAG_POISON:
         if( ch->curr_talent[TAL_DEATH] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_DGREEN, "Deadly venom oozes from your skin.", ch, NULL, NULL, TO_CHAR );
         act( AT_DGREEN, "Deadly venom oozes from $n's skin.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_ELECTRICITY:
         if( ch->curr_talent[TAL_LIGHTNING] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_YELLOW, "Torrents of cascading energy form around you.", ch, NULL, NULL, TO_CHAR );
         act( AT_YELLOW, "Torrents of cascading energy form around $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_SOUND:
         if( ch->curr_talent[TAL_LIGHTNING] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_PINK, "A discordant wail of deafening sound surrounds you.", ch, NULL, NULL, TO_CHAR );
         act( AT_PINK, "A discordant wail of deafening sound surrounds $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_BLUNT:
         if( ch->curr_talent[TAL_EARTH] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_DGREY, "A hail of stones rotates around you.", ch, NULL, NULL, TO_CHAR );
         act( AT_DGREY, "A hail of stones rotates around $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_PSIONIC:
         if( ch->curr_talent[TAL_MIND] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_PURPLE, "You concentrate, forming a mental barrier.", ch, NULL, NULL, TO_CHAR );
         act( AT_PURPLE, "$n concentrates, forming a mental barrier.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_EARTH:
         if( ch->curr_talent[TAL_EARTH] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_GREEN, "An array of thorny vines surrounds you.", ch, NULL, NULL, TO_CHAR );
         act( AT_GREEN, "An array of thorny vines surrounds $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_CHANGE:
         if( ch->curr_talent[TAL_CHANGE] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_DBLUE, "The air warps and distorts around you.", ch, NULL, NULL, TO_CHAR );
         act( AT_DBLUE, "The air warps and distorts around $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_WATER:
         if( ch->curr_talent[TAL_WATER] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_BLUE, "Foaming waves rise to encompass you.", ch, NULL, NULL, TO_CHAR );
         act( AT_BLUE, "Foaming waves rise to encompass $n.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_WIND:
         if( ch->curr_talent[TAL_WIND] < 40 )
         {
            ch_trick( victim, type );
            return;
         }
         act( AT_CYAN, "Swirling gusts of wind blow around you.", ch, NULL, NULL, TO_CHAR );
         act( AT_CYAN, "Swirling gusts of wind blow around $n.", ch, NULL, NULL, TO_ROOM );
         break;
   }
   ch->shield = type;
}  /* ch_focus */

void obj_focus( CHAR_DATA * ch, OBJ_DATA * obj, int type )
{
   char buf[MAX_STRING_LENGTH];

   if( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_MISSILE_WEAPON )
   {
      obj_trick( ch, obj, type );
      return;
   }
   if( obj->value[4] && type != MAG_HOLY && type != MAG_ANTIMAGIC )
   {
      obj_trick( ch, obj, type );
      return;
   }
   if( obj->mana < 500 && type != MAG_ANTIMAGIC )
   {
      obj_trick( ch, obj, type );
      return;
   }
   WAIT_STATE( ch, PULSE_VIOLENCE );
   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         return;

      case MAG_FIRE:
         if( ch->curr_talent[TAL_FIRE] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_FIRE, "$p bursts into flame!", ch, obj, NULL, TO_CHAR );
         act( AT_FIRE, "$n's $p bursts into flame!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_FLAMING;
         break;

      case MAG_COLD:
         if( ch->curr_talent[TAL_FROST] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_FROST, "$p shimmers with cold!", ch, obj, NULL, TO_CHAR );
         act( AT_FROST, "$n's $p shimmers with cold!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_FROZEN;
         break;

      case MAG_ELECTRICITY:
         if( ch->curr_talent[TAL_LIGHTNING] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_YELLOW, "$p crackles with electricity!", ch, obj, NULL, TO_CHAR );
         act( AT_YELLOW, "$n's $p crackles with electricity!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_ELEC;
         break;

      case MAG_ACID:
         if( ch->curr_talent[TAL_WATER] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_YELLOW, "$p hisses with acid!", ch, obj, NULL, TO_CHAR );
         act( AT_YELLOW, "$n's $p hisses with acid!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_ACID;
         break;

      case MAG_ANTIMATTER:
         if( ch->curr_talent[TAL_VOID] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_DGREY, "Black lightning erupts from $p!", ch, obj, NULL, TO_CHAR );
         act( AT_DGREY, "Black lightning erupts from $n's $p!", ch, obj, NULL, TO_ROOM );
         xREMOVE_BIT( obj->extra_flags, ITEM_GLOW );
         xSET_BIT( obj->extra_flags, ITEM_DARK );
         obj->value[4] = BRAND_ANTI_MATTER;
         break;

      case MAG_SOUND:
         if( ch->curr_talent[TAL_SPEECH] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_GREEN, "$p emits a sonic hum!", ch, obj, NULL, TO_CHAR );
         act( AT_GREEN, "$n's $p emits a sonic hum!", ch, obj, NULL, TO_ROOM );
         xSET_BIT( obj->extra_flags, ITEM_HUM );
         obj->value[4] = BRAND_SONIC;
         break;

      case MAG_CHANGE:
         if( ch->curr_talent[TAL_CHANGE] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_DBLUE, "The air warps and twists into $p!", ch, obj, NULL, TO_CHAR );
         act( AT_DBLUE, "The air warps and twists into $n's $p!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_DISTORT;
         break;

      case MAG_POISON:
         if( ch->curr_talent[TAL_DEATH] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_GREEN, "$p drips with venom!", ch, obj, NULL, TO_CHAR );
         act( AT_GREEN, "$n's $p drips with venom!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_POISON;
         break;

      case MAG_SLASH:
         if( ch->curr_talent[TAL_CATALYSM] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( obj->value[5] != SK_SWORD
             && obj->value[5] != SK_DAGGER
             && obj->value[5] != SK_AXE
             && obj->value[5] != SK_HALBERD
             && obj->value[5] != SK_SPEAR && obj->value[5] != SK_POLEARM && obj->value[5] != SK_CLAW )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_WHITE, "$p glints razor-sharp!", ch, obj, NULL, TO_CHAR );
         act( AT_WHITE, "$n's $p glints razor-sharp!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_VORPAL;
         break;

      case MAG_WIND:
         if( ch->curr_talent[TAL_WIND] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_CYAN, "Stormy winds swirl about $p!", ch, obj, NULL, TO_CHAR );
         act( AT_CYAN, "Stormy winds swirl about $n's $p!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_STORM;
         break;

      case MAG_LIGHT:
         if( ch->curr_talent[TAL_ILLUSION] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_YELLOW, "$p glows with a blinding radiance!", ch, obj, NULL, TO_CHAR );
         act( AT_YELLOW, "$n's $p glows with a blinding radiance!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_RADIANT;
         xREMOVE_BIT( obj->extra_flags, ITEM_DARK );
         xSET_BIT( obj->extra_flags, ITEM_GLOW );
         break;

      case MAG_BLUNT:
         if( ch->curr_talent[TAL_EARTH] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( obj->value[5] != SK_MACE && obj->value[5] != SK_STAFF && obj->value[5] != SK_WHIP )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_DGREY, "$p becomes harder than rock!", ch, obj, NULL, TO_CHAR );
         act( AT_DGREY, "$n's $p becomes harder than rock!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_CRUSH;
         break;

      case MAG_DRAIN:
         if( ch->curr_talent[TAL_DEATH] < 60 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_DGREY, "$p thirsts for blood!", ch, obj, NULL, TO_CHAR );
         act( AT_DGREY, "$n's $p thirsts for blood!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_DRAIN;
         break;

      case MAG_PSIONIC:
         if( ch->curr_talent[TAL_MIND] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_PURPLE, "A strange humming sound emanates from $p!", ch, obj, NULL, TO_CHAR );
         act( AT_PURPLE, "A strange humming sound emanates from $n's $p!", ch, obj, NULL, TO_ROOM );
         obj->value[4] = BRAND_PSIONIC;
         xSET_BIT( obj->extra_flags, ITEM_HUM );
         break;

      case MAG_HOLY:
         if( ch->curr_talent[TAL_SECURITY] < 50 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( IS_OBJ_STAT( obj, ITEM_LOYAL ) || IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( obj->mana < 500 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         obj->mana -= 500;
         obj->cost += 500000;
         xSET_BIT( obj->extra_flags, ITEM_LOYAL );
         act( AT_WHITE, "You bind $p with a wash of white light.", ch, obj, NULL, TO_CHAR );
         act( AT_WHITE, "White light washes over $n's $p.", ch, obj, NULL, TO_ROOM );
         return;

      case MAG_ANTIMAGIC:
         if( !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
         {
            obj_trick( ch, obj, type );
            return;
         }
         if( ch->curr_talent[TAL_VOID] < 30 )
         {
            obj_trick( ch, obj, type );
            return;
         }
         act( AT_DGREY, "You strip $p of its magical properties.", ch, obj, NULL, TO_CHAR );
         act( AT_DGREY, "Antimagic violently rips at $n's $p.", ch, NULL, NULL, TO_ROOM );
         xREMOVE_BIT( obj->extra_flags, ITEM_MAGIC );
         xREMOVE_BIT( obj->extra_flags, ITEM_HUM );
         xREMOVE_BIT( obj->extra_flags, ITEM_DARK );
         xREMOVE_BIT( obj->extra_flags, ITEM_GLOW );
         obj->cost -= 50000;
         if( obj->value[4] )
         {
            obj->value[4] = 0;
            obj->cost -= 500000;
         }
         obj->value[0] = 0;
         obj->value[7] = 0;
         obj->mana = 0;
         obj->raw_mana = 0;
         return;
   }

   obj->mana -= 500;
   obj->cost += 500000;
   sprintf( buf, "%s%s", weapon_brand[obj->value[4]], myobj( obj ) );
   STRFREE( obj->short_descr );
   obj->short_descr = STRALLOC( buf );
}  /* obj_focus */

void obj_move( CHAR_DATA * ch, OBJ_DATA * obj, int type, int power, char *argument )
{
   CHAR_DATA *victim;
   CHAR_DATA *target;
   ROOM_INDEX_DATA *location;

   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         break;

      case MAG_TELEKINETIC:
         if( ( target = get_char_world( ch, argument ) ) == NULL
             || !can_travel( 90, ch, target ) || ( TALENT( target, TAL_SECURITY ) > TALENT( ch, TAL_SEEKING ) ) )
         {
            act( AT_MAGIC, "You are unable to locate $t.", ch, argument, NULL, TO_CHAR );
            return;
         }

         if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD )   /* Thanks Smokey */
             || IS_OBJ_STAT( obj, ITEM_ARTIFACT )  /* Thanks Waffle */
             || victim->carry_weight + get_obj_weight( obj ) > can_carry_w( target ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         separate_obj( obj );

         act( AT_MAGIC, "$p slowly dematerializes from your hands.", ch, obj, NULL, TO_CHAR );
         act( AT_MAGIC, "$p slowly dematerializes from $n's hands.", ch, obj, NULL, TO_ROOM );

         obj_from_char( obj );

         if( !IS_AFFECTED( target, AFF_ETHEREAL ) )
         {
            obj_to_char( obj, target );
            act( AT_MAGIC, "$p slowly materializes in your hands, sent by $N.", target, obj, ch, TO_CHAR );
            act( AT_MAGIC, "$p slowly materializes in $n's hands.", target, obj, NULL, TO_ROOM );
         }
         else
         {
            obj_to_room( obj, target->in_room );
            act( AT_MAGIC, "$p slowly materializes beside you and drops to the ground.", target, obj, ch, TO_CHAR );
            act( AT_MAGIC, "$p slowly materializes beside $n and drops to the ground.", target, obj, NULL, TO_ROOM );
         }
         save_char_obj( ch );
         save_char_obj( target );
         WAIT_STATE( ch, PULSE_VIOLENCE );
         break;

      case MAG_CHANGE:
         for( ;; )
         {
            location = get_room_index( number_range( 0, 32000 ) );
            if( location )
               if( !IS_SET( location->room_flags, ROOM_PRIVATE )
                   && !IS_SET( location->room_flags, ROOM_SOLITARY )
                   && !IS_SET( location->room_flags, ROOM_NO_ASTRAL ) && !IS_SET( location->room_flags, ROOM_PROTOTYPE ) )
                  break;
         }
         act( AT_MAGIC, "You throw $p high into the air where it vanishes.", ch, obj, NULL, TO_CHAR );
         act( AT_MAGIC, "$n throws $p high into the air where it vanishes.", ch, obj, NULL, TO_ROOM );
         obj_from_char( obj );
         obj_to_room( obj, location );
         break;

      case MAG_WIND:
         if( obj->weight * 10 > power || !obj->in_room )
         {
            act( AT_MAGIC, "$p shakes a little.", ch, obj, NULL, TO_ROOM );
            act( AT_MAGIC, "$p shakes a little.", ch, obj, NULL, TO_CHAR );
            return;
         }
         act( AT_MAGIC, "$p suddenly lifts off the ground and flies through the air toward $n.", ch, obj, NULL, TO_ROOM );
         act( AT_MAGIC, "You lift $p with your magic and bring it toward you.", ch, obj, NULL, TO_CHAR );
         if( ch->curr_talent[TAL_MOTION] < number_range( 1, 30 ) )
         {
            act( AT_MAGIC, "$p flies past $n and falls to the ground.", ch, obj, NULL, TO_ROOM );
            act( AT_MAGIC, "$p flies past you and falls to the ground.", ch, obj, NULL, TO_CHAR );
            return;
         }
         if( ch->curr_talent[TAL_MOTION] < number_range( 1, 50 ) )
         {
            act( AT_MAGIC, "$p smacks into $n, but $e catches it.", ch, obj, NULL, TO_ROOM );
            act( AT_MAGIC, "$p smacks into you, but you catch it.", ch, obj, NULL, TO_CHAR );
            lose_hp( ch, obj->weight );
         }
         separate_obj( obj );
         obj_from_room( obj );
         obj_to_char( obj, ch );
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
         break;
   }
}  /* obj_move */

void ch_move( CHAR_DATA * victim, CHAR_DATA * ch, int type, char *arg1, char *arg2 )
{
   int slot, mem;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *target;
   char buf[MAX_STRING_LENGTH];

   location = NULL;

   switch ( type )
   {
      default:
         ch_trick( victim, type );
         break;

      case MAG_TELEKINETIC:
         if( ( target = get_char_world( ch, arg2 ) ) == NULL
             || !can_travel( 90, ch, target ) || ( TALENT( target, TAL_SECURITY ) > TALENT( ch, TAL_SEEKING ) ) )
         {
            act( AT_MAGIC, "You are unable to locate $t.", ch, arg1, NULL, TO_CHAR );
            return;
         }
         if( target == ch || victim == target )
         {
            ch_trick( victim, type );
            break;
         }

         if( victim != ch && ch->curr_talent[TAL_MOTION] < 90 )
         {
            ch_trick( victim, type );
            break;
         }
         if( !IS_NPC( victim ) )
         {
            sprintf( buf, "%s sent %s to %s (%d).", ch->name, victim->name, target->in_room->name, target->in_room->vnum );
            to_channel( buf, "Spell", PERMIT_SECURITY );
         }

         act( AT_MAGIC, "You send $N across the astral planes.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n sends you across the astral planes.", ch, NULL, victim, TO_VICT );
         act( AT_MAGIC, "$n sends $N across the astral planes.", ch, NULL, victim, TO_ROOM );
         char_from_room( victim );
         victim->in_obj = NULL;
         char_to_room( victim, target->in_room );
         if( victim->mount )
         {
            char_from_room( victim->mount );
            char_to_room( victim->mount, target->in_room );
         }
         act( AT_MAGIC, "A rift in the sky opens, and $n falls out.", victim, NULL, NULL, TO_ROOM );
         do_look( victim, "auto" );
         break;

      case MAG_CHANGE:
         if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) && ch->curr_talent[TAL_MOTION] < 80 )
         {
            send_to_char( "For some reason, nothing happens.\n\r", ch );
            return;
         }

         for( ;; )
         {
            location = get_room_index( number_range( 0, 32000 ) );
            if( location )
               if( !IS_SET( location->room_flags, ROOM_PRIVATE )
                   && !IS_SET( location->room_flags, ROOM_SOLITARY )
                   && !IS_SET( location->room_flags, ROOM_NO_ASTRAL )
                   && !IS_SET( location->area->flags, AFLAG_NOTELEPORT )
                   && !IS_SET( location->room_flags, ROOM_PROTOTYPE ) && !IS_SET( location->room_flags, ROOM_NO_RECALL ) )
                  break;
         }

         act( AT_MAGIC, "Reality seems to twist for a moment, and your surroundings warp and change.", victim, NULL, NULL,
              TO_CHAR );
         act( AT_MAGIC, "Reality seems to twist for a moment, and $n vanishes.", victim, NULL, NULL, TO_ROOM );

         char_from_room( victim );
         victim->in_obj = NULL;
         char_to_room( victim, location );
         if( victim->mount )
         {
            char_from_room( victim->mount );
            char_to_room( victim->mount, location );
         }
         act( AT_MAGIC, "Reality ripples for a moment, and $n suddenly appears.", ch, NULL, NULL, TO_ROOM );
         do_look( victim, "auto" );
         break;

      case MAG_PSIONIC:
         if( !arg2 || arg2[0] == '\0' )
            slot = 0;
         else
            slot = atoi( arg2 );

         if( !str_cmp( arg2, "nexus" ) )
         {
            if( IS_SET( ch->in_room->area->flags, AFLAG_FREEKILL ) )
               location = get_room_index( ROOM_VNUM_PKILL );
            else
               location = get_room_index( ROOM_VNUM_NEXUS );
         }
         else if( !str_cmp( arg2, "area" ) )
         {
            location = get_room_index( ch->in_room->area->hi_soft_range );
         }
         else
         {

            mem = ( ch->curr_talent[TAL_MOTION] - 20 ) / 10;
            if( mem > 7 )
               mem = 7;
            if( mem < 0 && ch->curr_talent[TAL_MOTION] > 0 )
               mem = 0;

            if( slot > mem )
            {
               send_to_char( "You cannot recall the details of that location.\n\r", ch );
               return;
            }

            if( IS_NPC( ch ) )
            {
               if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master && !IS_NPC( ch->master ) )
               {
                  location = get_room_index( ch->master->pcdata->memorize[slot] );
               }
               else
               {
                  return;
               }
            }
            else
            {
               location = get_room_index( ch->pcdata->memorize[slot] );
            }
         }

         if( !location )
            location = get_room_index( ROOM_VNUM_NEXUS );

         if( !location )
         {
            send_to_char( "You are completely lost.\n\r", ch );
            return;
         }

         if( ch->in_room == location )
         {
            send_to_char( "You are already here!\n\r", ch );
            return;
         }

         if( IS_AFFECTED( ch, AFF_DREAMWORLD ) && IS_SET( location->room_flags, ROOM_NO_DREAM ) )
         {
            send_to_char( "A mysterious shield prevents you from recalling.\n\r", ch );
            return;
         }

         if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) && ch->curr_talent[TAL_MOTION] < 80 )
         {
            send_to_char( "For some strange reason... nothing happens.\n\r", ch );
            return;
         }

         if( IS_AFFECTED( ch, AFF_CURSE ) && ch->curr_talent[TAL_MOTION] < 70 )
         {
            send_to_char( "You are cursed and cannot recall!\n\r", ch );
            return;
         }

         if( IS_FIGHTING( ch ) && ch->curr_talent[TAL_MOTION] < 60 )
         {
            send_to_char( "You cannot recall from combat!\r\n", ch );
            return;
         }

         act( AT_ACTION, "$n disappears in a swirl of mist.", ch, NULL, NULL, TO_ROOM );
         char_from_room( ch );
         ch->in_obj = NULL;
         char_to_room( ch, location );
         if( ch->mount )
         {
            char_from_room( ch->mount );
            char_to_room( ch->mount, location );
            do_look( ch->mount, "auto" );
         }
         act( AT_ACTION, "$n appears in the room, a ring of mist dissipating around $m.", ch, NULL, NULL, TO_ROOM );
         do_look( ch, "auto" );
         break;

      case MAG_DREAM:
         if( victim == ch )
         {
            if( IS_AFFECTED( ch, AFF_DREAMWORLD ) )
            {
               if( !IS_AFFECTED( ch, AFF_SLEEP ) )
               {
                  act( AT_MAGIC, "You step out of the world of dreams.", ch, NULL, NULL, TO_CHAR );
                  act( AT_MAGIC, "$n slowly fades out of the dream.", ch, NULL, NULL, TO_ROOM );
                  drop_dream_items( ch, ch->last_carrying );
                  if( IS_NPC( ch ) )
                     xREMOVE_BIT( ch->affected_by, AFF_DREAMWORLD );
                  else
                     xREMOVE_BIT( ch->pcdata->perm_aff, AFF_DREAMWORLD );
                  act( AT_MAGIC, "$n slowly fades into the waking world.", ch, NULL, NULL, TO_ROOM );
                  return;
               }
               else
               {
                  send_to_char( "You can't seem to wake up!\n\r", ch );
                  return;
               }
            }

            if( IS_AFFECTED( ch, AFF_VOID ) )
            {
               send_to_char( "The dream twists your mind!\n\r", ch );
               ch->mental_state = 100;
               return;
            }

            if( IS_SET( ch->in_room->room_flags, ROOM_NO_DREAM )
                && ( ch->curr_talent[TAL_DREAM] < 100
                     || ( IS_SET( ch->in_room->runes, RUNE_WARD ) && ch->curr_talent[TAL_DREAM] < 200 ) ) )
            {
               ch_trick( ch, type );
               break;
            }

            act( AT_MAGIC, "You step into the world of dreams.", ch, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n slowly fades out of the waking world.", ch, NULL, NULL, TO_ROOM );
            xSET_BIT( ch->pcdata->perm_aff, AFF_DREAMWORLD );
            ch->position = POS_STANDING;
            act( AT_MAGIC, "$n slowly fades into the dream.", ch, NULL, NULL, TO_ROOM );
            break;
         }
         else
         {
            if( ch->curr_talent[TAL_DREAM] < 50 )
            {
               ch_trick( ch, type );
               break;
            }
            if( !IS_AFFECTED( victim, AFF_DREAMWORLD ) )
            {
               if( IS_AFFECTED( victim, AFF_VOID ) )
               {
                  ch_trick( ch, type );
                  break;
               }
               if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
               {
                  ch_trick( ch, type );
                  break;
               }
               act( AT_MAGIC, "You pull $N into the dream!", ch, NULL, victim, TO_CHAR );
               victim->position = POS_STANDING;
               if( IS_NPC( victim ) )
                  xSET_BIT( victim->affected_by, AFF_DREAMWORLD );
               else
                  xSET_BIT( victim->pcdata->perm_aff, AFF_DREAMWORLD );
               act( AT_MAGIC, "The waking world seems to fade away.", ch, NULL, victim, TO_VICT );
               do_look( victim, "auto" );
               break;
            }
            act( AT_PURPLE, "You extract $N from the dream!", ch, NULL, victim, TO_CHAR );
            victim->position = POS_STANDING;
            if( IS_NPC( victim ) )
               xREMOVE_BIT( victim->affected_by, AFF_DREAMWORLD );
            else
               xREMOVE_BIT( victim->pcdata->perm_aff, AFF_DREAMWORLD );

            act( AT_PURPLE, "The dream vanishes and waking world fades into view.", ch, NULL, victim, TO_VICT );
            do_look( victim, "auto" );
            break;
         }

      case MAG_ANTIMATTER:
         if( number_range( 1, ch->curr_talent[TAL_VOID] ) < 20 )
         {
            ch_trick( ch, type );
            break;
         }
         if( victim == ch )
         {
            if( !IS_AFFECTED( ch, AFF_VOID ) )
            {
               if( IS_AFFECTED( ch, AFF_DREAMWORLD ) )
               {
                  ch_trick( ch, type );
                  break;
               }
               act( AT_DGREY, "You enter the Void.", ch, NULL, NULL, TO_CHAR );
               act( AT_DGREY, "$n steps out of reality.", ch, NULL, NULL, TO_ROOM );
               xSET_BIT( ch->pcdata->perm_aff, AFF_VOID );
               act( AT_DGREY, "$n steps into the Void.", ch, NULL, NULL, TO_ROOM );
               break;
            }
            act( AT_DGREY, "You step out of the Void.", ch, NULL, NULL, TO_CHAR );
            act( AT_DGREY, "$n steps out of the Void.", ch, NULL, NULL, TO_ROOM );
            xREMOVE_BIT( ch->pcdata->perm_aff, AFF_VOID );
            act( AT_DGREY, "$n steps into reality.", ch, NULL, NULL, TO_ROOM );
            break;
         }
         else
         {
            if( ch->curr_talent[TAL_VOID] < 40 )
            {
               ch_trick( ch, type );
               break;
            }
            if( !IS_AFFECTED( victim, AFF_VOID ) )
            {
               if( IS_AFFECTED( victim, AFF_DREAMWORLD ) )
               {
                  ch_trick( ch, type );
                  break;
               }
               if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) )
               {
                  ch_trick( ch, type );
                  break;
               }
               act( AT_MAGIC, "You pull $N into the Void!", ch, NULL, victim, TO_CHAR );
               victim->position = POS_STANDING;
               if( IS_NPC( victim ) )
                  xSET_BIT( victim->affected_by, AFF_VOID );
               else
                  xSET_BIT( victim->pcdata->perm_aff, AFF_VOID );
               act( AT_MAGIC, "Reality seems to fade away into nothingness.", ch, NULL, victim, TO_VICT );
               do_look( victim, "auto" );
               break;
            }
            act( AT_DGREY, "You extract $N from the Void!", ch, NULL, victim, TO_CHAR );
            victim->position = POS_STANDING;
            if( IS_NPC( victim ) )
               xREMOVE_BIT( victim->affected_by, AFF_VOID );
            else
               xREMOVE_BIT( victim->pcdata->perm_aff, AFF_VOID );

            act( AT_DGREY, "The Void vanishes and reality fades into view.", ch, NULL, victim, TO_VICT );
            do_look( victim, "auto" );
            break;
         }
         break;

      case MAG_WIND:
         if( IS_AFFECTED( victim, AFF_FLYING ) )
         {
            if( ch->curr_talent[TAL_WIND] < 20 && ch->curr_talent[TAL_MOTION] < 50 )
            {
               act( AT_CYAN, "You go plummeting toward the ground.", victim, NULL, NULL, TO_CHAR );
               act( AT_CYAN, "$n goes plummeting toward the ground.", victim, NULL, NULL, TO_ROOM );
               lose_hp( victim, victim->weight / 10 );
            }
            else
            {
               act( AT_CYAN, "You settle gracefully to the ground.", victim, NULL, NULL, TO_CHAR );
               act( AT_CYAN, "$n settles gracefully to the ground.", victim, NULL, NULL, TO_ROOM );
            }
            if( IS_NPC( victim ) )
               xREMOVE_BIT( victim->affected_by, AFF_FLYING );
            else
               xREMOVE_BIT( victim->pcdata->perm_aff, AFF_FLYING );
            break;
         }
         if( ch->curr_talent[TAL_WIND] < 30 && ch->curr_talent[TAL_MOTION] < 75 )
         {
            ch_trick( victim, type );
            break;
         }

         act( AT_MAGIC, "Mystic winds carry you off the ground.", victim, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "Mystic winds carry $n off the ground.", victim, NULL, NULL, TO_ROOM );
         if( IS_NPC( victim ) )
            xSET_BIT( victim->affected_by, AFF_FLYING );
         else
            xSET_BIT( victim->pcdata->perm_aff, AFF_FLYING );
         break;

   }
}  /* ch_move */

void summon_move( CHAR_DATA * ch, int type, int power, char *argument )
{
   int i, vnum;
   ROOM_INDEX_DATA *location;
   char buf[MAX_STRING_LENGTH];
   AREA_DATA *pArea;
   EXPLORED_AREA *xarea;
   DEITY_DATA *talent;
   CHAR_DATA *victim;
   CHAR_DATA *next_vict;
   bool found;
   OBJ_DATA *obj;

   location = NULL;

   switch ( type )
   {
      default:
         ch_trick( ch, type );
         break;

      case MAG_TELEKINETIC:
         if( ch->curr_talent[TAL_MOTION] < 20 )
         {
            ch_trick( ch, type );
            break;
         }

         if( IS_FIGHTING( ch ) && ch->curr_talent[TAL_MOTION] < 60 )
         {
            send_to_char( "You can't travel in combat!\n\r", ch );
            return;
         }

         if( !can_travel( 90, ch, ch ) )
         {
            send_to_char( "You feel trapped in this place.\n\r", ch );
            return;
         }
         /*
          * Start travel to location 
          */
         for( pArea = first_area; pArea; pArea = pArea->next )
         {
            if( nifty_is_name_prefix( argument, pArea->name ) )
            {

               /*
                * Only allow you to target areas you've been to already 
                */
               xarea = ch->pcdata->first_explored;
               while( xarea )
               {
                  if( xarea->index == pArea->low_soft_range )
                  {
                     ch_printf( ch, "&BYou bring to mind your memory of %s...\n\r", pArea->name );
                     break;
                  }
                  xarea = xarea->next;
               }
               if( !xarea )
               {
                  send_to_char( "&BYou don't know what that area looks like yet.\n\r", ch );
                  return;
               }

               if( IS_SET( pArea->flags, AFLAG_NOTRAVEL ) )
               {
                  send_to_char( "&BStrong wards surround the area, blocking your magic.\n\r", ch );
                  return;
               }

               for( i = 0; i <= 20; i++ )
               {
                  if( i == 20 )
                  {
                     send_to_char( "&BYou fail to penetrate the wards surrounding that area.\n\r", ch );
                     return;
                  }
                  vnum = number_range( pArea->low_r_vnum, pArea->hi_r_vnum );
                  if( ( location = get_room_index( vnum ) ) == NULL )
                     continue;
                  if( IS_SET( location->room_flags, ROOM_NO_ASTRAL ) )
                  {
                     if( IS_SET( location->runes, RUNE_WARD ) )
                        continue;
                     if( IS_SET( pArea->flags, AFLAG_NOBYPASS ) )
                        continue;
                     if( TALENT( ch, TAL_MOTION ) < 100 )
                        continue;
                  }
                  break;
               }

               if( !location )
               {
                  send_to_char( "You can't find a place to land.\n\r", ch );
                  return;
               }

               if( ch->pcdata->inborn == -1 )
                  talent = get_talent_by_index( TAL_MOTION );
               else
                  talent = get_talent_by_index( ch->pcdata->inborn );

               act( AT_MAGIC, "$T the room, and $n is gone.", ch, NULL, talent->message, TO_ROOM );
               act( AT_MAGIC, "$T the room, and you travel.", ch, NULL, talent->message, TO_CHAR );
               char_from_room( ch );
               ch->in_obj = NULL;
               char_to_room( ch, location );
               if( ch->mount )
               {
                  char_from_room( ch->mount );
                  char_to_room( ch->mount, location );
                  do_look( ch->mount, "auto" );
               }
               act( AT_MAGIC, "$T the room, and $n appears.", ch, NULL, talent->message, TO_ROOM );
               do_look( ch, "auto" );
               learn_talent( ch, TAL_MOTION );
               WAIT_STATE( ch, PULSE_VIOLENCE * 2 - ch->curr_talent[TAL_MOTION] / 5 );

               return;
            }  /* end travel to location */
         }
         if( ch->curr_talent[TAL_MOTION] < 50 )
         {
            act( AT_MAGIC, "You are unable to locate anyplace like that.", ch, NULL, NULL, TO_CHAR );
            return;
         }

         /*
          * start travel to person 
          */
         if( ( victim = get_char_world( ch, argument ) ) == NULL
             || !can_travel( 90, ch, victim ) || ( TALENT( victim, TAL_SECURITY ) > TALENT( ch, TAL_SEEKING ) ) )
         {
            send_to_char( "&BYou are unable to locate anyplace or anyone like that.\n\r", ch );
            return;
         }
         if( victim == ch )
         {
            send_to_char( "That was a short trip!\n\r", ch );
            return;
         }

         if( victim == ch->mount )
         {
            send_to_char( "You are riding them already!\n\r", ch );
            return;
         }

         if( ch->pcdata->inborn == -1 )
            talent = get_talent_by_index( TAL_MOTION );
         else
            talent = get_talent_by_index( ch->pcdata->inborn );

         act( AT_MAGIC, "$T the room, and $n is gone.", ch, NULL, talent->message, TO_ROOM );
         act( AT_MAGIC, "$T the room, and you travel.", ch, NULL, talent->message, TO_CHAR );
         char_from_room( ch );
         ch->in_obj = NULL;
         char_to_room( ch, victim->in_room );
         if( ch->mount )
         {
            char_from_room( ch->mount );
            char_to_room( ch->mount, victim->in_room );
            do_look( ch->mount, "auto" );
         }
         act( AT_MAGIC, "$T the room, and $n appears.", ch, NULL, talent->message, TO_ROOM );
         do_look( ch, "auto" );
         break;

      case MAG_SEEK:
         if( ch->curr_talent[TAL_MOTION] < 40 )
         {
            ch_trick( ch, type );
            break;
         }
         if( IS_SET( ch->in_room->area->flags, AFLAG_NOBYPASS ) )
         {
            ch_trick( ch, type );
            break;
         }
         if( IS_AFFECTED( ch, AFF_VOID ) )
         {
            ch_trick( ch, type );
            break;
         }
         victim = get_char_world( ch, argument );

         if( !victim )
         {
            send_to_char( "You cannot find anything like that.\n\r", ch );
            return;
         }

         if( victim == ch )
         {
            ch_trick( ch, type );
            break;
         }

         if( victim->in_room && !can_travel( 100, ch, victim ) )
         {
            send_to_char( "Something prevents you from opening a rift.\n\r", ch );
            return;
         }

         if( !IS_NPC( victim ) )
         {
            sprintf( buf, "%s summoned %s to %s (%d).", ch->name, victim->name, ch->in_room->name, ch->in_room->vnum );
            to_channel( buf, "Spell", PERMIT_SECURITY );
         }
         act( AT_MAGIC, "A swirling rift opens for a moment, sucking $n through.", victim, NULL, ch, TO_ROOM );
         act( AT_MAGIC, "A swirling rift opens suddenly, and $n pulls you through!", ch, NULL, victim, TO_VICT );
         act( AT_MAGIC, "You open a swirling rift and pull $N through.", ch, NULL, victim, TO_CHAR );
         act( AT_MAGIC, "$n opens a swirling rift and pulls $N through.", ch, NULL, victim, TO_ROOM );
         char_from_room( victim );
         victim->in_obj = NULL;
         char_to_room( victim, ch->in_room );
         do_look( victim, "auto" );
         WAIT_STATE( ch, PULSE_VIOLENCE );
         break;

      case MAG_DREAM:
         if( ch->curr_talent[TAL_DREAM] < 30 )
         {
            ch_trick( ch, type );
            break;
         }

         if( !IS_AFFECTED( ch, AFF_DREAMWORLD ) )
         {
            ch_trick( ch, type );
            break;
         }

         victim = get_char_world( ch, argument );
         if( !victim )
         {
            /*
             * Start travel to location 
             */
            for( pArea = first_area; pArea; pArea = pArea->next )
            {
               if( ( argument[0] == '\0' ||
                     nifty_is_name_prefix( argument, pArea->name ) ) && !IS_SET( pArea->flags, AFLAG_NOTRAVEL ) )
               {

                  /*
                   * Only allow you to target areas you've been to already 
                   */
                  xarea = ch->pcdata->first_explored;
                  while( xarea )
                  {
                     if( xarea->index == pArea->low_soft_range )
                     {
                        ch_printf( ch, "&BYou bring to mind your memory of %s...\n\r", pArea->name );
                        break;
                     }
                     xarea = xarea->next;
                  }
                  if( !xarea )
                  {
                     send_to_char( "&BYou don't know what that area looks like yet.\n\r", ch );
                     return;
                  }

                  for( i = 0; i < 20; i++ )
                  {
                     vnum = number_range( pArea->low_r_vnum, pArea->hi_r_vnum );
                     if( ( location = get_room_index( vnum ) ) == NULL )
                        continue;
                     if( IS_SET( location->room_flags, ROOM_NO_DREAM ) )
                        continue;
                     break;
                  }

                  if( !location )
                  {
                     ch_trick( ch, type );
                     break;
                  }

                  act( AT_MAGIC, "You shift into another dream.", ch, NULL, NULL, TO_CHAR );
                  act( AT_MAGIC, "$n fades out of existance.", ch, NULL, NULL, TO_ROOM );
                  char_from_room( ch );
                  ch->in_obj = NULL;
                  char_to_room( ch, location );
                  act( AT_MAGIC, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
                  do_look( ch, "auto" );
                  return;
               }
            }
            send_to_char( "You can't find anyplace like that.\n\r", ch );
            return;
         }


         if( victim == ch )
         {
            send_to_char( "That was a short trip!\n\r", ch );
            return;
         }

         if( IS_SET( victim->in_room->room_flags, ROOM_NO_DREAM )
             && ( ch->curr_talent[TAL_DREAM] < 100
                  || ( IS_SET( victim->in_room->runes, RUNE_WARD ) && ch->curr_talent[TAL_DREAM] < 200 ) ) )
         {
            act( AT_MAGIC, "You are unable to locate anyone like that.", ch, NULL, NULL, TO_CHAR );
            return;
         }

         act( AT_MAGIC, "You shift into another dream.", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n fades out of existance.", ch, NULL, NULL, TO_ROOM );
         char_from_room( ch );
         ch->in_obj = NULL;
         char_to_room( ch, victim->in_room );
         act( AT_MAGIC, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
         do_look( ch, "auto" );
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_DEATH] + ch->curr_talent[TAL_MOTION] < 95 )
         {
            ch_trick( ch, type );
            break;
         }

         victim = get_char_world( ch, argument );
         if( !victim )
         {
            ch_trick( ch, type );
            break;
         }

         if( victim != ch && !IS_CONSENTING( victim, ch ) )
         {
            send_to_char( "You can't retrieve their corpse without their consent.\n\r", ch );
            return;
         }

         found = FALSE;
         for( obj = first_object; obj; obj = obj->next )
         {
            if( obj->in_room && nifty_is_name( victim->name, obj->name ) && ( obj->pIndexData->vnum == 11 ) )
            {
               found = TRUE;
               if( obj->timer > 38 )
               {
                  act( AT_MAGIC, "The image of your corpse appears, but wavers away.", victim, NULL, NULL, TO_CHAR );
                  act( AT_MAGIC, "The image of $n's corpse appears, but wavers away.", victim, NULL, NULL, TO_ROOM );
                  return;
               }

               if( IS_SET( obj->in_room->room_flags, ROOM_NOSUPPLICATE )
                   && ch->curr_talent[TAL_DEATH] + ch->curr_talent[TAL_MOTION] < 200 )
               {
                  act( AT_MAGIC, "Only silence answers from that forsaken place.", ch, NULL, NULL, TO_CHAR );
                  return;
               }
               act( AT_MAGIC, "Your corpse appears suddenly, surrounded by a misty aura...", victim, NULL, NULL, TO_CHAR );
               act( AT_MAGIC, "$n's corpse appears suddenly, surrounded by a misty aura...", victim, NULL, NULL, TO_ROOM );
               obj_from_room( obj );
               obj = obj_to_room( obj, ch->in_room );
            }
         }
         if( !found )
         {
            send_to_char( "No corpse of that sort litters the world...\n\r", ch );
            return;
         }
         WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
         break;

      case MAG_PSIONIC:
         if( !argument || argument[0] == '\0' )
            vnum = 0;
         else
            vnum = atoi( argument );

         if( !str_cmp( argument, "nexus" ) )
         {
            if( IS_SET( ch->in_room->area->flags, AFLAG_FREEKILL ) )
               location = get_room_index( ROOM_VNUM_PKILL );
            else
               location = get_room_index( ROOM_VNUM_NEXUS );
         }
         else if( !str_cmp( argument, "area" ) )
         {
            location = get_room_index( ch->in_room->area->hi_soft_range );
         }
         else
         {

            i = ( ch->curr_talent[TAL_MOTION] - 20 ) / 10;
            if( i > 7 )
               i = 7;
            if( i < 0 && ch->curr_talent[TAL_MOTION] > 0 )
               i = 0;

            if( vnum > i )
            {
               send_to_char( "You cannot recall the details of that location.\n\r", ch );
               return;
            }

            if( IS_NPC( ch ) )
            {
               if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master && !IS_NPC( ch->master ) )
               {
                  location = get_room_index( ch->master->pcdata->memorize[vnum] );
               }
               else
               {
                  return;
               }
            }
            else
            {
               location = get_room_index( ch->pcdata->memorize[vnum] );
            }
         }

         if( !location )
            location = get_room_index( ROOM_VNUM_NEXUS );

         if( !location )
         {
            send_to_char( "You are completely lost.\n\r", ch );
            return;
         }

         if( ch->in_room == location )
         {
            send_to_char( "You are already here!\n\r", ch );
            return;
         }

         if( IS_AFFECTED( ch, AFF_DREAMWORLD ) && IS_SET( location->room_flags, ROOM_NO_DREAM ) )
         {
            ch_trick( ch, type );
            break;
         }

         if( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) && ch->curr_talent[TAL_MOTION] < 80 )
         {
            ch_trick( ch, type );
            break;
         }

         if( IS_AFFECTED( ch, AFF_CURSE ) && ch->curr_talent[TAL_MOTION] < 70 )
         {
            ch_trick( ch, type );
            break;
         }

         if( IS_FIGHTING( ch ) && ch->curr_talent[TAL_MOTION] < 60 )
         {
            ch_trick( ch, type );
            break;
         }

         act( AT_ACTION, "$n disappears in a swirl of mist.", ch, NULL, NULL, TO_ROOM );
         char_from_room( ch );
         ch->in_obj = NULL;
         char_to_room( ch, location );
         if( ch->mount )
         {
            char_from_room( ch->mount );
            char_to_room( ch->mount, location );
            do_look( ch->mount, "auto" );
         }
         act( AT_ACTION, "$n appears in the room, a ring of mist dissipating around $m.", ch, NULL, NULL, TO_ROOM );
         do_look( ch, "auto" );
         break;

      case MAG_ANTIMATTER:
         if( !IS_AFFECTED( ch, AFF_VOID ) )
         {
            ch_trick( ch, type );
            break;
         }

         victim = get_char_world( ch, argument );
         if( !victim )
         {
            /*
             * Start travel to location 
             */
            for( pArea = first_area; pArea; pArea = pArea->next )
            {
               if( ( argument[0] == '\0' ||
                     nifty_is_name_prefix( argument, pArea->name ) ) && !IS_SET( pArea->flags, AFLAG_NOTRAVEL ) )
               {

                  /*
                   * Only allow you to target areas you've been to already 
                   */
                  xarea = ch->pcdata->first_explored;
                  while( xarea )
                  {
                     if( xarea->index == pArea->low_soft_range )
                     {
                        ch_printf( ch, "&BYou bring to mind your memory of %s...\n\r", pArea->name );
                        break;
                     }
                     xarea = xarea->next;
                  }
                  if( !xarea )
                  {
                     send_to_char( "&BYou don't know what that area looks like yet.\n\r", ch );
                     return;
                  }

                  for( i = 0; i < 20; i++ )
                  {
                     vnum = number_range( pArea->low_r_vnum, pArea->hi_r_vnum );
                     if( ( location = get_room_index( vnum ) ) == NULL )
                        continue;
                     break;
                  }

                  if( !location )
                  {
                     ch_trick( ch, type );
                     break;
                  }


                  act( AT_DGREY, "You shift into another part of the Void.", ch, NULL, NULL, TO_CHAR );
                  act( AT_DGREY, "$n fades out of existance.", ch, NULL, NULL, TO_ROOM );
                  char_from_room( ch );
                  ch->in_obj = NULL;
                  char_to_room( ch, location );
                  act( AT_DGREY, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
                  do_look( ch, "auto" );
               }
            }
            ch_trick( ch, type );
            break;
         }

         if( victim == ch )
         {
            ch_trick( ch, type );
            break;
         }

         act( AT_DGREY, "You shift into another part of the Void.", ch, NULL, NULL, TO_CHAR );
         act( AT_DGREY, "$n fades out of existance.", ch, NULL, NULL, TO_ROOM );
         char_from_room( ch );
         ch->in_obj = NULL;
         char_to_room( ch, victim->in_room );
         act( AT_DGREY, "$n slows fades into view.", ch, NULL, NULL, TO_ROOM );
         do_look( ch, "auto" );
         break;

      case MAG_ETHER:
         if( ch->curr_talent[TAL_VOID] < 80 )
         {
            ch_trick( ch, type );
            break;
         }
         if( !IS_AFFECTED( ch, AFF_VOID ) )
         {
            ch_trick( ch, type );
            break;
         }

         victim = get_char_world( ch, argument );
         if( !victim || ( IS_NPC( victim ) && !xIS_SET( victim->act, ACT_NAMED ) ) )
            victim = generate_mob_nation( argument );

         if( !victim )
         {
            ch_trick( ch, type );
            break;
         }

         if( victim == ch )
         {
            ch_trick( ch, type );
            break;
         }

         if( victim->in_room )
         {
            act( AT_DGREY, "$n tears a rift in reality and pulls you through!", ch, NULL, victim, TO_VICT );
            act( AT_DGREY, "$n is ripped through a rift in reality!", victim, NULL, NULL, TO_ROOM );
            char_from_room( victim );
         }
         victim->in_obj = NULL;
         char_to_room( victim, ch->in_room );
         if( IS_NPC( victim ) )
            xSET_BIT( victim->affected_by, AFF_VOID );
         else
            xSET_BIT( victim->pcdata->perm_aff, AFF_VOID );

         act( AT_DGREY, "$n is pulled into the Void!", victim, NULL, NULL, TO_ROOM );
         do_look( victim, "auto" );
         break;

      case MAG_BLUNT:
         if( ( ch->curr_talent[TAL_EARTH] < 20 && !IS_NPC( ch ) ) || IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) )
         {
            ch_trick( ch, type );
            break;
         }

         act( AT_HUNGRY, "The earth trembles beneath you!", ch, NULL, NULL, TO_CHAR );
         act( AT_HUNGRY, "The ground begins to tremble and shake!", ch, NULL, NULL, TO_ROOM );
         for( victim = ch->in_room->first_person; victim; victim = next_vict )
         {
            next_vict = victim->next_in_room;

            if( IS_AFFECTED( victim, AFF_FLYING ) && !IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
               continue;

            ch_harm( victim, ch, power, MAG_BLUNT, FALSE );
            if( char_died( ch ) )
               return;
         }
         break;

   }
}  /* summon_move */

void obj_change( CHAR_DATA * ch, OBJ_DATA * obj, int type, int power, char *argument )
{
   CHAR_DATA *mob;
   OBJ_DATA *obj1;
   OBJ_DATA *obj_next;
   MOB_INDEX_DATA *pMobIndex;
   AFFECT_DATA af;
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   PART_DATA *part;
   int amt;
   bool found;

   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_DEATH] < 30 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->item_type == ITEM_CORPSE_PC && ch->curr_talent[TAL_DEATH] >= 50 )
         {
            for( d = first_descriptor; d; d = d->next )
            {
               if( d->connected == CON_PLAYING )
               {
                  if( nifty_is_name( d->character->name, obj->name ) )
                  {
                     victim = d->character;
                     break;
                  }
               }
            }
            if( !victim )
            {
               obj_trick( ch, obj, type );
               break;
            }
            if( !IS_CONSENTING( victim, ch ) )
            {
               send_to_char( "You cannot reanimate them without their consent.\n\r", ch );
               return;
            }
            if( power < victim->perm_con * 10 )
            {
               obj_trick( ch, obj, type );
               break;
            }
            if( !IS_AFFECTED( victim, AFF_ETHEREAL ) )
            {
               send_to_char( "You cannot reanimate them when they are not ethereal.\n\r", ch );
               return;
            }
            while( ( obj = victim->first_carrying ) )
            {
               obj_from_char( obj );
               obj_to_room( obj, victim->in_room );
            }
            while( victim->first_affect )
               affect_remove( victim, victim->first_affect );

            act( AT_DGREY, "$n screams in agony as $e is drawn back into $s body.", victim, NULL, NULL, TO_ROOM );
            act( AT_DGREY, "You scream in agony as you are drawn back into your body.", victim, NULL, NULL, TO_CHAR );
            if( victim->in_room != ch->in_room )
            {
               char_from_room( victim );
               victim->in_obj = NULL;
               char_to_room( victim, ch->in_room );
            }
            part = victim->first_part;
            while( part )
            {
               part->cond = PART_WELL;
               part->flags = PART_WELL;
               part = part->next;
            }
            victim->exp += obj->cost;
            if( get_timer( victim, TIMER_PKILLED ) > 0 )
               remove_timer( victim, TIMER_PKILLED );
            victim->weight = obj->weight;

            act( AT_DGREY, "An unholy black aura surrounds the corpse of $n.", victim, NULL, NULL, TO_ROOM );
            while( ( obj1 = obj->first_content ) )
            {
               obj_from_obj( obj1 );
               if( obj1->item_type == ITEM_MONEY )
               {
                  amt = obj1->value[0];
                  ch->gold += amt;
                  extract_obj( obj1 );
               }
               else
               {
                  obj_to_char( obj1, victim );
               }
            }
            separate_obj( obj );
            extract_obj( obj );
            fix_char( victim );
            victim->position = POS_RESTING;
            xCLEAR_BITS( victim->pcdata->perm_aff );
            xSET_BIT( victim->pcdata->perm_aff, AFF_UNDEAD );
            found = TRUE;
            if( !str_cmp( argument, "lich" ) && ch->curr_talent[TAL_DEATH] >= 150 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_FEAR );
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NONLIVING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
            }
            else if( !str_cmp( argument, "vampire" ) && ch->curr_talent[TAL_DEATH] >= 100 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
               SET_BIT( victim->pcdata->flags, PCFLAG_VAMPIRE );
            }
            else if( !str_cmp( argument, "skeleton" ) && ch->curr_talent[TAL_DEATH] >= 70 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NONLIVING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
               xSET_BIT( victim->pcdata->perm_aff, AFF_CONSTRUCT );
            }
            else if( !str_cmp( argument, "ghoul" ) && ch->curr_talent[TAL_DEATH] >= 60 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
            }
            else if( !str_cmp( argument, "wraith" ) && ch->curr_talent[TAL_DEATH] >= 110 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_ETHEREAL );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NONLIVING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NO_CORPSE );
               xSET_BIT( victim->pcdata->perm_aff, AFF_PASS_DOOR );
            }
            else if( !str_cmp( argument, "shadow" ) && ch->curr_talent[TAL_DEATH] >= 130 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_ETHEREAL );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NONLIVING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NO_CORPSE );
               xSET_BIT( victim->pcdata->perm_aff, AFF_DARK );
               xSET_BIT( victim->pcdata->perm_aff, AFF_UNHOLY );
               xSET_BIT( victim->pcdata->perm_aff, AFF_PASS_DOOR );
            }
            else if( !str_cmp( argument, "ghost" ) && ch->curr_talent[TAL_DEATH] >= 80 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_ETHEREAL );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NONLIVING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NO_CORPSE );
               xSET_BIT( victim->pcdata->perm_aff, AFF_CURSE );
               xSET_BIT( victim->pcdata->perm_aff, AFF_FLOATING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_PASS_DOOR );
            }
            else if( !str_cmp( argument, "banshee" ) && ch->curr_talent[TAL_DEATH] >= 140 )
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_ETHEREAL );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NONLIVING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NO_CORPSE );
               xSET_BIT( victim->pcdata->perm_aff, AFF_FEAR );
               xSET_BIT( victim->pcdata->perm_aff, AFF_FLOATING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_PASS_DOOR );
            }
            else
            {
               xSET_BIT( victim->pcdata->perm_aff, AFF_INFRARED );
               xSET_BIT( victim->pcdata->perm_aff, AFF_NONLIVING );
               xSET_BIT( victim->pcdata->perm_aff, AFF_AQUA_BREATH );
               found = FALSE;
            }
            STRFREE( victim->pcdata->type );
            if( found )
               victim->pcdata->type = STRALLOC( argument );
            else
               victim->pcdata->type = STRALLOC( "zombie" );
            act( AT_DGREY, "$n's eyes snap open, glowing with a faint reddish glow.", victim, NULL, NULL, TO_ROOM );
            act( AT_DGREY, "Your eyes snap open, and you look on the world with the sight of unlife.",
                 victim, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "You feel drained and exhausted from the reanimation.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = -100;
            STRFREE( ch->last_taken );
            sprintf( buf, "reanimating %s", victim->name );
            ch->last_taken = STRALLOC( buf );
            WAIT_STATE( ch, PULSE_VIOLENCE * 10 );
            STRFREE( victim->last_taken );
            ch->last_taken = STRALLOC( "being reanimated" );
            WAIT_STATE( victim, PULSE_VIOLENCE * 10 );
            return;
         }
         /*
          * end player reanimate 
          */

         if( get_mob_index( MOB_VNUM_ANIMATED_CORPSE ) == NULL )
         {
            bug( "Vnum 5 not found for do_reanimate!", 0 );
            return;
         }
         if( ( pMobIndex = get_mob_index( ( sh_int ) abs( obj->cost ) ) ) == NULL )
         {
            bug( "Can not find mob for cost of corpse, do_reanimate", 0 );
            return;
         }
         if( xIS_SET( pMobIndex->affected_by, AFF_NONLIVING ) )
         {
            obj_trick( ch, obj, type );
            break;
         }
         if( power - ( pMobIndex->perm_con * 4 ) < 0 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( chance( ch, 75 ) && pMobIndex->perm_con - ch->curr_talent[TAL_DEATH] < 10 )
         {
            mob = create_mobile( get_mob_index( MOB_VNUM_ANIMATED_CORPSE ) );
            char_to_room( mob, ch->in_room );
            mob->race = pMobIndex->race;
            xSET_BIT( mob->affected_by, AFF_UNDEAD );
            xSET_BIT( mob->affected_by, AFF_NONLIVING );
            xSET_BIT( mob->affected_by, AFF_TRUESIGHT );

            /*
             * Fix so mobs wont have 0 hps and crash mud - Scryn 2/20/96 
             */
            if( !pMobIndex->hitnodice )
               mob->max_hit = ( pMobIndex->perm_con * TALENT( ch, TAL_DEATH ) ) / 10;
            else
               mob->max_hit = UMAX( URANGE( mob->max_hit / 4,
                                            ( mob->max_hit * obj->value[3] ) / 10,
                                            ch->curr_talent[TAL_DEATH] * dice( 20, 10 ) ), 1 );

            xSET_BIT( mob->affected_by, AFF_INFRARED );
            mob->hit = mob->max_hit;
            mob->damroll = TALENT( ch, TAL_DEATH ) / 3;
            mob->hitroll = TALENT( ch, TAL_DEATH ) / 2;

            mob->perm_str = obj->value[0];
            mob->perm_int = obj->value[1];
            mob->perm_wis = obj->value[2];
            mob->perm_dex = obj->value[3];
            mob->perm_con = obj->value[4];
            mob->perm_cha = obj->value[5];
            mob->perm_lck = obj->value[6];

            act( AT_MAGIC, "$n makes $N rise from the grave!", ch, NULL, mob, TO_ROOM );
            act( AT_MAGIC, "You make $N rise from the grave!", ch, NULL, mob, TO_CHAR );

            sprintf( buf, "animated %s", obj->name );
            STRFREE( mob->name );
            mob->name = STRALLOC( buf );

            sprintf( buf, "animated %s", obj->short_descr );
            STRFREE( mob->short_descr );
            mob->short_descr = STRALLOC( buf );

            add_follower( mob, ch );
            if( obj->timer > 0 )
            {
               af.type = -1;
               af.duration = ( number_fuzzy( obj->timer ) * DUR_CONV );
               af.location = 0;
               af.modifier = 0;
               af.bitvector = meb( AFF_CHARM );
               affect_to_char( mob, &af );
            }
            else
            {
               xSET_BIT( mob->affected_by, AFF_CHARM );
            }

            /*
             * run through the part flags on the corpse and give it the
             * * ones it should have. if a part was severed before, it won't
             * * have it now. i wish this wasn't necessary, but its needed to
             * * be able to reattach them properly.
             */
            if( xIS_SET( obj->parts, BP_CHEST ) )
               add_part( mob, BP_CHEST, -1 );
            if( xIS_SET( obj->parts, BP_NECK ) )
               add_part( mob, BP_NECK, BP_CHEST );
            if( xIS_SET( obj->parts, BP_HEAD ) )
               add_part( mob, BP_HEAD, BP_NECK );
            if( xIS_SET( obj->parts, BP_REAR ) )
               add_part( mob, BP_REAR, BP_HEAD );
            if( xIS_SET( obj->parts, BP_LEAR ) )
               add_part( mob, BP_LEAR, BP_HEAD );
            if( xIS_SET( obj->parts, BP_NOSE ) )
               add_part( mob, BP_NOSE, BP_HEAD );
            if( xIS_SET( obj->parts, BP_REYE ) )
               add_part( mob, BP_REYE, BP_HEAD );
            if( xIS_SET( obj->parts, BP_LEYE ) )
               add_part( mob, BP_LEYE, BP_HEAD );
            if( xIS_SET( obj->parts, BP_TONGUE ) )
               add_part( mob, BP_TONGUE, BP_HEAD );
            if( xIS_SET( obj->parts, BP_RHORN ) )
               add_part( mob, BP_RHORN, BP_HEAD );
            if( xIS_SET( obj->parts, BP_LHORN ) )
               add_part( mob, BP_LHORN, BP_HEAD );
            if( xIS_SET( obj->parts, BP_RARM ) )
               add_part( mob, BP_RARM, BP_CHEST );
            if( xIS_SET( obj->parts, BP_LARM ) )
               add_part( mob, BP_LARM, BP_CHEST );
            if( xIS_SET( obj->parts, BP_STOMACH ) )
               add_part( mob, BP_STOMACH, -1 );
            if( xIS_SET( obj->parts, BP_RHAND ) )
               add_part( mob, BP_RWRIST, BP_RARM );
            if( xIS_SET( obj->parts, BP_LHAND ) )
               add_part( mob, BP_LWRIST, BP_LARM );
            if( xIS_SET( obj->parts, BP_RHAND ) )
               add_part( mob, BP_RHAND, BP_RWRIST );
            if( xIS_SET( obj->parts, BP_LHAND ) )
               add_part( mob, BP_LHAND, BP_LWRIST );
            if( xIS_SET( obj->parts, BP_RLEG ) )
               add_part( mob, BP_RLEG, BP_ASS );
            if( xIS_SET( obj->parts, BP_LLEG ) )
               add_part( mob, BP_LLEG, BP_ASS );
            if( xIS_SET( obj->parts, BP_RFOOT ) )
               add_part( mob, BP_RANKLE, BP_RLEG );
            if( xIS_SET( obj->parts, BP_LANKLE ) )
               add_part( mob, BP_LANKLE, BP_LLEG );
            if( xIS_SET( obj->parts, BP_RFOOT ) )
               add_part( mob, BP_RFOOT, BP_RANKLE );
            if( xIS_SET( obj->parts, BP_LFOOT ) )
               add_part( mob, BP_LFOOT, BP_LANKLE );
            if( xIS_SET( obj->parts, BP_ASS ) )
               add_part( mob, BP_ASS, -1 );
            if( xIS_SET( obj->parts, BP_NOSE ) )
               add_part( mob, BP_BACK, -1 );
            if( xIS_SET( obj->parts, BP_RWING ) )
               add_part( mob, BP_RWING, BP_BACK );
            if( xIS_SET( obj->parts, BP_LWING ) )
               add_part( mob, BP_LWING, BP_BACK );
            if( xIS_SET( obj->parts, BP_RPAW ) )
               add_part( mob, BP_RPAW, BP_RANKLE );
            if( xIS_SET( obj->parts, BP_LPAW ) )
               add_part( mob, BP_LPAW, BP_LANKLE );
            if( xIS_SET( obj->parts, BP_RHOOF ) )
               add_part( mob, BP_RHOOF, BP_RANKLE );
            if( xIS_SET( obj->parts, BP_LHOOF ) )
               add_part( mob, BP_LHOOF, BP_LANKLE );
            if( xIS_SET( obj->parts, BP_CLAWS ) )
               add_part( mob, BP_CLAWS, BP_RPAW );
            if( xIS_SET( obj->parts, BP_TAIL ) )
               add_part( mob, BP_TAIL, BP_ASS );
            if( xIS_SET( obj->parts, BP_FANGS ) )
               add_part( mob, BP_FANGS, BP_HEAD );
            if( xIS_SET( obj->parts, BP_RFLEG ) )
               add_part( mob, BP_RFLEG, BP_CHEST );
            if( xIS_SET( obj->parts, BP_LFLEG ) )
               add_part( mob, BP_LFLEG, BP_CHEST );
            if( xIS_SET( obj->parts, BP_RRLEG ) )
               add_part( mob, BP_RRLEG, BP_ASS );
            if( xIS_SET( obj->parts, BP_LRLEG ) )
               add_part( mob, BP_LRLEG, BP_ASS );
            if( xIS_SET( obj->parts, BP_RFIN ) )
               add_part( mob, BP_RFIN, BP_CHEST );
            if( xIS_SET( obj->parts, BP_LFIN ) )
               add_part( mob, BP_LFIN, BP_CHEST );
            if( xIS_SET( obj->parts, BP_DFIN ) )
               add_part( mob, BP_DFIN, BP_BACK );

            if( obj->first_content )
               for( obj1 = obj->first_content; obj1; obj1 = obj_next )
               {
                  obj_next = obj1->next_content;
                  obj_from_obj( obj1 );
                  obj_to_char( obj1, mob );
                  equip_char( mob, obj1 );
               }

            separate_obj( obj );
            extract_obj( obj );
            STRFREE( ch->last_taken );
            sprintf( buf, "reanimating %s", mob->name );
            ch->last_taken = STRALLOC( buf );
            WAIT_STATE( ch, PULSE_VIOLENCE * 4 );
            STRFREE( mob->last_taken );
            ch->last_taken = STRALLOC( "being reanimated" );
            WAIT_STATE( mob, PULSE_VIOLENCE * 4 );
            return;
         }
         else
         {
            send_to_char( "You fail to reanimate the corpse.\n\r", ch );
            use_magic( ch, TAL_DEATH, 300 - ch->curr_talent[TAL_DEATH] * 2 );
            STRFREE( ch->last_taken );
            ch->last_taken = STRALLOC( "trying to reanimate a corpse" );
            WAIT_STATE( ch, PULSE_VIOLENCE * 2 );
         }
         break;

      case MAG_HOLY:
         if( ch->curr_talent[TAL_HEALING] < 100 )
         {
            obj_trick( ch, obj, type );
            break;
         }
         if( obj->item_type != ITEM_CORPSE_PC )
         {
            obj_trick( ch, obj, type );
            break;
         }
         for( d = first_descriptor; d; d = d->next )
         {
            if( d->connected == CON_PLAYING )
            {
               if( nifty_is_name( d->character->name, obj->name ) )
               {
                  victim = d->character;
                  break;
               }
            }
         }
         if( !victim )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( power < victim->perm_con * 10 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !IS_CONSENTING( victim, ch ) )
         {
            send_to_char( "You cannot resurrect them without their consent.\n\r", ch );
            return;
         }

         if( !IS_AFFECTED( victim, AFF_ETHEREAL ) )
         {
            send_to_char( "You cannot resurrect them when they are not ethereal.\n\r", ch );
            return;
         }

         while( ( obj1 = victim->first_carrying ) )
         {
            obj_from_char( obj );
            obj_to_room( obj1, victim->in_room );
         }

         while( victim->first_affect )
            affect_remove( victim, victim->first_affect );

         act( AT_DGREY, "$n screams in agony as $e is drawn back into $s body.", victim, NULL, NULL, TO_ROOM );
         act( AT_DGREY, "You scream in agony as you are drawn back into your body.", victim, NULL, NULL, TO_CHAR );
         if( victim->in_room != ch->in_room )
         {
            char_from_room( victim );
            victim->in_obj = NULL;
            char_to_room( victim, ch->in_room );
         }
         part = victim->first_part;
         while( part )
         {
            part->cond = PART_WELL;
            part->flags = PART_WELL;
            part = part->next;
         }
         victim->exp += obj->cost;
         if( get_timer( victim, TIMER_PKILLED ) > 0 )
            remove_timer( victim, TIMER_PKILLED );

         act( AT_WHITE, "A radiant white aura surrounds the corpse of $n.", victim, NULL, NULL, TO_ROOM );
         while( ( obj1 = obj->first_content ) )
         {
            obj_from_obj( obj1 );
            if( obj1->item_type == ITEM_MONEY )
            {
               amt = obj->value[0];
               ch->gold += amt;
               extract_obj( obj1 );
            }
            else
            {
               obj_to_char( obj1, victim );
            }
         }
         separate_obj( obj );
         extract_obj( obj );
         fix_char( victim );
         victim->position = POS_RESTING;
         act( AT_WHITE, "$n's eyes flutter open, and $e takes in a deep breath.", victim, NULL, NULL, TO_ROOM );
         act( AT_WHITE, "Your eyes flutter open, and you take in a deep breath.", victim, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "You feel drained and exhausted from the resurrection.", ch, NULL, NULL, TO_CHAR );
         ch->mental_state = -100;
         STRFREE( ch->last_taken );
         sprintf( buf, "resurrecting %s", victim->name );
         ch->last_taken = STRALLOC( buf );
         WAIT_STATE( ch, PULSE_VIOLENCE * 10 );
         STRFREE( victim->last_taken );
         ch->last_taken = STRALLOC( "being resurrected" );
         WAIT_STATE( victim, PULSE_VIOLENCE * 10 );
         /*
          * If they were undead, resurrection will revive them 
          */
         if( xIS_SET( victim->pcdata->perm_aff, AFF_UNDEAD ) )
         {
            STRFREE( victim->pcdata->type );
            victim->pcdata->type = STRALLOC( "living" );
            xCLEAR_BITS( victim->pcdata->perm_aff );
            REMOVE_BIT( victim->pcdata->flags, PCFLAG_VAMPIRE );
         }
         break;

      case MAG_LIGHT:
         if( ch->curr_talent[TAL_ILLUSION] + ch->curr_talent[TAL_CHANGE] + ch->curr_talent[TAL_CATALYSM] < 100 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !argument || argument[0] == '\0' )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->material && obj->material->number == 116 )
         {  /* ether */
            obj_trick( ch, obj, type );
            break;
         }

         separate_obj( obj ); /* Thanks Zemus */

         act( AT_MAGIC, "Your magic twists around $p, making it look like $T!", ch, obj, argument, TO_CHAR );
         act( AT_MAGIC, "$n's magic twists around $p, and suddenly $e is holding $T!", ch, obj, argument, TO_ROOM );

         STRFREE( obj->name );
         STRFREE( obj->short_descr );
         STRFREE( obj->description );
         obj->name = STRALLOC( munch_colors( argument ) );
         obj->short_descr = STRALLOC( argument );
         strcpy( buf, argument );
         strcat( buf, " is here." );
         obj->description = STRALLOC( buf );
         break;

      case MAG_ETHER:
         if( ch->curr_talent[TAL_VOID] < 30 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !argument || argument[0] == '\0' )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !obj->material || obj->material->number != 116 )
         {  /* ether */
            obj_trick( ch, obj, type );
            break;
         }

         separate_obj( obj ); /* Thanks Zemus */

         act( AT_MAGIC, "Your magic twists around $p, making it look like $T!", ch, obj, argument, TO_CHAR );
         act( AT_MAGIC, "$n's magic twists around $p, and suddenly $e is holding $T!", ch, obj, argument, TO_ROOM );

         STRFREE( obj->name );
         STRFREE( obj->short_descr );
         STRFREE( obj->description );
         obj->name = STRALLOC( munch_colors( argument ) );
         obj->short_descr = STRALLOC( argument );
         strcpy( buf, argument );
         strcat( buf, " is here." );
         obj->description = STRALLOC( buf );
         break;

      case MAG_DREAM:
         if( ch->curr_talent[TAL_DREAM] < 40 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( IS_OBJ_STAT( obj, ITEM_ARTIFACT ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !argument || argument[0] == '\0' )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !IS_OBJ_STAT( obj, ITEM_DREAMWORLD ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         separate_obj( obj ); /* Thanks Zemus */

         act( AT_MAGIC, "Your magic twists around $p, making it look like $T!", ch, obj, argument, TO_CHAR );
         act( AT_MAGIC, "$n's magic twists around $p, and suddenly $e is holding $T!", ch, obj, argument, TO_ROOM );

         STRFREE( obj->name );
         STRFREE( obj->short_descr );
         STRFREE( obj->description );
         obj->name = STRALLOC( munch_colors( argument ) );
         obj->short_descr = STRALLOC( argument );
         strcpy( buf, argument );
         strcat( buf, " is here." );
         obj->description = STRALLOC( buf );
         break;
   }
}

void ch_change( CHAR_DATA * victim, CHAR_DATA * ch, int type, char *argument )
{
   NATION_DATA *nation;
   SPECIES_DATA *species;
   OBJ_DATA *obj;
   int i;
   char arg1[MAX_STRING_LENGTH];

   switch ( type )
   {
      default:
         ch_trick( victim, type );
         break;

      case MAG_CHANGE:
         if( !argument || argument[0] == '\0' )
         {
            ch_trick( victim, type );
            break;
         }
         if( ch->curr_talent[TAL_CHANGE] < 30 )
         {
            ch_trick( victim, type );
            break;
         }
         if( ch->mount )
         {
            send_to_char( "And scare your mount?\n\r", ch );
            return;
         }
         if( IS_NPC( victim ) )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_AFFECTED( ch, AFF_MOUNTED ) )
            throw_rider( ch );

         if( !str_cmp( argument, "anthro" ) )
         {
            if( has_bodypart( victim, BP_RHAND ) )
            {
               ch_trick( victim, type );
               break;
            }
            if( victim->height > 100 )
            {
               ch_trick( victim, type );
               break;
            }
            xSET_BIT( victim->pcdata->perm_aff, AFF_ANTHRO );
            check_bodyparts( victim );
            if( victim->height < 60 )
               victim->height = number_range( 36, 60 );
            else
               victim->height = number_range( 60, 76 );
            victim->weight = get_height_weight( victim );
            STRFREE( victim->pcdata->type );
            victim->pcdata->type = STRALLOC( "anthropomorphic" );
            act( AT_MAGIC, "Your body warps into an anthropomorphic form.", victim, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n's body warps into an anthropomorphic form.", ch, NULL, NULL, TO_NOTVICT );
            if( victim != ch )
               act( AT_MAGIC, "You warp $N's body into an anthropomorphic form.", ch, NULL, victim, TO_CHAR );
            return;
         }

         if( victim != ch )
         {
            if( TALENT( ch, TAL_CHANGE ) < 80 )
            {
               ch_trick( victim, type );
               break;
            }

            if( !IS_CONSENTING( victim, ch ) && TALENT( ch, TAL_CHANGE ) < 400 )
            {
               send_to_char( "You cannot do that without their consent.\n\r", ch );
               return;
            }

         }

         nation = find_nation( argument );

         if( !nation )
         {
            ch_trick( victim, type );
            break;
         }

         if( str_cmp( nation->species, victim->nation->species ) && ( ch->curr_talent[TAL_CHANGE] < 60 ) )
         {
            ch_trick( victim, type );
            break;
         }
         de_equip_char( victim );

         if( IS_AFFECTED( victim, AFF_FLYING ) )
            do_land( victim, "" );

         species = find_species( nation->species );
         if( !species )
         {
            bug( "do_change: cannot find species!" );
            send_to_char( "You suddenly forget what one of those is.", ch );
            return;
         }
         ch_printf( victim, "Your body changes and warps, and you become %s.\n\r", aoran( nation->name ) );
         act( AT_MAGIC, "$N's body changes and warps, and $E becomes $t.", ch, aoran( nation->name ), victim, TO_NOTVICT );
         if( ch != victim )
         {
            act( AT_MAGIC, "You change and warp $N's body into $t.", ch, aoran( nation->name ), victim, TO_CHAR );
         }

         if( IS_AFFECTED( victim, AFF_ANTHRO ) )
         {
            xREMOVE_BIT( victim->pcdata->perm_aff, AFF_ANTHRO );
            STRFREE( victim->pcdata->type );
            if( IS_AFFECTED( victim, AFF_UNDEAD ) )
               victim->pcdata->type = STRALLOC( "undead" );
            else
               victim->pcdata->type = STRALLOC( "living" );
         }
         victim->nation = nation;
         set_race( victim, FALSE );
         if( !IS_SET( nation->parts, PART_HANDS ) )
         {
            if( ch->main_hand )
            {
               obj_unaffect_ch( ch, ch->main_hand );
               ch->main_hand = NULL;
            }
            if( ch->off_hand )
            {
               obj_unaffect_ch( ch, ch->off_hand );
               ch->off_hand = NULL;
            }
         }
         if( ch->curr_talent[TAL_CHANGE] >= 50 )
         {
            for( obj = victim->first_carrying; obj; obj = obj->next_content )
            {
               obj->size = victim->height;
            }
         }
         re_equip_char( victim );
         update_aris( victim );
         victim->max_hit = UMAX( 20, victim->nation->hit * get_curr_con( victim ) + victim->base_hit );
         victim->max_mana = UMAX( 20, victim->nation->mana * get_curr_int( victim ) + victim->base_mana );
         victim->max_move = UMAX( 20, 25 * get_curr_end( victim ) + victim->base_move );
         victim->mana = UMIN( victim->max_mana, victim->mana );
         break;

      case MAG_TIME:
         if( ch->curr_talent[TAL_TIME] < 30 )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_NPC( victim ) )
         {
            ch_trick( victim, type );
            break;
         }
         i = atoi( argument );
         if( i < 1 || i > TALENT( ch, TAL_TIME ) )
         {
            ch_trick( victim, type );
            break;
         }
         if( !IS_CONSENTING( victim, ch ) && TALENT( ch, TAL_TIME ) < 400 )
         {
            send_to_char( "You cannot do that without their consent.\n\r", ch );
            return;
         }
         WAIT_STATE( ch, abs( victim->pcdata->age_adjust - i ) / 10 );
         if( victim->pcdata->age_adjust == i )
         {
            ch_trick( victim, type );
            break;
         }
         act( AT_BLUE, "$N's body changes as $E becomes $t.",
              ch, victim->pcdata->age_adjust < i ? "older" : "younger", victim, TO_NOTVICT );
         act( AT_BLUE, "Your body changes as you become $t.",
              victim, victim->pcdata->age_adjust < i ? "older" : "younger", NULL, TO_CHAR );
         if( ch != victim )
            act( AT_BLUE, "You change $N's body to become $t.",
                 ch, victim->pcdata->age_adjust < i ? "older" : "younger", victim, TO_CHAR );
         victim->pcdata->age_adjust = i;
         break;

      case MAG_SEEK:
         if( ch->curr_talent[TAL_CHANGE] < 25 )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_NPC( victim ) )
         {
            ch_trick( victim, type );
            break;
         }
         argument = one_argument( argument, arg1 );
         if( !argument || !arg1 )
         {
            ch_trick( victim, type );
            break;
         }

         if( victim != ch )
         {
            if( TALENT( ch, TAL_CHANGE ) < 50 )
            {
               send_to_char( "You are not strong enough to change others yet.\n\r", ch );
               return;
            }

            if( !IS_CONSENTING( victim, ch ) && TALENT( ch, TAL_CHANGE ) < 100 )
            {
               send_to_char( "You cannot do that without their consent.\n\r", ch );
               return;
            }
         }

         if( !str_cmp( arg1, "height" ) )
         {
            i = atoi( argument );
            if( i >= victim->nation->height * ( 1 + TALENT( ch, TAL_CHANGE ) / 100 ) )
            {
               send_to_char( "You can't make them that tall.\n\r", ch );
               return;
            }
            if( i <= victim->nation->height -
                victim->nation->height * TALENT( ch, TAL_CHANGE ) / 100 || victim->height <= 10 )
            {
               send_to_char( "You can't make them that short.\n\r", ch );
               return;
            }
            victim->height = i;
            if( victim != ch )
            {
               act( AT_MAGIC, "Your height changes.", ch, NULL, victim, TO_VICT );
               act( AT_MAGIC, "You make $N's height change.", ch, NULL, victim, TO_CHAR );
            }
            else
            {
               act( AT_MAGIC, "You change your height.", ch, NULL, NULL, TO_CHAR );
            }
            act( AT_MAGIC, "$N's height changes.", ch, NULL, victim, TO_NOTVICT );
         }
         else if( !str_cmp( arg1, "weight" ) )
         {
            i = atoi( argument );
            if( i >= victim->nation->weight * ( 1 + TALENT( ch, TAL_CHANGE ) / 100 ) )
            {
               send_to_char( "You can't make them that heavy.\n\r", ch );
               return;
            }
            if( i <= victim->nation->weight -
                victim->nation->weight * TALENT( ch, TAL_CHANGE ) / 100 || victim->weight <= 10 )
            {
               send_to_char( "You can't make them that light.\n\r", ch );
               return;
            }
            victim->weight = i;
            if( victim != ch )
            {
               if( victim != ch )
               {
                  act( AT_MAGIC, "Your weight changes.", ch, NULL, victim, TO_VICT );
                  act( AT_MAGIC, "You make $N's weight change.", ch, NULL, victim, TO_CHAR );
               }
               else
               {
                  act( AT_MAGIC, "You change your weight.", ch, NULL, NULL, TO_CHAR );
               }
               act( AT_MAGIC, "$N's weight changes.", ch, NULL, victim, TO_NOTVICT );
            }
         }
         else if( !str_cmp( arg1, "sex" ) )
         {
            if( !str_prefix( "f", argument ) )
               victim->sex = SEX_FEMALE;
            else if( !str_prefix( "m", argument ) )
               victim->sex = SEX_MALE;
            else if( !str_prefix( "n", argument ) )
               victim->sex = SEX_NEUTRAL;
            else if( !str_prefix( "h", argument ) )
               victim->sex = SEX_HERMAPH;
            if( victim != ch )
            {
               act( AT_MAGIC, "You feel something strange change in your crotch.", ch, NULL, victim, TO_VICT );
               act( AT_MAGIC, "You change $N's gender.", ch, NULL, victim, TO_CHAR );
            }
            else
            {
               act( AT_MAGIC, "You change your own sex.", ch, NULL, NULL, TO_CHAR );
            }
            act( AT_MAGIC, "$N's crotch seems to shift a little.", ch, NULL, victim, TO_NOTVICT );
            check_bodyparts( victim );
         }
         else if( victim->pcdata->eyes && !str_cmp( "eyes", arg1 ) )
         {
            STRFREE( victim->pcdata->eyes );
            victim->pcdata->eyes = STRALLOC( argument );
            act( AT_MAGIC, "Your eyes itch.", victim, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n's eyes change to $T.", victim, NULL, argument, TO_ROOM );
         }
         else if( victim->pcdata->hair && !str_cmp( "hair", arg1 ) )
         {
            STRFREE( victim->pcdata->hair );
            victim->pcdata->hair = STRALLOC( argument );
            act( AT_MAGIC, "Your hair shimmers for a moment.", victim, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n's hair changes to $T.", victim, NULL, argument, TO_ROOM );
         }
         else if( victim->pcdata->skin_type && !str_cmp( victim->pcdata->skin_type, arg1 ) )
         {
            STRFREE( victim->pcdata->skin_color );
            victim->pcdata->skin_color = STRALLOC( argument );
            act( AT_MAGIC, "Your $t shimmers for a moment.", victim, victim->pcdata->skin_type, NULL, TO_CHAR );
            act( AT_MAGIC, "$n's $t changes to $T.", victim, victim->pcdata->skin_type, argument, TO_ROOM );
         }
         else if( victim->pcdata->extra_type && !str_cmp( victim->pcdata->extra_type, arg1 ) )
         {
            STRFREE( victim->pcdata->extra_color );
            victim->pcdata->extra_color = STRALLOC( argument );
            act( AT_MAGIC, "Your $t shimmers for a moment.", victim, victim->pcdata->extra_type, NULL, TO_CHAR );
            act( AT_MAGIC, "$n's $t changes to $T.", victim, victim->pcdata->extra_type, argument, TO_ROOM );
         }
         else
         {
            send_to_char( "They don't have that.\n\r", ch );
            return;
         }
         break;

      case MAG_LIGHT:
         if( !argument || argument[0] == '\0' || argument[0] == ' ' )
         {
            STRFREE( ch->pcdata->name_disguise );
            act( AT_MAGIC, "Your form flickers as you drop the illusion.", ch, NULL, NULL, TO_CHAR );
            act( AT_MAGIC, "$n's form flickers for a moment.", ch, NULL, NULL, TO_ROOM );
            break;
         }
         if( ch->curr_talent[TAL_ILLUSION] < 100 )
         {
            ch_trick( victim, type );
            break;
         }
         STRFREE( ch->pcdata->name_disguise );
         ch->pcdata->name_disguise = STRALLOC( argument );
         act( AT_MAGIC, "Your form shimmers as you raise an illusion.", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n's form shimmers for a moment.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_ANTIMAGIC:
         if( ch->curr_talent[TAL_SEEKING] < 50 )
         {
            ch_trick( victim, type );
            break;
         }
         if( IS_NPC( victim ) || !victim->pcdata->name_disguise )
         {
            ch_trick( victim, type );
            break;
         }
         STRFREE( victim->pcdata->name_disguise );
         act( AT_MAGIC, "Your form flickers as your illusion vanishes.", victim, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n's form flickers for a moment.", victim, NULL, NULL, TO_ROOM );
         break;

      case MAG_HOLY:
         if( ch->curr_talent[TAL_HEALING] < 200 )
         {
            ch_trick( victim, type );
            break;
         }

         if( IS_AFFECTED( ch, AFF_ANGEL ) )
         {
            ch_trick( victim, type );
            break;
         }

         set_char_color( AT_WHITE, ch );

         act( AT_WHITE, "A luminous white aura surrounds $n, radiance in holiness...", ch, NULL, NULL, TO_ROOM );
         send_to_char( "A cleansing light washes through you, purifying you to the core of your being...\n\r", ch );

         STRFREE( ch->pcdata->type );
         ch->pcdata->type = STRALLOC( "angelic" );

         act( AT_WHITE, "In a blinding flash of light the luminence fades, leaving $n surrounded in a holy halo.", ch, NULL,
              NULL, TO_ROOM );
         send_to_char( "You feel changed, looking on the world in a whole new light.\n\r", ch );

         xSET_BIT( ch->pcdata->perm_aff, AFF_ANGEL );
         xSET_BIT( ch->pcdata->perm_aff, AFF_HOLY );
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_DEATH] < 200 )
         {
            ch_trick( victim, type );
            break;
         }

         if( IS_AFFECTED( ch, AFF_DEMON ) )
         {
            ch_trick( victim, type );
            break;
         }

         set_char_color( AT_DGREY, ch );
         act( AT_DGREY, "Twisting demonic shadows surround $n, writhing and groping at $s body.", ch, NULL, NULL, TO_ROOM );
         send_to_char( "You call upon the shadows of the underworld...\n\r", ch );
         send_to_char( "The shadows surround you, penetrating your body, clutching at your very soul.\n\r", ch );

         STRFREE( ch->pcdata->type );
         ch->pcdata->type = STRALLOC( "demonic" );

         act( AT_DGREY, "The shadows seem to sink into $n, rooting themselves deeply.", ch, NULL, NULL, TO_ROOM );
         send_to_char( "The shadows claim your soul, and leave in its place an unholy aura.\n\r", ch );

         xSET_BIT( ch->pcdata->perm_aff, AFF_DEMON );
         xSET_BIT( ch->pcdata->perm_aff, AFF_UNHOLY );
         break;

      case MAG_ALL:
         if( ch->curr_talent[TAL_CATALYSM] < 200 )
         {
            ch_trick( victim, type );
            break;
         }

         if( IS_AFFECTED( ch, AFF_ETHEREAL ) )
         {
            ch_trick( victim, type );
            break;
         }

         set_char_color( AT_PURPLE, ch );

         act( AT_MAGIC, "$n glows in a mystical purplish light, brilliant and all-consuming.", ch, NULL, NULL, TO_ROOM );
         send_to_char( "You focus yourself, gathering all your magic together in a solid core.\n\r", ch );

         STRFREE( ch->pcdata->type );
         ch->pcdata->type = STRALLOC( "ethereal" );

         act( AT_MAGIC, "$n seems to transcend $s mortal body, stepping away from it in a magical ethereal form.", ch, NULL,
              NULL, TO_ROOM );
         send_to_char( "In a step of tremendous power and will, you step out of your mortal body, leaving it behind.\n\r",
                       ch );

         make_corpse( ch );
         xCLEAR_BITS( ch->pcdata->perm_aff );
         xSET_BIT( ch->pcdata->perm_aff, AFF_ETHEREAL );
         xSET_BIT( ch->pcdata->perm_aff, AFF_MAGICAL );
         xSET_BIT( ch->pcdata->perm_aff, AFF_NO_CORPSE );
         break;

   }
}

void draw_rune( CHAR_DATA * ch, ROOM_INDEX_DATA * room, int type )
{
   OBJ_DATA *obj;

   if( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
   {
      ch_trick( ch, type );
      return;
   }

   switch ( type )
   {
      default:
         ch_trick( ch, type );
         break;

      case MAG_HOLY:
         if( ch->curr_talent[TAL_HEALING] < 100 )
         {
            ch_trick( ch, type );
            break;
         }
         act( AT_WHITE, "You trace runes of life on the walls.", ch, NULL, NULL, TO_CHAR );
         act( AT_WHITE, "$n traces glowing white runes on the walls.", ch, NULL, NULL, TO_ROOM );
         SET_BIT( room->runes, RUNE_LIFE );
         break;

      case MAG_DREAM:
         if( ch->curr_talent[TAL_DREAM] < 100 )
         {
            ch_trick( ch, type );
            break;
         }
         act( AT_PINK, "You trace runes of distraction on the walls.", ch, NULL, NULL, TO_CHAR );
         act( AT_PINK, "$n traces glowing pink runes on the walls.", ch, NULL, NULL, TO_ROOM );
         SET_BIT( room->runes, RUNE_DISTRACT );
         break;

      case MAG_ENERGY:
         if( ch->curr_talent[TAL_SECURITY] < 100 )
         {
            ch_trick( ch, type );
            break;
         }
         act( AT_CYAN, "You trace runes of warding on the walls.", ch, NULL, NULL, TO_CHAR );
         act( AT_CYAN, "$n traces glowing cyan runes on the walls.", ch, NULL, NULL, TO_ROOM );
         SET_BIT( room->runes, RUNE_WARD );
         break;

      case MAG_TELEKINETIC:
         if( ch->curr_talent[TAL_MOTION] < 65 )
         {
            ch_trick( ch, type );
            break;
         }
         act( AT_YELLOW, "You trace a yellow rune on the floor.", ch, NULL, NULL, TO_CHAR );
         act( AT_YELLOW, "$n traces a yellow rune on the floor.", ch, NULL, NULL, TO_ROOM );
         obj = create_object( get_obj_index( 87 ), 0 );
         obj = obj_to_room( obj, room );
         obj->obj_by = STRALLOC( ch->name );
         break;

      case MAG_ANTIMAGIC:
         if( ch->curr_talent[TAL_VOID] < 50 )
         {
            ch_trick( ch, type );
            break;
         }
         act( AT_MAGIC, "You wave your hands, dispelling any enchantments on the room.", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n has dispelled the room!", ch, NULL, NULL, TO_ROOM );
         ch->in_room->runes = 0;
         break;
   }
}

void obj_enhance( CHAR_DATA * ch, OBJ_DATA * obj, int type, int power )
{
   bool found;
   AFFECT_DATA *paf;
   int amount;

   if( obj->mana < 50 )
   {
      obj_trick( ch, obj, type );
      return;
   }

   amount = obj->mana / 50;
   amount = UMIN( amount, power / 100 );

   found = FALSE;
   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         return;

      case MAG_HOLY:
         if( obj->item_type != ITEM_ARMOR )
         {
            obj_trick( ch, obj, type );
            return;
         }
         amount = UMIN( amount, TALENT( ch, TAL_SECURITY ) / 10 - obj->value[0] + obj->pIndexData->value[0] );
         if( amount > 0 )
         {
            act( AT_WHITE, "You enhance the protective ability of $p.", ch, obj, NULL, TO_CHAR );
            obj->value[0] += amount;
         }
         else
         {
            obj_trick( ch, obj, type );
            return;
         }
         break;

      case MAG_EARTH:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_CON && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_EARTH ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_GREEN, "You enchant $p with the strength of the Earth.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_EARTH ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_CON;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_GREEN, "You enchant $p with the strength of the Earth.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_COLD:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_LCK && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_FROST ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_FROST, "You enchant $p with the strength of the Ice.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_FROST ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_LCK;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_FROST, "You enchant $p with the strength of the Ice.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_FIRE:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_STR && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_FIRE ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_FIRE, "You enchant $p with the strength of the Flames.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_FIRE ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_STR;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_FIRE, "You enchant $p with the strength of the Flames.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_SEEK:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_CHA && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_SEEKING ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_GREEN, "You enchant $p with the power of Seeking.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_SEEKING ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_CHA;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_GREEN, "You enchant $p with the power of Seeking.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_WIND:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_DEX && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_WIND ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_CYAN, "You enchant $p with the speed of the Wind.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_WIND ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_DEX;
            paf->modifier = 1;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_CYAN, "You enchant $p with the speed of the Wind.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_WATER:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_WIS && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_WATER ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_BLUE, "You enchant $p with the power of Water.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_WATER ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_WIS;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_BLUE, "You enchant $p with the power of Water.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_PSIONIC:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_INT && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_MIND ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_PURPLE, "You enchant $p with the power of the Mind.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_INT;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_PURPLE, "You enchant $p with the power of the Mind.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_ELECTRICITY:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_DAMROLL && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_LIGHTNING ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_YELLOW, "You enchant $p with the strength of the Storm.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_LIGHTNING ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_DAMROLL;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_YELLOW, "You enchant $p with the strength of the Storm.", ch, obj, NULL, TO_CHAR );
         }
         break;

      case MAG_TELEKINETIC:
         for( paf = obj->first_affect; paf; paf = paf->next )
         {
            if( paf->location == APPLY_HITROLL && !found )
            {
               amount = UMIN( amount, TALENT( ch, TAL_MOTION ) / 10 - paf->modifier );
               if( amount > 0 )
               {
                  act( AT_YELLOW, "You enchant $p with the speed of Motion.", ch, obj, NULL, TO_CHAR );
                  paf->modifier += amount;
               }
               else
               {
                  obj_trick( ch, obj, type );
                  return;
                  return;
               }
               found = TRUE;
            }
         }

         if( found == FALSE )
         {
            amount = UMIN( amount, TALENT( ch, TAL_MOTION ) / 10 );
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type = -1;
            paf->duration = -1;
            paf->location = APPLY_HITROLL;
            paf->modifier = amount;
            xCLEAR_BITS( paf->bitvector );
            paf->next = NULL;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
            act( AT_YELLOW, "You enchant $p with the speed of Motion.", ch, obj, NULL, TO_CHAR );
         }
         break;
   }
   WAIT_STATE( ch, PULSE_VIOLENCE * amount );
   obj->mana -= 50 * amount;
   obj->cost += 50000 * amount;
}  /* obj_enhance */

void ch_enhance( CHAR_DATA * victim, CHAR_DATA * ch, int type, int power )
{
   AFFECT_DATA af;
   SKILLTYPE *skill;
   int sn;

   switch ( type )
   {
      default:
         ch_trick( victim, type );
         return;

      case MAG_FIRE:
         sn = gsn_fire;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_STR;
         af.modifier = TALENT( ch, TAL_FIRE ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_WATER:
         sn = gsn_water;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_WIS;
         af.modifier = TALENT( ch, TAL_WATER ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_COLD:
         sn = gsn_frost;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_LCK;
         af.modifier = TALENT( ch, TAL_FROST ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_EARTH:
         sn = gsn_earth;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_CON;
         af.modifier = TALENT( ch, TAL_EARTH ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_WIND:
         sn = gsn_wind;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_DEX;
         af.modifier = TALENT( ch, TAL_WIND ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_PSIONIC:
         sn = gsn_mind;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_INT;
         af.modifier = TALENT( ch, TAL_MIND ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_SEEK:
         sn = gsn_seeking;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_CHA;
         af.modifier = TALENT( ch, TAL_SEEKING ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_TELEKINETIC:
         sn = gsn_motion;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_HITROLL;
         af.modifier = TALENT( ch, TAL_MOTION ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_ELECTRICITY:
         sn = gsn_lightning;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_DAMROLL;
         af.modifier = TALENT( ch, TAL_LIGHTNING ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_HOLY:
         sn = gsn_security;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_AC;
         af.modifier = TALENT( ch, TAL_SECURITY ) / 10;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_DREAM:
         if( ch->curr_talent[TAL_DREAM] < 100 )
         {
            ch_trick( victim, type );
            return;
         }

         sn = gsn_dream;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_IMMUNE;
         af.modifier = RIS_SLEEP;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_POISON:
         if( ch->curr_talent[TAL_HEALING] < 100 )
         {
            ch_trick( victim, type );
            return;
         }

         sn = gsn_security;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_IMMUNE;
         af.modifier = RIS_POISON;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         break;

      case MAG_LIGHT:
         if( ch->curr_talent[TAL_ILLUSION] < 20 )
         {
            ch_trick( victim, type );
            return;
         }

         sn = gsn_illusion;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_AFFECT;
         af.modifier = 0;
         af.bitvector = meb( AFF_GLOW );
         affect_join( victim, &af );
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_DEATH] < 20 )
         {
            ch_trick( victim, type );
            return;
         }

         sn = gsn_death;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            return;
         }
         af.type = sn;
         af.duration = power * 2;
         af.location = APPLY_AFFECT;
         af.modifier = 0;
         af.bitvector = meb( AFF_DARK );
         affect_join( victim, &af );
         break;

   }
   skill = get_skilltype( sn );
   if( skill->hit_vict && skill->hit_vict[0] != '\0' )
      act( AT_MAGIC, skill->hit_vict, victim, NULL, ch, TO_CHAR );
   if( victim != ch && skill->hit_char && skill->hit_char[0] != '\0' )
      act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
   if( skill->hit_room && skill->hit_room[0] != '\0' )
      act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
}

void ch_sing( CHAR_DATA * victim, CHAR_DATA * ch, int type, int power )
{
   switch ( type )
   {
      default:
         ch_trick( ch, type );
         break;
   }
}

void song_sing( CHAR_DATA * ch, int type, int power, char *argument )
{
   switch ( type )
   {
      default:
         ch_trick( ch, type );
         break;
   }
}

void obj_control( CHAR_DATA * ch, OBJ_DATA * obj, int type, int power )
{
   OBJ_DATA *obj1;
   NATION_DATA *nation;
   char buf[MAX_STRING_LENGTH];

   switch ( type )
   {
      default:
         obj_trick( ch, obj, type );
         break;

      case MAG_UNHOLY:
         if( ch->curr_talent[TAL_DEATH] < 100 )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->item_type != ITEM_CORPSE_NPC )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( obj->obj_by == NULL || ( nation = find_nation( obj->obj_by ) ) == NULL )
         {
            obj_trick( ch, obj, type );
            break;
         }

         if( !IS_AFFECTED( ch, AFF_ETHEREAL ) )
         {
            obj_trick( ch, obj, type );
            break;
         }

         while( ( obj1 = obj->first_content ) )
         {
            obj_from_obj( obj1 );
            if( obj1->item_type == ITEM_MONEY )
            {
               ch->gold += obj1->value[0];
               extract_obj( obj1 );
            }
            else
            {
               obj_to_char( obj1, ch );
            }
         }
         STRFREE( ch->last_taken );
         sprintf( buf, "possessing the %s", myobj( obj ) );
         ch->last_taken = STRALLOC( buf );
         WAIT_STATE( ch, PULSE_VIOLENCE * 12 );
         separate_obj( obj );
         extract_obj( obj );
         ch->nation = nation;
         STRFREE( ch->species );
         ch->species = STRALLOC( nation->name );
         STRFREE( ch->pcdata->type );
         ch->pcdata->type = STRALLOC( "living" );
         fix_char( ch );
         check_bodyparts( ch );
         xCLEAR_BITS( ch->pcdata->perm_aff );
         ch->position = POS_RESTING;
         act( AT_DGREY, "You take a deep breath and open your eyes.", ch, NULL, NULL, TO_CHAR );
         act( AT_DGREY, "$n takes a deep breath and opens $s eyes.", ch, NULL, NULL, TO_ROOM );
         if( get_timer( ch, TIMER_PKILLED ) > 0 )
            remove_timer( ch, TIMER_PKILLED );
         ch->mental_state = -100;
         send_to_char( "You feel drained and exhausted by the possession.\n\r", ch );
         break;
   }
}

void ch_control( CHAR_DATA * victim, CHAR_DATA * ch, int type, int power, char *argument )
{
   AFFECT_DATA af;
   char buf[MAX_STRING_LENGTH];
   int sn;

   switch ( type )
   {
      default:
         ch_trick( victim, type );
         break;

      case MAG_PSIONIC:
         if( !IS_NPC( ch ) &&
             ch->curr_talent[TAL_MIND] + ch->curr_talent[TAL_SPEECH] < ( IS_AFFECTED( ch, AFF_BEAUTY ) ? 100 : 200 ) )
         {
            ch_trick( victim, type );
            break;
         }

         if( victim == ch )
         {
            send_to_char( "You like yourself even better!\n\r", ch );
            return;
         }

         if( IS_SET( victim->immune, RIS_MAGIC )
             || IS_SET( victim->immune, RIS_CHARM ) || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PACIFIST ) ) )
         {
            ch_trick( victim, type );
            break;
         }

         if( !IS_NPC( victim ) )
         {
            send_to_char( "I don't think so...\n\r", ch );
            send_to_char( "Someone is trying to charm you...\n\r", victim );
            return;
         }

         if( IS_AFFECTED( victim, AFF_CHARM )
             || IS_AFFECTED( ch, AFF_CHARM )
             || number_range( 1, get_curr_wil( ch ) ) < get_curr_wil( victim ) || circle_follow( victim, ch ) )
         {
            act( AT_GREEN, "$N resists your attempts to tame $M.", ch, NULL, victim, TO_CHAR );
            act( AT_GREEN, "$n attempts to tame you, but you resist.", ch, NULL, victim, TO_VICT );
            return;
         }

         if( victim->master )
            stop_follower( victim );
         add_follower( victim, ch );
         af.type = -1;
         af.duration = number_fuzzy( get_curr_wil( ch ) );
         af.location = 0;
         af.modifier = 0;
         af.bitvector = meb( AFF_CHARM );
         affect_to_char( victim, &af );
         act( AT_GREEN, "You charm $N.", ch, NULL, victim, TO_CHAR );
         act( AT_GREEN, "You get the sudden urge to follow and obey $n.", ch, NULL, victim, TO_VICT );
         sprintf( buf, "%s has charmed %s.", ch->name, victim->name );
         to_channel( buf, "Spell", PERMIT_SECURITY );
         break;

      case MAG_FIRE:
         if( ch->curr_talent[TAL_FIRE] < 50 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_fire;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_SUSCEPTIBLE;
         af.modifier = RIS_FIRE;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         send_to_char( "You are vulnerable to heat.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You make $N vulnerable to heat.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N is vulnerable to heat.", ch, NULL, victim, TO_NOTVICT );
         break;

      case MAG_COLD:
         if( ch->curr_talent[TAL_FROST] < 50 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_frost;
         if( is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         af.type = sn;
         af.duration = power;
         af.location = APPLY_SUSCEPTIBLE;
         af.modifier = RIS_COLD;
         xCLEAR_BITS( af.bitvector );
         affect_join( victim, &af );
         update_pos( victim );
         send_to_char( "You are vulnerable to cold.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You make $N vulnerable to cold.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "$N is vulnerable to cold.", ch, NULL, victim, TO_NOTVICT );
         break;

      case MAG_WATER:
         if( ch->curr_talent[TAL_WATER] < 50 )
         {
            ch_trick( victim, type );
            return;
         }
         sn = gsn_sticky_flame;
         if( !is_affected( victim, sn ) )
         {
            ch_trick( victim, type );
            break;
         }
         affect_strip( victim, gsn_sticky_flame );
         send_to_char( "The flames surrounding you are quenched.\n\r", victim );
         if( ch != victim )
         {
            act( AT_MAGIC, "You quench the flames surrounding $N.", ch, NULL, victim, TO_CHAR );
         }
         act( AT_MAGIC, "The flames surroudning $N are quenched.", ch, NULL, victim, TO_NOTVICT );
         break;
   }
}

void room_control( CHAR_DATA * ch, int type, int power, char *argument )
{
   switch ( type )
   {
      default:
         ch_trick( ch, type );
         break;

      case MAG_EARTH:
         if( ch->curr_talent[TAL_EARTH] < 20 )
         {
            ch_trick( ch, type );
            return;
         }
         if( ch->in_room->curr_vegetation >= 100 )
         {
            ch_trick( ch, type );
            return;
         }
         ch->in_room->curr_vegetation += 10;
         act( AT_GREEN, "At your command, vegetation flourishes in the room.", ch, NULL, NULL, TO_CHAR );
         act( AT_GREEN, "Vegetation flourishes in the room.", ch, NULL, NULL, TO_ROOM );
         break;

      case MAG_WATER:
         if( ch->curr_talent[TAL_WATER] < 20 )
         {
            ch_trick( ch, type );
            return;
         }
         if( ch->in_room->curr_water >= 100 )
         {
            ch_trick( ch, type );
            return;
         }
         ch->in_room->curr_water += 10;
         act( AT_BLUE, "Water floods the room!", ch, NULL, NULL, TO_CHAR );
         act( AT_BLUE, "Water floods into the room!", ch, NULL, NULL, TO_ROOM );
         break;
   }
}

/* Check the verb for the spell */
int check_verb( char *argument )
{
   if( !str_cmp( argument, "tricks" ) )
      return ACT_TRICKS;
   else if( !str_cmp( argument, "harm" )
            || !str_cmp( argument, "hurt" )
            || !str_cmp( argument, "damage" )
            || !str_cmp( argument, "destroy" )
            || !str_cmp( argument, "slay" )
            || !str_cmp( argument, "attack" ) || !str_cmp( argument, "kill" ) || !str_cmp( argument, "burn" ) )
      return ACT_HARM;
   else if( !str_cmp( argument, "help" )
            || !str_cmp( argument, "assist" )
            || !str_cmp( argument, "cure" )
            || !str_cmp( argument, "heal" )
            || !str_cmp( argument, "aid" ) || !str_cmp( argument, "restore" ) || !str_cmp( argument, "revive" ) )
      return ACT_HELP;
   else if( !str_cmp( argument, "hinder" )
            || !str_cmp( argument, "slow" )
            || !str_cmp( argument, "stop" )
            || !str_cmp( argument, "halt" )
            || !str_cmp( argument, "anchor" )
            || !str_cmp( argument, "curse" ) || !str_cmp( argument, "reduce" ) || !str_cmp( argument, "immobilize" ) )
      return ACT_HINDER;
   else if( !str_cmp( argument, "create" )
            || !str_cmp( argument, "conjure" )
            || !str_cmp( argument, "shape" )
            || !str_cmp( argument, "form" )
            || !str_cmp( argument, "make" )
            || !str_cmp( argument, "summon" ) || !str_cmp( argument, "bring" ) || !str_cmp( argument, "fashion" ) )
      return ACT_CREATE;
   else if( !str_cmp( argument, "seek" )
            || !str_cmp( argument, "find" )
            || !str_cmp( argument, "identify" )
            || !str_cmp( argument, "examine" )
            || !str_cmp( argument, "locate" ) || !str_cmp( argument, "watch" ) || !str_cmp( argument, "discover" ) )
      return ACT_FIND;
   else if( !str_cmp( argument, "move" )
            || !str_cmp( argument, "send" )
            || !str_cmp( argument, "transfer" )
            || !str_cmp( argument, "shift" )
            || !str_cmp( argument, "travel" ) || !str_cmp( argument, "push" ) || !str_cmp( argument, "transport" ) )
      return ACT_MOVE;
   else if( !str_cmp( argument, "change" )
            || !str_cmp( argument, "alter" )
            || !str_cmp( argument, "transform" )
            || !str_cmp( argument, "morph" ) || !str_cmp( argument, "transmute" ) || !str_cmp( argument, "mutate" ) )
      return ACT_CHANGE;
   else if( !str_cmp( argument, "enhance" )
            || !str_cmp( argument, "enchant" )
            || !str_cmp( argument, "bless" )
            || !str_cmp( argument, "improve" ) || !str_cmp( argument, "strengthen" ) || !str_cmp( argument, "imbue" ) )
      return ACT_ENHANCE;
   else if( !str_cmp( argument, "focus" )
            || !str_cmp( argument, "brand" )
            || !str_cmp( argument, "shield" )
            || !str_cmp( argument, "protect" ) || !str_cmp( argument, "bind" ) || !str_cmp( argument, "circle" ) )
      return ACT_FOCUS;
   else if( !str_cmp( argument, "rune" )
            || !str_cmp( argument, "area" )
            || !str_cmp( argument, "room" )
            || !str_cmp( argument, "sigil" ) || !str_cmp( argument, "symbol" ) || !str_cmp( argument, "mark" ) )
      return ACT_RUNE;
   else if( !str_cmp( argument, "sing" )
            || !str_cmp( argument, "song" )
            || !str_cmp( argument, "music" ) || !str_cmp( argument, "speak" ) || !str_cmp( argument, "play" ) )
      return ACT_SING;
   else if( !str_cmp( argument, "control" )
            || !str_cmp( argument, "charm" )
            || !str_cmp( argument, "order" ) || !str_cmp( argument, "force" ) || !str_cmp( argument, "compel" ) )
      return ACT_CONTROL;
   else
      return ACT_TRICKS;
}

/* Check the power on the spell */
int check_power( char *argument )
{
   if( !str_cmp( argument, "miniscule" ) )
      return 10;
   else if( !str_cmp( argument, "tiny" ) )
      return 20;
   else if( !str_cmp( argument, "weak" ) )
      return 30;
   else if( !str_cmp( argument, "low" ) )
      return 50;
   else if( !str_cmp( argument, "small" ) )
      return 80;
   else if( !str_cmp( argument, "medium" ) )
      return 100;
   else if( !str_cmp( argument, "moderate" ) )
      return 200;
   else if( !str_cmp( argument, "intermediate" ) )
      return 300;
   else if( !str_cmp( argument, "high" ) )
      return 400;
   else if( !str_cmp( argument, "strong" ) )
      return 500;
   else if( !str_cmp( argument, "great" ) )
      return 600;
   else if( !str_cmp( argument, "large" ) )
      return 700;
   else if( !str_cmp( argument, "mighty" ) )
      return 800;
   else if( !str_cmp( argument, "huge" ) )
      return 900;
   else if( !str_cmp( argument, "powerful" ) )
      return 1000;
   else if( !str_cmp( argument, "deadly" ) )
      return 1100;
   else if( !str_cmp( argument, "massive" ) )
      return 1200;
   else if( !str_cmp( argument, "enormous" ) )
      return 1300;
   else if( !str_cmp( argument, "gargantuan" ) )
      return 1400;
   else if( !str_cmp( argument, "collosal" ) )
      return 1500;
   else if( !str_cmp( argument, "extreme" ) )
      return 1600;
   else if( !str_cmp( argument, "amazing" ) )
      return 1700;
   else if( !str_cmp( argument, "incredible" ) )
      return 1800;
   else if( !str_cmp( argument, "severe" ) )
      return 1900;
   else if( !str_cmp( argument, "intense" ) )
      return 2000;
   else if( !str_cmp( argument, "supreme" ) )
      return 2500;
   else if( !str_cmp( argument, "ultimate" ) )
      return 3000;
   else if( !str_cmp( argument, "godlike" ) )
      return 3500;
   else if( !str_cmp( argument, "omnipotent" ) )
      return 3800;
   else if( !str_cmp( argument, "godly" ) )
      return 4000;
   else
      return -1;
}

/* Return true if they are mentally capable of concentrating */
bool check_concentration( CHAR_DATA * ch, int type, int action, int power )
{
   AFFECT_DATA af;
   int sn;

   if( number_percent(  ) - 5 > ch->pcdata->condition[COND_DRUNK] + abs( ch->mental_state ) )
      return TRUE;

   switch ( number_range( 1, 3 ) )
   {
      default:
         if( type == MAG_FIRE )
         {
            sn = gsn_sticky_flame;
            if( is_affected( ch, sn ) )
            {
               ch_trick( ch, type );
               break;
            }
            af.type = sn;
            af.duration = power / 10;
            af.location = APPLY_DAMROLL;
            af.modifier = TALENT( ch, TAL_FIRE ) / 40;
            xCLEAR_BITS( af.bitvector );
            xSET_BIT( af.bitvector, AFF_FLAMING );
            affect_join( ch, &af );
            act( AT_FIRE, "You set yourself on fire!", ch, NULL, NULL, TO_CHAR );
            act( AT_FIRE, "$n sets $mself on fire!", ch, NULL, NULL, TO_ROOM );
         }
         else if( type == MAG_LIGHT )
         {
            sn = gsn_blindness;
            if( is_affected( ch, sn ) )
            {
               ch_trick( ch, type );
               break;
            }
            if( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
            {
               ch_trick( ch, type );
               break;
            }
            af.type = sn;
            af.duration = power / 10;
            af.location = APPLY_HITROLL;
            af.modifier = -1 * TALENT( ch, TAL_ILLUSION ) / 20;
            xCLEAR_BITS( af.bitvector );
            xSET_BIT( af.bitvector, AFF_FLAMING );
            affect_join( ch, &af );
            act( AT_YELLOW, "You are blinded!", ch, NULL, NULL, TO_CHAR );
            act( AT_YELLOW, "$n blinds $mself!", ch, NULL, NULL, TO_ROOM );
         }
         else if( action == ACT_CREATE )
         {
            act( AT_WHITE, "You are covered in confetti.", ch, NULL, NULL, TO_CHAR );
            act( AT_WHITE, "$n is covered in confetti.", ch, NULL, NULL, TO_ROOM );
            break;
         }
         else
         {
            ch_trick( ch, type );
            break;
         }
         break;
      case 1:
         huh( ch );
         break;
      case 2:
         ch_trick( ch, type );
         break;
   }
   return FALSE;
}

/* The big magic command and handler. Parses their input,
 * tries to figure out what they're trying to do, and
 * directs the results to the appropriate function above.
 */
void do_magic( CHAR_DATA * ch, char *argument )
{
   int talent;
   int damage;
   int power;
   int action;
   CHAR_DATA *victim;
   CHAR_DATA *next_vict;
   OBJ_DATA *obj;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) )
      return;

   if( check_manastorm( ch ) )
      return;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument_retain_case( argument, arg3 );
   argument = one_argument_retain_case( argument, arg4 );

   talent = TAL_CATALYSM;
   damage = MAG_ALL;
   power = 1;
   action = ACT_TRICKS;

   /*
    * First argument, a Talent 
    */
   if( !str_cmp( arg1, "fire" ) || !str_cmp( arg1, "flame" ) || !str_cmp( arg1, "flames" ) )
   {
      act( AT_FIRE, "$n begins to channel Fire Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_FIRE, "You begin to channel Fire Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_FIRE;
      damage = MAG_FIRE;
   }
   else if( !str_cmp( arg1, "burn" ) || !str_cmp( arg1, "incinerate" ) || !str_cmp( arg1, "scorch" ) )
   {
      act( AT_GREEN, "$n begins to channel Fire Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_GREEN, "You begin to channel Fire Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_FIRE;
      damage = MAG_FIRE;
      action = ACT_HARM;
   }
   else if( !str_cmp( arg1, "earth" ) || !str_cmp( arg1, "nature" ) || !str_cmp( arg1, "leaves" ) )
   {
      act( AT_GREEN, "$n begins to channel Earth Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_GREEN, "You begin to channel Earth Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_EARTH;
      damage = MAG_EARTH;
   }
   else if( !str_cmp( arg1, "stone" ) || !str_cmp( arg1, "dirt" ) || !str_cmp( arg1, "soil" ) || !str_cmp( arg1, "rock" ) )
   {
      act( AT_ORANGE, "$n begins to channel Earth Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_ORANGE, "You begin to channel Earth Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_EARTH;
      damage = MAG_BLUNT;
   }
   else if( !str_cmp( arg1, "crush" ) )
   {
      act( AT_GREEN, "$n begins to channel Earth Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_GREEN, "You begin to channel Earth Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_EARTH;
      damage = MAG_BLUNT;
      action = ACT_HARM;
   }
   else if( !str_cmp( arg1, "tangle" ) || !str_cmp( arg1, "entangle" ) )
   {
      act( AT_GREEN, "$n begins to channel Earth Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_GREEN, "You begin to channel Earth Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_EARTH;
      damage = MAG_EARTH;
      action = ACT_HINDER;
   }
   else if( !str_cmp( arg1, "lightning" ) || !str_cmp( arg1, "electric" ) || !str_cmp( arg1, "electricity" ) )
   {
      act( AT_YELLOW, "$n begins to channel Lightning Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_YELLOW, "You begin to channel Lightning Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_LIGHTNING;
      damage = MAG_ELECTRICITY;
   }
   else if( !str_cmp( arg1, "shock" ) || !str_cmp( arg1, "electrocute" ) || !str_cmp( arg1, "zap" ) )
   {
      act( AT_YELLOW, "$n begins to channel Lightning Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_YELLOW, "You begin to channel Lightning Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_LIGHTNING;
      damage = MAG_ELECTRICITY;
      action = ACT_HARM;
   }
   else if( !str_cmp( arg1, "wind" ) || !str_cmp( arg1, "air" ) || !str_cmp( arg1, "storm" ) || !str_cmp( arg1, "storms" ) )
   {
      act( AT_CYAN, "$n begins to channel Wind Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_CYAN, "You begin to channel Wind Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_WIND;
      damage = MAG_WIND;
   }
   else if( !str_cmp( arg1, "frost" ) || !str_cmp( arg1, "ice" ) || !str_cmp( arg1, "cold" ) || !str_cmp( arg1, "snow" ) )
   {
      act( AT_FROST, "$n begins to channel Frost Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_FROST, "You begin to channel Frost Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_FROST;
      damage = MAG_COLD;
   }
   else if( !str_cmp( arg1, "chill" ) || !str_cmp( arg1, "freeze" ) )
   {
      act( AT_FROST, "$n begins to channel Frost Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_FROST, "You begin to channel Frost Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_FROST;
      damage = MAG_COLD;
      action = ACT_HARM;
   }
   else if( !str_cmp( arg1, "water" )
            || !str_cmp( arg1, "bubbles" ) || !str_cmp( arg1, "sea" ) || !str_cmp( arg1, "ocean" ) )
   {
      act( AT_BLUE, "$n begins to channel Water Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_BLUE, "You begin to channel Water Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_WATER;
      damage = MAG_WATER;
   }
   else if( !str_cmp( arg1, "acid" ) )
   {
      act( AT_DGREEN, "$n begins to channel Acid Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREEN, "You begin to channel Acid Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_WATER;
      damage = MAG_ACID;
   }
   else if( !str_cmp( arg1, "dream" ) || !str_cmp( arg1, "sleep" ) || !str_cmp( arg1, "nightmare" ) )
   {
      act( AT_PINK, "$n begins to channel Dream Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_PINK, "You begin to channel Dream Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_DREAM;
      damage = MAG_DREAM;
   }
   else if( !str_cmp( arg1, "illusion" ) || !str_cmp( arg1, "light" ) )
   {
      act( AT_YELLOW, "$n begins to channel Light Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_YELLOW, "You begin to channel Light Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_ILLUSION;
      damage = MAG_LIGHT;
   }
   else if( !str_cmp( arg1, "speech" )
            || !str_cmp( arg1, "sound" ) || !str_cmp( arg1, "song" ) || !str_cmp( arg1, "sonics" ) )
   {
      act( AT_PINK, "$n begins to channel Speech Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_PINK, "You begin to channel Speech Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_SPEECH;
      damage = MAG_SOUND;
   }
   else if( !str_cmp( arg1, "healing" ) || !str_cmp( arg1, "life" ) || !str_cmp( arg1, "holy" ) )
   {
      act( AT_WHITE, "$n begins to channel Holy Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_WHITE, "You begin to channel Holy Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_HEALING;
      damage = MAG_HOLY;
   }
   else if( !str_cmp( arg1, "heal" ) || !str_cmp( arg1, "cure" ) )
   {
      act( AT_WHITE, "$n begins to channel Healing Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_WHITE, "You begin to channel Healing Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_HEALING;
      damage = MAG_HOLY;
      action = ACT_HELP;
   }
   else if( !str_cmp( arg1, "unholy" ) || !str_cmp( arg1, "shadow" ) )
   {
      act( AT_DGREY, "$n begins to channel Shadow Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREY, "You begin to channel Shadow Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_DEATH;
      damage = MAG_UNHOLY;
   }
   else if( !str_cmp( arg1, "drain" ) )
   {
      act( AT_DGREY, "$n begins to channel Death Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREY, "You begin to channel Death Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_DEATH;
      damage = MAG_DRAIN;
      action = ACT_HARM;
   }
   else if( !str_cmp( arg1, "death" ) )
   {
      act( AT_DGREY, "$n begins to channel Death Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREY, "You begin to channel Death Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_DEATH;
      damage = MAG_DRAIN;
   }
   else if( !str_cmp( arg1, "venom" ) || !str_cmp( arg1, "poison" ) )
   {
      act( AT_DGREEN, "$n begins to channel Poison Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREEN, "You begin to channel Poison Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_DEATH;
      damage = MAG_POISON;
   }
   else if( !str_cmp( arg1, "changing" ) || !str_cmp( arg1, "chaos" ) )
   {
      act( AT_ORANGE, "$n begins to channel Change Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_ORANGE, "You begin to channel Change Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CHANGE;
      damage = MAG_CHANGE;
   }
   else if( !str_cmp( arg1, "distort" ) )
   {
      act( AT_ORANGE, "$n begins to channel Change Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_ORANGE, "You begin to channel Change Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CHANGE;
      damage = MAG_CHANGE;
      action = ACT_HARM;
   }
   else if( !str_cmp( arg1, "change" ) || !str_cmp( arg1, "alter" ) || !str_cmp( arg1, "morph" ) )
   {
      act( AT_ORANGE, "$n begins to channel Change Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_ORANGE, "You begin to channel Change Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CHANGE;
      damage = MAG_CHANGE;
      action = ACT_CHANGE;
   }
   else if( !str_cmp( arg1, "time" ) || !str_cmp( arg1, "chronomancy" ) || !str_cmp( arg1, "prophecy" ) )
   {
      act( AT_DBLUE, "$n begins to channel Time Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_DBLUE, "You begin to channel Time Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_TIME;
      damage = MAG_TIME;
   }
   else if( !str_cmp( arg1, "mind" ) || !str_cmp( arg1, "psi" ) || !str_cmp( arg1, "psionics" ) )
   {
      act( AT_PURPLE, "$n begins to channel Mind Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_PURPLE, "You begin to channel Mind Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_MIND;
      damage = MAG_PSIONIC;
   }
   else if( !str_cmp( arg1, "motion" ) || !str_cmp( arg1, "telekinesis" ) || !str_cmp( arg1, "movement" ) )
   {
      act( AT_YELLOW, "$n begins to channel Motion Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_YELLOW, "You begin to channel Motion Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_MOTION;
      damage = MAG_TELEKINETIC;
   }
   else if( !str_cmp( arg1, "travel" ) || !str_cmp( arg1, "teleport" ) || !str_cmp( arg1, "send" ) )
   {
      act( AT_YELLOW, "$n begins to channel Motion Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_YELLOW, "You begin to channel Motion Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_MOTION;
      damage = MAG_TELEKINETIC;
      action = ACT_MOVE;
   }
   else if( !str_cmp( arg1, "recall" ) || !str_cmp( arg1, "return" ) || !str_cmp( arg1, "nexus" ) )
   {
      act( AT_YELLOW, "$n begins to channel Motion Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_YELLOW, "You begin to channel Motion Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_MOTION;
      damage = MAG_PSIONIC;
      action = ACT_MOVE;
   }
   else if( !str_cmp( arg1, "seeking" ) || !str_cmp( arg1, "finding" ) || !str_cmp( arg1, "divination" ) )
   {
      act( AT_GREEN, "$n begins to channel Seeking Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_GREEN, "You begin to channel Seeking Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_SEEKING;
      damage = MAG_SEEK;
   }
   else if( !str_cmp( arg1, "seek" )
            || !str_cmp( arg1, "find" ) || !str_cmp( arg1, "divine" ) || !str_cmp( arg1, "locate" ) )
   {
      act( AT_GREEN, "$n begins to channel Seeking Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_GREEN, "You begin to channel Seeking Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_SEEKING;
      damage = MAG_SEEK;
      action = ACT_FIND;
   }
   else if( !str_cmp( arg1, "security" ) || !str_cmp( arg1, "binding" ) || !str_cmp( arg1, "protection" ) )
   {
      act( AT_WHITE, "$n begins to channel Security Magic.", ch, NULL, NULL, TO_ROOM );
      act( AT_WHITE, "You begin to channel Security Magic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_SECURITY;
      damage = MAG_HOLY;
   }
   else if( !str_cmp( arg1, "void" ) || !str_cmp( arg1, "anti" ) || !str_cmp( arg1, "antimatter" ) )
   {
      act( AT_DGREY, "$n begins to channel antimatter.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREY, "You begin to channel antimatter.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_VOID;
      damage = MAG_ANTIMATTER;
   }
   else if( !str_cmp( arg1, "ether" ) )
   {
      act( AT_DGREY, "$n begins to draw upon ether.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREY, "You begin to draw upon ether.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_VOID;
      damage = MAG_ETHER;
   }
   else if( !str_cmp( arg1, "antimagic" ) || !str_cmp( arg1, "dispel" ) )
   {
      act( AT_DGREY, "$n begins to channel antimagic.", ch, NULL, NULL, TO_ROOM );
      act( AT_DGREY, "You begin to channel antimagic.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_VOID;
      damage = MAG_ANTIMAGIC;
   }
   else if( !str_cmp( arg1, "catalysm" ) || !str_cmp( arg1, "mana" ) || !str_cmp( arg1, "magic" ) )
   {
      act( AT_PURPLE, "$n begins to channel raw mana.", ch, NULL, NULL, TO_ROOM );
      act( AT_PURPLE, "You begin to channel raw mana.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CATALYSM;
      damage = MAG_ALL;
   }
   else if( !str_cmp( arg1, "energy" ) || !str_cmp( arg1, "power" ) || !str_cmp( arg1, "force" ) )
   {
      act( AT_PURPLE, "$n begins to channel raw energy.", ch, NULL, NULL, TO_ROOM );
      act( AT_PURPLE, "You begin to channel raw energy.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CATALYSM;
      damage = MAG_ENERGY;
   }
   else if( !str_cmp( arg1, "swords" )
            || !str_cmp( arg1, "blades" ) || !str_cmp( arg1, "blade" ) || !str_cmp( arg1, "sword" ) )
   {
      act( AT_PLAIN, "$n begins to dance with magic swords.", ch, NULL, NULL, TO_ROOM );
      act( AT_PLAIN, "You begin to dance with magic swords.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CATALYSM;
      damage = MAG_SLASH;
   }
   else if( !str_cmp( arg1, "daggers" )
            || !str_cmp( arg1, "dagger" ) || !str_cmp( arg1, "knife" ) || !str_cmp( arg1, "knives" ) )
   {
      act( AT_PLAIN, "$n begins to juggle magic daggers.", ch, NULL, NULL, TO_ROOM );
      act( AT_PLAIN, "You begin to juggle magic daggers.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CATALYSM;
      damage = MAG_PIERCE;
   }
   else
   {
      act( AT_PURPLE, "$n begins to channel raw mana.", ch, NULL, NULL, TO_ROOM );
      act( AT_PURPLE, "You begin to channel raw mana.", ch, NULL, NULL, TO_CHAR );
      talent = TAL_CATALYSM;
      damage = MAG_ALL;
      if( argument && argument[0] != '\0' && argument[0] != ' ' )
      {
         sprintf( buf, "%s %s", arg4, argument );
         strcpy( argument, buf );
      }
      else
      {
         strcpy( argument, arg4 );
      }
      strcpy( arg4, arg3 );
      strcpy( arg3, arg2 );
      strcpy( arg2, arg1 );
   }

   /*
    * Examine the second argument. 
    * * Power and verb are interchangable, so we have to check both.
    */
   if( action == ACT_TRICKS )
   {
      if( arg2 && ( action = check_verb( arg2 ) ) == ACT_TRICKS )
      {
         if( power < 10 )
         {
            if( arg2 && ( power = check_power( arg2 ) ) == -1 )
            {
               if( argument && argument[0] != '\0' && argument[0] != ' ' )
               {
                  sprintf( buf, "%s %s", arg4, argument );
                  strcpy( argument, buf );
               }
               else
               {
                  strcpy( argument, arg4 );
               }
               strcpy( arg4, arg3 );
               strcpy( arg3, arg2 );
            }
         }
         else
         {
            if( argument && argument[0] != '\0' && argument[0] != ' ' )
            {
               sprintf( buf, "%s %s", arg4, argument );
               strcpy( argument, buf );
            }
            else
            {
               strcpy( argument, arg4 );
            }
            strcpy( argument, buf );
            strcpy( arg4, arg3 );
            strcpy( arg3, arg2 );
         }
      }
   }
   else
   {
      if( power < 10 )
      {
         if( arg2 && ( power = check_power( arg2 ) ) == -1 )
         {
            if( argument && argument[0] != '\0' && argument[0] != ' ' )
            {
               sprintf( buf, "%s %s", arg4, argument );
               strcpy( argument, buf );
            }
            else
            {
               strcpy( argument, arg4 );
            }
            strcpy( arg4, arg3 );
            strcpy( arg3, arg2 );
         }
      }
      else
      {
         if( argument && argument[0] != '\0' && argument[0] != ' ' )
         {
            sprintf( buf, "%s %s", arg4, argument );
            strcpy( argument, buf );
         }
         else
         {
            strcpy( argument, arg4 );
         }
         strcpy( arg4, arg3 );
         strcpy( arg3, arg2 );
      }
   }

   /*
    * Examine the third argument. Again, power and verb. 
    */
   if( power < 10 )
   {
      if( arg3 && ( power = check_power( arg3 ) ) == -1 )
      {
         if( action == ACT_TRICKS )
         {
            if( arg3 && ( action = check_verb( arg3 ) ) == ACT_TRICKS )
            {
               if( argument && argument[0] != '\0' && argument[0] != ' ' )
               {
                  sprintf( buf, "%s %s", arg4, argument );
                  strcpy( argument, buf );
               }
               else
               {
                  strcpy( argument, arg4 );
               }
               strcpy( arg4, arg3 );
            }
         }
         else
         {
            if( argument && argument[0] != '\0' && argument[0] != ' ' )
            {
               sprintf( buf, "%s %s", arg4, argument );
               strcpy( argument, buf );
            }
            else
            {
               strcpy( argument, arg4 );
            }
            strcpy( arg4, arg3 );
         }
      }
   }
   else
   {
      if( action == ACT_TRICKS )
      {
         if( arg3 && ( action = check_verb( arg3 ) ) == ACT_TRICKS )
         {
            if( argument && argument[0] != '\0' && argument[0] != ' ' )
            {
               sprintf( buf, "%s %s", arg4, argument );
               strcpy( argument, buf );
            }
            else
            {
               strcpy( argument, arg4 );
            }
            strcpy( arg4, arg3 );
         }
      }
      else
      {
         if( argument && argument[0] != '\0' && argument[0] != ' ' )
         {
            sprintf( buf, "%s %s", arg4, argument );
            strcpy( argument, buf );
         }
         else
         {
            strcpy( argument, arg4 );
         }
         strcpy( arg4, arg3 );
      }
   }

   if( power < 10 )
      power = 10;

   if( !check_concentration( ch, damage, action, power ) )
      return;

   if( damage == MAG_SOUND && IS_SILENT( ch ) )
   {
      send_to_char( "No sound comes out.\n\r", ch );
      return;
   }

   /*
    * Fourth arguemnt, handle and find an appropriate target 
    */
   switch ( action )
   {
      default:
         if( !str_cmp( arg4, "all" ) )
         {
            for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
            {
               ch_trick( victim, damage );
            }
         }
         else if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_trick( victim, damage );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_trick( ch, obj, damage );
         }
         else
         {
            ch_trick( ch, damage );
         }
         break;
      case ACT_HARM:
         if( !str_cmp( arg4, "all" ) )
         {
            for( victim = ch->in_room->first_person; victim; victim = next_vict )
            {
               next_vict = victim->next_in_room;
               if( ch == victim )
                  continue;
               ch_harm( victim, ch, power, damage, FALSE );
            }
         }
         else if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_harm( victim, ch, power, damage, FALSE );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_harm( ch, obj, damage, power );
         }
         else if( IS_FIGHTING( ch ) )
         {
            ch_harm( ch->last_hit, ch, power, damage, FALSE );
         }
         else
         {
            ch_trick( ch, damage );
         }
         break;
      case ACT_HELP:
         if( !str_cmp( arg4, "all" ) )
         {
            for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
            {
               ch_help( victim, ch, damage, power / 2 );
            }
         }
         else if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_help( victim, ch, damage, power );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_help( ch, obj, damage, power );
         }
         else
         {
            ch_help( ch, ch, damage, power );
         }
         break;
      case ACT_HINDER:
         if( !str_cmp( arg4, "all" ) )
         {
            for( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
            {
               if( ch == victim )
                  continue;
               ch_hinder( victim, ch, damage, power / 2 );
            }
         }
         else if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_hinder( victim, ch, damage, power );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_hinder( ch, obj, damage );
         }
         else
         {
            room_hinder( ch, ch->in_room, damage, power, arg4 );
         }
         break;
      case ACT_CREATE:
         learn_talent( ch, TAL_CATALYSM );
         if( damage == MAG_ETHER && ch->curr_talent[TAL_VOID] >= 100 )
         {
            void_create( ch, power, arg4 );
         }
         else if( damage == MAG_DREAM && ch->curr_talent[TAL_DREAM] >= 30 )
         {
            dream_create( ch, power, arg4, argument );
         }
         else
         {
            obj_create( ch, damage, power, arg4 );
         }
         break;
      case ACT_FIND:
         learn_talent( ch, TAL_SEEKING );
         if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_find( ch, obj, damage );
         }
         else if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_find( victim, ch, damage );
         }
         else
         {
            seek_find( ch, damage, arg4 );
         }
         break;
      case ACT_FOCUS:
         if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_focus( victim, ch, damage, power );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_focus( ch, obj, damage );
         }
         else
         {
            ch_focus( ch, ch, damage, power );
         }
         break;
      case ACT_MOVE:
         learn_talent( ch, TAL_MOTION );
         if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_move( ch, obj, damage, power, argument );
         }
         else if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_move( victim, ch, damage, arg4, argument );
         }
         else if( !arg4 || arg4[0] == '\0' )
         {
            ch_move( ch, ch, damage, arg4, argument );
         }
         else
         {
            summon_move( ch, damage, power, arg4 );
         }
         break;
      case ACT_CHANGE:
         if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_change( victim, ch, damage, argument );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_change( ch, obj, damage, power, argument );
         }
         else
         {
            ch_change( ch, ch, damage, argument );
         }
         break;
      case ACT_ENHANCE:
         if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_enhance( victim, ch, damage, power );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_enhance( ch, obj, damage, power );
         }
         else
         {
            ch_enhance( ch, ch, damage, power );
         }
         break;
      case ACT_RUNE:
         draw_rune( ch, ch->in_room, damage );
         break;
      case ACT_SING:
         if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_sing( victim, ch, damage, power );
         }
         else
         {
            song_sing( ch, damage, power, arg4 );
         }
         break;
      case ACT_CONTROL:
         if( ( victim = get_char_room( ch, arg4 ) ) != NULL )
         {
            ch_control( victim, ch, damage, power, arg4 );
         }
         else if( ( obj = get_obj_here( ch, arg4 ) ) != NULL )
         {
            obj_control( ch, obj, damage, power );
         }
         else
         {
            room_control( ch, damage, power, arg4 );
         }
         break;
   }
   if( TALENT( ch, talent ) >= 20 )
      power /= ( ( TALENT( ch, talent ) + 100 ) / 100 );
   use_magic( ch, talent, power );
   STRFREE( ch->last_taken );
   ch->last_taken = STRALLOC( "channeling" );
   WAIT_STATE( ch, power / 100 );
}
