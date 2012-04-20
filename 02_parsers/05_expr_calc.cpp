/*
 * В этой программе осуществляется вычисления арифметического выражения, включающего операции + и *
 *
 * Грамматика:
 *   S -> E // Дополнительное правило для конца цепочки
 *   E -> F { + E }
 *   F -> G { * G }
 *   G -> 0 | 1 | 2 | 3 | 5 | 6 | 7 | 8 | 9
 */

#include <iostream> // Используем стандартный ввод и вывод - подключаем заголовочный файл
#include <cstdio> // Для функции getchar

typedef char Lexeme; // Тип для лексемы (пока это просто символ, нужно для того, чтобы потом было проще перейти к использованию лексера)

Lexeme currentLex; // Текущая лексема (символ)

void getNextLexeme() { // Функция получения следующей лексемы (символа)
    currentLex = getchar();
}

int parseG() {
   if ( currentLex >= '0' && currentLex <= '9' ) {
       int res = currentLex - '0';
       getNextLexeme();
       return res;
   } else {
       throw "Number required";
   }
}

int parseF() {
    int res = parseG();

    while ( currentLex == '*' ) {
        getNextLexeme();

        res *= parseG();
    }
    
    return res;
}

int parseE() {
    int res = parseF();

    while ( currentLex == '+' ) {
        getNextLexeme();

        res += parseF();
    }
    
    return res;
}

int parseS() {
    int res = parseE();

    if ( currentLex != '\n' && currentLex != '\r' ) // Проверяем, что достигнут конец строки
        throw "End of line required";

    return res;
}

int main(int argc, char ** argv) {
    try {
        getNextLexeme(); // Считываем первую лексему
        int res = parseS(); // Парсим выражение
        std::cout << "Ok: " << res << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << ", but " << currentLex << " got." << std::endl;
    }
}
