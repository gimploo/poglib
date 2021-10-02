#include "../../str.h"

int main(void)
{
    str_t input_text = new_str("Hello World, suck a dick\n");

    // This array stores all the words
    str_t *list_of_word[1024] = {0};
    size_t list_word_count = 0;

    char buffer[1024] = {0};
    for (int i = 0, buff_index = 0; i < input_text.len; i++)
    {
        buffer[buff_index++] = input_text.buf[i];

        if (input_text.buf[i] == ' ' || input_text.buf[i] == '\n') {

            list_of_word[list_word_count++] = new_pstr(buffer);

            // Reset the buffer to take in the next word
            buff_index = 0;
            memset(buffer, 0, sizeof(buffer));
        }
    }

    for (int i = 0; i < list_word_count; i++)
    {
        printf(STR_FMT "\n", STR_ARG(list_of_word[i]));
    }

    for (int i = 0; i < list_word_count; i++)
    {
        pstr_free(list_of_word[i]);
    }


    return 0;
}
