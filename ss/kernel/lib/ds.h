#ifndef OUTPUT
#define OUTPUT
struct output
{
	char *output_keys;
	char *output_vals;
	unsigned int *key_index;
	unsigned int *val_index;
};

struct ints
{
	short x;
	short y;
};

struct intl
{
	int x;
	int y;
};


typedef struct intl LONG_INT;

#endif