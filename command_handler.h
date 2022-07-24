#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

/*!
\file Заголовочный фаил с описанием класса обработчика команд

Данный фаил содержит в себе описание класса обработчика команд
*/

#include <memory>
#include <stack>
#include <queue>
#include <condition_variable>
#include <atomic>
#include "command_block.h"

/*!
Класс обработчика команд
*/
class CommandHandler
{
public:
    /*!
    Конструктор класса обработчика команд
    \param[in] size Размер статического блока команд
    */
    CommandHandler(int size);
    /*!
    Запуск обработчика
    */
    void start();
    /*! Завершения обработчика
     */
    void stop();
    /*!
    Запрашивает обработку команд и управляющих символов,
    представленных в виде строк
    \param[in] cmd Команда или управляющий символ
    */
    void handle(std::string cmd);

private:
    const int fileThreads = 2;
    /// Данные для логирования в фаил
    struct FileLoggingData
    {
        FileLoggingData(const std::string &timestamp, Commands cmds) : timestamp(timestamp),
                                                                       cmds(cmds) {}
        std::string timestamp;
        Commands cmds;
        std::mutex mtx;
        int readedCnt = 0;
    };

    /// Размер статического блока
    int staticBlockSize = 0;
    /// Текущая команда для обработки
    std::queue<std::string> cmds;
    /// Текущий командный блок
    std::shared_ptr<CommandBlock> currBlock = nullptr;
    /// Стек командных блоков
    std::stack<std::shared_ptr<CommandBlock>> blocks;
    /// Условная переменная прихода новой команды для обработки
    std::condition_variable cmdHandleCV;
    /// Условная переменная запроса завершения командного блока
    std::condition_variable blockReadyCV;
    /// Условная переменная запроса логирования в консоль
    std::condition_variable blockLogCV;
    /// Условная переменная запроса логирования в консоль
    std::condition_variable blockFileCV;
    /// Флаг нахождения программы в работе
    std::atomic<bool> execution;
    /// Мьютекс на очередь команд
    std::mutex cmdQueueMtx;
    /// Мьютекс на очередь логгирования
    std::mutex logQueueMtx;
    /// Комманды для вывода в консоль
    std::queue<Commands> logQueue;
    /// Мьютекс на очередь логгирования в фаил
    std::mutex fileQueueMtx;
    /// Очередь данных на вывод в фаил
    std::queue<std::shared_ptr<FileLoggingData>> fileQueue;
    /// Флаг работы потока логгирования в консоль
    std::atomic<bool> logWork;
    /// Флаг работы первого потока логгирования в фаил
    std::atomic<int> fileWork;


    /// Обрабатывает приходящие команды
    void handle_();
    /// Логирует коммандные блоки в консоль
    void log();
    /// Логгирует коммандные блоки в файл
    void writeToFile(int id);
    /*!
    Добавляет новый динамический командный блок в стек
    и делает новый блок текущим
    */
    void createNewDynamicBlock();
    /*!
    Добавляет новый статический командный блок в стек
    и делает новый блок текущим
    */
    void createNewStaticBlock();
    /*!
    Удаляет текущий блок из стека и назначает предыдущий командный
    текущим, в случае если стек не пуст
    */
    void releaseCurrentBlock();
};

#endif // COMMAND_HANDLER_H
