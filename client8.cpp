#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUF_SIZE 5000
#define FIFO1 "fifo1"
#define FIFO2 "fifo2"

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <input1> <input2> <output1> <output2>\n", argv[0]);
    exit(1);
  }

  // Создаем именованные каналы
  mkfifo(FIFO1, 0666);
  mkfifo(FIFO2, 0666);

  // Открываем входные файлы
  int fd_input1 = open(argv[1], O_RDONLY);
  int fd_input2 = open(argv[2], O_RDONLY);
  if (fd_input1 == -1 || fd_input2 == -1) {
    perror("open input file");
    exit(1);
  }

  // Читаем данные из файлов
  char buf1[BUF_SIZE], buf2[BUF_SIZE];
  ssize_t bytes_read1 = read(fd_input1, buf1, BUF_SIZE);
  ssize_t bytes_read2 = read(fd_input2, buf2, BUF_SIZE);
  if (bytes_read1 == -1 || bytes_read2 == -1) {
    perror("read input file");
    exit(1);
  }
  close(fd_input1);
  close(fd_input2);

  // Открываем FIFO1 для записи
  int fd_fifo1 = open(FIFO1, O_WRONLY);
  if (fd_fifo1 == -1) {
    perror("open FIFO1");
    exit(1);
  }

  // Передаем данные через FIFO1
  write(fd_fifo1, buf1, bytes_read1);
  write(fd_fifo1, buf2, bytes_read2);
  close(fd_fifo1);

  // Открываем FIFO2 для чтения
  int fd_fifo2 = open(FIFO2, O_RDONLY);
  if (fd_fifo2 == -1) {
    perror("open FIFO2");
    exit(1);
  }

  // Читаем результаты из FIFO2
  char result1[BUF_SIZE], result2[BUF_SIZE];
  read(fd_fifo2, result1, BUF_SIZE);
  read(fd_fifo2, result2, BUF_SIZE);
  close(fd_fifo2);

  // Записываем результаты в выходные файлы
  int fd_output1 = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  int fd_output2 = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd_output1 == -1 || fd_output2 == -1) {
    perror("open output file");
    exit(1);
  }
  write(fd_output1, result1, strlen(result1));
  write(fd_output2, result2, strlen(result2));
  close(fd_output1);
  close(fd_output2);

  // Удаляем именованные каналы
  unlink(FIFO1);
  unlink(FIFO2);

  return 0;
}