#include <iostream>

// S -> E // Дополнительное правило для конца цепочки
// E -> '0' | '1' | '!' E | '(' E '&' E ')'

typedef char Lexeme; // Тип для лексемы

Lexeme currentLex; // Текущая лексема

void getNextLexeme() { // Функция получения следующей лексемы 
    std::cin >> currentLex;
}

bool parseE() {
    if ( currentLex == '1' || currentLex == '0' ) {
        bool r = currentLex == '1'; // Определяем значение терминального выражения
        getNextLexeme();     
        
        return r;
    } else if ( currentLex == '!' ) {
        getNextLexeme();
        
        return !parseE(); // Операнд
    } else if ( currentLex == '(' ) {
        getNextLexeme();
        
        bool r1 = parseE(); // Первый операнд
        
        if ( currentLex != '&' ) // Проверяем знак операции
            throw "& needed";
        getNextLexeme();
        
        bool r2 = parseE(); // Второй операнд
        
        if ( currentLex != ')' ) // Проверяем закрывающую скобку
            throw ") needed";
        getNextLexeme();
        
        return r1 && r2;
    } else {
        throw "Start of expression needed";
    }
}

bool parseS() {
    bool r = parseE();
    
    if ( currentLex != '$' ) // Проверяем конец цепочки
        throw "End of line needed";
    
    return r;
}

int main(int argc, char ** argv) {
    try {
        getNextLexeme(); // Считываем первую лексему
        bool result = parseS(); // Парсим  и вычисляем выражение
        std::cout << "Calculated: " << result << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << ", but " << currentLex << " got." << std::endl;
    }
}