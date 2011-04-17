//#include "stdafx.h"
#include <iostream> // Для ввода-вывода
#include <string> // Для std::string
#include <map> // Для std::map
#include <ctype.h> // Для функции isalpha, определяющей является ли символ буквой
#include <string.h> // Для функции сравнения строк strcmp
#include <stdlib.h>

/*
  В этой программе производим разбор и вычисление арифметического выражения с
  переменными, значения которых вводятся с клавиатуры пользователем перед вводом
  выражения.
  
  В выражении доступны операции +/-/* и учитывается их правильный приоритет.
  Грамматика:

  E0 -> E1 { (+|-) E1 }
  E1 -> E2 { * E2 }
  E2 -> '(' E0 ')' | number | var
*/
 
/* Сначала необходимо построить лексический анализатор, который разбивает входной
поток на лексемы и классифицирует их.
В нашей грамматике можно выделить два основных типа лексем - константы
(состоящие из букв) и разделители (из неалфавитных символов). К разделителям здесь
относятся скобки и знаки операций (*,+,-). Также введем
дополнительные типы для ошибки и конца ввода. */
enum LexType {
    LEX_NULL, // Тип для нераспознанных лексем - ошибок
    LEX_CONST, // Константы, состоящие из цифр
    LEX_VAR, // Переменные, начинающиеся с буквы
    LEX_DELIM, // Разделители, состояще из неалфавитных символов
    LEX_EOF // Конец ввода
};
 
// Какие бывают разделители:
enum LexDelims {
    LEX_DEL_NULL, // Для ошибки
    LEX_DEL_ADD, // Сложение
    LEX_DEL_SUB, // Вычитание
    LEX_DEL_MUL, // Умножение
    LEX_DEL_BROPEN, // Открывающая скобка
    LEX_DEL_BRCLOSE // Закрывающая скобка
};
 
// Описываем класс, представляющий лексему
class Lexeme {
public:
    // У него 4 поля:
    int type; // Тип лексемы (константа, разделитель, ошибка, конец)
    int index; // Конкретный индекс лексемы (по соответсвующим таблицам для типов)
    
    std::string buf; // Строка, из которой была получена лексема - для выдачи диагностических сообщений
    
    int value; // Значение лексемы - используется для констант
    
    // Здесь объявляем конструктор лексемы
    Lexeme( int type = LEX_NULL, int index = LEX_NULL, const std::string & buf = "", int value = 0 ) :
      type( type ), // Инициализируем поле type значением одноименного параметра
      index( index ), // Аналогично для index и остальных полей
      buf( buf ),
      value( value ) {}
 
};

// Вспомогательные структуры объявлены, пора переходить к разбору
 
char currentChar; // Переменная для текущего символа
 
void gc() { // Функция чтения следующего символа
    std::cin.get( currentChar );
}
 
 
/* Теперь сама функция для лексического анализа, выделяющая из входного 
потока символов очередную лексему. Она предполагает, что при ее вызове
текущий символ - первый символ лексемы, после ее завершения текущий
символ - следующий за лексемой.
Возвращает она как раз найденную лексему.

А это регулярная грамматика, на основе которой производится разбор, здесь
\w обозначает букву, \d - цифру, а \s - пробельный символ (по аналогии с регулярными выражениями):
 
S -> \w W | \d D | + | - | * | ) | ( | $ | \s S
D -> \d D |
W -> \w W | \d W

*/
Lexeme readNextLexeme() {
    enum State { S, W, D }; // Итак, у нас есть три состояния - дополнительных здесь объявлять не будем
    
    State currentState = S; // Переменная для состояния, начальное состояние - S
    
    std::string buf; // Это строка-буфер, в котором будут накапливаться символы, образующие лексему
    
    while (true) { // Цикл обработки
        switch (currentState) { // В зависимости от текущего состояния
            case S: // Если мы в начальном состоянии
                 // Если у нас здесь пробельный символ, то мы его просто пропускаем
                if ( currentChar == ' ' ) {
                    gc(); // То есть считываем следующий
                    currentState = S; // И остаемся в том же состоянии
                } else if ( isalpha( currentChar ) ) { // Если текущий символ - буква
                    buf += currentChar; // Тогда добавляем ее в буфер-накопитель лексемы
                    gc(); // Считываем следующий символ
                    currentState = W; // И переходим в состояние W - для переменных
                } else if ( isdigit( currentChar ) ) { // Если символ - цифра, то начинаем считывать число
                    buf += currentChar; // Добавляем его в строку-буфер
                    gc(); // Считываем следующий символ
                    currentState = D; // И переходим в состояние D
                } else if ( currentChar == '+' ) { // Сложение
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_ADD, "+" ); 
                } else if ( currentChar == '-' ) { // Вычитание
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_SUB, "-" ); 
                } else if ( currentChar == '*' ) { // Произведение
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_MUL, "*" ); 
                } else if ( currentChar == '(' ) { // Открывающаяся скобка
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_BROPEN, "(" );
                } else if ( currentChar == ')' ) { // Закрывающаяся скобка
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_BRCLOSE, ")" );
                } else if ( currentChar == '$' ) { // Если символ - конец ввода
                    return Lexeme( LEX_EOF, LEX_NULL, "$" ); // То возвращаем лексему конца ввода
                } else { // Иначе - какой-то непонятный символ
                    buf += currentChar; // Добавляем его в строку-буфер
                    return Lexeme( LEX_NULL, LEX_NULL, buf ); // И возвращаем ошибочную лексему
                }
                break;
            case W: // Состояние распознавания переменных
                if ( isalpha( currentChar ) || isdigit( currentChar ) ) { // Если символ - буква или цифра
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий
                    currentState = W; // И остаемся в том же состоянии
                } else { // Другой символ - значит переменная уже закончилась
                    return Lexeme( LEX_VAR, 0, buf );
                }
                break;
            case D: // Состояние распознавания двухсимвольного разделителя
                if ( isdigit( currentChar ) ) { // Если символ - цифра
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий
                    currentState = D; // И остаемся в том же состоянии
                } else { // Иначе
                    return Lexeme( LEX_CONST, 0, buf, atoi( buf.c_str() ) ); // И возвращаем ошибочную лексему
                }
                break;
        };
    }
}
 
Lexeme currentLex; // Текущая лексема
 
void getNextLexeme() { // Функция получения следующей лексемы
    currentLex = readNextLexeme();
}

/*
В параметр функций передаем ассоциативный массив с переменными.
Передаем по константной ссылке, чтобы избежать копирования.

В принципе можно было бы использовать глобальную переменную.
*/
int parseE0( const std::map<std::string,int> & vars );

int parseE2( const std::map<std::string,int> & vars ) {
    if ( currentLex.type == LEX_CONST ) {
        int r = currentLex.value; // Определяем значение терминального выражения
        getNextLexeme();        
        return r;
    } else if ( currentLex.type == LEX_VAR ) {
        int r = vars.find( currentLex.buf )->second;
        getNextLexeme();
        return r;
    } else if ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_BROPEN ) {
        getNextLexeme();
        int r = parseE0( vars );        
         
        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BRCLOSE ) // Проверяем закрывающую скобку
            throw "Closing bracket required";

        getNextLexeme();
        return r;
    } else {
        throw "Expression (E2) required";
    }
}

int parseE1( const std::map<std::string,int> & vars ) {
    int r = parseE2( vars );
    
    while ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_MUL ) {
        getNextLexeme();
        r *= parseE2( vars );
    }
    
    return r;
}

int parseE0( const std::map<std::string,int> & vars ) {
    int r = parseE1( vars );
    
    while ( currentLex.type == LEX_DELIM && ( currentLex.index == LEX_DEL_ADD || currentLex.index == LEX_DEL_SUB ) ) {
        if ( currentLex.index == LEX_DEL_ADD ) {        
            getNextLexeme();
        
            r += parseE1( vars );
        } else if ( currentLex.index == LEX_DEL_SUB ) {
            getNextLexeme();
        
            r -= parseE1( vars );
        } else {
            throw "ADD or SUB required";
        }
    }
    
    return r;
}
 
int parseS( const std::map<std::string,int> & vars ) {
    int r = parseE0( vars );
    
    if ( currentLex.type != LEX_EOF ) // Проверяем конец цепочки
        throw "End of line needed";
    
    return r;
}
 
int main(int argc, char ** argv) {
    std::map<std::string,int> vars; // Ассоциативный массив со значениями переменных
    
    while ( true ) {
        std::string varName; // Имя переменной     
        
        std::cout << "Enter var name ($ to end): ";
        std::cin >> varName; // Вводим имя переменной
        
        if ( varName == "$" )
            break;
        
        int varValue;
        std::cout << "Enter var value: ";
        std::cin >> varValue; // Вводим значение
        
        vars.insert(std::make_pair(varName,varValue)); // Добавляем пару имя переменной-значение
    }
    
    std::cout << "Enter expression: ";
    
    try {
        std::cin >> currentChar; // Считываем первый символ
        getNextLexeme(); // Считываем первую лексему
        int result = parseS( vars ); // Парсим и вычисляем выражение
        std::cout << "Calculated: " << result << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << std::endl;
        std::cout << "Got: {" << currentLex.type // Печатаем ее тип
                  << "," << currentLex.index // Индекс
                  << "," << currentLex.buf // Строку, из которой она получена
                  << "," << currentLex.value // Булево значение
                  << "}" << std::endl; 
    }
}
