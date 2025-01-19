FILES = ./src/main.c
NAME = Temperaio

brun:
	gcc -o ./bin/$(NAME).exe $(FILES)
	./bin/$(NAME).exe