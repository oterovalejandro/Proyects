####
#
# MODULO: Makefile
# DESCRIPCION:  Compilacion de la Practica 5, Cthilos
#	
####

CC = gcc
CFLAGS = -Wall -g 
LFLAGS = -g -lpthread

# Ejemplos
PROG = Cthilos
OBJ_PROG = cthilos.o

all: $(PROG)

# Generacion del programa "Cthilos"
$(PROG) : $(OBJ_PROG) 
	$(CC) -o $@ $^ $(LFLAGS)

clean limpiar :
	rm -f $(PROG) *~ *.o

# Mensaje de ayuda
ayuda : 
	@echo 
	@echo "Fichero para la compilacion de Cthilos"
	@echo

