#include <bits/stdc++.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <fcntl.h>

using namespace std;
using namespace std::chrono;

#define semaphoreName "/binarySemaphore"
#define INITIAL_VALUE 1
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

struct pixel{
    int r, g, b; //Red, Green, Blue Color Defined
};

//Function for RGB_to_Grayscale
void RBG_2_Gray_Conversion(key_t limitKeyofT2, key_t key, int pid, int height, int width){ 
    if (pid > 0)
        return;
    
    int shmid = shmget(key, sizeof(struct pixel) * height * width, 0666 | IPC_CREAT);
    struct pixel *arrayOfData;
    arrayOfData = (struct pixel *)shmat(shmid, NULL, 0);

    //access named semaphore
    sem_t *binarySemaphore = sem_open(semaphoreName, O_RDWR);

    int *goBack;
    int shmid2 = shmget(limitKeyofT2, sizeof(int) * 2, 0666 | IPC_CREAT);
    goBack = (int *)shmat(shmid2, NULL, 0);

    for (int i = 0; i < height; i++){      // Traverse through arrayofData (given image in height)
        for (int j = 0; j < width; j++){   // Traverse through arrayofData (given image in width)
            
            sem_wait(binarySemaphore);
            struct pixel temp;
            temp = arrayOfData[i * width + j];

			int w_mean = (temp.r * 0.3) + (temp.g * 0.59) + (temp.b * 0.11);

    // By considering weights for grayscale, appropriate weighted means are taken
            temp.r = w_mean;
            temp.g = w_mean;
            temp.b = w_mean;

            arrayOfData[i * width + j] = temp;
            goBack[0] = i + 1; goBack[1] = j + 1;

            sem_post(binarySemaphore); //unlocks a semaphore i.e decrements it form 1 to 0
        }
    }
}

void Edge_Detection(key_t limitKeyofT2, key_t key, int pid, int height, int width){ 
    // Ref: https://stackoverflow.com/questions/16385570/sobel-edge-detector-using-c-without-any-special-library-or-tool
    
    if(pid > 0)
        return;
    
    int shmid = shmget(key, sizeof(struct pixel) * height * width, 0666 | IPC_CREAT);
    struct pixel *arrayOfData;
    arrayOfData = (struct pixel *)shmat(shmid, NULL, 0);

    struct pixel temp;
    
    vector<vector<pixel>> newArrayofData (height, vector<pixel>(width));

    sem_t *binarySemaphore = sem_open(semaphoreName, O_RDWR);

    int *goBack;
    int shmid2 = shmget(limitKeyofT2, sizeof(int) * 2, 0666 | IPC_CREAT);
    goBack = (int *)shmat(shmid2, NULL, 0);

    int pixelWorked = 0;

    for(int i = 1; i <= height - 2; i++){
        for(int j = 1; j <= width - 2; j++){

            sem_wait(binarySemaphore);

            if(j + 1 <= goBack[1] || i + 1 <= goBack[0]){
                newArrayofData[i][j].r = sqrt(pow((arrayOfData[(i - 1) * width + (j - 1)].r + 2 * arrayOfData[i * width + (j - 1)].r + arrayOfData[(i + 1) * width + (j - 1)].r) - (arrayOfData[(i - 1) * width + (j + 1)].r + 2 * arrayOfData[i * width + (j + 1)].r + arrayOfData[(i + 1) * width + (j + 1)].r), 2) + pow((arrayOfData[(i - 1) * width + (j - 1)].r + 2 * arrayOfData[(i - 1) * width + j].r + arrayOfData[(i - 1) * width + (j + 1)].r) - (arrayOfData[(i + 1) * width + (j - 1)].r + 2 * arrayOfData[(i + 1) * width + j].r + arrayOfData[(i + 1) * width + (j + 1)].r), 2));
                newArrayofData[i][j].g = sqrt(pow((arrayOfData[(i - 1) * width + (j - 1)].g + 2 * arrayOfData[i * width + (j - 1)].g + arrayOfData[(i + 1) * width + (j - 1)].g) - (arrayOfData[(i - 1) * width + (j + 1)].g + 2 * arrayOfData[i * width + (j + 1)].g + arrayOfData[(i + 1) * width + (j + 1)].g), 2) + pow((arrayOfData[(i - 1) * width + (j - 1)].g + 2 * arrayOfData[(i - 1) * width + j].g + arrayOfData[(i - 1) * width + (j + 1)].g) - (arrayOfData[(i + 1) * width + (j - 1)].g + 2 * arrayOfData[(i + 1) * width + j].g + arrayOfData[(i + 1) * width + (j + 1)].g), 2));
                newArrayofData[i][j].b = sqrt(pow((arrayOfData[(i - 1) * width + (j - 1)].b + 2 * arrayOfData[i * width + (j - 1)].b + arrayOfData[(i + 1) * width + (j - 1)].b) - (arrayOfData[(i - 1) * width + (j + 1)].b + 2 * arrayOfData[i * width + (j + 1)].b + arrayOfData[(i + 1) * width + (j + 1)].b), 2) + pow((arrayOfData[(i - 1) * width + (j - 1)].b + 2 * arrayOfData[(i - 1) * width + j].b + arrayOfData[(i - 1) * width + (j + 1)].b) - (arrayOfData[(i + 1) * width + (j - 1)].b + 2 * arrayOfData[(i + 1) * width + j].b + arrayOfData[(i + 1) * width + (j + 1)].b), 2));
                pixelWorked = 1;
            }
            else
                pixelWorked = 0;
            sem_post(binarySemaphore);
            
            if(pixelWorked == 0) //unlocks a semaphore i.e decrements it form 1 to 0
                j -= 1;
        }
        if(pixelWorked == 0)
            i -= 1;
    }


/*
Essentially, the operation being done here is as follows:

[ x, p, q    =   [(i+1,j-1), (i+1,j), (i+1,j+1)
  y, s, b    =    (i,j-1),   (i,j),   (i,j+1)
  z, q, c]   =    (i-1,j-1), (i-1,j), (i-1,j+1)]

for each respective pixel, calculate edge detection = sqrt[ [(x+2y+z)-(a+2b+c)]^2 + [(c+2q+z)-(x+2p+q)]^2 ]
*/
// Writing in newArrayofData with new estimated gradient values
    for(int i = height - 1; i > 0; i--){
        for(int j = 1; j <= width - 1; j++){
            arrayOfData[i * width + j] = newArrayofData[i][j];
        }
    }
}


// Function to get current time slot
auto currTime(){
    return chrono::high_resolution_clock::now();
}

// Main function
int main(int argc, char** argv){

    char PPcheck[10]; // String to check PP version

    int height, width, colorMAX; // Integers declared to check height, width, colorMAX (=255)
    int r, g, b; // Integers for colors defined
    
    // Open file and read content
    FILE *inputImage = fopen(argv[1], "r");
    fscanf(inputImage, "%s%d%d%d", PPcheck, &width, &height, &colorMAX);
    
    // Vector of a vector defined arrayofData

    struct pixel *arrayOfData;

    key_t key = 0x1234;
    int shmid = shmget(key, sizeof(struct pixel) * (height) * width, 0666 | IPC_CREAT);
    arrayOfData = (struct pixel *)shmat(shmid, NULL, 0);
    // storeRGB(inputImage, arrayOfData, height, width);

    struct pixel temp;

    // Loop to store the read values in vectors
    for (int i = height - 1; i >= 0; i--){
    // for (int i = 0; i <= height - 1; i++){
        for (int j = 0; j <= width - 1; j++){
            fscanf(inputImage, "%d%d%d", &r, &g, &b);
            temp.r = r;
            temp.g = g;
            temp.b = b;
            arrayOfData[(i * width) + j] = temp;
        }
    }
    fclose(inputImage);
    
    key_t limitKeyOfT2 = 0x1235;
    int *goBack;
    int shmid2 = shmget(limitKeyOfT2, sizeof(int) * 2, 0666 | IPC_CREAT);
    goBack = (int *)shmat(shmid2, NULL, 0);
    
    // Time connection start
    auto start = currTime();
    
    sem_t *binarySemaphore = sem_open(semaphoreName, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE); //named semaphore

    //Function calling
    Edge_Detection(limitKeyOfT2, key, fork(), height, width);
    RBG_2_Gray_Conversion(limitKeyOfT2, key, fork(), height, width);

    wait(NULL); wait(NULL);

    // Time connection stop
    auto stop = currTime();

    auto duration = chrono::duration_cast<microseconds>(stop - start);
    cout << "Time Elapsed: " << duration.count() << " microseconds" << endl;

    // Output file writing
    FILE *outputImage = fopen(argv[2], "w");
    fprintf(outputImage, "%s\n%d %d\n%d\n", PPcheck, width, height, colorMAX);

    // Printing all values in final output image
    for (int i = height - 1; i >= 0; i--){
        for (int j = 0; j <= width - 1; j++){
            temp = arrayOfData[(i * width) + j];
            fprintf(outputImage, "%d ", temp.r);
            fprintf(outputImage, "%d ", temp.g);
            fprintf(outputImage, "%d ", temp.b);
        }
        fprintf(outputImage, "\n");
    }
    fclose(outputImage);

    return 0;
}
