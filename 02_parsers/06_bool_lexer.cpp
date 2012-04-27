/*
 * В этой программе осуществляется построение лексического анализатора для булевых выражений,
 * который разбивает входной поток символов на лексемы.
 * 
 * Предполагается, что выражения записываются следующим образом:
 * 
 * (true && false) && !true
 */


//#include "stdafx.h"
#include <iostream> // Для ввода-вывода
#include <string> // Для std::string

#include <cstdio> // Для функции getchar
#include <cctype> // Для функции isalpha, определяющей является ли символ буквой

/* В грамматике можно выделить два основных типа лексем - константы true и false
(состоящие из букв) и разделители (из неалфавитных символов). К разделителям здесь
относятся скобки и знаки операций (! и &&). Также можно ввести дополнительные
типы лексем для ошибки и конца ввода. */

enum LexType {
    LEX_NULL, // Тип для нераспознанных лексем - ошибок
    LEX_CONST, // Константы, состоящие из букв
    LEX_DELIM, // Разделители, состояще из неалфавитных символов
    LEX_EOF // Конец ввода
};

// Здесь определяем, какие у нас бывают константы
enum LexConsts {
    LEX_CONST_NULL, // Это для ошибки
    LEX_CONST_TRUE, // true
    LEX_CONST_FALSE // false
};

/* Теперь необходимо перечислить, а как же эти константы записываются -
порядок совпадает с описанием перечисления LexConsts */
const std::string LEX_CONSTS[] = {
    "",
    "true",
    "false",
    "" // Заканчиваем список пустой строкой, чтобы при поиске определять по нему конец
};

// Какие бывают разделители:
enum LexDelims {
    LEX_DEL_NULL, // Для ошибки
    LEX_DEL_AND, // Операция "И"
    LEX_DEL_NOT, // Операция "НЕ"
    LEX_DEL_BROPEN, // Открывающая скобка
    LEX_DEL_BRCLOSE // Закрывающая скобка
};

/* Перечисляем как записываются разделители - опять же порядок совпадает
с описанием перечисления LexDelims */
const std::string LEX_DELIMS[] = {
    "",
    "&&",
    "!",
    "(",
    ")",
    "" // Заканчиваем список пустой строкой, чтобы при поиске определять по нему конец
};

// Описываем класс, представляющий лексему
class Lexeme {
public:
    int type; // Тип лексемы (константа, разделитель, ошибка, конец)
    int index; // Конкретный индекс лексемы (по соответсвующим таблицам для типов)

    std::string buf; // Строка, из которой была получена лексема - для выдачи диагностических сообщений

    // Здесь объявляем конструктор лексемы
    Lexeme( int type = LEX_NULL, int index = LEX_NULL, const std::string & buf = "" ) :
      type( type ), // Инициализируем поле type значением одноименного параметра
      index( index ), // Аналогично для index и остальных полей
      buf( buf ) {}

};

// Оператор вывод лексемы в поток
std::ostream & operator <<(std::ostream & out, const Lexeme & lex) {
    return out << "{" << lex.type   // Печатаем ее тип
               << "," << lex.index  // Индекс
               << "," << lex.buf    // Строку, в которой она накоплена
               << "}";
}

/* Функция для поиска строки в списке
Она принимает первым аргументом строку, а вторым -
список строк, и если в этом списке есть такая же строка, как в
первом аргументе, то возвращает ее индекс, а иначе - 0 */
int find( const std::string & buf, const std::string * list ) {
    for ( int i = 1; !list[i].empty(); ++ i ) // Пока не встретили пустую строку
        if ( list[i] == buf ) // Если строка совпадает с текущим элементов
            return i; // То возвращаем его индекс

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
\w обозначает букву, а \s пробельный символ (по аналогии с регулярными выражениями):

S -> \w W | [()] D | ! | ) | ( | $ | \s S
W -> \w W | eps
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
                } else if ( currentChar == '!' || currentChar == '(' || currentChar == ')' ) { // Если же это односимвольный разделитель
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий символ

                    int index = find( buf, LEX_DELIMS ); // Находим этот разделитель в таблице

                    return Lexeme( LEX_DELIM, index, buf ); // И возвращаем соответствующую лексему
                } else if ( currentChar == '\n' || currentChar == '\r' ) { // Если символ - конец ввода
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
                    int index = find( buf, LEX_CONSTS ); // Находим ее в таблице

                    return Lexeme( LEX_CONST, index, buf ); // Просто возвращаем соответствующую константе лексему
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

// Все, распознавание написано, теперь функция main для тестов
int main(int argc, char ** argv) {
    gc(); // Считываем начальный символ
    while ( true ) { // Внутри цикла
        Lexeme lex = readNextLexeme(); // Читаем лексему из входного потока

        std::cout << lex << std::endl;

        if ( lex.type == LEX_NULL || lex.type == LEX_EOF ) // Если лексема - конец строки или ошибка - выходим
            return 0;
    }
}
