#include <iostream>
#include <memory> // std::auto_ptr
#include <utility> // std::pair
#include <string> // std::string

//-------------------------------------------------------

// Результат разбора
// Имеет параметр T - тип значения
template<typename T>
class ParseResult {
public:
  // Метод, возвращающий успешность разбора
  virtual bool success() const = 0;

  // Метод, возвращающий значение разбора. В случае неуспешного разбора порождается исключение
  virtual const T & result() const = 0;
  // Метод, возвращающий остаток неразобранной строки. В случае неуспешного разбора порождается исключение
  virtual const char * tail() const = 0;
};

// Неуспешный результат разбора
template<typename T>
class FailResult : public ParseResult<T> {
public:
  virtual bool success() const { return false; }

  virtual const T & result() const { throw "Fail doesn't contain result"; }
  virtual const char * tail() const { throw "Fail doesn't contain tail"; }
};

// Успешный результат разбора
template<typename T>
class SuccessParseResult : public ParseResult<T> {
public:
  SuccessParseResult( T result, const char * tail ) : _result( result ), _tail( tail ) {}

  virtual bool success() const { return true; }

  virtual const T & result() const { return _result; }
  virtual const char * tail() const { return _tail; }

private:
  T _result;
  const char * _tail;
};

// Парсер
// Параметр T - тип значения разбора
template<typename T>
class Parser {
public:
  virtual ~Parser() {}
  virtual std::auto_ptr<ParseResult<T> > parse( const char * start, const char * end ) = 0;

protected:
  std::auto_ptr<ParseResult<T> > success( T result, const char * tail ) { return std::auto_ptr<ParseResult<T> >( new SuccessParseResult<T>( result, tail ) ); }
  std::auto_ptr<ParseResult<T> > fail() { return std::auto_ptr<ParseResult<T> >( new FailResult<T>() ); }
};

// Парсер строки
// Тип значения разбора - строка
class StrParser : public Parser<std::string> {
public:
  StrParser( const std::string & str ) : str(str) {}

  std::auto_ptr<ParseResult<std::string> > parse( const char * start, const char * end ) {
    if ( end - start < str.size() )
      return fail();

    if ( std::string( start, str.size() ) == str )
      return success( str, start + str.size() );

    return fail();
  }
private:
  std::string str;
};

// Парсер последовательности
// Имеет параметры - типы T1 и T2
// Тип значения разбора - пара из значений T1 и T2
template<typename T1, typename T2>
class SeqParser : public Parser<std::pair<T1,T2> > {
public:
  SeqParser( std::auto_ptr<Parser<T1> > first, std::auto_ptr<Parser<T2> > second ) : first( first ), second( second ) {}

  std::auto_ptr<ParseResult<std::pair<T1,T2> > > parse( const char * start, const char * end ) {
    std::auto_ptr<ParseResult<T1> > result1 = first->parse( start, end );

    if ( result1->success() ) {
      std::auto_ptr<ParseResult<T2> > result2 = second->parse( result1->tail(), end );

      if ( result2->success() )
        return success( std::make_pair( result1->result(), result2->result() ), result2->tail() );
    }

    return std::auto_ptr<ParseResult<std::pair<T1,T2> > >( new FailResult<std::pair<T1,T2> >() );
  }
private:
  std::auto_ptr<Parser<T1> > first;
  std::auto_ptr<Parser<T2> > second;
};

// Парсер альтернативы
// Объединяет два варианта разбора с одинаковым типом значения
template<typename T>
class AltParser : public Parser<T> {
public:
  AltParser( std::auto_ptr<Parser<T> > first, std::auto_ptr<Parser<T> > second ) : first( first ), second( second ) {}

  std::auto_ptr<ParseResult<T> > parse( const char * start, const char * end ) {
    std::auto_ptr<ParseResult<T> > firstResult = first->parse( start, end );

    if (firstResult->success())
      return firstResult;

    return second->parse( start, end );
  }
private:
  std::auto_ptr<Parser<T> > first, second;
};

// Парсер преобразования
// Получает результат некоторого парсера и применяет к его значению заданную функцию
template<typename T1,typename T2>
class TransformParser : public Parser<T1> {
public:
  typedef T1 (*Func)(T2); // Тип функции, осуществляющей преобразование

  TransformParser( std::auto_ptr<Parser<T2> > parser, Func f ): parser(parser), func(f) {}

  std::auto_ptr<ParseResult<T1> > parse( const char * start, const char * end ) {
    std::auto_ptr<ParseResult<T2> > result = parser->parse( start, end ); // Получаем результат разбора

    if (result->success()) // Если он успешен
      return success(func(result->result()),result->tail()); // Возвращаем успешный результат, преобразовав значение в нем

    return std::auto_ptr<ParseResult<T1> >( new FailResult<T1>() ); // Иначе возвращаем неуспех
  }
private:
  std::auto_ptr<Parser<T2> > parser;
  Func func;
};

//------------------------------------------------------

template<typename T>
class Rule; // Упреждающее объявление правила

// Обертка вокруг парсера для применения операций
template<typename T>
class ParserHolder {
public:
  ParserHolder( Parser<T> * parser ) : parser( parser ) {} // Конструктор на основе нового парсера
  ParserHolder( const ParserHolder<T> & holder ) : parser( holder.parser ) {} // Конструктор копирования

  // Последовательное применение двух парсеров
  template<typename T2>
  ParserHolder<std::pair<T,T2> > operator >> ( const ParserHolder<T2> & h ) {
    return new SeqParser<T,T2>( parser, h.parser );
  }
  
  // Преобразование значения результата разбора
  template<typename T2>
  ParserHolder<T2> operator >> ( T2 (*f)(T) ) {
    return new TransformParser<T2,T>( parser, f );
  }

  // Альтернативное применение двух парсеров
  ParserHolder<T> operator || (const ParserHolder<T> & h ) {
    return new AltParser<T>( parser, h.parser );
  }


  mutable std::auto_ptr<Parser<T> > parser;
};

// Функция для создания парсера строки
inline ParserHolder<std::string> str( const std::string & s ) {
  return new StrParser( s );
}

// Правило разбора
// Параметризовано типом значения разбора
template<typename T>
class Rule : public Parser<T> {
public:
  Rule() {} // Конструктор по умолчанию
  Rule(const ParserHolder<T> & p): parser(p.parser) {} // Конструктор для привзывания выражения

  std::auto_ptr<ParseResult<T> > parse( const char * start, const char * end ) {
    if ( parser.get() )
      return parser->parse( start, end );

    throw "Rule not initialized";
  }

  // Оператор присваивания для привязывания выражения
  void operator = ( const ParserHolder<T> & p ) {
    parser = p.parser;
  }

  // Преобразование правила к выражению
  ParserHolder<T> operator &();

private:
  std::auto_ptr<Parser<T> > parser; // Парсер, привязанный к выражению
};

template<typename T>
class RuleParser : public Parser<T> {
public:
  RuleParser( Rule<T> & rule ) : rule( rule ) {}

  std::auto_ptr<ParseResult<T> > parse( const char * start, const char * end ) { return rule.parse( start, end ); }
private:
  Rule<T> & rule;
};

template<typename T>
ParserHolder<T> Rule<T>::operator &() {
  return new RuleParser<T>( *this );
}

//---------------------------------------------------------------------------------------------------------

int zero(std::string s) {
  return 0;
}

int inc(std::pair<std::pair<std::string,int>,std::string> p) {
  return p.first.second + 1;
}

int main() {

  Rule<int> a;
  a = str("a") >> zero || str("b") >> &a >> str("c") >> inc;

  std::string str;

  std::cin >> str;
  std::cout << a.parse(str.c_str(),str.c_str() + str.size())->result() << std::endl;
}
