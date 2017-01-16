CC		= gcc
LD		= gcc
CFLAGS 	= -DRASPBERRY_PI
LDFLAGS	= -lwiringPi

OBJ		= main.o

web_server: $(OBJ)
		$(LD) $(CFLAGS) -o smsAlarm $(OBJ) $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $<
