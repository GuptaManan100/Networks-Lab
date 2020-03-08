#include <stdio.h>
#include <stdlib.h>
#include "product.h"

struct product fill_prd(char str[]){	// This function converts details of a product from string format to product format
	struct product prd; // For storing details of a product in structure format
	prd.is_error = 0;	// setting the error bit to 0 if product exists in the database
	char upc_str[100], name_str[100], price_str[100]; // Individual strings for parameters of product
	int pos_str = 0;	// Iterator for str
	int pos_val = 0;

	// printf("%s\n", str);

	while(str[pos_str] != ' '){		// Reading upc code of product
		upc_str[pos_val] = str[pos_str];
		pos_str++;
		pos_val++;
	}
	upc_str[pos_val] = '\0';
	pos_str++;
	// printf("%s\n", upc_str);
	prd.upc = atoi(upc_str); 	// Converting sting to int

	pos_val = 0;
	while(str[pos_str] != ' '){
		prd.name[pos_val] = str[pos_str];
		pos_str++;
		pos_val++;
	}
	prd.name[pos_val] = '\0';
	pos_str++;
	// printf("%s\n", prd.name);

	pos_val = 0;
	while(str[pos_str] != ' ' && str[pos_str] != '\0'){ 	// Converting sting to int
		price_str[pos_val] = str[pos_str];
		pos_str++;
		pos_val++;
	}
	price_str[pos_val] = '\0';
	pos_str++;
	// printf("%s\n", price_str);	
	prd.price = atoi(price_str);

	return prd;
}

struct product do_transaction(int upc){

	FILE *fptr;

	// Whole databse is stored in Database.txt 
	// Database is stored in the following format upc followed by name of product followed by it's price.

	fptr = fopen("Database.txt", "r+"); // Opening database file

	char str[100];	// For storing details of a product in string format
	struct product prd; // For storing details of a product in structure format

	int i;
	char ch;

	while(ch != EOF){ // Scanning the whole database

		i = 0;
		while(ch != '\n'){	// Scanning a single product
			ch = fgetc(fptr);
			str[i] = ch;
			i++;
		}
		ch = fgetc(fptr);
		str[i] = '\0';

		prd = fill_prd(str); // Converting details of a product from string format to product format

		if(prd.upc == upc) return prd; // Product found

	}

	// There is no product in the database with given upc

	fclose(fptr);
	prd.is_error = 1; // setting the error bit to 1 if product doesn't exist in the database
	return prd;	

}