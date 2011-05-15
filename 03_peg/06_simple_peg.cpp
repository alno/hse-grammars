#include <string>
#include <memory>
#include <string>
#include <iostream>

// Структура с информацией о результатах разбора. 
// Хранит флаг успешности разбора и указатель на неразобранную строку
struct ParseInfo {
  ParseInfo() {}
  ParseInfo( bool success, const char * tail ) : success( success ), tail( tail ) {}
  
  bool success;
  const char * tail;
};

// Базовый класс парсера
// Принимает на вход указатели на начало и конец разбираемой строки и
// возвращает информацию об успешности разбора
class Parser {
public:
  virtual ~Parser() {}
  virtual ParseInfo parse( const char * start, const char * end ) = 0; // Функция разбора
  
protected:
  // Вспомогательные функции для генерации успешного и неуспешного результата разбора
  ParseInfo success( const char * tail ) { return ParseInfo( true, tail ); }
  ParseInfo fail() { return ParseInfo( false, 0 ); }
};

// Парсер, принимающий указанную строку
// Разбор успешен тогда, когда входная строка начинается с заданной
class StrParser : public Parser {
public:
  // В конструкторе запоминаем заданную строку
  StrParser( const std::string & str ) : str(str) {}
  
  ParseInfo parse( const char * start, const char * end ) {    
    if ( end - start < str.size() ) // Проверяем, что во входной строке достаточно символов
      return fail();

    if ( std::string( start, str.size() ) == str ) // Проверяем, что префикс входной строки совпадает с заданной
      return success( start + str.size() );
    
    return fail(); // Иначе - неуспех
  }
private:
  std::string str;
};

// Парсер, представляющий последовательность двух парсеров
class SeqParser : public Parser {
public:
  // Запоминаем первый и второй парсер в последовательности
  SeqParser( std::auto_ptr<Parser> first, std::auto_ptr<Parser> second ) : first( first ), second( second ) {}
  
  ParseInfo parse( const char * start, const char * end ) {
    ParseInfo firstResult = first->parse( start, end ); // Разбираем строку первым парсером
    
    if (firstResult.success) // Если успешно
      return second->parse( firstResult.tail, end ); // Разбираем остаток вторым парсером
    
    return fail(); // Иначе - неуспех
  }
private:
  std::auto_ptr<Parser> first, second;
};

// Парсер, представляющий альтернативу двух парсеров
class AltParser : public Parser {
public:
  // Запоминаем первый и второй парсер в альтернативе
  AltParser( std::auto_ptr<Parser> first, std::auto_ptr<Parser> second ) : first( first ), second( second ) {}
  
  ParseInfo parse( const char * start, const char * end ) {
    ParseInfo firstResult = first->parse( start, end ); // Разбираем строку первым парсером
    
    if (firstResult.success) // Если успешно
      return firstResult; // То возвращаем результата первого парсера
    
    return second->parse( start, end ); // Иначе разбираем вторым парсером
  }
private:
  std::auto_ptr<Parser> first, second;
};

// Упреждающее объявление класса правила разбора
class Rule;

// Класс-обертка для парсера, реализующий операции для комбинации парсеров
class ParserHolder {
public:
  ParserHolder( Parser * parser ) : parser( parser ) {}
  ParserHolder( const ParserHolder & holder ) : parser( holder.parser ) {}
  
  // Операция последовательного применения парсеров
  ParserHolder operator >> ( const ParserHolder & h ) { 
    return new SeqParser( parser, h.parser ); // Создаем новый парсер последовательности
  }
  
  // Операция последовательного применения парсеров, где второй элемент - правило
  ParserHolder operator >> ( Rule & r );
  
  // Операция альтернативного примерения парсеров
  ParserHolder operator || (const ParserHolder & h ) {
    return new AltParser( parser, h.parser );
  }
  
private:
  mutable std::auto_ptr<Parser> parser;
  
  friend class Rule;
};

// Функция, создающая парсер строки
ParserHolder str( const std::string & s ) {
  return new StrParser( s );
}

// Правило разбора
// Должно быть связано с некоторым парсером, реализующим его
class Rule : public Parser {
public:
  
  // При разборе просто вызываем связанный парсер если он есть
  ParseInfo parse( const char * start, const char * end ) {
    if ( parser.get() )
      return parser->parse( start, end );
    
    throw "Rule not initialized";
  }
  
  // Функция связывания правила и его реализации
  void operator = ( const ParserHolder & p ) {
    parser = p.parser;
  }
  
private:
  std::auto_ptr<Parser> parser;
};

// Парсер, принимающий правило разбора
class RuleParser : public Parser {
public:
  RuleParser( Rule & rule ) : rule( rule ) {}
  
  ParseInfo parse( const char * start, const char * end ) { return rule.parse( start, end ); }
private:
  Rule & rule;
};

// Реализация операции связывания парсера и правила разбора
ParserHolder ParserHolder::operator >> ( Rule & r ) { 
  return new SeqParser( parser, std::auto_ptr<Parser>( new RuleParser( r ) ) );
}

int main() {
  // Определяем грамматику
  Rule A;
  
  A = str("c") || ( str("a") >> A >> str("b") ); // Грамматика A -> c | aAb - язык a^ncb^n
  
  // Проверяем строку на соответствие грамматике
  std::string s;  
  std::cin >> s;
  
  if ( A.parse( s.data(), s.data() + s.size() ).success )
    std::cout << "Yes" << std::endl;
  else
    std::cout << "No" << std::endl;
}
