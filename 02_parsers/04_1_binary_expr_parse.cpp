#include <iostream>

// S -> E // Дополнительное правило для конца цепочки
// E -> '0' | '1' | '!' E | '(' E '&' E ')'

typedef char Lexeme; // Тип для лексемы

Lexeme currentLex; // Текущая лексема

void getNextLexeme() { // Функция получения следующей лексемы
    std::cin >> currentLex;
}

void parseE() {
    if ( currentLex == '1' || currentLex == '0' ) {
        getNextLexeme();
    } else if ( currentLex == '!' ) {
        getNextLexeme();

        parseE(); // Операнд
    } else if ( currentLex == '(' ) {
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
    if ( currentLex != '$' ) // Проверяем конец цепочки
        throw "End of line needed";
}

int main(int argc, char ** argv) {
    try {
        getNextLexeme(); // Считываем первую лексему
        parseS(); // Парсим выражение
        std::cout << "Parsed" << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << ", but " << currentLex << " got." << std::endl;
    }
}
