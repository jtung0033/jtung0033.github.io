//============================================================================
// Name        : VectorSorting.cpp
// Author      : Jeffrey Tung
// Version     : 1.0
// Copyright   : Copyright © 2017 SNHU COCE
// Description : Vector Sorting Algorithms
//============================================================================

#include <algorithm>
#include <iostream>
#include <time.h>

#include "CSVparser.hpp"

using namespace std;

//============================================================================
// Global definitions visible to all methods and classes
//============================================================================

// forward declarations
double strToDouble(string str, char ch);

// define a structure to hold bid information
struct Bid {
    string bidId; // unique identifier
    string title;
    string fund;
    double amount;
    Bid() {
        amount = 0.0;
    }
};

//============================================================================
// Static methods used for testing
//============================================================================

/**
 * Display the bid information to the console (std::out)
 *
 * @param bid struct containing the bid info
 */
void displayBid(Bid bid) {
    cout << bid.bidId << ": " << bid.title << " | " << bid.amount << " | "
            << bid.fund << endl;
    return;
}

/**
 * Prompt user for bid information using console (std::in)
 *
 * @return Bid struct containing the bid info
 */
Bid getBid() {
    Bid bid;

    cout << "Enter Id: ";
    cin.ignore();
    getline(cin, bid.bidId);

    cout << "Enter title: ";
    getline(cin, bid.title);

    cout << "Enter fund: ";
    cin >> bid.fund;

    cout << "Enter amount: ";
    cin.ignore();
    string strAmount;
    getline(cin, strAmount);
    bid.amount = strToDouble(strAmount, '$');

    return bid;
}

/**
 * Load a CSV file containing bids into a container
 *
 * @param csvPath the path to the CSV file to load
 * @return a container holding all the bids read
 */
vector<Bid> loadBids(string csvPath) {
    cout << "Loading CSV file " << csvPath << endl;

    // Define a vector data structure to hold a collection of bids.
    vector<Bid> bids;

    // initialize the CSV Parser using the given path
    csv::Parser file = csv::Parser(csvPath);

    try {
        // loop to read rows of a CSV file
        for (unsigned int i = 0; i < file.rowCount(); i++) {

            // Create a data structure and add to the collection of bids
            Bid bid;
            bid.bidId = file[i][1];
            bid.title = file[i][0];
            bid.fund = file[i][8];
            bid.amount = strToDouble(file[i][4], '$');

            //cout << "Item: " << bid.title << ", Fund: " << bid.fund << ", Amount: " << bid.amount << endl;

            // push this bid to the end
            bids.push_back(bid);
        }
    } catch (csv::Error &e) {
        std::cerr << e.what() << std::endl;
    }
    return bids;
}

//

/**
 * Partition the vector of bids into two parts, low and high
 *
 * @param bids Address of the vector<Bid> instance to be partitioned
 * @param begin Beginning index to partition
 * @param end Ending index to partition
 */
int partition(vector<Bid>& bids, int begin, int end, string sortArg) {
	int low = begin;
	int high = end;

	// establishing the middle element as the pivot
	int pivot = begin + (end - begin) / 2;

	bool done = false;

	while (!done) {
		// increment low while bids at low < pivot
		if (sortArg == "bidId") {
			while (bids.at(low).bidId.compare(bids.at(pivot).bidId) < 0) {
				++low;
			}
		// decrement high while pivot < bids at high
			while (bids.at(pivot).bidId.compare(bids.at(high).bidId) < 0) {
				--high;
			}
		} else if (sortArg == "title") {
			while (bids.at(low).title.compare(bids.at(pivot).title) < 0) {
				++low;
			}
		// decrement high while pivot < bids at high
			while (bids.at(pivot).title.compare(bids.at(high).title) < 0) {
				--high;
			}
		} else if (sortArg == "fund") {
			while (bids.at(low).fund.compare(bids.at(pivot).fund) < 0) {
				++low;
			}
		// decrement high while pivot < bids at high
			while (bids.at(pivot).fund.compare(bids.at(high).fund) < 0) {
				--high;
			}
		} else if (sortArg == "amount") {
			while (bids.at(low).amount < bids.at(pivot).amount) {
				++low;
			}
		// decrement high while pivot < bids at high
			while (bids.at(pivot).amount < bids.at(high).amount) {
				--high;
			}
		}

		if (low >= high) {
			done = true;
		} else {
			// swap the low and high bids
			swap(bids.at(low), bids.at(high));
			//move low and high closer
			++low;
			--high;
		}
	}
	return high;
}

/**
 * Perform a quick sort on bid title
 * Average performance: O(n log(n))
 * Worst case performance O(n^2))
 *
 * @param bids address of the vector<Bid> instance to be sorted
 * @param begin the beginning index to sort on
 * @param end the ending index to sort on
 */
void quickSort(vector<Bid>& bids, int begin, int end, string sortArg) {
	int midPt = 0;

	// If there are zero of one bids to sort, exit recursive loop
	if (begin >= end) {
		return;
	}

	// partition bids to low and high partitions
	midPt = partition(bids, begin, end, sortArg);
	// recursive call of quicksort for low partition
	quickSort(bids, begin, midPt, sortArg);
	// recursive call of quicksort for high partition
	quickSort(bids, midPt + 1, end, sortArg);
}

//

/**
 * Perform a selection sort on bid title
 * Average performance: O(n^2))
 * Worst case performance O(n^2))
 *
 * @param bid address of the vector<Bid>
 *            instance to be sorted
 */
void selectionSort(vector<Bid>& bids, string sortArg) {
	//initializing variable to the current minimum bid
	unsigned int minBid;
	// start at the first position within bids and increment
	for (unsigned int i = 0; i < bids.size(); ++i) {
		minBid = i;
		//inner nested for loop to check for minBid for the rest of the vector
		for (unsigned int j = i + 1; j < bids.size(); ++j) {
			if (sortArg == "bidId") {
				if (bids.at(j).bidId.compare(bids.at(minBid).bidId)<0) {
					minBid = j;
				}
			} else if (sortArg == "title") {
				if (bids.at(j).title.compare(bids.at(minBid).title)<0) {
					minBid = j;
				}
			} else if (sortArg == "fund") {
				if (bids.at(j).fund.compare(bids.at(minBid).fund)<0) {
					minBid = j;
				}
			} else if (sortArg == "amount") {
				if (bids.at(j).amount < bids.at(minBid).amount) {
					minBid = j;
				}
			}
		}
		//swap minimum bid if the minBid has changed
		if (minBid != i) {
			swap(bids.at(i), bids.at(minBid));
		}
	}
}

/**
 * Simple C function to convert a string to a double
 * after stripping out unwanted char
 *
 * credit: http://stackoverflow.com/a/24875936
 *
 * @param ch The character to strip out
 */
double strToDouble(string str, char ch) {
    str.erase(remove(str.begin(), str.end(), ch), str.end());
    return atof(str.c_str());
}

/**
 * The one and only main() method
 */
int main(int argc, char* argv[]) {

    // process command line arguments
    string csvPath;
    switch (argc) {
    case 2:
        csvPath = argv[1];
        break;
    default:
        csvPath = "eBid_Monthly_Sales_Dec_2016.csv";
        //csvPath = "eBid_Monthly_Sales.csv";
    }

    // Define a vector to hold all the bids
    vector<Bid> bids;

    // Define a timer variable
    clock_t ticks;

    int choice = 0;
    int sortChoice;
    string sortArg;
    unsigned int displayLimit = 100000000;
    while (choice != 9) {
        cout << "Menu:" << endl;
        cout << "  1. Load Bids" << endl;
        cout << "  2. Display All Bids" << endl;
        cout << "  3. Selection Sort All Bids" << endl;
        cout << "  4. Quick Sort All Bids" << endl;
        cout << "  5. Display Limited Amount of Bids" << endl;
        cout << "  9. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {

        case 1:
            // Initialize a timer variable before loading bids
            ticks = clock();

            // Complete the method call to load the bids
            bids = loadBids(csvPath);

            cout << bids.size() << " bids read" << endl;

            // Calculate elapsed time and display result
            ticks = clock() - ticks; // current clock ticks minus starting clock ticks
            cout << "time: " << ticks << " clock ticks" << endl;
            cout << "time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;

            break;

        case 2:
            // Loop and display the bids read
            for (unsigned int i = 0; i < bids.size(); ++i) {
                displayBid(bids[i]);
            }
            cout << endl;

            break;


        case 3:
        	cout << "Sorting Options:" << endl;
        	cout << "  1. Sort By ID" << endl;
        	cout << "  2. Sort By Title" << endl;
        	cout << "  3. Sort By Fund" << endl;
        	cout << "  4. Sort By Amount" << endl;
        	cout << "Enter Sorting Choice: ";
        	cin >> sortChoice;
        	//decision tree for inputted sorting argument
        	ticks = clock();
        	if (sortChoice == 1) {
        		//Sorting by ID
        		sortArg = "bidId";
        		// Complete the method call to sort the bids by ID
        		selectionSort(bids, sortArg);
        	} else if (sortChoice == 2) {
        		//Sorting by title
        		sortArg = "title";
        		// Complete the method call to sort the bids by title
        		selectionSort(bids, sortArg);
        	} else if (sortChoice == 3) {
        		//Sorting by fund
        		sortArg = "fund";
        		// Complete the method call to sort the bids by fund
        		selectionSort(bids, sortArg);
        	} else if (sortChoice == 4) {
        		//Sorting by amount
        		sortArg = "amount";
        		// Complete the method call to sort the bids by amount
        		selectionSort(bids, sortArg);
        	}
    		cout << bids.size() << " bids read" << endl;

            ticks = clock() - ticks; // current clock ticks minus starting clock ticks
            cout << "time: " << ticks << " clock ticks" << endl;
            cout << "time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
            break;

        case 4:
        	cout << "Sorting Options:" << endl;
        	cout << "  1. Sort By ID" << endl;
        	cout << "  2. Sort By Title" << endl;
        	cout << "  3. Sort By Fund" << endl;
        	cout << "  4. Sort By Amount" << endl;
        	cout << "Enter Sorting Choice: ";
        	cin >> sortChoice;
        	//decision tree for inputted sorting argument
        	ticks = clock();
        	if (sortChoice == 1) {
        		//Sorting by ID
        		sortArg = "bidId";
        		// Complete the method call to sort the bids by ID
        		quickSort(bids, 0, bids.size() - 1, sortArg);
        	} else if (sortChoice == 2) {
        		//Sorting by title
        		sortArg = "title";
        		// Complete the method call to sort the bids by title
        		quickSort(bids, 0, bids.size() - 1, sortArg);
        	} else if (sortChoice == 3) {
        		//Sorting by fund
        		sortArg = "fund";
        		// Complete the method call to sort the bids by fund
        		quickSort(bids, 0, bids.size() - 1, sortArg);
        	} else if (sortChoice == 4) {
        		//Sorting by amount
        		sortArg = "amount";
        		// Complete the method call to sort the bids by amount
        		quickSort(bids, 0, bids.size() - 1, sortArg);
        	}
    		cout << bids.size() << " bids read" << endl;

            ticks = clock() - ticks; // current clock ticks minus starting clock ticks
            cout << "time: " << ticks << " clock ticks" << endl;
            cout << "time: " << ticks * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
            break;

        case 5:
        	while (displayLimit > bids.size()) {
            	cout << "Enter the number of bids to display (must be less than number of bids):";
    			cin >> displayLimit;
        	}
            for (unsigned int i = 0; i < displayLimit; ++i) {
                displayBid(bids[i]);
            }
            cout << endl;
            //resetting displayLimit to original value
            displayLimit = 100000000;

            break;
        }
    }

    cout << "Good bye." << endl;

    return 0;
}
