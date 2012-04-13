/*
 * В этой программе осуществляется проверка соответствия входной строки языку выражений над константами 0 и 1
 *
 * Грамматика:
 *   S -> E // Дополнительное правило для конца цепочки
 *   E -> '0' | '1' | '!' E | '(' E '&' E ')'
 */

#include <iostream> // Используем стандартный ввод и вывод - подключаем заголовочный файл
#include <cstdio> // Для функции getchar

typedef char Lexeme; // Тип для лексемы (пока это просто символ, нужно для того, чтобы потом было проще перейти к использованию лексера)

Lexeme currentLex; // Текущая лексема (символ)

void getNextLexeme() { // Функция получения следующей лексемы (символа)
    currentLex = getchar();
}

void parseE() {
    if ( currentLex == '1' || currentLex == '0' ) { // Выражение может состоять из констант 0 или 1
        getNextLexeme();
    } else if ( currentLex == '!' ) { // Или из унарной операции отрицания
        getNextLexeme();

        parseE(); // Операнд
    } else if ( currentLex == '(' ) { // Или из бинарной операции в скобках
        getNextLexeme();

        parseE(); // Первый операнд

        if ( currentLex != '&' ) // Проверяем знак операции
            throw "& needed";
        getNextLexeme();

        parseE(); // Второй операнд

        if ( currentLex != ')' ) // Проверяем закрывающую скобку
            throw ") needed";
        getNextLexeme();
    } else {
        throw "Start of expression needed";
    }
}

void parseS() {
    parseE();

    if ( currentLex != '\n' && currentLex != '\r' ) // Проверяем, что достигнут конец строки
        throw "End of line needed";
}

int main(int argc, char ** argv) {
    try {
        getNextLexeme(); // Считываем первую лексему
        parseS(); // Парсим выражение
        std::cout << "Ok" << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << ", but " << currentLex << " got." << std::endl;
    }
}
