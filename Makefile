# cs335 duckhunt
# to compile your project, type make and press enter

all: duckhunt

duckhunt: duckhunt.cpp
	g++ duckhunt.cpp -Wall -o duckhunt -lX11 -lGL -lGLU -lm

clean:
	rm -f duckhunt
	rm -f *.o

