#ifndef STATIC_COMMAND_BLOCK_H
#define STATIC_COMMAND_BLOCK_H

/*!
\file Заголовочный фаил с описанием класса статического коммандного блока

Статическим является блок, размер которого определен
*/

#include "command_block.h"

class StaticCommandBlock : public CommandBlock
{
public:
    /*!
    Конструктор класса статического коммандного блока
    \param[in] size Размер блока
    */
    StaticCommandBlock(int size) : CommandBlock(),
                                   size(size)
    {
    }
    /*!
    Добавляет комманду в блок:
    в случае, если доваляемая команда является последней в блоке,
    блок завершает работу
    \param[in] cmd Указатель на команду
    \return true, если комманда добавлена в блок и блок не завершил работу,
    иначе false
    */
    bool add(std::shared_ptr<Command> cmd) override
    {
        CommandBlock::add(cmd);
        if (commands.size() == size)
        {
            return false;
        }
        return true;
    }
    /*!
    Проверяет, является ли блок динамическим
    \return false
    */
    bool isDynamic() override
    {
        return false;
    }

private:
    /// Размер блока
    int size = 0;
};

#endif // STATIC_COMMAND_BLOCK_H
