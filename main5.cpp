#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

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

  const char *fifo1 = "/tmp/fifo1";
  const char *fifo2 = "/tmp/fifo2";

  // Создаем именованные каналы
  mkfifo(fifo1, 0666);
  mkfifo(fifo2, 0666);

  pid_t pid1 = fork();
  if (pid1 < 0) {
    perror("Ошибка создания процесса");
    return 1;
  }

  if (pid1 == 0) { // Процесс 2: обработка данных
    int fd1 = open(fifo1, O_RDONLY);
    int fd2 = open(fifo2, O_WRONLY);

    char str1[BUFFER_SIZE], str2[BUFFER_SIZE];
    read(fd1, str1, BUFFER_SIZE);
    read(fd1, str2, BUFFER_SIZE);

    char result1[BUFFER_SIZE], result2[BUFFER_SIZE];
    find_unique_chars(str1, str2, result1, result2);

    write(fd2, result1, BUFFER_SIZE);
    write(fd2, result2, BUFFER_SIZE);

    close(fd1);
    close(fd2);
    exit(0);
  } else {
    pid_t pid2 = fork();
    if (pid2 < 0) {
      perror("Ошибка создания процесса");
      return 1;
    }

    if (pid2 == 0) { // Процесс 3: запись результата
      int fd2 = open(fifo2, O_RDONLY);

      char result1[BUFFER_SIZE], result2[BUFFER_SIZE];
      read(fd2, result1, BUFFER_SIZE);
      read(fd2, result2, BUFFER_SIZE);

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
      close(fd2);
      exit(0);
    } else { // Процесс 1: чтение данных
      int fd1 = open(fifo1, O_WRONLY);

      int input1_fd = open(argv[1], O_RDONLY);
      int input2_fd = open(argv[2], O_RDONLY);
      if (input1_fd < 0 || input2_fd < 0) {
        perror("Ошибка открытия входного файла");
        return 1;
      }

      char str1[BUFFER_SIZE], str2[BUFFER_SIZE];
      read(input1_fd, str1, BUFFER_SIZE);
      read(input2_fd, str2, BUFFER_SIZE);

      write(fd1, str1, BUFFER_SIZE);
      write(fd1, str2, BUFFER_SIZE);

      close(input1_fd);
      close(input2_fd);
      close(fd1);

      wait(NULL); // Ожидаем завершения дочерних процессов
      wait(NULL);

      // Удаляем именованные каналы
      unlink(fifo1);
      unlink(fifo2);
    }
  }

  return 0;
}