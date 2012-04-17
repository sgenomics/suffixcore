#ifndef GLOBALDEFS
#define GLOBALDEFS

#define suffixnodestore_type SuffixNodeStoreMemVec

//define one of these to delect a storage mode
#define object_store_type ObjectStoreMemVec

//define one of these to select a memory allocator
//#define use_malloc 1
#define use_tialloc 1

#define alphabet_size  255
#define symbol_size  253
#define final_symbol 254

#define store_type string

#endif
