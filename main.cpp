/*
Process the Carts.csv file using regular expressions. 
The data in the "cart" contains a binary representation 
of the first 5 characters of the product name. Process
each cart by decrypting the barcode to its character 
representation, and then finding the product price in 
the list.
*/

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <bitset>
#include <stdlib.h>
#include <sstream>

using namespace std;

#define SIZE 500

struct keyValue {
	string key;
	string charecter;
};



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

			// n = narrow = 0, w = wide = 1
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

	Product(const Product& obj) {
		name = obj.name;
		barcode.barcode = obj.barcode.barcode;
		price.price = obj.price.price;
	}

	friend ostream& operator<<(ostream& output, const Product& D) {
		output << "Name: " << D.name << "   $" << D.price.price << endl << "Barcode: " << D.barcode.barcode;
		return output;
	}
};



class Cart {
private:
	string name;
	vector<Product> products;
public:
	Cart() {};
	Cart(vector<Product> p) { products = p; };
	vector<Product> getProducts() { return products; };
	void setProducts(vector<Product> p) { products = p; };
	string getName() { return name; };
	void setName(string n) { name = n; };
	void Push_Back(Product p) { products.push_back(p); };

	friend ostream& operator<<(ostream& output, const Cart& D) {
		output << D.name << endl;
		output << "=============================================================" << endl;
		for_each(D.products.begin(), D.products.end(), [&output](Product product) {output << product << endl; });
		output << "=============================================================" << endl;
		return output;
	}
};



string getName(string barcode) {
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
		vector<keyValue>::iterator found = find_if(d.dict.begin(), d.dict.end(), [&](keyValue k) {return temp == k.key; });
		if (found != d.dict.end())
			name += found->charecter;
	}
	return name;
}



int hashFunction(string key) {
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



vector<Product> readProductXML(string filename) {
	ifstream xmlFile;
	smatch text;
	string xmlText;
	xmlFile.open(filename);

	// regular expressions
	regex expressionProduct ("(<.*>)");
	regex expressionBarcode ("(<.*>)([0-9]*)(<.*>)");
	regex expressionPrice ("(<.*>)(.*)(<.*>)");

	vector<Product> DB(500);
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
		
		// make hash table
		int index = hashFunction(product.getBarcode().barcode);
		// collisions handling
		while (DB[index].getPrice().price != -1) {
			if (index >= SIZE) 
				index = 0;
			index++;
		}
		DB[index] = product;
	}
	return DB;
}



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



vector<Cart> readCartCSV(vector<Product> dataBase) {
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

	// read file
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
		for (auto x : barcode) {
			binary = HexToBin(x);
			string strBinary = binary;

			// get hash index
			int index = hashFunction(binary);
			// if found in index, push back to cart
			if (dataBase[index].getBarcode().barcode == binary)
				cart.Push_Back(dataBase[index]);
			// if not found
			else {
				Product product;
				// search from index to end
				vector<Product>::iterator foundProduct = find_if(dataBase.begin() + index, dataBase.end(),
					[&](Product product) {return product.getBarcode().barcode == binary; });
				// if found, sec product to found product
				if(foundProduct != dataBase.end())
					product = *foundProduct;
				// else search from beginning to index
				else {
					foundProduct = find_if(dataBase.begin(), dataBase.begin() + index,
					[&](Product product) {return product.getBarcode().barcode == binary; });
					if (foundProduct != dataBase.begin() + index)
						product = *foundProduct;
				}
				cart.Push_Back(product);
			}
		}
		carts.push_back(cart);
	}
	return carts;
}



int main() {
	vector<Cart> carts;
	vector<Product> dataBase;
	dataBase = readProductXML("ProductPrice.xml");
	carts = readCartCSV(dataBase);

	int option = 0;

	while (option != 3) {
		cout << "Enter a number to decide option: " << endl;
		cout << "1. Output specific cart" << endl
			 << "2. Output all carts" << endl
			 << "3. Exit Program" << endl << endl;
		cout << "enter your choice --> ";

		cin >> option;

		system("cls");
		if (option == 1) {
			cout << "Enter cart name in proper/case sensetive formatt (ex: Cart123): ";
			string name;
			cin >> name;
			vector<Cart>::iterator foundCart = find_if(carts.begin(), carts.end(), [&](Cart c) {return c.getName() == name; });
			if (foundCart != carts.end()) {
				system("cls");
				cout << *foundCart << endl;
			}
			else {
				cout << "...invalid name..." << endl;
			}
			system("pause");
		}
		else if (option == 2) {
			for_each(carts.begin(), carts.end(), [&](Cart cart) {
				cout << cart << endl;
			});
			system("pause");
		}
		else if (option == 3) {
			cout << "...Goodbye..." << endl;
			system("pause");
		}
		else {
			system("cls");
			cout << "...Invalid option... try capitalizing the first charecter ..." << endl;
			system("pause");
		}
		system("cls");
		
	}
	return 0;
}	


/*
example output:

Enter a number to decide option:
1. Output specific cart
2. Output all carts
3. Exit Program

enter your choice -->1

Enter cart name in its case sensetive formatt (ex: Cart123): Cart210

Cart210
=============================================================
Name: RASPB   $2.51
Barcode: 100000110100001001001000110001010010001001001000
Name: ALFRE   $19.88
Barcode: 100001001001000011001011000100000110100011000000
Name: KOHLR   $5.53
Barcode: 100000011100010010100001100001000011100000110000
Name: HALIB   $18.97
Barcode: 100001100100001001001000011001001100001001001000
Name: MINI    $2.99
Barcode: 101000010001001100000010011001001100011000100000
Name: SPAGH   $15.08
Barcode: 001000110001010010100001001000001101100001100000
Name: DISH    $14.09
Barcode: 000011001001001100001000110100001100011000100000
Name: COTTO   $19.36
Barcode: 101001000100010010000010110000010110100010010000
Name: STIR    $5.61
Barcode: 001000110000010110001001100100000110011000100000
Name: BASIL   $11.91
Barcode: 001001001100001001001000110001001100001000011000
Name: TOMAT   $2.45
Barcode: 000010110100010010101000010100001001000010110000
Name: SOURD   $10.75
Barcode: 001000110100010010110000001100000110000011001000
Name: VEGET   $2.23
Barcode: 011000001100011000000001101100011000000010110000
Name: VINEG   $5.11
Barcode: 011000001001001100000010011100011000000001101000
Name: CARRO   $4.87
Barcode: 101001000100001001100000110100000110100010010000
=============================================================

Press any key to continue . . .

*/
