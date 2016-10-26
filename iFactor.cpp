#include <iostream>
#include <fstream>
#include <sstream>
#include <gmp.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

using namespace std;

// max input size
const unsigned int MAX_INPUT_SIZE = 512;

// N - the number we are trying to factor
mpz_t N;

// Create factorOne variable mutex for thread safe operation
mutex factorMutex;

// Number of threads to use
unsigned int numberOfThreads = 2;

// Variable to which tells threads whether or not another thread already has the answer
atomic<bool> factorFound(false);

/*
 *  psuedoRandom - uses a pseudo random function to generate random feeling values.
 */
inline void pseudoRandom(mpz_t x, unsigned long randomNumber) {
	// x^2 + a mod N
	mpz_mul(x,x,x);	// x^2
	mpz_add_ui(x, x, randomNumber);	// x^2 + a
	mpz_tdiv_r(x, x, N);		// (x^2 + a) mod n
}

/*
 * Runs the pollard rho algorithm with Floyd's cycle detection scheme to attempt to factor N.
 */
void pollardRho(mpz_t factor) {
	// Create random number for pseudoRandom
	srand(time(NULL));	// Generate the seed
	unsigned long randNumb = rand() % 100 + 2;		// Get a random number between 2 and 100
	
	// Try and find the factors of N
	mpz_t a;
	mpz_t b;
	mpz_init_set_ui(a, 2);
	mpz_init_set_ui(b, 2);
	
	// Initialize p
	mpz_t p;
	mpz_init(p);
	
	// Calculate the next a and b
	pseudoRandom(a, randNumb);
	pseudoRandom(b, randNumb);
	pseudoRandom(b, randNumb);

	// Search while they are not equal
	while(mpz_cmp(b, a) != 0) {
		// Calculate the GCD
		mpz_sub(p, b, a);
		mpz_abs(p, p);
		mpz_gcd(p, p, N);
		
		// See if we've found a factor
		if(mpz_cmp_ui(p, 1) > 0 && mpz_cmp(p, N) < 0) {
			// Tell the other threads its over
			factorFound = true;
			
			// Lock factor for thread safe operation
			lock_guard<mutex> lock(factorMutex);
			
			// Set the factor for return
			mpz_set(factor, p);
			
			// End the loop
			break;
		}
		
		// See if another thread has already found the answer
		if (factorFound.load()) {
			// end the loop so we may be done
			break;
		}
		
		pseudoRandom(a, randNumb);
		pseudoRandom(b, randNumb);
		pseudoRandom(b, randNumb);
	}
	// Clear initialized variables
	mpz_clear(p);
	mpz_clear(a);
	mpz_clear(b);
}

/*
 * Generate and displays the public and private keys.  Also, write the private keys to
 * a file.
 */
void generateKeys(mpz_t p, mpz_t q, mpz_t e) {
	// k = (p-1)*(q-1)
	mpz_t k;
	mpz_init(k);
	
	mpz_t tmp1;
	mpz_t tmp2;
	mpz_init(tmp1);
	mpz_init(tmp2);
	
	mpz_sub_ui(tmp1, p, 1);
	mpz_sub_ui(tmp2, q, 1);
	mpz_mul(k, tmp1, tmp2);
	
	mpz_clear(tmp1);
	mpz_clear(tmp2);
	
	// find d using e*d = 1 mod k
	mpz_t d;
	mpz_init(d);
	
	mpz_invert(d, e, k);
	
	cout << "\tRSA Keys:\n";
	cout << "\t\tN: "; 
	mpz_out_str(stdout, 10, N);
	cout << endl;
	
	cout << "\t\te: "; 
	mpz_out_str(stdout, 10, e);
	cout << endl;
	
	cout << "\t\td: "; 
	mpz_out_str(stdout, 10, d);
	cout << endl << endl;
	
	// Create a privateKey.txt file with N on the first line and d on the next.
	ofstream privateKeyFile;
	privateKeyFile.open("privateKey.txt", ios::trunc);
	
	unsigned long charSize = mpz_sizeinbase(N, 10);
	char *mpzToStr = new char[charSize+2];
	mpz_get_str(mpzToStr, 10, N);
	
	privateKeyFile << mpzToStr << endl;
	delete [] mpzToStr;
	
	charSize = mpz_sizeinbase(d, 10);
	mpzToStr = new char[charSize+2];
	mpz_get_str(mpzToStr, 10, d);
	
	privateKeyFile << mpzToStr << endl;
	privateKeyFile.close();
	
	delete [] mpzToStr;
	
	mpz_clear(d);
	
	cout << "Private Key written to privateKey.txt\n";
}

// Prints the proper program usage.
void printProperUsage(char *name) {
	// Print proper usage
	cout << "Usage:\t" << name << " numberToFactor [-t numberOfthreads]" << endl;
	cout << "\t" << name << " -k keyFile [-t numberOfthreads]\n";
	cout << "\t\t-k - Attempts to factor N from a public key file.  If successful, outputs privateKey.txt\n";
}

int main(int argc, char *argv[]) {
	// Define flags for varried behavier to be toggled by command line options
	bool genKeys = false;
	
	// Variable to hold second part of key if present
	mpz_t e;
	mpz_init(e);
	
	// Get input from command line
	if (argc < 2) {
		printProperUsage(argv[0]);
		return 0;
	} else {
		// Checking each argument for validity
		string input = argv[1];
		if (input == "-k" && argc >= 3) {
			// Outputting keys
			genKeys = true;
			
			// Check file argument for validity
			ifstream publicKey(argv[2]);
			if (publicKey.good()) {
				// Get N from the first line of the file
				// File input char buffer
				char fileCharBuff[MAX_INPUT_SIZE];
				
				// Get N from file
				publicKey.getline(fileCharBuff, MAX_INPUT_SIZE);
				if(mpz_init_set_str(N, fileCharBuff, 0) != 0) {
						cout << "First file line is not a number!";
						return 3;
				}
				
				// Get E from file
				publicKey.getline(fileCharBuff, MAX_INPUT_SIZE);
				if(mpz_init_set_str(e, fileCharBuff, 0) != 0) {
						cout << "Second file line is not a number!\n";
						return 4;
				}
			} else {
				// File is no good!
				cout << "\t\tFile name is no good!\n\n";
				printProperUsage(argv[0]);
					
				return 2;
			}
			
			// See if the number of threads to use was also added
			if (argc == 5) {
				input = argv[3];
				if (input == "-t") {
					int numInput = atoi(argv[4]);
					
					if (numInput < 1) {
						numberOfThreads = 1;
					} else {
						numberOfThreads = numInput;
					}
				}
			} else {
				if (argc > 3) {
					printProperUsage(argv[0]);
					return 6;
				}
			}
		} else if (!mpz_init_set_str(N, input.c_str(), 0)) {
			// See if more arguments were entered
			if (argc > 2) {
				input = argv[2];
				
				// See if number of threads to use was entered.
				if (input == "-t" && argc == 4) {
					int numInput = atoi(argv[3]);
					
					if (numInput < 1) {
						numberOfThreads = 1;
					} else {
						numberOfThreads = numInput;
					}
				} else {
					printProperUsage(argv[0]);
					return 5;
				}
			}
		} else {
			printProperUsage(argv[0]);
			return 1;
		}	
	}
	
	cout << "\nAttempting to factor ";
	mpz_out_str(NULL, 10, N);
	cout << endl << endl;

	// Start timing the factorization process
	chrono::high_resolution_clock::time_point startTime, endTime;
	startTime = chrono::high_resolution_clock::now();
	
	// Initialize the result variable
	mpz_t factorOne;
	mpz_init(factorOne);
	
	// Create a vector of x threads and start them each running pollardRho
	std::vector<std::thread> threads;
	
	for(unsigned int i=0; i<numberOfThreads; ++i) {
		threads.push_back(thread(pollardRho, factorOne));
	}
	
	// Wait for the threads to finish
	for(auto& x : threads) {
		x.join();
	}
	
	// Retreive the end time
	endTime = chrono::high_resolution_clock::now();
	
	// Find the second factor
	mpz_t factorTwo;
	mpz_init(factorTwo);

	mpz_tdiv_q(factorTwo, N, factorOne);	// factorTwo = N/factorOne
	
	// Display the Factors that we found
	cout << "\tThe following factors where found:\n";
	cout << "\t\t1st: "; 
	mpz_out_str(stdout, 10, factorOne);
	cout << endl;

	cout << "\t\t2nd: ";
	mpz_out_str(NULL, 10, factorTwo);
	cout << endl << endl;

	// Display the time the factorization took
	cout << "Total time to factor: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << " ms\n\n";
	
	// See if we should generate the public and private keys based on the factorization we just did.
	if (genKeys) {
		if (mpz_cmp(factorOne, factorTwo) < 0) {
			generateKeys(factorOne, factorTwo, e);
		} else {
			generateKeys(factorTwo, factorOne, e);
		}
	}
	
	// Release all the variables
	mpz_clear(N);
	mpz_clear(e);
	mpz_clear(factorOne);
	mpz_clear(factorTwo);

	return 0;
}
