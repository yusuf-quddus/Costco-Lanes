
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <regex>
#include <bitset>
#include <stdlib.h>

using namespace std;


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
	Product(string, string, float);
	//Product(string, float);
	void setBarcode(string);
	void setPrice(float);
	Barcode getBarcode();
	Price getPrice();

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

Product::Product(string n = "", string s = "NULL", float f = 0) {
	name = n;
	barcode.barcode = s;
	price.price = f;
}

/*Product::Product(string s = "NULL", float f = 0 ) {
	barcode.barcode = s;
	price.price = f;
}*/

void Product::setBarcode(string s) {
	barcode.barcode = s;
}

void Product::setPrice(float s) {
	price.price = s;
}

Barcode Product::getBarcode() {
	return barcode;
}

Price Product::getPrice() {
	return price;
}




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

/* 
int hashFunction(Product p) {
	bitset<48> barcode(string(p.getBarcode().barcode));
	int numOn = barcode.count();
	string sbarcode = barcode.to_string();
	int index = numOn;

	while (index >= SIZE) {
		index = index - (SIZE / 2);
	}
	return index;
}
*/


string getName(string barcode) {
	Dictionary d;
	string name = "";
	int i = 0;
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
//	cout << name << endl;
	return name;
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

	vector<Product> DB;
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
		DB.push_back(product);
	}
	return DB;
}

string HexToBin(string hexdec)
{
	long int i = 0;
	//vector<string> binaryCode;
	string binary;
	string binaryCode = "";
	//	for_each(hexdec.begin(), hexdec.end(), [](shared_ptr<char> i){bitset<4> binary(*i); binaryCode.push_back(binary.to_string()); });
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
		cout << "\nInvalid hexadecimal digit "
			<< hexdec[i];
}
binaryCode = binaryCode + binary;
i++;
	}
	//return binaryCode;
	return binaryCode;
}

vector<Cart> readCartCSV(vector<Product> dataBase) {
	string text;
	//smatch barcode;

	vector<Cart> carts;
	string binary;
	
	
	regex barcodeExpression("[^, ]+");

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
		regex pattern(barcodeExpression);
		// regex_search(text, barcode, barcodeExpression);


		sregex_token_iterator end;  // default value is npos regex pattern(split); int count = 0; 
		for (sregex_token_iterator pos(text.begin(), text.end(), pattern); pos != end; ++pos) 
		{ 
			if ((*pos).length() > 0) { 
				if ((static_cast<string>(*pos))[0] != 0x20) { 
					barcode.push_back(*pos);
				} 
			}
		}

		for (auto x : barcode) {
			binary = HexToBin(x);
			string strBinary = "";

			for (auto y : binary) {
				strBinary = strBinary + y;
			}

			vector<Product>::iterator foundProduct = find_if(dataBase.begin(), dataBase.end(), 
				[&](Product product) {return product.getBarcode().barcode == strBinary; });

			if (foundProduct != dataBase.end()) {
				Product product = *foundProduct;
				//cout << product.getBarcode().barcode << endl;
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

	for_each(carts.begin(), carts.end(), [&](Cart cart) {
		cout << cart << endl;
	});

	return 0;
}	

// put this working code to github
// code option to print 1 cart
// turn binary converter to bitset
// create hash table/hash function



/*
string HexToBin(string hexdec)
{


		long int i = 0;
	string binaryCode = "";
	while (hexdec[i]) {
		char bit = hexdec[i] - 'A' + 10;
		bitset<4> binary(bit);
		//bitset<4> binary(hexdec[i]);
		binaryCode = binaryCode + binary.to_string();
		//cout << binaryCode << endl;
		i++;
	}
	//return binaryCode;
	return binaryCode;
}
*/