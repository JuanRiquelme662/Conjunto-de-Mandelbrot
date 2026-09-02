CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -fopenmp -lpthread
TARGET = mandelbrot
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.pgm times.txt