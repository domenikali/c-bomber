CC = gcc
CFLAGS = -Wall -Wextra -g -lncurses

INCDIR = include

EXEC = serveur client

SOBJS = ./src/serveur.o ./src/serveur_connexions.o ./src/game.o ./src/utils.o  
COBJS = ./src/client.o ./src/client_connexions.o ./src/utils.o ./src/ncurs.o

LDFLAGS_CLIENT = -lncurses

all: $(EXEC)

serveur: $(SOBJS)
	$(CC) $(CFLAGS) $^ -o $@ 

client: $(COBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS_CLIENT)

clean:
	rm -f $(EXEC) $(SOBJS) $(COBJS)
# CC = gcc
# CFLAGS = -Wall -Wextra -g -Iinclude
# LDFLAGS = -lpthread  # Ajoutez d'autres flags de linker si nécessaire

# # Dossiers
# SRCDIR = src
# OBJDIR = obj
# BINDIR = bin
# INCDIR = include

# # Fichiers exécutables
# EXEC = $(BINDIR)/serveur $(BINDIR)/client

# # Fichiers objet
# SOBJS = $(OBJDIR)/serveur.o $(OBJDIR)/serveur_connexions.o $(OBJDIR)/utils.o
# COBJS = $(OBJDIR)/client.o $(OBJDIR)/client_connexions.o $(OBJDIR)/utils.o

# # Cibles phony
# .PHONY: all clean directories

# # Règle principale
# all: directories $(EXEC)

# # Création des répertoires
# directories:
# 	mkdir -p $(OBJDIR) $(BINDIR)

# # Lien pour l'exécutable serveur
# $(BINDIR)/serveur: $(SOBJS)
# 	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# # Lien pour l'exécutable client
# $(BINDIR)/client: $(COBJS)
# 	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# # Compilation des fichiers source serveur
# $(OBJDIR)/serveur.o: $(SRCDIR)/serveur.c $(INCDIR)/serveur_connexions.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(OBJDIR)/serveur_connexions.o: $(SRCDIR)/serveur_connexions.c $(INCDIR)/serveur_connexions.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(OBJDIR)/utils.o: $(SRCDIR)/utils.c $(INCDIR)/utils.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# # Compilation des fichiers source client
# $(OBJDIR)/client.o: $(SRCDIR)/client.c $(INCDIR)/client_connexions.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(OBJDIR)/client_connexions.o: $(SRCDIR)/client_connexions.c $(INCDIR)/client_connexions.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# # Nettoyage des fichiers compilés
# clean:
# 	rm -rf $(BINDIR)/* $(OBJDIR)/*
