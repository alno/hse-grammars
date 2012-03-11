//#include "stdafx.h"
#include <ctype.h> // Для функции isdigit, определяющей является ли символ цифрой
#include <string.h> // Для функции сравнения строк strcmp
#include <stdlib.h> // Для atoi

#include <iostream> // Для ввода-вывода (std::сin и std::cout)
#include <string> // Для std::string

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

/* Абстрактный класс, представлющий узел выражения, который можно вычислить */
class ExpressionNode {
public:
    virtual ~ExpressionNode() {} // Виртуальный деструктор

    virtual int calculate() const = 0; // Функция для вычисления значения выражения в узле
};

/* Класс, представляющий узел, содержащий бинарную операцию */
class BinaryOperationNode : public ExpressionNode {
public:
    BinaryOperationNode( int index, const ExpressionNode * arg1, const ExpressionNode * arg2 ) : index( index ), arg1( arg1 ), arg2( arg2 ) {}

    virtual ~BinaryOperationNode() {
        delete arg1; // Удаляем первый аргумент
        delete arg2; // Удаляем второй аргумент
    }

    /* Вычисление бинарной операции */
    virtual int calculate() const {
        int v1 = arg1->calculate(); // Вычисляем значение первого аргумента
        int v2 = arg2->calculate(); // Вычисляем значение второго аргумента

        switch ( index ) { // В зависимости от конкретной операции
            case LEX_DEL_ADD:
                return v1 + v2; // Возвращаем результат сложения
            case LEX_DEL_MUL:
                return v1 * v2; // Возвращаем результат умножения
            default:
                throw "Unknown binary operation"; // Неизвестная бинарная операция
        }
    }

private:
    int index; // Индекс операции
    const ExpressionNode * arg1; // Первый аргумент
    const ExpressionNode * arg2; // Второй аргумент
};

/* Класс, представляющий узел, содержащий константу */
class ConstNode : public ExpressionNode {
public:
    ConstNode( int value ) : value( value ) {}

    virtual ~ConstNode() {}

    /* Вычисление выражение-константы */
    virtual int calculate() const {
        return value;
    }

private:
    int value; // Значение константы
};

Lexeme currentLex; // Текущая лексема

/* Функция получения следующей лексемы, тут синтаксический интерпретатор связан с лексическим */
void getNextLexeme() {
    currentLex = readNextLexeme(); // Обращаемся к лексическому анализатору
}

/* Разбор выражения */
ExpressionNode * parseE() {
    if ( currentLex.type == LEX_NUMBER ) { // Если текущая лексема - число
        ExpressionNode * node = new ConstNode( currentLex.value ); // Создаем узел, содержащий константу

        getNextLexeme();

        return node; // Возвращаем узел
    } else if ( currentLex.type == LEX_DELIM && currentLex.index == LEX_DEL_BROPEN ) { // Если текущая лексема - скобка
        getNextLexeme();

        ExpressionNode * arg1 = parseE(); // Первый операнд

        if ( currentLex.type != LEX_DELIM || ( currentLex.index != LEX_DEL_ADD && currentLex.index != LEX_DEL_MUL ) ) // Проверяем знак операции
            throw "& needed";

        Lexeme operation = currentLex; // Запоминаем текущую операцию

        getNextLexeme();

        ExpressionNode * arg2 = parseE(); // Второй операнд

        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BRCLOSE ) // Проверяем закрывающую скобку
            throw ") needed";

        getNextLexeme();

        return new BinaryOperationNode( operation.index, arg1, arg2 ); // Возвращаем узел с бинарной операцией
    } else { // Иначе ошибка
        throw "Start of expression needed";
    }
}

/* Разбор для начального состояния грамматики */
ExpressionNode * parseS() {
    ExpressionNode * node = parseE();

    if ( currentLex.type != LEX_EOF ) // Проверяем конец цепочки
        throw "End of line needed";

    return node;
}

int main(int argc, char ** argv) {
    ExpressionNode * node;

    try {
        std::cout << "Enter expression: " << std::endl;
        gc();
        getNextLexeme(); // Считываем первую лексему
        node = parseS(); // Парсим выражение, строим дерево
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

    try {
        std::cout << "Result: " << node->calculate() << std::endl; // Вычисляем выражение
    } catch ( const char * err ) {
        std::cout << "Error calculating: " << err << std::endl;
        return 2;
    }

    delete node; // Удаляем дерево выражения

    return 0;
}
