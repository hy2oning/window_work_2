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

