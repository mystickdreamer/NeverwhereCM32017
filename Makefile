CC      = gcc
#PROF    = -p

# NOTE:  This MUD by default will start on port 4200, please adjust
#        "rmstart" before attempting to compile or startup.

#DMALLOC = -L/home/cm2/dmalloc/lib -ldmalloclp -ldmalloc -ldl

# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket

C_FLAGS = -g2 -Wall $(PROF) $(SOLARIS_FLAG)
L_FLAGS = $(PROF) $(SOLARIS_LINK)

#Intermud-3 - Comment out to disable I3 support in your code
I3 = 1

#IMC2 - Comment out to disable IMC2 support
IMC = 1

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c \
	  arena.c ban.c birth.c bodyparts.c build.c bits.c city.c \
	  clan.c combat.c comm.c const.c db.c deity.c elkandu.c \
	  fight.c grub.c hiscores.c handler.c hashstr.c \
	  interp.c magic.c makeobjs.c makezone.c md5.c misc.c mpxset.c \
	  mud_comm.c mud_prog.c namegen.c player.c reset.c \
	  save.c shops.c skills.c special.c tables.c talent.c \
	  track.c update.c house.c oedit.c redit.c medit.c

O_FILES = $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h)

ifdef IMC
   C_FILES := imc.c $(C_FILES)
   C_FLAGS := $(C_FLAGS) -DIMC -DIMCSMAUG
endif

ifdef I3
   C_FILES := i3.c $(C_FILES)
   C_FLAGS := $(C_FLAGS) -DI3 -DI3SMAUG
endif

all:
	make -s rmexe

rmexe: $(O_FILES)
	rm -f rmexe
	$(CC) $(L_FLAGS) -o rmexe $(O_FILES)
	echo "Done compiling rmexe...";
	chmod g+w rmexe
	chmod a+x rmexe
	chmod g+w $(O_FILES)

indent:
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(C_FILES)
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(H_FILES)

indentclean:
	rm *.c~ *.h~

o/%.o: %.c
	echo "  Now compiling... $@";
	$(CC) -c $(C_FLAGS) $< -o $@

clean:
	rm -f o/*.o rmexe *~

backup:
	rm -f o/*.o rmexe src.tar.gz
	tar -cvzf src.tar.gz *

restore:
	tar -xzvf src.tar.gz
	make
