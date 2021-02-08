#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "try_macros.h"
#include "tsp_io.h"

enum {
	LINE_NAME,
	LINE_TYPE,
	LINE_COMMENT,
	LINE_DIMENSION,
	LINE_EDGE_WEIGHT_TYPE,
	LINE_NODE_COORD_SECTION,
	LINE_EOF,
	LINE_UNKNOWN
};

typedef struct {
	int type;
	const char *name;
} line_t;


line_t LINE_TYPES[] = {
	{LINE_NAME, "NAME"},
	{LINE_NAME, "TYPE"},
	{LINE_NAME, "COMMENT"},
	{LINE_NAME, "DIMENSION"},
	{LINE_NAME, "EDGE_WEIGHT"},
	{LINE_NAME, "NODE_COORD_SECTION"},
	{LINE_NAME, "EOF"}
};


static int read_problem_npts(FILE *fp, uint32_t *output);
static int read_problem(FILE *fp, vtsp_points_t *output);
static int get_line(FILE *fp, uint32_t max_len, char *line, uint32_t *len);
static int get_line_type(const char* line, uint32_t len, int *type);
static int split_str(char *line, uint32_t len, char sep,
		     char** lpart, uint32_t *llen,
		     char** rpart, uint32_t *rlen);
static int trim_ref(char *line, uint32_t len, char** ref, uint32_t *ref_len);
static int is_blank(char c);
static int parse_int(const char* input, int *output);

int vtsp_read_problem_npts(const char *input_filename, uint32_t *output)
{
	FILE *fp;
	TRY_PTR( fopen(input_filename, "r"), fp, ERROR_OPEN );

	int status = read_problem_npts(fp, output);
	
	TRY_GOTO( fclose(fp), ERROR_CLOSE );

	return status;
ERROR_OPEN:
	TRY( log_flush(stderr, "Error opening file") );
	return ERROR;
ERROR_CLOSE:
	TRY( log_flush(stderr, "Error closing file") );
	return ERROR;
}

int vtsp_read_problem(const char *input_filename, vtsp_points_t *output)
{
	FILE *fp;
	TRY_PTR( fopen(input_filename, "r"), fp, ERROR_OPEN );

	int status = read_problem(fp, output);
	
	TRY_GOTO( fclose(fp), ERROR_CLOSE );

	return status;
ERROR_OPEN:
	TRY( log_flush(stderr, "Error opening file") );
	return ERROR;
ERROR_CLOSE:
	TRY( log_flush(stderr, "Error closing file") );
	return ERROR;
}


int vtsp_read_tour(const char *input_filename, vtsp_perm_t *output)
{
	// PEDNING
}

int vtsp_write_tour(const vtsp_perm_t *input, const char *output_filename) {
	// PENDING
}

static int read_problem_npts(FILE *fp, uint32_t *output)
{
	char line[255];
	uint32_t len;
	int status;
	char *lpart, *rpart;
	uint32_t llen, rlen;
	int type;
	int val;
	while ((status = get_line(fp, 255, line, &len)) == 0) {
		TRY( split_str(line, len, ':', &lpart, &llen, &rpart, &rlen) );
		TRY( get_line_type(lpart, llen, &type) );
		if (type == LINE_DIMENSION) {
			TRY(  parse_int(rpart, &val) );
			THROW ( val <= 0, 10 ); /* Error: Invalid dimension */
			*output = val;
			return SUCCESS;
		}
	}
	TRHOW ( status > 0, status );

	return ERROR;
}

static int read_problem(FILE *fp, vtsp_points_t *output)
{
	char line[255];
	uint32_t len;
	int status;
	char *lpart, *rpart;
	uint32_t llen, rlen;
	int type;
	while ((status = get_line(fp, 255, line, &len)) == 0) {
		// PENDING: Operate depending on line type
	}
	TRHOW ( status > 0, status );

	return SUCCESS;
}

static int get_line(FILE *fp, uint32_t max_len, char *line, uint32_t *len)
{
	/* Max size must be 2 chars longer than max line,
	 * to handle 'new line' and null character.
	 */
	memset(line, 0, max_size);
	char *out = fgets(line, max_size, fp);

	if (out == 0) {
		if(feof(fp)) {
			return -1; /* End of file */
		} else {
			return 1; /* Error: while reading */
		}
	}

	*size = strlen(line) - 1;

	THROW( line[*size] != '\n', 2 ); /* Error: Line longer than max_size*/
	
	line[*size] = 0; /* Remove 'new line ' */
	return SUCCESS;
}

static int get_line_type(const char* line, uint32_t len, int *type)
{
	int len = sizeof(LINE_TYPES) / sizeof(line_t);
	int i = 0;

	for (i = 0; i < len; i++) {
		if (strncmp(LINE_TYPES[i].name, line, len) == 0) {
			*type = LINE_TYPES[i].type;
			return SUCCESS;
		}
	}
	
	*type = LINE_UNKNOWN;
	return SUCCESS;
}


static int split_str(char *line, uint32_t len, char sep,
		     char** lpart, uint32_t *llen,
		     char** rpart, uint32_t *rlen) {
	char *pch = strchr(line, sep);

	char *lp, *rp;
	uint32_t ll, rl;
	if (pch == 0) {
		lp = line;
		ll = len;
		rp = line + len;
		rl = 0;
	} else {
		lp = line;
		ll = (uint32_t)(pch - line);
		rp = pch + 1;
		rl = len - ll - 1;
	}
	TRY( trim_ref(lp, ll, lpart, llen) );
	TRY( trim_ref(rp, rl, rpart, rlen) );

	return SUCCESS;
}

static int trim_ref(char *line, uint32_t len, char** ref, uint32_t *ref_len)
{
	/* Remove heading and trailing blank spaces */
	int i = 0;
	while (is_blank(line[i]) && i < len) {
		i ++;
	}

	int j = len;
	while (j > i && is_blank(line[j - 1])) {
		j --; 
	}
	
	*ref = line + i;
	*ref_len = j - i;
	return SUCCESS;
}

static int is_blank(char c)
{
	return isspace(c);
}


static int parse_int(const char* input, int *output)
{
	*output = strtol(input, 0, 0);
	return SUCCESS;
}
