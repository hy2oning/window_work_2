#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <algorithm>
#include <windows.h>
#include <sstream>
#include <mutex>
#include <chrono>
#include <functional>
#include <algorithm>
#include <numeric>

using namespace std;

std::mutex mtx;  // For synchronization

enum ProcessType { FG, BG };

struct Process {
    int pid;                // ���μ��� ID
    ProcessType type;       // ���μ��� Ÿ��
};

struct Command {
    std::string name;
    std::vector<std::string> args;
    int repeat = 1;
    int duration = 300; // Default duration (seconds)
    int period = 0;
    int threads = 1;
};

deque<Process*> fg_list;    // Foreground ���μ��� ����Ʈ
deque<Process*> bg_list;    // Background ���μ��� ����Ʈ
deque<pair<Process*, int>> wait_queue;  // Wait Queue (��� ť)
int current_pid = 0;       // ���� ���μ��� ID
const int threshold = 10;  // �Ӱ谪
//Process* create_process(ProcessType type) {
//    return new Process{ current_pid++, type };
//}
//
//void print_queue_status() {
//    lock_guard<mutex> lock(mtx);
//    cout << "Foreground Queue: ";
//    for (auto& process : fg_list) {
//        cout << process->pid << (process->type == FG ? "F " : "B ");
//    }
//    cout << endl;
//
//    cout << "Background Queue: ";
//    for (auto& process : bg_list) {
//        cout << process->pid << (process->type == FG ? "F " : "B ");
//    }
//    cout << endl;
//
//    cout << "Wait Queue: ";
//    for (auto& entry : wait_queue) {
//        cout << entry.first->pid << (entry.first->type == FG ? "F:" : "B:") << entry.second << " ";
//    }
//    cout << endl;
//}
//
//
//void enqueue(Process* process) {
//    lock_guard<mutex> lock(mtx);  // �ٸ� ��������� ���� ������ �����ϱ� ���� mutex�� ����Ͽ� ���� �̴ϴ�.
//    if (process->type == FG) {
//        fg_list.push_back(process);  // Foreground ���μ����� fg_list�� ����
//    }
//    else {
//        bg_list.push_back(process);  // Background ���μ����� bg_list�� ����
//    }
//}
//
//Process* dequeue() {
//    lock_guard<mutex> lock(mtx);
//    Process* process = nullptr;
//
//    if (!fg_list.empty()) {
//        // Foreground ����Ʈ�� ������� ������ ù ��° ���μ����� ������
//        process = fg_list.front();
//        fg_list.pop_front();
//
//        // ����Ʈ�� ��������� �ش� ����Ʈ ��带 ����
//        if (fg_list.empty()) {
//            fg_list.clear();
//        }
//    }
//    else if (!bg_list.empty()) {
//        // Foreground ����Ʈ�� ����ְ� Background ����Ʈ�� ������� ������ ù ��° ���μ����� ������
//        process = bg_list.front();
//        bg_list.pop_front();
//
//        // ����Ʈ�� ��������� �ش� ����Ʈ ��带 ����
//        if (bg_list.empty()) {
//            bg_list.clear();
//        }
//    }
//
//    return process;
//}
//
//
//void promote() {
//    lock_guard<mutex> lock(mtx);
//    static auto promote_pointer = bg_list.begin();
//
//    if (!bg_list.empty()) {
//        // ���� promote �����Ͱ� ����Ű�� ���μ����� �����ɴϴ�.
//        Process* process = *promote_pointer;
//
//        // promote �����͸� ����� ���� ��ġ�� �̵��մϴ�.
//        promote_pointer = bg_list.erase(promote_pointer);
//        if (promote_pointer == bg_list.end()) promote_pointer = bg_list.begin();
//
//        // Foreground ����Ʈ�� ���� ���μ����� �߰��մϴ�.
//        fg_list.push_back(process);
//
//        // Background ����Ʈ�� ��� ������ promote �����͸� �ʱ�ȭ�մϴ�.
//        if (bg_list.empty() && promote_pointer != bg_list.end()) {
//            promote_pointer = bg_list.erase(promote_pointer);
//        }
//    }
//
//    // ���ο� ���� ��带 �����ϴ� ������ �߰��մϴ�.
//    if (fg_list.size() == 1 && fg_list.front()->type == FG) {
//        Process* new_node = create_process(FG);
//        fg_list.push_front(new_node);
//    }
//
//    // Clock-wise ������ ���� ��带 ����ŵ�ϴ�.
//    promote_pointer++;
//    if (promote_pointer == bg_list.end()) {
//        promote_pointer = bg_list.begin();
//    }
//}
//
//
//void split_and_merge() {
//    lock_guard<mutex> lock(mtx);
//    if (bg_list.size() > threshold) {  // Background ����Ʈ�� ũ�Ⱑ �Ӱ谪�� �ʰ��ϸ�
//        vector<Process*> upper_half(bg_list.begin(), bg_list.begin() + bg_list.size() / 2);  // ���� ������ �߶󳻾�
//        bg_list.erase(bg_list.begin(), bg_list.begin() + bg_list.size() / 2);  // ����Ʈ���� �����մϴ�.
//        fg_list.insert(fg_list.end(), upper_half.begin(), upper_half.end());  // Foreground ����Ʈ�� ���� �߰��մϴ�.
//
//        if (fg_list.size() > threshold) {
//            split_and_merge();  // Foreground ����Ʈ�� �Ӱ谪�� �ʰ��ϸ� ��������� ȣ���մϴ�.
//        }
//    }
//}
//
//
//void test_enqueue() {
//    Process* p1 = create_process(FG);
//    Process* p2 = create_process(BG);
//    enqueue(p1);
//    enqueue(p2);
//
//    cout << "Foreground Queue: ";
//    for (auto& process : fg_list) {
//        cout << process->pid << " ";
//    }
//    cout << endl;
//
//    cout << "Background Queue: ";
//    for (auto& process : bg_list) {
//        cout << process->pid << " ";
//    }
//    cout << endl;
//}
//
//void test_dequeue() {
//    // �ʱ� ���� ����
//    Process* p1 = create_process(FG);
//    Process* p2 = create_process(BG);
//    enqueue(p1);
//    enqueue(p2);
//
//    cout << "Initial state:" << endl;
//    print_queue_status();
//
//    // ù ��° dequeue ȣ��
//    Process* dq1 = dequeue();
//    cout << "Dequeued Process 1: ID=" << dq1->pid << ", Type=" << (dq1->type == FG ? "FG" : "BG") << endl;
//    print_queue_status();
//
//    // �� ��° dequeue ȣ��
//    Process* dq2 = dequeue();
//    cout << "Dequeued Process 2: ID=" << dq2->pid << ", Type=" << (dq2->type == FG ? "FG" : "BG") << endl;
//    print_queue_status();
//
//    delete p1;
//    delete p2;
//}
//
//void test_promote() {
//    // �ʱ� ���� ����
//    for (int i = 0; i < 5; ++i) {
//        Process* p = create_process(BG);
//        enqueue(p);
//    }
//
//    // ���� ���
//    cout << "Initial state:" << endl;
//    print_queue_status();
//
//    // promote �Լ� ȣ��
//    promote();
//
//    // ���� ���
//    cout << "After promote:" << endl;
//    print_queue_status();
//}
//
//
//void test_split_and_merge() {
//    for (int i = 0; i < 15; ++i) {
//        Process* p = create_process(BG);
//        enqueue(p);
//    }
//
//    split_and_merge();
//
//    cout << "Foreground Queue after split_and_merge: ";
//    for (auto& process : fg_list) {
//        cout << process->pid << " ";
//    }
//    cout << endl;
//
//    cout << "Background Queue after split_and_merge: ";
//    for (auto& process : bg_list) {
//        cout << process->pid << " ";
//    }
//    cout << endl;
//}

void FG_function(const std::string& line);
void BG_function(const std::string& line);
void run_command_with_options(const std::string& command, const std::vector<std::string>& args, int n, int duration, int period);

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int count_primes(int n) {
    if (n < 2) return 0;
    std::vector<bool> is_prime(n + 1, true);
    is_prime[0] = is_prime[1] = false;
    for (int i = 2; i * i <= n; ++i) {
        if (is_prime[i]) {
            for (int j = i * i; j <= n; j += i) {
                is_prime[j] = false;
            }
        }
    }
    return std::count(is_prime.begin(), is_prime.end(), true);
}

long long sum_range(int start, int end) {
    long long sum = 0;
    for (int i = start; i <= end; ++i) {
        sum += i;
    }
    return sum;
}

int sum_mod(int x, int m) {
    if (m <= 1) {
        long long sum = (long long)x * (x + 1) / 2;
        return sum % 1000000;
    }

    int step = x / m;
    std::vector<std::thread> threads;
    std::vector<long long> results(m, 0);

    for (int i = 0; i < m; ++i) {
        int start = i * step + 1;
        int end = (i == m - 1) ? x : (i + 1) * step;
        threads.emplace_back([&results, i, start, end]() {
            results[i] = sum_range(start, end);
            });
    }

    for (auto& t : threads) {
        t.join();
    }

    long long total_sum = std::accumulate(results.begin(), results.end(), 0LL);
    return total_sum % 1000000;
}

void execute_command(const std::string& command, const std::vector<std::string>& args) {
    std::lock_guard<std::mutex> lock(mtx);  // Lock for thread-safe output
    if (command == "echo" && args.size() == 1) {
        std::cout << args[0] << std::endl;
    }
    else if (command == "dummy") {
        // Do nothing
    }
    else if (command == "gcd" && args.size() == 2) {
        int x = std::stoi(args[0]);
        int y = std::stoi(args[1]);
        std::cout << gcd(x, y) << std::endl;
    }
    else if (command == "prime" && args.size() == 1) {
        int x = std::stoi(args[0]);
        std::cout << count_primes(x) << std::endl;
    }
    else if (command == "sum" && (args.size() == 1 || args.size() == 2)) {
        int x = std::stoi(args[0]);
        int m = (args.size() == 2) ? std::stoi(args[1]) : 1;
        std::cout << sum_mod(x, m) << std::endl;
    }
    else {
        std::cerr << "Unknown command or wrong arguments: " << command << std::endl;
    }
}

void run_command_with_options(const std::string& command, const std::vector<std::string>& args, int n, int duration, int period) {
    auto task = [&command, &args, period, duration]() {
        auto start = std::chrono::steady_clock::now();
        while (true) {
            execute_command(command, args);
            if (period > 0) {
                std::this_thread::sleep_for(std::chrono::seconds(period));
            }
            if (duration > 0) {
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= duration) {
                    break;
                }
            }
            else {
                break;
            }
        }
        };

    std::vector<std::thread> threads;
    for (int i = 0; i < n; ++i) {
        threads.emplace_back(task);
    }
    for (auto& t : threads) {
        t.join();
    }
}

void FG_function(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) return;

    std::string command = tokens[0];
    std::vector<std::string> args;
    int n = 1;  // Number of processes
    int duration = 0;  // Duration in seconds
    int period = 0;  // Period in seconds
    int m = 1;  // Number of threads for sum command

    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i] == "-n" && i + 1 < tokens.size()) {
            n = std::stoi(tokens[++i]);
        }
        else if (tokens[i] == "-d" && i + 1 < tokens.size()) {
            duration = std::stoi(tokens[++i]);
        }
        else if (tokens[i] == "-p" && i + 1 < tokens.size()) {
            period = std::stoi(tokens[++i]);
        }
        else if (tokens[i] == "-m" && i + 1 < tokens.size() && command == "sum") {
            m = std::stoi(tokens[++i]);
        }
        else {
            args.push_back(tokens[i]);
        }
    }

    if (command == "sum" && m > 1) {
        args.push_back(std::to_string(m));
    }

    run_command_with_options(command, args, n, duration, period);
}

void BG_function(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) return;

    std::string command = tokens[0];
    std::vector<std::string> args;
    int n = 1;  // Number of processes
    int duration = 0;  // Duration in seconds
    int period = 0;  // Period in seconds
    int m = 1;  // Number of threads for sum command

    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i] == "-n" && i + 1 < tokens.size()) {
            n = std::stoi(tokens[++i]);
        }
        else if (tokens[i] == "-d" && i + 1 < tokens.size()) {
            duration = std::stoi(tokens[++i]);
        }
        else if (tokens[i] == "-p" && i + 1 < tokens.size()) {
            period = std::stoi(tokens[++i]);
        }
        else if (tokens[i] == "-m" && i + 1 < tokens.size() && command == "sum") {
            m = std::stoi(tokens[++i]);
        }
        else {
            args.push_back(tokens[i]);
        }
    }

    if (command == "sum" && m > 1) {
        args.push_back(std::to_string(m));
    }

    run_command_with_options(command, args, n, duration, period);
}

void shell_function() {
    std::ifstream infile("commands.txt");
    if (!infile.is_open()) {
        std::cerr << "Failed to open commands.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;

        if (line[0] == '&') {
            line = line.substr(1);  // Remove '&' character
            std::thread bg(BG_function, line);
            bg.detach();
        }
        else {
            FG_function(line);
        }
    }
}


int main() {
    /* test_enqueue();*/
    /* test_dequeue();*/
    /* test_promote();*/
    /* test_split_and_merge();*/

    std::thread shell(shell_function);

    shell.join();

    return 0;
}