/*
Program Description:
- Reads in Product data from file and save it into hash based database
- Reads cart data and barcodes contained in each cart from file
- Creates lanes with a queue of carts in each lane
- Lanes process cart at front of queue by reading barcodes using
  barcode to access product name and price from database. Cart 
  is then popped from font of queue
- Multiple lanes process carts simealtaneosly through threading
- mutex commands used so one lane can access database at a time
- outputs products as they are processed
*/

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <stdlib.h>
#include <sstream>
#include <queue>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>

using namespace std;

#define SIZE 500
#define NUMLANES 5
#define PRODUCTFILE "ProductPrice.xml"

mutex globalMutex;

string HexToBin(string);
int hashFunction(string);
string getName(string);



/****************************************************************
hold Key Value pair
*****************************************************************/
struct keyValue {
	string key;
	string charecter;
};




/****************************************************************
hold dictionary for converting between barcode and charecter
read from dictionary.txt
based on 3of9 barcode symbology.
*****************************************************************/
class Dictionary {
public:
	vector<keyValue> dict;
	Dictionary() {
		regex expression("'(.+)'");
		fstream inFile;
		inFile.open("dictionary.txt");
		while (!inFile.eof()) {
			keyValue kv;
			string fileText;
			smatch text;
			getline(inFile, fileText);
			regex_search(fileText, text, expression);
			string charecter = text.str(1);
			getline(inFile, fileText);
			string key = "";

			// in file: n = narrow = 0, w = wide = 1
			for (int i = 0; i < fileText.size(); i++) {
				if (fileText[i] == 'n') {
					key = key + '0';
				}
				if (fileText[i] == 'w') {
					key = key + '1';
				}
			}
			kv.charecter = charecter;
			kv.key = key;
			dict.push_back(kv);
		}
	}
};




struct Barcode {
	string barcode;
};



struct Price {
	float price;
};



/****************************************************************
represents product from store
data members: name, product, price
*****************************************************************/
class Product {
private:
	string name;
	Barcode barcode;
	Price price;
public:
	Product(string n = "", string s = "NULL", float f = -1)  
	{ name = n; barcode.barcode = s; price.price = f;}
	void setBarcode(string s) { barcode.barcode = s; };
	Barcode getBarcode() { return barcode; };
	void setPrice(float s) { price.price = s; };
	Price getPrice() { return price; };

	// copy constructor
	Product(const Product& obj) {
		name = obj.name;
		barcode.barcode = obj.barcode.barcode;
		price.price = obj.price.price;
	}

	// output operator
	friend ostream& operator<<(ostream& output, const Product& D) {
		output << "Name: " << D.name << "   $" << D.price.price << endl << "Barcode: " 
			   << D.barcode.barcode;
		return output;
	}
};




/****************************************************************
container holding products or barcode of products
*****************************************************************/
class Cart {
private:
	string name;
	vector<Product> products;
	vector<string> barcodes;
	bool isBarcode;
public:
	Cart(bool b = true) { isBarcode = b; };
	Cart(vector<string> p) { barcodes = p; isBarcode = true;  };
	Cart(vector<Product> p) { products = p; isBarcode = false; };
	vector<Product> getProducts() { return products; };
	vector<string> getBarcodes() { return barcodes; };
	void setProducts(vector<Product> p) { products = p; };
	string getName() { return name; };
	void setName(string n) { name = n; };
	void Push_Back(Product p) { products.push_back(p); };
	void Push_Back(string p) { barcodes.push_back(p); };

	// output operator
	friend ostream& operator<<(ostream& output, const Cart& D) {
		output << D.name << endl;
		output << "=============================================================" << endl;
		if (D.isBarcode)
			for_each(D.barcodes.begin(), D.barcodes.end(), [&output](string product) {output << product << endl;});
		else
			for_each(D.products.begin(), D.products.end(), [&output](Product product) {output << product << endl; });
		output << "=============================================================" << endl;
		return output;
	}
};

// read cart function header
vector<Cart> readCartCSV();




/****************************************************************
hash based container holding all products read from file
*****************************************************************/
class DataBase {
private:
	vector<Product> DB;
public:
	DataBase() {};
	DataBase(string);
	DataBase(vector<Product> p) { DB = p; };
	void setDB(vector<Product> p) {DB = p;};
	vector<Product> getDB() { return DB; };
	Product findProduct(string);
};


DataBase::DataBase(string file) 
{
	ifstream xmlFile;
	smatch text;
	string xmlText;
	xmlFile.open(file);

	// regular expressions
	regex expressionProduct("(<.*>)");
	regex expressionBarcode("(<.*>)([0-9]*)(<.*>)");
	regex expressionPrice("(<.*>)(.*)(<.*>)");

	vector<Product> products(500);
	string barcode, price;

	int count = 0, collisions = 0;
	// parse xml file
	getline(xmlFile, xmlText);
	while (!xmlFile.eof()) {
		regex_search(xmlText, text, expressionProduct);
		if (text.str(1) == "<Product>") {
			getline(xmlFile, xmlText);
			regex_search(xmlText, text, expressionBarcode);
		}
		if (text.str(1) == "<Barcode>") {
			barcode = text.str(2);
			getline(xmlFile, xmlText);
			regex_search(xmlText, text, expressionPrice);
		}
		if (text.str(1) == "<Price>") {
			price = text.str(2);
			getline(xmlFile, xmlText);
			regex_search(xmlText, text, expressionProduct);
		}
		getline(xmlFile, xmlText);
		string name = getName(barcode);
		Product product(name, barcode, stof(price));

		// produce hash index
		int index = hashFunction(product.getBarcode().barcode);
		// collisions handling
		while (products[index].getPrice().price != -1) {
			if (index >= SIZE)
				index = 0;
			index++;
		}
		products[index] = product;
	}
	DB = products;
}


Product DataBase::findProduct(string barcode) 
{
	// convert hex barcode to binary barcode
	string binary = HexToBin(barcode);
	// get hash index
	int index = hashFunction(binary);
	// if found in index return
	if (DB[index].getBarcode().barcode == binary) {
		cout << DB[index] << endl;
		return DB[index];
	}
	// if not found
	else {
		// search from index to end
		vector<Product>::iterator foundProduct = find_if(DB.begin() + index, DB.end(),
			[&](Product product) {return product.getBarcode().barcode == binary; });
		// if found, sec product to found product
		if (foundProduct != DB.end() || foundProduct->getPrice().price != -1)
		{
			cout << *foundProduct << endl;
			return *foundProduct;
		}
		// else search from beginning to index
		else {
			foundProduct = find_if(DB.begin(), DB.begin() + index,
				[&](Product product) {return product.getBarcode().barcode == binary; });
			if (foundProduct != DB.begin() + index)
			{
				cout << *foundProduct << endl;
				return *foundProduct;
			}
		}
	}
}




/****************************************************************
process queue of carts
reads barcodes from carts and searches database for product and
price of product based on barcode
*****************************************************************/
class Lane {
private:
	int laneNumber;
	DataBase DB;
	queue<Cart> line;
public:
	Lane() { DB = DataBase (PRODUCTFILE); };
	Lane(queue<Cart> c, DataBase d) { line = c;  DB.setDB(d.getDB()); };
	void setCart(queue<Cart> c) { line = c; };
	queue<Cart> getCarts() { return line; };
	vector<Cart> processLine();
	void enQueue(Cart c) { line.push(c); };
	void setLaneNumber(int x) { laneNumber = x; }
};


vector<Cart> Lane::processLine() 
{
	vector<Cart> processedCarts;
	while (!line.empty()) {
		Cart processedCart(false);
		// get products from cart in front of the line
		vector<string> unprocessedProducts = line.front().getBarcodes();
		// process each item in the cart
		for_each(unprocessedProducts.begin(), unprocessedProducts.end(), [&](string x) {
			this_thread::sleep_for(std::chrono::seconds(1));
			// one lane accesses database at a time
			globalMutex.lock();		
			cout << "Lane: " << laneNumber << endl; 
			processedCart.Push_Back(DB.findProduct(x)); 
			cout << endl;
			globalMutex.unlock();
		});
		processedCarts.push_back(processedCart);
		line.pop();		// next cart moves to front of the line
	}
	return processedCarts;
}




/****************************************************************
						*Main*
*****************************************************************/
int main() 
{
	vector<Lane> lanes(NUMLANES);
	vector<Cart> carts = readCartCSV();

	// give each lane a lane number
	for (int i = 0; i < lanes.size(); i++)
		lanes[i].setLaneNumber(i);
	// place carts into a lane queue
	for (int i = 0, j = 0; i < carts.size(); i++, j++) {
		if (j == NUMLANES)
			j = 0;
		lanes[j].enQueue(carts[i]);
	}

	// each lane has its own thread to process carts
	deque<thread> qThread;
	for_each(lanes.begin(), lanes.end(), [&](Lane x) {qThread.emplace_back(&Lane::processLine, x); });

	cout << "Transaction Record..." << endl;
	for (auto& x : qThread)
		x.join();

	return 0;
}	


/*
output:

Transaction Record...
Lane: 4
Name: SAUSA   $8.28
Barcode: 001000110100001001110000001001000110100001001000

Lane: 2
Name: TRAIL   $19.18
Barcode: 000010110100000110100001001001001100001000011000

Lane: 1
Name: RASPB   $2.51
Barcode: 100000110100001001001000110001010010001001001000

Lane: 3
Name: CRANB   $3.95
Barcode: 101001000100000110100001001000010011001001001000

Lane: 0
Name: LIMES   $20.52
Barcode: 001000011001001100101000010100011000001000110000

Lane: 4
Name: VACUU   $14.78
Barcode: 011000001100001001101001000110000001110000001000

Lane: 2
Name: GRAPE   $16.96
Barcode: 000001101100000110100001001001010010100011000000


... [continued] ...

*/



/****************************************************************
read file with info of cart and barcodes in cart
*****************************************************************/
vector<Cart> readCartCSV()
{
	vector<Cart> carts;
	string text;
	string binary;

	ifstream csvFile;
	csvFile.open("Carts.csv");

	// open file
	if (!csvFile.is_open()) {
		cout << "file not open" << endl;
		system("pause");
	}

	while (!csvFile.eof()) {
		Cart cart;
		vector<string> barcode;
		getline(csvFile, text);
		cart.setName(text);
		getline(csvFile, text);

		// regex parsing
		regex barcodeExpression("[^, ]+");
		regex pattern(barcodeExpression);

		sregex_token_iterator end;  // default value is npos regex pattern(split); int count = 0; 
		for (sregex_token_iterator pos(text.begin(), text.end(), pattern); pos != end; ++pos)
			if ((*pos).length() > 0)
				if ((static_cast<string>(*pos))[0] != 0x20)
					barcode.push_back(*pos);
		for_each(barcode.begin(), barcode.end(), [&](string x) {cart.Push_Back(x); });
		carts.push_back(cart);
	}
	return carts;
}



/****************************************************************
returns string based on 3of9barcode using Dictionary Object for
conversions from barcode to charecter
*****************************************************************/
string getName(string barcode)
{
	Dictionary d;
	string name = "";
	int i = 0;
	// take each 9 bits and compare to dictionary 
	while (i < barcode.length()) {
		int j = i + 9;
		string temp = "";
		while (i < j && barcode[i] != NULL) {
			temp += barcode[i];
			i++;
		}
		vector<keyValue>::iterator found = find_if(d.dict.begin(), d.dict.end(),
			[&](keyValue k) {return temp == k.key; });
		if (found != d.dict.end())
			name += found->charecter;
	}
	return name;
}


/****************************************************************
hash function using barcode as key
*****************************************************************/
int hashFunction(string key)
{
	string str = getName(key);
	int index = 0;
	for_each(str.begin(), str.end(), [&](char c) {index += (int)c; });
	int ones = index % 10;
	int tens = (index / 10) % 10;
	int hundreds = index / 100;
	index = ones * tens * hundreds;
	if (index > SIZE)
		index = index - SIZE;
	return index;
}



/****************************************************************
convert Hex symbol to Binary
*****************************************************************/
string HexToBin(string hexdec)
{
	long int i = 0;
	//vector<string> binaryCode;
	string binary;
	string binaryCode = "";
	while (hexdec[i]) {
		switch (hexdec[i]) {
		case '0':
			binary = "0000";
			break;
		case '1':
			binary = "0001";
			break;
		case '2':
			binary = "0010";
			break;
		case '3':
			binary = "0011";
			break;
		case '4':
			binary = "0100";
			break;
		case '5':
			binary = "0101";
			break;
		case '6':
			binary = "0110";
			break;
		case '7':
			binary = "0111";
			break;
		case '8':
			binary = "1000";
			break;
		case '9':
			binary = "1001";
			break;
		case 'A':
		case 'a':
			binary = "1010";
			break;
		case 'B':
		case 'b':
			binary = "1011";
			break;
		case 'C':
		case 'c':
			binary = "1100";
			break;
		case 'D':
		case 'd':
			binary = "1101";
			break;
		case 'E':
		case 'e':
			binary = "1110";
			break;
		case 'F':
		case 'f':
			binary = "1111";
			break;
		default:
			cout << "\nInvalid hexadecimal digit " << hexdec[i];
		}
		binaryCode = binaryCode + binary;
		i++;
	}
	return binaryCode;
}
