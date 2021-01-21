#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mysql.h"

#pragma comment(lib, "libmysql.lib")

#define CRASH 1721

const char* host = "localhost";
const char* user = "root";
const char* pw = "Ksb0465*";
const char* db = "project2";


typedef struct _code {
	int type;
	int subtype;
	int number;
	char character[15];
	int year;
	int month;
}code;

code prompt(int flag);

int main(void) {

	MYSQL* connection = NULL;
	MYSQL conn;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;

	FILE* crfile = fopen("20170364_1.txt", "r");
	FILE* udfile = fopen("20170364_2.txt", "r");
	FILE* bill = NULL; char billname[50];

	if (mysql_init(&conn) == NULL)
		printf("mysql_init() error!");

	// connection to MYSQL server
	connection = mysql_real_connect(&conn, host, user, pw, db, 3306, (const char*)NULL, 0);
	if (connection == NULL)
	{
		printf("%d ERROR : %s\n", mysql_errno(&conn), mysql_error(&conn));
		return 1;
	}

	else
	{
		printf("Connection Succeed\n");

		if (mysql_select_db(&conn, db))
		{
			printf("%d ERROR : %s\n", mysql_errno(&conn), mysql_error(&conn));
			return 1;
		}

		char query[300] = { '\0' };			// array to save query
		char largequery[800] = { '\0' };	// big array to save large auery (ex. create table query)
		int state = 0;
		int exitflag = 0, flag = 0;			// exitflag: variable to check input '0. QUIT' and end program, flag: variable to check previous type selected by user
		char* ptr;
		char month_buf[3]; char year_buf[5];
		int Vcount = 0;						// variable for making 3 types of bills
		char contractbuf[30], cautionbuf[10];

		printf("\n***********************************************\n");
		printf("** Sogang Package Delivery Management System **\n");
		printf("***********************************************\n\n");


		// create & insert database
		printf("Creating Tables..\n");
		while (fgets(largequery, 800, crfile) != NULL) {
			ptr = strtok(largequery, "\n");
			mysql_query(connection, ptr);
		}

		code mycode = prompt(flag);
		while (1) {
			flag = 0;

			// Generate Query
			switch (mycode.type) {
			case 1: // TYPE I
				flag = 1;
				if (mycode.subtype == 1) { // TYPE I-1
					sprintf(query, "select customer_id from timetable, package where transportation = 'truck' and transp_id = %d and arrive = 0 and timetable.package_id = package.package_id", CRASH);
				}
				else if (mycode.subtype == 2) { // TYPE I-2
					sprintf(query, "select name from timetable, package where transportation = 'truck' and transp_id = %d and arrive = 0 and timetable.package_id = package.package_id", CRASH);

				}
				else if (mycode.subtype == 3) { // TYPE I-3
					sprintf(query, "select package_id from timetable where transportation = 'truck' and transp_id = %d and arrive = 1 and datetime = (select max(datetime) from timetable where transportation = 'truck' and transp_id = %d and arrive = 1)", CRASH, CRASH);
				}
				else if (mycode.subtype == 0) {
					flag = 0;
					break;
				}
				else { // error
					printf("Please choose between 1, 2, 3 !!!\n\n");
					query[0] = '\0';
				}
				break;
			case 2:	// TYPE II
				flag = 2;
				if (mycode.number == 0) {
					flag = 0; break;
				}
				sprintf(query, "with sent as (select customer_id, count(customer_id) as each_sent from history where year = %d group by customer_id) select customer_id from sent where each_sent = (select max(each_sent) from sent)", mycode.number);
				break;
			case 3:	// TYPE III
				flag = 3;
				if (mycode.number == 0) {
					flag = 0; break;
				}
				sprintf(query, "select customer_id from bill where year = %d and owed = (select max(owed) from bill where year = %d)", mycode.number, mycode.number);
				break;
			case 4: // TYPE IV
				flag = 4;
				strcpy(query, "with timetable_done as (select package_id, datetime from timetable where arrive = 1) select package_id from package natural join timetable_done where promissed < datetime");
				break;
			case 5: // TYPE V
				flag = 5;
				if (mycode.year == 0) {
					flag = 0; break;
				}
				if (Vcount == 3) Vcount = 0;
				switch (++Vcount) {
				case 1:
					// simple bill
					sprintf(query, "select customer_id, name, address, owed from bill natural join customer where customer_id = '%s' and year = %d and month = %d", mycode.character, mycode.year, mycode.month);
					break;
				case 2:
					// bill listingcharges by type of service
					sprintf(query, "select package.customer_id, package_type, sum(price) as total_price, weight, timeliness, address from package natural join history where year = %d and month = %d and customer_id = '%s' group by customer_id, package_type", mycode.year, mycode.month, mycode.character);
					break;
				case 3:
					// itemized billing
					sprintf(query, "select * from history natural join package where customer_id = '%s' and year = %d and month = %d", mycode.character, mycode.year, mycode.month);
					break;
				}
				break;
			case 0:	// QUIT
				exitflag = 1;
				break;
			default:
				flag = 0;
				printf("Please choose between 0 ~ 5 !!!\n\n");
				break;
			}

			if (query[0] != '\0') {
				state = mysql_query(connection, query);
				if (state == 0)
				{
					// when the given type input is 5 and subtype not 0, make .txt file to write bill
					if (mycode.type == 5 && mycode.year != 0) {
						strcpy(billname, "bill_");
						strcat(billname, mycode.character);
						strcat(billname, "_");
						_itoa(mycode.year, year_buf, 10);
						strcat(billname, year_buf);
						strcat(billname, "_");
						_itoa(mycode.month, month_buf, 10);
						strcat(billname, month_buf);
						char tmp[5]; sprintf(tmp, "(%d)", Vcount);
						strcat(billname, tmp);
						strcat(billname, ".txt\0");
						bill = fopen(billname, "w");
						if (Vcount == 1) {	// Simple bill
							printf("(1) Generating Simple Bill..\n");
							fprintf(bill, "<SIMIPLE BILL>\n| ID | NAME | ADDRESS | OWED |\n");
						}
						else if (Vcount == 2) {	// Type specified bill
							printf("(2) Generating Type Specified Bill..\n");
							fprintf(bill, "<TYPE SPECIFIED BILL>\n| ID | TYPE | TOTAL PRICE | WEIGHT | TIMELINESS | ADDRESS |\n");
						}
						else if (Vcount == 3) {	// Itemized bill
							printf("(3) Generating Itemized Bill..\n");
							fprintf(bill, "<ITEMIZED BILL>\n| ID | TYPE | TOTAL PRICE | TIMELINESS | ADDRESS | NAME | PHONE NUMBER |\n");
						}
					}

					sql_result = mysql_store_result(connection);
					while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
					{
						// print out query result
						switch (mycode.type) {
						case 1:
							if (mycode.subtype == 1)
								printf("Customer ID : %s\n", sql_row[0]);
							else if (mycode.subtype == 2)
								printf("Customer Name : %s\n", sql_row[0]);
							else if (mycode.subtype == 3)
								printf("Package ID : %s\n", sql_row[0]);
							break;
						case 3:
							printf("Customer ID : %s\n", sql_row[0]);
							break;
						case 2:
							printf("Customer ID : %s\n", sql_row[0]);
							break;
						case 4:
							printf("Package ID : %s\n", sql_row[0]);
							break;
						case 5:
							if (mycode.year == 0) break;
							if (Vcount == 1) fprintf(bill, "| %s | %s | %s | %s |\n", sql_row[0], sql_row[1], sql_row[2], sql_row[3]);
							else if (Vcount == 2) {
								// when frequent customer (history.price == NULL)
								if (sql_row[2] == NULL) strcpy(contractbuf, "Frequent Client(Contract)\0");
								else strcpy(contractbuf, sql_row[2]);
								fprintf(bill, "| %s | %s | %s | %s | %s | %s |\n", sql_row[0], sql_row[1], contractbuf, sql_row[3], sql_row[4], sql_row[5]);
							}
							else if (Vcount == 3) {
								// when frequent customer (history.price == NULL)
								if (sql_row[6] == NULL) strcpy(contractbuf, "Frequent Client(Contract)\0");
								else strcpy(contractbuf, sql_row[6]);
								// when no caution code (package.caution_code == NULL)
								if (sql_row[11] == NULL) strcpy(cautionbuf, "None\0");
								else strcpy(cautionbuf, sql_row[11]);
								fprintf(bill, "| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n", sql_row[0], sql_row[1], sql_row[2], sql_row[3], sql_row[4], sql_row[5], contractbuf, sql_row[7], sql_row[8], sql_row[9], sql_row[10], cautionbuf, sql_row[12], sql_row[13], sql_row[14]);
							}
							break;
						case 0:
							break;
						}
					}

					mysql_free_result(sql_result);

					// if current type input is 5, close file
					if (flag == 5) {
						fclose(bill);
						printf("Generation Completed\n");
					}
				}
			}
			// when input == 0 (QUIT)
			if (exitflag) {
				// drop table 
				while (fgets(query, 300, udfile) != NULL) {
					ptr = strtok(query, "\n");
					mysql_query(connection, ptr);
				}
				printf("Tables dropped!!!\n");
				break;
			}
			// when input = 5, but not all three types of bill have been generated yet
			if (flag == 5 && Vcount < 3) {
				// continue => mycode.type still saves 5
				// and Vcount will be incread by 1 due to line 173/177/181
				continue;
			}
			// read new input
			else mycode = prompt(flag);
		}

		printf("HAVE A GOOD DAY :) 조교님 한 학기 고생 많으셨습니다 감사합니다 !!\n");

		mysql_close(connection);
	}

	return 0;
}

//function to print out prompt
code prompt(int flag) {
	code mycode;
	mycode.type = -1;  mycode.subtype = -1; mycode.character[0] = "\0"; mycode.month = -1; mycode.year = -1; mycode.number = -1;

	// when previous input type was 0(escape from subquery) or 4(which does not require any extra input)
	// print out main prompt and get new type input
	if (flag == 0 || flag == 4) {
		printf("\n------- SELECT QUERY TYPES -------\n");
		printf("\t1. TYPE I\n");
		printf("\t2. TYPE II\n");
		printf("\t3. TYPE III\n");
		printf("\t4. TYPE IV\n");
		printf("\t5. TYPE V\n");
		printf("\t0. QUIT\n");
		printf("----------------------------------\n");

		printf("Which type of query? ");
		fscanf(stdin, "%d", &mycode.type);
	}
	// if else, keep type value same as previous type value
	// until the user types 0 for subtype value
	else mycode.type = flag;

	switch (mycode.type) {
	case 0:
		break;
	case 1:
		// get input for the truck ID until ID == CRASH(1721)
		while (1) {
			if (flag == 1) break;
			printf("\n---- TYPE I ----\n");
			printf("Input the number of truck : ");
			fscanf(stdin, "%d", &mycode.number);
			if (mycode.number == 0 || mycode.number == CRASH) break;
			else printf("Truck %d is not destroyed.\n", mycode.number);
		}
		// if 0 for subtype, escape subtype
		if (mycode.number == 0) {
			flag = 0;  break;
		}
		printf("\n----- Subtypes in TYPE I -----\n");
		printf("\t1. TYPE I-1.\n");
		printf("\t2. TYPE I-2.\n");
		printf("\t3. TYPE I-3.\n");
		printf("------------------------------\n");
		printf("Which type of query? ");
		fscanf(stdin, "%d", &mycode.subtype);
		switch (mycode.subtype) {
		case 1:
			printf("\n** Find all customers who had a package on the truck at the time of the crash **\n");
			break;
		case 2:
			printf("\n** Find all recipients who had a package on that truck at the time of the crash **\n");
			break;
		case 3:
			printf("\n** Find the last successful delivery by that truck prior to the crash **\n");
			break;
		case 0: break;
		}
		break;
	case 2:
		printf("\n---- TYPE II ----\n");
		printf("\n** Find the customer who has shipped the most packages in certain year **\n");
		printf("Which Year? : ");
		fscanf(stdin, "%d", &mycode.number);
		if (mycode.number == 0) flag = 0;
		break;
	case 3:
		printf("\n---- TYPE III ----\n");
		printf("\n** Find the customer who has spent the most money on shipping in the past certain year **\n");
		printf("Which Year? : ");
		fscanf(stdin, "%d", &mycode.number);
		break;
	case 4:
		printf("\n---- TYPE IV ----\n");
		printf("\n** Find those packages that were not delivered within the promised time **\n");
		break;
	case 5:
		printf("\n---- TYPE V ----\n");
		printf("\n** Generate the bill for each customer for the past certain month **\n");
		printf("Which year(YYYY)? ");
		scanf("%d", &mycode.year);
		// if 0 for subtype, escape subtype
		if (mycode.year == 0) break;
		printf("Which month(MM)? ");
		scanf("%d", &mycode.month);
		printf("Customer ID : ");
		scanf("%s", mycode.character);
		break;
	}

	return mycode;
}




