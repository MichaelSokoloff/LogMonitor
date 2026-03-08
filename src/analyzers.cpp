// src/analyzers.cpp
#include "analyzers.hpp"      // Заголовочный файл с объявлениями
#include "log_entry.hpp"      // Структура LogEntry
#include "stats.hpp"          // Атомарная статистика
#include "blocking_queue.hpp" // Очередь задач

// ─────────────────────────────────────────────────────────────
// Анализатор 1: Подсчёт ошибок по уровням
// ─────────────────────────────────────────────────────────────
void count_errors(blocking_queue<LogEntry>& queue, atomic_stats& stats) {
    for (;;) {
        // Пытаемся извлечь запись из очереди
        auto entry = queue.try_pop();

        // Если очередь пуста
        if (!entry.has_value()) {
            // Проверяем, не остановлен ли пул
            if (queue.is_shutdown()) {
                break;  // Выходим из цикла, завершаем поток
            }
            // Ждём немного перед следующей попыткой
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Обновляем статистику атомарно
        stats.total++;

        switch (entry->level) {
        case LogLevel::ERROR:
        case LogLevel::FATAL:
            stats.errors++;
            break;
        case LogLevel::WARN:
            stats.warnings++;
            break;
        case LogLevel::DEBUG:
            stats.debug_count++;
            break;
        case LogLevel::INFO:
            stats.info_count++;
            break;
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Анализатор 2: Поиск паттернов в сообщениях
// ─────────────────────────────────────────────────────────────
void find_patterns(blocking_queue<LogEntry>& queue, atomic_stats& stats) {
    // Словарь паттернов для поиска
    const std::vector<std::string> patterns = {
        "timeout",
        "failed",
        "error",
        "denied",
        "exhausted"
    };

    for (;;) {
        auto entry = queue.try_pop();

        if (!entry.has_value()) {
            if (queue.is_shutdown()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Ищем паттерны в сообщении (регистронезависимо)
        std::string msg_lower = entry->message;
        std::transform(msg_lower.begin(), msg_lower.end(), msg_lower.begin(), ::tolower);

        for (const auto& pattern : patterns) {
            if (msg_lower.find(pattern) != std::string::npos) {
                // Нашли паттерн — можно логировать или считать
                // В данной версии просто считаем общее количество
                break;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────
// Анализатор 3: Агрегация метрик по источникам
// ─────────────────────────────────────────────────────────────
void aggregate_metrics(blocking_queue<LogEntry>& queue, atomic_stats& stats) {
    for (;;) {
        auto entry = queue.try_pop();

        if (!entry.has_value()) {
            if (queue.is_shutdown()) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Здесь можно вести статистику по источникам (entry->source)
        // Для простоты просто считаем общее количество
        // В расширенной версии можно использовать std::map<std::string, std::atomic<size_t>>
    }
}