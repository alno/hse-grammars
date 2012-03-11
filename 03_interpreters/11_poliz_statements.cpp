//#include "stdafx.h"
#include <ctype.h> // Для функции isdigit, определяющей является ли символ цифрой
#include <string.h> // Для функции сравнения строк strcmp
#include <stdlib.h> // Для atoi
#include <stdio.h> // Для getchar

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
    LEX_KEYWORD, // Ключевые слова
    LEX_VAR, // Переменные
    LEX_EOF // Конец ввода
};

// Какие бывают разделители:
enum LexDelims {
    LEX_DEL_NULL, // Для ошибки
    LEX_DEL_ADD, // Операция "+"
    LEX_DEL_SUB, // Операция "-", не важно бинарный или унарный
    LEX_DEL_MUL, // Операция "*"
    LEX_DEL_BROPEN, // Открывающая скобка
    LEX_DEL_BRCLOSE, // Закрывающая скобка
    LEX_DEL_BLOPEN,
    LEX_DEL_BLCLOSE,
    LEX_DEL_EQUALS, // Знак равенства
    LEX_DEL_SEMICOLON, // Точка с запятой
    LEX_DEL_LESS, // Меньше
    LEX_DEL_MORE // Больше
};

/* Перечисляем как записываются разделители - опять же порядок совпадает
с описанием перечисления LexDelims */
const char * LEX_DELIMS[] = {
    "",
    "+",
    "-", // Строка для минуса
    "*",
    "(",
    ")",
    "{",
    "}",
    "=",
    ";",
    "<", // Сравнение на меньше
    ">", // Сравнение на меньше
    0 // Заканчиваем список нулем, чтобы при поиске определять по нему конец
};

// Какие бывают ключевые слова:
enum LexKeywords {
    LEX_KW_NULL, // Для ошибки
    LEX_KW_VAR, // Объявление переменной
    LEX_KW_IF // Условие
};

/* Перечисляем как записываются разделители - опять же порядок совпадает
с описанием перечисления LexDelims */
const char * LEX_KEYWORDS[] = {
    "",
    "var", // Объявление переменной
    "if", // Условие
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
    currentChar = getchar();
}


/* Теперь сама функция для лексического анализа, выделяющая из входного
потока символов очередную лексему. Она предполагает, что при ее вызове
текущий символ - первый символ лексемы, после ее завершения текущий
символ - следующий за лексемой.
Возвращает она как раз найденную лексему.

А это регулярная грамматика, на основе которой производится разбор, здесь
\d обозначает цифру, а \s пробельный символ (по аналогии с регулярными выражениями):

S -> \d N | \w W | + | - | * | ) | ( | $ | \s S
N -> \d N |
W -> \w W |

Первая альтернатива в S - выделение чисел, далее - разделители и конец
ввода. Последняя альтернатива в S - это правило для пропуска пробелов.

N выделяет числовые константы
*/
Lexeme readNextLexeme() {
    enum State { S, N, W }; // Итак, у нас есть три состояния - дополнительных здесь объявлять не будем

    State currentState = S; // Переменная для состояния, начальное состояние - S

    std::string buf; // Это строка-буфер, в котором будут накапливаться символы, образующие лексему

    while (true) { // Цикл обработки
        switch (currentState) { // В зависимости от текущего состояния
            case S: // Если мы в начальном состоянии
                 // Если у нас здесь пробельный символ, то мы его просто пропускаем
                if ( currentChar == '_' || currentChar == ' ' ) {
                    gc(); // То есть считываем следующий
                    currentState = S; // И остаемся в том же состоянии
                } else if ( isdigit( currentChar ) ) { // Если текущий символ - цифра
                    buf += currentChar; // Тогда добавляем ее в буфер-накопитель лексемы
                    gc(); // Считываем следующий символ
                    currentState = N; // И переходим в состояние N - для числовых констант
                } else if ( isalpha( currentChar ) ) { // Если текущий символ - буква
                    buf += currentChar; // Тогда добавляем ее в буфер-накопитель лексемы
                    gc(); // Считываем следующий символ
                    currentState = W; // И переходим в состояние W - для ключевых слов и переменных
                } else if ( currentChar == '$' ) { // Если символ - конец ввода
                    return Lexeme( LEX_EOF, LEX_NULL, "$" ); // То возвращаем лексему конца ввода
                } else { // Может быть это односимвольный разделитель?
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий символ

                    int index = find( buf.c_str(), LEX_DELIMS ); // Находим этот разделитель в таблице

                    if ( index != 0 )
                        return Lexeme( LEX_DELIM, index, buf ); // И возвращаем соответствующую лексему
                    else
                        return Lexeme( LEX_NULL, LEX_NULL, buf ); // Нет такого разделителя
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
            case W: // Состояние распознавания ключевых слов и переменных
                if ( isalpha( currentChar ) ) { // Если символ - буква
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий
                    currentState = W; // И остаемся в том же состоянии
                } else { // Другой символ - значит имя уже закончилось
                    int index = find( buf.c_str(), LEX_KEYWORDS ); // Находим ключевое слово в таблице

                    if ( index != 0 )
                        return Lexeme( LEX_KEYWORD, index, buf ); // Просто возвращаем соответствующую константе лексему
                    else
                        return Lexeme( LEX_VAR, LEX_NULL, buf );
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
        BINARY, // Бинарная операция
        UNARY, // Унарная операция
        ASSIGN, // Присваивание
        VAR, // Получение значения переменной
        JMP_FALSE // Переход по false
    };

    Operation( int type, int data ) : type( type ), data( data ) {}

    int type; // Тип
    int data; // Дополнительные данные
};

Lexeme currentLex; // Текущая лексема

/* Функция получения следующей лексемы, тут синтаксический интерпретатор связан с лексическим */
void getNextLexeme() {
    currentLex = readNextLexeme(); // Обращаемся к лексическому анализатору
}

std::vector<std::string> variables; // Объявленные переменные
std::vector<Operation> program; // Программа - как список операций ПОЛИЗ

/* Функция для поиска переменной в списке объялвенных.
Если переменная объявлена, то возвращается ее индекс, иначе -1 */
int findVariable( const std::string & var ) {
    for ( int i = 0; i < variables.size(); ++ i )
        if ( variables[i] == var )
            return i;

    return -1;
}

/* Тип выражения */
enum ExpType {
    EXP_INT,
    EXP_BOOL
};

/* Разбор выражения */
ExpType parseE() {
    if ( currentLex.type == LEX_NUMBER ) { // Если текущая лексема - число
        program.push_back( Operation( Operation::CONST, currentLex.value ) ); // Добавляем константу в код программы

        getNextLexeme();

        return EXP_INT; // Константы всегда целочисленные
    } else if ( currentLex.type == LEX_VAR ) { // Текущая лексема - переменная
        int varIndex = findVariable( currentLex.buf ); // Находим переменную в списке объявленных

        if ( varIndex < 0 )
            throw "Unknown variable";

        program.push_back( Operation( Operation::VAR, varIndex ) );

        getNextLexeme();

        return EXP_INT; // Переменные считаем всегда целочисленными
    } else if ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_SUB ) { // Если текущая лексема - минус
        Lexeme operation = currentLex; // Запоминаем текущую операцию

        getNextLexeme(); // Считываем следующую лексему
        ExpType argType = parseE(); // И разбираем выражение-аргумент

        if ( argType != EXP_INT )
            throw "Int expression required";

        program.push_back( Operation( Operation::UNARY, operation.index ) ); // Добавляем операцию (унарный минус) в ПОЛИЗ

        return EXP_INT; // Унарный минус дает целое число
    } else if ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_BROPEN ) { // Если текущая лексема - скобка
        getNextLexeme();

        ExpType argType1 = parseE(); // Первый операнд

        if ( currentLex.type != LEX_DELIM || ( currentLex.index != LEX_DEL_ADD && currentLex.index != LEX_DEL_MUL && currentLex.index != LEX_DEL_SUB && currentLex.index != LEX_DEL_LESS && currentLex.index != LEX_DEL_MORE ) ) // Проверяем знак операции
            throw "& needed";

        Lexeme operation = currentLex; // Запоминаем текущую операцию

        getNextLexeme();

        ExpType argType2 = parseE(); // Второй операнд

        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BRCLOSE ) // Проверяем закрывающую скобку
            throw ") needed";

        getNextLexeme();

        program.push_back( Operation( Operation::BINARY, operation.index ) ); // Добавляем операцию в ПОЛИЗ

        if ( argType1 != EXP_INT || argType2 != EXP_INT ) // Все операции требуют целочисленных аргументов
            throw "Int expression required";

        if ( operation.index == LEX_DEL_LESS || operation.index == LEX_DEL_MORE ) // Если операции сравнения
            return EXP_BOOL; // То результат - булево значение
        else
            return EXP_INT; // Иначе число
    } else { // Иначе ошибка
        throw "Start of expression needed";
    }
}

void parseP();

void parseQ() {
    if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BLOPEN )
        throw "Block open expected";

    do {
        getNextLexeme();

        parseP();
    } while ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_SEMICOLON );

    if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BLCLOSE )
        throw "BLock close needed";

    getNextLexeme();
}

/* Разбор команды программы */
void parseP() {
    if ( currentLex.type == LEX_KEYWORD && currentLex.index  == LEX_KW_VAR ) { // Объявление переменной
        getNextLexeme();

        if ( currentLex.type != LEX_VAR )
            throw "Variable expected";

        variables.push_back( currentLex.buf );

        getNextLexeme();
    } else if ( currentLex.type == LEX_VAR ) {
        int varIndex = findVariable( currentLex.buf ); // Находим переменную в списке

        if ( varIndex < 0 )
            throw "Unknown variable";

        getNextLexeme();

        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_EQUALS )
            throw "Assignment expected";

        getNextLexeme();

        parseE();

        program.push_back( Operation( Operation::ASSIGN, varIndex ) );
    } else if ( currentLex.type == LEX_KEYWORD && currentLex.index == LEX_KW_IF ) {
        getNextLexeme();

        parseE(); // Выражение в условии

        program.push_back( Operation( Operation::JMP_FALSE, 0 ) );
        int jumpPos = program.size() - 1; // Запоминаем позицию операции

        parseQ(); // THEN-ветвь

        program[jumpPos].data = program.size();
    } else {
        throw "Program statement expected";
    }
}

/* Разбор для начального состояния грамматики - последовательность команд, затем одно выражение*/
void parseS() {
    while ( currentLex.type == LEX_VAR || currentLex.type == LEX_KEYWORD ) { // Список команд
        parseP();

        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_SEMICOLON )
            throw "; needed";

        getNextLexeme();
    }

    parseE(); // Заключительное выражение

    if ( currentLex.type != LEX_EOF ) // Проверяем конец цепочки
        throw "End of line needed";
}

/* Вычислить значение выражения */
int calculate() {
    std::stack<int> stack; // Стек значений, используемых при вычислении
    std::vector<int> values;

    values.resize( variables.size() );

    int currOp = 0;
    while ( currOp < program.size() ) { // Для каждой операции
        Operation op = program[currOp];
        int nextOp = currOp + 1; // Следующая операция - следующая по порядку

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
                    case LEX_DEL_SUB:
                        stack.push( v1 - v2 ); // Кладем в стек результат разности
                        break;
                    case LEX_DEL_LESS:
                        stack.push( v1 < v2 ); // Кладем в стек результат разности
                        break;
                    case LEX_DEL_MORE:
                        stack.push( v1 > v2 ); // Кладем в стек результат разности
                        break;
                    default:
                        throw "Unknown binary operation"; // Неизвестная бинарная операция
                }
                break;
            }
            case Operation::UNARY: // Для унарной операции
            {
                int v1 = stack.top(); // Получаем первый аргумент из стека
                stack.pop(); // И удаляем его из стека

                switch ( op.data ) { // В зависимости от конкретной операции
                    case LEX_DEL_SUB:
                        stack.push( -v1 ); // Кладем в стек результат применения минуса
                        break;
                    default:
                        throw "Unknown unary operation"; // Неизвестная бинарная операция
                }
                break;
            }
            case Operation::ASSIGN: // Присваивание значение переменной
            {
                int v1 = stack.top(); // Получаем первый аргумент из стека
                stack.pop(); // И удаляем его из стека

                values[ op.data ] = v1; // Записываем в список значений
                break;
            }
            case Operation::VAR: // Получение значения переменной
            {
                stack.push( values[ op.data ] ); // Кладем в стек значение переменной
                break;
            }
            case Operation::JMP_FALSE:
            {
                int v1 = stack.top(); // Получаем первый аргумент из стека
                stack.pop(); // И удаляем его из стека

                if (!v1)
                    nextOp = op.data;
                break;
            }
            default:
                throw "Unknown operation type"; // Неизвестный тип операции
        }

        currOp = nextOp;
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

        return 1;
    }

    std::cout << "Expression parsed, calculating..." << std::endl;

    // Здесь печатается сгенерированный ПОЛИЗ
    //for ( int i = 0; i < program.size(); ++ i ) {
    //    Operation op = program[i];
    //    std::cout << "{" << op.type << "," << op.value << "}" << std::endl;
    //}

    try {
        std::cout << "Result: " << calculate() << std::endl; // Вычисляем выражение
    } catch ( const char * err ) {
        std::cout << "Error calculating: " << err << std::endl;
        return 2;
    }

    return 0;
}
