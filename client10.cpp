#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define BUF_SIZE 128
#define MSG_TYPE_DATA 1
#define MSG_TYPE_RESULT 2

// Структура сообщения
struct msg_buffer {
  long msg_type;
  char msg_text[BUF_SIZE];
};

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <input1> <input2> <output1> <output2>\n", argv[0]);
    exit(1);
  }

  // Создаем очереди сообщений
  key_t key1 = ftok("queue1", 65);
  key_t key2 = ftok("queue2", 66);
  int msgid1 = msgget(key1, 0666 | IPC_CREAT);
  int msgid2 = msgget(key2, 0666 | IPC_CREAT);

  // Открываем входные файлы
  int fd_input1 = open(argv[1], O_RDONLY);
  int fd_input2 = open(argv[2], O_RDONLY);
  if (fd_input1 == -1 || fd_input2 == -1) {
    perror("open input file");
    exit(1);
  }

  // Читаем и передаем данные по частям
  struct msg_buffer message;
  ssize_t bytes_read;

  while ((bytes_read = read(fd_input1, message.msg_text, BUF_SIZE)) > 0) {
    message.msg_type = MSG_TYPE_DATA;
    msgsnd(msgid1, &message, bytes_read, 0);
  }
  while ((bytes_read = read(fd_input2, message.msg_text, BUF_SIZE)) > 0) {
    message.msg_type = MSG_TYPE_DATA;
    msgsnd(msgid1, &message, bytes_read, 0);
  }
  close(fd_input1);
  close(fd_input2);

  // Открываем выходные файлы
  int fd_output1 = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  int fd_output2 = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd_output1 == -1 || fd_output2 == -1) {
    perror("open output file");
    exit(1);
  }

  // Читаем и записываем результаты по частям
  while (msgrcv(msgid2, &message, BUF_SIZE, MSG_TYPE_RESULT, 0) > 0) {
    write(fd_output1, message.msg_text, strlen(message.msg_text));
  }
  while (msgrcv(msgid2, &message, BUF_SIZE, MSG_TYPE_RESULT, 0) > 0) {
    write(fd_output2, message.msg_text, strlen(message.msg_text));
  }

  // Закрываем файлы и удаляем очереди
  close(fd_output1);
  close(fd_output2);
  msgctl(msgid1, IPC_RMID, NULL);
  msgctl(msgid2, IPC_RMID, NULL);

  printf("Программа успешно завершена.\n");
  return 0;
}