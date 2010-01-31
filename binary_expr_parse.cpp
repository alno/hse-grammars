#include <iostream>

// S -> E // Дополнительное правило для конца цепочки
// E -> '0' | '1' | '!' E | '(' E '&' E ')'

typedef char Lexeme; // Тип для лексемы

Lexeme currentLex; // Текущая лексема

void getNextLexeme() { // Функция получения следующей лексемы 
    std::cin >> currentLex;
}

void parseExpr() {
    if ( currentLex == '1' || currentLex == '0' ) {
        getNextLexeme();     
    } else if ( currentLex == '!' ) {
        getNextLexeme();
        parseExpr();
    } else if ( currentLex == '(' ) {
        getNextLexeme();
        parseExpr();
        if ( currentLex != '&' )
            throw currentLex;
        getNextLexeme();
        parseExpr();
        if ( currentLex != ')' )
            throw currentLex;
        getNextLexeme();
    } else {
        throw currentLex;
    }
}

void parseGrammar() {
    parseExpr();
    if ( currentLex != '$' ) // Проверяем конец цепочки
        throw currentLex;
}

int main(int argc, char ** argv) {
    try {
        getNextLexeme(); // Считываем первую лексему
        parseGrammar(); // Парсим выражение
        std::cout << "Parsed" << std::endl;
    } catch ( Lexeme lex ) {
        std::cout << "Error parsing: " << lex << std::endl;
    }
}