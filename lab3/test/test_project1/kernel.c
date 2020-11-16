#define STAT_ADDR 0xffffffffbfe00005
#define DATA_ADDR 0xffffffffbfe00000
#define PSTR 0xffffffff8f0d5534
#define PCH 0xffffffff8f0d5570

char *hello = "\r\nHello OS\r\n";

void __attribute__((section(".entry_function"))) _start(void)
{
    // my code begin
    // printstr hello
    void (*pstr)(char *) = (void *)PSTR; // func_ptr "*pstr" -> printstr() function
    (*pstr)(hello);                      // use func_ptr "*pstr" to invoke printstr() function
                                         // the parameter is "char *hello"
    // echo printch
    void (*pch)(char) = (void *)PCH; // func_ptr "*pch" -> printch() function
    while (1)
    {
        int newc = *((char *)STAT_ADDR) & 0x01; // check state
        char input = *((char *)DATA_ADDR);      // fetch data
        if (newc)
            (*pch)(input); // use func_ptr "*pstr" to invoke printstr() function, "char input" as parameter
    }
    // my code end
}
