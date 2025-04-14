CFLAGS = -Wall -Wextra -O3 -march=native -pipe
LDFLAGS = 
TARGET = czram

SRC = main.c commands.c validation.c utils.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/bin

.PHONY: all clean install
