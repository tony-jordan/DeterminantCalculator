// Team: rGames.co
// Comments: Tony Jordan
// Math: Tony Jordan
// Threading: Tyler Spicer and Tony Jordan

// We tested some smaller matrices on https://matrix.reshish.com/determinant.php
// and https://www.symbolab.com/solver/matrix-determinant-calculator to get
// our basic algorithm working correctly

// Find the determinant of a matrix with any number of x by x elements
// while optimizing code to run efficiently on multi-core systems.
#include <iostream>
#include <vector>
#include <cmath>
// stdlib and stdio are used for the fragments of C code present to open .bin files
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <chrono>
// the ctpl_stl.h header file was used so we would have access to
// thread pools, the documentation is available at https://github.com/vit-vit/CTPL/blob/master/ctpl_stl.h
#include "ctpl_stl.h"
// mutex is used so we can utilize locks to safely push tasks to our threadpool
#include <mutex>

using namespace std;
using namespace std::chrono;
// initialize lock
std::mutex mtx;

const double dimensions = 16;
// declare a 2d vector of size dimensions
std::vector<std::vector<long double>> matrix(dimensions, vector<long double>(dimensions, 0));
// the number of threads in the threadpool will be the same as the number of
// cores on the device being used, this was done to maximize efficiency
int cores = std::thread::hardware_concurrency();
// logr is used in the findBase10 function, it needs to be a class variable so each thread
// can interact with it.
long double logr = 0;
// same with the determinant variable. It also needs to be initialized to 1 to preform
// the math that will be explained further in the code
long double determinant = 1;

// Without getting too in depth with how the math works just yet, it is important to know that in order
// to calculate the determinant with this specific mathematical process, the first 
// element of the matrix cannot be equivalent to 0, under any curcumstances
// so the exchange row method was created so that if this is the case, we can
// preform a row swap to make sure the criteria of utilizing  our mathematical
// process is correct 
void exchangeRow() {
	// make sure the first element is not 0;
	bool result = false;
	while (!result) {
		if (matrix[0][0] != 0) {
			result = true;
		}
		else {
			vector <long double> temp(dimensions);
		// get an array consisting of the row to swap
			for (int y = 0; y < matrix[0].size(); y++) {
				temp[y] = matrix[0][y];
			}
		// swap the elements
			for (int y = 0; y < matrix[0].size(); y++) {
				matrix[0][y] = matrix[1][y];
			}
		// finish swapping the elements
			for (int y = 0; y < matrix[0].size(); y++) {
				matrix[1][y] = temp[y];
			}
			result = true;
		}
	}
}
// The triangular method has three parameters, for some strange reason when we use threadpools
// the push method ingnores the first parameter, so we created a throw away parameter
// and included our necessary parameters after that.
// The reason this method only focuses on two rows at a time is for threading purposes.
// essentially we use row operations to create a specific pattern within the matrix,
// documentation on how row operations work can be found at https://www.purplemath.com/modules/mtrxrows.htm
// The pattern is called upper triangular, and since I can't think of a different way to
// explain how that looks, I will show a demonstration
/*
		| 1 2 3 |
		| 0 4 5 |
		| 0 0 6 |
 Every value below the diagonal line of 1, 4 & 6 is 0
*/
// We basically have to get the entire matrix in that form. A website to help explain how to
// get a matrix in this form can be found here
// https://www.sciencedirect.com/topics/computer-science/upper-triangular-form
// if you don't want to look at that, just know the basic equation is
// Row 2, at position x = Row 2 at postion x - (Row 2 0 position / Row 1 0 position) * Row 1 at position x
// (Where 0 position is the element we are trying to get to equal 0)
void triangular(int param, int MainRow, int setRow) {
	if (matrix[setRow][MainRow] != 0) {
		double mul = matrix[setRow][MainRow];
		for (int y = MainRow; y < matrix.size(); y++) {
			matrix[setRow][y] -= mul / matrix[MainRow][MainRow] * matrix[MainRow][y];
		}
	}
}
// To find the log base 10 of the determinant we had to do some cheeky math
// in reality, its the Log10(abs(det)), but the absolute value can be applied at the end
// because of the process of splitting up logs.
// The issue came about when finding the determinant of a very large matrix, the determinant
// was so big, that C++ recognizes it as infinity, and what is the log base 10 of infinity?
// well, infinity. So we used a property that lets us take apart the determinant.
// A website detailing the properties of logs can be found here
// http://dl.uncw.edu/digilib/Mathematics/Algebra/mat111hb/EandL/logprop/logprop.html
// but essentially, log(x) + log(y) = log(xy)
// knowing this, we can take the values of the diagonal, and put them in a loop
// that preforms this operation, without having to worry about
// the end result being infinity.
void findBase10() {
	// make a vector that holds the number of elements in the diagonal
	// we are going to multiply each number together until we reach infinity,
	// if that's the case, we'll define the next element of the vector
	vector<long double> dets(dimensions, 1);
	int index = 0;
	// find the log base 10
	for (int x = 0; x < matrix.size(); x++) {
		if (!isinf(dets[index] * matrix[x][x])) {
			dets[index] *= matrix[x][x];
		}
		else {
			index++;
			dets[index] *= matrix[x][x];
		}
	}
	// this is where we preform the log property
	for (int x = 0; x <= index; x++) {
		logr += log10(abs(dets[x]));
	}
}
// finding the determinant is as simple as multiplying the diagonal of the matrix
void findDet() {
	for (int x = 0; x < matrix.size(); x++) {
		determinant *= matrix[x][x];
	}
}

int main() {
	// get the start time
	auto start = high_resolution_clock::now();
	// this is C code to read bin files, we
	// adopted sample code from http://morpheus.mcs.utulsa.edu/~papama/hpc/
	// which is the website for the competition this code was used in.
	char f_name[50];
	int i, j;
	//Create filename
	// you would insert your bin file location here
	sprintf(f_name, "E:\\BinReader\\m0016x0016.bin");
	//Open file
	FILE *datafile = fopen(f_name, "rb");
	//Read elelements
	for (i = 0; i < dimensions; i++)
		for (j = 0; j < dimensions; j++)
		{
			fread(&matrix[i][j], sizeof(double), 1, datafile);
		}
	printf("Matrix has been read.\n");
	// make sure the first element is not 0
	if (matrix[0][0] == 0) {
		exchangeRow();
	}
	// Create a thread pool where the number of threads is equivalent to the number of cores on this device
	ctpl::thread_pool p(cores);
	// Create a loop to push necessary tasks to the threadpool
	for (int x = 0; x < dimensions; x++) {
		// lock the process, so multiple threads don't work on the same thing
		mtx.lock();
		for (int y = x + 1; y < dimensions; y++) {
			p.push(triangular, x, y);
			// So, a working theory that we had was that, the threads did not have enought time 
			// to finish what they were doing, so the main continued on while the threads were still working
			// and we were given the wrong answer, I (Tony Jordan speaking) toiled on this for WEEKS
			// trying all sorts of different stuff, I tried incrementing a thread variable and printing
			// out every value, which seemed to work, but it looked really bad. Then I tried coming 
			// up with a proportion, that printed out variables based on how long (I thought) each
			// calculation needed to finish. I enlisted Tyler to help me with this issue, after I had ripped
			// my hair out because of it, and at first he didn't have too much success either. We
			// tried pausing the main thread, but that didn't work because it paused processes that really didn't
			// need to sleep. This is actually why the log base 10 and diagonal determinant calculator 
			// functions are separate, because I tried making a big thread with the threadpool contained within it
			// and joining that thread, so that the main would wait to continue until the big thread was done
			// but that broke EVERYTHING, eventually we went back to what I did at first, printing out
			// every thread number, just so we could actually get the answer right, but then, the mad
			// lad Tyler decided to just print out a space, and then and endline. And then all of the sudden
			// it worked, and not only did it work it was FAST. And after that, he just made it 
			// an endline. which worked EVEN FaStEr!
			// At the time, this made absolutely NO sense at all, but after doing some testing and 
			// critical thinking, the theory is that when the thread completed its task, it was left to
			// its own devices which is REALLY bad when using threads. So when it prints a newline character,
			// The thread is occupied until it is called again. (Althought that shouldn't be the
			// case because I'm only using the threadpool to push the triangular method, so it
			// shouldn't interact with the cout, but it works very well so I'm not complaining.) 
			cout << endl;
		}
		mtx.unlock();
		
	}

	// invoke functions to find determinant and log base 10
	findBase10();
	findDet();
	// setprecision to print out everything very nicely
	cout << std::scientific << setprecision(6);
	// format cout like on the competition website
	// (basically, if the determinant is positive infinity, add a plus sign before it)
	if (determinant > 0 && isinf(determinant)) {
		cout << endl << "det = +" << determinant << endl;
	}
	else {
		cout << endl << "det = " << determinant << endl;
	}

	cout << "log(abs(det)) = " << logr << endl;
	// find out how long it took to finish, and display
	auto stop = high_resolution_clock::now();

	auto duration = duration_cast<seconds>(stop - start);
	cout << endl << "Completed in " << duration.count() << " Seconds" << endl;

	return 0;
}



