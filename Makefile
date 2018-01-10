CC = g++
CFLAGS = `pkg-config opencv --cflags --libs`
TARGET = mocap
HEADER = multitrack

all:
	$(CC) $(TARGET).cpp $(HEADER).h $(HEADER).cpp $(CFLAGS) -o $(TARGET)

clean:
	$(RM) $(TARGET)
