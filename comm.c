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
 *			 Low-level communication module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"
#include "md5.h"

/*
 * Socket and TCP/IP stuff.
 */
#ifdef WIN32
#include <io.h>
#undef EINTR
#undef EMFILE
#define EINTR WSAEINTR
#define EMFILE WSAEMFILE
#define EWOULDBLOCK WSAEWOULDBLOCK
#define MAXHOSTNAMELEN 32

#define  TELOPT_ECHO        '\x01'
#define  GA                 '\xF9'
#define  SB                 '\xFA'
#define  WILL               '\xFB'
#define  WONT               '\xFC'
#define  DO                 '\xFD'
#define  DONT               '\xFE'
#define  IAC                '\xFF'
void bailout( void );
void shutdown_checkpoint( void );
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#define closesocket close
#endif

#ifdef sun
int gethostname( char *name, int namelen );
#endif

const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const char go_ahead_str[] = { IAC, GA, '\0' };

void save_sysdata args( ( SYSTEM_DATA sys ) );
void drop_artifacts( CHAR_DATA * ch, OBJ_DATA * obj );
void save_world( CHAR_DATA * ch );

extern NATION_DATA *find_nation( char *name );
extern SPECIES_DATA *find_species( char *name );

/*
 * Global variables.
 */
DESCRIPTOR_DATA *first_descriptor;  /* First descriptor     */
DESCRIPTOR_DATA *last_descriptor;   /* Last descriptor      */
DESCRIPTOR_DATA *d_next;   /* Next descriptor in loop */
int num_descriptors;
FILE *fpReserve;  /* Reserved file handle    */
bool mud_down; /* Shutdown       */
bool service_shut_down; /* Shutdown by operator closing down service */
bool wizlock;  /* Game is wizlocked    */
time_t boot_time;
HOUR_MIN_SEC set_boot_time_struct;
HOUR_MIN_SEC *set_boot_time;
struct tm *new_boot_time;
struct tm new_boot_struct;
char str_boot_time[MAX_INPUT_LENGTH];
char lastplayercmd[MAX_INPUT_LENGTH * 2];
time_t current_time; /* Time of this pulse      */
int port;   /* Port number to be used       */
int control;   /* Controlling descriptor  */
int newdesc;   /* New descriptor    */
fd_set in_set; /* Set of desc's for reading  */
fd_set out_set;   /* Set of desc's for writing  */
fd_set exc_set;   /* Set of desc's with errors  */
int maxdesc;
char *alarm_section = "(unknown)";

/*
 * OS-dependent local functions.
 */
void game_loop args( (  ) );
int init_socket args( ( int port ) );
void new_descriptor args( ( int new_desc ) );
bool read_from_descriptor args( ( DESCRIPTOR_DATA * d ) );
bool write_to_descriptor args( ( int desc, char *txt, int length ) );


/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args( ( char *name, bool newchar ) );
bool check_reconnect args( ( DESCRIPTOR_DATA * d, char *name, bool fConn ) );
bool check_playing args( ( DESCRIPTOR_DATA * d, char *name, bool kick ) );
int main args( ( int argc, char **argv ) );
void nanny args( ( DESCRIPTOR_DATA * d, char *argument ) );
bool flush_buffer args( ( DESCRIPTOR_DATA * d, bool fPrompt ) );
void read_from_buffer args( ( DESCRIPTOR_DATA * d ) );
void stop_idling args( ( CHAR_DATA * ch ) );
void free_desc args( ( DESCRIPTOR_DATA * d ) );
void display_prompt args( ( DESCRIPTOR_DATA * d ) );
int make_color_sequence args( ( const char *col, char *buf, DESCRIPTOR_DATA * d ) );
void set_pager_input args( ( DESCRIPTOR_DATA * d, char *argument ) );
bool pager_output args( ( DESCRIPTOR_DATA * d ) );
void mail_count args( ( CHAR_DATA * ch ) );
void init_signals args( ( void ) );

/* from act_comm.c */
extern CHANNEL_DATA *find_chan args( ( char *argument ) );
extern void add_to_chan args( ( CHAR_DATA * ch, CHANNEL_DATA * chan ) );
extern bool remove_from_chan args( ( CHAR_DATA * ch, CHANNEL_DATA * chan ) );
extern CHANNEL_MEMBER *get_chatter args( ( CHAR_DATA * ch, CHANNEL_DATA * chan ) );

/* from birth.c */
extern void birth args( ( CHAR_DATA * ch ) );
extern void choose_sex args( ( CHAR_DATA * ch, char *argument ) );
extern void choose_species args( ( CHAR_DATA * ch, char *argument ) );
extern void choose_nation args( ( CHAR_DATA * ch, char *argument ) );

int port;

#ifdef WIN32
int mainthread( int argc, char **argv )
#else
int main( int argc, char **argv )
#endif
{
   struct timeval now_time;
   char hostn[128];
   bool fCopyOver = FALSE;
#ifdef IMC
   int imcsocket = -1;
#endif

   /*
    * Memory debugging if needed.
    */
#if defined(MALLOC_DEBUG)
   malloc_debug( 2 );
#endif

   num_descriptors = 0;
   first_descriptor = NULL;
   last_descriptor = NULL;
   sysdata.NO_NAME_RESOLVING = TRUE;

   /*
    * Init time.
    */
   gettimeofday( &now_time, NULL );
   current_time = ( time_t ) now_time.tv_sec;
/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
   boot_time = time( 0 );  /*  <-- I think this is what you wanted */
   strcpy( str_boot_time, ctime( &current_time ) );

   /*
    * Init boot time.
    */
   set_boot_time = &set_boot_time_struct;
   set_boot_time->manual = 0;

   new_boot_time = update_time( localtime( &current_time ) );
   /*
    * Copies *new_boot_time to new_boot_struct, and then points
    * new_boot_time to new_boot_struct again. -- Alty 
    */
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;
   new_boot_time->tm_mday += 1;
   if( new_boot_time->tm_hour > 12 )
      new_boot_time->tm_mday += 1;
   new_boot_time->tm_sec = 0;
   new_boot_time->tm_min = 0;
   new_boot_time->tm_hour = 6;

   /*
    * Update new_boot_time (due to day increment) 
    */
   new_boot_time = update_time( new_boot_time );
   new_boot_struct = *new_boot_time;
   new_boot_time = &new_boot_struct;
   /*
    * Bug fix submitted by Gabe Yoder 
    */
   new_boot_time_t = mktime( new_boot_time );
   reboot_check( mktime( new_boot_time ) );
   /*
    * Set reboot time string for do_time 
    */
   get_reboot_string(  );

   /*
    * Reserve two channels for our use.
    */
   if( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
   {
      perror( NULL_FILE );
      exit( 1 );
   }
   if( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
   {
      perror( NULL_FILE );
      exit( 1 );
   }

   /*
    * Get the port number.
    */
   port = 4000;
   if( argc > 1 )
   {
      if( !is_number( argv[1] ) )
      {
         fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
         exit( 1 );
      }
      else if( ( port = atoi( argv[1] ) ) <= 1024 )
      {
         fprintf( stderr, "Port number must be above 1024.\n" );
         exit( 1 );
      }
      if( argv[2] && argv[2][0] )
      {
         fCopyOver = TRUE;
         control = atoi( argv[3] );
#ifdef I3
         I3_socket = atoi( argv[4] );
#endif
#ifdef IMC
         imcsocket = atoi( argv[5] );
#endif
      }
      else
         fCopyOver = FALSE;
   }

   /*
    * Run the game.
    */
#ifdef WIN32
   {
      /*
       * Initialise Windows sockets library 
       */

      unsigned short wVersionRequested = MAKEWORD( 1, 1 );
      WSADATA wsadata;
      int err;

      /*
       * Need to include library: wsock32.lib for Windows Sockets 
       */
      err = WSAStartup( wVersionRequested, &wsadata );
      if( err )
      {
         fprintf( stderr, "Error %i on WSAStartup\n", err );
         exit( 1 );
      }

      /*
       * standard termination signals 
       */
      signal( SIGINT, ( void * )bailout );
      signal( SIGTERM, ( void * )bailout );
   }
#endif /* WIN32 */

   log_string( "Booting Database" );
   boot_db(  );
   log_string( "Initializing socket" );
   if( !fCopyOver )  /* We have already the port if copyover'ed */
      control = init_socket( port );

   init_signals(  );

   /*
    * I don't know how well this will work on an unnamed machine as I don't
    * have one handy, and the man pages are ever-so-helpful.. -- Alty 
    */
   if( gethostname( hostn, sizeof( hostn ) ) < 0 )
   {
      perror( "main: gethostname" );
      strcpy( hostn, "unresolved" );
   }
   sprintf( log_buf, "%s ready at address %s on port %d.", sysdata.mud_name, hostn, port );
   log_string( log_buf );

#ifdef I3
   /*
    * Initialize and connect to Intermud-3 
    */
   I3_main( FALSE, port, fCopyOver );
#endif
#ifdef IMC
   /*
    * Initialize and connect to IMC2 
    */
   imc_startup( FALSE, imcsocket, fCopyOver );
#endif

   if( fCopyOver )
   {
      log_string( "Running copyover_recover." );
      copyover_recover(  );
   }

   game_loop(  );

   /*
    * Save the world's position -- Scion 
    */
   {
      CHAR_DATA *vch;

      save_world( NULL );
      for( vch = first_char; vch; vch = vch->next )
      {
         if( !IS_NPC( vch ) )
            save_char_obj( vch );
      }
   }

   closesocket( control );

#ifdef WIN32
   if( service_shut_down )
   {
      CHAR_DATA *vch;

      /*
       * Save all characters before booting. 
       */
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
         {
            shutdown_checkpoint(  );
            save_char_obj( vch );
         }
   }
   /*
    * Shut down Windows sockets 
    */

   WSACleanup(  );   /* clean up */
   kill_timer(  );   /* stop timer thread */
#endif


   /*
    * That's all, folks.
    */
   log_string( "Normal termination of game." );
   exit( 0 );
   return 0;
}


int init_socket( int port )
{
   char hostname[64];
   struct sockaddr_in sa;
   int x = 1;
   int fd;

   gethostname( hostname, sizeof( hostname ) );


   if( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
   {
      perror( "Init_socket: socket" );
      exit( 1 );
   }

   if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( void * )&x, sizeof( x ) ) < 0 )
   {
      perror( "Init_socket: SO_REUSEADDR" );
      closesocket( fd );
      exit( 1 );
   }

#if defined(SO_DONTLINGER) && !defined(SYSV)
   {
      struct linger ld;

      ld.l_onoff = 1;
      ld.l_linger = 1000;

      if( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER, ( void * )&ld, sizeof( ld ) ) < 0 )
      {
         perror( "Init_socket: SO_DONTLINGER" );
         closesocket( fd );
         exit( 1 );
      }
   }
#endif

   memset( &sa, '\0', sizeof( sa ) );
   sa.sin_family = AF_INET;
   sa.sin_port = htons( port );

   /*
    * This line will allow the mud to run on a different IP 
    */
/*   sa.sin_addr.s_addr = inet_addr("64.242.77.131"); */

   if( bind( fd, ( struct sockaddr * )&sa, sizeof( sa ) ) == -1 )
   {
      perror( "Init_socket: bind" );
      closesocket( fd );
      exit( 1 );
   }

   if( listen( fd, 50 ) < 0 )
   {
      perror( "Init_socket: listen" );
      closesocket( fd );
      exit( 1 );
   }

   return fd;
}

/*
static void SegVio()
{
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    sprintf( buf, "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ',
    		ch->name, ch->in_room->vnum );
    log_string( buf );  
  }
  exit(0);
}
*/

/*
 * LAG alarm!							-Thoric
 */
void caught_alarm(  )
{
   char buf[MAX_STRING_LENGTH];

   sprintf( buf, "ALARM CLOCK!" );
   bug( buf );
   strcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\n\r" );
   echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
   if( newdesc )
   {
      FD_CLR( newdesc, &in_set );
      FD_CLR( newdesc, &out_set );
      FD_CLR( newdesc, &exc_set );
      log_string( "clearing newdesc" );
   }
}

bool check_bad_desc( int desc )
{
   if( FD_ISSET( desc, &exc_set ) )
   {
      FD_CLR( desc, &in_set );
      FD_CLR( desc, &out_set );
      log_string( "Bad FD caught and disposed." );
      return TRUE;
   }
   return FALSE;
}


void accept_new( int ctrl )
{
   static struct timeval null_time;
   DESCRIPTOR_DATA *d;
   /*
    * int maxdesc; Moved up for use with id.c as extern 
    */

#if defined(MALLOC_DEBUG)
   if( malloc_verify(  ) != 1 )
      abort(  );
#endif

   /*
    * Poll all active descriptors.
    */
   FD_ZERO( &in_set );
   FD_ZERO( &out_set );
   FD_ZERO( &exc_set );
   FD_SET( ctrl, &in_set );
   maxdesc = ctrl;
   newdesc = 0;
   for( d = first_descriptor; d; d = d->next )
   {
      maxdesc = UMAX( maxdesc, d->descriptor );
      FD_SET( d->descriptor, &in_set );
      FD_SET( d->descriptor, &out_set );
      FD_SET( d->descriptor, &exc_set );
      if( d == last_descriptor )
         break;
   }

   if( select( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
   {
      perror( "accept_new: select: poll" );
      exit( 1 );
   }

   if( FD_ISSET( ctrl, &exc_set ) )
   {
      bug( "Exception raise on controlling descriptor %d", ctrl );
      FD_CLR( ctrl, &in_set );
      FD_CLR( ctrl, &out_set );
   }
   else if( FD_ISSET( ctrl, &in_set ) )
   {
      newdesc = ctrl;
      new_descriptor( newdesc );
   }
}

void game_loop(  )
{
   struct timeval last_time;
   char cmdline[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
/*  time_t	last_check = 0;  */

#ifndef WIN32
   signal( SIGPIPE, SIG_IGN );
   signal( SIGALRM, caught_alarm );
#endif

   /*
    * signal( SIGSEGV, SegVio ); 
    */
   gettimeofday( &last_time, NULL );
   current_time = ( time_t ) last_time.tv_sec;

   /*
    * Main loop 
    */
   while( !mud_down )
   {
      accept_new( control );

      /*
       * Kick out descriptors with raised exceptions
       * or have been idle, then check for input.
       */
      for( d = first_descriptor; d; d = d_next )
      {
         if( d == d->next )
         {
            bug( "descriptor_loop: loop found & fixed" );
            d->next = NULL;
         }
         d_next = d->next;

         d->idle++;  /* make it so a descriptor can idle out */
         if( FD_ISSET( d->descriptor, &exc_set ) )
         {
            FD_CLR( d->descriptor, &in_set );
            FD_CLR( d->descriptor, &out_set );
            if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
               save_char_obj( d->character );
            d->outtop = 0;
            close_socket( d, TRUE );
            continue;
         }
         else if( ( !d->character && d->idle > 360 )  /* 2 mins */
                  || ( d->connected != CON_PLAYING && d->idle > 1200 )  /* 5 mins */
                  || ( d->idle > 28800 /* 2 hrs  */
                       && !IS_SET( d->character->pcdata->permissions, PERMIT_BUILD ) ) )
         {
            write_to_descriptor( d->descriptor, "Idle timeout... disconnecting.\n\r", 0 );
            d->outtop = 0;
            close_socket( d, TRUE );
            continue;
         }
         else
         {
            d->fcommand = FALSE;

            if( FD_ISSET( d->descriptor, &in_set ) )
            {
               d->idle = 0;
               if( d->character )
                  d->character->timer = 0;
               if( !read_from_descriptor( d ) )
               {
                  FD_CLR( d->descriptor, &out_set );
                  if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                     save_char_obj( d->character );
                  d->outtop = 0;
                  close_socket( d, FALSE );
                  continue;
               }
            }

            if( d->character && d->character->wait > 0 )
               --d->character->wait;

            /*
             * Check for running aliases -- Scion 
             */
            if( d->connected == CON_PLAYING && d->character->pcdata && d->character->pcdata->alias_queue != NULL )
            {
               ALIAS_QUEUE *temp;

               temp = d->character->pcdata->alias_queue;
               interpret( d->character, d->character->pcdata->alias_queue->cmd, FALSE );
               d->character->pcdata->alias_queue = d->character->pcdata->alias_queue->next;

               DISPOSE( temp );

               d->character->pcdata->alias_used++;
               if( d->character->pcdata->alias_used >= 20 )
               {
                  d->character->pcdata->alias_queue = NULL;
                  d->character->pcdata->alias_used = 0;
                  send_to_char( "Infinite loop detected... Your alias has been terminated.\r\n", d->character );
               }
               continue;
            }

            read_from_buffer( d );
            if( d->incomm[0] != '\0' )
            {
               d->fcommand = TRUE;
               stop_idling( d->character );

               strcpy( cmdline, d->incomm );
               d->incomm[0] = '\0';

               if( d->character )
                  set_cur_char( d->character );

               if( d->pagepoint )
                  set_pager_input( d, cmdline );
               else
                  switch ( d->connected )
                  {
                     default:
                        nanny( d, cmdline );
                        break;
                     case CON_PLAYING:
                        if( d->character && !IS_NPC( d->character ) )
                           d->character->pcdata->alias_used = 0;
                        interpret( d->character, cmdline, FALSE );
                        break;
                     case CON_EDITING:
                        edit_buffer( d->character, cmdline );
                        break;
                     case CON_CHOOSE_SEX:
                        choose_sex( d->character, cmdline );
                        break;
                     case CON_CHOOSE_SPECIES:
                        choose_species( d->character, cmdline );
                        break;
                     case CON_CHOOSE_NATION:
                        choose_nation( d->character, cmdline );
                        break;
                  }
            }
         }
         if( d == last_descriptor )
            break;
      }

#ifdef I3
      I3_loop(  );
#endif
#ifdef IMC
      imc_loop(  );
#endif

      /*
       * Autonomous game motion.
       */
      update_handler(  );

      /*
       * Output.
       */
      for( d = first_descriptor; d; d = d_next )
      {
         d_next = d->next;

         if( ( d->fcommand || d->outtop > 0 ) && FD_ISSET( d->descriptor, &out_set ) )
         {
            if( d->pagepoint )
            {
               if( !pager_output( d ) )
               {
                  if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                     save_char_obj( d->character );
                  d->outtop = 0;
                  close_socket( d, FALSE );
               }
            }
            else if( !flush_buffer( d, TRUE ) )
            {
               if( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                  save_char_obj( d->character );
               d->outtop = 0;
               close_socket( d, FALSE );
            }
         }
         if( d == last_descriptor )
            break;
      }

      /*
       * Synchronize to a clock.
       * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
       * Careful here of signed versus unsigned arithmetic.
       */
      {
         struct timeval now_time;
         long secDelta;
         long usecDelta;

         gettimeofday( &now_time, NULL );
         usecDelta = ( ( int )last_time.tv_usec ) - ( ( int )now_time.tv_usec ) + 1000000 / PULSE_PER_SECOND;
         secDelta = ( ( int )last_time.tv_sec ) - ( ( int )now_time.tv_sec );
         while( usecDelta < 0 )
         {
            usecDelta += 1000000;
            secDelta -= 1;
         }

         while( usecDelta >= 1000000 )
         {
            usecDelta -= 1000000;
            secDelta += 1;
         }

         if( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
         {
            struct timeval stall_time;

            stall_time.tv_usec = usecDelta;
            stall_time.tv_sec = secDelta;
#ifdef WIN32
            Sleep( ( stall_time.tv_sec * 1000L ) + ( stall_time.tv_usec / 1000L ) );
#else
            if( select( 0, NULL, NULL, NULL, &stall_time ) < 0 && errno != EINTR )
            {
               perror( "game_loop: select: stall" );
               exit( 1 );
            }
#endif
         }
      }

      gettimeofday( &last_time, NULL );
      current_time = ( time_t ) last_time.tv_sec;

      /*
       * Check every 10 seconds...  
       * if ( last_check+1800 < current_time )
       * {
       * CHECK_LINKS(first_char, last_char, next, prev, CHAR_DATA);
       * last_check = current_time;
       * }
       */

   }
   fflush( stderr ); /* make sure strerr is flushed */
   return;
}

void init_descriptor( DESCRIPTOR_DATA * dnew, int desc )
{
   dnew->next = NULL;
   dnew->descriptor = desc;
   dnew->connected = CON_GET_NAME;
   dnew->outsize = 2000;
   dnew->idle = 0;
   dnew->lines = 0;
   dnew->auth_fd = -1;
   dnew->auth_inc = 0;
   dnew->auth_state = 0;
   dnew->scrlen = 24;
   dnew->user = STRALLOC( "unknown" );
   dnew->newstate = 0;
   dnew->prevcolor = 0x07;
   CREATE( dnew->outbuf, char, dnew->outsize );
}


void new_descriptor( int new_desc )
{
   char buf[MAX_STRING_LENGTH];
   DESCRIPTOR_DATA *dnew;
   struct sockaddr_in sock;
   struct hostent *from;
   int desc;
   socklen_t size;
   char bugbuf[MAX_STRING_LENGTH];
#ifdef WIN32
   unsigned long arg = 1;
#endif

   size = sizeof( sock );
   if( check_bad_desc( new_desc ) )
   {
      set_alarm( 0 );
      return;
   }
   set_alarm( 20 );
   alarm_section = "new_descriptor::accept";
   if( ( desc = accept( new_desc, ( struct sockaddr * )&sock, &size ) ) < 0 )
   {
      perror( "New_descriptor: accept" );
      sprintf( bugbuf, "[*****] BUG: New_descriptor: accept" );
      log_string_plus( bugbuf, LOG_COMM, 0 );
      set_alarm( 0 );
      return;
   }
   if( check_bad_desc( new_desc ) )
   {
      set_alarm( 0 );
      return;
   }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

   set_alarm( 20 );
   alarm_section = "new_descriptor: after accept";

#ifdef WIN32
   if( ioctlsocket( desc, FIONBIO, &arg ) == -1 )
#else
   if( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
#endif
   {
      perror( "New_descriptor: fcntl: FNDELAY" );
      set_alarm( 0 );
      return;
   }
   if( check_bad_desc( new_desc ) )
      return;

   CREATE( dnew, DESCRIPTOR_DATA, 1 );
   dnew->next = NULL;
   dnew->descriptor = desc;
   dnew->connected = CON_GET_NAME;
   dnew->outsize = 2000;
   dnew->idle = 0;
   dnew->lines = 0;
   dnew->scrlen = 24;
   dnew->port = ntohs( sock.sin_port );
   dnew->user = STRALLOC( "(unknown)" );
   dnew->newstate = 0;
   dnew->prevcolor = 0x07;

   CREATE( dnew->outbuf, char, dnew->outsize );

   strcpy( buf, inet_ntoa( sock.sin_addr ) );
   sprintf( log_buf, "Sock.sinaddr:  %s, port %hd.", buf, dnew->port );
   log_string_plus( log_buf, LOG_COMM, 0 );
   if( sysdata.NO_NAME_RESOLVING )
      dnew->host = STRALLOC( buf );
   else
   {
      from = gethostbyaddr( ( char * )&sock.sin_addr, sizeof( sock.sin_addr ), AF_INET );
      dnew->host = STRALLOC( ( char * )( from ? from->h_name : buf ) );
   }

   if( check_total_bans( dnew ) )
   {
      write_to_descriptor( desc, "Your site has been banned from this Mud.\n\r", 0 );
      free_desc( dnew );
      set_alarm( 0 );
      return;
   }

   /*
    * Init descriptor data.
    */

   if( !last_descriptor && first_descriptor )
   {
      DESCRIPTOR_DATA *d;

      bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
      for( d = first_descriptor; d; d = d->next )
         if( !d->next )
            last_descriptor = d;
   }

   LINK( dnew, first_descriptor, last_descriptor, next, prev );

   /*
    * Send the greeting.
    */
   {
      extern char *help_greeting;
      if( help_greeting[0] == '.' )
         write_to_buffer( dnew, help_greeting + 1, 0 );
      else
         write_to_buffer( dnew, help_greeting, 0 );
   }

   if( ++num_descriptors > sysdata.maxplayers )
      sysdata.maxplayers = num_descriptors;
   if( sysdata.maxplayers > sysdata.alltimemax )
   {
      if( sysdata.time_of_max )
         DISPOSE( sysdata.time_of_max );
      sprintf( buf, "%24.24s", ctime( &current_time ) );
      sysdata.time_of_max = str_dup( buf );
      sysdata.alltimemax = sysdata.maxplayers;
      sprintf( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );
      log_string_plus( log_buf, LOG_COMM, 0 );
      to_channel( log_buf, "&RAttention", 0 );
      save_sysdata( sysdata );
   }
   set_alarm( 0 );
   return;
}

void free_desc( DESCRIPTOR_DATA * d )
{
   closesocket( d->descriptor );
   STRFREE( d->host );
   DISPOSE( d->outbuf );
   STRFREE( d->user );  /* identd */
   if( d->pagebuf )
      DISPOSE( d->pagebuf );
   DISPOSE( d );
   return;
}

void close_socket( DESCRIPTOR_DATA * dclose, bool force )
{
   CHAR_DATA *ch;
   DESCRIPTOR_DATA *d;
   bool DoNotUnlink = FALSE;

   /*
    * flush outbuf 
    */
   if( !force && dclose->outtop > 0 )
      flush_buffer( dclose, FALSE );

   /*
    * say bye to whoever's snooping this descriptor 
    */
   if( dclose->snoop_by )
      write_to_buffer( dclose->snoop_by, "Your victim has left the game.\n\r", 0 );

   /*
    * stop snooping everyone else 
    */
   for( d = first_descriptor; d; d = d->next )
      if( d->snoop_by == dclose )
         d->snoop_by = NULL;

   /*
    * Check for switched people who go link-dead. -- Altrag 
    */
   if( dclose->original )
   {
      if( ( ch = dclose->character ) != NULL )
         do_return( ch, "" );
      else
      {
         bug( "Close_socket: dclose->original without character %s",
              ( dclose->original->name ? dclose->original->name : "unknown" ) );
         dclose->character = dclose->original;
         dclose->original = NULL;
      }
   }

   ch = dclose->character;

   /*
    * sanity check :( 
    */
   if( !dclose->prev && dclose != first_descriptor )
   {
      DESCRIPTOR_DATA *dp, *dn;
      bug( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!",
           ch ? ch->name : d->host, dclose, first_descriptor );
      dp = NULL;
      for( d = first_descriptor; d; d = dn )
      {
         dn = d->next;
         if( d == dclose )
         {
            bug( "Close_socket: %s desc:%p found, prev should be:%p, fixing.", ch ? ch->name : d->host, dclose, dp );
            dclose->prev = dp;
            break;
         }
         dp = d;
      }
      if( !dclose->prev )
      {
         bug( "Close_socket: %s desc:%p could not be found!.", ch ? ch->name : dclose->host, dclose );
         DoNotUnlink = TRUE;
      }
   }
   if( !dclose->next && dclose != last_descriptor )
   {
      DESCRIPTOR_DATA *dp, *dn;
      bug( "Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!",
           ch ? ch->name : d->host, dclose, last_descriptor );
      dn = NULL;
      for( d = last_descriptor; d; d = dp )
      {
         dp = d->prev;
         if( d == dclose )
         {
            bug( "Close_socket: %s desc:%p found, next should be:%p, fixing.", ch ? ch->name : d->host, dclose, dn );
            dclose->next = dn;
            break;
         }
         dn = d;
      }
      if( !dclose->next )
      {
         bug( "Close_socket: %s desc:%p could not be found!.", ch ? ch->name : dclose->host, dclose );
         DoNotUnlink = TRUE;
      }
   }

   if( dclose->character )
   {
      do_channels( dclose->character, "quiet" );
      sprintf( log_buf, "Closing link to %s.", ch->pcdata->filename );
      log_string_plus( log_buf, LOG_COMM, 0 );
      if( dclose->connected == CON_PLAYING || dclose->connected == CON_EDITING )
      {
         act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_CANSEE );
         ch->desc = NULL;
      }
      else
      {
         /*
          * clear descriptor pointer to get rid of bug message in log 
          */
         dclose->character->desc = NULL;
         free_char( dclose->character );
      }
   }


   if( !DoNotUnlink )
   {
      /*
       * make sure loop doesn't get messed up 
       */
      if( d_next == dclose )
         d_next = d_next->next;
      UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
   }

   if( dclose->descriptor == maxdesc )
      --maxdesc;

   free_desc( dclose );
   --num_descriptors;
   return;
}


bool read_from_descriptor( DESCRIPTOR_DATA * d )
{
   int iStart, iErr;

   /*
    * Hold horses if pending command already. 
    */
   if( d->incomm[0] != '\0' )
      return TRUE;

   /*
    * Check for overflow. 
    */
   iStart = strlen( d->inbuf );
   if( iStart >= sizeof( d->inbuf ) - 10 )
   {
      sprintf( log_buf, "%s input overflow!", d->host );
      log_string( log_buf );
      write_to_descriptor( d->descriptor,
                           "\n\r*** PUT A LID ON IT!!! ***\n\rYou cannot enter the same command more than 20 consecutive times!\n\r",
                           0 );
      return FALSE;
   }

   for( ;; )
   {
      int nRead;

      nRead = recv( d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart, 0 );
#ifdef WIN32
      iErr = WSAGetLastError(  );
#else
      iErr = errno;
#endif
      if( nRead > 0 )
      {
         iStart += nRead;
         if( d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r' )
            break;
      }
      else if( nRead == 0 )
      {
         log_string_plus( "EOF encountered on read.", LOG_COMM, 0 );
         return FALSE;
      }
      else if( iErr == EWOULDBLOCK )
         break;
      else
      {
         perror( "Read_from_descriptor" );
         return FALSE;
      }
   }

   d->inbuf[iStart] = '\0';
   return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA * d )
{
   int i, j, k;

   /*
    * Hold horses if pending command already.
    */
   if( d->incomm[0] != '\0' )
      return;

   /*
    * Look for at least one new line.
    */
   for( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i < MAX_INBUF_SIZE; i++ )
   {
      if( d->inbuf[i] == '\0' )
         return;
   }

   /*
    * Canonical input processing.
    */
   for( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
   {
      if( k >= 254 )
      {
         write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

         /*
          * skip the rest of the line 
          */
         /*
          * for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
          * {
          * if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
          * break;
          * }
          */
         d->inbuf[i] = '\n';
         d->inbuf[i + 1] = '\0';
         break;
      }

      if( d->inbuf[i] == '\b' && k > 0 )
         --k;
      else if( isascii( d->inbuf[i] ) && isprint( (int)d->inbuf[i] ) )
         d->incomm[k++] = d->inbuf[i];
   }

   /*
    * Finish off the line.
    */
   if( k == 0 )
      d->incomm[k++] = ' ';
   d->incomm[k] = '\0';

   /*
    * Deal with bozos with #repeat 1000 ...
    */
   if( k > 1 || d->incomm[0] == '!' )
   {
      if( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
      {
         d->repeat = 0;
      }
      else
      {
         if( ++d->repeat >= 100 && d->connected >= 0 )
         {
            sprintf( log_buf, "%s input spamming: %s!", d->host, d->incomm );
            log_string( log_buf );
/*
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\rYou cannot enter the same command more than 100 consecutive times!\n\r", 0 );
		strcpy( d->incomm, "quit" );
*/ }
      }
   }

   /*
    * Do '!' substitution.
    */
   if( d->incomm[0] == '!' && d->incomm[1] == '!' )
      sprintf( d->incomm, "%s %s", d->inlast, d->incomm + 2 );
   else if( d->incomm[0] == '!' )
      strcpy( d->incomm, d->inlast );
   else
      strcpy( d->inlast, d->incomm );

   /*
    * Shift the input buffer.
    */
   while( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
      i++;
   for( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != '\0'; j++ )
      ;
   return;
}



/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA * d, bool fPrompt )
{
   char buf[MAX_INPUT_LENGTH];
   extern bool mud_down;

   /*
    * If buffer has more than 4K inside, spit out .5K at a time   -Thoric
    */
   if( !mud_down && d->outtop > 4096 )
   {
      memcpy( buf, d->outbuf, 512 );
      memmove( d->outbuf, d->outbuf + 512, d->outtop - 512 );
      d->outtop -= 512;
      if( d->snoop_by )
      {
         char snoopbuf[MAX_INPUT_LENGTH];

         buf[512] = '\0';
         if( d->character && d->character->name )
         {
            if( d->original && d->original->name )
               sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
            else
               sprintf( snoopbuf, "%s", d->character->name );
            write_to_buffer( d->snoop_by, snoopbuf, 0 );
         }
         write_to_buffer( d->snoop_by, "% ", 2 );
         write_to_buffer( d->snoop_by, buf, 0 );
      }
      if( !write_to_descriptor( d->descriptor, buf, 512 ) )
      {
         d->outtop = 0;
         return FALSE;
      }
      return TRUE;
   }


   /*
    * Bust a prompt.
    */
   if( fPrompt && !mud_down && d->connected == CON_PLAYING )
   {
      CHAR_DATA *ch;

      ch = d->original ? d->original : d->character;
      if( xIS_SET( ch->act, PLR_BLANK ) )
         write_to_buffer( d, "\n\r", 2 );


      if( xIS_SET( ch->act, PLR_PROMPT ) )
         display_prompt( d );
      if( xIS_SET( ch->act, PLR_TELNET_GA ) )
         write_to_buffer( d, go_ahead_str, 0 );
   }

   /*
    * Short-circuit if nothing to write.
    */
   if( d->outtop == 0 )
      return TRUE;

   /*
    * Snoop-o-rama.
    */
   if( d->snoop_by )
   {
      /*
       * without check, 'force mortal quit' while snooped caused crash, -h 
       */
      if( d->character && d->character->name )
      {
         /*
          * Show original snooped names. -- Altrag 
          */
         if( d->original && d->original->name )
            sprintf( buf, "%s (%s)", d->character->name, d->original->name );
         else
            sprintf( buf, "%s", d->character->name );
         write_to_buffer( d->snoop_by, buf, 0 );
      }
      write_to_buffer( d->snoop_by, "% ", 2 );
      write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
   }

   /*
    * OS-dependent output.
    */
   if( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
   {
      d->outtop = 0;
      return FALSE;
   }
   else
   {
      d->outtop = 0;
      return TRUE;
   }
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA * d, const char *txt, int length )
{
   if( !d )
   {
      bug( "Write_to_buffer: NULL descriptor" );
      return;
   }

   /*
    * Normally a bug... but can happen if loadup is used.
    */
   if( !d->outbuf )
      return;

   /*
    * Find length in case caller didn't.
    */
   if( length <= 0 )
      length = strlen( txt );

/* Uncomment if debugging or something
    if ( length != strlen(txt) )
    {
	bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
	length = strlen(txt);
    }
*/

   /*
    * Initial \n\r if needed.
    */
   if( d->outtop == 0 && !d->fcommand )
   {
      d->outbuf[0] = '\n';
      d->outbuf[1] = '\r';
      d->outtop = 2;
   }

   /*
    * Expand the buffer as needed.
    */
   while( d->outtop + length >= d->outsize )
   {
      if( d->outsize > 32000 )
      {
         /*
          * empty buffer 
          */
         d->outtop = 0;
         close_socket( d, TRUE );
         bug( "Buffer overflow. Closing (%s).", d->character ? d->character->name : "???" );
         return;
      }
      d->outsize *= 2;
      RECREATE( d->outbuf, char, d->outsize );
   }

   /*
    * Copy.
    */
   strncpy( d->outbuf + d->outtop, txt, length );
   d->outtop += length;
   d->outbuf[d->outtop] = '\0';
   return;
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
   int iStart;
   int nWrite;
   int nBlock;

   if( length <= 0 )
      length = strlen( txt );

   for( iStart = 0; iStart < length; iStart += nWrite )
   {
      nBlock = UMIN( length - iStart, 4096 );
      if( ( nWrite = send( desc, txt + iStart, nBlock, 0 ) ) < 0 )
      {
         perror( "Write_to_descriptor" );
         return FALSE;
      }
   }

   return TRUE;
}

/* Attempt to clean up some things -keo */
void show_species( DESCRIPTOR_DATA * d )
{
   SPECIES_DATA *species;
   char buf[MAX_STRING_LENGTH];

   write_to_buffer( d, "\n\rYou may choose from the following species:\n\r", 0 );
   buf[0] = '\0';
   species = first_species;
   while( species )
   {
      if( ( strlen( buf ) + strlen( species->name ) ) > 75 )
      {
         strcat( buf, "\n\r" );
         write_to_buffer( d, buf, 0 );
         buf[0] = '\0';
      }
      else
         strcat( buf, " | " );
      strcat( buf, species->name );
      species = species->next;
   }
   strcat( buf, " |\n\r " );
   write_to_buffer( d, buf, 0 );
   write_to_buffer( d, "Type HELP <species> for more information.\n\r", 0 );
   write_to_buffer( d, "Choose your species, or type RANDOM for a random race:", 0 );
}

void show_nations( DESCRIPTOR_DATA * d, SPECIES_DATA * species )
{
   char buf[MAX_STRING_LENGTH];
   NATION_DATA *nation;

   if( !species )
   {
      write_to_buffer( d, "\n\rNo species selected.\n\r", 0 );
      return;
   }
   write_to_buffer( d, "\n\rYou may choose from the following nations:\n\r", 0 );
   buf[0] = '\0';
   nation = first_nation;
   while( nation )
   {
      if( ( strlen( buf ) + strlen( nation->name ) ) > 75 )
      {
         strcat( buf, "\n\r" );
         write_to_buffer( d, buf, 0 );
         buf[0] = '\0';
      }
      else if( !str_cmp( nation->species, species->name ) && !IS_SET( nation->flags, NAT_NPC ) )
      {
         strcat( buf, " | " );
         strcat( buf, nation->name );
      }
      nation = nation->next;
   }
   strcat( buf, " |\n\r " );
   write_to_buffer( d, buf, 0 );
   write_to_buffer( d, "Type HELP <nation> for more information,\n\r", 0 );
   write_to_buffer( d, "or type BACK to return to species selection.\n\r", 0 );
   write_to_buffer( d, "Choose your nation:", 0 );
}

char *smaug_crypt( const char *pwd )
{
   md5_state_t state;
   md5_byte_t digest[16];
   static char passwd[16];
   unsigned int x;

   md5_init( &state );
   md5_append( &state, ( const md5_byte_t * )pwd, strlen( pwd ) );
   md5_finish( &state, digest );

   strncpy( passwd, ( const char * )digest, 16 );

   /*
    * The listed exceptions below will fubar the MD5 authentication packets, so change them 
    */
   for( x = 0; x < strlen( passwd ); x++ )
   {
      if( passwd[x] == '\n' )
         passwd[x] = 'n';
      if( passwd[x] == '\r' )
         passwd[x] = 'r';
      if( passwd[x] == '\t' )
         passwd[x] = 't';
      if( passwd[x] == ' ' )
         passwd[x] = 's';
      if( ( int )passwd[x] == 11 )
         passwd[x] = 'x';
      if( ( int )passwd[x] == 12 )
         passwd[x] = 'X';
      if( passwd[x] == '~' )
         passwd[x] = '+';
      if( passwd[x] == EOF )
         passwd[x] = 'E';
   }
   return ( passwd );
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA * d, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *ch;
   char *pwdnew;
   char *p;
   bool fOld, chk;

   while( isspace( (int)*argument ) )
      argument++;

   ch = d->character;

   switch ( d->connected )
   {

      default:
         bug( "Nanny: bad d->connected %d.", d->connected );
         close_socket( d, TRUE );
         return;

	case CON_OEDIT:
        oedit_parse( d, argument );
        break;

    case CON_REDIT:
        redit_parse( d, argument );
        break;

    case CON_MEDIT:
        medit_parse( d, argument );
        break;

      case CON_GET_NAME:
         if( argument[0] == '\0' )
         {
            close_socket( d, FALSE );
            return;
         }

         argument[0] = UPPER( argument[0] );

         /*
          * Old players can keep their characters. -- Alty 
          */
         if( !check_parse_name( argument, ( d->newstate != 0 ) ) )
         {
            write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
            return;
         }

         if( !str_prefix( argument, "Name" ) )
         {
            char name[MAX_STRING_LENGTH];

            sprintf( buf, "\r\nRandom name: %s\r\nName: ", random_name( name, LANG_DROW ) );
            write_to_buffer( d, buf, 0 );
            return;
         }

         if( !str_cmp( argument, "New" ) )
         {
            if( d->newstate == 0 )
            {
               /*
                * New player 
                */
               /*
                * Don't allow new players if DENY_NEW_PLAYERS is true 
                */
               if( sysdata.DENY_NEW_PLAYERS == TRUE )
               {
                  sprintf( buf, "The mud is currently preparing for a reboot.\n\r" );
                  write_to_buffer( d, buf, 0 );
                  sprintf( buf, "New players are not accepted during this time.\n\r" );
                  write_to_buffer( d, buf, 0 );
                  sprintf( buf, "Please try again in a few minutes.\n\r" );
                  write_to_buffer( d, buf, 0 );
                  close_socket( d, FALSE );
               }
               sprintf( buf, "\n\rChoosing a name is one of the most important parts of this game...\n\r"
                        "Make sure to pick a name appropriate to the character you are going\n\r"
                        "to role play, and be sure that it is not offensive to anyone.\n\r"
                        "If the name you select is not acceptable, you will be asked to choose\n\r"
                        "another one.\n\r"
                        "If you need help choosing a name, type the word 'name' and the game will\n\r"
                        "suggest a randomly generated name. We do not promise that generated names\n\r"
                        "will be appropriate or even valid names. They are intended to help you use\n\r"
                        "your imagination to invent a name for yourself.\n\r\n\rPlease choose a name for your character: " );
               write_to_buffer( d, buf, 0 );
               d->newstate++;
               d->connected = CON_GET_NAME;
               return;
            }
            else
            {
               write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
               return;
            }
         }

         if( check_playing( d, argument, FALSE ) == BERR )
         {
            write_to_buffer( d, "Name: ", 0 );
            return;
         }

         fOld = load_char_obj( d, argument, TRUE );
         if( !d->character )
         {
            sprintf( log_buf, "Bad player file %s@%s.", argument, d->host );
            log_string( log_buf );
            write_to_buffer( d, "Your playerfile is corrupt...Please notify ", 0 );
            write_to_buffer( d, sysdata.email, 0 );
            close_socket( d, FALSE );
            return;
         }
         ch = d->character;

         if( check_multi( ch ) )
         {
            write_to_buffer( d, "\r\nYou have reached the multiplaying limit, please log a character off first.\r\n", 0 );
            log_string_plus( "Multiplaying limit.", LOG_COMM, 0 );
            close_socket( d, FALSE );
            return;
         }

         if( xIS_SET( ch->act, PLR_DENY ) )
         {
            sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
            log_string_plus( log_buf, LOG_COMM, 0 );
            if( d->newstate != 0 )
            {
               write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
               d->connected = CON_GET_NAME;
               d->character->desc = NULL;
               free_char( d->character ); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               return;
            }
            write_to_buffer( d, "You are denied access.\n\r", 0 );
            close_socket( d, FALSE );
            return;
         }

         chk = check_reconnect( d, argument, FALSE );
         if( chk == BERR )
            return;

         if( chk )
         {
            fOld = TRUE;
         }
         else
         {
            if( wizlock && !ch->pcdata->permissions )
            {
               write_to_buffer( d, "The game is wizlocked.  Only permitted characters can connect now.\n\r", 0 );
               write_to_buffer( d, "Please try back later.\n\r", 0 );
               close_socket( d, FALSE );
               return;
            }
         }

         if( fOld )
         {
            if( d->newstate != 0 )
            {
               write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
               d->connected = CON_GET_NAME;
               d->character->desc = NULL;
               free_char( d->character ); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               return;
            }
            /*
             * Old player 
             */
            write_to_buffer( d, "Password: ", 0 );
            write_to_buffer( d, echo_off_str, 0 );
            d->connected = CON_GET_OLD_PASSWORD;
            return;
         }
         else
         {
            if( d->newstate == 0 )
            {
               /*
                * No such player 
                */
               write_to_buffer( d,
                                "\n\rNo such player exists.\n\rPlease check your spelling, or type new to start a new player.\n\r\n\rName: ",
                                0 );
               d->connected = CON_GET_NAME;
               d->character->desc = NULL;
               free_char( d->character ); /* Big Memory Leak before --Shaddai */
               d->character = NULL;
               return;
            }

            sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
            write_to_buffer( d, buf, 0 );
            STRFREE( ch->name );
            ch->name = STRALLOC( capitalize( argument ) );
            d->connected = CON_CONFIRM_NEW_NAME;
            return;
         }
         break;

      case CON_GET_OLD_PASSWORD:
         write_to_buffer( d, "\n\r", 2 );

         if( str_cmp( smaug_crypt( argument ), ch->pcdata->pwd ) )
         {
            write_to_buffer( d, "Wrong password, disconnecting.\n\r", 0 );
            /*
             * clear descriptor pointer to get rid of bug message in log 
             */
            d->character->desc = NULL;
            close_socket( d, FALSE );
            return;
         }

         write_to_buffer( d, echo_on_str, 0 );

         if( check_playing( d, ch->pcdata->filename, TRUE ) )
            return;

         chk = check_reconnect( d, ch->pcdata->filename, TRUE );
         if( chk == BERR )
         {
            if( d->character && d->character->desc )
               d->character->desc = NULL;
            close_socket( d, FALSE );
            return;
         }
         if( chk == TRUE )
            return;

         strncpy( buf, ch->pcdata->filename, MAX_STRING_LENGTH );
         d->character->desc = NULL;
         free_char( d->character );
         d->character = NULL;
         fOld = load_char_obj( d, buf, FALSE );
         ch = d->character;
         if( ch->position > POS_SITTING && ch->position < POS_STANDING )
            ch->position = POS_STANDING;

         sprintf( log_buf, "%s@%s(%s) has connected.", ch->pcdata->filename, d->host, d->user );
         log_string_plus( log_buf, LOG_COMM, 0 );
         write_to_buffer( d, "\r\nPress [ENTER] ", 0 );
         d->connected = CON_PRESS_ENTER;
         break;

      case CON_CONFIRM_NEW_NAME:
         switch ( *argument )
         {
            case 'y':
            case 'Y':
               sprintf( buf, "\n\rMake sure to use a password that won't be easily guessed by someone else."
                        "\n\rPick a good password for %s: %s", ch->name, echo_off_str );
               write_to_buffer( d, buf, 0 );
               d->connected = CON_GET_NEW_PASSWORD;
               break;

            case 'n':
            case 'N':
               write_to_buffer( d, "Ok, what IS it, then? ", 0 );
               /*
                * clear descriptor pointer to get rid of bug message in log 
                */
               d->character->desc = NULL;
               free_char( d->character );
               d->character = NULL;
               d->connected = CON_GET_NAME;
               break;

            default:
               write_to_buffer( d, "Please type Yes or No. ", 0 );
               break;
         }
         break;

      case CON_GET_NEW_PASSWORD:
         write_to_buffer( d, "\n\r", 2 );

         if( strlen( argument ) < 5 )
         {
            write_to_buffer( d, "Password must be at least five characters long.\n\rPassword: ", 0 );
            return;
         }

         pwdnew = smaug_crypt( argument );
         for( p = pwdnew; *p != '\0'; p++ )
         {
            if( *p == '~' )
            {
               write_to_buffer( d, "New password not acceptable, try again.\n\rPassword: ", 0 );
               return;
            }
         }

         DISPOSE( ch->pcdata->pwd );
         ch->pcdata->pwd = str_dup( pwdnew );
         write_to_buffer( d, "\n\rPlease retype the password to confirm: ", 0 );
         d->connected = CON_CONFIRM_NEW_PASSWORD;
         break;

      case CON_CONFIRM_NEW_PASSWORD:
         write_to_buffer( d, "\n\r", 2 );

         if( str_cmp( smaug_crypt( argument ), ch->pcdata->pwd ) )
         {
            write_to_buffer( d, "Passwords don't match.\n\rRetype password: ", 0 );
            d->connected = CON_GET_NEW_PASSWORD;
            return;
         }

         write_to_buffer( d, echo_on_str, 0 );
         write_to_buffer( d, "\n\rWould you like ANSI or no color support, (A/N)?", 0 );
         d->connected = CON_GET_WANT_RIPANSI;
         break;

      case CON_GET_WANT_RIPANSI:
         switch ( argument[0] )
         {
            case 'a':
            case 'A':
               xSET_BIT( ch->act, PLR_ANSI );
               break;
            case 'n':
            case 'N':
               break;
            default:
               write_to_buffer( d, "Invalid selection.\n\rANSI or NONE? ", 0 );
               return;
         }

         write_to_buffer( d, "Your character is being created...\n\r", 0 );
         {
            char motdbuf[MAX_STRING_LENGTH];
            sprintf( motdbuf, "\n\rWelcome to %s...\n\r", sysdata.mud_name );
            write_to_buffer( d, motdbuf, 0 );
         }
         ch->nation = first_nation;
         ch->pcdata->inborn = -1;
         ch->pcdata->points = 300;
         birth( ch );
         ch->first_timer = NULL;
         ch->perm_str = 100;
         ch->perm_wil = 100;
         ch->perm_int = 100;
         ch->perm_dex = 100;
         ch->perm_con = 100;
         ch->perm_per = 100;
         ch->perm_lck = 100;
         sprintf( log_buf, "New player %s from %s.", ch->name, d->host );
         log_string_plus( log_buf, LOG_COMM, 0 );
         to_channel( log_buf, "Newbie", PERMIT_HELP );

         STRFREE( ch->pcdata->channels );
         ch->pcdata->channels = STRALLOC( "newbie" );
         ch->gold = 2000;  /* a little cash to help them get started -keo */

         ch->pcdata->memorize[0] = ROOM_VNUM_NEXUS;
         set_title( ch, "" );

         xSET_BIT( ch->act, PLR_AUTOGOLD );
         xSET_BIT( ch->act, PLR_AUTOEXIT );
         xSET_BIT( ch->act, PLR_LONG_EXIT );
         SET_BIT( ch->pcdata->flags, PCFLAG_SPAR );

         /*
          * Display_prompt interprets blank as default 
          */
         ch->pcdata->prompt = STRALLOC( "" );

         ch->position = POS_STANDING;
         if( d->connected == CON_GET_WANT_RIPANSI )
            d->connected = CON_PLAYING;
         SET_BIT( ch->mood, MOOD_RELAXED );
         do_channels( ch, "join newbie" );
         break;

      case CON_PRESS_ENTER:
         /*
          * fixes 2nd , 3rd pager pages from not showing up - shogar 
          */
         set_pager_color( AT_GREY, ch );
         set_char_color( AT_GREY, ch );

         if( xIS_SET( ch->act, PLR_ANSI ) )
            send_to_pager( "\033[2J", ch );
         else
            send_to_pager( "\014", ch );
         do_new_help( ch, "motd" );
         send_to_pager( "\n\rPress [ENTER] ", ch );
         d->connected = CON_READ_MOTD;

      case CON_READ_MOTD:
      {
         char motdbuf[MAX_STRING_LENGTH];

         sprintf( motdbuf, "\n\rWelcome to %s...\n\r", sysdata.mud_name );
         write_to_buffer( d, motdbuf, 0 );
      }
         add_char( ch );
         d->connected = CON_PLAYING;

         if( ch->pcdata->release_date > 0 && ch->pcdata->release_date > current_time )
         {
            if( ch->in_room->vnum == 6 )
               char_to_room( ch, ch->in_room );
            else
               char_to_room( ch, get_room_index( ROOM_VNUM_NEXUS ) );
         }
         else if( ch->in_room->vnum == ROOM_VNUM_LIMBO )
         {
            char_to_room( ch, get_room_index( ROOM_VNUM_NEXUS ) );
         }
         else if( ch->in_room )
         {
            char_to_room( ch, ch->in_room );
         }
         else
         {
            char_to_room( ch, get_room_index( ROOM_VNUM_NEXUS ) );
         }

         if( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
            remove_timer( ch, TIMER_SHOVEDRAG );

         ch->pcdata->outputprefix = NULL;
         ch->pcdata->outputsuffix = NULL;
         ch->pcdata->alias_queue = NULL;
         if( !ch->species )
         {
            bug( "No species? What the hell?", 0 );
            send_to_char( "&RNo species? What the hell?\n\r", ch );
            send_to_char( "Ask an imm to fix you.\n\r", ch );
            return;
         }

         act( AT_ACTION, "$n has awakened again into the world.", ch, NULL, NULL, TO_ROOM );

         send_to_char( "Your body is reshaped before your eyes...\n\r", ch );

		 check_loginmsg(ch);
		 
         /*
          * I hate to put this shit here, but... -keo 
          */
         update_aris( ch );
         if( !ch->speed )
            ch->speed = 100;

         sprintf( buf, "%s has entered the game in %s (%d).", ch->name, ch->in_room->name, ch->in_room->vnum );
         log_string_plus( buf, LOG_COMM, 0 );
         SET_BIT( ch->mood, MOOD_RELAXED );
         do_channels( ch, "rejoin" );
         fix_char( ch );

         if( ch->pcdata->pet )
         {
            act( AT_ACTION, "$n returns to $s master from the Void.", ch->pcdata->pet, NULL, ch, TO_NOTVICT );
            act( AT_ACTION, "$N returns with you to the planes.", ch, NULL, ch->pcdata->pet, TO_CHAR );
         }
         do_look( ch, "auto" );

         if( !ch->was_in_room && ch->in_room == get_room_index( ROOM_VNUM_NEXUS ) )
            ch->was_in_room = get_room_index( ROOM_VNUM_NEXUS );
         else if( ch->was_in_room == get_room_index( ROOM_VNUM_NEXUS ) )
            ch->was_in_room = get_room_index( ROOM_VNUM_NEXUS );
         else if( !ch->was_in_room )
            ch->was_in_room = ch->in_room;

         break;

   }

   return;
}

bool is_reserved_name( char *name )
{
   RESERVE_DATA *res;

   for( res = first_reserved; res; res = res->next )
      if( ( *res->name == '*' && !str_infix( res->name + 1, name ) ) || !str_cmp( res->name, name ) )
         return TRUE;
   return FALSE;
}


/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name, bool newchar )
{
   /*
    * Names checking should really only be done on new characters, otherwise
    * we could end up with people who can't access their characters.  Would
    * have also provided for that new area havoc mentioned below, while still
    * disallowing current area mobnames.  I personally think that if we can
    * have more than one mob with the same keyword, then may as well have
    * players too though, so I don't mind that removal.  -- Alty
    */

   if( is_reserved_name( name ) && newchar )
      return FALSE;

   /*
    * Length restrictions.
    */
   if( strlen( name ) < 3 )
      return FALSE;

   if( strlen( name ) > 12 )
      return FALSE;

   /*
    * Alphanumerics only.
    * Lock out IllIll twits.
    */
   {
      char *pc;
      bool fIll;

      fIll = TRUE;
      for( pc = name; *pc != '\0'; pc++ )
      {
         if( !isalpha( (int)*pc ) && pc[0] != '-' )
            return FALSE;
         if( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
            fIll = FALSE;
      }

      if( fIll )
         return FALSE;
   }
   return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA * d, char *name, bool fConn )
{
   CHAR_DATA *ch;

   for( ch = first_char; ch; ch = ch->next )
   {
      if( !IS_NPC( ch ) && ( !fConn || !ch->desc ) && ch->pcdata->filename && !str_cmp( name, ch->pcdata->filename ) )
      {
         if( fConn && ch->switched )
         {
            write_to_buffer( d, "Already playing.\n\rName: ", 0 );
            d->connected = CON_GET_NAME;
            if( d->character )
            {
               /*
                * clear descriptor pointer to get rid of bug message in log 
                */
               d->character->desc = NULL;
               free_char( d->character );
               d->character = NULL;
            }
            return BERR;
         }
         if( fConn == FALSE )
         {
            DISPOSE( d->character->pcdata->pwd );
            d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
         }
         else
         {
            /*
             * clear descriptor pointer to get rid of bug message in log 
             */
            d->character->desc = NULL;
            free_char( d->character );
            d->character = ch;
            ch->desc = d;
            ch->timer = 0;
            send_to_char( "Reconnecting.\n\r", ch );
            do_look( ch, "auto" );
            act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_CANSEE );
            sprintf( log_buf, "%s@%s(%s) reconnected.", ch->pcdata->filename, d->host, d->user );
            log_string_plus( log_buf, LOG_COMM, 0 );
            d->connected = CON_PLAYING;
            do_channels( d->character, "rejoin" );
         }
         return TRUE;
      }
   }

   return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA * d, char *name, bool kick )
{
   CHAR_DATA *ch;

   DESCRIPTOR_DATA *dold;
   int cstate;

   for( dold = first_descriptor; dold; dold = dold->next )
   {
      if( dold != d
          && ( dold->character || dold->original )
          && !str_cmp( name, dold->original ? dold->original->pcdata->filename : dold->character->pcdata->filename ) )
      {
         cstate = dold->connected;
         ch = dold->original ? dold->original : dold->character;
         if( !ch->name || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
         {
            write_to_buffer( d, "Already connected - try again.\n\r", 0 );
            sprintf( log_buf, "%s already connected.", ch->pcdata->filename );
            log_string_plus( log_buf, LOG_COMM, 0 );
            return BERR;
         }
         if( !kick )
            return TRUE;
         write_to_buffer( d, "Already playing... Kicking off old connection.\n\r", 0 );
         write_to_buffer( dold, "Kicking off old connection... bye!\n\r", 0 );
         close_socket( dold, FALSE );
         /*
          * clear descriptor pointer to get rid of bug message in log 
          */
         d->character->desc = NULL;
         free_char( d->character );
         d->character = ch;
         ch->desc = d;
         ch->timer = 0;
         if( ch->switched )
            do_return( ch->switched, "" );
         ch->switched = NULL;
         send_to_char( "Reconnecting.\n\r", ch );
         do_look( ch, "auto" );
         act( AT_ACTION, "$n has reconnected, kicking off old link.", ch, NULL, NULL, TO_CANSEE );
         sprintf( log_buf, "%s@%s reconnected, kicking off old link.", ch->pcdata->filename, d->host );
         log_string_plus( log_buf, LOG_COMM, 0 );
         d->connected = cstate;
         do_channels( d->character, "rejoin" );
         return TRUE;
      }
   }

   return FALSE;
}



void stop_idling( CHAR_DATA * ch )
{
   ROOM_INDEX_DATA *was_in_room;
   if( !ch
       || !ch->desc
       || ch->desc->connected != CON_PLAYING || !ch->was_in_room || ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
      return;

   ch->timer = 0;
   was_in_room = ch->was_in_room;
   char_from_room( ch );
   char_to_room( ch, was_in_room );
   /*
    * ch->was_in_room  = NULL;
    */
   act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
   return;
}

/*  From Erwin  */

void log_printf( char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   log_string( buf );
}

/*
 * Write to one char.
 */
/* void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( !ch )
    {
      bug( "Send_to_char: NULL *ch" );
      return;
    }
    if ( txt && ch->desc )
	write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
} */

/*
 * Same as above, but converts &color codes to ANSI sequences..
 */
void send_to_char_color( const char *txt, CHAR_DATA * ch )
{
   DESCRIPTOR_DATA *d;
   char *colstr;
   const char *prevstr = txt;
   char colbuf[20];
   int ln;

   if( !ch )
   {
      bug( "Send_to_char_color: NULL *ch" );
      return;
   }
   if( !txt || !ch->desc )
      return;
   d = ch->desc;
   /*
    * Clear out old color stuff 
    */
/*  make_color_sequence(NULL, NULL, NULL);*/
   while( ( colstr = strpbrk( prevstr, "&^" ) ) != NULL )
   {
      if( colstr > prevstr )
         write_to_buffer( d, prevstr, ( colstr - prevstr ) );
      ln = make_color_sequence( colstr, colbuf, d );
      if( ln < 0 )
      {
         prevstr = colstr + 1;
         break;
      }
      else if( ln > 0 )
         write_to_buffer( d, colbuf, ln );
      prevstr = colstr + 2;
   }
   if( *prevstr )
      write_to_buffer( d, prevstr, 0 );
   return;
}

void write_to_pager( DESCRIPTOR_DATA * d, const char *txt, int length )
{
   int pageroffset;  /* Pager fix by thoric */

   if( length <= 0 )
      length = strlen( txt );
   if( length == 0 )
      return;
   if( !d->pagebuf )
   {
      d->pagesize = MAX_STRING_LENGTH;
      CREATE( d->pagebuf, char, d->pagesize );
   }
   if( !d->pagepoint )
   {
      d->pagepoint = d->pagebuf;
      d->pagetop = 0;
      d->pagecmd = '\0';
   }
   if( d->pagetop == 0 && !d->fcommand )
   {
      d->pagebuf[0] = '\n';
      d->pagebuf[1] = '\r';
      d->pagetop = 2;
   }
   pageroffset = d->pagepoint - d->pagebuf;  /* pager fix (goofup fixed 08/21/97) */
   while( d->pagetop + length >= d->pagesize )
   {
      if( d->pagesize > 32000 )
      {
         bug( "Pager overflow.  Ignoring.\n\r" );
         d->pagetop = 0;
         d->pagepoint = NULL;
         DISPOSE( d->pagebuf );
         d->pagesize = MAX_STRING_LENGTH;
         return;
      }
      d->pagesize *= 2;
      RECREATE( d->pagebuf, char, d->pagesize );
   }
   d->pagepoint = d->pagebuf + pageroffset;  /* pager fix (goofup fixed 08/21/97) */
   strncpy( d->pagebuf + d->pagetop, txt, length );
   d->pagetop += length;
   d->pagebuf[d->pagetop] = '\0';
   return;
}

/* void send_to_pager( const char *txt, CHAR_DATA *ch )
{
  if ( !ch )
  {
    bug( "Send_to_pager: NULL *ch" );
    return;
  }
  if ( txt && ch->desc )
  {
    DESCRIPTOR_DATA *d = ch->desc;
    
    ch = d->original ? d->original : d->character;
    if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
    {
	send_to_char(txt, d->character);
	return;
    }
    write_to_pager(d, txt, 0);
  }
  return;
} */

void send_to_pager_color( const char *txt, CHAR_DATA * ch )
{
   DESCRIPTOR_DATA *d;
   char *colstr;
   const char *prevstr = txt;
   char colbuf[20];
   int ln;

   if( !ch )
   {
      bug( "Send_to_pager_color: NULL *ch" );
      return;
   }
   if( !txt || !ch->desc )
      return;
   d = ch->desc;
   ch = d->original ? d->original : d->character;
   if( IS_NPC( ch ) || !IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
   {
      send_to_char_color( txt, d->character );
      return;
   }
   /*
    * Clear out old color stuff 
    */
/*  make_color_sequence(NULL, NULL, NULL);*/
   while( ( colstr = strpbrk( prevstr, "&^" ) ) != NULL )
   {
      if( colstr > prevstr )
         write_to_pager( d, prevstr, ( colstr - prevstr ) );
      ln = make_color_sequence( colstr, colbuf, d );
      if( ln < 0 )
      {
         prevstr = colstr + 1;
         break;
      }
      else if( ln > 0 )
         write_to_pager( d, colbuf, ln );
      prevstr = colstr + 2;
   }
   if( *prevstr )
      write_to_pager( d, prevstr, 0 );
   return;
}

void set_char_color( sh_int AType, CHAR_DATA * ch )
{
   char buf[16];
   CHAR_DATA *och;

   if( !ch || !ch->desc )
      return;

   och = ( ch->desc->original ? ch->desc->original : ch );
   if( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) )
   {
      if( AType == 7 )
         strcpy( buf, "\033[m" );
      else
         sprintf( buf, "\033[0;%d;%s%dm", ( AType & 8 ) == 8, ( AType > 15 ? "5;" : "" ), ( AType & 7 ) + 30 );
      ch->desc->prevcolor = AType;  /* added this, was in 1.02 */
      ch->desc->pagecolor = AType;  /* cooridnate page & nopage shogar */
      write_to_buffer( ch->desc, buf, strlen( buf ) );
   }
   return;
}

void set_pager_color( sh_int AType, CHAR_DATA * ch )
{
   char buf[16];
   CHAR_DATA *och;

   if( !ch || !ch->desc )
      return;

   och = ( ch->desc->original ? ch->desc->original : ch );
   if( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) )
   {
      if( AType == 7 )
         strcpy( buf, "\033[m" );
      else
         sprintf( buf, "\033[0;%d;%s%dm", ( AType & 8 ) == 8, ( AType > 15 ? "5;" : "" ), ( AType & 7 ) + 30 );
      ch->desc->pagecolor = AType;
      ch->desc->prevcolor = AType;  /* cooridnate page & nopage shogar */
      send_to_pager( buf, ch );
   }
   return;
}


/* source: EOD, by John Booth <???> */
void ch_printf( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2]; /* better safe than sorry */
   va_list args;

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   send_to_char( buf, ch );
}

void pager_printf( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   send_to_pager( buf, ch );
}


/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.		-Thoric
 */
char *myobj( OBJ_DATA * obj )
{
   if( !str_prefix( "a ", obj->short_descr ) )
      return obj->short_descr + 2;
   if( !str_prefix( "an ", obj->short_descr ) )
      return obj->short_descr + 3;
   if( !str_prefix( "the ", obj->short_descr ) )
      return obj->short_descr + 4;
   if( !str_prefix( "some ", obj->short_descr ) )
      return obj->short_descr + 5;
   return obj->short_descr;
}

/* Same as myobj, but for mobs. -keo */
char *themob( CHAR_DATA * mob )
{
   if( !str_prefix( "a ", mob->short_descr ) )
      return mob->short_descr + 2;
   if( !str_prefix( "an ", mob->short_descr ) )
      return mob->short_descr + 3;
   if( !str_prefix( "the ", mob->short_descr ) )
      return mob->short_descr + 4;
   if( !str_prefix( "some ", mob->short_descr ) )
      return mob->short_descr + 5;
   return mob->short_descr;
}

char *obj_short( OBJ_DATA * obj )
{
   static char buf[MAX_STRING_LENGTH];

   if( obj->count > 1 )
   {
      sprintf( buf, "%s (%d)", obj->short_descr, obj->count );
      return buf;
   }
   return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */

void ch_printf_color( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   send_to_char_color( buf, ch );
}

void pager_printf_color( CHAR_DATA * ch, char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH * 2];
   va_list args;

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   send_to_pager_color( buf, ch );
}

#define NAME(ch)        (IS_NPC(ch) ? aoran(themob(ch)) : \
ch->pcdata->name_disguise ? ch->pcdata->name_disguise : ch->name)

char *act_string( const char *format, CHAR_DATA * to, CHAR_DATA * ch, const void *arg1, const void *arg2, int flags )
{
   static char *const he_she[] = { "it", "he", "she", "shi" };
   static char *const him_her[] = { "it", "him", "her", "hir" };
   static char *const his_her[] = { "its", "his", "her", "hir" };
   static char buf[MAX_STRING_LENGTH];
   char fname[MAX_INPUT_LENGTH];
   char *point = buf;
   const char *str = format;
   const char *i;
   CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
   OBJ_DATA *obj1 = ( OBJ_DATA * ) arg1;
   OBJ_DATA *obj2 = ( OBJ_DATA * ) arg2;

   while( str && *str != '\0' )
   {
      if( *str != '$' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      if( !arg2 && *str >= 'A' && *str <= 'Z' )
      {
         bug( "Act: missing arg2 for code %c:", *str );
         bug( format );
         i = " <@@@@@@> ";
      }
      else
      {
         switch ( *str )
         {
            default:
               i = str;
               break;
#ifdef I3
            case '$':
               i = "$";
               break;
#endif
            case 't':
               i = ( char * )arg1;
               break;
            case 'T':
               i = ( char * )arg2;
               break;
            case 'n':
               i = ( to ? PERS( ch, to ) : NAME( ch ) );
               break;
            case 'N':
               i = ( to ? PERS( vch, to ) : NAME( vch ) );
               break;

            case 'e':
               if( ch->sex > 3 || ch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
                  i = "it";
               }
               else
                  i = he_she[URANGE( 0, ch->sex, 3 )];
               break;
            case 'E':
               if( vch->sex > 3 || vch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
                  i = "it";
               }
               else
                  i = he_she[URANGE( 0, vch->sex, 3 )];
               break;
            case 'm':
               if( ch->sex > 3 || ch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
                  i = "it";
               }
               else
                  i = him_her[URANGE( 0, ch->sex, 3 )];
               break;
            case 'M':
               if( vch->sex > 3 || vch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
                  i = "it";
               }
               else
                  i = him_her[URANGE( 0, vch->sex, 3 )];
               break;
            case 's':
               if( ch->sex > 3 || ch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
                  i = "its";
               }
               else
                  i = his_her[URANGE( 0, ch->sex, 3 )];
               break;
            case 'S':
               if( vch->sex > 3 || vch->sex < 0 )
               {
                  bug( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
                  i = "its";
               }
               else
                  i = his_her[URANGE( 0, vch->sex, 3 )];
               break;
            case 'q':
               i = ( to == ch ) ? "" : "s";
               break;
            case 'Q':
               i = ( to == ch ) ? "your" : his_her[URANGE( 0, ch->sex, 2 )];
               break;
            case 'p':
               i = ( !to || can_see_obj( to, obj1 ) ? aoran( myobj( obj1 ) ) : "something" );
               break;
            case 'P':
               i = ( !to || can_see_obj( to, obj2 ) ? aoran( myobj( obj2 ) ) : "something" );
               break;
            case 'd':
               if( !arg2 || ( ( char * )arg2 )[0] == '\0' )
                  i = "door";
               else
               {
                  one_argument( ( char * )arg2, fname );
                  i = fname;
               }
               break;
         }
      }
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;

   }
   strcpy( point, "\n\r" );
   buf[0] = UPPER( buf[0] );
   return buf;
}

#undef NAME

void act( sh_int AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type )
{
   char *txt;
   CHAR_DATA *to;
   CHAR_DATA *vch = ( CHAR_DATA * ) arg2;

   /*
    * Discard null and zero-length messages.
    */
   if( !format || format[0] == '\0' )
      return;

   if( !ch )
   {
      bug( "Act: null ch. (%s)", format );
      return;
   }

   if( !ch->in_room )
      to = NULL;
   else if( type == TO_CHAR )
      to = ch;
   else
      to = ch->in_room->first_person;

   /*
    * ACT_SECRETIVE handling
    */
   if( IS_NPC( ch ) && xIS_SET( ch->act, ACT_SECRETIVE ) && type != TO_CHAR )
      return;

   if( type == TO_VICT )
   {
      if( !vch )
      {
         bug( "Act: null vch with TO_VICT." );
         bug( "%s (%s)", ch->name, format );
         return;
      }
      if( !vch->in_room )
      {
         bug( "Act: vch in NULL room!" );
         bug( "%s -> %s (%s)", ch->name, vch->name, format );
         return;
      }
      to = vch;
   }

   if( to && !to->in_room )
   {
      bug( "Act: to in NULL room!" );
      bug( "%s -> %s (%s)", ch->name, to->name, format );
      return;
   }

   if( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
   {
      OBJ_DATA *to_obj;

      txt = act_string( format, NULL, ch, arg1, arg2, STRING_NONE );
      if( HAS_PROG( to->in_room, ACT_PROG ) )
         rprog_act_trigger( txt, to->in_room, ch, ( OBJ_DATA * ) arg1, ( void * )arg2 );
      for( to_obj = to->in_room->first_content; to_obj; to_obj = to_obj->next_content )
         if( HAS_PROG( to_obj->pIndexData, ACT_PROG ) )
            oprog_act_trigger( txt, to_obj, ch, ( OBJ_DATA * ) arg1, ( void * )arg2 );
   }

   /*
    * Anyone feel like telling me the point of looping through the whole
    * room when we're only sending to one char anyways..? -- Alty 
    */
   for( ; to; to = ( type == TO_CHAR || type == TO_VICT ) ? NULL : to->next_in_room )
   {
      if( ( !to->desc
            && ( IS_NPC( to ) && !HAS_PROG( to->pIndexData, ACT_PROG ) ) )
          || !IS_AWAKE( to ) || ( !CAN_SEE_PLANE( to, ch ) && ( to->curr_talent[TAL_SEEKING] < 100 ) ) )
         continue;

      if( type == TO_CHAR && to != ch )
         continue;
      if( type == TO_VICT && ( to != vch || to == ch ) )
         continue;
      if( type == TO_ROOM && to == ch )
         continue;
      if( type == TO_NOTVICT && ( to == ch || to == vch ) )
         continue;
      if( type == TO_CANSEE && ( to == ch || can_see( to, ch ) ) )
	    continue;
	  if ( to->desc && is_inolc(to->desc) )
         continue;

      txt = act_string( format, to, ch, arg1, arg2, STRING_NONE );

      if( to->desc )
      {
         set_char_color( AType, to );
         send_to_char_color( txt, to );
      }
      if( MOBtrigger )
      {
         /*
          * Note: use original string, not string with ANSI. -- Alty 
          */
         mprog_act_trigger( txt, to, ch, ( OBJ_DATA * ) arg1, ( void * )arg2 );
      }
   }
   MOBtrigger = TRUE;
   return;
}


char *default_fprompt( CHAR_DATA * ch )
{
   static char buf[60];

   strcpy( buf, "&w< &G%h/%Hhp " );
   strcat( buf, "&P%m/%Mm " );
   strcat( buf, "&C%v/%Vep " );
   strcat( buf, "&W| Enemy: &R%c &W> " );
   return buf;
}

char *default_prompt( CHAR_DATA * ch )
{
   static char buf[60];
	
   strcpy( buf, "&W< &G%h/%Hhp " );
   strcat( buf, "&P%m/%Mm " );
   strcat( buf, "&C%v/%Vep " );
   	if ( IS_SET( ch->pcdata->flags, PCFLAG_BUILDWALK ) )
	{
	strcat( buf, "&RBUILDWALKING" );
	}
	strcat( buf, "&W>");
   return buf;
}

int getcolor( char clr )
{
   static const char colors[16] = "xrgObpcwzRGYBPCW";
   int r;

   for( r = 0; r < 16; r++ )
      if( clr == colors[r] )
         return r;
   return -1;
}

void display_prompt( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch = d->character;
   CHAR_DATA *och = ( d->original ? d->original : d->character );
   CHAR_DATA *victim;
   bool ansi = ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) );
   const char *prompt;
   const char *helpstart = "<Type HELP NEWBIE>";
   char buf[MAX_STRING_LENGTH];
   char *pbuf = buf;
   int stat, percent;
   char *strtime;

   if( !ch )
   {
      bug( "display_prompt: NULL ch" );
      return;
   }

   if( !IS_NPC( ch ) && !IS_SET( ch->pcdata->flags, PCFLAG_HELPSTART ) )
      prompt = helpstart;
   else if( !IS_NPC( ch ) && ch->substate != SUB_NONE && ch->pcdata->subprompt && ch->pcdata->subprompt[0] != '\0' )
      prompt = ch->pcdata->subprompt;
   else if( IS_NPC( ch ) || ( !IS_FIGHTING( ch ) && ( !ch->pcdata->prompt || !*ch->pcdata->prompt ) ) )
      prompt = default_prompt( ch );
   else if( IS_FIGHTING( ch ) )
   {
      if( !ch->pcdata->fprompt || !*ch->pcdata->fprompt )
         prompt = default_fprompt( ch );
      else
         prompt = ch->pcdata->fprompt;
   }
   else
      prompt = ch->pcdata->prompt;
   if( ansi )
   {
      strcpy( pbuf, "\033[m" );
      d->prevcolor = 0x07;
      pbuf += 3;
   }
   /*
    * Clear out old color stuff 
    */
/*  make_color_sequence(NULL, NULL, NULL);*/
   for( ; *prompt; prompt++ )
   {
      /*
       * '&' = foreground color/intensity bit
       * '^' = background color/blink bit
       * '%' = prompt commands
       * Note: foreground changes will revert background to 0 (black)
       */
      if( *prompt != '&' && *prompt != '^' && *prompt != '%' )
      {
         *( pbuf++ ) = *prompt;
         continue;
      }
      ++prompt;
      if( !*prompt )
         break;
      if( *prompt == *( prompt - 1 ) )
      {
         *( pbuf++ ) = *prompt;
         continue;
      }
      switch ( *( prompt - 1 ) )
      {
         default:
            bug( "Display_prompt: bad command char '%c'.", *( prompt - 1 ) );
            break;
         case '&':
         case '^':
            stat = make_color_sequence( &prompt[-1], pbuf, d );
            if( stat < 0 )
               --prompt;
            else if( stat > 0 )
               pbuf += stat;
            break;
         case '%':
            *pbuf = '\0';
            stat = 0x80000000;
            switch ( *prompt )
            {
               case '%':
                  *pbuf++ = '%';
                  *pbuf = '\0';
                  break;
               case 'A':
                  sprintf( pbuf, "%s%s%s", IS_AFFECTED( ch, AFF_INVISIBLE ) ? "I" : "",
                           IS_AFFECTED( ch, AFF_HIDE ) ? "H" : "", IS_AFFECTED( ch, AFF_SNEAK ) ? "S" : "" );
                  break;
               case 'C':  /* Tank */
                  if( !IS_FIGHTING( ch ) || ( victim = ch->last_hit ) == NULL )
                     strcpy( pbuf, "N/A" );
                  else if( !IS_FIGHTING( victim ) || ( victim = victim->last_hit ) == NULL )
                     strcpy( pbuf, "N/A" );
                  else
                  {
                     if( victim->max_hit > 0 )
                        percent = ( 100 * victim->hit ) / victim->max_hit;
                     else
                        percent = -1;
                     if( percent >= 100 )
                        strcpy( pbuf, "perfect health" );
                     else if( percent >= 90 )
                        strcpy( pbuf, "slightly scratched" );
                     else if( percent >= 80 )
                        strcpy( pbuf, "few bruises" );
                     else if( percent >= 70 )
                        strcpy( pbuf, "some cuts" );
                     else if( percent >= 60 )
                        strcpy( pbuf, "several wounds" );
                     else if( percent >= 50 )
                        strcpy( pbuf, "nasty wounds" );
                     else if( percent >= 40 )
                        strcpy( pbuf, "bleeding freely" );
                     else if( percent >= 30 )
                        strcpy( pbuf, "covered in blood" );
                     else if( percent >= 20 )
                        strcpy( pbuf, "leaking guts" );
                     else if( percent >= 10 )
                        strcpy( pbuf, "almost dead" );
                     else
                        strcpy( pbuf, "DYING" );
                  }
                  break;
               case 'c':

                  if( !IS_FIGHTING( ch ) || ( victim = ch->last_hit ) == NULL )
                     strcpy( pbuf, "N/A" );
                  else
                  {
                     if( victim->max_hit > 0 )
                        percent = ( 100 * victim->hit ) / victim->max_hit;
                     else
                        percent = -1;
                     if( percent >= 100 )
                        strcpy( pbuf, "perfect health" );
                     else if( percent >= 90 )
                        strcpy( pbuf, "slightly scratched" );
                     else if( percent >= 80 )
                        strcpy( pbuf, "few bruises" );
                     else if( percent >= 70 )
                        strcpy( pbuf, "some cuts" );
                     else if( percent >= 60 )
                        strcpy( pbuf, "several wounds" );
                     else if( percent >= 50 )
                        strcpy( pbuf, "nasty wounds" );
                     else if( percent >= 40 )
                        strcpy( pbuf, "bleeding freely" );
                     else if( percent >= 30 )
                        strcpy( pbuf, "covered in blood" );
                     else if( percent >= 20 )
                        strcpy( pbuf, "leaking guts" );
                     else if( percent >= 10 )
                        strcpy( pbuf, "almost dead" );
                     else
                        strcpy( pbuf, "DYING" );
                  }
                  break;
               case 'h':
                  stat = ch->hit;
                  break;
               case 'H':
                  stat = ch->max_hit;
                  break;
               case 'm':
                  stat = ch->mana;
                  break;
               case 'M':
                  stat = ch->max_mana;
                  break;
               case 'N':  /* Tank */
                  if( !IS_FIGHTING( ch ) || ( victim = ch->last_hit ) == NULL )
                     strcpy( pbuf, "N/A" );
                  else if( !IS_FIGHTING( victim ) || ( victim = victim->last_hit ) == NULL )
                     strcpy( pbuf, "N/A" );
                  else
                  {
                     if( ch == victim )
                        strcpy( pbuf, "You" );
                     else if( IS_NPC( victim ) )
                        strcpy( pbuf, victim->short_descr );
                     else
                        strcpy( pbuf, victim->name );
                     pbuf[0] = UPPER( pbuf[0] );
                  }
                  break;
               case 'n':
                  if( !IS_FIGHTING( ch ) || ( victim = ch->last_hit ) == NULL )
                     strcpy( pbuf, "N/A" );
                  else
                  {
                     if( ch == victim )
                        strcpy( pbuf, "You" );
                     else if( IS_NPC( victim ) )
                        strcpy( pbuf, victim->short_descr );
                     else
                        strcpy( pbuf, victim->name );
                     pbuf[0] = UPPER( pbuf[0] );
                  }
                  break;
               case 't':
                  if( time_info.hour < 5 )
                     strcpy( pbuf, "night" );
                  else if( time_info.hour < 6 )
                     strcpy( pbuf, "dawn" );
                  else if( time_info.hour < 12 )
                     strcpy( pbuf, "morning" );
                  else if( time_info.hour < 13 )
                     strcpy( pbuf, "noon" );
                  else if( time_info.hour < 16 )
                     strcpy( pbuf, "afternoon" );
                  else if( time_info.hour < 20 )
                     strcpy( pbuf, "evening" );
                  else
                     strcpy( pbuf, "night" );
                  break;
               case 'T':
                  strtime = ctime( &current_time );
                  strtime[strlen( strtime ) - 1] = '\0';
                  strcpy( pbuf, strtime );
                  break;
               case 'b':
                  stat = 0;
                  break;
               case 'u':
                  stat = num_descriptors;
                  break;
               case 'U':
                  stat = sysdata.maxplayers;
                  break;
               case 'v':
                  stat = ch->move;
                  break;
               case 'V':
                  stat = ch->max_move;
                  break;
               case 'g':
                  stat = ch->gold;
                  break;
               case 'r':
                  sprintf( pbuf, IS_SET( ch->mood, MOOD_READY ) ? "ready" : "relaxed" );
                  break;
               case 'F':
                  if( IS_SET( ch->pcdata->permissions, PERMIT_BUILD ) )
                     sprintf( pbuf, "%s", flag_string( ch->in_room->room_flags, r_flags ) );
                  break;
               case 'R':
                  if( xIS_SET( och->act, PLR_ROOMVNUM ) )
                     sprintf( pbuf, "<#%d> ", ch->in_room->vnum );
                  else
                     snprintf( pbuf, sizeof( pbuf ), "%s", ch->nation->name );
                  break;
               case 'i':
                  if( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_WIZINVIS ) ) ||
                      ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_MOBINVIS ) ) )
                     sprintf( pbuf, "(Invis %d) ", ( IS_NPC( ch ) ? ch->mobinvis : ch->pcdata->wizinvis ) );
                  else if( IS_AFFECTED( ch, AFF_INVISIBLE ) )
                     sprintf( pbuf, "(Invis) " );
                  break;
               case 'I':
                  stat = ( IS_NPC( ch ) ? ( xIS_SET( ch->act, ACT_MOBINVIS ) ? ch->mobinvis : 0 )
                           : ( xIS_SET( ch->act, PLR_WIZINVIS ) ? ch->pcdata->wizinvis : 0 ) );
                  break;
            }
            if( stat != 0x80000000 )
               sprintf( pbuf, "%d", stat );
            pbuf += strlen( pbuf );
            break;
      }
   }
   *pbuf = '\0';
   write_to_buffer( d, buf, ( pbuf - buf ) );
   return;
}

int make_color_sequence( const char *col, char *buf, DESCRIPTOR_DATA * d )
{
   int ln;
   const char *ctype = col;
   unsigned char cl;
   CHAR_DATA *och;
   bool ansi;

   och = ( d->original ? d->original : d->character );
   ansi = ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) );
   col++;
   if( !*col )
      ln = -1;
   else if( *ctype != '&' && *ctype != '^' )
   {
      bug( "Make_color_sequence: command '%c' not '&' or '^'.", *ctype );
      ln = -1;
   }
   else if( *col == *ctype )
   {
      buf[0] = *col;
      buf[1] = '\0';
      ln = 1;
   }
   else if( !ansi )
      ln = 0;
   else
   {
      cl = d->prevcolor;
      switch ( *ctype )
      {
         default:
            bug( "Make_color_sequence: bad command char '%c'.", *ctype );
            ln = -1;
            break;
         case '&':
            if( *col == '-' )
            {
               buf[0] = '~';
               buf[1] = '\0';
               ln = 1;
               break;
            }
         case '^':
         {
            int newcol;

            if( ( newcol = getcolor( *col ) ) < 0 )
            {
               ln = 0;
               break;
            }
            else if( *ctype == '&' )
               cl = ( cl & 0xF0 ) | newcol;
            else
               cl = ( cl & 0x0F ) | ( newcol << 4 );
         }
            if( cl == d->prevcolor )
            {
               ln = 0;
               break;
            }
            strcpy( buf, "\033[" );
            if( ( cl & 0x88 ) != ( d->prevcolor & 0x88 ) )
            {
               strcat( buf, "m\033[" );
               if( ( cl & 0x08 ) )
                  strcat( buf, "1;" );
               if( ( cl & 0x80 ) )
                  strcat( buf, "5;" );
               d->prevcolor = 0x07 | ( cl & 0x88 );
               ln = strlen( buf );
            }
            else
               ln = 2;
            if( ( cl & 0x07 ) != ( d->prevcolor & 0x07 ) )
            {
               sprintf( buf + ln, "3%d;", cl & 0x07 );
               ln += 3;
            }
            if( ( cl & 0x70 ) != ( d->prevcolor & 0x70 ) )
            {
               sprintf( buf + ln, "4%d;", ( cl & 0x70 ) >> 4 );
               ln += 3;
            }
            if( buf[ln - 1] == ';' )
               buf[ln - 1] = 'm';
            else
            {
               buf[ln++] = 'm';
               buf[ln] = '\0';
            }
            d->prevcolor = cl;
      }
   }
   if( ln <= 0 )
      *buf = '\0';
   return ln;
}

void set_pager_input( DESCRIPTOR_DATA * d, char *argument )
{
   while( isspace( (int)*argument ) )
      argument++;
   d->pagecmd = *argument;
   return;
}

bool pager_output( DESCRIPTOR_DATA * d )
{
   register char *last;
   CHAR_DATA *ch;
   int pclines;
   register int lines;
   bool ret;

   if( !d || !d->pagepoint || d->pagecmd == -1 )
      return TRUE;
   ch = d->original ? d->original : d->character;
   pclines = UMAX( ch->pcdata->pagerlen, 5 ) - 1;
   switch ( LOWER( d->pagecmd ) )
   {
      default:
         lines = 0;
         break;
      case 'b':
         lines = -1 - ( pclines * 2 );
         break;
      case 'r':
         lines = -1 - pclines;
         break;
      case 'q':
         d->pagetop = 0;
         d->pagepoint = NULL;
         flush_buffer( d, TRUE );
         DISPOSE( d->pagebuf );
         d->pagesize = MAX_STRING_LENGTH;
         return TRUE;
   }
   while( lines < 0 && d->pagepoint >= d->pagebuf )
      if( *( --d->pagepoint ) == '\n' )
         ++lines;
   if( *d->pagepoint == '\n' && *( ++d->pagepoint ) == '\r' )
      ++d->pagepoint;
   if( d->pagepoint < d->pagebuf )
      d->pagepoint = d->pagebuf;
   for( lines = 0, last = d->pagepoint; lines < pclines; ++last )
      if( !*last )
         break;
      else if( *last == '\n' )
         ++lines;
   if( *last == '\r' )
      ++last;
   if( last != d->pagepoint )
   {
      if( !write_to_descriptor( d->descriptor, d->pagepoint, ( last - d->pagepoint ) ) )
         return FALSE;
      d->pagepoint = last;
   }
   while( isspace( (int)*last ) )
      ++last;
   if( !*last )
   {
      d->pagetop = 0;
      d->pagepoint = NULL;
      flush_buffer( d, TRUE );
      DISPOSE( d->pagebuf );
      d->pagesize = MAX_STRING_LENGTH;
      return TRUE;
   }
   d->pagecmd = -1;
   if( xIS_SET( ch->act, PLR_ANSI ) )
      if( write_to_descriptor( d->descriptor, "\033[1;36m", 7 ) == FALSE )
         return FALSE;
   if( ( ret = write_to_descriptor( d->descriptor, "(C)ontinue, (R)efresh, (B)ack, (Q)uit: [C] ", 0 ) ) == FALSE )
      return FALSE;
   if( xIS_SET( ch->act, PLR_ANSI ) )
   {
      char buf[32];

      if( d->pagecolor == 7 )
         strcpy( buf, "\033[m" );
      else
         sprintf( buf, "\033[0;%d;%s%dm", ( d->pagecolor & 8 ) == 8,
                  ( d->pagecolor > 15 ? "5;" : "" ), ( d->pagecolor & 7 ) + 30 );
      ret = write_to_descriptor( d->descriptor, buf, 0 );
   }
   return ret;
}

/* CrashGuard - A reduced version of Warm reboot - Josh */
void do_crashguard(  )
{
   FILE *fp;
   DESCRIPTOR_DATA *d, *d_next;
   char buf[100], buf2[100], buf3[100], buf4[100];
   fp = fopen( COPYOVER_FILE, "w" );
   if( !fp )
   {
      log_printf( "Could not write to copyover file: %s", COPYOVER_FILE );
      perror( "do_copyover:fopen" );
      return;
   }
   sprintf( buf, "\r\n[INFO] CRASHGUARD - please remain seated!\r\n\a" );
   for( d = first_descriptor; d; d = d_next )
   {
      CHAR_DATA *och = CH( d );
      d_next = d->next;
      if( !d->character || d->connected < 0 )
      {
         write_to_descriptor( d->descriptor, "\n\rSorry, we are rebooting." " Come back in a few minutes.\n\r", 0 );
         close_socket( d, FALSE );
      }
      else
      {
         fprintf( fp, "%d %s %s\n", d->descriptor, och->name, d->host );
         save_char_obj( och );
         write_to_descriptor( d->descriptor, buf, 0 );
      }
   }
   fprintf( fp, "-1\n" );
   fclose( fp );
#ifdef I3
   if( I3_is_connected(  ) )
   {
      I3_savechanlist(  );
      I3_savemudlist(  );
      I3_savehistory(  );
   }
#endif
#ifdef IMC
   imc_hotboot(  );
#endif
   fclose( fpReserve );
   fclose( fpLOG );
   fpReserve = NULL;
   fpLOG = NULL;
   sprintf( buf, "%d", port );
   sprintf( buf2, "%d", control );
#ifdef I3
   sprintf( buf3, "%d", I3_socket );
#else
   strcpy( buf3, "-1" );
#endif
#ifdef IMC
   if( this_imcmud )
      snprintf( buf4, 100, "%d", this_imcmud->desc );
   else
      strncpy( buf4, "-1", 100 );
#else
   strncpy( buf4, "-1", 100 );
#endif

   execl( EXE_FILE, "rmexe", buf, "copyover", buf2, buf3, buf4, ( char * )NULL );
   perror( "do_copyover: execl" );
   if( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
   {
      perror( NULL_FILE );
      exit( 1 );
   }
   if( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
   {
      perror( NULL_FILE );
      exit( 1 );
   }

}

/* End of CrashGuard - Josh */

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */

void do_copyover( CHAR_DATA * ch, char *argument )
{
   FILE *fp;
   DESCRIPTOR_DATA *d, *d_next;
   char buf[100], buf2[100], buf3[100], buf4[100];
   CHANNEL_DATA *chan;
   char buf1[MAX_STRING_LENGTH];

   fp = fopen( COPYOVER_FILE, "w" );

   if( !fp )
   {
      send_to_char( "Copyover file not writeable, aborted.\n\r", ch );
      log_printf( "Could not write to copyover file: %s", COPYOVER_FILE );
      perror( "do_copyover:fopen" );
      return;
   }

   /*
    * Consider changing all saved areas here, if you use OLC 
    */

   do_saveall( ch, "" );

   sprintf( buf, "\r\nCopyover by %s - the game will resume shortly.\r\n\a", ch->name );
   /*
    * For each playing descriptor, save its state 
    */
   for( d = first_descriptor; d; d = d_next )
   {
      CHAR_DATA *och = CH( d );
      d_next = d->next; /* We delete from the list , so need to save this */
      if( !d->character || d->connected != 0 )  /* drop those logging on */
      {
         write_to_descriptor( d->descriptor, "\n\rSorry, we are rebooting." " Come back in a few minutes.\n\r", 0 );
         close_socket( d, FALSE );  /* throw'em out */
      }
      else
      {
         fprintf( fp, "%d %s %s\n", d->descriptor, och->name, d->host );
         STRFREE( ch->pcdata->channels );
         ch->pcdata->channels = STRALLOC( "" );
         for( chan = first_channel; chan; chan = chan->next )
         {
            if( get_chatter( ch, chan ) )
            {
               sprintf( buf1, "%s %s", ch->pcdata->channels, chan->name );
               STRFREE( ch->pcdata->channels );
               ch->pcdata->channels = STRALLOC( buf1 );
            }
         }
         /*
          * Problems with affects from wielded weapons 
          */
         if( ch->main_hand )
            obj_unaffect_ch( ch, ch->main_hand );
         if( ch->off_hand )
            obj_unaffect_ch( ch, ch->off_hand );
         save_char_obj( och );
         write_to_descriptor( d->descriptor, buf, 0 );
      }
   }
   fprintf( fp, "-1\n" );
   fclose( fp );

#ifdef I3
   if( I3_is_connected(  ) )
   {
      I3_savechanlist(  );
      I3_savemudlist(  );
      I3_savehistory(  );
   }
#endif
#ifdef IMC
   imc_hotboot(  );
#endif
   /*
    * Close reserve and other always-open files and release other resources 
    */
   fclose( fpReserve );
   fclose( fpLOG );
/* more redhat fixes - shogar */
   fpReserve = NULL;
   fpLOG = NULL;

   /*
    * exec - descriptors are inherited 
    */

   sprintf( buf, "%d", port );
   sprintf( buf2, "%d", control );
#ifdef I3
   sprintf( buf3, "%d", I3_socket );
#else
   strcpy( buf3, "-1" );
#endif
#ifdef IMC
   if( this_imcmud )
      snprintf( buf4, 100, "%d", this_imcmud->desc );
   else
      strncpy( buf4, "-1", 100 );
#else
   strncpy( buf4, "-1", 100 );
#endif
   save_world( NULL );

   execl( EXE_FILE, "rmexe", buf, "copyover", buf2, buf3, buf4, ( char * )NULL );

   /*
    * Failed - sucessful exec will not return 
    */

   perror( "do_copyover: execl" );
   send_to_char( "Copyover FAILED!\n\r", ch );

   /*
    * Here you might want to reopen fpReserve 
    */
   /*
    * Since I'm a neophyte type guy, I'll assume this is
    * a good idea and cut and paste from main()  
    */

   if( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
   {
      perror( NULL_FILE );
      exit( 3 );
   }
   if( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
   {
      perror( NULL_FILE );
      exit( 3 );
   }

}

/* Recover from a copyover - load players */
void copyover_recover(  )
{
   DESCRIPTOR_DATA *d;
   FILE *fp;
   char name[100];
   char host[MAX_STRING_LENGTH];
   int desc;
   bool fOld;

   log_string( "Copyover recovery initiated" );

   fp = fopen( COPYOVER_FILE, "r" );

   if( !fp )   /* there are some descriptors open which will hang forever then ? */
   {
      perror( "copyover_recover:fopen" );
      log_string( "Copyover file not found. Exitting.\n\r" );
      exit( 1 );
   }

   unlink( COPYOVER_FILE );   /* In case something crashes
                               * - doesn't prevent reading */
   for( ;; )
   {
      fscanf( fp, "%d %s %s\n", &desc, name, host );
      if( desc == -1 )
         break;

      /*
       * Write something, and check if it goes error-free 
       */
      if( !write_to_descriptor( desc, "\n\rSwirling chaos streaks across the world.\n\r", 0 ) )
      {
         close( desc ); /* nope */
         continue;
      }

      CREATE( d, DESCRIPTOR_DATA, 1 );
      init_descriptor( d, desc );   /* set up various stuff */

      d->host = STRALLOC( host );

      LINK( d, first_descriptor, last_descriptor, next, prev );
      d->connected = CON_COPYOVER_RECOVER;   /* negative so close_socket
                                              * will cut them off */

      /*
       * Now, find the pfile 
       */

      fOld = load_char_obj( d, name, FALSE );

      if( !fOld ) /* Player file not found?! */
      {
         write_to_descriptor( desc, "\n\rSomehow, your character was lost in the copyover sorry.\n\r", 0 );
         close_socket( d, FALSE );
      }
      else  /* ok! */
      {
         write_to_descriptor( desc, "\n\rChaos slowly retracts, leaving the world fresh and new.\n\r", 0 );
         /*
          * no longer needed - but save for rainy day, shogar 
          */
         /*
          * write_to_descriptor (desc, "\n\rLoading mass quantities of areas...\n\rPerhaps a good time to go wash the car or rotate the tires\n\r",0);
          */
         /*
          * Just In Case,  Someone said this isn't necassary, but _why_
          * do we want to dump someone in limbo? 
          */
         if( !d->character->in_room )
            d->character->in_room = get_room_index( ROOM_VNUM_NEXUS );

         /*
          * Insert in the char_list 
          */
         LINK( d->character, first_char, last_char, next, prev );

         char_to_room( d->character, d->character->in_room );
         do_look( d->character, "auto noprog" );
         act( AT_ACTION, "$n materializes from the Void!", d->character, NULL, NULL, TO_ROOM );
         d->connected = CON_PLAYING;
         num_descriptors++;
         d->character->nation = find_nation( d->character->species );
         do_channels( d->character, "rejoin" );
      }

   }
   fclose( fp );
}

/* Color align functions by Justice@Aaern */
const char *const_color_align( const char *argument, int size, int align )
{
   int space = ( size - const_color_str_len( argument ) );
   static char buf[MAX_STRING_LENGTH];

   if( align == ALIGN_RIGHT || const_color_str_len( argument ) >= size )
      sprintf( buf, "%*.*s", const_color_strnlen( argument, size ), const_color_strnlen( argument, size ), argument );
   else if( align == ALIGN_CENTER )
      sprintf( buf, "%*s%s%*s", ( space / 2 ), "", argument,
               ( ( space / 2 ) * 2 ) == space ? ( space / 2 ) : ( ( space / 2 ) + 1 ), "" );
   else
      sprintf( buf, "%s%*s", argument, space, "" );

   return buf;
}

char *color_align( char *argument, int size, int align )
{
   int space = ( size - color_str_len( argument ) );
   static char buf[MAX_STRING_LENGTH];

   if( align == ALIGN_RIGHT || color_str_len( argument ) >= size )
      sprintf( buf, "%*.*s", color_strnlen( argument, size ), color_strnlen( argument, size ), argument );
   else if( align == ALIGN_CENTER )
      sprintf( buf, "%*s%s%*s", ( space / 2 ), "", argument,
               ( ( space / 2 ) * 2 ) == space ? ( space / 2 ) : ( ( space / 2 ) + 1 ), "" );
   else if( align == ALIGN_LEFT )
      sprintf( buf, "%s%*s", argument, space, "" );

   return buf;
}

int const_color_str_len( const char *argument )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '^' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }
   }

   return count;
}

int const_color_strnlen( const char *argument, int maxlength )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '^' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }

      if( count >= maxlength )
         break;
   }
   if( count < maxlength )
      return ( ( str - count ) + maxlength );

   str++;
   return str;
}

int color_str_len( char *argument )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '^' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }
   }

   return count;
}

int color_strnlen( char *argument, int maxlength )
{
   int str, count = 0;
   bool IS_COLOR = FALSE;

   for( str = 0; argument[str] != '\0'; str++ )
   {
      if( argument[str] == '&' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else if( argument[str] == '^' )
      {
         if( IS_COLOR == TRUE )
         {
            count++;
            IS_COLOR = FALSE;
         }
         else
            IS_COLOR = TRUE;
      }
      else
      {
         if( IS_COLOR == FALSE )
            count++;
         else
            IS_COLOR = FALSE;
      }

      if( count >= maxlength )
         break;
   }
   if( count < maxlength )
      return ( ( str - count ) + maxlength );

   str++;
   return str;
}

#ifdef WIN32

void shutdown_mud( char *reason );

void bailout( void )
{
   echo_to_all( AT_IMMORT, "MUD shutting down by system operator NOW!!", ECHOTAR_ALL );
   shutdown_mud( "MUD shutdown by system operator" );
   log_string( "MUD shutdown by system operator" );
   Sleep( 5000 ); /* give "echo_to_all" time to display */
   mud_down = TRUE;  /* This will cause game_loop to exit */
   service_shut_down = TRUE;  /* This will cause characters to be saved */
   fflush( stderr );
   return;
}

#endif

/* Signal handling - Josh */
void init_signals(  )
{
   struct sigaction setup_action;
   sigset_t block_mask;

   sigemptyset( &block_mask );
   /*
    * Block other terminal-generated signals while handler runs. 
    */
   sigaddset( &block_mask, SIGINT );
   sigaddset( &block_mask, SIGQUIT );
   setup_action.sa_handler = sig_handler;
   setup_action.sa_mask = block_mask;
   setup_action.sa_flags = 0;
   sigaction( SIGBUS, &setup_action, NULL );
   sigaction( SIGTERM, &setup_action, NULL );
   // sigaction (SIGABRT, &setup_action, NULL);
   sigaction( SIGSEGV, &setup_action, NULL );
}

void sig_handler( int sig )
{
   CHAR_DATA *ch;
   char buf[MAX_STRING_LENGTH];
   /*
    * Ignor SIGSEGV and SIGBUS by Josh - Signal Blocking by Josh 
    */
   /*
    * Modified 8/28/99 by Scion 
    */

   log_string( lastplayercmd );
   for( ch = first_char; ch; ch = ch->next )
   {
      if( IS_NPC( ch ) )
         continue;
      sprintf( buf, "PC: %s room: %d", ch->name, ch->in_room ? ch->in_room->vnum : -1 );
      log_string( buf );
   }

   if( sysdata.crashguard == FALSE )
   {  /* in case I -want- a core dump -- Scion */
      switch ( sig )
      {
         case SIGBUS:
            bug( "Signal SIGBUS.", 0 );
         case SIGTERM:
            bug( "Signal SIGTERM.", 0 );
         case SIGABRT:
            bug( "Signal SIGABRT.", 0 );
         case SIGSEGV:
            bug( "Signal SIGSEGV.", 0 );
      }
      log_string( "Crashguard is OFF.  Dumping core..." );
      abort(  );
   }

   switch ( sig )
   {
      case SIGBUS:
         bug( "Sig handler SIGBUS.", 0 );
         do_crashguard(  );
         break;
      case SIGTERM:
         bug( "Sig handler SIGTERM.", 0 );
         do_crashguard(  );
         break;
      case SIGABRT:
         bug( "Sig handler SIGABRT.", 0 );
         do_crashguard(  );
         break;
      case SIGSEGV:
         bug( "Sig handler SIGSEGV.", 0 );
         do_crashguard(  );
         break;
   }
}

/* End of Signal Handler - Josh */
