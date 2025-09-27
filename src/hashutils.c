#include "hashutils.h"

#include <assert.h>

utils_hash_t utils_djb2_hash(void* data, size_t length)
{
    assert(data != NULL);

    utils_hash_t hash = 0;
    char*        data_ptr = (char*) data;
    for(size_t ia = 0; ia < length; ++ia) {
        hash = (hash << 5) + hash + (utils_hash_t)(*data_ptr);
        ++data_ptr;
    }
    return hash;
}
