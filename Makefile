CC = g++
CFLAGS = `pkg-config opencv --cflags --libs`
TARGET = mocap

all:
	$(CC) -o $(TARGET) $(TARGET).cpp $(CFLAGS)

clean:
	$(RM) $(TARGET)
