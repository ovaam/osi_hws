#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUF_SIZE 128
#define FIFO1 "fifo1"
#define FIFO2 "fifo2"

// Функция для вычисления разности символов
void diff_chars(const char *str1, const char *str2, char *result) {
  int count[256] = {0};
  int j = 0;

  // Помечаем символы из второй строки
  for (int i = 0; str2[i]; i++) {
    count[(unsigned char)str2[i]] = 1;
  }

  // Добавляем символы из первой строки, которых нет во второй
  for (int i = 0; str1[i]; i++) {
    if (count[(unsigned char)str1[i]] == 0) {
      result[j++] = str1[i];
      count[(unsigned char)str1[i]] = 1; // Чтобы избежать дубликатов
    }
  }
  result[j] = '\0';
}

int main() {
  // Открываем FIFO1 для чтения
  int fd_fifo1 = open(FIFO1, O_RDONLY);
  if (fd_fifo1 == -1) {
    perror("open FIFO1");
    exit(1);
  }

  // Открываем FIFO2 для записи
  int fd_fifo2 = open(FIFO2, O_WRONLY);
  if (fd_fifo2 == -1) {
    perror("open FIFO2");
    exit(1);
  }

  // Читаем и обрабатываем данные по частям
  char buf1[BUF_SIZE], buf2[BUF_SIZE];
  char result1[BUF_SIZE], result2[BUF_SIZE];
  ssize_t bytes_read1, bytes_read2;

  while ((bytes_read1 = read(fd_fifo1, buf1, BUF_SIZE)) > 0) {
    bytes_read2 = read(fd_fifo1, buf2, BUF_SIZE);
    if (bytes_read2 <= 0) break;

    // Обрабатываем данные
    diff_chars(buf1, buf2, result1);
    diff_chars(buf2, buf1, result2);

    // Передаем результаты
    write(fd_fifo2, result1, strlen(result1) + 1);
    write(fd_fifo2, result2, strlen(result2) + 1);
  }

  // Закрываем каналы
  close(fd_fifo1);
  close(fd_fifo2);

  return 0;
}