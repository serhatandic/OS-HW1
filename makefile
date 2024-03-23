# main: eshell.cpp parser.c
# 	g++ -o eshell eshell.cpp parser.c -I. -std=c++11

main: eshellv2.cpp parser.c
	g++ -o eshellv2 eshellv2.cpp parser.c -I. -std=c++11 -w