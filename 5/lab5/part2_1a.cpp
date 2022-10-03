#include <bits/stdc++.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <vector>

using namespace std;
using namespace std::chrono;

// Global variables
atomic_flag flag = ATOMIC_FLAG_INIT; // Initializes the atomicFlag to FALSE
int index_done[2] = {0,0};

//Function for RGB_to_Grayscale
void RBG_2_Gray_Conversion(vector<vector<int>> &r, vector<vector<int>> &g, vector<vector<int>> &b, int height, int width){ 

    for (int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            
            // Spin lock
            while (atomic_flag_test_and_set(&flag));
            
            //Store r, g, b values in each iteration for calculations
            int weighted_mean = ((r[i][j] * 0.3) + (g[i][j] * 0.59) + (b[i][j] * 0.11));

    		// By considering weights for grayscale, appropriate weighted means are taken
            r[i][j] = weighted_mean;
            g[i][j] = weighted_mean;
            b[i][j] = weighted_mean;
            
            index_done[0] = i+1;
            index_done[1] = j+1;
            
            atomic_flag_clear(&flag); // set to false
        }
    }
}

//Function for Edge Detection
void Edge_Detection(vector<vector<int>> &r, vector<vector<int>> &g, vector<vector<int>> &b, int height, int width){ 
    // Ref: https://stackoverflow.com/questions/16385570/sobel-edge-detector-using-c-without-any-special-library-or-tool

	vector<vector<int> > rnew(height, vector<int>(width)), gnew(height, vector<int>(width)), bnew(height, vector<int>(width));
	int op_done = 0;
	
    for(int i = 1; i <= height - 2; i++){
        for(int j = 1; j <= width - 2; j++){
			
			// Spin lock
			while (atomic_flag_test_and_set(&flag));
			
			if (i+1 <= index_done[0] && j+1 <= index_done[1]) {
            rnew[i][j] = sqrt(pow((r[i - 1][j - 1] + 2 * r[i][j - 1] + r[i + 1][j - 1]) - (r[i - 1][j + 1] + 2 * r[i][j + 1] + r[i + 1][j + 1]), 2) + pow((r[i - 1][j - 1] + 2 * r[i - 1][j] + r[i - 1][j + 1]) - (r[i + 1][j - 1] + 2 * r[i + 1][j] + r[i + 1][j + 1]), 2));
            gnew[i][j] = sqrt(pow((g[i - 1][j - 1] + 2 * g[i][j - 1] + g[i + 1][j - 1]) - (g[i - 1][j + 1] + 2 * g[i][j + 1] + g[i + 1][j + 1]), 2) + pow((g[i - 1][j - 1] + 2 * g[i - 1][j] + g[i - 1][j + 1]) - (g[i + 1][j - 1] + 2 * g[i + 1][j] + g[i + 1][j + 1]), 2));
            bnew[i][j] = sqrt(pow((b[i - 1][j - 1] + 2 * b[i][j - 1] + b[i + 1][j - 1]) - (b[i - 1][j + 1] + 2 * b[i][j + 1] + b[i + 1][j + 1]), 2) + pow((b[i - 1][j - 1] + 2 * b[i - 1][j] + b[i - 1][j + 1]) - (b[i + 1][j - 1] + 2 * b[i + 1][j] + b[i + 1][j + 1]), 2));
            
            op_done = 1;
            }
            else
            	op_done = 0;
            
            atomic_flag_clear(&flag); // set to false
            
            if (op_done == 0)
            	j--;
        }
        
        if (op_done == 0)
        	i--;
    }

	// Writing in newArrayofData with new estimated gradient values
    for(int i = height - 1; i > 0; i--){
        for(int j = 1; j <= width - 1; j++){
            r[i][j] = rnew[i][j];
            g[i][j] = gnew[i][j];
            b[i][j] = bnew[i][j];
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
    
    // Open file and read content
    FILE *inputImage = fopen(argv[1], "r");
    fscanf(inputImage, "%s%d%d%d", PPcheck, &width, &height, &colorMAX);

    vector<vector<int> > r(height, vector<int>(width)), g(height, vector<int>(width)), b(height, vector<int>(width));

    // Loop to store the read values in vectors
    for (int i = height - 1; i >= 0; i--){
        for (int j = 0; j <= width - 1; j++){
            fscanf(inputImage, "%d%d%d", &r[i][j], &g[i][j], &b[i][j]);
        }
    }
    fclose(inputImage);
    
    // Time connection start
    auto start = currTime();

    // Threads made for each image transformation
    thread T1 (RBG_2_Gray_Conversion, std::ref(r), std::ref(g), std::ref(b), height, width);
    thread T2 (Edge_Detection, std::ref(r), std::ref(g), std::ref(b), height, width);

    // Waiting for T1 & T2 call by using join()
    T1.join();
    T2.join();

    // Time connection stop
    auto stop = currTime();

    auto duration = chrono::duration_cast<microseconds>(stop - start);
    cout << "Time Elapsed: " << duration.count() << " microseconds" << endl;

    // Output file writing
    FILE *outputImage = fopen(argv[2], "w");
    fprintf(outputImage, "%s\n%d %d\n%d\n", PPcheck, width, height, colorMAX);

    for (int i = height - 1; i >= 0; i--){
        for (int j = 0; j <= width - 1; j++){
            fprintf(outputImage, "%d ", r[i][j]);
            fprintf(outputImage, "%d ", g[i][j]);
            fprintf(outputImage, "%d ", b[i][j]);
        }
        fprintf(outputImage, "\n");
    }
    fclose(outputImage);

    return 0;
}
