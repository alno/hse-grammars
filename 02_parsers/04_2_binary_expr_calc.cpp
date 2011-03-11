#include <iostream>

// S -> E $ // Дополнительное правило для конца цепочки
// E -> '0' | '1' | '!' E | '(' E '&' E ')'

char c; // Текущий символ

void getNextLexeme() { // Функция получения следующей лексемы 
    std::cin >> c;
}

/*
 * Функция разбора и вычисления нетерминала E (выражения). Возвращает результат вычисления разобранного выражения
 */
bool parseE() {
    if ( c == '1' || c == '0' ) { // Константа 0 или 1
        bool r = c == '1'; // Определяем значение терминального выражения - true если символ 1, false если 0
        getNextLexeme(); // Считываем следующую лексему
        
        return r;
    } else if ( c == '!' ) { // Унарная операция - отрицание
        getNextLexeme();
        
        return !parseE(); // Разбираем и вычисляем операнд
    } else if ( c == '(' ) { // Бинарная операция в скобках - операция И
        getNextLexeme(); // Считываем следующую за скобкой лексему, с нее начинается первый операнд
        
        bool r1 = parseE(); // Разбираем и вычисляем первый операнд
        
        if ( c != '&' ) // Проверяем знак операции
            throw "& needed";
        getNextLexeme(); // Считываем следующую за знаком операции лексему, с нее начинается второй операнд
        
        bool r2 = parseE(); // Разбираем и вычисляем второй операнд
        
        if ( c != ')' ) // Проверяем закрывающую скобку
            throw ") needed";
        getNextLexeme(); // Считываем следующую за скобкой лексему
        
        return r1 && r2; // Вычисляем результат операции
    } else {
        throw "Start of expression needed";
    }
}

bool parseS() {
    bool r = parseE();
    
    if ( c != '$' ) // Проверяем конец цепочки
        throw "End of line needed";
    
    return r;
}

// argc - количество аргументов, передаваемых программе через командную строку
// argv - массив аргументов
// В этой программе мы их не используем
int main(int argc, char ** argv) {
    try {
        getNextLexeme(); // Считываем первую лексему
        bool result = parseS(); // Парсим  и вычисляем выражение
        std::cout << "Calculated: " << result << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << ", but " << c << " got." << std::endl;
    }

    return 0;
}
