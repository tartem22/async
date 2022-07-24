#ifndef COMMAND_BLOCK_H
#define COMMAND_BLOCK_H

/*!
\file Заголовочный фаил, содержащий описание абстрактного
класса командного блока
*/

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <queue>
#include "command.h"

using Commands = std::queue<std::shared_ptr<Command>>;

/*!
Абстрактный класс командного блока
*/
class CommandBlock
{
public:
    /*!
    Добавляет комманду в блок
    \param[in] cmd Указатель на команду
    \return Признак успешности добавления команды в блок
    */
    virtual bool add(std::shared_ptr<Command> cmd);
    /*!
    Проверяет, является ли блок динамическим
    \return true, если блок динамический, иначе false
    */
    virtual bool isDynamic() = 0;
    /*!
    Проверяет, является ли блок игнорируемым
    \return true, если блок игнорируемый, иначе false
    */
    bool isIgnored();
    /// Делает блок игнорируемым
    void setAsIgnored();
    /// Возвращает временную марку
    std::string getTimestamp()
    {
        return timeCode;
    }
    /// Возвращает список комманд
    Commands getCommands()
    {
        return commands;
    }

protected:
    /// Перечень команд в блоке
    Commands commands;
    /// Устанавливает таймкод блока
    void setTimeCode();

private:
    /// Таймкод блока: время добавление первой команды
    std::string timeCode;
    /// Признак игнорируемости блока
    bool isIgnored_ = false;
};

#endif // COMMAND_BLOCK_H
