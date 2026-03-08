#pragma once

#include <atomic>
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>

struct atomic_stats {
    std::atomic<size_t> total{ 0 };
    std::atomic<size_t> errors{ 0 };
    std::atomic<size_t> warnings{ 0 };
    std::atomic<size_t> debug_count{ 0 };
    std::atomic<size_t> info_count{ 0 };
    std::chrono::steady_clock::time_point start_time;

    atomic_stats() : start_time(std::chrono::steady_clock::now()) {}

    void print_report() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

        double rate = elapsed > 0 ? static_cast<double>(total.load()) / elapsed : 0;

        std::cout << "\n=== Statistics Report ===\n";
        std::cout << "Total entries: " << total.load() << "\n";
        std::cout << "Errors: " << errors.load()
            << " (" << (total.load() > 0 ? 100.0 * errors.load() / total.load() : 0) << "%)\n";
        std::cout << "Warnings: " << warnings.load()
            << " (" << (total.load() > 0 ? 100.0 * warnings.load() / total.load() : 0) << "%)\n";
        std::cout << "Debug: " << debug_count.load() << "\n";
        std::cout << "Info: " << info_count.load() << "\n";
        std::cout << "Processing rate: " << static_cast<size_t>(rate) << " entries/sec\n";
        std::cout << "Elapsed time: " << elapsed << " sec\n";
        std::cout << "========================\n\n";
    }

    void print_final_report() const {
        print_report();

        std::cout << "\n=== Final Report ===\n";
        std::cout << "Total processed: " << total.load() << " entries\n";
        std::cout << "Error rate: " << (total.load() > 0 ? 100.0 * errors.load() / total.load() : 0) << "%\n";
    }
};