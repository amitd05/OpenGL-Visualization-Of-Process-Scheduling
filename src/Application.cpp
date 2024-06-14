#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <queue>
#include<tuple>
#include<array>


using namespace std;

// Structure to represent a process
struct Process {
    int id;
    int pid; // Process ID
    int arrivalTime; // Arrival time
    int burstTime; // Burst time
    int originalBurstTime;
    int x, y; // Position of the process
    float radius; // Radius of the process circle
    bool isExecuting; // Is the process currently executing?
    int priority;// Priority of the process
    int waitingTime; // Waiting time for the process
    int turnaroundTime; // Turnaround time for the process
    int responseTime;   // Response time for the process
    array<float, 3> color;// Remaining burst time of the process
};

// Structure to represent the CPU
struct CPU {
    float x, y; // Position of the CPU rectangle
    float width, height; // Dimensions of the CPU rectangle
    float color[3]; // Color of the CPU rectangle
};

// Structure to represent the Gantt chart
struct GanttChart {
    float x, y; // Position of the Gantt chart
    float width, height; // Dimensions of the Gantt chart
    float color[3]; // Color of the Gantt chart
    vector<pair<int, tuple<int, int, array<float,3>>>> chart; // List of process IDs in the Gantt chart
};

// Global variables
vector<Process> processes; // List of processes
CPU cpu; // CPU object
GanttChart ganttChart;
queue<Process> readyQueue;
priority_queue<Process> priorityQueue;
int currentTime = 0; // Current time
int nextProcessIndex = 0; // Index of the next process to execute
int prevExecutingProcessIndex = -1;
int selectedAlgorithm = 1; // Selected scheduling algorithm: 1 = RR, 2 = SJF, 3 = Priority
int quantum = 0;
int numProcesses;


// Function prototypes
void drawCircle(float x, float y, float radius, int pid);
void drawRectangle(float x, float y, float width, float height);
void drawGanttChart();
void update();
void display();
void keyboard(unsigned char key, int x, int y);
void init();
void UpdateRoundRobin();
void UpdatenonPreemptiveSJF();
void UpdatePreemptiveSJF();
void UpdatePreemptivePriority();
void UpdateNonPreemptivePriorityScheduling();

// Function to draw a circle (process)
void drawCircle(float x, float y, float radius, int pid) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159 / 180;
        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
    }
    glEnd();

    // Draw the process ID at the center of the circle
    char str[10];
    sprintf_s(str, "P%d", pid);
    glColor3f(0.0, 0.0, 0.0); // Black color for process ID
    glRasterPos2f(x, y);
    for (int j = 0; str[j] != '\0'; j++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
    }
}

// Function to draw a rectangle (CPU)
void drawRectangle(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}
// Function to draw the ready queue on the OpenGL window
void drawReadyQueue() {
    // Set the color for the ready queue
    glColor3f(0.5, 0.5, 0.5); // Light gray color

    // Draw the outline of the ready queue
    glBegin(GL_LINE_LOOP);
    glVertex2f(50, 20); // Top-left corner
    glVertex2f(150, 20); // Top-right corner
    glVertex2f(150, 70); // Bottom-right corner
    glVertex2f(50, 70); // Bottom-left corner
    glEnd();

    // Display text label for the ready queue
    glColor3f(0.0, 0.0, 0.0); // Black color for text
    glRasterPos2f(55, 80); // Position for text

    // Draw each character of the label string
    const char* label = "Ready Queue";
    for (int i = 0; label[i] != '\0'; ++i) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, label[i]);
    }

    // Draw the processes in the ready queue without removing them
    int xPos = 65; // Initial x-position for process circles
    int yPos = 50; // Initial y-position for process circles
    int count = readyQueue.size(); // Number of processes in the ready queue

    // Create a temporary queue to hold the processes while drawing
    queue<Process> tempQueue = readyQueue;

    // Loop through the processes in the ready queue and draw them as circles
    while (!tempQueue.empty()) {
        Process p = tempQueue.front(); // Get the front process
        tempQueue.pop(); // Remove the front process from the temporary queue

        // Draw the process circle
        vector<float> colorVector = { p.color[0], p.color[1], p.color[2] };
        glColor3fv(colorVector.data());// Blue color for processes
        drawCircle(xPos, yPos, p.radius, p.pid);

        // Update x-position for the next process circle
        xPos += 25;
    }
}

int totalTime() {
   int total = 0;
    for (const Process& p : processes) {
        total += p.burstTime;
    }
    return total;
}

float  totaltime ;
// Function to draw the Gantt chart
void drawGanttChart() {
    if (selectedAlgorithm == 1)
    {
        glColor3fv(ganttChart.color);
        drawRectangle(ganttChart.x, ganttChart.y, ganttChart.width, ganttChart.height);
        glColor3f(0.0, 0.0, 0.0); // Set text color to white
        glRasterPos2f(ganttChart.x + ganttChart.width / 2 - 10, ganttChart.y + ganttChart.height +5); // Position the text
        string text = "GanntChart";
        for (int i = 0; i < text.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
        }

      
        // Draw vertical lines representing divisions based on number of processes
       // Number of processes
     
        float divisionWidth = ganttChart.width / ((totaltime/quantum)+1);

        glColor3f(0.5, 0.5, 0.5); // Gray color for division lines
        glBegin(GL_LINES);
        for (int i = 1; i <( (totaltime/quantum)+1); i++) {
            float x = ganttChart.x + i * divisionWidth;
            glVertex2f(x, ganttChart.y);
            glVertex2f(x, ganttChart.y + ganttChart.height);
        }
        glEnd();

        // Draw the process bars using the Gantt chart data
        float y = ganttChart.y + ganttChart.height / 2;

        for (int i = 0; i < ganttChart.chart.size(); i++) {
            int pid = ganttChart.chart[i].first; // Process ID
            int startTime = get<0>(ganttChart.chart[i].second); // Process start time
            int endTime = get<1>(ganttChart.chart[i].second); // Process end time
            array<float, 3> color = get<2>(ganttChart.chart[i].second);
            float startX = ganttChart.x + i * divisionWidth;
            float barWidth = divisionWidth;

            // Draw the process bar
            glColor3fv(color.data()); // Yellow color for process bars
            glBegin(GL_QUADS);
            glVertex2f(startX, y - 5);
            glVertex2f(startX + barWidth, y - 5);
            glVertex2f(startX + barWidth, y + 5);
            glVertex2f(startX, y + 5);
            glEnd();

            // Draw the process ID above the bar
            glColor3f(0.0, 0.0, 0.0); // Black color for process IDs
            char str[20];
            sprintf_s(str, "P%d", pid);
            glRasterPos2f(startX + barWidth / 2 - 5, y + 10);
            for (int j = 0; str[j] != '\0'; j++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
            }

            // Draw the ending time below the bar
            sprintf_s(str, "%d", endTime);
            glRasterPos2f(startX + barWidth, y - 20);
            for (int j = 0; str[j] != '\0'; j++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
            }
        }
    }
    else if (selectedAlgorithm == 4||selectedAlgorithm==5)
    {
        glColor3fv(ganttChart.color);
        drawRectangle(ganttChart.x, ganttChart.y, ganttChart.width, ganttChart.height);
        glColor3f(0.0, 0.0, 0.0); // Set text color to white
        glRasterPos2f(ganttChart.x + ganttChart.width / 2 - 10, ganttChart.y + ganttChart.height + 5); // Position the text
        string text = "GanntChart";
        for (int i = 0; i < text.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
        }

       
        // Draw vertical lines representing divisions based on number of processes
       // Number of processes

        float divisionWidth = ganttChart.width / ganttChart.chart.size();

        glColor3f(0.5, 0.5, 0.5); // Gray color for division lines
        glBegin(GL_LINES);
        for (int i = 1; i <ganttChart.chart.size(); i++) {
            float x = ganttChart.x + i * divisionWidth;
            glVertex2f(x, ganttChart.y);
            glVertex2f(x, ganttChart.y + ganttChart.height);
        }
        glEnd();

        // Draw the process bars using the Gantt chart data
        float y = ganttChart.y + ganttChart.height / 2;

        for (int i = 0; i < ganttChart.chart.size(); i++) {
            int pid = ganttChart.chart[i].first; // Process ID
            int startTime = get<0>(ganttChart.chart[i].second); // Process start time
            int endTime = get<1>(ganttChart.chart[i].second); // Process end time
            array<float, 3> color = get<2>(ganttChart.chart[i].second);
            float startX = ganttChart.x + i * divisionWidth;
            float barWidth = divisionWidth;

            // Draw the process bar
            glColor3fv(color.data()); // Yellow color for process bars
            glBegin(GL_QUADS);
            glVertex2f(startX, y - 5);
            glVertex2f(startX + barWidth, y - 5);
            glVertex2f(startX + barWidth, y + 5);
            glVertex2f(startX, y + 5);
            glEnd();

            // Draw the process ID above the bar
            glColor3f(0.0, 0.0, 0.0); // Black color for process IDs
            char str[20];
            sprintf_s(str, "P%d", pid);
            glRasterPos2f(startX + barWidth / 2 - 5, y + 10);
            for (int j = 0; str[j] != '\0'; j++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
            }

            // Draw the ending time below the bar
            sprintf_s(str, "%d", endTime);
            glRasterPos2f(startX + barWidth, y - 20);
            for (int j = 0; str[j] != '\0'; j++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
            }
        }
    }
    else
    {
        glColor3fv(ganttChart.color);
        drawRectangle(ganttChart.x, ganttChart.y, ganttChart.width, ganttChart.height);

        // Draw vertical lines representing divisions based on number of processes
       // Number of processes
        float divisionWidth = ganttChart.width / numProcesses;

        glColor3f(0.5, 0.5, 0.5); // Gray color for division lines
        glBegin(GL_LINES);
        for (int i = 1; i < numProcesses; i++) {
            float x = ganttChart.x + i * divisionWidth;
            glVertex2f(x, ganttChart.y);
            glVertex2f(x, ganttChart.y + ganttChart.height);
        }
        glEnd();

        // Draw the process bars using the Gantt chart data
        float y = ganttChart.y + ganttChart.height / 2;

        for (int i = 0; i < ganttChart.chart.size(); i++) {
            int pid = ganttChart.chart[i].first; // Process ID
            int startTime = get<0>(ganttChart.chart[i].second); // Process start time
            int endTime = get<1>(ganttChart.chart[i].second); // Process end time
            array<float, 3> color = get<2>(ganttChart.chart[i].second);
            float startX = ganttChart.x + i * divisionWidth;
            float barWidth = divisionWidth;

            // Draw the process bar
            glColor3fv(color.data()); // Yellow color for process bars
            glBegin(GL_QUADS);
            glVertex2f(startX, y - 5);
            glVertex2f(startX + barWidth, y - 5);
            glVertex2f(startX + barWidth, y + 5);
            glVertex2f(startX, y + 5);
            glEnd();

            // Draw the process ID above the bar
            glColor3f(0.0, 0.0, 0.0); // Black color for process IDs
            char str[20];
            sprintf_s(str, "P%d", pid);
            glRasterPos2f(startX + barWidth / 2 - 5, y + 10);
            for (int j = 0; str[j] != '\0'; j++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
            }

            // Draw the ending time below the bar
            sprintf_s(str, "%d", endTime);
            glRasterPos2f(startX + barWidth, y - 20);
            for (int j = 0; str[j] != '\0'; j++) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
            }
        }


    }
 
}




// Function to handle keyboard input
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'q':
    case 'Q':
        exit(0);
        break;
    case '1':
        selectedAlgorithm = 1; // Non-preemptive Round Robin
        break;
    case '2':
        selectedAlgorithm = 2; // Non-preemptive SJF
        break;
    case '3':
        selectedAlgorithm = 3; // Non-preemptive Priority Scheduling
        break;
    case 4:
        selectedAlgorithm = 4;// preemptive SJF
    }
}

// Function to initialize OpenGL
void init() {
    // Set up the CPU
    cpu.x = 200;
    cpu.y = 100;
    cpu.width = 50;
    cpu.height = 100;
    cpu.color[0] = 0.0;
    cpu.color[1] = 1.0;
    cpu.color[2] = 0.0;

    // Set up the Gantt chart
    ganttChart.x = 175;
    ganttChart.y = 20;
    ganttChart.width = 200;
    ganttChart.height = 50;
    ganttChart.color[0] = 0.0;
    ganttChart.color[1] = 1.0;
    ganttChart.color[2] = 1.0;

    // Set up the OpenGL window
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 400, 0, 300, -1, 1);
}

// Function to simulate non-preemptive Round Robin scheduling
void UpdateRoundRobin() {
    // Sort processes based on arrival time if not already sorted
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrivalTime < b.arrivalTime;
        });

    // Check if any process has arrived and add it to the ready queue
    while (nextProcessIndex < processes.size() && processes[nextProcessIndex].arrivalTime <= currentTime) {
        Process p = processes[nextProcessIndex];
        p.x = -200 - nextProcessIndex * 20; // Initial x position
        p.y = 100; // Initial y position
        p.radius = 10; // Radius of the process circle
        p.isExecuting = false;
        p.id = nextProcessIndex + 1;// Set process ID using index
        
        processes[nextProcessIndex]=p;
        readyQueue.push(p);
        nextProcessIndex++;
    }

    // Check if no process is currently executing
 
    if (prevExecutingProcessIndex == -1 && !readyQueue.empty()) {

        Process& p = readyQueue.front(); // Get the front process
        p.isExecuting = true;
        prevExecutingProcessIndex = p.id-1 ;
        

        // Update current time by the minimum of quantum and remaining burst time of the process
        int executionTime = min(quantum, p.burstTime);
        currentTime += executionTime;
       
       
        p.burstTime -= min(quantum, p.burstTime);

      

    }

    // Execute the currently executing process (if any)
    if (prevExecutingProcessIndex != -1) {
       

        Process& p = processes[prevExecutingProcessIndex];
        
        // Calculate the distance to move the process towards the CPU
        float dx = cpu.x - p.x;
        float dy = cpu.y - p.y;
        float dist = sqrt(dx * dx + dy * dy);

        // Move the process towards the CPU
        p.x += dx / dist * 1;
        p.y += dy / dist * 1;

        // Check if the process has reached the CPU
        if (p.x >= cpu.x - 20 && p.x <= cpu.x + 20 && p.y >= cpu.y - 20 && p.y <= cpu.y + 20) {
            // Execute the process for one time unit (quantum)
            cpu.color[0] = 1.0;
            cpu.color[1] = 0.0;
            cpu.color[2] = 0.0;
          
            
            p.burstTime -= min(quantum, p.burstTime);
            cout << p.responseTime << endl;
            if (p.responseTime == 0 && p.arrivalTime>0)
            {
                p.responseTime = currentTime - p.arrivalTime-quantum;
            }
            if (p.burstTime != 0)
            {
                ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime, currentTime, p.color) });
                readyQueue.pop();
                readyQueue.push(p);
                p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                p.y = 100; // Initial y position
                prevExecutingProcessIndex = -1;


            }
           
          
           
            // If the process has completed execution, update Gantt chart and remove the process
            else if (p.burstTime == 0) {
              
                ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime, currentTime, p.color) });
                readyQueue.pop(); 
               
                p.turnaroundTime = currentTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.originalBurstTime;
                // Remove the process from the processes vector
                p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                p.y = 100; // Initial y position
                prevExecutingProcessIndex = -1; // Reset the currently executing process index to allow context switching
                cpu.color[0] = 0.0;
                cpu.color[1] = 1.0;
                cpu.color[2] = 0.0;
            }
           
            
        }
    }
    else
    {
        cpu.color[0] = 0.0;
        cpu.color[1] = 1.0;
        cpu.color[2] = 0.0;
    }

    // Update the display
    glutPostRedisplay();
}
// Function to simulate non-preemptive SJF (Shortest Job First) scheduling
void UpdatenonPreemptiveSJF() {
    // End time is the start time plus burst time
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        // If arrival time is 0, prioritize it regardless of burst time
        if (a.arrivalTime == 0 && b.arrivalTime != 0) {
            return true;
        }
        else if (a.arrivalTime != 0 && b.arrivalTime == 0) {
            return false;
        }
        else if (a.arrivalTime == 0 && b.arrivalTime == 0) {
            // If both arrival times are 0, compare burst times
            if (a.burstTime != b.burstTime) {
                return a.burstTime < b.burstTime;
            }
            // If burst times are the same, compare arrival times
            return a.arrivalTime < b.arrivalTime;
        }
        else {
            // If arrival times are not 0, compare based on burst time
            if (a.burstTime != b.burstTime) {
                return a.burstTime < b.burstTime;
            }
            // If burst times are the same, compare arrival times
            return a.arrivalTime < b.arrivalTime;
        }
        });
    // Check if a process has arrived
    while (nextProcessIndex < processes.size() && processes[nextProcessIndex].arrivalTime <= currentTime) {
        // Create a new process
        Process p = processes[nextProcessIndex];
        p.x = -200 - nextProcessIndex * 20; // Initial x position
        p.y = 100; // Initial y position
        p.radius = 10; // Radius of the process circle
        p.isExecuting = false;
        processes[nextProcessIndex] = p;
        nextProcessIndex++;
    }

    // Check if a process is ready to execute
    if (prevExecutingProcessIndex == -1) {
       
        for (int i = 0; i < processes.size(); i++) {
            Process& p = processes[i];

            if (!p.isExecuting) {
                // Start executing the process
                p.isExecuting = true;
                prevExecutingProcessIndex = i;
                currentTime += p.burstTime;
                p.responseTime = currentTime - p.arrivalTime;
                p.turnaroundTime = currentTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.burstTime;


                break; // Execute only one process at a time
            }
        }
    }

    // Execute the process
    if (prevExecutingProcessIndex != -1) {
        Process& p = processes[prevExecutingProcessIndex];
       
        float dx = cpu.x - p.x;
        float dy = cpu.y - p.y;
        float dist = sqrt(dx * dx + dy * dy);

        // Move the process towards the CPU
        p.x += dx / dist * 1;
        p.y += dy / dist * 1;

        // Check if the process has reached the CPU
        if (p.x >= cpu.x - 20 && p.x <= cpu.x + 20 && p.y >= cpu.y - 20 && p.y <= cpu.y + 20) {
            cpu.color[0] = 1.0;
            cpu.color[1] = 0.0;
            cpu.color[2] = 0.0;

            // Execute the process for one time unit


            p.burstTime--;
            cout << currentTime << endl;

            // If the process has completed execution, update Gantt chart and remove the process
            if (p.burstTime == 0) {
                ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime,currentTime,p.color) });
                p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                p.y = 100; // Initial y position
                prevExecutingProcessIndex = -1;

            }

        }
        else {

            cpu.color[0] = 1.0; // Keep CPU color red
            cpu.color[1] = 0.0;
            cpu.color[2] = 0.0;
        }



    }
    else {

        cpu.color[0] = 0.0; // Keep CPU color red
        cpu.color[1] = 1.0;
        cpu.color[2] = 0.0;
    }
  // Update the display
    glutPostRedisplay();
}


// Function to simulate non-preemptive Priority Scheduling
void UpdateNonPreemptivePriorityScheduling() {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        // If arrival time is 0, prioritize it regardless of priority
        if (a.arrivalTime == 0 && b.arrivalTime != 0) {
            return true;
        }
        else if (a.arrivalTime != 0 && b.arrivalTime == 0) {
            return false;
        }
        else if (a.arrivalTime == 0 && b.arrivalTime == 0) {
            // If both arrival times are 0, compare priorities
            if (a.priority != b.priority) {
                return a.priority < b.priority;
            }
            // If priorities are the same, compare burst times
            return a.burstTime < b.burstTime;
        }
        else {
            // If arrival times are not 0, compare based on priority
            if (a.priority != b.priority) {
                return a.priority < b.priority;
            }
            // If priorities are the same, compare arrival times
            if (a.arrivalTime != b.arrivalTime) {
                return a.arrivalTime < b.arrivalTime;
            }
            // If arrival times and priorities are the same, compare burst times
            return a.burstTime < b.burstTime;
        }
        });


    while (nextProcessIndex < processes.size() && processes[nextProcessIndex].arrivalTime <= currentTime) {
        // Create a new process
        Process p = processes[nextProcessIndex];
        p.x = -200 - nextProcessIndex * 20; // Initial x position
        p.y = 100; // Initial y position
        p.radius = 10; // Radius of the process circle
        p.isExecuting = false;
        processes[nextProcessIndex] = p;
        nextProcessIndex++;
    }
  

   

    // The rest of the code remains the same
    if (prevExecutingProcessIndex == -1) {

        // Find the highest priority job

        int highestPriority = INT_MAX;
        for (int i = 0; i <processes.size(); i++) {
            Process& p = processes[i];
            if (!p.isExecuting && p.burstTime > 0 && p.arrivalTime == 0) {
                p.isExecuting = true;
                prevExecutingProcessIndex = i;
                currentTime += p.burstTime;
                p.responseTime = currentTime - p.arrivalTime;
                p.turnaroundTime = currentTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.burstTime;
                break;

            }
            else if (!p.isExecuting && p.burstTime > 0 && p.arrivalTime <= currentTime && p.priority < highestPriority) {
                p.isExecuting = true;
                prevExecutingProcessIndex = i;
                currentTime += p.burstTime;
                p.turnaroundTime = currentTime - p.arrivalTime;
                p.waitingTime = p.turnaroundTime - p.burstTime;
                highestPriority = p.priority;
                break;

            }
        }
    }

        if (prevExecutingProcessIndex != -1) {
            // Start executing the highest priority job
            Process& p = processes[prevExecutingProcessIndex];
            float dx = cpu.x - p.x;
            float dy = cpu.y - p.y;
            float dist = sqrt(dx * dx + dy * dy);

            // Move the process towards the CPU
            p.x += dx / dist * 1;
            p.y += dy / dist * 1;

            // Check if the process has reached the CPU
            if (p.x >= cpu.x - 20 && p.x <= cpu.x + 20 && p.y >= cpu.y - 20 && p.y <= cpu.y + 20) {
                cpu.color[0] = 1.0;
                cpu.color[1] = 0.0;
                cpu.color[2] = 0.0;

                // Execute the process for one time unit


                p.burstTime--;
                cout << currentTime << endl;

                // If the process has completed execution, update Gantt chart and remove the process
                if (p.burstTime == 0) {
                    ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime,currentTime,p.color) });
                    p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                    p.y = 100; // Initial y position
                    prevExecutingProcessIndex = -1;

                }

            }
            else {

                cpu.color[0] = 0.0; // Keep CPU color red
                cpu.color[1] = 1.0;
                cpu.color[2] = 0.0;
            }



        }
        else {

            cpu.color[0] = 0.0; // Keep CPU color red
            cpu.color[1] = 1.0;
            cpu.color[2] = 0.0;
        }

      // Update the display
        glutPostRedisplay();
    }
    void sortQueueByBurstAndArrival(queue<Process>& readyQueue) {
        // Extract elements from the queue
        vector<Process> temp;
        while (!readyQueue.empty()) {
            temp.push_back(readyQueue.front());
            readyQueue.pop();
        }

        // Sort the vector based on burst time and arrival time
        sort(temp.begin(), temp.end(), [](const Process& a, const Process& b) {
            if (a.burstTime != b.burstTime) {
                return a.burstTime < b.burstTime;
            }
            else {
                return a.arrivalTime < b.arrivalTime;
            }
            });

        // Copy elements back to the queue
        for (const auto& process : temp) {
            readyQueue.push(process);
        }
    }

    

    void UpdatePreemptiveSJF() {
        // Sort processes based on arrival time if not already sorted
       // End time is the start time plus burst time
        sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            // If arrival time is 0, prioritize it regardless of burst time
            if (a.arrivalTime == 0 && b.arrivalTime != 0) {
                return true;
            }
            else if (a.arrivalTime != 0 && b.arrivalTime == 0) {
                return false;
            }
            else if (a.arrivalTime == 0 && b.arrivalTime == 0) {
                // If both arrival times are 0, compare burst times
                if (a.burstTime != b.burstTime) {
                    return a.burstTime < b.burstTime;
                }
                // If burst times are the same, compare arrival times
                return a.arrivalTime < b.arrivalTime;
            }
            else {
                // If arrival times are not 0, compare based on burst time
                
                // If burst times are the same, compare arrival times
                return a.arrivalTime < b.arrivalTime;
            }
            });

        // Check if any process has arrived and add it to the ready queue
        while (nextProcessIndex < processes.size() && processes[nextProcessIndex].arrivalTime <= currentTime) {
            Process p = processes[nextProcessIndex];
            p.x = -200 - nextProcessIndex * 20; // Initial x position
            p.y = 100; // Initial y position
            p.radius = 10; // Radius of the process circle
            p.isExecuting = false;
            p.id = nextProcessIndex + 1; // Set process ID using index
            processes[nextProcessIndex] = p;
            readyQueue.push(p);
            nextProcessIndex++;
        }

        // Check if no process is currently executing or context switch is needed
        if (prevExecutingProcessIndex == -1 && !readyQueue.empty()) {
            
         // Print the queue before sorting

            // Sort the ready queue based on burst time and arrival time
            sortQueueByBurstAndArrival(readyQueue);

            // Print the queue after sorting
            
              // Preempt current process
            Process& p = readyQueue.front();
                // Update Gantt chart and remove the process if pre-empted
                // ...


                // Update currently executing process
                p.isExecuting = true;

                // Select the next process from the ready queue
               
               
                prevExecutingProcessIndex =p.id - 1;

                currentTime++;
                }
            
            // Move the process towards the CPU
            if (prevExecutingProcessIndex != -1  ) {
                Process& p = processes[prevExecutingProcessIndex];
                float dx = cpu.x - p.x;
                float dy = cpu.y - p.y;
                float dist = sqrt(dx * dx + dy * dy);
               
                p.x += dx / dist * 1;
                p.y += dy / dist * 1;

                // Check if the process has reached the CPU
                if (p.x >= cpu.x - 20 && p.x <= cpu.x + 20 && p.y >= cpu.y - 20 && p.y <= cpu.y + 20) {
                    // Execute the process for one time unit (preemptive)
                    cpu.color[0] = 1.0;
                    cpu.color[1] = 0.0;
                    cpu.color[2] = 0.0;
                    p.burstTime--;
                   
                   
                    if (p.responseTime == 0 && p.arrivalTime>0)
                    {
                        p.responseTime = currentTime - p.arrivalTime-1;
                    }
                    if (p.burstTime != 0)
                    {
                       
                        ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime, currentTime, p.color) });

                        readyQueue.pop();
                        readyQueue.push(p);
                        p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                        p.y = 100; // Initial y position
                        prevExecutingProcessIndex = -1;


                    }
                    // Update Gantt chart and remove the process if completed
                    // ...

                    else if (p.burstTime == 0) {

                        ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime, currentTime, p.color) });
                        readyQueue.pop();

                        p.turnaroundTime = currentTime - p.arrivalTime;
                        p.waitingTime = p.turnaroundTime - p.originalBurstTime;
                        // Remove the process from the processes vector
                        p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                        p.y = 100; // Initial y position
                        prevExecutingProcessIndex = -1; // Reset the currently executing process index to allow context switching
                        cpu.color[0] = 0.0;
                        cpu.color[1] = 1.0;
                        cpu.color[2] = 0.0;
                    }
                }
                else {

                    cpu.color[0] = 0.0; // Keep CPU color red
                    cpu.color[1] = 1.0;
                    cpu.color[2] = 0.0;
                }

            }
            else {

                cpu.color[0] = 0.0; // Keep CPU color red
                cpu.color[1] = 1.0;
                cpu.color[2] = 0.0;
            }
        

        // Update the display
        glutPostRedisplay();
    }
   

    void printProcess(const Process& p) {
        cout <<"PID:, "<<p.pid<<"ID: " << p.id << ", Arrival Time : " << p.arrivalTime << ", Burst Time : " << p.burstTime << endl;
    }
    void sortQueueByPriorityAndBurst(queue<Process>& readyQueue) {
        // Extract elements from the queue
        vector<Process> temp;
        while (!readyQueue.empty()) {
            temp.push_back(readyQueue.front());
            readyQueue.pop();
        }

        // Sort the vector based on burst time and arrival time
        sort(temp.begin(), temp.end(), [](const Process& a, const Process& b) {
            if (a.priority != b.priority) {
                return  a.priority < b.priority;
            }
            else {
                return a.burstTime < b.burstTime;;
            }
            });

        // Copy elements back to the queue
        for (const auto& process : temp) {
            readyQueue.push(process);
        }
    }
    void UpdatePreemptivePriority() {
        // Sort processes based on arrival time if not already sorted
        sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            // If arrival time is 0, prioritize it regardless of priority
            if (a.arrivalTime == 0 && b.arrivalTime != 0) {
                return true;
            }
            else if (a.arrivalTime != 0 && b.arrivalTime == 0) {
                return false;
            }
            else if (a.arrivalTime == 0 && b.arrivalTime == 0) {
                // If both arrival times are 0, compare priorities
                if (a.priority != b.priority) {
                    return a.priority < b.priority;
                }
                // If priorities are the same, compare burst times
                return a.burstTime < b.burstTime;
            }
            else {
                // If arrival times are not 0, compare based on priority
                if (a.arrivalTime != b.arrivalTime) {
                    return a.arrivalTime < b.arrivalTime;
                }
                // If priorities are the same, compare arrival times
                 if (a.priority != b.priority) {
                    return a.priority < b.priority;
                }
                // If arrival times and priorities are the same, compare burst times
                return a.burstTime < b.burstTime;
            }
            });


        // Check if any process has arrived and add it to the ready queue
        // Check if any process has arrived and add it to the ready queue
        while (nextProcessIndex < processes.size()&& processes[nextProcessIndex].arrivalTime <= currentTime) {
           
                Process p = processes[nextProcessIndex];
                p.x = -200 - nextProcessIndex * 20; // Initial x position
                p.y = 100; // Initial y position
                p.radius = 10; // Radius of the process circle
                p.isExecuting = false;
                p.id = nextProcessIndex + 1; // Set process ID using index

                // Update response time if it's not set
                if (p.responseTime == 0) {
                    p.responseTime = currentTime - p.arrivalTime;
                }

                processes[nextProcessIndex] = p;
                readyQueue.push(p);
                nextProcessIndex++;
          
        }

        // Check if no process is currently executing or context switch is needed
        if (prevExecutingProcessIndex == -1 && !readyQueue.empty()) {

            // Print the queue before sorting

               // Sort the ready queue based on burst time and arrival time
            sortQueueByPriorityAndBurst(readyQueue);

            // Print the queue after sorting

              // Preempt current process
            Process& p = readyQueue.front();
            // Update Gantt chart and remove the process if pre-empted
            // ...


            // Update currently executing process
            p.isExecuting = true;

            // Select the next process from the ready queue


            prevExecutingProcessIndex = p.id - 1;

            currentTime++;
        
            // Update response time if it's not set
           
        }


        // Move the process towards the CPU
        if (prevExecutingProcessIndex != -1) {
            Process& p = processes[prevExecutingProcessIndex];
            float dx = cpu.x - p.x;
            float dy = cpu.y - p.y;
            float dist = sqrt(dx * dx + dy * dy);

            p.x += dx / dist * 1;
            p.y += dy / dist * 1;

            // Check if the process has reached the CPU
            if (p.x >= cpu.x - 20 && p.x <= cpu.x + 20 && p.y >= cpu.y - 20 && p.y <= cpu.y + 20) {
                // Execute the process for one time unit (preemptive)
                cpu.color[0] = 1.0;
                cpu.color[1] = 0.0;
                cpu.color[2] = 0.0;
                p.burstTime--;
                cout << p.burstTime << endl;
                if (p.responseTime == 0) {
                    p.responseTime = currentTime - p.arrivalTime;
                }
                if (p.burstTime > 0)
                {

                    ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime, currentTime, p.color) });
                    readyQueue.pop();
                    readyQueue.push(p);
                    p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                    p.y = 100; // Initial y position
                    prevExecutingProcessIndex = -1;


                }
                // Update Gantt chart and remove the process if completed
                // ...

                else if (p.burstTime <= 0) {

                    ganttChart.chart.push_back({ p.pid, make_tuple(p.arrivalTime, currentTime, p.color) });
                    readyQueue.pop();

                    p.turnaroundTime = currentTime - p.arrivalTime;
                    p.waitingTime = p.turnaroundTime - p.originalBurstTime;
                    // Remove the process from the processes vector
                    p.x = -200 - prevExecutingProcessIndex * 20; // Initial x position
                    p.y = 100; // Initial y position
                    prevExecutingProcessIndex = -1; // Reset the currently executing process index to allow context switching
                    cpu.color[0] = 0.0;
                    cpu.color[1] = 1.0;
                    cpu.color[2] = 0.0;
                }
            }
            else {

                cpu.color[0] = 0.0; // Keep CPU color red
                cpu.color[1] = 1.0;
                cpu.color[2] = 0.0;
            }
        }
        else {

            cpu.color[0] = 0.0; // Keep CPU color red
            cpu.color[1] = 1.0;
            cpu.color[2] = 0.0;
        }

        // Update the display
        glutPostRedisplay();
    }


void update() {
    // Implement the scheduling logic based on the selected algorithm
    switch (selectedAlgorithm) {
    case 1:
        UpdateRoundRobin();
        break;
    case 2:
        UpdatenonPreemptiveSJF();
        break;
    case 3:
        UpdateNonPreemptivePriorityScheduling();
        break;
    case 4:
        UpdatePreemptiveSJF();
        break;
    case 5:
        UpdatePreemptivePriority();
        break;
    default:
        break;
    }

    // Update the display
    glutPostRedisplay();
}

// Function to display the animation
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the CPU
    glColor3fv(cpu.color);
    drawRectangle(cpu.x, cpu.y, cpu.width, cpu.height);
    glColor3f(1.0, 1.0, 1.0); // Set text color to white
    glRasterPos2f(cpu.x + cpu.width / 2 - 10, cpu.y + cpu.height / 2); // Position the text
    string text = "CPU";
    for (int i = 0; i < text.length(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
    }
    if (selectedAlgorithm == 1) {
        glColor3f(1.0, 0.0, 0.0); // Set text color to white
        glRasterPos2f(140, 270); // Position the text
        string text = "Round Robin Scheduling";
        for (int i = 0; i < text.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
        }
    }
    else if (selectedAlgorithm == 2) {
        glColor3f(1.0, 0.0, 0.0); // Set text color to white
        glRasterPos2f(125, 270); // Position the text
        string text = "Non Preemptive SJF Scheduling";
        for (int i = 0; i < text.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
    }
       
    }else if (selectedAlgorithm == 3) {
        glColor3f(1.0, 0.0, 0.0); // Set text color to white
        glRasterPos2f(120, 270); // Position the text
        string text = "Non Preemptive Priority Scheduling";
        for (int i = 0; i < text.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);

    }
    }
    else if (selectedAlgorithm == 4) {
        glColor3f(1.0, 0.0, 0.0); // Set text color to white
        glRasterPos2f(130, 270); // Position the text
        string text = "Preemptive SJF Scheduling";
        for (int i = 0; i < text.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);

        }
    }
    else  {
        glColor3f(1.0, 0.0, 0.0); // Set text color to white
        glRasterPos2f(125, 270); // Position the text
        string text = "Preemptive Priority Scheduling";
        for (int i = 0; i < text.length(); i++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
        }
      }
       
    float yPos = 250; // Initial y-position for process text at the top corner
    // Draw the processes
    int pid;
    float Totalwaitingtime = 0, Totalturnaroundtime = 0;
   
    for (int i = 0; i < processes.size(); i++) {
        
        Process& p = processes[i];
        vector<float> colorVector = { p.color[0], p.color[1], p.color[2] };
        glColor3fv(colorVector.data());// Blue color for processes
        drawCircle(p.x, p.y, p.radius, p.pid);
        glColor3fv(colorVector.data());
        stringstream ss;
        pid = p.pid;
        Totalwaitingtime += p.waitingTime;
        Totalturnaroundtime += p.turnaroundTime;
        ss << "PID: " << pid << " | Waiting Time: " << p.waitingTime << " | Turnaround Time: " << p.turnaroundTime << "|Response Time:" << p.responseTime;
        glRasterPos2f(20.0, yPos);

        string processText = ss.str();
        
        for (int j = 0; j < processText.length(); j++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, processText[j]);
        }
        yPos -= 20.0; // Move to the next line for the next process

    }
    stringstream pp;
    pp << "Average Waiting Time: " << Totalwaitingtime/numProcesses << " |  Average Turnaround Time: " << Totalturnaroundtime/numProcesses;
    glRasterPos2f(240, 250);
    string processText = pp.str();

    for (int j = 0; j < processText.length(); j++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, processText[j]);
    }
    // Draw the ready queue
    drawReadyQueue();
    drawGanttChart();
   

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Scheduling Algorithm Animation");
    init();

    // Select the scheduling algorithm
    cout << "Select Scheduling Algorithm:\n";
    cout << "1. N Round Robin\n";
    cout << "2. Non-preemptive SJF (Shortest Job First)\n";
    cout << "3. Non-preemptive Priority Scheduling\n";
    cout << "4. Preemptive SJF (Shortest Job First)\n";
    cout << "5. preemptive Priority Scheduling\n";
    cout << "Enter your choice (1/2/3/4/5): ";
    cin >> selectedAlgorithm;

    // Take process details based on the selected algorithm
    
    
    cout << "Enter the number of processes: ";
    cin >> numProcesses;
    cin.ignore();

    for (int i = 0; i <numProcesses; i++) {
        int pid, arrivalTime, burstTime, priority;
        float color[3];
        cout << "Enter process " << i + 1 << " details (pid, arrival, burst";
        if (selectedAlgorithm == 2|| selectedAlgorithm == 4) {
            cout << ", color R, G, B):): ";
            cin >> pid >> arrivalTime >> burstTime >> color[0] >> color[1] >> color[2]; 
            processes.push_back({0, pid, arrivalTime, burstTime,burstTime, 0, 0, 10, false, 0,0,0,0,{color[0], color[1], color[2]} });
        }
        else if (selectedAlgorithm == 3|| selectedAlgorithm == 5)
        {
            cout << ", priority, color R, G, B):): ";
            cin >> pid >> arrivalTime >> burstTime >>priority >> color[0] >> color[1] >> color[2];
            processes.push_back({0, pid, arrivalTime, burstTime,burstTime, 0, 0, 10, false, priority,0,0,0,{color[0], color[1], color[2]} });
        }
       else {
            cout << ", color R, G, B):): ";
            cin >> pid >> arrivalTime >> burstTime >> color[0] >> color[1] >> color[2];
            processes.push_back({ 0,pid, arrivalTime, burstTime,burstTime, 0, 0, 10, false, 0,0,0,0,{color[0], color[1], color[2]} });
        }
    }
    if (selectedAlgorithm == 1)
    {
        cout << "enter quantum" << endl;
        cin >> quantum;
    }

    // Sort the processes based on arrival time
   
    totaltime = totalTime();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(update);
    glutMainLoop();
    return 0;
}


