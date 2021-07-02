#ifndef CSV_H
#define CSV_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// my string header file
#include "str.h"

// my debug header file
#ifdef DEBUG
#include "dbg/dbg.h"
extern dbg_t debug;
#endif

#define KB 1024

//TODO: Clean up this mess

typedef struct {

    str_t **buffer;
    size_t field_count;

} Row;

// Holds meta data on positions of each entry in buffer 
// and the count of total entries
typedef struct {

    size_t *entry_pos_list;
    size_t entry_count;

} EntryList;

typedef struct {

    char **header;
    size_t header_count;

} Header;

typedef struct {

    Header header;
    EntryList entries;
    str_t *buffer;

} CSV;


static size_t csv_get_total_entries(str_t *a)
{
    assert(a);

    //NOTE: fixed dont change!
    
    size_t count = 0;
    for (size_t i = 0; i < a->size; i++)
        if (a->buf[i] == '\n')
            count++;
    return count;
}


static size_t csv_get_header_count(str_t *buffer)
{
    assert(buffer);
    size_t count = 0;
    for (size_t i = 0; buffer->buf[i] != '\n'; i++) 
        if (buffer->buf[i] == ',' )
            count++;
    return count + 1;
}

static EntryList csv_entry_list_init(str_t *a)
{
    assert(a);

    size_t entry_count = csv_get_total_entries(a);
    printf("entry count = %li\n", entry_count);
    assert(entry_count > 0);

    size_t entry_array_size = sizeof(size_t) * (entry_count - 1);
    size_t *entry_positions = (size_t *)malloc(entry_array_size);
    assert(entry_positions);
    memset(entry_positions, 0, entry_array_size);

    size_t entry_pos = 0;
    size_t count = 0;
    for (size_t i = 0; i < a->size; i++)
    {
        entry_pos = i + 1; 
        if (a->buf[i] == '\n' && entry_pos < (a->size-1) )
            entry_positions[count++] = entry_pos;
    }
    /*printf("COUNT: %li\n", count); */

    return (EntryList) {
        .entry_pos_list = entry_positions,
        .entry_count = entry_count
    };
}

static Header csv_header_init(str_t *buffer)
{
    assert(buffer);

    size_t count = csv_get_header_count(buffer);
    char **header = (char **)malloc(count * sizeof(char *));

    assert(header);

    for (size_t i = 0; i < count; i++) 
    {
        header[i] = (char *)malloc(KB);
        assert(header[i]);
    }

    char word[KB] = {0};
    char c;
    for (size_t i = 0, word_pos = 0, header_count = 0; i <= buffer->size; i++) {

        c = buffer->buf[i];

        if (c == ',' ) {

            strcpy(header[header_count], word); 
            word[word_pos] = '\0';
            word_pos = 0;
            header_count++;

        } else if (c == '\n') {

            strcpy(header[header_count], word); 
            word[word_pos] = '\0';
            word_pos = 0;
            header_count++;
            break;
        } else {
            word[word_pos++] = c;
        }

    }
    return (Header) {
        .header = header,
        .header_count = count 
    };
}


static CSV csv_init(str_t *buffer)
{
    assert(buffer);
    return (CSV) {
        .header = csv_header_init(buffer),
        .entries = csv_entry_list_init(buffer),
        .buffer = buffer
    };
}

static size_t csv_get_entry_pos_from_list(CSV *csv, size_t offset)
{
    assert(csv);
    return *(csv->entries.entry_pos_list + offset);
}

static void csv_print_entry_from_line_num(CSV *csv, size_t lnum)
{
    assert(csv);
    assert(lnum > 0);
    assert(lnum <= csv->entries.entry_count);

    lnum = lnum-1;

    size_t buf_pos = csv_get_entry_pos_from_list(csv, lnum);

    str_t *buffer = csv->buffer;
    char ch;
    while ((ch = buffer->buf[buf_pos]) != '\n') {
        printf("%c", ch);
        buf_pos++;
    }
    printf("\n");
}

static size_t csv_get_entry_pos_from_general_buffer_pos(CSV *csv, size_t gpos)
{
    assert(csv);

    size_t *arr = csv->entries.entry_pos_list;
    assert(arr);
    size_t total_entry = csv->entries.entry_count;

    for (size_t i = 0; i < total_entry; i++)
    {
        if (gpos  == arr[i]) 
            return arr[i];
        else if (gpos < arr[i]) 
            return i != 0 ? arr[--i] : arr[0];
    }

    fprintf(stderr, "%s: gpos = %li\n", __func__, gpos);
    fprintf(stderr, "%s: failed to find general position\n", __func__);
    exit(1);
}

static size_t csv_get_line_num_from_entry_pos(CSV *csv, size_t epos)
{
    assert(csv);

    size_t line_num = 1;
    size_t *arr = csv->entries.entry_pos_list;
    size_t total_entry = csv->entries.entry_count;
    for (size_t i = 0; i < total_entry; i++, line_num++)
        if (epos == arr[i])
            return line_num;
     
    fprintf(stderr, "%s: epos = %li\n", __func__, epos);
    fprintf(stderr, "%s: failed to find line_num\n", __func__);
    exit(1);
}

static void csv_print_all_entries(CSV *a)
{
    assert(a);
    CSV csv = *a;
    size_t i = 0;
    while (i < csv.entries.entry_count)
    {
        size_t epos = csv.entries.entry_pos_list[i];
        size_t line_num = csv_get_line_num_from_entry_pos(&csv, epos);
        csv_print_entry_from_line_num(&csv, line_num);
        i++;
    }
}


static size_t csv_get_entry_pos_from_line_number(CSV *csv, size_t line_num)
{
    assert(csv);
    assert(line_num > 0);

    return csv->entries.entry_pos_list[line_num-1];
}

Row csv_get_row_from_line_number(CSV *csv, size_t line_num)
{
    assert(csv); 
    assert(line_num > 0); 

    Row row = {0};
    
    size_t epos = csv_get_entry_pos_from_line_number(csv, line_num);

    str_t *tmp = csv->buffer; 
    assert(tmp);

    size_t fcount = csv->header.header_count;

    str_t **list_buff = (str_t **)malloc(fcount * sizeof(str_t *));
    assert(list_buff);
    for (size_t i = 0; i < fcount; i++) 
    {
        list_buff[i] = (str_t *)malloc(sizeof(str_t));
        assert(list_buff[i]);
    }

    char word[KB] = {0};

    for (size_t i = epos, wcount = 0, lcount = 0; ;i++) {

        if (tmp->buf[i] == ',') {

            word[wcount] = '\0';

            list_buff[lcount] = new_pstr(word);

            lcount++;
            wcount = 0;
            memset(word, 0, KB);
            continue;

        } else if (tmp->buf[i] == '\n') {

            word[wcount] = '\0';

            list_buff[lcount] = new_pstr(word);

            lcount++;
            break;
        }
        word[wcount] = tmp->buf[i];
        wcount++;

    }
    
    row.field_count = fcount;
    row.buffer = list_buff;
    return row;
}

static void csv_print_row(Row *row)
{
    assert(row);
    
    for (size_t i = 0; i < row->field_count; i++)
    {
        str_print(row->buffer[i]);
        printf("\n");
    }
}

static inline int csv_get_line_num_from_string(CSV *csv, str_t *find)
{
    int gpos = str_is_string_in_buffer(find, csv->buffer);
    if (gpos == -1) return gpos;

    size_t epos = csv_get_entry_pos_from_general_buffer_pos(csv, gpos); 
    size_t line_num = csv_get_line_num_from_entry_pos(csv, epos);

    return line_num;
}

static void csv_destroy(CSV *csv)
{

    // TODO: Rows not accounted for yet
    
    assert(csv);

    // str buffer
    if (csv->buffer != NULL) str_free(csv->buffer);

    //Entry list
    if (csv->entries.entry_pos_list != NULL) free(csv->entries.entry_pos_list);

    // Header
    if (csv->header.header != NULL) {
        for (size_t i = 0; i < csv->header.header_count; i++)
            free(csv->header.header[i]);
        free(csv->header.header);
    }

}

#endif
