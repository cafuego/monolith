bin_PROGRAMS = yawc_port yawc_client
CFLAGS = -O3 -Wall 

yawc_port_SOURCES = clipboard.c friends.c chat.c fun.c help.c menu.c \
  normal_input.c newuser.c registration.c key.c inter.c vote.c \
  uadmin.c wholist.c read_menu.c rooms.c bbsconfig.c enter_message.c qc.c

yawc_client_SOURCES = clipboard.c friends.c chat.c fun.c help.c menu.c \
  client_input.c newuser.c registration.c key.c inter.c \
  vote.c wholist.c uadmin.c read_menu.c rooms.c bbsconfig.c enter_message.c qc.c

yawc_port_LDADD = p_commands.o p_express.o p_main.o p_messages.o cprintf.o \
  routines2.o p_statusbar.o p_usertools.o p_display_message.o \
  -L../lib -L../shix -lyawc -lm -lshix -L/usr/local/mysql/lib 

yawc_client_LDADD =  c_commands.o c_express.o c_main.o c_messages.o \
  c_routines2.o c_statusbar.o c_usertools.o c_display_message.o \
  -L../lib -L../shix -lyawc -lm -lshix -L/usr/local/mysql/lib

c_commands.o: commands.c
	$(COMPILE) -DCLIENTSRC -c commands.c -o c_commands.o

c_express.o: express.c
	$(COMPILE) -DCLIENTSRC -c express.c -o c_express.o

c_display_message.o: display_message.c
	$(COMPILE) -DCLIENTSRC -c display_message.c -o c_display_message.o

c_messages.o: messages.c
	$(COMPILE) -DCLIENTSRC -c messages.c -o c_messages.o

c_routines2.o: routines2.c
	$(COMPILE) -DCLIENTSRC -c routines2.c -o c_routines2.o

c_usertools.o: usertools.c
	$(COMPILE) -DCLIENTSRC -c usertools.c -o c_usertools.o

c_statusbar.o: statusbar.c
	$(COMPILE) -DCLIENTSRC -c statusbar.c -o c_statusbar.o

c_main.o: main.c
	$(COMPILE) -DCLIENTSRC -c main.c -o c_main.o

# port

cprintf.o: cprintf.c
	$(COMPILE) -DPORT -c cprintf.c -o cprintf.o

p_commands.o: commands.c
	$(COMPILE) -c commands.c -o p_commands.o

p_display_message.o: display_message.c
	$(COMPILE) -c display_message.c -o p_display_message.o

p_express.o: express.c
	$(COMPILE) -c express.c -o p_express.o

p_messages.o: messages.c
	$(COMPILE) -c messages.c -o p_messages.o

p_routines2.o: routines2.c
	$(COMPILE) -c routines2.c -o p_routines2.o

p_usertools.o: usertools.c
	$(COMPILE) -c usertools.c -o p_usertools.o

p_statusbar.o: statusbar.c
	$(COMPILE) -c statusbar.c -o p_statusbar.o

p_main.o: main.c
	$(COMPILE) -c main.c -o p_main.o

#CFLAGS += -w -W \
#  -Wall \
#  -Waggregate-return -Wcast-align -Wcast-qual \
#  -Wcomment -Wconversion -Werror -Wformat -Wimplicit \
#  -Wimplicit-int -Wimplicit-function-declaration -Winline \
#  -Wmissing-prototypes -Wmissing-declarations \
#  -Wnested-externs -Wno-import -Wparentheses -Wpointer-arith \
#  -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes \
#  -Wswitch
