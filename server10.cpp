#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
  // Создаем очереди сообщений
  key_t key1 = ftok("queue1", 65);
  key_t key2 = ftok("queue2", 66);
  int msgid1 = msgget(key1, 0666 | IPC_CREAT);
  int msgid2 = msgget(key2, 0666 | IPC_CREAT);

  // Читаем и обрабатываем данные по частям
  struct msg_buffer message;
  char result1[BUF_SIZE], result2[BUF_SIZE];

  while (msgrcv(msgid1, &message, BUF_SIZE, MSG_TYPE_DATA, 0) > 0) {
    char buf1[BUF_SIZE], buf2[BUF_SIZE];
    strcpy(buf1, message.msg_text);
    msgrcv(msgid1, &message, BUF_SIZE, MSG_TYPE_DATA, 0);
    strcpy(buf2, message.msg_text);

    // Обрабатываем данные
    diff_chars(buf1, buf2, result1);
    diff_chars(buf2, buf1, result2);

    // Передаем результаты
    message.msg_type = MSG_TYPE_RESULT;
    strcpy(message.msg_text, result1);
    msgsnd(msgid2, &message, strlen(result1) + 1, 0);
    strcpy(message.msg_text, result2);
    msgsnd(msgid2, &message, strlen(result2) + 1, 0);
  }

  // Удаляем очереди
  msgctl(msgid1, IPC_RMID, NULL);
  msgctl(msgid2, IPC_RMID, NULL);

  return 0;
}