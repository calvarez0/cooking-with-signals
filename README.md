# cooking-with-signals
This C++ project implements Unix signals and timers to execute recipe instructions. It reads a CSV file with steps, dependencies, and durations, then executes them sequentially, managing dependencies and timing using signals (SIGRTMIN for timers and SIGUSR1 for removing dependencies).
