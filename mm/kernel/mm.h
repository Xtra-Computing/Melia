#ifndef MM
#define MM

#define DIM 2000

struct pos_t{
        unsigned int x;
        unsigned int y;
};

struct global_data_t{
        float * A;
        float * B;
        int dim;
};
#endif
