#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE   0x00000002
#define FDT_PROP       0x00000003
#define FDT_NOP        0x00000004
#define FDT_END        0x00000009

struct fdt_header {
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

static inline uint32_t bswap32(uint32_t x) {
    return __builtin_bswap32(x);
}

static inline uint64_t bswap64(uint64_t x) {
    return __builtin_bswap64(x);
}

static inline const void* align_up(const void* ptr, size_t align) {
    return (const void*)(((uintptr_t)ptr + align - 1) & ~(align - 1));
}

// FDT Header and structures as per spec
struct fdt_prop {
    uint32_t len;
    uint32_t nameoff;
};


/* * fdt_path_offset: Find a node's offset based on its full path string.
 * This version handles unit addresses (e.g., matching "memory" to "memory@80000000").
 */
int fdt_path_offset(const void* fdt, const char* path) {
    const struct fdt_header* hdr = (const struct fdt_header*)fdt;
    const char* struct_base = (const char*)fdt + bswap32(hdr->off_dt_struct);
    const char* struct_end  = struct_base + bswap32(hdr->size_dt_struct);

    const char* p = struct_base;
    char current_path[1024] = "";
    size_t path_len_stack[128]; // Tracks path length to backtrack on END_NODE
    int depth = 0;

    // Handle root path case
    if (strcmp(path, "/") == 0) return 0;

    while (p < struct_end) {
        const char* token_pos = p;
        uint32_t token = bswap32(*(const uint32_t*)p);
        p += sizeof(uint32_t);

        switch (token) {
        case FDT_BEGIN_NODE: {
            const char* name = p;
            size_t name_len = strlen(name);
            path_len_stack[depth] = strlen(current_path);

            // 1. Build the path string for the current node
            if (depth == 0) { // Root node
                strcpy(current_path, "/");
            } else {
                if (strcmp(current_path, "/") != 0) strcat(current_path, "/");
                strcat(current_path, name);
            }

            // 2. Exact Match Check
            if (strcmp(current_path, path) == 0) {
                return (int)(token_pos - struct_base);
            }

            // 3. Unit Address Match Check (e.g., match "/memory" if node is "/memory@0")
            char* at_ptr = strchr(current_path, '@');
            if (at_ptr) {
                *at_ptr = '\0'; // Temporarily truncate
                int match = (strcmp(current_path, path) == 0);
                *at_ptr = '@'; // Restore
                if (match) return (int)(token_pos - struct_base);
            }

            depth++;
            p = (const char*)align_up(name + name_len + 1, 4);
            break;
        }

        case FDT_END_NODE:
            if (depth <= 0) return -1;
            depth--;
            // Backtrack the path string to the parent's level
            current_path[path_len_stack[depth]] = '\0';
            if (current_path[0] == '\0' && depth == 0) strcpy(current_path, "/");
            break;

        case FDT_PROP: {
            uint32_t len = bswap32(*(const uint32_t*)p);
            p += sizeof(struct fdt_prop); // Skip len and nameoff
            p += len;                     // Skip the data
            p = (const char*)align_up(p, 4);
            break;
        }

        case FDT_NOP:
            break;

        case FDT_END:
            return -1;

        default:
            return -1;
        }
    }
    return -1;
}

/* * fdt_getprop: Retrieve a property pointer and length from a specific node offset.
 */
const void* fdt_getprop(const void* fdt, int nodeoffset, const char* name, int* lenp) {
    const struct fdt_header* hdr = (const struct fdt_header*)fdt;
    const char* struct_base = (const char*)fdt + bswap32(hdr->off_dt_struct);
    const char* strings_ptr = (const char*)fdt + bswap32(hdr->off_dt_strings);
    
    const char* p = struct_base + nodeoffset;
    
    // Ensure we are starting at a node
    if (bswap32(*(const uint32_t*)p) != FDT_BEGIN_NODE) return NULL;
    p += 4;
    p = (const char*)align_up(p + strlen(p) + 1, 4); // Skip node name

    while (1) {
        uint32_t tag = bswap32(*(const uint32_t*)p);
        p += 4;

        if (tag == FDT_PROP) {
            uint32_t len = bswap32(*(const uint32_t*)p);
            uint32_t nameoff = bswap32(*(const uint32_t*)(p + 4));
            p += 8; // skip len and nameoff

            if (strcmp(strings_ptr + nameoff, name) == 0) {
                if (lenp) *lenp = (int)len;
                return (const void*)p;
            }
            p = (const char*)align_up(p + len, 4);
        } else if (tag == FDT_NOP) {
            continue;
        } else {
            // New node or end of node means properties section is over
            break;
        }
    }
    return NULL;
}

int main() {
    /* Prepare the device tree blob */
    FILE* fp = fopen("qemu.dtb", "rb");
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    void* fdt = malloc(sz);
    fseek(fp, 0, SEEK_SET);
    if (fread(fdt, 1, sz, fp) != sz) {
        fprintf(stderr, "Failed to read the device tree blob\n");
        free(fdt);
        fclose(fp);
        return EXIT_FAILURE;
    }
    fclose(fp);

    /* Find the node offset */
    int offset = fdt_path_offset(fdt, "/cpus/cpu@0/interrupt-controller");
    if (offset < 0) {
        fprintf(stderr, "fdt_path_offset\n");
        free(fdt);
        return EXIT_FAILURE;
    }

    /* Get the node property */
    int len;
    const void* prop = fdt_getprop(fdt, offset, "compatible", &len);
    if (!prop) {
        fprintf(stderr, "fdt_getprop\n");
        free(fdt);
        return EXIT_FAILURE;
    }
    printf("compatible: %.*s\n", len, (const char*)prop);

    offset = fdt_path_offset(fdt, "/memory");
    prop = fdt_getprop(fdt, offset, "reg", &len);
    const uint64_t* reg = (const uint64_t*)prop;
    printf("memory: base=0x%lx size=0x%lx\n", bswap64(reg[0]), bswap64(reg[1]));

    offset = fdt_path_offset(fdt, "/chosen");
    prop = fdt_getprop(fdt, offset, "linux,initrd-start", &len);
    const uint64_t* initrd_start = (const uint64_t*)prop;
    printf("initrd-start: 0x%lx\n", bswap64(initrd_start[0]));

    free(fdt);
    return 0;
}
