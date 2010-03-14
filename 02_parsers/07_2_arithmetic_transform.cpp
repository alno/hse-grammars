#include <iostream>
#include <ctype.h> // Для функции isdigit, определяющей является ли символ цифрой
 
// S -> E // Дополнительное правило для конца цепочки
// E -> '0' | ... | '9' | '(' E '+' E ')' | '(' E '*' E ')'
 
typedef char Lexeme; // Тип для лексемы
 
Lexeme currentLex; // Текущая лексема
 
void getNextLexeme() { // Функция получения следующей лексемы
    std::cin >> currentLex;
}
 
void parseE() {
    if ( isdigit(currentLex) ) {
        std::cout << currentLex << " "; // Выводим цифру на печать
        
        getNextLexeme();
    } else if ( currentLex == '(' ) {
        getNextLexeme();
        
        parseE(); // Первый операнд
        
        if ( currentLex != '+' && currentLex != '*' ) // Проверяем знак операции
            throw "& needed";
        
        Lexeme operation = currentLex; // Запоминаем текущую операцию
        
        getNextLexeme();        
        
        parseE(); // Второй операнд
        
        if ( currentLex != ')' ) // Проверяем закрывающую скобку
            throw ") needed";
        
        getNextLexeme();
        
        std::cout << operation << " "; // Выводим операцию
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