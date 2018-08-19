CC:=gcc

TARGET_SRV=server_pbj
TARGET_CLN=client_pbj

LDFLAGS=-D_REENTRANT -pthread

all : $(TARGET_SRV) $(TARGET_CLN)

$(TARGET_SRV):$(TARGET_SRV).o
	$(CC) -o $@ $< $(LDFLAGS)

$(TARGET_CLN):$(TARGET_CLN).o
	$(CC) -o $@ $< $(LDFLAGS)

%.o:%.c
	$(CC) -c -o $@ $<

clean:
	rm -f *.o $(TARGET_SRV) $(TARGET_CLN)
