//#include "stdafx.h"
#include <iostream> // Для ввода-вывода
#include <string> // Для std::string
#include <ctype.h> // Для функции isalpha, определяющей является ли символ буквой
#include <string.h> // Для функции сравнения строк strcmp
 
/* Итак, необходимо построить лексический анализатор, который разбивает входной
поток на лексемы и классифицирует их.
В нашей грамматике можно выделить два основных типа лексем - константы
(состоящие из букв) и разделители (из неалфавитных символов). К разделителям здесь
относятся скобки и знаки операций (! и &&). Также введем
дополнительные типы для ошибки и конца ввода. */
enum LexType {
    LEX_NULL, // Тип для нераспознанных лексем - ошибок
    LEX_CONST, // Константы, состоящие из букв
    LEX_DELIM, // Разделители, состояще из неалфавитных символов
    LEX_EOF // Конец ввода
};
 
// Здесь мы определяем, какие у нас бывают константы
enum LexConsts {
    LEX_CONST_NULL, // Это для ошибки
    LEX_CONST_TRUE, // true
    LEX_CONST_FALSE // false
};
 
// Какие бывают разделители:
enum LexDelims {
    LEX_DEL_NULL, // Для ошибки
    LEX_DEL_AND, // Операция "И"
    LEX_DEL_NOT, // Операция "НЕ"
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
    
    bool value; // Значение лексемы - используется для констант
    
    // Здесь объявляем конструктор лексемы
    Lexeme( int type = LEX_NULL, int index = LEX_NULL, const std::string & buf = "", bool value = false ) :
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
\w обозначает букву, а \s пробельный символ (по аналогии с регулярными выражениями):
 
S -> \w W | & D | ! | ) | ( | $ | \s S
W -> \w W |
D -> &

Первая альтернатива в S - выделение констант (из алфавитных символов), вторая - 
двусимвольного разделителя (&&), далее - разделители (отрицание, скобки) и конец
ввода. Последняя альтернатива в S - это правило для пропуска пробелов.

W выделяет константы (цепочки букв), а D - двухсимвольный разделитель (&&).
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
                    currentState = W; // И переходим в состояние W - для констант
                } else if ( currentChar == '&' ) { // Если символ - & - то есть часть операции "И"
                    buf += currentChar; // Добавляем его в строку-буфер
                    gc(); // Считываем следующий символ
                    currentState = D; // И переходим в состояние D
                } else if ( currentChar == '!' ) { // Отрицание
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_NOT, "!" ); 
                } else if ( currentChar == '(' ) { // Открывающаяся скобка
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_BROPEN, "(" );
                } else if ( currentChar == ')' ) { // Если же закрывающаяся скобка
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_BRCLOSE, ")" ); // И возвращаем соответствующую лексему
                } else if ( currentChar == '$' ) { // Если символ - конец ввода
                    return Lexeme( LEX_EOF, LEX_NULL, "$" ); // То возвращаем лексему конца ввода
                } else { // Иначе - какой-то непонятный символ
                    buf += currentChar; // Добавляем его в строку-буфер
                    return Lexeme( LEX_NULL, LEX_NULL, buf ); // И возвращаем ошибочную лексему
                }
                break;
            case W: // Состояние распознавания констант
                if ( isalpha( currentChar ) ) { // Если символ - буква
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий
                    currentState = W; // И остаемся в том же состоянии
                } else { // Другой символ - значит константа уже закончилась
                    if ( strcmp( buf.c_str(), "true" ) == 0 )
                        return Lexeme( LEX_CONST, LEX_CONST_TRUE, "true", true );
                    else if ( strcmp( buf.c_str(), "false" ) == 0 )
                        return Lexeme( LEX_CONST, LEX_CONST_FALSE, "false", false );
                    else
                        return Lexeme( LEX_NULL, LEX_NULL, buf );
                }
                break;
            case D: // Состояние распознавания двухсимвольного разделителя
                if ( currentChar == '&' ) { // Он у нас один - &&, и если это он
                    gc(); // То, считываем следующий символ
                    return Lexeme( LEX_DELIM, LEX_DEL_AND, "&&" ); // И возвращаем соответствующую лексему
                } else { // Иначе
                    buf += currentChar; // Добавляем символ в строку-буфер
                    return Lexeme( LEX_NULL, LEX_NULL, buf ); // И возвращаем ошибочную лексему
                }
                break;
        };
    }
}
 
Lexeme currentLex; // Текущая лексема
 
void getNextLexeme() { // Функция получения следующей лексемы
    currentLex = readNextLexeme();
}
 
bool parseE() {
    if ( currentLex.type == LEX_CONST ) {
        bool r = currentLex.value; // Определяем значение терминального выражения
        getNextLexeme();
        
        return r;
    } else if ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_NOT ) {
        getNextLexeme();
        
        return !parseE(); // Операнд
    } else if ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_BROPEN ) {
        getNextLexeme();
        
        bool r1 = parseE(); // Первый операнд
        
        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_AND ) // Проверяем знак операции
            throw "& needed";
        getNextLexeme();
        
        bool r2 = parseE(); // Второй операнд
        
        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BRCLOSE ) // Проверяем закрывающую скобку
            throw ") needed";
        getNextLexeme();
        
        return r1 && r2;
    } else {
        throw "Start of expression needed";
    }
}
 
bool parseS() {
    bool r = parseE();
    
    if ( currentLex.type != LEX_EOF ) // Проверяем конец цепочки
        throw "End of line needed";
    
    return r;
}
 
int main(int argc, char ** argv) {
    try {
        gc(); // Считываем первый символ
        getNextLexeme(); // Считываем первую лексему
        bool result = parseS(); // Парсим и вычисляем выражение
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
