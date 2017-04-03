#include <stdio.h>
#include <Windows.h>
#pragma warning(disable:4996)

/*
Bookstore2
 - Codegate 2015 본선문제
 - 버그 클래스
	- UAF
	- UAF를 통한 OOB READ
 - 보호기법
	- ASLR		: On
	- DEP		: On
	- SAFE SEH	: On
	- GS		: On
 - Description
	- Advanced Bookstore Management Application
	  More Secure, More Stable... But.. Really?
 - Flag
 - Write-Up
	- UAF 버그를 이용한 Info Leak
	- UAF 버그를 이용한 EIP Control
*/

void print_description(char *description);
void print_freeshipping(char *bookname);

char temp[300];

struct booklist // process heap
{
	int num; // automatic increasement
	int type; // 1 = book / 2 = ebook
	int available; // 0 = unavailable / 1 = available
	LPVOID ptr_to_struct;
};

struct book_info // isolated heap 40
{
	void(*func_print_description)(char *str) = print_description;
	int num;
	int is_book;
	char* bookname;
	int price;// controllable
	int stoke;// controllable
	int available;// controllable
	char* description;
	int is_free_shipping;
};

struct ebook_info // isolated heap 36
{
	int num;
	int is_book;
	char* bookname;
	int cookie2 = 0x0a000c0d;
	int available;
	int pdf = 1;
	int cookie = 0x0a000c0d;
	int cookie1 = 0x0a000c0d;
	int price; // controllable
	int stoke;// controllable
	int max_download;// controllable
	char* description;

};

HANDLE Isolatedheap = NULL;
HANDLE Processheap = NULL;
const int max_book = 1024;
int current_count = 0;
booklist* books;

void special_gift()
{
	if (0)
	{
		__asm{
			xchg eax, esp;
			pop ecx;
			pop eax;
			ret;
		}
	}
}

void free_wrap_i(LPVOID target)
{
	HeapFree(Isolatedheap, 8, target);
}

void free_wrap_p(LPVOID target)
{
	HeapFree(Processheap, 8, target);
}

void print_description(char *description)
{
	printf("[*] Description : %s\n", description);
}

void print_freeshipping(char *bookname)
{
	printf("[*] Free Shipping for this product (%s)\n", bookname);
}

LPVOID virtualalloc_wrap(LPVOID addr, int size, int protect)
{
	LPVOID test = VirtualAlloc(addr, size, 0x1000, protect);
	return test;
}

void virtualprotect_wrap(LPVOID target, int size, int a)
{
	DWORD b;
	DWORD *asd = &b;
	VirtualProtect(target, sizeof(target), a, asd);
}

void initialize()
{
	Processheap = GetProcessHeap();
	Isolatedheap = HeapCreate(0, 0, 0); // create private heap

	LPVOID temp = virtualalloc_wrap(NULL, sizeof(booklist) * max_book, PAGE_READWRITE);
	books = (booklist *)temp;
	memset(books, 0, sizeof(booklist) * max_book);

	special_gift();

}

void printinformation()
{
	puts("============== [ADVANCED] =============");
	puts("== Bookstore Management Application ==");
	puts("1. Add new item"); //
	puts("2. Modify the item"); //
	puts("3. Remove the item");//
	puts("4. View item's information."); //
	puts("5. Show item list"); //
	puts("0. Exit"); //
	puts("=======================================");
}

void fgets_wrap(char *target, int size)
{
	fflush(stdin);
	fgets(target, size, stdin);
}

int login()
{
	LPVOID id = HeapAlloc(Processheap, 8, 20);
	LPVOID pw = HeapAlloc(Processheap, 8, 20);

	char c_id[] = "helloadmin";
	char c_pw[] = "Iulover!@#";

	printf("id : ", id);
	fgets_wrap((char*)id, 15);
	
	printf("pw : ", pw);
	fgets_wrap((char*)pw, 15);
	
	if (strncmp(c_id, (char *)id, 10) == 0)
	{
		if (strncmp(c_pw, (char *)pw, 10) == 0)
		{
			puts("Correct!");
			return 1;
		}
		
	}
	return 0;
}



void modify_bookname(int i)
{

	char* bookname = (char *)HeapAlloc(Processheap, 8, 20);
	puts("Bookname?");
	fgets(bookname, 20, stdin);

	int type = books[i].type;
	if (type == 1) // book
	{
		book_info *book = (book_info *)books[i].ptr_to_struct;
		book->bookname = temp;
	}
	else if (type == 2) // ebook
	{
		ebook_info *ebook = (ebook_info*)books[i].ptr_to_struct;
		ebook->bookname = temp;
	}
	else
	{
		puts("Something went wrong..");
	}
}
void modify_description(int i)
{
	char* description = (char *)HeapAlloc(Processheap, 8, 300);
	puts("Description?");
	fgets(temp, 300, stdin);

	int type = books[i].type;
	if (type == 1) // book
	{
		book_info *book = (book_info *)books[i].ptr_to_struct;
		book->description = temp;
	}
	else if (type == 2) // ebook
	{
		ebook_info *ebook = (ebook_info*)books[i].ptr_to_struct;
		ebook->description = temp;
	}
	else
	{
		puts("Something went wrong..");
	}
}
void modify_Properties(int i) // price and stoke
{
	int type = books[i].type;

	unsigned int stoke; // vuln 
	puts("Stoke?");
	scanf("%d", &stoke);

	if (stoke <= 0)
	{
		puts("Price cannot be less or equal zero.");
		stoke = 1;
	}

	unsigned int price; // vuln type conversion
	puts("Price?");
	scanf("%d", &price);
	if (price <= 0)
	{
		puts("Price cannot be less or equal zero.");
		price = 1;
	}

	if (type == 1) // book
	{
		book_info *book = (book_info *)books[i].ptr_to_struct;

		int freeship;
		puts("Free Shipping?");
		scanf("%d", &freeship);

		if (freeship != 1 && freeship != 0)
		{
			puts("Wrong.. Free Shipping for this product.");
			freeship = 1;
		}

		book->stoke = stoke;
		book->price = price;
		book->is_free_shipping = freeship;

	}
	else if (type == 2) // ebook
	{

		int maxdownload;
		puts("Max Download?");
		scanf("%d", &maxdownload);
	

		if (maxdownload <= 0)
		{
			puts("Max downalod cannot be less or equal zero.");
			maxdownload = 1;
		}

		ebook_info *ebook = (ebook_info*)books[i].ptr_to_struct;
		ebook->stoke = stoke;
		ebook->price = price;
		ebook->max_download = maxdownload;

	}
	else
	{
		puts("Something went wrong..");
	}
}
void modify_Available(int i)
{
	int type = books[i].type;
	int available;
	puts("Available?");
	scanf("%d", &available);

	if (available != 1 && available != 0)
	{
		puts("Wrong..");
	}
	else
	{
		books[i].available = available;
		if (type == 1) // book
		{
			book_info *book = (book_info *)books[i].ptr_to_struct;
			

			if (book->stoke > 0)
			{
				puts("Wrong..");
			}
			else
			{
				book->available = available;
			}
			
		}
		else if (type == 2) // ebook
		{
			ebook_info *ebook = (ebook_info*)books[i].ptr_to_struct;
			if (ebook->stoke > 0)
			{
				puts("Wrong..");
			}
			else
			{
				ebook->available = available;
			}

		}
		else
		{
			puts("Something went wrong..");
		}
	}

}

void modify_print_menu()
{
	puts("1. Modify bookname");
	puts("2. Modify description");
	puts("3. Modify Properties"); // stoke and price
	puts("4. Modify Available");
	puts("0. back to main menu!");
}

void modify_item(int i)
{
	// removed item 체크
	if (books[i].available == 0)
	{
		puts("Wrong..");
	}
	else
	{
		while (1)
		{
			unsigned int c = 0;
			modify_print_menu();
			printf("Select Menu : ");
			scanf("%d", &c);

			fflush(stdin);

			if (c == 1)
			{
				modify_bookname(i);
			}
			else if (c == 2)
			{
				modify_description(i);
			}
			else if (c == 3)
			{
				modify_Properties(i);
			}
			else if (c == 4)
			{
				modify_Available(i);
			}
			else if (c == 0)
			{
				puts("Back to main menu.");
				break;
			}
			else
			{
				puts("Wrong.. Back to main menu.");
				break;
			}
		}
	}
}

void view_item(int i)
{
	printf("[*] Index : %d\n", i);
	int type = books[i].type;

	if (type == 1)
	{
		puts("[*] Type : Book");
		book_info *book = (book_info *)books[i].ptr_to_struct;
		char *bookname = book->bookname;
		int available = book->available;
		int price = book->price;
		int stoke = book->stoke;
		printf("[*] Name : %s\n", bookname);
		printf("[*] Available : %d\n", available);
		char *description = book->description;
		char *description1 = description;
		{
			book->func_print_description(description1);
		}

		printf("[*] Price : %d\n", book->price);
		printf("[*] Stoke : %d\n", stoke);

		if (book->is_free_shipping == 1)
		{
			printf("[*] Free shipping for this product (%s)\n", book->bookname);

		}


	}
	else if (type == 2)
	{
		puts("[*] Type : EBook");
		ebook_info *ebook = (ebook_info*)books[i].ptr_to_struct;
		printf("[*] Name : %s\n", ebook->bookname);
		char *description = ebook->bookname;
		char *description1 = description;
		{
			printf("[*] Description : %s", description1);
		}
		printf("[*] Available : %d\n", ebook->available);
		printf("[*] Price : %d\n", ebook->price);
		printf("[*] Stoke : %d\n", ebook->stoke);
		printf("[*] Max Download : %d\n", ebook->max_download);
	}
	else
	{
		puts("Something went wrong..");
	}
	puts("====================================================================");
}

int remove_the_item(int i)
{
	
	int type = books[i].type;

	if (type == 1)
	{
		book_info *book = (book_info *)books[i].ptr_to_struct;
		if (book->available == 0 && books[i].available == 0)
		{
			free_wrap_i(book);
			if (book->stoke == 0)
			{
				books[i].ptr_to_struct = NULL;
				puts("Removed");
				return 1;
			}
		}
	}
	else if (type == 2)
	{
		ebook_info *ebook = (ebook_info*)books[i].ptr_to_struct;
		if (ebook->available == 0 && books[i].available == 0)
		{
			free_wrap_i(ebook);
			if (ebook->stoke == 0)
			{
				books[i].ptr_to_struct = NULL;
				puts("Removed");
				return 1;
			}
		}
	}
	puts("Wrong..");
	return 0;
}

int add_new_item()
{
	int select;

	puts("Add New Item!\n1. Book\n2. E-Book");
	scanf("%d", &select);

	fflush(stdin);

	if (select == 1)
	{

		LPVOID temp = HeapAlloc(Isolatedheap, 8, sizeof(book_info));
		book_info *book = (book_info *)temp;
		books[current_count].available = 1;
		books[current_count].num = current_count;
		books[current_count].type = 1;
		books[current_count].ptr_to_struct = temp;

		LPVOID bookname_ = HeapAlloc(Processheap, 8, 20);
		char* bookname = (char *)bookname_;
		puts("Bookname?");
		fgets(bookname, 20, stdin);
		
		char* description = (char *)HeapAlloc(Processheap, 8, 300);
		puts("Description?");
		fgets(description, 300, stdin);

		int stoke;
		puts("Stoke?");
		scanf("%d", &stoke);

		if (stoke <= 0)
		{
			puts("Stoke cannot be less or equal zero.");
			stoke = 1;
		}
		
		int price;
		puts("Price?");
		scanf("%d", &price);

		if (price <= 0)
		{
			puts("Price cannot be less or equal zero.");
			price = 1;
		}

		int freeship;
		puts("Free Shipping?");
		scanf("%d", &freeship);

		if (freeship != 1 && freeship != 0)
		{
			puts("Wrong.. Free Shipping for this product.");
			freeship = 1;
		}

		book->num = current_count;
		book->bookname = bookname;
		book->description = description;
		book->price = price;
		book->stoke = stoke;
		book->is_free_shipping = freeship;
		book->func_print_description = print_description;

	}
	else if (select == 2)
	{
		LPVOID temp = HeapAlloc(Isolatedheap, 8, sizeof(ebook_info));
		ebook_info *ebook = (ebook_info *)temp;

		books[current_count].available = 1;
		books[current_count].num = current_count;
		books[current_count].type = 2;
		books[current_count].ptr_to_struct = temp;

		char* bookname = (char *)HeapAlloc(Processheap, 8, 20);
		puts("Bookname?");
		fgets(bookname, 20, stdin);

		char* description = (char *)HeapAlloc(Processheap, 8, 300);
		puts("Description?");
		fgets(description, 300, stdin);

		int stoke;
		puts("Stoke?");
		scanf("%d", &stoke);

		if (stoke <= 0)
		{
			puts("Stoke cannot be less or equal zero.");
			stoke = 1;
		}

		int price;
		puts("Price?");
		scanf("%d", &price);
		if (price <= 0)
		{
			puts("Price cannot be less or equal zero.");
			price = 1;
		}

		int maxdownload;
		puts("Max Download?");
		scanf("%d", &maxdownload);
		if (maxdownload <= 0)
		{
			puts("Max downlaod cannot be less or equal zero.");
			maxdownload = 1;
		}
		ebook->num = current_count;
		ebook->bookname = bookname;
		ebook->description = description;
		ebook->price = price;
		ebook->stoke = stoke;
		ebook->max_download = maxdownload;
		
	}
	else
	{
		puts("Wrong.. Return to main menu.");
		return 0;
	}
	current_count++;
	return 1;
}

void show_all_items()
{
	puts("====================================================================");
	puts("=== Item List");
	puts("====================================================================");
	for (int i = 0; i < current_count; i++)
	{
		if (books[i].ptr_to_struct == NULL)
		{
			continue;
		}
		printf("[*] Index : %d\n", i);
		int type = books[i].type;
		
		if (type == 1)
		{
			puts("[*] Type : Book");
			book_info *book = (book_info *)books[i].ptr_to_struct;
			printf("[*] Name : %s\n", book->bookname);
		}
		else if (type == 2)
		{
			puts("[*] Type : EBook");
			ebook_info *ebook = (ebook_info*)books[i].ptr_to_struct;
			printf("[*] Name : %s\n", ebook->bookname);			
		}
		else
		{
			puts("Something went wrong..");
		}
		puts("====================================================================");
	}

}

int main()
{
	setvbuf(__iob_func() + 1, 0, 4, 0);
	initialize();
	int login_f = login();
	if (login_f != 1)
	{
		puts("Access Denied....");
		exit(0);
	}
	while (1)
	{
		printinformation();
		printf("> ");
		unsigned int c = 0;
		scanf("%d", &c);
		if (c == 1)
		{
			if (current_count >= max_book)
			{
				puts("Wrong...");
			}
			else
			{
				add_new_item();
			}

		}
		else if (c == 2)
		{
			if (current_count == 0)
			{
				puts("Wrong..");
			}
			else
			{
				puts("Select Index");
				scanf("%d", &c);

				if (c < current_count && books[c].ptr_to_struct != NULL)
				{
					modify_item(c);
				}
				else
				{
					puts("Wrong..");
				}
			}
		}
		else if (c == 3)
		{
			if (current_count == 0)
			{
				puts("Wrong..");
			}
			else
			{
				puts("Select Index");
				scanf("%d", &c);

				if (c < current_count && books[c].ptr_to_struct != NULL)
				{
					remove_the_item(c);
				}
				else
				{
					puts("Wrong..");
				}
			}

		}
		else if (c == 4)
		{
			if (current_count == 0)
			{
				puts("Wrong..");
			}
			else
			{
				puts("Select Index");
				scanf("%d", &c);

				if (c < current_count && books[c].ptr_to_struct != NULL)
				{
					view_item(c);
				}
				else
				{
					puts("Wrong..");
				}
			}

		}
		else if (c == 5) // finish
		{
			if (current_count == 0)
			{
				puts("Wrong..");
			}
			else
			{
				show_all_items();
			}
		}
		else if (c == 0) // finish
		{
			puts("Good bye :)");
			break;
		}
		else // finish
		{
			puts("Wrong...");
			exit(0);
		}

	}
	HeapDestroy(Isolatedheap);
	PDWORD test = 0;
	return 0;
}
