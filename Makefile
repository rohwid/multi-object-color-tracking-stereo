CC = g++
CFLAGS = `pkg-config opencv --cflags --libs`
TARGET = balldetect

all:
	$(CC) -o $(TARGET) $(TARGET).cpp $(CFLAGS)

clean:
	$(RM) $(TARGET)
