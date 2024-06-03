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

using namespace std;

std::mutex mtx;  // For synchronization

enum ProcessType { FG, BG };

struct Process {
    int pid;                // 프로세스 ID
    ProcessType type;       // 프로세스 타입
};

struct Command {
    std::string name;
    std::vector<std::string> args;
    int repeat = 1;
    int duration = 300; // Default duration (seconds)
    int period = 0;
    int threads = 1;
};

deque<Process*> fg_list;    // Foreground 프로세스 리스트
deque<Process*> bg_list;    // Background 프로세스 리스트
deque<pair<Process*, int>> wait_queue;  // Wait Queue (대기 큐)
int current_pid = 0;       // 현재 프로세스 ID
const int threshold = 10;  // 임계값
Process* create_process(ProcessType type) {
    return new Process{ current_pid++, type };
}

void print_queue_status() {
    lock_guard<mutex> lock(mtx);
    cout << "Foreground Queue: ";
    for (auto& process : fg_list) {
        cout << process->pid << (process->type == FG ? "F " : "B ");
    }
    cout << endl;

    cout << "Background Queue: ";
    for (auto& process : bg_list) {
        cout << process->pid << (process->type == FG ? "F " : "B ");
    }
    cout << endl;

    cout << "Wait Queue: ";
    for (auto& entry : wait_queue) {
        cout << entry.first->pid << (entry.first->type == FG ? "F:" : "B:") << entry.second << " ";
    }
    cout << endl;
}


void enqueue(Process* process) {
    lock_guard<mutex> lock(mtx);  // 다른 스레드와의 동시 접근을 방지하기 위해 mutex를 사용하여 락을 겁니다.
    if (process->type == FG) {
        fg_list.push_back(process);  // Foreground 프로세스는 fg_list에 삽입
    }
    else {
        bg_list.push_back(process);  // Background 프로세스는 bg_list에 삽입
    }
}

Process* dequeue() {
    lock_guard<mutex> lock(mtx);
    Process* process = nullptr;

    if (!fg_list.empty()) {
        // Foreground 리스트가 비어있지 않으면 첫 번째 프로세스를 가져옴
        process = fg_list.front();
        fg_list.pop_front();

        // 리스트가 비어있으면 해당 리스트 노드를 제거
        if (fg_list.empty()) {
            fg_list.clear();
        }
    }
    else if (!bg_list.empty()) {
        // Foreground 리스트가 비어있고 Background 리스트가 비어있지 않으면 첫 번째 프로세스를 가져옴
        process = bg_list.front();
        bg_list.pop_front();

        // 리스트가 비어있으면 해당 리스트 노드를 제거
        if (bg_list.empty()) {
            bg_list.clear();
        }
    }

    return process;
}


void promote() {
    lock_guard<mutex> lock(mtx);
    static auto promote_pointer = bg_list.begin();

    if (!bg_list.empty()) {
        // 현재 promote 포인터가 가리키는 프로세스를 가져옵니다.
        Process* process = *promote_pointer;

        // promote 포인터를 지우고 다음 위치로 이동합니다.
        promote_pointer = bg_list.erase(promote_pointer);
        if (promote_pointer == bg_list.end()) promote_pointer = bg_list.begin();

        // Foreground 리스트의 끝에 프로세스를 추가합니다.
        fg_list.push_back(process);

        // Background 리스트가 비어 있으면 promote 포인터를 초기화합니다.
        if (bg_list.empty() && promote_pointer != bg_list.end()) {
            promote_pointer = bg_list.erase(promote_pointer);
        }
    }

    // 새로운 스택 노드를 생성하는 조건을 추가합니다.
    if (fg_list.size() == 1 && fg_list.front()->type == FG) {
        Process* new_node = create_process(FG);
        fg_list.push_front(new_node);
    }

    // Clock-wise 순서로 다음 노드를 가리킵니다.
    promote_pointer++;
    if (promote_pointer == bg_list.end()) {
        promote_pointer = bg_list.begin();
    }
}


void split_and_merge() {
    lock_guard<mutex> lock(mtx);
    if (bg_list.size() > threshold) {  // Background 리스트의 크기가 임계값을 초과하면
        vector<Process*> upper_half(bg_list.begin(), bg_list.begin() + bg_list.size() / 2);  // 앞쪽 절반을 잘라내어
        bg_list.erase(bg_list.begin(), bg_list.begin() + bg_list.size() / 2);  // 리스트에서 제거합니다.
        fg_list.insert(fg_list.end(), upper_half.begin(), upper_half.end());  // Foreground 리스트의 끝에 추가합니다.

        if (fg_list.size() > threshold) {
            split_and_merge();  // Foreground 리스트도 임계값을 초과하면 재귀적으로 호출합니다.
        }
    }
}


void test_enqueue() {
    Process* p1 = create_process(FG);
    Process* p2 = create_process(BG);
    enqueue(p1);
    enqueue(p2);

    cout << "Foreground Queue: ";
    for (auto& process : fg_list) {
        cout << process->pid << " ";
    }
    cout << endl;

    cout << "Background Queue: ";
    for (auto& process : bg_list) {
        cout << process->pid << " ";
    }
    cout << endl;
}

void test_dequeue() {
    // 초기 상태 설정
    Process* p1 = create_process(FG);
    Process* p2 = create_process(BG);
    enqueue(p1);
    enqueue(p2);

    cout << "Initial state:" << endl;
    print_queue_status();

    // 첫 번째 dequeue 호출
    Process* dq1 = dequeue();
    cout << "Dequeued Process 1: ID=" << dq1->pid << ", Type=" << (dq1->type == FG ? "FG" : "BG") << endl;
    print_queue_status();

    // 두 번째 dequeue 호출
    Process* dq2 = dequeue();
    cout << "Dequeued Process 2: ID=" << dq2->pid << ", Type=" << (dq2->type == FG ? "FG" : "BG") << endl;
    print_queue_status();

    delete p1;
    delete p2;
}

void test_promote() {
    // 초기 상태 설정
    for (int i = 0; i < 5; ++i) {
        Process* p = create_process(BG);
        enqueue(p);
    }

    // 상태 출력
    cout << "Initial state:" << endl;
    print_queue_status();

    // promote 함수 호출
    promote();

    // 상태 출력
    cout << "After promote:" << endl;
    print_queue_status();
}


void test_split_and_merge() {
    for (int i = 0; i < 15; ++i) {
        Process* p = create_process(BG);
        enqueue(p);
    }

    split_and_merge();

    cout << "Foreground Queue after split_and_merge: ";
    for (auto& process : fg_list) {
        cout << process->pid << " ";
    }
    cout << endl;

    cout << "Background Queue after split_and_merge: ";
    for (auto& process : bg_list) {
        cout << process->pid << " ";
    }
    cout << endl;
}

void FG_function() {
    // Implement foreground processing tasks here
}

void BG_function() {
    // Implement background processing tasks here
}

void shell_function() {
    std::ifstream infile("commands.txt");
    if (!infile.is_open()) {
        std::cerr << "Failed to open commands.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) continue;

        std::string command = tokens[0];

        if (command == "echo" && tokens.size() == 2) {
            std::cout << tokens[1] << std::endl;
        }
        else {
            std::cerr << "Unknown command or wrong arguments: " << line << std::endl;
        }
    }
}



int main() {
    /* test_enqueue();*/
    /* test_dequeue();*/
    /* test_promote();*/
    /* test_split_and_merge();*/

    std::thread FG(FG_function);
    std::thread BG(BG_function);
    std::thread shell(shell_function);

    FG.join();
    BG.join();
    shell.join();

    return 0;
}