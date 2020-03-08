// This structure stores all the necessary details for a particular product

struct product{
	int upc;
	char name[50];
	int price;
	int is_error; // is_error is 1 if there is an error otherwise 0
};