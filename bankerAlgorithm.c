// Daniel Hernandez
// Rudy Leiva
// 4/13/16
// Project 4

/* Project 4: This program is passed four command line arguments at the time of execution. The first 3 command line arguments are integers that define the available number of resources of resources R1, R2, and R3. The fourth argument defines how long the program will run. For this project, we are given that the maximum number of customers (threads) is five and that the maximum number or resources is three, so these numbers are hard coded into the project and some parts were coded for these particular numbers. The program next creates five customer threads, giving each thread a customer number (from 0 to 4). Once the threads are created, they begin execution and the main will sleep for the user specified time. Each customer thread contains its own customer number, so the customer thread function contains if blocks for each customer number. This is required to access the correct location of the 2D arrays. Each if block begins with the creation of a request array and populating it with random numbers that range from 0 to value stored in the array need and add 1 to that so that it's also possible to get the actual value stored in need as a request number. Next a call to request_resources() function is performed and passed the customer number and the request array. This function will perform the banker's algorithm to determin if the request can be granted or if its denied. If granted, resources are allocated and appropriate values are updated for available, allocated, and need arrays. If not a -1 is returned and the program will indicate that the requesting customer's request cant be granted due to insufficient resources. Now, regardless of a granted or denied request, the thread will sleep for a random amount of time units (from 0 to 5). If resources were allocated, the program indicates that resources are being used for that many time units. After the random sleep time has passed, the thread will now call release_resources() and pass it the customer number and a release array that contains the number of allocated resources. The function will check that there are indeed resources to release and update the appropriate entries in the avaiable, allocated, and need arrays. Each thread will continue to repeat this process of requesting resources and releasing resources until the user specified run time provided from the command line has elapsed.
*/

// Required imports
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h> // required for the use of the intptr_t data type when passing pointer to threads

#define NUMBER_OF_CUSTOMERS 5 // Given
#define NUMBER_OF_RESOURCES 3 // Given
#define TRUE 1 // Given

pthread_mutex_t mutex; // Required mutex lock variable that is used to  prevent race conditions

// declare and populate the available, allocated, maximum, and need arrays.
// Only the available array is not a 2D array.
int available[NUMBER_OF_RESOURCES] = {0, 0, 0};
int allocated[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {
						{0, 0, 0},
						{0, 0, 0},
						{0, 0, 0},
						{0, 0, 0},
						{0, 0, 0}
					};
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {
						{7, 5, 3},
						{3, 2, 2},
						{9, 0, 2},
						{2, 2, 2},
						{4, 3, 3}
					};
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {
						{7, 5, 3},
						{3, 2, 2},
						{9, 0, 2},
						{2, 2, 2},
						{4, 3, 3}
					};

// used at the end when we record the final current time of each thread when they are interrupted
int finalTime[NUMBER_OF_CUSTOMERS] = {0, 0, 0, 0, 0};

void *customer(void * ptr); // required function prototype

// this function performs the banker's algorith to determine if a request is granted or denied
int request_resources(int customer_num, int request[])
{
	int i;
	int j;
	int safe = 0; // 0 indicates safe state, -1 indicates unsafe. This is the return value used
	int allZeros = 0;
	
	for(i = 0; i < NUMBER_OF_RESOURCES; i++)
	{
		if(request[i] > available[i])
		{
			safe = -1; // check if the request of a resource exceeds available resources
		}
		
		if(request[i] == 0)
		{
			allZeros = allZeros + 1; // check if no resources are requested at all
		}
	}
	
	// this used more to indicate that you cant grant resources if non were requested
	if(allZeros == 3)
	{
		safe = -1;
	}
	
	// Indicate that there are not enough resources to grant the request
	if(safe == -1 && allZeros < 3)
	{
		printf("INSUFFICIENT RESOURCES\n");
	}
	
	// If the request can be granted, perform necessary calculation
	if(safe == 0)
	{
		for(j = 0; j < NUMBER_OF_RESOURCES; j++)
		{
			available[j] = available[j] - request[j];
			allocated[customer_num][j] = request[j];
			need[customer_num][j] = maximum[customer_num][j] - allocated[customer_num][j];
		}
	}
	
	return safe; // return the value of safe.
}

// this function releases the resources from a customer thread back to the available resources array
int release_resources(int customer_num, int release[])
{
	int i;
	int j;
	int allocationCheck = 0; // checks that the customer thread has resources to release
	int allZeros = 0; // used with allocationCheck variable
	
	// check to make sure that the release array doesnt just contain three 0's
	// which would indicate that the thread has no resources to release
	for(i = 0; i < NUMBER_OF_RESOURCES; i++)
	{
		if(release[i] == 0)
		{
			allZeros = allZeros + 1;
		}
	}
	
	if(allZeros == 3)
	{
		allocationCheck = -1; // indicates that there are no resources to release
	}
	
	if(allocationCheck == -1)
	{
		// Do nothing here.
	}
	
	// if there are resources to release, it will be done here.
	// Necessary calculations are performed and appropriate arrays are updated
	if(allocationCheck == 0)
	{
		for(j = 0; j < NUMBER_OF_RESOURCES; j++)
		{
			available[j] = available[j] + release[j];
			allocated[customer_num][j] = allocated[customer_num][j] - release[j];
			need[customer_num][j] = maximum[customer_num][j] - allocated[customer_num][j];
		}
	}
	
	return allocationCheck; // return the value of allocationCheck
}

int main(int argc, char *argv[])
{
	// required variables
	int runTime; // user defined, specifies how long program will run
	int customerThreads; // number of threads to create
	int i;
	int j;
	
	// displayed when not enough command line arguments were provided.
	if(argc != 5)
	{
		fprintf(stderr, "Usage: Available Resources <R1> <R2> <R3> <Seconds to run>\n");
		return -1; // ends the program
	}
	
	// initialize the available number of resources of each type, provided via command line
	available[0] = atoi(argv[1]);
	available[1] = atoi(argv[2]);
	available[2] = atoi(argv[3]);
	
	// initializes variable to value provided via command line
	runTime = atoi(argv[4]);
	
	// initializes how many threads to create, for this project, it is 5 threads
	customerThreads = NUMBER_OF_CUSTOMERS;
	
	// creates the threads, each with a unique customer number (from 0 to 4)
	for(i = 0; i < customerThreads; i++)
	{
		pthread_t threadID;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&threadID, &attr, customer, (void *)(intptr_t) i);
	}
	
	// indicate that threads were created and have now started
	printf("FACTORY: created threads\n");
	printf("FACTORY: started threads\n\n");
	
	sleep(runTime); // sleep for user specified time
	
	// Now that sleep time has passed, indicate that threads were interrupted
	printf("FACTORY: Interrupting threads\n");
	
	// output the time at which each thread was interrupted
	for(j = 0; j < NUMBER_OF_CUSTOMERS; j++)
	{
		printf("At time %d Customer #%d I'm done.\n", finalTime[j], j);
	}
}

// This is the customer thread that will request and release resources while sleeping for a random
// amount of time (from 0 to 5) in between each request_resources() and release_resources() call
// We will only explain what is going on with the case for the customer number of 0 with comments
// since the other cases have the exact same code/procedure.
void *customer(void * ptr)
{
	int customer_num = (intptr_t) ptr; // each thread will receive a unique number
	int currentTime = 0; // set the logic time to 0
	
	while(TRUE)
	{
		int r = rand() % 6; // generate random number of time units to sleep (from 0 to 5)
		sleep(r); // sleep r time units
		currentTime = currentTime + r; // update currentTime
		
		// the case for when the thread has customer_num 0
		if(customer_num == 0)
		{
			int array[NUMBER_OF_RESOURCES]; // required - passed to request_resources
			int array1[NUMBER_OF_RESOURCES]; // required - passed to release_resources
			int rng; // used to generate random numbers for resource request
			int i;
			int j;
			
			pthread_mutex_lock(&mutex); // acquire the mutex lock
			
			// generate random numbers and put into array
			for(i = 0; i < NUMBER_OF_RESOURCES; i++)
			{
				rng = rand() % (need[customer_num][i] + 1);
				array[i] = rng;
			}
			
			// output how many resources are being requested and other information
			printf("At time %d Customer #%d requesting %d %d %d Available = %d %d %d\n", currentTime, customer_num, array[0], array[1], array[2], available[0], available[1], available[2]);
			
			// if the call to request_resources() return 0 (which is success)
			if(request_resources(customer_num, array) == 0)
			{
				// output that resources are being used and show state information
				printf("Customer #%d using resources\nAvailable = %d %d %d Allocated = %d %d %d\n\n", customer_num, available[0], available[1], available[2], allocated[customer_num][0], allocated[customer_num][1], allocated[customer_num][2]);
			}
			else
			{
				// if -1 was returned, output that the request was denied.
				printf("Customer #%d is denied to avoid deadlock.\n\n", customer_num);
			}
			
			pthread_mutex_unlock(&mutex); // release the mutex lock
			
			sleep(r); // sleep for r time units
			currentTime = currentTime + r; // update currentTime
			
			pthread_mutex_lock(&mutex); // acquire the mutex lock
			
			// populate array1 with the values from the allocated array
			for(j = 0; j < NUMBER_OF_RESOURCES; j++)
			{
				array1[j] = allocated[customer_num][j];
			}
			
			// if call to release_resources() returns 0 (indicating success)
			// output that resources were used for r time units
			// output that resources were released and output state information
			if(release_resources(customer_num, array1) == 0)
			{
				printf("Customer #%d using resources for %d time units\n", customer_num, r);
				printf("At time %d Customer #%d releasing %d %d %d Available = %d %d %d\n\n", currentTime, customer_num, array1[0], array1[1], array1[2], available[0], available[1], available[2]);
			
			}
			
			pthread_mutex_unlock(&mutex); // release the mutex lock
			
			finalTime[customer_num] = currentTime; // records the currentTime
		}
		
		// The rest of the cases below follow the same procedure, see comments for the case of
		// customer_num = 0 for explanation.
		
		if(customer_num == 1)
		{
			int array[NUMBER_OF_RESOURCES];
			int array1[NUMBER_OF_RESOURCES];
			int rng;
			int i;
			int j;
			
			pthread_mutex_lock(&mutex);
			
			for(i = 0; i < NUMBER_OF_RESOURCES; i++)
			{
				rng = rand() % (need[customer_num][i] + 1);
				array[i] = rng;
			}
			
			printf("At time %d Customer #%d requesting %d %d %d Available = %d %d %d\n", currentTime, customer_num, array[0], array[1], array[2], available[0], available[1], available[2]);
			
			if(request_resources(customer_num, array) == 0)
			{
				printf("Customer #%d using resources\nAvailable = %d %d %d Allocated = %d %d %d\n\n", customer_num, available[0], available[1], available[2], allocated[customer_num][0], allocated[customer_num][1], allocated[customer_num][2]);
			}
			else
			{
				printf("Customer #%d is denied to avoid deadlock.\n\n", customer_num);
			}
			
			pthread_mutex_unlock(&mutex);
			
			sleep(r);
			currentTime = currentTime + r;
			
			pthread_mutex_lock(&mutex);
			
			for(j = 0; j < NUMBER_OF_RESOURCES; j++)
			{
				array1[j] = allocated[customer_num][j];
			}
			
			if(release_resources(customer_num, array1) == 0)
			{
				printf("Customer #%d using resources for %d time units\n", customer_num, r);
				printf("At time %d Customer #%d releasing %d %d %d Available = %d %d %d\n\n", currentTime, customer_num, array1[0], array1[1], array1[2], available[0], available[1], available[2]);
			}
			
			pthread_mutex_unlock(&mutex);
			
			finalTime[customer_num] = currentTime;
		}
		
		if(customer_num == 2)
		{
			int array[NUMBER_OF_RESOURCES];
			int array1[NUMBER_OF_RESOURCES];
			int rng;
			int i;
			int j;
			
			pthread_mutex_lock(&mutex);
			
			for(i = 0; i < NUMBER_OF_RESOURCES; i++)
			{
				rng = rand() % (need[customer_num][i] + 1);
				array[i] = rng;
			}
			
			printf("At time %d Customer #%d requesting %d %d %d Available = %d %d %d\n", currentTime, customer_num, array[0], array[1], array[2], available[0], available[1], available[2]);
			
			if(request_resources(customer_num, array) == 0)
			{
				printf("Customer #%d using resources\nAvailable = %d %d %d Allocated = %d %d %d\n\n", customer_num, available[0], available[1], available[2], allocated[customer_num][0], allocated[customer_num][1], allocated[customer_num][2]);
			}
			else
			{
				printf("Customer #%d is denied to avoid deadlock.\n\n", customer_num);
			}
			
			pthread_mutex_unlock(&mutex);
			
			sleep(r);
			currentTime = currentTime + r;
			
			pthread_mutex_lock(&mutex);
			
			for(j = 0; j < NUMBER_OF_RESOURCES; j++)
			{
				array1[j] = allocated[customer_num][j];
			}
			
			if(release_resources(customer_num, array1) == 0)
			{
				printf("Customer #%d using resources for %d time units\n", customer_num, r);
				printf("At time %d Customer #%d releasing %d %d %d Available = %d %d %d\n\n", currentTime, customer_num, array1[0], array1[1], array1[2], available[0], available[1], available[2]);
			}
			
			pthread_mutex_unlock(&mutex);
			
			finalTime[customer_num] = currentTime;
		}
		
		if(customer_num == 3)
		{
			int array[NUMBER_OF_RESOURCES];
			int array1[NUMBER_OF_RESOURCES];
			int rng;
			int i;
			int j;
			
			pthread_mutex_lock(&mutex);
			
			for(i = 0; i < NUMBER_OF_RESOURCES; i++)
			{
				rng = rand() % (need[customer_num][i] + 1);
				array[i] = rng;
			}
			
			printf("At time %d Customer #%d requesting %d %d %d Available = %d %d %d\n", currentTime, customer_num, array[0], array[1], array[2], available[0], available[1], available[2]);
			
			if(request_resources(customer_num, array) == 0)
			{
				printf("Customer #%d using resources\nAvailable = %d %d %d Allocated = %d %d %d\n\n", customer_num, available[0], available[1], available[2], allocated[customer_num][0], allocated[customer_num][1], allocated[customer_num][2]);
			}
			else
			{
				printf("Customer #%d is denied to avoid deadlock.\n\n", customer_num);
			}
			
			pthread_mutex_unlock(&mutex);
			
			sleep(r);
			currentTime = currentTime + r;
			
			pthread_mutex_lock(&mutex);
			
			for(j = 0; j < NUMBER_OF_RESOURCES; j++)
			{
				array1[j] = allocated[customer_num][j];
			}
			
			if(release_resources(customer_num, array1) == 0)
			{
				printf("Customer #%d using resources for %d time units\n", customer_num, r);
				printf("At time %d Customer #%d releasing %d %d %d Available = %d %d %d\n\n", currentTime, customer_num, array1[0], array1[1], array1[2], available[0], available[1], available[2]);
			}
			
			pthread_mutex_unlock(&mutex);
			
			finalTime[customer_num] = currentTime;
		}
		
		if(customer_num == 4)
		{
			int array[NUMBER_OF_RESOURCES];
			int array1[NUMBER_OF_RESOURCES];
			int rng;
			int i;
			int j;
			
			pthread_mutex_lock(&mutex);
			
			for(i = 0; i < NUMBER_OF_RESOURCES; i++)
			{
				rng = rand() % (need[customer_num][i] + 1);
				array[i] = rng;
			}
			
			printf("At time %d Customer #%d requesting %d %d %d Available = %d %d %d\n", currentTime, customer_num, array[0], array[1], array[2], available[0], available[1], available[2]);
			
			if(request_resources(customer_num, array) == 0)
			{
				printf("Customer #%d using resources\nAvailable = %d %d %d Allocated = %d %d %d\n\n", customer_num, available[0], available[1], available[2], allocated[customer_num][0], allocated[customer_num][1], allocated[customer_num][2]);
			}
			else
			{
				printf("Customer #%d is denied to avoid deadlock.\n\n", customer_num);
			}
			
			pthread_mutex_unlock(&mutex);
			
			sleep(r);
			currentTime = currentTime + r;
			
			pthread_mutex_lock(&mutex);
			
			for(j = 0; j < NUMBER_OF_RESOURCES; j++)
			{
				array1[j] = allocated[customer_num][j];
			}
			
			if(release_resources(customer_num, array1) == 0)
			{
				printf("Customer #%d using resources for %d time units\n", customer_num, r);
				printf("At time %d Customer #%d releasing %d %d %d Available = %d %d %d\n\n", currentTime, customer_num, array1[0], array1[1], array1[2], available[0], available[1], available[2]);
			}
			
			pthread_mutex_unlock(&mutex);
			
			finalTime[customer_num] = currentTime;
		}
	}
}
