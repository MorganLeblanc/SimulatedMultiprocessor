#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

//Running boolean
int isRunning = 1;

//Initial process amounts
int fcfsProcesses;
int prioProcesses;

//Processor time quantums
double fcfsQuantum = 1;
double prioQuantum = .5;

//Jobless booleans
int fcfsJobless = 0;
int prioJobless = 0;

//Static pid value and its mutex
int PID = 0;
pthread_mutex_t pidMutex = PTHREAD_MUTEX_INITIALIZER;

//stdout mutex
pthread_mutex_t outMutex = PTHREAD_MUTEX_INITIALIZER;

//fcfs mutex
pthread_mutex_t fcfsMutex = PTHREAD_MUTEX_INITIALIZER;
//prio mutex
pthread_mutex_t prioMutex = PTHREAD_MUTEX_INITIALIZER;

//Enum representing process states
enum processState {CREATE = 0, WAITING = 1, EXECUTING = 2, READY = 3, TERMINATING = 4};

//Struct representing a singular process
struct process{
    int processID;
    double burstTime;
    double timeBursted;
    int priority;
    int priorityOffset;
    enum processState currentState;
};

//Data structures to store processes
struct process * fcfsProcessList;
int * fcfsArrivalOrder;
struct process * prioProcessList;

//Mutex locks and produces next PID
int getNextPID()
{
    pthread_mutex_lock(&pidMutex);
    PID++;
    int temp = PID;
    pthread_mutex_unlock(&pidMutex);
    return temp;
}

int isJobless(int core)
{
    if(core == 1)
    {
    	pthread_mutex_lock(&fcfsMutex);
    	for(int i = 0; i < fcfsProcesses; i++)
    	{
    		if(fcfsProcessList[i].currentState == TERMINATING)
    		{

    		}
    		else
    		{
    			pthread_mutex_unlock(&fcfsMutex);
    			return 0;
    		}
    	}
    	pthread_mutex_unlock(&fcfsMutex);
    	return 1;
    }
    else
    {
    	pthread_mutex_lock(&prioMutex);
    	for(int i = 0; i < prioProcesses; i++)
    	{
    		if(prioProcessList[i].currentState == TERMINATING)
    		{

    		}
    		else
    		{
    			pthread_mutex_unlock(&prioMutex);
    			return 0;
    		}
    	}
    	pthread_mutex_unlock(&prioMutex);
    	return 1;
    }
}

int fcfsRunNext(int core)
{
    //Lock mutex
    pthread_mutex_lock(&fcfsMutex);
    //Variable to store target process
    int targetFound = 0;
    int isFirst = 1;
    int selectedProcess;
    //Find the next process
    for(int i = 0; i < fcfsProcesses; i++)
    {
        if(fcfsProcessList[i].currentState == READY)
        {
            if(isFirst)
            {
                targetFound = 1;
                selectedProcess = i;
                isFirst = 0;
            }
            else if(fcfsArrivalOrder[i] < fcfsArrivalOrder[selectedProcess])
            {

                selectedProcess = i;
            }
            else
            {

            }
        }
    }

    if(targetFound == 0)
    {
        printf("Processor %d could find no ready processes.\n", core);
        pthread_mutex_unlock(&fcfsMutex);
        sleep(3);
        return 1;
    }

    fcfsProcessList[selectedProcess].currentState = EXECUTING;

    //Run the process
    if(core == 1)
    {
        //Print that process is beginning to run
        printf("Processor %d has chosen to execute process %d\n", core, fcfsProcessList[selectedProcess].processID);
        //increment time bursted by processor time quantum
        fcfsProcessList[selectedProcess].timeBursted += fcfsQuantum;
        //Wait for the required time quantum
        sleep(fcfsQuantum);
        //Print completion status
        if(fcfsProcessList[selectedProcess].timeBursted >= fcfsProcessList[selectedProcess].burstTime)
        {
            printf("Processor %d has run process %d. Process has finished burst.\n", core, fcfsProcessList[selectedProcess].processID);
            printf("Process %d is now terminating\n", fcfsProcessList[selectedProcess].processID);
            fcfsProcessList[selectedProcess].currentState = TERMINATING;
            fcfsProcessList[selectedProcess].timeBursted = 0;
        }
        else
        {
            printf("Processor %d has run process %d. Process has %f seconds of burst remaining.\n", core, fcfsProcessList[selectedProcess].processID, (fcfsProcessList[selectedProcess].burstTime - fcfsProcessList[selectedProcess].timeBursted));
        }
    }
    else if(core == 2)
    {
        //Print that process is beginning to run
        printf("Processor %d has chosen to execute process %d\n", core, fcfsProcessList[selectedProcess].processID);
        //increment time bursted by processor time quantum
        fcfsProcessList[selectedProcess].timeBursted += prioQuantum;
        //Wait for the required time quantum
        sleep(prioQuantum);
        //Print completion status
        if(fcfsProcessList[selectedProcess].timeBursted >= fcfsProcessList[selectedProcess].burstTime)
        {
            printf("Processor %d has run process %d. Process has finished burst.\n", core, fcfsProcessList[selectedProcess].processID);
            printf("Process %d is now terminating\n", fcfsProcessList[selectedProcess].processID);
            fcfsProcessList[selectedProcess].currentState = TERMINATING;
        }
        else
        {
            printf("Processor %d has run process %d. Process has %f seconds of burst remaining.\n", core, fcfsProcessList[selectedProcess].processID, (fcfsProcessList[selectedProcess].burstTime - fcfsProcessList[selectedProcess].timeBursted));
        }

    }
    //Reset aging
    fcfsProcessList[selectedProcess].priorityOffset = 0;
    //Update arrival order
    for(int i = 0; i < fcfsProcesses; i++)
    {
        if(fcfsProcessList[i].currentState == READY || fcfsProcessList[i].currentState == WAITING)
        {
            fcfsArrivalOrder[i]--;
        }
    }
    fcfsArrivalOrder[selectedProcess] = fcfsProcesses - 1;
    //Unlock Mutex
    pthread_mutex_unlock(&fcfsMutex);

    return 0;
}

int prioRunNext(int core)
{
    //Lock mutex
    pthread_mutex_lock(&prioMutex);
    //Variable to store target process
    int targetFound = 0;
    int isFirst = 1;
    int selectedProcess;
    //Find the next process
    for(int i = 0; i < prioProcesses; i++)
    {
        if(prioProcessList[i].currentState == READY)
        {
            if(isFirst)
            {
                targetFound = 1;
                selectedProcess = i;
                isFirst = 0;
            }
            else if((prioProcessList[i].priority - prioProcessList[i].priorityOffset) < (prioProcessList[selectedProcess].priority - prioProcessList[selectedProcess].priorityOffset))
            {
                selectedProcess = i;
            }
            else
            {

            }
        }
    }

    if(targetFound == 0)
    {
        printf("Processor %d could find no ready processes.\n", core);
        pthread_mutex_unlock(&prioMutex);
        sleep(3);
        return 1;
    }

    prioProcessList[selectedProcess].currentState = EXECUTING;

    //Run the process
    if(core == 1)
    {
        //Print that process is beginning to run
        printf("Processor %d has chosen to execute process %d\n", core, prioProcessList[selectedProcess].processID);
        //increment time bursted by processor time quantum
        prioProcessList[selectedProcess].timeBursted +=fcfsQuantum;
        //Wait for the required time quantum
        sleep(fcfsQuantum);
        //Print completion status
        if(prioProcessList[selectedProcess].timeBursted >= prioProcessList[selectedProcess].burstTime)
        {
            sleep(1);
            printf("Processor %d has run process %d. Process has finished burst.\n", core, prioProcessList[selectedProcess].processID);
            printf("Process %d is now terminating\n", prioProcessList[selectedProcess].processID);
            prioProcessList[selectedProcess].currentState = TERMINATING;
            prioProcessList[selectedProcess].timeBursted = 0;
        }
        else
        {
            sleep(1);
            printf("Processor %d has run process %d. Process has %f seconds of burst remaining.\n", core, prioProcessList[selectedProcess].processID, (prioProcessList[selectedProcess].burstTime - prioProcessList[selectedProcess].timeBursted));
        }
    }
    else if(core == 2)
    {
        //Print that process is beginning to run
        printf("Processor %d has chosen to execute process %d\n", core, prioProcessList[selectedProcess].processID);
        //increment time bursted by processor time quantum
        prioProcessList[selectedProcess].timeBursted += prioQuantum;
        //Wait for the required time quantum
        sleep(prioQuantum);
        //Print completion status
        if(prioProcessList[selectedProcess].timeBursted >= prioProcessList[selectedProcess].burstTime)
        {
            printf("Processor %d has run process %d. Process has finished burst.\n", core, prioProcessList[selectedProcess].processID);
            printf("Process %d is now terminating\n", prioProcessList[selectedProcess].processID);
            prioProcessList[selectedProcess].currentState = TERMINATING;
        }
        else
        {
            printf("Processor %d has run process %d. Process has %f seconds of burst remaining.\n", core, prioProcessList[selectedProcess].processID, (prioProcessList[selectedProcess].burstTime - prioProcessList[selectedProcess].timeBursted));
        }

    }
    //Reset aging
    prioProcessList[selectedProcess].priorityOffset = 0;
    //Unlock Mutex
    pthread_mutex_unlock(&prioMutex);

    return 0;
}

//Thread control for first come first serve processor
void *fcfsController()
{
    //Display start up message
    printf("Processor 1 has started on new thread, using FCFS scheduling\n");

    //Initialise process array
    fcfsProcessList = malloc(fcfsProcesses * sizeof(struct process));
    fcfsArrivalOrder = malloc(fcfsProcesses * sizeof(int));

    //Initialize processes
    for(int i = 0; i < fcfsProcesses; i++)
    {
        pthread_mutex_lock(&fcfsMutex);
        //Set up the initial state of the process
        fcfsProcessList[i].processID = getNextPID();
        fcfsProcessList[i].burstTime = (((double)(rand() % 40)) + 1 )/ 10;
        fcfsProcessList[i].timeBursted = 0;
        fcfsProcessList[i].priority = (rand() % 128);
        fcfsProcessList[i].currentState = READY;
        //Note the time that it arrived
        fcfsArrivalOrder[i] = i;
        pthread_mutex_unlock(&fcfsMutex);
        //Print new process to screen
        pthread_mutex_lock(&outMutex);
        printf("\nProcessor 1 has received process %d with the following properties:\n", fcfsProcessList[i].processID);
        printf("Burst Time: %f\n", fcfsProcessList[i].burstTime);
        printf("System Priority: %d\n", fcfsProcessList[i].priority);
        printf("Current State: READY\n\n");
        pthread_mutex_unlock(&outMutex);
    }

    while(isRunning != 0)
    {
        int status = fcfsRunNext(1);
        if (status == 1) {
            if (isJobless(1) == 1) {
                fcfsJobless = 1;
                if(prioJobless == 1)
                {
                    isRunning = 0;
                }
                printf("Processor 1 has finished its processes and is now load balancing processor 2.\n");
                prioRunNext(1);
            }
        }
    }
    sleep(5);
    return NULL;
}

void *prioController()
{
    //Display start up message
    printf("Processor 2 has started on new thread, using priority based scheduling\n");

    //Initialise process array
    prioProcessList = malloc(prioProcesses * sizeof(struct process));

    //Initialize processes
    for(int i = 0; i < prioProcesses; i++)
    {
        pthread_mutex_lock(&prioMutex);
        //Create initial process descriptions
        prioProcessList[i].processID = getNextPID();
        prioProcessList[i].burstTime = (((double)(rand() % 40)) + 1 )/ 10;
        prioProcessList[i].timeBursted = 0;
        prioProcessList[i].priority = (rand() % 128);
        prioProcessList[i].priorityOffset = 0;
        prioProcessList[i].currentState = READY;
        pthread_mutex_unlock(&prioMutex);
        //Print process to stdout
        pthread_mutex_lock(&outMutex);
        printf("\nProcessor 2 has received process %d with the following properties:\n", prioProcessList[i].processID);
        printf("Burst Time: %f\n", prioProcessList[i].burstTime);
        printf("System Priority: %d\n", prioProcessList[i].priority);
        printf("Current State: READY\n\n");
        pthread_mutex_unlock(&outMutex);
    }

    while(isRunning != 0)
    {
            int status = prioRunNext(2);
            if (status == 1) {
                if (isJobless(2) == 1) {
                    prioJobless = 1;
                    if(fcfsJobless == 1)
                    {
                        isRunning = 0;
                    }
                    printf("Processor 2 has finished its processes and is now load balancing processor 1.\n");
                    fcfsRunNext(2);
                }
            }
    }

    sleep(10);
    return NULL;
}

void *agingController()
{
    printf("Aging controller thread has been created.\n");

    while(isRunning) {
        sleep(2);
        if (isRunning) {
            pthread_mutex_lock(&fcfsMutex);
        printf("Aging controller is now aging all processes \n");
        for (int i = 0; i < fcfsProcesses; i++) {
            fcfsProcessList[i].priorityOffset++;
        }
        pthread_mutex_unlock(&fcfsMutex);
        pthread_mutex_lock(&prioMutex);
        for (int i = 0; i < prioProcesses; i++) {
            prioProcessList[i].priorityOffset++;
        }
        pthread_mutex_unlock(&prioMutex);
        printf("Aging controller has aged all processes \n");
    }
    }
    sleep(15);
    return NULL;
}

void *statusController()
{
    printf("Status controller thread has been created.\n");

    while(isRunning) {
        sleep(5);

        printf("Status controller is now updating status of all applicable processes.\n");

        //Randomize all executing values in both cores
        pthread_mutex_lock(&fcfsMutex);
        for (int i = 0; i < fcfsProcesses; i++) {
            if (fcfsProcessList[i].currentState == EXECUTING || fcfsProcessList[i].currentState == WAITING) {
                int newState = rand() % 2;
                if (newState == 0) {
                    printf("Status controller has set process %d to READY.\n", fcfsProcessList[i].processID);
                    fcfsProcessList[i].currentState = READY;
                } else if (newState == 1) {
                    if (fcfsProcessList[i].currentState == WAITING) {

                    } else {
                        printf("Status controller has set process %d to WAITING.\n", fcfsProcessList[i].processID);
                        fcfsProcessList[i].currentState = WAITING;
                    }
                } else {
                     printf("Status controller has set process %d to TERMINATE.\n", fcfsProcessList[i].processID);
                    fcfsProcessList[i].currentState = TERMINATING;
                }
            }
        }
        pthread_mutex_unlock(&fcfsMutex);

        pthread_mutex_lock(&prioMutex);
        for (int i = 0; i < prioProcesses; i++) {
            if (prioProcessList[i].currentState == EXECUTING || prioProcessList[i].currentState == WAITING) {
                int newState = rand() % 2;
                if (newState == 0) {
                    printf("Status controller has set process %d to READY.\n", prioProcessList[i].processID);
                    prioProcessList[i].currentState = READY;
                } else if (newState == 1) {
                    if (prioProcessList[i].currentState == WAITING) {

                    } else {
                        printf("Status controller has set process %d to WAITING.\n", prioProcessList[i].processID);
                        prioProcessList[i].currentState = WAITING;
                    }
                } else {
                    printf("Status controller has set process %d to TERMINATE.\n", prioProcessList[i].processID);
                    prioProcessList[i].currentState = TERMINATING;
                }
            }
        }
        pthread_mutex_unlock(&prioMutex);
    }
    printf("All processes have finished, program now exiting.\n");
    sleep(20);
    exit(0);
}

int main(int argc, char * argv[]) {
    //check if there are the correct number of arguments
    if(argc == 3)
    {
        //Set the initial process amounts
        fcfsProcesses = atoi(argv[1]);
        prioProcesses = atoi(argv[2]);
    }
    else
    {
        perror("Initial process amounts missing.");
        return 1;
    }

    srand(time(NULL));

    pthread_t processors[4];

    int threads = 4;

    //Create the needed threads
    for(int i = 0; i < threads; i++)
    {
        if(i == 0)
        {
            if(pthread_create(&processors[i], NULL, fcfsController, NULL)) {

                perror("Error creating thread\n");
                return 1;

            }
        }
        else if(i == 1)
        {
            if(pthread_create(&processors[i], NULL, prioController, NULL)) {

                perror("Error creating thread\n");
                return 1;

            }
        }
        else if(i == 2)
        {
            if(pthread_create(&processors[i], NULL, statusController, NULL)) {

                perror("Error creating thread\n");
                return 1;

            }
        }
        else if(i == 3)
        {
            if(pthread_create(&processors[i], NULL, agingController(), NULL)) {

                perror("Error creating thread\n");
                return 1;

            }
        }
    }

    for(int i = 0; i < 4; i++)
    {
        pthread_join(processors[i], NULL);
        printf("Thread %d joined.\n", i);
    }

    return 0;
}