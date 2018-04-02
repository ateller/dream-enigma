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

//подпрограмма для генерации
//гаммы
void generate(char *dst);

//и для шифрования
void xor(int src1, int src2);

int main(int argc, char *argv[])
{
	char args;
	int pipe1[2], pipe2[2], child_pid;
	printf("Program 1: %s\nProgram 2: %s\n", argv[1], argv[2]);
	if (argc < 3) {
		printf("Error: too few arguments");
		exit(1);
	}
	//должно быть не меньше 2 аргументов
	//(не считая названия программы)
	if(!strcmp(argv[1], "--generate") {
		generate(argv[2]);
		exit(0);
	}
	else 
		if (!strcmp(argv[2], "--generate") {
			generate(argv[1]);
			exit(0);
		}
	//Если в одном из двух первых
	//аргументов стоит флаг, 
	//генерируем гамму на основе 
	//остального аргумента
	IF_ERR(pipe(pipe1), -1, "Pipe error", exit(errno););
	IF_ERR(pipe(pipe2), -1, "Pipe error", exit(errno););
	child_pid = fork();
	//Копируем процесс
	switch (child_pid) {
	case -1:
		//если ошибка
		perror("Fork error:");
		exit(errno);
	case 0:
		IF_ERR(close(1), -1, "Close error", exit(errno);)
		//Мы - потомок. Закрываем свой
		//выход
		child_pid = fork();
		//Мы- потомок. Дублируемся
		switch(child_pid) {
		case -1:
			//если ошибка
			perror("Fork error:");
			exit(errno);
		case 0;
			//Мы - внук. Берем 1 аргумент
			args = split(argv[1]);
			IF_ERR(dup(pipe1[1], -1, "Dup error", exit(errno);
			//Вывод в первую трубу
			break;
		default:
			//Мы - потомок. Берем 2 аргумент
			args = split(argv[2]);
			IF_ERR(dup(pipe2[1], -1, "Dup error", exit(errno);
			//Вывод во 2 трубу
			IF_ERR(wait(&i), -1, "Wait error", exit(errno););
			//Ждем внука
		}
		//Тут уже неважно, кто мы. Просто
		//Исполняем то, что дадут
		IF_ERR(execvp(args[0], args), -1, "Exec error", exit(errno););
	default:
		//Мы - дед. Ждем сына, который
		//дождался внука и вывел свой вывод
		IF_ERR(wait(NULL), -1, "Wait error", exit(errno););
		xor(pipe1, pipe2);
		//Ну и начинаем неистово гаммировать
	}
	exit(0);
}
