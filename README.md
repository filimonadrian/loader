Name: Filimon Adrian
Group: 334CC

# Third Homework - Executables loader


## Code Organization

For simplicity, I've choosed to store in `void *data` field from `struct so_seg_t` an array of bools, which represents if the page is valid. 
This aray contains entries for all pages and is modified when a new page is mapped.
The allocation for valid_pages array:
```
segment->data = calloc(0, nr_pages * sizeof(int));
```

All page mapping logic is implemented in `sig_handler` function.  
As an overview, this is what sig_handler function does:
- check every segment and extract array with valid pages from data field.
- if the segmentation fault address is in this segment, then need to check all pages of this segment
- the smarter solution was to calculate directy page_index using start_segment address, segfault address and the dimension of a page.
- **if** the file_size is smaller than page_address, we need to map a new page in RAM **else** we will write the data in the page with segment permissions


## Implementation

- I do not know why, but the last test fails. I think there is a little catch, but i did not discovered it yet.

## Usage
- The exposed interface of the library is present in loader.h header. This contains functions for loader initialization and binary execution.

- For using the `libso_loader.so` library in the project, you need to add `loader.h` header in your source file and specify the path to the libso_loader.so.
- if you want to generate `libso_loader.so` library, run in a terminal:
```
make build
```

## Bibliography
- [Laboratory 6](https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-06)
- [Linux Manual Page](https://man7.org/linux/man-pages)