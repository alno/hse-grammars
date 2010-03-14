//#include "stdafx.h"
#include <ctype.h> // Для функции isdigit, определяющей является ли символ цифрой
#include <string.h> // Для функции сравнения строк strcmp
#include <stdlib.h> // Для atoi

#include <iostream> // Для ввода-вывода (std::сin и std::cout)
#include <string> // Для std::string
#include <vector> // Для std::vector
#include <stack> // Для std::stack
 
/* Итак, необходимо построить лексический анализатор, который разбивает входной
поток на лексемы и классифицирует их.
В нашей грамматике можно выделить два основных типа лексем - числовые константы
(состоящие из цифр) и разделители (из неалфавитных символов). К разделителям здесь
относятся скобки и знаки операций (! и &&). Также введем
дополнительные типы для ошибки и конца ввода. */
enum LexType {
    LEX_NULL, // Тип для нераспознанных лексем - ошибок
    LEX_NUMBER, // Числа
    LEX_DELIM, // Разделители, состояще из неалфавитных символов
    LEX_EOF // Конец ввода
};

// Какие бывают разделители:
enum LexDelims {
    LEX_DEL_NULL, // Для ошибки
    LEX_DEL_ADD, // Операция "+"
    LEX_DEL_MUL, // Операция "*"
    LEX_DEL_BROPEN, // Открывающая скобка
    LEX_DEL_BRCLOSE // Закрывающая скобка
};
 
/* Перечисляем как записываются разделители - опять же порядок совпадает
с описанием перечисления LexDelims */
const char * LEX_DELIMS[] = {
    "",
    "+",
    "*",
    "(",
    ")",
    0 // Заканчиваем список нулем, чтобы при поиске определять по нему конец
};
 
// Описываем класс, представляющий лексему
class Lexeme {
public:
    // У него 4 поля:
    int type; // Тип лексемы (константа, разделитель, ошибка, конец)
    int index; // Конкретный индекс лексемы (по соответсвующим таблицам для типов)
    
    std::string buf; // Строка, из которой была получена лексема - для выдачи диагностических сообщений
    
    int value; // Значение лексемы - используется для числовых констант
    
    // Здесь объявляем конструктор лексемы
    Lexeme( int type = LEX_NULL, int index = LEX_NULL, const std::string & buf = "", int value = 0 ) :
      type( type ), // Инициализируем поле type значением одноименного параметра
      index( index ), // Аналогично для index и остальных полей
      buf( buf ),
      value( value ) {}
 
};
 
/* Функция для поиска строки в списке
Она принимает первым аргументом строку C, а вторым -
список строк, и если в этом списке есть такая же строка, как в
первом аргументе, то возвращает ее индекс, а иначе - 0 */
int find( const char * buf, const char * list[] ) {
    int i = 0; // Это текущий индекс
    while ( list[i] != 0 ) { // Пока не встретили 0 (вспоминаем, в списках последний элемент как раз 0)
        if ( strcmp( list[i], buf ) == 0 ) // Если строка совпадает с текущим элементов
            return i; // То возвращаем его индекс
            
        i ++; // Ну а иначе, переходим к следующему элементу
    }
 
    return 0; // Ничего не нашли - возвращаем 0
}
 
// Вспомогательные структуры объявлены, пора переходить к разбору
 
char currentChar; // Переменная для текущего символа
 
void gc() { // Функция чтения следующего символа
    std::cin >> currentChar;
}
 
 
/* Теперь сама функция для лексического анализа, выделяющая из входного 
потока символов очередную лексему. Она предполагает, что при ее вызове
текущий символ - первый символ лексемы, после ее завершения текущий
символ - следующий за лексемой.
Возвращает она как раз найденную лексему.

А это регулярная грамматика, на основе которой производится разбор, здесь
\d обозначает цифру, а \s пробельный символ (по аналогии с регулярными выражениями):
 
S -> \d N | + | * | ) | ( | $ | \s S
N -> \d N |

Первая альтернатива в S - выделение чисел, далее - разделители и конец
ввода. Последняя альтернатива в S - это правило для пропуска пробелов.

N выделяет числовые константы
*/
Lexeme readNextLexeme() {
    enum State { S, N }; // Итак, у нас есть три состояния - дополнительных здесь объявлять не будем
    
    State currentState = S; // Переменная для состояния, начальное состояние - S
    
    std::string buf; // Это строка-буфер, в котором будут накапливаться символы, образующие лексему
    
    while (true) { // Цикл обработки
        switch (currentState) { // В зависимости от текущего состояния
            case S: // Если мы в начальном состоянии
                 // Если у нас здесь пробельный символ, то мы его просто пропускаем
                if ( currentChar == '_' ) {
                    gc(); // То есть считываем следующий
                    currentState = S; // И остаемся в том же состоянии
                } else if ( isdigit( currentChar ) ) { // Если текущий символ - цифра
                    buf += currentChar; // Тогда добавляем ее в буфер-накопитель лексемы
                    gc(); // Считываем следующий символ
                    currentState = N; // И переходим в состояние N - для числовых констант
                } else if ( currentChar == '+' || currentChar == '*' || currentChar == '(' || currentChar == ')' ) { // Если же это односимвольный разделитель
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий символ
                    
                    int index = find( buf.c_str(), LEX_DELIMS ); // Находим этот разделитель в таблице
                        
                    return Lexeme( LEX_DELIM, index, buf ); // И возвращаем соответствующую лексему
                } else if ( currentChar == '$' ) { // Если символ - конец ввода
                    return Lexeme( LEX_EOF, LEX_NULL, "$" ); // То возвращаем лексему конца ввода
                } else { // Иначе - какой-то непонятный символ
                    buf += currentChar; // Добавляем его в строку-буфер
                    return Lexeme( LEX_NULL, LEX_NULL, buf ); // И возвращаем ошибочную лексему
                }
                break;
            case N: // Состояние распознавания числовых констант
                if ( isdigit( currentChar ) ) { // Если символ - цифра
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий
                    currentState = N; // И остаемся в том же состоянии
                } else { // Другой символ - значит число уже закончилось                    
                    return Lexeme( LEX_NUMBER, LEX_NULL, buf, atoi( buf.c_str() ) ); // Просто возвращаем соответствующую константе лексему
                }
                break;
        };
    }
}

/* Это класс, представляющий операцию в ПОЛИЗ. Он содержит тип операции и дополнительные данные,
которые интерпретируются в зависимости от типа операции (например, для констант - значение, для 
бинарных операций - индекс действия)
*/
class Operation {
public:
    enum Type {
        CONST, // Операция загрузки константы в стек
        BINARY // Бинарная операция
    };
    
    Operation( int type, int data ) : type( type ), data( data ) {}
    
    int type; // Тип
    int data; // Дополнительные данные
};

Lexeme currentLex; // Текущая лексема
 
void getNextLexeme() { // Функция получения следующей лексемы
    currentLex = readNextLexeme(); // Обращаемся к лексическому анализатору
}

std::vector<Operation> program; // Программа - как список операций
 
void parseE() {
    if ( currentLex.type == LEX_NUMBER ) {
        program.push_back( Operation( Operation::CONST, currentLex.value ) ); // Добавляем константу в код программы
        
        getNextLexeme();
    } else if ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_BROPEN ) {
        getNextLexeme();
        
        parseE(); // Первый операнд
        
        if ( currentLex.type != LEX_DELIM || ( currentLex.index != LEX_DEL_ADD && currentLex.index != LEX_DEL_MUL ) ) // Проверяем знак операции
            throw "& needed";
        
        Lexeme operation = currentLex; // Запоминаем текущую операцию
        
        getNextLexeme();        
        
        parseE(); // Второй операнд
        
        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BRCLOSE ) // Проверяем закрывающую скобку
            throw ") needed";
        
        getNextLexeme();
        
        program.push_back( Operation( Operation::BINARY, operation.index ) ); // Добавляем операцию в код программы
    } else {
        throw "Start of expression needed";
    }
}
 
void parseS() {
    parseE();
    if ( currentLex.type != LEX_EOF ) // Проверяем конец цепочки
        throw "End of line needed";
}

/* Вычислить значение выражения */
int calculate() {
    std::stack<int> stack; // Стек значений, используемых при вычислении
    
    for ( int i = 0; i < program.size(); ++ i ) { // Для каждой операции
        Operation op = program[i];
        
        switch ( op.type ) { // В зависимости от типа
            case Operation::CONST: // Для константы
                stack.push( op.data ); // Кладем в стек значение
                break;
            case Operation::BINARY: // Для бинарной операции
            {
                int v2 = stack.top(); // Получаем второй аргумент из стека
                stack.pop(); // И удаляем его из стека
                
                int v1 = stack.top(); // Получаем первый аргумент из стека                
                stack.pop(); // И удаляем его из стека
                
                switch ( op.data ) { // В зависимости от конкретной операции
                    case LEX_DEL_ADD:
                        stack.push( v1 + v2 ); // Кладем в стек результат сложения
                        break;
                    case LEX_DEL_MUL:
                        stack.push( v1 * v2 ); // Кладем в стек результат умножения
                        break;
                    default:
                        throw "Unknown binary operation"; // Неизвестная бинарная операция
                }
                break;
            }
            default:
                throw "Unknown operation type"; // Неизвестный тип операции
        }
    }
    
    return stack.top(); // Возвращаем верхнее значение в стеке
}
 
int main(int argc, char ** argv) {
    try {
        std::cout << "Enter expression: " << std::endl;
        gc();
        getNextLexeme(); // Считываем первую лексему
        parseS(); // Парсим выражение
    } catch ( const char * err ) {
        std::cout << "Error parsing: " << err << ", but " 
             << "{" << currentLex.type // Печатаем ее тип
             << "," << currentLex.index // Индекс
             << "," << currentLex.buf // Строку, в которой она накоплена
             << "," << currentLex.value // Булево значение
             << "} got." << std::endl;
    }
    
    std::cout << "Expression parsed, calculating..." << std::endl;
    
    //for ( int i = 0; i < program.size(); ++ i ) {
    //    Operation op = program[i];
    //    std::cout << "{" << op.type << "," << op.value << "}" << std::endl;
    //}
    
    try {
        std::cout << "Result: " << calculate() << std::endl; // Вычисляем выражение
    } catch ( const char * err ) {
        std::cout << "Error calculating: " << err << std::endl;
    }
}
