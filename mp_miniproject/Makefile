all: myShell arguments hello re_input

myShell:myShell.cpp
	g++ -g -o myShell -pedantic -Werror -Wall -std=gnu++03 myShell.cpp
arguments:argu.cpp
	g++ -g -o arguments -pedantic -Werror -Wall -std=gnu++03 argu.cpp
hello:hello.cpp
	g++ -g -o hello -pedantic -Werror -Wall -std=gnu++03 hello.cpp
re_input::re_input.cpp
	g++ -g -o re_input -pedantic -Werror -Wall -std=gnu++03 re_input.cpp
.PHONY:clean
clean:
	rm -f myShell arguments hello re_input *~ *.o \#*
