//#include "stdafx.h" // Подключаем прекомлируемый заголовочный файл, если он есть в проекте
#include <boost/xpressive/xpressive.hpp> // Подключаем заголовочный файл для регулярных выражений

#include <string>
#include <iostream>

using namespace boost::xpressive; // Включаем пространство имен

bool check( const std::string & s ) {
    static const sregex e = sregex::compile( "[+-]?[01]+" ); // Создаем регулярное выражение

    return regex_match( s, e ); // Проверяем строку на соответствие
}

int main( int argc, char ** argv ) {
    std::string s;

    std::cin >> s;
    std::cout << check( s ) << std::endl;

    return 0;
}