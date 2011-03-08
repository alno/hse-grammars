#include <iostream>

// S -> E $ // Дополнительное правило для конца цепочки
// E -> '0' | '1' | '!' E | '(' E '&' E ')'

typedef char Lexeme; // Тип для лексемы

Lexeme currentLex; // Текущая лексема

void getNextLexeme() { // Функция получения следующей лексемы 
    std::cin >> currentLex;
}

/*
 * Функция разбора нетерминала E (выражения). Возвращает результат вычисления разобранного выражения
 */
bool parseE() {
    if ( currentLex == '1' || currentLex == '0' ) { // Константа 0 или 1
        bool r = currentLex == '1'; // Определяем значение терминального выражения
        getNextLexeme(); // Считываем следующую лексему
        
        return r;
    } else if ( currentLex == '!' ) { // Унарная операция - отрицание
        getNextLexeme();
        
        return !parseE(); // Операнд
    } else if ( currentLex == '(' ) { // Бинарная операция в скобках - И
        getNextLexeme(); // Считываем следующую за скобкой лексему
        
        bool r1 = parseE(); // С нее начинается первый операнд
        
        if ( currentLex != '&' ) // Проверяем знак операции
            throw "& needed";
        getNextLexeme(); // Считываем следующую за знаком операции лексему
        
        bool r2 = parseE(); // С нее начинается второй операнд
        
        if ( currentLex != ')' ) // Проверяем закрывающую скобку
            throw ") needed";
        getNextLexeme(); // Считываем следующую за скобкой лексему
        
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