#define HASH_LEN 64
#define SIGN_LEN 64

typedef struct {
    int number;
    int time;
    char previous_hash[HASH_LEN];
    int data_len;
    char *data;
} block_data_t;

typedef struct {
    size_t sign_len;
    char *sign;
} sign_t;

typedef struct {
    block_data_t block_data;
    sign_t sign;
} sign_block_t;