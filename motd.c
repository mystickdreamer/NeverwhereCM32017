#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* MOTD Displayer -- Chris/James/Bill -- 4/99 */

void do_motd ( CHAR_DATA *ch, char *argument )
{		
    if ( IS_NPC(ch) )
        return;

        do_help( ch, "motd" );
}

/* MOTD File Edit -- Chris -- 4/99 */

/*
 *	Do no add do_medit, it is not done and does not work well -- KCAH
 */

void do_medit ( CHAR_DATA *ch, char *argument )
{		
    if ( IS_NPC(ch) )
        return;
    
/*      if ( argument[0] == '\0' )
        {
            send_to_char( "&wSyntax: medit <amotd/smotd/cmotd/nmotd/save>\n\r",ch );
            return;
        }
        else
        { */

/*	    interpret (ch, "clear"); */
/*	    set_char_color ( AT_BLUE, ch ); */
/*          send_to_char( "                           &w^x&wMessage of the Day Editor\n\r",ch ); */

            if ( !str_cmp( argument, "amotd" ) )
            {
/*              send_to_char( "&CAdministration MOTD\n\r\n\r",ch ); */
		do_hedit ( ch, "_amotd_");
/*		interpret (ch, "hedit _amotd_"); */
/*		return; */
            }
            else if ( !str_cmp( argument, "smotd" ) )
            {
                send_to_char( "&CStaff MOTD\n\r\n\r",ch );
		do_hedit ( ch, "_smotd_");
		return;
            }
            else if ( !str_cmp( argument, "cmotd" ) )
            {
                send_to_char( "&CCitizen MOTD\n\r\n\r",ch );
		do_hedit ( ch, "_cmotd_");
		return;
            }
            else if ( !str_cmp( argument, "nmotd" ) )
            {
                send_to_char( "&CNewbie Citizen MOTD\n\r\n\r",ch );
		do_hedit ( ch, "_nmotd_");
		return;
            }
/*            else if ( !str_cmp( argument, "/" ) )
            {
		do_hedit ( ch, "");
		return;
            } */
            else if ( !str_cmp( argument, "save" ) )
            {
		do_hset( ch, "level -1 _amotd_");
		do_hset( ch, "level -1 _smotd_");
		do_hset( ch, "level -1 _cmotd_");
		do_hset( ch, "level -1 _nmotd_");
		do_hset( ch, "save");
                send_to_char( "&w^x&wAll MOTD files saved.\n\r",ch );
		return;
            }
/*            else
            {
		set_char_color ( AT_BLUE, ch );
                send_to_char( "&w^x&wSyntax: medit <amotd/smotd/cmotd/nmotd/save>\n\r",ch );
                return;
            } */

        }
/* } */
