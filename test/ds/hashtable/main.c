#include <poglib/basic.h> 

arena_t *global_arena = NULL;

// ---------------------------------------------------------
// TEST 1: The Prefix Collision Bug
// ---------------------------------------------------------
void test_string_prefix_collision() {
    printf("Test 1: String Prefix Collision ('app' vs 'apple')...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_STR, u32, global_arena);

    hashtable_key_t key1 = { .str = str("app") };
    hashtable_key_t key2 = { .str = str("apple") };
    
    u32 val1 = 100;
    u32 val2 = 200;

    // Insert 'app'
    hashtable_insert(&ht, key1, val1);
    // Insert 'apple'
    hashtable_insert(&ht, key2, val2);

    // Retrieve 'app'
    const u32 retrieved_val1 = (const u32)hashtable_get_value(&ht, key1);
    
    assert(retrieved_val1 != NULL);
    
    // EXPECTED FAILURE: 
    // This will fail because your current strncmp implementation 
    // only checks up to the length of 'app' (3), meaning 'apple' overwrites it.
    if (retrieved_val1 != 100) {
        printf("  -> [FAILED] Prefix bug detected. Expected 100, got %u\n", retrieved_val1);
    } else {
        printf("  -> [PASS]\n");
    }

    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 2: Inline Update Memory Corruption
// ---------------------------------------------------------
void test_inline_update_corruption() {
    printf("Test 2: Inline Update Memory Corruption...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_STR, u32, global_arena);

    hashtable_key_t key = { .str = str("score") };
    
    u32 val = 50;
    hashtable_insert(&ht, key, val);

    // Create a new scope to simulate a temporary stack variable updating the table
    {
        u32 temporary_val = 999;
        hashtable_insert(&ht, key, temporary_val);
    } // temporary_val is destroyed here

    // Retrieve the value
    const u32 retrieved_val = (u32)hashtable_get_value(&ht, key);
    
    assert(retrieved_val != NULL);

    // EXPECTED FAILURE:
    // This will likely print garbage memory instead of 999 because your existing code 
    // assigns the pointer to the stack variable (entry->ptr = value_addr) 
    // instead of copying the value into the union when updating an inline type.
    if (retrieved_val != 999) {
        printf("  -> [FAILED] Memory corruption detected. Expected 999, got %u\n", retrieved_val);
    } else {
        printf("  -> [PASS]\n");
    }

    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 3: Unsafe 8-Byte Cast on Initialization
// ---------------------------------------------------------
void test_unsafe_8_byte_cast() {
    printf("Test 3: Unsafe 8-Byte Read on 4-Byte Value...\n");
    
    // Initialize a table for 4-byte integers (u32)
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_U32, u32, global_arena);

    hashtable_key_t key = { .u32 = 1001 };
    
    // Allocate exactly 4 bytes on the heap to prevent stack padding from saving the cast
    u32 *exact_4_bytes = malloc(sizeof(u32));
    *exact_4_bytes = 42;

    // EXPECTED FAILURE / SEGFAULT:
    // This may crash with an Address Sanitizer (ASAN) or return a corrupt value 
    // because your code does `.ptr = *(void **)value_addr;`, reading 8 bytes from a 4-byte allocation.
    hashtable_insert(&ht, key, *exact_4_bytes);

    const u32 retrieved = (const u32)hashtable_get_value(&ht, key);
    
    if (retrieved != 42) {
        printf("  -> [FAILED] 8-byte cast corrupted the read. Expected 42, got %u\n", retrieved);
    } else {
        printf("  -> [PASS]\n");
    }

    free(exact_4_bytes);
    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 4: Deletion Shift-Back Logic (Tombstone Prevention)
// ---------------------------------------------------------
void test_deletion_shift_back() {
    printf("Test 4: Deletion and Shift-Back Continuity...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_U32, u32, global_arena);

    // Insert 8 elements to create a contiguous probe chain
    for (u32 i = 1; i <= 8; i++) {
        hashtable_key_t key = { .u32 = i };
        hashtable_insert(&ht, key, i * 10);
    }

    // Delete items from the middle of the chain
    hashtable_delete(&ht, (hashtable_key_t){ .u32 = 3 });
    hashtable_delete(&ht, (hashtable_key_t){ .u32 = 4 });

    // EXPECTED FAILURE (If shift logic is broken):
    // If the backward shift fails to properly update probe distances or leaves an empty gap,
    // the linear probing for element 8 will hit an empty slot and prematurely return NULL.
    bool pass = true;
    for (u32 i = 1; i <= 8; i++) {
        if (i == 3 || i == 4) continue; // Skip deleted

        const u32 val = (const u32)hashtable_get_value(&ht, (hashtable_key_t){ .u32 = i });
        if (!val || val != i * 10) {
            pass = false;
            printf("  -> [FAILED] Chain broken. Could not retrieve key %u after deletions.\n", i);
            break;
        }
    }

    if (pass) printf("  -> [PASS]\n");
    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 5: Large Structs (Pointer Mode Integrity)
// ---------------------------------------------------------
typedef struct {
    f32 data[16]; // 64 bytes - forces VALUE_MODE_POINTER
} test_large_struct_t;

void test_large_struct_pointer_mode() {
    printf("Test 5: Large Struct (Pointer Mode) Integrity...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_STR, test_large_struct_t, global_arena);

    hashtable_key_t key = { .str = str("matrix_A") };
    
    // Allocate struct and set identifiable data
    test_large_struct_t my_data = {0};
    my_data.data[0] = 3.14f;
    my_data.data[15] = 9.99f;

    // _Generic macro should route this to hashtable__internal_insert_ptr
    hashtable_insert(&ht, key, &my_data); 

    const test_large_struct_t *retrieved = (const test_large_struct_t *)hashtable_get_value(&ht, key);

    // EXPECTED FAILURE:
    // If the struct is accidentally copied inline, it truncates to 8 bytes.
    // Reading data[15] will result in a segfault or garbage memory.
    if (retrieved == NULL) {
        printf("  -> [FAILED] Retrieved NULL.\n");
    } else if (retrieved->data[0] != 3.14f || retrieved->data[15] != 9.99f) {
        printf("  -> [FAILED] Pointer mode corrupted or truncated large struct.\n");
    } else {
        printf("  -> [PASS]\n");
    }

    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 6: High Load Robin Hood Swapping
// ---------------------------------------------------------
void test_high_load_probing() {
    printf("Test 6: High Load Robin Hood Probing...\n");
    
    // Capacity 16. Insert 15 items to saturate the table and force massive probe chains.
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_U32, u32, global_arena);
    
    bool pass = true;
    for (u32 i = 1; i <= 15; i++) {
        hashtable_key_t key = { .u32 = i * 100 };
        hashtable_insert(&ht, key, i * 5);
    }

    // EXPECTED FAILURE:
    // If the swap_memory logic for key, value, and probe_distance inside the Robin Hood 
    // insertion loop is misaligned, keys will overwrite each other under heavy load.
    for (u32 i = 1; i <= 15; i++) {
        hashtable_key_t key = { .u32 = i * 100 };
        const u32 val = (const u32)hashtable_get_value(&ht, key);
        if (val != i * 5) {
            printf("  -> [FAILED] Lost or corrupted key %u under high collision load.\n", key.u32);
            pass = false;
            break;
        }
    }
    
    if (pass) printf("  -> [PASS]\n");
    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 7: Signed 32-bit Integer (i32) Sign-Extension
// ---------------------------------------------------------
void test_i32_negative_sign_extension() {
    printf("Test 7: i32 Negative Value Sign-Extension...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_STR, i32, global_arena);

    hashtable_key_t key = { .str = str("x_offset") };
    i32 original_val = -12345;

    hashtable_insert(&ht, key, original_val);

    // Retrieve and carefully cast back down through uintptr_t to prevent compiler warnings
    // and check if the raw bit pattern was preserved.
    const i32 retrieved = (i32)hashtable_get_value(&ht, key);

    // EXPECTED FAILURE:
    // If the i32 to void* cast lacked the intermediate unsigned 64-bit bridge, 
    // the negative sign bit propagates through the upper 32 bits, altering the value.
    if (retrieved != original_val) {
        printf("  -> [FAILED] i32 sign-extension corrupted data. Expected %d, got %d\n", original_val, retrieved);
    } else {
        printf("  -> [PASS]\n");
    }

    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 8: Unsigned 32-bit Integer (u32) Boundary Integrity
// ---------------------------------------------------------
void test_u32_max_boundary() {
    printf("Test 8: u32 MAX Boundary Integrity...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_STR, u32, global_arena);

    hashtable_key_t key = { .str = str("max_score") };
    u32 original_val = 0xFFFFFFFF; // 4,294,967,295 (Max u32 value)

    hashtable_insert(&ht, key, original_val);

    const u32 retrieved = (u32)hashtable_get_value(&ht, key);

    // EXPECTED FAILURE:
    // If the data was stored using a signed 64-bit bridge (i64) instead of u64, 
    // the highest bit of the u32 (which is 1) triggers sign extension, filling the 
    // top 32 bits with 1s and corrupting the strict unsigned 32-bit representation.
    if (retrieved != original_val) {
        printf("  -> [FAILED] u32 MAX boundary corrupted. Expected %u, got %u\n", original_val, retrieved);
    } else {
        printf("  -> [PASS]\n");
    }

    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 9: Bug 1 - Robin Hood Key Obliteration (Data Loss)
// ---------------------------------------------------------
void test_bug1_robin_hood_key_obliteration() {
    printf("Test 9: Bug 1 - Robin Hood Key Obliteration...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_U32, u32, global_arena);

    // Force a heavily congested table to guarantee multiple Robin Hood swaps.
    // Capacity is 16, inserting 15 ensures long probe chains and displacement.
    for (u32 i = 1; i <= 15; i++) {
        hashtable_insert(&ht, (hashtable_key_t){ .u32 = i }, i * 10);
    }

    bool pass = true;
    u32 missing_keys = 0;

    for (u32 i = 1; i <= 15; i++) {
        const u32 val = (u32)(uintptr_t)hashtable_get_value(&ht, (hashtable_key_t){ .u32 = i });
        if (val != i * 10) {
            missing_keys++;
            pass = false;
        }
    }

    // EXPECTED FAILURE:
    // If `memcpy` is used instead of `swap_memory` for the keys inside the Robin Hood
    // displacement loop, displaced entries inherit the key of the inserting element. 
    // The original keys vanish entirely, resulting in missing_keys > 0.
    if (!pass) {
        printf("  -> [FAILED] %u keys were completely obliterated during Robin Hood swaps.\n", missing_keys);
    } else {
        printf("  -> [PASS]\n");
    }

    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// TEST 10: Bug 2 - Shift-Delete State Desync
// ---------------------------------------------------------
void test_bug2_shift_delete_desync() {
    printf("Test 10: Bug 2 - Shift-Delete State Desync...\n");
    hashtable_t ht = hashtable_init(16, HT_KEY_TYPE_U32, u32, global_arena);

    // 1. Insert 5 elements to create a localized probe chain
    for (u32 i = 1; i <= 5; i++) {
        hashtable_insert(&ht, (hashtable_key_t){ .u32 = i }, i * 10);
    }

    // 2. Delete the first element. This forces the remaining elements to shift backward.
    // The hashtable manually sets the tail end of the shift to `is_occupied = false`.
    hashtable_delete(&ht, (hashtable_key_t){ .u32 = 1 });

    // 3. Count the actual raw `true` booleans in the slot array's internal tracking
    u32 actual_occupied_slots = 0;
    for (u32 i = 0; i < ht.entries.internal.capacity; i++) {
        if (ht.entries.internal.index_table[i]) {
            actual_occupied_slots++;
        }
    }

    // EXPECTED FAILURE:
    // `ht.entries.len` will be 4 (because `slot_delete` was called once).
    // However, because the backward shift bypassed the slot API to clear the tail,
    // `actual_occupied_slots` will still be 5. 
    // This desync means the next time `slot_insert` tries to write to that tail slot, 
    // it will throw a fatal error because the index_table incorrectly claims it is full.
    if (actual_occupied_slots != ht.entries.len) {
        printf("  -> [FAILED] State desync! Slot len is %lu, but %u indices are marked true.\n", ht.entries.len, actual_occupied_slots);
    } else {
        printf("  -> [PASS]\n");
    }

    hashtable_destroy(&ht);
}

// ---------------------------------------------------------
// Main Test Runner
// ---------------------------------------------------------
int main(void) {
    printf("========================================\n");
    printf("   RUNNING HASHTABLE REGRESSION TESTS   \n");
    printf("========================================\n\n");

    runtimectx_init();
    arena_t *local_arena = arena_init(NULL, 1 * GB);
    global_arena = local_arena;
    
    test_string_prefix_collision();
    printf("\n");
    
    test_inline_update_corruption();
    printf("\n");
    
    test_unsafe_8_byte_cast();
    printf("\n");

    test_deletion_shift_back();
    printf("\n");

    test_large_struct_pointer_mode();
    printf("\n");

    test_high_load_probing();
    printf("\n");

    test_i32_negative_sign_extension();
    printf("\n");

    test_u32_max_boundary();
    printf("\n");

    test_bug1_robin_hood_key_obliteration();
    printf("\n");

    test_bug2_shift_delete_desync();
    printf("\n");

    printf("========================================\n");
    runtimectx_destroy();
    return 0;
}
