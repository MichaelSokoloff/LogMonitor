// src/main.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include "blocking_queue.hpp"
#include "log_entry.hpp"
#include "stats.hpp"
#include "threadpool.hpp"
#include "analyzers.hpp"  // ← Добавили

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: LogMonitor <logfile>\n";
        return 1;
    }

    std::cout << "=== LogMonitor Started ===\n\n";

    // Инициализация
    blocking_queue<LogEntry> queue(1000);
    atomic_stats stats;
    threadpool pool(std::thread::hardware_concurrency());

    // ─────────────────────────────────────────────────────────
    // Запуск анализаторов через пул потоков
    // ─────────────────────────────────────────────────────────
    auto future1 = pool.enqueue([&queue, &stats]() {
        count_errors(queue, stats);  // ← Вызов вынесенной функции
        });

    auto future2 = pool.enqueue([&queue, &stats]() {
        find_patterns(queue, stats);  // ← Вызов вынесенной функции
        });

    auto future3 = pool.enqueue([&queue, &stats]() {
        aggregate_metrics(queue, stats);  // ← Вызов вынесенной функции
        });

    // Producer: чтение файла
    std::thread reader([&queue, filename = argv[1]]() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << "\n";
            queue.shutdown();
            return;
        }

        std::string line;
        size_t line_count = 0;
        while (std::getline(file, line)) {
            if (auto entry = LogEntry::parse(line)) {
                queue.push(std::move(*entry));
                line_count++;

                if (line_count % 1000 == 0) {
                    std::cout << "Read " << line_count << " lines...\n";
                }
            }
        }

        std::cout << "Finished reading " << line_count << " lines\n";
        queue.shutdown();
        });

    // Мониторинг
    while (!queue.is_shutdown() || queue.size() > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        stats.print_report();
    }

    // Завершение
    reader.join();
    future1.wait();
    future2.wait();
    future3.wait();

    stats.print_final_report();
    std::cout << "\n=== Processing Complete ===\n";

    return 0;
}