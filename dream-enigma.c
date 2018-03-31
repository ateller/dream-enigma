#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

//Я не настолько хорошо знаю английский,
//чтобы писать на нем комментарии и не
//позориться, а исправлять перевод
//после гугла мне сейчас лень.

#ifndef IF_ERR
#define IF_ERR(check, error, info, handle)\
	do { \
		if (check == error) { \
			perror(info);\
			handle\
		} \
	} while (0)
#endif
//Макрос для обработки ошибок

char **split(char *input)
{
	char **args;
	//Он как argv
	int i = 0;

	args = malloc(sizeof(char *));
	IF_ERR(args, NULL, "Malloc error:", exit(errno););
	//Выделяем память под массив
	//аргументов
	args[i] = strtok(input, " \t\n");
	//Бьем строку
	for (; args[i] != NULL; ) {
		char **dup_args;
		//Сохраним сюда указатель перед
		//реаллоком

		i++;
		dup_args = args;
		args = realloc(args, (i + 1) * sizeof(char *));
		//Увеличиваем вектор
		if (args == NULL) {
			perror("Realloc error");
			free(dup_args);
			exit(errno);
		}
		args[i] = strtok(NULL, " \t\n");
		//Записываем очередную лексему
	}
	//Когда случился NULL, выход
	return args;
}

int main(int argc, char *argv[])
{
	char **args1, **args2;
	printf("Program 1: %s\nProgram 2: %s\n", argv[1], argv[2]);
	if (argc < 3) {
		printf("Error: too few arguments");
		exit(1);
	}
	args1 = split(argv[1]);
	args2 = split(argv[2]);
	exit(0);
}
