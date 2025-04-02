#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <tuple>
#include <map>
#include <functional>

using namespace std;

struct Process {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int initial_priority;
    int dynamic_priority;
    int time_slice;
    int enter_ready_queue_time;
    Process()=default;

    Process(int id, int arr, int burst, int prio, int ts)
        : id(id), arrival_time(arr), burst_time(burst), remaining_time(burst),
          initial_priority(prio), dynamic_priority(prio), time_slice(ts),
          enter_ready_queue_time(-1) {}
};

vector<Process> read_processes() {
    vector<Process> processes;
    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;
        int id, arr, burst, prio, ts;
        sscanf(line.c_str(), "%d/%d/%d/%d/%d", &id, &arr, &burst, &prio, &ts);
        processes.emplace_back(id, arr, burst, prio, ts);
    }
    return processes;
}

vector<tuple<int, int, int, int>> fcfs(vector<Process> processes) {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        if (a.arrival_time != b.arrival_time) return a.arrival_time < b.arrival_time;
        return a.id < b.id;
    });
    vector<tuple<int, int, int, int>> result;
    int current_time = 0;
    for (auto& p : processes) {
        current_time = max(current_time, p.arrival_time);
        int start = current_time;
        current_time += p.burst_time;
        result.emplace_back(p.id, start, current_time, p.initial_priority);
    }
    return result;
}

vector<tuple<int, int, int, int>> sjf(vector<Process> processes) {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    });
    auto cmp = [](const Process& a, const Process& b) {
        if (a.burst_time != b.burst_time) return a.burst_time > b.burst_time;
        return a.id > b.id;
    };
    priority_queue<Process, vector<Process>, decltype(cmp)> ready_queue(cmp);
    vector<tuple<int, int, int, int>> result;
    int ptr = 0, current_time = 0;
    while (ptr < processes.size() || !ready_queue.empty()) {
        while (ptr < processes.size() && processes[ptr].arrival_time <= current_time) {
            ready_queue.push(processes[ptr]);
            ptr++;
        }
        if (ready_queue.empty()) {
            current_time = processes[ptr].arrival_time;
            continue;
        }
        Process p = ready_queue.top();
        ready_queue.pop();
        int start = current_time;
        current_time += p.burst_time;
        result.emplace_back(p.id, start, current_time, p.initial_priority);
    }
    return result;
}

vector<tuple<int, int, int, int>> srtf(vector<Process> processes) {
    vector<tuple<int, int, int, int>> result;
    auto cmp = [](const Process& a, const Process& b) {
        if (a.remaining_time != b.remaining_time) return a.remaining_time > b.remaining_time;
        return a.arrival_time > b.arrival_time;
    };
    priority_queue<Process, vector<Process>, decltype(cmp)> ready_queue(cmp);
    vector<Process> sorted = processes;
    sort(sorted.begin(), sorted.end(), [](const Process& a, const Process& b) { return a.arrival_time < b.arrival_time; });
    int ptr = 0, current_time = 0;
    Process* current = nullptr;
    int start_time = -1;

    while (ptr < sorted.size() || !ready_queue.empty() || current != nullptr) {
        while (ptr < sorted.size() && sorted[ptr].arrival_time <= current_time) {
            ready_queue.push(sorted[ptr]);
            ptr++;
        }

        if (current != nullptr && current->remaining_time == 0) {
            result.emplace_back(current->id, start_time, current_time, current->initial_priority);
            current = nullptr;
        }

        if (current == nullptr && !ready_queue.empty()) {
            Process p = ready_queue.top();
            ready_queue.pop();
            current = new Process(p);
            start_time = current_time;
        }

        if (current == nullptr) {
            if (ptr < sorted.size()) current_time = sorted[ptr].arrival_time;
            continue;
        }

        int next_event = current_time + current->remaining_time;
        if (ptr < sorted.size()) next_event = min(next_event, sorted[ptr].arrival_time);

        if (!ready_queue.empty()) {
            Process next_p = ready_queue.top();
            if (next_p.remaining_time < current->remaining_time) {
                next_event = min(next_event, current_time);
            }
        }

        int time_spent = next_event - current_time;
        current->remaining_time -= time_spent;
        current_time = next_event;

        if (current->remaining_time == 0) {
            result.emplace_back(current->id, start_time, current_time, current->initial_priority);
            delete current;
            current = nullptr;
        } else if (ptr < sorted.size() && current_time == sorted[ptr].arrival_time) {
            Process p = sorted[ptr];
            if (p.remaining_time < current->remaining_time) {
                ready_queue.push(*current);
                delete current;
                current = new Process(p);
                start_time = current_time;
                ptr++;
            } else {
                ready_queue.push(p);
                ptr++;
            }
        }
    }
    return result;
}

vector<tuple<int, int, int, int>> rr(vector<Process> processes, int time_slice) {
    queue<Process> ready_queue;
    vector<tuple<int, int, int, int>> result;
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) { return a.arrival_time < b.arrival_time; });
    int ptr = 0, current_time = 0;

    while (ptr < processes.size() || !ready_queue.empty()) {
        while (ptr < processes.size() && processes[ptr].arrival_time <= current_time) {
            ready_queue.push(processes[ptr]);
            ptr++;
        }

        if (ready_queue.empty()) {
            if (ptr < processes.size()) current_time = processes[ptr].arrival_time;
            continue;
        }

        Process current = ready_queue.front();
        ready_queue.pop();

        int run_time = min(current.remaining_time, time_slice);
        int start = current_time;
        current_time += run_time;
        current.remaining_time -= run_time;

        result.emplace_back(current.id, start, current_time, current.initial_priority);

        while (ptr < processes.size() && processes[ptr].arrival_time <= current_time) {
            ready_queue.push(processes[ptr]);
            ptr++;
        }

        if (current.remaining_time > 0) {
            ready_queue.push(current);
        }
    }
    return result;
}

vector<tuple<int, int, int, int>> dynamic_priority(vector<Process> processes) {
    vector<tuple<int, int, int, int>> result;
    auto cmp = [](const Process& a, const Process& b) {
        if (a.dynamic_priority != b.dynamic_priority) return a.dynamic_priority > b.dynamic_priority;
        return a.arrival_time > b.arrival_time;
    };
    priority_queue<Process, vector<Process>, decltype(cmp)> ready_queue(cmp);
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) { return a.arrival_time < b.arrival_time; });
    int ptr = 0, current_time = 0;
    map<int, Process> process_map;
    for (auto& p : processes) process_map[p.id] = p;

    while (ptr < processes.size() || !ready_queue.empty()) {
        while (ptr < processes.size() && processes[ptr].arrival_time <= current_time) {
            Process p = processes[ptr];
            p.enter_ready_queue_time = current_time;
            ready_queue.push(p);
            ptr++;
        }

        if (ready_queue.empty()) {
            if (ptr < processes.size()) current_time = processes[ptr].arrival_time;
            continue;
        }

        Process current_p = ready_queue.top();
        ready_queue.pop();

        int time_slice = current_p.time_slice;
        int start = current_time;
        int run_time = min(current_p.remaining_time, time_slice);
        current_p.remaining_time -= run_time;
        current_time += run_time;

        current_p.dynamic_priority += 3;
        result.emplace_back(current_p.id, start, current_time, current_p.dynamic_priority);

        while (ptr < processes.size() && processes[ptr].arrival_time <= current_time) {
            Process p = processes[ptr];
            p.enter_ready_queue_time = current_time;
            ready_queue.push(p);
            ptr++;
        }

        if (current_p.remaining_time > 0) {
            current_p.enter_ready_queue_time = current_time;
            ready_queue.push(current_p);
        } else {
            process_map.erase(current_p.id);
        }

        vector<Process> updated;
        while (!ready_queue.empty()) {
            Process p = ready_queue.top();
            ready_queue.pop();
            int wait_time = current_time - p.enter_ready_queue_time;
            int num_slices = wait_time / p.time_slice;
            p.dynamic_priority += num_slices;
            p.enter_ready_queue_time += num_slices * p.time_slice;
            updated.push_back(p);
        }
        for (auto& p : updated) ready_queue.push(p);
    }
    return result;
}

int main() {
    // Read dispatch type from user
    int dispatch_type;
    cin >> dispatch_type;

    cin.ignore(); 
    vector<Process> processes = read_processes();

    vector<tuple<int, int, int, int>> result;
    switch (dispatch_type) {
        case 1:
            result = fcfs(processes);
            break;
        case 2:
            result = sjf(processes);
            break;
        case 3:
            result = srtf(processes);
            break;
        case 4: {
            int time_slice = processes[0].time_slice;
            result = rr(processes, time_slice);
            break;
        }
        case 5:
            result = dynamic_priority(processes);
            break;
        default:
            cerr << "Invalid dispatch type" << endl;
            return 1;
    }

    for (size_t i = 0; i < result.size(); ++i) {
        auto& t = result[i];
        cout << i+1 << "/" << get<0>(t) << "/" << get<1>(t) << "/" << get<2>(t) << "/" << get<3>(t) << endl;
    }

    return 0;
}