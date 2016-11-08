ALLEGRO_VERSION=5.0.10
MINGW_VERSION=4.7.0
FOLDER=C:

FOLDER_NAME=\allegro-$(ALLEGRO_VERSION)-mingw-$(MINGW_VERSION)
PATH_ALLEGRO=$(FOLDER)$(FOLDER_NAME)
LIB_ALLEGRO=\lib\liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=\include

all: KTH.exe

KTH.exe: KTH.o
	gcc -o KTH.exe KTH.o $(PATH_ALLEGRO)$(LIB_ALLEGRO) -Os -O2

KTH.o: KTH.c
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c KTH.c -Os -O2
 
clean:
	del KTH.o 
	del KTH.exe