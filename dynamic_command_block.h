#ifndef DYMANIC_COMMAND_BLOCK_H
#define DYNAMIC_COMMAND_BLOCK_H

/*!
\file Заголовочный фаил с описанием класса динамического коммандного блока

Динамическим является блок, для которого не задан размер
*/

#include "command_block.h"

/// Класс динамиского командного блока
class DynamicCommandBlock : public CommandBlock
{
public:
    DynamicCommandBlock() : CommandBlock()
    {
    }
    /*!
    Проверяет, является ли блок динамическим
    \return true
    */
    bool isDynamic() override
    {
        return true;
    }
};

#endif // DYNAMIC_COMMAND_BLOCK_H
