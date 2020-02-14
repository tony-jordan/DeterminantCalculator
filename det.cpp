// Find the determinant of a matrix with any number of x by x elements
#include <iostream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <chrono>
#include <complex>
#include <thread>
#include "ctpl_stl.h"
#include <mutex>

using namespace std;
using namespace std::chrono;
std::mutex mtx;

const double dimensions = 64;
std::vector<std::vector<long double>> matrix(dimensions, vector<long double>(dimensions, 0));
int cores = std::thread::hardware_concurrency();

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
void triangular(int stupid, int MainRow, int setRow) {
	// make a for loop do get the matrix in row echelon form, the pattern is R2[x] = R2[x] - R2/R1 * R1[x]
	// we need to make it to where this method only changes one row at a time
	if (matrix[setRow][MainRow] != 0) {
		double mul = matrix[setRow][MainRow];
		for (int y = MainRow; y < matrix.size(); y++) {
			matrix[setRow][y] -= mul / matrix[MainRow][MainRow] * matrix[MainRow][y];
		}
	}
	/*
	for (int row0s = 0; row0s < matrix.size(); row0s++) {
		for (int x = row0s + 1; x < matrix.size(); x++) {
			if (matrix[x][row0s] != 0) {
				double mult = matrix[x][row0s];
				for (int y = row0s; y < matrix.size(); y++) {
					matrix[x][y] -= mult / matrix[row0s][row0s] * matrix[row0s][y];
				}
			}
		}
	}
	*/
}

int main() {
	// get the start time
	auto start = high_resolution_clock::now();

	char f_name[50];
	int i, j;
	//Create filename
	sprintf(f_name, "E:\\BinReader\\m0064x0064.bin");
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
	ctpl::thread_pool p(cores);
	for (int x = 0; x < dimensions - 1; x++) {
		mtx.lock();
		for (int y = x + 1; y < dimensions; y++) {
			p.push(triangular, x, y);
		}
		mtx.unlock();
	}
	mtx.lock();
	p.push(triangular, matrix.size() - 2, matrix.size() - 1);
	mtx.unlock();
	//for (int x = 0; x < matrix.size(); x++) {
		//for (int y = 0; y < matrix.size(); y++) {
			//cout << matrix[x][y] << " ";
		//}
		///cout << endl << endl;
	//}
	long double determinant = 1;
	// make a vector that holds an earnest amount of values, just to be safe
	vector<long double> dets(dimensions, 1);
	// set an index variable to traverse the vector in the loop
	int index = 0;
	// find determinant and da log
	for (int x = 0; x < matrix.size(); x++) {
		determinant *= matrix[x][x];
		if (!isinf(dets[index] * matrix[x][x])) {
			dets[index] *= matrix[x][x];
		}
		else {
			index++;
			dets[index] *= matrix[x][x];
		}
	}
	cout << std::scientific << setprecision(6);
	long double logr = 0;
	// I need a loop to go through the vectors and test to see which values where changed, then add all those
	// values to the base log 10 to the logr variable
	for (int x = 0; x <= index; x++) {
		logr += log10(abs(dets[x]));
	}
	if (determinant > 0 && isinf(determinant)) {
		cout << endl << "det = +" << determinant << endl;
	}
	else {
		cout << endl << "det = " << determinant << endl;
	}

	cout << "log(abs(det)) = " << logr << endl;

	auto stop = high_resolution_clock::now();

	auto duration = duration_cast<seconds>(stop - start);
	cout << endl << "Completed in " << duration.count() << " Seconds" << endl;

	
	return 0;
}



