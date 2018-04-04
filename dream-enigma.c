#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

//Я не настолько хорошо знаю английский,
//чтобы писать на нем комментарии и не
//позориться, а исправлять перевод
//после гугла мне сейчас лень.

#define PART_SIZE 1024
#define PATH_SIZE 512
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

int run_with_pipe(char **args)
{
	int descriptors[2];

	IF_ERR(pipe(descriptors), -1, "Pipe error", exit(errno););
	//Открываем трубу
	switch (fork()) {
	//И форкаемся
	case -1:
		//если ошибка
		perror("Fork error:");
		exit(errno);
	case 0:
		IF_ERR(close(1), -1, "Close error", exit(errno););
		IF_ERR(close(descriptors[0]), -1, "Close error", exit(errno););
		//Мы - потомок. Закрываем свой
		//выход и выход из трубы
		IF_ERR(dup(descriptors[1]), -1, "Dup error", exit(errno););
		//Вывод в трубу
		IF_ERR(execvp(args[0], args), -1, "Exec error", exit(errno););
		//Исполняем то, что дали
	default:
		//Мы родитель. Зыкрываем вход
		//в трубу
		IF_ERR(close(descriptors[1]), -1, "Close error", exit(errno););
		free(args);
		return descriptors[0];
	}
}

//Подбираем имя файла, которое
//не занято
int createoutfile(char *name)
{
	int i = 0, len, descr;
	char path[PATH_SIZE + strlen(name) + 10];
	//10 состоит из 5 разрядов числа и
	//4 байта от .out
	struct stat temp;

	//path = malloc(PATH_SIZE + strlen(name) + 10);
	//IF_ERR(path, NULL, "Malloc error", exit(errno););
	IF_ERR(getcwd(path, PATH_SIZE), NULL, "Getcwd error", exit(errno););
	//Будем записывать в файл в текущей
	//директории
	len = strlen(path);
	if (path[len] != '/') {
		path[len] = '/';
		path[len+1] = 0;
	}
	//Допишем / в конец пути
	strcat(path, name);
	//Файл будет называться
	//имяi.out, где i = самое маленькое
	//число, которое удалось найти такое,
	//чтобы файла с таким именем не было
	len = strlen(path);
	//С len-го элемента начинается
	//число
	do {
		path[len] = 0;
		gcvt((double) i, 6, path + len);
		//У нас инт, так что не больше 5
		//разрядов и 0 байт
		strcat(path, ".out");
		i++;
	} while (stat(path, &temp) != -1);
	//Если стат вернет -1, такого файла
	//нету
	descr = open(path, O_CREAT|O_WRONLY|O_TRUNC, 0664);
	IF_ERR(descr, -1, path, exit(errno););
	printf("File %s created\n", path);
	//Создаем файл и возвращаем дескриптор
	return descr;
}

//подпрограмма для генерации
//гаммы
void generate(char *dst)
{
	int input, out, i;
	char buf[PART_SIZE];

	input = run_with_pipe(split(dst));
	//Запускаем процесс с трубой
	IF_ERR(wait(NULL), -1, "Wait error", exit(errno););
	//Ждем, пока он положит в трубу
	//все, что нужно

	out = createoutfile("gamma");
	//Создаем выходной файл с
	//говорящим именем
	srand(time(NULL));
	while (true) {
		int size = read(input, buf, PART_SIZE);
		//Читаем и проверяем
		switch (size) {
		case -1:
			//Ошибка
			perror("Read error");
			exit(errno);
		case 0:
			//Файл окончен
			return;
		}
		for (i = 0; i < size; i++)
			buf[i] = rand();
		//Делаем рандомные байты
		write(out, buf, size);
	}
}

//и для шифрования
void xor(char *arg1, char *arg2)
{
	int out, size, i, src1, src2;
	char piece1[PART_SIZE], piece2[PART_SIZE];

	src1 = run_with_pipe(split(arg1));
	src2 = run_with_pipe(split(arg2));
	//Запускаем два процесса с трубами
	for (i = 0; i < 2; i++) {
		IF_ERR(wait(&out), -1, "Wait error", exit(errno););
		if (WEXITSTATUS(out) != EXIT_SUCCESS)
			exit(EXIT_FAILURE);
	}
	//И ждем их
	out = createoutfile("enigma");
	while (true) {
		i = read(src1, piece1, PART_SIZE);
		//Читаем из первой трубы и
		//Запоминаем, сколько
		switch (i) {
		case -1:
			//Ошибка
			perror("Read error");
			exit(errno);
		case 0:
			//Файл окончен
			return;
		}
		size = read(src2, piece2, PART_SIZE);
		if (size < i) {
			//В size нужен размер
			//меньшего куска
			switch (size) {
			case -1:
				perror("Read error");
				exit(errno);
			case 0:
				return;
			}
		} else
			size = i;
			//Если size больше, чем i,
			//ошибки нет и проверять
			//нет смысла
		for (i = 0; i < size; i++)
			piece1[i] ^= piece2[i];
		//Побайтно xor
		write(out, piece1, size);
		//Кусок на выход
	}
}

int main(int argc, char *argv[])
{
	char args;
	int input1, input2;

	printf("Program 1: %s\nProgram 2: %s\n", argv[1], argv[2]);
	if (argc < 3) {
		printf("Error: too few arguments");
		exit(EXIT_FAILURE);
	}
	//должно быть не меньше 2 аргументов
	//(не считая названия программы)
	if (!strcmp(argv[1], "--generate")) {
		generate(argv[2]);
		exit(EXIT_SUCCESS);
	} else if (!strcmp(argv[2], "--generate")) {
		generate(argv[1]);
		exit(EXIT_SUCCESS);
	}
	//Если в одном из двух первых
	//аргументов стоит флаг,
	//генерируем гамму на основе
	//остального аргумента
	xor(argv[1], argv[2]);
	//Если флага не нашлось, пытаемся
	//гаммировать по 2 первым аргументам
	exit(EXIT_SUCCESS);
}
