bin_PROGRAMS = yawc_port yawc_client
CFLAGS += -O3 -Wall -g 

INCLUDES += -I../include

#ifdef USE_MYSQL
  INCLUDES += $(MYSQL_INCLUDE)
#endif

yawc_port_SOURCES = clipboard.c key.c fun.c friends.c chat.c help.c menu.c \
  normal_input.c newuser.c registration.c inter.c \
  uadmin.c wholist.c read_menu.c rooms.c bbsconfig.c enter_message.c qc.c

yawc_client_SOURCES = clipboard.c key.c fun.c friends.c chat.c help.c menu.c \
  newuser.c registration.c inter.c wholist.c uadmin.c \
  read_menu.c rooms.c bbsconfig.c enter_message.c qc.c

yawc_port_LDADD = p_commands.o p_express.o p_main.o p_messages.o p_cprintf.o \
  p_routines2.o p_statusbar.o p_usertools.o p_display_message.o \
  $(MYSQL_LIB) -L../lib -L../shix -L../icq -lyawc -lm -lshix -licq \
  @INTLLIBS@

yawc_client_LDADD = c_commands.o c_client_input.o c_express.o c_main.o \
  c_messages.o c_routines2.o c_statusbar.o c_usertools.o c_display_message.o \
  $(MYSQL_LIB) -L../lib -L../shix -L../icq -lyawc -lm -lshix -licq \
  @INTLLIBS@

c_commands.o: commands.c
	$(COMPILE) \
		-DCLIENTSRC -c commands.c -o c_commands.o

c_client_input.o: client_input.c
	$(COMPILE) \
		-DCLIENTSRC -c client_input.c -o c_client_input.o

c_express.o: express.c
	$(COMPILE) \
		-DCLIENTSRC -c express.c -o c_express.o

c_display_message.o: display_message.c
	$(COMPILE) \
		-DCLIENTSRC -c display_message.c -o c_display_message.o

c_messages.o: messages.c
	$(COMPILE) \
		-DCLIENTSRC -c messages.c -o c_messages.o

c_routines2.o: routines2.c
	$(COMPILE) \
		-DCLIENTSRC -c routines2.c -o c_routines2.o

c_usertools.o: usertools.c
	$(COMPILE) \
		-DCLIENTSRC -c usertools.c -o c_usertools.o

c_statusbar.o: statusbar.c
	$(COMPILE) \
		-DCLIENTSRC -c statusbar.c -o c_statusbar.o

c_main.o: main.c
	$(COMPILE) \
		-DCLIENTSRC -c main.c -o c_main.o

# port

p_cprintf.o: cprintf.c
	$(COMPILE) \
		-DPORT -c cprintf.c -o p_cprintf.o

p_commands.o: commands.c
	$(COMPILE) \
		-DPORT -c commands.c -o p_commands.o

p_display_message.o: display_message.c
	$(COMPILE) \
		-DPORT -c display_message.c -o p_display_message.o

p_express.o: express.c
	$(COMPILE) \
		-DPORT -c express.c -o p_express.o

p_messages.o: messages.c
	$(COMPILE) \
		-DPORT -c messages.c -o p_messages.o

p_routines2.o: routines2.c
	$(COMPILE) \
		-DPORT -c routines2.c -o p_routines2.o

p_usertools.o: usertools.c
	$(COMPILE) \
		-DPORT -c usertools.c -o p_usertools.o

p_statusbar.o: statusbar.c
	$(COMPILE) \
		-DPORT -c statusbar.c -o p_statusbar.o

p_main.o: main.c
	$(COMPILE) \
		-DPORT -c main.c -o p_main.o

#CFLAGS += -w -W \
#  -Wall \
#  -Waggregate-return -Wcast-align -Wcast-qual \
#  -Wcomment -Wconversion -Werror -Wformat -Wimplicit \
#  -Wimplicit-int -Wimplicit-function-declaration -Winline \
#  -Wmissing-prototypes -Wmissing-declarations \
#  -Wnested-externs -Wno-import -Wparentheses -Wpointer-arith \
#  -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes \
#  -Wswitch
