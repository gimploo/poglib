#include <poglib/file.h>

void test01(void)
{
    file_t *file = file_init("test.txt", "w");
    char buffer[32] = "This is a line\n";
    char readbuf[32] = {0};
    file_writebytes(
            file, 
            buffer, sizeof(buffer));

    file_readbytes(file, readbuf, sizeof(readbuf));
    printf("%s", readbuf);

    file_destroy(file);
}

void test02(void)
{
    char buffer[4][32] = {
        "line01\n",
        "line02\n",
        "line03\n",
        "line04\n"
    };

    file("tmp.txt", "w") {
        for (int i = 0; i < 4; i++)
            file_writebytes(
                    file, 
                    buffer[i], sizeof(buffer[i]));
    }
}

int main(void)
{
    dbg_init();
    test02();
    dbg_destroy();
    return 0;
}
