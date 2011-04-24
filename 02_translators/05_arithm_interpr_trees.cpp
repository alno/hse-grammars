//#include "stdafx.h"
#include <iostream> // Для ввода-вывода
#include <string> // Для std::string
#include <map> // Для std::map
#include <vector>
#include <memory> // Для std::auto_ptr
#include <ctype.h> // Для функции isalpha, определяющей является ли символ буквой
#include <string.h> // Для функции сравнения строк strcmp
#include <stdlib.h>

/*
  В этой программе производим разбор и вычисление арифметического выражения с
  переменными, значения которых вводятся с клавиатуры пользователем перед вводом
  выражения.
  
  В выражении доступны операции +/-/* и учитывается их правильный приоритет.
  Грамматика:

  ET -> E0 [ ? E0 : E0 ]
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
 
// Описываем класс, представляющий лексему
class Lexeme {
public:
    enum LexType {
        UNKNOWN, // Тип для нераспознанных лексем - ошибок
        CONST, // Константы, состоящие из цифр
        VAR, // Переменные, начинающиеся с буквы
        DELIM, // Разделители, состояще из неалфавитных символов
        LEOF // Конец ввода
    };
 
    // Какие бывают разделители:
    enum LexDelims {
        DEL_UNKNOWN, // Для ошибки
        DEL_ADD, // Сложение
        DEL_SUB, // Вычитание
        DEL_MUL, // Умножение
        DEL_BROPEN, // Открывающая скобка
        DEL_BRCLOSE, // Закрывающая скобка
        DEL_QUEST, // Вопросительный знак
        DEL_COLON // Двоеточие
    };
    // У него 4 поля:
    int type; // Тип лексемы (константа, разделитель, ошибка, конец)
    int index; // Конкретный индекс лексемы (по соответсвующим таблицам для типов)
    int value; // Значение лексемы - используется для констант
    
    std::string buf; // Строка, из которой была получена лексема - для выдачи диагностических сообщений
    
    // Здесь объявляем конструктор лексемы
    Lexeme( int type = Lexeme::UNKNOWN, int index = Lexeme::UNKNOWN, const std::string & buf = "", int value = 0 ) :
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
                    return Lexeme( Lexeme::DELIM, Lexeme::DEL_ADD, "+" ); 
                } else if ( currentChar == '-' ) { // Вычитание
                    gc();
                    return Lexeme( Lexeme::DELIM, Lexeme::DEL_SUB, "-" ); 
                } else if ( currentChar == '*' ) { // Произведение
                    gc();
                    return Lexeme( Lexeme::DELIM, Lexeme::DEL_MUL, "*" ); 
                } else if ( currentChar == '(' ) { // Открывающаяся скобка
                    gc();
                    return Lexeme( Lexeme::DELIM, Lexeme::DEL_BROPEN, "(" );
                } else if ( currentChar == ')' ) { // Закрывающаяся скобка
                    gc();
                    return Lexeme( Lexeme::DELIM, Lexeme::DEL_BRCLOSE, ")" );
                } else if ( currentChar == '?' ) { // Вопросительный знак
                    gc();
                    return Lexeme( Lexeme::DELIM, Lexeme::DEL_QUEST, "?" );
                } else if ( currentChar == ':' ) { // Двоеточие
                    gc();
                    return Lexeme( Lexeme::DELIM, Lexeme::DEL_COLON, ":" );
                } else if ( currentChar == '$' ) { // Если символ - конец ввода
                    return Lexeme( Lexeme::LEOF, Lexeme::UNKNOWN, "$" ); // То возвращаем лексему конца ввода
                } else { // Иначе - какой-то непонятный символ
                    buf += currentChar; // Добавляем его в строку-буфер
                    return Lexeme( Lexeme::UNKNOWN, Lexeme::UNKNOWN, buf ); // И возвращаем ошибочную лексему
                }
                break;
            case W: // Состояние распознавания переменных
                if ( isalpha( currentChar ) || isdigit( currentChar ) ) { // Если символ - буква или цифра
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий
                    currentState = W; // И остаемся в том же состоянии
                } else { // Другой символ - значит переменная уже закончилась
                    return Lexeme( Lexeme::VAR, 0, buf );
                }
                break;
            case D: // Состояние распознавания двухсимвольного разделителя
                if ( isdigit( currentChar ) ) { // Если символ - цифра
                    buf += currentChar; // То добавляем его в строку-буфер
                    gc(); // Считываем следующий
                    currentState = D; // И остаемся в том же состоянии
                } else { // Иначе
                    return Lexeme( Lexeme::CONST, 0, buf, atoi( buf.c_str() ) ); // И возвращаем ошибочную лексему
                }
                break;
        };
    }
}
 
Lexeme currentLex; // Текущая лексема
 
void getNextLexeme() { // Функция получения следующей лексемы
    currentLex = readNextLexeme();
}

class Expression {
public:
    virtual ~Expression() {}
    
    virtual int evaluate( std::vector<int> & vars ) = 0;
};

class ConstExpression : public Expression {
public:
    ConstExpression( int value ) : value( value ) {}
    virtual ~ConstExpression() {}
    
    virtual int evaluate( std::vector<int> & vars ) {
        return value;
    }
    
private:
    int value;
};

class VarExpression : public Expression {
public:
    VarExpression( int index ) : index( index ) {}
    virtual ~VarExpression() {}
    
    virtual int evaluate( std::vector<int> & vars ) {
        return vars[ index ];
    }
    
private:
    int index;
};

class BinaryExpression : public Expression {
public:
    enum Operation {
        ADD,
        SUB,
        MUL
    };

    BinaryExpression( int op, std::auto_ptr<Expression> left, std::auto_ptr<Expression> right ) : op( op ), left( left ), right( right ) {}
    virtual ~BinaryExpression() {}
    
    virtual int evaluate( std::vector<int> & vars ) {
        int l = left->evaluate( vars );
        int r = right->evaluate( vars );
        
        switch ( op ) {
        case ADD:
            return l + r;
        case SUB:
            return l - r;
        case MUL:
            return l * r;
        default:
            throw "Unknown operation";
        }
    }

private:
    int op;
    std::auto_ptr<Expression> left, right;
};

class CondExpression : public Expression {
public:

    CondExpression( std::auto_ptr<Expression> cond, std::auto_ptr<Expression> left, std::auto_ptr<Expression> right ) : cond( cond ), left( left ), right( right ) {}
    virtual ~CondExpression() {}
    
    virtual int evaluate( std::vector<int> & vars ) {
        if ( cond->evaluate( vars ) )
            return left->evaluate( vars );
        else
            return right->evaluate( vars );
    }

private:
    std::auto_ptr<Expression> cond, left, right;
};

std::map<std::string,int> variableIndices; 

std::auto_ptr<Expression> parseET();

std::auto_ptr<Expression> parseE2() {
    if ( currentLex.type == Lexeme::CONST ) {
        int val = currentLex.value;
        getNextLexeme();
        return std::auto_ptr<Expression>( new ConstExpression( val ) );
    } else if ( currentLex.type == Lexeme::VAR ) {
        int index;
        if ( variableIndices.find( currentLex.buf ) != variableIndices.end() ) {
            index = variableIndices.find( currentLex.buf )->second;
        } else {
            index = variableIndices.size();
            variableIndices.insert( std::make_pair( currentLex.buf, index ) );
        }
        
        getNextLexeme();
        return std::auto_ptr<Expression>( new VarExpression( index ) );
    } else if ( currentLex.type == Lexeme::DELIM && currentLex.index == Lexeme::DEL_BROPEN ) {
        getNextLexeme();
        std::auto_ptr<Expression> e = parseET();        
         
        if ( currentLex.type != Lexeme::DELIM || currentLex.index != Lexeme::DEL_BRCLOSE ) // Проверяем закрывающую скобку
            throw "Closing bracket required";

        getNextLexeme();
        
        return e;
    } else {
        throw "Expression (E2) required";
    }
}

std::auto_ptr<Expression> parseE1() {
    std::auto_ptr<Expression> exp = parseE2();
    
    while ( currentLex.type == Lexeme::DELIM && currentLex.index == Lexeme::DEL_MUL ) {
        getNextLexeme();
        
        exp = std::auto_ptr<Expression>( new BinaryExpression( BinaryExpression::MUL, exp, parseE1() ) );
    }
    
    return exp;
}

std::auto_ptr<Expression> parseE0() {    
    std::auto_ptr<Expression> exp = parseE1();
        
    while ( currentLex.type == Lexeme::DELIM && ( currentLex.index == Lexeme::DEL_ADD || currentLex.index == Lexeme::DEL_SUB ) ) {
        if ( currentLex.index == Lexeme::DEL_ADD ) {        
            getNextLexeme();
        
            exp = std::auto_ptr<Expression>( new BinaryExpression( BinaryExpression::ADD, exp, parseE1() ) );
        } else if ( currentLex.index == Lexeme::DEL_SUB ) {
            getNextLexeme();
        
            exp = std::auto_ptr<Expression>( new BinaryExpression( BinaryExpression::SUB, exp, parseE1() ) );
        } else {
            throw "ADD or SUB required";
        }
    }
    
    return exp;
}

std::auto_ptr<Expression> parseET() {
    std::auto_ptr<Expression> exp = parseE0();
    
    if ( currentLex.type == Lexeme::DELIM && currentLex.index == Lexeme::DEL_QUEST ) {
        getNextLexeme();
                
        std::auto_ptr<Expression> left = parseE0();        
        
        if ( currentLex.type != Lexeme::DELIM || currentLex.index != Lexeme::DEL_COLON )
            throw "Colon required";
        
        getNextLexeme();
        
        std::auto_ptr<Expression> right = parseE0();
        
        exp = std::auto_ptr<Expression>( new CondExpression( exp, left, right ) );
    }
    
    return exp;
}
 
std::auto_ptr<Expression> parseS() {
    std::auto_ptr<Expression> exp = parseET();
    
    if ( currentLex.type != Lexeme::LEOF ) // Проверяем конец цепочки
        throw "End of line needed";
    
    return exp;
}

std::vector<int> readVariables() {
    std::vector<int> vars( variableIndices.size() );

    while ( true ) {
        std::string varName; // Имя переменной     
        
        std::cout << "Enter var name ($ to end): ";
        std::cin >> varName; // Вводим имя переменной
        
        if ( varName == "$" )
            break;
        
        if ( variableIndices.find( varName ) == variableIndices.end() ) {
            std::cout << "Undefined variable" << std::endl;
            continue;
        }
        
        int varValue;
        std::cout << "Enter var value: ";
        std::cin >> varValue; // Вводим значение
                
        vars[ variableIndices.find( varName )->second ] = varValue; // Добавляем значение переменной
    }
    
    return vars;
}
 
int main(int argc, char ** argv) {
    std::cout << "Enter expression: ";
    
    try {
        gc(); // Считываем первый символ
        getNextLexeme(); // Считываем первую лексему
        std::auto_ptr<Expression> exp = parseS(); // Парсим и вычисляем выражение
        std::vector<int> vars = readVariables();
                
        std::cout << "Calculated: " << exp->evaluate(vars) << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << std::endl;
        std::cout << "Got: {" << currentLex.type // Печатаем ее тип
                  << "," << currentLex.index // Индекс
                  << "," << currentLex.buf // Строку, из которой она получена
                  << "," << currentLex.value // Булево значение
                  << "}" << std::endl; 
    }
}
