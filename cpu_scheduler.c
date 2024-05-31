#include <stdio.h>
#include <stdlib.h> // rand()
#include <time.h> // time
#include <string.h> // strcat

#define MAX_PROCESS_NUM 3
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
    int cpu_remaining_time;
    int io_remaining_time;
} process;

process *waiting_queue[MAX_PROCESS_NUM];
process *ready_queue[MAX_PROCESS_NUM];
process *job_queue[MAX_PROCESS_NUM];
process *terminated_queue[MAX_PROCESS_NUM];

int Time = 0;
int waiting_process_num = 0;
int ready_process_num = 0;
int job_process_num = 0;
int terminated_process_num = 0;
process *running_process = NULL;
char gantt_middle[1000];
char gantt_top_down[1000];
char gantt_time[1000];

char *seperate = "=========================================================\n";

void print_status(process **queue) {
    puts("+-----+--------------+-----------------+--------------+----------------+---------------+----------+");
    puts("| PID | Waiting Time | Turnaround Time | Arrival Time | CPU Busrt Time | IO Burst Time | Priority |");
    puts("+-----+--------------+-----------------+--------------+----------------+---------------+----------+");

    for (int i = 0; i < MAX_PROCESS_NUM; i++) {
        if (!queue[i])
            break;

        printf("| %2d  |      %2d      |        %2d       |      %2d      |       %2d       |       %2d      |    %2d    |\n"
               , queue[i]->pid, queue[i]->waiting_time, queue[i]->turnaround_time, queue[i]->arrival_time, queue[i]->cpu_burst_time, queue[i]->io_burst_time, queue[i]->priority);
        puts("+-----+--------------+-----------------+--------------+----------------+---------------+----------+");
    }
    printf("\n\n");
}

int find_index(process *queue) {
    // pid�� unique�� ���̹Ƿ� index�� search�ϱ⿡ ������
    for (int i = 0; i < MAX_PROCESS_NUM; i++) {
        if (ready_queue[i]->pid == queue->pid)
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
    process *new_process;

    pid = job_process_num; // ���������� ������ process�� pid���� 1�̻� 5������ random�� ����

    int cpu_burst_time = rand()%MAX_CPU_BURST_TIME + 3; // 3 �̻�
    int io_burst_time = rand()%MAX_IO_BURST_TIME; // io burst time�� �ִ� 2�� �����Ͽ� waiting queue�� 2�� �̻��� ���μ����� �������� �ʵ��� ������
    int arrival_time = rand()%MAX_ARRIVAL_TIME;
    int priority = rand()%MAX_PRIORITY + 1; // 1 �̻�

    new_process = malloc(sizeof(process));
    new_process->pid = pid;
    new_process->cpu_burst_time = cpu_burst_time;
    new_process->io_burst_time = io_burst_time;
    new_process->priority = priority;
    new_process->arrival_time = arrival_time;
    new_process->turnaround_time = 0;
    new_process->waiting_time = 0;
    new_process->cpu_remaining_time = 0;
    new_process->io_remaining_time = 0;

    job_queue[job_process_num++] = new_process;    
}

void job_queue_to_ready_queue(int idx) {
    // arrival time�� �� process�� job queue���� ready queue�� �ű�� ����
    if (ready_process_num == MAX_PROCESS_NUM) {
        printf("Error : Ready queue is full\n");
        return ;
    }

    ready_queue[ready_process_num++] = job_queue[idx]; // jon queue���� ready queue�� �ű��

    if (idx == job_process_num-1) {
        job_queue[idx] = NULL; // ������ process�� ���, �׳� �ʱ�ȭ
    } else {
        for (int i = idx; i < job_process_num-1; i++) {
            job_queue[i] = job_queue[i+1];
        }
        job_queue[job_process_num-1] = NULL; // �ϳ��� �մ������ ������ index �ʱ�ȭ
    }
    job_process_num--;
}

void ready_queue_to_terminated_queue(int idx) {
    if (terminated_process_num == MAX_PROCESS_NUM) {
        printf("Error : Ready queue is full\n");
        return ;
    }

    terminated_queue[terminated_process_num++] = ready_queue[idx];

    if (idx == ready_process_num-1) {
        ready_queue[idx] = NULL;
    } else {
        for (int i = idx; i < ready_process_num-1; i++) {
            ready_queue[i] = ready_queue[i+1];
        }
        ready_queue[ready_process_num-1] = NULL;
    }
    ready_process_num--;
}

void ready_queue_to_waiting_queue(int idx) {
    if (waiting_process_num == MAX_PROCESS_NUM) {
        printf("Error : Ready queue is full\n");
        return ;
    }

    waiting_queue[waiting_process_num++] = ready_queue[idx];

    if (idx == ready_process_num-1) {
        ready_queue[idx] = NULL;
    } else {
        for (int i = idx; i < ready_process_num-1; i++) {
            ready_queue[i] = ready_queue[i+1];
        }
        ready_queue[ready_process_num-1] = NULL;
    }
    ready_process_num--;
}

void waiting_queue_to_ready_queue(int idx) {
    if (ready_process_num == MAX_PROCESS_NUM) {
        printf("Error : Ready queue is full\n");
        return ;
    }

    ready_queue[ready_process_num++] = waiting_queue[idx];

    if (idx == waiting_process_num-1) {
        waiting_queue[idx] = NULL;
    } else {
        for (int i = idx; i < waiting_process_num-1; i++) {
            waiting_queue[i] = waiting_queue[i+1];
        }
        waiting_queue[waiting_process_num-1] = NULL;
    }
    waiting_process_num--;
}

void update_waiting_time(process *running_process) {
    for (int i = 0; i < ready_process_num; i++) {
        if (running_process->pid != ready_queue[i]->pid)
            ready_queue[i]->waiting_time++;
    }    
}

process* FCFS() {
    // running process�� �ٽ� �����ϴ� �κ�
    if (ready_process_num != 0) {
        // ���� ���� process ����
        int idx = -1;
        for (int i = 0; i < ready_process_num; i++) {
            if (idx == -1 || ready_queue[i]->arrival_time < ready_queue[idx]->arrival_time)
                idx = i;
        }
        running_process = ready_queue[idx];
    }
    return running_process;
}

process* non_preemptive_SJF() {
    // running process�� �ٽ� �����ϴ� �κ�
    if (ready_process_num != 0) {
        // ���� cpu burst time�� ���� process ����
        int idx = -1;
        for (int i = 0; i < ready_process_num; i++) {
            if (idx == -1 || ready_queue[i]->cpu_burst_time < ready_queue[idx]->cpu_burst_time)
                idx = i;
        }
        running_process = ready_queue[idx];
    }
    return running_process;
}

process* non_preemptive_priority() {
    // running process�� �ٽ� �����ϴ� �κ�
    if (ready_process_num != 0) {
        // ���� cpu burst time�� ū process ����
        int idx = -1;
        for (int i = 0; i < ready_process_num; i++) {
            if (idx == -1 || ready_queue[i]->priority > ready_queue[idx]->priority)
                idx = i;
        }
        running_process = ready_queue[idx];
    }
    return running_process;
}

process* preemptive_SJF() {
    // running process�� �ٽ� �����ϴ� �κ�
    if (ready_process_num != 0) {
        // ���� cpu burst time�� ���� process ����
        int idx = -1;
        for (int i = 0; i < ready_process_num; i++) {
            if (idx == -1 || ready_queue[i]->cpu_burst_time < ready_queue[idx]->cpu_burst_time)
                idx = i;
        }
        running_process = ready_queue[idx];
    }
    return running_process;
}

process* preemptive_priority() {
    // running process�� �ٽ� �����ϴ� �κ�
    if (ready_process_num != 0) {
        // ���� cpu burst time�� ū process ����
        int idx = -1;
        for (int i = 0; i < ready_process_num; i++) {
            if (idx == -1 || ready_queue[i]->priority > ready_queue[idx]->priority)
                idx = i;
        }
        running_process = ready_queue[idx];
    }
    return running_process;
}

void shift_ready_queue() {
    process* ori_first = ready_queue[0];
    for (int i = 0; i < ready_process_num-1; i++) {
        ready_queue[i] = ready_queue[i+1];
    }
    ready_queue[ready_process_num-1] = ori_first;
    printf("%d\n", Time);
    print_status(ready_queue);
}

process* round_robin() {
    if (ready_process_num != 0) {
        running_process = ready_queue[0];
        if (running_process->io_burst_time == 0) {
            if ((running_process->cpu_remaining_time)%3 == 0 && running_process->cpu_remaining_time != 0)
                shift_ready_queue();
        } else {
            if ((running_process->cpu_remaining_time)%3 == 2 && running_process->cpu_remaining_time != 2)
                shift_ready_queue();
        }
        running_process = ready_queue[0];
    }
    return running_process;
}

void evaluation();

process* choose_algo(int idx) {
    switch (idx)
    {
    case 0:
        return FCFS();
        break;
    case 1:
        return non_preemptive_SJF();
        break;    
    case 2:
        return non_preemptive_priority();
        break;    
    case 3:
        return preemptive_SJF();
        break;    
    case 4:
        return preemptive_priority();
        break;    
    case 5:
        return round_robin();
        break;    
    default:
        break;
    }
}

void draw_middle(int s) {
    char str[10];
    
    if (s != -1)
        sprintf(str, "|P%d", s);
    else
        sprintf(str, "|--");

    strcat(gantt_middle, str);
}
void draw_time() {
    char str[10];
    if (Time < 10)
        sprintf(str, "%d  ", Time);
    else
        sprintf(str, "%d ", Time);
    strcat(gantt_time, str);
}
void draw_top_down(int s) {
    strcat(gantt_top_down, "+--");
}
void print_gantt_chart() {
    strcat(gantt_top_down, "+");
    strcat(gantt_middle, "|");
    printf("%s\n", gantt_top_down);
    printf("%s\n", gantt_middle);    
    printf("%s\n", gantt_top_down);
    printf("%s\n", gantt_time);
}

void schedule(int idx, int preemptive) {
    while(terminated_process_num < MAX_PROCESS_NUM) {

        if (waiting_process_num) {
            // io request�� ���� ���
            if (waiting_queue[0]->io_burst_time == waiting_queue[0]->io_remaining_time)
                waiting_queue_to_ready_queue(0);
            else
                waiting_queue[0]->io_remaining_time++;
        }

        for (int i = 0; i < job_process_num; i++) {
            if (job_queue[i]->arrival_time == Time) {
                job_queue_to_ready_queue(i);
                i--;
            }
        }

        if ((running_process == NULL && !preemptive) || preemptive)
            running_process = choose_algo(idx);

        // gantt chart �׸���
        int s;
        if (running_process)
            s = running_process->pid;            
        else
            s = -1;
        draw_middle(s);
        draw_top_down(s);
        draw_time();


        // �̹� �������� process�� �ִ� ���
        if (running_process != NULL) {
            update_waiting_time(running_process); // �������� process�� ������ ready queue�� �ִ� ��� process�� waiting time 1�� ����
            running_process->cpu_remaining_time++;

            // �������� 2�� ������ io request�� �ִ� ���
            if (running_process->cpu_remaining_time == 2 && running_process->io_burst_time != 0) {
                ready_queue_to_waiting_queue(find_index(running_process));
                running_process = NULL;
            }

            // ���� �ð��� ���� ���
            else if (running_process->cpu_burst_time == running_process->cpu_remaining_time) {
                running_process->turnaround_time = Time - running_process->arrival_time + 1;
                ready_queue_to_terminated_queue(find_index(running_process));
                running_process = NULL;
            }
        }

        Time++;
    }
}

void simulate() {
    puts("job queue :");
    print_status(job_queue);

    schedule(5, 1);

    puts("terminated queue :");
    print_status(terminated_queue);
    print_gantt_chart();
}

int main() {
    srand(time(NULL));

    for (int i = 0; i < MAX_PROCESS_NUM; i++)
        create_process();

    simulate();

    return 0;
}