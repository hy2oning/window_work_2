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

using namespace std;

enum ProcessType { FG, BG };

struct Process {
    int pid;                // 프로세스 ID
    ProcessType type;       // 프로세스 타입
};

deque<Process*> fg_list;    // Foreground 프로세스 리스트
deque<Process*> bg_list;    // Background 프로세스 리스트
deque<pair<Process*, int>> wait_queue;  // Wait Queue (대기 큐)
mutex mtx;                 // 동기화 객체
int current_pid = 0;       // 현재 프로세스 ID
const int threshold = 10;  // 임계값

Process* create_process(ProcessType type) {
    return new Process{ current_pid++, type };
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

int main() {
    test_enqueue();
    return 0;
}

