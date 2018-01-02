CC = g++
CFLAGS = `pkg-config opencv --cflags --libs`
TARGET = calib
OUTPUT1 = *.yml
OUTPUT2 = *.jpg

all:
	$(CC) -o $(TARGET) $(TARGET).cpp $(CFLAGS)

clean:
	$(RM) $(TARGET) $(OUTPUT1) $(OUTPUT2)
