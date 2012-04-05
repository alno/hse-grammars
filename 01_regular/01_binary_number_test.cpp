/*
 В данной программе анализируется язык, описывающий целые двоичные числа с опциональным знаком

 Грамматика:
   START -> + T0 | - T0 | 0 T1 | 1 T1
   T0 -> 0 T1 | 1 T1
   T1 -> 0 T1 | 1 T1 | $
*/

//#include "stdafx.h" // Подключаем прекомлируемый заголовочный файл, если он есть в проекте

#include <iostream> // Используем стандартный ввод и вывод - подключаем заголовочный файл
#include <cstdio> // Для функции getchar

char c; // Текущий считанный символ

/*
  Процедура считывания следующего символа
*/
void gc() {
  c = getchar();
}

/*
  Функция проверки ввода на принадлежность языку
*/
bool check() {
  enum State { START, T0, T1, ERR, FIN }; // Все нетерминалы + завершающее состояние и состояние ошибки

  State state = START; // Переменная, хранящая состояние
  gc(); // Считываем первый символ

  do { // Вся обработка производится в цикле
    switch ( state ) { // Обработка различных состояний
      case START: // Начальное состояние
        if ( c == '+' || c == '-' ) { // Если текущий символ - знак, то считываем следующий и переходим в состояние T0 (не считано ни одной цифры)
          gc();
          state = T0;
        } else if ( c == '0' || c == '1' ) { // Если текущий символ - цифра, то считываем следующий и переходим в состояние T1 (считана хотя бы одна цифра)
          gc();
          state = T1;
        } else { // Иначе ошибка
          state = ERR;
        }
        break;
      case T0: // Состояние, когда еще ни одной цифры не считано
        if ( c == '0' || c == '1' ) { // Если текущий символ - цифра, то считываем следующий и переходим в следующее состояние
          gc();
          state = T1;
        } else { // Иначе ошибка
          state = ERR;
        }
        break;
      case T1: // Считана хотя бы одна цифра
        if ( c == '0' || c == '1' ) { // Если текущий символ - цифра, то считываем следующий и переходим в следующее состояние
          gc();
          state = T1;
        } else if (c == '\n' || c == '\r') {
          state = FIN;
        } else { // Иначе ошибка
          state = ERR;
        }
        break;
      default:
        state = ERR;
    };
  } while ( state != FIN && state != ERR ); // Цикл продолжается пока состояние не станет завершающим или ошибочным

  return state == FIN; // Если состояние завершающее, то цепочка принята
}

/*
  Основная программа, просто выводим результат проверки цепочки
*/
int main(int argc,char ** argv) {
  std::cout << (check() ? "yes" : "no") << std::endl;

  return 0;
}