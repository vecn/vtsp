#include <ctype.h>
#include <stdbool.h>
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
	LINE_TOUR_SECTION,
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
	{LINE_NAME, "TOUR_SECTION"},
	{LINE_NAME, "EOF"}
};


static int read_problem_npts(FILE *fp, uint32_t *output);
static int read_problem(FILE *fp, vtsp_points_t *output);
static int read_tour(FILE *fp, vtsp_perm_t *output);
static int write_tour(FILE *fp, const char *name, const vtsp_perm_t *input);
static int get_line(FILE *fp, uint32_t max_len, char *line, uint32_t *len);
static int get_line_type(const char* line, uint32_t len, int *type);
static int get_string_of_line_type(int type, char *output, int max_len);
static int split_str(char *line, uint32_t len, char sep,
		     char** lpart, uint32_t *llen,
		     char** rpart, uint32_t *rlen);
static int trim_ref(char *line, uint32_t len, char** ref, uint32_t *ref_len);
static int is_blank(char c);
static int parse_int(const char* input, int *output);
static int parse_float(const char* input, float *output);
static int read_coordinates(const char *line, uint32_t len, vtsp_points_t *output,
			    int *pos);
static int read_tour_point(const char *line, uint32_t len, vtsp_perm_t *output,
			   int i, int *pos);
static int copy_str(const char* input, int input_len, char *output, int max_len);
static int read_problem_metadata(const char *line, uint32_t len,
				 bool *reading_coords, vtsp_points_t *output);
static int read_tour_metadata(const char *line, uint32_t len,
			      bool *reading_tour, vtsp_perm_t *output);
static int log_name(const char *name);
static int validate_type(const char* type, const char *expected_type);
static int log_comment(const char *comment);
static int get_problem_dimension(const char *dim, vtsp_points_t *output);
static int get_tour_dimension(const char *dim, vtsp_perm_t *output);
static int log_ignored_line(const char *line);
static int log_flush(FILE* fp, const char *msg);

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
	FILE *fp;
	TRY_PTR( fopen(input_filename, "r"), fp, ERROR_OPEN );

	int status = read_tour(fp, output);
	
	TRY_GOTO( fclose(fp), ERROR_CLOSE );

	return status;
ERROR_OPEN:
	TRY( log_flush(stderr, "Error opening file") );
	return ERROR;
ERROR_CLOSE:
	TRY( log_flush(stderr, "Error closing file") );
	return ERROR;
}

int vtsp_write_tour(const vtsp_perm_t *input, const char *output_filename)
{
	FILE *fp;
	TRY_PTR( fopen(output_filename, "w"), fp, ERROR_OPEN );

	int status = write_tour(fp, output_filename, input);
	
	TRY_GOTO( fclose(fp), ERROR_CLOSE );

	return status;
ERROR_OPEN:
	TRY( log_flush(stderr, "Error opening file") );
	return ERROR;
ERROR_CLOSE:
	TRY( log_flush(stderr, "Error closing file") );
	return ERROR;
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
	THROW ( status > 0, status );

	return ERROR;
}

static int read_problem(FILE *fp, vtsp_points_t *output)
{
	char line[255];
	uint32_t len;
	int status;
	int sum_check = 0;
	int n_read = 0;
	bool reading_coords = false;
	while ((status = get_line(fp, 255, line, &len)) == 0) {
		if (reading_coords) {
			int i;
			TRY( read_coordinates(line, len, output, &i) );

			n_read ++;
			sum_check += (i + 1);
			
			if (n_read >= output->num) {
				reading_coords = false;
			}
		} else {
			TRY( read_problem_metadata(line, len, &reading_coords,
						   output) );
		}
	}
	THROW( reading_coords, ERROR );
	THROW( status > 0, status );

	int expected_sum_check = (1 + output->num) * (output->num) / 2;
	THROW( sum_check != expected_sum_check, ERROR );

	return SUCCESS;
}


static int read_tour(FILE *fp, vtsp_perm_t *output)
{
	char line[255];
	uint32_t len;
	int status;
	int sum_check = 0;
	int i = 0;
	bool reading_tour = false;
	while ((status = get_line(fp, 255, line, &len)) == 0) {
		if (reading_tour) {
			int j = 0;
			TRY( read_tour_point(line, len, output, i, &j) );

			i ++;
			sum_check += (j + 1);
			if (i >= output-> num || j < 0) {
				reading_tour = false;
			}
		} else {
			TRY( read_tour_metadata(line, len, &reading_tour,
						output) );
		}
	}
	THROW( reading_tour, ERROR );
	THROW( status > 0, status );

	int expected_sum_check = (1 + output->num) * (output->num) / 2;
	THROW( sum_check != expected_sum_check, ERROR );

	return SUCCESS;
}

static int write_tour(FILE *fp, const char *name, const vtsp_perm_t *input)
{
	char tmp_str[100];
	int max_str_len = 99;
	/* Write name */
	TRY( get_string_of_line_type(LINE_NAME, tmp_str, max_str_len) );
	TRY_NONEG( fprintf(fp, "%s : %s\n", tmp_str, name), ERROR );
	
	/* Write comment */
	TRY( get_string_of_line_type(LINE_COMMENT, tmp_str, max_str_len) );
	const char *comment = "Optimal tour";
	TRY_NONEG( fprintf(fp, "%s : %s\n", tmp_str, comment), ERROR );

	/* Write type */
	TRY( get_string_of_line_type(LINE_TYPE, tmp_str, max_str_len) );
	const char *type = "TOUR";
	TRY_NONEG( fprintf(fp, "%s : %s\n", tmp_str, type), ERROR );
	
	/* Write dimension */
	TRY( get_string_of_line_type(LINE_DIMENSION, tmp_str, max_str_len) );
	TRY_NONEG( fprintf(fp, "%s : %u\n", tmp_str, input->num), ERROR );

	/* Write tour section */
	TRY( get_string_of_line_type(LINE_TOUR_SECTION, tmp_str, max_str_len) );
	TRY_NONEG( fprintf(fp, "%s\n", tmp_str), ERROR );

	int i;
	for (i = 0; i < input->num; i++) {
		TRY_NONEG( fprintf(fp, "%u\n", input->index[i]), ERROR );
	}
	TRY_NONEG( fprintf(fp, "%i\n", -1), ERROR );
	
	
	TRY( get_string_of_line_type(LINE_EOF, tmp_str, max_str_len) );
	TRY_NONEG( fprintf(fp, "%s", tmp_str), ERROR );

	return SUCCESS;
ERROR:
	return ERROR;
}

static int get_line(FILE *fp, uint32_t max_len, char *line, uint32_t *len)
{
	/* Max size must be 2 chars longer than max line,
	 * to handle 'new line' and null character.
	 */
	memset(line, 0, max_len);
	char *out = fgets(line, max_len, fp);

	if (out == 0) {
		if(feof(fp)) {
			return -1; /* End of file */
		} else {
			return 1; /* Error: while reading */
		}
	}

	*len = strlen(line) - 1;

	THROW( line[*len] != '\n', 2 ); /* Error: Line longer than max_size*/
	
	line[*len] = 0; /* Remove 'new line ' */
	return SUCCESS;
}

static int get_line_type(const char* line, uint32_t len, int *type)
{
	int types_len = sizeof(LINE_TYPES) / sizeof(line_t);
	int i = 0;

	for (i = 0; i < types_len; i++) {
		if (strncmp(LINE_TYPES[i].name, line, len) == 0) {
			*type = LINE_TYPES[i].type;
			return SUCCESS;
		}
	}
	
	*type = LINE_UNKNOWN;
	return SUCCESS;
}

static int get_string_of_line_type(int type, char *output, int max_len)
{
	int len = sizeof(LINE_TYPES) / sizeof(line_t);
	int i = 0;

	for (i = 0; i < len; i++) {
		if (type == LINE_TYPES[i].type) {
			strncpy(output, LINE_TYPES[i].name, max_len);
			output[max_len] = 0; /* Null terminate */
		}
	}
	output[0] = 0; /* Not found */
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

static int parse_float(const char* input, float *output)
{
	*output = strtof(input, 0);
	return SUCCESS;
}

static int read_coordinates(const char *line, uint32_t len, vtsp_points_t *output,
			    int *pos)
{
	char *lpart, *rpart;
	uint32_t llen, rlen;
	char tmp_str[20];

	/* Split "position coordinates" */
	TRY( split_str((char*)line, len, ' ', &lpart, &llen, &rpart, &rlen) );

	TRY( copy_str(lpart, llen, tmp_str, 19) );
	TRY( parse_int(tmp_str, pos) );
	*pos = (*pos) - 1;

	THROW( *pos >= output->num, ERROR );
	
	/* Split coordinates */
	char *xs, *ys;
	uint32_t lxs, lys;
	TRY( split_str(rpart, rlen, ' ', &xs, &lxs, &ys, &lys) );

	float x, y;
	TRY( copy_str(xs, lxs, tmp_str, 19) );
	TRY( parse_float(tmp_str, &x) );
	
	TRY( copy_str(ys, lys, tmp_str, 19) );
	TRY( parse_float(tmp_str, &y) );

	output->pts[*pos].x = x;
	output->pts[*pos].y = y;
	
	return SUCCESS;
}

static int read_tour_point(const char *line, uint32_t len, vtsp_perm_t *output,
			   int i, int *pos)
{
	char tmp_str[20];
	TRY( copy_str(line, len, tmp_str, 19) );
	TRY( parse_int(tmp_str, pos) );
	*pos = (*pos) - 1;

	THROW( *pos >= output->num, ERROR );

	if (*pos >= 0) {
		output->index[i] = *pos;
	}
	
	return SUCCESS;
}

static int copy_str(const char* input, int input_len, char *output, int max_len)
{
	THROW( input_len > max_len, ERROR );
	
	strncpy(output, input, input_len);
	output[input_len] = 0;

	return SUCCESS;
}

static int read_problem_metadata(const char *line, uint32_t len,
				 bool *reading_coords, vtsp_points_t *output)
{
	char *lpart, *rpart;
	uint32_t llen, rlen;
	int type;
	TRY( split_str((char*)line, len, ':', &lpart, &llen, &rpart, &rlen) );
	TRY( get_line_type(lpart, llen, &type) );
	switch (type) {
	case LINE_NAME:
		TRY( log_name(rpart) );
		break;
	case LINE_TYPE:
		TRY( validate_type(rpart, "TSP") );
		break;
	case LINE_COMMENT:
		TRY( log_comment(rpart) );
		break;
	case LINE_DIMENSION:
		TRY( get_problem_dimension(rpart, output) );
		break;
	case LINE_EDGE_WEIGHT_TYPE:
		TRY( validate_type(rpart, "EUC_2D") );
		break;
	case LINE_NODE_COORD_SECTION:
		*reading_coords = true;
		break;
	case LINE_EOF:
		*reading_coords = false;
		break;
	case LINE_UNKNOWN:
	default:
		TRY( log_ignored_line(line) );
		break;
	}
	return SUCCESS;
}


static int read_tour_metadata(const char *line, uint32_t len,
			      bool *reading_tour, vtsp_perm_t *output)
{
	char *lpart, *rpart;
	uint32_t llen, rlen;
	int type;
	TRY( split_str((char*)line, len, ':', &lpart, &llen, &rpart, &rlen) );
	TRY( get_line_type(lpart, llen, &type) );
	switch (type) {
	case LINE_NAME:
		TRY( log_name(rpart) );
		break;
	case LINE_TYPE:
		TRY( validate_type(rpart, "TOUR") );
		break;
	case LINE_COMMENT:
		TRY( log_comment(rpart) );
		break;
	case LINE_DIMENSION:
		TRY( get_tour_dimension(rpart, output) );
		break;
	case LINE_TOUR_SECTION:
		*reading_tour = true;
		break;
	case LINE_EOF:
		*reading_tour = false;
		break;
	case LINE_UNKNOWN:
	default:
		TRY( log_ignored_line(line) );
		break;
	}
	return SUCCESS;
}

static int log_name(const char *name)
{
	char message[255];
	TRY_NONEG( sprintf(message, "Reading TSP file: \"%s\"", name), ERROR);
	TRY( log_flush(stdout, message) );
	return SUCCESS;
ERROR:
	TRY( log_flush(stderr, "Error reading name") );
	return ERROR;
}

static int validate_type(const char* type, const char *expected_type)
{
	THROW( strcmp(type, expected_type) != 0, ERROR );
	return SUCCESS;
}

static int log_comment(const char *comment)
{
	char message[255];
	TRY_NONEG( sprintf(message, "Reading comment: \"%s\"", comment), ERROR);
	TRY( log_flush(stdout, message) );
	return SUCCESS;
ERROR:
	TRY( log_flush(stderr, "Error reading comment") );
	return ERROR;
}

static int get_problem_dimension(const char *dim, vtsp_points_t *output)
{
	int val;
	TRY( parse_int(dim, &val) );
	THROW( output->n_alloc < val, ERROR );
	output->num = val;
	return SUCCESS;
}

static int get_tour_dimension(const char *dim, vtsp_perm_t *output)
{
	int val;
	TRY( parse_int(dim, &val) );
	THROW( output->n_alloc < val, ERROR );
	output->num = val;
	return SUCCESS;
}


static int log_ignored_line(const char *line)
{
	char message[255];
	TRY_NONEG( sprintf(message, "Ignored line \"%s\"", line), ERROR);
	TRY( log_flush(stdout, message) );
	return SUCCESS;
ERROR:
	TRY( log_flush(stderr, "Error reading 'unknown' line") );
	return ERROR;
}

static int log_flush(FILE* fp, const char *msg)
{
	TRY_NONEG( fprintf(fp, "%s\n", msg), ERROR );
	TRY_GOTO( fflush(fp), ERROR );
	return SUCCESS;
ERROR:
	return ERROR;
}
