#include <stdio.h>

/*
Concept : Bookstore Management Application
x86 binary

ASLR : ON
PIE : ON
NX : ON

Bug : Uninitialized Memory Reference.

*/

typedef struct book_info
{
        int num;
        int is_ebook; // 0 : book | 1: ebook
        char bookname[20];
        int price;
        int stoke;
        void (* func_print_description)();
        int is_free_shipping; // 0 : no | 1 : yes | book only
        void (* func_print_freeshipping)();
        //////////////////////////////////////////////////////
        // ebook struct
        int max_download;
        //////////////////////////////////////////////////////
        int avaliable;
        char description[300];

}book;

int num = 0;

const int MAX = 20;

void readfile_wrap(char *a)
{
        FILE* f;
        char data[1024] = {0};

        f = fopen(a, "rb");
        if(f)
        {
                fread(data, 1024, 1, f);
                printf("%s", data);
        }
        fclose(f);
}

void print_description(char* description)
{
	printf("Description : %s\n", description);
}

void print_freeshipping(char bookname)
{
	printf("Free Shipping for this product\n - name : '%s'.", bookname);
}

book add_new_item_book()
{
	char description[300];
	char bookname[20];
	int is_ebook;
	int stock;
	int price;
	int max_download;

	memset(bookname, 0, sizeof(bookname));
	puts("Input Bookname : ");

	read(0, bookname, 20);
	puts("Input Description : ");

	memset(description, 0, sizeof(description));
	read(0, description, 300);

	book b;

	memset(b.bookname, 0, sizeof(b.bookname));
	memset(b.description, 0, sizeof(b.description));

	strncpy(b.description, description, 299);
	strncpy(b.bookname, bookname, 19);

	b.description[sizeof(description)-1] = '\0';
	b.bookname[sizeof(bookname)-1] = '\0';

	b.num = num; // 

	while(1)
	{
		puts("Type (0 : Book, 1 : EBook)");
		scanf("%d", &is_ebook);
		
		if(is_ebook == 1 || is_ebook == 0)
		{
			b.is_ebook = is_ebook;
			break;
		}
		else
		{
			puts("Wrong value. And Set Default value.");
			b.is_ebook = 0;
			break;

		}
	}


	puts("Default Stock : 1024");
	b.stoke = 1024;

	puts("Default Price : 2048");
	b.price = 2048;

	b.func_print_description = &print_description;

	if(b.is_ebook == 1)
	{
		puts("Input Max Download : ");
		scanf("%d", &price);
		b.max_download = max_download;
	}
	else
	{
		b.is_free_shipping = 1; // force
		if(b.is_free_shipping == 1)
		{
			b.func_print_freeshipping = &print_freeshipping;
		}
	}
	num++;
	b.avaliable = 1;
	return b;

}

book modify_the_book(int is_ebook, int num)
{
	int new_stock;
	int new_price;
	int new_freeship;
	int new_maxdownload;
	int new_avaliable;

	book b;
	b.num = num;//

	b.is_ebook = is_ebook;

	puts("Input Stock : ");
	scanf("%d", &new_stock);
	b.stoke = new_stock;
	puts("Input Price : ");
	scanf("%d", &new_price);
	b.price = new_price;

	b.func_print_description = &print_description;

	while(1)
	{
		puts("Set Free Shipping? (1 : free shipping | 0 : not) ");
		scanf("%d", &new_freeship);
		if(new_freeship == 1 || new_freeship == 0)
		{
			b.is_free_shipping = new_freeship; // can choice
			break;
		}
		else
		{
			puts("Wrong Value.. and set free-shipping");
			b.is_free_shipping = 1;
			break;
		}
	}


	if(b.is_free_shipping == 1)
	{
		b.func_print_freeshipping = &print_freeshipping; // uninit issue 
	}
	if(b.is_ebook == 1)
	{
		puts("Set max download :");
		scanf("%d", &new_maxdownload);
		b.max_download = new_maxdownload;
	}
	puts("Set Avaliable :");
	scanf("%d", &new_avaliable);
	if(new_avaliable != 0 && new_avaliable !=1)
	{
		puts("wrong value and set avaliable.");
		new_avaliable = 1;
	}
	b.avaliable = new_avaliable;

	modify_book_name(&b);
	modify_book_description(&b);
	return b;

}

void modify_book_name(book *b)
{
	puts("Input new bookname");
	char bookname[500];
	memset(bookname, 0, 500);
	read(0, bookname, 500);

	memset(b->bookname, 0, 20);
	strncpy(b->bookname, bookname, 20);
	puts("Complete!");
}

void modify_book_description(book *b)
{
	puts("Input new description");
	char description[3000];
	memset(description, 0, 3000);
	read(0, description, 3000);

	memset(b->description, 0, 300);
	strncpy(b->description, description, 300);
	puts("Complete!");

}

void modify_book_shipping_option(book *b)
{
	int new_freeship = 0;
	puts("free shipping? (0. not | 1. free shipping)");
	scanf("%d", &new_freeship);

	if((new_freeship == 0) || (new_freeship == 1))
	{
		puts("changed");
		b->is_free_shipping = new_freeship;
	}
	else
	{
		puts("Wrong.");
	}
}

void modify_book_avaliable(book *b)
{
	int new_avaliable = 0;
	puts("Avaliable? (0. stop the sale, 1. Avaliable)");

	scanf("%d", &new_avaliable);

	if((new_avaliable == 0) || (new_avaliable == 1))
	{
		puts("changed");
		b->avaliable = new_avaliable;
	}
	else
	{
		puts("Wrong.");
	}

}

void print_modify_hello()
{
	puts("1. Modify bookname"); 
	puts("2. Modify description"); // stack spray with 0x41
	puts("3. Modify information"); // 
	puts("4. Modify Shipping option");
	puts("5. Modify Avaliable");
	puts("0. back to main menu!");
}

book modify_book(book *b)
{
	while(1)
	{
		print_modify_hello();
		int c;
		scanf("%d", &c);
		if(c == 1)
		{
			modify_book_name(b);
		}
		else if(c == 2)
		{
			modify_book_description(b);
		}
		else if(c == 3)
		{
			book newbook;
			newbook= modify_the_book(b->is_ebook, b->num);
			b = &newbook;			
		}
		else if(c == 4)
		{
			modify_book_shipping_option(b);
		}
		else if(c == 5)
		{
			modify_book_avaliable(b);
		}
		else if(c == 0)
		{
			printf("exit");
			break;
		}
		else
		{
			puts("wrong!");
			break;
		}
	}
	return *b;
}

int systemwrap(int a, int b)
{
	system("ls -al");
}


int preview_the_item(book b)
{
	puts("====================================================================");
	printf("No : %d\n", b.num);
	if(b.is_ebook == 1)
	{
		puts("Type : Ebook\n");
	}
	else
	{
		puts("Type : book\n");
	}
	
	printf("Stoke : %d\n", b.stoke);
	printf("Price : %d\n", b.price);
	if(b.avaliable == 1)
	{
		puts("Status : Avaliable\n");
	}
	else
	{
		puts("Status : Unavaliable\n");
	}
	b.func_print_description(b.description);

	if(b.is_ebook == 1)
	{
		printf("Max download : %d\n", b.max_download);
	}
	else
	{
		if(b.is_free_shipping)
		{
			b.func_print_freeshipping(b.bookname);
		}
	}

	puts("====================================================================");
	return 1;
}

int login()
{
	readfile_wrap("/root/codegate/famous_saying.txt");
	char correct_id[11] = "helloadmin";
	char correct_pw[12] = "iulover!@#$";
	char id[20];
	char pw[20];
	puts("== Bookstore Management Application ==");
	puts("== Login");
	printf("== Input Your ID : ");
	read(0, id, 20);
	// get id
	printf("== Input Your PASSWORD : ");
	read(0, pw, 20);
	// get password
	puts("=======================================\n");

	if(strncmp(correct_id, id, 10) == 0)
	{
		if(strncmp(correct_pw, pw, 11) == 0)
		{
			return 1;
		}
	}

	
	return 0;
}

void exit_wrap()
{
	exit(0);
}

void helloworld()
{
	puts("== Bookstore Management Application ==");
	puts("1. Add new item");
	puts("2. Modify the item");
	puts("3. View item's information.");
	puts("4. Show item list");
	puts("0. Exit");
	puts("=======================================");
}


int main()
{
	setvbuf(stdout, 0, 2, 0);
	book books[MAX];
	if(login() == 1) // patch
	{
		while(1)
		{
			helloworld();
			printf("> ");
			int c;
			scanf("%d", &c);
			if(c == 1) // add new item
			{
				if(num > MAX)
				{
					printf("Too many items.");
				}
				else
				{
					// add new item
					books[num] = add_new_item_book();
					puts("Complete");
				}
			}
			else if(c == 2) // modify the item
			{
				if(num != 0)
				{
					int select = 0;
					printf("Input No : ");
					scanf("%d", &select);
					if((select < MAX) && (select < num) && (select > -1))
					{
						books[select] = modify_book(&books[select]);
					}
				}
				else
				{
					puts("Error");
				}
				// modify the item
			}
			else if(c == 3) // view item's info 
			{
				if(num != 0)
				{
					// preview the item
					int select = 0;
					printf("Input No : ");
					scanf("%d", &select);
					if((select < MAX) && (select < num) && (select > -1))
					{
						preview_the_item(books[select]);
					}
					else
					{
						puts("error");
					}
				}
				else
				{
					puts("Error");
				}
			}
			else if(c == 4) // show all item 
			{

				if(num != 0)
				{
					puts("====================================================================");
					for(int i =0; i<num; i++)
					{
						printf("No : %d | type (1: ebook, 0: book): %d | name : %s | description : %s | price : %d | stock : %d\n", books[i].num, books[i].is_ebook, books[i].bookname, books[i].description, books[i].price, books[i].stoke); // infoleak point
						puts("");
					}
					puts("====================================================================");
				}
				else
				{
					puts("Error");
				}
			}
			else if(c == 0)
			{
				exit_wrap();
			}
			else
			{
				puts("Wrong Number.");
				break;
			}

		}
	}
	else
	{
		puts("Permission Denied. This incident will be reported. :( ");
	}
}

