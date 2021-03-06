/**
 * init_database.c
 *
 * Copyright (c) 2013, 2014
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

/**
 * @file init_database.c
 *
 * @brief Initialization of system dictionary and phone phrase tree.\n
 *
 *      This program reads in source of dictionary.\n
 *      Output a database file containing a phone phrase tree, and a dictionary file\n
 * filled with non-duplicate phrases.\n
 *      Each node represents a single phone.\n
 *      The output file contains a random access array, where each record includes:\n
 *      \code{
 *            [16-bit uint] key; may be phone data or record of input keys
 *            [24-bit uint] child.begin, child.end; for internal nodes (key != 0)
 *            [24-bit uint] phrase.pos; for leaf nodes (key == 0), position of phrase in dictionary
 *            [24-bit uint] phrase.freq; for leaf nodes (key == 0), frequency of the phrase
 *      }\endcode
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "taigi-private.h"
#include "taigi-utf8-util.h"
#include "global-private.h"
#include "key2pho-private.h"
#include "memory-private.h"
#include "bopomofo-private.h"

/* For ALC macro */
#include "private.h"

#define CHARDEF               "%chardef"
#define BEGIN                 "begin"
#define END                   "end"
#define MAX_LINE_LEN          (1024)
#define MAX_WORD_DATA         (60000)
#define MAX_PHRASE_BUF_LEN    (149)
#define MAX_PHRASE_DATA       (420000)

const char USAGE[] =
    "Usage: %s <phone.cin> <tsi.src>\n"
    "This program creates the following new files:\n"
    "* " PHONE_TREE_FILE "\n\tindex to phrase file (dictionary)\n" "* " DICT_FILE "\n\tmain phrase file\n";

/* An additional pos helps avoid duplicate Chinese strings. */
typedef struct {
    char phrase[MAX_PHRASE_BUF_LEN];
    uint32_t type;
    uint32_t freq;
    uint32_t phone[MAX_PHRASE_LEN + 1];
    long pos;
} PhraseData;

typedef struct {
    PhraseData *text;           /* Common part shared with PhraseData. */
    int index;                  /* For stable sorting. */
} WordData;

/*
 * Please see TreeType for data field. pFirstChild points to the first of its
 * child list. pNextSibling points to its right sibling, where it and its right
 * sibling are both in the child list of its parent. However, pNextSibling will
 * become next-pointer like linked list, which makes writing of index-tree file
 * become a sequential traversal rather than BFS.
 */
typedef struct _tNODE {
    TreeType data;
    struct _tNODE *pFirstChild;
    struct _tNODE *pNextSibling;
} NODE;

WordData word_data[MAX_WORD_DATA];
char word_matched[MAX_WORD_DATA];
int num_word_data = 0;

PhraseData phrase_data[MAX_PHRASE_DATA];
int num_phrase_data = 0;
int top_phrase_data = MAX_PHRASE_DATA;

NODE *root;

void strip(char *line)
{
    char *end;
    size_t i;

    /* remove comment */
    for (i = 0; i < strlen(line); ++i) {
        if (line[i] == '#') {
            line[i] = '\0';
            break;
        }
    }

    /* remove tailing space */
    end = line + strlen(line) - 1;
    while (end >= line && isspace((unsigned char) *end)) {
        *end = 0;
        --end;
    }
}

/* word_data is sorted reversely, for stack-like push operation. */
int compare_word_by_phone(const void *x, const void *y)
{
    const WordData *a = (const WordData *) x;
    const WordData *b = (const WordData *) y;

    if (a->text->phone[0] != b->text->phone[0])
        return b->text->phone[0] - a->text->phone[0];

    /* Compare original index for stable sort */
    return b->index - a->index;
}

int compare_word_by_text(const void *x, const void *y)
{
    const WordData *a = (const WordData *) x;
    const WordData *b = (const WordData *) y;
    int ret = strcmp(a->text->phrase, b->text->phrase);

    if (ret != 0)
        return ret;

    if (a->text->phone[0] != b->text->phone[0])
        return a->text->phone[0] - b->text->phone[0];

    return 0;
}

int compare_word_no_duplicated(const void *x, const void *y)
{
    int ret = compare_word_by_text(x, y);

    if (!ret) {
        const WordData *a = (const WordData *) x;
	const WordData *b = (const WordData *) y;

        fprintf(stderr, "Duplicated word found (`%s', %d).('%s', %d)\n", a->text->phrase, a->text->phone[0], b->text->phrase, b->text->phone[0]);
        exit(-1);
    }

    return ret;
}

int is_exception_phrase(PhraseData *phrase, int pos)
{
    return 0;
}

void store_phrase(const char *line, int line_num)
{
    const char DELIM[] = " \t\n";
    char buf[MAX_LINE_LEN];
    char *phrase;
    char *freq;
    char *endptr = NULL;
    char *bopomofo;
    char bopomofo_buf[32];
    size_t phrase_len;
    WordData word;              /* For check. */
    WordData *found_word = NULL;
    size_t i, j;

    snprintf(buf, sizeof(buf), "%s", line);
    strip(buf);
    if (strlen(buf) == 0)
        return;

    if (num_phrase_data >= top_phrase_data) {
        fprintf(stderr, "Need to increase MAX_PHRASE_DATA to process\n");
        exit(-1);
    }

    /* read phrase */
    phrase = strtok(buf, DELIM);
    if (!phrase) {
        fprintf(stderr, "Error reading line %d, `%s'\n", line_num, line);
        exit(-1);
    }
    strncpy(phrase_data[num_phrase_data].phrase, phrase, sizeof(phrase_data[0].phrase) - 1);

    /* read frequency */
    freq = strtok(NULL, DELIM);
    if (!freq) {
        fprintf(stderr, "Error reading line %d, `%s'\n", line_num, line);
        exit(-1);
    }

    phrase_data[num_phrase_data].freq = strtoul(freq, &endptr, 0);
    phrase_data[num_phrase_data].type = TYPE_HAN;
    if ((*freq == '\0' || *endptr != '\0') ||
        (phrase_data[num_phrase_data].freq == UINT32_MAX && errno == ERANGE)) {
        fprintf(stderr, "Error reading frequency `%s' in line %d, `%s'\n", freq, line_num, line);
        exit(-1);
    }

    /* read bopomofo */
    for (bopomofo = strtok(NULL, DELIM), phrase_len = 0;
         bopomofo && phrase_len < MAX_PHRASE_LEN; bopomofo = strtok(NULL, DELIM), ++phrase_len) {

        phrase_data[num_phrase_data].phone[phrase_len] = UintFromPhone(bopomofo);
        if (phrase_data[num_phrase_data].phone[phrase_len] == 0) {
            fprintf(stderr, "Error reading bopomofo `%s' in line %d, `%s'\n", bopomofo, line_num, line);
            exit(-1);
        }
    }
    if (bopomofo) {
        fprintf(stderr, "Phrase `%s' too long in line %d\n", phrase, line_num);
    }
#if 0
    /* check phrase length & bopomofo length */
    if ((size_t) ueStrLen(phrase_data[num_phrase_data].phrase) != phrase_len) {
        fprintf(stderr, "Phrase length and bopomofo length mismatch in line %d, `%s'\n", line_num, line);
        fprintf(stderr, "\tcalculated len=%d, recorded len=%d\n", ueStrLen(phrase_data[num_phrase_data].phrase), phrase_len);
        exit(-1);
    }
#endif
    /* Check that each word in phrase can be found in word list. */
    word.text = ALC(PhraseData, 1);

    assert(word.text);
    for (i = 0; i < phrase_len; ++i) {
        ueStrNCpy(word.text->phrase, ueStrSeek(phrase_data[num_phrase_data].phrase, i), 1, 1);
        word.text->phone[0] = phrase_data[num_phrase_data].phone[i];
        found_word = bsearch(&word, word_data, num_word_data, sizeof(word), compare_word_by_text);
        if ((found_word == NULL ||
             (phrase_len == 1 &&
              word_matched[found_word - word_data])) && !is_exception_phrase(&phrase_data[num_phrase_data], i)) {

            PhoneFromUint(bopomofo_buf, sizeof(bopomofo_buf), word.text->phone[0]);

            fprintf(stderr, "Error in phrase `%s'. Word `%s' has no phone %d (%s) in line %d\n",
                    phrase_data[num_phrase_data].phrase, word.text->phrase, word.text->phone[0], bopomofo_buf,
                    line_num);
            fprintf(stderr, "\tAdd the following struct to EXCEPTION_PHRASE if this is good phrase\n\t{\"");
            for (j = 0; j < strlen(phrase_data[num_phrase_data].phrase); ++j) {
                fprintf(stderr, "\\x%02X", (unsigned char) phrase_data[num_phrase_data].phrase[j]);
            }
            fprintf(stderr, "\" /* %s */ , 0, {%d", phrase_data[num_phrase_data].phrase,
                    phrase_data[num_phrase_data].phone[0]);
            for (j = 1; j < phrase_len; ++j) {
                fprintf(stderr, ", %d", phrase_data[num_phrase_data].phone[j]);
            }
            fprintf(stderr, "} /* ");
            for (j = 0; j < phrase_len; ++j) {
                PhoneFromUint(bopomofo_buf, sizeof(bopomofo_buf), phrase_data[num_phrase_data].phone[j]);
                fprintf(stderr, "%s ", bopomofo_buf);
            }
            fprintf(stderr, "*/, 0},\n");
            exit(-1);
        }
    }
    free(word.text);

    if (phrase_len >= 2)
        ++num_phrase_data;
    else
        word_matched[found_word - word_data] = 1;
}

int compare_phrase(const void *x, const void *y)
{
    const PhraseData *a = (const PhraseData *) x;
    const PhraseData *b = (const PhraseData *) y;
    int cmp = strcmp(a->phrase, b->phrase);

    /* If phrases are different, it returns the result of strcmp(); else it
     * reports an error when the same phone sequence is found.
     */
    if (cmp)
        return cmp;
    if (!memcmp(a->phone, b->phone, sizeof(a->phone))) {
        fprintf(stderr, "Duplicated phrase `%s' found.\n", a->phrase);
        exit(-1);
    }
    return b->freq - a->freq;
}

void read_tsi_src(const char *filename)
{
    FILE *tsi_src;
    char buf[MAX_LINE_LEN];
    int line_num = 0;

    tsi_src = fopen(filename, "r");
    if (!tsi_src) {
        fprintf(stderr, "Error opening the file %s\n", filename);
        exit(-1);
    }

    while (fgets(buf, sizeof(buf), tsi_src)) {
        ++line_num;
        store_phrase(buf, line_num);
    }

    qsort(phrase_data, num_phrase_data, sizeof(phrase_data[0]), compare_phrase);
    fclose(tsi_src);
}

void store_tailo(const char *line, const int line_num)
{
    char phone_buf[32];
    char key_buf[16];
    char buf[MAX_LINE_LEN + 1] = {0};

    strncpy(buf, line, sizeof(buf) - 1);

    strip(buf);
    if (strlen(buf) == 0)
        return;

    if (num_word_data >= MAX_WORD_DATA) {
        fprintf(stderr, "Need to increase MAX_WORD_DATA to process\n");
        exit(-1);
    }
    if (top_phrase_data <= num_phrase_data) {
        fprintf(stderr, "Need to increase MAX_PHRASE_DATA to process\n");
        exit(-1);
    }
    word_data[num_word_data].text = &phrase_data[--top_phrase_data];

#define UTF8_FORMAT_STRING(len1, len2) \
    "%" __stringify(len1) "[^ ]" " " \
    "%" __stringify(len2) "[^ ]"
    sscanf(buf, UTF8_FORMAT_STRING(16, 16), key_buf, word_data[num_word_data].text->phrase);

    if (strlen(key_buf) > 16) {
        fprintf(stderr, "Error reading line %d, `%s'\n", line_num, line);
        exit(-1);
    }
    PhoneFromKey(phone_buf, key_buf, KB_DEFAULT, 1);
    word_data[num_word_data].text->phone[0] = UintFromPhone(phone_buf);
    word_data[num_word_data].text->type = TYPE_TAILO;
    printf("phone_buf=%s, key_buf=%s, phrase=%s, phone=0x%x, type=%d\n", phone_buf, key_buf,
			    word_data[num_word_data].text->phrase, word_data[num_word_data].text->phone[0],
			    word_data[num_word_data].text->type);
    word_data[num_word_data].index = num_word_data;
    ++num_word_data;
}

void store_word(const char *line, const int line_num)
{
    char phone_buf[32];
    char key_buf[16];
    char buf[MAX_LINE_LEN + 1] = {0};

    strncpy(buf, line, sizeof(buf) - 1);

    strip(buf);
    if (strlen(buf) == 0)
        return;

    if (num_word_data >= MAX_WORD_DATA) {
        fprintf(stderr, "Need to increase MAX_WORD_DATA to process\n");
        exit(-1);
    }
    if (top_phrase_data <= num_phrase_data) {
        fprintf(stderr, "Need to increase MAX_PHRASE_DATA to process\n");
        exit(-1);
    }
    word_data[num_word_data].text = &phrase_data[--top_phrase_data];

#define UTF8_FORMAT_STRING(len1, len2) \
    "%" __stringify(len1) "[^ ]" " " \
    "%" __stringify(len2) "[^ ]"
    sscanf(buf, UTF8_FORMAT_STRING(16, 16), key_buf, word_data[num_word_data].text->phrase);

    if (strlen(key_buf) > 16) {
        fprintf(stderr, "Error reading line %d, `%s'\n", line_num, line);
        exit(-1);
    }
    PhoneFromKey(phone_buf, key_buf, KB_DEFAULT, 1);
    word_data[num_word_data].text->phone[0] = UintFromPhone(phone_buf);
    word_data[num_word_data].text->type = TYPE_HAN;
    printf("phone_buf=%s, key_buf=%s, phrase=%s, phone=0x%x, type=%d\n", phone_buf, key_buf,
		    word_data[num_word_data].text->phrase, word_data[num_word_data].text->phone[0],
		    word_data[num_word_data].text->type);
    word_data[num_word_data].index = num_word_data;
    ++num_word_data;
}

void read_tailo_cin(const char *filename)
{
    FILE *phone_cin;
    char buf[MAX_LINE_LEN];
    char *ret;
    int line_num = 0;
    enum { INIT, HAS_CHARDEF_BEGIN, HAS_CHARDEF_END } status;

    phone_cin = fopen(filename, "r");
    if (!phone_cin) {
        fprintf(stderr, "Error opening the file %s\n", filename);
        exit(-1);
    }

    for (status = INIT; status != HAS_CHARDEF_BEGIN;) {
        ret = fgets(buf, sizeof(buf), phone_cin);
        ++line_num;
        if (!ret) {
            fprintf(stderr, "%s: No expected %s %s\n", filename, CHARDEF, BEGIN);
            exit(-1);
        }

        strip(buf);
        ret = strtok(buf, " \t");
        if (!strcmp(ret, CHARDEF)) {
            ret = strtok(NULL, " \t");
            if (!strcmp(ret, BEGIN))
                status = HAS_CHARDEF_BEGIN;
            else {
                fprintf(stderr, "%s:%d: Unexpected %s %s\n", filename, line_num, CHARDEF, ret);
                exit(-1);
            }
        }
    }

    while (status != HAS_CHARDEF_END) {
        ret = fgets(buf, sizeof(buf), phone_cin);
        ++line_num;
        if (!ret) {
            fprintf(stderr, "%s: No expected %s %s\n", filename, CHARDEF, END);
            exit(-1);
        }

        strip(buf);
        if (!strncmp(buf, CHARDEF, strlen(CHARDEF))) {
            strtok(buf, " \t");
            ret = strtok(NULL, " \t");
            if (!strcmp(ret, END))
                status = HAS_CHARDEF_END;
            else {
                fprintf(stderr, "%s:%d: Unexpected %s %s\n", filename, line_num, CHARDEF, ret);
                exit(-1);
            }
        } else
            store_tailo(buf, line_num);
    }

    fclose(phone_cin);
    qsort(word_data, num_word_data, sizeof(word_data[0]), compare_word_no_duplicated);
}

void read_phone_cin(const char *filename)
{
    FILE *phone_cin;
    char buf[MAX_LINE_LEN];
    char *ret;
    int line_num = 0;
    enum { INIT, HAS_CHARDEF_BEGIN, HAS_CHARDEF_END } status;

    phone_cin = fopen(filename, "r");
    if (!phone_cin) {
        fprintf(stderr, "Error opening the file %s\n", filename);
        exit(-1);
    }

    for (status = INIT; status != HAS_CHARDEF_BEGIN;) {
        ret = fgets(buf, sizeof(buf), phone_cin);
        ++line_num;
        if (!ret) {
            fprintf(stderr, "%s: No expected %s %s\n", filename, CHARDEF, BEGIN);
            exit(-1);
        }

        strip(buf);
        ret = strtok(buf, " \t");
        if (!strcmp(ret, CHARDEF)) {
            ret = strtok(NULL, " \t");
            if (!strcmp(ret, BEGIN))
                status = HAS_CHARDEF_BEGIN;
            else {
                fprintf(stderr, "%s:%d: Unexpected %s %s\n", filename, line_num, CHARDEF, ret);
                exit(-1);
            }
        }
    }

    while (status != HAS_CHARDEF_END) {
        ret = fgets(buf, sizeof(buf), phone_cin);
        ++line_num;
        if (!ret) {
            fprintf(stderr, "%s: No expected %s %s\n", filename, CHARDEF, END);
            exit(-1);
        }

        strip(buf);
        if (!strncmp(buf, CHARDEF, strlen(CHARDEF))) {
            strtok(buf, " \t");
            ret = strtok(NULL, " \t");
            if (!strcmp(ret, END))
                status = HAS_CHARDEF_END;
            else {
                fprintf(stderr, "%s:%d: Unexpected %s %s\n", filename, line_num, CHARDEF, ret);
                exit(-1);
            }
        } else
            store_word(buf, line_num);
    }

    fclose(phone_cin);
    qsort(word_data, num_word_data, sizeof(word_data[0]), compare_word_no_duplicated);
}

NODE *new_node(uint32_t key)
{
    NODE *pnew = ALC(NODE, 1);

    if (pnew == NULL) {
        fprintf(stderr, "Memory allocation failed on constructing phrase tree.\n");
        exit(-1);
    }
    memset(&pnew->data, 0, sizeof(pnew->data));
    PutUint32(key, pnew->data.key);
    pnew->pFirstChild = NULL;
    pnew->pNextSibling = NULL;
    return pnew;
}

/*
 * This function puts FindKey() and Insert() together. It first searches for the
 * specified key and performs FindKey() on hit. Otherwise, it inserts a new node
 * at proper position and returns the newly inserted child.
 */
NODE *find_or_insert(NODE * parent, uint32_t key)
{
    NODE *prev = NULL;
    NODE *p;
    NODE *pnew;

    for (p = parent->pFirstChild; p && GetUint32(p->data.key) <= key; prev = p, p = p->pNextSibling)
        if (GetUint32(p->data.key) == key)
            return p;

    pnew = new_node(key);
    pnew->pNextSibling = p;
    if (prev == NULL)
        parent->pFirstChild = pnew;
    else
        prev->pNextSibling = pnew;
    pnew->pNextSibling = p;
    return pnew;
}

void insert_leaf(NODE * parent, long phr_pos, uint32_t freq, uint32_t type)
{
    NODE *prev = NULL;
    NODE *p;
    NODE *pnew;

    for (p = parent->pFirstChild; p && GetUint32(p->data.key) == 0; prev = p, p = p->pNextSibling)
        if (GetUint32(p->data.phrase.freq) <= freq)
            break;

    pnew = new_node(0);
    PutUint32((uint32_t) phr_pos, pnew->data.phrase.pos);
    PutUint32(freq, pnew->data.phrase.freq);
    PutUint32(type, pnew->data.type);
    if (prev == NULL)
        parent->pFirstChild = pnew;
    else
        prev->pNextSibling = pnew;
    pnew->pNextSibling = p;
}

void construct_phrase_tree()
{
    NODE *levelPtr;
    int i;
    int j;

    /* First, assume that words are in order of their phones and indices. */
    qsort(word_data, num_word_data, sizeof(word_data[0]), compare_word_by_phone);

    /* Key value of root will become tree_size later. */
    root = new_node(1);

    /* Second, insert word_data as the first level of children. */
    for (i = 0; i < num_word_data; i++) {
        if (i == 0 || word_data[i].text->phone[0] != word_data[i - 1].text->phone[0]) {
            levelPtr = new_node(word_data[i].text->phone[0]);
            levelPtr->pNextSibling = root->pFirstChild;
            root->pFirstChild = levelPtr;
        }
        levelPtr = new_node(0);
        PutUint32((uint32_t) word_data[i].text->pos, levelPtr->data.phrase.pos);
        PutUint32(word_data[i].text->freq, levelPtr->data.phrase.freq);
        PutUint32(word_data[i].text->type, levelPtr->data.type);
        levelPtr->pNextSibling = root->pFirstChild->pFirstChild;
        root->pFirstChild->pFirstChild = levelPtr;
    }

    /* Third, insert phrases having length at least 2. */
    for (i = 0; i < num_phrase_data; ++i) {
        levelPtr = root;
        for (j = 0; phrase_data[i].phone[j] != 0; ++j)
            levelPtr = find_or_insert(levelPtr, phrase_data[i].phone[j]);
        insert_leaf(levelPtr, phrase_data[i].pos, phrase_data[i].freq, phrase_data[i].type);
    }
}

void write_phrase_data()
{
    FILE *dict_file;
    PhraseData *cur_phr;
    PhraseData *last_phr = NULL;
    int i;
    int j;

    dict_file = fopen(DICT_FILE, "wb");

    if (!dict_file) {
        fprintf(stderr, "Cannot open output file.\n");
        exit(-1);
    }

    /*
     * Duplicate Chinese strings with common pronunciation are detected and
     * not written into system dictionary. Written phrases are separated by
     * '\0', for convenience of mmap usage.
     * Note: word_data and phrase_data have been qsorted by strcmp in
     *       reading.
     */
    for (i = j = 0; i < num_word_data || j < num_phrase_data; last_phr = cur_phr) {
        if (i == num_word_data)
            cur_phr = &phrase_data[j++];
        else if (j == num_phrase_data)
            cur_phr = word_data[i++].text;
        else if (strcmp(word_data[i].text->phrase, phrase_data[j].phrase) < 0)
            cur_phr = word_data[i++].text;
        else
            cur_phr = &phrase_data[j++];

        if (last_phr && !strcmp(cur_phr->phrase, last_phr->phrase))
            cur_phr->pos = last_phr->pos;
        else {
            cur_phr->pos = ftell(dict_file);
            fwrite(cur_phr->phrase, strlen(cur_phr->phrase) + 1, 1, dict_file);
        }
    }

    fclose(dict_file);
}

/*
 * This function performs BFS to compute child.begin and child.end of each node.
 * It sponteneously converts tree structure into a linked list. Writing the tree
 * into index file is then implemented by pure sequential traversal.
 */
void write_index_tree()
{
    /* (Circular) queue implementation is hidden within this function. */
    NODE **queue;
    NODE *p;
    NODE *pNext;
    size_t head = 0, tail = 0;
    size_t tree_size = 1;
    size_t q_len = num_word_data + num_phrase_data + 1;

    FILE *output = fopen(PHONE_TREE_FILE, "wb");

    if (!output) {
        fprintf(stderr, "Error opening file " PHONE_TREE_FILE " for output.\n");
        exit(-1);
    }

    queue = ALC(NODE *, q_len);
    assert(queue);

    queue[head++] = root;
    while (head != tail) {
        p = queue[tail++];
        if (tail >= q_len)
            tail = 0;
        if (GetUint32(p->data.key) != 0) {
            PutUint32(tree_size, p->data.child.begin);

            /*
             * The latest inserted element must have a NULL
             * pNextSibling value, and the following code let
             * it point to the next child list to serialize
             * them.
             */
            if (head == 0)
                queue[q_len - 1]->pNextSibling = p->pFirstChild;
            else
                queue[head - 1]->pNextSibling = p->pFirstChild;

            for (pNext = p->pFirstChild; pNext; pNext = pNext->pNextSibling) {
                queue[head++] = pNext;
                if (head == q_len)
                    head = 0;
                tree_size++;
            }

            PutUint32(tree_size, p->data.child.end);
        }
    }
    PutUint32(tree_size, root->data.key);

    for (p = root; p; p = pNext) {
        fwrite(&p->data, sizeof(TreeType), 1, output);
        pNext = p->pNextSibling;
        free(p);
    }
    free(queue);

    fclose(output);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf(USAGE, argv[0]);
        return -1;
    }

    read_tailo_cin("tailo.cin");
    read_phone_cin(argv[1]);
    printf("------- %s, %d --------\n", __func__, __LINE__);
    read_tsi_src(argv[2]);
    printf("------- %s, %d --------\n", __func__, __LINE__);
    write_phrase_data();
    printf("------- %s, %d --------\n", __func__, __LINE__);
    construct_phrase_tree();
    printf("------- %s, %d --------\n", __func__, __LINE__);
    write_index_tree();
    return 0;
}
