#ifndef COMMAND_H
#define COMMAND_H

/*!
\file
\brief Заголовочный файл с реализацией класса команды
Данный файл содержит в себе еализацию класса команды
*/

#include <string>

/*!
Класс команды
*/
class Command
{
public:
    /*!
    Конструктор класса команды
    \param[in] cmd представление команды в виде строки
    */
    Command(std::string cmd) : cmd(cmd) {}
    /*!
    Позволяет получить команду в виде строки
    \return Представление комманды в виде строки 
    */
    std::string get()
    {
        return cmd;
    }

private:
    std::string cmd;
};

#endif // COMMAND_H