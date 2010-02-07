#include <iostream>
#include <ctype.h>
#include <string.h>

// S -> E // Дополнительное правило для конца цепочки
// E -> '0' | '1' | '!' E | '(' E '&' E ')'

/*
true
false
!
&&
*/


enum LexType {
    LEX_NULL,
    LEX_WORD,
    LEX_DELIM
};

enum LexWords {
    LEX_WORD_NULL,
    LEX_WORD_TRUE,
    LEX_WORD_FALSE
};

enum LexWord {
    LEX_DEL_NULL,
    LEX_DEL_AND,
    LEX_DEL_NOT,
    LEX_DEL_BROPEN,
    LEX_DEL_BRCLOSE,
    LEX_DEL_EOF
};

const char * LEX_WORDS[] = {
    "",
    "true",
    "false",
    0
};
  
const char * LEX_DELIMS[] = {
    "",
    "&&",
    "!",
    "(",
    ")",
    "$",
    0
};

class Lexeme {
public:    
    Lexeme( int type, int index, std::string buf, bool value ) : type( type ), index( index ), buf( buf ), value( value ) {}
    Lexeme( int type, int index, std::string buf ) : type( type ), index( index ), buf( buf ) {}
    Lexeme() : type( LEX_NULL ), index( LEX_NULL ), buf( "" ) {}
    
    int type;
    int index;
    
    std::string buf;
    
    bool value;
};

char currentChar; // Текущий символ

void gc() {
    std::cin >> currentChar;
}

/*
S -> \w W | & D | ! | ) | ( | \s S | $
W -> \w W
D -> & DE
*/

int findLex( const char * list[], const char * buf ) {
    int i = 0;
    while ( list[i] ) {
        if ( strcmp( list[i], buf ) == 0 )
            return i;
            
        i ++;
    }

    return 0;
}

Lexeme readLexeme() {
    enum State {
        S, W, D
    };
    
    State currentState = S;
    
    std::string buf;
    
    while (true) {        
        switch (currentState) {
            case S:
                if ( currentChar == ' ' || currentChar == '_' ) {
                    currentState = S;
                } else if ( isalpha( currentChar ) ) {
                    buf += currentChar;
                    gc();
                    currentState = W;
                } else if ( currentChar == '&' ) {
                    buf += currentChar;
                    gc();
                    currentState = D;
                } else if ( currentChar == '!' || currentChar == '(' || currentChar == ')' ) {
                    buf += currentChar;
                    gc();
                    int index = findLex( LEX_DELIMS, buf.c_str() );
                    
                    if ( index <= 0 )
                        return Lexeme( LEX_NULL, LEX_NULL, buf );
                        
                    return Lexeme( LEX_DELIM, index, buf );
                } else if ( currentChar == '$' ) {
                    buf += currentChar;
                    return Lexeme( LEX_DELIM, LEX_DEL_EOF, buf );
                } else {
                    buf += currentChar;
                    return Lexeme( LEX_NULL, LEX_NULL, buf );
                }
                break;
            case W:
                if ( isalpha( currentChar ) ) {
                    buf += currentChar;
                    gc();
                    
                    currentState = W;
                } else {
                    int index = findLex( LEX_WORDS, buf.c_str() );
                    
                    if ( index <= 0 )
                        return Lexeme( LEX_NULL, LEX_NULL, buf );                   
                    
                    if ( index == LEX_WORD_TRUE )
                        return Lexeme( LEX_WORD, LEX_WORD_TRUE, buf, true );
                    if ( index == LEX_WORD_FALSE )
                        return Lexeme( LEX_WORD, LEX_WORD_FALSE, buf, false );
                    
                    return Lexeme( LEX_WORD, index, buf );
                }
                break;
            case D:
                if ( currentChar == '&' ) {
                    buf += currentChar;
                    gc();
                    return Lexeme( LEX_DELIM, LEX_DEL_AND, buf );
                } else {
                    buf += currentChar;
                    gc();
                    return Lexeme( LEX_NULL, LEX_NULL, buf );
                }
                break;            
            default:
                buf += currentChar;
                gc();
                return Lexeme( LEX_NULL, LEX_NULL, buf );
        };       
    }
}

Lexeme currentLex; // Текущая лексема

void getNextLexeme() { // Функция получения следующей лексемы
    currentLex = readLexeme();
}

bool parseE() {
    if ( currentLex.type == LEX_WORD && (currentLex.index == LEX_WORD_TRUE || currentLex.index == LEX_WORD_FALSE) ) {
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
            throw "'&&' needed";
        getNextLexeme();
        
        bool r2 = parseE(); // Второй операнд
        
        if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_BRCLOSE ) // Проверяем закрывающую скобку
            throw "')' needed";
        getNextLexeme();
        
        return r1 && r2;
    } else {
        throw "Start of expression needed";
    }
}

bool parseS() {
    bool r = parseE();
    
    if ( currentLex.type != LEX_DELIM || currentLex.index != LEX_DEL_EOF ) // Проверяем конец цепочки
        throw "End of line needed";
    
    return r;
}

int main(int argc, char ** argv) {
    try {
        gc(); // Считываем первый символ
        getNextLexeme(); // Считываем первую лексему
        bool result = parseS(); // Парсим  и вычисляем выражение
        std::cout << "Calculated: " << result << std::endl;
    } catch ( const char * err ) {
        std::cout << "Error: " << err << ", but '" << currentLex.buf << "' got." << std::endl;
    }
}