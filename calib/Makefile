CC = g++
CFLAGS = `pkg-config opencv --cflags --libs`
TARGET = calib

all:
	$(CC) -o $(TARGET) $(TARGET).cpp $(CFLAGS)

clean:
	$(RM) $(TARGET)
