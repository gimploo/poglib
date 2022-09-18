#ifndef CSV_H
#define CSV_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// my string header file
#include "str.h"
#include "../poglib/ds/stack.h"

// my debug header file
#ifdef DEBUG
#include "dbg/dbg.h" 
extern dbg_t debug;
#endif


typedef struct {

    str_t **buffer;
    size_t buff_count;

    size_t header_count;

} Row;

// Holds meta data on positions of each entry in buffer 
// and the count of total entries
typedef struct {

    size_t *entry_pos_list;
    size_t entry_count;

} EntryMeta;

typedef struct {

    str_t **header;
    size_t header_count;

} Header;

typedef struct {

    Header header;
    EntryMeta entries;
    str_t *buffer;
    
    Row **row_list;
    int row_list_index;

} CSV;




CSV         csv_init(str_t *buffer);

Row *       csv_get_row_from_line_number(const CSV *csv, size_t line_num);
void        csv_row_destroy(Row *row);

int         csv_get_line_num_from_string(CSV *csv, str_t *find); // Depricated
size_t      csv_get_line_num_of_string_restricted_to_a_header_field(CSV *csv, size_t field_num, str_t *find_word);
size_t      csv_get_line_num_from_entry_pos(CSV *csv, size_t epos);

size_t      csv_get_entry_pos_from_general_buffer_pos(CSV *csv, size_t gpos);
size_t      csv_get_entry_pos_from_line_number(const CSV *csv, size_t line_num);
size_t      csv_get_entry_pos_from_entry_pos_list(CSV *csv, size_t offset);

void        csv_print_row(Row *row);
void        csv_print_all_entries(CSV *a);
void        csv_print_entry_from_line_num(CSV *csv, size_t lnum);

void        csv_destroy(CSV *csv);


/*
 *
 * ======================================================================
 * | 
 * |
 * |             IMPLEMENTATION
 * |
 * |
 * ======================================================================
 *
 */



size_t _get_total_entries(str_t *a)
{
    assert(a);

    //NOTE: fixed dont change!
    
    size_t count = 0;
    for (size_t i = 0; i < a->len; i++)
        if (a->buf[i] == '\n')
            count++;
    return count;
}


size_t _get_header_count(str_t *buffer)
{
    assert(buffer);
    size_t count = 0;
    for (size_t i = 0; buffer->buf[i] != '\n'; i++) 
        if (buffer->buf[i] == ',' )
            count++;
    return count + 1;
}

EntryMeta _csv_entry_list_init(str_t *a)
{
    assert(a);

    size_t entry_count = _get_total_entries(a);
    assert(entry_count > 0);

    size_t entry_array_size = sizeof(size_t) * (entry_count);
    size_t *entry_positions = (size_t *)malloc(entry_array_size);
    assert(entry_positions);
    memset(entry_positions, 0, entry_array_size);

    size_t entry_pos = 0;
    size_t count = 0;
    for (size_t i = 0; i < a->len; i++)
    {
        entry_pos = i + 1; 
        if (a->buf[i] == '\n' &&  a->buf[i] != '\0') {// entry_pos < (a->size-1) ) {
            entry_positions[count] = entry_pos;
            count++;
        }

    }
    //printf("COUNT: %li\n", count); 

    return (EntryMeta) {
        .entry_pos_list = entry_positions,
        .entry_count = entry_count
    };
}

Header _csv_header_init(str_t *buffer)
{
    assert(buffer);

    size_t count = _get_header_count(buffer);
    str_t **header = (str_t **)malloc(count * sizeof(str_t *));

    assert(header);


    char word[KB] = {0};
    char c;
    for (size_t i = 0, word_pos = 0, header_count = 0; i <= buffer->len; i++) {

        c = buffer->buf[i];

        if (c == ',' ) {

            word[word_pos] = '\0';
            *header[header_count] = str_init(word);
            word_pos = 0;
            header_count++;

        } else if (c == '\n') {

            word[word_pos] = '\0';
            *header[header_count] = str_init(word);
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

Row ** _csv_row_array_init(size_t ecount)
{
    if (ecount == 0) {
        fprintf(stderr, "%s: ecount == 0 wtf\n", __func__);
        return NULL;
    }

    Row **row = (Row **)malloc(sizeof(Row *) * ecount);
    assert(row);
    memset(row, 0, (sizeof(Row *) * ecount));

    return row;
}

CSV csv_init(str_t *buffer)
{
    assert(buffer);
    CSV csv = {0};

    csv.header = _csv_header_init(buffer);
    csv.entries = _csv_entry_list_init(buffer);
    csv.buffer = buffer; // TO avoid the confusion of whether to pass an allocated or static buffer

    //csv.row_list_index = -1;
    //csv.row_list = _csv_row_array_init(csv.entries.entry_count);

    return csv;
}

size_t csv_get_entry_pos_from_entry_pos_list(CSV *csv, size_t offset)
{
    assert(csv);
    return *(csv->entries.entry_pos_list + offset);
}

void csv_print_entry_from_line_num(CSV *csv, size_t lnum)
{
    assert(csv);
    assert(lnum > 0);
    assert(lnum <= csv->entries.entry_count);

    lnum = lnum-1;

    size_t buf_pos = csv_get_entry_pos_from_entry_pos_list(csv, lnum);

    str_t *buffer = csv->buffer;
    char ch;
    for(size_t i = buf_pos; i < buffer->len; i++)
    {
        ch = buffer->buf[i];
        if (ch == '\n') {
            break;
        }
        printf("%c", ch);
    }
    printf("\n");
}

size_t csv_get_entry_pos_from_general_buffer_pos(CSV *csv, size_t gpos)
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

size_t csv_get_line_num_from_entry_pos(CSV *csv, size_t epos)
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


void csv_print_all_entries(CSV *a)
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


size_t csv_get_entry_pos_from_line_number(const CSV *csv, size_t line_num)
{
    assert(csv);
    assert(line_num > 0);

    return csv->entries.entry_pos_list[line_num-1];
}


Row * csv_get_row_from_line_number(const CSV *csv, size_t line_num)
{
    assert(csv); 
    assert(line_num > 0); 

    if (csv->row_list_index == (int )csv->entries.entry_count) {

        //TODO: row list has reached max capacity, what do u do?
        fprintf(stderr, "%s: row list has reached max capacity\n", __func__);
        exit(2);
    }


    Row *row = (Row *)malloc(sizeof(Row));
    assert(row);
    
    size_t epos = csv_get_entry_pos_from_line_number(csv, line_num);

    str_t *tmp = csv->buffer; 
    assert(tmp);

    size_t fcount = csv->header.header_count;
    assert(fcount != 0);

    
    size_t list_buff_size = (fcount + 1) * sizeof(str_t *);

    str_t **list_buff = (str_t **)malloc(list_buff_size);

    assert(list_buff);
    memset(list_buff, 0, list_buff_size);


    char word[KB] = {0};

    size_t lcount = 0;
    for (size_t i = epos, wcount = 0; i <= csv->buffer->len; i++) {

        if (tmp->buf[i] == '"') { 

            while (tmp->buf[++i] != '"') {
                word[wcount] = tmp->buf[i];
                wcount++;
            }
            word[wcount] = '\0';

            *list_buff[lcount] = str_init(word);

            lcount++;
            wcount = 0;
            memset(word, 0, KB);
            continue;

        } else if (tmp->buf[i] == ',') {

            word[wcount] = '\0';

            *list_buff[lcount] = str_init(word);

            lcount++;
            wcount = 0;
            memset(word, 0, KB);
            continue;

        } else if (tmp->buf[i] == '\n' || i == csv->buffer->len) {

            word[wcount] = '\0';

            *list_buff[lcount] = str_init(word);

            break;
        } 

        word[wcount] = tmp->buf[i];
        wcount++;
    }
    
    assert(fcount == csv->header.header_count);
    row->buff_count = lcount;
    row->header_count = fcount;
    row->buffer = list_buff;

     //To free it later (memory management)
    //csv->row_list[++csv->row_list_index] = row;

    return row;
}


void csv_print_row(Row *row)
{
    assert(row);
    
    for (size_t i = 0; i < row->header_count; i++)
    {
        str_print(row->buffer[i]);
        printf("\n");
    }
}


int csv_get_line_num_from_string(CSV *csv, str_t *find)
{
    assert(csv);
    assert(find);

    int gpos = str_is_word_in_buffer(find, csv->buffer);
    if (gpos == -1) return gpos;

    size_t epos = csv_get_entry_pos_from_general_buffer_pos(csv, gpos); 
    size_t line_num = csv_get_line_num_from_entry_pos(csv, epos);

    return line_num;
}

void csv_row_destroy(Row *row)
{
    if (row == NULL) eprint("row argument is null");

    for (size_t j = 0; j < row->buff_count; j++)
    {
        str_free(row->buffer[j]);
    }
    free(row->buffer);
    row->buffer = NULL;
    free(row);
    row = NULL;

}


void csv_destroy(CSV *csv)
{
    assert(csv);

    // str_t buffer
    str_free(csv->buffer);

    //Meta list
    if (csv->entries.entry_pos_list != NULL) {

        free(csv->entries.entry_pos_list);
        csv->entries.entry_pos_list = NULL;

    }

     //Row list
    //if (csv->row_list != NULL) {

        //Row *list = NULL;
        //for (int i = 0; i <= csv->row_list_index; i++)
        //{
            //list = csv->row_list[i];
            //assert(list);
            //for (size_t j = 0; j < list->buff_count; j++)
            //{
                //pstr_free(list->buffer[j]);
            //}
            //free(list->buffer);
            //list->buffer = NULL;
            //free(list);
            //list = NULL;
        //}
        //free(csv->row_list);
        //csv->row_list = NULL;
    //}

    // Header
    if (csv->header.header != NULL) {

        for (size_t i = 0; i < csv->header.header_count; i++) {
            str_free(csv->header.header[i]);
            csv->header.header[i] = NULL;
        }
        free(csv->header.header);
        csv->header.header = NULL;
    }

}

size_t csv_get_line_num_of_string_restricted_to_a_header_field(CSV *csv, size_t field_num, str_t *find_word)
{
    if (csv == NULL) {
        fprintf(stderr, "%s: csv argument is null\n", __func__);
        exit(1);
    }

    if (field_num  > csv->header.header_count && field_num != 0) {
        fprintf(stderr, "%s: field_num argument\n", __func__);
        exit(1);
    }

    if (find_word == NULL) {
        fprintf(stderr, "%s: buffer null argument\n", __func__);
        exit(1);
    }

    size_t line_num = 0;
    size_t buffer_index = field_num-1;

    for (size_t i = 0; i < csv->entries.entry_count; i++)
    {
        size_t epos = csv->entries.entry_pos_list[i];
        size_t lnum = csv_get_line_num_from_entry_pos(csv, epos);

        Row *row = csv_get_row_from_line_number(csv, lnum);

        str_t *str_field = row->buffer[buffer_index]; 
        assert(str_field);

        if (str_is_word_in_buffer(find_word, str_field)) {
            line_num = i+1;
            break;
        }

    }

    return line_num;
}

void csv_get_all_line_nums_of_string_restricted_to_a_header_field(CSV *csv, stack_t *buffer, size_t field_num, str_t *find_word)
{
    if (csv == NULL) {
        fprintf(stderr, "%s: csv argument is null\n", __func__);
        exit(1);
    }

    if (field_num  > csv->header.header_count && field_num != 0) {
        fprintf(stderr, "%s: field_num argument\n", __func__);
        exit(1);
    }

    if (find_word == NULL) {
        fprintf(stderr, "%s: buffer null argument\n", __func__);
        exit(1);
    }

    size_t line_num_list_index = 0;
    size_t buffer_index = field_num-1;

    for (size_t i = 0; i < csv->entries.entry_count; i++)
    {
        size_t epos = csv->entries.entry_pos_list[i];
        size_t lnum = csv_get_line_num_from_entry_pos(csv, epos);

        Row *row = csv_get_row_from_line_number(csv, lnum);

        str_t *str_field = row->buffer[buffer_index]; 
        assert(str_field);

        if (str_is_word_in_buffer(find_word, str_field)) {
            line_num_list_index = i+1;
            stack_push(buffer, line_num_list_index);
        }

    }
}

#endif
