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
    int pid;                // ���μ��� ID
    ProcessType type;       // ���μ��� Ÿ��
};

deque<Process*> fg_list;    // Foreground ���μ��� ����Ʈ
deque<Process*> bg_list;    // Background ���μ��� ����Ʈ
deque<pair<Process*, int>> wait_queue;  // Wait Queue (��� ť)
mutex mtx;                 // ����ȭ ��ü
int current_pid = 0;       // ���� ���μ��� ID
const int threshold = 10;  // �Ӱ谪

Process* create_process(ProcessType type) {
    return new Process{ current_pid++, type };
}

void enqueue(Process* process) {
    lock_guard<mutex> lock(mtx);  // �ٸ� ��������� ���� ������ �����ϱ� ���� mutex�� ����Ͽ� ���� �̴ϴ�.
    if (process->type == FG) {
        fg_list.push_back(process);  // Foreground ���μ����� fg_list�� ����
    }
    else {
        bg_list.push_back(process);  // Background ���μ����� bg_list�� ����
    }
}

Process* dequeue() {
    lock_guard<mutex> lock(mtx);
    if (!fg_list.empty()) {
        Process* process = fg_list.front();
        fg_list.pop_front();
        return process;
    }
    else if (!bg_list.empty()) {
        Process* process = bg_list.front();
        bg_list.pop_front();
        return process;
    }
    return nullptr;
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
    Process* p1 = create_process(FG);
    Process* p2 = create_process(BG);
    enqueue(p1);
    enqueue(p2);

    Process* dq1 = dequeue();
    cout << "Dequeued Process 1: ID=" << dq1->pid << ", Type=" << (dq1->type == FG ? "FG" : "BG") << endl;

    Process* dq2 = dequeue();
    cout << "Dequeued Process 2: ID=" << dq2->pid << ", Type=" << (dq2->type == FG ? "FG" : "BG") << endl;

    delete p1;
    delete p2;
}

int main() {
   /* test_enqueue();*/
    test_dequeue();
    return 0;
}

