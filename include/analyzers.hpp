// include/analyzers.hpp
#pragma once

#include "blocking_queue.hpp"
#include "log_entry.hpp"
#include "stats.hpp"
#include <thread>
#include <chrono>
#include <algorithm>
#include <vector>
#include <string>

// Объявления функций-анализаторов
void count_errors(blocking_queue<LogEntry>& queue, atomic_stats& stats);
void find_patterns(blocking_queue<LogEntry>& queue, atomic_stats& stats);
void aggregate_metrics(blocking_queue<LogEntry>& queue, atomic_stats& stats);