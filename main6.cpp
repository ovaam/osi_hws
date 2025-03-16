#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 5000

// Функция для удаления дубликатов символов в строке
void remove_duplicates(char *str) {
  int len = strlen(str);
  int index = 0;

  for (int i = 0; i < len; i++) {
    int j;
    for (j = 0; j < i; j++) {
      if (str[i] == str[j]) {
        break;
      }
    }
    if (j == i) {
      str[index++] = str[i];
    }
  }
  str[index] = '\0';
}

// Функция для нахождения символов, которые есть в первой строке, но отсутствуют во второй
void find_unique_chars(const char *str1, const char *str2, char *result1, char *result2) {
  int index1 = 0, index2 = 0;

  // Символы, которые есть в str1, но отсутствуют в str2
  for (int i = 0; str1[i] != '\0'; i++) {
    if (strchr(str2, str1[i]) == NULL) {
      result1[index1++] = str1[i];
    }
  }
  result1[index1] = '\0';
  remove_duplicates(result1);

  // Символы, которые есть в str2, но отсутствуют в str1
  for (int i = 0; str2[i] != '\0'; i++) {
    if (strchr(str1, str2[i]) == NULL) {
      result2[index2++] = str2[i];
    }
  }
  result2[index2] = '\0';
  remove_duplicates(result2);
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    printf("Использование: %s <входной файл 1> <входной файл 2> <выходной файл 1> <выходной файл 2>\n", argv[0]);
    return 1;
  }

  int pipe1[2], pipe2[2];
  if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
    perror("Ошибка создания канала");
    return 1;
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("Ошибка создания процесса");
    return 1;
  }

  if (pid == 0) { // Процесс 2: обработка данных
    close(pipe1[1]); // Закрываем запись в первый канал
    close(pipe2[0]); // Закрываем чтение из второго канала

    char str1[BUFFER_SIZE], str2[BUFFER_SIZE];
    read(pipe1[0], str1, BUFFER_SIZE);
    read(pipe1[0], str2, BUFFER_SIZE);

    char result1[BUFFER_SIZE], result2[BUFFER_SIZE];
    find_unique_chars(str1, str2, result1, result2);

    write(pipe2[1], result1, BUFFER_SIZE);
    write(pipe2[1], result2, BUFFER_SIZE);

    close(pipe1[0]);
    close(pipe2[1]);
    exit(0);
  } else { // Процесс 1: чтение данных и запись результата
    close(pipe1[0]); // Закрываем чтение из первого канала
    close(pipe2[1]); // Закрываем запись во второй канал

    int input1_fd = open(argv[1], O_RDONLY);
    int input2_fd = open(argv[2], O_RDONLY);
    if (input1_fd < 0 || input2_fd < 0) {
      perror("Ошибка открытия входного файла");
      return 1;
    }

    char str1[BUFFER_SIZE], str2[BUFFER_SIZE];
    read(input1_fd, str1, BUFFER_SIZE);
    read(input2_fd, str2, BUFFER_SIZE);

    write(pipe1[1], str1, BUFFER_SIZE);
    write(pipe1[1], str2, BUFFER_SIZE);

    close(input1_fd);
    close(input2_fd);
    close(pipe1[1]);

    char result1[BUFFER_SIZE], result2[BUFFER_SIZE];
    read(pipe2[0], result1, BUFFER_SIZE);
    read(pipe2[0], result2, BUFFER_SIZE);

    int output1_fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int output2_fd = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output1_fd < 0 || output2_fd < 0) {
      perror("Ошибка открытия выходного файла");
      return 1;
    }

    write(output1_fd, result1, strlen(result1));
    write(output2_fd, result2, strlen(result2));

    close(output1_fd);
    close(output2_fd);
    close(pipe2[0]);

    wait(NULL); // Ожидаем завершения дочернего процесса
  }

  return 0;
}