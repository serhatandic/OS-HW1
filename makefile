main: eshell.cpp parser.c
	g++ -o eshell eshell.cpp parser.c -I. -std=c++11