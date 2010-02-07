#include <iostream>

// S -> E // Дополнительное правило для конца цепочки
// E -> '0' | '1' | '!' E | '(' E '&' E ')'

/*
true
false
not
and

S -> t T1 | f F1 | n N1 | a A1 | ( | ) | $
T1 -> r T2
T2 -> u T3
T3 -> e
F1 -> a F2
F2 -> l F3
F3 -> s F4
F4 -> e
N1 -> o N2
N2 -> t
A1 -> n A2
A2 -> d
*/

class Lexeme {    
public:
    enum Type {
        CONST,
        NOT,
        AND,
        BROPEN,
        BRCLOSE,
        END,
        UNKNOWN
    };
    
    Lexeme( int type, bool value ) : type( type ), value( value ) {}
    Lexeme( int type ) : type( type ) {}
    Lexeme() : type( UNKNOWN ) {}
    
    int type;
    bool value;
};

char currentChar; // Текущий символ

void gc() {
    std::cin >> currentChar;
}


Lexeme readLexeme() {
    enum State {
        S, T1, T2, T3, F1, F2, F3, F4, N1, N2, A1, A2, FIN, ERR
    };
    
    State currentState = S;
    
    while (true) {
        gc();
        
        switch (currentState) {
            case S:
                if ( currentChar == ' ' || currentChar == '_' )
                    currentState = S;
                else if ( currentChar == 't' )
                    currentState = T1;
                else if ( currentChar == 'f' )
                    currentState = F1;
                else if ( currentChar == 'a' )
                    currentState = A1;
                else if ( currentChar == 'n' )
                    currentState = N1;
                else if ( currentChar == '(' )
                    return Lexeme( Lexeme::BROPEN );
                else if ( currentChar == ')' )
                    return Lexeme( Lexeme::BRCLOSE );
                else if ( currentChar == '$' )
                    return Lexeme( Lexeme::END );
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
            case T1:
                if ( currentChar == 'r' )
                    currentState = T2;
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
            case T2:
                if ( currentChar == 'u' )
                    currentState = T3;
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;    
            case T3:
                if ( currentChar == 'e' )
                    return Lexeme( Lexeme::CONST, true );
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
           case F1:
                if ( currentChar == 'a' )
                    currentState = F2;
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
            case F2:
                if ( currentChar == 'l' )
                    currentState = F3;
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
            case F3:
                if ( currentChar == 's' )
                    currentState = F4;
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break; 
            case F4:
                if ( currentChar == 'e' )
                    return Lexeme( Lexeme::CONST, false );
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
            case N1:
                if ( currentChar == 'o' )
                    currentState = N2;
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break; 
            case N2:
                if ( currentChar == 't' )
                    return Lexeme( Lexeme::NOT );
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
            case A1:
                if ( currentChar == 'n' )
                    currentState = A2;
                else
                    currentState = ERR;
                break; 
            case A2:
                if ( currentChar == 'd' )
                    return Lexeme( Lexeme::AND );
                else
                    return Lexeme( Lexeme::UNKNOWN );
                break;
            default:
                currentState = ERR;
        };
    }
}

int main(int argc, char ** argv) {
    std::cout << readLexeme().type << std::endl;
}