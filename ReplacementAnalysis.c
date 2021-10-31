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
        else if (value != -1) { index = value; }                                        // if found in set, set index to that point
        else { index = getMinimumIndex(counters, size); faults++; }                     // if not found, page fault, get the smallest count value and replace at that index
        
        set[index] = data[i];                                                           // replace value at calculated index with the data value
        counters[index] = counter;                                                      // set the counter value at the index to the largest as of that moment (most recent)
        counter++;                                                                      // increment count
    }
    return faults;                                                                      // return the number of page faults calculated
}

/********************************************************************************************************************************************************
* int hasValue (int value, int set [], int size)
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  returns the index of a value in a set, -1 if not found
* Parameters:
*    size - int - size of the working set
*    value - int - the value we're querying for in the set
*    set - int [] - set we are searching in
********************************************************************************************************************************************************/

int hasValue (int value, int set [], int size) {
    for (int i = 0; i < size; i++) {                                                    // iterate through each value in the set
        if (set[i] == value) { return i; }                                              // return index it was found
    }
    return -1;                                                                          // return -1 for couldn't find
}

/********************************************************************************************************************************************************
* int getMinimumIndex (int counters [], int size)
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  returns the smallest number in a given set, used to find which count value is the smallest (representing oldest page number)
* Parameters:
*    counters - int [] - counter set we are checking for smallest value
*    size - int - size of the set
********************************************************************************************************************************************************/

int getMinimumIndex (int counters [], int size) {
    int minimum = counters[0];                                                          // sets default minimum to be the first index in the set
    int minimumIndex = 0;                                                               // capture index of minimum value
    for (int i = 1; i < size; i++) {                                                    // iterate through all values in the set
        if (counters[i] < minimum) {                                                    // detect if the value is less than the current minimum value
            minimum = counters[i];                                                      // set minimum value to be the value we iterated over
            minimumIndex = i;                                                           // set the index to be the minimum value's index
        }
    }
    return minimumIndex;                                                                // return the minimum value's index
}

/********************************************************************************************************************************************************
* int FIFO (int counters [], int size)
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  returns the number of page faults encountered during page number replacement of the given data set. Utilizes a first-in-first-out
                strategy to determine which index is replaced. Uses index-based age determination for FIFO
* Parameters:
*    data - int* - data set for the given experiment
*    size - int - size of the set
********************************************************************************************************************************************************/

int FIFO(int size, int *data) {
    int faults = 0;                                                                     // set the default number of faults to 0
    int set[size];                                                                      // allocate space for the working set based on given size
    for (int i = 0; i < 1000; i++) {                                                    // iterate over all page numbers in set, fitting them into the working set
        if (i < size) { set[i] = data[i]; }                                             // if working set is still not full, set value at given index
        else if (hasValue(data[i], set, size) == -1) {                                  // if the value is not present within the working set, shift all
            replace(0, set, size, data[i]);                                             // values in working set over 1 to the left and attach value to end
            faults++;                                                                   // increment number of faults found
        }
    }
    return faults;                                                                      // return sum of faults encountered during data processing
}

/********************************************************************************************************************************************************
* int replace (int startingIndex, int set [], int size, int value)
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  shifts all values of an array over by 1 to the left and attaches the given value to the end, used for index-based FIFO
* Parameters:
*    startingIndex - int - index that is to be removed, with everything to the right shifting towards it
*    set - int [] - current working set
*    size - int - size of the set
*    value - int - value getting attached to the end of the set once everything is shifted over one to the left
********************************************************************************************************************************************************/

void replace(int startingIndex, int set [], int size, int value) {
    for (int i = startingIndex+1; i < size; i++) {                                      // iterate over all values to the right of the index being replaced
        set[i-1] = set[i];                                                              // set the previous index to be the current index (offset by 1, shift)
    }
    set[size-1] = value;                                                                // attach given value to end of the set
}

/********************************************************************************************************************************************************
* int Clock (int size, int *data)
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  returns the number of page faults encountered during page number replacement of the given data set. Utilizes a clock
                strategy to determine which index is replaced. Uses FIFO variant in retrieving first 0 use-bit. Uses index-based FIFO
* Parameters:
*    data - int* - data set for the given experiment
*    size - int - size of the set
********************************************************************************************************************************************************/

int Clock(int size, int *data) {
    int faults = 0;                                                                     // set default number of faults to 0
    int set[size];                                                                      // the current working set is allocated space
    int useBits[size];                                                                  // store the use bits in a parallel array alongside working set
    for (int i = 0; i < 1000; i++) {                                                    // iterate over all page numbers in the data set
        int value = hasValue(data[i], set, size);                                       // capture if the value is already in the working set
        int index = 0;                                                                  // set default index value to be 0
        if (i < size) { index = i; }                                                    // if working set isn't full, the index to be replaced is simply
        else if (value != -1) { index = value; }                                        // the iterator value, if contained in set already index is where it
        else {                                                                          // is at
            index = getClockIndex(size, useBits);                                       // not in the working set, have to find the FIFO first 0 use-bit
            replace(index, set, size, data[i]);                                         // once the index of first 0 captured, shift working set and attach
            replace(index, useBits, size, 0);                                           // value to end, do the same for use-bits
            faults++;                                                                   // increase number of page faults encountered
            index = size-1;                                                             // index is set to the end
        }
        set[index] = data[i];
        useBits[index] = value == -1 ? 0 : 1;                                           // if the value was referenced, give second-life. Otherwise, set to 0
    }                                                                                   // if just added
    return faults;                                                                      // return number of faults encountered
}

/********************************************************************************************************************************************************
* int getClockIndex (int size, int *useBits)
* Author: Anton Horvath
* Date: 30 October 2021
* Description:  iterate over the use-bits set and attempt to find the first 0. Since index-based FIFO is being used, we simply start at the beginning and
                find the first 0, return the index
* Parameters:
*    useBits - int* - use-bit set given
*    size - int - size of the set
********************************************************************************************************************************************************/

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

/********************************************************************************************************************************************************
* double normal (double mu, double sigma)
* Author: Phoxis
* //https://phoxis.org/2013/05/04/generating-random-numbers-from-normal-distribution-in-c/
* Modifier: Anton Horvath
* Date: 30 October 2021
* Description:  obtain a normally distributed random number with a mean of 10 and standard deviation of 2
* Parameters:
*    mu - double - mean of the normal distribution
*    sigma - double - standard deviation of the distribution
********************************************************************************************************************************************************/

double normal (double mu, double sigma) { 
  double U1, U2, W, mult;
  static double X1, X2;                                                                     // scalars generated that can be applied to the std
  static int call = 0;                                                                      // static flag that indicates a second value is available
 
  if (call == 1)                                                                            // flag indicates the second scalar is still available
    {
      call = !call;
      return (mu + sigma * (double) X2);                                                    // no need to generate two new scalars, simply return second
    }
 
  do                                                                                        // generate scalars, part of the Polars method 
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);
 
  mult = sqrt ((-2 * log (W)) / W);                                                         // multiplier calculated, scalar against two U values calculated above
  X1 = U1 * mult;                                                                           // Assign first scalar for standard deviation
  X2 = U2 * mult;                                                                           // Assign second scalar for standard deviation
 
  call = !call;                                                                             // indicate that generation is/is not needed
 
  return (mu + sigma * (double) X1);                                                        // return the first normally distributed value
}
