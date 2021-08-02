# Compilador
CC := gcc

# Nombre de la carpeta source donde se encuentran los archivos a compilar
SRCDIR := src

# Nombre del ejecutable
TARGET := main

# Extension de los archivos a compilar
SRCEXT := c

# Recorro todos los directorios dentro de 'src' para obtener
# todos los archivos .c
# --- SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

# Flags para compilar
# -g : Genera informacion util para debuggear
# -Wall : Habilita todos los mensajes de tipo 'Warning'
CFLAGS := -g -Wall

# Librerias utilizadas en la compilacion
LIB := -lcommons -lreadline -pthread -lunnamed -lnivel-gui -lncurses -lm

# make o make all : Compila todos los archivos y ejecuta la aplicacion
all: build run

# make clean : Elimina el ejecutable y los archivos de log
clean:
	rm ./main ./*.log swapFile.bin *.dmp

# make build : Compila todos los archivos .c y genera un ejecutable
build:
	$(CC) -o $(TARGET) ./*.$(SRCEXT) $(LIB)

# make run : Ejecuta la aplicacion
run:
	./main

# make test : Ejecuta la aplicacion utilizando Valgrind
test:
	# valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./main
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file="valgrind.log" --verbose ./main
