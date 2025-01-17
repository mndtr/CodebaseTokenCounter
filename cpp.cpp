#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <cstddef>
#include <algorithm>
#include <cctype>
#include <thread>
#include <mutex>
#include <sstream>
#include <atomic>
#include <memory>

namespace fs = std::filesystem;

// Генерация дерева директорий
std::string generate_tree(const fs::path& path, const std::string& prefix = "") {
    std::string tree;
    std::vector<fs::path> entries;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().filename() == ".git") continue;
        entries.push_back(entry.path());
    }
    std::sort(entries.begin(), entries.end(), [](const fs::path& a, const fs::path& b) {
        return a.filename() < b.filename();
    });
    for (size_t i = 0; i < entries.size(); ++i) {
        bool is_last = (i == entries.size() - 1);
        std::string connector = is_last ? "└── " : "├── ";
        if (fs::is_directory(entries[i])) {
            tree += prefix + connector + entries[i].filename().string() + "/\n";
        } else {
            tree += prefix + connector + entries[i].filename().string() + "\n";
        }
        if (fs::is_directory(entries[i])) {
            std::string new_prefix = prefix + (is_last ? "    " : "│   ");
            tree += generate_tree(entries[i], new_prefix);
        }
    }
    return tree;
}

// Проверка, является ли файл бинарным
bool is_binary(const fs::path& file_path) {
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file) return true;
        char buffer[1024];
        while (file.read(buffer, sizeof(buffer))) {
            for (int i = 0; i < file.gcount(); ++i) {
                if (static_cast<unsigned char>(buffer[i]) < 32 && 
                    buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t') {
                    return true;
                }
            }
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error checking binary file " << file_path << ": " << e.what() << '\n';
        return true;
    }
}

// Обработка файла и добавление его содержимого в output
void process_file(const fs::path& file_path, std::ostringstream& oss) {
    try {
        std::ifstream file(file_path, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << file_path << '\n';
            return;
        }
        oss << file_path.filename().string() << "```\n";
        oss << std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) << "\n```\n\n";
    } catch (const std::exception& e) {
        std::cerr << "Error processing file " << file_path << ": " << e.what() << '\n';
    }
}

// Обработка группы файлов в одном потоке
void process_file_chunk(const std::vector<fs::path>& files, std::ostringstream& oss) {
    for (const auto& file_path : files) {
        process_file(file_path, oss);
    }
}

// Основная функция для многопоточной обработки файлов
std::string process_files_multithreaded(const fs::path& path, int num_threads) {
    std::vector<fs::path> files_to_process;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                fs::path file_path = entry.path();
                if (file_path.parent_path().filename() == ".git") continue;
                if (!is_binary(file_path)) {
                    files_to_process.push_back(file_path);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error traversing directory " << path << ": " << e.what() << '\n';
    }
    std::sort(files_to_process.begin(), files_to_process.end());

    // Разделение файлов на chunks для многопоточной обработки
    std::vector<std::vector<fs::path>> chunks(num_threads);
    int chunk_size = files_to_process.size() / num_threads;
    int remainder = files_to_process.size() % num_threads;
    int start = 0;
    for (int i = 0; i < num_threads; ++i) {
        int current_chunk_size = chunk_size + (i < remainder ? 1 : 0);
        chunks[i] = std::vector<fs::path>(files_to_process.begin() + start, files_to_process.begin() + start + current_chunk_size);
        start += current_chunk_size;
    }

    // Многопоточная обработка
    std::vector<std::ostringstream> thread_outputs(num_threads);
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            process_file_chunk(chunks[i], thread_outputs[i]);
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    // Объединение результатов
    std::ostringstream final_output;
    for (auto& oss : thread_outputs) {
        final_output << oss.str();
    }
    return final_output.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_traverse>\n";
        return 1;
    }
    fs::path path(argv[1]);
    if (!fs::exists(path) || !fs::is_directory(path)) {
        std::cerr << "The provided path is not a directory or doesn't exist.\n";
        return 1;
    }

    // Генерация дерева директорий
    std::string tree = generate_tree(path);

    // Обработка файлов с использованием многопоточности
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads <= 0) num_threads = 4; // Fallback to 4 threads if hardware_concurrency() returns 0
    std::string output = process_files_multithreaded(path, num_threads);

    // Объединение дерева и содержимого файлов
    std::string final_output = tree + '\n' + output;

    // Запись результата в файл prompt.txt
    fs::path prompt_file = path.parent_path() / "prompt.txt";
    std::ofstream f(prompt_file);
    if (!f.is_open()) {
        std::cerr << "Error writing to prompt.txt\n";
        return 1;
    }
    f.write(final_output.c_str(), final_output.size());
    f.close();
    std::cout << "prompt.txt has been created at " << prompt_file << '\n';

    // Подсчет токенов в prompt.txt
    std::ifstream infile(prompt_file);
    if (!infile.is_open()) {
        std::cerr << "Error reading prompt.txt for token counting\n";
        return 1;
    }
    std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    infile.close();

    // Упрощенный подсчет токенов (по пробелам)
    std::size_t token_count = 0;
    bool in_token = false;
    for (char c : content) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (in_token) {
                ++token_count;
                in_token = false;
            }
        } else {
            in_token = true;
        }
    }
    if (in_token) ++token_count;

    std::cout << "Number of tokens in prompt.txt: " << token_count << '\n';
    return 0;
}