#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/********************************************************************************************************************************************************
* File Name: montecarlo_aih180000.c
* Author: Anton Horvath
* Modification History:
    > 10/30/2021 Added normal random number generator
    > 10/30/2021 Added least recently used replacement algorithm
    > 10/30/2021 Added first-in-first-out replacement algorithm
    > 10/30/2021 Added clock replacement algorithm
    > 10/30/2021 Added detection of value in set method
    > 10/30/2021 Added set shift method for index-based FIFO
    > 10/31/2021 Fixed clock algorithm
* Procedures:
* main                - creates resultant arrays, iterates 1000 times representing 1000 experiments. Generates 1000 page numbers for each experiment.
                        Uses generated data set to loop between working set sizes 4-20, capturing the number of page faults for each replacement
                        algorithm before saving it to the resultant arrays created earlier. Loops over each possible working set size and ouputs
                        results
* LRU                 - gets the number of page faults generated from a least-recently-used strategy
* hasValue            - returns the index of the given value in a set, -1 if not found
* getMinimumIndex     - returns the minimum value in a given set
* FIFO                - gets the number of page faults generated from a first-in-first-out strategy
* replace             - shifts values over in a given set at a given index to ensure index-based FIFO is valid
* Clock               - gets the number of page faults generated from a clock strategy
* getClockIndex       - iterates over values in a use-bit set and decrements if not 0. Returns first 0's index
********************************************************************************************************************************************************/

double normal (double mu, double sigma);
int LRU(int size, int data []);
int hasValue (int value, int set [], int size);
int getMinimumIndex (int counters [], int size);
int FIFO(int size, int *data);
void replace(int startingIndex, int set [], int size, int value);
int Clock(int size, int *data);
int getClockIndex(int size, int *useBits);

/********************************************************************************************************************************************************
* int main (int argc, char *argv[])
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  creates resultant arrays, iterates 1000 times representing 1000 experiments. Generates 1000 page numbers for each experiment.
                Uses generated data set to loop between working set sizes 4-20, capturing the number of page faults for each replacement
                algorithm before saving it to the resultant arrays created earlier. Loops over each possible working set size and ouputs
                results
* Parameters:
*    argc - int - number of arguments sent from the command line
*    argv - char*[] - arguments sent from the command line
********************************************************************************************************************************************************/

int main(int argc, char *argv[]) {
    int data[1000];                                                                     // data set which will store all page numbers for the experiments
    int LRUResults[17];                                                                 // result set which will store LRU results for each working set
    int FIFOResults[17];                                                                // result set which will store FIFO results for each working set
    int ClockResults[17];                                                               // result set which will store Clock results for each working set
    for (int trace = 0; trace < 1000; trace++) {                                        // iterate over 1000 experiments, creating 1000 unique traces

        for (int pageNumber = 0; pageNumber < 1000; pageNumber++) {                     // create 1000 normally distributed random numbers for the data set
            double randomNumber = normal(10,2);                                         // generate the number
            data[pageNumber] = (10 * ((int) (pageNumber / 100))) + randomNumber;        // number is scaled depending on the field it is located within
        }

        for (int wss = 4; wss < 21; wss++) {                                            // iterate over all working set sizes from 4-20 (inclusive)
            LRUResults[wss-4] += LRU(wss, data);                                        // Add results of LRU replacement to the resultant array
            FIFOResults[wss-4]+= FIFO(wss, data);                                       // Add results of FIFO replacement to the resultant array
            ClockResults[wss-4] += Clock(wss, data);                                    // Add results of Clock replacement to the resultant array
        }
    }

    for (int wss = 4; wss < 21; wss++) {                                                // iterate over each working set size
        printf("Working Set %d - LRU - %d\n", wss, LRUResults[wss-4]);                  // print the number of page faults for LRU replacement for the set size
        printf("Working Set %d - FIFO - %d\n", wss, FIFOResults[wss-4]);                // print the number of page faults for FIFO replacement for the set size
        printf("Working Set %d - Clock - %d\n", wss, ClockResults[wss-4]);              // print the number of page faults for Clock replacement for the set size
        printf("\n");
    }
}

/********************************************************************************************************************************************************
* int LRU (int size, int data [])
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  returns the number of page faults encountered during page number replacement of the given data set. Utilizes a least-recently-used
                strategy to determine which index is replaced. Keeps a counter variable that indicates how recently-accessed a value is
* Parameters:
*    size - int - size of the working set
*    data - int [] - data set we are performing analysis on
********************************************************************************************************************************************************/

int LRU(int size, int data []) {
    int counter = 0;                                                                    // counter is kept to track the age of values in the set
    int faults = 0;                                                                     // the number of faults is tracked throughout the program
    int set[size];                                                                      // the state of the working set
    int counters[size];                                                                 // the state of the counter set (parallel array to working set)
    for (int i = 0; i < 1000; i++) {                                                    // iterate over each value in the data set and attempt to fit page into ws
        int index = 0;                                                                  // start with index of 0 for which value is getting replaced
        int value = hasValue(data[i], set, size);                                       // determine if the value is already in the working set
        if (counter < size) { index = counter; }                                        // if still not full, just set the index to be replaced as the the iterator
        else if (value != -1) { index = value; }                                        // 
        else { index = getMinimumIndex(counters, size); faults++; }
        
        set[index] = data[i];
        counters[index] = counter;
        counter++;
    }
    return faults;
}

/********************************************************************************************************************************************************
* int hasValue (int size, int data [])
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  returns the number of page faults encountered during page number replacement of the given data set. Utilizes a least-recently-used
                strategy to determine which index is replaced. Keeps a counter variable that indicates how recently-accessed a value is
* Parameters:
*    size - int - size of the working set
*    data - int [] - data set we are performing analysis on
********************************************************************************************************************************************************/

int hasValue (int value, int set [], int size) {
    for (int i = 0; i < size; i++) {
        if (set[i] == value) { return i; }
    }
    return -1;
}

int getMinimumIndex (int counters [], int size) {
    int minimum = counters[0];
    int minimumIndex = 0;
    for (int i = 1; i < size; i++) {
        if (counters[i] < minimum) {
            minimum = counters[i];
            minimumIndex = i;
        }
    }
    return minimumIndex;
}

int FIFO(int size, int *data) {
    int faults = 0;
    int set[size];
    for (int i = 0; i < 1000; i++) {
        if (i < size) { set[i] = data[i]; }
        else if (hasValue(data[i], set, size) == -1) {
            replace(0, set, size, data[i]);
            faults++;
        }
    }
    return faults;
}

void replace(int startingIndex, int set [], int size, int value) {
    for (int i = startingIndex+1; i < size; i++) {
        set[i-1] = set[i];
    }
    set[size-1] = value;
}

int Clock(int size, int *data) {
    int faults = 0;
    int set[size];
    int useBits[size];
    for (int i = 0; i < 1000; i++) {
        int value = hasValue(data[i], set, size);
        int index = 0;
        if (i < size) { index = i; }
        else if (value != -1) { index = value; }
        else { 
            index = getClockIndex(size, useBits);
            replace(index, set, size, data[i]);
            replace(index, useBits, size, 0); 
            faults++;
            index = size-1; 
        }
        set[index] = data[i];
        useBits[index] = value == -1 ? 0 : 1;
    }
    return faults;
}

int getClockIndex(int size, int *useBits) {
    int index = -1;
    while (index < 0) {
        for (int i = 0; i < size; i++) {
            if (useBits[i] == 0) { return i; }
            else { useBits[i] = useBits[i] - 1; }
        }
    }
    return index;
}

double normal (double mu, double sigma) { //https://phoxis.org/2013/05/04/generating-random-numbers-from-normal-distribution-in-c/
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;
 
  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }
 
  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);
 
  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;
 
  call = !call;
 
  return (mu + sigma * (double) X1);
}