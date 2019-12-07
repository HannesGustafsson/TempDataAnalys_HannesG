#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <windows.h>
#include <vector>

using namespace std;

// Max elements limit, also used in the algorithms, to simplify testing
// 304480 is the max
const int inputSize = 304480;

typedef chrono::high_resolution_clock Clock;

// Creates the struct for storing input data
// Date is saved as string to avoid conversion, and simply beacause it's the easiest solution
// Numerical values are saved as float so minimize storage use and preserve decimal
// Date (.date)
// True = Indoors, False = outdoors (.indoors)
// Temperature (.temp)
// Humidity (.hum)
struct dataStruct
{
	string date;
	bool indoors;	
	float temp;		
	float hum;		
	float mould;
	dataStruct() : date("0"), indoors(false), temp(0), hum(0), mould(0) {} //Initializing values
};


//Creating 3 separate vectors for indoors, outdoors and both
vector <dataStruct> dataVec;
vector <dataStruct> dataVecIn;
vector <dataStruct> dataVecOut;

vector <dataStruct> tempDiff(130); // Create vector to hold temperature difference values, used in case 8

// Int for the menu switches
int menuChoice;

//<=========================<FUNCTION START>=========================>

// Opens text file and puts content into main vector
// (bool (will show progress bar if true, silent operation if false))
void getData(bool output)
{
	ifstream text("TempFuktData.txt");
	//If able to open file
	if (text.is_open())
	{
		int c = 0; // Counter 
		while (!(text.eof()) && c < inputSize)
		{
			string temp[4]; // Creates a temporary array to store string values

			getline(text, temp[0], ',');//Assigns element inputData[0] value from textfile until ","
			getline(text, temp[1], ',');//Assigns element inputData[1] value from textfile until ","
			getline(text, temp[2], ',');//Assigns element inputData[2] value from textfile until ","
			getline(text, temp[3]);		//Assigns element inputData[3] value from textfile until "\n"

			// Converts temperature & humidity value to float to preserve decimal and minimize storage usage
			float temp2 = stof(temp[2]);
			float temp3 = stof(temp[3]);

			// Creates prototype object of type dataStruct, to be added in vector
			dataStruct p;
			p.date = temp[0];
			p.temp = temp2;
			p.hum = temp3;

			// If indoors, set .indoors to true
			if (temp[1] == "Inne") 
			{
				p.indoors = true;
			}
			// If outdoors, set .indoors to false
			else 
			{
				p.indoors = false;
			}

			dataVec.push_back(p); // Adds p to vector

			//Shows progress in steps of 10%, only if "output" == true
			if ((c % inputSize) == 0 && output == true)
			{
				cout << "Data analysis";
			}
			if ((c % (inputSize / 10) == 0) && output == true)
			{
				cout << ".";
			}

			c++;
		}

		text.close();//Close the textfile
	}
	//If unable to open file
	else
	{
		cout << "Unable to open file";
	}
}

//<=========================<FUNCTION START>=========================>
// Fills two vectors with temperature and humidity averages, one indoor, and one outdoor
void getAverage()
{
	float inTemp = 0; // Saves the total indoors temperature
	float inHum = 0; // Saves the total indoors humidity
	float inCounter = 0; // Counts number of indoor inputs

	float outTemp = 0; // Saves the total outdoors temperature
	float outHum = 0; // Saves the total outdoors humidity
	float outCounter = 0; // Counts number of outdoor inputs

	dataStruct p = dataVec[0]; // Creates a base dataStruct object to compare dataVec[i] with

	for (size_t i = 1; i < dataVec.size(); i++) // Cycles through dataVec
	{

		// If current date is the same as previous date (not including the time)
		if (p.date.substr(0, 10) == dataVec[i].date.substr(0,10))
		{
			// If indoors, save values to indoors variables and increase its counter
			if (dataVec[i].indoors == true)
			{
				inTemp += dataVec[i].temp;
				inHum += dataVec[i].hum;
				inCounter++;
			}
			// If outdoors, save values to outdoors variables and increase its counter
			else if (dataVec[i].indoors == false)
			{
				outTemp += dataVec[i].temp;
				outHum += dataVec[i].hum;
				outCounter++;
			}
		}

		// If current date is not the same as previous, calculate averages and add to vectors
		else
		{
			inTemp /= inCounter; // Calculate average indoor temp for previous day
			inHum /= inCounter; // Calculate average indoor humidity for previous day
			outTemp /= outCounter; // Calculate average outdoor temp for previous day
			outHum /= outCounter; // Calculate average outdoor humidity for previous day

			p.indoors = true;
			p.temp = roundf(inTemp * 10) / 10; //Multiplying value by 10, rounding up, and dividing by 10. To reduce it to one decimal
			p.hum = roundf(inHum * 10) / 10;
			dataVecIn.push_back(p); // Add values to vector
			inTemp = 0; inHum = 0; inCounter = 0; // Reset variables

			p.indoors = false;
			p.temp = roundf(outTemp * 10) / 10;
			p.hum = roundf(outHum * 10) / 10;
			dataVecOut.push_back(p); // Add values to vector
			outTemp = 0; outHum = 0; outCounter = 0; // Reset variables

		}
		p = dataVec[i];
	}
}

//<=========================<FUNCTION START>=========================>
// Get mould risk for chosen vector, output value 0 - 10, higher number means higher risk.
// (vector)
void getMould(vector<dataStruct>& vector)
{
	// Increase in humidity required to cross threshold for mould development, per degree c from (< 15 - 0).
	// (100% - 78% = 22%). (22% / 15 = 1.47%) (1°C needs a humidity of (78% + ((15 - 1°C) * 1.47%) = 98.58%) to be in the risk zone.
	float increase = 1.47; 

	for (size_t i = 0; i < vector.size(); i++)
	{
		// If temperature is over threshold (but not above 50°C), the humidity is what deciedes the mould risk, since risk does not increase with temp over 15°C
		// Since 100% is max and 78% is min, that leaves a span of 0-22
		if ((vector[i].temp > 15 && vector[i].temp < 50) && vector[i].hum > 78) 
		{
			float tempMould = vector[i].hum - 78;
			tempMould = roundf(tempMould * 10) / 10;
			vector[i].mould = tempMould * 0.27; // Times 0.45 to turn the result interval from 0-22 to 0-6
		}
		// If temperature < 15, higher humidity is required to compensate for low temperature (1.47% per each degree below 15°C, until it reaches 0°C)
		else if ((vector[i].temp < 15 && vector[i].temp > 0) && (vector[i].hum > (78 + (15 - vector[i].temp) * increase)))
		{
			float tempMould = (vector[i].hum - (78 + (15 - vector[i].temp) * increase));
			tempMould = roundf(tempMould * 10) / 10;
			vector[i].mould = tempMould * 0.45;
		}
	}
}

//<=========================<FUNCTION START>=========================>
// Creates 2 subarrays and merging them into the input vector
// (vector, (l)eft index, (m)iddle index, (r)ight index, val (0 = date, 1 = temp, 2 = hum, 3 = mould))
void merge(vector<dataStruct>& vector, int l, int m, int r, int val)
{
	int i, j, k;
	int n1 = m - l + 1; // Number of elements in left array
	int n2 = r - m;		// Number of elements in right array

	//Create temprary arrays
	dataStruct* L = new dataStruct[n1];	// (L)eft subarray
	dataStruct* R = new dataStruct[n2];	// (R)ight subarray

	// Copies vector values into subarrays
	for (i = 0; i < n1; i++)
	{
		L[i] = vector[l + i]; // (L)eft values
	}
	for (j = 0; j < n2; j++)
	{
		R[j] = vector[m + 1 + j]; // (R)ight values
	}

	// Merge the subarrays into vector
	i = 0; // First index of (L)eft subarray 
	j = 0; // First index of (R)ight subarray 
	k = l; // First index of merged vector
	while (i < n1 && j < n2) // While limits of subarray indexes have not been reached
	{
		// Sorts by date if val is 0
		if (val == 0)
		{
			if (L[i].date <= R[j].date) // If the first element in Left array is smaller or same as the first in Right array
			{
				vector[k] = L[i]; // Place the smallest into merged array
				i++; // Increase Left array index
			}
			else
			{
				vector[k] = R[j];
				j++;
			}
			k++;
		}
		// Sorts by temp if val = 1
		// Sorts from highest to lowest unlike the rest, that sort from lowest to highest
		else if (val == 1)
		{
			if (L[i].temp >= R[j].temp) // >= instead of <= to change sorting direction
			{
				vector[k] = L[i];
				i++;
			}
			else
			{
				vector[k] = R[j];
				j++;
			}
			k++;
		}
		// Sorts by hum if val = 2
		else if (val == 2)
		{
			if (L[i].hum <= R[j].hum)
			{
				vector[k] = L[i];
				i++;
			}
			else
			{
				vector[k] = R[j];
				j++;
			}
			k++;
		}
		// sorts by mould index if val = 3
		else if (val == 3)
		{
			if (L[i].mould <= R[j].mould)
			{
				vector[k] = L[i];
				i++;
			}
			else
			{
				vector[k] = R[j];
				j++;
			}
			k++;
		}
	}

	// Adds rest if L[]
	while (i < n1)
	{
		vector[k] = L[i];
		i++;
		k++;
	}

	// Adds rest if R[]
	while (j < n2)
	{
		vector[k] = R[j];
		j++;
		k++;
	}
}

//<=========================<FUNCTION START>=========================>
// Sorts chosen vector through a Merge Sort, because I wanted to be able to sort the entire list
// within reasonable time and none of the O(n^2) methods were fast enough
// (vector, (l)eft index, (r)ight index, val (0 = date, 1 = temp, 2 = hum, 3 = mould))
void mergeSort(vector<dataStruct>& vector, int l, int r, int val)
{
	if (l < r)
	{
		// Creates middle point
		int m = l + (r - l) / 2;

		// Sort first and second halves 
		mergeSort(vector, l, m, val);
		mergeSort(vector, m + 1, r, val);

		merge(vector, l, m, r, val);
	}
}

//<=========================<FUNCTION START>=========================>
//Changes text color
//2: Green
//4: Red
//7: Default white
//8: Gray
//10: Brigth green
//12: Bright red
//15: Bright white
void Color(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

//<=========================<FUNCTION START>=========================>
// Outputs "Invalid choice" with red text
void invalidChoice()
{
	Color(12);
	cout << "Invalid choice" << endl << endl;
	Color(7);
}

//<=========================<FUNCTION START>=========================>
// Prints a colored numberbox
// menuColor(color, number in box)
void menuColor(int c, int n)
{
	Color(c);
	cout << "[" << n << "]";
	Color(15);
}

//<=========================<FUNCTION START>=========================>
// Reads user input and assigns it to the menuChoice variable
// Controls for failed inputs (eg. inputting char to int)
void menuInput()
{
	cin >> menuChoice;

	if (cin.fail())
	{
		cin.clear();
		cin.ignore(256, '\n');
		menuChoice = 100;
	}
}

//<=========================<FUNCTION START>=========================>
// Asks user to choose location
void inOut()
{
	menuColor(2, 0);
	cout << ": Outdoors" << endl;
	menuColor(2, 1);
	cout << ": Indoors" << endl;
	menuColor(4, 2);
	Color(4);
	cout << ": Entire list" << endl << endl;
	Color(7);
	cout << "Choice: ";

	menuInput();
	if (menuChoice > 2)
	{
		invalidChoice();
	}
}

//<=========================<FUNCTION START>=========================>
// Finds the entered date through a binary search algorithm. Faster than the linear one.
// (vector, first index, last index, searchInput, int value (0 = Entire List, 1 = In/Outdoors))
void binarySearch(vector<dataStruct>& vector, int l, int r, string searchInput, int val)
{
	int c = 0;

	if (r >= l) 
	{
		int mid = l + (r - l) / 2;

		// If the correct element is at mid
		// If val = 1, only print one value end exit
		if (vector[mid].date.substr(0, 10) == searchInput && val == 1)
		{
			cout << "Date found: ";
			cout << vector[mid].date << "    \t" << vector[mid].indoors << "   \t\t" << vector[mid].temp << "        \t" << vector[mid].hum << "      \t" << vector[mid].mould << endl;
		}

		// If the correct element is at mid
		// If val = 0, look for all mtching values and print them.
		// Useful for printing all temperatures in a single day.
		else if (vector[mid].date.substr(0, 10) == searchInput && val == 0)
		{
			// While the previous value is also correct, move mid to that index
			while (vector[mid-1].date.substr(0, 10) == searchInput)
			{
				mid--;
			}

			//When value at previous index is no longer also correct, start printing and moving mid until searchInput no longer matches
			while (vector[mid].date.substr(0, 10) == searchInput)
			{
				if (vector[mid].indoors == true)
				{
					cout << vector[mid].date << "    \t" << vector[mid].indoors << "   \t\t" << vector[mid].temp << "        \t" << vector[mid].hum << "      \t" << vector[mid].mould << endl;
					c++;
				}
				mid++;
			}
			val = 2;
			cout << c << " Elements" << endl;
		}

		// If element is smaller than mid, then it is in the smaller subarray
		if (vector[mid].date.substr(0, 10) > searchInput)
		{
			binarySearch(vector, l, mid - 1, searchInput, val);
		}

		// Otherwise is it in the larger subarray
		else
		{
			binarySearch(vector, mid + 1, r, searchInput, val);
		}
	}
}

//<=========================<FUNCTION START>=========================>
// Finds date for meteorological autumn or winter
// (vector, max temp)
void getDate(vector<dataStruct>& vector, int temp)
{
	bool found = false;

	for (size_t i = 0; i < vector.size(); i++) // Starts by looking at the first date in vector
	{
		int c = 1; // Counter

		// If that date was colder than temp (10 or 0)
		// then look att the 5 following dates, one by one, to see if they are too, in that case, increase counter
		if (vector[i].temp < temp) 
		{

			for (size_t j = i + 1; j < (i + 5); j++)
			{
				if (vector[j].temp < temp)
				{
					c++;
				}
			}
		}
		// If conditions were med 5 times in a row, counter will be 5
		// print out the last date in the chain and break the loop to avoid multiple results
		if (c == 5)
		{
			cout << "Meteorological date found: ";
			cout << vector[i+4].date << "    \t" << vector[i+4].indoors << "   \t\t" << vector[i+4].temp << "        \t" << vector[i+4].hum << "      \t" << vector[i+4].mould << endl;
			found = true;
			break;
		}
	}
	// If conditions were not met, type out error
	if (found == false)
	{
		Color(12);
		cout << "Error: Meteorological date not found" << endl;
		Color(7);
	}
}

//<=========================<FUNCTION START>=========================>
// Prints vector
// (vector, int (0 = types out 5 highest and 5 lowest, 1 = entire list))
void print(vector<dataStruct>& vector, int val)
{
	if (val == 0)
	{
		for (size_t i = 0; i < 5; i++)
		{
			cout << vector[i].date.substr(0, 10) << "    \t" << vector[i].indoors << "   \t\t" << vector[i].temp << "        \t" << vector[i].hum << "      \t" << vector[i].mould << endl;
		}

		cout  << "[...]" << endl;

		for (size_t i = vector.size() - 5; i < vector.size(); i++)
		{
			cout << vector[i].date.substr(0, 10) << "    \t" << vector[i].indoors << "   \t\t" << vector[i].temp << "        \t" << vector[i].hum << "      \t" << vector[i].mould << endl;
		}
	}
	else if (val == 1)
	{
		for (size_t i = 0; i < vector.size(); i++)
		{
			cout << vector[i].date << "    \t" << vector[i].indoors << "   \t\t" << vector[i].temp << "        \t" << vector[i].hum << "      \t" << vector[i].mould << endl;
		}
	}
	

	cout << "Date" << "    \t" << "1n/0utdoors" << "   \t" << "Temperature (C)" << "\t" << "Humidity (%)" << "  \t" << "Mould risk (0 - 6)" << endl << endl;
	cout << vector.size() << " Elements in Outdoors list" << endl << endl;
}

//<=========================<FUNCTION START>=========================>
// Uses sorting algorithm and prints time lapsed, created to prevent repetition as this code is used 11 times
// (vector, val to sort by (0 = date, 1 = temp, 2 = hum, 3 = mould), val name) 
void sort(vector<dataStruct>& vector, int val, string valName)
{
	auto t1 = Clock::now();
	cout << "Sorting... ";
	mergeSort(vector, 0, vector.size() - 1, val);
	Color(10);
	cout << "Sorted by " << valName << endl;
	Color(7);
	auto t2 = Clock::now();
	cout << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() * 0.001 << " Seconds" << endl;
}

//<=========================<FUNCTION START>=========================>
// Calculates amount of minutes the balcony door has been open each day
// Does not include humidity as a parameter in this version unfortunately since the humidity value proved often both unreliable and unchanged thoughout the day.
// (vector)
void balcony(vector<dataStruct>& vector)
{
	// Set begin to '254727' and enable all the 'cout' to get date of '2016-12-02' and see how it functions in the console
	// Set begin to '1' and set all the 'cout' to comments to get entire list
	const long int begin = 1;

	dataStruct previous = vector[begin -1]; // Continuously saves time and date from the previous index

	dataStruct start; // Saves time of the opening of the balcony door (Not really used in this version)
	dataStruct end; // Saves time of the closing of the balcony door (Not really used in this version)

	int result = 0;

	bool increasing = true;
	int c = 0;

	// Loop through vector
	for (int i = begin; i < vector.size(); i++)
	{
		// If date is indoors, otherwise nothing happens
		if (vector[i].indoors == true)
		{
			// If day is same as previous day
			if (previous.date.substr(0, 10) == vector[i].date.substr(0, 10))
			{
				// < TEMPERATURE GOING UP >
				// If temperature is increasing or steady after increasing (print out '+' to indicate increasing temp)
				if (vector[i].temp >= previous.temp && increasing == true)
				{
					//cout << previous.temp << "\t" << vector[i].temp << "\t+" << endl;
				}

				else if (vector[i].temp > previous.temp) // Set "end" value at the end of the decrease & compare it with start
				{
					if (vector[i].temp > end.temp + 0.1) // If temp is higher than (lowest temp + 0.1) to prevent detecting small fluctuations as closing of window
					{
						increasing = true;
					}

					//cout << previous.temp << "\t" << vector[i].temp << "\tEND SET, CALCULATE TO " << vector[i].date.substr(11, 8) << "\t" << result << " Minutes" << endl;
				}

				// < TEMPERATURE GOING DOWN >
				// If temperature is decreasing or steady after decreasing, add 1 minute to counter (print out sum of minutes passed with open door)
				// Needs to be 0.2c below starting point of measurement, to prevent detecting small fluctuations as open window
				if (vector[i].temp <= previous.temp && vector[i].temp < start.temp && increasing == false)
				{
					result += 1;
					end = previous; // Save ending point to
					//cout << previous.temp << "\t" << vector[i].temp << "\t" << result << endl;
					
				}
					
				else if (vector[i].temp < previous.temp) // Set "start" value at the beginning of the decrease, and "increasing" to false to enable the counting through previous if-statement
				{
					increasing = false;
					start = vector[i];
					//cout << previous.temp << "\t" << vector[i].temp << "\tSTART SET AT " << vector[i].date.substr(11, 8) << endl;
				}

				/*else if (increasing == false) // Has no real function but to print out lines between the initial decrease in temp and the second decrease that enables the counting. Troubleshooting.
				{
					cout << previous.temp << "\t" << vector[i].temp << "\t-" << endl;
				}*/
			}

			// < AT THE END OF EACH DAY >
			// Save results to tempDiff[c] and increase counter
			else if (c < tempDiff.size())
			{
				tempDiff[c].date = previous.date;
				tempDiff[c].hum = result;

				//cout << "================================================================================"  << endl;
				//cout << result << endl;

				result = 0; // Resets the total number of minutes lapsed 

				c++;
			}

			// Sets end to final log that day, and start to first log the next day, to prevent overlapping data from different days if the day ends with the "door open"
			if (!(previous.date.substr(0, 10) == vector[i].date.substr(0, 10)) && increasing == false)
			{
				end = previous; // Reset
				start = vector[i]; // reset
				//cout << previous.temp << "\t" << vector[i].temp << "\tEND SET, CALCULATE TO " << end.date.substr(11, 8) << endl;
			}

			previous = vector[i]; // Sets current day as previous before going to the next
		}
	}
}

//<==================================================><PROGRAM START><==================================================>
int main()
{
	auto t1 = Clock::now();

	getData(true); // Imports data with the getData() function
	getAverage(); // Creates two vectors with In/Outdoor averages
	getMould(dataVecIn); // Adds mould index to Indoor
	getMould(dataVecOut); // Adds mould index to Outdoor

	cout << " Complete." << endl;

	auto t2 = Clock::now();
	cout << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() * 0.001 << " Seconds"; // Types out seconds passed

	// Types out main menu options
	while (true)
	{
		// Strings used in function arguments in the menu switch
		string date = "date";
		string temperature = "temperature";
		string humidity = "humidity";
		string mouldRisk = "mould risk";

		string searchInput; // Search input for binary search
		
		string graphInput;

		Color(15);
		cout << "\n================< MAIN MENU >===================" << endl;
		menuColor(2, 0);
		cout << ": Search average temperature" << endl << "--------------------------------------------" << endl;
		menuColor(2, 1);
		cout << ": Sort by date (Low to High)" << endl;
		menuColor(2, 2);
		cout << ": Sort by temperature (High to Low)" << endl;
		menuColor(2, 3);
		cout << ": Sort by humidity (Low to High)" << endl;
		menuColor(2, 4);
		cout << ": Sort by least risk of mould development (Low to High)" << endl << "--------------------------------------------" << endl;
		menuColor(2, 5);
		cout << ": Print list" << endl << "--------------------------------------------" << endl;
		menuColor(2, 6);
		cout << ": Get date of meteorological autumn" << endl;
		menuColor(2, 7);
		cout << ": Get date of meteorological winter" << endl << "--------------------------------------------" << endl;
		menuColor(2, 8);
		cout << ": Sort by temperature difference & print" << endl;
		menuColor(2, 9);
		cout << ": Calculate how long the balcony door has been open, & sort by time" << endl << "================================================" << endl << endl;
		Color(7);
		cout << "Choice: ";

		menuInput();

		switch (menuChoice)
		{

			// Search
		case 0:
			Color(15);
			cout << "\n<Search>" << endl;
			Color(7);
			inOut();
			if (menuChoice == 0) // Outdoor list
			{
				mergeSort(dataVecOut, 0, dataVecOut.size() - 1, 0); // Sort list by date first to enable binary search
				cout << "Enter date numerically to search for in the following format 'YEAR-MONTH-DAY': " << endl;
				cin >> searchInput; // Let user input search date

				binarySearch(dataVecOut, 0, dataVecOut.size() - 1, searchInput, 1);
			}
			else if (menuChoice == 1) // Indoor list
			{
				mergeSort(dataVecIn, 0, dataVecIn.size() - 1, 0); // Sort list by date first to enable binary search
				cout << "Enter date numerically to search for in the following format 'YEAR-MONTH-DAY': " << endl;
				cin >> searchInput; // Let user input search date

				binarySearch(dataVecIn, 0, dataVecOut.size() - 1, searchInput, 1);
			}
			else if (menuChoice == 2) // Entire list
			{
				//Having some memory issues with sorting entire list back and fourth, so let's assume it is not scrambled by the user, and left in it's original state. Thus no mergeSort needed here
				cout << "Enter date numerically to search for in the following format 'YYYY-MM-DD': " << endl;
				cin >> searchInput; // Let user input search date

				binarySearch(dataVec, 0, dataVec.size() - 1, searchInput, 0);
			}
			break;

			// Sorts by date[1]
		case 1:
			Color(15);
			cout << "\n<Sort by date>" << endl;
			Color(7);
			inOut();
			if (menuChoice == 0) // Outdoor list
			{
				sort(dataVecOut, 0, date);
			}
			else if (menuChoice == 1) // Indoor list
			{
				sort(dataVecIn, 0, date);
			}
			else if (menuChoice == 2) // Entire list
			{
				sort(dataVec, 0, date);
			}
			break;

			// Sorts by Temperature[2]
		case 2:
			Color(15);
			cout << "\n<Sort by temperature>" << endl;
			Color(7);
			inOut();
			if (menuChoice == 0) // Outdoor list
			{
				sort(dataVecOut, 1, temperature);
			}
			else if (menuChoice == 1) // Indoor list
			{
				sort(dataVecIn, 1, temperature);
			}
			else if (menuChoice == 2) // Entire list
			{
				sort(dataVec, 1, temperature);
			}
			break;

			// Sorts by Humidity[3]
		case 3:
			Color(15);
			cout << "\n<Sort by humidity>" << endl;
			Color(7);
			inOut();
			if (menuChoice == 0) // Outdoor list
			{
				sort(dataVecOut, 2, humidity);
			}
			else if (menuChoice == 1) // Indoor list
			{
				sort(dataVecIn, 2, humidity);
			}
			else if (menuChoice == 2) // Entire list
			{
				sort(dataVec, 2, humidity);
			}
			break;

			// Sorts by Mould[4]
		case 4:
			Color(15);
			cout << "\n<Sort by risk of mould>" << endl;
			Color(7);
			inOut();
			if (menuChoice == 0) // Outdoor list
			{
				sort(dataVecOut, 3, mouldRisk);
			}
			else if (menuChoice == 1) // Indoor list
			{
				sort(dataVecIn, 3, mouldRisk);
			}
			else if (menuChoice == 2) // Mould index does not exist in raw list
			{
				Color(12);
				cout << "Error: Value does not exist in this list" << endl;
				Color(7);
			}
			break;

			// Prints out the list
		case 5:
			Color(15);
			cout << "\n<Print out>" << endl;
			Color(7);
			inOut();
			if (menuChoice == 0) // Print outdoor list
			{
				print(dataVecOut, 0);
			}
			else if (menuChoice == 1) // Print indoor list
			{
				print(dataVecIn, 0);
			}
			else if (menuChoice == 2) // Print entire list
			{
				print(dataVec, 1);
			}
			break;

			// Finds meteorological autumn
		case 6:
			mergeSort(dataVecOut, 0, dataVecOut.size() - 1, 0); // Sort list by date first, to avoid mixed values
			getDate(dataVecOut, 10);
			break;

			// Finds meteorological winter
		case 7:
			mergeSort(dataVecOut, 0, dataVecOut.size() - 1, 0); // Sort list by date first
			getDate(dataVecOut, 0);
			break;

			// Calculates and sorts by temperature difference
		case 8:
			mergeSort(dataVecOut, 0, dataVecOut.size() - 1, 0); // Sort list by date first
			mergeSort(dataVecIn, 0, dataVecIn.size() - 1, 0); // Sort list by date first

			for (int i = 0; i < tempDiff.size(); i++) // Loop through tempDiff Array
			{
				tempDiff[i].date = dataVecIn[i].date; // Add correct date
				tempDiff[i].temp = (dataVecIn[i].temp - dataVecOut[i].temp); // Add Temperature difference to tempDiff[i].temp variable

				// To prevent negative temp difference numbers
				if (tempDiff[i].temp < 0)
				{
					tempDiff[i].temp *= -1;
				}
			}
			mergeSort(tempDiff, 0, tempDiff.size() - 1, 1); // Sort the vector by temp value

			for (size_t i = 0; i < tempDiff.size(); i++) // Loop through and print dates and values
			{
				cout << tempDiff[i].date.substr(0, 10) << "\t" << tempDiff[i].temp << endl;
			}
			cout << "Date" << "    \t" << "Temperature difference, In/Outdoors (C)" << endl; // Clarifying what is being typed

			break;

			// Get balcony door open time
		case 9:
			mergeSort(tempDiff, 0, dataVecOut.size() - 1, 0); // Sort list by date first, to make sure dates match
			balcony(dataVec);
			mergeSort(tempDiff, 0, dataVecOut.size() - 1, 2); // Sort list by "hum" which in this case is used to store the number of minutes balcony has been open

			for (size_t i = 0; i < tempDiff.size(); i++)
			{
				cout << tempDiff[i].date.substr(0, 10) << "\t" << (int)tempDiff[i].hum / 60 << " hours and "<< (int)tempDiff[i].hum  % 60 << " minutes"<< endl;
			}
			cout << "Date" << "    \t" << "Time with balcony door left open" << endl; // Clarifying what is being typed

			break;

			// In case the user enters an invalid number
		default:
			invalidChoice();

			break;
		}
	}
}
