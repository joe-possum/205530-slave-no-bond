/**
 * Allocation pool types
 */
enum bg_pool_object{
  bg_pool_object_hci,           // 0
  bg_pool_object_bgbuf,         // 1
  bg_pool_object_acl,           // 2
  bg_pool_object_bgbuf_small,   // 3
  bg_pool_object_hci_small,     // 4
  bg_pool_object_ll_connection,  //5
  bg_pool_object_ll_adv_set,    // 6
  bg_pool_object_gap_context,   // 7
  bg_pool_object_ll_validator,  // 8
  bg_pool_object_mesh_prov_db,  // 9
  bg_pool_object_bg_message,    // 10
  bg_pool_object_endpoint,      // 11
  bg_pool_object_soft_timer_message, // 12
  bg_pool_object_dfu,           // 13
  bg_pool_object_foundation_cmd,//14
  bg_pool_object_ll_adv_sync,   //15
  bg_pool_object_ll_scan_sync,  //16
  bg_pool_object_gap_periodic_adv,  //17
  bg_pool_object_gap_periodic_sync, //18
  bg_pool_object_l2cap_coc_channel, //19
  bg_pool_object_mesh_prov_db_appkey, // 20
  bg_pool_object_max
};

struct bg_pool_alloc_data{
  struct bg_pool_alloc_data *next;
  uint8_t rest_of_data[];
};

/**
 * Memory allocation pool, stores fixed size memory buffers
 */
struct bg_pool_alloc{
  uint16_t    object_size;
  uint16_t    free;
  uint8_t    *buffer;
  uint16_t    size;
#ifdef BG_DEBUG
  uint16_t    guard_index;
#endif

  struct bg_pool_alloc_data  *head;
};

/**
 * array of different size of memory pools in ascending order
 */
extern struct bg_pool_alloc bg_pool_pools[bg_pool_object_max];
