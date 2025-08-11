#include <stdint.h>

//vga text mode video memory address
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

//sector size and where data is stored
#define SECTOR_SIZE 512
#define SECTOR 10

//reserves area in kernel's data area
static char file_buffer[SECTOR_SIZE];

//start curosr at 0,0
//unsigned bc cant be negative
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

//to show to screen
static uint16_t* const vga_buffer = (uint16_t*)VGA_ADDRESS;

//creates 20 sector ram disk
static char ram_disk[SECTOR_SIZE * 20];

//so start runs
void kernel_main();

void _start() {
    kernel_main();
}

//to read from harware port 
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ( "inb %1, %0" 
                          : "=a"(ret) //stores output in ret
                          : "Nd"(port) );
    return ret;
}

// //writes to hardware port
// static inline void outb(uint16_t port, uint8_t val) {
//     __asm__ __volatile__ ( "outb %0, %1"
//                           :
//                           : "a"(val), "Nd"(port) );
// }

//char 2 screen
void print_char(char c, uint8_t color) {
    if (c == '\n') {  //newline logic
        cursor_x = 0;
        cursor_y++;
        return;
    }
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x; //where to put char 
    vga_buffer[pos] = (uint16_t)c | (uint16_t)(color << 8); //put char and color into screen buffer at pos
    cursor_x++; //so it writes next over
    if (cursor_x >= VGA_WIDTH) { //new line if too long
        cursor_x = 0;
        cursor_y++;
    }
}

//QOL
void print(const char* str) {
    while (*str) {
        print_char(*str++, 0x0F);
    }
}

//get char from keyboard
char get_char() {
    uint8_t scancode;
    char c = 0;
    const char keymap[128] = {
        0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
        'a','s','d','f','g','h','j','k','l',';','\'','`',  0,'\\',
        'z','x','c','v','b','n','m',',','.','/',  0,'*', 0,' ',
    };

    //til key press
    while (1) {
        if ((inb(0x64) & 1) != 0) { //check for data
            scancode = inb(0x60); //read key
            if (!(scancode & 0x80)) { 
                c = keymap[scancode]; //acc read char
                break;
            }
        }
    }
    return c;
}

//QOL
void input_line(char* buffer, int max_len) {
    int i = 0;
    char c;
    while (i < max_len - 1) {
        c = get_char(); //wait for key
        if (c == '\r' || c == '\n') break; //stop if enter
        buffer[i++] = c; //safe 
        print_char(c, 0x0F); //show
    }
    buffer[i] = '\0'; //end w null
}

//read byte from disk for length of sector
void read_sector(int sector, char* buffer) {
    for (int i = 0; i < SECTOR_SIZE; i++) {
        buffer[i] = ram_disk[sector * SECTOR_SIZE + i]; 
    }
}

//read_sector backwards
void write_sector(int sector, char* buffer) {
    for (int i = 0; i < SECTOR_SIZE; i++) {
        ram_disk[sector * SECTOR_SIZE + i] = buffer[i];
    }
}

//fill screen blank then reset cursor
void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = ((uint16_t)0x07 << 8) | ' '; 
    }
    cursor_x = 0;
    cursor_y = 0;
}

void kernel_main() {
    while(1) { //so it resets
        clear_screen();
        print("BasicOS\n");
        print("1.) open file\n");
        print("2.) new file\n");
        print("3.) edit file\n");
        print("choose an option...\n");

        char choice = get_char();
        print_char(choice, 0x0F);
        print("\n");

        char* file_buffer = file_buffer;  // just use the static array

        if (choice == '1') {
            read_sector(SECTOR, file_buffer);
            print("\nFile Contents:\n");
            print("press any key to continue...\n");
            print(file_buffer);
            get_char();
        } else if (choice == '2') {
            print("\nenter text (max 500 characters):\n");
            input_line(file_buffer, 500);
            write_sector(SECTOR, file_buffer);
            print("\nfile saved.\n");
        } else if (choice == '3') {
            print("\noverwriting file. \nenter new text:\n");
            input_line(file_buffer, 500);
            write_sector(SECTOR, file_buffer);
            print("\nfile edited.\n");
        } else {
            print("\ninvalid option.\n");
        }

        print("\ndone.\n"); //should never run but good measure
    }
}
