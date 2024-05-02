#include <GL/glut.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <istream>

using namespace std;

// Structure to represent a process
struct Process {
    int pid; // Process ID
    int arrivalTime; // Arrival time
    int burstTime; // Burst time
    int x, y; // Position of the process
    float radius; // Radius of the process circle
    bool isExecuting; // Is the process currently executing?
};

// Structure to represent the CPU
struct CPU {
    float x, y; // Position of the CPU rectangle
    float width, height; // Dimensions of the CPU rectangle
    float color[3]; // Color of the CPU rectangle
};
struct GanttChart {
    float x, y; // Position of the Gantt chart
    float width, height; // Dimensions of the Gantt chart
    float color[3]; // Color of the Gantt chart
    vector<int> chart; // List of process IDs in the Gantt chart
};
// Global variables
vector<Process> processes; // List of processes
CPU cpu;// CPU object
GanttChart ganttChart;
int currentTime = 0; // Current time
int nextProcessIndex = 0; // Index of the next process to execute

// Function to draw a circle (process)
void drawCircle(float x, float y, float radius) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159 / 180;
        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
    }
    glEnd();
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
void drawGanttChart() {
    glColor3fv(ganttChart.color);
    drawRectangle(ganttChart.x, ganttChart.y, ganttChart.width, ganttChart.height);

    // Draw the process IDs in the Gantt chart
    float x = ganttChart.x;
    float y = ganttChart.y + ganttChart.height / 2;
    for (int i = 0; i < ganttChart.chart.size(); i++) {
        int pid = ganttChart.chart[i];
        glColor3f(0.0, 0.0, 0.0); // Black color for process IDs
        char str[10];
        sprintf_s(str, "P%d", pid);
        for (int j = 0; str[j] != '\0'; j++) {
            glRasterPos2f(x, y);
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[j]);
            x += 10;
        }
        x += 10;
    }
}
// Function to update the animation
void update() {
    // Update the current time
    currentTime++;

    // Check if a process has arrived
    while (nextProcessIndex < processes.size() && processes[nextProcessIndex].arrivalTime <= currentTime) {
        // Create a new process
        Process p = processes[nextProcessIndex];
        p.x = -200 - nextProcessIndex * 20; // Initial x position
        p.y = 100 ; // Initial y position
        p.radius = 10; // Radius of the process circle
        p.isExecuting = false;
        processes[nextProcessIndex] = p;
        nextProcessIndex++;
    }
    // Check if a process is executing
    for (int i = 0; i < processes.size(); i++) {
        Process& p = processes[i];
        if (p.isExecuting) {
            // Move the process towards the CPU

            p.x += 1;
            if (p.x >= cpu.x) {
                // Process has reached the CPU, start executing
                cpu.color[0] = 1.0; // Change CPU color to red
                cpu.color[1] = 0.0;
                cpu.color[2] = 0.0;
                p.isExecuting = false;
                p.burstTime--;
                if (p.burstTime == 0) {
                    // Process has completed execution, erase it
                    ganttChart.chart.push_back(p.pid);
                    processes.erase(processes.begin() + i);
                    i--;
                }
            }
        }
    }
    // Check if a process is ready to execute
    for (int i = 0; i < processes.size(); i++) {
        Process& p = processes[i];
        if (!p.isExecuting && p.arrivalTime <= currentTime) {
            // Find the process with the shortest burst time
            int minBurstTime = INT_MAX;
            int minIndex = -1;
            for (int j = 0; j < processes.size(); j++) {
                if (!processes[j].isExecuting && processes[j].arrivalTime <= currentTime && processes[j].burstTime < minBurstTime) {
                    minBurstTime = processes[j].burstTime;
                    minIndex = j;
                }
            }
            if (minIndex != -1) {
                // Start executing the process with the shortest burst time
                processes[minIndex].isExecuting = true;
            }
        }
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

    // Draw the processes
    for (int i = 0; i < processes.size(); i++) {
        Process& p = processes[i];
        glColor3f(0.0, 0.0, 1.0); // Blue color for processes
        drawCircle(p.x, p.y, p.radius);
    }
    drawGanttChart();

    glutSwapBuffers();
}

// Function to handle keyboard input
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'q':
    case 'Q':
        exit(0);
        break;
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

    ganttChart.x = 150;
    ganttChart.y = 20;
    ganttChart.width = 100;
    ganttChart.height = 50;
    ganttChart.color[0] = 0.0;
    ganttChart.color[1] = 0.0;
    ganttChart.color[2] = 1.0;

    // Set up the processes
    int numProcesses;
    cout << "Enter the number of processes: ";
    cin >> numProcesses;
    cin.ignore();
  
    for (int i = 0; i < numProcesses; i++) {
        int pid, arrivalTime, burstTime;
        cout << "Enter process " << i + 1 << " details:(pid,arrival,burst) ";
        string input;
        getline(cin, input);
        istringstream iss(input);
        iss >> pid >> arrivalTime >> burstTime;
        processes.push_back({ pid, arrivalTime, burstTime, 0, 0, false });
       
    }

    // Sort the processes based on arrival time
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        if (a.arrivalTime != b.arrivalTime) {
            return a.arrivalTime < b.arrivalTime;
        }
        else {
            return a.burstTime < b.burstTime;
        }
        });

    // Set up the OpenGL window
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 400, 0, 300, -1, 1);
    ganttChart.chart.clear();
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("SJF Algorithm Animation");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(update);
    glutMainLoop();
    return 0;
}