#include <stdio.h>
#include <stdlib.h> // rand()
#include <time.h> // time

#define MAX_PROCESS_NUM 10
#define MAX_CPU_BURST_TIME 10
#define MAX_IO_BURST_TIME 3
#define MAX_PRIORITY 10
#define MAX_ARRIVAL_TIME 20

typedef struct{
    int pid;
    int cpu_burst_time;
    int io_burst_time;
    int arrival_time;
    int priority;
    int turnaround_time;
    int waiting_time;
    int remaining_time;
} process;

process waiting_queue[MAX_PROCESS_NUM];
process ready_queue[MAX_PROCESS_NUM];
process job_queue[MAX_PROCESS_NUM];
process terminated_queue[MAX_PROCESS_NUM];

int Time = 0;
int waiting_process_num = 0;
int ready_process_num = 0;
int job_process_num = 0;
int terminated_process_num = 0;
char *seperate = "=========================================================\n";

void print_status(process *queue) {
    puts("+-----+--------------+-----------------+--------------+----------------+---------------+----------------+----------+");
    puts("| PID | Waiting Time | Turnaround Time | Arrival Time | CPU Busrt Time | IO Burst Time | Remaining Time | Priority |");
    puts("+-----+--------------+-----------------+--------------+----------------+---------------+----------------+----------+");

    for (int i = 0; i < MAX_PROCESS_NUM; i++) {
        if (queue[i].pid == 0)
            break;
        printf("| %2d |      %2d      |        %2d       |      %2d      |       %2d       |       %2d      |       %2d       |    %2d    |\n"
               , queue[i].pid, queue[i].waiting_time, queue[i].turnaround_time, queue[i].arrival_time, queue[i].cpu_burst_time, queue[i].io_burst_time, queue[i].remaining_time, queue[i].priority);
        puts("+-----+--------------+-----------------+--------------+----------------+---------------+----------------+----------+");
    }
    printf("\n\n");
}

void init_process(process *to_init) {
    to_init->pid = 0;
    to_init->cpu_burst_time = 0;
    to_init->io_burst_time = 0;
    to_init->arrival_time = 0;
    to_init->priority = 0;
    to_init->turnaround_time = 0;
    to_init->waiting_time = 0;
    to_init->remaining_time = 0;
}
void update_process(process *to_update, process *target) {
    to_update->pid = target->pid;
    to_update->cpu_burst_time = target->cpu_burst_time;
    to_update->io_burst_time = target->io_burst_time;
    to_update->arrival_time = target->arrival_time;
    to_update->priority = target->priority;
    to_update->turnaround_time = target->turnaround_time;
    to_update->waiting_time = target->waiting_time;
    to_update->remaining_time = target->remaining_time;
}
int find_index(process *queue, int pid) {
    // pid�� unique�� ���̹Ƿ� index�� search�ϱ⿡ ������
    for (int i = 0; i < MAX_PROCESS_NUM; i++) {
        if (queue[i].pid == pid)
            return i;
    }
    return -1;
}

void create_process() {
    if (job_process_num == MAX_PROCESS_NUM) {
        printf("Error : Job queue is full\n");
        return ;
    }

    int pid;
    if (job_process_num == 0) {
        pid = 100;
    } else {
        pid = job_queue[job_process_num-1].pid + rand()%5 + 1; // ���������� ������ process�� pid���� 1�̻� 5������ random�� ����
    }
    int cpu_burst_time = rand()%MAX_CPU_BURST_TIME + 5; // 5 �̻�
    int io_burst_time = rand()%MAX_IO_BURST_TIME;
    int arrival_time = rand()%MAX_ARRIVAL_TIME;
    int priority = rand()%MAX_PRIORITY + 1; // 1 �̻�

    process new_process;
    new_process.pid = pid;
    new_process.cpu_burst_time = cpu_burst_time;
    new_process.io_burst_time = io_burst_time;
    new_process.priority = priority;
    new_process.arrival_time = arrival_time;
    new_process.turnaround_time = 0;
    new_process.waiting_time = 0;

    job_queue[job_process_num++] = new_process;    
}

void job_queue_to_ready_queue(int idx) {
    // arrival time�� �� process�� job queue���� ready queue�� �ű�� ����
    if (ready_process_num == MAX_PROCESS_NUM) {
        printf("Error : Ready queue is full\n");
        return ;
    }
    update_process(&ready_queue[ready_process_num++], &job_queue[idx]); // jon queue���� ready queue�� �ű��

    if (idx == job_process_num-1) {
        init_process(&job_queue[idx]); // ������ process�� ���, �׳� �ʱ�ȭ
    } else {
        for (int i = idx; i < job_process_num-1; i++) {
            job_queue[i] = job_queue[i+1];
        }
        init_process(&job_queue[job_process_num-1]); // �ϳ��� �մ������ ������ index �ʱ�ȭ
    }
    job_process_num--;
}

void ready_queue_to_terminated_queue(int idx) {
    if (terminated_process_num == MAX_PROCESS_NUM) {
        printf("Error : Ready queue is full\n");
        return ;
    }

    update_process(&terminated_queue[terminated_process_num++], &ready_queue[idx]);

    if (idx == ready_process_num-1) {
        init_process(&ready_queue[idx]);
    } else {
        for (int i = idx; i < ready_process_num-1; i++) {
            ready_queue[i] = ready_queue[i+1];
        }
        init_process(&ready_queue[ready_process_num-1]);
    }
    ready_process_num--;
}

void FCFS() {
    while(terminated_process_num < MAX_PROCESS_NUM) {
        int ori_job_process_num = job_process_num;

        for (int i = 0; i < job_process_num; i++) {
            if (job_queue[i].arrival_time <= Time) {
                job_queue_to_ready_queue(i);
                i--;
            }
        }
        if (ori_job_process_num == job_process_num)
            Time++;

        if (ready_process_num == 0)
            continue ;

        while(ready_process_num) {
            // ���� ���� ������ process ã��
            int fastest_idx = -1;
            for (int i = 0; i < ready_process_num; i++) {
                if (fastest_idx == -1 || ready_queue[i].arrival_time < ready_queue[fastest_idx].arrival_time)
                    fastest_idx = i;
            }

            (&ready_queue[fastest_idx])->waiting_time = Time;
            (&ready_queue[fastest_idx])->turnaround_time = ready_queue[fastest_idx].cpu_burst_time;
            Time += ready_queue[fastest_idx].cpu_burst_time;
            ready_queue_to_terminated_queue(fastest_idx);
            (&terminated_queue[terminated_process_num-1])->waiting_time -= terminated_queue[0].arrival_time; // ���� ó�� terminate�� process�� arrival time�� �� process�� waiting time���� ������
        }
    }
}

void non_preemptive_SJF();
void preemptive_SJF();
void preemptive_priority();
void round_robin();

void evaluation();

void simulate() {
    puts("job queue :");
    print_status(job_queue);
    FCFS();
    puts("terminated queue :");
    print_status(terminated_queue);
}

int main() {
    srand(time(NULL));

    for (int i = 0; i < MAX_PROCESS_NUM; i++)
        create_process();
    process ori_job[MAX_PROCESS_NUM];

    for (int i = 0; i < MAX_PROCESS_NUM; i++)
        update_process(&ori_job[i], &job_queue[i]);

    simulate();

//    print_status(ori_job);

    return 0;
}

