#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_LEN 1000  // Максимальная длина строки

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
void find_unique_chars(const char *str1, const char *str2, char *result) {
  int index = 0;
  for (int i = 0; str1[i] != '\0'; i++) {
    if (strchr(str2, str1[i]) == NULL) {
      result[index++] = str1[i];
    }
  }
  result[index] = '\0';
  remove_duplicates(result);
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Использование: %s <входной файл 1> <входной файл 2> <выходной файл 1> <выходной файл 2>\n", argv[0]);
    return 1;
  }

  const char *input_file1 = argv[1];
  const char *input_file2 = argv[2];
  const char *output_file1 = argv[3];
  const char *output_file2 = argv[4];

  // Открываем входные файлы
  FILE *input1 = fopen(input_file1, "r");
  FILE *input2 = fopen(input_file2, "r");
  if (!input1 || !input2) {
    perror("Ошибка открытия входного файла");
    return 1;
  }

  // Читаем строки из входных файлов
  char str1[MAX_STR_LEN], str2[MAX_STR_LEN];
  fgets(str1, MAX_STR_LEN, input1);
  fgets(str2, MAX_STR_LEN, input2);

  // Удаляем символ новой строки, если он есть
  str1[strcspn(str1, "\n")] = '\0';
  str2[strcspn(str2, "\n")] = '\0';

  // Закрываем входные файлы
  fclose(input1);
  fclose(input2);

  // Находим уникальные символы
  char result1[MAX_STR_LEN], result2[MAX_STR_LEN];
  find_unique_chars(str1, str2, result1);  // Символы, которые есть в str1, но отсутствуют в str2
  find_unique_chars(str2, str1, result2);  // Символы, которые есть в str2, но отсутствуют в str1

  // Открываем выходные файлы
  FILE *output1 = fopen(output_file1, "w");
  FILE *output2 = fopen(output_file2, "w");
  if (!output1 || !output2) {
    perror("Ошибка открытия выходного файла");
    return 1;
  }

  // Записываем результаты в выходные файлы
  fprintf(output1, "%s\n", result1);
  fprintf(output2, "%s\n", result2);

  // Закрываем выходные файлы
  fclose(output1);
  fclose(output2);

  printf("Результаты записаны в файлы %s и %s\n", output_file1, output_file2);

  return 0;
}