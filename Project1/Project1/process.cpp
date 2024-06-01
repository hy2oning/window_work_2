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
    Process* process = nullptr;

    if (!fg_list.empty()) {
        // Foreground ����Ʈ�� ������� ������ ù ��° ���μ����� ������
        process = fg_list.front();
        fg_list.pop_front();

        // ����Ʈ�� ��������� �ش� ����Ʈ ��带 ����
        if (fg_list.empty()) {
            fg_list.clear();
        }
    }
    else if (!bg_list.empty()) {
        // Foreground ����Ʈ�� ����ְ� Background ����Ʈ�� ������� ������ ù ��° ���μ����� ������
        process = bg_list.front();
        bg_list.pop_front();

        // ����Ʈ�� ��������� �ش� ����Ʈ ��带 ����
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
        // ���� promote �����Ͱ� ����Ű�� ���μ����� �����ɴϴ�.
        Process* process = *promote_pointer;

        // promote �����͸� ����� ���� ��ġ�� �̵��մϴ�.
        promote_pointer = bg_list.erase(promote_pointer);
        if (promote_pointer == bg_list.end()) promote_pointer = bg_list.begin();

        // Foreground ����Ʈ�� ���� ���μ����� �߰��մϴ�.
        fg_list.push_back(process);

        // Background ����Ʈ�� ��� ������ promote �����͸� �ʱ�ȭ�մϴ�.
        if (bg_list.empty() && promote_pointer != bg_list.end()) {
            promote_pointer = bg_list.erase(promote_pointer);
        }
    }

    // ���ο� ���� ��带 �����ϴ� ������ �߰��մϴ�.
    if (fg_list.size() == 1 && fg_list.front()->type == FG) {
        Process* new_node = create_process(FG);
        fg_list.push_front(new_node);
    }

    // Clock-wise ������ ���� ��带 ����ŵ�ϴ�.
    promote_pointer++;
    if (promote_pointer == bg_list.end()) {
        promote_pointer = bg_list.begin();
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
    // �ʱ� ���� ����
    Process* p1 = create_process(FG);
    Process* p2 = create_process(BG);
    enqueue(p1);
    enqueue(p2);

    cout << "Initial state:" << endl;
    print_queue_status();

    // ù ��° dequeue ȣ��
    Process* dq1 = dequeue();
    cout << "Dequeued Process 1: ID=" << dq1->pid << ", Type=" << (dq1->type == FG ? "FG" : "BG") << endl;
    print_queue_status();

    // �� ��° dequeue ȣ��
    Process* dq2 = dequeue();
    cout << "Dequeued Process 2: ID=" << dq2->pid << ", Type=" << (dq2->type == FG ? "FG" : "BG") << endl;
    print_queue_status();

    delete p1;
    delete p2;
}

void test_promote() {
    // �ʱ� ���� ����
    for (int i = 0; i < 5; ++i) {
        Process* p = create_process(BG);
        enqueue(p);
    }

    // ���� ���
    cout << "Initial state:" << endl;
    print_queue_status();

    // promote �Լ� ȣ��
    promote();

    // ���� ���
    cout << "After promote:" << endl;
    print_queue_status();
}

int main() {
   /* test_enqueue();*/
   /* test_dequeue();*/
    test_promote();

    return 0;
}

