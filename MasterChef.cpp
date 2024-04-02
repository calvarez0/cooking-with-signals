#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <vector>
#include "StepList.h"

using namespace std;

StepList* recipeSteps;
vector<int>* completedSteps;
int completeCount = 0;

void PrintHelp() {
    cout << "Usage: ./MasterChef -i <file>\n\n";
    cout << "--------------------------------------------------------------------------\n";
    cout << "<file>:    " << "csv file with Step, Dependencies, Time (m), Description\n";
    cout << "--------------------------------------------------------------------------\n";
    exit(1);
}

string ProcessArgs(int argc, char** argv) {
    string result = "";
    if (argc < 3) {
        PrintHelp();
    }

    while (true) {
        const auto opt = getopt(argc, argv, "i:h");

        if (-1 == opt)
            break;

        switch (opt) {
            case 'i':
                result = std::string(optarg);
                break;
            case 'h': 
            default:
                PrintHelp();
                break;
        }
    }

    return result;
}

void makeTimer(Step *timerID, int expire) {
    struct sigevent te;
    struct itimerspec its;

    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = SIGRTMIN;
    te.sigev_value.sival_ptr = timerID;
    timer_create(CLOCK_REALTIME, &te, &(timerID->t_id));

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = expire;
    its.it_value.tv_nsec = 0;
    timer_settime(timerID->t_id, 0, &its, NULL);
}

static void timerHandler(int sig, siginfo_t *si, void *uc) {
    (void) sig;
    (void) uc;
    Step* comp_item = (Step*)si->si_value.sival_ptr;

    comp_item->PrintComplete();
    completedSteps->push_back(comp_item->id);
    completeCount++;

    raise(SIGUSR1);
}

void RemoveDepHandler(int sig) {
    (void) sig;
    for(int id : *completedSteps) {
        recipeSteps->RemoveDependency(id);
    }
    completedSteps->clear();
}

int main(int argc, char **argv) {
    string input_file = ProcessArgs(argc, argv);
    if(input_file.empty()) {
        exit(1);
    }
    
    completedSteps = new vector<int>();
    recipeSteps = new StepList(input_file);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGRTMIN, &sa, NULL);

    signal(SIGUSR1, RemoveDepHandler);

    while(recipeSteps->Count() != completeCount) {
        vector<Step*> readySteps = recipeSteps->GetReadySteps();
        for(Step* step : readySteps) {
            if (!step->running) {
                step->running = true;
                makeTimer(step, step->duration);
            }
        }
        sleep(1);
    }

    cout << "Enjoy!" << endl;
    return 0;
}

